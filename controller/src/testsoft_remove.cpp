#include "orm.h"
#include "httppeer.h"
#include "testsoft_remove.h"
#include "datetime.h"

namespace http
{
//@urlpath(null,testsoftremove)
std::string testsoftremove(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<p> soft_remove </p>";
    auto a          = orm::cms::Testa();
    a.data.value_id = 2;
    a.data.content  = rand_string(4, 0);
    auto[effect_num,last_id] = a.save();
    if (effect_num > 0)
    {
        client << " new id " << a.data.id;
        a.soft_remove();
        if (a.effect() > 0)
        {
            client << " deleted ok ";
        }
        else
        {
            client << " error! ";
        }
        client << " deletetime " << a.data.deletetime;
    }

    return "";
}

}// namespace http