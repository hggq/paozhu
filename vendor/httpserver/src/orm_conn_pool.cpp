/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <map>
#include <list>
#include <array>
#include <vector>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#if defined(_WIN32) && !defined(_OPENSSL_Applink) && \
    defined(HAVE_OPENSSL_APPLINK_C)
#include <openssl/applink.c>
#endif
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "mysql_conn.h"
#include "orm_conn_pool.h"
#include "pg_conn.h"
#include "sqlite_conn.h"
#include "orm_common.h"
#include "cost_define.h"
#include "parse_ini.h"
#include <algorithm>
#include <filesystem>
#include <cctype>

namespace orm
{
bool orm_conn_pool::is_postgresql() const
{
    return conf_data[0].db_type == DB_TYPE::POSTGRESQL;
}
bool orm_conn_pool::is_sqlite() const
{
    return conf_data[0].db_type == DB_TYPE::SQLITE;
}
bool str_casecmp(std::string_view str1, std::string_view str2)
{
    if (str1.size() != str2.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < str1.size(); i++)
    {
        if (str1[i] != str2[i])
        {
            if (str1[i] < 91 && str1[i] > 64)
            {
                if ((str1[i] + 32) == str2[i])
                {
                    continue;
                }
            }
            else if (str2[i] < 91 && str2[i] > 64)
            {
                if (str1[i] == (str2[i] + 32))
                {
                    continue;
                }
            }
            return false;
        }
    }
    return true;
}
std::map<std::string, std::shared_ptr<orm_conn_pool>> &get_orm_conn_pool_obj()
{
    static std::map<std::string, std::shared_ptr<orm_conn_pool>> instance;
    return instance;
}
// ---- 4 个内部 helper ----
static std::string normalize_tag(const std::string &raw)
{
    std::string out;
    for (char ch : raw)
    {
        if (out.size() > 20)
            break;
        if (ch >= '0' && ch <= '9')
            out += ch;
        else if (ch == '_')
            out += ch;
        else if (ch >= 'A' && ch <= 'Z')
            out += (ch + 32);
        else if (ch >= 'a' && ch <= 'z')
            out += ch;
    }
    return out;
}

static bool issock_suffix(const std::string &host)
{
    if (host.size() < 5)
        return false;
    return host.compare(host.size() - 5, 5, ".sock") == 0;
}

static bool parse_bool(const std::string &v)
{
    return v == "1" || v == "true" || v == "True" || v == "TRUE" || v == "On" || v == "ON";
}

static DB_TYPE parse_dbtype(const std::string &v)
{
    if (v == "postgresql" || v == "pg" || v == "POSTGRESQL" || v == "PG")
        return DB_TYPE::POSTGRESQL;
    if (v == "sqlite" || v == "sqlite3" || v == "sq3" || v == "SQLITE" || v == "SQLITE3")
        return DB_TYPE::SQLITE;
    return DB_TYPE::MYSQL;
}

static unsigned char clamp_stoi(const std::string &s, unsigned char def)
{
    try
    {
        int v = std::stoi(s);
        if (v < 0)
            return 0;
        if (v > 255)
            return 255;
        return (unsigned char)v;
    }
    catch (...)
    {
        return def;
    }
}

// 判断字符串是否含口令不允许的字节。统一约束：orm.conf 中 mysql/pg/sqlite 的口令只允许
// 可打印 ASCII 的 0x21 ~ 0x7E（! 到 ~），空格(0x20)、全部控制码(0x00~0x1F、0x7F)以及
// 非 ASCII 字节(>=0x80)一律非法，解析时发现即打印错误并跳过该条配置，而非等到连接阶段才失败。
static bool contains_forbidden_password_char(const std::string &s)
{
    for (unsigned char ch : s)
    {
        if (ch < 0x21 || ch > 0x7E)
            return true;
    }
    return false;
}

std::vector<orm_conn_t> get_orm_config_file(const std::string &filename)
{
    std::vector<orm_conn_t> result;
    auto charset_obj = std::make_unique<mysql_charset_store>();
    charset_obj->mysql_charset_init();

    namespace fs = std::filesystem;
    if (!fs::exists(filename))
        return result;

    http::parse_ini ini;
    ini.parse_file(filename);

    for (auto &section : ini.config)
    {
        std::string tag = normalize_tag(section.name);
        if (tag.empty())
            continue;

        auto groups = section.value.splits();// 默认 skip_empty_name=true, 跳过注释行
        for (const auto &group : groups)
        {
            orm_conn_t c;
            c.tag         = tag;
            c.db_type     = DB_TYPE::MYSQL;// 默认
            c.link_type   = 0;
            c.max_pool    = 0;
            c.min_pool    = 0;
            c.issock      = false;
            c.islocal     = false;
            c.isssl       = false;
            c.sslverify   = false;
            c.isdebug     = false;
            c.charset_val = 0;

            for (const auto &kv : group)
            {
                const std::string &k = kv.name;
                const std::string &v = kv.value;

                if (k == "type")
                    c.link_type = (v == "main") ? 0 : 1;
                else if (k == "host")
                {
                    c.host    = v;
                    c.issock  = issock_suffix(v);
                    c.islocal = (v == "127.0.0.1" || v == "localhost");
                }
                else if (k == "port")
                    c.port = v;
                else if (k == "dbname")
                    c.dbname = v;
                else if (k == "user")
                    c.user = v;
                else if (k == "password")
                    c.password = v;
                else if (k == "pretable")
                    c.pretable = v;
                else if (k == "maxpool")
                    c.max_pool = clamp_stoi(v, 2);
                else if (k == "minpool")
                    c.min_pool = clamp_stoi(v, 2);
                else if (k == "ssl")
                    c.isssl = parse_bool(v);
                else if (k == "sslverify")
                    c.sslverify = parse_bool(v);
                else if (k == "sslhost")
                    c.sslhost = v;
                else if (k == "server_public_key")
                    c.server_public_key = v;
                else if (k == "debug")
                    c.isdebug = parse_bool(v);
                else if (k == "dbtype")
                {
                    c.dbtype  = v;
                    c.db_type = parse_dbtype(v);
                }
                else if (k == "charset")
                {
                    std::string low = v;
                    std::transform(low.begin(), low.end(), low.begin(), [](unsigned char x)
                                   { return std::tolower(x); });
                    c.charset     = low;
                    c.charset_val = charset_obj->mysql_charset_find(low);
                }
            }

            // 统一口令规则：mysql/pg/sqlite 的口令只允许可打印 ASCII 0x21~0x7E
            // （不含空格 0x20 与控制码），发现非法字节即报错并跳过该条配置，不进入连接池。
            if (!c.password.empty() && contains_forbidden_password_char(c.password))
            {
                std::string dtype_name = "mysql";
                if (c.db_type == DB_TYPE::POSTGRESQL)
                    dtype_name = "postgresql";
                else if (c.db_type == DB_TYPE::SQLITE)
                    dtype_name = "sqlite";

                std::cerr << "[orm.conf] ERROR: password allows printable ASCII 0x21-0x7E only "
                             "(spaces/control/non-ASCII bytes are forbidden). "
                          << "Skip config [tag=" << tag
                          << ", dbtype=" << dtype_name
                          << (c.host.empty() ? "" : ", host=" + c.host)
                          << (c.dbname.empty() ? "" : ", dbname=" + c.dbname)
                          << (c.user.empty() ? "" : ", user=" + c.user)
                          << "]. Reset the database password to characters in 0x21-0x7E."
                          << std::endl;
                continue;
            }
            result.push_back(std::move(c));
        }
    }

    charset_obj->mysql_charset_clear();
    return result;
}

namespace
{
// 角色下标：与 conf_data / orm_slot 的两套计数一致
constexpr unsigned int LINK_EDIT   = 0;
constexpr unsigned int LINK_SELECT = 1;

// 槽数组是否已经与本次传入的 ioc 集合一一对齐。重新 init 时若对齐就保留现有连接，
// 避免同一 tag 的 edit/select 两次配置互相清池。
bool pool_slots_match(const orm_conn_pool &pool,
                      const std::vector<std::shared_ptr<asio::io_context>> &iocs)
{
    if (pool.slots.size() != iocs.size())
    {
        return false;
    }
    for (std::size_t i = 0; i < iocs.size(); ++i)
    {
        if (pool.slots[i]->io_context != iocs[i].get())
        {
            return false;
        }
    }
    return true;
}
}// namespace

orm_slot &orm_conn_pool::slot_ref(unsigned int slot_index)
{
    if (slots.empty())
    {
        // 兼容直接 make_shared<orm_conn_pool>() 的老用法（httpcli 代码生成器）：
        // 这类调用不经过 init，先手工设置 io_context 再取连接。
        auto slot        = std::make_unique<orm_slot>();
        slot->io_context = io_context;
        slots.emplace_back(std::move(slot));
    }
    if (slot_index >= slots.size())
    {
        slot_index = 0;
    }
    orm_slot &slot = *slots[slot_index];
    if (!slot.io_context)
    {
        slot.io_context = io_context;
    }
    return slot;
}

unsigned int orm_conn_pool::slot_of(const asio::execution_context &ctx) const
{
    if (ioc_index)
    {
        auto iter = ioc_index->find(&ctx);
        if (iter != ioc_index->end() && iter->second < slots.size())
        {
            return iter->second;
        }
    }
    // 未注册的 context / 越界一律回退槽 0
    return 0;
}

unsigned int orm_conn_pool::slot_of(const asio::io_context *ioc) const
{
    if (!ioc)
    {
        return 0;
    }
    return slot_of(static_cast<const asio::execution_context &>(*ioc));
}

unsigned int orm_conn_pool::max_pool_of(unsigned int link_type) const
{
    unsigned int m = conf_data[link_type & 1u].max_pool;
    // 未配置（0）按 1
    return m == 0 ? 1u : m;
}

std::string init_orm_conn_pool_release()
{
    std::map<std::string, std::shared_ptr<orm_conn_pool>> &int_pool = get_orm_conn_pool_obj();

    for (auto iter = int_pool.begin(); iter != int_pool.end(); iter++)
    {
        // clear_* 内部遍历所有槽，调用点不变
        iter->second->clear_select_conn();
        iter->second->clear_edit_conn();
    }

    return "";
}

std::string init_orm_conn_pool(
    std::vector<std::shared_ptr<asio::io_context>> iocs,
    std::shared_ptr<const std::unordered_map<const asio::execution_context *, unsigned int>> ioc_index,
    const std::string &orm_config_file)
{
    iocs.erase(std::remove(iocs.begin(), iocs.end(), nullptr), iocs.end());
    if (iocs.empty())
    {
        return "init_orm_conn_pool failed: io_context is null\n";
    }

    std::map<std::string, std::shared_ptr<orm_conn_pool>> &int_pool = get_orm_conn_pool_obj();
    std::vector<orm_conn_t> myconfig                                = get_orm_config_file(orm_config_file);
    std::string error_log;

    error_log.append("-- begin init_orm_conn_pool -- \n");
    for (auto &item : myconfig)
    {
        if (item.link_type > 1)
        {
            continue;
        }
        if (item.min_pool < 1)
        {
            item.min_pool = 1;
        }

        auto iter = int_pool.find(item.tag);
        std::shared_ptr<orm_conn_pool> conn;
        if (iter != int_pool.end())
        {
            conn = iter->second;
        }
        else
        {
            conn = std::make_shared<orm_conn_pool>();
        }

        // ioc 集合变了（或首次建池）：旧连接必须先关，再按 iocs 下标重建槽。
        // 同一 tag 的 edit/select 两次配置传入同一组 ioc，第二次不会重复清池。
        if (!pool_slots_match(*conn, iocs))
        {
            conn->clear_select_conn();
            conn->clear_edit_conn();
            conn->slots.clear();
            for (auto &ioc : iocs)
            {
                auto slot               = std::make_unique<orm_slot>();
                slot->io_context_holder = ioc;
                slot->io_context        = ioc.get();
                conn->slots.emplace_back(std::move(slot));
            }
        }
        conn->ioc_index  = ioc_index;
        conn->io_context = conn->slots[0]->io_context;
        conn->conf_data[item.link_type] = item;

        unsigned int n = 0;
        try
        {
            // 槽 0 预建 min_pool，其余槽懒建（0 条）；SQLite 单连接只用槽 0。
            for (unsigned int si = 0; si < conn->slots.size(); ++si)
            {
                unsigned char prebuild = (si == 0) ? item.min_pool : 0;
                if (item.db_type == DB_TYPE::POSTGRESQL)
                {
                    n += (item.link_type == 0)
                             ? conn->init_pg_edit_conn(prebuild, si)
                             : conn->init_pg_select_conn(prebuild, si);
                }
                else if (item.db_type == DB_TYPE::SQLITE)
                {
                    n += (item.link_type == 0)
                             ? conn->init_sqlite_edit_conn(prebuild, si)
                             : conn->init_sqlite_select_conn(prebuild, si);
                    break;// SQLite 单连接只用槽 0
                }
                else
                {
                    n += (item.link_type == 0)
                             ? conn->init_mysql_edit_conn(prebuild, si)
                             : conn->init_mysql_select_conn(prebuild, si);
                }
            }
        }
        catch (const std::exception &e)
        {
            error_log.append(e.what());
        }
        if (n == 0)
        {
            conn->error_msg.append(" int_pool.init_");
            conn->error_msg.append(item.link_type == 0 ? "edit" : "select");
            conn->error_msg.append("_conn failed for tag " + item.tag);
        }
        if (iter == int_pool.end())
        {
            int_pool.emplace(item.tag, conn);
        }
    }
    error_log.append("\n-- end init_orm_conn_pool -- \n");
    return error_log;
}

std::string init_orm_conn_pool(std::shared_ptr<asio::io_context> ioc,
                               const std::string &orm_config_file)
{
    if (!ioc)
    {
        return "init_orm_conn_pool failed: io_context is null\n";
    }
    std::vector<std::shared_ptr<asio::io_context>> iocs;
    iocs.emplace_back(std::move(ioc));
    // 单 ioc 不需要反查表：slot_of 查不到时固定回退槽 0。
    return init_orm_conn_pool(std::move(iocs), nullptr, orm_config_file);
}

std::string init_orm_conn_pool(asio::io_context &ioc, const std::string &orm_config_file)
{
    // 非拥别名（deleter 空操作）：pool 不因此延长 ioc 的寿命，退出期关闭 socket
    // 前调用方必须自己把连接关完。
    return init_orm_conn_pool(std::shared_ptr<asio::io_context>(&ioc,
                                                                [](asio::io_context *) {}),
                              orm_config_file);
}

// ======================== MySQL connections ========================

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_mysql_edit_connect(orm_slot &slot)
{
    auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::MYSQL);
    auto conn = std::make_shared<mysql_conn_base>(std::move(link));
    bool isok = co_await conn->async_connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        conn->conn_link->bind_slot_total(slot.total_count[LINK_EDIT]);
        co_return conn;
    }
    throw std::runtime_error(" add_mysql_edit_connect failed for tag " + conn->error_msg);
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_mysql_edit_connect()
{
    co_return co_await async_add_mysql_edit_connect(slot_ref(0));
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::add_mysql_edit_connect(orm_slot &slot)
{
    auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::MYSQL);
    auto conn = std::make_shared<mysql_conn_base>(std::move(link));
    bool isok = conn->connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        conn->conn_link->bind_slot_total(slot.total_count[LINK_EDIT]);
        return conn;
    }
    throw std::runtime_error(" add_mysql_edit_connect failed for tag " + conn->error_msg);
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::add_mysql_edit_connect()
{
    return add_mysql_edit_connect(slot_ref(0));
}

void orm_conn_pool::back_mysql_edit_conn(std::shared_ptr<mysql_conn_base> conn)
{
    // 约定：事务经 orm_query.h edit_query 独立使用（外层异常包裹自管回滚），
    // 归还仅检查连接未断开，不做事务清理
    if (!conn)
    {
        return;
    }
    // 归还按连接自己的 ioc 定位槽，不按归还时的 executor：连接可能跨槽借出。
    orm_slot &slot = slot_ref(slot_of(conn->conn_link ? conn->conn_link->io_ctx : nullptr));
    if (conn->isclose)
    {
        // 连接已断开：丢弃这条对象，名额随它的析构归还
        return;
    }
    std::unique_lock<std::mutex> lock(slot.edit_mutex);
    if (conn->pooled_)
    {
        std::cerr << "[orm_pool] duplicate return ignored: mysql_edit" << std::endl;
    }
    else
    {
        conn->pooled_ = true;
        conn->issynch = false;
        slot.mysql_edit_pool.emplace_back(std::move(conn));
    }
}

unsigned int orm_conn_pool::init_mysql_edit_conn(unsigned char n, unsigned int slot_index)
{
    orm_slot &slot   = slot_ref(slot_index);
    unsigned int i   = 0;
    for (; i < n; ++i)
    {
        try
        {
            auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::MYSQL);
            auto conn = std::make_shared<mysql_conn_base>(std::move(link));
            bool isok = conn->connect(conf_data[0]);
            if (isok)
            {
                if (conf_data[0].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(slot.edit_mutex);
                slot.mysql_edit_pool.emplace_back(conn);
                conn->conn_link->bind_slot_total(slot.total_count[LINK_EDIT]);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw std::runtime_error(error_msg);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::add_mysql_select_connect(orm_slot &slot)
{
    auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::MYSQL);
    auto conn = std::make_shared<mysql_conn_base>(std::move(link));
    bool isok = conn->connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        conn->conn_link->bind_slot_total(slot.total_count[LINK_SELECT]);
        return conn;
    }
    throw std::runtime_error(" add_mysql_select_connect failed for tag ");
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::add_mysql_select_connect()
{
    return add_mysql_select_connect(slot_ref(0));
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_mysql_select_connect(orm_slot &slot)
{
    auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::MYSQL);
    auto conn = std::make_shared<mysql_conn_base>(std::move(link));
    bool isok = co_await conn->async_connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        conn->conn_link->bind_slot_total(slot.total_count[LINK_SELECT]);
        co_return conn;
    }
    throw std::runtime_error(" add_mysql_select_connect failed for tag ");
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_mysql_select_connect()
{
    co_return co_await async_add_mysql_select_connect(slot_ref(0));
}

unsigned int orm_conn_pool::init_mysql_select_conn(unsigned char n, unsigned int slot_index)
{
    orm_slot &slot   = slot_ref(slot_index);
    unsigned int i   = 0;
    for (; i < n; ++i)
    {
        try
        {
            auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::MYSQL);
            auto conn = std::make_shared<mysql_conn_base>(std::move(link));
            bool isok = conn->connect(conf_data[1]);
            if (isok)
            {
                if (conf_data[1].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(slot.select_mutex);
                slot.mysql_select_pool.emplace_back(conn);
                conn->conn_link->bind_slot_total(slot.total_count[LINK_SELECT]);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw std::runtime_error(error_msg);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}

void orm_conn_pool::back_mysql_select_conn(std::shared_ptr<mysql_conn_base> conn)
{
    if (!conn)
    {
        return;
    }
    orm_slot &slot = slot_ref(slot_of(conn->conn_link ? conn->conn_link->io_ctx : nullptr));
    if (conn->isclose)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(slot.select_mutex);
    if (conn->pooled_)
    {
        std::cerr << "[orm_pool] duplicate return ignored: mysql_select" << std::endl;
    }
    else
    {
        conn->pooled_ = true;
        conn->issynch = false;
        slot.mysql_select_pool.emplace_back(std::move(conn));
    }
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_get_mysql_edit_conn()
{
    auto ex                            = co_await asio::this_coro::executor;
    // any_io_executor 不支持 query(execution::context)，用通用的 context_as 查询取底层 context。
    const asio::execution_context &ctx =
        asio::query(ex, asio::execution::context_as_t<asio::execution_context &>());
    orm_slot &slot                     = slot_ref(slot_of(ctx));

    const unsigned int maxn = max_pool_of(LINK_EDIT);
    asio::steady_timer timer(ex);
    std::chrono::milliseconds backoff(1);
    // 本槽该角色名额已满后的最长等待；到点抛异常退出，请求不会一直占着协程。
    const auto wait_deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(CONST_ORM_CONN_WAIT_MS);

    for (;;)
    {
        {
            std::unique_lock<std::mutex> lock(slot.edit_mutex);
            while (!slot.mysql_edit_pool.empty())
            {
                auto temp = std::move(slot.mysql_edit_pool.front());
                slot.mysql_edit_pool.pop_front();
                temp->pooled_ = false;
                if (!temp->is_closed())
                {
                    lock.unlock();
                    co_return temp;
                }
                // 空闲连接已断开：丢弃这条对象，名额随它的析构归还
            }

            if (slot.total_count[LINK_EDIT]->load(std::memory_order_relaxed) +
                    slot.connecting[LINK_EDIT].load(std::memory_order_relaxed) <
                maxn)
            {
                // 锁内只占坑，建连在锁外
                slot.connecting[LINK_EDIT].fetch_add(1, std::memory_order_relaxed);
                lock.unlock();

                try
                {
                    auto conn = co_await async_add_mysql_edit_connect(slot);
                    slot.connecting[LINK_EDIT].fetch_sub(1, std::memory_order_relaxed);
                    co_return conn;
                }
                catch (...)
                {
                    slot.connecting[LINK_EDIT].fetch_sub(1, std::memory_order_relaxed);
                    throw;
                }
            }
        }

        // 已达本槽上限：短退避后重试，等归还或清理把名额让出来。
        auto now       = std::chrono::steady_clock::now();
        if (now >= wait_deadline)
        {
            // 到点仍没等到名额：抛异常退出，由 ORM 的 catch 写成 error_msg。
            // 退避重试不保证先到先得，等满上限就放弃，不让请求无限占着协程。
            throw std::runtime_error(std::string(" orm conn pool wait timeout for tag ") +
                                     conf_data[0].tag + " role=edit" +
                                     " max_pool=" + std::to_string(maxn) +
                                     " total=" + std::to_string(slot.total_count[LINK_EDIT]->load(std::memory_order_relaxed)) +
                                     " connecting=" + std::to_string(slot.connecting[LINK_EDIT].load(std::memory_order_relaxed)) +
                                     " wait_ms=" + std::to_string(CONST_ORM_CONN_WAIT_MS));
        }
        auto remain   = std::chrono::duration_cast<std::chrono::milliseconds>(wait_deadline - now);
        auto wait_for = backoff < remain ? backoff : remain;
        timer.expires_after(wait_for);
        co_await timer.async_wait(asio::use_awaitable);
        if (backoff < std::chrono::milliseconds(50))
        {
            backoff *= 2;
        }
    }
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::get_mysql_edit_conn()
{
    // 同步路径固定槽 0。业务线程池没有 executor，阻塞等门闩会拖死工作线程，
    // 因此保持旧语义：无空闲就直接建。
    orm_slot &slot = slot_ref(0);
    std::unique_lock<std::mutex> lock(slot.edit_mutex);
    while (!slot.mysql_edit_pool.empty())
    {
        auto temp = std::move(slot.mysql_edit_pool.front());
        slot.mysql_edit_pool.pop_front();
        temp->pooled_ = false;
        temp->issynch = true;
        if (!temp->is_closed())
        {
            lock.unlock();
            return temp;
        }
    }
    lock.unlock();
    return add_mysql_edit_connect(slot);
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_get_mysql_select_conn()
{
    auto ex                            = co_await asio::this_coro::executor;
    // any_io_executor 不支持 query(execution::context)，用通用的 context_as 查询取底层 context。
    const asio::execution_context &ctx =
        asio::query(ex, asio::execution::context_as_t<asio::execution_context &>());
    orm_slot &slot                     = slot_ref(slot_of(ctx));

    const unsigned int maxn = max_pool_of(LINK_SELECT);
    asio::steady_timer timer(ex);
    std::chrono::milliseconds backoff(1);
    // 本槽该角色名额已满后的最长等待；到点抛异常退出，请求不会一直占着协程。
    const auto wait_deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(CONST_ORM_CONN_WAIT_MS);

    for (;;)
    {
        {
            std::unique_lock<std::mutex> lock(slot.select_mutex);
            while (!slot.mysql_select_pool.empty())
            {
                auto temp = std::move(slot.mysql_select_pool.front());
                slot.mysql_select_pool.pop_front();
                temp->pooled_ = false;
                if (!temp->is_closed())
                {
                    lock.unlock();
                    co_return temp;
                }
            }

            if (slot.total_count[LINK_SELECT]->load(std::memory_order_relaxed) +
                    slot.connecting[LINK_SELECT].load(std::memory_order_relaxed) <
                maxn)
            {
                slot.connecting[LINK_SELECT].fetch_add(1, std::memory_order_relaxed);
                lock.unlock();

                try
                {
                    auto conn = co_await async_add_mysql_select_connect(slot);
                    slot.connecting[LINK_SELECT].fetch_sub(1, std::memory_order_relaxed);
                    co_return conn;
                }
                catch (...)
                {
                    slot.connecting[LINK_SELECT].fetch_sub(1, std::memory_order_relaxed);
                    throw;
                }
            }
        }

        auto now       = std::chrono::steady_clock::now();
        if (now >= wait_deadline)
        {
            // 到点仍没等到名额：抛异常退出，由 ORM 的 catch 写成 error_msg。
            // 退避重试不保证先到先得，等满上限就放弃，不让请求无限占着协程。
            throw std::runtime_error(std::string(" orm conn pool wait timeout for tag ") +
                                     conf_data[1].tag + " role=select" +
                                     " max_pool=" + std::to_string(maxn) +
                                     " total=" + std::to_string(slot.total_count[LINK_SELECT]->load(std::memory_order_relaxed)) +
                                     " connecting=" + std::to_string(slot.connecting[LINK_SELECT].load(std::memory_order_relaxed)) +
                                     " wait_ms=" + std::to_string(CONST_ORM_CONN_WAIT_MS));
        }
        auto remain   = std::chrono::duration_cast<std::chrono::milliseconds>(wait_deadline - now);
        auto wait_for = backoff < remain ? backoff : remain;
        timer.expires_after(wait_for);
        co_await timer.async_wait(asio::use_awaitable);
        if (backoff < std::chrono::milliseconds(50))
        {
            backoff *= 2;
        }
    }
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::get_mysql_select_conn()
{
    orm_slot &slot = slot_ref(0);
    std::unique_lock<std::mutex> lock(slot.select_mutex);
    while (!slot.mysql_select_pool.empty())
    {
        auto temp = std::move(slot.mysql_select_pool.front());
        slot.mysql_select_pool.pop_front();
        temp->pooled_ = false;
        temp->issynch = true;
        if (!temp->is_closed())
        {
            lock.unlock();
            return temp;
        }
    }
    lock.unlock();
    return add_mysql_select_connect(slot);
}

// ======================== PostgreSQL connections ========================

asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_add_pg_edit_connect(orm_slot &slot)
{
    auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::POSTGRESQL);
    auto conn = std::make_shared<pg_conn_base>(std::move(link));
    bool isok = co_await conn->async_connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        conn->conn_link->bind_slot_total(slot.total_count[LINK_EDIT]);
        co_return conn;
    }
    throw std::runtime_error(" add_pg_edit_connect failed for tag ");
}

asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_add_pg_edit_connect()
{
    co_return co_await async_add_pg_edit_connect(slot_ref(0));
}

std::shared_ptr<pg_conn_base> orm_conn_pool::add_pg_edit_connect(orm_slot &slot)
{
    auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::POSTGRESQL);
    auto conn = std::make_shared<pg_conn_base>(std::move(link));
    bool isok = conn->connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        conn->conn_link->bind_slot_total(slot.total_count[LINK_EDIT]);
        return conn;
    }
    throw std::runtime_error(" add_pg_edit_connect failed for tag ");
}

std::shared_ptr<pg_conn_base> orm_conn_pool::add_pg_edit_connect()
{
    return add_pg_edit_connect(slot_ref(0));
}

void orm_conn_pool::back_pg_edit_conn(std::shared_ptr<pg_conn_base> conn)
{
    if (!conn)
    {
        return;
    }
    orm_slot &slot = slot_ref(slot_of(conn->conn_link ? conn->conn_link->io_ctx : nullptr));
    if (conn->isclose)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(slot.edit_mutex);
    if (conn->pooled_)
    {
        std::cerr << "[orm_pool] duplicate return ignored: pg_edit" << std::endl;
    }
    else
    {
        conn->pooled_ = true;
        conn->issynch = false;
        slot.pg_edit_pool.emplace_back(std::move(conn));
    }
}

unsigned int orm_conn_pool::init_pg_edit_conn(unsigned char n, unsigned int slot_index)
{
    orm_slot &slot = slot_ref(slot_index);
    unsigned int i = 0;
    for (; i < n; ++i)
    {
        try
        {
            auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::POSTGRESQL);
            auto conn = std::make_shared<pg_conn_base>(std::move(link));
            bool isok = conn->connect(conf_data[0]);
            if (isok)
            {
                if (conf_data[0].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(slot.edit_mutex);
                slot.pg_edit_pool.emplace_back(conn);
                conn->conn_link->bind_slot_total(slot.total_count[LINK_EDIT]);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw std::runtime_error(error_msg);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}

std::shared_ptr<pg_conn_base> orm_conn_pool::add_pg_select_connect(orm_slot &slot)
{
    auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::POSTGRESQL);
    auto conn = std::make_shared<pg_conn_base>(std::move(link));
    bool isok = conn->connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        conn->conn_link->bind_slot_total(slot.total_count[LINK_SELECT]);
        return conn;
    }
    throw std::runtime_error(" add_pg_select_connect failed for tag ");
}

std::shared_ptr<pg_conn_base> orm_conn_pool::add_pg_select_connect()
{
    return add_pg_select_connect(slot_ref(0));
}

asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_add_pg_select_connect(orm_slot &slot)
{
    auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::POSTGRESQL);
    auto conn = std::make_shared<pg_conn_base>(std::move(link));
    bool isok = co_await conn->async_connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        conn->conn_link->bind_slot_total(slot.total_count[LINK_SELECT]);
        co_return conn;
    }
    throw std::runtime_error(" add_pg_select_connect failed for tag ");
}

asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_add_pg_select_connect()
{
    co_return co_await async_add_pg_select_connect(slot_ref(0));
}

unsigned int orm_conn_pool::init_pg_select_conn(unsigned char n, unsigned int slot_index)
{
    orm_slot &slot = slot_ref(slot_index);
    unsigned int i = 0;
    for (; i < n; ++i)
    {
        try
        {
            auto link = orm_conn_link_t::create(*slot.io_context, DB_TYPE::POSTGRESQL);
            auto conn = std::make_shared<pg_conn_base>(std::move(link));
            bool isok = conn->connect(conf_data[1]);
            if (isok)
            {
                if (conf_data[1].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(slot.select_mutex);
                slot.pg_select_pool.emplace_back(conn);
                conn->conn_link->bind_slot_total(slot.total_count[LINK_SELECT]);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw std::runtime_error(error_msg);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}

void orm_conn_pool::back_pg_select_conn(std::shared_ptr<pg_conn_base> conn)
{
    if (!conn)
    {
        return;
    }
    orm_slot &slot = slot_ref(slot_of(conn->conn_link ? conn->conn_link->io_ctx : nullptr));
    if (conn->isclose)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(slot.select_mutex);
    if (conn->pooled_)
    {
        std::cerr << "[orm_pool] duplicate return ignored: pg_select" << std::endl;
    }
    else
    {
        conn->pooled_ = true;
        conn->issynch = false;
        slot.pg_select_pool.emplace_back(std::move(conn));
    }
}

asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_get_pg_edit_conn()
{
    auto ex                            = co_await asio::this_coro::executor;
    // any_io_executor 不支持 query(execution::context)，用通用的 context_as 查询取底层 context。
    const asio::execution_context &ctx =
        asio::query(ex, asio::execution::context_as_t<asio::execution_context &>());
    orm_slot &slot                     = slot_ref(slot_of(ctx));

    const unsigned int maxn = max_pool_of(LINK_EDIT);
    asio::steady_timer timer(ex);
    std::chrono::milliseconds backoff(1);
    // 本槽该角色名额已满后的最长等待；到点抛异常退出，请求不会一直占着协程。
    const auto wait_deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(CONST_ORM_CONN_WAIT_MS);

    for (;;)
    {
        {
            std::unique_lock<std::mutex> lock(slot.edit_mutex);
            while (!slot.pg_edit_pool.empty())
            {
                auto temp = std::move(slot.pg_edit_pool.front());
                slot.pg_edit_pool.pop_front();
                temp->pooled_ = false;
                if (!temp->is_closed())
                {
                    lock.unlock();
                    co_return temp;
                }
            }

            if (slot.total_count[LINK_EDIT]->load(std::memory_order_relaxed) +
                    slot.connecting[LINK_EDIT].load(std::memory_order_relaxed) <
                maxn)
            {
                slot.connecting[LINK_EDIT].fetch_add(1, std::memory_order_relaxed);
                lock.unlock();

                try
                {
                    auto conn = co_await async_add_pg_edit_connect(slot);
                    slot.connecting[LINK_EDIT].fetch_sub(1, std::memory_order_relaxed);
                    co_return conn;
                }
                catch (...)
                {
                    slot.connecting[LINK_EDIT].fetch_sub(1, std::memory_order_relaxed);
                    throw;
                }
            }
        }

        auto now       = std::chrono::steady_clock::now();
        if (now >= wait_deadline)
        {
            // 到点仍没等到名额：抛异常退出，由 ORM 的 catch 写成 error_msg。
            // 退避重试不保证先到先得，等满上限就放弃，不让请求无限占着协程。
            throw std::runtime_error(std::string(" orm conn pool wait timeout for tag ") +
                                     conf_data[0].tag + " role=edit" +
                                     " max_pool=" + std::to_string(maxn) +
                                     " total=" + std::to_string(slot.total_count[LINK_EDIT]->load(std::memory_order_relaxed)) +
                                     " connecting=" + std::to_string(slot.connecting[LINK_EDIT].load(std::memory_order_relaxed)) +
                                     " wait_ms=" + std::to_string(CONST_ORM_CONN_WAIT_MS));
        }
        auto remain   = std::chrono::duration_cast<std::chrono::milliseconds>(wait_deadline - now);
        auto wait_for = backoff < remain ? backoff : remain;
        timer.expires_after(wait_for);
        co_await timer.async_wait(asio::use_awaitable);
        if (backoff < std::chrono::milliseconds(50))
        {
            backoff *= 2;
        }
    }
}

std::shared_ptr<pg_conn_base> orm_conn_pool::get_pg_edit_conn()
{
    orm_slot &slot = slot_ref(0);
    std::unique_lock<std::mutex> lock(slot.edit_mutex);
    while (!slot.pg_edit_pool.empty())
    {
        auto temp = std::move(slot.pg_edit_pool.front());
        slot.pg_edit_pool.pop_front();
        temp->pooled_ = false;
        temp->issynch = true;
        if (!temp->is_closed())
        {
            lock.unlock();
            return temp;
        }
    }
    lock.unlock();
    return add_pg_edit_connect(slot);
}

asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_get_pg_select_conn()
{
    auto ex                            = co_await asio::this_coro::executor;
    // any_io_executor 不支持 query(execution::context)，用通用的 context_as 查询取底层 context。
    const asio::execution_context &ctx =
        asio::query(ex, asio::execution::context_as_t<asio::execution_context &>());
    orm_slot &slot                     = slot_ref(slot_of(ctx));

    const unsigned int maxn = max_pool_of(LINK_SELECT);
    asio::steady_timer timer(ex);
    std::chrono::milliseconds backoff(1);
    // 本槽该角色名额已满后的最长等待；到点抛异常退出，请求不会一直占着协程。
    const auto wait_deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(CONST_ORM_CONN_WAIT_MS);

    for (;;)
    {
        {
            std::unique_lock<std::mutex> lock(slot.select_mutex);
            while (!slot.pg_select_pool.empty())
            {
                auto temp = std::move(slot.pg_select_pool.front());
                slot.pg_select_pool.pop_front();
                temp->pooled_ = false;
                if (!temp->is_closed())
                {
                    lock.unlock();
                    co_return temp;
                }
            }

            if (slot.total_count[LINK_SELECT]->load(std::memory_order_relaxed) +
                    slot.connecting[LINK_SELECT].load(std::memory_order_relaxed) <
                maxn)
            {
                slot.connecting[LINK_SELECT].fetch_add(1, std::memory_order_relaxed);
                lock.unlock();

                try
                {
                    auto conn = co_await async_add_pg_select_connect(slot);
                    slot.connecting[LINK_SELECT].fetch_sub(1, std::memory_order_relaxed);
                    co_return conn;
                }
                catch (...)
                {
                    slot.connecting[LINK_SELECT].fetch_sub(1, std::memory_order_relaxed);
                    throw;
                }
            }
        }

        auto now       = std::chrono::steady_clock::now();
        if (now >= wait_deadline)
        {
            // 到点仍没等到名额：抛异常退出，由 ORM 的 catch 写成 error_msg。
            // 退避重试不保证先到先得，等满上限就放弃，不让请求无限占着协程。
            throw std::runtime_error(std::string(" orm conn pool wait timeout for tag ") +
                                     conf_data[1].tag + " role=select" +
                                     " max_pool=" + std::to_string(maxn) +
                                     " total=" + std::to_string(slot.total_count[LINK_SELECT]->load(std::memory_order_relaxed)) +
                                     " connecting=" + std::to_string(slot.connecting[LINK_SELECT].load(std::memory_order_relaxed)) +
                                     " wait_ms=" + std::to_string(CONST_ORM_CONN_WAIT_MS));
        }
        auto remain   = std::chrono::duration_cast<std::chrono::milliseconds>(wait_deadline - now);
        auto wait_for = backoff < remain ? backoff : remain;
        timer.expires_after(wait_for);
        co_await timer.async_wait(asio::use_awaitable);
        if (backoff < std::chrono::milliseconds(50))
        {
            backoff *= 2;
        }
    }
}

std::shared_ptr<pg_conn_base> orm_conn_pool::get_pg_select_conn()
{
    orm_slot &slot = slot_ref(0);
    std::unique_lock<std::mutex> lock(slot.select_mutex);
    while (!slot.pg_select_pool.empty())
    {
        auto temp = std::move(slot.pg_select_pool.front());
        slot.pg_select_pool.pop_front();
        temp->pooled_ = false;
        temp->issynch = true;
        if (!temp->is_closed())
        {
            lock.unlock();
            return temp;
        }
    }
    lock.unlock();
    return add_pg_select_connect(slot);
}

// ======================== SQLite connections ========================
// SQLite 为本地文件库, 单连接模式: 同库任务已由框架级常驻 worker 线程串行,
// 每个角色 (edit/select) 保持一条常驻连接共享使用, 无需借出/归还式连接池。
// 公开接口签名与 MySQL/PG 保持一致, 调用方无感知。分槽后固定只用槽 0。

asio::awaitable<std::shared_ptr<sqlite_conn_base>> orm_conn_pool::async_add_sqlite_edit_connect()
{
    orm_slot &slot = slot_ref(0);
    auto conn      = std::make_shared<sqlite_conn_base>();
    conn->start_worker_thread(*slot.io_context, sqlite_db_path_of(conf_data[0]));
    bool isok = co_await conn->async_connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw std::runtime_error(" add_sqlite_edit_connect failed for tag ");
}

std::shared_ptr<sqlite_conn_base> orm_conn_pool::add_sqlite_edit_connect()
{
    orm_slot &slot = slot_ref(0);
    auto conn      = std::make_shared<sqlite_conn_base>();
    conn->start_worker_thread(*slot.io_context, sqlite_db_path_of(conf_data[0]));
    bool isok = conn->connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        return conn;
    }
    throw std::runtime_error(" add_sqlite_edit_connect failed for tag ");
}

void orm_conn_pool::back_sqlite_edit_conn(std::shared_ptr<sqlite_conn_base> conn)
{
    // 单连接模式: 连接常驻共享, 无需归还, 仅保留用量统计 (与 MySQL/PG 接口一致)
    if (conn)
    {
        conn->time_start = time((time_t *)NULL);
        conn->query_num++;
    }
}

unsigned int orm_conn_pool::init_sqlite_edit_conn(unsigned char n, unsigned int slot_index)
{
    (void)n;
    if (slot_index != 0)
    {
        return 0;
    }
    // 单连接模式: 忽略 n, 只建一条常驻连接 (同库任务已在 worker 线程串行)
    orm_slot &slot = slot_ref(0);
    {
        std::unique_lock<std::mutex> lock(slot.edit_mutex);
        if (slot.sqlite_edit_conn && !slot.sqlite_edit_conn->is_closed())
        {
            return 1;
        }
    }
    try
    {
        auto conn = add_sqlite_edit_connect();
        std::unique_lock<std::mutex> lock(slot.edit_mutex);
        slot.sqlite_edit_conn = conn;
        return 1;
    }
    catch (const std::exception &e)
    {
        error_msg.append(e.what());
        return 0;
    }
}

std::shared_ptr<sqlite_conn_base> orm_conn_pool::add_sqlite_select_connect()
{
    orm_slot &slot = slot_ref(0);
    auto conn      = std::make_shared<sqlite_conn_base>();
    conn->start_worker_thread(*slot.io_context, sqlite_db_path_of(conf_data[1]));
    bool isok = conn->connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        return conn;
    }
    throw std::runtime_error(" add_sqlite_select_connect failed for tag ");
}

asio::awaitable<std::shared_ptr<sqlite_conn_base>> orm_conn_pool::async_add_sqlite_select_connect()
{
    orm_slot &slot = slot_ref(0);
    auto conn      = std::make_shared<sqlite_conn_base>();
    conn->start_worker_thread(*slot.io_context, sqlite_db_path_of(conf_data[1]));
    bool isok = co_await conn->async_connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw std::runtime_error(" add_sqlite_select_connect failed for tag ");
}

void orm_conn_pool::back_sqlite_select_conn(std::shared_ptr<sqlite_conn_base> conn)
{
    // 单连接模式: 连接常驻共享, 无需归还, 仅保留用量统计 (与 MySQL/PG 接口一致)
    if (conn)
    {
        conn->time_start = time((time_t *)NULL);
        conn->query_num++;
    }
}

unsigned int orm_conn_pool::init_sqlite_select_conn(unsigned char n, unsigned int slot_index)
{
    (void)n;
    if (slot_index != 0)
    {
        return 0;
    }
    // 单连接模式: 忽略 n, 只建一条常驻连接 (同库任务已在 worker 线程串行)
    orm_slot &slot = slot_ref(0);
    {
        std::unique_lock<std::mutex> lock(slot.select_mutex);
        if (slot.sqlite_select_conn && !slot.sqlite_select_conn->is_closed())
        {
            return 1;
        }
    }
    try
    {
        auto conn = add_sqlite_select_connect();
        std::unique_lock<std::mutex> lock(slot.select_mutex);
        slot.sqlite_select_conn = conn;
        return 1;
    }
    catch (const std::exception &e)
    {
        error_msg.append(e.what());
        return 0;
    }
}

asio::awaitable<std::shared_ptr<sqlite_conn_base>> orm_conn_pool::async_get_sqlite_edit_conn()
{
    // 固定槽 0：单连接模式, 直接共享常驻连接; 断开时重建 (并发重建时复用先到的)
    orm_slot &slot = slot_ref(0);
    std::unique_lock<std::mutex> lock(slot.edit_mutex);
    if (slot.sqlite_edit_conn && !slot.sqlite_edit_conn->is_closed())
    {
        co_return slot.sqlite_edit_conn;
    }
    lock.unlock();
    auto conn = co_await async_add_sqlite_edit_connect();
    lock.lock();
    if (!slot.sqlite_edit_conn || slot.sqlite_edit_conn->is_closed())
    {
        slot.sqlite_edit_conn = conn;
    }
    else
    {
        conn = slot.sqlite_edit_conn;
    }
    co_return conn;
}

std::shared_ptr<sqlite_conn_base> orm_conn_pool::get_sqlite_edit_conn()
{
    // 固定槽 0：单连接模式, 直接共享常驻连接; 断开时重建 (并发重建时复用先到的)
    orm_slot &slot = slot_ref(0);
    std::unique_lock<std::mutex> lock(slot.edit_mutex);
    if (slot.sqlite_edit_conn && !slot.sqlite_edit_conn->is_closed())
    {
        return slot.sqlite_edit_conn;
    }
    lock.unlock();
    auto conn = add_sqlite_edit_connect();
    lock.lock();
    if (!slot.sqlite_edit_conn || slot.sqlite_edit_conn->is_closed())
    {
        slot.sqlite_edit_conn = conn;
    }
    else
    {
        conn = slot.sqlite_edit_conn;
    }
    return conn;
}

asio::awaitable<std::shared_ptr<sqlite_conn_base>> orm_conn_pool::async_get_sqlite_select_conn()
{
    // 固定槽 0：单连接模式, 直接共享常驻连接; 断开时重建 (并发重建时复用先到的)
    orm_slot &slot = slot_ref(0);
    std::unique_lock<std::mutex> lock(slot.select_mutex);
    if (slot.sqlite_select_conn && !slot.sqlite_select_conn->is_closed())
    {
        co_return slot.sqlite_select_conn;
    }
    lock.unlock();
    auto conn = co_await async_add_sqlite_select_connect();
    lock.lock();
    if (!slot.sqlite_select_conn || slot.sqlite_select_conn->is_closed())
    {
        slot.sqlite_select_conn = conn;
    }
    else
    {
        conn = slot.sqlite_select_conn;
    }
    co_return conn;
}

std::shared_ptr<sqlite_conn_base> orm_conn_pool::get_sqlite_select_conn()
{
    orm_slot &slot = slot_ref(0);
    std::unique_lock<std::mutex> lock(slot.select_mutex);
    if (slot.sqlite_select_conn && !slot.sqlite_select_conn->is_closed())
    {
        return slot.sqlite_select_conn;
    }
    lock.unlock();
    auto conn = add_sqlite_select_connect();
    lock.lock();
    if (!slot.sqlite_select_conn || slot.sqlite_select_conn->is_closed())
    {
        slot.sqlite_select_conn = conn;
    }
    else
    {
        conn = slot.sqlite_select_conn;
    }
    return conn;
}

// ======================== 逐槽清理 ========================

asio::awaitable<bool> orm_conn_pool::clear_select_conn_2hour()
{
    // 注意：这里的 2 小时判的是**连接年龄**（time_start 只在构造时写一次，归还时不刷新），
    // 不是空闲时长——一直在用的连接到期也会被换掉一条，属意的轮转。
    // 每拍每槽最多弹一条，另一条并列判据是 query_num 用量上限。
    time_t nowtimeid = time((time_t *)NULL);
    if (nowtimeid > CONST_ORM_CLEAR_TIME)
    {
        nowtimeid = nowtimeid - CONST_ORM_CLEAR_TIME;
    }

    bool cleared = false;
    for (auto &item : slots)
    {
        orm_slot &slot = *item;
        if (!slot.io_context)
        {
            continue;
        }
        // close 与其余续体都留在该槽自己的 ioc 上
        auto clear_one = [&slot, nowtimeid]() -> asio::awaitable<bool>
        {
            bool one_cleared = false;
            {
                std::unique_lock<std::mutex> lock(slot.select_mutex);
                if (!slot.mysql_select_pool.empty())
                {
                    auto temp = std::move(slot.mysql_select_pool.front());
                    slot.mysql_select_pool.pop_front();
                    temp->pooled_ = false;

                    if (temp->time_start < nowtimeid || temp->query_num > CONST_ORM_CLEAR_NUMBER)
                    {
                        lock.unlock();
                        co_await temp->async_close();
                        one_cleared = true;
                    }
                    else
                    {
                        temp->pooled_ = true;
                        slot.mysql_select_pool.emplace_back(temp);
                    }
                }
            }
            {
                std::unique_lock<std::mutex> lock(slot.select_mutex);
                if (!slot.pg_select_pool.empty())
                {
                    auto temp = std::move(slot.pg_select_pool.front());
                    slot.pg_select_pool.pop_front();
                    temp->pooled_ = false;

                    if (temp->time_start < nowtimeid || temp->query_num > CONST_ORM_CLEAR_NUMBER)
                    {
                        lock.unlock();
                        co_await temp->async_close();
                        one_cleared = true;
                    }
                    else
                    {
                        temp->pooled_ = true;
                        slot.pg_select_pool.emplace_back(temp);
                    }
                }
            }
            // SQLite 单连接常驻共享, 不参与定时回收 (断开时由 get 自动重建)
            co_return one_cleared;
        };

        if (co_await asio::co_spawn(*slot.io_context, clear_one(), asio::use_awaitable))
        {
            cleared = true;
        }
    }

    co_return cleared;
}

asio::awaitable<bool> orm_conn_pool::clear_edit_conn_2hour()
{
    // 判据与上面 clear_select_conn_2hour 同：连接年龄，不是空闲时长
    time_t nowtimeid = time((time_t *)NULL);
    if (nowtimeid > CONST_ORM_CLEAR_TIME)
    {
        nowtimeid = nowtimeid - CONST_ORM_CLEAR_TIME;
    }

    bool cleared = false;
    for (auto &item : slots)
    {
        orm_slot &slot = *item;
        if (!slot.io_context)
        {
            continue;
        }
        auto clear_one = [&slot, nowtimeid]() -> asio::awaitable<bool>
        {
            bool one_cleared = false;
            {
                std::unique_lock<std::mutex> lock(slot.edit_mutex);
                if (!slot.mysql_edit_pool.empty())
                {
                    auto temp = std::move(slot.mysql_edit_pool.front());
                    slot.mysql_edit_pool.pop_front();
                    temp->pooled_ = false;

                    if (temp->time_start < nowtimeid || temp->query_num > CONST_ORM_CLEAR_NUMBER)
                    {
                        lock.unlock();
                        co_await temp->async_close();
                        one_cleared = true;
                    }
                    else
                    {
                        temp->pooled_ = true;
                        slot.mysql_edit_pool.emplace_back(temp);
                    }
                }
            }
            {
                std::unique_lock<std::mutex> lock(slot.edit_mutex);
                if (!slot.pg_edit_pool.empty())
                {
                    auto temp = std::move(slot.pg_edit_pool.front());
                    slot.pg_edit_pool.pop_front();
                    temp->pooled_ = false;

                    if (temp->time_start < nowtimeid || temp->query_num > CONST_ORM_CLEAR_NUMBER)
                    {
                        lock.unlock();
                        co_await temp->async_close();
                        one_cleared = true;
                    }
                    else
                    {
                        temp->pooled_ = true;
                        slot.pg_edit_pool.emplace_back(temp);
                    }
                }
            }
            // SQLite 单连接常驻共享, 不参与定时回收 (断开时由 get 自动重建)
            co_return one_cleared;
        };

        if (co_await asio::co_spawn(*slot.io_context, clear_one(), asio::use_awaitable))
        {
            cleared = true;
        }
    }

    co_return cleared;
}

unsigned int orm_conn_pool::clear_select_conn()
{
    unsigned int total = 0;
    for (auto &item : slots)
    {
        orm_slot &slot = *item;
        std::unique_lock<std::mutex> lock(slot.select_mutex);
        unsigned int closed = 0;
        for (; !slot.mysql_select_pool.empty();)
        {
            auto temp = std::move(slot.mysql_select_pool.front());
            slot.mysql_select_pool.pop_front();
            temp->close();
            ++closed;
        }
        for (; !slot.pg_select_pool.empty();)
        {
            auto temp = std::move(slot.pg_select_pool.front());
            slot.pg_select_pool.pop_front();
            temp->close();
            ++closed;
        }
        bool had_sqlite = false;
        if (slot.sqlite_select_conn)
        {
            slot.sqlite_select_conn->close();
            slot.sqlite_select_conn.reset();
            had_sqlite = true;
        }
        lock.unlock();

        // SQLite 单连接不占名额，只计入返回的条数
        total += closed + (had_sqlite ? 1u : 0u);
    }
    return total;
}

unsigned int orm_conn_pool::clear_edit_conn()
{
    unsigned int total = 0;
    for (auto &item : slots)
    {
        orm_slot &slot = *item;
        std::unique_lock<std::mutex> lock(slot.edit_mutex);
        unsigned int closed = 0;
        for (; !slot.mysql_edit_pool.empty();)
        {
            auto temp = std::move(slot.mysql_edit_pool.front());
            slot.mysql_edit_pool.pop_front();
            temp->close();
            ++closed;
        }
        for (; !slot.pg_edit_pool.empty();)
        {
            auto temp = std::move(slot.pg_edit_pool.front());
            slot.pg_edit_pool.pop_front();
            temp->close();
            ++closed;
        }
        bool had_sqlite = false;
        if (slot.sqlite_edit_conn)
        {
            slot.sqlite_edit_conn->close();
            slot.sqlite_edit_conn.reset();
            had_sqlite = true;
        }
        lock.unlock();

        total += closed + (had_sqlite ? 1u : 0u);
    }
    return total;
}
}// namespace orm
