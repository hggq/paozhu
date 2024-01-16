#include "orm.h"
#include "httppeer.h"
#include "testfield_num.h"

namespace http
{
//@urlpath(null,testfieldnum)
std::string testfieldnum(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    long long a      = 0;
    double h         = 0.0;
    client << "<p>test field to num</p>";
    auto testb = orm::cms::Testb();
    testb.limit(2).fetch();

    if (testb.record.size() > 0)
    {
        for (unsigned i = 0; i < testb.record.size(); i++)
        {
            a = testb.record[i].score;
            h = testb.getScoreToNum(testb.record[i].score);
            client << "<p>a|" << a << " h:" << h << " h to num:" << testb.getNumToScore(h) << "</p>";
            h = h + 0.055;

            testb.setScoreToNum(h);
            testb.clear(false);
            testb.where("tid", testb.record[i].tid).limit(1);
            testb.update("score");
        }
    }
    return "";
}

}// namespace http