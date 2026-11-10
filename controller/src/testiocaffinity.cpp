#include <thread>
#include "orm.h"
#include "httppeer.h"
#include "server.h"
#include "testiocaffinity.h"

namespace http
{
// 验证注解协程与 ORM 协程落在同一个 io_context：
// 会话 strand 的 context、协程 executor 的 context、ORM 连接的 io_ctx 三者相等，
// 且连接所属槽下标 == 该 ioc 在 io_contexts 里的下标。每个 ioc 只有一条 run 线程，
// 所以 await 前后线程号相同是同一件事的另一面。
//@urlpath(null,testiocaffinity)
asio::awaitable<std::string> test_ioc_affinity(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto ex          = co_await asio::this_coro::executor;
    auto &ctx        = asio::query(ex, asio::execution::context_as_t<asio::execution_context &>());
    auto &sess_ctx   = peer->socket_session->strand_.get_inner_executor().context();
    auto tid_before  = std::this_thread::get_id();

    auto model = orm::World();
    model.lock_conn();
    model.where("id", 1);
    co_await model.async_fetch_one();

    auto tid_after             = std::this_thread::get_id();
    asio::io_context *conn_ioc = (model.select_conn && model.select_conn->conn_link)
                                     ? model.select_conn->conn_link->io_ctx
                                     : nullptr;

    auto &app            = get_server_app();
    unsigned int ioc_idx = static_cast<unsigned int>(-1);
    unsigned int conn_slot = static_cast<unsigned int>(-1);
    for (unsigned int i = 0; i < app.io_contexts.size(); ++i)
    {
        if (app.io_contexts[i].get() == static_cast<asio::io_context *>(&ctx))
        {
            ioc_idx = i;
        }
    }
    for (unsigned int i = 0; i < model.conn_obj->slots.size(); ++i)
    {
        if (model.conn_obj->slots[i]->io_context_holder.get() == conn_ioc)
        {
            conn_slot = i;
        }
    }

    bool ok = (&ctx == &sess_ctx) && (conn_ioc == static_cast<asio::io_context *>(&ctx)) &&
              (ioc_idx == conn_slot) && (tid_before == tid_after) && !model.iserror;
    std::string err = model.error_msg;
    model.unlock_conn();

    // 指针/布尔先转成非模板重载吃得下的类型：httppeer 的模板 operator<< 定义只在
    // httppeer.cpp、无显式实例化，跨 TU 调 void*/bool 的版本会变成未定义引用。
    client << (ok ? "PASS" : "FAIL") << " ioc_idx=" << ioc_idx << " conn_slot=" << conn_slot
           << " ctx=" << reinterpret_cast<unsigned long long>(&ctx)
           << " sess=" << reinterpret_cast<unsigned long long>(&sess_ctx)
           << " conn=" << reinterpret_cast<unsigned long long>(conn_ioc)
           << " tid_same=" << ((tid_before == tid_after) ? 1 : 0) << " err=" << err;
    co_return "";
}
}// namespace http