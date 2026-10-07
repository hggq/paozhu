/*
 * Redis pubsub 同步版框架级验证订阅 + 自验证测试
 *
 * 线程模型：
 *   pump on_message  → post_conn_step → clientrunpool 业务线程池
 *   run_loop()       → tick 线程直接调（跟 mqtt 同款）
 *
 * 与 echo_subscriber_co（协程版）配合：co 版是持旗者，主动 PUBLISH 本版订阅的频道，
 * 本版用来验证"同步 on_message 回调确实收到了 Redis 推送"。
 */
#pragma once

#include "redis_subpub.h"
#include <atomic>
#include <cstdio>
#include <string>

namespace redis_framework_test
{

class echo_subscriber : public pz::redis::redis_subpub_client
{
  public:
    echo_subscriber()
    {
        is_coroutine_ = false;
        durtime       = 3;
        loop_num      = 999999;
    }

    std::string section_name() const override { return "default"; }

    std::vector<std::string> channels() const override
    {
        return {"echo_test"};// co 版会 PUBLISH 到这里
    }

    std::vector<std::string> patterns() const override
    {
        return {"stock.*"};// 两个订阅者都订阅了同一个 pattern
    }

    void on_subscribe_ok() override
    {
        fprintf(stderr, "[echo_sub] ✅ SUBSCRIBE OK  channels=%zu patterns=%zu\n", channels().size(), patterns().size());
    }

    void on_disconnect() override
    {
        fprintf(stderr, "[echo_sub] DISCONNECT, reconnecting in 3s\n");
    }

    void on_message(const std::string &channel, const std::string &payload) override
    {
        // 跑在 clientrunpool 业务线程池里
        fprintf(stderr, "[echo_sub] 📨  MSG  channel=%-15s payload=%s\n", channel.c_str(), payload.c_str());
    }

    void on_pmessage(const std::string &pattern,
                     const std::string &channel,
                     const std::string &payload) override
    {
        fprintf(stderr, "[echo_sub] 📨  PMSG pattern=%-10s channel=%-15s payload=%s\n", pattern.c_str(), channel.c_str(), payload.c_str());
    }

    void run_loop() override
    {
        // tick 线程里直接调（跟 mqtt 同款）；本版是纯接收者，不持旗不 publish
        fprintf(stderr, "[echo_sub] TICK  state=%d  active_sub=%p\n", (int)sub_state_.load(), (void *)active_sub().get());
    }
};

}// namespace redis_framework_test
