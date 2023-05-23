#include <chrono>
#include <thread>
#include "httppeer.h"
#include "imageapi.h"
#include "serverconfig.h"
#include "func.h"
#include "upload_images.h"
namespace http
{
//@urlpath(null,imageapi/gateway)
std::string imageapi_gateway(std::shared_ptr<httppeer> peer)
{
    httppeer &client      = peer->getpeer();
    std::string action_do = client.get["action"].to_string();

    auto img = std::make_unique<upload_images>(peer);
    img->get_config();
    if (action_do == "config")
    {
        client << img->jsoncontent;
        client.out_json();
        return "";
    }
    else if (action_do == "uploadimage")
    {
        bool issuccess = img->upload_img("upfile");
        if (issuccess)
        {
            //is success upload
        }
        client << img->upload_info.to_json();
        client.out_json();
    }
    else if (action_do == "uploadscrawl")
    {
        bool issuccess = img->upload_base64("upfile");
        if (issuccess)
        {
            //is success upload
        }
        client << img->upload_info.to_json();
        client.out_json();
    }
    else if (action_do == "listimage")
    {
        img->list_img();
        client.out_json();
    }
    else if (action_do == "listfile")
    {
        img->list_file();
        client.out_json();
    }
    else if (action_do == "uploadvideo")
    {
        bool issuccess = img->upload_video("upfile");
        if (issuccess)
        {
            //is success upload
        }
        client << img->upload_info.to_json();
        client.out_json();
    }
    else if (action_do == "uploadfile")
    {
        bool issuccess = img->upload_upfile("upfile");
        if (issuccess)
        {
            //is success upload
        }
        client << img->upload_info.to_json();
        client.out_json();
    }
    return "";
}
//@urlpath(null,imageapi/upload)
std::string imageapi_upload(std::shared_ptr<httppeer> peer)
{
    httppeer &client      = peer->getpeer();
    std::string action_do = client.get["action"].to_string();

    auto img = std::make_unique<upload_images>(peer);
    img->init();

    if (action_do == "uploadimage")
    {
        bool issuccess = img->upload_img("upfile");
        if (issuccess)
        {
            //is success upload
        }
        client << img->upload_info.to_json();
        client.out_json();
    }
    else if (action_do == "listimage")
    {
        img->list_img();
        client.out_json();
    }

    return "";
}
} //namespace http