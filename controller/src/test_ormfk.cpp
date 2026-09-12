#include "orm.h"
#include "httppeer.h"
#include "func.h"
#include "test_ormfk.h"
#include <memory>
#include <string>
#include <vector>

namespace http
{

//@urlpath(null,test_ormfk_one)
std::string test_ormfk_one(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto child = orm::FkChild();
    child.where(orm::fk_child_info::cols::id, 1);
    child.fetch_one();
    client << "<p>child id:" << child.data.id << " parent_id:" << child.data.parent_id << "</p>";

    // outgoing: fk_child.parent_id -> fk_parent.id, scoped by current row data
    auto parent = child.oneFkParent();
    parent.fetch_one();
    client << "<p>oneFkParent sql:" << parent.sqlstring << "</p>";
    client << "<p>oneFkParent name:" << parent.data.name << "</p>";

    // explicit id overload
    auto parent2 = child.oneFkParent(2);
    parent2.fetch_one();
    client << "<p>oneFkParent(2) name:" << parent2.data.name << "</p>";

    // overload that decorates an existing model, so extra conditions still chain
    auto decorated = orm::FkParent();
    child.oneFkParent(decorated);
    decorated.select("id,name");
    decorated.fetch_one();
    client << "<p>oneFkParent(obj&) sql:" << decorated.sqlstring << "</p>";
    return "";
}

//@urlpath(null,test_ormfk_many)
std::string test_ormfk_many(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto parents = orm::FkParent();
    parents.fetch();
    client << "<p>parents:" << parents.record.size() << "</p>";

    // incoming: fk_child.parent_id -> fk_parent.id, scoped by every id in record
    auto children = parents.manyFkChild();
    children.fetch();
    client << "<p>manyFkChild sql:" << children.sqlstring << "</p>";
    client << "<p>manyFkChild rows:" << children.record.size() << "</p>";

    // group in memory to avoid the N+1 round trip
    std::vector<int> child_count;
    for (auto &p : parents.record)
    {
        int n = 0;
        for (auto &c : children.record)
        {
            if (c.parent_id == p.id)
            {
                n++;
            }
        }
        child_count.emplace_back(n);
    }
    for (size_t i = 0; i < parents.record.size(); i++)
    {
        client << "<p>parent " << parents.record[i].id << " name:" << parents.record[i].name
               << " children:" << child_count[i] << "</p>";
    }

    auto decorated = orm::FkChild();
    parents.manyFkChild(decorated);
    decorated.where(orm::fk_child_info::cols::extra, orm::wq::nq, "");
    decorated.fetch();
    client << "<p>manyFkChild(obj&) sql:" << decorated.sqlstring << "</p>";
    return "";
}

//@urlpath(null,test_ormfk_join)
std::string test_ormfk_join(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto world = orm::World();
    world.btId(2).ltId(6);
    world.select("id,randomnumber");
    world.leftJoin<orm::Fortune>().joinOn("id", "id").joinSelect("message");
    world.fetch();
    client << "<p>leftJoin sql:" << world.sqlstring << "</p>";

    auto topic = orm::cms::Topic();
    topic.eqUserid(1);
    topic.select("topicid,title AS topictitle");
    topic.innerJoin<orm::cms::Article>()
         .joinOn("topicid", orm::cms::topic_info::cols::topicid)
         .joinSelect("aid, title AS arttitle")
         .joinWhere("userid", orm::wq::eq, 1)
         .joinGroup("topicid")
         .joinDesc("aid")
         .joinLimit(2);
    topic.fetch();
    client << "<p>innerJoin top-2 per group sql:" << topic.sqlstring << "</p>";
    client << "<p>rows:" << topic.record.size() << " error:" << topic.error_msg << "</p>";
    return "";
}

//@urlpath(null,test_ormfk_co)
asio::awaitable<std::string> test_ormfk_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto parents = orm::FkParent();
    co_await parents.async_fetch();

    auto children = parents.manyFkChild();
    co_await children.async_fetch();

    client << "<p>async parents:" << parents.record.size()
           << " children:" << children.record.size() << "</p>";
    client << "<p>" << children.sqlstring << "</p>";
    co_return "";
}

}// namespace http
