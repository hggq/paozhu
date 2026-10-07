#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "testaddclienttask.h"

namespace http
{
//@urlpath(null,testnotaddclienttaskpre)
std::string testaddclienttaskpre(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " testaddclienttaskpre <br/> ";
    return "ok";
}
// testaddclienttask must has pre method
//@urlpath(testaddclienttaskpre,testaddclienttask)
std::string testaddclienttask(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " Interval task test <br/> ";
    client << client.get_hosturl();
    // 登记下去的任务每 5 秒一拍、共 5 拍，每一拍都往 cms 库做一次 SELECT 加一次 UPDATE，
    // 也就是点一下这条 URL 换来约 25 秒的周期性写库。页面照旧对外（路径本身不是秘密），
    // 但只有本机/内网来源才真的登记任务（ip_is_local()，与证书下载页同一个闸门）。
    if (ip_is_local(client.client_ip))
    {
        peer->add_timeloop_task("executeclienttask", 5);//(regfunc,second)
        return "T";
    }
    client << "<br/> interval task not registered (local-only) <br/> ";
    return "";
}
//@urlpath(null,executeclienttask)
std::string testexecuteclienttask(std::shared_ptr<httppeer> peer)
{
    // httppeer &client = peer->get_peer();
    auto users = orm::cms::Sysuser();

    try
    {
        users.where("name", "admin").limit(1).fetch();
        if (users.getAdminid() > 0)
        {
            // not output
            users.update_col("level", 1);
            peer->add_timeloop_count();
            if (peer->get_timeloop_count() > 5)
            {
                peer->clear_timeloop_task();
            }
            return "";
        }
        else
        {
            return "";
        }
    }
    catch (std::exception &e)
    {
        return "";
    }
    return "";
}

}// namespace http