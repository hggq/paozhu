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

// RPC 私有协议（vendor/httpserver/src/rpc_parse.cpp）的三道上界：
// 请求行累计字节（含查询串）、KV 段条数、KV 段总字节；超限按协议错误断连。
// RPC private-protocol caps: request-line cumulative bytes, KV count, KV total bytes.
#define CONST_RPC_LINE_MAX 8192
#define CONST_RPC_KV_MAX_ITEMS 64
#define CONST_RPC_KV_MAX_BYTES (256 * 1024)
// RPC 响应 body 在内存中的上限（字节）：声明大小 > 此值才落盘到临时文件，
// <= 此值留在 page.content 内存。与 WebSocket 的 16M 内存口径对齐。
// RPC response body in-memory cap: declared size above this spills to a temp file;
// at/below stays in page.content. Aligned with the WebSocket 16MB in-memory stance.
#define CONST_RPC_BODY_MEM_LIMIT (16 * 1024 * 1024)

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
// 条数是主尺子（list::size() O(1) 且无漂移）；字节尺子非维护计数，两把都在入队那一刻判：
// push 里同一把锁内现场遍历求和（content_list 由业务代码自行 pop_front，维护式计数必漂；
// 健康队列只有几条，堵到几十条才变贵，而那正是准备丢弃的路径）。
// 协程模式由派发包装协程成对 reserve/release。
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

// WebSocket 保活（每连接）。默认值：服务端不发 ping，空闲连接靠 IDLE_LIMIT 兜底回收。
//   IDLE_LIMIT     入站活动空闲上限（秒）。读循环每收到一批字节就把回收期限推到 now+该值，
//                  到点由 httpwatch 的超时扫描断连。该扫描还带 clean_cron 宽限
//                  （conf/server.conf 的 t600 ⇒ 实际断连比上限晚约 11 分钟）。
//   PING_INTERVAL  服务端主动发 ping 的周期（秒）；0 = 不发。空闲连接自己不会产生入站字节，
//                  要把 IDLE_LIMIT 设到比对端静默时长更短就得开 ping：对端的 pong 就是那条
//                  入站活动。半开的死连接一个字节都收不到，上限一到即回收。
//                  必须小于 IDLE_LIMIT，否则连接会在下一次 ping 之前就被回收掉。
// WebSocket keepalive per connection; defaults mean no ping and idle reaping after IDLE_LIMIT.
#define CONST_WEBSOCKET_IDLE_LIMIT 86400
#define CONST_WEBSOCKET_PING_INTERVAL 0

// WebSocket 客户端保活（每条出站连接）。默认 0 = 关闭，与未实现时行为一致。
//   客户端主动 ping 的周期（秒）。半开连接（对端进程僵死、NAT 静默过期）收不到任何入站字节，
//   只能靠 ping 换 pong 来证明对端还活着；连续 3 个周期没有入站字节即判死，交给看护线程回收。
//   0 之外还要求连接在 client_context 的超时链表里（async_connect / add_client_task_loop 都会登记）。
// WebSocket client keepalive ping interval in seconds; 0 disables it. Silence over 3 intervals kills the link.
#define CONST_WEBSOCKET_CLIENT_PING_INTERVAL 0

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

// 静态压缩缓存的年龄上限（秒，默认 7 天）：按文件的最后写入时间算，超过才回收。
// 命中会给大条目续期（见下面的 REFRESH_*），所以热点大缓存长期保留；其余条目
// 写入 7 天后到期，下一次请求再重新压缩。实际删除时刻还要加上一个扫除周期
// （httpwatch 的 temp_clean_time，默认 24 小时）。
// httpwatch_clear_temp_files 是非递归的，永远扫不到 statichtml 子树；ETag 含 mtime，
// 每次发版都会生成新键，没有回收就是单向增长。
// 不加 server.conf 键：缓存是纯派生数据，删掉最多重压一次。
// Age limit of the static compression cache (seconds, default 7d), measured from the file's
// last-write time. Hits refresh large entries (see REFRESH_* below), so hot large caches stay
// indefinitely; everything else expires 7 days after it was written and is recompressed on the
// next request. Actual deletion happens at the first sweep (temp_clean_time, default 24h) after
// that. httpwatch_clear_temp_files is non-recursive and never reaches the statichtml subtree;
// the ETag embeds mtime so every deploy mints a new key. No server.conf key: the cache is pure
// derived data; deleting it costs one recompress.
#define CONST_HTTP_STATIC_CACHE_AGE_LIMIT (3600 * 24 * 7)

// 命中续期门槛：源文件 ≥ 该值才续期。压缩比高的文件缓存可能只有几 KB，
// 但重压代价按源文件算，所以门槛用源文件长度，不用缓存文件长度。
// Hits refresh the last-write time only for sources at least this large: a highly compressible
// file may cache down to a few KB, while the recompression cost still follows the source size.
#define CONST_HTTP_STATIC_CACHE_REFRESH_MIN_SIZE (1024 * 1024)
// 只在条目年龄超过该值时写一次 mtime（默认半个回收周期）。
// Write the last-write time at most once per this age (default half the collection period).
#define CONST_HTTP_STATIC_CACHE_REFRESH_AGE ((CONST_HTTP_STATIC_CACHE_AGE_LIMIT) / 2)

#define COOKIE_SESSION_NAME "PHPSESSID"

// HTTP/1 keepalive 单连接请求数上限：一条连接累计应答到这么多请求就关连接。
// 口径含 CORS 预检——预检同样占一条连接的一次请求，只是不进业务；触发上限的这一条已完整应答。
// 编译期常量，不进 server.conf；开销是每轮循环一次比较（计数本来就在涨）。
// Max requests served on one HTTP/1 keepalive connection: once this many are answered, the
// connection closes after the current response is flushed. CORS preflight counts too — it occupies
// a request slot on the connection, it just does not run business code. The request that reaches the
// limit is answered in full. Compile-time constant, no server.conf key.
#define CONST_HTTP1_KEEPALIVE_MAX_REQUESTS 1000

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

// 本端 HPACK 解码动态表的字节上限。本端 SETTINGS 帧没有广告 0x01
// SETTINGS_HEADER_TABLE_SIZE，按 RFC 9113 §6.5.2 就等于这个默认 4096；对端在头块里发
// Dynamic Table Size Update 只能在本端上限以内调整（正常只会更小）。
// 对端自己 SETTINGS 里的 header_table_size 是它那边解码表的大小，管不到本端。
// Byte ceiling of our HPACK decoder dynamic table. Our SETTINGS frame sends no
// SETTINGS_HEADER_TABLE_SIZE (0x01), so per RFC 9113 §6.5.2 we advertise exactly this 4096;
// a peer's Dynamic Table Size Update may only resize within it. The peer's own advertised
// header_table_size bounds its decoder, not ours.
#define CONST_HTTP2_DYNAMIC_TABLE_MAX_SIZE 4096

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
// INTERNAL_ERROR(0x02)：响应已经承诺了 Content-Length，但本地读不出这么多字节
// （fseek 失败 / fread 短读 / 文件在传输途中被截断）。此时发 END_STREAM 等于用一个
// 长度不符的响应当正常结束，对端要解完 body 才发现短了；RST 能让它立刻判错。
// INTERNAL_ERROR(0x02): Content-Length was promised but the bytes aren't there locally
// (fseek failure / short fread / file truncated mid-transport). END_STREAM here would
// launder it as a normal-but-short response; RST lets the peer fail fast.
#define CONST_HTTP2_STREAM_ERROR_INTERNAL 0x02

// 请求对象复用缓冲的归还阈值（字节）。响应缓冲跟着连接活到下一个请求，每次清空都收缩
// 就等于每请求重新分配一次；只有被大响应撑到过容量阈值的才还回分配器。
// Reuse-buffer trim threshold (bytes). Response buffers live on to the next request; shrinking
// on every clear means a fresh allocation each time. Only buffers whose capacity grew past this
// are returned to the allocator.
#define CONST_PEER_BUFFER_SHRINK_SIZE 131072

// OAuth 2.1 授权服务器（libs/oauth + controller/src/oauth）的编译期常量。下面每一块自带单位。
// 授权码 5 分钟、access token 15 分钟；refresh 的这个值是配置文件里没写 refresh_ttl 时的兜底值。
// refresh 每次轮换都按"现在 + 有效期"重算到期时间，所以它是闲置期而不是绝对上限：
// 一直轮换就一直在续，要做绝对上限得让族根自己携带到期时间。
// OAuth 2.1 authorization server constants; each block below states its own unit.
// Code 5 min, access token 15 min; this refresh value is the fallback when conf/oauth.conf
// leaves refresh_ttl unset. Rotation recomputes "now + ttl", so it is an idle window, not a cap.
#define CONST_OAUTH_CODE_TTL 300
#define CONST_OAUTH_ACCESS_TTL 900
#define CONST_OAUTH_REFRESH_TTL 2592000

// 过期行清理：节拍每小时一拍，删除门槛是"这一行自己的 expires_at 早于 now - 保留期"。
// 保留期必须大于 refresh 的有效期：revoked 的 refresh 行是重放检测的依据，
// 按"已撤销就立刻删"会让轮换过的旧 token 再拿到时查无此据。
// 配置文件里的 refresh_ttl 也受这条约束，读的时候就不让写进不小于保留期的值。
// Expired-row purge: hourly tick, deletes rows whose own expires_at is older than now - keep.
// Keep must exceed the refresh TTL: revoked refresh rows are the replay evidence for rotation.
#define CONST_OAUTH_PURGE_INTERVAL 3600
#define CONST_OAUTH_PURGE_KEEP 7776000
// 每一拍最多删多少行。DELETE ... LIMIT 是给 InnoDB 的 undo 与主从延迟封顶用的：
// 表里攒了几百万行过期码时，宁可几十拍打回去，不要一拍锁住整张表。
// Rows deleted per tick; a few ticks is better than one statement that locks the whole table.
#define CONST_OAUTH_PURGE_BATCH 1000
// 一拍里最多连着删几轮，每张表各这么多轮。某轮删不满 CONST_OAUTH_PURGE_BATCH 就收手，
// 所以空闲的一拍只有两条语句；这个上限只在"积压真的来了"时才起作用。
// Rounds per tick, per table; a short round ends the loop, so an idle tick costs two statements.
#define CONST_OAUTH_PURGE_MAX_ROUNDS 20

// RS256 密钥对的文件名，conf/oauth.conf 没写 private_key_file / public_key_file 时用它兜底。
// 口径与 server.conf 的 certificate_chain_file 相同：相对 server.conf 所在目录，绝对路径原样用。
// 私钥只用于签发，公钥同时用于 Bearer 验签和 /oauth/jwks 输出，必须是
// PEM_read_bio_PUBKEY 读得出的 "PUBLIC KEY" 格式（jwt.cpp 的 verify 只认这种）。
// 两个文件缺任何一个时，token / revoke / bearerauth 一律 500 server_error，
// 不降级到对称算法。
// Default RS256 key file names, used when conf/oauth.conf leaves the entry blank. Resolved
// against the directory that holds server.conf, same as certificate_chain_file.
// Missing either file makes token/revoke/bearerauth answer 500 server_error; there is no
// fallback to a symmetric alg.
#define CONST_OAUTH_PRIVKEY_FILE "oauth_rsa_private.pem"
#define CONST_OAUTH_PUBKEY_FILE "oauth_rsa_public.pem"
// 密钥文件的读取上限（字节）。每次请求现读这两个文件，路径又来自配置，所以先把尺寸钳住：
// 有人把 private_key_file 写成一个日志文件时，代价是"这一单签失败"，不是把整份日志读进内存。
// Per-request key files are capped at this size; an oversized file is a signing failure,
// not a memory spike.
#define CONST_OAUTH_PEM_MAX_SIZE 65536
// JWT 头里的 kid，也是 /oauth/jwks 里那一枚密钥的 kid。轮换密钥时要一起改。
// Fixed kid, also emitted in /oauth/jwks. Change it together with a key rotation.
#define CONST_OAUTH_KID "oauth-rs256-1"
// kid 的最大长度。它要原样拼进 JWT 头，所以先按"配置里的短标识"钳住长度，字符集另判。
// Max kid length: the value is spliced straight into the JWT header, so it is bounded first.
#define CONST_OAUTH_KID_MAX_LEN 64
// verify 的时钟容差：签发方与校验方同一台，留 30 秒只兜 NTP 抖动。
// Clock skew tolerance for jwt::verify (issuer and verifier are the same process).
#define CONST_OAUTH_JWT_LEEWAY 30
// jwt::verify() 入口对整枚令牌的长度界。三段全部解出来才判形状、扫 claim、验签，这一界是先拒
// 再干活。取"单枚头部行"的同一个数：令牌从 Authorization 头进来，入站已经按它钳过一遍，所以框架
// 能收到的令牌一定在界内；直接喂网络字节的调用点（不走 http 的）才有真正的作用。
// Cap on the whole token at the verify() entry, checked before any segment is decoded. It is
// the per-header-line ceiling: a token arrives in the Authorization header, which the inbound
// parser already bounds by the same value, so nothing the transport can deliver is refused.
#define CONST_JWT_TOKEN_MAX_LEN CONST_HTTP_HEADER_BODY_SIZE
// introspect 的 token 参数长度上限。自己签的 JWT 连签名不到 1 KB（2048 位模数与签名各 342 个
// base64url 字符），opaque 令牌恒 43 字符；4096 是给"将来换更长的密钥或更多 claim"留的余量。
// 超长的提交在做任何验签或查库之前就回 active:false——那一格的答案与"查无此令牌"同形，
// 所以这个闸不泄露"你猜的长度对不对"。
// Cap on the introspect token parameter; over-length input is answered before any crypto or
// query, with the same body as "no such token".
#define CONST_OAUTH_TOKEN_MAX_LEN 4096
// 客户端注册的列宽上界，跟 oauth_client 的列声明一致（client_id varchar(64)、name varchar(128)）。
// 超长的提交在这里拒掉；MySQL 非严格模式只会静默截断，截断后的 client_id 就没人认识了。
// Registration input bounds, matching oauth_client's column widths. Over-length input is refused
// rather than silently truncated by MySQL's non-strict mode.
#define CONST_OAUTH_CLIENT_ID_MAX_LEN 64
#define CONST_OAUTH_CLIENT_NAME_MAX_LEN 128
// 两列策略串的列宽上界（allowed_scopes varchar(255)、grant_types varchar(127)）。
// 超长同样是入口拒、不靠库的静默截断：截断后的白名单会少几项，表现为"合法 scope 换不到令牌"。
// Policy-string bounds matching the two column widths; over-length input is refused at the
// registration endpoint instead of being silently truncated.
#define CONST_OAUTH_CLIENT_SCOPES_MAX_LEN 255
#define CONST_OAUTH_CLIENT_GRANTS_MAX_LEN 127
// redirect_uris 那一格是 TEXT（列宽 65535），所以下面这个数不是列宽，是策略界：它钳的是逗号分隔的
// 整串。8192 按"一个客户端挂几条回调地址"给量（一条 URL 约 120 字符，放得下几十条），同时只有列宽
// 的八分之一，离静默截断还有余量——截短的白名单少几项，表现成"合法回调换不到码"，
// 和上面那两列同一个道理，一样在入口拒。
// Policy bound (not a column width) on the whole comma-separated redirect_uris string. The column
// is TEXT, so this only has to stay clear of silent truncation while leaving room for many
// redirect URIs per client.
#define CONST_OAUTH_REDIRECT_URIS_MAX_LEN 8192

// GitHub OAuth 客户端这一侧（libs/github + controller/src/github）的编译期常量。
// 与上一段的授权服务器是两套东西：这里的凭据跟着每个 admin 的绑定落在 github_account 行里，
// conf/github.conf 只存一把 master key 和通用参数，不存任何 client 凭据。
// OAuth-client-side constants. Client credentials live per binding in github_account;
// conf/github.conf holds only the master key and shared parameters.
// AES-256-GCM 的密钥长度（字节）。配置文件里写成 64 个 hex 字符，读出来必须是这 32 字节，
// 短了就补零、长了就截断都会让同一把"配错的密钥"照样能加解密，换回来的是一堆解不开的密文。
// AES-256 key length in bytes; a hex string of the wrong length is rejected, not padded or cut.
#define CONST_GH_CRYPTO_KEY_LEN 32
// client_secret 的**明文**长度闸。GitHub 现发的 secret 是 40 字符 hex，128 留了三倍余量；
// 这一格必须比列宽小，因为落库存的是密文（明文 + 12 IV + 16 tag）。
// Plaintext bound for client_secret; the column stores ciphertext, so this gate is 28 bytes
// below the column width.
#define CONST_GH_SECRET_MAX_LEN 128
// 密文落库前的列宽闸（varbinary(255)）。越界宁可拒写也不要交给 MySQL 截断——
// 截掉的 tag 会让这一行永远解不开，且没有任何报错指向截断。
// Ciphertext bound matching the varbinary(255) column; over-length is refused rather than
// truncated by the server.
#define CONST_GH_CIPHER_MAX_LEN 255
// 令牌与 client_id 的列宽闸（varchar(255) / varchar(64)），与 github_account 的声明一致。
// Token and client_id bounds, matching the github_account column widths.
#define CONST_GH_TOKEN_MAX_LEN 255
#define CONST_GH_CLIENT_ID_MAX_LEN 64
// redirect_uri 的列宽闸（varchar(255)）。它是绑定表单里唯一由 admin 整段粘进来的 URL，
// 也是换令牌时"必须逐字节相同"的那一项——超限交给 MySQL 截断，下一次换令牌就会以
// 一个看不出原因的 error 失败。
// Bound for the redirect_uri column; letting the server truncate it would make every later
// token exchange fail with a reason that points nowhere.
#define CONST_GH_REDIRECT_MAX_LEN 255
// state 票据的有效期（秒）。GitHub 那枚 code 是 10 分钟过期，这一格必须严格大于它，
// 否则票据先于 code 失效，用户授权完了我们却不认。
// state lifetime in seconds; strictly greater than GitHub's 10-minute code expiry.
#define CONST_GH_STATE_TTL 900
// code_verifier 的字符集长度下限（RFC 7636 是 43..128）。
// PKCE code_verifier bounds (RFC 7636 allows 43..128 characters).
#define CONST_GH_PKCE_VERIFIER_MIN 43
#define CONST_GH_PKCE_VERIFIER_MAX 128
// 同步节拍（秒）与每一拍的账号数上限。间隔 900 秒 + 每账号每轮最多 1+3 次请求，
// 折合约 16 次/小时，对 5000 次/小时/令牌的额度留了两个数量级以上的余量。
// Sync tick in seconds and accounts per tick; about 16 requests/hour per account.
#define CONST_GH_SYNC_INTERVAL 900
#define CONST_GH_SYNC_BATCH 5
// 每账号每轮最多翻多少页仓库。这是额度闸，不是"翻页翻到爽"的开关：
// 上限乘每页条数就是这一轮能看见的仓库数，超过的部分由"没拉到"推断，所以软删对比
// 必须先确认这一轮真的翻完了。
// Pages per account per round; bounds how many repos one round can see, which is what the
// soft-delete comparison has to check before it marks anything.
#define CONST_GH_SYNC_MAX_PAGES 3
// 仓库列表一页多少条，GitHub 的上限就是 100。
// Repos per page; GitHub's own ceiling.
#define CONST_GH_REPO_PER_PAGE 100
// 连续失败后 next_sync_time 的指数退避封顶（秒）。
// Ceiling for the exponential backoff applied to next_sync_time after consecutive failures.
#define CONST_GH_SYNC_BACKOFF_MAX 3600
// 出站 HTTP 超时（秒）。框架内部把 exptime 钳在 90，写更大的值不会生效。
// Outbound HTTP timeout in seconds; the client clamps it to 90 internally.
#define CONST_GH_HTTP_TIMEOUT 8
// 两个端点的默认值，conf/github.conf 里留空时用它。必须是 https：client_secret 和令牌
// 都从这两条口出去，写成 http 就是明文上网。
// Default endpoints, used when conf/github.conf leaves the key blank. Must be https:
// the client_secret and the tokens go out over these two.
#define CONST_GH_API_BASE "https://api.github.com"
#define CONST_GH_AUTH_BASE "https://github.com"
// 默认 scope。逗号是配置文件的写法，发授权请求前要换成空格再整体 url_encode。
// read:user 是强制附加项：没有它 GET /user 回 403，绑定行上的身份三列永远填不上。
// Default scopes, stored comma-separated in conf and converted to space-separated at
// request time. read:user is appended unconditionally; without it GET /user is a 403.
#define CONST_GH_DEFAULT_SCOPES "repo,read:user"
// last_error 里要落库的文本长度闸（列宽 255）。落库前还要过一遍敏感片段过滤，
// 这一格限的是过滤之后再截断的长度。
// Bound for the last_error text, applied after the secret-scrubbing pass.
#define CONST_GH_ERROR_MAX_LEN 255
// github_repo.description 的列宽（varchar(512)）。GitHub 侧的 description 没有长度上限，
// 所以这一刀必须在写库前下：非严格模式下的静默截断会把句子腰斩，严格模式下直接报错丢一整页。
// Column width for the cached repo description; GitHub puts no bound on it, so the cut
// happens before the write instead of inside MySQL's silent truncation.
#define CONST_GH_REPO_DESC_MAX_LEN 512
// gh_avatar 的列宽（varchar(255)）。头像 URL 由 GitHub 给，长度不可预期，同上一条口径。
// Column width for the avatar URL; GitHub decides its length, so it is bounded here too.
#define CONST_GH_AVATAR_MAX_LEN 255
// sync_fails 那一列是 smallint unsigned，写入前先钳住：这个数只用来算退避的档位，
// 满了就停在界里，不让一次溢出把"往后推得越久"变成"回到最小间隔重试"。
// sync_fails is smallint unsigned; clamp before writing so an overflow cannot reset the
// backoff ladder to its lowest rung.
#define CONST_GH_SYNC_FAILS_MAX 65535
#endif
