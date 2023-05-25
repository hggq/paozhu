
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sstream>
#include <filesystem>
#include "serverconfig.h"
#include "unicode.h"
#include "httppeer.h"
#include "request.h"
#include "upload_images.h"
#include "func.h"
#include "datetime.h"
#include "json_reflect_headers.h"
#include "base64.h"

namespace http
{
bool upload_images::init()
{
    sitepath = client.get_sitepath();
    return true;
}
bool upload_images::get_config()
{
    serverconfig &sysconfigpath = getserversysconfig();
    configpath                  = sysconfigpath.configpath;

    if (configpath.size() > 0 && configpath.back() != '/')
    {
        configpath.push_back('/');
    }
    configpath.append("ueditor_config.json");
    //configpath = configpath + "ueditor_config.json";

    jsoncontent = file_get_contents(configpath);
    jsoncontent = strip_annot(jsoncontent);
    if (jsoncontent.size() < 2)
    {
        return false;
    }
    config_json.from_json(jsoncontent);

    sitepath = client.get_sitepath();
    return true;
}
bool upload_images::upload_img(const std::string &fieldname)
{
    namespace fs      = std::filesystem;
    fs::path paths    = sitepath;
    upload_info.state = "未知错误";
    if (fs::exists(paths))
    {
        std::string upimgpath = sitepath;
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append("upload");
        paths           = upimgpath;
        bool is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }

        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append("image");
        paths      = upimgpath;
        is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }
        std::string tempstr = dateid("%Y%m");
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append(tempstr);
        paths      = upimgpath;
        is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }

        url_path = "/";
        url_path.append("upload/image/");
        url_path.append(tempstr);
        url_path.push_back('/');

        std::string filename = dateid("%Y%m%d%H%M%S");
        filename.append(rand_string(6, 4));
        //filename.append("_");

        tempstr = client.files[fieldname]["filename"].to_string();
        // filename.append(mb_substr(tempstr, 0, 10));

        bool isshowfile          = true;
        unsigned int extfilesize = tempstr.size();
        if (extfilesize > 3)
        {
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'n' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".png";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'j' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".jpg";
                isshowfile       = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'v' &&
                tempstr[extfilesize - 3] == 's' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".svg";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'f' && tempstr[extfilesize - 2] == 'i' &&
                tempstr[extfilesize - 3] == 'g' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".gif";
                isshowfile       = false;
            }

            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'e' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == 'j' && tempstr[extfilesize - 5] == '.')
            {
                upload_info.type = ".jpeg";
                isshowfile       = false;
            }
            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'p' && tempstr[extfilesize - 2] == 'b' &&
                tempstr[extfilesize - 3] == 'e' && tempstr[extfilesize - 4] == 'w' && tempstr[extfilesize - 5] == '.')
            {
                upload_info.type = ".webp";
                isshowfile       = false;
            }

            if (!isshowfile)
            {
                filename.append(upload_info.type);
                upimgpath.append(filename);
                url_path.append(filename);
                fs::rename(client.files[fieldname]["tempfile"].as_string(), upimgpath);
                upload_info.state    = "SUCCESS";
                upload_info.url      = url_path;
                upload_info.title    = filename;
                upload_info.original = tempstr;
                upload_info.size     = client.files[fieldname]["size"].to_int();
            }
            else
            {
                upload_info.state = "文件类型不允许";
            }
            return isshowfile;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
bool upload_images::upload_video(const std::string &fieldname)
{
    namespace fs      = std::filesystem;
    fs::path paths    = sitepath;
    upload_info.state = "未知错误";
    if (fs::exists(paths))
    {
        std::string upimgpath = sitepath;
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append("upload");
        paths           = upimgpath;
        bool is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }

        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append("video");
        paths      = upimgpath;
        is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }
        std::string tempstr = dateid("%Y%m");
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append(tempstr);
        paths      = upimgpath;
        is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        std::string filename = dateid("%Y%m%d%H%M%S");
        filename.append(rand_string(6, 4));

        url_path = "/";
        url_path.append("upload/video/");
        url_path.append(tempstr);
        url_path.push_back('/');

        tempstr                  = client.files[fieldname]["filename"].to_string();
        bool isshowfile          = true;
        unsigned int extfilesize = tempstr.size();
        if (extfilesize > 3)
        {
            if (isshowfile && tempstr[extfilesize - 1] == '4' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'm' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".mp4";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'v' && tempstr[extfilesize - 2] == 'k' &&
                tempstr[extfilesize - 3] == 'm' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".mkv";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == '3' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'm' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".mp3";
                isshowfile       = false;
            }
            if (!isshowfile)
            {
                filename.append(upload_info.type);
                upimgpath.append(filename);
                url_path.append(filename);
                fs::rename(client.files[fieldname]["tempfile"].as_string(), upimgpath);
                upload_info.state    = "SUCCESS";
                upload_info.url      = url_path;
                upload_info.title    = filename;
                upload_info.original = tempstr;
                upload_info.size     = client.files[fieldname]["size"].to_int();
            }
            else
            {
                upload_info.state = "文件类型不允许";
            }
            return isshowfile;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool upload_images::upload_base64(const std::string &fieldname)
{
    namespace fs      = std::filesystem;
    fs::path paths    = sitepath;
    upload_info.state = "未知错误";
    if (fs::exists(paths))
    {
        std::string upimgpath = sitepath;
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append("upload");
        paths           = upimgpath;
        bool is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }

        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append("image");
        paths      = upimgpath;
        is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }
        std::string tempstr = dateid("%Y%m");
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append(tempstr);
        paths      = upimgpath;
        is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        std::string filename = std::to_string(timeid());
        filename.append(rand_string(6, 4));

        url_path = "/";
        url_path.append("upload/image/");
        url_path.append(tempstr);
        url_path.push_back('/');

        upload_info.type = ".png";

        filename.append(upload_info.type);
        upimgpath.append(filename);
        url_path.append(filename);

        std::string imgcontent = client.post[fieldname].as_string();
        imgcontent             = base64_decode(&imgcontent[0], imgcontent.size());
        file_put_contents(upimgpath, imgcontent);
        upload_info.state    = "SUCCESS";
        upload_info.url      = url_path;
        upload_info.title    = filename;
        upload_info.original = "scrawl.png";
        upload_info.size     = imgcontent.size();

        return true;
    }
    else
    {
        return false;
    }
    return true;
}

bool upload_images::upload_upfile(const std::string &fieldname)
{
    namespace fs      = std::filesystem;
    fs::path paths    = sitepath;
    upload_info.state = "未知错误";
    if (fs::exists(paths))
    {
        std::string upimgpath = sitepath;
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append("upload");
        paths           = upimgpath;
        bool is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }

        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append("file");
        paths      = upimgpath;
        is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }
        std::string tempstr = dateid("%Y%m");
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        upimgpath.append(tempstr);
        paths      = upimgpath;
        is_success = false;
        if (!fs::exists(paths))
        {
            is_success = fs::create_directories(paths);
            if (is_success)
            {
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        else
        {
            is_success = true;
        }
        if (upimgpath.back() != '/')
        {
            upimgpath.push_back('/');
        }
        // std::string filename = std::to_string(timeid());
        // filename.append(rand_string(6, 4));

        url_path = "/";
        url_path.append("upload/file/");
        url_path.append(tempstr);
        url_path.push_back('/');

        std::string filename = dateid("%Y%m%d%H%M%S");
        filename.append(rand_string(3, 4));
        filename.append("_");

        tempstr = client.files[fieldname]["filename"].to_string();

        unsigned int extfilesize = 0;
        for (extfilesize = 0; extfilesize < tempstr.size(); extfilesize++)
        {
            if (tempstr[extfilesize] == '.')
            {
                break;
            }
        }
        tempstr = tempstr.substr(0, extfilesize);
        filename.append(mb_substr(tempstr, 0, 13));

        tempstr         = client.files[fieldname]["filename"].to_string();
        bool isshowfile = true;
        extfilesize     = tempstr.size();
        if (extfilesize > 3)
        {
            if (isshowfile && tempstr[extfilesize - 1] == 'p' && tempstr[extfilesize - 2] == 'i' &&
                tempstr[extfilesize - 3] == 'z' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".zip";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'r' && tempstr[extfilesize - 2] == 'a' &&
                tempstr[extfilesize - 3] == 'r' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".rar";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'r' && tempstr[extfilesize - 2] == 'a' &&
                tempstr[extfilesize - 3] == 't' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".tar";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'f' && tempstr[extfilesize - 2] == 'd' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".pdf";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'c' && tempstr[extfilesize - 2] == 'o' &&
                tempstr[extfilesize - 3] == 'd' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".doc";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == '4' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'm' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".mp4";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == '3' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'm' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".mp3";
                isshowfile       = false;
            }
            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'p' && tempstr[extfilesize - 2] == 'b' &&
                tempstr[extfilesize - 3] == 'e' && tempstr[extfilesize - 4] == 'w' && tempstr[extfilesize - 5] == '.')
            {
                upload_info.type = ".webp";
                isshowfile       = false;
            }
            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'x' && tempstr[extfilesize - 2] == 'c' &&
                tempstr[extfilesize - 3] == 'o' && tempstr[extfilesize - 4] == 'd' && tempstr[extfilesize - 5] == '.')
            {
                upload_info.type = ".docx";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 's' && tempstr[extfilesize - 2] == 'l' &&
                tempstr[extfilesize - 3] == 'x' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".xls";
                isshowfile       = false;
            }

            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'x' && tempstr[extfilesize - 2] == 's' &&
                tempstr[extfilesize - 3] == 'l' && tempstr[extfilesize - 4] == 'x' && tempstr[extfilesize - 5] == '.')
            {
                upload_info.type = ".xlsx";
                isshowfile       = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 't' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".ppt";
                isshowfile       = false;
            }

            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'x' && tempstr[extfilesize - 2] == 't' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == 'p' && tempstr[extfilesize - 5] == '.')
            {
                upload_info.type = ".pptx";
                isshowfile       = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'j' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".jpg";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'n' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".png";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'v' &&
                tempstr[extfilesize - 3] == 's' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".svg";
                isshowfile       = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 's' && tempstr[extfilesize - 2] == 's' &&
                tempstr[extfilesize - 3] == 'c' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".css";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'm' && tempstr[extfilesize - 2] == 't' &&
                tempstr[extfilesize - 3] == 'h' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".htm";
                isshowfile       = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 'p' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'a' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".app";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'k' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".pkg";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'm' &&
                tempstr[extfilesize - 3] == 'd' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".dmg";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'a' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'i' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".ipa";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'k' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'a' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".apk";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'e' && tempstr[extfilesize - 2] == 'x' &&
                tempstr[extfilesize - 3] == 'e' && tempstr[extfilesize - 4] == '.')
            {
                upload_info.type = ".exe";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 's' && tempstr[extfilesize - 2] == 'j' &&
                tempstr[extfilesize - 3] == '.')
            {
                upload_info.type = ".js";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'd' && tempstr[extfilesize - 2] == 'm' &&
                tempstr[extfilesize - 3] == '.')
            {
                upload_info.type = ".md";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'z' && tempstr[extfilesize - 2] == 'g' &&
                tempstr[extfilesize - 3] == '.')
            {
                upload_info.type = ".gz";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'z' && tempstr[extfilesize - 2] == '7' &&
                tempstr[extfilesize - 3] == '.')
            {
                upload_info.type = ".7z";
                isshowfile       = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'z' && tempstr[extfilesize - 2] == 'x' &&
                tempstr[extfilesize - 3] == '.')
            {
                upload_info.type = ".xz";
                isshowfile       = false;
            }
            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'b' && tempstr[extfilesize - 2] == 'u' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == 'e' && tempstr[extfilesize - 5] == '.')
            {
                upload_info.type = ".epub";
                isshowfile       = false;
            }
            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'l' && tempstr[extfilesize - 2] == 'm' &&
                tempstr[extfilesize - 3] == 't' && tempstr[extfilesize - 4] == 'h' && tempstr[extfilesize - 5] == '.')
            {
                upload_info.type = ".html";
                isshowfile       = false;
            }

            if (!isshowfile)
            {
                filename.append(upload_info.type);
                upimgpath.append(filename);
                url_path.append(filename);
                fs::rename(client.files[fieldname]["tempfile"].as_string(), upimgpath);
                upload_info.state    = "SUCCESS";
                upload_info.url      = url_path;
                upload_info.title    = filename;
                upload_info.original = tempstr;
                upload_info.size     = client.files[fieldname]["size"].to_int();
            }
            else
            {
                upload_info.state = "文件类型不允许";
            }
            return isshowfile;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

unsigned int upload_images::list_img()
{
    std::map<unsigned long long, std::string> listobj;
    std::string uploadpath = sitepath;

    if (uploadpath.size() > 0 && uploadpath.back() != '/')
    {
        uploadpath.append("/");
    }
    uploadpath.append("upload/image/");
    std::string urlpath = "/upload/image/";
    get_directory_all_file(listobj, uploadpath, urlpath, ".jpg|.png|.gif|.svg|.webp|.jpeg");

    img_upload_outjson_t imglistout;
    imglistout.state = "SUCCESS";
    imglistout.start = 0;
    imglistout.total = listobj.size();

    img_upload_list_t ta;

    unsigned int start_num  = client.get["start"].to_int();
    unsigned int offset_num = client.get["size"].to_int();

    unsigned int offset  = 0;
    unsigned int end_num = 0;

    imglistout.start = start_num;
    for (auto iter = listobj.crbegin(); iter != listobj.crend(); ++iter)
    {
        if (offset >= start_num)
        {
            ta.mtime = iter->first;
            ta.url   = iter->second;
            imglistout.list.push_back(ta);
            end_num += 1;
        }

        offset += 1;

        if (end_num >= offset_num)
        {
            break;
        }
    }
    client << json_encode(imglistout);
    return imglistout.total;
}
unsigned int upload_images::list_file()
{
    std::map<unsigned long long, std::string> listobj;
    std::string uploadpath = sitepath;

    if (uploadpath.size() > 0 && uploadpath.back() != '/')
    {
        uploadpath.append("/");
    }
    uploadpath.append("upload/file/");
    std::string urlpath = "/upload/file/";
    get_directory_all_file(listobj,
                           uploadpath,
                           urlpath,
                           ".jpg|.png|.gif|.svg|.jpeg|.js|.htm|.html|.zip|.gz|.rar|.tar|.7z|.md|.css|.doc|.docx|.xls|."
                           "xlsx|.ppt|.pptx|.pdf|.css|.mp4|.webp|.epub|.exe|.dmg|.app|.ipa|.pkg|.bin|.xz");

    uploadpath = sitepath;
    if (uploadpath.size() > 0 && uploadpath.back() != '/')
    {
        uploadpath.append("/");
    }
    uploadpath.append("upload/video/");
    urlpath = "/upload/video/";
    get_directory_all_file(listobj, uploadpath, urlpath, "|.mp4|.mkv");

    img_upload_outjson_t imglistout;
    imglistout.state = "SUCCESS";
    imglistout.start = 0;
    imglistout.total = listobj.size();

    img_upload_list_t ta;

    unsigned int start_num  = client.get["start"].to_int();
    unsigned int offset_num = client.get["size"].to_int();

    unsigned int offset  = 0;
    unsigned int end_num = 0;

    imglistout.start = start_num;
    for (auto iter = listobj.crbegin(); iter != listobj.crend(); ++iter)
    {
        if (offset >= start_num)
        {
            ta.mtime = iter->first;
            ta.url   = iter->second;
            imglistout.list.push_back(ta);
            end_num += 1;
        }

        offset += 1;

        if (end_num >= offset_num)
        {
            break;
        }
    }
    client << json_encode(imglistout);
    return imglistout.total;
}
} //namespace http
