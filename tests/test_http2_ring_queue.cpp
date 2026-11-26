/*
 * test_http2_ring_queue.cpp — http2 发送环行为契约单测
 * 契约：push 必须把帧字节完整放进槽（拷贝两版深拷贝、移动版移交缓冲），
 * 入环成功后源内存可被改写/回收/释放；移动版失败时源原样保留可重试。
 * 契约来源：vendor/httpserver/include/http2_ring_queue.h（push 的入环要求）。
 * 输出：每组一行 PASS/FAIL，末行 SUMMARY；全过退出码 0，任一失败退出码 1。
 */
#include <iostream>
#include <string>
#include <vector>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include "http2_ring_queue.h"

static int g_failures = 0;
static int g_groups   = 0;

static void group_result(const char *name, bool ok)
{
    g_groups++;
    if (ok)
    {
        std::cout << "PASS: " << name << std::endl;
    }
    else
    {
        g_failures++;
        std::cout << "FAIL: " << name << std::endl;
    }
}

int main()
{
    // 组1: string 版 push 后改写源内容，pop 必须仍得原值（深拷贝）。
    {
        http::http2_send_queue_cache ring;
        std::string src = "src-1";
        ring.push(src);
        src.assign("mutated");
        std::string out;
        bool ok = ring.pop(out) && out == "src-1";
        group_result("deep-copy: string source mutated after push", ok);
    }

    // 组2: string 版 push 后源被回收（clear+swap 释放堆缓冲），pop 必须完好。
    {
        http::http2_send_queue_cache ring;
        std::string src(4096, 'x');
        ring.push(src);
        std::string().swap(src);
        std::string out;
        bool ok = ring.pop(out) && out.size() == 4096 && out[0] == 'x' && out[4095] == 'x';
        group_result("deep-copy: string source recycled after push", ok);
    }

    // 组3: buffer 指针版 push 后改写缓冲，pop 必须仍得原字节。
    {
        http::http2_send_queue_cache ring;
        unsigned char buf[5] = {1, 2, 3, 4, 5};
        ring.push(buf, 5);
        buf[0] = 99;
        std::string out;
        bool ok = ring.pop(out) && out.size() == 5 && out[0] == 1 && out[4] == 5;
        group_result("deep-copy: byte buffer mutated after push", ok);
    }

    // 组4: FIFO 顺序 + head/tail 回绕（8 轮 × 7 = 56 帧，跨环容量回绕）。
    {
        http::http2_send_queue_cache ring;
        bool ok = true;
        for (int round = 0; round < 8 && ok; round++)
        {
            for (int i = 0; i < 7; i++)
            {
                ok = ring.push("v" + std::to_string(round * 7 + i));
            }
            for (int i = 0; i < 7 && ok; i++)
            {
                std::string out;
                ok = ring.pop(out) && out == "v" + std::to_string(round * 7 + i);
            }
        }
        group_result("fifo order across wraparound", ok);
    }

    // 组5: 满环 capacity_-1 帧；再 push 一帧拒收；腾一格后恢复；按序取空后 pop 失败。
    {
        http::http2_send_queue_cache ring;
        bool ok           = true;
        const int usable  = static_cast<int>(ring.capacity_) - 1;
        for (int i = 0; i < usable; i++)
        {
            ok = ok && ring.push("f" + std::to_string(i));
        }
        ok = ok && ring.has_size() == usable;

        std::string out;
        ok = ok && !ring.push("f" + std::to_string(usable));
        ok = ok && ring.pop(out) && out == "f0";
        ok = ok && ring.push("f" + std::to_string(usable + 1));
        ok = ok && ring.has_size() == usable;

        for (int i = 1; i < usable && ok; i++)
        {
            ok = ring.pop(out) && out == "f" + std::to_string(i);
        }
        ok = ok && ring.pop(out) && out == "f" + std::to_string(usable + 1);
        ok = ok && !ring.pop(out);
        group_result("full ring: capacity_-1 slots, next rejected, drain then empty", ok);
    }

    // 组6: reset() 清空槽内容（大帧走交换释放分支），has_size 归零。
    {
        http::http2_send_queue_cache ring;
        ring.push(std::string(200000, 'y'));
        ring.reset();
        bool ok = ring.has_size() == 0 && ring.data[0].size() == 0;
        group_result("reset clears slots and size", ok);
    }

    // 组7: move 版 push 成功移交缓冲（源变空、pop 得原值）；满环拒收时源原样保留，
    // 腾一格后重试成功。
    {
        http::http2_send_queue_cache ring;
        std::string src(4096, 'm');
        bool ok = ring.push(std::move(src));
        ok      = ok && src.empty();
        std::string out;
        ok = ok && ring.pop(out) && out.size() == 4096 && out[0] == 'm' && out[4095] == 'm';

        const int usable = static_cast<int>(ring.capacity_) - 1;
        for (int i = 0; i < usable && ok; i++)
        {
            ok = ring.push("f" + std::to_string(i));
        }
        std::string keep(64, 'k');
        ok = ok && !ring.push(std::move(keep));
        ok = ok && keep.size() == 64 && keep[0] == 'k' && keep[63] == 'k';
        std::string tmp;
        ok = ok && ring.pop(tmp);
        ok = ok && ring.push(std::move(keep));
        ok = ok && keep.empty();
        group_result("move push: success empties source, full-ring rejection keeps source", ok);
    }

    // 组8: 完成计数归零才唤醒等待者，且唤醒投递到等待协程的 executor。
    {
        asio::io_context io;
        auto completion = std::make_shared<http::h2_borrow_state>(io.get_executor());
        completion->add(2);
        bool resumed = false;
        bool value   = false;
        asio::co_spawn(io,
                       [completion, &resumed, &value]() -> asio::awaitable<void>
                       {
                           value   = co_await completion->wait();
                           resumed = true;
                       },
                       asio::detached);
        io.poll();
        completion->release();
        io.restart();
        io.poll();
        bool first = !resumed;
        completion->release();
        io.restart();
        io.poll();
        group_result("borrow completion wakes after final release", first && resumed && value);
    }

    // 组9: 消费者退出时 cancel 能唤醒等待者，并报告失败。
    {
        asio::io_context io;
        auto completion = std::make_shared<http::h2_borrow_state>(io.get_executor());
        completion->add(1);
        bool resumed = false;
        bool value   = true;
        asio::co_spawn(io,
                       [completion, &resumed, &value]() -> asio::awaitable<void>
                       {
                           value   = co_await completion->wait();
                           resumed = true;
                       },
                       asio::detached);
        io.poll();
        completion->cancel();
        io.restart();
        io.poll();
        group_result("borrow cancellation wakes waiter", resumed && !value);
    }

    // 组10: 消费者取消后，迟到注册被拒绝，迟到释放不会破坏计数。
    {
        asio::io_context io;
        auto completion = std::make_shared<http::h2_borrow_state>(io.get_executor());
        completion->cancel();
        bool ok = !completion->add(2);
        completion->release(2);
        ok = ok && !completion->add(1);
        group_result("cancelled borrow rejects late add and release", ok);
    }

    if (g_failures == 0)
    {
        std::cout << "SUMMARY: PASS (" << g_groups << "/" << g_groups << ")" << std::endl;
        return 0;
    }
    std::cout << "SUMMARY: FAIL (" << g_failures << "/" << g_groups << " groups failed)" << std::endl;
    return 1;
}
