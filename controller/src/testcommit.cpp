#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testcommit.h"
namespace http
{

//@urlpath(null,testcommit)
std::string testcommit(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << " mysql commit test ";
    orm::cms::Testb ot1;
    orm::cms::Testa ot2;

    orm::commit_lists_callback commitlist;

    ot2.data.value   = 65;
    ot2.data.content = "commit test";
    ot2.open_commit();
    ot2.insert();

    commitlist.push_back({ot2.get_query(),
                          [&ot2](long long status, long long lastid) -> void
                          {
                              if (status == 0)
                              {
                                  ot2.setPK(lastid);
                              }
                          }});

    ot1.data.tid  = 4;
    ot1.data.name = "caname";
    ot1.open_commit();
    ot1.insert();
    // Visit commit again is fail,Because tid is fixed to 4
    // commitlist.push_back({ot1.get_query(),[&ot1](long long status,long long lastid)->void{  ot1.setPK(lastid);  }});
    commitlist.push_back({ot1.get_query(), nullptr});

    ot2.submit_commit(commitlist);

    // ot1.close_commit();
    // ot2.close_commit();
    for (auto [first, second] : commitlist)
    {
        client << "<p>" << first << "</p>";
    }

    client << "<p>ot1:" << std::to_string(ot1.getPK()) << "</p>";
    client << "<p>ot2:" << std::to_string(ot2.getPK()) << "</p>";
    client << "status:<p>" << ot2.error_msg << "</p>";
    return "";
}

}// namespace http
