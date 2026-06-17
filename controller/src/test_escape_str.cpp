#include "orm.h" 
#include "httppeer.h"
#include "test_escape_str.h"
#include "request.h"
#include "world_base.h"
#include "func.h"
#include "unicode.h"
#include <memory>
#include <string>

 
namespace http
{

//@urlpath(null,test_escapestr)
std::string test_escapestr(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string str1="admin' or '1'='1"; 
    obj_val obj1;
    obj1=str1;
    client <<str1<<"|";
    client << obj1.to_escape()<< "|exp:"<<"admin\\\' or \\\'1\\\'=\\\'1<br />";

    std::string str2=" 111 333  \t"; 
    obj1=str2;
    client <<"|"<<str2<<"|exp:"<< "|";
    client << obj1.to_trim()<<"|exp:111 333|<br />";
    obj1.trim();
    client << "| 111 333  |";
    client << obj1.to_string();
    client << "|exp:111 333|<br />";


    std::string str3=" \n \t"; 
    obj1=str3;
    client <<"|"<<str3<<"|"<< "|exp:[empty]|";
    client << obj1.to_trim();
    client << "|"<<obj1.size()<<"|exp:4";
    return "";
}
 

}// namespace http