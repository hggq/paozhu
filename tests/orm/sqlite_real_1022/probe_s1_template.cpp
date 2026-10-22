// 落码检查: 把改过的 12 处模板调用点(文本 8 + 预编译 4)全部实例化一遍,
// 确认元组接口在同步与协程两条路上都能编译。生成器跑之前用它自证。
#include "fortune_base.h"
#include "sqliteorm.hpp"

namespace probe_s1
{
    class M : public orm::lite::mysql_orm<M, orm::lite::fortune_base>
    {
      public:
        M() : orm::lite::mysql_orm<M, orm::lite::fortune_base>("lite") {}
    };
}

using probe_s1::M;

unsigned int sync_paths()
{
    unsigned int n = 0;
    M m;
    n += std::get<0>(m.insert());
    n += std::get<0>(m.insert(m.data));
    n += std::get<0>(m.insert(m.record));
    n += std::get<0>(m.save(true));
    n += std::get<0>(m.exec_insert());
    n += std::get<0>(m.exec_insert_batch());
    return n;
}

asio::awaitable<unsigned int> async_paths()
{
    unsigned int n = 0;
    M m;
    n += std::get<0>(co_await m.async_insert());
    n += std::get<0>(co_await m.async_insert(m.data));
    n += std::get<0>(co_await m.async_insert(m.record));
    n += std::get<0>(co_await m.async_save(true));
    n += std::get<0>(co_await m.async_exec_insert());
    n += std::get<0>(co_await m.async_exec_insert_batch());
    co_return n;
}

int main()
{
    return 0;
}
