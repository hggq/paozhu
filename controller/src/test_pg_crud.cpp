#include "orm.h"
#include <chrono>
#include <thread>
#include "md5.h"
#include "func.h"
#include "httppeer.h"
#include "request.h"
#include "test_pg_crud.h"
namespace http
{

 
//@urlpath(null,cms/pglist)
std::string article_pg_list(std::shared_ptr<httppeer> peer)
{
    // step3 content list
    try
    {
        httppeer &client = peer->get_peer();
        auto articles = orm::ph::Fortune();
        int page      = client.get["page"].to_int();
        if (page < 0)
        {
            page = 0;
        }
        articles.btId(0);
        auto [bar_min, bar_max, current_page, total_page] = articles.page(page, 10, 5);

        client.val["pageinfo"].set_object();
        client.val["pageinfo"]["min"]     = bar_min;
        client.val["pageinfo"]["max"]     = bar_max;
        client.val["pageinfo"]["current"] = current_page;
        client.val["pageinfo"]["total"]   = total_page;

        //自动分页 automatic pagination
        articles.fetch();
 
        client << "<p>page list</p>";
        client << "<p>" << articles.sqlstring << "</p>";
        client << "<p>size:" << std::to_string(articles.size()) << "</p>";
        if (articles.size() > 0)
        {
            for (auto &bb : articles)
            {
                client << "<p><a href=\"/cms/pgshow?id=" << bb.id << "\">" << bb.id << "</a></p>";
                //client.val["list"].push(std::move(item));
            }
        }
        //peer->view("cms/list");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return "";
}

//@urlpath(null,cms/pgshow)
std::string article_pg_show(std::shared_ptr<httppeer> peer)
{
    // step4
    httppeer &client = peer->get_peer();
    auto articles    = orm::ph::Fortune();
    int aid          = client.get["id"].to_int();

    articles.where("id", aid).fetch_one();

    client.val["title"]   = std::to_string(aid);
    client.val["content"] = articles.getMessage();

    peer->view("cms/show");
    return "";
}
 
}//namespace http