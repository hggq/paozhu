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
#include <thread>

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
            c.max_pool    = 4;
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
                    c.max_pool = clamp_stoi(v, 128);
                else if (k == "minpool")
                {
                    if(v.size()==0)
                    {
                        c.min_pool = 0;
                    }
                    else
                    {
                        c.min_pool = clamp_stoi(v, 1);
                    }
                }
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

// 进程级一套 ORM 执行域：所有 tag 共用，不属于任何单个池。
struct orm_io_host_t
{
    std::vector<std::shared_ptr<asio::io_context>> iocs;
    std::vector<asio::executor_work_guard<asio::io_context::executor_type>> guards;
    std::vector<std::thread> threads;
    std::shared_ptr<const std::unordered_map<const asio::execution_context *, unsigned int>> index;
    bool started = false;

    // 兜底：不调 stop() 的入口（dev / daemon）与启动失败路径都靠它收摊
    ~orm_io_host_t()
    {
        stop();
    }

    // 幂等：三个 vector 清空后再调是空操作，外部 ioc 模式（从未 started）调它也是空操作
    void stop()
    {
        guards.clear();
        for (auto &ioc : iocs)
        {
            ioc->stop();
        }
        for (auto &t : threads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
        threads.clear();
        iocs.clear();
        index.reset();
        started = false;
    }
};

orm_io_host_t &orm_io_host()
{
    static orm_io_host_t host;
    return host;
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

unsigned int orm_conn_pool::max_pool_of(unsigned int link_type) const
{
    unsigned int m = conf_data[link_type & 1u].max_pool;
    // 未配置（0）按 1
    return m == 0 ? 1u : m;
}

unsigned int orm_conn_pool::slot_max_of(unsigned int link_type, unsigned int slot_index) const
{
    const unsigned int budget = max_pool_of(link_type);
    const unsigned int n      = static_cast<unsigned int>(slots.size());
    // 单槽（单 ioc 兼容路径）：全局预算就是本槽配额，与改造前逐值相等。
    if (n <= 1)
    {
        return budget;
    }
    const unsigned int base = budget / n;
    const unsigned int rem  = budget % n;
    // 余数给下标小的槽，保证每次启动、每台机器的分配一致，便于对比读数。
    const unsigned int m = base + (slot_index < rem ? 1u : 0u);
    // budget < N 时按 1 保底，避免某些槽永久拿不到连接（init 会为此打告警）。
    return m == 0 ? 1u : m;
}

unsigned int orm_conn_pool::slot_min_of(unsigned int link_type, unsigned int slot_index) const
{
    const unsigned int role_min = conf_data[link_type & 1u].min_pool;
    // min_pool 为 0：不预建任何连接，全部按需懒建
    if (role_min == 0)
    {
        return 0;
    }
    const unsigned int n        = static_cast<unsigned int>(slots.size());
    // 单槽：预建量就是 min_pool 原值（不夹上限），与改造前逐值相等。
    if (n <= 1)
    {
        return role_min;
    }
    const unsigned int mbase = role_min / n;
    const unsigned int mrem  = role_min % n;
    const unsigned int m     = mbase + (slot_index < mrem ? 1u : 0u);
    // 预建不能超过本槽上限；每槽保底 1 条，冷槽首请求不用等建连
    const unsigned int capped = std::min(m, slot_max_of(link_type, slot_index));
    return capped == 0 ? 1u : capped;
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

    // 连接关完再撤执行域
    orm_io_host().stop();

    return "-- orm host stopped\n";
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
                slot->index             = static_cast<unsigned int>(conn->slots.size());
                conn->slots.emplace_back(std::move(slot));
            }
        }
        conn->ioc_index  = ioc_index;
        conn->io_context = conn->slots[0]->io_context;
        conn->conf_data[item.link_type] = item;

        // 预算切分自检：打印每槽实际生效的上限/预建。min_pool / max_pool 是全局预算，
        // 这里按槽数切分；N == 1 时应逐值等于 orm.conf 里的原值。
        // SQLite 是单连接模式（只认槽 0、忽略条数），不打印，避免误读。
        if (item.db_type != DB_TYPE::SQLITE)
        {
            std::string quota_log = "-- orm pool tag=" + item.tag +
                                    " role=" + (item.link_type == 0 ? "edit" : "select") +
                                    " io_contexts=" + std::to_string(conn->slots.size()) +
                                    " slot_max=";
            std::string min_log  = " slot_min=";
            unsigned int min_sum = 0;
            for (unsigned int si = 0; si < conn->slots.size(); ++si)
            {
                if (si > 0)
                {
                    quota_log += ",";
                    min_log += ",";
                }
                quota_log += std::to_string(conn->slot_max_of(item.link_type, si));
                min_log += std::to_string(conn->slot_min_of(item.link_type, si));
                min_sum += conn->slot_min_of(item.link_type, si);
            }
            error_log.append(quota_log + min_log + " prebuild=" + std::to_string(min_sum) + "\n");

            // 每槽保底 1 会让 Σslot_max 超过 max_pool：提示把预算配到不小于槽数。
            const unsigned int budget = item.max_pool == 0 ? 1u : item.max_pool;
            if (conn->slots.size() > 1 && budget < conn->slots.size())
            {
                error_log.append("[ORM-WARN] tag=" + item.tag +
                                 " role=" + (item.link_type == 0 ? "edit" : "select") +
                                 " max_pool=" + std::to_string(budget) +
                                 " < io_contexts=" + std::to_string(conn->slots.size()) +
                                 ", per-slot quota floored to 1\n");
            }
        }

        unsigned int n = 0;
        try
        {
            // min_pool 是全局预建预算，按槽切分后每槽预建各自的 slot_min，其余按需懒建。
            // 预建总量与原单池一致，只是从槽 0 摊到各槽。
            for (unsigned int si = 0; si < conn->slots.size(); ++si)
            {
                unsigned char prebuild = static_cast<unsigned char>(conn->slot_min_of(item.link_type, si));
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

std::string init_orm_conn_pool(const std::string &orm_config_file, unsigned int orm_ioc_num)
{
    // orm.conf 不存在或为空：不起 host（ioc/guard/thread），直接返回空日志
    // 进程里没有任何 ORM tag 时，host 线程白跑一圈没有意义
    std::vector<orm_conn_t> myconfig = get_orm_config_file(orm_config_file);
    if (myconfig.empty())
    {
        return "";
    }

    orm_io_host_t &host = orm_io_host();
    std::string host_log;
    if (!host.started)
    {
        unsigned int hw = std::thread::hardware_concurrency();
        if (hw == 0)
        {
            hw = 1;// 标准允许返回 0（拿不到核数）
        }
        unsigned int n = orm_ioc_num < 1 ? 1u : orm_ioc_num;
        if (n > hw)
        {
            n = hw;// 每个 ioc 至少一条线程，否则那槽的建连与查询 I/O 永不推进
        }

        for (unsigned int i = 0; i < n; ++i)
        {
            host.iocs.emplace_back(std::make_shared<asio::io_context>(0));
        }

        auto index = std::make_shared<std::unordered_map<const asio::execution_context *, unsigned int>>();
        for (unsigned int i = 0; i < n; ++i)
        {
            index->emplace(static_cast<const asio::execution_context *>(host.iocs[i].get()), i);
            host.guards.emplace_back(host.iocs[i]->get_executor());
        }
        host.index = std::move(index);

        // 线程总数固定 = 核数，按 ioc 数均分，余数给下标小的 ioc
        const unsigned int base = hw / n;
        const unsigned int rem  = hw % n;
        for (unsigned int i = 0; i < n; ++i)
        {
            const unsigned int per = base + (i < rem ? 1u : 0u);
            for (unsigned int j = 0; j < per; ++j)
            {
                host.threads.emplace_back(
                    [ioc = host.iocs[i]]()
                    {
                        do
                        {
                            try
                            {
                                ioc->run();
                            }
                            catch (...)
                            {
                            }
                        } while (!ioc->stopped());
                    });
            }
        }
        host.started = true;

        host_log = "-- orm host ioc=" + std::to_string(n) +
                   " threads=" + std::to_string(hw) + " per_ioc=";
        for (unsigned int i = 0; i < n; ++i)
        {
            if (i > 0)
            {
                host_log += ",";
            }
            host_log += std::to_string(base + (i < rem ? 1u : 0u));
        }
        host_log += "\n";
    }
    return host_log + init_orm_conn_pool(host.iocs, host.index, orm_config_file);
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
        conn->conn_link->slot_index = slot.index;
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
        conn->conn_link->slot_index = slot.index;
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
    // 归还按建连时记下的槽下标定位，不按归还时的 executor：连接可能跨槽借出。
    orm_slot &slot = slot_ref(conn->conn_link ? conn->conn_link->slot_index : 0);
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
                conn->conn_link->slot_index = slot.index;
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
        conn->conn_link->slot_index = slot.index;
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
        conn->conn_link->slot_index = slot.index;
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
                conn->conn_link->slot_index = slot.index;
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
    orm_slot &slot = slot_ref(conn->conn_link ? conn->conn_link->slot_index : 0);
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
    auto ex                       = co_await asio::this_coro::executor;// 门闩定时器仍挂调用方 ioc
    const unsigned int n          = slots.empty() ? 1u : static_cast<unsigned int>(slots.size());
    const unsigned int slot_index = rr_counter.fetch_add(1, std::memory_order_relaxed) % n;
    orm_slot &slot                = slot_ref(slot_index);

    const unsigned int maxn = slot_max_of(LINK_EDIT, slot_index);
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
    // 同步路径不等门闩：业务线程池没有 executor，阻塞等会拖死工作线程，无空闲就直接建。
    const unsigned int n = slots.empty() ? 1u : static_cast<unsigned int>(slots.size());
    orm_slot &slot       = slot_ref(rr_counter.fetch_add(1, std::memory_order_relaxed) % n);
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
    auto ex                       = co_await asio::this_coro::executor;// 门闩定时器仍挂调用方 ioc
    const unsigned int n          = slots.empty() ? 1u : static_cast<unsigned int>(slots.size());
    const unsigned int slot_index = rr_counter.fetch_add(1, std::memory_order_relaxed) % n;
    orm_slot &slot                = slot_ref(slot_index);

    const unsigned int maxn = slot_max_of(LINK_SELECT, slot_index);
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
    const unsigned int n = slots.empty() ? 1u : static_cast<unsigned int>(slots.size());
    orm_slot &slot       = slot_ref(rr_counter.fetch_add(1, std::memory_order_relaxed) % n);
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
        conn->conn_link->slot_index = slot.index;
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
        conn->conn_link->slot_index = slot.index;
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
    orm_slot &slot = slot_ref(conn->conn_link ? conn->conn_link->slot_index : 0);
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
                conn->conn_link->slot_index = slot.index;
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
        conn->conn_link->slot_index = slot.index;
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
        conn->conn_link->slot_index = slot.index;
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
                conn->conn_link->slot_index = slot.index;
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
    orm_slot &slot = slot_ref(conn->conn_link ? conn->conn_link->slot_index : 0);
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
    auto ex                       = co_await asio::this_coro::executor;// 门闩定时器仍挂调用方 ioc
    const unsigned int n          = slots.empty() ? 1u : static_cast<unsigned int>(slots.size());
    const unsigned int slot_index = rr_counter.fetch_add(1, std::memory_order_relaxed) % n;
    orm_slot &slot                = slot_ref(slot_index);

    const unsigned int maxn = slot_max_of(LINK_EDIT, slot_index);
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
    const unsigned int n = slots.empty() ? 1u : static_cast<unsigned int>(slots.size());
    orm_slot &slot       = slot_ref(rr_counter.fetch_add(1, std::memory_order_relaxed) % n);
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
    auto ex                       = co_await asio::this_coro::executor;// 门闩定时器仍挂调用方 ioc
    const unsigned int n          = slots.empty() ? 1u : static_cast<unsigned int>(slots.size());
    const unsigned int slot_index = rr_counter.fetch_add(1, std::memory_order_relaxed) % n;
    orm_slot &slot                = slot_ref(slot_index);

    const unsigned int maxn = slot_max_of(LINK_SELECT, slot_index);
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
    const unsigned int n = slots.empty() ? 1u : static_cast<unsigned int>(slots.size());
    orm_slot &slot       = slot_ref(rr_counter.fetch_add(1, std::memory_order_relaxed) % n);
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
