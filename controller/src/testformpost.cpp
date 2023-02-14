#include <chrono>
#include <thread>
#include <filesystem>
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
      // from /addpost.html url
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
   //@urlpath(null,addpostfile)
   std::string testuploadpostfile(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      client << "hello world!  upload file";

      try
      {
         client << "<p>fileField:name:" << client.files["fileField"]["name"] << "</p>";
         client << "<p>fileField:filename:" << client.files["fileField"]["filename"] << "</p>";
         client << "<p>fileField:tempfile:" << client.files["fileField"]["tempfile"] << "</p>";
         client << "<p>fileField:size:" << client.files["fileField"]["size"] << "</p>";
         client << "<p>fileField:error:" << client.files["fileField"]["error"] << "</p>";

         std::string sitepath = client.get_sitepath();

         if (!sitepath.empty())
         {

            namespace fs = std::filesystem;
            fs::path paths = sitepath;
            if (fs::exists(paths))
            {
               if (sitepath.back() != '/')
               {
                  sitepath.push_back('/');
               }
               sitepath.append("upload");
               paths = sitepath;
               bool is_success = false;
               if (!fs::exists(paths))
               {
                  bool is_success = fs::create_directories(paths);
                  if (is_success)
                  {
                     fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                     fs::perm_options::add);

                     is_success = true;
                  }
               }
               else
               {
                  is_success = true;
               }
               if (is_success)
               {
                  std::string filename = client.files["fileField"]["filename"].as_string();
                  filename = str2safefile((const char *)&filename[0], filename.size());

                  unsigned int extfilesize = filename.size();
                  bool isshowfile = true;
                  if (extfilesize > 3)
                  {
                     if (filename[extfilesize - 1] == 's' && filename[extfilesize - 2] == 'j' && filename[extfilesize - 3] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 'd' && filename[extfilesize - 2] == 'm' && filename[extfilesize - 3] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 'g' && filename[extfilesize - 2] == 'n' && filename[extfilesize - 3] == 'p' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 'g' && filename[extfilesize - 2] == 'p' && filename[extfilesize - 3] == 'j' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 't' && filename[extfilesize - 2] == 'x' && filename[extfilesize - 3] == 't' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 'f' && filename[extfilesize - 2] == 'd' && filename[extfilesize - 3] == 'p' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 's' && filename[extfilesize - 2] == 's' && filename[extfilesize - 3] == 'c' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 'm' && filename[extfilesize - 2] == 't' && filename[extfilesize - 3] == 'h' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 'g' && filename[extfilesize - 2] == 'v' && filename[extfilesize - 3] == 's' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 'f' && filename[extfilesize - 2] == 'i' && filename[extfilesize - 3] == 'g' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && filename[extfilesize - 1] == 'p' && filename[extfilesize - 2] == 'a' && filename[extfilesize - 3] == 'm' & filename[extfilesize - 4] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && extfilesize > 4 && filename[extfilesize - 1] == 'l' && filename[extfilesize - 2] == 'm' && filename[extfilesize - 3] == 't' & filename[extfilesize - 4] == 'h' && filename[extfilesize - 5] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && extfilesize > 4 && filename[extfilesize - 1] == 'g' && filename[extfilesize - 2] == 'e' && filename[extfilesize - 3] == 'p' & filename[extfilesize - 4] == 'j' && filename[extfilesize - 5] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                     if (isshowfile && extfilesize > 4 && filename[extfilesize - 1] == 'p' && filename[extfilesize - 2] == 'b' && filename[extfilesize - 3] == 'e' & filename[extfilesize - 4] == 'w' && filename[extfilesize - 5] == '.')
                     {
                        sitepath.push_back('/');
                        sitepath.append(filename);
                        isshowfile = false;
                     }
                  }
                  if (isshowfile)
                  {
                     sitepath.push_back('/');
                     sitepath.append(get_filename(client.files["fileField"]["tempfile"].as_string()));
                     sitepath.append(".data");
                     client << "<p><a href=\"" << client.get_hosturl() << "/upload/" << get_filename(client.files["fileField"]["tempfile"].as_string()) << ".data\" target=_blank>show</a></p>";
                  }
                  else
                  {
                     client << "<p><a href=\"" << client.get_hosturl() << "/upload/" << filename << "\" target=_blank>show</a></p>";
                  }
                  fs::rename(client.files["fileField"]["tempfile"].as_string(), sitepath);
               }
            }
         }
      }
      catch (std::exception &e)
      {
         client << "<p>" << e.what() << "</p>";
         return "";
      }
      return "";
   }
}