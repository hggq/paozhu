#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testpinyin.h"
#include "hanyu_to_pinyin.h"
namespace http
{
//@urlpath(null,testpinyin)
std::string testpinyin(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
     
    hanyu_to_pinyin &a= get_hanyu_to_pinyin();
    std::string b="aa你好世界bb掠夺";
    client<<"<p>";
    client<<b;
    client<<"</p>";
    
    if(!a.is_loaded()) 
    {
        a.init_data();
    }
    client<<"<p>";
    client<<a.get_pinyin(b);
    client<<"</p>";
    return "";
}

//@urlpath(null,testpinyin_loaded)
std::string testpinyin_loaded(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
     
    hanyu_to_pinyin &a= get_hanyu_to_pinyin();
    std::string b="aa你好世界bb 已经加载";
    client<<"<p>";
    client<<b;
    client<<"</p>";
    
    if(a.is_loaded()) 
    {
        client<<"<p>";
        client<<a.get_pinyin(b);
        client<<"</p>";

        b="码";
        client<<"<p>";
        client<<b;
        client<<"</p>";
        client<<"<p>";
        client<<a.get_pinyin(b);
        client<<"</p>"; 
    }

    return "";
}

}//namespace http