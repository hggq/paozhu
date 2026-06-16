#include "orm.h" 
#include "httppeer.h"
#include "test_leftjoin.h"
#include "func.h"
#include "unicode.h"
#include <memory>
#include <string>

namespace orm::cust
{
    struct LocalArtStruct : orm::Base<LocalArtStruct> 
    {
        unsigned int topicid;
        unsigned int aid;
        std::string topictitle;
        std::string arttitle;
        ORM_NAMES(topicid, aid, topictitle, arttitle);
    };

}

namespace http
{

//@urlpath(null,test_leftjoinlimit)
asio::awaitable<std::string> test_leftjoinlimit(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::vector<orm::cust::LocalArtStruct> cust_record;
    auto worldtest = orm::cms::Topic();
    worldtest.eqUserid(1);
    worldtest.select("topicid,title AS topictitle");
    worldtest.leftJoin<orm::cms::Article>().joinOn("topicid", "topicid").joinSelect("topicid, aid, title AS arttitle");
    worldtest.joinDesc("aid").joinLimit(2).joinWhere("userid",orm::wq::eq,1).joinGroup("topicid");
    //where userid提前限定，左表，joinGroup 分组id joinLimit 每组抽多少个
    
    unsigned int n= co_await worldtest.async_fetch_to(cust_record);

    client << worldtest.sqlstring;
    if(n > 0)
    {
        client << "<hr>=== JSON Output from db ===<br>";
        if(cust_record.size()>0)
        {
            for(unsigned i= 0;i< cust_record.size(); i++)
            {
               client << cust_record[i].to_json() << "<br>";
            }
        }
              
    }

    co_return "";
}

//@urlpath(null,test_leftjoinfull)
asio::awaitable<std::string> test_leftjoinfull(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    std::vector<orm::cust::LocalArtStruct> cust_record;

 
    auto worldtest = orm::cms::Topic();
    worldtest.eqUserid(1);
    worldtest.select("topicid,title AS topictitle");
    worldtest.leftJoin<orm::cms::Article>().joinOn("topicid", "topicid").joinSelect("aid, title AS arttitle");
    
    unsigned int n= co_await worldtest.async_fetch_to(cust_record);

    client << worldtest.sqlstring;
    if(n > 0)
    {
        client << "<hr>=== JSON Output from db ===<br>";

        if(cust_record.size()>0)
        {
            for(unsigned i= 0;i< cust_record.size(); i++)
            {
               client << cust_record[i].to_json() << "<br>";
            }
        }
              
    }

    co_return "";
}

}// namespace http