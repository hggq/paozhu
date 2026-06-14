#include "Testb.h"
#include "orm.h"
#include "httppeer.h"
#include <string>
#include <sstream>
#include "testfield_num.h"

ORM_CMS_TESTB_TREE_PTR_STRUCT(MyPtrStruct, tid, name,pricenum);
namespace http
{
//@urlpath(null,testfieldnum)
std::string testfieldnum(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    long long a = 0, b = 0;
    double h = 0.0;
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
            b = b + a;
            testb.setScoreToNum(h);
            testb.clear(false);
            testb.where("tid", testb.record[i].tid).limit(1);
            testb.update("score");
            client << "<p>  float name:" << testb.record[i].name << "</p>";
            std::stringstream stream;
     
            stream.precision(std::numeric_limits<double>::max_digits10);
  
            stream << testb.record[i].pricenum;

            client << "<p>  float getPricenum:" << stream.str()<< "</p>";
            client << "<p>  float orgprice:" << testb.record[i].orgprice  << "</p>";
            client << "<p>  float subprice:" << testb.record[i].subprice << "</p>";
        }
    }
    client << "<p>double b total:" << testb.getScoreToNum(b) << "</p>";

    orm::cms::testb_info::MyPtrStruct ta;
    std::vector<orm::cms::testb_info::MyPtrStruct> temm;
    testb.clear();
    testb.limit(2).fetch_to(temm);

    client << "<p>to_json:" << orm::cms::testb_info::to_json(temm) << "</p>";
    return "";
}

}// namespace http