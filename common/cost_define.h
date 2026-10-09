#pragma once
#ifndef _CONST_DEFINE_FUNC_H
#define _CONST_DEFINE_FUNC_H

#define CONST_MONEY_PART 1000000
#define CONST_HTTP_HEADER_BODY_SIZE 16384
#define CONST_PHP_BODY_POST_SIZE 16777216

#define CONST_HTTP_BODY_POST_SIZE 33554432
#define CONST_HTTP_JSON_POST_SIZE 2097152
#define CONST_MQTT_SESSION_BODY_SIZE 2097152

// MQTT 5 Will 延迟发布（will_delay_interval，MQTT 5 §3.2.2.26.3）的两道界。
// 上限：客户端声明的秒数是 uint32，最大约 136 年，不钳的话一条遗嘱能把一个定时器
// 挂到进程结束；钳到 1 小时，超出的按 1 小时发布（不丢弃、不改语义，只把等待封顶）。
// 在途条数：反复掉线的客户端每次断开都会挂一个活定时器，不计数就是无界堆积；
// 越界丢最新一条并计数（与 WebSocket 入站水位同一口径：可丢的最新单元 + 计数，不断连）。
// MQTT 5 Will delayed-publish caps. will_delay_interval is a client-supplied uint32
// (~136 years) so the wait is clamped to one hour; the pending count is bounded because a
// flapping client otherwise leaves one live timer per disconnect. Over the cap the newest
// Will is dropped and counted (same policy as the WebSocket ingress watermark).
#define CONST_MQTT_WILL_DELAY_MAX_SEC 3600
#define CONST_MQTT_WILL_DELAY_MAX_PENDING 1024

// socket rpc 握手（请求行 + query）累积上限（字节）：超限直接断连，防对端慢速灌包耗尽内存
// Socket RPC handshake (request line + query) cumulative cap (bytes); disconnect on hit to block slow-lorris memory exhaustion
#define CONST_TCP_HANDSHAKE_MAX 4096

#define CONST_WEBSOCKET_POST_TEXT_SIZE 16777216
#define CONST_WEBSOCKET_POST_DATA_SIZE 4294967295

// WebSocket 入站限额：单帧载荷 / 分片重组消息 / 累计落盘阈值（字节）
// 口径不变式：spill < max_frame <= max_message（超限回 Close(1009)，不静默截断）
// WebSocket ingress caps: per-frame payload / reassembled message / disk-spill threshold (bytes); invariant: spill < max_frame <= max_message (Close(1009), no silent truncation)
#define CONST_WEBSOCKET_MAX_FRAME_SIZE 16777216
#define CONST_WEBSOCKET_MAX_MESSAGE_SIZE 16777216
#define CONST_WEBSOCKET_SPILL_THRESHOLD 2097152

// WebSocket 出站 permessage-deflate 压缩门槛（字节）：text/binary 载荷超过该值才压缩
// WebSocket egress permessage-deflate minimum (bytes); only compress when text/binary payload exceeds this
#define CONST_WEBSOCKET_DEFLATE_MIN_SIZE 256

// WebSocket 入站派发水位（每连接）。两种模式各量自己的那条无界队列，共用同一组阈值：
//   非协程模式 = websockets_api::content_list 深度/载荷字节；
//   协程模式   = 在途 async_onmessage 条数/载荷字节（那条路径没有队列，协程本身就是队列）。
// 越水位先暂停 async_read 做背压（每轮只喂 4096B，未读字节留在内核缓冲，压力退回对端 TCP 窗口），
// 有界等待到期仍未退到 LOW 水位才丢弃最新一条并计数——默认不断连。
// 条数是主尺子（list::size() O(1) 且无漂移）；字节尺子非维护计数：非协程模式在越限慢路径现场遍历求和
// （content_list 由业务代码自行 pop_front，维护式计数必漂），协程模式由派发包装协程成对 reserve/release。
// 阈值依据：本机实测合法快消费路径在 30MB/s × 100MB 洪泛下自然峰值仅 7 条 / 448KB。
// WebSocket ingress dispatch watermark (per connection). Both modes bound their own unbounded queue:
// non-coroutine = content_list depth/bytes; coroutine = in-flight async_onmessage count/bytes.
// Over the watermark the loop first pauses async_read (backpressure to the peer's TCP window, no loss,
// no disconnect); only after the bounded wait, if still above LOW, the newest message is dropped and counted.
// Measured natural peak on a healthy consumer: 7 messages / 448 KB under a 100 MB flood.
#define CONST_WEBSOCKET_QUEUE_HIGH_ITEMS 256
#define CONST_WEBSOCKET_QUEUE_LOW_ITEMS 64
#define CONST_WEBSOCKET_QUEUE_HIGH_BYTES 8388608
#define CONST_WEBSOCKET_QUEUE_LOW_BYTES 2097152
#define CONST_WEBSOCKET_QUEUE_STALL_MAX_MS 2000

// socket 入站接收队列水位（每连接）。这里的单位是"一次 read 拿到的字节切片"，不是"一条消息"：
// read_socket 是 async_read_some 进 4096 字节缓冲（与 ws/h1 的 _cache_data 对齐），切片没有帧边界，
// 丢一片就等于把这条流咬掉一口。
// 所以这条路不丢数据：队列满（push 在 size >= HIGH_ITEMS 或字节数 > HIGH_BYTES 时拒收）就暂停读取，
// 把压力退回对端的 TCP 窗口，停读间隔从 200ms 起每轮翻倍，200/400/800/1000 各一轮，之后固定 2s 轮询队列；
// 累计停读满 5 分钟仍然进不了队，就关闭这条连接退出。
// 双闸：HIGH_ITEMS 防海量极小包把 list 节点数打爆（保留 512 条数上限）；HIGH_BYTES 锁单连接入站内存上界。
// 读窗口提到 4096 后，若不锁字节闸，512 片就会变成 2MB/连接——所以这里把上界显式钉成 512KB，与改窗口前一致。
#define CONST_SOCKET_QUEUE_HIGH_ITEMS 512
#define CONST_SOCKET_QUEUE_HIGH_BYTES (512 * 1024)

#define CONST_SOCKET_MAX_CONN_PER_IP 10000
#define CONST_SOCKET_QUEUE_STALL_MS 200
#define CONST_SOCKET_QUEUE_STALL_TOP_MS 1000
#define CONST_SOCKET_QUEUE_STALL_LOOP_MS 2000
#define CONST_SOCKET_QUEUE_STALL_GIVEUP_MS 300000

// 发送线程每轮的空转兜底（纳秒）。它同时是单流吞吐的上界之一：发送序列一次调用只出一个
// 帧、线程循环一轮至多喂两帧，所以一条还在推进的流每轮都在几微秒内干完活，这根 tick 每轮
// 都触发 —— 上界 = 帧大小 × 每轮帧数 / 本值（16384 × 2 / 100000ns = 328 MB/s）。
// 保持"固定睡"，不要改成"补齐到本值"：那是行为改动。
// Send-loop round idle-tick (ns). Also upper bound on single-stream throughput (frame_size × 2 / tick_ns). Keep "fixed sleep", do NOT switch to "pad up to tick" — behavior change.
#define CONST_HTTP2_SlEEP_MIN_TIME 100000

#define CONST_ORM_CLEAR_TIME 7200
#define CONST_ORM_CLEAR_NUMBER 1024
#define CONST_ORM_QUERY_CONNECT_TIMEOUT 30
#define CONST_ORM_QUERY_LOG_TIME 30

#define CONST_ERROR_COUNT_TIME 28800
#define CONST_HARD_KILL_TIME 28800

// 请求落盘临时文件（pzraw_/pzup_，见 func.h）在 temp_path 下的生命周期（秒），默认 24 小时。
// httpwatch 每隔这么长时间扫描一次 temp_path，只删除「前缀命中框架白名单」且「修改时间超过这么长时间」的文件，
// 因此 temp_path 下的会话文件（*_sess）、静态压缩缓存目录（statichtml）、业务文件都不会被碰到。
// 可用 server.conf 的 [default] temp_clean_time 覆盖。
// Temp-file (pzraw_/pzup_) lifetime under temp_path (seconds, default 24h). httpwatch scans at this interval; only whitelisted prefixes + files older than N are removed. Session/static-compress/business files untouched. Overridable via server.conf [default] temp_clean_time.
#define CONST_HTTP_TEMP_FILE_LIVE_TIME (3600 * 24)

#define COOKIE_SESSION_NAME "PHPSESSID"

// ---- HTTP/2 单连接资源上限（http2_parse.cpp）----
// 单连接上「未完成头部块」+「已建流但 body 未结束」的流数量上限。
// server.cpp 的 steam_count 只统计「已派发」的流，客户端只发 HEADERS 不开 END_STREAM
// 时永远不会触发，因此这里必须再卡一道，否则 http_data/http_post_data 可被撑爆。
// HTTP/2 per-connection resource caps (http2_parse.cpp). Guards against clients that send HEADERS without END_STREAM — server.cpp's stream_count only tracks dispatched streams and never fires for this case.
#define CONST_HTTP2_MAX_STREAMS 512

// HTTP/2 的 host / :authority 最大长度（与 HTTP/1 的 host 口径一致）。
// 超过这个长度一律拒绝，避免超长值进入 header_host_process 的逐字符循环。
// HTTP/2 host/:authority max length (same semantics as HTTP/1 Host). Reject oversize to avoid O(N) scan in header_host_process.
#define CONST_HTTP2_HOST_MAX_SIZE 72

// HTTP/2 协议允许的最大窗口值（2^31-1）。SETTINGS_INITIAL_WINDOW_SIZE 与
// WINDOW_UPDATE 累加超过该值必须按 RFC 9113 §6.9.1 报 FLOW_CONTROL_ERROR。
// HTTP/2 protocol maximum window value (2^31-1). Accumulated SETTINGS_INITIAL_WINDOW_SIZE + WINDOW_UPDATE over this ⇒ FLOW_CONTROL_ERROR per RFC 9113 §6.9.1.
#define CONST_HTTP2_MAX_WINDOW 0x7FFFFFFF

// 连接级流控窗口的初值。RFC 9113 §6.9.2：连接的初始流控窗口固定为 65,535，
// 且 SETTINGS_INITIAL_WINDOW_SIZE 只作用于「每一条流」，不会改变连接级窗口，
// 连接级窗口只能通过 stream id = 0 的 WINDOW_UPDATE 抬升。
// Connection-level flow-control window initial value. Per RFC 9113 §6.9.2: fixed 65535; SETTINGS_INITIAL_WINDOW_SIZE affects per-stream only; connection window rises only via WINDOW_UPDATE on stream id = 0.
#define CONST_HTTP2_DEFAULT_WINDOW 65535

// 本端广告给对端的 SETTINGS_INITIAL_WINDOW_SIZE（client_session::co_send_setting）。
// 该值描述「每一条流」的初始窗口，同时也是本端接收侧流级窗口的目标水位。
// 发送侧不得用该值初始化「连接级」窗口（见 CONST_HTTP2_DEFAULT_WINDOW）。
// SETTINGS_INITIAL_WINDOW_SIZE advertised to peer (client_session::co_send_setting). Per-stream initial window; also our receive-side target watermark. Do NOT use to initialize connection-level window (see CONST_HTTP2_DEFAULT_WINDOW).
#define CONST_HTTP2_LOCAL_INITIAL_WINDOW 0xFFFFFF

// 把连接级窗口从初值抬到本端目标水位所需的一次性增量：0xFFFFFF - 65535 = 16711680。
// 只在首个 SETTINGS 之后用一次；每来一个请求就重发会顶穿 2^31-1 而被对端断连
// （RFC 9113 §6.9.1）。
// One-shot bump to raise connection-level window from initial to our target (0xFFFFFF − 65535 = 16711680). Send only after the initial SETTINGS; re-sending per-request overflows 2^31-1 (RFC 9113 §6.9.1).
#define CONST_HTTP2_WINDOW_UPDATE_STEP (CONST_HTTP2_LOCAL_INITIAL_WINDOW - CONST_HTTP2_DEFAULT_WINDOW)

// 本端接收侧流级窗口低于「目标水位的一半」时才补窗口，避免每个 DATA 帧都回一个
// WINDOW_UPDATE（RFC 9113 §6.9 允许批量化补量）。
// Refill per-stream receive window only when below half of target watermark. Avoids a WINDOW_UPDATE per DATA frame; RFC 9113 §6.9 allows batching.
#define CONST_HTTP2_WINDOW_UPDATE_THRESHOLD (CONST_HTTP2_LOCAL_INITIAL_WINDOW / 2)

// 收到 stream id = 0 的 WINDOW_UPDATE 时，抬升后是否仍然合法（true = 放行）。
//
// RFC 9113 §6.9.1 的 2^31-1 上限约束的是「当前可用窗口」，不是「累计获准额度」：
// 连接级记账里 window_update_num 是累计获准、has_send_update_num 是累计已发，
// 可用额 = 两者之差。判在累计量上，一条长连接累计发满 2GB 之后每一个合法的
// WINDOW_UPDATE 都会被判连接级 FLOW_CONTROL_ERROR，整条连接被 GOAWAY 带走。
//
// 单独成 constexpr 函数是为了让测试代码能直接调用这一条算式本身，而不是照抄一份。
// Validity check for incoming WINDOW_UPDATE on stream id = 0 (true = accept).
// The 2^31-1 cap in RFC 9113 §6.9.1 bounds the *current available* window, not cumulative grants. Encapsulated as constexpr so unit tests call the formula directly.
constexpr bool http2_wu_conn_avail_ok(unsigned long long granted, unsigned long long sent, unsigned long long inc)
{
    unsigned long long avail = (granted > sent) ? (granted - sent) : 0;
    // 直接算和，绕开 "MAX - inc" 在 inc > MAX 时的无符号下溢（wrap 成 uint64_max，判断恒真）。
    // Sum both directly — avoids unsigned underflow of "MAX - inc" when inc > MAX (wraps to uint64_max, always true).
    return avail + inc <= static_cast<unsigned long long>(CONST_HTTP2_MAX_WINDOW);
}

// 收到流级 WINDOW_UPDATE 时，这个流 id 是否是本端认得的客户端流（true = 继续处理）。
// ① 偶数流 id 留给服务端发起的流，本实现不开 push；
// ② 大于本连接已见过的最大客户端流 id = 这条流还没 OPEN。
// 两道门都是纯算术，故抽成独立函数供测试代码直调；「这条流是否已经结束」还要查本端的流表，
// 留调用点自己做（见 http2parse::readwinupdate）。
// Is a stream-level WINDOW_UPDATE on a locally known client stream (true = proceed).
// ① even ids are server-push streams (we don't push); ② sid > max seen ⇒ stream not OPEN yet. Arithmetic-only checks live here; "stream still alive" must be queried at call-site (http2parse::readwinupdate).
constexpr bool http2_wu_stream_id_ok(unsigned long long sid, unsigned long long max_seen_sid)
{
    return ((sid & 1) == 1) && sid <= max_seen_sid;
}

// 单个 DATA 帧载荷缓冲的上限（含 9 字节帧头）。分片大小由发送额度算出，
// 这个值只规定「一轮最多打算发多少」，再与对端 SETTINGS_MAX_FRAME_SIZE 取小。
// Single DATA frame payload buffer cap (incl. 9-byte frame header). Fragment size derived from send quota; this caps "max send per round", then clamped by peer's SETTINGS_MAX_FRAME_SIZE.
#define CONST_HTTP2_SEND_FRAME_BUF 15360
// 一条流连续多久没有任何字节真正推进，就放弃它（RST_STREAM，不断连接）。
// 量的是「多久没推进」而不是「等了多久」：慢客户端只要一直在收，
// 等得再久也不该撤流；只有彻底没进展才该撤。
// Stream abandoned (RST_STREAM, no GOAWAY) when no bytes actually advance for N seconds. Measures "time without progress", not "time waited" — slow clients that keep receiving are fine.
#define CONST_HTTP2_SEND_NO_PROGRESS_TIMEOUT 120
// 发送环积压到几个槽就让路（16 槽环，留 6 槽给控制帧收尾）。发送侧的挂起条件和
// 回灌条件必须用同一个数，否则会出现「挂起来、回灌去」的空转循环。
// Send-ring backpressure slot threshold (16-slot ring, 6 reserved for control-frame tail). Suspend and refill conditions MUST use the same value to avoid "hang → refill → hang" thrashing.
#define CONST_HTTP2_RING_BACKPRESSURE_SLOTS 10
// 一条流一轮之内除了首帧还能补喂几帧。首帧由闸门放行，补喂每次之前重取发送环积压，
// 达到 CONST_HTTP2_RING_FEED_SLOTS 或本轮额度用完就停 —— 这个上限是「一条流不许在
// 本轮独占发送线程」的界，不是吞吐旋钮：环空着的时候多喂的帧，最终仍受窗口与传输约束。
// Max extra frames per stream per round (beyond the gate-released first frame). Prevents one stream from hogging the send thread; NOT a throughput knob — ring-empty extra frames still subject to flow-control and transport constraints.
#define CONST_HTTP2_SEND_FEED_MAX 3
// 补喂允许的发送环积压水位：环里已排队帧数低于它才继续喂同一条流。取在让路阈值
// （10）以下，是给 HEADERS/SETTINGS 这类收尾控制帧和别的连接留余量。
// Feed-watermark: keep feeding same stream while ring queued < this. Chosen below backpressure threshold (10) to reserve headroom for HEADERS/SETTINGS tail frames and other connections.
#define CONST_HTTP2_RING_FEED_SLOTS 5
// 发送线程最长能睡多久：等待新发送对象的条件变量带这个期限，醒来跑一次挂起表兜底扫描。
// 取 6 而不是 5，是为了与 httpwatch 主循环那根 5 秒错开 —— 两条周期任务若同刻醒来会
// 去抢同一批锁（idle 清理要持 socket_session_lists_mutex，兜底要持 send_data_mutex）。
// 没有这个期限，「所有发送对象都挂在表上」这个状态本身就让发送线程睡在无超时的 wait 上，
// 而挂在表上的对象恰恰只有这一趟扫描会去续期与撤流。
// Max send-thread sleep (seconds) — wake at least this often to run the parked-belt sweep (renews time_limit + issues RST for stuck streams). Chosen 6 (not 5) to de-phase from httpwatch's 5s cycle (avoids simultaneous lock contention). Without deadline: all sends parked ⇒ thread sleeps forever; belt never runs.
#define CONST_HTTP2_BELT_SWEEP_SECONDS 6

// RST_STREAM 流错误码（RFC 9113 §7）。CANCEL 表示「流被撤销」，不表示对端违反协议；
// 本地背压/过载放弃发送时应使用它，而不是 PROTOCOL_ERROR(0x01)。
// RST_STREAM error codes (RFC 9113 §7). CANCEL means "stream cancelled", not peer protocol violation. Use it for local backpressure/overload aborts, not PROTOCOL_ERROR (0x01).
#define CONST_HTTP2_STREAM_ERROR_PROTOCOL 0x01
// 流级窗口被对端的 WINDOW_UPDATE 顶穿 2^31-1：只撤这一条流，不带走整条连接
// （RFC 9113 §5.1 允许流级 FLOW_CONTROL_ERROR 与连接级两种处理，取较温和的那个）。
// Per-stream window blown past 2^31-1 by peer's WINDOW_UPDATE: RST_STREAM only, no GOAWAY. RFC 9113 §5.1 allows both per-stream and connection-level FLOW_CONTROL_ERROR; we pick the milder.
#define CONST_HTTP2_STREAM_ERROR_FLOW_CONTROL 0x03
#define CONST_HTTP2_STREAM_ERROR_CANCEL 0x08
#define CONST_HTTP2_STREAM_ERROR_ENHANCE_YOUR_CALM 0x0B
#endif
