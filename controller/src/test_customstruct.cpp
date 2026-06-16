#include "orm.h" 
#include "httppeer.h"
#include "test_customstruct.h"
#include "world_base.h"
#include "func.h"
#include "unicode.h"
#include <memory>
#include <string>

namespace orm::cust
{
    struct LocalStruct : orm::Base<LocalStruct> 
    {
        orm::world_info::type::id id;
        orm::world_info::type::randomnumber randomnumber;
        std::string message;
        ORM_NAMES(id, randomnumber, message);
    };

}

namespace http
{

//@urlpath(null,test_customstruct)
asio::awaitable<std::string> test_customstruct(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    orm::cust::LocalStruct cust_record;

    char id_buf[] = "10086";
    cust_record.set_val("id",(const unsigned char *)id_buf, 5,0);

    char num_buf[] = "-42";
    cust_record.set_val("randomnumber", (const unsigned char *)num_buf, 3,0);
 

    char msg_buf[] = "hello \"orm\"\nnewline";
    cust_record.set_val("message", (const unsigned char *)msg_buf, sizeof(msg_buf) - 1, 0);

    client << "<hr>=== JSON Output ===<br>"
              << cust_record.to_json() << "<br>";
    std::string json_content = R"({"id":333333,"randomnumber":-44444,"message":"testhello \"orm\"\nnewline"})";
    client <<json_content ;

    cust_record.from_json(json_content);
    client << "<hr>=== JSON Output ===<br>"
              << cust_record.to_json() << "<br>";


 
    auto world = orm::World();
    world.eqId(2);
    world.select("id,randomnumber");
    world.leftJoin<orm::Fortune>().joinOn("id", "id").joinSelect("message");
    
    unsigned int n= co_await world.async_fetch_one_to(cust_record);

    client << world.sqlstring;
    if(n > 0)
    {
        client << "<hr>=== JSON Output from db ===<br>"
              << cust_record.to_json() << "<br>";
    }

    co_return "";
}

}// namespace http