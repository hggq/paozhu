#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "testsqltuple.h"

namespace http
{
//@urlpath(null,mtuple)
std::string testsqltuple(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  testsqltuple ";

    try
    {
        auto users = orm::cms::User();

        auto [key_names, key_mapname, lists] = users.query("SELECT * FROM `user` ");
        client << "<p>--------------</p>";
        client << users.error_msg;
        if (key_names.size() > 0)
        {
            for (size_t i = 0; i < lists.size(); i++)
            {
                client << key_names[1] << ":" << lists[i][1];
                client << key_names[2] << ":" << lists[i][2];
                client << key_names[key_mapname["level"]] << ":" << lists[i][key_mapname["level"]];
            }
        }

        client << "<p>--------------</p>";

        users.clear();
        auto [colnames, colnamemaps, vallists] = users.where("name", "admin").limit(1).fetch_row();

        client << "<p>--------------</p>";
        client << users.error_msg;
        if (colnames.size() > 0)
        {
            for (size_t i = 0; i < vallists.size(); i++)
            {
                client << colnames[1] << ":" << vallists[i][1];
                client << colnames[2] << ":" << vallists[i][2];
                client << colnames[colnamemaps["level"]] << ":" << vallists[i][colnamemaps["level"]];
            }
        }
        client << "<p>--------------</p>";
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    return "";
}

}// namespace http