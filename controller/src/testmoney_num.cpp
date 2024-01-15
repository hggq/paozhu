#include "httppeer.h"
#include "testmoney_num.h"
#include "func.h"

namespace http
{
//@urlpath(null,testtestmoneynum)
std::string testtestmoneynum(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    long long a = 0, b = 0;
    a = 2001;
    b = 495700;
    a = a + b;
    client << "<p>a|" << a << "</p>";
    long long c = num_put_money(a);
    client << "<p>c|num_put_money|" << c << "</p>";
    client << c;

    long long d = num_get_money(c);
    client << "<p>d|num_put_money|" << d << "</p>";

    double e = (double)a / 63;
    client << "<p>e|" << e << "</p>";
    long long f = money_put_num(e);
    double h    = money_get_num(f);

    client << "<p>money_put_num|" << f << "</p>";
    client << "<p>money_get_num|" << h << "</p>";

    return "";
}

}// namespace http