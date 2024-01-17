#include "orm.h"
#include "httppeer.h"
#include "testto_tree.h"

namespace http
{
//@urlpath(null,testtotree)
std::string testtotree(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    //client << "<p>test record to tree</p>";
    auto testa = orm::cms::Testa();
    testa.fetch();
    //record to tree data database table must has parentid  or parentid field annotation is [userid tree]
    auto treedata = testa.to_tree();

    //client << testa.tree_json(treedata);
    client << testa.tree_tojson(treedata, "id,parentid,content");
    client.json_type();
    return "";
}

}// namespace http