#ifndef _ORM_CMS_TOPIC_OPERATE_H
#define _ORM_CMS_TOPIC_OPERATE_H

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
    class topic_mysql : public B_BASE
    {
      public:
        topic_mysql(const std::string &tag) : dbtag(tag)
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
        topic_mysql() : dbtag(B_BASE::_rmstag)
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return 0;
                    }
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
                    if (n==0)
                    {
                        co_return 0;
                    }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if (n==0)
                {
                    co_return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
            data_temp.parentid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.parentid= data_temp.parentid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 3:
            data_temp.cateid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.cateid= data_temp.cateid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 4:
            data_temp.sorttype=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.sorttype= data_temp.sorttype * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 5:
            data_temp.languagetype=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.languagetype= data_temp.languagetype * 10 + (result_temp_data[i]-'0');
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
            data_temp.isside=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.isside= data_temp.isside * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 8:
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
                case 9:
            data_temp.title.clear();
            data_temp.title.resize(value_size);
            
            std::memcpy(data_temp.title.data(), result_temp_data, value_size);
            break;
                case 10:
            data_temp.twotitle.clear();
            data_temp.twotitle.resize(value_size);
            
            std::memcpy(data_temp.twotitle.data(), result_temp_data, value_size);
            break;
                case 11:
            data_temp.memo.clear();
            data_temp.memo.resize(value_size);
            
            std::memcpy(data_temp.memo.data(), result_temp_data, value_size);
            break;
                case 12:
            data_temp.templatename.clear();
            data_temp.templatename.resize(value_size);
            
            std::memcpy(data_temp.templatename.data(), result_temp_data, value_size);
            break;
                case 13:
            data_temp.url.clear();
            data_temp.url.resize(value_size);
            
            std::memcpy(data_temp.url.data(), result_temp_data, value_size);
            break;
                case 14:
            data_temp.urlpath.clear();
            data_temp.urlpath.resize(value_size);
            
            std::memcpy(data_temp.urlpath.data(), result_temp_data, value_size);
            break;
                case 15:
            data_temp.imgurl.clear();
            data_temp.imgurl.resize(value_size);
            
            std::memcpy(data_temp.imgurl.data(), result_temp_data, value_size);
            break;
                case 16:
            data_temp.topimg.clear();
            data_temp.topimg.resize(value_size);
            
            std::memcpy(data_temp.topimg.data(), result_temp_data, value_size);
            break;
                case 17:
            data_temp.accesscode=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.accesscode= data_temp.accesscode * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                
        }
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
    

M_MODEL& eqParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid = ");

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
    

M_MODEL& nqParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid != ");

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
    

M_MODEL& inParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inParentid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

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
    

M_MODEL& inParentid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

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
    

M_MODEL& ninParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninParentid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

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
    

M_MODEL& ninParentid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

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
    

M_MODEL& btParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid > ");

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
    

M_MODEL& beParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid >= ");

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
    

M_MODEL& ltParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid < ");

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
    

M_MODEL& leParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid <= ");

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
    

M_MODEL& or_eqParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid = ");

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
    

M_MODEL& or_nqParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid != ");

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
    

M_MODEL& or_inParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inParentid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

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
    

M_MODEL& or_inParentid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

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
    

M_MODEL& or_ninParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninParentid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

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
    

M_MODEL& or_ninParentid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

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
    

M_MODEL& or_btParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid > ");

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
    

M_MODEL& or_beParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid >= ");

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
    

M_MODEL& or_ltParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid < ");

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
    

M_MODEL& or_leParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid <= ");

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
M_MODEL& eqParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid = ");

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
    

M_MODEL& nqCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid != ");

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
    

M_MODEL& inCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inCateid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid IN(");

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
    

M_MODEL& inCateid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid IN(");

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
    

M_MODEL& ninCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninCateid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid NOT IN(");

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
    

M_MODEL& ninCateid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid NOT IN(");

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
    

M_MODEL& btCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid > ");

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
    

M_MODEL& beCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid >= ");

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
    

M_MODEL& ltCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid < ");

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
    

M_MODEL& leCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid <= ");

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
    

M_MODEL& or_eqCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid = ");

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
    

M_MODEL& or_nqCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid != ");

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
    

M_MODEL& or_inCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inCateid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid IN(");

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
    

M_MODEL& or_inCateid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid IN(");

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
    

M_MODEL& or_ninCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninCateid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid NOT IN(");

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
    

M_MODEL& or_ninCateid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid NOT IN(");

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
    

M_MODEL& or_btCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid > ");

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
    

M_MODEL& or_beCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid >= ");

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
    

M_MODEL& or_ltCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid < ");

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
    

M_MODEL& or_leCateid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid <= ");

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
M_MODEL& eqCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leCateid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" cateid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype = ");

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
    

M_MODEL& nqSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype != ");

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
    

M_MODEL& inSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inSorttype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype IN(");

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
    

M_MODEL& inSorttype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype IN(");

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
    

M_MODEL& ninSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninSorttype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype NOT IN(");

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
    

M_MODEL& ninSorttype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype NOT IN(");

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
    

M_MODEL& btSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype > ");

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
    

M_MODEL& beSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype >= ");

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
    

M_MODEL& ltSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype < ");

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
    

M_MODEL& leSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype <= ");

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
    

M_MODEL& or_eqSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype = ");

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
    

M_MODEL& or_nqSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype != ");

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
    

M_MODEL& or_inSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inSorttype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype IN(");

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
    

M_MODEL& or_inSorttype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype IN(");

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
    

M_MODEL& or_ninSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninSorttype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype NOT IN(");

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
    

M_MODEL& or_ninSorttype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype NOT IN(");

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
    

M_MODEL& or_btSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype > ");

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
    

M_MODEL& or_beSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype >= ");

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
    

M_MODEL& or_ltSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype < ");

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
    

M_MODEL& or_leSorttype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype <= ");

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
M_MODEL& eqSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leSorttype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" sorttype <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype = ");

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
    

M_MODEL& nqLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype != ");

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
    

M_MODEL& inLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inLanguagetype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype IN(");

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
    

M_MODEL& inLanguagetype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype IN(");

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
    

M_MODEL& ninLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninLanguagetype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype NOT IN(");

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
    

M_MODEL& ninLanguagetype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype NOT IN(");

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
    

M_MODEL& btLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype > ");

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
    

M_MODEL& beLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype >= ");

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
    

M_MODEL& ltLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype < ");

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
    

M_MODEL& leLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype <= ");

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
    

M_MODEL& or_eqLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype = ");

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
    

M_MODEL& or_nqLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype != ");

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
    

M_MODEL& or_inLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inLanguagetype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype IN(");

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
    

M_MODEL& or_inLanguagetype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype IN(");

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
    

M_MODEL& or_ninLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninLanguagetype(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype NOT IN(");

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
    

M_MODEL& or_ninLanguagetype(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype NOT IN(");

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
    

M_MODEL& or_btLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype > ");

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
    

M_MODEL& or_beLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype >= ");

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
    

M_MODEL& or_ltLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype < ");

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
    

M_MODEL& or_leLanguagetype(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype <= ");

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
M_MODEL& eqLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leLanguagetype(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" languagetype <= ");

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
    

M_MODEL& eqIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside = ");

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
    

M_MODEL& nqIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside != ");

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
    

M_MODEL& inIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inIsside(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside IN(");

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
    

M_MODEL& inIsside(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside IN(");

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
    

M_MODEL& ninIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninIsside(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside NOT IN(");

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
    

M_MODEL& ninIsside(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside NOT IN(");

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
    

M_MODEL& btIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside > ");

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
    

M_MODEL& beIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside >= ");

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
    

M_MODEL& ltIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside < ");

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
    

M_MODEL& leIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside <= ");

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
    

M_MODEL& or_eqIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside = ");

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
    

M_MODEL& or_nqIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside != ");

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
    

M_MODEL& or_inIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inIsside(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside IN(");

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
    

M_MODEL& or_inIsside(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside IN(");

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
    

M_MODEL& or_ninIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninIsside(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside NOT IN(");

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
    

M_MODEL& or_ninIsside(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside NOT IN(");

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
    

M_MODEL& or_btIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside > ");

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
    

M_MODEL& or_beIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside >= ");

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
    

M_MODEL& or_ltIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside < ");

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
    

M_MODEL& or_leIsside(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside <= ");

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
M_MODEL& eqIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leIsside(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" isside <= ");

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
    

M_MODEL& nullTitle()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullTitle()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inTitle(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title IN(");

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
M_MODEL& inTitle(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title IN(");

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
    

M_MODEL& ninTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninTitle(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title NOT IN(");

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
M_MODEL& ninTitle(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title NOT IN(");

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
    

M_MODEL& likeTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullTitle()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullTitle()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inTitle(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title IN(");

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
M_MODEL& or_inTitle(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title IN(");

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
    

M_MODEL& or_ninTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninTitle(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title NOT IN(");

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
M_MODEL& or_ninTitle(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title NOT IN(");

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
    

M_MODEL& or_likeTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leTitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leTitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" title <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullTwotitle()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullTwotitle()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inTwotitle(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle IN(");

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
M_MODEL& inTwotitle(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle IN(");

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
    

M_MODEL& ninTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninTwotitle(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle NOT IN(");

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
M_MODEL& ninTwotitle(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle NOT IN(");

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
    

M_MODEL& likeTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullTwotitle()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullTwotitle()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inTwotitle(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle IN(");

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
M_MODEL& or_inTwotitle(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle IN(");

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
    

M_MODEL& or_ninTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninTwotitle(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle NOT IN(");

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
M_MODEL& or_ninTwotitle(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle NOT IN(");

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
    

M_MODEL& or_likeTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leTwotitle(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leTwotitle(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" twotitle <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullMemo()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullMemo()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inMemo(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo IN(");

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
M_MODEL& inMemo(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo IN(");

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
    

M_MODEL& ninMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninMemo(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo NOT IN(");

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
M_MODEL& ninMemo(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo NOT IN(");

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
    

M_MODEL& likeMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullMemo()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullMemo()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inMemo(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo IN(");

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
M_MODEL& or_inMemo(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo IN(");

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
    

M_MODEL& or_ninMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninMemo(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo NOT IN(");

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
M_MODEL& or_ninMemo(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo NOT IN(");

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
    

M_MODEL& or_likeMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leMemo(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leMemo(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" memo <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullTemplatename()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullTemplatename()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inTemplatename(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename IN(");

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
M_MODEL& inTemplatename(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename IN(");

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
    

M_MODEL& ninTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninTemplatename(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename NOT IN(");

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
M_MODEL& ninTemplatename(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename NOT IN(");

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
    

M_MODEL& likeTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullTemplatename()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullTemplatename()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inTemplatename(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename IN(");

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
M_MODEL& or_inTemplatename(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename IN(");

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
    

M_MODEL& or_ninTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninTemplatename(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename NOT IN(");

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
M_MODEL& or_ninTemplatename(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename NOT IN(");

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
    

M_MODEL& or_likeTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leTemplatename(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leTemplatename(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" templatename <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullUrl()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullUrl()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inUrl(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url IN(");

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
M_MODEL& inUrl(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url IN(");

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
    

M_MODEL& ninUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninUrl(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url NOT IN(");

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
M_MODEL& ninUrl(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url NOT IN(");

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
    

M_MODEL& likeUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullUrl()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullUrl()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inUrl(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url IN(");

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
M_MODEL& or_inUrl(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url IN(");

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
    

M_MODEL& or_ninUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninUrl(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url NOT IN(");

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
M_MODEL& or_ninUrl(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url NOT IN(");

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
    

M_MODEL& or_likeUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leUrl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leUrl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" url <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullUrlpath()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullUrlpath()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inUrlpath(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath IN(");

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
M_MODEL& inUrlpath(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath IN(");

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
    

M_MODEL& ninUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninUrlpath(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath NOT IN(");

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
M_MODEL& ninUrlpath(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath NOT IN(");

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
    

M_MODEL& likeUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullUrlpath()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullUrlpath()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inUrlpath(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath IN(");

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
M_MODEL& or_inUrlpath(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath IN(");

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
    

M_MODEL& or_ninUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninUrlpath(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath NOT IN(");

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
M_MODEL& or_ninUrlpath(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath NOT IN(");

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
    

M_MODEL& or_likeUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leUrlpath(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leUrlpath(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" urlpath <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullImgurl()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullImgurl()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inImgurl(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl IN(");

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
M_MODEL& inImgurl(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl IN(");

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
    

M_MODEL& ninImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninImgurl(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl NOT IN(");

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
M_MODEL& ninImgurl(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl NOT IN(");

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
    

M_MODEL& likeImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullImgurl()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullImgurl()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inImgurl(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl IN(");

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
M_MODEL& or_inImgurl(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl IN(");

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
    

M_MODEL& or_ninImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninImgurl(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl NOT IN(");

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
M_MODEL& or_ninImgurl(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl NOT IN(");

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
    

M_MODEL& or_likeImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leImgurl(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leImgurl(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" imgurl <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullTopimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullTopimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inTopimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg IN(");

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
M_MODEL& inTopimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg IN(");

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
    

M_MODEL& ninTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninTopimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg NOT IN(");

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
M_MODEL& ninTopimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg NOT IN(");

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
    

M_MODEL& likeTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullTopimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullTopimg()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inTopimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg IN(");

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
M_MODEL& or_inTopimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg IN(");

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
    

M_MODEL& or_ninTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninTopimg(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg NOT IN(");

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
M_MODEL& or_ninTopimg(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg NOT IN(");

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
    

M_MODEL& or_likeTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leTopimg(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leTopimg(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" topimg <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& eqAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode = ");

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
    

M_MODEL& nqAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode != ");

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
    

M_MODEL& inAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inAccesscode(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode IN(");

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
    

M_MODEL& inAccesscode(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode IN(");

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
    

M_MODEL& ninAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninAccesscode(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode NOT IN(");

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
    

M_MODEL& ninAccesscode(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode NOT IN(");

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
    

M_MODEL& btAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode > ");

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
    

M_MODEL& beAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode >= ");

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
    

M_MODEL& ltAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode < ");

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
    

M_MODEL& leAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode <= ");

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
    

M_MODEL& or_eqAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode = ");

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
    

M_MODEL& or_nqAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode != ");

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
    

M_MODEL& or_inAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inAccesscode(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode IN(");

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
    

M_MODEL& or_inAccesscode(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode IN(");

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
    

M_MODEL& or_ninAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninAccesscode(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode NOT IN(");

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
    

M_MODEL& or_ninAccesscode(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode NOT IN(");

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
    

M_MODEL& or_btAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode > ");

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
    

M_MODEL& or_beAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode >= ");

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
    

M_MODEL& or_ltAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode < ");

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
    

M_MODEL& or_leAccesscode(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode <= ");

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
M_MODEL& eqAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leAccesscode(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" accesscode <= ");

        wheresql.append(std::to_string(val));
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return temprecord;
                    }
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                    }
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

                std::size_t n = conn->write_sql(sqlstring);
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return *mod;
                }

                //unsigned int querysql_len = sqlstring.length() + 1;

                // conn->send_data.clear();

                // conn->send_data.push_back((querysql_len & 0xFF));
                // conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                // conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                // conn->send_data.push_back(0x00);
                // conn->send_data.push_back(0x03);
                // conn->send_data.append(sqlstring);

                // std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                // if(conn->ec)
                // {
                //     error_msg = conn->ec.message();
                //     iserror   = true;
                //     return *mod;
                // }
                
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return *mod;
                    }
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

                std::size_t n = co_await conn->async_write_sql(sqlstring);
                
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    co_return 0;
                }

                // unsigned int querysql_len = sqlstring.length() + 1;

                // conn->send_data.clear();

                // conn->send_data.push_back((querysql_len & 0xFF));
                // conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                // conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                // conn->send_data.push_back(0x00);
                // conn->send_data.push_back(0x03);
                // conn->send_data.append(sqlstring);

                // std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                
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
                    if (n==0)
                    {
                        co_return 0;
                    }
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return 0;
                    }
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
                    if (n==0)
                    {
                        co_return 0;
                    }
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

                std::size_t n = conn->write_sql(sqlstring);
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }

                // unsigned int querysql_len = sqlstring.length() + 1;

                // conn->send_data.clear();
                // conn->send_data.push_back((querysql_len & 0xFF));
                // conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                // conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                // conn->send_data.push_back(0x00);
                // conn->send_data.push_back(0x03);
                // conn->send_data.append(sqlstring);

                // std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                // if(conn->ec)
                // {
                //     error_msg = conn->ec.message();
                //     iserror   = true;
                //     return 0;
                // }

                
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return 0;
                    }
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

                std::size_t n = co_await conn->async_write_sql(sqlstring);
                
                if (n==0)
                {
                    error_msg = conn->error_msg;
                    co_return 0;
                }

                // unsigned int querysql_len = sqlstring.length() + 1;

                // conn->send_data.clear();
                // conn->send_data.push_back((querysql_len & 0xFF));
                // conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                // conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                // conn->send_data.push_back(0x00);
                // conn->send_data.push_back(0x03);
                // conn->send_data.append(sqlstring);

                // std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                
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
                    if (n==0)
                    {
                        co_return 0;
                    }
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return 0;
                    }
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
                    if (n==0)
                    {
                        co_return valuetemp;
                    }
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return 0;
                    }
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
                    if (n==0)
                    {
                        co_return 0;
                    }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }    
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

                std::size_t n = conn->write_sql(sqlstring);
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }

                // unsigned int querysql_len = sqlstring.length() + 1;

                // conn->send_data.clear();
                // conn->send_data.push_back((querysql_len & 0xFF));
                // conn->send_data.push_back((querysql_len >> 8 & 0xFF));
                // conn->send_data.push_back((querysql_len >> 16 & 0xFF));
                // conn->send_data.push_back(0x00);
                // conn->send_data.push_back(0x03);
                // conn->send_data.append(sqlstring);

                // std::size_t n = asio::write(*conn->socket, asio::buffer(conn->send_data), conn->ec);
                
                // if(conn->ec)
                // {
                //     error_msg = conn->ec.message();
                //     iserror   = true;
                //     return 0;
                // }

                unsigned int offset = 0;
                n                   = conn->read_loop();
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if (n==0)
                {
                    co_return 0;
                }
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
                if (n==0)
                {
                    co_return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if (n==0)
                {
                    co_return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if (n==0)
                {
                    co_return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if (n==0)
                {
                    co_return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if (n==0)
                {
                    co_return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if (n==0)
                {
                    co_return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                if(n==0)
                {
                    error_msg = conn->error_msg;
                    return 0;
                }
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
                    if (n==0)
                    {
                        co_return 0;
                    }
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
                    if (n==0)
                    {
                        co_return 0;
                    }
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
                    if(n==0)
                    {
                        error_msg = conn->error_msg;
                        return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                    }
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
                    if (n==0)
                    {
                        co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                    }
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

        std::shared_ptr<mysql_conn_base> select_conn;
        std::shared_ptr<mysql_conn_base> edit_conn;
        std::shared_ptr<orm_conn_pool> conn_obj;
    };
} /*tagnamespace_replace*/
}// namespace orm
#endif