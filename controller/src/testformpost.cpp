#include <chrono>
#include <thread>

#include "httppeer.h"
#include "func.h"
#include "testformpost.h"

namespace http
{
   //@urlpath(null,tformpost)
   std::string testurlencoded(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      client << "hello world!  application/x-www-form-urlencoded ";

      try
      {
         client << "<p>--------------</p>";
         client << "<p>textfield2:" << client.post["textfield2"] << "</p>";
         client << "<p>cc[22][]:" << client.post["cc"][22][0] << "</p>";
         client << "<p>cc[aabbcc][]:" << client.post["cc"]["aabbcc"][0] << "</p>";
         client << "<p>textarea2:" << client.post["textarea2"] << "</p>";
         client << "<p>submit2:" << client.post["submit2"] << "</p>";
         client << "<p>--------------</p>";
      }
      catch (std::exception &e)
      {
         client << "<p>" << e.what() << "</p>";
         return "";
      }
      return "";
   }
   //@urlpath(null,tfilepost)
   std::string testformmultipart(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      client << "hello world!  multipart/form-data ";

      try
      {
         client << "<p>--------------</p>";
         client << "<p>textfield:" << client.post["textfield"] << "</p>";
         client << "<p>aa[]:" << client.post["aa"][0] << "</p>";
         client << "<p>aa[]:" << client.post["aa"][1] << "</p>";
         client << "<p>aa[8]:" << client.post["aa"][8] << "</p>";
         client << "<p>textarea:" << client.post["textarea"] << "</p>";

         client << "<p>fileField:name:" << client.files["fileField"]["name"] << "</p>";
         client << "<p>fileField:filename:" << client.files["fileField"]["filename"] << "</p>";
         client << "<p>fileField:tempfile:" << client.files["fileField"]["tempfile"] << "</p>";
         client << "<p>fileField:size:" << client.files["fileField"]["size"] << "</p>";
         client << "<p>fileField:error:" << client.files["fileField"]["error"] << "</p>";

         client << "<p>submit:" << client.post["submit"] << "</p>";

         client << "<p>--------------</p>";
      }
      catch (std::exception &e)
      {
         client << "<p>" << e.what() << "</p>";
         return "";
      }
      return "";
   }
   //@urlpath(null,tjsonpost)
   std::string testformjsonpost(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      client << "hello world!  application/json ";

      try
      {
         client << "<p>--------------</p>";
         client << "<p>firstName:" << client.json["firstName"] << "</p>";
         client << "<p>lastName:" << client.json["lastName"] << "</p>";

         client << "<p>--------------</p>";
         client << "<p>rawcontent:" << client.rawcontent << "</p>";
      }
      catch (std::exception &e)
      {
         client << "<p>" << e.what() << "</p>";
         return "";
      }
      return "";
   }
   //@urlpath(null,txmlupload)
   std::string testformxmlpost(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      client << "hello world!  application/xml ";

      try
      {
         client << "<p>--------------</p>";

         client << "<p>rawcontent:" << client.rawcontent << "</p>";

         client << "<p>--------------</p>";
      }
      catch (std::exception &e)
      {
         client << "<p>" << e.what() << "</p>";
         return "";
      }
      return "";
   }
}