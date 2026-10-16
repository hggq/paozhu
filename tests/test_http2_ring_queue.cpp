/*
 * test_http2_ring_queue.cpp — http2 发送环行为契约单测
 * 契约：两个 push 必须把帧字节深拷贝进槽，入环成功后源内存可被改写/回收/释放。
 * 契约来源：vendor/httpserver/include/http2_ring_queue.h（push 的深拷贝要求）。
 * 输出：每组一行 PASS/FAIL，末行 SUMMARY；全过退出码 0，任一失败退出码 1。
 */
#include <iostream>
#include <string>
#include <vector>
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

    // 组4: FIFO 顺序 + head/tail 回绕（4 轮 × 7 = 28 帧，跨 16 槽回绕两圈）。
    {
        http::http2_send_queue_cache ring;
        bool ok = true;
        for (int round = 0; round < 4 && ok; round++)
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

    // 组5: 满环 15 帧；第 16 帧拒收；腾一格后恢复；按序取空后 pop 失败。
    {
        http::http2_send_queue_cache ring;
        bool ok = true;
        for (int i = 0; i < 15; i++)
        {
            ok = ok && ring.push("f" + std::to_string(i));
        }
        ok = ok && ring.has_size() == 15;

        std::string out;
        ok = ok && !ring.push(std::string("f15"));
        ok = ok && ring.pop(out) && out == "f0";
        ok = ok && ring.push(std::string("f16"));
        ok = ok && ring.has_size() == 15;

        for (int i = 1; i < 15 && ok; i++)
        {
            ok = ring.pop(out) && out == "f" + std::to_string(i);
        }
        ok = ok && ring.pop(out) && out == "f16";
        ok = ok && !ring.pop(out);
        group_result("full ring: 15 slots, 16th rejected, drain then empty", ok);
    }

    // 组6: reset() 清空槽内容（大帧走交换释放分支），has_size 归零。
    {
        http::http2_send_queue_cache ring;
        ring.push(std::string(200000, 'y'));
        ring.reset();
        bool ok = ring.has_size() == 0 && ring.data[0].size() == 0;
        group_result("reset clears slots and size", ok);
    }

    if (g_failures == 0)
    {
        std::cout << "SUMMARY: PASS (" << g_groups << "/" << g_groups << ")" << std::endl;
        return 0;
    }
    std::cout << "SUMMARY: FAIL (" << g_failures << "/" << g_groups << " groups failed)" << std::endl;
    return 1;
}
