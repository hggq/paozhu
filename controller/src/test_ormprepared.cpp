#include "orm.h"
#include "httppeer.h"
#include "func.h"
#include "test_ormprepared.h"
#include <memory>
#include <string>
#include <vector>

namespace orm::cust
{
    struct WorldRowStruct : orm::Base<WorldRowStruct>
    {
        unsigned int id;
        int randomnumber;
        ORM_NAMES(id, randomnumber);
    };
}// namespace orm::cust

namespace http
{

//@urlpath(null,test_ormprepared_dsl)
std::string test_ormprepared_dsl(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto art = orm::cms::Article();
    art.beIsopen(1)
       .btAid(0)
       .andsub()
       .likeTitle("paozhu")
       .orlikeContent("paozhu")
       .endsub()
       .desc(orm::cms::article_info::cols::aid)
       .limit(5)
       .fetch();

    client << "<p>" << art.sqlstring << "</p>";
    client << "<p>rows:" << art.record.size() << " error:" << art.error_msg << "</p>";
    return "";
}

//@urlpath(null,test_ormprepared_exec)
std::string test_ormprepared_exec(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto world = orm::World();
    world.AND("id", orm::wq::be, 1)
         .AND("id", orm::wq::le, 200)
         .orsub()
         .AND("randomnumber", orm::wq::bt, 5000)
         .OR("randomnumber", orm::wq::lt, 200)
         .endsub();

    unsigned int total = world.exec_count();
    client << "<p>count:" << total << "</p>";

    world.select("id,randomnumber");
    std::vector<orm::cust::WorldRowStruct> rows;
    world.exec_fetch_to(rows);

    client << "<p>" << world.sqlstring << "</p>";
    client << "<p>fetched:" << rows.size() << " error:" << world.error_msg << "</p>";
    for (auto &row : rows)
    {
        client << "<p>" << row.to_json() << "</p>";
    }
    return "";
}

//@urlpath(null,test_ormprepared_write)
std::string test_ormprepared_write(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    std::vector<unsigned long long> inserted_ids;

    auto one = orm::cms::Testa();
    one.data.parentid = 7;
    one.data.content  = "prepared insert";
    auto [effect, newid] = one.exec_insert();
    if (effect > 0)
    {
        inserted_ids.emplace_back(newid);
    }
    client << "<p>exec_insert effect:" << effect << " id:" << newid << " pk:" << one.getPK() << "</p>";

    one.clear();
    one.data.content = "prepared update";
    one.where(orm::cms::testa_info::cols::id, newid);
    client << "<p>exec_update effect:" << one.exec_update("content") << "</p>";

    one.clear();
    one.where(orm::cms::testa_info::cols::id, newid);
    client << "<p>exec_update_col effect:" << one.exec_update_col("parentid", 3, '+') << "</p>";

    one.clear();
    one.where(orm::cms::testa_info::cols::id, newid);
    client << "<p>exec_replace_col effect:" << one.exec_replace_col("content", "prepared", "replaced") << "</p>";

    auto batch = orm::cms::Testa();
    for (unsigned int i = 0; i < 3; i++)
    {
        orm::cms::testa_info::meta row;
        row.parentid = 8;
        row.content  = "batch row";
        batch.record.push_back(std::move(row));
    }
    auto [batch_effect, first_id] = batch.exec_insert_batch();
    client << "<p>exec_insert_batch effect:" << batch_effect << " first_id:" << first_id << "</p>";

    unsigned int keep = 0;
    if (batch_effect > 0)
    {
        auto probe = orm::cms::Testa();
        probe.where(orm::cms::testa_info::cols::parentid, 8);
        probe.select("id");
        probe.exec_fetch();
        for (auto &row : probe.record)
        {
            inserted_ids.emplace_back(row.id);
        }
        keep = probe.record.size();
    }
    client << "<p>batch rows found:" << keep << "</p>";

    auto clean = orm::cms::Testa();
    clean.whereIn(orm::cms::testa_info::cols::id,
                  [&]
                  {
                      std::vector<long long> ids;
                      ids.reserve(inserted_ids.size());
                      for (auto id : inserted_ids)
                      {
                          ids.emplace_back(static_cast<long long>(id));
                      }
                      return ids;
                  }());
    unsigned int removed = clean.exec_remove();
    client << "<p>exec_remove effect:" << removed << " sql:" << clean.sqlstring << "</p>";
    return "";
}

//@urlpath(null,test_ormprepared_co)
asio::awaitable<std::string> test_ormprepared_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto world = orm::World();
    world.lock_conn();

    world.where(orm::world_info::cols::id, orm::wq::bt, 0);
    unsigned int total = co_await world.async_exec_count();

    std::vector<orm::cust::WorldRowStruct> rows;
    world.clearWhere();
    world.AND("id", orm::wq::be, 1).AND("id", orm::wq::le, 50);
    world.select("id,randomnumber");
    co_await world.async_exec_fetch_to(rows);

    auto one = orm::World();
    one.where(orm::world_info::cols::id, 1);
    co_await one.async_exec_one();

    world.unlock_conn();

    client << "<p>async_exec_count:" << total << "</p>";
    client << "<p>" << world.sqlstring << "</p>";
    client << "<p>rows:" << rows.size() << "</p>";
    client << "<p>one randomnumber:" << one.data.randomnumber << "</p>";
    co_return "";
}

//@urlpath(null,test_ormprepared_appendco)
asio::awaitable<std::string> test_ormprepared_appendco(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int n = client.get["n"].to_int();
    if (n == 0)
    {
        n = 10;
    }
    else if (n > 200)
    {
        n = 200;
    }

    try
    {
        // 1) 普通列名，第一次走冷路径，第二次同一句 sql 走语句缓存热路径；record 只增不清
        auto world = orm::World();
        world.AND("id", orm::wq::be, 1).AND("id", orm::wq::le, 500);
        world.select("id,randomnumber");
        world.limit(0, n);
        unsigned int cold = co_await world.async_exec_fetch_append();
        client << "<p>append cold rows:" << cold << " record:" << world.record.size() << " err:" << world.error_msg << "</p>";
        unsigned int warm = co_await world.async_exec_fetch_append();
        client << "<p>append warm rows:" << warm << " record:" << world.record.size() << " err:" << world.error_msg << "</p>";
        if (!world.record.empty())
        {
            client << "<p>append first row id:" << world.record[0].id << " randomnumber:" << world.record[0].randomnumber << "</p>";
        }
        client << "<p>" << world.sqlstring << "</p>";

        // 2) 别名查询：靠 conn->prepared_col_org_names() 物理列名兜底，id 落得进来说明回调里读到了连接
        auto alias = orm::World();
        alias.AND("id", orm::wq::be, 1).AND("id", orm::wq::le, 500);
        alias.select("id AS myid,randomnumber");
        alias.limit(0, n);
        unsigned int arows = co_await alias.async_exec_fetch_append();
        client << "<p>append alias rows:" << arows << " record:" << alias.record.size() << " err:" << alias.error_msg << "</p>";
        if (!alias.record.empty())
        {
            client << "<p>append alias first id:" << alias.record[0].id << " randomnumber:" << alias.record[0].randomnumber << "</p>";
        }
        unsigned int arows2 = co_await alias.async_exec_fetch_append();
        client << "<p>append alias warm rows:" << arows2 << " record:" << alias.record.size() << " err:" << alias.error_msg << "</p>";

        // 3) 同一个别名给 async_exec_one()：也有兜底，id 应非 0
        auto aliasone = orm::World();
        aliasone.where(orm::world_info::cols::id, 1);
        aliasone.select("id AS myid,randomnumber");
        co_await aliasone.async_exec_one();
        client << "<p>exec_one alias id:" << aliasone.data.id << " randomnumber:" << aliasone.data.randomnumber << "</p>";

        // 4) 对照：async_exec_fetch_one() 的回调没有兜底，同一条别名查询 id 应为 0
        auto oneark = orm::World();
        oneark.where(orm::world_info::cols::id, 1);
        oneark.select("id AS myid,randomnumber");
        co_await oneark.async_exec_fetch_one();
        client << "<p>fetch_one alias id:" << oneark.data.id << " randomnumber:" << oneark.data.randomnumber << "</p>";

        auto real = orm::World();
        real.where(orm::world_info::cols::id, 1);
        real.select("id,randomnumber");
        co_await real.async_exec_fetch_one();
        client << "<p>fetch_one real id:" << real.data.id << " randomnumber:" << real.data.randomnumber << "</p>";
    }
    catch (std::exception &e)
    {
        client << "<p>exception:" << e.what() << "</p>";
    }
    co_return "";
}

//@urlpath(null,stresshello)
asio::awaitable<std::string> stresshello(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto users = orm::cms::Sysuser();
    try
    {
        unsigned int n = co_await users.eqName("admin").async_exec_fetch_one();
        if (n > 0)
        {
            client << "<p>found:" << users.data.name << "</p>";
        }
        else
        {
            client << "<p>Not found</p>";
        }
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
    }
    co_return "";
}

}// namespace http
