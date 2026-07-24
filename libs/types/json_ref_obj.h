#ifndef PAOZHU_JSON_TYPES_JSON_REF_OBJ_H
#define PAOZHU_JSON_TYPES_JSON_REF_OBJ_H
#include <string>
#include <vector>
#include <map>

namespace json
{
struct json_ref_obj_recent_orders_t
{
    std::string id;
    double amount = 0;
    std::vector<std::string> items;
    std::string remark;
};

struct json_ref_obj
{
    std::string status;
    long long code = 0;
    std::string api_version;
    struct json_ref_obj_config_t
    {
        bool debug = false;
        double timeout = 0;
        long long retry = 0;
        std::string nullable_field = "null";
    } config;
    struct json_ref_obj_user_t
    {
        long long id = 0;
        struct json_ref_obj_user_name_t
        {
            std::string zh;
            std::string en;
            std::string ar;
        } name;
        std::string bio;
        struct json_ref_obj_user_address_t
        {
            std::string street;
            std::string city;
            struct json_ref_obj_user_address_geo_t
            {
                double lat = 0;
                double lng = 0;
            } geo;
        } address;
        std::vector<std::string> tags;
        struct json_ref_obj_user_settings_t
        {
            bool notifications = false;
            std::string language;
        } settings;
    } user;
    std::vector<json_ref_obj_recent_orders_t> recent_orders;
    struct json_ref_obj_escape_sequence_test_t
    {
        std::string quotes;
        std::string backslash;
        std::string json_boundary;
        std::string unicode_emoji_raw;
    } escape_sequence_test;
    std::string nested_arrays_test = "null";
    struct json_ref_obj_metadata_t
    {
        std::string generated_at;
        std::string source;
        double rnd_seed = 0;
    } metadata;
};

}// namespace json
#endif
