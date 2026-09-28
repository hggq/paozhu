#ifndef HTTP_MQTT_BROKER_H
#define HTTP_MQTT_BROKER_H

// MQTT broker：订阅表 / retained 消息 / topic 匹配 / client_id 会话索引。
// 订阅者用 weak_ptr 防野指针；投递前锁内序列化、出锁发送。
// 支持 retained / No Local / 共享订阅 ($share/{group}/...)。

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "mqtt_frame.h"
#include "mqtt_session.h"

namespace http
{

// ===================== 订阅条目 =====================

struct mqtt_subscriber_entry
{
    std::string reg_key;
    std::string group;
    std::string device;
    std::string topic_filter;
    uint8_t max_qos          = 0;
    bool no_local            = false;   // MQTT 5 No Local
    bool retain_as_published = false;   // MQTT 5 RAP
    uint8_t retain_handling  = 0;       // MQTT 5 Retain Handling
    std::weak_ptr<mqtt_session> session;// 替代旧的裸指针
};

// ===================== retained 消息 =====================
// 内嵌 mqtt_publish_props，避免手动平铺每个字段（加新属性时改一处即可）。

struct mqtt_retained_message
{
    std::string topic;
    mqtt_payload_ptr payload;
    uint8_t qos         = 0;
    uint64_t created_at = 0; // 单调秒
    mqtt_publish_props props;// 原样存入站 PUBLISH 的属性

    // 还原为"出站 PUBLISH 属性" —— 剩余寿命递减 + 丢弃方向相关字段。
    // 内部复用 forward_publish_props 的同一套清洗逻辑，避免三处维护白名单。
    bool to_publish_props(mqtt_publish_props &out, uint64_t now_sec) const;

    // retained 记账口径：与 mqtt_publish_info::accounted_bytes() 同式
    // （topic + payload + props）。入库/更新/prune 三处必须同口径，否则字节账漂移。
    size_t accounted_bytes() const
    {
        return topic.size() + (payload ? payload->size() : 0) + props.accounted_bytes();
    }
};

// ===================== 投递目标 =====================

struct mqtt_delivery
{
    std::shared_ptr<mqtt_session> session;
    uint8_t out_qos          = 0;
    bool retain_as_published = false;
    uint8_t retain_handling  = 0;
};

// ===================== 订阅结果 =====================

// 订阅数配额：只按 filter 长度/条数防不住内存——单连接可连灌百万级 filter，
// 每个 filter 最长 64KB（topic 上限），且 publish() 每次都要遍历全部 filter。
enum class mqtt_subscribe_result : uint8_t
{
    added,          // 新增订阅
    updated,        // 同一会话在同一 filter 上重订阅（只更新选项，不再占配额）
    rejected_filter,// filter 非法 → SUBACK 0x8F
    rejected_quota  // 会话级 / 全局订阅数超限 → SUBACK 0x97
};

class mqtt_broker
{
  public:
    static mqtt_broker &instance()
    {
        static mqtt_broker inst;
        return inst;
    }

    // ===================== 订阅管理 =====================

    static constexpr size_t kMaxSubscriptionsPerSession = 256;     // 单会话 filter 数上限
    static constexpr size_t kMaxSubscriptionsTotal      = 100000;  // 全局 (filter, 订阅者) 条目上限

    static bool subscribe_ok(mqtt_subscribe_result r)
    {
        return r == mqtt_subscribe_result::added || r == mqtt_subscribe_result::updated;
    }

    mqtt_subscribe_result subscribe(const std::string &reg_key,
                                    const std::string &group,
                                    const std::string &device,
                                    const std::string &topic_filter,
                                    uint8_t max_qos,
                                    bool no_local,
                                    bool retain_as_published,
                                    uint8_t retain_handling,
                                    const std::shared_ptr<mqtt_session> &session)
    {
        if (!is_valid_topic_filter(topic_filter))
            return mqtt_subscribe_result::rejected_filter;
        std::lock_guard<std::mutex> lk(mu_);
        auto &vec = subs_[topic_filter];

        for (auto it = vec.begin(); it != vec.end(); ++it)
        {
            // 同一会话（同一 socket）在同一 filter 上视为重订阅 → 更新选项。
            // 不占新配额，所以即使已到上限也必须放行（否则重订阅会被误拒）。
            if (!it->session.expired() && it->session.lock() == session)
            {
                it->max_qos             = max_qos;
                it->no_local            = no_local;
                it->retain_as_published = retain_as_published;
                it->retain_handling     = retain_handling;
                it->reg_key             = reg_key;
                it->group               = group;
                it->device              = device;
                return mqtt_subscribe_result::updated;
            }
        }

        // 新增条目的配额闸（会话级优先，便于诊断）
        if (session && session->subscription_count() >= kMaxSubscriptionsPerSession)
        {
            if (vec.empty()) subs_.erase(topic_filter);// 清掉 operator[] 造出的空壳
            return mqtt_subscribe_result::rejected_quota;
        }
        if (subs_total_ >= kMaxSubscriptionsTotal)
        {
            if (vec.empty()) subs_.erase(topic_filter);
            return mqtt_subscribe_result::rejected_quota;
        }

        mqtt_subscriber_entry e;
        e.reg_key             = reg_key;
        e.group               = group;
        e.device              = device;
        e.topic_filter        = topic_filter;
        e.max_qos             = max_qos;
        e.no_local            = no_local;
        e.retain_as_published = retain_as_published;
        e.retain_handling     = retain_handling;
        e.session             = session;
        vec.push_back(std::move(e));

        ++subs_total_;
        // 会话侧计数：只由 broker 在 mu_ 之下维护，会话对象销毁即随之消失，
        // 因此不存在「失效条目留下陈旧计数、裸指针复用后误判配额」的问题。
        if (session) session->on_broker_subscribe(1);
        return mqtt_subscribe_result::added;
    }

    void unsubscribe(const std::string &topic_filter, const std::shared_ptr<mqtt_session> &session)
    {
        std::lock_guard<std::mutex> lk(mu_);
        auto it = subs_.find(topic_filter);
        if (it == subs_.end())
            return;
        auto &vec = it->second;
        size_t removed = std::erase_if(vec, [&session](const mqtt_subscriber_entry &e)
                                       { return matches(e.session, session); });
        decr_total(removed);
        if (session) session->on_broker_unsubscribe(removed);
        if (vec.empty())
            subs_.erase(it);
    }

    // 会话清理：移除该会话的所有订阅
    void unsubscribe_all(const std::shared_ptr<mqtt_session> &session)
    {
        std::lock_guard<std::mutex> lk(mu_);
        size_t removed_all = 0;
        for (auto it = subs_.begin(); it != subs_.end();)
        {
            auto &vec = it->second;
            removed_all += std::erase_if(vec, [&session](const mqtt_subscriber_entry &e)
                                         { return matches(e.session, session); });
            if (vec.empty())
                it = subs_.erase(it);
            else
                ++it;
        }
        decr_total(removed_all);
        if (session) session->on_broker_unsubscribe(removed_all);
        prune();
    }

    // ===================== client_id → session 索引（同 ClientID 踢线）=====================

    // 注册新会话；若该 client_id 已有会话，返回旧会话（调用方负责 stop 它），否则返回 nullptr
    std::shared_ptr<mqtt_session> register_client(const std::string &client_id,
                                                  const std::shared_ptr<mqtt_session> &session)
    {
        std::lock_guard<std::mutex> lk(mu_);
        std::shared_ptr<mqtt_session> old;
        auto it = clients_.find(client_id);
        if (it != clients_.end())
        {
            old        = it->second.lock();
            it->second = session;
        }
        else
        {
            clients_.emplace(client_id, session);
        }
        return old;
    }

    void unregister_client(const std::string &client_id,
                           const std::shared_ptr<mqtt_session> &session)
    {
        std::lock_guard<std::mutex> lk(mu_);
        auto it = clients_.find(client_id);
        if (it == clients_.end())
            return;
        auto cur = it->second.lock();
        if (!cur || cur == session)
            clients_.erase(it);
    }

    std::shared_ptr<mqtt_session> find_client(const std::string &client_id)
    {
        std::lock_guard<std::mutex> lk(mu_);
        auto it = clients_.find(client_id);
        if (it == clients_.end())
            return nullptr;
        return it->second.lock();
    }

    // ===================== retained 消息 =====================

    // 写入 / 更新 / 清除 retained。
    // 返回 false = 因配额被拒（消息未入 retained 库，调用方据此回 0x97，不再静默丢弃）。
    // 记账口径统一为 accounted_bytes()（topic + payload + props）：只算 payload 的话，
    // 「1 字节 payload + 16MB user_properties」乘以 10000 条即可撑爆 64MB 预算。
    bool retain(const mqtt_publish_info &pub, uint64_t now_sec)
    {
        std::lock_guard<std::mutex> lk(mu_);
        auto it = retained_.find(pub.topic);
        if (!pub.payload || pub.payload->empty())
        {
            // 空 payload 表示清除 retained（与消息是否过期无关，永远视为成功）
            if (it != retained_.end())
            {
                retained_bytes_ -= it->second.accounted_bytes();
                retained_.erase(it);
            }
            return true;
        }
        size_t new_bytes = pub.accounted_bytes();
        if (it == retained_.end())
        {
            // 新增 topic：条数 + 字节双重配额
            if (retained_.size() >= kMaxRetained) return false;
            if (retained_bytes_ + new_bytes > kMaxRetainedBytes) return false;
        }
        else
        {
            // 更新已有 topic：新总字节 = 旧总 - 旧条目 + 新条目，超限则拒绝
            size_t old_bytes = it->second.accounted_bytes();
            if (retained_bytes_ - old_bytes + new_bytes > kMaxRetainedBytes) return false;
            retained_bytes_ -= old_bytes;
        }
        mqtt_retained_message m;
        m.topic              = pub.topic;
        m.payload            = pub.payload;// 共享引用，不拷贝字节
        m.qos                = pub.qos;
        m.created_at         = now_sec;
        m.props              = pub.props;// 原样存入站 PUBLISH 的属性
        retained_[pub.topic] = std::move(m);
        retained_bytes_ += new_bytes;
        prune_retained(now_sec);
        return true;
    }

    std::vector<mqtt_retained_message> match_retained(std::string_view topic_filter, uint64_t now_sec)
    {
        std::vector<mqtt_retained_message> out;
        std::lock_guard<std::mutex> lk(mu_);
        prune_retained(now_sec);
        for (auto &kv : retained_)
        {
            if (topic_match(topic_filter, kv.first))
                out.push_back(kv.second);
        }
        return out;
    }

    size_t retained_count() const
    {
        std::lock_guard<std::mutex> lk(mu_);
        return retained_.size();
    }

    // ===================== 发布寻址 =====================

    // exclude_session 通常为发布者自身。是否跳过发布者由该订阅的 No Local 决定
    // （MQTT 5 §3.8.3.1 / §4.2.1）：No Local=1 跳过，No Local=0 必须投递给发布者。
    std::vector<mqtt_delivery> publish(const std::string &topic, uint8_t qos, const std::shared_ptr<mqtt_session> &exclude_session)
    {
        std::vector<mqtt_delivery> out;
        std::lock_guard<std::mutex> lk(mu_);
        for (auto &kv : subs_)
        {
            if (!topic_match(kv.first, topic))
                continue;
            for (auto &e : kv.second)
            {
                auto sp = e.session.lock();
                if (!sp)
                    continue;// 会话已释放 → 自然淘汰
                // No Local（MQTT 5 §3.8.3.1 / §4.2.1）：
                //   1 → 不得把消息回发给发布者自身
                //   0 → 必须回发给发布者自身（只要订阅匹配），因此这里不能无条件排除发布者
                if (e.no_local && exclude_session && sp == exclude_session)
                    continue;
                uint8_t out_qos = (qos < e.max_qos) ? qos : e.max_qos;
                mqtt_delivery d;
                d.session             = sp;// 持 strongly ref 出锁，杜绝 UAF
                d.out_qos             = out_qos;
                d.retain_as_published = e.retain_as_published;
                d.retain_handling     = e.retain_handling;
                out.push_back(std::move(d));
            }
        }
        // 失效订阅（weak_ptr 过期）由 server tick 的 prune_expired() 定期清理，
        // 不在每消息热路径上做 O(N) 全表扫描。
        return out;
    }

    // ===================== topic 匹配（MQTT 5 规则，零堆分配）=====================

    // 逐 level 原地比较，避免 split_topic 每次分配两个 vector<string>。
    // 发布热路径上每消息对每个 filter 调用一次，去分配对吞吐影响显著。
    static bool topic_match(std::string_view topic_filter, std::string_view topic)
    {
        // 共享订阅：$share/{ShareName}/{filter}，剥掉前缀后按正常 filter 匹配。
        // MQTT 5 §4.8.2：$share 必须自成首段 —— 原实现只比较前 6 字节，
        // 于是 "$sharefoo/g/x" 被误判为共享订阅并剥成 "x"（P1-7）。
        constexpr std::string_view kShare = "$share/";
        if (topic_filter.size() > kShare.size() &&
            topic_filter.compare(0, kShare.size(), kShare) == 0)
        {
            size_t second = topic_filter.find('/', kShare.size());
            if (second != std::string_view::npos)
                topic_filter = topic_filter.substr(second + 1);
        }

        if (topic_filter.empty() || topic.empty())
            return false;

        // 主题名不得含通配符
        if (topic.find('+') != std::string_view::npos || topic.find('#') != std::string_view::npos)
            return false;

        // MQTT 规范：以 '$' 开头的主题不被 '#' 或 '+' 匹配（需显式订阅）
        bool filter_has_wildcard = (topic_filter.find('#') != std::string_view::npos) ||
                                   (topic_filter.find('+') != std::string_view::npos);
        if (filter_has_wildcard && !topic.empty() && topic[0] == '$')
        {
            if (topic_filter.empty() || topic_filter[0] != '$')
                return false;
        }

        size_t fpos = 0, tpos = 0;
        while (true)
        {
            // 取 filter 当前 level（可能为空串，对应连续 // 或末尾 /）
            size_t fslash = topic_filter.find('/', fpos);
            std::string_view flevel = (fslash == std::string_view::npos)
                                          ? topic_filter.substr(fpos)
                                          : topic_filter.substr(fpos, fslash - fpos);

            // '#' 只能是最后一个 level，匹配剩余全部（含 0 个）level
            if (flevel == "#")
                return (fslash == std::string_view::npos);

            // 取 topic 当前 level
            if (tpos > topic.size())
                return false;
            size_t tslash = (tpos == topic.size()) ? std::string_view::npos : topic.find('/', tpos);
            std::string_view tlevel = (tslash == std::string_view::npos)
                                          ? topic.substr(tpos)
                                          : topic.substr(tpos, tslash - tpos);

            // '+' 匹配任意单个 level；否则逐字相等
            if (flevel != "+" && flevel != tlevel)
                return false;

            // 推进到下一 level
            fpos = (fslash == std::string_view::npos) ? topic_filter.size() + 1 : fslash + 1;
            tpos = (tslash == std::string_view::npos) ? topic.size() + 1 : tslash + 1;

            bool filter_done = (fpos > topic_filter.size());
            bool topic_done  = (tpos > topic.size());

            if (filter_done && topic_done)
                return true;
            if (filter_done)
                return false;  // topic 还有多余 level
            if (topic_done)
            {
                // topic 已用完，filter 剩余部分只能是 '#' 才匹配（# 匹配 0 个剩余 level）
                return topic_filter.substr(fpos) == "#";
            }
        }
    }

    // ===================== 定期清理 =====================

    // 清理已失效的订阅与 client_id 记录（weak_ptr 过期）。由 server tick 定期调用，
    // 取代原先 publish() 每消息 O(N) 全表扫描。publish 热路径只跳过过期条目，不做删除。
    void prune_expired()
    {
        std::lock_guard<std::mutex> lk(mu_);
        prune();
    }

    // ===================== 观测 =====================

    size_t subscription_count() const
    {
        // 与配额同源（含尚未被 prune 回收的失效条目），避免两套计数漂移
        std::lock_guard<std::mutex> lk(mu_);
        return subs_total_;
    }

    size_t topic_count() const
    {
        std::lock_guard<std::mutex> lk(mu_);
        return subs_.size();
    }

    size_t client_count() const
    {
        std::lock_guard<std::mutex> lk(mu_);
        return clients_.size();
    }

  private:
    mqtt_broker()                               = default;
    ~mqtt_broker()                              = default;
    mqtt_broker(const mqtt_broker &)            = delete;
    mqtt_broker &operator=(const mqtt_broker &) = delete;

    static bool matches(const std::weak_ptr<mqtt_session> &wp,
                        const std::shared_ptr<mqtt_session> &sp)
    {
        auto cur = wp.lock();
        return cur && cur == sp;
    }

    // 清理已失效的订阅与 client_id 记录（需在持有 mu_ 时调用）
    void prune()
    {
        size_t removed_all = 0;
        for (auto it = subs_.begin(); it != subs_.end();)
        {
            auto &vec = it->second;
            // 会话已释放：其自身计数随对象销毁，无需回改；这里只维护全局总数
            removed_all += std::erase_if(vec, [](const mqtt_subscriber_entry &e)
                                         { return e.session.expired(); });
            if (vec.empty())
                it = subs_.erase(it);
            else
                ++it;
        }
        decr_total(removed_all);
        for (auto it = clients_.begin(); it != clients_.end();)
        {
            if (it->second.expired())
                it = clients_.erase(it);
            else
                ++it;
        }
    }

    // 全局条目计数递减（需在持有 mu_ 时调用）
    void decr_total(size_t n)
    {
        subs_total_ = (subs_total_ > n) ? (subs_total_ - n) : 0;
    }

    void prune_retained(uint64_t now_sec)
    {
        for (auto it = retained_.begin(); it != retained_.end();)
        {
            const auto &m = it->second;
            bool expired  = m.props.has_message_expiry &&
                           m.props.message_expiry_interval > 0 &&
                           (now_sec >= m.created_at + m.props.message_expiry_interval);
            if (expired)
            {
                retained_bytes_ -= m.accounted_bytes();
                it = retained_.erase(it);
            }
            else
                ++it;
        }
    }

    static constexpr size_t kMaxRetained      = 10000;
    static constexpr size_t kMaxRetainedBytes = 64 * 1024 * 1024;// broker 级 retained 总字节上限（accounted_bytes 口径）

    mutable std::mutex mu_;
    std::unordered_map<std::string, std::vector<mqtt_subscriber_entry>> subs_;
    size_t subs_total_ = 0;// subs_ 中 (filter, 订阅者) 条目总数，与配额同源
    std::unordered_map<std::string, mqtt_retained_message> retained_;
    size_t retained_bytes_ = 0;// retained_ 中 accounted_bytes() 总字节
    std::unordered_map<std::string, std::weak_ptr<mqtt_session>> clients_;
};

// mqtt_retained_message::to_publish_props — 算 waited 后调统一的核心。
// 与 forward_publish_props 同一套剩余寿命递减 + 清洗逻辑（MQTT 5 §3.3.2.3.3）。
inline bool mqtt_retained_message::to_publish_props(mqtt_publish_props &out,
                                                    uint64_t now_sec) const
{
    uint64_t waited = (now_sec > created_at) ? (now_sec - created_at) : 0;
    return forward_publish_props_core(props, waited, out);
}

}// namespace http

#endif
