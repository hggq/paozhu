#include <chrono>
#include <thread>
#include "httppeer.h"
#include "attachfile.h"
#include "serverconfig.h"
#include "directory_fun.h"
#include "func.h"

namespace http
{
//@urlpath(admin_islogin,attachfile/gateway)
std::string attachfile_gateway(std::shared_ptr<httppeer> peer)
{
    httppeer &client            = peer->get_peer();
    std::string pathattach      = client.session["sitepath"].to_string();
    serverconfig &sysconfigpath = getserversysconfig();
    if (pathattach.size() > 0 && pathattach.back() != '/')
    {
        pathattach.push_back('/');
    }
    pathattach.append("upload/file");
    peer->urlpath             = "/attachfile/gateway";
    std::string deleteurlpath = "/attachfile/delete";

    std::string realurlpath = "/upload/file";
    bool isroot             = false;
    for (unsigned char i = 2; i < peer->pathinfos.size(); i++)
    {
        peer->urlpath.append("/");
        peer->urlpath.append(peer->pathinfos[i]);

        pathattach.append("/");
        pathattach.append(peer->pathinfos[i]);

        deleteurlpath.append("/");
        deleteurlpath.append(peer->pathinfos[i]);

        realurlpath.append("/");
        realurlpath.append(peer->pathinfos[i]);
        isroot = true;
    }
    std::string content = displaydirectory_attachfile(pathattach,
                                                      peer->urlpath,
                                                      realurlpath,
                                                      deleteurlpath,
                                                      peer->get["sort"].as_string(),
                                                      sysconfigpath.configpath,
                                                      isroot);
    client << content;
    return "";
}
//@urlpath(admin_islogin,attachimg/gateway)
std::string attachimg_gateway(std::shared_ptr<httppeer> peer)
{
    httppeer &client            = peer->get_peer();
    std::string pathattach      = client.session["sitepath"].to_string();
    serverconfig &sysconfigpath = getserversysconfig();
    if (pathattach.size() > 0 && pathattach.back() != '/')
    {
        pathattach.push_back('/');
    }
    pathattach.append("upload/image");

    peer->urlpath             = "/attachimg/gateway";
    std::string realurlpath   = "/upload/image";
    std::string deleteurlpath = "/attachimg/delete";
    bool isroot               = false;

    for (unsigned char i = 2; i < peer->pathinfos.size(); i++)
    {
        peer->urlpath.append("/");
        peer->urlpath.append(peer->pathinfos[i]);

        pathattach.append("/");
        pathattach.append(peer->pathinfos[i]);

        deleteurlpath.append("/");
        deleteurlpath.append(peer->pathinfos[i]);

        realurlpath.append("/");
        realurlpath.append(peer->pathinfos[i]);
        isroot = true;
    }
    std::string content = displaydirectory_attachimg(pathattach,
                                                     peer->urlpath,
                                                     realurlpath,
                                                     deleteurlpath,
                                                     peer->get["sort"].as_string(),
                                                     sysconfigpath.configpath,
                                                     isroot);
    client << content;
    return "";
}

//@urlpath(admin_islogin,attachfile/delete)
std::string attachfile_delete(std::shared_ptr<httppeer> peer)
{
    httppeer &client       = peer->get_peer();
    std::string pathattach = client.session["sitepath"].to_string();
    if (pathattach.size() > 0 && pathattach.back() != '/')
    {
        pathattach.push_back('/');
    }
    pathattach.append("upload/file");

    for (unsigned char i = 2; i < peer->pathinfos.size(); i++)
    {
        pathattach.append("/");
        pathattach.append(peer->pathinfos[i]);
    }

    std::string filename = client.get["file"].to_string();

    for (unsigned int i = 0; i < filename.size(); i++)
    {
        if (filename[i] == '.')
        {
            unsigned int j = i + 1;
            if (j < filename.size() && filename[j] == '.')
            {
                client.goto_url("/attachfile/gateway", 1, "file name error!");
                return "";
            }
        }
        else if (filename[i] == 0x2F)
        {
            client.goto_url("/attachfile/gateway", 1, "file name error!");
            return "";
        }
    }

    pathattach.append("/");
    pathattach.append(filename);

    std::string delete_state = "Delete";
    if (std::filesystem::exists(pathattach))
    {
        if (std::filesystem::remove(pathattach))
        {
            delete_state.append(" success. ");
        }
        else
        {
            delete_state.append(" error! ");
        }
    }
    else
    {
        delete_state.append(" not exists! ");
    }
    pathattach = "/attachfile/gateway";
    for (unsigned char i = 2; i < peer->pathinfos.size(); i++)
    {
        pathattach.append("/");
        pathattach.append(peer->pathinfos[i]);
    }

    client.goto_url(pathattach, 1, delete_state);
    return "";
}

//@urlpath(admin_islogin,attachimg/delete)
std::string attachimg_delete(std::shared_ptr<httppeer> peer)
{
    httppeer &client       = peer->get_peer();
    std::string pathattach = client.session["sitepath"].to_string();
    if (pathattach.size() > 0 && pathattach.back() != '/')
    {
        pathattach.push_back('/');
    }
    pathattach.append("upload/image");

    for (unsigned char i = 2; i < peer->pathinfos.size(); i++)
    {
        pathattach.append("/");
        pathattach.append(peer->pathinfos[i]);
    }

    std::string filename = client.get["file"].to_string();
    filename             = str2safefile(filename.data(), filename.size());
    pathattach.append("/");
    pathattach.append(filename);

    std::string delete_state = "Delete";
    if (std::filesystem::exists(pathattach))
    {
        if (std::filesystem::remove(pathattach))
        {
            delete_state.append(" success. ");
        }
        else
        {
            delete_state.append(" error! ");
        }
    }
    else
    {
        delete_state.append(" not exists! ");
    }
    pathattach = "/attachimg/gateway";
    for (unsigned char i = 2; i < peer->pathinfos.size(); i++)
    {
        pathattach.append("/");
        pathattach.append(peer->pathinfos[i]);
    }

    client.goto_url(pathattach, 1, delete_state);
    return "";
}

//@urlpath(admin_islogin,attachfile/upload)
std::string attachfile_upload(std::shared_ptr<httppeer> peer)
{
    //httppeer &client = peer->get_peer();
    peer->view("admin/fileupload");
    return "";
}
//@urlpath(admin_islogin,attachfile/uploadpost)
std::string attachfile_uploadpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client     = peer->get_peer();
    std::string sitepath = client.session["sitepath"].to_string();
    namespace fs         = std::filesystem;

    fs::path paths = sitepath;
    if (sitepath.size() > 0 && sitepath.back() != '/')
    {
        sitepath.push_back('/');
    }
    std::string upload_info_state = "未知错误";
    std::string fieldname         = "upfile";
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
        std::string tempstr = get_date("%Y%m");
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

        std::string filename = get_date("%Y%m%d%H%M%S");
        //filename.append(rand_string(3, 4));
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
        filename.append(mb_substr(tempstr, 0, 15));

        tempstr = client.files[fieldname]["filename"].to_string();
        for (unsigned int jj = extfilesize; jj < tempstr.size(); jj++)
        {
            if (tempstr[jj] > 0x40 && tempstr[jj] < 0x5B)
            {
                tempstr[jj] = tempstr[jj] + 32;
            }
        }
        bool isshowfile = true;
        extfilesize     = tempstr.size();
        std::string file_ext;
        if (extfilesize > 1)
        {
            if (isshowfile && tempstr[extfilesize - 1] == 'p' && tempstr[extfilesize - 2] == 'i' &&
                tempstr[extfilesize - 3] == 'z' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".zip";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'r' && tempstr[extfilesize - 2] == 'a' &&
                tempstr[extfilesize - 3] == 'r' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".rar";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'r' && tempstr[extfilesize - 2] == 'a' &&
                tempstr[extfilesize - 3] == 't' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".tar";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'f' && tempstr[extfilesize - 2] == 'd' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".pdf";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'c' && tempstr[extfilesize - 2] == 'o' &&
                tempstr[extfilesize - 3] == 'd' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".doc";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == '4' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'm' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".mp4";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == '3' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'm' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".mp3";
                isshowfile = false;
            }
            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'p' && tempstr[extfilesize - 2] == 'b' &&
                tempstr[extfilesize - 3] == 'e' && tempstr[extfilesize - 4] == 'w' && tempstr[extfilesize - 5] == '.')
            {
                file_ext   = ".webp";
                isshowfile = false;
            }
            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'x' && tempstr[extfilesize - 2] == 'c' &&
                tempstr[extfilesize - 3] == 'o' && tempstr[extfilesize - 4] == 'd' && tempstr[extfilesize - 5] == '.')
            {
                file_ext   = ".docx";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 's' && tempstr[extfilesize - 2] == 'l' &&
                tempstr[extfilesize - 3] == 'x' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".xls";
                isshowfile = false;
            }

            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'x' && tempstr[extfilesize - 2] == 's' &&
                tempstr[extfilesize - 3] == 'l' && tempstr[extfilesize - 4] == 'x' && tempstr[extfilesize - 5] == '.')
            {
                file_ext   = ".xlsx";
                isshowfile = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 't' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".ppt";
                isshowfile = false;
            }

            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'x' && tempstr[extfilesize - 2] == 't' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == 'p' && tempstr[extfilesize - 5] == '.')
            {
                file_ext   = ".pptx";
                isshowfile = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'j' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".jpg";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'n' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".png";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'v' &&
                tempstr[extfilesize - 3] == 's' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".svg";
                isshowfile = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 'p' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'a' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".app";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'k' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".pkg";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'g' && tempstr[extfilesize - 2] == 'm' &&
                tempstr[extfilesize - 3] == 'd' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".dmg";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'a' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'i' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".ipa";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'k' && tempstr[extfilesize - 2] == 'p' &&
                tempstr[extfilesize - 3] == 'a' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".apk";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'e' && tempstr[extfilesize - 2] == 'x' &&
                tempstr[extfilesize - 3] == 'e' && tempstr[extfilesize - 4] == '.')
            {
                file_ext   = ".exe";
                isshowfile = false;
            }

            if (isshowfile && tempstr[extfilesize - 1] == 'z' && tempstr[extfilesize - 2] == 'g' &&
                tempstr[extfilesize - 3] == '.')
            {
                file_ext   = ".gz";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'z' && tempstr[extfilesize - 2] == '7' &&
                tempstr[extfilesize - 3] == '.')
            {
                file_ext   = ".7z";
                isshowfile = false;
            }
            if (isshowfile && tempstr[extfilesize - 1] == 'z' && tempstr[extfilesize - 2] == 'x' &&
                tempstr[extfilesize - 3] == '.')
            {
                file_ext   = ".xz";
                isshowfile = false;
            }
            if (isshowfile && extfilesize > 4 && tempstr[extfilesize - 1] == 'b' && tempstr[extfilesize - 2] == 'u' &&
                tempstr[extfilesize - 3] == 'p' && tempstr[extfilesize - 4] == 'e' && tempstr[extfilesize - 5] == '.')
            {
                file_ext   = ".epub";
                isshowfile = false;
            }

            if (!isshowfile)
            {
                filename.append(file_ext);
                upimgpath.append(filename);
                fs::rename(client.files[fieldname]["tempfile"].as_string(), upimgpath);
                upload_info_state = "SUCCESS";
            }
            else
            {
                upload_info_state = "文件类型不允许";
            }
        }
    }

    client.goto_url("/attachfile/upload", 1, upload_info_state);
    return "";
}

//@urlpath(admin_islogin,admin/attachselectfiles)
std::string admin_attachselectfiles(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    std::string pathattach      = client.session["sitepath"].to_string();
    serverconfig &sysconfigpath = getserversysconfig();
    if (pathattach.size() > 0 && pathattach.back() != '/')
    {
        pathattach.push_back('/');
    }
    pathattach.append("upload/file");
    peer->urlpath             = "/upload/file";
    std::string deleteurlpath = "/attachfile/delete";

    std::string realurlpath = "/upload/file";
    bool isroot             = false;
    for (unsigned char i = 2; i < peer->pathinfos.size(); i++)
    {
        peer->urlpath.append("/");
        peer->urlpath.append(peer->pathinfos[i]);

        pathattach.append("/");
        pathattach.append(peer->pathinfos[i]);

        deleteurlpath.append("/");
        deleteurlpath.append(peer->pathinfos[i]);

        realurlpath.append("/");
        realurlpath.append(peer->pathinfos[i]);
        isroot = true;
    }
    auto dirlists = displaydirectory_attachfilejson(pathattach,
                                                    peer->urlpath,
                                                    realurlpath,
                                                    deleteurlpath,
                                                    peer->get["sort"].as_string(),
                                                    sysconfigpath.configpath,
                                                    isroot);

    client << "{\"code\":0,\"list\":";
    client << json_encode(dirlists);
    client << "}";
    client.out_json();
    return "";
}

}//namespace http