/*
 * Redis pubsub 协程版框架级验证订阅（+ 自验证测试）
 *
 * 线程模型：
 *   pump 回调  → co_spawn(io_ctx, async_on_message) → io_context 新协程
 *   钩子      → co_await async_on_subscribe_ok / async_on_message（本协程里）
 *
 * 自验证流程：on_subscribe_ok 时 test_and_set 抢本实例旗 → 跑 self_test PUBLISH 10 轮 →
 * 这条自验证协程结束时（含异常退出）放旗。重连/重订阅后可以重新抢旗再跑一轮。
 *
 * test_publisher_ 是**本 client 实例**成员旗：保护 active_sub_ 对应的 subscriber socket
 * 不被 self_test 的 async_publish 和将来业务在 pump 回调里的 publish 竞争。
 * 断连**不**放旗：持旗的自验证协程还在跑，此时放旗等于允许第二次订阅并发进同一条 socket 的写路径。
 */
#pragma once

#include "redis_subpub.h"
#include "redis_pool.h"
#include <atomic>
#include <cstdio>
#include <string>

namespace redis_framework_test
{

// 本实例的写路径保护旗：同一份订阅对象上只允许一条自验证协程进 async_publish
class echo_subscriber_co : public pz::redis::redis_subpub_client
{
  public:
    echo_subscriber_co()
    {
        is_coroutine_ = true;
        durtime       = 3;
        loop_num      = 999999;
    }

    std::string section_name() const override { return "default"; }
    std::vector<std::string> channels() const override { return {"echo_co_test"}; }
    std::vector<std::string> patterns() const override { return {"stock.*"}; }

    asio::awaitable<void> async_on_subscribe_ok() override
    {
        fprintf(stderr, "[echo_co_sub] ✅ SUBSCRIBE OK  channels=%zu patterns=%zu\n", channels().size(), patterns().size());

        if (!io_ctx)
            co_return;

        // 抢全局旗：只有一个实例能进入写路径（self_test 里 async_publish）
        if (!test_publisher_.test_and_set())// 之前是 clear → 抢到旗
        {
            auto base = shared_from_this();
            auto self = std::dynamic_pointer_cast<echo_subscriber_co>(base);
            if (self)
            {
                asio::co_spawn(*io_ctx, [self]() -> asio::awaitable<void>
                               {
                                    // 放旗只在这一处、且不管自验证是正常跑完还是抛出来：
                                    // 抢旗的是这条协程，只有它知道自己什么时候不再写 socket。
                                    // timer 被取消（停服/strand 撤销）会从 co_await 抛出，
                                    // 那种时候漏放就会把旗永久卡住。
                                    try
                                    {
                                        co_await self->run_self_test();
                                    }
                                    catch (...)
                                    {
                                    }
                                    self->test_publisher_.clear();
                                    co_return; },
                               asio::detached);
            }
        }
        co_return;
    }

    asio::awaitable<void> async_on_disconnect() override
    {
        fprintf(stderr, "[echo_co_sub] DISCONNECT, reconnecting in 3s\n");
        // 这里不放旗。持旗的自验证协程可能还在跑（run_self_test 看到 isclose 会自己收手，
        // 收手时由它放），此处再 clear 一次等于把别人正持着的旗交出去 —— 下一次订阅就能
        // 并发进同一条 subscriber socket 的写路径。
        co_return;
    }

    asio::awaitable<void> async_on_message(
        const std::string &channel, const std::string &payload) override
    {
        fprintf(stderr, "[echo_co_sub] 📨  MSG  channel=%-15s payload=%s\n", channel.c_str(), payload.c_str());
        co_return;
    }

    asio::awaitable<void> async_on_pmessage(
        const std::string &pattern,
        const std::string &channel,
        const std::string &payload) override
    {
        fprintf(stderr, "[echo_co_sub] 📨  PMSG pattern=%-10s channel=%-15s payload=%s\n", pattern.c_str(), channel.c_str(), payload.c_str());
        co_return;
    }

    asio::awaitable<void> async_run_loop() override
    {
        fprintf(stderr, "[echo_co_sub] TICK  state=%d  active_sub=%p\n", (int)sub_state_.load(), (void *)active_sub().get());
        co_return;
    }

    asio::awaitable<void> run_self_test()
    {
        static int round = 0;
        fprintf(stderr, "\n========== 🧪 SELF-TEST START (round %d) ==========\n", ++round);

        auto &pool = pz::redis::get_redis_pool();
        auto ioc   = co_await asio::this_coro::executor;

        for (int i = 1; i <= 10; ++i)
        {
            if (isclose)
                break;

            auto r1 = co_await pool.async_publish("default", "echo_test", std::string("echo_sub PUBLISH#") + std::to_string(i));
            auto r2 = co_await pool.async_publish("default", "echo_co_test", std::string("echo_co_sub PUBLISH#") + std::to_string(i));
            auto r3 = co_await pool.async_publish("default", "stock.GOOG", std::string("stock.GOOG PUBLISH#") + std::to_string(i));

            fprintf(stderr,
                    "[echo_co_sub] 📤  PUBLISH#%d  echo_test→%lld  echo_co_test→%lld  stock.GOOG→%lld\n",
                    i,
                    (long long)r1,
                    (long long)r2,
                    (long long)r3);

            asio::steady_timer t(ioc, std::chrono::milliseconds(200));
            co_await t.async_wait(asio::use_awaitable);
        }

        fprintf(stderr, "========== SELF-TEST DONE - flag cleared, next subscribe can claim it again ==========\n\n");
        co_return;
    }

    // 本 client 实例的 socket 保护旗：self_test 跑 async_publish 时持旗，
    // 防止将来业务在 pump 回调里也调 publish 时竞争。断连/跑完放旗。
    std::atomic_flag test_publisher_ = ATOMIC_FLAG_INIT;
};

}// namespace redis_framework_test
