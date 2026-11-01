// 本轮（写回闸 read_is_clean）探针：在第二十章探针基础上补 async_fetch_append()，
// 让 4 处文本 record 写回点全被实例化；其余调用面沿用原探针。
// Probe TU: instantiate one ORM dialect template straight from vendor/httpserver/include
// so the exec_* cache edits are syntax-checked before paozhu_cli orm regenerates.
#include "../../../orm/lite/include/fortune_base.h"
#include "sqliteorm.hpp"

namespace probe_lite
{
    class M : public orm::lite::mysql_orm<M, orm::lite::fortune_base>
    {
      public:
        M() : orm::lite::mysql_orm<M, orm::lite::fortune_base>("lite") {}
    };
}

using probe_lite::M;

unsigned int sync_paths()
{
    unsigned int n = 0;
    M a;
    a.where("id", 1).use_cache(60);
    n += a.exec_one();
    M b;
    b.where("id", 1).use_cache(60);
    n += b.exec_fetch();
    M c;
    c.where("id", 1).use_cache(60);
    n += c.exec_fetch_one();
    M d;
    d.where("id", 1).use_cache(0);
    n += d.exec_one();
    M e;
    e.where("id", 1).use_cache(60);
    n += e.fetch_one();
    M f;
    f.where("id", 1).use_cache(60);
    n += f.fetch();
    M g;
    g.use_cache(60);
    n += g.get_one(1);
    M h;
    h.where("id", 1).use_cache(60);
    h.fetch_append();
    M k;
    k.where("id", 1).use_cache(60);
    k.exec_one();
    n += k.remove_cache(k.last_exec_cache_hash) ? 1 : 0;
    k.exec_fetch();
    n += k.remove_cache(k.last_exec_cache_hash) ? 1 : 0;
    return n;
}

asio::awaitable<unsigned int> async_paths()
{
    unsigned int n = 0;
    M a;
    a.where("id", 1).use_cache(60);
    n += co_await a.async_exec_one();
    M b;
    b.where("id", 1).use_cache(60);
    n += co_await b.async_exec_fetch();
    M c;
    c.where("id", 1).use_cache(60);
    n += co_await c.async_exec_fetch_one();
    M e;
    e.where("id", 1).use_cache(60);
    n += co_await e.async_fetch_one();
    M f;
    f.where("id", 1).use_cache(60);
    n += co_await f.async_fetch();
    M g;
    g.use_cache(60);
    n += co_await g.async_get_one(1);
    M x;
    x.where("id", 1).use_cache(60);
    co_await x.async_fetch_append();
    co_return n;
}

int main() { return 0; }
