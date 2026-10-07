#include "httppeer.h"
#include "testdepthprobe.h"

// 路由深度探针：验证"整条 URL 精确命中 → 未命中落磁盘 → 磁盘没有才走 ≤6 段前缀兜底"
// 这条放置顺序。四条注册只有本文件会触发，正文刻意各不相同，便于 h1/h2 逐条对照。
namespace http
{
//@urlpath(null,dpshadow)
std::string testdpshadowshort(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "PROBE-WALK-SHORT dpshadow";
    return "";
}
//@urlpath(null,dpshadow/sub)
std::string testdpshadowexact(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "PROBE-EXACT dpshadow/sub";
    return "";
}
//@urlpath(null,dp6/a/b/c/d/e/:tail)
std::string testdpprobedepth6(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "PROBE-DEPTH6 tail:";
    client << client.get["tail"].to_string();
    return "";
}
//@urlpath(null,dp7/a/b/c/d/e/f/:tail)
std::string testdpprobedepth7(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "PROBE-DEPTH7 tail:";
    client << client.get["tail"].to_string();
    return "";
}
//@urlpath(null,testuser/info/:userid/:groupid)
std::string testdpuserinfo2(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "PROBE-USER2 userid:";
    client << client.get["userid"].to_string();
    client << " groupid:";
    client << client.get["groupid"].to_string();
    return "";
}
}// namespace http
