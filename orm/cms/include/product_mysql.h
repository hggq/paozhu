#ifndef _ORM_CMS_PRODUCT_OPERATE_H
#define _ORM_CMS_PRODUCT_OPERATE_H

#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include <set>
#include <string_view>
#include <thread>
#include "request.h"
#include "unicode.h"
#include "datetime.h"
#include <stdexcept>
#include <iostream>
#include <functional>
#include <tuple>
#include <typeinfo>
#include <memory>
#include <list>
#include <queue>
#include <cmath>
#include <condition_variable>
#include <sstream>
#include <algorithm>
#include <vector>

#include "mysql_conn.h"
#include "mysql_conn_pool.h"
#include "orm_cache.hpp"
/*baseincludefile*/
namespace orm
{
// mysql Operational SQL middleware
namespace cms
{ /*tagnamespace_replace*/
    template <typename M_MODEL, typename B_BASE>
    class product_mysql : public B_BASE
    {
      public:
        product_mysql(const std::string &tag) : dbtag(tag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                conn_obj = iter->second;
            }
            else
            {
                conn_obj  = nullptr;
                iserror   = true;
                error_msg = "conn_pool not found " + dbtag;
            }
        }
        product_mysql() : dbtag(B_BASE::_rmstag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                conn_obj = iter->second;
            }
            else
            {
                conn_obj  = nullptr;
                iserror   = true;
                error_msg = "conn_pool not found " + dbtag;
            }
        }
        M_MODEL &switchDB(const std::string &temptag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(temptag);
            if (iter != conn_pool_obj.end())
            {
                conn_obj = iter->second;
            }
            else
            {
                conn_obj  = nullptr;
                iserror   = true;
                error_msg = "conn_pool not found " + temptag;
            }
        }
        M_MODEL &set_table(const std::string &table_name)
        {
            if (original_tablename.empty())
            {
                original_tablename = B_BASE::tablename;
            }
            if (table_name.size() > 0)
            {
                B_BASE::tablename = table_name;
            }
            return *mod;
        }
        M_MODEL &reset_table()
        {
            if (original_tablename.empty())
            {
                return *mod;
            }
            B_BASE::tablename = original_tablename;
            return *mod;
        }
        unsigned int count()
        {
            std::string countsql;
            countsql = "SELECT count(*) as total_countnum  FROM ";
            countsql.append(B_BASE::tablename);
            countsql.append(" WHERE ");
            if (wheresql.empty())
            {
                countsql.append(" 1 ");
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = countsql.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(countsql);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);

                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                //std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                querysql_len = 0;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int tempnum = 0;

                                unsigned int name_length = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                querysql_len = 0;
                                for (unsigned int ik = 0; ik < name_length; ik++)
                                {
                                    if (temp_pack_data.data[tempnum] >= '0' && temp_pack_data.data[tempnum] <= '9')
                                    {
                                        querysql_len = querysql_len * 10 + (temp_pack_data.data[tempnum] - '0');
                                    }
                                    tempnum++;
                                }
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }

                conn_obj->back_select_conn(conn);

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return querysql_len;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                return 0;
            }

            return 0;
        }
        std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>
        page(unsigned int page, unsigned int per_page = 10, unsigned int list_num = 5)
        {
            unsigned int total_page = count();
            if (per_page == 0)
            {
                per_page = 10;
            }
            if (list_num < 1)
            {
                list_num = 1;
            }
            total_page = std::ceil((float)total_page / per_page);

            if (total_page < 1)
            {
                total_page = 1;
            }
            if (page > total_page)
            {
                page = total_page;
            }
            if (page < 1)
            {
                page = 1;
            }
            unsigned int mid_num  = std::floor(list_num / 2);
            unsigned int last_num = list_num - 1;

            int temp_num = page - mid_num;

            unsigned int minpage = temp_num < 1 ? 1 : temp_num;
            unsigned int maxpage = minpage + last_num;

            if (maxpage > total_page)
            {
                maxpage  = total_page;
                temp_num = (maxpage - last_num);
                if (temp_num < 1)
                {
                    minpage = 1;
                }
                else
                {
                    minpage = temp_num;
                }
            }
            limit((page - 1) * per_page, per_page);
            return std::make_tuple(minpage, maxpage, page, total_page);
        }
        asio::awaitable<unsigned int> async_count()
        {
            std::string countsql;
            countsql = "SELECT count(*) as total_countnum  FROM ";
            countsql.append(B_BASE::tablename);
            countsql.append(" WHERE ");
            if (wheresql.empty())
            {
                countsql.append(" 1 ");
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_select_conn();

                unsigned int querysql_len = countsql.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(countsql);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                //std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                querysql_len = 0;

                for (; is_sql_item == false;)
                {
                    n      = co_await conn->async_read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int tempnum = 0;

                                unsigned int name_length = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                querysql_len = 0;
                                for (unsigned int ik = 0; ik < name_length; ik++)
                                {
                                    if (temp_pack_data.data[tempnum] >= '0' && temp_pack_data.data[tempnum] <= '9')
                                    {
                                        querysql_len = querysql_len * 10 + (temp_pack_data.data[tempnum] - '0');
                                    }
                                    tempnum++;
                                }
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }

                conn_obj->back_select_conn(conn);

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return querysql_len;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                co_return 0;
            }
            co_return 0;
        }

        asio::awaitable<std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>>
        async_page(unsigned int page, unsigned int per_page = 10, unsigned int list_num = 5)
        {
            unsigned int total_page = async_count();
            if (per_page == 0)
            {
                per_page = 10;
            }
            if (list_num < 1)
            {
                list_num = 1;
            }
            total_page = std::ceil((float)total_page / per_page);

            if (total_page < 1)
            {
                total_page = 1;
            }
            if (page > total_page)
            {
                page = total_page;
            }
            if (page < 1)
            {
                page = 1;
            }
            unsigned int mid_num  = std::floor(list_num / 2);
            unsigned int last_num = list_num - 1;

            int temp_num = page - mid_num;

            unsigned int minpage = temp_num < 1 ? 1 : temp_num;
            unsigned int maxpage = minpage + last_num;

            if (maxpage > total_page)
            {
                maxpage  = total_page;
                temp_num = (maxpage - last_num);
                if (temp_num < 1)
                {
                    minpage = 1;
                }
                else
                {
                    minpage = temp_num;
                }
            }
            limit((page - 1) * per_page, per_page);
            co_return std::make_tuple(minpage, maxpage, page, total_page);
        }

        unsigned int update_col(std::string colname, int num, char symbol = '+')
        {
            effect_num = 0;
            std::string countsql;
            countsql = "UPDATE ";
            countsql.append(B_BASE::tablename);
            countsql.append(" SET ");
            countsql.append(colname);
            if (num > 0)
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.push_back(' ');
                countsql.push_back(symbol);
                countsql.append(std::to_string(num));
            }
            else
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.push_back(' ');
                countsql.push_back(symbol);
                countsql.push_back('(');
                countsql.push_back('-');
                countsql.append(std::to_string(std::abs(num)));
                countsql.push_back(')');
            }
            countsql.append(" where ");
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    countsql.append(tempsql.str());
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
            }
            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = countsql.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(countsql);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    //insertid      = pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }

                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {

            }
            return 0;
        }

        asio::awaitable<unsigned int> async_update_col(std::string colname, int num, char symbol = '+')
        {
            effect_num = 0;
            std::string countsql;
            countsql = "UPDATE ";
            countsql.append(B_BASE::tablename);
            countsql.append(" SET ");
            countsql.append(colname);
            if (num > 0)
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.push_back(' ');
                countsql.push_back(symbol);
                countsql.append(std::to_string(num));
            }
            else
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.push_back(' ');
                countsql.push_back(symbol);
                countsql.push_back('(');
                countsql.push_back('-');
                countsql.append(std::to_string(std::abs(num)));
                countsql.push_back(')');
            }
            countsql.append(" where ");
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    countsql.append(tempsql.str());
                }
                else
                {
                    co_return 0;
                }
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
            }
            if (iscommit)
            {
                iscommit = false;
                co_return 0;
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {

                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_edit_conn();

                unsigned int querysql_len = countsql.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(countsql);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                unsigned int offset = 0;
                n                   = co_await conn->async_read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    //insertid      = pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    co_return effect_num;
                }

                co_return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            co_return 0;
        }

        int replace_col(std::string colname, const std::string &old_string, const std::string &new_string)
        {
            effect_num = 0;
            std::string countsql;
            countsql = "UPDATE ";
            countsql.append(B_BASE::tablename);
            countsql.append(" SET ");
            countsql.append(colname);

            countsql.append(" = REPLACE(");
            countsql.append(colname);
            countsql.append(",'");
            countsql.append(old_string);
            countsql.append("','");
            countsql.append(new_string);
            countsql.append("') ");

            countsql.append(" where ");
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    countsql.append(tempsql.str());
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
            }
            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = countsql.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(countsql);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    //insertid      = pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }

                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            return 0;
        }

        void assign_field_value(unsigned char index_pos, unsigned char *result_temp_data, unsigned int value_size, typename B_BASE::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0:
            data_temp.pid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.pid= data_temp.pid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 1:
            data_temp.userid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.userid= data_temp.userid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 2:
            data_temp.topicid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.topicid= data_temp.topicid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 3:
            data_temp.bigid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.bigid= data_temp.bigid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 4:
            data_temp.smallid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.smallid= data_temp.smallid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 5:
            data_temp.brandid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.brandid= data_temp.brandid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 6:
            data_temp.isview=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.isview= data_temp.isview * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 7:
            data_temp.isstore=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.isstore= data_temp.isstore * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 8:
            data_temp.ishome=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.ishome= data_temp.ishome * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 9:
            data_temp.showtype=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.showtype= data_temp.showtype * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 10:
            data_temp.sntype.clear();
            data_temp.sntype.resize(value_size);
            
            std::memcpy(data_temp.sntype.data(), result_temp_data, value_size);
            break;
                case 11:
            data_temp.name.clear();
            data_temp.name.resize(value_size);
            
            std::memcpy(data_temp.name.data(), result_temp_data, value_size);
            break;
                case 12:
            data_temp.keywords.clear();
            data_temp.keywords.resize(value_size);
            
            std::memcpy(data_temp.keywords.data(), result_temp_data, value_size);
            break;
                case 13:
            data_temp.introduce.clear();
            data_temp.introduce.resize(value_size);
            
            std::memcpy(data_temp.introduce.data(), result_temp_data, value_size);
            break;
                case 14:
            data_temp.listimg.clear();
            data_temp.listimg.resize(value_size);
            
            std::memcpy(data_temp.listimg.data(), result_temp_data, value_size);
            break;
                case 15:
            data_temp.bigimg.clear();
            data_temp.bigimg.resize(value_size);
            
            std::memcpy(data_temp.bigimg.data(), result_temp_data, value_size);
            break;
                case 16:
            data_temp.maincontent.clear();
            data_temp.maincontent.resize(value_size);
            
            std::memcpy(data_temp.maincontent.data(), result_temp_data, value_size);
            break;
                case 17:
            data_temp.paracontent.clear();
            data_temp.paracontent.resize(value_size);
            
            std::memcpy(data_temp.paracontent.data(), result_temp_data, value_size);
            break;
                case 18:
            data_temp.samepro.clear();
            data_temp.samepro.resize(value_size);
            
            std::memcpy(data_temp.samepro.data(), result_temp_data, value_size);
            break;
                case 19:
            data_temp.attatchfiles.clear();
            data_temp.attatchfiles.resize(value_size);
            
            std::memcpy(data_temp.attatchfiles.data(), result_temp_data, value_size);
            break;
                case 20:
            data_temp.price=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.price= data_temp.price * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 21:
            data_temp.sortid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.sortid= data_temp.sortid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 22:
            data_temp.adddate.clear();
            data_temp.adddate.resize(value_size);
            
            std::memcpy(data_temp.adddate.data(), result_temp_data, value_size);
            break;
                case 23:
            data_temp.editdate.clear();
            data_temp.editdate.resize(value_size);
            
            std::memcpy(data_temp.editdate.data(), result_temp_data, value_size);
            break;
                
        }
    }
    

M_MODEL& eqPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inPid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inPid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninPid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninPid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& bePid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& lePid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inPid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inPid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninPid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninPid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_bePid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltPid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_lePid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqPid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqPid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btPid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& bePid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltPid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& lePid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqPid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqPid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btPid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_bePid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltPid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_lePid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" pid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inUserid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inUserid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninUserid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninUserid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inUserid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inUserid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninUserid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninUserid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leUserid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leUserid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" userid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inTopicid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inTopicid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninTopicid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninTopicid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inTopicid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inTopicid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninTopicid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninTopicid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leTopicid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leTopicid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topicid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inBigid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inBigid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninBigid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninBigid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inBigid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inBigid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninBigid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninBigid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leBigid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leBigid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inSmallid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inSmallid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninSmallid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninSmallid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inSmallid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inSmallid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninSmallid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninSmallid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leSmallid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leSmallid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" smallid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inBrandid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inBrandid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninBrandid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninBrandid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inBrandid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inBrandid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninBrandid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninBrandid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leBrandid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leBrandid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" brandid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inIsview(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inIsview(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninIsview(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninIsview(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inIsview(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inIsview(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninIsview(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninIsview(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leIsview(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leIsview(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isview <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inIsstore(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inIsstore(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninIsstore(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninIsstore(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inIsstore(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inIsstore(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninIsstore(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninIsstore(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leIsstore(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leIsstore(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isstore <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inIshome(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inIshome(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninIshome(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninIshome(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inIshome(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inIshome(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninIshome(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninIshome(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leIshome(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leIshome(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" ishome <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inShowtype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inShowtype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninShowtype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninShowtype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inShowtype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inShowtype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninShowtype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninShowtype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leShowtype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leShowtype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" showtype <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& nullSntype()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullSntype()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inSntype(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inSntype(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninSntype(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninSntype(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullSntype()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullSntype()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inSntype(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inSntype(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninSntype(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninSntype(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leSntype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leSntype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sntype <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullName()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullName()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inName(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inName(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninName(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninName(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullName()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullName()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inName(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inName(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninName(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninName(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leName(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leName(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" name <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullKeywords()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullKeywords()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inKeywords(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inKeywords(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninKeywords(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninKeywords(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullKeywords()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullKeywords()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inKeywords(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inKeywords(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninKeywords(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninKeywords(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leKeywords(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leKeywords(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" keywords <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullIntroduce()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullIntroduce()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inIntroduce(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inIntroduce(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninIntroduce(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninIntroduce(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullIntroduce()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullIntroduce()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inIntroduce(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inIntroduce(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninIntroduce(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninIntroduce(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leIntroduce(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leIntroduce(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" introduce <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullListimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullListimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inListimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inListimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninListimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninListimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullListimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullListimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inListimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inListimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninListimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninListimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leListimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leListimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" listimg <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullBigimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullBigimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inBigimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inBigimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninBigimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninBigimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullBigimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullBigimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inBigimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inBigimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninBigimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninBigimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leBigimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leBigimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" bigimg <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullMaincontent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullMaincontent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inMaincontent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inMaincontent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninMaincontent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninMaincontent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullMaincontent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullMaincontent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inMaincontent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inMaincontent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninMaincontent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninMaincontent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leMaincontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leMaincontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" maincontent <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullParacontent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullParacontent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inParacontent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inParacontent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninParacontent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninParacontent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullParacontent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullParacontent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inParacontent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inParacontent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninParacontent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninParacontent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leParacontent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leParacontent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" paracontent <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullSamepro()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullSamepro()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inSamepro(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inSamepro(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninSamepro(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninSamepro(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullSamepro()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullSamepro()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inSamepro(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inSamepro(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninSamepro(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninSamepro(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leSamepro(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leSamepro(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" samepro <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullAttatchfiles()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullAttatchfiles()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inAttatchfiles(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inAttatchfiles(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninAttatchfiles(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninAttatchfiles(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullAttatchfiles()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullAttatchfiles()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inAttatchfiles(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inAttatchfiles(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninAttatchfiles(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninAttatchfiles(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leAttatchfiles(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leAttatchfiles(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" attatchfiles <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& eqPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inPrice(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inPrice(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninPrice(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninPrice(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& bePrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& lePrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inPrice(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inPrice(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninPrice(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninPrice(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_bePrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltPrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_lePrice(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqPrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqPrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btPrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& bePrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltPrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& lePrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqPrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqPrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btPrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_bePrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltPrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_lePrice(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" price <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inSortid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inSortid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninSortid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninSortid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inSortid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inSortid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninSortid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninSortid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leSortid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leSortid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sortid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& nullAdddate()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullAdddate()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inAdddate(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inAdddate(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninAdddate(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninAdddate(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullAdddate()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullAdddate()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inAdddate(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inAdddate(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninAdddate(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninAdddate(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leAdddate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leAdddate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" adddate <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullEditdate()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullEditdate()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inEditdate(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inEditdate(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninEditdate(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninEditdate(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullEditdate()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullEditdate()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inEditdate(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inEditdate(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninEditdate(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninEditdate(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leEditdate(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leEditdate(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" editdate <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

        M_MODEL &select(const std::string &fieldname)
        {
            if (selectsql.size() > 0)
            {
                selectsql.push_back(',');
            }
            selectsql.append(fieldname);
            return *mod;
        }

        M_MODEL &where(const std::string &wq)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(wq);
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &where(const std::string &wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            std::stringstream _stream;
            _stream << val;
            wheresql.append(_stream.str());
            return *mod;
        }

        M_MODEL &where(const std::string &wq, char bi, http::OBJ_VALUE &obj)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back(bi);
            if (obj.is_string())
            {
                wheresql.push_back('\'');
                wheresql.append(obj.as_string());
                wheresql.push_back('\'');
            }
            else
            {

                wheresql.append(obj.to_string());
            }
            return *mod;
        }
        M_MODEL &where(const std::string &wq, http::OBJ_VALUE &obj)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            if (obj.is_string())
            {
                wheresql.push_back('\'');
                wheresql.append(obj.as_string());
                wheresql.push_back('\'');
            }
            else
            {

                wheresql.append(obj.to_string());
            }
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &where(const std::string &wq, char bi, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back(bi);
            std::stringstream _stream;
            _stream << val;
            wheresql.append(_stream.str());
            return *mod;
        }

        M_MODEL &where(const std::string &wq, char bi, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back(bi);
            wheresql.push_back('\'');

            wheresql.append(val);
            wheresql.push_back('\'');
            return *mod;
        }
        M_MODEL &where(const std::string &wq, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            wheresql.push_back('\'');
            wheresql.append(val);
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &between(const std::string &wq, const std::string &a, const std::string &b)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(" (");
            wheresql.append(wq);
            wheresql.append(" BETWEEN '");
            std::stringstream _stream;
            _stream << a;
            _stream << "' AND '";
            _stream << b;
            _stream << "' ) ";
            wheresql.append(_stream.str());
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &between(const std::string &wq, _SQL_Value a, _SQL_Value b)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(" (");
            wheresql.append(wq);
            wheresql.append(" BETWEEN ");
            std::stringstream _stream;
            _stream << a;
            _stream << " AND ";
            _stream << b;
            _stream << " ) ";
            wheresql.append(_stream.str());
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &orBetween(const std::string &wq, _SQL_Value a, _SQL_Value b)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(" (");
            wheresql.append(wq);
            wheresql.append(" BETWEEN ");
            std::stringstream _stream;
            _stream << a;
            _stream << " AND ";
            _stream << b;
            _stream << " ) ";
            wheresql.append(_stream.str());
            return *mod;
        }
        M_MODEL &whereLike(const std::string &wq, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" like '");
            if (val.size() > 0 && (val[0] == '%' || val.back() == '%'))
            {
                wheresql.append(val);
                wheresql.append("' ");
            }
            else
            {
                wheresql.push_back('%');
                wheresql.append(val);
                wheresql.append("%' ");
            }
            return *mod;
        }
        M_MODEL &whereLikeLeft(const std::string &wq, const std::string &val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" like '");
            wheresql.push_back('%');
            wheresql.append(val);
            wheresql.append("' ");
            return *mod;
        }
        M_MODEL &whereLikeRight(const std::string &wq, const std::string &val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" like '");
            wheresql.append(val);
            wheresql.append("%' ");
            return *mod;
        }
        M_MODEL &whereOrLike(const std::string &wq, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" like '");
            if (val[0] == '%' || val.back() == '%')
            {
                wheresql.append(val);
                wheresql.append("' ");
            }
            else
            {
                wheresql.push_back('%');
                wheresql.append(val);
                wheresql.append("%' ");
            }
            return *mod;
        }
        M_MODEL &whereAnd(const std::string &wq)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereAnd(const std::string &wq, _SQL_Value val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }
            std::stringstream _stream;
            _stream << val;
            wheresql.append(_stream.str());
            return *mod;
        }
        M_MODEL &whereAnd(const std::string &wq, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            wheresql.push_back('\'');
            wheresql.append(val);
            wheresql.push_back('\'');

            return *mod;
        }
        M_MODEL &whereOr(const std::string &wq)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereOr(const std::string &wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }
            std::stringstream _stream;
            _stream << val;
            wheresql.append(_stream.str());
            return *mod;
        }
        M_MODEL &whereOr(const std::string &wq, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            wheresql.push_back('\'');
            wheresql.append(val);
            wheresql.push_back('\'');
            return *mod;
        }
        M_MODEL &whereIn(const std::string &k)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(k);
            return *mod;
        }
        M_MODEL &whereIn(const std::string &k, const std::string &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" IN(");
            wheresql.append(a);
            wheresql.append(") ");
            return *mod;
        }

        M_MODEL &whereIn(const std::string &k, const std::vector<std::string> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",\'");
                }
                else
                {
                    wheresql.append("\'");
                }
                wheresql.append(key);
                wheresql.append("\'");
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        M_MODEL &whereNotIn(const std::string &k, const std::vector<std::string> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",\'");
                }
                else
                {
                    wheresql.append("\'");
                }
                wheresql.append(key);
                wheresql.append("\'");
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereIn(const std::string &k, const std::list<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            std::stringstream _stream;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                _stream << key;
                wheresql.append(_stream.str());
                i++;
                _stream.str("");
            }
            wheresql.append(") ");
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereIn(const std::string &k, const std::vector<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" IN(");
            int i = 0;
            std::stringstream _stream;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                _stream << key;
                wheresql.append(_stream.str());
                i++;
                _stream.str("");
            }
            wheresql.append(") ");
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereNotIn(const std::string &k, const std::vector<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            std::stringstream _stream;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                _stream << key;
                wheresql.append(_stream.str());
                i++;
                _stream.str("");
            }
            wheresql.append(") ");
            return *mod;
        }

        M_MODEL &order(const std::string &wq)
        {
            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            return *mod;
        }
        M_MODEL &asc(const std::string &wq)
        {

            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            ordersql.append(" ASC ");
            return *mod;
        }

        M_MODEL &desc(const std::string &wq)
        {

            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            ordersql.append(" DESC ");
            return *mod;
        }

        M_MODEL &having(const std::string &wq)
        {

            groupsql.append(" HAVING by ");
            groupsql.append(wq);
            return *mod;
        }

        M_MODEL &group(const std::string &wq)
        {

            groupsql.append(" GROUP BY ");
            groupsql.append(wq);
            return *mod;
        }

        M_MODEL &orsub()
        {

            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            else
            {
                wheresql.append(" OR (");
                iskuohao     = true;
                ishascontent = false;
            }
            return *mod;
        }
        M_MODEL &andsub()
        {

            if (iskuohao == true)
            {
                iskuohao = false;
                wheresql.append(" )");
                ishascontent = false;
            }
            else
            {
                wheresql.append(" AND (");
                iskuohao     = true;
                ishascontent = false;
            }

            return *mod;
        }

        M_MODEL &endsub()
        {
            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            return *mod;
        }

        M_MODEL &or_b()
        {

            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            else
            {
                wheresql.append(" OR (");
                iskuohao     = true;
                ishascontent = false;
            }
            return *mod;
        }
        M_MODEL &and_b()
        {

            if (iskuohao == true)
            {
                iskuohao = false;
                wheresql.append(" )");
                ishascontent = false;
            }
            else
            {
                wheresql.append(" AND (");
                iskuohao     = true;
                ishascontent = false;
            }

            return *mod;
        }

        M_MODEL &or_e()
        {
            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            return *mod;
        }

        M_MODEL &and_e()
        {
            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            return *mod;
        }

        M_MODEL &limit(unsigned int num)
        {
            limitsql.clear();
            limitsql.append(" limit ");
            limitsql.append(std::to_string(num));
            return *mod;
        }
        M_MODEL &limit(unsigned int num, unsigned int endnum)
        {
            limitsql.clear();
            limitsql.append(" limit ");
            limitsql.append(std::to_string(num));
            limitsql.push_back(',');
            limitsql.append(std::to_string(endnum));
            return *mod;
        }

        std::vector<std::map<std::string, std::string>> fetch_obj()
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            std::vector<std::map<std::string, std::string>> temprecord;

            if (iserror)
            {
                return temprecord;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    return temprecord;
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return temprecord;
                }

                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::map<std::string, std::string> data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    std::string temp_str;
                                    temp_str.resize(name_length);
                                    std::memcpy(temp_str.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        data_temp.insert({field_array[ij].name, std::move(temp_str)});
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        data_temp.insert({field_array[ij].org_name, std::move(temp_str)});
                                    }

                                    tempnum = tempnum + name_length;
                                }
                                temprecord.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }

                conn_obj->back_select_conn(conn);

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return temprecord;
        }
        std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>
        fetch_row()
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            std::vector<std::vector<std::string>> temprecord;
            std::vector<std::string> table_fieldname;
            std::map<std::string, unsigned int> table_fieldmap;

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);

                model_meta_cache<std::vector<std::string>> &temp_cache =
                    model_meta_cache<std::vector<std::string>>::getinstance();
                temprecord = temp_cache.get(sqlhashid);
                if (temprecord.size() > 0)
                {
                    iscache                                    = false;
                    model_meta_cache<std::string> &table_cache = model_meta_cache<std::string>::getinstance();
                    table_fieldname                            = table_cache.get(sqlhashid);

                    model_meta_cache<std::map<std::string, unsigned int>> &tablemap_cache =
                        model_meta_cache<std::map<std::string, unsigned int>>::getinstance();
                    table_fieldmap = tablemap_cache.get_obj(sqlhashid);

                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
            }

            if (iserror)
            {
                return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }
 
            try
            {
                if (conn_empty())
                {
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                // std::vector<std::vector<std::string>> field_value;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                        table_fieldmap.insert({field_array[ii].org_name,table_fieldname.size()});
                                        table_fieldname.push_back(field_array[ii].org_name);
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::vector<std::string> temp_v_record; 
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    std::string tempstr;
                                    tempstr.resize(name_length);
                                    std::memcpy(tempstr.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    temp_v_record.push_back(std::move(tempstr));
                                    tempnum = tempnum + name_length;
                                }
                                temprecord.push_back(temp_v_record);
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);

                

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        if (temprecord.size() > 0)
                        {
                            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);

                            model_meta_cache<std::vector<std::string>> &temp_cache =
                                model_meta_cache<std::vector<std::string>>::getinstance();
                            temp_cache.save(sqlhashid, temprecord, exptime);

                            exptime += 1;
                            model_meta_cache<std::string> &table_cache = model_meta_cache<std::string>::getinstance();
                            table_cache.save(sqlhashid, table_fieldname, exptime);

                            model_meta_cache<std::map<std::string, unsigned int>> &tablemap_cache =
                                model_meta_cache<std::map<std::string, unsigned int>>::getinstance();
                            tablemap_cache.save(sqlhashid, table_fieldmap, exptime);
                            exptime = 0;
                            iscache = false;
                        }
                    }
                }

                return std::make_tuple(std::move(table_fieldname), std::move(table_fieldmap), std::move(temprecord));
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }
        M_MODEL &fetch()
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_cacherecord(sqlhashid))
                {
                    iscache = false;
                    return *mod;
                }
            }

            B_BASE::record_reset();
            if (iserror)
            {
                return *mod;
            }

            try
            {
                if (conn_empty())
                {
                    return *mod;
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return *mod;
                }
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                typename B_BASE::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                    tempnum = tempnum + name_length;
                                }
                                B_BASE::record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return *mod;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return *mod;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return *mod;
        }

        asio::awaitable<unsigned int> async_fetch()
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_cacherecord(sqlhashid))
                {
                    iscache = false;
                    co_return 0;
                }
            }

            B_BASE::record_reset();
            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = co_await conn->async_read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                typename B_BASE::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                    tempnum = tempnum + name_length;
                                }
                                B_BASE::record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return 0;
        }
        M_MODEL &fetch_append()
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_cacherecord(sqlhashid))
                {
                    iscache = false;
                    return *mod;
                }
            }

            if (iserror)
            {
                return *mod;
            }
 
            try
            {
 
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                typename B_BASE::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                    tempnum = tempnum + name_length;
                                }
                                B_BASE::record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);
                
                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return *mod;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return *mod;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return *mod;
        }

        asio::awaitable<unsigned int> async_fetch_append()
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_cacherecord(sqlhashid))
                {
                    iscache = false;
                    co_return 1;
                }
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                effect_num = 0;

                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                // asio::error_code ec;

                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                // std::vector<std::vector<std::string>> field_value;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;
                // std::map<unsigned char, std::string> other_col;

                for (; is_sql_item == false;)
                {
                    // std::memset(result_data, 0x00, 4096);
                    // n      = co_await conn->socket->async_read_some(asio::buffer(result_data), asio::use_awaitable);
                    n      = co_await conn->async_read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                typename B_BASE::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                    tempnum = tempnum + name_length;
                                }
                                effect_num++;
                                B_BASE::record.emplace_back(std::move(data_temp));
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return 0;
        }
        unsigned int fetch_one(bool isappend = false)
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }

            sqlstring.append(" limit 1");

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_cacherecord(sqlhashid))
                {
                    iscache = false;
                    return 0;
                }
            }

            B_BASE::data_reset();
            if (iserror)
            {
                return 0;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                if (isappend)
                                {
                                    typename B_BASE::meta data_temp;
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                        assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                        tempnum = tempnum + name_length;
                                    }
                                    B_BASE::record.emplace_back(std::move(data_temp));
                                    effect_num++;
                                }
                                else
                                {
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                        assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, B_BASE::data);
                                        tempnum = tempnum + name_length;
                                    }
                                    effect_num++;
                                }
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }

                conn_obj->back_select_conn(conn);

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return 0;
        }

        asio::awaitable<unsigned int> async_fetch_one(bool isappend = false)
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }

            sqlstring.append(" limit 1");

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_cacherecord(sqlhashid))
                {
                    iscache = false;
                    co_return 0;
                }
            }

            B_BASE::data_reset();
            if (iserror)
            {
                co_return 0;
            }

            try
            {
                effect_num = 0;

                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                // std::vector<std::vector<std::string>> field_value;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;
                // std::map<unsigned char, std::string> other_col;

                for (; is_sql_item == false;)
                {
                    n      = co_await conn->async_read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                if (isappend)
                                {
                                    typename B_BASE::meta data_temp;
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                        assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                        tempnum = tempnum + name_length;
                                    }
                                    B_BASE::record.emplace_back(std::move(data_temp));
                                    effect_num++;
                                }
                                else
                                {
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                        assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, B_BASE::data);
                                        tempnum = tempnum + name_length;
                                    }
                                    effect_num++;
                                }
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }

                conn_obj->back_select_conn(conn);

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return 0;
        }

        M_MODEL &use_cache(int cache_time = 0)
        {
            iscache = true;
            exptime = cache_time;
            return *mod;
        }
        bool isuse_cache(bool iscachedate = false)
        {
            if (iscachedate)
            {
                return exptime == 0 && iscache == false;
            }
            return iscache;
        }
        void set_cache_state(bool isrestatus = false) { iscache = isrestatus; }
        void remove_exptime_cache()
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            temp_cache.remove_exptime();
        }
        void clear_cache()
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            temp_cache.clear();
        }
        bool remove_cache()
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            return temp_cache.remove(sqlhashid);
        }
        bool remove_cache(std::size_t cache_key_name)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            return temp_cache.remove(cache_key_name);
        }
        int check_cache(std::size_t cache_key_name)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            return temp_cache.check(cache_key_name);
        }
        std::vector<typename B_BASE::meta> get_cache_data(std::size_t cache_key_name)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            auto cache_data                                     = temp_cache.get(cache_key_name);
            return cache_data;
        }
        typename B_BASE::meta get_cache_obj(std::size_t cache_key_name)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            auto cache_data                                     = temp_cache.get_obj(cache_key_name);
            return cache_data;
        }
        M_MODEL &get_cache(std::size_t cache_key_name)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            B_BASE::record                                      = temp_cache.get(cache_key_name);
            if (B_BASE::record.size() == 0)
            {
                B_BASE::record_reset();
            }
            else
            {
                B_BASE::data = B_BASE::record[0];
            }
            return *mod;
        }
        int update_cache(int exp_time = 0)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            return temp_cache.update(sqlhashid, exp_time);
        }
        int update_cache(std::size_t cache_key_name, int exp_time)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            return temp_cache.update(cache_key_name, exp_time);
        }
        bool save_cache(int exp_time = 0)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::record, exp_time);
            return true;
        }

        bool save_cache(std::size_t cache_key_name, typename B_BASE::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            temp_cache.save(cache_key_name, cache_data, exp_time);
            return true;
        }

        bool save_cache(std::size_t cache_key_name, std::vector<typename B_BASE::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            temp_cache.save(cache_key_name, cache_data, exp_time);
            return true;
        }
        bool get_cacherecord(std::size_t cache_key_name)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            B_BASE::record                                      = temp_cache.get(cache_key_name);
            if (B_BASE::record.size() == 0)
            {
                return false;
            }
            else
            {
                B_BASE::data = B_BASE::record[0];
                return true;
            }
        }
        http::OBJ_VALUE fetch_json()
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            http::OBJ_VALUE valuetemp;
            valuetemp.set_array();

            if (iserror)
            {
                return valuetemp;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
 

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                http::OBJ_VALUE json_temp_v;  
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    std::string temp_str;    
                                    temp_str.resize(name_length);
                                    std::memcpy(temp_str.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    if(field_array[ij].name.size()>0)
                                    {
                                        //or alias name
                                        json_temp_v[field_array[ij].name]=std::move(temp_str);
                                    }
                                    else if(field_array[ij].org_name.size()>0)
                                    {
                                        json_temp_v[field_array[ij].org_name]=std::move(temp_str);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                valuetemp.push(json_temp_v);
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);

            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return valuetemp;
        }

        asio::awaitable<http::OBJ_VALUE> async_fetch_json()
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            http::OBJ_VALUE valuetemp;
            valuetemp.set_array();

            if (iserror)
            {
                co_return valuetemp;
            }
 
            try
            {
                if (conn_empty())
                {
                    co_return valuetemp;
                }
                auto conn = co_await conn_obj->async_get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;
                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
 

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                for (; is_sql_item == false;)
                {
                    n      = co_await conn->async_read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                http::OBJ_VALUE json_temp_v;  
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    std::string temp_str;    
                                    temp_str.resize(name_length);
                                    std::memcpy(temp_str.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    if(field_array[ij].name.size()>0)
                                    {
                                        //or alias name
                                        json_temp_v[field_array[ij].name]=std::move(temp_str);
                                    }
                                    else if(field_array[ij].org_name.size()>0)
                                    {
                                        json_temp_v[field_array[ij].org_name]=std::move(temp_str);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                valuetemp.push(json_temp_v);
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);

            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return valuetemp;
        }

        long long get_one(long long id)
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(B_BASE::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));
            sqlstring.append(" limit 1");
            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_cacherecord(sqlhashid))
                {
                    iscache = false;
                    return 0;
                }
            }

            B_BASE::data_reset();

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, B_BASE::data);
                                    tempnum = tempnum + name_length;
                                }
                                 
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);               

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return 0;
        }

        asio::awaitable<long long> async_get_one(long long id)
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(B_BASE::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));
            sqlstring.append(" limit 1");
            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_cacherecord(sqlhashid))
                {
                    iscache = false;
                    co_return 0;
                }
            }

            B_BASE::data_reset();

            if (iserror)
            {
                co_return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_select_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = co_await conn->async_read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, B_BASE::data);
                                    tempnum = tempnum + name_length;
                                }
                                 
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);               

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return 0;
        }

        int update()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }
            sqlstring = B_BASE::_makeupdatesql("");
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            return 0;
        }
        int update(const std::string &fieldname)
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    return 0;
                }
            }

            sqlstring = B_BASE::_makeupdatesql(fieldname);
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            return 0;
        }

        asio::awaitable<int> async_update(const std::string &fieldname)
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    co_return 0;
                }
            }

            sqlstring = B_BASE::_makeupdatesql(fieldname);
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscommit)
            {
                iscommit = false;
                co_return 0;
            }

            if (iserror)
            {
                co_return 0;
            }
            try
            {

                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                unsigned int offset = 0;
                n                   = co_await conn->async_read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);

                    co_return effect_num;
                }
                co_return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {

            }
            co_return 0;
        }
        asio::awaitable<int> async_update()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    co_return 0;
                }
            }

            sqlstring = B_BASE::_makeupdatesql("");
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscommit)
            {
                iscommit = false;
                co_return 0;
            }

            if (iserror)
            {
                co_return 0;
            }
            try
            {

                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                unsigned int offset = 0;
                n                   = co_await conn->async_read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);

                    co_return effect_num;
                }
                co_return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {

            }
            co_return 0;
        }

        int update_batch(const std::string &fieldname)
        {
            effect_num = 0;
            if (B_BASE::record.size() == 0)
            {
                return 0;
            }
            if (fieldname.size() > 0)
            {
                sqlstring = B_BASE::_make_insert_into_sql(fieldname);
            }
            else
            {
                sqlstring = B_BASE::_make_replace_into_sql();
            }

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            return 0;
        }
        int remove()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }

            sqlstring = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            return 0;
        }

        asio::awaitable<unsigned int> async_remove()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    co_return 0;
                }
            }

            sqlstring = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscommit)
            {
                iscommit = false;
                co_return 0;
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                
                unsigned int offset = 0;
                n                   = co_await conn->async_read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    co_return effect_num;
                }
                co_return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            co_return 0;
        }

        int remove(long long id)
        {
            effect_num = 0;
            sqlstring  = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(B_BASE::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));

            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            return 0;
        }

        asio::awaitable<unsigned int> async_remove(long long id)
        {
            effect_num = 0;
            sqlstring  = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(B_BASE::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));

            if (iscommit)
            {
                iscommit = false;
                co_return 0;
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                
                unsigned int offset = 0;
                n                   = co_await conn->async_read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    co_return effect_num;
                }
                co_return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            co_return 0;
        }

        int soft_remove(const std::string &fieldsql)
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }

            sqlstring = B_BASE::soft_remove_sql(fieldsql);
            if (sqlstring.empty())
            {
                error_msg = "soft delete field empty.";
                return 0;
            }
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            return 0;
        }
        int soft_remove()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    effect_num = 1;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }
            if (effect_num == 1)
            {
                sqlstring = B_BASE::soft_remove_sql(" ");
            }
            else
            {
                sqlstring = B_BASE::soft_remove_sql("");
            }
            effect_num = 0;
            if (sqlstring.empty())
            {
                error_msg = "soft delete field empty.";
                return 0;
            }
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
                
            }
            return 0;
        }
        long long insert(typename B_BASE::meta &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makerecordinsertsql(insert_data);
            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    long long   insert_last_id= conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);    

                    return insert_last_id;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return 0;
        }

        asio::awaitable<long long> async_insert(typename B_BASE::meta &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makerecordinsertsql(insert_data);
            if (iscommit)
            {
                iscommit = false;
                co_return 0;
            }

            if (iserror)
            {
                co_return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                
                unsigned int offset = 0;
                n                   = co_await conn->async_read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    long long   insert_last_id= conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);    

                    co_return insert_last_id;
                }
                co_return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return 0;
        }

        long long insert(std::vector<typename B_BASE::meta> &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makerecordinsertsql(insert_data);
            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    long long   insert_last_id= conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);    

                    return insert_last_id;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return 0;
        }

        asio::awaitable<long long> async_insert(std::vector<typename B_BASE::meta> &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makerecordinsertsql(insert_data);
            if (iscommit)
            {
                iscommit = false;
                co_return 0;
            }

            if (iserror)
            {
                co_return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                
                unsigned int offset = 0;
                n                   = co_await conn->async_read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    long long   insert_last_id= conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);    

                    co_return insert_last_id;
                }
                co_return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return 0;
        }

        long long insert()
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makeinsertsql();
            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    long long   insert_last_id= conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);    

                    return insert_last_id;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return 0;
        }

        asio::awaitable<long long> async_insert()
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makeinsertsql();
            if (iscommit)
            {
                iscommit = false;
                co_return 0;
            }

            if (iserror)
            {
                co_return 0;
            }
 
            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                auto conn = co_await conn_obj->async_get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                
                unsigned int offset = 0;
                n                   = co_await conn->async_read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    long long   insert_last_id= conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);    

                    co_return insert_last_id;
                }
                co_return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return 0;
        }


        long long save(bool isrealnew = false)
        {
            effect_num = 0;
            if (B_BASE::getPK() > 0 && isrealnew == false)
            {
                if (wheresql.empty())
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                sqlstring = B_BASE::_makeupdatesql("");
                sqlstring.append(" where ");
                if (wheresql.empty())
                {
                    return 0;
                }
                else
                {
                    sqlstring.append(wheresql);
                }
                if (!groupsql.empty())
                {
                    sqlstring.append(groupsql);
                }
                if (!ordersql.empty())
                {
                    sqlstring.append(ordersql);
                }
                if (!limitsql.empty())
                {
                    sqlstring.append(limitsql);
                }
                if (iscommit)
                {
                    iscommit = false;
                    return 0;
                }

                if (iserror)
                {
                    return 0;
                }
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    return effect_num;
                }
                return 0;
            }
            else
            {
                sqlstring  = B_BASE::_makeinsertsql();
                if (conn_empty())
                {
                    return 0;
                }
                auto conn = conn_obj->get_edit_conn();

                unsigned int querysql_len = sqlstring.length() + 1;

                conn->send_data.clear();
                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(sqlstring);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return 0;
                }

                unsigned int offset = 0;
                n                   = conn->read_loop();

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                conn_obj->back_edit_conn(conn);

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    long long   insert_last_id= conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);    

                    return insert_last_id;
                }
                return 0;
            }
            return 0;
        }

        asio::awaitable<long long> async_save(bool isrealnew = false)
        {
            effect_num = 0;
            if (B_BASE::getPK() > 0 && isrealnew == false)
            {
                if (wheresql.empty())
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                sqlstring = B_BASE::_makeupdatesql("");
                sqlstring.append(" where ");
                if (wheresql.empty())
                {
                    co_return 0;
                }
                else
                {
                    sqlstring.append(wheresql);
                }
                if (!groupsql.empty())
                {
                    sqlstring.append(groupsql);
                }
                if (!ordersql.empty())
                {
                    sqlstring.append(ordersql);
                }
                if (!limitsql.empty())
                {
                    sqlstring.append(limitsql);
                }
                if (iscommit)
                {
                    iscommit = false;
                    co_return 0;
                }

                if (iserror)
                {
                    co_return 0;
                }

                try
                {
                    if (conn_empty())
                    {
                        co_return 0;
                    }
                    auto conn = co_await conn_obj->async_get_edit_conn();

                    unsigned int querysql_len = sqlstring.length() + 1;

                    conn->send_data.clear();
                    conn->send_data.push_back((querysql_len & 0xFF));
                    conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                    conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                    conn->send_data.push_back(0x00);
                    conn->send_data.push_back(0x03);
                    conn->send_data.append(sqlstring);

                    std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                    unsigned int offset = 0;
                    n                   = co_await conn->async_read_loop();

                    pack_info_t temp_pack_data;
                    temp_pack_data.seq_id = 1;
                    conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                    conn_obj->back_edit_conn(conn);

                    if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                    {
                        error_msg = temp_pack_data.data.substr(3);
                        iserror = true;
                    }
                    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                    {

                        unsigned int d_offset = 1;
                        effect_num            = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);

                        co_return effect_num;
                    }
                    co_return 0;
                }
                catch (const std::exception &e)
                {
                    error_msg = std::string(e.what());
                    co_return 0;
                }
                catch (const std::string &e)
                {
                    error_msg = e;
                }
                catch (...)
                {
                    co_return 0;
                }
                co_return 0;
            }
            else
            {
                sqlstring  = B_BASE::_makeinsertsql();
                try
                {
                    if (conn_empty())
                    {
                        co_return 0;
                    }
                    auto conn = co_await conn_obj->async_get_edit_conn();

                    unsigned int querysql_len = sqlstring.length() + 1;

                    conn->send_data.clear();
                    conn->send_data.push_back((querysql_len & 0xFF));
                    conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                    conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                    conn->send_data.push_back(0x00);
                    conn->send_data.push_back(0x03);
                    conn->send_data.append(sqlstring);

                    std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                    
                    unsigned int offset = 0;
                    n                   = co_await conn->async_read_loop();

                    pack_info_t temp_pack_data;
                    temp_pack_data.seq_id = 1;
                    conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);

                    conn_obj->back_edit_conn(conn);

                    if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                    {
                        error_msg = temp_pack_data.data.substr(3);
                    }
                    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                    {

                        unsigned int d_offset = 1;
                        effect_num      = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                        long long   insert_last_id= conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);    

                        co_return insert_last_id;
                    }
                    co_return 0;
                }
                catch (const std::exception &e)
                {
                    error_msg = std::string(e.what());
                }
                catch (const std::string &e)
                {
                    error_msg = e;
                }
                catch (...)
                {
                }
                co_return 0;
            }
            co_return 0;
        }

        std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>
        query(const std::string &rawsql)
        {

            std::vector<std::vector<std::string>> temprecord;
            std::vector<std::string> table_fieldname;
            std::map<std::string, unsigned int> table_fieldmap;

            if(rawsql.size()>10)
            {
                unsigned int i=0;
                for(;i<rawsql.size();i++)
                {
                    if(rawsql[i]!=0x20)
                    {
                        break;
                    }
                }
                if(i<5)
                {
                    //must be select
                    if(rawsql[i]!='s' && rawsql[i]!='S')
                    {
                        iserror = true;   
                    }
                }
                else
                {
                    iserror = true;   
                }
            }
            else
            {
                iserror = true;   
            }

            if (iserror)
            {
                return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }

            try
            {
                if (conn_empty())
                {
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
                auto conn = conn_obj->get_select_conn();

                unsigned int querysql_len = rawsql.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(rawsql);

                std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                if(conn->ec)
                {
                    error_msg = conn->ec.message();
                    iserror   = true;
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = conn->read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                        table_fieldmap.insert({field_array[ii].org_name,table_fieldname.size()});
                                        table_fieldname.push_back(field_array[ii].org_name);
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::vector<std::string> temp_v_record; 
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    std::string tempstr;
                                    tempstr.resize(name_length);
                                    std::memcpy(tempstr.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    temp_v_record.push_back(std::move(tempstr));
                                    tempnum = tempnum + name_length;
                                }
                                temprecord.push_back(temp_v_record);
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);

                return std::make_tuple(std::move(table_fieldname), std::move(table_fieldmap), std::move(temprecord));
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }

        asio::awaitable<std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>>
        async_query(const std::string &rawsql)
        {

            std::vector<std::vector<std::string>> temprecord;
            std::vector<std::string> table_fieldname;
            std::map<std::string, unsigned int> table_fieldmap;

            if(rawsql.size()>10)
            {
                unsigned int i=0;
                for(;i<rawsql.size();i++)
                {
                    if(rawsql[i]!=0x20)
                    {
                        break;
                    }
                }
                if(i<5)
                {
                    //must be select
                    if(rawsql[i]!='s' && rawsql[i]!='S')
                    {
                        iserror = true;   
                    }
                }
                else
                {
                    iserror = true;   
                }
            }
            else
            {
                iserror = true;   
            }

            if (iserror)
            {
                co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }

            try
            {
                if (conn_empty())
                {
                    co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
                auto conn = co_await conn_obj->async_get_select_conn();

                unsigned int querysql_len = rawsql.length() + 1;

                conn->send_data.clear();

                conn->send_data.push_back((querysql_len & 0xFF));
                conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                conn->send_data.push_back(0x00);
                conn->send_data.push_back(0x03);
                conn->send_data.append(rawsql);

                std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);
                
                
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = co_await conn->async_read_loop();
                    offset = 0;
                    for (; offset < n;)
                    {
                        conn->read_field_pack(conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.length == temp_pack_data.current_length)
                        {
                            if (conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                        table_fieldmap.insert({field_array[ii].org_name,table_fieldname.size()});
                                        table_fieldname.push_back(field_array[ii].org_name);
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::vector<std::string> temp_v_record; 
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    std::string tempstr;
                                    tempstr.resize(name_length);
                                    std::memcpy(tempstr.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    temp_v_record.push_back(std::move(tempstr));
                                    tempnum = tempnum + name_length;
                                }
                                temprecord.push_back(temp_v_record);
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }
                conn_obj->back_select_conn(conn);

                co_return std::make_tuple(std::move(table_fieldname), std::move(table_fieldmap), std::move(temprecord));
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }

        // long long edit_query(const std::string &rawsql, bool isinsert = false)
        // {
        //     if (iserror)
        //     {
        //         return 0;
        //     }

        //     return 0;
        // }
        M_MODEL &clear(bool both = true)
        {
            selectsql.clear();
            wheresql.clear();
            ordersql.clear();
            groupsql.clear();
            limitsql.clear();
            sqlstring.clear();
            error_msg.clear();
            iskuohao     = false;
            ishascontent = false;
            iscommit     = false;
            iscache      = false;
            effect_num   = 0;
            if (both)
            {
                B_BASE::record_reset();
                B_BASE::data_reset();
            }
            return *mod;
        }
        M_MODEL &clearWhere()
        {
            selectsql.clear();
            wheresql.clear();
            ordersql.clear();
            groupsql.clear();
            limitsql.clear();
            sqlstring.clear();
            error_msg.clear();
            iskuohao     = false;
            ishascontent = false;
            iscommit     = false;
            iscache      = false;
            effect_num   = 0;
            return *mod;
        }
        M_MODEL &set_data(typename B_BASE::meta indata)
        {
            B_BASE::data = indata;
            return *mod;
        }
        M_MODEL &get() { return *mod; }
        std::string get_query() { return sqlstring; }
        M_MODEL &start_commit()
        {
            iscommit = true;
            return *mod;
        }
        M_MODEL &end_commit()
        {
            iscommit = false;
            return *mod;
        }

        unsigned int effect()
        {
            return effect_num;
        }
        bool conn_empty()
        {
            if (conn_obj)
            {
                return false; 
            }
            error_msg = "conn_obj is null";
            iserror   = true;
            return true;
        }

      public:
        std::string selectsql;
        std::string wheresql;
        std::string ordersql;
        std::string groupsql;
        std::string limitsql;
        std::string sqlstring;
        std::string dbtag;
        std::string error_msg;
        std::string original_tablename;

        // std::list<std::string> commit_sqllist;
        bool iskuohao           = false;
        bool iscommit           = false;
        bool ishascontent       = false;
        bool iscache            = false;
        bool iserror            = false;
        int exptime             = 0;
        unsigned int effect_num = 0;

        M_MODEL *mod;

        std::shared_ptr<orm_conn_pool> conn_obj;
    };
} /*tagnamespace_replace*/
}// namespace orm
#endif