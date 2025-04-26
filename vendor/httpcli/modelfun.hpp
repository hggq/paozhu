/*
 *  @author 黄自权 huangziquan
 *  @date 2022-05-04
 *  @dest ORM文件生成器
 */

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <array>
#include <filesystem>
#include <vector>
#include <cmath>
#include "mysql_conn.h"
#include "mysql_conn_pool.h"

namespace fs = std::filesystem;

int strtointval(std::string vval)
{
    int temp = 0;
    for (unsigned int i = 0; i < vval.size(); i++)
    {
        if (vval[i] == 0x20)
        {
            break;
        }
        if (vval[i] >= '0' && vval[i] <= '9')
        {
            temp = temp * 10 + (vval[i] - '0');
        }
        else
        {
            break;
        }
    }
    return temp;
}

std::string strtoline(const std::string &vval)
{
    std::string temp;
    for (unsigned int i = 0; i < vval.size(); i++)
    {
        if (vval[i] == 0x0D)
        {
            temp.append("\\r");
            continue;
        }
        else if (vval[i] == 0x0A)
        {
            temp.append("\\n");
            continue;
        }
        temp.push_back(vval[i]);
    }
    return temp;
}

bool isparent(const std::string &vval)
{
    if (vval.size() > 6)
    {
        if (vval[0] == 'p' && vval[1] == 'a' && vval[2] == 'r' && vval[3] == 'e' && vval[4] == 'n' && vval[5] == 't')
        {
            return true;
        }
    }
    return false;
}

std::string getgmtdatetime(time_t inputtime = 0)
{
    time_t curr_time;
    if (inputtime == 0)
    {
        curr_time = time((time_t *)NULL);
    }
    else
    {
        curr_time = inputtime;
    }
    tm *timeInfo;

    char timestr[30] = {'\0'};
    timeInfo         = gmtime(&curr_time);
    strftime(timestr, sizeof(timestr), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

    std::string temp(timestr);
    return temp;
}
struct table_columns_info_t
{
    std::string col_name;
    std::string comment;
    std::string default_value;
    unsigned char col_type;
    unsigned int col_length;
    unsigned char decimals;
    unsigned char big_type;//string number float
    bool is_pk       = false;
    bool is_auto_inc = false;
    bool is_unsigned = false;
    bool is_datetime = false;
};
//Field 	Type 	Collation 	Null 	Key 	Default 	Extra 	Privileges 	Comment
const static std::array<std::string, 9> table_columns_fields = {"field", "type", "collation", "null", "key", "default", "extra", "privileges", "comment"};
void assign_field_value(unsigned char index_pos, unsigned char *result_temp_data, unsigned int value_size, table_columns_info_t &data_temp)
{
    switch (index_pos)
    {
    case 0:
        data_temp.col_name.clear();
        data_temp.col_name.resize(value_size);
        for (size_t i = 0; i < value_size; i++)
        {
            if (result_temp_data[i] >= 'A' && result_temp_data[i] <= 'Z')
            {
                data_temp.col_name[i] = result_temp_data[i] + 32;
            }
            else
            {
                data_temp.col_name[i] = result_temp_data[i];
            }
        }
        break;
    case 1:

        data_temp.col_length = 0;
        data_temp.decimals   = 0;
        for (size_t i = 0; i < value_size; i++)
        {
            if (result_temp_data[i] == '(')
            {
                i++;
                for (; i < value_size; i++)
                {
                    if (result_temp_data[i] == ')')
                    {
                        break;
                    }
                    else if (result_temp_data[i] == ',')
                    {
                        i++;
                        for (; i < value_size; i++)
                        {
                            if (result_temp_data[i] == ')')
                            {
                                break;
                            }
                            if (result_temp_data[i] >= '0' && result_temp_data[i] <= '9')
                            {
                                data_temp.decimals = data_temp.decimals * 10 + (result_temp_data[i] - '0');
                            }
                        }
                        break;
                    }
                    if (result_temp_data[i] >= '0' && result_temp_data[i] <= '9')
                    {
                        data_temp.col_length = data_temp.col_length * 10 + (result_temp_data[i] - '0');
                    }
                }
            }
        }
        break;
    case 4:
        if (value_size > 2)
        {
            if (result_temp_data[0] == 'P' && result_temp_data[1] == 'R' && result_temp_data[2] == 'I')
            {
                data_temp.is_pk = true;
            }
        }
        break;
    case 5:
        data_temp.default_value.clear();
        data_temp.default_value.resize(value_size);
        for (size_t i = 0; i < value_size; i++)
        {
            data_temp.default_value[i] = result_temp_data[i];
        }
        break;
    case 6:

        if (value_size > 6)
        {
            if (result_temp_data[0] == 'a' && result_temp_data[1] == 'u' && result_temp_data[2] == 't' && result_temp_data[3] == 'o' && result_temp_data[4] == '_' && result_temp_data[5] == 'i')
            {
                data_temp.is_auto_inc = true;
            }
        }
        break;
    case 8:
        data_temp.comment.clear();
        data_temp.comment.resize(value_size);
        for (size_t i = 0; i < value_size; i++)
        {
            data_temp.comment[i] = result_temp_data[i];
        }
        break;
    default:
        break;
    }
}
unsigned int string_replace(std::string &content, const std::string &astr, const std::string &bstr)
{
    std::string::size_type start_num = 0;
    start_num                        = content.find(astr);

    if (std::string::npos != start_num)
    {
        std::string temp_content;
        temp_content.reserve(content.size());
        temp_content = content.substr(0, start_num);
        temp_content.append(bstr);
        temp_content.append(content.substr(start_num + astr.size()));

        content = temp_content;
        return 1;
    }
    return 0;
}
unsigned int string_replace_all(std::string &content, const std::string &astr, const std::string &bstr)
{
    std::string::size_type start_num = 0;
    std::string::size_type end_num   = content.find(astr, start_num);
    unsigned int count               = 0;
    std::string temp_content;
    if (std::string::npos != end_num)
    {
        temp_content.reserve(content.size());
    }
    while (std::string::npos != end_num)
    {
        temp_content.append(content.substr(start_num, end_num - start_num));
        temp_content.append(bstr);
        start_num = end_num + astr.size();
        count++;

        end_num = content.find(astr, start_num);
    }
    if (start_num > 0)
    {
        temp_content.append(content.substr(start_num));
        content = temp_content;
    }

    return count;
}
void colname_first_touper(std::string &a)
{
    if (a.size() > 0)
    {
        if (a[0] >= 'a' && a[0] <= 'z')
        {
            a[0] = a[0] - 32;
        }
    }
}
std::string colname_to_hump(const std::string &a)
{
    std::string b;
    for (unsigned int i = 0; i < a.size(); i++)
    {
        if (a[i] == '_')
        {
            i++;
            if (a[i] >= 'a' && a[i] <= 'z')
            {
                b.push_back(a[i] - 32);
            }
            continue;
        }
        b.push_back(a[i]);
    }
    return b;
}
/*
struct table_columns_info_t
{
    std::string col_name;
    std::string comment;
    std::string default_value;
    unsigned char col_type;
    unsigned int col_length;
    unsigned char decimals;
    unsigned char big_type;//string number float
    bool is_pk       = false;
    bool is_auto_inc = false;
    bool is_unsigned = false;
    bool is_datetime = false;
};
enum class protocol_field_type : std::uint8_t
{
    decimal = 0x00,    // Apparently not sent
    tiny = 0x01,       // TINYINT
    short_ = 0x02,     // SMALLINT
    long_ = 0x03,      // INT
    float_ = 0x04,     // FLOAT
    double_ = 0x05,    // DOUBLE
    null = 0x06,       // Apparently not sent
    timestamp = 0x07,  // TIMESTAMP
    longlong = 0x08,   // BIGINT
    int24 = 0x09,      // MEDIUMINT
    date = 0x0a,       // DATE
    time = 0x0b,       // TIME
    datetime = 0x0c,   // DATETIME   二进制 和 二进制协议共用 日期模式
    year = 0x0d,       // YEAR
    varchar = 0x0f,    // Apparently not sent
    bit = 0x10,        // BIT
    newdecimal = 0xf6, // DECIMAL
    enum_ = 0xf7,      // Apparently not sent
    set = 0xf8,        // Apperently not sent
    tiny_blob = 0xf9,  // Apparently not sent
    medium_blob = 0xfa,// Apparently not sent
    long_blob = 0xfb,  // Apparently not sent
    blob = 0xfc,       // Used for all TEXT and BLOB types
    var_string = 0xfd, // Used for VARCHAR and VARBINARY
    string = 0xfe,     // Used for CHAR and BINARY, ENUM (enum flag set), SET (set flag set)
    geometry = 0xff    // GEOMETRY
};
*/
std::string get_field_type(const table_columns_info_t &tp)
{
    std::string a;
    if (tp.col_type == 0x01)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned char ";
        }
        else
        {
            a = "char ";
        }
    }
    else if (tp.col_type == 0x02)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned short ";
        }
        else
        {
            a = "short ";
        }
    }
    else if (tp.col_type == 0x03)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned int ";
        }
        else
        {
            a = "int ";
        }
    }
    else if (tp.col_type == 0x04)
    {
        a = "float ";
    }
    else if (tp.col_type == 0x05)
    {
        a = "double ";
    }
    else if (tp.col_type == 0x07)
    {
        a = "std::string ";
    }
    else if (tp.col_type == 0x08)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned long long ";
        }
        else
        {
            a = "long long  ";
        }
    }
    else if (tp.col_type == 0x09)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned int ";
        }
        else
        {
            a = "int ";
        }
    }
    else if (tp.col_type == 0x0A || tp.col_type == 0x0B || tp.col_type == 0x0C || tp.col_type == 0x0D)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned int ";
        }
        else
        {
            a = "int ";
        }
    }
    else if (tp.col_type == 0xF6)
    {
        a = "double ";
    }
    else if (tp.col_type == 0xF9 || tp.col_type == 0xFA || tp.col_type == 0xFB || tp.col_type == 0xFC || tp.col_type == 0xFD || tp.col_type == 0xFE)
    {
        a = "std::string ";
    }
    return a;
}

std::string get_field_type_num_arg(const table_columns_info_t &tp)
{
    std::string a;
    if (tp.col_type == 0x01)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned char val";
        }
        else
        {
            a = "char val";
        }
    }
    else if (tp.col_type == 0x02)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned short val";
        }
        else
        {
            a = "short val";
        }
    }
    else if (tp.col_type == 0x03)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned int val";
        }
        else
        {
            a = "int val";
        }
    }
    else if (tp.col_type == 0x04)
    {
        a = "float val";
    }
    else if (tp.col_type == 0x05)
    {
        a = "double val";
    }
    else if (tp.col_type == 0x07)
    {
        a = "long long val";
    }
    else if (tp.col_type == 0x08)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned long long val";
        }
        else
        {
            a = "long long val";
        }
    }
    else if (tp.col_type == 0x09)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned int val";
        }
        else
        {
            a = "int val";
        }
    }
    else if (tp.col_type == 0x0A || tp.col_type == 0x0B || tp.col_type == 0x0C || tp.col_type == 0x0D)
    {
        if (tp.is_unsigned)
        {
            a = "unsigned int val";
        }
        else
        {
            a = "int val";
        }
    }
    else if (tp.col_type == 0xF6)
    {
        a = "double val";
    }
    else if (tp.col_type == 0xF9 || tp.col_type == 0xFA || tp.col_type == 0xFB || tp.col_type == 0xFC || tp.col_type == 0xFD || tp.col_type == 0xFE)
    {
        a = "long long val";
    }
    return a;
}

std::string get_field_type_string_arg([[maybe_unused]] const table_columns_info_t &tp)
{
    std::string a;
    a = "const std::string& val";
    return a;
}

std::string get_field_type_in_arg(const table_columns_info_t &tp)
{
    std::string a;
    if (tp.col_type == 0x01)
    {
        if (tp.is_unsigned)
        {
            a = "const std::vector<unsigned char>& val";
        }
        else
        {
            a = "const std::vector<char>& val";
        }
    }
    else if (tp.col_type == 0x02)
    {
        if (tp.is_unsigned)
        {
            a = "const std::vector<unsigned short>& val";
        }
        else
        {
            a = "const std::vector<short>& val";
        }
    }
    else if (tp.col_type == 0x03)
    {
        if (tp.is_unsigned)
        {
            a = "const std::vector<unsigned int>& val";
        }
        else
        {
            a = "const std::vector<int>& val";
        }
    }
    else if (tp.col_type == 0x04)
    {
        a = "const std::vector<float>& val";
    }
    else if (tp.col_type == 0x05)
    {
        a = "const std::vector<double>& val";
    }
    else if (tp.col_type == 0x07)
    {
        a = "const std::vector<long long>& val ";
    }
    else if (tp.col_type == 0x08)
    {
        if (tp.is_unsigned)
        {
            a = "const std::vector<unsigned long long>& val";
        }
        else
        {
            a = "const std::vector<long long>& val";
        }
    }
    else if (tp.col_type == 0x09)
    {
        if (tp.is_unsigned)
        {
            a = "const std::vector<unsigned int>& val";
        }
        else
        {
            a = "const std::vector<int>& val";
        }
    }
    else if (tp.col_type == 0x0A || tp.col_type == 0x0B || tp.col_type == 0x0C || tp.col_type == 0x0D)
    {
        a = "const std::vector<std::string>& val";
    }
    else if (tp.col_type == 0xF6)
    {
        a = "const std::vector<double>& val";
    }
    else if (tp.col_type == 0xF9 || tp.col_type == 0xFA || tp.col_type == 0xFB || tp.col_type == 0xFC || tp.col_type == 0xFD || tp.col_type == 0xFE)
    {
        a = "const std::vector<std::string>& val";
    }
    return a;
}

std::string get_field_type_in2_arg(const table_columns_info_t &tp)
{
    std::string a;
    if (tp.col_type == 0x01)
    {
        if (tp.is_unsigned)
        {
            a = "const std::vector<unsigned int>& val";
        }
        else
        {
            a = "const std::vector<int>& val";
        }
    }
    else if (tp.col_type == 0x02)
    {
        if (tp.is_unsigned)
        {
            a = "const std::vector<unsigned short>& val";
        }
        else
        {
            a = "const std::vector<short>& val";
        }
    }
    else if (tp.col_type == 0x03)
    {
    }
    else if (tp.col_type == 0x04)
    {
    }
    else if (tp.col_type == 0x05)
    {
    }
    else if (tp.col_type == 0x07)
    {
    }
    else if (tp.col_type == 0x08)
    {
        if (tp.is_unsigned)
        {
            a = "const std::vector<unsigned int>& val";
        }
        else
        {
            a = "const std::vector<int>& val";
        }
    }
    else if (tp.col_type == 0x09)
    {
    }
    else if (tp.col_type == 0x0A || tp.col_type == 0x0B || tp.col_type == 0x0C || tp.col_type == 0x0D)
    {
    }
    else if (tp.col_type == 0xF6)
    {
    }
    else if (tp.col_type == 0xF9 || tp.col_type == 0xFA || tp.col_type == 0xFB || tp.col_type == 0xFC || tp.col_type == 0xFD || tp.col_type == 0xFE)
    {
    }
    return a;
}
//begin number sql
std::string create_mysql_orm_where_eqstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& eq");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_nqstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& nq");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_instring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& in");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");
    append_content += R"(
        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_invecnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& in");
    append_content.append(humpname);
    append_content.append("(const std::vector<T>& val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");
    append_content += R"(
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
    )";
    return append_content;
}

std::string create_mysql_orm_where_invecstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& in");
    append_content.append(humpname);
    append_content.append("(const std::vector<std::string>& val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");
    append_content += R"(
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
    )";
    return append_content;
}

std::string create_mysql_orm_where_ninstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& nin");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");
    append_content += R"(
        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ninvecnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& nin");
    append_content.append(humpname);
    append_content.append("(const std::vector<T>& val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");
    append_content += R"(
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
    )";
    return append_content;
}

std::string create_mysql_orm_where_ninvecstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& nin");
    append_content.append(humpname);
    append_content.append("(const std::vector<std::string>& val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");
    append_content += R"(
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
    )";
    return append_content;
}

std::string create_mysql_orm_where_btstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& bt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_bestring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& be");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" >= \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ltstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& lt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_lestring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& le");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" <= \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_oreqstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_eq");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ornqstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_nq");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orinstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_in");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");
    append_content += R"(
        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orinvecnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_in");
    append_content.append(humpname);
    append_content.append("(const std::vector<T>& val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");
    append_content += R"(
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
    )";
    return append_content;
}

std::string create_mysql_orm_where_orinvecstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_in");
    append_content.append(humpname);
    append_content.append("(const std::vector<std::string>& val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");
    append_content += R"(
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
    )";
    return append_content;
}

std::string create_mysql_orm_where_orninstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_nin");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");
    append_content += R"(
        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orninvecnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_nin");
    append_content.append(humpname);
    append_content.append("(const std::vector<T>& val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");
    append_content += R"(
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
    )";
    return append_content;
}

std::string create_mysql_orm_where_orninvecstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_nin");
    append_content.append(humpname);
    append_content.append("(const std::vector<std::string>& val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");
    append_content += R"(
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
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbtstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_bt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbestring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_be");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" >= \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orltstring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_lt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orlestring_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_le");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" <= \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

//begin number number sql
std::string create_mysql_orm_where_eqnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& eq");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_nqnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& nq");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_btnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& bt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_benumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& be");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" >= \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ltnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& lt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_lenumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& le");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" <= \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_oreqnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_eq");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ornqnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_nq");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbtnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_bt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbenumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_be");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" >= \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orltnumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_lt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orlenumber_number(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_le");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" <= \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_number_feild_where(const std::string &modelName_file, const table_columns_info_t &tp)
{
    //number feild list
    std::string append_content;
    std::string temp1 = colname_to_hump(tp.col_name);

    colname_first_touper(temp1);

    append_content += create_mysql_orm_where_eqstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_nqstring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_instring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_invecnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_invecstring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_ninstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ninvecnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ninvecstring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_btstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_bestring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_ltstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_lestring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_oreqstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ornqstring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orinstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orinvecnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orinvecstring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orninstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orninvecnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orninvecstring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orbtstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orbestring_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orltstring_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orlestring_number(modelName_file, tp.col_name, temp1);

    ////////////////////////////////////
    //int
    ////////////////////////////////////
    append_content += create_mysql_orm_where_eqnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_nqnumber_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_btnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_benumber_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_ltnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_lenumber_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_oreqnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ornqnumber_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orbtnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orbenumber_number(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orltnumber_number(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orlenumber_number(modelName_file, tp.col_name, temp1);

    return append_content;
}

//begin float sql
std::string create_mysql_orm_where_btstring_float(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& bt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stod(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ltstring_float(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& lt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stod(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbtstring_float(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_bt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stod(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orltstring_float(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_lt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < \");\n");
    append_content += R"(
        try
        {
            wheresql.append(std::to_string(std::stod(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_btnumber_float(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_floating_point_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& bt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ltnumber_float(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_floating_point_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& lt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbtnumber_float(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_floating_point_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_bt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orltnumber_float(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_floating_point_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_lt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < \");\n");
    append_content += R"(
        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_float_feild_where(const std::string &modelName_file, const table_columns_info_t &tp)
{
    //float feild list
    std::string append_content;
    std::string temp1 = colname_to_hump(tp.col_name);

    colname_first_touper(temp1);

    append_content += create_mysql_orm_where_btstring_float(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ltstring_float(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orbtstring_float(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orltstring_float(modelName_file, tp.col_name, temp1);

    //float
    append_content += create_mysql_orm_where_btnumber_float(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ltnumber_float(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orbtnumber_float(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orltnumber_float(modelName_file, tp.col_name, temp1);

    return append_content;
}

std::string create_mysql_orm_where_null_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& null");
    append_content.append(humpname);
    append_content.append("()\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = NULL \");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_nnull_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& nnull");
    append_content.append(humpname);
    append_content.append("()\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != NULL \");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_eqstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& eq");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_neqstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& nq");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_instring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& in");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");
    append_content.append("\t\t\t\twheresql.append(val);\n");
    append_content.append("\t\t\t\twheresql.push_back(')');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_invecstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& in");
    append_content.append(humpname);
    append_content.append("(const std::vector<std::string> &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");

    append_content += R"(
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
    

    )";
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_invecnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& in");
    append_content.append(humpname);
    append_content.append("(const std::vector<T> &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");

    append_content += R"(
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
    

    )";
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ninstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& nin");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");
    append_content.append("\t\t\t\twheresql.append(val);\n");
    append_content.append("\t\t\t\twheresql.push_back(')');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ninvecstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& nin");
    append_content.append(humpname);
    append_content.append("(const std::vector<std::string> &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");

    append_content += R"(
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
    

    )";
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ninvecnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& nin");
    append_content.append(humpname);
    append_content.append("(const std::vector<T> &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");

    append_content += R"(
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
    

    )";
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_likestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& like");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" LIKE '%\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.append(\"%'\");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_llikestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& l_like");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" LIKE '%\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.append(\"'\");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_rlikestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& r_like");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" LIKE '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.append(\"%'\");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_btstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& bt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_bestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& be");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" >= '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ltstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& lt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_lestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& le");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" <= '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

//begin or

std::string create_mysql_orm_where_ornull_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_null");
    append_content.append(humpname);
    append_content.append("()\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = NULL \");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ornnull_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_nnull");
    append_content.append(humpname);
    append_content.append("()\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != NULL \");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_oreqstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_eq");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orneqstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_nq");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orinstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_in");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");
    append_content.append("\t\t\t\twheresql.append(val);\n");
    append_content.append("\t\t\t\twheresql.push_back(')');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orinvecstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_in");
    append_content.append(humpname);
    append_content.append("(const std::vector<std::string> &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");

    append_content += R"(
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
    

    )";
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orinvecnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_in");
    append_content.append(humpname);
    append_content.append("(const std::vector<T> &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" IN(\");\n");

    append_content += R"(
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
    

    )";
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orninstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_nin");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");
    append_content.append("\t\t\t\twheresql.append(val);\n");
    append_content.append("\t\t\t\twheresql.push_back(')');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orninvecstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_nin");
    append_content.append(humpname);
    append_content.append("(const std::vector<std::string> &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");

    append_content += R"(
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
    

    )";
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orninvecnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\trequires std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_nin");
    append_content.append(humpname);
    append_content.append("(const std::vector<T> &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" NOT IN(\");\n");

    append_content += R"(
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
    

    )";
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orlikestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_like");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" LIKE '%\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.append(\"%'\");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orllikestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& orl_like");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" LIKE '%\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.append(\"'\");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orrlikestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& orr_like");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" LIKE '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.append(\"%'\");\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbtstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_bt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_be");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" >= '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orltstring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_lt");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orlestring_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\n";
    append_content.append(modelName_file);
    append_content.append("& or_le");
    append_content.append(humpname);
    append_content.append("(const std::string &val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" <= '\");\n");
    append_content.append("\t\t\t\twheresql.append(B_BASE::stringaddslash(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

//where string begin number assign

std::string create_mysql_orm_where_eqnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& eq");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_nqnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& nq");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_btnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& bt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_benumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& be");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" >= '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ltnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& lt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < '\");\n");
    append_content.append("\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_lenumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& le");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" <= '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_oreqnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_eq");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" = '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_ornqnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_nq");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" != '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbtnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_bt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" > '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orbenumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_be");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" >= '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orltnumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_lt");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" < '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_where_orlenumber_string(const std::string &modelName_file, const std::string &colname, const std::string &humpname)
{
    std::string append_content;
    append_content += "\n\ntemplate <typename T>\n\t\trequires std::is_floating_point_v<T>||std::is_integral_v<T>\n";
    append_content.append(modelName_file);
    append_content.append("& or_le");
    append_content.append(humpname);
    append_content.append("(T val)\n\t{");
    append_content += R"(
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" )";
    append_content.append(colname);
    append_content.append(" <= '\");\n");
    append_content.append("\t\t\t\twheresql.append(std::to_string(val));\n");
    append_content.append("\t\t\t\twheresql.push_back('\\'');\n");
    append_content += R"(
        return *mod;   
    }   
    )";
    return append_content;
}

std::string create_mysql_orm_string_feild_where(const std::string &modelName_file, const table_columns_info_t &tp)
{

    std::string append_content;
    std::string temp1 = colname_to_hump(tp.col_name);

    colname_first_touper(temp1);

    append_content += create_mysql_orm_where_null_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_nnull_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_eqstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_neqstring_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_instring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_invecstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_invecnumber_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_ninstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ninvecstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ninvecnumber_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_likestring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_llikestring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_rlikestring_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_btstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_bestring_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_ltstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_lestring_string(modelName_file, tp.col_name, temp1);

    //begin or
    ////////////////////////////////

    append_content += create_mysql_orm_where_ornull_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ornnull_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_oreqstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orneqstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orinstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orinvecstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orinvecnumber_string(modelName_file, tp.col_name, temp1);
    //

    append_content += create_mysql_orm_where_orninstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orninvecstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orninvecnumber_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orlikestring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orllikestring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orrlikestring_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orbtstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orbestring_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orltstring_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orlestring_string(modelName_file, tp.col_name, temp1);

    //begin number

    append_content += create_mysql_orm_where_eqnumber_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_nqnumber_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_btnumber_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_benumber_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_ltnumber_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_lenumber_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_oreqnumber_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_ornqnumber_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orbtnumber_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orbenumber_string(modelName_file, tp.col_name, temp1);

    append_content += create_mysql_orm_where_orltnumber_string(modelName_file, tp.col_name, temp1);
    append_content += create_mysql_orm_where_orlenumber_string(modelName_file, tp.col_name, temp1);

    return append_content;
}

void create_mysql_orm_operate_file(const std::string &prj_root_path, const std::string &db_tag, const std::string &table_name, const std::string &model_name, const std::vector<orm::field_info_t> &field_array, const std::vector<table_columns_info_t> &table_column_info_lists)
{
    //read orm template
    std::string header_name = "_ORM_" + db_tag + "_" + table_name + "_OPERATE_H";
    std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::toupper);

    std::string real_model_name = model_name;
    std::string real_tag;
    std::string real_tag_sp;
    std::string modelName_file = model_name;
    colname_first_touper(modelName_file);

    if (db_tag != "default")
    {
        real_tag    = db_tag;
        real_tag_sp = db_tag + "::";
    }

    if(field_array.size()>0)
    {
        //
    }

    std::transform(model_name.begin(), model_name.end(), real_model_name.begin(), ::tolower);
    std::string template_file = prj_root_path;
    std::string template_content;
    std::string append_content;
    std::string modelName_m          = model_name + "_mysql";
    //std::string real_model_base_name = real_model_name + "base";

    if (template_file.size() > 0 && template_file.back() != '/')
    {
        template_file.push_back('/');
    }
    template_file.append("vendor/httpserver/include/mysqlorm.hpp");

    std::FILE *fp = fopen(template_file.c_str(), "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        unsigned long long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        template_content.resize(file_size);
        file_size = fread(&template_content[0], 1, file_size, fp);
        template_content.resize(file_size);
        fclose(fp);
    }

    if (template_content.size() == 0)
    {
        std::cout << "You must execute the CLI command in the project root directory." << std::endl;
        return;
    }

    std::size_t n = string_replace_all(template_content, "HTTP_MYSQL_ORM_HPP", header_name);
    if(n == 0)
    {
        //error 
    }
    n = string_replace_all(template_content, "mysql_orm", modelName_m);
    if(n == 0)
    {
        //error 
    }
    if (real_tag.size() > 0)
    {
        n = string_replace(template_content, "/*tagnamespace*/", "namespace " + real_tag);
    }
    else
    {
        n = string_replace(template_content, "{ /*tagnamespace_replace*/", "//{ /*tagnamespace_replace*/");
        n = string_replace(template_content, "} /*tagnamespace_replace*/", "//} /*tagnamespace_replace*/");
    }

    append_content +=R"(void assign_field_value(unsigned char index_pos, unsigned char *result_temp_data, unsigned int value_size, typename B_BASE::meta &data_temp)
    {
        switch(index_pos)
        {
            )";
        for (unsigned int m = 0; m < table_column_info_lists.size(); m++)
        {
            append_content +=R"(case )";

            append_content.append(std::to_string(m));
            if (table_column_info_lists[m].big_type == 1 && table_column_info_lists[m].is_datetime == false )
            {
                append_content +=R"(:
            data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);
            append_content +=R"(.clear();
            data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);
            append_content +=R"(.resize(value_size);
            )";
            append_content +=R"(
            std::memcpy(data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);
            append_content +=R"(.data(), result_temp_data, value_size);
            break;
                )";
            }
            else if (table_column_info_lists[m].big_type == 1 && table_column_info_lists[m].is_datetime == true )
            {
                append_content +=R"(:
            data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);
            append_content +=R"(.clear();
            data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);
            append_content +=R"(.resize(value_size);
            )";
            append_content +=R"(
            std::memcpy(data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);
            append_content +=R"(.data(), result_temp_data, value_size);
            break;
                )";
            }
            else if (table_column_info_lists[m].big_type == 2)
            {
                append_content +=R"(:
            data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);
            append_content +=R"(=0;
            )";

            append_content +=R"(
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);       
            append_content +=R"(= data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);    
                append_content +=R"( * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                )";
            }
            else if (table_column_info_lists[m].big_type == 3)
            {
                append_content +=R"(:
            data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);
            append_content +=R"(=0.0;
            )";

            append_content +=R"(
            for(unsigned int i=0,j=0; i< value_size; i++)
            {
                if(result_temp_data[i]=='.')
                {
                    j++;
                }else if(j> 0)
                {
                    data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);       
                append_content +=R"(= data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);    
                append_content +=R"( + (result_temp_data[i]-'0')/(j*10);
                j++;
                }
                else
                {
                    data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);       
            append_content +=R"(= data_temp.)";
            append_content.append(table_column_info_lists[m].col_name);    
                append_content +=R"( * 10 + (result_temp_data[i]-'0');
                }
                if(i>32)
                {
                    break;
                }
            }
            break;
                )";
            }

        }

        append_content +=R"(
        }
    }
    )";

    std::string raw_template="M_MODEL";
    for (unsigned int m = 0; m < table_column_info_lists.size(); m++)
    {
        //where
        if (table_column_info_lists[m].big_type == 1)
        {
            append_content.append(create_mysql_orm_string_feild_where(raw_template, table_column_info_lists[m]));
        }
        else if (table_column_info_lists[m].big_type == 2)
        {
            append_content.append(create_mysql_orm_number_feild_where(raw_template, table_column_info_lists[m]));
        }
        else if (table_column_info_lists[m].big_type == 3)
        {
            append_content.append(create_mysql_orm_float_feild_where(raw_template, table_column_info_lists[m]));
        }
    }
 
    /*
    enum class protocol_field_type : std::uint8_t
    {
        decimal = 0x00,    // Apparently not sent
        tiny = 0x01,       // TINYINT
        short_ = 0x02,     // SMALLINT
        long_ = 0x03,      // INT
        float_ = 0x04,     // FLOAT
        double_ = 0x05,    // DOUBLE
        null = 0x06,       // Apparently not sent
        timestamp = 0x07,  // TIMESTAMP
        longlong = 0x08,   // BIGINT
        int24 = 0x09,      // MEDIUMINT
        date = 0x0a,       // DATE
        time = 0x0b,       // TIME
        datetime = 0x0c,   // DATETIME   二进制 和 二进制协议共用 日期模式
        year = 0x0d,       // YEAR
        varchar = 0x0f,    // Apparently not sent
        bit = 0x10,        // BIT
        newdecimal = 0xf6, // DECIMAL
        enum_ = 0xf7,      // Apparently not sent
        set = 0xf8,        // Apperently not sent
        tiny_blob = 0xf9,  // Apparently not sent
        medium_blob = 0xfa,// Apparently not sent
        long_blob = 0xfb,  // Apparently not sent
        blob = 0xfc,       // Used for all TEXT and BLOB types
        var_string = 0xfd, // Used for VARCHAR and VARBINARY
        string = 0xfe,     // Used for CHAR and BINARY, ENUM (enum flag set), SET (set flag set)
        geometry = 0xff    // GEOMETRY
    };

    struct table_columns_info_t
    {
        std::string col_name;
        std::string comment;
        std::string default_value;
        unsigned char col_type;
        unsigned int col_length;
        unsigned char decimals;
        unsigned char big_type;//string number float
        bool is_pk       = false;
        bool is_auto_inc = false;
        bool is_unsigned = false;
        bool is_datetime = false;
    };
    */


    ///*appendwhere*/
    n = string_replace(template_content, "/*appendwhere*/", append_content);
    //save template
    template_file.clear();
    template_file = prj_root_path;

    if (template_file.size() > 0 && template_file.back() != '/')
    {
        template_file.push_back('/');
    }
    template_file.append("orm/");

    if (db_tag != "default")
    {
        template_file.append(db_tag);
        template_file.push_back('/');
    }

    template_file.append("include/");
    template_file.append(table_name);
    template_file.append("_mysql");
    template_file.append(".h");
    fp = fopen(template_file.c_str(), "wb");
    fwrite(template_content.data(), 1, template_content.size(), fp);
    fclose(fp);
}

int create_orm_model_baseinfo_file(const std::string &prj_root_path, const std::string &db_tag, const std::string &table_name, const std::string &model_name, const std::vector<orm::field_info_t> &field_array, const std::vector<table_columns_info_t> &table_column_info_lists)
{

    std::string modelspath = "models/";
    std::string sqlqueryring;
    std::string filebasefilename;
    std::string basefilepath;
    std::string model_name_obj=model_name;
 
    colname_first_touper(model_name_obj);
    filebasefilename.resize(model_name.size());

    std::transform(model_name.begin(), model_name.end(), filebasefilename.begin(), ::tolower);
    std::string tablenamebase = filebasefilename;
    std::cout << "create \033[1m\033[31m" << table_name << "\033[0m table to models 🚗" << std::endl;
    std::string rmstag = db_tag;
    if (rmstag.empty())
    {
        rmstag = "default";
    }

    if(field_array.size()>0)
    {
        //
    }

    if (rmstag == "default")
    {
        basefilepath = prj_root_path;
        if (basefilepath.size() > 0 && basefilepath.back() != '/')
        {
            basefilepath.push_back('/');
        }
        basefilepath = basefilepath + "orm/";
    }
    else
    {
        basefilepath = prj_root_path;
        if (basefilepath.size() > 0 && basefilepath.back() != '/')
        {
            basefilepath.push_back('/');
        }
        basefilepath = basefilepath + "orm/" + rmstag;
    }
    if (basefilepath.size() > 0 && basefilepath.back() != '/')
    {
        basefilepath.push_back('/');
    }

    modelspath = prj_root_path;
    if (modelspath.size() > 0 && modelspath.back() != '/')
    {
        modelspath.push_back('/');
    }
    modelspath.append("models/");
    if (rmstag != "default")
    {
        modelspath.append(rmstag);
        modelspath.push_back('/');
    }

    fs::path paths = modelspath;
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }
    paths = modelspath + "include";
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    int num_fields = table_column_info_lists.size();// mysql_num_fields(result);

    std::vector<std::string> table, tablecollist;
    std::vector<unsigned int> tablecelwidth;

    table.resize(num_fields);
    tablecelwidth.resize(num_fields);

    // int defaultcolnamepos = 255;

    std::map<std::string, std::map<std::string, std::string>> tableinfo;
    std::map<std::string, std::pair<std::string, std::string>> tablefieldscale;
    std::map<std::string, std::pair<std::string, std::string>> tablefieldtree;
    std::map<unsigned char, unsigned char> table_type;
    std::map<unsigned char, unsigned char> table_type_unsigned;
    unsigned int ikkk = 0;
    for (auto &row : table_column_info_lists)
    {
        table[ikkk].append(row.col_name);
        std::transform(table[ikkk].begin(), table[ikkk].end(), table[ikkk].begin(), ::tolower);
        table_type[ikkk]          = row.col_type;
        table_type_unsigned[ikkk] = row.is_unsigned ? 1 : 0;
        ikkk++;
    }

    std::ostringstream tp;
    std::string temp;
    std::string fieldname;
    std::string tablepkname, tablepriname;

    int row_num_count = 0;
    tablecollist.reserve(row_num_count);
    bool ismeta_tree = false;

    for (auto &row : table_column_info_lists)
    {
        fieldname = row.col_name;
        std::transform(fieldname.begin(), fieldname.end(), fieldname.begin(), ::tolower);
        tablecollist.push_back(fieldname);

        if (fieldname == "pid" || fieldname == "parentid" || fieldname == "parent_id")
        {
            ismeta_tree = true;
        }
        else if (isparent(fieldname))
        {
            ismeta_tree = true;
        }

        if (row.is_auto_inc)
        {
            tablepkname = fieldname;
        }
        if (row.is_pk)
        {
            tablepriname = fieldname;
        }

        temp.clear();

        temp = row.comment;
        if (temp.size() > 0)
        {
            std::string scalename;
            std::string scalenum;
            bool isc      = false;
            char act_type = 0;
            for (unsigned int n = 0; n < temp.size(); n++)
            {
                if (temp[n] == '[')
                {
                    isc = true;
                    continue;
                }

                if (isc)
                {
                    if (temp[n] == '*')
                    {
                        isc = false;
                        n++;
                        act_type = 1;
                        for (; n < temp.size(); n++)
                        {
                            if (temp[n] == ']')
                            {
                                break;
                            }
                            if (temp[n] > 0x2F && temp[n] < 0x3A)
                            {
                                scalenum.push_back(temp[n]);
                            }
                            else
                            {
                                scalenum.clear();
                                break;
                            }
                        }
                    }
                    else if (temp[n] == 0x20)
                    {
                        isc = false;
                        n++;
                        act_type = 2;
                        for (; n < temp.size(); n++)
                        {
                            if (temp[n] == ']')
                            {
                                break;
                            }
                            if ((temp[n] > 0x40 && temp[n] < 0x5B) || (temp[n] > 0x60 && temp[n] < 0x7B))
                            {
                                scalenum.push_back(temp[n]);
                            }
                            else
                            {
                                scalenum.clear();
                                break;
                            }
                        }
                    }
                    if (isc)
                    {
                        scalename.push_back(temp[n]);
                    }
                    else
                    {
                        break;
                    }
                }
            }

            switch (act_type)
            {
            case 1:
                if (scalename.size() > 0 && scalenum.size() > 0)
                {
                    tablefieldscale.insert({fieldname, {scalename, scalenum}});
                }
                break;
            case 2:
                if (scalename.size() > 0 && scalenum.size() > 0)
                {
                    tablefieldtree.insert({fieldname, {scalename, scalenum}});
                }
                break;
            }
        }
        //}
    }

    std::string temptype;
    std::vector<std::string> metalist;
    std::string outlist;
    //int colpos = 0;
    std::vector<std::pair<int, std::string>> stringcollist;
    std::vector<std::pair<int, std::string>> numbercollist;
    std::vector<std::pair<int, std::string>> floatcollist;
    std::map<unsigned char, std::string> collisttype;

    std::map<unsigned char, unsigned char> colltypeshuzi;

    std::string collnametemp;
    for (unsigned int i = 0; i < table_column_info_lists.size(); i++)
    {
        if (table_column_info_lists[i].big_type == 1)
        {
            stringcollist.push_back({i, table_column_info_lists[i].col_name});
        }
        else if (table_column_info_lists[i].big_type == 2)
        {
            numbercollist.push_back({i, table_column_info_lists[i].col_name});
        }
        else if (table_column_info_lists[i].big_type == 3)
        {
            floatcollist.push_back({i, table_column_info_lists[i].col_name});
        }

        colltypeshuzi[i] = 1;
        if (table_column_info_lists[i].col_type == 0x09 || table_column_info_lists[i].col_type == 0x03)
        {
            colltypeshuzi[i] = 1;
        }
        if (table_column_info_lists[i].big_type == 1 && table_column_info_lists[i].is_datetime == false)
        {
            colltypeshuzi[i] = 30;
        }
        if (table_column_info_lists[i].col_type == 0x0C || table_column_info_lists[i].col_type == 0x07)
        {
            colltypeshuzi[i] = 60;
        }
        if (table_column_info_lists[i].col_type == 0x0A)
        {
            colltypeshuzi[i] = 61;
        }
        if (table_column_info_lists[i].col_type == 0xF6 || table_column_info_lists[i].col_type == 0x04 || table_column_info_lists[i].col_type == 0x05)
        {
            colltypeshuzi[i] = 20;
        }

        collnametemp.clear();
        switch (table_column_info_lists[i].col_type)
        {
        case 1:
            if (table_column_info_lists[i].is_unsigned)
            {
                collnametemp = " unsigned ";
            }
            collnametemp.append(" char ");
            break;
        case 2:
            if (table_column_info_lists[i].is_unsigned)
            {
                collnametemp = " unsigned ";
            }
            collnametemp.append(" short ");
            break;
        case 3:
            if (table_column_info_lists[i].is_unsigned)
            {
                collnametemp = " unsigned ";
            }
            collnametemp.append(" int ");
            break;
        case 4:
            collnametemp.append(" float ");
            break;
        case 5:
            collnametemp.append(" double ");
            break;
        case 7:
            collnametemp.append(" std::string ");
            break;
        case 8:
            if (table_column_info_lists[i].is_unsigned)
            {
                collnametemp = " unsigned ";
            }
            collnametemp.append(" long long ");
            break;
        case 9:
            if (table_column_info_lists[i].is_unsigned)
            {
                collnametemp = " unsigned ";
            }
            collnametemp.append(" int ");
            break;
        case 10:
            collnametemp.append(" std::string ");
            break;
        case 11:
            collnametemp.append(" std::string ");
            break;
        case 12:
            collnametemp.append(" std::string ");
            break;
        case 13:
            collnametemp.append(" std::string ");
            break;
        case 14:
            collnametemp.append(" unsigned int ");
            break;
        case 0xF5://json
            collnametemp.append(" std::string ");
            break;
        case 0xF6:
            if (table_column_info_lists[i].col_length > 6)
            {
                collnametemp.append(" double ");
            }
            else
            {
                collnametemp.append(" float ");
            }
            break;
        default:
            collnametemp.append(" std::string ");
            break;
        }

        collisttype[i] = collnametemp;

        if (table_column_info_lists[i].big_type == 1)
        {
            collnametemp = collnametemp + " " + table_column_info_lists[i].col_name + " = \"" + table_column_info_lists[i].default_value + "\"; ///*" + table_column_info_lists[i].comment + "*/";
        }
        else
        {
            std::string at = table_column_info_lists[i].default_value;
            if (at.size() == 0)
            {
                at = "0";
            }
            collnametemp = collnametemp + " " + table_column_info_lists[i].col_name + " = " + at + "; ///*" + table_column_info_lists[i].comment + "*/";
        }
        metalist.push_back(collnametemp);
    }

    std::string filebasename = modelspath;

    std::string modelfileclass = modelspath;
    std::string createfilename;
    std::string headtxt;
    if (modelfileclass.back() != '/')
    {
        modelfileclass.push_back('/');
    }

    createfilename = model_name_obj;

    modelfileclass.append(createfilename);
    std::string modelfileclasscpp = modelfileclass;
    modelfileclasscpp.append(".cpp");

    std::ostringstream filemodelstremcpp;

    if (rmstag != "default")
    {
        filemodelstremcpp << "\n#include \"";
        filemodelstremcpp << model_name;
        filemodelstremcpp << "_mysql.h\"";
        filemodelstremcpp << "\n#include \"";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << "/include/";
        filemodelstremcpp << tablenamebase << "_base.h\"\n";
        filemodelstremcpp << "#include \"";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << "/include/";
        filemodelstremcpp << model_name_obj << ".h\"\n";
    }
    else
    {
        filemodelstremcpp << "\n#include \"";
        filemodelstremcpp << model_name;
        filemodelstremcpp << "_mysql.h\"";
        filemodelstremcpp << "\n#include \"" << tablenamebase << "_base.h\"\n#include \""
                          << model_name_obj << ".h\"\n";
    }

    filemodelstremcpp << "\n/* 如果此文件存在不会自动覆盖，没有则会自动生成。\n*If this file exists, it will not be "
                         "overwritten automatically. If not, it will be generated automatically. */\n";
    filemodelstremcpp << "\n";

    filemodelstremcpp << "\t \n namespace orm{\r\n";
    if (rmstag != "default")
    {
        filemodelstremcpp << "\t namespace ";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << "{ ";
    }
    filemodelstremcpp << " ";
    filemodelstremcpp << "\n\t\t\t " << model_name_obj << "::" << model_name_obj
                      << "(std::string dbtag):";
    filemodelstremcpp << model_name;
    filemodelstremcpp << "_mysql(dbtag){ mod=this; }\n";
    filemodelstremcpp << "\t\t\t " << model_name_obj << "::" << model_name_obj << "():";
    filemodelstremcpp << model_name;
    filemodelstremcpp << "_mysql(){ mod=this; }\n";

    if (rmstag != "default")
    {
        filemodelstremcpp << "\n\n\t\t} ";
    }
    filemodelstremcpp << "\n\n\t  }\n";

    // filemodelstremcpp<<"};\n";

    headtxt.append(filemodelstremcpp.str());
    filemodelstremcpp.str("");

    if (!fs::exists(modelfileclasscpp))
    {
        FILE *ff = fopen(modelfileclasscpp.c_str(), "wb");
        fwrite(&headtxt[0], headtxt.size(), 1, ff);
        fclose(ff);

        std::cout << " create model .cpp file: \033[1m\033[31m" << modelfileclasscpp << "\033[0m" << std::endl;
    }
    headtxt.clear();
    modelfileclass = modelspath;
    if (modelfileclass.back() != '/')
    {
        modelfileclass.push_back('/');
    }
    modelfileclass.append("include/");
    modelfileclasscpp = modelfileclass;
    modelfileclasscpp.append(createfilename);

    modelfileclasscpp.append(".h");
    headtxt.append("ORM_");
    headtxt.append(rmstag);
    headtxt.append("_");
    headtxt.append(createfilename);

    headtxt.append("_h");
    std::transform(headtxt.begin(), headtxt.end(), headtxt.begin(), ::toupper);
    filemodelstremcpp << "#ifndef " << headtxt;
    filemodelstremcpp << "\n#define " << headtxt;

    if (rmstag != "default")
    {
        filemodelstremcpp << "\n#include \"";
        filemodelstremcpp << model_name;
        filemodelstremcpp << "_mysql.h\" \n#include \"";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << "/include/";
        filemodelstremcpp << tablenamebase << "_base.h\"\n";

    }
    else
    {
        filemodelstremcpp << "\n#include \"";
        filemodelstremcpp << model_name;
        filemodelstremcpp << "_mysql.h\" \n#include \"";
        filemodelstremcpp << tablenamebase << "_base.h\"\n";
    }
    filemodelstremcpp << "\n/* 如果此文件存在不会自动覆盖，没有则会自动生成。\n*If this file exists, it will not be "
                         "overwritten automatically. If not, it will be generated automatically. */\n";
    filemodelstremcpp << "\n namespace orm {\n";

    if (rmstag != "default")
    {
        filemodelstremcpp << "\tnamespace ";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << " { \n";

    }
    filemodelstremcpp << "\t\tclass " << model_name_obj << " : public "
                      << model_name << "_mysql<" << model_name_obj << ","
                      << tablenamebase << "_base>{\n";

    filemodelstremcpp << "\t\t public:\n";
    filemodelstremcpp << "\t\t " << model_name_obj << "(std::string dbtag);\n";
    filemodelstremcpp << "\t\t " << model_name_obj << "();\n";
    filemodelstremcpp << "\t\t};\n";
    if (rmstag != "default")
    {
        filemodelstremcpp << "\t }\n ";
    }
    filemodelstremcpp << "}\n#endif\n";

    headtxt.clear();
    headtxt.append(filemodelstremcpp.str());
    filemodelstremcpp.str("");

    if (!fs::exists(modelfileclasscpp))
    {

        FILE *ff = fopen(modelfileclasscpp.c_str(), "wb");
        fwrite(&headtxt[0], headtxt.size(), 1, ff);

        fclose(ff);
        std::cout << " create model .h file: \033[1m\033[31m " << modelfileclasscpp << "\033[0m" << std::endl;
    }
    headtxt.clear();

    filebasename = basefilepath;
    if (filebasename.back() != '/')
    {
        filebasename.push_back('/');
    }
    filebasename.append("include/");

    filebasename.append(filebasefilename);
    filebasename.append("_base.h");

    FILE *f = fopen(filebasename.c_str(), "wb");
    if (!f)
    {
        std::cout << "file fail:" << filebasename << std::endl;
        return 0;
    }
    headtxt.append("ORM_");
    headtxt.append(rmstag);
    headtxt.append("_");
    headtxt.append(filebasefilename);
    headtxt.append("basemata_h");
    std::transform(headtxt.begin(), headtxt.end(), headtxt.begin(), ::toupper);

    filemodelstremcpp.str("");

    filemodelstremcpp << "#ifndef " << headtxt;
    filemodelstremcpp << "\n#define " << headtxt;
    filemodelstremcpp << "\n";
    headtxt.clear();
    headtxt.append(filemodelstremcpp.str());
    filemodelstremcpp.str("");

    // getgmtdatetime
    headtxt += R"(/*
*This file is auto create from paozhu_cli
*本文件为自动生成 )";
    headtxt.append(getgmtdatetime());
    headtxt += R"(
***/
#include <iostream>
#include <cstdio>
#include <sstream>
#include <array>
#include <map> 
#include <string_view> 
#include <string> 
#include <vector>
#include <ctime>
#include <array>
#include "unicode.h"

namespace orm { 
   
    )";
    if (rmstag != "default")
    {
        headtxt.append(" namespace ");
        headtxt.append(rmstag);
        headtxt.append(" { \n");
    }

    headtxt += R"(
struct )";
    headtxt.append(tablenamebase);
    headtxt += R"(_base
{
    struct meta{
    )";

    std::ostringstream filemodelstrem;

    for (unsigned int j = 0; j < metalist.size(); j++)
    {
        filemodelstrem << metalist[j] << std::endl;
    }
    filemodelstrem << " } data;\n ";
    if (ismeta_tree || tablefieldtree.size() > 0)
    {
        filemodelstrem << R"( 
        struct meta_tree{
        )";
        for (unsigned int j = 0; j < metalist.size(); j++)
        {
            filemodelstrem << metalist[j] << std::endl;
        }
        filemodelstrem << "\n\tstd::vector<meta_tree> children;\n };\n ";
    }

    filemodelstrem << "std::vector<" << tablenamebase << "_base::meta> record;\n";
    filemodelstrem << "std::string _rmstag=\"" << rmstag
                   << "\";//this value must be default or tag value, tag in mysqlconnect config file .\n";
    filemodelstrem << "unsigned int _offset=0;\n";
    //filemodelstrem << "MYSQL_ROW _row;\n";

    filemodelstrem << "std::vector<" << tablenamebase
                   << "_base::meta>::iterator begin(){     return record.begin(); }\n";
    filemodelstrem << "std::vector<" << tablenamebase << "_base::meta>::iterator end(){     return record.end(); }\n";
    filemodelstrem << "std::vector<" << tablenamebase
                   << "_base::meta>::const_iterator begin() const{     return record.begin(); }\n";
    filemodelstrem << "std::vector<" << tablenamebase
                   << "_base::meta>::const_iterator end() const{     return record.end(); }\n";

    // may be used to optimize the const static std::array<std::string,N> col_names={"xxx"};
    filemodelstrem << "static constexpr std::array<std::string_view," << std::to_string(table_column_info_lists.size()) << "> col_names={";

    for (unsigned int j = 0; j < table_column_info_lists.size(); j++)
    {
        if (j > 0)
        {
            filemodelstrem << ",";
        }
        filemodelstrem << "\"" << table_column_info_lists[j].col_name << "\"";
    }
    filemodelstrem << "};\r\n";

    filemodelstrem << "static constexpr std::array<unsigned char," << std::to_string(table_column_info_lists.size()) << "> col_types={";

    for (unsigned char j = 0; j < table_column_info_lists.size(); j++)
    {
        if (j > 0)
        {
            filemodelstrem << ",";
        }
        filemodelstrem << std::to_string(table_column_info_lists[j].col_type);
    }
    filemodelstrem << "};\r\n";

    filemodelstrem << "static constexpr std::array<unsigned char," << std::to_string(table_column_info_lists.size()) << "> col_length={";

    for (unsigned char j = 0; j < table_column_info_lists.size(); j++)
    {
        if (j > 0)
        {
            filemodelstrem << ",";
        }
        if(table_column_info_lists[j].col_length>255)
        {
            filemodelstrem << "0";
        }
        else
        {
            filemodelstrem << std::to_string(table_column_info_lists[j].col_length);
        }
    }
    filemodelstrem << "};\r\n";

    filemodelstrem << "static constexpr std::array<unsigned char," << std::to_string(table_column_info_lists.size()) << "> col_decimals={";

    for (unsigned char j = 0; j < table_column_info_lists.size(); j++)
    {
        if (j > 0)
        {
            filemodelstrem << ",";
        }
        filemodelstrem << std::to_string(table_column_info_lists[j].decimals);
    }
    filemodelstrem << "};\r\n";

    headtxt.append(filemodelstrem.str());
    filemodelstrem.str("");
    // 头部定义

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    headtxt = "std::string tablename=\"";
    headtxt.append(table_name);
    headtxt += "\";\n";
    headtxt.append("static constexpr std::string_view modelname=\"");
    headtxt.append(model_name_obj);
    headtxt.append("\";\n");

    bool iscolpospppc = false;
    filemodelstrem.str("");
    std::map<char, std::vector<unsigned char>> alpaz;
    for (unsigned char j = 0; j < tablecollist.size(); j++)
    {
        char taa = tablecollist[j][0];
        alpaz[taa].emplace_back(j);
    }
    for (auto itter = alpaz.begin(); itter != alpaz.end(); itter++)
    {
        filemodelstrem << "case '" << itter->first << "':\n";
        if (itter->second.size() == 1)
        {
            filemodelstrem << "   \t return " << std::to_string(itter->second[0]) << ";\nbreak;\n";
        }
        else
        {
            filemodelstrem << " switch(coln.size()){  \n";
            std::map<unsigned char, std::vector<unsigned char>> sizecolz;

            for (unsigned int m = 0; m < itter->second.size(); m++)
            {

                unsigned char taa = tablecollist[itter->second[m]].size();
                sizecolz[taa].emplace_back(itter->second[m]);
            }
            for (auto ittter = sizecolz.begin(); ittter != sizecolz.end(); ittter++)
            {
                filemodelstrem << "case " << std::to_string(ittter->first) << ":\n";
                if (ittter->second.size() == 1)
                {
                    filemodelstrem << "   \t return " << std::to_string(ittter->second[0]) << ";\nbreak;\n";
                }
                else
                {
                    char ttttp = 0x00;
                    std::map<unsigned char, std::vector<unsigned char>> backcolz;
                    for (unsigned int mm = 0; mm < ittter->second.size(); mm++)
                    {
                        ttttp = tablecollist[ittter->second[mm]].back();
                        backcolz[ttttp].emplace_back(ittter->second[mm]);
                    }
                    std::ostringstream backcachep;

                    backcachep << "  colpospppc=coln.back();\n    if(colpospppc<91){ colpospppc+=32; }\n";
                    bool isbackppp = false;
                    std::map<unsigned char, unsigned char> hasbackpos;
                    for (auto ipper = backcolz.begin(); ipper != backcolz.end(); ipper++)
                    {

                        if (ipper->second.size() == 1)
                        {
                            backcachep << " if(colpospppc=='" << tablecollist[ipper->second[0]].back() << "'){ return "
                                       << std::to_string(ipper->second[0]) << "; }\n";
                            isbackppp                    = true;
                            hasbackpos[ipper->second[0]] = 1;
                        }
                    }
                    if (isbackppp)
                    {
                        iscolpospppc = true;
                        filemodelstrem << backcachep.str();
                    }
                    std::vector<std::pair<unsigned char, std::string>> samesizecols;
                    for (unsigned int mm = 0; mm < ittter->second.size(); mm++)
                    {
                        auto ippter = hasbackpos.find(ittter->second[mm]);
                        if (ippter == hasbackpos.end())
                        {
                            samesizecols.push_back({ittter->second[mm], tablecollist[ittter->second[mm]]});
                        }
                    }
                    //first and last same
                    if (samesizecols.size() > 0)
                    {
                        unsigned char pos_char    = 0x00;
                        unsigned char colnamesize = samesizecols[0].second.size();
                        std::map<unsigned char, unsigned char> butong_total;
                        std::map<unsigned char, unsigned char> butong_pos;
                        std::map<unsigned char, unsigned char> butong_has;
                        for (unsigned char ii = 0; ii < samesizecols.size(); ii++)
                        {
                            butong_has[(unsigned char)samesizecols[ii].first] = 0;
                        }
                        for (unsigned char iii = 0; iii < colnamesize; iii++)
                        {
                            butong_total.clear();
                            butong_pos.clear();
                            for (unsigned char ii = 0; ii < samesizecols.size(); ii++)
                            {
                                if (samesizecols[ii].second.size() > iii)
                                {
                                    if (butong_has[(unsigned char)samesizecols[ii].first] == 0)
                                    {
                                        pos_char               = (unsigned char)samesizecols[ii].second[iii];
                                        butong_total[pos_char] = butong_total[pos_char] + 1;
                                        butong_pos[pos_char]   = (unsigned char)samesizecols[ii].first;
                                    }
                                }
                            }
                            for (auto &[indexchar, samenum] : butong_total)
                            {
                                if (samenum == 1 && butong_has[butong_pos[indexchar]] == 0)
                                {
                                    butong_has[butong_pos[indexchar]] = 1;
                                    filemodelstrem << " if(coln.size()>" << std::to_string(iii) << "&&(coln[" << std::to_string(iii) << "]=='" << (char)indexchar << "'||coln[" << std::to_string(iii) << "]=='" << (char)(indexchar - 32) << "')){ return " << std::to_string(butong_pos[indexchar]) << "; }\n";
                                }
                            }
                        }
                    }

                    filemodelstrem << "   \t break;\n";
                }
            }

            filemodelstrem << " }\n break;\n";
        }
    }

    headtxt += R"(
	  unsigned char findcolpos(const std::string &coln){
            if(coln.size()==0)
            {
                return 255;
            }
		    unsigned char  bi=coln[0];
         )";
    if (iscolpospppc)
    {
        headtxt.append("char colpospppc;");
    }

    headtxt += R"(

	         if(bi<91&&bi>64){
				bi+=32;
			}
            switch(coln[0]){


         )";
    headtxt.append(filemodelstrem.str());
    filemodelstrem.str("");

    headtxt += R"(
             }
             return 255;
           }
         )";
    headtxt += R"(
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return ")";
    headtxt.append(tablepkname);
    headtxt.append("\";\n}\n");

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
      void record_reset()
      {
            record.clear();     
      })";
    headtxt += R"(
      void data_reset(){
     )";
    headtxt += tablenamebase;
    headtxt += R"(_base::meta metatemp;    
            data = metatemp; 
      }
      )";
    headtxt += R"(
      std::string soft_remove_sql([[maybe_unused]] const std::string &fieldsql){
          std::string temp;
     )";
    modelfileclasscpp.clear();
    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {
        if (tablecollist[j] == "isdelete" || tablecollist[j] == "deleted" || tablecollist[j] == "isdeleted" || tablecollist[j] == "is_deleted" || tablecollist[j] == "is_delete" || tablecollist[j] == "order_deleted" || tablecollist[j] == "order_delete")
        {
            if (modelfileclasscpp.size() > 0)
            {
                modelfileclasscpp.append("\ttemp.push_back(',');\n\t");
            }
            modelfileclasscpp.append("\ttemp.push_back('`');\n\t");
            modelfileclasscpp.append("\ttemp.append(\"");
            modelfileclasscpp.append(tablecollist[j]);
            modelfileclasscpp.append("\");\n\t");
            modelfileclasscpp.append("\ttemp.push_back('`');\n\t");
            modelfileclasscpp.append("\ttemp.append(\"=1 \");\n\t");
            modelfileclasscpp.append("\t if(fieldsql.size()>0){ data.");
            modelfileclasscpp.append(tablecollist[j]);
            modelfileclasscpp.append("=1; }\n\t");
        }
        else if (tablecollist[j] == "deletetime" || tablecollist[j] == "delete_time" || tablecollist[j] == "deletedtime" || tablecollist[j] == "deleted_time" || tablecollist[j] == "deleted_at" || tablecollist[j] == "delete_at")
        {
            if (modelfileclasscpp.size() > 0)
            {
                modelfileclasscpp.append("\ttemp.push_back(',');\n\t");
            }
            modelfileclasscpp.append("\ttemp.push_back('`');\n\t");
            modelfileclasscpp.append("\ttemp.append(\"");
            modelfileclasscpp.append(tablecollist[j]);
            modelfileclasscpp.append("\");\n\t");
            modelfileclasscpp.append("\ttemp.push_back('`');\n\t");
            modelfileclasscpp.append("\ttemp.append(\"=\");\n\t");
            modelfileclasscpp.append("\tunsigned int t=time((time_t *)NULL);\n\t");
            modelfileclasscpp.append("\ttemp.append(std::to_string(t));\n\t");
            modelfileclasscpp.append("\tif(fieldsql.size()>0){ data.");
            modelfileclasscpp.append(tablecollist[j]);
            modelfileclasscpp.append("=t; }\n\t");
        }
    }
    if (modelfileclasscpp.size() > 0)
    {
        headtxt += R"(
         if(fieldsql.size()<2)
         {
            temp="UPDATE `";
            temp.append(tablename);
            temp.push_back('`');
            temp.append(" SET ");
         }
         else
         {
            temp=_makeupdatesql(fieldsql);
            if(temp.size()>2)
            {
                if(temp.back()==0x20&&temp[temp.size()-2]=='T')
                {

                }
                else
                {
                    temp.push_back(',');
                }
            }
            
         }   
         
      )";
        headtxt.append(modelfileclasscpp);
    }
    //
    headtxt += R"(
         return temp;
     }
     )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(

  inline  std::string stringaddslash(const std::string &content){
        std::string temp;
        for(unsigned int i=0;i<content.size();i++){
            if(content[i]=='\''){
                temp.append("\\'");
                continue;
            }else if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }else if(content[i]=='\\'){
                temp.append("\\\\");
                continue;
            }
            temp.push_back(content[i]);
        }
        return temp;
   }  
  inline  std::string jsonaddslash(const std::string &content){
        std::string temp;
        for(unsigned int i=0;i<content.size();i++){
            if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }
            else if(content[i]=='\\'){
                temp.append("\\\\");
                continue;
            }
            temp.push_back(content[i]);
        }
        return temp;
   }  

   std::string _makeinsertsql(){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    std::string insertstring;
    std::ostringstream insertstrem;

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {
        // 数字
        // if (j == 0)
        if (table_type[j] < 10 || table_type[j] == 246)
        {
            if (table_type[j] != 7)
            {
                if (j == 0)
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        insertstrem << "if(data." << tablecollist[j] << "==0){\n";
                        insertstrem << "tempsql<<\"null\";\n";
                        insertstrem << " }else{ \n";
                        insertstrem << "\ttempsql<<std::to_string(data." << tablecollist[j] << ");\n";
                        insertstrem << "}\n";
                    }
                    else
                    {
                        insertstrem << "if(data." << tablecollist[j] << "==0){\n";
                        insertstrem << "\ttempsql<<\"0\";\n";
                        insertstrem << " }else{ \n";
                        insertstrem << "\ttempsql<<std::to_string(data." << tablecollist[j] << ");\n";
                        insertstrem << "}\n";
                    }
                }
                else
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        insertstrem << "if(data." << tablecollist[j] << "==0){\n";
                        insertstrem << "tempsql<<\",null\";\n";
                        insertstrem << " }else{ \n";
                        insertstrem << "\ttempsql<<\",\"<<std::to_string(data." << tablecollist[j] << ");\n";
                        insertstrem << "}\n";
                    }
                    else
                    {
                        insertstrem << "if(data." << tablecollist[j] << "==0){\n";
                        insertstrem << "\ttempsql<<\",0\";\n";
                        insertstrem << " }else{ \n";
                        insertstrem << "\ttempsql<<\",\"<<std::to_string(data." << tablecollist[j] << ");\n";
                        insertstrem << "}\n";
                    }
                }

                continue;
            }
        }
        // 数字
        if (j == 0)
        {
            if (tablecollist[j] == tablepkname)
            {
                insertstrem << "if(data." << tablecollist[j] << "==0){\n";
                insertstrem << "tempsql<<\"null\";\n";
                insertstrem << " }else{ \n";
                insertstrem << "\ttempsql<<std::to_string(data." << tablecollist[j] << ");\n";
                insertstrem << "}\n";
            }
            else
            {
                if (colltypeshuzi[j] < 30)
                {
                    insertstrem << "if(data." << tablecollist[j] << "==0){\n";
                    insertstrem << "\ttempsql<<\"0\";\n";
                    insertstrem << " }else{ \n";
                    insertstrem << "\ttempsql<<\"\"<<std::to_string(data." << tablecollist[j] << ");\n";
                    insertstrem << "}\n";
                }
                else if (colltypeshuzi[j] == 60)
                {
                    insertstrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
                    insertstrem << "tempsql<<\" CURRENT_TIMESTAMP \";\n";
                    insertstrem << " }else{ \n tempsql<<\"'\"<<data." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else if (colltypeshuzi[j] == 61)
                {
                    insertstrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
                    insertstrem << "tempsql<<\"CURRENT_DATE \";\n";
                    insertstrem << " }else{ \n tempsql<<\"'\"<<data." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else
                {

                    insertstrem << "tempsql<<\"'\"<<stringaddslash(data." << tablecollist[j] << ")<<\"'\";\n";
                }
            }

            continue;
        }
        if (colltypeshuzi[j] < 30)
        {
            insertstrem << "if(data." << tablecollist[j] << "==0){\n";
            insertstrem << "\ttempsql<<\",0\";\n";
            insertstrem << " }else{ \n";
            insertstrem << "\ttempsql<<\",\"<<std::to_string(data." << tablecollist[j] << ");\n";
            insertstrem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            insertstrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            insertstrem << "tempsql<<\", CURRENT_TIMESTAMP \";\n";
            insertstrem << " }else{ \n tempsql<<\",'\"<<data." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            insertstrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            insertstrem << "tempsql<<\", CURRENT_DATE \";\n";
            insertstrem << " }else{ \n tempsql<<\",'\"<<data." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else
        {

            insertstrem << "tempsql<<\",'\"<<stringaddslash(data." << tablecollist[j] << ")<<\"'\";\n";
        }
    }
    insertstrem << "tempsql<<\")\";\n";
    headtxt.append(insertstrem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
     
       return tempsql.str();
   } 
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    // insert mate

    headtxt = R"(   
      std::string _makerecordinsertsql(const meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    insertstrem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        // if (j == 0)
        if (table_type[j] < 10 || table_type[j] == 246)
        {
            if (table_type[j] != 7)
            {
                if (j == 0)
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        insertstrem << "if(insert_data." << tablecollist[j] << "==0){\n";
                        insertstrem << "tempsql<<\"null\";\n";
                        insertstrem << " }else{ \n";
                        insertstrem << "\ttempsql<<std::to_string(insert_data." << tablecollist[j] << ");\n";
                        insertstrem << "}\n";
                    }
                    else
                    {
                        insertstrem << "if(insert_data." << tablecollist[j] << "==0){\n";
                        insertstrem << "\ttempsql<<\"0\";\n";
                        insertstrem << " }else{ \n";
                        insertstrem << "\ttempsql<<std::to_string(insert_data." << tablecollist[j] << ");\n";
                        insertstrem << "}\n";
                    }
                }
                else
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        insertstrem << "if(insert_data." << tablecollist[j] << "==0){\n";
                        insertstrem << "tempsql<<\",null\";\n";
                        insertstrem << " }else{ \n";
                        insertstrem << "\ttempsql<<\",\"<<std::to_string(insert_data." << tablecollist[j] << ");\n";
                        insertstrem << "}\n";
                    }
                    else
                    {
                        insertstrem << "if(insert_data." << tablecollist[j] << "==0){\n";
                        insertstrem << "\ttempsql<<\",0\";\n";
                        insertstrem << " }else{ \n";
                        insertstrem << "\ttempsql<<\",\"<<std::to_string(insert_data." << tablecollist[j] << ");\n";
                        insertstrem << "}\n";
                    }
                }

                continue;
            }
        }
        // 数字
        if (j == 0)
        {
            if (tablecollist[j] == tablepkname)
            {
                insertstrem << "if(insert_data." << tablecollist[j] << "==0){\n";
                insertstrem << "tempsql<<\"null\";\n";
                insertstrem << " }else{ \n";
                insertstrem << "\ttempsql<<std::to_string(insert_data." << tablecollist[j] << ");\n";
                insertstrem << "}\n";
            }
            else
            {
                if (colltypeshuzi[j] < 30)
                {
                    insertstrem << "if(insert_data." << tablecollist[j] << "==0){\n";
                    insertstrem << "\ttempsql<<\"0\";\n";
                    insertstrem << " }else{ \n";
                    insertstrem << "\ttempsql<<\"\"<<std::to_string(insert_data." << tablecollist[j] << ");\n";
                    insertstrem << "}\n";
                }
                else if (colltypeshuzi[j] == 60)
                {
                    insertstrem << "  \nif(insert_data." << tablecollist[j] << ".size()==0){ \n";
                    insertstrem << "tempsql<<\" CURRENT_TIMESTAMP \";\n";
                    insertstrem << " }else{ \n tempsql<<\"'\"<<insert_data." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else if (colltypeshuzi[j] == 61)
                {
                    insertstrem << "  \nif(insert_data." << tablecollist[j] << ".size()==0){ \n";
                    insertstrem << "tempsql<<\"CURRENT_DATE \";\n";
                    insertstrem << " }else{ \n tempsql<<\"'\"<<insert_data." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else
                {

                    insertstrem << "tempsql<<\"'\"<<stringaddslash(insert_data." << tablecollist[j] << ")<<\"'\";\n";
                }
            }

            continue;
        }
        if (colltypeshuzi[j] < 30)
        {
            insertstrem << "if(insert_data." << tablecollist[j] << "==0){\n";
            insertstrem << "\ttempsql<<\",0\";\n";
            insertstrem << " }else{ \n";
            insertstrem << "\ttempsql<<\",\"<<std::to_string(insert_data." << tablecollist[j] << ");\n";
            insertstrem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            insertstrem << "  \nif(insert_data." << tablecollist[j] << ".size()==0){ \n";
            insertstrem << "tempsql<<\", CURRENT_TIMESTAMP \";\n";
            insertstrem << " }else{ \n tempsql<<\",'\"<<insert_data." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            insertstrem << "  \nif(insert_data." << tablecollist[j] << ".size()==0){ \n";
            insertstrem << "tempsql<<\", CURRENT_DATE \";\n";
            insertstrem << " }else{ \n tempsql<<\",'\"<<insert_data." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else
        {

            insertstrem << "tempsql<<\",'\"<<stringaddslash(insert_data." << tablecollist[j] << ")<<\"'\";\n";
        }
    }
    insertstrem << "tempsql<<\")\";\n";
    headtxt.append(insertstrem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
     
       return tempsql.str();
   } 
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt.clear();
    insertstrem.str("");
    // insert mate array

    headtxt = R"(   
    std::string _makerecordinsertsql(const std::vector<meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES ";

        for(unsigned int i=0;i<insert_data.size();i++)
        {
            if(i>0)
            {
                tempsql<<",";	
            }
            tempsql<<"(";


            )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    insertstrem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        // if (j == 0)
        if (table_type[j] < 10 || table_type[j] == 246)
        {
            if (table_type[j] != 7)
            {
                if (j == 0)
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        insertstrem << "\tif(insert_data[i]." << tablecollist[j] << "==0){\n";
                        insertstrem << "\ttempsql<<\"null\";\n";
                        insertstrem << "\t }else{ \n";
                        insertstrem << "\ttempsql<<std::to_string(insert_data[i]." << tablecollist[j] << ");\n";
                        insertstrem << "\t}\n";
                    }
                    else
                    {
                        insertstrem << "\tif(insert_data[i]." << tablecollist[j] << "==0){\n";
                        insertstrem << "\ttempsql<<\"0\";\n";
                        insertstrem << "\t }else{ \n";
                        insertstrem << "\ttempsql<<std::to_string(insert_data[i]." << tablecollist[j] << ");\n";
                        insertstrem << "\t}\n";
                    }
                }
                else
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        insertstrem << "\tif(insert_data[i]." << tablecollist[j] << "==0){\n";
                        insertstrem << "\ttempsql<<\",null\";\n";
                        insertstrem << "\t }else{ \n";
                        insertstrem << "\ttempsql<<\",\"<<std::to_string(insert_data[i]." << tablecollist[j] << ");\n";
                        insertstrem << "\t}\n";
                    }
                    else
                    {
                        insertstrem << "\tif(insert_data[i]." << tablecollist[j] << "==0){\n";
                        insertstrem << "\ttempsql<<\",0\";\n";
                        insertstrem << "\t }else{ \n";
                        insertstrem << "\ttempsql<<\",\"<<std::to_string(insert_data[i]." << tablecollist[j] << ");\n";
                        insertstrem << "\t}\n";
                    }
                }

                continue;
            }
        }
        // 数字
        if (j == 0)
        {
            if (tablecollist[j] == tablepkname)
            {
                insertstrem << "\tif(insert_data[i]." << tablecollist[j] << "==0){\n";
                insertstrem << "\ttempsql<<\"null\";\n";
                insertstrem << "\t }else{ \n";
                insertstrem << "\ttempsql<<std::to_string(insert_data[i]." << tablecollist[j] << ");\n";
                insertstrem << "\t}\n";
            }
            else
            {
                if (colltypeshuzi[j] < 30)
                {
                    insertstrem << "\tif(insert_data[i]." << tablecollist[j] << "==0){\n";
                    insertstrem << "\ttempsql<<\"0\";\n";
                    insertstrem << "\t }else{ \n";
                    insertstrem << "\ttempsql<<\"\"<<std::to_string(insert_data[i]." << tablecollist[j] << ");\n";
                    insertstrem << "\t}\n";
                }
                else if (colltypeshuzi[j] == 60)
                {
                    insertstrem << "  \nif(insert_data[i]." << tablecollist[j] << ".size()==0){ \n";
                    insertstrem << "tempsql<<\" CURRENT_TIMESTAMP \";\n";
                    insertstrem << " }else{ \n tempsql<<\"'\"<<insert_data[i]." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else if (colltypeshuzi[j] == 61)
                {
                    insertstrem << "  \n\tif(insert_data[i]." << tablecollist[j] << ".size()==0){ \n";
                    insertstrem << "\ttempsql<<\"CURRENT_DATE \";\n";
                    insertstrem << "\t }else{ \n tempsql<<\"'\"<<insert_data[i]." << tablecollist[j]
                                << "<<\"'\";\n }\n";
                }
                else
                {

                    insertstrem << "\ttempsql<<\"'\"<<stringaddslash(insert_data[i]." << tablecollist[j]
                                << ")<<\"'\";\n";
                }
            }

            continue;
        }
        if (colltypeshuzi[j] < 30)
        {
            insertstrem << "\tif(insert_data[i]." << tablecollist[j] << "==0){\n";
            insertstrem << "\ttempsql<<\",0\";\n";
            insertstrem << "\t }else{ \n";
            insertstrem << "\ttempsql<<\",\"<<std::to_string(insert_data[i]." << tablecollist[j] << ");\n";
            insertstrem << "\t}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            insertstrem << "  \n\t\tif(insert_data[i]." << tablecollist[j] << ".size()==0){ \n";
            insertstrem << "\t\ttempsql<<\", CURRENT_TIMESTAMP \";\n";
            insertstrem << "\t\t }else{ \n tempsql<<\",'\"<<insert_data[i]." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            insertstrem << "  \n\t\tif(insert_data[i]." << tablecollist[j] << ".size()==0){ \n";
            insertstrem << "\t\ttempsql<<\", CURRENT_DATE \";\n";
            insertstrem << "\t\t }else{ \n tempsql<<\",'\"<<insert_data[i]." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else
        {

            insertstrem << "\t\ttempsql<<\",'\"<<stringaddslash(insert_data[i]." << tablecollist[j] << ")<<\"'\";\n";
        }
    }
    insertstrem << "\t\ttempsql<<\")\";\n\t } \n";
    headtxt.append(insertstrem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
     
       return tempsql.str();
   } 
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    ///////////////////
    // update sql
    headtxt.clear();
    headtxt = R"(   
    std::string _makeupdatesql(const std::string &fileld){
        std::ostringstream tempsql;
        tempsql<<"UPDATE ";
        tempsql<<tablename;
        tempsql<<" SET ";

        bool isall=false;
        if(fileld.empty()){
            isall=true;
        }
        if(isall){

        )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    std::string updatestring;
    std::ostringstream updatestrem;

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        if (colltypeshuzi[j] < 30)
        {
            updatestrem << "if(data." << tablecollist[j] << "==0){\n";
            if (j > 0)
            {
                updatestrem << "\ttempsql<<\",`" << tablecollist[j] << "`=0\";\n";
            }
            else
            {
                updatestrem << "\ttempsql<<\"`" << tablecollist[j] << "`=0\";\n";
            }
            updatestrem << " }else{ \n";
            if (j > 0)
            {
                updatestrem << "\ttempsql<<\",`" << tablecollist[j] << "`=\"<<std::to_string(data." << tablecollist[j]
                            << ");\n";
            }
            else
            {
                updatestrem << "\ttempsql<<\"`" << tablecollist[j] << "`=\"<<std::to_string(data." << tablecollist[j]
                            << ");\n";
            }

            updatestrem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            updatestrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            updatestrem << "tempsql<<\",`" << tablecollist[j] << "`=CURRENT_TIMESTAMP\";\n";
            updatestrem << " }else{ \n tempsql<<\",`" << tablecollist[j] << "'='\"<<data." << tablecollist[j]
                        << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            updatestrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            updatestrem << "tempsql<<\",`" << tablecollist[j] << "`=CURRENT_DATE\";\n";
            updatestrem << " }else{ \n tempsql<<\",`" << tablecollist[j] << "'='\"<<data." << tablecollist[j]
                        << "<<\"'\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                updatestrem << "tempsql<<\",`" << tablecollist[j] << "`='\"<<stringaddslash(data." << tablecollist[j]
                            << ")<<\"'\";\n";
            }
            else
            {
                updatestrem << "tempsql<<\"`" << tablecollist[j] << "`='\"<<stringaddslash(data." << tablecollist[j]
                            << ")<<\"'\";\n";
            }
        }
    }
    headtxt.append(updatestrem.str());

    headtxt.append(" }else{ \n");

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
     
  unsigned int jj=0;
                  std::string keyname;
                  std::vector<unsigned char> keypos;
                  for(;jj<fileld.size();jj++){
                        if(fileld[jj]==','){
                                unsigned char bpos_i=findcolpos(keyname);
                               keypos.emplace_back(bpos_i); 
#ifdef DEBUG
                    if (bpos_i == 255)
                    {
                        std::cout << "\033[1m\033[31m-----------\n"
                                  << keyname << " not in " << tablename << " table Field.\n-----------\033[0m"
                                  << std::endl;
                    }
#endif                               
                               keyname.clear();
                             continue;   
                        }
                        if(fileld[jj]==0x20){

                             continue;   
                        }
                        keyname.push_back(fileld[jj]);

                  }  
                 if(keyname.size()>0){
                                unsigned char bpos_i=findcolpos(keyname);
 #ifdef DEBUG
                    if (bpos_i == 255)
                    {
                        std::cout << "\033[1m\033[31m-----------\n"
                                  << keyname << " not in " << tablename << " table Field.\n-----------\033[0m"
                                  << std::endl;
                    }
#endif                                       
                                keypos.emplace_back(bpos_i); 
                                keyname.clear();
                 }
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){

        )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    std::string update2string;
    std::ostringstream update2strem;

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        update2strem << " case " << std::to_string(j) << ":\n";
        update2strem << " if(jj>0){ tempsql<<\",\"; } \n";
        if (colltypeshuzi[j] < 30)
        {
            update2strem << "if(data." << tablecollist[j] << "==0){\n";
            if (j > 0)
            {
                update2strem << "\ttempsql<<\"`" << tablecollist[j] << "`=0\";\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"`" << tablecollist[j] << "`=0\";\n";
            }
            update2strem << " }else{ \n";
            if (j > 0)
            {
                update2strem << "\ttempsql<<\"`" << tablecollist[j] << "`=\"<<std::to_string(data." << tablecollist[j]
                             << ");\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"`" << tablecollist[j] << "`=\"<<std::to_string(data." << tablecollist[j]
                             << ");\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"`" << tablecollist[j] << "`=CURRENT_TIMESTAMP\";\n";
            update2strem << " }else{ \n tempsql<<\",`" << tablecollist[j] << "'='\"<<data." << tablecollist[j]
                         << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"`" << tablecollist[j] << "`=CURRENT_DATE\";\n";
            update2strem << " }else{ \n tempsql<<\"`" << tablecollist[j] << "'='\"<<data." << tablecollist[j]
                         << "<<\"'\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "tempsql<<\"`" << tablecollist[j] << "`='\"<<stringaddslash(data." << tablecollist[j]
                             << ")<<\"'\";\n";
            }
            else
            {
                update2strem << "tempsql<<\"`" << tablecollist[j] << "`='\"<<stringaddslash(data." << tablecollist[j]
                             << ")<<\"'\";\n";
            }
        }
        update2strem << " break;\n";
    }
    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
     
                  default:
                                ;
                     }
                 }   

            }        

        return tempsql.str();
   } 
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    ////////////////////////////////////////////////////////////////////////////////////
    //batch update
    headtxt.clear();
    update2strem.str("");
    headtxt = R"(
    std::string _make_replace_into_sql()
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
        tempsql << "REPLACE INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << col_names[j];
        }
        if (j > 0)
        {
            tempsql << "`";
        }
        tempsql << ") VALUES ";

        for (unsigned int i = 0; i < record.size(); i++)
        {
            if (i > 0)
            {
                tempsql << ",\n";
            }
            tempsql << "(";
            )";

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {
        // 数字
        // if (j == 0)
        if (table_type[j] < 10 || table_type[j] == 246)
        {
            if (table_type[j] != 7)
            {
                if (j == 0)
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                        update2strem << "\ttempsql<<\"null\";\n";
                        update2strem << "\t }else{ \n";
                        update2strem << "\ttempsql<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                        update2strem << "\t}\n";
                    }
                    else
                    {
                        update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                        update2strem << "\ttempsql<<\"0\";\n";
                        update2strem << "\t }else{ \n";
                        update2strem << "\ttempsql<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                        update2strem << "\t}\n";
                    }
                }
                else
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                        update2strem << "\ttempsql<<\",null\";\n";
                        update2strem << "\t }else{ \n";
                        update2strem << "\ttempsql<<\",\"<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                        update2strem << "\t}\n";
                    }
                    else
                    {
                        update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                        update2strem << "\ttempsql<<\",0\";\n";
                        update2strem << "\t }else{ \n";
                        update2strem << "\ttempsql<<\",\"<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                        update2strem << "\t}\n";
                    }
                }
                continue;
            }
        }
        // 数字
        if (j == 0)
        {
            if (tablecollist[j] == tablepkname)
            {
                update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                update2strem << "\ttempsql<<\"null\";\n";
                update2strem << "\t }else{ \n";
                update2strem << "\ttempsql<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                update2strem << "\t}\n";
            }
            else
            {
                if (colltypeshuzi[j] < 30)
                {
                    update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                    update2strem << "\ttempsql<<\"0\";\n";
                    update2strem << "\t }else{ \n";
                    update2strem << "\ttempsql<<\"\"<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                    update2strem << "\t}\n";
                }
                else if (colltypeshuzi[j] == 60)
                {
                    update2strem << "  \n\tif(record[i]." << tablecollist[j] << ".size()==0){ \n";
                    update2strem << "\ttempsql<<\" CURRENT_TIMESTAMP \";\n";
                    update2strem << "\t }else{ \n tempsql<<\"'\"<<record[i]." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else if (colltypeshuzi[j] == 61)
                {
                    update2strem << "  \n\tif(record[i]." << tablecollist[j] << ".size()==0){ \n";
                    update2strem << "\ttempsql<<\"CURRENT_DATE \";\n";
                    update2strem << "\t }else{ \n tempsql<<\"'\"<<record[i]." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else
                {

                    update2strem << "\ttempsql<<\"'\"<<stringaddslash(record[i]." << tablecollist[j] << ")<<\"'\";\n";
                }
            }
            continue;
        }
        if (colltypeshuzi[j] < 30)
        {
            update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
            update2strem << "\ttempsql<<\",0\";\n";
            update2strem << "\t }else{ \n";
            update2strem << "\ttempsql<<\",\"<<std::to_string(record[i]." << tablecollist[j] << ");\n";
            update2strem << "\t}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \n\tif(record[i]." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "\ttempsql<<\", CURRENT_TIMESTAMP \";\n";
            update2strem << "\t }else{ \n tempsql<<\",'\"<<record[i]." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \n\tif(record[i]." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "\ttempsql<<\", CURRENT_DATE \";\n";
            update2strem << "\t }else{ \n tempsql<<\",'\"<<record[i]." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else
        {

            update2strem << "\ttempsql<<\",'\"<<stringaddslash(record[i]." << tablecollist[j] << ")<<\"'\";\n";
        }
    }
    update2strem << "\ttempsql<<\")\";\n";
    update2strem << "\n }\n return tempsql.str();\n}\n";

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    update2strem.str("");

    ////////////////////////////////////////////////////////////////////////////////////
    //batch update
    headtxt.clear();
    update2strem.str("");
    headtxt = R"(
    std::string _make_insert_into_sql(const std::string &fileld)
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
        tempsql << "INSERT INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << col_names[j];
        }
        if (j > 0)
        {
            tempsql << "`";
        }
        tempsql << ") VALUES ";

        for (unsigned int i = 0; i < record.size(); i++)
        {
            if (i > 0)
            {
                tempsql << ",\n";
            }
            tempsql << "(";
            )";

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {
        // 数字
        // if (j == 0)
        if (table_type[j] < 10 || table_type[j] == 246)
        {
            if (table_type[j] != 7)
            {
                if (j == 0)
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                        update2strem << "\ttempsql<<\"null\";\n";
                        update2strem << "\t }else{ \n";
                        update2strem << "\ttempsql<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                        update2strem << "\t}\n";
                    }
                    else
                    {
                        update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                        update2strem << "\ttempsql<<\"0\";\n";
                        update2strem << "\t }else{ \n";
                        update2strem << "\ttempsql<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                        update2strem << "\t}\n";
                    }
                }
                else
                {
                    if (tablecollist[j] == tablepkname)
                    {
                        update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                        update2strem << "\ttempsql<<\",null\";\n";
                        update2strem << "\t }else{ \n";
                        update2strem << "\ttempsql<<\",\"<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                        update2strem << "\t}\n";
                    }
                    else
                    {
                        update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                        update2strem << "\ttempsql<<\",0\";\n";
                        update2strem << "\t }else{ \n";
                        update2strem << "\ttempsql<<\",\"<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                        update2strem << "\t}\n";
                    }
                }
                continue;
            }
        }
        // 数字
        if (j == 0)
        {
            if (tablecollist[j] == tablepkname)
            {
                update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                update2strem << "\ttempsql<<\"null\";\n";
                update2strem << "\t }else{ \n";
                update2strem << "\ttempsql<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                update2strem << "\t}\n";
            }
            else
            {
                if (colltypeshuzi[j] < 30)
                {
                    update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
                    update2strem << "\ttempsql<<\"0\";\n";
                    update2strem << "\t }else{ \n";
                    update2strem << "\ttempsql<<\"\"<<std::to_string(record[i]." << tablecollist[j] << ");\n";
                    update2strem << "\t}\n";
                }
                else if (colltypeshuzi[j] == 60)
                {
                    update2strem << "  \n\tif(record[i]." << tablecollist[j] << ".size()==0){ \n";
                    update2strem << "\ttempsql<<\" CURRENT_TIMESTAMP \";\n";
                    update2strem << "\t }else{ \n tempsql<<\"'\"<<record[i]." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else if (colltypeshuzi[j] == 61)
                {
                    update2strem << "  \n\tif(record[i]." << tablecollist[j] << ".size()==0){ \n";
                    update2strem << "\ttempsql<<\"CURRENT_DATE \";\n";
                    update2strem << "\t }else{ \n tempsql<<\"'\"<<record[i]." << tablecollist[j] << "<<\"'\";\n }\n";
                }
                else
                {

                    update2strem << "\ttempsql<<\"'\"<<stringaddslash(record[i]." << tablecollist[j] << ")<<\"'\";\n";
                }
            }
            continue;
        }
        if (colltypeshuzi[j] < 30)
        {
            update2strem << "\tif(record[i]." << tablecollist[j] << "==0){\n";
            update2strem << "\ttempsql<<\",0\";\n";
            update2strem << "\t }else{ \n";
            update2strem << "\ttempsql<<\",\"<<std::to_string(record[i]." << tablecollist[j] << ");\n";
            update2strem << "\t}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \n\tif(record[i]." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "\ttempsql<<\", CURRENT_TIMESTAMP \";\n";
            update2strem << "\t }else{ \n tempsql<<\",'\"<<record[i]." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \n\tif(record[i]." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "\ttempsql<<\", CURRENT_DATE \";\n";
            update2strem << "\t }else{ \n tempsql<<\",'\"<<record[i]." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else
        {

            update2strem << "\ttempsql<<\",'\"<<stringaddslash(record[i]." << tablecollist[j] << ")<<\"'\";\n";
        }
    }
    update2strem << "\ttempsql<<\")\";\n";
    update2strem << "\t }\n\t tempsql<<\" as new ON DUPLICATE KEY UPDATE \";\n";
    headtxt.append(update2strem.str());

    headtxt += R"(
     
        std::string keyname;
        unsigned char jj=0;
        j=0;
        if(fileld.size()>0){
        for(;jj<fileld.size();jj++){
            if(fileld[jj]==','){
                if(findcolpos(keyname)<255)
                {
                    if(j>0)
                    {
                        tempsql<<",";
                    }
                    tempsql<<keyname;
                    tempsql<<"=new.";
                    tempsql<<keyname;
                }
                continue;   
            }
            if(fileld[jj]==0x20){
                continue;   
            }
            keyname.push_back(fileld[jj]);

        }  
        if(keyname.size()>0){
            if(findcolpos(keyname)<255)
            {
                if(j>0)
                {
                    tempsql<<",";
                }
                tempsql<<keyname;
                tempsql<<"=new.";
                tempsql<<keyname;
                
            }
        }

    } 
 )";

    headtxt.append("\n return tempsql.str();\n}\n");
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    update2strem.str("");

    /////////////////////////////////////////////////////////////////////////////////////

    headtxt = R"(
   std::vector<std::string> data_toarray(std::string fileld=""){
        std::vector<std::string> temparray;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>1){
            for(;jj<fileld.size();jj++){
                if(fileld[jj]==','){
                    keypos.emplace_back(findcolpos(keyname)); 
                    keyname.clear();
                    continue;   
                }
                if(fileld[jj]==0x20){

                    continue;   
                }
                keyname.push_back(fileld[jj]);

            }  
            if(keyname.size()>0){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
            }
        }else{
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
        )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    update2strem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        update2strem << " case " << std::to_string(j) << ":\n";

        if (colltypeshuzi[j] < 30)
        {
            update2strem << "if(data." << tablecollist[j] << "==0){\n";
            if (j > 0)
            {
                update2strem << "\ttemparray.push_back(\"0\");\n";
            }
            else
            {
                update2strem << "\ttemparray.push_back(\"0\");\n";
            }
            update2strem << " }else{ \n";
            if (j > 0)
            {
                update2strem << "\ttemparray.push_back(std::to_string(data." << tablecollist[j] << "));\n";
            }
            else
            {
                update2strem << "\ttemparray.push_back(std::to_string(data." << tablecollist[j] << "));\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "\ttemparray.push_back(\"0000-00-00 00:00:00\");\n";
            update2strem << " }else{ \n\t temparray.push_back(data." << tablecollist[j] << ");\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "\ttemparray.push_back(\"0000-00-00\");\n";
            update2strem << " }else{ \n\t temparray.push_back(data." << tablecollist[j] << ");\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "\ttemparray.push_back(data." << tablecollist[j] << ");\n";
            }
            else
            {
                update2strem << "\ttemparray.push_back(data." << tablecollist[j] << ");\n";
            }
        }
        update2strem << " break;\n";
    }

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
                             default:
                                ;
                     }
                 }   
   
     return temparray;             
   }   
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////////////////////////////////////////////////
    headtxt = R"(
   std::map<std::string,std::string> data_tomap(std::string fileld=""){
       std::map<std::string,std::string> tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>1){
        for(;jj<fileld.size();jj++){
            if(fileld[jj]==','){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
                continue;   
            }
            if(fileld[jj]==0x20){

                continue;   
            }
            keyname.push_back(fileld[jj]);

        }  
        if(keyname.size()>0){
            keypos.emplace_back(findcolpos(keyname)); 
            keyname.clear();
        }
        }else{
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
        )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    update2strem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        update2strem << " case " << std::to_string(j) << ":\n";

        if (colltypeshuzi[j] < 30)
        {
            update2strem << "if(data." << tablecollist[j] << "==0){\n";
            if (j > 0)
            {
                update2strem << "\ttempsql.insert({\"" << tablecollist[j] << "\",\"0\"});\n";
            }
            else
            {
                update2strem << "\ttempsql.insert({\"" << tablecollist[j] << "\",\"0\"});\n";
            }
            update2strem << " }else{ \n";
            if (j > 0)
            {
                update2strem << "\ttempsql.insert({\"" << tablecollist[j] << "\",std::to_string(data."
                             << tablecollist[j] << ")});\n";
            }
            else
            {
                update2strem << "\ttempsql.insert({\"" << tablecollist[j] << "\",std::to_string(data."
                             << tablecollist[j] << ")});\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "\ttempsql.insert({\"" << tablecollist[j] << "\",\"0000-00-00 00:00:00\"});\n";
            update2strem << " }else{ \n\t tempsql.insert({\"" << tablecollist[j] << "\",data." << tablecollist[j]
                         << "});\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "\ttempsql.insert({\"" << tablecollist[j] << "\",\"0000-00-00\"});\n";
            update2strem << " }else{ \n\t tempsql.insert({\"" << tablecollist[j] << "\",data." << tablecollist[j]
                         << "});\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "\ttempsql.insert({\"" << tablecollist[j] << "\",data." << tablecollist[j] << "});\n";
            }
            else
            {
                update2strem << "\ttempsql.insert({\"" << tablecollist[j] << "\",data." << tablecollist[j] << "});\n";
            }
        }
        update2strem << " break;\n";
    }

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
                             default:
                                ;
                     }
                 }   
    
     return tempsql;             
   }   
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    /////////////////////////////////////////////////////////////////////////////////////

    headtxt = R"(
   std::string data_tojson(){
       std::ostringstream tempsql;

        )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    std::ostringstream jsonstrem;
    headtxt = "tempsql<<\"{\";\n";
    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        if (colltypeshuzi[j] < 30)
        {
            jsonstrem << "if(data." << tablecollist[j] << "==0){\n";
            if (j > 0)
            {
                jsonstrem << "\ttempsql<<\",\\\"" << tablecollist[j] << "\\\":0\";\n";
            }
            else
            {
                jsonstrem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
            }
            jsonstrem << " }else{ \n";
            if (j > 0)
            {
                jsonstrem << "\ttempsql<<\",\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(data."
                          << tablecollist[j] << ");\n";
            }
            else
            {
                jsonstrem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(data."
                          << tablecollist[j] << ");\n";
            }
            // jsonstrem<<"\ttempsql<<\"}\";\n";

            jsonstrem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            jsonstrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            jsonstrem << "tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
            jsonstrem << " }else{ \n tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"\"<<data." << tablecollist[j]
                      << "<<\"\\\"\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            jsonstrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            jsonstrem << "tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
            jsonstrem << " }else{ \n tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"\"<<data." << tablecollist[j]
                      << "<<\"\\\"\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                jsonstrem << "tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(data."
                          << tablecollist[j] << ");\n";
            }
            else
            {
                jsonstrem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(data."
                          << tablecollist[j] << ");\n";
            }

            jsonstrem << "tempsql<<\"\\\"\";\n";
        }
    }
    headtxt.append(jsonstrem.str());
    headtxt.append("tempsql<<\"}\";\n");

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
     
     return tempsql.str();             
   }   
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////////
    headtxt = R"(
   std::string data_tojson(std::string fileld){
        std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>0){
        for(;jj<fileld.size();jj++){
            if(fileld[jj]==','){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
                continue;   
            }
            if(fileld[jj]==0x20){

                continue;   
            }
            keyname.push_back(fileld[jj]);

        }  
        if(keyname.size()>0){
            keypos.emplace_back(findcolpos(keyname)); 
            keyname.clear();
        }
        }else{
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
        )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    update2strem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        update2strem << " case " << std::to_string(j) << ":\n";
        update2strem << " if(jj>0){ tempsql<<\",\"; } \n";
        if (colltypeshuzi[j] < 30)
        {
            update2strem << "if(data." << tablecollist[j] << "==0){\n";
            if (j > 0)
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
            }
            update2strem << " }else{ \n";
            if (j > 0)
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(data."
                             << tablecollist[j] << ");\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(data."
                             << tablecollist[j] << ");\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<data." << tablecollist[j]
                         << "<<\"\\\"\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<data." << tablecollist[j]
                         << "<<\"\\\"\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(data."
                             << tablecollist[j] << ")<<\"\\\"\";\n";
            }
            else
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(data."
                             << tablecollist[j] << ")<<\"\\\"\";\n";
            }
        }
        update2strem << " break;\n";
    }

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
     return tempsql.str();             
   }   
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // from_json
    headtxt = R"(
    void from_json(const std::string &json_content)
   {
        record.clear();
        )";
    headtxt += tablenamebase;
    headtxt += R"(_base::meta metatemp; 
        data=metatemp;
        unsigned int json_offset=0;
        bool isarray=false;
        //std::vector<std::string> list_content;
        for(;json_offset<json_content.size();json_offset++)
        {
            if(json_content[json_offset]=='{')
            {
                break;
            }
            if(json_content[json_offset]=='[')
            {
                isarray=true;
                break;
            }
        }
        if(isarray)
        {
            json_offset+=1; 
            std::string json_key_name,json_value_name; 
            for(;json_offset<json_content.size();json_offset++)
            {
                for(;json_offset<json_content.size();json_offset++)
                {
                    if(json_content[json_offset]=='{')
                    {
                        json_offset+=1;
                        break;
                    }
                }
                if(record.size()>0)
                {
                    data=metatemp;
                }
                if(json_offset>=json_content.size())
                {
                    break;
                }
                for(;json_offset<json_content.size();json_offset++)
                {
    
                            if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                            {
                                continue;
                            }
                            else
                            {
                                if(json_content[json_offset]==0x22)
                                {
                                    unsigned int temp_offset=json_offset;
                                    json_key_name=http::jsonstring_to_utf8(&json_content[json_offset],json_content.size()-json_offset,temp_offset);
                                    json_offset=temp_offset;
                                    if(json_content[json_offset]==0x22)
                                    {
                                        json_offset+=1;
                                    }
                                    for(;json_offset<json_content.size();json_offset++)
                                    {
                                    
                                        if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                        {
                                            continue;
                                        }
                                        break;
                                    }       
                                    if(json_content[json_offset]!=':')
                                    {
                                        break;
                                    }
                                    for(;json_offset<json_content.size();json_offset++)
                                    {
                                        if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                        {
                                            continue;
                                        }
                                        break;
                                    } 
                                    json_offset+=1;
                                    if(json_offset>=json_content.size())
                                    {
                                        break;
                                    }
                                    json_value_name.clear();
                                    if(json_content[json_offset]==0x22)
                                    {
                                        
                                        temp_offset=json_offset;
                                        json_value_name=http::jsonstring_to_utf8(&json_content[json_offset],json_content.size()-json_offset,temp_offset);
                                        json_offset=temp_offset;
                                        if(json_content[json_offset]==0x22)
                                        {
                                            json_offset+=1;
                                        }
                                    }
                                    else
                                    {
                                        if(json_content[json_offset]!='{'&&json_content[json_offset]!=']')
                                        {
                                            for(;json_offset<json_content.size();json_offset++)
                                            {
                                                if(json_content[json_offset]==0x5D||json_content[json_offset]==0x7D||json_content[json_offset]==0x22||json_content[json_offset]==0x2C||json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                                {
                                                    if(json_content[json_offset]==0x7D)
                                                    {
                                                        json_offset-=1;
                                                    } 
                                                    break;
                                                }
                                                json_value_name.push_back(json_content[json_offset]);
                                            }   
                                        }
                                    }
                                    //////////////////////////
                                    set_val(json_key_name,json_value_name);
                                    continue;
                                }
                                else
                                {
                                    break;
                                }
                            }
    
                }
                record.emplace_back(data);
                
                json_offset+=1;
            }
            if(record.size()>1)
            {
                data=record[0];
            }
        }
        else
        {
           if(json_content[json_offset]=='{')
            {
                json_offset+=1; 
                std::string json_key_name,json_value_name; 
                 
                
                for(;json_offset<json_content.size();json_offset++)
                {
 
                        if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                        {
                            continue;
                        }
                        else
                        {
                            if(json_content[json_offset]==0x22)
                            {
                                 unsigned int temp_offset=json_offset;
                                 json_key_name=http::jsonstring_to_utf8(&json_content[json_offset],json_content.size()-json_offset,temp_offset);
                                 json_offset=temp_offset;
                                 if(json_content[json_offset]==0x22)
                                 {
                                    json_offset+=1;
                                 }
                                for(;json_offset<json_content.size();json_offset++)
                                {
                                
                                    if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                    {
                                        continue;
                                    }
                                    break;
                                }       
                                if(json_content[json_offset]!=':')
                                {
                                    break;
                                }
                                for(;json_offset<json_content.size();json_offset++)
                                {
                                    if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                    {
                                        continue;
                                    }
                                    break;
                                } 
                                json_offset+=1;
                                if(json_offset>=json_content.size())
                                {
                                    break;
                                }
                                json_value_name.clear();
                                if(json_content[json_offset]==0x22)
                                {
                                    
                                    temp_offset=json_offset;
                                    json_value_name=http::jsonstring_to_utf8(&json_content[json_offset],json_content.size()-json_offset,temp_offset);
                                    json_offset=temp_offset;
                                    if(json_content[json_offset]==0x22)
                                    {
                                        json_offset+=1;
                                    }
                                }
                                else
                                {
                                    if(json_content[json_offset]!='{'&&json_content[json_offset]!=']')
                                    {
                                        for(;json_offset<json_content.size();json_offset++)
                                        {
                                            if(json_content[json_offset]==0x5D||json_content[json_offset]==0x7D||json_content[json_offset]==0x22||json_content[json_offset]==0x2C||json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                            {
                                               if(json_content[json_offset]==0x7D)
                                               {
                                                   json_offset-=1;
                                               } 
                                               break;
                                            }
                                            json_value_name.push_back(json_content[json_offset]);
                                        }   
                                    }
                                }
                                //////////////////////////
                                set_val(json_key_name,json_value_name);
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }
 
                }
                record.emplace_back(data);
            } 
        }
   }   
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // set_val
    filemodelstrem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {
        if (table_type[j] < 10)
        {
            switch (table_type[j])
            {
            case 0:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=std::stof(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";
                break;
            case 1:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=std::stoi(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=std::stoi(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }

                break;
            case 2:

                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=std::stoi(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 3:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=std::stoul(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata."
                                   << tablecollist[j] << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=std::stoi(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                break;

            case 4:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=std::stof(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 5:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=std::stod(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 7:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << ".append(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 8:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=std::stoull(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata."
                                   << tablecollist[j] << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=std::stoll(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata."
                                   << tablecollist[j] << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                break;
            case 9:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=std::stoi(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
            }
        }
        else if (table_type[j] == 12)
        {
            // filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t
            // try{\n\t\t\tdata."<<tablecollist[j]<<"=std::stoul(set_value_name);\n\t\t}catch (...) {
            // \n\t\t\tdata."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << ".append(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";
        }
        else if (table_type[j] == 246)
        {
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << "=std::stof(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";
        }
        else
        {
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << ".append(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";
        }
    }

    filemodelstrem << "\tdefault:\n\t\t { }\n\t\t\t\n";

    headtxt = R"(
    void set_val(const std::string& set_key_name,const std::string& set_value_name)
    {
        switch(findcolpos(set_key_name))
        {
    )";
    headtxt.append(filemodelstrem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    filemodelstrem.str("");
    headtxt.clear();

    headtxt = R"(

        }
   } 
    )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    //////////////////////////////////////////////
    // set_val long long
    filemodelstrem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {
        if (table_type[j] < 10)
        {
            switch (table_type[j])
            {
            case 0:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";
                break;
            case 1:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }

                break;
            case 2:

                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 3:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                break;

            case 4:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 5:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=(double)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 7:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=std::to_string(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata."
                               << tablecollist[j] << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 8:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                break;
            case 9:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
            }
        }
        else if (table_type[j] == 12)
        {
            // filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t
            // try{\n\t\t\tdata."<<tablecollist[j]<<"=std::stoul(set_value_name);\n\t\t}catch (...) {
            // \n\t\t\tdata."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << "=std::to_string(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";
        }
        else if (table_type[j] == 246)
        {
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << "=(float)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";
        }
        else
        {
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << "=std::to_string(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";
        }
    }

    filemodelstrem << "\tdefault:\n\t\t { }\n\t\t\t\n";

    headtxt = R"(
    void set_val(const std::string& set_key_name,const long long set_value_name)
    {
        switch(findcolpos(set_key_name))
        {
    )";
    headtxt.append(filemodelstrem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    filemodelstrem.str("");
    headtxt.clear();

    headtxt = R"(

        }
   } 
    )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    //////////////////////////////////////////////
    // set_val
    filemodelstrem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {
        if (table_type[j] < 10)
        {
            switch (table_type[j])
            {
            case 0:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=(float)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";
                break;
            case 1:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=(int)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=(int)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }

                break;
            case 2:

                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=(int)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 3:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=(unsigned int)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata."
                                   << tablecollist[j] << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=(int)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                break;

            case 4:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=(float)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 5:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 7:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=std::to_string(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata."
                               << tablecollist[j] << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";

                break;
            case 8:
                if (table_type_unsigned[j] == 1)
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=(unsigned long long)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata."
                                   << tablecollist[j] << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                else
                {
                    filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                                   << "=(long long)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                                   << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
                }
                break;
            case 9:
                filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                               << "=(int)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                               << "=0;\n\t\t\t }\n\t\t\tbreak;\n";
            }
        }
        else if (table_type[j] == 12)
        {
            // filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t
            // try{\n\t\t\tdata."<<tablecollist[j]<<"=std::stoul(set_value_name);\n\t\t}catch (...) {
            // \n\t\t\tdata."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << "=std::to_string(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";
        }
        else if (table_type[j] == 246)
        {
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << "=(float)set_value_name;\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << "=0.0;\n\t\t\t }\n\t\t\tbreak;\n";
        }
        else
        {
            filemodelstrem << "\t\tcase " << std::to_string(j) << ":\n\t\t try{\n\t\t\tdata." << tablecollist[j]
                           << "=std::to_string(set_value_name);\n\t\t}catch (...) { \n\t\t\tdata." << tablecollist[j]
                           << ".clear();\n\t\t\t }\n\t\t\tbreak;\n";
        }
    }

    filemodelstrem << "\tdefault:\n\t\t { }\n\t\t\t\n";

    headtxt = R"(
    void set_val(const std::string& set_key_name,const double set_value_name)
    {
        switch(findcolpos(set_key_name))
        {
    )";
    headtxt.append(filemodelstrem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    filemodelstrem.str("");
    headtxt.clear();

    headtxt = R"(

        }
   } 
    )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////////
    headtxt = R"(
   std::string to_json(std::string fileld=""){
    std::ostringstream tempsql;
    std::string keyname;
    unsigned char jj=0;
    std::vector<unsigned char> keypos;
    if(fileld.size()>0){
        for(;jj<fileld.size();jj++){
            if(fileld[jj]==','){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
                continue;   
            }
            if(fileld[jj]==0x20){

                continue;   
            }
            keyname.push_back(fileld[jj]);

        }  
        if(keyname.size()>0){
            keypos.emplace_back(findcolpos(keyname)); 
            keyname.clear();
        }
    }else{
        for(jj=0;jj<col_names.size();jj++){
            keypos.emplace_back(jj); 
        }
    }
    tempsql<<"[";
    for(size_t n=0;n<record.size();n++){
        if(n>0){
            tempsql<<",{";
        }else{
            tempsql<<"{";
        }  
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
        )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    update2strem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        update2strem << " case " << std::to_string(j) << ":\n";
        update2strem << " if(jj>0){ tempsql<<\",\"; } \n";
        if (colltypeshuzi[j] < 30)
        {
            update2strem << "if(record[n]." << tablecollist[j] << "==0){\n";
            if (j > 0)
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
            }
            update2strem << " }else{ \n";
            if (j > 0)
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(record[n]."
                             << tablecollist[j] << ");\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(record[n]."
                             << tablecollist[j] << ");\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<record[n]."
                         << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<record[n]."
                         << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j]
                             << "\\\":\\\"\"<<http::utf8_to_jsonstring(record[n]." << tablecollist[j]
                             << ")<<\"\\\"\";\n";
            }
            else
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j]
                             << "\\\":\\\"\"<<http::utf8_to_jsonstring(record[n]." << tablecollist[j]
                             << ")<<\"\\\"\";\n";
            }
        }
        update2strem << " break;\n";
    }

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    ///////////////////////////////////////////////
    headtxt = R"(
   std::string to_json(std::function<bool(std::string&,meta&)> func,std::string fileld=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>0){
            for(;jj<fileld.size();jj++){
                if(fileld[jj]==','){
                    keypos.emplace_back(findcolpos(keyname)); 
                    keyname.clear();
                    continue;   
                }
                if(fileld[jj]==0x20){

                    continue;   
                }
                keyname.push_back(fileld[jj]);

            }  
            if(keyname.size()>0){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
            }
        }else{
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"[";
        for(size_t n=0;n<record.size();n++){
            keyname.clear();
            if(func(keyname,record[n])){ 
                if(n>0){
                    tempsql<<",{";
                }else{
                    tempsql<<"{";
                } 
                tempsql<<keyname;
            }else{
            continue;
            } 
        
        for(jj=0;jj<keypos.size();jj++){
            
            switch(keypos[jj]){
        )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    update2strem.str("");

    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        update2strem << " case " << std::to_string(j) << ":\n";
        update2strem << " if(jj>0){ tempsql<<\",\"; } \n";

        if (colltypeshuzi[j] < 30)
        {
            update2strem << "if(record[n]." << tablecollist[j] << "==0){\n";
            if (j > 0)
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
            }
            update2strem << " }else{ \n";
            if (j > 0)
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(record[n]."
                             << tablecollist[j] << ");\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(record[n]."
                             << tablecollist[j] << ");\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<record[n]."
                         << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<record[n]."
                         << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j]
                             << "\\\":\\\"\"<<http::utf8_to_jsonstring(record[n]." << tablecollist[j]
                             << ")<<\"\\\"\";\n";
            }
            else
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j]
                             << "\\\":\\\"\"<<http::utf8_to_jsonstring(record[n]." << tablecollist[j]
                             << ")<<\"\\\"\";\n";
            }
        }
        update2strem << " break;\n";
    }

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    //////////////////////////////////////////////////////////
    if (tablepkname.empty())
    {
        if (tablepriname.size() > 0)
        {
            headtxt = "long long getPK(){  return data." + tablepriname + "; } \n";
        }
        else
        {
            headtxt = "long long getPK(){  return 0; } \n";
        }
        headtxt.append(" void setPK([[maybe_unused]] long long val){   } \n");
    }
    else
    {
        headtxt = "long long getPK(){  return data." + tablepkname + "; } \n";
        headtxt.append(" void setPK(long long val){  data." + tablepkname + "=val;} \n");
    }

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    std::ostringstream getsetstrem;
    std::string uptempstring;
    // 自动get set方法
    for (unsigned int j = 0; j < tablecollist.size(); j++)
    {
        getsetstrem << collisttype[j];
        uptempstring     = tablecollist[j];
        unsigned int re  = 0;
        bool isupperchar = false;
        for (unsigned int n = 0; n < uptempstring.size(); n++)
        {
            if (uptempstring[n] == '_')
            {
                isupperchar = true;
                continue;
            }
            if (isupperchar)
            {
                if (uptempstring[n] >= 'a' && uptempstring[n] <= 'z')
                {
                    uptempstring[re] = uptempstring[n] - 32;
                }
                else
                {
                    uptempstring[re] = uptempstring[n];
                }
                isupperchar = false;
            }
            else
            {
                uptempstring[re] = uptempstring[n];
            }
            re++;
        }
        uptempstring.resize(re);

        if (uptempstring[0] >= 'a' && uptempstring[0] <= 'z')
        {
            uptempstring[0] = uptempstring[0] - 32;
        }

        getsetstrem << " get" << uptempstring << "(){  return data." + tablecollist[j] + "; } \n";

        if (colltypeshuzi[j] > 29)
        {
            getsetstrem << collisttype[j];
            getsetstrem << "& getRef" << uptempstring << "(){  return std::ref(data." + tablecollist[j] + "); } \n";
            getsetstrem << " void set" << uptempstring << "(" << collisttype[j]
                        << " &val){  data." + tablecollist[j] + "=val;} \n";
            if (collisttype[j].find("std::string") != std::string::npos)
            {
                getsetstrem << " void set" << uptempstring
                            << "(std::string_view val){  data." + tablecollist[j] + "=val;} \n";
            }
            else
            {
                getsetstrem << " void set" << uptempstring << "(" << collisttype[j]
                            << " &val){  data." + tablecollist[j] + "=val;} \n";
                getsetstrem << " void set" << uptempstring << "(" << collisttype[j]
                            << " val){  data." + tablecollist[j] + "=val;} \n";
            }
        }
        else
        {
            getsetstrem << " void set" << uptempstring << "(" << collisttype[j]
                        << " val){  data." + tablecollist[j] + "=val;} \n";
        }
        getsetstrem << "\n";
    }

    headtxt.append(getsetstrem.str());

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = tablenamebase + "_base::meta getnewData(){\n \t struct meta newdata;\n\t return newdata; \n} \n";

    headtxt.append(tablenamebase);
    headtxt.append("_base::meta getData(){\n \t return data; \n} \n");
    headtxt.append("std::vector<");
    headtxt.append(tablenamebase);
    headtxt.append("_base::meta> getRecord(){\n \t return record; \n} \n");

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////

    std::ostringstream getcollstrem;
    //scale field
    //tablefieldscale
    headtxt.clear();
    update2strem.str("");
    if (tablefieldscale.size() > 0)
    {
        for (auto &kaa : tablefieldscale)
        {

            for (auto &nbb : numbercollist)
            {
                if (nbb.second == kaa.first)
                {
                    uptempstring.clear();
                    uptempstring     = kaa.first;
                    unsigned int re  = 0;
                    bool isupperchar = false;
                    for (unsigned int n = 0; n < uptempstring.size(); n++)
                    {
                        if (uptempstring[n] == '_')
                        {
                            isupperchar = true;
                            continue;
                        }
                        if (isupperchar)
                        {
                            if (uptempstring[n] >= 'a' && uptempstring[n] <= 'z')
                            {
                                uptempstring[re] = uptempstring[n] - 32;
                            }
                            else
                            {
                                uptempstring[re] = uptempstring[n];
                            }
                            isupperchar = false;
                        }
                        else
                        {
                            uptempstring[re] = uptempstring[n];
                        }
                        re++;
                    }
                    uptempstring.resize(re);
                    if (uptempstring[0] >= 'a' && uptempstring[0] <= 'z')
                    {
                        uptempstring[0] = uptempstring[0] - 32;
                    }
                    std::string scalename = kaa.second.first;
                    std::string scalnum   = kaa.second.second;

                    if (scalename.size() > 0)
                    {
                        if (scalename[0] >= 'a' && scalename[0] <= 'z')
                        {
                            scalename[0] = scalename[0] - 32;
                        }
                    }

                    headtxt.append("\n double get");
                    headtxt.append(uptempstring);
                    headtxt.append("To");
                    headtxt.append(scalename);
                    headtxt.append("()\n{\n");
                    headtxt.append("\treturn (double)data.");
                    headtxt.append(nbb.second);
                    headtxt.append("/");
                    if (scalnum.size() > 0)
                    {
                        headtxt.append(scalnum);
                    }
                    else
                    {
                        headtxt.append("1");
                    }
                    headtxt.append(";\n}\n");
                    headtxt.append("\n template<typename T>");
                    headtxt.append("\n requires std::is_arithmetic_v<T>");
                    headtxt.append("\n double get");
                    headtxt.append(uptempstring);
                    headtxt.append("To");
                    headtxt.append(scalename);
                    headtxt.append("(T a)\n{\n");
                    headtxt.append("\treturn (double)((long double)a/");
                    if (scalnum.size() > 0)
                    {
                        headtxt.append(scalnum);
                    }
                    else
                    {
                        headtxt.append("1");
                    }
                    headtxt.push_back(')');
                    headtxt.append(";\n}\n");

                    headtxt.append("\n template<typename T>");
                    headtxt.append("\n requires std::is_arithmetic_v<T>");
                    headtxt.append("\n long long get");
                    headtxt.append(scalename);
                    headtxt.append("To");
                    headtxt.append(uptempstring);
                    headtxt.append("(T a)\n{\n");
                    headtxt.append("\treturn std::round(a*");
                    if (scalnum.size() > 0)
                    {
                        headtxt.append(scalnum);
                    }
                    else
                    {
                        headtxt.append("1");
                    }
                    headtxt.push_back(')');
                    headtxt.append(";\n}\n");

                    headtxt.append("\n template<typename T>");
                    headtxt.append("\n requires std::is_arithmetic_v<T>");
                    headtxt.append("\n void set");
                    headtxt.append(uptempstring);
                    headtxt.append("To");
                    headtxt.append(scalename);
                    headtxt.append("(T a)\n{\n\tdata.");
                    headtxt.append(nbb.second);
                    headtxt.append("=std::round(a*");
                    if (scalnum.size() > 0)
                    {
                        headtxt.append(scalnum);
                    }
                    else
                    {
                        headtxt.append("1");
                    }
                    headtxt.push_back(')');
                    headtxt.append(";\n}\n");
                }
            }
        }
    }

    if (headtxt.size() > 0)
    {
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }

    headtxt.clear();
    update2strem.str("");
    ///////////////////////////////////////////////////////
    // mete_tree to json

    headtxt.clear();
    if (ismeta_tree || tablefieldtree.size() > 0)
    {
        headtxt = R"(
   std::string tree_tojson(const std::vector<meta_tree> &tree_data, std::string fileld=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>0){
            for(;jj<fileld.size();jj++){
                if(fileld[jj]==','){
                    keypos.emplace_back(findcolpos(keyname)); 
                    keyname.clear();
                    continue;   
                }
                if(fileld[jj]==0x20){

                    continue;   
                }
                keyname.push_back(fileld[jj]);

            }  
            if(keyname.size()>0){
                            keypos.emplace_back(findcolpos(keyname)); 
                            keyname.clear();
            }
        }else{
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"[";
        for(size_t n=0;n<tree_data.size();n++){
            if(n>0){
                tempsql<<",{";
            }else{
                tempsql<<"{";
            }  
        
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
        )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        update2strem.str("");

        for (unsigned int j = 0; j < tablecollist.size(); j++)
        {

            // 数字
            update2strem << " case " << std::to_string(j) << ":\n";
            update2strem << " if(jj>0){ tempsql<<\",\"; } \n";
            if (colltypeshuzi[j] < 30)
            {
                update2strem << "if(tree_data[n]." << tablecollist[j] << "==0){\n";
                if (j > 0)
                {
                    update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
                }
                else
                {
                    update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
                }
                update2strem << " }else{ \n";
                if (j > 0)
                {
                    update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(tree_data[n]."
                                 << tablecollist[j] << ");\n";
                }
                else
                {
                    update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(tree_data[n]."
                                 << tablecollist[j] << ");\n";
                }

                update2strem << "}\n";
            }
            else if (colltypeshuzi[j] == 60)
            {
                update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
                update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<tree_data[n]."
                             << tablecollist[j] << "<<\"\\\"\";\n }\n";
            }
            else if (colltypeshuzi[j] == 61)
            {
                update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
                update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<tree_data[n]."
                             << tablecollist[j] << "<<\"\\\"\";\n }\n";
            }
            else
            {
                if (j > 0)
                {
                    update2strem << "tempsql<<\"\\\"" << tablecollist[j]
                                 << "\\\":\\\"\"<<http::utf8_to_jsonstring(tree_data[n]." << tablecollist[j]
                                 << ")<<\"\\\"\";\n";
                }
                else
                {
                    update2strem << "tempsql<<\"\\\"" << tablecollist[j]
                                 << "\\\":\\\"\"<<http::utf8_to_jsonstring(tree_data[n]." << tablecollist[j]
                                 << ")<<\"\\\"\";\n";
                }
            }
            update2strem << " break;\n";
        }

        headtxt.append(update2strem.str());

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                             default:
                                ;
                     }
                 }

        tempsql<<",\"children\":";
         tempsql<<tree_tojson(tree_data[n].children, fileld);     
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        ///////////////////////////////////////////////
        headtxt = R"(
   std::string tree_tojson(const std::vector<meta_tree> &tree_data,std::function<bool(std::string&,const meta_tree&)> func,std::string fileld=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>0){
            for(;jj<fileld.size();jj++){
                if(fileld[jj]==','){
                    keypos.emplace_back(findcolpos(keyname)); 
                    keyname.clear();
                    continue;   
                }
                if(fileld[jj]==0x20){

                    continue;   
                }
                keyname.push_back(fileld[jj]);

            }  
            if(keyname.size()>0){
                            keypos.emplace_back(findcolpos(keyname)); 
                            keyname.clear();
            }
        }else{
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    tempsql<<"[";
    for(size_t n=0;n<tree_data.size();n++){
        keyname.clear();
        if(func(keyname,tree_data[n])){ 
                if(n>0){
                    tempsql<<",{";
                }else{
                    tempsql<<"{";
                } 
                tempsql<<keyname;
        }else{
        continue;
        } 
        
        for(jj=0;jj<keypos.size();jj++){
            
            switch(keypos[jj]){
        )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        update2strem.str("");

        for (unsigned int j = 0; j < tablecollist.size(); j++)
        {

            // 数字
            update2strem << " case " << std::to_string(j) << ":\n";
            update2strem << " if(jj>0){ tempsql<<\",\"; } \n";

            if (colltypeshuzi[j] < 30)
            {
                update2strem << "if(tree_data[n]." << tablecollist[j] << "==0){\n";
                if (j > 0)
                {
                    update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
                }
                else
                {
                    update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":0\";\n";
                }
                update2strem << " }else{ \n";
                if (j > 0)
                {
                    update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(tree_data[n]."
                                 << tablecollist[j] << ");\n";
                }
                else
                {
                    update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(tree_data[n]."
                                 << tablecollist[j] << ");\n";
                }

                update2strem << "}\n";
            }
            else if (colltypeshuzi[j] == 60)
            {
                update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
                update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<tree_data[n]."
                             << tablecollist[j] << "<<\"\\\"\";\n }\n";
            }
            else if (colltypeshuzi[j] == 61)
            {
                update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
                update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<tree_data[n]."
                             << tablecollist[j] << "<<\"\\\"\";\n }\n";
            }
            else
            {
                if (j > 0)
                {
                    update2strem << "tempsql<<\"\\\"" << tablecollist[j]
                                 << "\\\":\\\"\"<<http::utf8_to_jsonstring(tree_data[n]." << tablecollist[j]
                                 << ")<<\"\\\"\";\n";
                }
                else
                {
                    update2strem << "tempsql<<\"\\\"" << tablecollist[j]
                                 << "\\\":\\\"\"<<http::utf8_to_jsonstring(tree_data[n]." << tablecollist[j]
                                 << ")<<\"\\\"\";\n";
                }
            }
            update2strem << " break;\n";
        }

        headtxt.append(update2strem.str());

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                             default:
                                ;
                     }
                 }   
         tempsql<<",\"children\":";
         tempsql<<tree_tojson(tree_data[n].children,func,fileld);     
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
    }
    headtxt.clear();

    ///////////////////////////////////////////////////////
    // record to tree

    if (tablefieldtree.size() > 0)
    {
        auto iter = tablefieldtree.begin();

        std::string parentitemname = iter->first;
        std::string sourceidname   = iter->second.first;

        std::string itemmember_str, tree_torecord_str;
        itemmember_str.append("\t\tmeta_tree temp_obja;\n");
        tree_torecord_str.append("\n\t\tmeta temp_obja;\n");
        unsigned int j = 0;
        for (auto &fe : tablecollist)
        {
            if (fe == "level" && colltypeshuzi[j] < 30)
            {
                itemmember_str.append("\t\t\t\t\t\ttemp_obja.level=level;\n");
                tree_torecord_str.append("\t\t\ttemp_obja.level=level;\n");
            }
            else
            {
                itemmember_str.append("\t\t\t\t\t\ttemp_obja.");
                itemmember_str.append(fe);
                itemmember_str.append("=record[i].");
                itemmember_str.append(fe);
                itemmember_str.append(";\n");

                tree_torecord_str.append("\t\t\ttemp_obja.");
                tree_torecord_str.append(fe);
                tree_torecord_str.append("=sourcedata[i].");
                tree_torecord_str.append(fe);
                tree_torecord_str.append(";\n");
            }
            j++;
        }

        headtxt = R"(
    meta_tree treedata_from_record(unsigned int i=0)
    {
        meta_tree temp_obja;
        if(i>=record.size())
        {
           return  temp_obja;   
        }
        )";
        for (auto &fe : tablecollist)
        {

            headtxt.append("\ttemp_obja.");
            headtxt.append(fe);
            headtxt.append("=record[i].");
            headtxt.append(fe);
            headtxt.append(";\n");
        }
        headtxt += R"(
        return  temp_obja;   
    }
    meta_tree treedata_from_data()
    {
        meta_tree temp_obja;

        )";
        for (auto &fe : tablecollist)
        {

            headtxt.append("\ttemp_obja.");
            headtxt.append(fe);
            headtxt.append("=data.");
            headtxt.append(fe);
            headtxt.append(";\n");
        }
        headtxt += R"(
        return  temp_obja;   
    }      
    meta_tree treedata_from_data(const meta &tempdata)
    {
        meta_tree temp_obja;
        )";
        for (auto &fe : tablecollist)
        {

            headtxt.append("\ttemp_obja.");
            headtxt.append(fe);
            headtxt.append("=tempdata.");
            headtxt.append(fe);
            headtxt.append(";\n");
        }
        headtxt += R"(
        return  temp_obja;   
    }     
    std::vector<meta_tree> to_tree(unsigned int beginid=0)
    {
       std::vector<meta_tree> temp;
       unsigned int level=0; 
       if(beginid==0)
       {
            for (unsigned int i = 0; i < record.size(); i++)
            {
                if (record[i].)";
        headtxt.append(parentitemname);
        headtxt += R"( == 0)
                {
                    )";
        headtxt.append(itemmember_str);
        headtxt += R"(
                    temp.push_back(temp_obja);
                }
            }
       }
       else
       {
           for (unsigned int i = 0; i < record.size(); i++)
            {
                if (record[i].)";
        headtxt.append(sourceidname);
        headtxt += R"( == beginid)
                {
                    )";
        headtxt.append(itemmember_str);
        headtxt += R"(
                    temp.push_back(temp_obja);
                    break;
                }
            }
       }

       if(temp.size()==0)
       {
          return temp; 
       }
       level+=1;
       for (unsigned int j = 0; j < temp.size(); j++)
       {
         record_to_tree(temp[j].children,temp[j].)";
        headtxt.append(sourceidname);
        headtxt += R"(,level);
       }
       return temp; 
    }    
    void record_to_tree(std::vector<meta_tree> &targetdata,long long t_vid,unsigned int level=0)
    {
        for (unsigned int i = 0; i < record.size(); i++)
        {
            if (record[i].)";
        headtxt.append(parentitemname);
        headtxt += R"(== t_vid)
            {
                )";
        headtxt.append(itemmember_str);
        headtxt += R"(
                targetdata.push_back(temp_obja);
            }
        }
        level+=1;
        for (unsigned int j = 0; j < targetdata.size(); j++)
        {
         record_to_tree(targetdata[j].children,targetdata[j].)";
        headtxt.append(sourceidname);
        headtxt += R"(,level);
        }
    }
    void tree_torecord(const std::vector<meta_tree> &sourcedata,unsigned int level=0)
    {
        for (unsigned int i = 0; i < sourcedata.size(); i++)
        {)";
        headtxt.append(tree_torecord_str);
        headtxt += R"(
            record.push_back(temp_obja);
            if(sourcedata[i].children.size()>0)
            {
                tree_torecord(sourcedata[i].children,level+1);
            }
        }
    }      
   )";
    }
    if (headtxt.size() > 0)
    {
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    ///////////////////////////////////////////////////////
    // get_meta string
    headtxt.clear();
    update2strem.str("");
    headtxt = R"(

    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   )";

    for (auto &kaa : stringcollist)
    {
        update2strem << "\t\t if(key_name==\"" << kaa.second << "\")\n\t\t{\n";
        update2strem << "\t\t\treturn data." << kaa.second << ";\n";
        update2strem << "\t\t}\n";
    }
    headtxt.append(update2strem.str());
    // if (stringcollist.size() == 0)
    // {
    //     headtxt.append("\n if(sizeof(key_name)){}; \n");
    // }
    headtxt.append("\t\treturn nullptr; \n\t}\n");

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    ///////////////////////////////////////////////////////
    // get_meta is_integral_v
    headtxt.clear();
    update2strem.str("");
    headtxt = R"(

    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   )";

    for (auto &kaa : numbercollist)
    {
        update2strem << "\t\t if(key_name==\"" << kaa.second << "\")\n\t\t{\n";
        update2strem << "\t\t\treturn data." << kaa.second << ";\n";
        update2strem << "\t\t}\n";
    }
    headtxt.append(update2strem.str());
    // if (numbercollist.size() == 0)
    // {
    //     headtxt.append("\n if(sizeof(key_name)){};  \n");
    // }
    headtxt.append("\t\treturn nullptr; \n\t}\n");

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////////////////
    // get_meta float
    headtxt.clear();
    update2strem.str("");
    headtxt = R"(

    template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   )";

    for (auto &kaa : floatcollist)
    {
        update2strem << "\t\t if(key_name==\"" << kaa.second << "\")\n\t\t{\n";
        update2strem << "\t\t\treturn data." << kaa.second << ";\n";
        update2strem << "\t\t}\n";
    }
    headtxt.append(update2strem.str());
    // if (floatcollist.size() == 0)
    // {
    //     headtxt.append("\n if(sizeof(key_name)){};  \n");
    // }
    headtxt.append("\t\treturn nullptr; \n\t}\n");

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    //////////////////////
    // getCol 1 int

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t a.emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }
    sqlqueryring = getcollstrem.str();

    headtxt = R"(
            template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >  
            std::vector<T> getCol([[maybe_unused]] std::string keyname)
            {
                std::vector<T> a;
                
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {

        headtxt = R"(
                unsigned char kpos;
                kpos=findcolpos(keyname);               
                for(auto &iter:record)
                {
                    switch(kpos)
                    {
   )";
        headtxt.append(sqlqueryring);
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                    }
                }
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }

    headtxt = R"(
                return a;
            }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    //////////////////
    // getCol 2 float
    sqlqueryring.clear();
    getcollstrem.str("");
    for (auto &kaa : floatcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t\t a.emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t break;\n";
    }
    sqlqueryring = getcollstrem.str();

    headtxt = R"(
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >    
			std::vector<T> getCol([[maybe_unused]] std::string keyname)
			{
				std::vector<T> a;
				
)";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {

        headtxt = R"(
                unsigned char kpos;
				kpos = findcolpos(keyname);  
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

)";
        headtxt.append(sqlqueryring);
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                    }
                }
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }

    headtxt = R"(
                return a;
            }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////
    // getVal is_integral_v
    ////////////////////////////////////////////////
    sqlqueryring.clear();
    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t return data." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }
    sqlqueryring = getcollstrem.str();

    headtxt = R"(
            template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
            T getVal([[maybe_unused]] std::string keyname)
            {
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {
        headtxt = R"(
                    unsigned char kpos;
                    kpos=findcolpos(keyname);                   
                    switch(kpos)
                    {

   )";

        headtxt.append(sqlqueryring);
        headtxt.append("\t\t\t}");
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }

    headtxt = R"(
                return 0;
            }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    /////////////////////////////////
    /// getVal base::meta  is_integral_v
    sqlqueryring.clear();
    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t return iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }
    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] )";
    headtxt += tablenamebase;
    headtxt += R"(_base::meta & iter,[[maybe_unused]] std::string keyname)
        {

          )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {
        headtxt = R"(
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   )";
        headtxt += sqlqueryring;
        headtxt += "\n\t\t\t}\n";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    // if (sqlqueryring.size() == 0)
    // {
    //     headtxt.append("\n if(sizeof(iter)){};  \n");
    // }
    headtxt += R"(
            return 0;
        }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    //////////////////////////
    ////getVal float

    headtxt = R"(
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
            T getVal(std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
            
                switch(kpos)
                {

    )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t return data." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
                    }
                   
    )";

    headtxt += R"(
                    return 0.0;
            }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //--------------------
    headtxt = R"(
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
            T getVal([[maybe_unused]] )";
    headtxt += tablenamebase;
    headtxt += R"(_base::meta & iter,std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
                switch(kpos)
                {
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    getcollstrem.str("");
    for (auto &kaa : floatcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t return iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    headtxt = R"(
                }
                 
    )";
    // if (floatcollist.size() == 0)
    // {
    //     headtxt.append("\n if(sizeof(iter)){};  \n");
    // }
    headtxt += R"(
            
                return 0.0;
            }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    /////////////////////////

    headtxt = R"(
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal(std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
        
                switch(kpos)
                {

   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t return data." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    headtxt = R"(
                }
                return "";
            }  
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ////////////////////////////////
    //-----------------------

    headtxt = R"(
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] )";
    headtxt += tablenamebase;
    headtxt += R"(_base::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t return iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    headtxt = R"(
                }
                
    )";
    // if (stringcollist.size() == 0)
    // {
    //     headtxt.append("\n if(sizeof(iter)){};  \n");
    // }

    headtxt += R"(
                 
                return "";
            }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    /////////////////////////
    // getCol
    sqlqueryring.clear();
    getcollstrem.str("");
    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t a.emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"( 
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >   
            std::vector<std::string> getCol([[maybe_unused]] std::string keyname)
            {
                std::vector<std::string> a;

           )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {

        headtxt = R"(
                unsigned char kpos;
                kpos=findcolpos(keyname);                    
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

    )";
        headtxt += sqlqueryring;
        headtxt += "\t\t\t\t\t}\n\t\t\t\t}";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }

    headtxt = R"(

        return a;
    }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    ////////////////////////////////////////////
    // getstrCol("userid",false); // 33,44
    // getstrCol("userid",true);  // "33","44"
    getcollstrem.str("");
    sqlqueryring.clear();

    for (unsigned int jj = 0; jj < tablecollist.size(); jj++)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(jj) << ": \n ";

        if (colltypeshuzi[jj] < 30)
        {
            getcollstrem << "\t\t\t\t a<<std::to_string(iter." << tablecollist[jj] << ");"
                         << "\n";
        }
        else
        {
            getcollstrem << "\t\t\t\t if(isyinhao){ a<<jsonaddslash(iter." << tablecollist[jj]
                         << "); \n\t\t\t\t }else{\n";
            getcollstrem << "\t\t\t\t a<<iter." << tablecollist[jj] << ";\n";
            getcollstrem << "\t\t\t\t }\n";
        }

        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"( 
        std::string getstrCol(std::string keyname,[[maybe_unused]] bool isyinhao=false)
        {
            std::ostringstream a;
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {

        headtxt = R"(
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            int j=0;
            if(isyinhao&&record.size()>0)
            {
                a<<'"';
            }
            for(auto &iter:record)
            {
                    if(j>0)
                    {
                        if(isyinhao)
                        {
                            a<<"\",\"";
                        }else{
                            a<<',';    
                        }
                    }
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                    }
                    j++;
            } 
            if(isyinhao&&j>0){
                a<<'"';
            }      
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }

    // if (sqlqueryring.size() == 0)
    // {
    //     headtxt.append("\n if(sizeof(isyinhao)){}; \n");
    // }
    headtxt += R"(
                return a.str();
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////
    // getCols std::string,std::string
    // getCols<std::string,std::string>("name","mobile");
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t\t } \n\t\t\tswitch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>     
    std::map<std::string,std::string> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
    {
        std::map<std::string,std::string> a;
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"(
        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);        
         std::string ktemp,vtemp;
         for(auto &iter:record)
         {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
                if(ktemp.size()>0)
                {
                    a.emplace(ktemp,vtemp);
                }
            }       

   )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(     
            return a;
        } 
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    ///////////////////////
    // getCols<std::string,U>
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t } \n\t\t switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }
    sqlqueryring = getcollstrem.str();

    headtxt = R"(

        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<std::string,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::map<std::string,U> a;
     )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"( 
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);            
                std::string ktemp;
                U vtemp;
                for(auto &iter:record)
                {    
                    switch(kpos)
                    {
 
       )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                    }
                    if(ktemp.size()>0)
                    {
                        a.emplace(ktemp,vtemp);
                    }
                }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(    
            return a;
        } 
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    ///////////////////////////////////////////////////////
    // getCols <int,float>
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t } \n \t\t  switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {
        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>       
        std::map<T,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,U> a;
       )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);        
            T ktemp;
            U vtemp;
            for(auto &iter:record)
            {
                switch(kpos)
                {
 
       )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
                if(ktemp.size()>0)
                {
                    a.emplace(ktemp,vtemp);
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"( 
        return a;
    } )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    /////////////////////////
    // getCols int,std::string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n \t\t\tswitch(vpos){\n";

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"( 
            template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>      
            std::map<T,std::string> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
            {
                std::map<T,std::string> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"(
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);         
                T ktemp;
                std::string vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                    }
                    if(ktemp.size()>0)
                    {
                        a.emplace(ktemp,vtemp);
                    }
                } 
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }

    headtxt = R"(     
                return a;
            }     
        )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ////////////////////////////
    // getCols std::string int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n \t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>       
        std::map<std::string,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<std::string,U> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);            
            std::string  ktemp;
            U  vtemp;
            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
                if(ktemp.size()>0)
                {
                    a.emplace(ktemp,vtemp);
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(  
        return a;
    }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    /////////////////////////
    // getCols <int,int>
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n \t\t\tswitch(vpos){\n";

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>   
        std::map<T,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,U> a;
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);            
            T ktemp;
            U vtemp;
            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
                if(ktemp.size()>0)
                {
                    a.emplace(ktemp,vtemp);
                }
            }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }   
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ////////////////////////////
    /// getmapRows int meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        // getcollstrem<<"\t ktemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t\t\t\t a.emplace(iter." << kaa.second << ",iter);\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >         
        std::map<T,meta> getmapRows([[maybe_unused]] std::string keyname)
        {
            std::map<T,meta> a;
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {

        headtxt = R"(
            unsigned char kpos;
            kpos=findcolpos(keyname);                        
            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }

    headtxt = R"( 
            return a;
        }     
    )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////////
    // getmapRows std::string,meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        // getcollstrem<<"\t ktemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t\t\t\t a.emplace(iter." << kaa.second << ",iter);\n";
        getcollstrem << "\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >    
        std::map<std::string,meta> getmapRows([[maybe_unused]] std::string keyname)
        {
            std::map<std::string,meta> a;

    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {

        headtxt = R"(
            unsigned char kpos;
            kpos=findcolpos(keyname);            
            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
                //a.emplace(ktemp,iter);
            }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(

        return a;
    }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    /////////////////////////////////
    // getvecCols std::string float
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t \t\t }\n \t\t\tswitch(vpos){\n";

    for (auto &kaa : floatcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>  
        std::vector<std::pair<std::string,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::vector<std::pair<std::string,U>> a;
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);                   
            std::string ktemp;
            U vtemp;
            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                   }

                 a.emplace_back(ktemp,vtemp);
            }       

     )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(

            return a;
        }   
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////
    // getvecCols int float
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\t\t\t switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"(
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                   }

                   a.emplace_back(ktemp,vtemp);
                }       

    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(

            return a;
        }   
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ////////////////////////////////////////////
    // getvecCols int string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n \t\t\tswitch(vpos){\n";

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;

   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"(
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                   }

                    a.emplace_back(ktemp,vtemp);
                }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ////////////////////////////////////////////////////
    // getvecCols string int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n \t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>     
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"(
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);                
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                   }
                    a.emplace_back(ktemp,vtemp);
                }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////////
    // getvecCols int int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {

        headtxt = R"(
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                   }
                    a.emplace_back(ktemp,vtemp);
                }       
      )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }   
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ////////////////////////////////////////////////////////
    // getvecCols std::string std::string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>     
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 30)
    {
        headtxt = R"(
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                   }

                    a.emplace_back(ktemp,vtemp);
                }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }  
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////////////
    // getvecRows int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        // getcollstrem<<"\t ktemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t a.emplace_back(iter." << kaa.second << ",iter);\n";
        getcollstrem << "\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
        std::vector<std::pair<T,meta>> getvecRows([[maybe_unused]] std::string keyname)
        {
            std::vector<std::pair<T,meta>> a;
     )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {
        headtxt = R"(
            unsigned char kpos;
            kpos=findcolpos(keyname);                  
            for(auto &iter:record)
            { 
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
        return a;
    })";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////
    // getvecRows std::string meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        // getcollstrem<<"\t ktemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t a.emplace_back(iter." << kaa.second << ",iter);\n";
        getcollstrem << "\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >  
        std::vector<std::pair<std::string,meta>> getvecRows([[maybe_unused]] std::string keyname)
        {
            std::vector<std::pair<std::string,meta>> a;
      )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {
        headtxt = R"(
            unsigned char kpos;
            kpos=findcolpos(keyname);                     
            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(

        return a;
    })";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    ////////////////////////////////////////////////////
    // getgroupCols int int float
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : floatcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        // getcollstrem<<"\t dtemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        // a[ktemp][vtemp].emplace_back(dtemp);
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);      
            T ktemp;
            U vtemp;
            for(auto &iter:record)
            { 
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
            }       

    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////
    // getgroupCols int int int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);          
            T ktemp;
            U vtemp;
            //D vtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
            }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    /////////////////////////
    // getgroupCols int int string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);       
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
            }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // getgroupCols int string float
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
                std::map<T,std::map<U,std::vector<D>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
                unsigned char kpos,vpos,dpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                dpos=findcolpos(dataname);
                T ktemp;
                U vtemp;
            // D dtemp;

                for(auto &iter:record)
                {
                    
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                   }
                }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // getgroupCols int string int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
    std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
    {
        std::map<T,std::map<U,std::vector<D>>> a;

   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);             
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record)
         {
            switch(kpos)
            {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
            }
         }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
        return a;
    })";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // getgroupCols int string string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            { 
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
            }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // getgroupCols string int float
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : floatcollist)
    {
        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        // getcollstrem<<"\t dtemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        // a[ktemp][vtemp].emplace_back(dtemp);
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
                std::map<T,std::map<U,std::vector<D>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
                unsigned char kpos,vpos,dpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                dpos=findcolpos(dataname);               
                T ktemp;
                U vtemp;
                //D vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                    }
                }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(

            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////
    // groupby string int int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);            
            T ktemp;
            U vtemp;
            //D vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
                }
            }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    /////////////////////////
    // getgroupCols string int string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\tswitch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\tswitch(dpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
    std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
    {
        std::map<T,std::map<U,std::vector<D>>> a;

   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
        // D dtemp;

         for(auto &iter:record)
         {
             
            switch(kpos)
            {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();

        headtxt = R"(
            }
         }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // getgroupCols string string float
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);        
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       

    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        })";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // getgroupCols string string int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // getgroupCols string string string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(dpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);        
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // 2D data
    // getgroupCols string string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;

   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////
    // getgroupCols string float
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t\t\t\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       

    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////
    // getgroupCols string int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////////
    // getgroupCols int string
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       

    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////
    // getgroupCols int float
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(            
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////
    // getgroupCols int int
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////
    // meta row
    //////////////////////////////////////////
    // getgroupRows int meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename std::enable_if<std::is_integral_v<T>,bool>::type = true>    
        std::map<T,std::vector<meta>> getgroupRows([[maybe_unused]] std::string keyname)
        {
            std::map<T,std::vector<meta>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {
        headtxt = R"(
            unsigned char kpos;
            kpos=findcolpos(keyname);

            for(auto &iter:record)
            {
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    ///////////////////////////////////////////////
    // getgroupRows string meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<meta>> getgroupRows([[maybe_unused]] std::string keyname)
        {
            std::map<T,std::vector<meta>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 0)
    {
        headtxt = R"(
            unsigned char kpos;
            kpos=findcolpos(keyname);

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////
    ////3D meta row
    ////////////////////////////////////
    // getgroupRows string string meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////
    // getgroupRows string int meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);            
            T ktemp;
            
            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       

    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////
    // getgroupRows int int meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<U>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;
   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);        
        T ktemp;
        
            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////
    // getgroupRows int string meta
    getcollstrem.str("");
    sqlqueryring.clear();

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    getcollstrem << "\t\t\t  }\n\n\t\t\t switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "\t\t\tcase " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t\t\t\t a[ktemp][iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t\t\t\t break;\n";
    }

    sqlqueryring = getcollstrem.str();

    headtxt = R"(
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;

   )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    if (sqlqueryring.size() > 70)
    {
        headtxt = R"(
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);            
            T ktemp;
            
            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   )";
        headtxt += sqlqueryring;
        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
        headtxt = R"(
                }
            }       
    )";

        fwrite(&headtxt[0], headtxt.size(), 1, f);
        headtxt.clear();
    }
    headtxt = R"(
            return a;
        }
    )";
    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();
    //////////////////////////////////////////
    // end orm basefile
    //////////////////////////////////////////////
    //////////////////////
    headtxt = R"(
  };
    
)";

    if (rmstag != "default")
    {
        headtxt.append("} ");
    }
    headtxt += R"(
}
#endif
   )";

    fwrite(&headtxt[0], headtxt.size(), 1, f);
    headtxt.clear();

    fclose(f);

    std::cout << " create table metainfo file: \033[1m\033[31m" << filebasename << "\033[0m" << std::endl;
    return 0;
}

void addhfiletoormfile(std::string mpath, std::string modelname, std::string rmstag)
{
    std::string ormfilename = mpath;
    if (ormfilename.back() != '/')
    {
        ormfilename.push_back('/');
    }
    ormfilename.append("orm.h");

    FILE *f = fopen(ormfilename.c_str(), "rb");
    if (f == NULL)
    {
        if (!fs::exists(ormfilename))
        {

            f = fopen(ormfilename.c_str(), "wb");
            if (f == NULL)
            {
                return;
            }
            std::string tempc = "/*build this file time ";
            tempc.append(getgmtdatetime());
            tempc.append("*/\n");
            fwrite(&tempc[0], 1, tempc.size(), f);
            fclose(f);
        }
        f = fopen(ormfilename.c_str(), "rb");
        if (f == NULL)
        {
            return;
        }
    }
    fseek(f, 0, SEEK_END);
    auto const size = ftell(f);
    fseek(f, 0, SEEK_SET);

    std::string s, contenttxt;

    std::string real_model_name = modelname;
    colname_first_touper(real_model_name);
    s.resize(size);

    auto nread = fread(&s[0], 1, size, f);
    s.resize(nread);
    fclose(f);
    std::string includename;
    if (rmstag == "default")
    {
        includename = "\"" + real_model_name + ".h\"";
    }
    else
    {
        includename = "\"" + rmstag + "/include/" + real_model_name + ".h\"";
    }

    auto itepos = s.find(includename);
    if (itepos == std::string::npos)
    {
        int j = s.size();
        for (; j > 0; j--)
        {
            if (s[j] == 0x20 || s[j] == '\t')
            {
                continue;
            }
            break;
        }

        s.append("\n#include ");

        s.append(includename);

        FILE *mfd = fopen(ormfilename.c_str(), "wb");
        if (mfd == NULL)
        {
            return;
        }
        fwrite(&s[0], 1, s.size(), mfd);

        fclose(mfd);
    }

    s.clear();
    s.shrink_to_fit();
}

int modelcli()
{

    fs::path current_path = fs::current_path();

    std::cout << "\033[36m 🍄 current path:\033[0m \033[1m\033[35m" << current_path.string() << "\033[0m" << std::endl;
    std::string ormfilepath     = "orm/";
    std::string ormnowpath      = "orm/";
    std::string rootcontrolpath = "models/";
    std::string controlpath     = rootcontrolpath;
    std::string controlrunpath;
    std::string prj_root_path = current_path.string();
    if (prj_root_path.size() > 0 && prj_root_path.back() != '/')
    {
        prj_root_path.push_back('/');
    }

    fs::path vpath   = controlpath;
    fs::path ormpath = ormfilepath;
    if (!fs::exists(vpath))
    {
        std::cout << " ⛑ \033[1m\033[31m Error\033[0m Current path not project root path " << std::endl;
        return 0;
    }

    if (!fs::exists(ormpath))
    {
        fs::create_directories(ormpath);
        fs::permissions(ormpath,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    ormpath = ormnowpath + "include";
    if (!fs::exists(ormpath))
    {
        fs::create_directories(ormpath);
        fs::permissions(ormpath,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    fs::path paths = controlpath;
    if (controlpath.back() != '/')
    {
        controlpath.push_back('/');
    }
    controlrunpath = controlpath + "include/";
    paths          = controlrunpath;
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    std::string command;
    std::string rmstag = "default";
    std::string pretable;
    //std::vector<mysqlconnect_t> myconfig = getmysqlconfig("conf/orm.conf");
    std::vector<orm::orm_conn_t> myconfig = orm::get_orm_config_file("conf/orm.conf");
    // std::string mysqlconnect = ""; //"mysqlx://root:123456@127.0.0.1:33060/aaa";
    if (myconfig.size() == 0)
    {
        std::cout
            << " Sorry, not found [config/orm.conf] config file \033[1m\033[31m This file format example: \033[0m "
            << std::endl;
        command = R"(
[default]
type=main
host=127.0.0.1
port=3306
dbname=hello_world
user=root
password=12345678
pretable=
maxpool=120
minpool=20
dbtype=mysql


type=second
host=127.0.0.1
port=3306
dbname=hello_world
user=root
password=12345678
pretable=
maxpool=120
minpool=20
dbtype=mysql

        )";
        command.clear();
        return 0;
    }

    for (unsigned int i = 0; i < myconfig.size(); i++)
    {
        if (myconfig[i].link_type == 0)
        {
            std::cout << " \033[1m\033[31m" << (i + 1) << "\033[0m " << myconfig[i].dbname << std::endl;
        }
    }

    unsigned int indexsdb = 0;
    command.clear();
    while (true)
    {
        std::cout << "select db index:";
        indexsdb = 0;
        std::cin >> command;
        if (command[0] < '0' || command[0] >= '9')
        {
            return 0;
        }
        for (unsigned int i = 0; i < command.size(); i++)
        {
            if (command[i] == 0x20)
            {
                return 0;
            }
            if (command[i] >= '0' && command[i] <= '9')
            {
                indexsdb = indexsdb * 10 + (command[i] - '0');
            }
            if (i > 2)
            {
                return 0;
            }
        }

        if (indexsdb > myconfig.size())
        {
            command.clear();
            continue;
        }

        command.clear();
        break;
    }

    orm::orm_conn_t link_config_item;
    for (unsigned int i = 0; i < myconfig.size(); i++)
    {
        if (myconfig[i].link_type == 0)
        {
            if ((i + 1) == indexsdb)
            {
                link_config_item = myconfig[i];
            }
        }
    }
    asio::io_context io_context;
    std::shared_ptr<orm::orm_conn_pool> conn = std::make_shared<orm::orm_conn_pool>();

    conn->io_context   = &io_context;
    conn->conf_data[0] = link_config_item;
    rmstag             = link_config_item.tag;

    std::string model_name;//strip pretable

    std::shared_ptr<orm::mysql_conn_base> db_conn;
    try
    {
        db_conn = conn->add_edit_connect();
    }
    catch (const std::exception &e)
    {
        std::cerr <<" add_edit_connect "<< e.what() << '\n';
        return 0;
    }
    catch (const std::string &e)
    {
        std::cerr <<" error_msg "<< e << '\n';
        return 0;
    }
    catch (char const* e)
    {
        std::cerr <<" error_msg "<< e << '\n';
        return 0;
    }
    //create tag directories
    if (rmstag != "default")
    {
        if (controlpath.back() != '/')
        {
            controlpath.push_back('/');
        }
        controlpath.append(rmstag);
        controlpath.push_back('/');
        controlrunpath = controlpath;

        paths = controlrunpath;
        if (!fs::exists(paths))
        {
            fs::create_directories(paths);
            fs::permissions(paths,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }
        controlrunpath = controlpath + "include";
        paths          = controlrunpath;
        if (!fs::exists(paths))
        {
            fs::create_directories(paths);
            fs::permissions(paths,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }

        ormnowpath = ormfilepath;
        ormnowpath.append(rmstag);
        ormnowpath.push_back('/');
        ormpath = ormnowpath;

        if (!fs::exists(ormpath))
        {
            fs::create_directories(ormpath);
            fs::permissions(ormpath,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }
        ormnowpath = ormnowpath + "include";
        ormpath    = ormnowpath;

        ormnowpath = ormfilepath;
        ormnowpath.append(rmstag);
        ormnowpath.push_back('/');

        if (!fs::exists(ormpath))
        {
            fs::create_directories(ormpath);
            fs::permissions(ormpath,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }
    }

    std::string sqlstring     = "show tables;";
    //unsigned int querysql_len = sqlstring.length() + 1;
    //std::cout<<sqlstring <<std::endl;
    // db_conn->send_data.clear();

    // db_conn->send_data.push_back((querysql_len & 0xFF));
    // db_conn->send_data.push_back((querysql_len >> 8 & 0xFF));
    // db_conn->send_data.push_back((querysql_len >> 16 & 0xFF));
    // db_conn->send_data.push_back(0x00);
    // db_conn->send_data.push_back(0x03);
    // db_conn->send_data.append(sqlstring);

    std::size_t n = db_conn->write_sql(sqlstring);
    //std::size_t n = asio::write(*db_conn->socket, asio::buffer(db_conn->send_data), db_conn->ec);

    orm::pack_info_t temp_pack_data;
    temp_pack_data.seq_id = 1;
    bool is_sql_item      = false;
    std::vector<orm::field_info_t> field_array;
    unsigned char action_setup = 0;
    unsigned int column_num    = 0;
    unsigned int offset        = 0;

    std::vector<std::string> table_lists;

    for (; is_sql_item == false;)
    {
        n = db_conn->read_loop();
        offset = 0;
        for (; offset < n;)
        {
            db_conn->read_field_pack(db_conn->_cache_data, n, offset, temp_pack_data);
            if (temp_pack_data.length == temp_pack_data.current_length)
            {
                if (db_conn->pack_eof_check(temp_pack_data))
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
                    orm::field_info_t temp_filed_col;
                    db_conn->read_col_info(temp_pack_data.data, temp_filed_col);

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

                    for (unsigned int ij = 0; ij < column_num; ij++)
                    {
                        unsigned long long name_length = 0;// temp_pack_data.data[tempnum] & 0xff;
                        name_length                    = db_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                        table_lists.emplace_back(std::string(&temp_pack_data.data[tempnum], name_length));
                        tempnum = tempnum + name_length;
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

    for (unsigned int i_table = 0; i_table < table_lists.size(); i_table++)
    {
        bool table_refresh = true;

        //create raw sql file
        sqlstring = "SHOW CREATE TABLE ";
        sqlstring.append(table_lists[i_table]);
        sqlstring.append(";");

        // querysql_len = sqlstring.length() + 1;

        // db_conn->send_data.clear();

        // db_conn->send_data.push_back((querysql_len & 0xFF));
        // db_conn->send_data.push_back((querysql_len >> 8 & 0xFF));
        // db_conn->send_data.push_back((querysql_len >> 16 & 0xFF));
        // db_conn->send_data.push_back(0x00);
        // db_conn->send_data.push_back(0x03);
        // db_conn->send_data.append(sqlstring);
        n = db_conn->write_sql(sqlstring);
        //n = asio::write(*db_conn->socket, asio::buffer(db_conn->send_data), db_conn->ec);

        temp_pack_data.length         = 0;
        temp_pack_data.current_length = 0;
        temp_pack_data.error          = 0;

        temp_pack_data.seq_id = 1;
        temp_pack_data.error  = 0;
        temp_pack_data.data.clear();
        is_sql_item = false;
        field_array.clear();

        action_setup = 0;
        column_num   = 0;
        offset       = 0;

        std::vector<std::string> table_create_info_lists;

        for (; is_sql_item == false;)
        {
            n = db_conn->read_loop();
            offset = 0;
            for (; offset < n;)
            {
                db_conn->read_field_pack(db_conn->_cache_data, n, offset, temp_pack_data);
                if (temp_pack_data.length == temp_pack_data.current_length)
                {
                    if (db_conn->pack_eof_check(temp_pack_data))
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
                        orm::field_info_t temp_filed_col;
                        db_conn->read_col_info(temp_pack_data.data, temp_filed_col);

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

                        for (unsigned int ij = 0; ij < column_num; ij++)
                        {
                            unsigned long long name_length = 0;// temp_pack_data.data[tempnum] & 0xff;
                            name_length                    = db_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                            table_create_info_lists.emplace_back(std::string(&temp_pack_data.data[tempnum], name_length));
                            tempnum = tempnum + name_length;
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

        std::string ormsqlfile = ormfilepath;

        if (table_create_info_lists.size() == 2)
        {
            if (ormsqlfile.back() != '/')
            {
                ormsqlfile.push_back('/');
            }
            ormsqlfile.append("/_rawsqlfile");
            fs::path paths = ormsqlfile;
            if (!fs::exists(paths))
            {
                fs::create_directories(paths);
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
            ormsqlfile.push_back('/');
            ormsqlfile.append(rmstag);

            paths = ormsqlfile;
            if (!fs::exists(paths))
            {
                fs::create_directories(paths);
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
            ormsqlfile.push_back('/');
            std::string fieldname = ormsqlfile;
            fieldname.append(table_create_info_lists[0]);
            fieldname.push_back('_');
            fieldname.append(std::to_string(std::hash<std::string>{}(table_create_info_lists[1])));
            fieldname.append(".sql");

            if (fs::exists(fieldname))
            {
                table_refresh = false;
            }
            else
            {
                std::FILE *fp = fopen(fieldname.c_str(), "wb");
                if (fp)
                {
                    fwrite(table_create_info_lists[1].data(), 1, table_create_info_lists[1].size(), fp);
                    fclose(fp);
                }
            }
        }

        std::vector<table_columns_info_t> table_column_info_lists;
        //SHOW FULL COLUMNS FROM
        table_refresh = true;
        if (table_refresh)
        {
            sqlstring = "SHOW FULL COLUMNS FROM ";
            sqlstring.append(table_lists[i_table]);
            sqlstring.append(";");

            // querysql_len = sqlstring.length() + 1;
            // db_conn->send_data.clear();

            // db_conn->send_data.push_back((querysql_len & 0xFF));
            // db_conn->send_data.push_back((querysql_len >> 8 & 0xFF));
            // db_conn->send_data.push_back((querysql_len >> 16 & 0xFF));
            // db_conn->send_data.push_back(0x00);
            // db_conn->send_data.push_back(0x03);
            // db_conn->send_data.append(sqlstring);
            n = db_conn->write_sql(sqlstring);

            //n = asio::write(*db_conn->socket, asio::buffer(db_conn->send_data), db_conn->ec);

            temp_pack_data.length         = 0;
            temp_pack_data.current_length = 0;
            temp_pack_data.error          = 0;

            temp_pack_data.seq_id = 1;
            temp_pack_data.error  = 0;
            temp_pack_data.data.clear();
            is_sql_item = false;
            field_array.clear();

            action_setup = 0;
            column_num   = 0;
            offset       = 0;

            std::vector<unsigned char> field_pos;

            for (; is_sql_item == false;)
            {
                n = db_conn->read_loop();
                offset = 0;
                for (; offset < n;)
                {
                    db_conn->read_field_pack(db_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.length == temp_pack_data.current_length)
                    {
                        if (db_conn->pack_eof_check(temp_pack_data))
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
                            orm::field_info_t temp_filed_col;
                            db_conn->read_col_info(temp_pack_data.data, temp_filed_col);
                            std::string find_field_name = temp_filed_col.name;
                            std::transform(find_field_name.begin(), find_field_name.end(), find_field_name.begin(), ::tolower);

                            for (unsigned int j = 0; j < table_columns_fields.size(); j++)
                            {
                                if (find_field_name == table_columns_fields[j])
                                {
                                    field_pos.push_back(j);
                                }
                            }

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
                            table_columns_info_t temp_tb_info;

                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;// temp_pack_data.data[tempnum] & 0xff;
                                name_length                    = db_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, temp_tb_info);
                                tempnum = tempnum + name_length;
                            }
                            table_column_info_lists.push_back(temp_tb_info);
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

            //shwo

            sqlstring = "SELECT * FROM ";
            sqlstring.append(table_lists[i_table]);
            sqlstring.append(" where 0;");

            // querysql_len = sqlstring.length() + 1;

            // db_conn->send_data.clear();

            // db_conn->send_data.push_back((querysql_len & 0xFF));
            // db_conn->send_data.push_back((querysql_len >> 8 & 0xFF));
            // db_conn->send_data.push_back((querysql_len >> 16 & 0xFF));
            // db_conn->send_data.push_back(0x00);
            // db_conn->send_data.push_back(0x03);
            // db_conn->send_data.append(sqlstring);

            n = db_conn->write_sql(sqlstring);
            //n = asio::write(*db_conn->socket, asio::buffer(db_conn->send_data), db_conn->ec);

            temp_pack_data.length         = 0;
            temp_pack_data.current_length = 0;
            temp_pack_data.error          = 0;

            temp_pack_data.seq_id = 1;
            temp_pack_data.error  = 0;
            temp_pack_data.data.clear();
            is_sql_item = false;
            field_array.clear();

            action_setup = 0;
            column_num   = 0;
            offset       = 0;

            field_pos.clear();

            for (; is_sql_item == false;)
            {
                n = db_conn->read_loop();
                offset = 0;
                for (; offset < n;)
                {
                    db_conn->read_field_pack(db_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.length == temp_pack_data.current_length)
                    {
                        if (db_conn->pack_eof_check(temp_pack_data))
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
                            orm::field_info_t temp_filed_col;
                            db_conn->read_col_info(temp_pack_data.data, temp_filed_col);
                            std::string find_field_name = temp_filed_col.name;
                            std::transform(find_field_name.begin(), find_field_name.end(), find_field_name.begin(), ::tolower);

                            for (unsigned int j = 0; j < table_columns_fields.size(); j++)
                            {
                                if (find_field_name == table_columns_fields[j])
                                {
                                    field_pos.push_back(j);
                                }
                            }

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

                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;// temp_pack_data.data[tempnum] & 0xff;
                                name_length                    = db_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

                                // assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, temp_tb_info);
                                tempnum = tempnum + name_length;
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

            for (unsigned int k = 0; k < field_array.size(); k++)
            {
                std::transform(field_array[k].org_name.begin(), field_array[k].org_name.end(), field_array[k].org_name.begin(), ::tolower);

                for (unsigned int m = 0; m < table_column_info_lists.size(); m++)
                {
                    if (field_array[k].org_name == table_column_info_lists[m].col_name)
                    {
                        table_column_info_lists[m].col_type = field_array[k].field_type;
                        if (table_column_info_lists[m].col_type == 0xFC || table_column_info_lists[m].col_type == 0xFD || table_column_info_lists[m].col_type == 0xFE)
                        {
                            table_column_info_lists[m].big_type = 1;
                        }
                        else if (table_column_info_lists[m].col_type == 0xFB || table_column_info_lists[m].col_type == 0xFA || table_column_info_lists[m].col_type == 0x0F)
                        {
                            table_column_info_lists[m].big_type = 1;
                        }
                        else if (table_column_info_lists[m].col_type == 0xF6 || table_column_info_lists[m].col_type == 0x05 || table_column_info_lists[m].col_type == 0x04 || table_column_info_lists[m].col_type == 0x00)
                        {
                            table_column_info_lists[m].big_type = 3;
                        }
                        else if (table_column_info_lists[m].col_type == 0x01 || table_column_info_lists[m].col_type == 0x02 || table_column_info_lists[m].col_type == 0x03 || table_column_info_lists[m].col_type == 0x08)
                        {
                            table_column_info_lists[m].big_type = 2;
                        }
                        else if (table_column_info_lists[m].col_type == 0x09)
                        {
                            table_column_info_lists[m].big_type = 3;
                        }
                        else if (table_column_info_lists[m].col_type == 0x07 || table_column_info_lists[m].col_type == 0x0A || table_column_info_lists[m].col_type == 0x0B)
                        {
                            table_column_info_lists[m].big_type    = 1;
                            table_column_info_lists[m].is_datetime = true;
                        }
                        else if (table_column_info_lists[m].col_type == 0x0C)
                        {
                            table_column_info_lists[m].big_type    = 1;
                            table_column_info_lists[m].is_datetime = true;
                        }
                        else if (table_column_info_lists[m].col_type == 0xF5)
                        {
                            //json
                            table_column_info_lists[m].big_type = 1;
                        }

                        if (field_array[k].flags & 32)
                        {
                            table_column_info_lists[m].is_unsigned = true;
                        }
                        if (field_array[k].flags & 2)
                        {
                            table_column_info_lists[m].is_pk = true;
                        }
                        if (field_array[k].flags & 512)
                        {
                            table_column_info_lists[m].is_auto_inc = true;
                        }
                    }
                }
            }

            model_name.clear();
            model_name = table_lists[i_table].substr(link_config_item.pretable.size());

            //create mysql operate file
            create_mysql_orm_operate_file(prj_root_path, rmstag, table_lists[i_table], model_name, field_array, table_column_info_lists);
            create_orm_model_baseinfo_file(prj_root_path, rmstag, table_lists[i_table], model_name, field_array, table_column_info_lists);
            addhfiletoormfile(prj_root_path + "orm/", model_name, rmstag);
        }
    }

    return 0;
}
