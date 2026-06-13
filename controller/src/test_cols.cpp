#include "orm.h" 
#include "httppeer.h"
#include "test_cols.h"
#include "world_base.h"
#include "func.h"
#include <memory>
#include <string>

ORM_WORLD_DEFINE_STRUCT(Mystruct, id, randomnumber);
ORM_WORLD_TREE_STRUCT(MyTreeStruct, id, randomnumber);
ORM_WORLD_TREE_PTR_STRUCT(MyPtrStruct, id, randomnumber);
ORM_WORLD_SELF_STRUCT(MyCustomStruct, int userid; std::string name;,(userid, name), id, randomnumber);

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
    
    

    orm::world_info::Mystruct b;
    b.id = 10;
    b.randomnumber = 22;

    client << "<p>Mystruct: " << b.to_json() << "</p>";

    
    orm::world_info::MyTreeStruct c,cc;
    c.id = 1;
    c.randomnumber = 33;

    cc.id = b.id ;
    cc.randomnumber = 99;
    c.children.push_back(cc);

    cc.id = b.id + 1;
    cc.randomnumber = 100;

    c.children.push_back(cc);

    client << "<p>MyTreeStruct: " << c.to_json() << "</p>";

    orm::world_info::MyPtrStruct d;
    d.id = 2;
    d.randomnumber = 44;
    
    client << "<p>MyPtrStruct: " << d.to_json() << "</p>";

    orm::world_info::MyCustomStruct cust_a;
    
    cust_a.id = 2;
    cust_a.randomnumber = 44;
    cust_a.userid = 278;
    cust_a.name = "李四";
    
    client << "<p>MyCustomStruct: " << cust_a.to_json() << "</p>";

    struct LocalStruct 
    {
        orm::world_info::type::id id;
        orm::world_info::type::randomnumber randomnumber;
        std::vector<std::unique_ptr<LocalStruct>> children;
    };
    return "";
}

}// namespace http