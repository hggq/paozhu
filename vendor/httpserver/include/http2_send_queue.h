#ifndef HTTP2_SEND_QUEUE_H__
#define HTTP2_SEND_QUEUE_H__
/*
 * http2 send queue
 * 黄自权(huang ziquan)
 * 2025-01-02
 */
#include <iostream>
#include <string>
#include <list>
#include <mutex>
#include <atomic>
#include <chrono>
#include "terminal_color.h"
#include "httppeer.h"

namespace http
{

struct http2_send_data_t
{
    bool isfinish                 = false;
    bool is_sendheader            = false;
    bool only_send_header         = false;
    unsigned char type            = 0;//1 file 2 not use 3 gip file 4 br file    10 data (php fast-cgi) 11 peer.output  16 compress gzip content 16 compress br content
    std::atomic_bool standby_next = true;
    unsigned int stream_id        = 0;

    //unsigned int file_modify_time=0; //modify time

    unsigned long long content_length = 0;//content_length
    unsigned long long current_num    = 0;

    // 所有权状态。一个发送对象任一时刻只能有一个持有者，转换必须发生在锁内：
    //   0 空闲池（queue_list 里）
    //   1 全局 sent_data_list 那一侧（含刚从池里取出、尚未入队的调用方）
    //   2 某个发送线程私有 thread_sent_data_list
    //   3 挂起表 parked_list（等窗口/环事件回灌）
    // 回灌前必须先把对象从线程链表摘除：同一条流被两个线程各发一轮 = 比超发更糟的错误。
    std::atomic<unsigned char> own_state = 0;
    // 最近一轮被闸门挡住的原因：0 未被挡 / 1 发送窗口见底 / 2 发送环腾不出槽位。
    // 发送线程据此决定「挂起等事件」还是「下一轮再来」，两者都不会丢内容。
    std::atomic<unsigned char> block_reason = 0;

    // 本轮最后一次真正推进偏移的时刻。分片节奏由发送额度决定，这里只留一个用途：
    // 连续 CONST_HTTP2_SEND_NO_PROGRESS_TIMEOUT 秒没有任何字节发出去，才撤这条流。
    std::chrono::time_point<std::chrono::steady_clock> last_progress_time = std::chrono::steady_clock::now();

    std::unique_ptr<std::FILE, int (*)(FILE *)> fp = {nullptr, std::fclose};
    std::shared_ptr<httppeer> peer;

    std::string file_name;
    std::string etag;
    std::string file_ext;
    std::string content_type;
    std::string header;
    std::string content;
    std::string cache_data;
    void reset()
    {
        isfinish         = false;
        standby_next     = true;
        is_sendheader    = false;
        only_send_header = false;
        type             = 0;
        stream_id        = 0;
        //        file_modify_time=0;

        content_length      = 0;
        current_num         = 0;
        last_progress_time  = std::chrono::steady_clock::now();
        own_state           = 0;
        block_reason        = 0;

        fp.reset();
        peer.reset();

        file_name.clear();
        etag.clear();
        file_ext.clear();
        content_type.clear();
        header.clear();
        content.clear();
        cache_data.clear();
    }
};

class http2_send_queue
{
  public:
    http2_send_queue() {};
    ~http2_send_queue();
    std::shared_ptr<http2_send_data_t> get_cache_ptr();
    void fix_queue_list(unsigned int total);
    void back_cache_ptr(std::shared_ptr<http2_send_data_t>);

    // ---- 挂起表（park）----
    // 被发送窗口或发送环挡住的发送对象停在这里，由三条事件边（WINDOW_UPDATE /
    // SETTINGS 的 IWS 变化 / 发送环腾出槽位，外加额度退还）回灌，而不是靠发送线程
    // 逐 tick 复查。表放这儿而不是放 client_session 上，是为了让兜底扫描有一张
    // 现成的名册：挂在 session 上就得每拍遍历 socket_session_lists 才能找到它们。
    //
    // session_obj 传空 = 摘出全部（兜底扫描）。摘出后所有权归调用方，own_state 仍是 3。
    bool detach_parked(const client_session *session_obj, std::list<std::shared_ptr<http2_send_data_t>> &out);
    // 发送线程发现闸门关上时把对象挂进来；own_state 的转换在锁内完成。
    void park(std::shared_ptr<http2_send_data_t> sp, unsigned char reason);
    // 闸门重算后仍不够格的挂回表里（调用方不再持有它们）。
    void reattach_parked(std::list<std::shared_ptr<http2_send_data_t>> &in);

  public:
    std::list<std::shared_ptr<http2_send_data_t>> queue_list;
    std::mutex lock_queue;
    bool isclose = false;

    std::list<std::shared_ptr<http2_send_data_t>> parked_list;
    std::mutex parked_mutex;
    // parked_list 当前长度。热路径（每写完一帧）先读它，为 0 就不抢锁。
    std::atomic<unsigned int> parked = 0;
    // 已取出但未回池的对象数：get_cache_ptr ++、back_cache_ptr --。
    // 连接全部收尾后必须归零，不归零就是泄漏（挂起表拽着 peer 和文件句柄）。
    std::atomic<unsigned int> outstanding = 0;
    // 累计挂起次数（只增不减）。parked 是瞬时值，几秒一拍的兜底扫描抓不到瞬态，
    // 也就分不清"快路径根本没挂起"和"每帧都挂起又立刻被事件边捞走"——
    // 这两种情况下 parked 读数都是 0，但吞吐差别在两倍。
    std::atomic<unsigned long long> park_total = 0;
    // 累计「补喂」发生了多少次（首帧之外每一次进补喂分支计 1，只增不减；这一帧随后被
    // 窗口或环挡下也计）。没有这个计数，补喂跑没跑过只能靠代码形状断言：一次都没触发
    // 说明这条支路是死的；触发很多次而吞吐不动说明瓶颈在补喂之外——两种读数的处置
    // 相反，必须分得开。
    std::atomic<unsigned long long> extra_feed_total = 0;
    // 兜底扫描的归属：值是「上一次扫描发生在第几秒」（timeid()）。多条发送线程都会在有
    // 活儿和没活儿两种场合醒来，每一拍只允许抢到这条 CAS 的线程真去扫；抢不到的那一次
    // 开销就是一次原子读。扫描函数里的边沿计数（上一次 parked / outstanding / 连续滞留
    // 拍数）都是普通静态量，靠这条 CAS 的 acq_rel 提供 happens-before 才不构成数据竞争，
    // 把它们挪到 CAS 之外访问就是竞争。
    std::atomic<unsigned long long> belt_sweep_second = 0;
    // 单趟扫描的互斥标志。上面那条 CAS 只保证两次扫描的*起点*相隔不少于一拍，不保证上一拍
    // 已经结束——一趟扫描要摘全表、逐条重算闸门，表足够长（或 ASan 拖慢）时能跑过一拍，
    // 而函数里那三个边沿静态量必须由单趟扫描独占。抢不到这个标志的那一次直接跳过本拍：
    // 兜底本来就允许某一拍被漏掉，事件边才是正常路径。
    std::atomic<unsigned char> belt_sweep_busy = 0;
};
http2_send_queue &get_http2_send_queue();

}// namespace http
#endif
