/*
 * 测试 ORM 生成的类: orm::cms::Sysuser (新 ORM, 独立使用)
 * 验证 fetch / fetch_one / count / insert / update / remove 等基本操作
 *
 * 编译:
g++ startup/test_mysql_orm.cpp -std=c++20 -Imodels -Imodels/cms/include models/cms/Sysuser.cpp vendor/httpserver/src/mysql_conn.cpp vendor/httpserver/src/orm_conn_pool.cpp -Ivendor/httpserver/include -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -ldl -Iorm -Iorm/cms -Icommon -Iorm/cms/include -Iorm/include -Imodels/include vendor/httpserver/src/orm_connect_mar.cpp vendor/httpserver/src/pg_conn.cpp vendor/httpserver/src/orm_common.cpp vendor/httpserver/src/func.cpp vendor/httpserver/src/datetime.cpp vendor/httpserver/src/clientdatacache.cpp models/cms/Article.cpp -g -fsanitize=address -Wall
 * 运行: ./a.out
 */
#include <iostream>
#include <string>
#include <vector>
#include <asio.hpp>
#include "orm_conn_pool.h"
#include "orm.h"

static void print_sep(const char *title)
{
    std::cout << "\n========== " << title << " ==========" << std::endl;
}

// ============================================================
// CMS (MySQL) Sysuser ORM 测试
// ============================================================
void test_cms_sysuser_fetch()
{
    print_sep("CMS - Sysuser fetch()");
    auto user = orm::cms::Sysuser();
    user.btAdminid(10000).ltAdminid(10010);
    unsigned int n = user.fetch();
    std::cout << "fetch got " << n << " records" << std::endl;
    for (auto &r : user.record)
    {
        std::cout << "  adminid=" << r.adminid
                  << ", name=" << r.name
                  << ", nickname=" << r.nickname << std::endl;
    }
    if (n == 0 && !user.error_msg.empty())
        std::cerr << "Error: " << user.error_msg << std::endl;
}

void test_cms_sysuser_fetch_one()
{
    print_sep("CMS - Sysuser fetch_one()");
    auto user = orm::cms::Sysuser();
    user.eqName("admin");
    unsigned int n = user.fetch_one();
    std::cout << "fetch_one got " << n << " record" << std::endl;
    if (n > 0)
    {
        std::cout << "  adminid=" << user.data.adminid
                  << ", name=" << user.data.name
                  << ", nickname=" << user.data.nickname << std::endl;
    }
    if (n == 0 && !user.error_msg.empty())
        std::cerr << "Error: " << user.error_msg << std::endl;
}

void test_cms_sysuser_count()
{
    print_sep("CMS - Sysuser count()");
    auto user = orm::cms::Sysuser();
    unsigned int n = user.count();
    std::cout << "count=" << n << std::endl;
}

void test_cms_sysuser_insert_update_remove()
{
    print_sep("CMS - Sysuser insert/update/remove");
    auto user = orm::cms::Sysuser();

    // insert
    orm::cms::sysuser_info::meta new_user;
    new_user.name = "test_orm_model";
    new_user.nickname = "ORM模型测试";
    new_user.password = "test123";
    new_user.textword = "test";
    new_user.companyid = 1;
    new_user.realname = "测试用户";
    new_user.level = 1;
    new_user.isopen = 1;

    auto [ins_ok, ins_id] = user.insert(new_user);
    std::cout << "insert: ok=" << ins_ok << ", id=" << ins_id << std::endl;
    if (ins_ok == 0)
    {
        std::cerr << "Insert error: " << user.error_msg << std::endl;
        return;
    }

    // update
    auto upd_user = orm::cms::Sysuser();
    upd_user.eqAdminid(ins_id);
    upd_user.data.adminid = ins_id;
    upd_user.data.nickname = "ORM已修改";
    upd_user.data.email = "model@test.com";
    int upd_ok = upd_user.update();
    std::cout << "update: ok=" << upd_ok << std::endl;
    std::cout << "sql =" << upd_user.sqlstring << std::endl;
    if (upd_ok == 0)
    {
        std::cerr << "CMS Update error: " << upd_user.error_msg << ", iserror=" << upd_user.iserror << std::endl;
    }

    // verify update
    auto verify = orm::cms::Sysuser();
    verify.eqAdminid(ins_id);
    verify.fetch_one();
    if (verify.effect_num > 0)
    {
        std::cout << "after update: nickname=" << verify.data.nickname
                  << ", email=" << verify.data.email << std::endl;
    }

    // remove
    auto del_user = orm::cms::Sysuser();
    del_user.eqAdminid(ins_id);
    int del_ok = del_user.remove();
    std::cout << "remove: ok=" << del_ok << std::endl;
}

// ============================================================
// main
// ============================================================
int main(int argc, char *argv[])
{
    std::cout << "ORM Models Test (orm::cms::Sysuser)" << std::endl;
    std::cout << "====================================" << std::endl;

    try
    {
        asio::io_context io_context{0};
        std::string configpath = "./conf/orm.conf";

        std::string init_log = orm::init_orm_conn_pool(io_context, configpath);
        if (!init_log.empty())
        {
            std::cout << "Init log:\n" << init_log << std::endl;
        }
        
        test_cms_sysuser_count();
        test_cms_sysuser_fetch();
        test_cms_sysuser_fetch_one();
        test_cms_sysuser_insert_update_remove();

        // 释放连接池
        orm::init_orm_conn_pool_release();
    }
    catch (std::exception &e)
    {
        std::printf("Exception: %s\n", e.what());
    }
    catch (const char *e)
    {
        std::printf("Exception: %s\n", e);
    }

    std::cout << "\n--end--" << std::endl;
    return 0;
}
