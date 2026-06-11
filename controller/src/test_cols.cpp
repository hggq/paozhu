#include "orm.h" 
#include "httppeer.h"
#include "test_cols.h"
#include "func.h"
#include "world_base.h"
#include <string>
namespace http
{
//@urlpath(null,test_cols)
std::string test_cols(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
     
    unsigned char idx = static_cast<unsigned char>(orm::world::cols::randomnumber);

    client<<"randomnumber idx:"<<std::to_string(idx);

    unsigned char idx_count = orm::world_base::col_names.size();

    for (int i = 0; i < idx_count; ++i) 
    {
        client<<"<p>colname:"<<orm::world_base::col_names[i]<<"</p>";
    }
    return "";
}

}// namespace http