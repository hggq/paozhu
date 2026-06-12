#include "orm.h" 
#include "httppeer.h"
#include "test_cols.h"
#include "func.h"
#include <string>
namespace http
{
//@urlpath(null,test_cols)
std::string test_cols(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
     
    unsigned char idx = static_cast<unsigned char>(orm::world_info::cols::randomnumber);

   client<<"randomnumber idx:"<<std::to_string(idx);

    unsigned char idx_count = orm::world_info::col_names.size();

    for (int i = 0; i < idx_count; ++i) 
    {
        client<<"<p>colname:"<<orm::world_info::col_names[i]<<"</p>";
    }

    auto world = orm::World();
    world.btId(200).ltId(250).fetch();
    client << world.sqlstring;
    auto map_all = world.get_cols<orm::world_info::cols::id, orm::world_info::cols::randomnumber>();
    client << "<p>Total size: " << map_all.size() << "</p>";

    auto map_filtered = world.get_cols<orm::world_info::cols::id, orm::world_info::cols::randomnumber>(
        [](const auto& key, const auto& value) -> bool {
            if(key == 0)
            {
                return false;
            }
            return value > 4000; 
        }
    );
    client << "<p>Total size: " << map_filtered.size() << "</p>";

    client << "<p>list: " <<  world.get_cols_to_str<orm::world_info::cols::randomnumber>() << "</p>";
    
    return "";
}

}// namespace http