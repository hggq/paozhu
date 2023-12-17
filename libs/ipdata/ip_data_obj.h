#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<utility>

namespace http
{
struct ipdata_area_t
{
    unsigned int a;
    unsigned int b;
    unsigned short c;  
};

class ip_data_obj
{
    public:
        void init();
        unsigned int ip2long(const std::string &ip);
        std::string search(const std::string &ip,bool iscity=false);
        void dichotomy_search(unsigned int a,unsigned int b);
    public:
    unsigned int ip;
    unsigned short result;
    std::vector<std::string> citylists;
    std::vector<std::string> provincelists;
    std::vector<std::string> countrylists;
    std::vector<ipdata_area_t> ipdatalist;
};

ip_data_obj &get_ip_data_obj();
}