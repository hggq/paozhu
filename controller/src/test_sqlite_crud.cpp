#include "orm.h"
#include <chrono>
#include <string>
#include <thread>
#include "md5.h"
#include "func.h"
#include "httppeer.h"
#include "request.h"
#include "test_sqlite_crud.h"
namespace http
{

//@urlpath(null,cms/sqlist)
std::string article_sqlite_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // step3 content list
#ifdef ENABLE_SQLITE
    try
    {
        auto articles    = orm::lite::Fortune();
        int page         = client.get["page"].to_int();
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

        client << "<p>page list for sqlite</p>";
        client << "<p>" << articles.sqlstring << "</p>";
        client << "<p>size:" << std::to_string(articles.size()) << "</p>";
        if (articles.size() > 0)
        {
            for (auto &bb : articles)
            {
                client << "<p><a href=\"/cms/sqshow?id=" << std::to_string(bb.id) << "\">" << std::to_string(bb.id) << "</a></p>";
                //client.val["list"].push(std::move(item));
            }
        }
        //peer->view("cms/list");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
#else
    client << "<p>Please: cmake .. -DENABLE_SQLITE=ON</p>";
#endif
    return "";
}

//@urlpath(null,cms/sqshow)
std::string article_sqlite_show(std::shared_ptr<httppeer> peer)
{
    // step4
    httppeer &client = peer->get_peer();
#ifdef ENABLE_SQLITE
    auto articles = orm::lite::Fortune();
    int aid       = client.get["id"].to_int();

    articles.where("id", aid).fetch_one();

    client.val["title"]   = std::to_string(aid);
    client.val["content"] = articles.getMessage();
#else
    client << "<p>Please: cmake .. -DENABLE_SQLITE=ON</p>";
#endif
    peer->view("cms/show");
    return "";
}

}//namespace http