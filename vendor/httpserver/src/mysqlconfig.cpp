
#include <string>
#include "mysql.h"
#include "mysqlconfig.h"
#include "mysqlpool.h"
#include "serverconfig.h"
#include "func.h"
namespace http
{
// mysqlconfig &getmysqlinfo()
// {
//   static mysqlconfig instance;
//   return instance;
// }
std::vector<mysqlconnect_t> getmysqlconfig(std::string filename)
{
    std::vector<mysqlconnect_t> myconfig;
    FILE *f = fopen(filename.c_str(), "rt");
    if (f == NULL)
    {
        return myconfig;
    }
    fseek(f, 0, SEEK_END);
    auto const size = ftell(f);
    fseek(f, 0, SEEK_SET);

    std::string s, linestr, keyname, strval;
    s.resize(size);

    auto nread = fread(&s[0], 1, size, f);
    s.resize(nread);
    fclose(f);
    struct mysqlconnect_t mysqlconf;
    bool readkey = false;
    bool isvalue = false;

    keyname = "default";
    std::string typeone;
    for (unsigned int i = 0; i < s.size(); i++)
    {
        if (s[i] == ';' || s[i] == '#')
        {
            i++;
            if (linestr.size() > 0)
            {
                //  mysqldbconfig[keyname][linestr]=strval;
                if (linestr == "type")
                {
                    if (typeone.empty())
                    {
                        typeone          = strval;
                        mysqlconf.spname = keyname;
                    }
                    else
                    {

                        myconfig.push_back(mysqlconf);
                        typeone        = strval;
                        mysqlconf.type = strval;
                        mysqlconf.host.clear();
                        mysqlconf.port.clear();
                        mysqlconf.dbname.clear();
                        mysqlconf.user.clear();
                        mysqlconf.password.clear();
                        mysqlconf.pretable.clear();
                        mysqlconf.maxpool.clear();
                        mysqlconf.spname = keyname;
                        mysqlconf.dbtype.clear();
                        mysqlconf.unix_socket.clear();
                    }
                }
            }
            linestr.clear();
            strval.clear();
            isvalue = false;
            for (; i < s.size(); i++)
            {
                if (s[i] == 0x0A)
                {
                    break;
                }
            }
        }
        if (s[i] == 0x0A)
        {
            readkey = false;
            // myconfig[linestr]=strval;
            if (linestr.size() > 0)
            {
                // mysqldbconfig[keyname][linestr]=strval;
                if (str_casecmp(linestr, "type"))
                {
                    if (typeone.empty())
                    {
                        typeone          = strval;
                        mysqlconf.spname = keyname;
                    }
                    else
                    {

                        myconfig.push_back(mysqlconf);
                        typeone        = strval;
                        mysqlconf.type = strval;
                        mysqlconf.host.clear();
                        mysqlconf.port.clear();
                        mysqlconf.dbname.clear();
                        mysqlconf.user.clear();
                        mysqlconf.password.clear();
                        mysqlconf.pretable.clear();
                        mysqlconf.maxpool.clear();
                        mysqlconf.spname = keyname;
                        mysqlconf.unix_socket.clear();
                    }
                }
                if (str_casecmp(linestr, "host"))
                {
                    mysqlconf.host = strval;
                }
                if (str_casecmp(linestr, "port"))
                {
                    mysqlconf.port = strval;
                }
                if (str_casecmp(linestr, "dbname"))
                {
                    mysqlconf.dbname = strval;
                }
                if (str_casecmp(linestr, "user"))
                {

                    mysqlconf.user = strval;
                }
                if (str_casecmp(linestr, "password"))
                {
                    mysqlconf.password = strval;
                }
                if (str_casecmp(linestr, "pretable"))
                {
                    mysqlconf.pretable = strval;
                }
                if (str_casecmp(linestr, "maxpool"))
                {
                    mysqlconf.maxpool = strval;
                }
                if (str_casecmp(linestr, "unix_socket"))
                {
                    mysqlconf.unix_socket = strval;
                }
                if (str_casecmp(linestr, "dbtype"))
                {
                    mysqlconf.dbtype = strval;
                }
            }

            linestr.clear();
            strval.clear();
            isvalue = false;
            continue;
        }
        if (s[i] == '[')
        {
            keyname.clear();
            readkey = true;
            continue;
        }
        if (s[i] == ']')
        {
            readkey = false;
            continue;
        }
        if (s[i] == 0x20)
        {

            continue;
        }
        if (s[i] == '\t')
        {

            continue;
        }
        if (s[i] == '"')
        {

            continue;
        }
        if (s[i] == '=')
        {
            isvalue = true;
            continue;
        }
        if (readkey)
        {
            keyname.push_back(s[i]);
        }
        else
        {
            if (isvalue)
            {
                strval.push_back(s[i]);
            }
            else
            {
                linestr.push_back(s[i]);
            }
        }
    }
    if (mysqlconf.host.size() > 0)
    {
        if (str_casecmp(linestr, "host"))
        {
            mysqlconf.host = strval;
        }
        if (str_casecmp(linestr, "port"))
        {
            mysqlconf.port = strval;
        }
        if (str_casecmp(linestr, "dbname"))
        {
            mysqlconf.dbname = strval;
        }
        if (str_casecmp(linestr, "user"))
        {
            mysqlconf.user = strval;
        }
        if (str_casecmp(linestr, "password"))
        {
            mysqlconf.password = strval;
        }
        if (str_casecmp(linestr, "pretable"))
        {
            mysqlconf.pretable = strval;
        }
        if (str_casecmp(linestr, "maxpool"))
        {
            mysqlconf.maxpool = strval;
        }
        if (str_casecmp(linestr, "unix_socket"))
        {
            mysqlconf.unix_socket = strval;
        }
        if (str_casecmp(linestr, "dbtype"))
        {
            mysqlconf.dbtype = strval;
        }
        mysqlconf.spname = keyname;
        myconfig.push_back(mysqlconf);
    }

    return myconfig;
}
void mysqlconfig_init_link()
{
    try
    {
        std::string mysqlconnect;      // = "mysqlx://root:123456@127.0.0.1/mysql";
        std::string mysqlconnectselect;// = "mysqlx://root:123456@127.0.0.1/mysql";
        std::string rmstag;
        std::hash<std::string> hash_fn;
        size_t dcon;
        serverconfig &sysconfigpath = getserversysconfig();

        rmstag = sysconfigpath.configpath;

        if (rmstag.size() > 0 && rmstag.back() != '/')
        {
            rmstag.push_back('/');
        }
        rmstag.append("orm.conf");
        std::vector<mysqlconnect_t> myconfig = getmysqlconfig(rmstag);
        std::map<std::string, std::vector<struct mysql_connect_link_info>> mysqldblinkgroupjion;
        std::map<size_t, mysqllinkpool> &mysql_pool_map = get_mysqlpool();
        std::map<std::string, std::string> poolmax;
        rmstag.clear();
        // 转为rmstag分组
        if (myconfig.size() > 0)
        {
            for (std::size_t li = 0; li < myconfig.size(); li++)
            {

                if (myconfig[li].dbtype != "mysql")
                {
                    continue;
                }

                rmstag = myconfig[li].port;
                if (rmstag.size() < 3 || rmstag.size() > 6)
                {
                    rmstag = "3306";
                }

                struct mysql_connect_link_info temp;
                temp.host     = myconfig[li].host;
                temp.username = myconfig[li].user;
                temp.password = myconfig[li].password;
                temp.db       = myconfig[li].dbname;
                try
                {
                    temp.port = std::atoi(rmstag.c_str());
                }
                catch (const std::exception &e)
                {
                    temp.port = 0;
                }
                temp.unix_socket = myconfig[li].unix_socket;
                rmstag           = myconfig[li].spname;

                poolmax[rmstag] = myconfig[li].maxpool;

                mysqldblinkgroupjion[rmstag].emplace_back(temp);
            }
        }
        for (auto iterl = mysqldblinkgroupjion.begin(); iterl != mysqldblinkgroupjion.end(); iterl++)
        {
            dcon = hash_fn(iterl->first);
            if (iterl->second.size() == 1)
            {
                mysqllinkpool db(iterl->second[0], iterl->second[0]);

                try
                {
                    db.select_max_pool_num = std::atoi(poolmax[rmstag].c_str());
                    db.edit_max_pool_num   = std::atoi(poolmax[rmstag].c_str());

                    if (db.select_max_pool_num < 10)
                    {
                        db.select_max_pool_num = 10;
                    }
                    if (db.edit_max_pool_num < 10)
                    {
                        db.edit_max_pool_num = 10;
                    }
                }
                catch (const std::exception &e)
                {
                }
                mysql_pool_map.insert({dcon, std::move(db)});
            }
            else if (iterl->second.size() > 1)
            {
                mysqllinkpool db(iterl->second[0], iterl->second[1]);
                try
                {
                    db.select_max_pool_num = std::atoi(poolmax[rmstag].c_str());
                    db.edit_max_pool_num   = std::atoi(poolmax[rmstag].c_str());

                    if (db.select_max_pool_num < 10)
                    {
                        db.select_max_pool_num = 10;
                    }
                    if (db.edit_max_pool_num < 10)
                    {
                        db.edit_max_pool_num = 10;
                    }
                }
                catch (const std::exception &e)
                {
                }
                mysql_pool_map.insert({dcon, std::move(db)});
            }
        }
    }
    catch (const char *e)
    {
        throw e;
    }
    catch (...)
    {
        //std::cout << "ERROR: " << err << ::std::endl;
    }
}
}// namespace http