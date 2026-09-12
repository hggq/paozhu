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

}// namespace http
