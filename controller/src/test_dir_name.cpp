#include "httppeer.h"
#include "test_dir_name.h"
#include "func.h"
#include <memory>
#include <string>

namespace http
{
//@urlpath(null,test_dir_name)
std::string test_dir_name(std::shared_ptr<httppeer> peer)
{
    httppeer &client                   = peer->get_peer();
 
    client << "C:\\xampp\\htdocs\\ssss/aa/file.txt" << ":" << dir_name("C:\\xampp\\htdocs\\ssss/aa/file.txt");
    client << "<br>C:\\xampp\\htdocs\\file.txt" << ":" << dir_name("C:\\xampp\\htdocs\\file.txt");
    client << "<br>C:\\xampp\\htdocs\\..\\file.txt" << ":" << dir_name("C:\\xampp\\htdocs\\..\\file.txt");
    client << "<br>/var/www/html/index.php" << ":" << dir_name("/var/www/html/index.php");
    client << "<br>file.txt" << ":" << dir_name("file.txt");
    client << "<br>/var/www/html/index/" << ":" << dir_name("/var/www/html/index/");
    client << "<br>/var/www/html/../index/" << ":" << dir_name("/var/www/html/../index/");

    return "";
}

}// namespace http