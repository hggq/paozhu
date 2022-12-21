
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <filesystem>
#include "mysql.h"
#include <vector>
#include <cmath>
namespace fs = std::filesystem;

int strtointval(std::string vval)
{
    int temp = 0;
    for (int i = 0; i < vval.size(); i++)
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
    timeInfo = gmtime(&curr_time);
    strftime(timestr, sizeof(timestr), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

    std::string temp(timestr);
    return temp;
}
int createtabletoorm(std::string basefilepath,std::string modelspath, std::string realtablename, std::string tablename, std::string rmstag,MYSQL *conn)
{

    // std::string modelspath="models/";
    std::string sqlqueryring = "SHOW FULL COLUMNS FROM ";
    sqlqueryring.append(realtablename);
    std::string filebasefilename;
    filebasefilename.resize(tablename.size());

    std::transform(tablename.begin(), tablename.end(), filebasefilename.begin(), ::tolower);
    std::string tablenamebase = filebasefilename;
    std::cout << "create \033[1m\033[31m" << realtablename << "\033[0m table to models 🚗" << std::endl;

    if (rmstag.empty())
    {
        rmstag = "default";
    }

    MYSQL_RES *result=nullptr;
    int readnum = mysql_query(conn,&sqlqueryring[0]);
    if(readnum!=0)
    {
        std::cout<<mysql_error(conn)<<std::endl;
        return 0;
    }

    result = mysql_store_result(conn);

    if(!result)
    {
        std::cout<<"not show tables COLUMNS! "<<std::endl;
        return 0;
    }
    int num_fields = mysql_num_fields(result);
    MYSQL_FIELD *fields;
    fields = mysql_fetch_fields(result);

    std::vector<std::string> table, tablecollist;
    std::vector<int> tablecelwidth;

    table.resize(num_fields);
    tablecelwidth.resize(num_fields);

    int defaultcolnamepos = 255;

    std::map<std::string, std::map<std::string, std::string>> tableinfo;
     std::map<unsigned char, unsigned char> table_type;
     std::map<unsigned char, unsigned char> table_type_unsigned;   
    for (unsigned char index = 0; index < num_fields; index++)
    {
        tablecelwidth[index] = 0;
        table[index].append(std::string(fields[index].name));
        std::transform(table[index].begin(), table[index].end(), table[index].begin(), ::tolower);
        if (tablecelwidth[index] < table[index].size())
        {
            tablecelwidth[index] = table[index].size();
        }
        if (table[index] == "default")
        {
            defaultcolnamepos = index;
        }
        //table_type[index]=fields[index].type;
    }

    //int j = 0;
    std::ostringstream tp;
    std::string temp;
    std::string fieldname;
    std::string tablepkname,tablepriname;

    MYSQL_ROW row;
    int row_num_count=0;
    row_num_count=mysql_num_rows(result);
    tablecollist.reserve(row_num_count);

    while ((row = mysql_fetch_row(result)))
    {
        fieldname = std::string(row[0]);
        std::transform(fieldname.begin(), fieldname.end(), fieldname.begin(), ::tolower);
        tablecollist.push_back(fieldname);

        for(int j = 1; j < num_fields; j++)
        {

            temp.clear();
            if(row[j])
            {
                temp.append(std::string(row[j]));
            }
            
            if (table[j]!="default")
            {
                std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
                
            }
            tableinfo[fieldname][table[j]] = temp;

            if(table[j]=="type")
            {
                bool isc = false;
                bool isnum = false;
                std::string limitchar;
                for (int n = 0; n < temp.size(); n++)
                {
                    if (temp[n] == '(')
                    {
                        isc = true;
                        temp[n] = 0x00;
                        isnum = true;
                        limitchar.clear();
                        continue;
                    }
                    if (isc)
                    {

                        if (temp[n] == ')')
                        {
                            tableinfo[fieldname]["limitchar"] = limitchar;
                            isnum = false;
                        }
                        if (isnum)
                        {
                            limitchar.push_back(temp[n]);
                        }
                        temp[n] = 0x00;
                    }
                }
            }
            if (table[j] == "extra")
            {
                //PRI
                if (temp == "auto_increment")
                {
                    tablepkname = fieldname;
                }
                
            }
            if (table[j] == "key")
            {
                //PRI
               
                if (temp == "pri")
                {
                    tablepriname = fieldname;
                }
                
            }
        }

    }
    mysql_free_result(result);

    sqlqueryring="SHOW CREATE TABLE ";//SHOW CREATE TABLE article; 
    sqlqueryring.append(realtablename);

    readnum = mysql_query(conn,&sqlqueryring[0]);
    if(readnum!=0)
    {
        std::cout<<mysql_error(conn)<<std::endl;
        return 0;
    }
    result = mysql_store_result(conn);
    num_fields = mysql_num_fields(result); 
 
    while ((row = mysql_fetch_row(result)))
    {
        if(num_fields>0)
        {
            fieldname = basefilepath;
            std::string filename=(std::string(row[0]));
            fieldname.append(tablename);
            std::string content=   (std::string(row[1])); 
            fieldname.append(".sql");
      
            FILE* fp=fopen(fieldname.c_str(),"wb");
            if(fp)
            {
                fwrite(&content[0],1,content.size(),fp);
                fclose(fp);
            }
 
            break;
        }
    }

    mysql_free_result(result);

    sqlqueryring="select * from ";//SHOW CREATE TABLE article; 
    sqlqueryring.append(realtablename);
    sqlqueryring.append(" WHERE 0 LIMIT 1");

    readnum = mysql_query(conn,&sqlqueryring[0]);
    if(readnum!=0)
    {
        std::cout<<mysql_error(conn)<<std::endl;
        return 0;
    }
    result = mysql_store_result(conn);
    num_fields = mysql_num_fields(result); 
    
    if(result)
    {
        int num_fields = mysql_num_fields(result);

        MYSQL_FIELD *fields;

        std::string type_temp;
        fields = mysql_fetch_fields(result);
        for(int i = 0; i < num_fields; i++)
        {
            table_type[i]=fields[i].type;
            
        }
    }

    mysql_free_result(result);


    std::string temptype;
    std::vector<std::string> metalist;
    std::string outlist;
    int colpos = 0;
    std::vector<std::pair<int, std::string>> stringcollist;
    std::vector<std::pair<int, std::string>> numbercollist;
    std::vector<std::pair<int, std::string>> floatcollist;
    std::map<unsigned char, std::string> collisttype;

    std::map<unsigned char, unsigned char> colltypeshuzi;
   
    std::string collnametemp;
    for (auto &bb : tablecollist)
    {

        outlist.clear();
        collnametemp.clear();
        temptype = tableinfo[bb]["type"];
        table_type_unsigned[colpos]=0;
        if (temptype[0] == 'i' && temptype[1] == 'n' && temptype[2] == 't')
        {
            // int
            std::string pretype;
            std::string typedifinde;

            if (temptype.size() > 3)
            {
                for (int n = 0; n < temptype.size(); n++)
                {
                    if (temptype[n] == 0x20)
                    {
                        if (temptype[n + 1] == 'u')
                        {
                            pretype = "unsigned ";
                            table_type_unsigned[colpos]=1;
                        }
                        break;
                    }
                }
            }
            typedifinde = " int ";

            outlist.append(pretype);
            outlist.append(typedifinde);
            outlist.append(bb);

            std::string defaultvalue;
            if (tableinfo[bb]["default"][0] == 'n' || tableinfo[bb]["default"][1] == 'N')
            {
                defaultvalue.append("0");
            }
            else
            {
               if(tableinfo[bb]["default"].size()>0)
               {
                  defaultvalue.append(tableinfo[bb]["default"]);
               }
               else
               {
                  defaultvalue.append("0");
               } 
                
            }

            outlist.append("= ");
            outlist.append(defaultvalue);
            outlist.append("; //");

            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            numbercollist.push_back({colpos, bb});
            collnametemp = pretype + typedifinde;

            colltypeshuzi[colpos] = 1;
        }
        if (temptype[0] == 'c' && temptype[1] == 'h' && temptype[2] == 'a' && temptype[3] == 'r')
        {
            // char
            int limitnumber = strtointval(tableinfo[bb]["limitchar"]);
            limitnumber += 1;

            outlist.append(" std::string ");

            outlist.append(bb);
            std::string defaultvalue;
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("; //");
            }
            else
            {
                outlist.append("=\"");
                outlist.append(tableinfo[bb]["default"]);
                outlist.append("\"; //");
            }

            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = "std::string";
            colltypeshuzi[colpos] = 30;
        }
        if (temptype[0] == 't' && temptype[1] == 'e' && temptype[2] == 'x' && temptype[3] == 't')
        {

            // text
            outlist.append(" std::string ");

            outlist.append(bb);

            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("; //");
            }
            else
            {
                outlist.append("=\"");
                outlist.append(tableinfo[bb]["default"]);
                outlist.append("\"; //");
            }

            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = "std::string";
            colltypeshuzi[colpos] = 30;
        }
        if (temptype[0] == 'e' && temptype[1] == 'n' && temptype[2] == 'u' && temptype[3] == 'm')
        {

            // enum
            outlist.append(" std::string ");

            outlist.append(bb);
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {

                std::string ttemp = tableinfo[bb]["limitchar"];
                std::string keyenumname;
                for (unsigned char jjk = 0; jjk < ttemp.size(); jjk++)
                {
                    if (ttemp[jjk] == '\'')
                    {
                        continue;
                    }
                    if (ttemp[jjk] == ',')
                    {
                        break;
                    }
                    keyenumname.push_back(ttemp[jjk]);
                    if (jjk > 200)
                    {
                        break;
                    }
                }

                outlist.append("=\"");
                outlist.append(keyenumname);
                outlist.append("\"; //");
            }
            else
            {
                outlist.append("=\"");
                outlist.append(tableinfo[bb]["default"]);
                outlist.append("\"; //");
            }

            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = "std::string";
            colltypeshuzi[colpos] = 30;
        }
        if (temptype[0] == 'm' && temptype[1] == 'e' && temptype[2] == 'd' && temptype[3] == 'i' && temptype[4] == 'u' && temptype[5] == 'm' && temptype[6] == 't' && temptype[7] == 'e' && temptype[8] == 'x' && temptype[9] == 't')
        {

            // mediumtext
            outlist.append(" std::string ");

            outlist.append(bb);

            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("; //");
            }
            else
            {
                outlist.append("=\"");
                outlist.append(tableinfo[bb]["default"]);
                outlist.append("\"; //");
            }

            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = "std::string";
            colltypeshuzi[colpos] = 30;
        }
        if (temptype[0] == 'l' && temptype[1] == 'o' && temptype[2] == 'n' && temptype[3] == 'g' && temptype[4] == 't' && temptype[5] == 'e' && temptype[6] == 'x' && temptype[7] == 't')
        {

            // longtext

            outlist.append(" std::string ");

            outlist.append(bb);

            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("; //");
            }
            else
            {
                outlist.append("=\"");
                outlist.append(tableinfo[bb]["default"]);
                outlist.append("\"; //");
            }

            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = "std::string";
            colltypeshuzi[colpos] = 30;
        }
        if (temptype[0] == 'v' && temptype[1] == 'a' && temptype[2] == 'r' && temptype[3] == 'c' && temptype[4] == 'h' && temptype[5] == 'a' && temptype[6] == 'r')
        {
            // varchar
            int limitnumber = strtointval(tableinfo[bb]["limitchar"]);
            limitnumber += 1;

            outlist.append(" std::string ");

            outlist.append(bb);
            std::string defaultvalue;
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("; //");
            }
            else
            {
                outlist.append("=\"");
                outlist.append(tableinfo[bb]["default"]);
                outlist.append("\"; //");
            }
            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = "std::string";
            colltypeshuzi[colpos] = 30;
        }
        if (temptype[0] == 't' && temptype[1] == 'i' && temptype[2] == 'n' && temptype[3] == 'y' && temptype[4] == 'i' && temptype[5] == 'n' && temptype[6] == 't')
        {

            // tinyint
            std::string pretype;
            std::string typedifinde;

            if (temptype.size() > 8)
            {
                for (int n = 0; n < temptype.size(); n++)
                {
                    if (temptype[n] == 0x20)
                    {
                        if (temptype[n + 1] == 'u')
                        {
                            pretype = "unsigned ";
                            table_type_unsigned[colpos]=1;
                        }
                        break;
                    }
                }
            }
            outlist.append(pretype);
            outlist.append(" char ");

            outlist.append(bb);
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("=0; //");
            }
            else
            {
                 if(tableinfo[bb]["default"].empty())
               {
                   outlist.append("=0; //");
               }
               else if (tableinfo[bb]["default"][0] >= '0' && tableinfo[bb]["default"][0] <= '9')
                {
                    outlist.append("=");
                    outlist.append(tableinfo[bb]["default"]);
                    outlist.append("; //");
                }
                else
                {
                    outlist.append("='");
                    outlist.append(tableinfo[bb]["default"]);
                    outlist.append("'; //");
                }
            }
            outlist.append(tableinfo[bb]["comment"]);
            metalist.push_back(outlist);
            numbercollist.push_back({colpos, bb});
            collnametemp = pretype + " int ";
            colltypeshuzi[colpos] = 1;
        }
        if (temptype[0] == 'b' && temptype[1] == 'i' && temptype[2] == 'g' && temptype[3] == 'i' && temptype[4] == 'n' && temptype[5] == 't')
        {

            // bigint
            std::string pretype;
            std::string typedifinde;

            if (temptype.size() > 7)
            {
                for (int n = 0; n < temptype.size(); n++)
                {
                    if (temptype[n] == 0x20)
                    {
                        if (temptype[n + 1] == 'u')
                        {
                            pretype = "unsigned ";
                            table_type_unsigned[colpos]=1;
                        }
                        break;
                    }
                }
            }

            outlist.append(pretype);
            outlist.append(" long long ");

            outlist.append(bb);

            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("=0; //");
            }
            else
            {
                if(tableinfo[bb]["default"].empty())
                {
                    outlist.append("=0; //");
                }
                else
                {
                    outlist.append("=");
                    outlist.append(tableinfo[bb]["default"]);
                    outlist.append("; //");
                } 
                
            }

            outlist.append(tableinfo[bb]["comment"]);
            metalist.push_back(outlist);
            numbercollist.push_back({colpos, bb});
            collnametemp = pretype + " long long ";
            colltypeshuzi[colpos] = 1;
        }
        if (temptype[0] == 't' && temptype[1] == 'i' && temptype[2] == 'm' && temptype[3] == 'e' && temptype[4] == 's' && temptype[5] == 't' && temptype[6] == 'a' && temptype[7] == 'm' && temptype[8] == 'p')
        {

            // timestamp
            std::string pretype;
            std::string typedifinde;

            pretype = "  ";

            outlist.append(pretype);
            outlist.append(" std::string ");

            outlist.append(bb);

            outlist.append("; //");

            outlist.append(tableinfo[bb]["comment"]);
            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = pretype + " std::string ";
            colltypeshuzi[colpos] = 60;
        }
        if (temptype[0] == 'd' && temptype[1] == 'a' && temptype[2] == 't' && temptype[3] == 'e' && temptype[4] == 't' && temptype[5] == 'i' && temptype[6] == 'm' && temptype[7] == 'e')
        {

            // datetime
            std::string pretype;
            std::string typedifinde;

            pretype = "  ";

            outlist.append(pretype);
            outlist.append(" std::string ");

            outlist.append(bb);

            outlist.append("; //");

            outlist.append(tableinfo[bb]["comment"]);
            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = pretype + " std::string ";
            colltypeshuzi[colpos] = 60;
        }
        else if (temptype[0] == 'd' && temptype[1] == 'a' && temptype[2] == 't' && temptype[3] == 'e')
        {

            // date
            std::string pretype;
            std::string typedifinde;

            pretype = "  ";

            outlist.append(pretype);
            outlist.append(" std::string ");

            outlist.append(bb);

            outlist.append("; //");

            outlist.append(tableinfo[bb]["comment"]);
            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = pretype + " std::string ";
            colltypeshuzi[colpos] = 61;
        }

        if (temptype[0] == 's' && temptype[1] == 'm' && temptype[2] == 'a' && temptype[3] == 'l' && temptype[4] == 'l' && temptype[5] == 'i' && temptype[6] == 'n' && temptype[7] == 't')
        {

            // smallint
            std::string pretype;
            std::string typedifinde;

            if (temptype.size() > 8)
            {
                for (int n = 0; n < temptype.size(); n++)
                {
                    if (temptype[n] == 0x20)
                    {
                        if (temptype[n + 1] == 'u')
                        {
                            pretype = "unsigned ";
                            table_type_unsigned[colpos]=1;
                        }
                        break;
                    }
                }
            }

            outlist.append(pretype);
            outlist.append(" short ");

            outlist.append(bb);
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("=0; //");
            }
            else
            {
                if(tableinfo[bb]["default"].empty())
                {
                    outlist.append("=0; //");
                }
                else
                {
                    outlist.append("=");
                    outlist.append(tableinfo[bb]["default"]);
                    outlist.append("; //");
                } 

            }
            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            numbercollist.push_back({colpos, bb});
            collnametemp = pretype + " int ";
            colltypeshuzi[colpos] = 1;
        }

        // decimal
        if (temptype[0] == 'd' && temptype[1] == 'e' && temptype[2] == 'c' && temptype[3] == 'i' && temptype[4] == 'm' && temptype[5] == 'a' && temptype[6] == 'l')
        {

            outlist.append(" float ");

            outlist.append(bb);
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("=0.0; //");
            }
            else
            {
                 if(tableinfo[bb]["default"].empty())
                {
                    outlist.append("=0.0; //");
                }
                else
                {
                    outlist.append("=");
                    outlist.append(tableinfo[bb]["default"]);
                    outlist.append("; //");
                } 

            }
            outlist.append(tableinfo[bb]["comment"]);
            metalist.push_back(outlist);
            floatcollist.push_back({colpos, bb});
            collnametemp = " double ";
            colltypeshuzi[colpos] = 20;
        }
        if (temptype[0] == 'f' && temptype[1] == 'l' && temptype[2] == 'o' && temptype[3] == 'a' && temptype[4] == 't')
        {
            // float
            outlist.append(" float ");

            outlist.append(bb);
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("=0.0; //");
            }
            else
            {
                outlist.append("=");
                outlist.append(tableinfo[bb]["default"]);
                outlist.append("; //");
            }
            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            floatcollist.push_back({colpos, bb});
            collnametemp = " double ";
            colltypeshuzi[colpos] = 20;
        }
        if (temptype[0] == 'd' && temptype[1] == 'o' && temptype[2] == 'u' && temptype[3] == 'b' && temptype[4] == 'l' && temptype[5] == 'e')
        {
            // double

            outlist.append(" double ");

            outlist.append(bb);
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("=0.0; //");
            }
            else
            {
                outlist.append("=");
                outlist.append(tableinfo[bb]["default"]);
                outlist.append("; //");
            }
            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);

            floatcollist.push_back({colpos, bb});
            collnametemp = " double ";
            colltypeshuzi[colpos] = 20;
        }
        if (temptype[0] == 'r' && temptype[1] == 'e' && temptype[2] == 'a' && temptype[3] == 'l')
        {
            // real

            outlist.append(" double ");

            outlist.append(bb);
            if (tableinfo[bb]["default"] == "<null>" || tableinfo[bb]["default"] == "null")
            {
                outlist.append("=0.0; //");
            }
            else
            {
                 if(tableinfo[bb]["default"].empty())
                {
                    outlist.append("=0.0; //");
                }
                else
                {
                    outlist.append("=");
                    outlist.append(tableinfo[bb]["default"]);
                    outlist.append("; //");
                } 
                
            }
            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);

            floatcollist.push_back({colpos, bb});
            collnametemp = " double ";
            colltypeshuzi[colpos] = 20;
        }
        if (collnametemp.size() == 0)
        {
            outlist.append(" std::string ");

            outlist.append(bb);

            outlist.append("; //");

            outlist.append(tableinfo[bb]["comment"]);

            metalist.push_back(outlist);
            stringcollist.push_back({colpos, bb});
            collnametemp = "std::string";
            colltypeshuzi[colpos] = 90;
        }
        collisttype[colpos] = collnametemp;
        colpos++;
    }

    std::string filebasename = modelspath;

    std::string modelfileclass = modelspath;
    std::string createfilename;
    std::string headtxt;
    if (modelfileclass.back() != '/')
    {
        modelfileclass.push_back('/');
    }

    createfilename = tablename;

    //  if(createfilename[0]>96&&createfilename[0]<123){
    // 				createfilename[0]=createfilename[0]-32;
    // 			}

    modelfileclass.append(createfilename);
    std::string modelfileclasscpp = modelfileclass;
    modelfileclasscpp.append(".cpp");

    std::ostringstream filemodelstremcpp;

    if (rmstag != "default")
    {
        filemodelstremcpp << "\n#include \"mysqlmodel.hpp\" \n#include \"";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << "/include/";
        filemodelstremcpp << tablenamebase << "base.h\"\n";
        filemodelstremcpp << "#include \"";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << "/include/";
        filemodelstremcpp << createfilename << ".h\"\n";
    }
    else
    {
        filemodelstremcpp << "#include \"mysqlmodel.hpp\" \n#include \"" << tablenamebase << "base.h\"\n#include \"" << createfilename << ".h\"\n";
    }

    filemodelstremcpp << "\n/* 如果此文件存在不会自动覆盖，没有则会自动生成。\n*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */\n";
    filemodelstremcpp << "\n";
    // filemodelstremcpp<<"class "<<createfilename<<" : public HTTP::mysqlclienDB<"<<createfilename<<","<<tablename<<"base>{\n";
    // filemodelstremcpp<<"\t public:\n";
    filemodelstremcpp << "\t \n namespace orm{\r\n";
    if (rmstag != "default")
    {
        filemodelstremcpp << "\t namespace ";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << "{ ";
    }
    filemodelstremcpp << " ";
    filemodelstremcpp << "\r\n\r\n\t\t\t " << createfilename << "::" << createfilename << "(std::string dbtag):mysqlclientDB(dbtag){}\n";
    filemodelstremcpp << "\t\t\t " << createfilename << "::" << createfilename << "():mysqlclientDB(){}\n";
    if (rmstag != "default")
    {
        filemodelstremcpp << "\r\n\r\n\t\t} ";
    }
    filemodelstremcpp << "\r\n\r\n\t  }\n";

    // filemodelstremcpp<<"};\n";

    headtxt.append(filemodelstremcpp.str());
    filemodelstremcpp.str("");

    if (!fs::exists(modelfileclasscpp))
    {

        FILE *ff = fopen(modelfileclasscpp.c_str(), "wb");
        fwrite(&headtxt[0], 1, headtxt.size(), ff);

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
        filemodelstremcpp << "\n#include \"mysqlmodel.hpp\" \n#include \"";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << "/include/";
        filemodelstremcpp << tablenamebase << "base.h\"\n";
    }
    else
    {
        filemodelstremcpp << "\n#include \"mysqlmodel.hpp\" \n#include \"" << tablenamebase << "base.h\"\n";
    }
    filemodelstremcpp << "\n/* 如果此文件存在不会自动覆盖，没有则会自动生成。\n*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */\n";
    filemodelstremcpp << "\n namespace orm {\n";
    if (rmstag != "default")
    {
        filemodelstremcpp << "\t namespace ";
        filemodelstremcpp << rmstag;
        filemodelstremcpp << " { \n";
    }
    filemodelstremcpp << "\t\tclass " << createfilename << " : public mysqlclientDB<" << createfilename << "," << tablenamebase << "base>{\n";
    filemodelstremcpp << "\t\t public:\n";
    filemodelstremcpp << "\t\t " << createfilename << "(std::string dbtag);\n";
    filemodelstremcpp << "\t\t " << createfilename << "();\n";
    filemodelstremcpp << "\t\t};\n";
    if (rmstag != "default")
    {
        filemodelstremcpp << "\t }\n ";
    }
    filemodelstremcpp << "};\n#endif\n";

    headtxt.clear();
    headtxt.append(filemodelstremcpp.str());
    filemodelstremcpp.str("");

    if (!fs::exists(modelfileclasscpp))
    {

        FILE *ff = fopen(modelfileclasscpp.c_str(), "wb");
        fwrite(&headtxt[0], 1, headtxt.size(), ff);

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

    // if(filebasename.back()!='/'){
    //     filebasename.push_back('/');
    // }
    //   filebasename.append("meta/");
    filebasename.append(filebasefilename);
    filebasename.append("base.h");

    FILE *f = fopen(filebasename.c_str(), "wb");

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
*This file is auto create from cli
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
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mysql.h"
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
    headtxt += R"(base
{
    struct meta{
    )";

    std::ostringstream filemodelstrem;

    for (int j = 0; j < metalist.size(); j++)
    {
        filemodelstrem << metalist[j] << std::endl;
    }
    filemodelstrem << " } data;\n ";
    filemodelstrem << "std::vector<" << tablenamebase << "base::meta> record;\n";
    filemodelstrem << "std::string _rmstag=\"" << rmstag << "\";//this value must be default or tag value, tag in mysqlconnect config file .\n";
    filemodelstrem << "std::vector<unsigned char> _keypos{0x00};\n";
    filemodelstrem << "MYSQL_ROW _row;\n";

    filemodelstrem << "std::vector<" << tablenamebase << "base::meta>::iterator begin(){     return record.begin(); }\n";
    filemodelstrem << "std::vector<" << tablenamebase << "base::meta>::iterator end(){     return record.end(); }\n";
    filemodelstrem << "std::vector<" << tablenamebase << "base::meta>::const_iterator begin() const{     return record.begin(); }\n";
    filemodelstrem << "std::vector<" << tablenamebase << "base::meta>::const_iterator end() const{     return record.end(); }\n";

    filemodelstrem << "const std::array<std::string," << std::to_string(tablecollist.size()) << "> colnames={";

    for (int j = 0; j < tablecollist.size(); j++)
    {
        if (j > 0)
        {
            filemodelstrem << ",";
        }
        filemodelstrem << "\"" << tablecollist[j] << "\"";
    }
    filemodelstrem << "};\r\n";

    filemodelstrem << "const std::array<unsigned char," << std::to_string(tablecollist.size()) << "> colnamestype= {";

    for (unsigned char j = 0; j < tablecollist.size(); j++)
    {

        if (j > 0)
        {
            filemodelstrem << ",";
        }
        filemodelstrem << std::to_string(table_type[j]);
    }
    filemodelstrem << "};\r\n";

    headtxt.append(filemodelstrem.str());
    filemodelstrem.str("");
    // 头部定义

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    headtxt = "std::string tablename=\"";
    headtxt.append(realtablename);
    headtxt += "\";\n";
    headtxt.append("std::string modelname=\"");
    headtxt.append(tablename);
    headtxt.append("\";\n");

    headtxt += R"(
	  unsigned char findcolpos(std::string coln){
		    unsigned char  bi=coln[0];
            char colpospppc;
	         if(bi<91&&bi>64){
				bi+=32;
			}
            switch(coln[0]){


         )";
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

            for (int m = 0; m < itter->second.size(); m++)
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
                    for (int mm = 0; mm < ittter->second.size(); mm++)
                    {
                        ttttp = tablecollist[ittter->second[mm]].back();
                        backcolz[ttttp].emplace_back(ittter->second[mm]);
                    }
                    std::ostringstream backcachep;
                    backcachep << "  colpospppc=coln.back();\n    if(colpospppc<91&&bi>64){ colpospppc+=32; }\n";
                    bool isbackppp = false;
                    std::map<unsigned char, unsigned char> hasbackpos;
                    for (auto ipper = backcolz.begin(); ipper != backcolz.end(); ipper++)
                    {

                        if (ipper->second.size() == 1)
                        {
                            backcachep << " if(colpospppc=='" << tablecollist[ipper->second[0]].back() << "'){ return " << std::to_string(ipper->second[0]) << "; }\n";
                            isbackppp = true;
                            hasbackpos[ipper->second[0]] = 1;
                        }
                    }
                    if (isbackppp)
                    {
                        filemodelstrem << backcachep.str();
                    }
                    for (int mm = 0; mm < ittter->second.size(); mm++)
                    {
                        auto ippter = hasbackpos.find(ittter->second[mm]);
                        if (ippter == hasbackpos.end())
                        {
                            filemodelstrem << " if(strcasecmp(coln.c_str(), \"" << tablecollist[ittter->second[mm]] << "\") == 0){ return " << std::to_string(ittter->second[mm]) << "; }\n";
                        }
                    }
                    filemodelstrem << "   \t break;\n";
                }
            }

            filemodelstrem << " }\n break;\n";
        }
    }

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

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
      void _setColnamevalue(){
          )";
    headtxt += tablenamebase;
    headtxt += R"(base::meta metatemp;   
         for(unsigned char i=0;i<_keypos.size();i++){
                 switch(_keypos[i]){
        )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    filemodelstrem.str("");
 
    for (int j = 0; j < tablecollist.size(); j++)
    {
         if(table_type[j]<10)
        {
                    switch(table_type[j])
                    {
                        case 0:
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stof(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0.0;\n\t\t\t }\n\t\t\tbreak;\n";
                            break;
                        case 1:
                           if(table_type_unsigned[j]==1)
                           {
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoi(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
                           }
                           else
                           {
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoi(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
                           }


                            break;  
                        case 2:
     
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoi(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";

                            break;
                        case 3:
                           if(table_type_unsigned[j]==1)
                           {
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoul(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
                           }
                           else
                           {
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoi(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
                           }
                            break;  

                        case 4:
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stof(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0.0;\n\t\t\t }\n\t\t\tbreak;\n";

                            break;  
                       case 5:
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stod(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0.0;\n\t\t\t }\n\t\t\tbreak;\n";

                            break;
                        case 7:
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<".append(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<".clear();\n\t\t\t }\n\t\t\tbreak;\n";

                            break;    
                        case 8:
                            if(table_type_unsigned[j]==1)
                            {
                                filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoull(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
                            }
                            else
                            {
                                filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoll(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
                            }
                            break;    
                        case 9:
                            filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoi(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
                    
                    }
            }else if(table_type[j]==12)
            {
                //filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stoul(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0;\n\t\t\t }\n\t\t\tbreak;\n";
                filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<".append(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<".clear();\n\t\t\t }\n\t\t\tbreak;\n";
            } else if(table_type[j]==246)
            {
                filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<"=std::stof(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<"=0.0;\n\t\t\t }\n\t\t\tbreak;\n";
            }
            else
            {
                filemodelstrem<<"\tcase "<<std::to_string(j)<<":\n\t\t try{\n\t\t\tmetatemp."<<tablecollist[j]<<".append(_row["<<std::to_string(j)<<"]);\n\t\t}catch (...) { \n\t\t\tmetatemp."<<tablecollist[j]<<".clear();\n\t\t\t }\n\t\t\tbreak;\n";

            }

        
    }
 
    filemodelstrem<<"\tdefault:\n\t\t { }\n\t\t\t\n";
    

    headtxt.clear();
    headtxt.append(filemodelstrem.str());
    // filemodelstrem.str("");
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                 }

                 if(i>210){
                     break;
                 }
          }
          data=metatemp;
          record.emplace_back(metatemp);   
   } 
         void _addnewrowvalue(){
           )";
    headtxt += tablenamebase;
    headtxt += R"(base::meta metatemp;   

          for(unsigned char i=0;i<_keypos.size();i++){
 
                 switch(_keypos[i]){

        )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt.append(filemodelstrem.str());
    filemodelstrem.str("");
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                  }
                 if(i>210){
                     break;
                 }
          }
           record.emplace_back(std::move(metatemp)); 
   } 

  inline  std::string stringaddslash(std::string &content){
        std::string temp;
        for(int i=0;i<content.size();i++){
            if(content[i]=='\''){
                temp.append("\\'");
                continue;
            }else if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }
            temp.push_back(content[i]);
        }
        return temp;
   }  
  inline  std::string jsonaddslash(std::string &content){
        std::string temp;
        for(int i=0;i<content.size();i++){
            if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }
            temp.push_back(content[i]);
        }
        return temp;
   }  

   std::string _makeinsertsql(){
       int j=0;
                std::ostringstream tempsql;
                tempsql<<"INSERT INTO ";
                    tempsql<<tablename;
                   tempsql<<" (";
                    for(;j<colnames.size();j++){
                            if(j>0){
                                tempsql<<"`,`";
                            }else{
                                tempsql<<"`";
                            }
                            tempsql<<colnames[j];
                    }
                    if(j>0){
                        tempsql<<"`";
                    }
            tempsql<<") VALUES (";

        )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    std::string insertstring;
    std::ostringstream insertstrem;

    for (int j = 0; j < tablecollist.size(); j++)
    {

        // 数字
        // if (j == 0)
        if (table_type[j]<10||table_type[j]==246)
        {
           if(table_type[j]!=7)
           {
            insertstrem << "if(data." << tablecollist[j] << "==0){\n";
            insertstrem << "tempsql<<\"null\";\n";
            insertstrem << " }else{ \n";
            insertstrem << "\ttempsql<<std::to_string(data." << tablecollist[j] << ");\n";
            insertstrem << "}\n";
            continue;
           } 
            
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

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
     
       return tempsql.str();
   }     
    std::string _makeupdatesql(std::string fileld){
       int j=0;
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

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    std::string updatestring;
    std::ostringstream updatestrem;

    for (int j = 0; j < tablecollist.size(); j++)
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
                updatestrem << "\ttempsql<<\",`" << tablecollist[j] << "`=\"<<std::to_string(data." << tablecollist[j] << ");\n";
            }
            else
            {
                updatestrem << "\ttempsql<<\"`" << tablecollist[j] << "`=\"<<std::to_string(data." << tablecollist[j] << ");\n";
            }

            updatestrem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            updatestrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            updatestrem << "tempsql<<\",`" << tablecollist[j] << "`=CURRENT_TIMESTAMP\";\n";
            updatestrem << " }else{ \n tempsql<<\",`" << tablecollist[j] << "'='\"<<data." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            updatestrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            updatestrem << "tempsql<<\",`" << tablecollist[j] << "`=CURRENT_DATE\";\n";
            updatestrem << " }else{ \n tempsql<<\",`" << tablecollist[j] << "'='\"<<data." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                updatestrem << "tempsql<<\",`" << tablecollist[j] << "`='\"<<stringaddslash(data." << tablecollist[j] << ")<<\"'\";\n";
            }
            else
            {
                updatestrem << "tempsql<<\"`" << tablecollist[j] << "`='\"<<stringaddslash(data." << tablecollist[j] << ")<<\"'\";\n";
            }
        }
    }
    headtxt.append(updatestrem.str());

    headtxt.append(" }else{ \n");

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
     
  int jj=0;
                  std::string keyname;
                  std::vector<unsigned char> keypos;
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
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){

        )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    std::string update2string;
    std::ostringstream update2strem;

    for (int j = 0; j < tablecollist.size(); j++)
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
                update2strem << "\ttempsql<<\"`" << tablecollist[j] << "`=\"<<std::to_string(data." << tablecollist[j] << ");\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"`" << tablecollist[j] << "`=\"<<std::to_string(data." << tablecollist[j] << ");\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"`" << tablecollist[j] << "`=CURRENT_TIMESTAMP\";\n";
            update2strem << " }else{ \n tempsql<<\",`" << tablecollist[j] << "'='\"<<data." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"`" << tablecollist[j] << "`=CURRENT_DATE\";\n";
            update2strem << " }else{ \n tempsql<<\"`" << tablecollist[j] << "'='\"<<data." << tablecollist[j] << "<<\"'\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "tempsql<<\"`" << tablecollist[j] << "`='\"<<stringaddslash(data." << tablecollist[j] << ")<<\"'\";\n";
            }
            else
            {
                update2strem << "tempsql<<\"`" << tablecollist[j] << "`='\"<<stringaddslash(data." << tablecollist[j] << ")<<\"'\";\n";
            }
        }
        update2strem << " break;\n";
    }
    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
     
                  default:
                                ;
                     }
                 }   

            }        

        return tempsql.str();
   } 
   std::string datatojson(){
       std::ostringstream tempsql;

        )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    std::ostringstream jsonstrem;
    headtxt = "tempsql<<\"{\";\n";
    for (int j = 0; j < tablecollist.size(); j++)
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
                jsonstrem << "\ttempsql<<\",\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(data." << tablecollist[j] << ");\n";
            }
            else
            {
                jsonstrem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(data." << tablecollist[j] << ");\n";
            }
            // jsonstrem<<"\ttempsql<<\"}\";\n";

            jsonstrem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            jsonstrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            jsonstrem << "tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
            jsonstrem << " }else{ \n tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"\"<<data." << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            jsonstrem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            jsonstrem << "tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
            jsonstrem << " }else{ \n tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"\"<<data." << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                jsonstrem << "tempsql<<\",\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(data." << tablecollist[j] << ");\n";
            }
            else
            {
                jsonstrem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(data." << tablecollist[j] << ");\n";
            }

            jsonstrem << "tempsql<<\"\\\"\";\n";
        }
    }
    headtxt.append(jsonstrem.str());
    headtxt.append("tempsql<<\"}\";\n");

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
     
     return tempsql.str();             
   }   
   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ///////////////////////////////////////////////
    headtxt = R"(
   std::string datatojson(std::string fileld){
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
                     for(jj=0;jj<colnames.size();jj++){
                         keypos.emplace_back(jj); 
                     }
                 }
                 tempsql<<"{";
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){
        )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    update2strem.str("");

    for (int j = 0; j < tablecollist.size(); j++)
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
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(data." << tablecollist[j] << ");\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(data." << tablecollist[j] << ");\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<data." << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<data." << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(data." << tablecollist[j] << ")<<\"\\\"\";\n";
            }
            else
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(data." << tablecollist[j] << ")<<\"\\\"\";\n";
            }
        }
        update2strem << " break;\n";
    }

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], 1, headtxt.size(), f);
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

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ///////////////////////////////////////////////
    headtxt = R"(
   std::string tojson(std::string fileld=""){
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
                     for(jj=0;jj<colnames.size();jj++){
                         keypos.emplace_back(jj); 
                     }
                 }
                tempsql<<"[";
              for(int n=0;n<record.size();n++){
                  if(n>0){
                      tempsql<<",{";
                  }else{
                      tempsql<<"{";
                  }  
                 
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){
        )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    update2strem.str("");

    for (int j = 0; j < tablecollist.size(); j++)
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
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(record[n]." << tablecollist[j] << ");\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(record[n]." << tablecollist[j] << ");\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<record[n]." << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<record[n]." << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(record[n]." << tablecollist[j] << ")<<\"\\\"\";\n";
            }
            else
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(record[n]." << tablecollist[j] << ")<<\"\\\"\";\n";
            }
        }
        update2strem << " break;\n";
    }

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], 1, headtxt.size(), f);
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

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    ///////////////////////////////////////////////
    headtxt = R"(
   std::string tojson(std::function<bool(std::string&,meta&)> func,std::string fileld=""){
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
                     for(jj=0;jj<colnames.size();jj++){
                         keypos.emplace_back(jj); 
                     }
                 }
                tempsql<<"[";
              for(int n=0;n<record.size();n++){
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
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    update2strem.str("");

    for (int j = 0; j < tablecollist.size(); j++)
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
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(record[n]." << tablecollist[j] << ");\n";
            }
            else
            {
                update2strem << "\ttempsql<<\"\\\"" << tablecollist[j] << "\\\":\"<<std::to_string(record[n]." << tablecollist[j] << ");\n";
            }

            update2strem << "}\n";
        }
        else if (colltypeshuzi[j] == 60)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00 00:00:00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<record[n]." << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else if (colltypeshuzi[j] == 61)
        {
            update2strem << "  \nif(data." << tablecollist[j] << ".size()==0){ \n";
            update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"0000-00-00\\\"\";\n";
            update2strem << " }else{ \n tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<record[n]." << tablecollist[j] << "<<\"\\\"\";\n }\n";
        }
        else
        {
            if (j > 0)
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(record[n]." << tablecollist[j] << ")<<\"\\\"\";\n";
            }
            else
            {
                update2strem << "tempsql<<\"\\\"" << tablecollist[j] << "\\\":\\\"\"<<http::utf8_to_jsonstring(record[n]." << tablecollist[j] << ")<<\"\\\"\";\n";
            }
        }
        update2strem << " break;\n";
    }

    headtxt.append(update2strem.str());

    fwrite(&headtxt[0], 1, headtxt.size(), f);
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

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    //////////////////////////////////////////////////////////
    if(tablepkname.empty())
    {
        headtxt = "long long getPK(){  return data." + tablepriname + "; } \n";
        headtxt.append(" void setPK(long long val){   } \n");
    }
    else
    {
        headtxt = "long long getPK(){  return data." + tablepkname + "; } \n";
        headtxt.append(" void setPK(long long val){  data." + tablepkname + "=val;} \n");
    }


    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    std::ostringstream getsetstrem;
    std::string uptempstring;
    // 自动get set方法
    for (int j = 0; j < tablecollist.size(); j++)
    {
        getsetstrem << collisttype[j];
        uptempstring = tablecollist[j];
        if (uptempstring[0] >= 'a' && uptempstring[0] <= 'z')
        {
            uptempstring[0] = uptempstring[0] - 32;
        }
        getsetstrem << " get" << uptempstring << "(){  return data." + tablecollist[j] + "; } \n";

        if (colltypeshuzi[j] > 29)
        {
            getsetstrem << collisttype[j];
            getsetstrem << "& getRef" << uptempstring << "(){  return std::ref(data." + tablecollist[j] + "); } \n";
            getsetstrem << " void set" << uptempstring << "(" << collisttype[j] << " &val){  data." + tablecollist[j] + "=val;} \n";
            if (collisttype[j].find("std::string")!= std::string::npos)
            {
                getsetstrem << " void set" << uptempstring << "(std::string_view val){  data." + tablecollist[j] + "=val;} \n";
            }
            else
            {
                getsetstrem << " void set" << uptempstring << "(" << collisttype[j] << " &val){  data." + tablecollist[j] + "=val;} \n";
                getsetstrem << " void set" << uptempstring << "(" << collisttype[j] << " val){  data." + tablecollist[j] + "=val;} \n";
            }
        }
        else
        {
            getsetstrem << " void set" << uptempstring << "(" << collisttype[j] << " val){  data." + tablecollist[j] + "=val;} \n";
        }
        getsetstrem << "\n";
    }

    headtxt.append(getsetstrem.str());

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = tablenamebase + "base::meta getnewData(){\n \t struct meta newdata;\n\t return newdata; \n} \n";

    headtxt.append(tablenamebase);
    headtxt.append("base::meta getData(){\n \t return data; \n} \n");
    headtxt.append("std::vector<");
    headtxt.append(tablenamebase);
    headtxt.append("base::meta> getRecord(){\n \t return record; \n} \n");

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    std::ostringstream getcollstrem;

    headtxt = R"(
    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >  
    std::vector<T> getCol(std::string keyname){
         std::vector<T> a;
         unsigned char kpos;
         kpos=findcolpos(keyname);
        for(auto &iter:record){
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a.emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
          }
        }
        return a;
    }
    )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    //////////////////

    headtxt = R"(
    template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >    
    std::vector<T> getCol(std::string keyname){
        std::vector<T> a;
         unsigned char kpos;
         kpos=findcolpos(keyname);
        for(auto &iter:record){
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a.emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
    }
            }
            return a;
        }  
    )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////

    ////////////////////////////////////////////////

    headtxt = R"(
    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
    T getVal(std::string keyname){
         
         unsigned char kpos;
         kpos=findcolpos(keyname);
      
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t return data." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }
    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
    }
            return 0;
        }  
    )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ///------------------
    headtxt = R"(
     template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
    T getVal()";
    headtxt += tablenamebase;
    headtxt += R"(base::meta & iter,std::string keyname){
         
         unsigned char kpos;
         kpos=findcolpos(keyname);
       
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t return iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
    }
            return 0;
        }  
    )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    //////////////////////////

    headtxt = R"(
        template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
        T getVal(std::string keyname){
            
            unsigned char kpos;
            kpos=findcolpos(keyname);
        
            switch(kpos){

    )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t return data." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
    }
            return 0.0;
        }  
    )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //--------------------
    headtxt = R"(
     template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
    T getVal()";
    headtxt += tablenamebase;
    headtxt += R"(base::meta & iter,std::string keyname){
         
         unsigned char kpos;
        kpos=findcolpos(keyname);
         
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");
    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t return iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
    }
            return 0.0;
        }  
    )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    /////////////////////////

    headtxt = R"(
    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
    std::string getVal(std::string keyname){
         
         unsigned char kpos;
        kpos=findcolpos(keyname);
        
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t return data." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
  }
        return "";
    }  
   )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    //-----------------------

    headtxt = R"(
    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
    std::string getVal()";
    headtxt += tablenamebase;
    headtxt += R"(base::meta & iter,std::string keyname){
         
         unsigned char kpos;
        kpos=findcolpos(keyname);
       
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t return iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
    }
            return "";
        }  
    )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    /////////////////////////
    headtxt = R"( 
    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >   
    std::vector<std::string> getCol(std::string keyname){
        std::vector<std::string> a;
         unsigned char kpos;
        kpos=findcolpos(keyname);
 
         for(auto &iter:record){
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a.emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
   }
        }       

        return a;
    }  
    std::string getstrCol(std::string keyname,bool isyinhao=false){
        std::ostringstream a;
             unsigned char kpos;
        kpos=findcolpos(keyname);
  
         int j=0;
         if(isyinhao&&record.size()>0){
             a<<'"';
         }
         for(auto &iter:record){
                if(j>0){
                    if(isyinhao){
                        a<<"\",\"";
                    }else{
                        a<<',';    
                    }
                }
                 switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (int jj = 0; jj < tablecollist.size(); jj++)
    {

        getcollstrem << "case " << std::to_string(jj) << ": \n ";

        if (colltypeshuzi[jj] < 30)
        {
            getcollstrem << "\t a<<std::to_string(iter." << tablecollist[jj] << ");"
                         << "\n";
        }
        else
        {
            getcollstrem << "\t if(isyinhao){ a<<jsonaddslash(iter." << tablecollist[jj] << "); \n\t }else{\n";
            getcollstrem << "\t a<<iter." << tablecollist[jj] << ";\n";
            getcollstrem << "\t }\n";
        }

        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
    }
                j++;
        } 
        if(isyinhao&&j>0){
             a<<'"';
        }      
        return a.str();
    }
    )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////

    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>     
    std::map<std::string,std::string> getCols(std::string keyname,std::string valname){
        std::map<std::string,std::string> a;
        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
         std::string ktemp,vtemp;
         for(auto &iter:record){
                
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t } \n\t\t switch(vpos){ \n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
        }
                    a.emplace(ktemp,vtemp);
            }       
            return a;
        } )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    ///////////////////////
    headtxt = R"(

  template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
  std::map<std::string,U> getCols(std::string keyname,std::string valname){
        std::map<std::string,U> a;
        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
         std::string ktemp;
         U vtemp;
         for(auto &iter:record){
                
                switch(kpos){
 
       )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t } \n        switch(vpos){ \n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
            }
                a.emplace(ktemp,vtemp);
        }       
        return a;
    } )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    ///////////////////////////////////////////////////////
    headtxt = R"(
  template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>       
  std::map<T,U> getCols(std::string keyname,std::string valname){
        std::map<T,U> a;
        unsigned char kpos,vpos;
         kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
         T ktemp;
         U vtemp;
         for(auto &iter:record){
                
                switch(kpos){
 
       )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t } \n \t\t   switch(vpos){ \n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
             }
                a.emplace(ktemp,vtemp);
        }       
        return a;
    } )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    /////////////////////////
    headtxt = R"( 
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>      
    std::map<T,std::string> getCols(std::string keyname,std::string valname){
        std::map<T,std::string> a;
                unsigned char kpos,vpos;
         kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             std::string vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
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

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
  }
                a.emplace(ktemp,vtemp);
        }       
        return a;
    }     )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ////////////////////////////

    headtxt = R"(
     template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>       
    std::map<std::string,U> getCols(std::string keyname,std::string valname){
        std::map<std::string,U> a;
             unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             std::string  ktemp;
             U  vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                 }

             
                a.emplace(ktemp,vtemp);
        }       
        return a;
    }  )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    /////////////////////////
    headtxt = R"(
    template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>   
    std::map<T,U> getCols(std::string keyname,std::string valname){
        std::map<T,U> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }

                    a.emplace(ktemp,vtemp);
            }       

            return a;
        }   )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ////////////////////////////
    headtxt = R"(
    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >         
    std::map<T,meta> getmapRows(std::string keyname){
        std::map<T,meta> a;

            unsigned char kpos;
         kpos=findcolpos(keyname);
 
          //   long long ktemp;
        
         for(auto &iter:record){
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        // getcollstrem<<"\t ktemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t a.emplace(iter." << kaa.second << ",iter);\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                }
        }       
        return a;
    }     )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////////

    headtxt = R"(
   template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >    
   std::map<std::string,meta> getmapRows(std::string keyname){
        std::map<std::string,meta> a;

         unsigned char kpos;
 
        kpos=findcolpos(keyname);
    

        //std::string ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        // getcollstrem<<"\t ktemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t a.emplace(iter." << kaa.second << ",iter);\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                }

                //a.emplace(ktemp,iter);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    /////////////////////////////////
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>  
    std::vector<std::pair<std::string,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<std::string,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             std::string ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////
    headtxt = R"(
  template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ////////////////////////////////////////////
    headtxt = R"(
  template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
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

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ////////////////////////////////////////////////////
    headtxt = R"(
  template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>     
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////////

    headtxt = R"(
  template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ////////////////////////////////////////////////////////
    headtxt = R"(
  template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>     
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

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

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   )";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ///////////////////////////////////////////////////
    headtxt = R"(
    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
    std::vector<std::pair<T,meta>> getvecRows(std::string keyname){
        std::vector<std::pair<T,meta>> a;

         unsigned char kpos;
 
        kpos=findcolpos(keyname);
    

       // T ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        // getcollstrem<<"\t ktemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t a.emplace_back(iter." << kaa.second << ",iter);\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                }

                
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ///////////////////////////////////////////
    headtxt = R"(
    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >  
    std::vector<std::pair<std::string,meta>> getvecRows(std::string keyname){
        std::vector<std::pair<std::string,meta>> a;

         unsigned char kpos;
        kpos=findcolpos(keyname);
        //std::string ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        // getcollstrem<<"\t ktemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t a.emplace_back(iter." << kaa.second << ",iter);\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                }

                
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    ////////////////////////////////////////////////////
    // groupby int int float
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        
        T ktemp;
        U vtemp;
        //D vtemp;
         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        // getcollstrem<<"\t dtemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        // a[ktemp][vtemp].emplace_back(dtemp);
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////
    // groupby int int int
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
        //D vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    /////////////////////////
    // groupby int int string
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    // groupby int string float
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    // groupby int string int
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    // groupby int string string
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    /////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // groupby string int float
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        
        T ktemp;
        U vtemp;
        //D vtemp;
         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";
        // getcollstrem<<"\t dtemp=iter."<<kaa.second<<";"<<"\n";
        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        // a[ktemp][vtemp].emplace_back(dtemp);
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////
    // groupby string int int
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
        //D vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    /////////////////////////
    // groupby string int string
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    // groupby string string float
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    // groupby string string int
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    // groupby string string string
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t vtemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(dpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][vtemp].emplace_back(iter." << kaa.second << ");\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    // 2D data
    /////////////////////////////////////////////
    ////////////////////////////
    // groupby string string
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ///////////////////////////////////////////
    ////////////////////////////
    // groupby string float
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////
    // groupby string int
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////////
    ////////////////////////////////////////////
    ////////////////////////////
    // groupby int string
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ///////////////////////////////////////////
    ////////////////////////////
    // groupby int float
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : floatcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////
    // groupby int int
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp].emplace_back(iter." << kaa.second << ");"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////
    // meta row
    //////////////////////////////////////////
    // groupby int meta
    headtxt = R"(
    template<typename T,typename std::enable_if<std::is_integral_v<T>,bool>::type = true>    
  std::map<T,std::vector<meta>> getgroupRows(std::string keyname){
        std::map<T,std::vector<meta>> a;

        unsigned char kpos;
        kpos=findcolpos(keyname);

        //T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    ///////////////////////////////////////////////
    // groupby string meta
    headtxt = R"(
    template<typename T,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>    
  std::map<T,std::vector<meta>> getgroupRows(std::string keyname){
        std::map<T,std::vector<meta>> a;

        unsigned char kpos;
        kpos=findcolpos(keyname);

        //T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////
    ////3D meta row
    ////////////////////////////////////
    // groupby string string meta
    headtxt = R"(
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname){
        std::map<T,std::map<U,std::vector<meta>>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        T ktemp;

         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////
    // groupby string int meta
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname){
        std::map<T,std::map<U,std::vector<meta>>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        T ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////
    // groupby int int meta
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<U>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname){
        std::map<T,std::map<U,std::vector<meta>>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        T ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////
    // groupby int string meta
    headtxt = R"(
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname){
        std::map<T,std::map<U,std::vector<meta>>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        T ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    getcollstrem.str("");

    for (auto &kaa : numbercollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t ktemp=iter." << kaa.second << ";"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    getcollstrem << "\t  }\n\n switch(vpos){\n";

    for (auto &kaa : stringcollist)
    {

        getcollstrem << "case " << std::to_string(kaa.first) << ": \n ";

        getcollstrem << "\t a[ktemp][iter." << kaa.second << "].emplace_back(iter);"
                     << "\n";
        getcollstrem << "\t break;\n";
    }

    headtxt.append(getcollstrem.str());
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    headtxt = R"(
                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    })";
    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();
    //////////////////////////////////////////

    //////////////////////////////////////////////
    //////////////////////
    headtxt = R"(
};
)";
    if (rmstag != "default")
    {
        headtxt.append(" } ");
    }
    headtxt += R"(
}
#endif
   )";

    fwrite(&headtxt[0], 1, headtxt.size(), f);
    headtxt.clear();

    fclose(f);

    std::cout << " create table metainfo file: \033[1m\033[31m" << filebasename << "\033[0m" << std::endl;
    return 0;
}
struct mysqlconnect_t
{
    std::string type;
    std::string host;
    std::string port;
    std::string dbname;
    std::string user;
    std::string unix_socket;
    std::string password;
    std::string pretable;
    std::string maxpool;
    std::string spname;
};

std::vector<mysqlconnect_t> getmysqlconfig(std::string filename)
{
    std::vector<mysqlconnect_t> myconfig;
    FILE *f = fopen(filename.c_str(), "rb");
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
    for (int i = 0; i < s.size(); i++)
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
                        typeone = strval;
                        mysqlconf.spname = keyname;
                    }
                    else
                    {

                        myconfig.push_back(mysqlconf);
                        typeone = strval;
                        mysqlconf.type = strval;
                        mysqlconf.host.clear();
                        mysqlconf.port.clear();
                        mysqlconf.dbname.clear();
                        mysqlconf.user.clear();
                        mysqlconf.password.clear();
                        mysqlconf.pretable.clear();
                        mysqlconf.maxpool.clear();
                        mysqlconf.spname = keyname;
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
                if (strcasecmp(linestr.c_str(), "type") == 0)
                {
                    if (typeone.empty())
                    {
                        typeone = strval;
                        mysqlconf.spname = keyname;
                    }
                    else
                    {

                        myconfig.push_back(mysqlconf);
                        typeone = strval;
                        mysqlconf.type = strval;
                        mysqlconf.host.clear();
                        mysqlconf.port.clear();
                        mysqlconf.dbname.clear();
                        mysqlconf.user.clear();
                        mysqlconf.password.clear();
                        mysqlconf.pretable.clear();
                        mysqlconf.maxpool.clear();
                        mysqlconf.spname = keyname;
                    }
                }
                if (strcasecmp(linestr.c_str(), "host") == 0)
                {
                    mysqlconf.host = strval;
                }
                if (strcasecmp(linestr.c_str(), "port") == 0)
                {
                    mysqlconf.port = strval;
                }
                if (strcasecmp(linestr.c_str(), "dbname") == 0)
                {
                    mysqlconf.dbname = strval;
                }
                if (strcasecmp(linestr.c_str(), "user") == 0)
                {

                    mysqlconf.user = strval;
                }
                if (strcasecmp(linestr.c_str(), "password") == 0)
                {
                    mysqlconf.password = strval;
                }
                if (strcasecmp(linestr.c_str(), "pretable") == 0)
                {
                    mysqlconf.pretable = strval;
                }
                if (strcasecmp(linestr.c_str(), "maxpool") == 0)
                {
                    mysqlconf.maxpool = strval;
                }
                if (strcasecmp(linestr.c_str(), "unix_socket") == 0)
                {
                    mysqlconf.unix_socket = strval;
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
        // std::cout<<s[i];
    }
    if (mysqlconf.host.size() > 0)
    {
        if (strcasecmp(linestr.c_str(), "host") == 0)
        {
            mysqlconf.host = strval;
        }
        if (strcasecmp(linestr.c_str(), "port") == 0)
        {
            mysqlconf.port = strval;
        }
        if (strcasecmp(linestr.c_str(), "dbname") == 0)
        {
            mysqlconf.dbname = strval;
        }
        if (strcasecmp(linestr.c_str(), "user") == 0)
        {
            mysqlconf.user = strval;
        }
        if (strcasecmp(linestr.c_str(), "password") == 0)
        {
            mysqlconf.password = strval;
        }
        if (strcasecmp(linestr.c_str(), "pretable") == 0)
        {
            mysqlconf.pretable = strval;
        }
        if (strcasecmp(linestr.c_str(), "maxpool") == 0)
        {
            mysqlconf.maxpool = strval;
        }
        if (strcasecmp(linestr.c_str(), "unix_socket") == 0)
        {
            mysqlconf.unix_socket = strval;
        }
        mysqlconf.spname = keyname;
        myconfig.push_back(mysqlconf);
    }
    return myconfig;
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
    s.resize(size);

    auto nread = fread(&s[0], 1, size, f);
    s.resize(nread);
    fclose(f);
    std::string includename;
    if (rmstag == "default")
    {
        includename = "\"" + modelname + ".h\"";
    }
    else
    {
        includename = "\"" + rmstag + "/include/" + modelname + ".h\"";
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
    std::string ormfilepath="orm/";
    std::string ormnowpath="orm/";
    std::string rootcontrolpath = "models/";
    std::string controlpath = rootcontrolpath;
    std::string controlrunpath;
    fs::path vpath = controlpath;
    fs::path ormpath = ormfilepath;
    if (!fs::exists(vpath))
    {
        std::cout << " ⛑ \033[1m\033[35m error\033[0m controller directory not in current path " << std::endl;
        return 0;
    }

    if (!fs::exists(ormpath))
    {
        fs::create_directories(ormpath);
        fs::permissions(ormpath, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    ormpath = ormnowpath+"include";
    if (!fs::exists(ormpath))
    {
        fs::create_directories(ormpath);
        fs::permissions(ormpath, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    fs::path paths = controlpath;
    if (controlpath.back() != '/')
    {
        controlpath.push_back('/');
    }
    controlrunpath = controlpath + "include/";
    paths = controlrunpath;
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    std::string command;
    std::string rmstag = "default";
    std::string pretable;
    std::vector<mysqlconnect_t> myconfig = getmysqlconfig("conf/mysqlorm.conf");

    //std::string mysqlconnect = ""; //"mysqlx://root:123456@127.0.0.1:33060/aaa";
    if (myconfig.size() == 0)
    {
        std::cout << " Sorry, not found [config/mysqlorm.conf] config file \033[1m\033[31m This file format example: \033[0m " << std::endl;
        command = R"(
[default]
type=main
host=127.0.0.1
port=3306
dbname=aaa
user=root
password=123456
pretable=
maxpool=5

type=second
host=127.0.0.1
port=3306
dbname=aaa
user=root
password=123456
pretable=
maxpool=20
        )";
        std::cout << command << std::endl;
        command.clear();
        return 0;
    }

    std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(new MYSQL, &mysql_close);
    //MYSQL conn;
    mysql_init(conn.get()); 

    

    if (myconfig.size() > 0)
    {
        if (myconfig.size() < 3)
        {
            std::string port = myconfig[0].port;
            unsigned int myport = atol(port.c_str());
 
            rmstag = myconfig[0].spname;
            command = myconfig[0].dbname;
            pretable = myconfig[0].pretable;

            if(!mysql_real_connect(conn.get(), myconfig[0].host.c_str(),myconfig[0].user.c_str(),myconfig[0].password.c_str(),myconfig[0].dbname.c_str(),myport,(myconfig[0].unix_socket.size()>0?myconfig[0].unix_socket.c_str():NULL),0))
            {
                std::cout<<"mysql db link error!"<<std::endl;
                std::cout<<mysql_error(conn.get())<<std::endl;
            } 

        }
        else
        {
            for (int i = 0; i < myconfig.size(); i++)
            {
                std::cout << " \033[1m\033[31m" << (i + 1) << "\033[0m " << myconfig[i].dbname << std::endl;
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
                for (int i = 0; i < command.size(); i++)
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

                indexsdb = command[0] - '0';

                if (indexsdb > myconfig.size())
                {
                    continue;
                }

                command.clear();
                break;
            }

            //   std::cin>>indexsdb;

            if (indexsdb > 0)
            {
                indexsdb -= 1;
            }
            // if(indexsdb>myconfig.size()){
            //     indexsdb=0;
            // }
            std::string port = myconfig[indexsdb].port;
            unsigned int myport = atol(port.c_str());

            rmstag = myconfig[indexsdb].spname;
            command = myconfig[indexsdb].dbname;
            pretable = myconfig[indexsdb].pretable;

            if(!mysql_real_connect(conn.get(), myconfig[indexsdb].host.c_str(),myconfig[indexsdb].user.c_str(),myconfig[indexsdb].password.c_str(),myconfig[indexsdb].dbname.c_str(),myport,(myconfig[indexsdb].unix_socket.size()>0?myconfig[indexsdb].unix_socket.c_str():NULL),0))
            {
                std::cout<<"mysql db link error 2!"<<std::endl;
                std::cout<<mysql_error(conn.get())<<std::endl;
            } 

        }
    }
    else
    {

        std::string host;
        std::string port;
        std::string dbname;
        std::string user;
        std::string password;
        std::string pretable;
        std::string maxpool;
        std::string spname;

        std::cout << "input db host:";
        std::cin >> host;
        std::cout << "input db port:";
        std::cin >> port;

        std::cout << "input db name:";
        std::cin >> dbname;

        std::cout << "input db user name:";
        std::cin >> user;

        std::cout << "input db user password:";
        std::cin >> password;

       
        if (port.size() < 3)
        {
            port = "3306";
        }
         unsigned int myport = atol(port.c_str());
        //mysqlconnect = "mysqlx://" + user + ":" + password + "@" + host + ":" + port + "/" + dbname;

        command = dbname;
        rmstag = "default";
        pretable = "";

        if(!mysql_real_connect(conn.get(), host.c_str(),user.c_str(),password.c_str(),dbname.c_str(),myport,NULL,0))
            {
                std::cout<<"mysql db link error!"<<std::endl;
                std::cout<<mysql_error(conn.get())<<std::endl;
            } 

    }

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
            fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }
        controlrunpath = controlpath + "include";
        paths = controlrunpath;
        if (!fs::exists(paths))
        {
            fs::create_directories(paths);
            fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }

        ormnowpath = ormfilepath;
        ormnowpath.append(rmstag);
        ormnowpath.push_back('/');
        ormpath = ormnowpath;

     

         if (!fs::exists(ormpath))
        {
            fs::create_directories(ormpath);
            fs::permissions(ormpath, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }
        ormnowpath = ormnowpath + "include";
        ormpath = ormnowpath;

        ormnowpath = ormfilepath;
        ormnowpath.append(rmstag);
        ormnowpath.push_back('/');

        if (!fs::exists(ormpath))
        {
            fs::create_directories(ormpath);
            fs::permissions(ormpath, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }

    }

    std::string tablename;
    std::string sqlqueryring;
    // sqlqueryring.append(realtablename);
    std::cout << "create \033[1m\033[31m" << command << "\033[0m table to models 🚀 " << std::endl;

    std::vector<std::string> tablelist;
    std::vector<std::string> tableshow;
    int maxchar = 0;
    int loopnumcount = 0;
    while (true)
    {

        if (command == "x")
        {
            break;
        }

        sqlqueryring = "show tables; ";
        std::cout << "show tables:" << std::endl;
        command.clear();
        maxchar = 1;

        int readnum = mysql_query(conn.get(),&sqlqueryring[0]);
        
        if(readnum!=0)
        {
            std::cout<<mysql_error(conn.get())<<std::endl;
        }
        MYSQL_RES *result=nullptr;
        result = mysql_store_result(conn.get());


        tablelist.clear();
        tableshow.clear();

        if(result)
        {
          int num_fields = mysql_num_fields(result);
           MYSQL_ROW row;

            while ((row = mysql_fetch_row(result)))
            {
                if(num_fields>0)
                {
                    command = std::string(row[0]);
                    std::transform(command.begin(), command.end(), command.begin(), ::tolower);
                    tablelist.push_back(command);
                }
                
            }  
          mysql_free_result(result);     
        } 

        int percolnum = 30;
        if (tablelist.size() > 60)
        {
            percolnum = ceil((float)tablelist.size() / 6 / 10) * 10;
            // percolnum=percolnum*10;
        }

        int groupp = ceil((float)tablelist.size() / percolnum);
        int pc = 0;
        maxchar += 4;
        int n = 0,  toffset = 0;

        tableshow.resize(percolnum);
        for (int i = 0; i < groupp; i++)
        {
            pc = i * percolnum;
            maxchar = 15;
            for (int j = 0; j < percolnum; j++)
            {
                toffset = pc + j;
                if (toffset < tablelist.size() && maxchar < tablelist[toffset].size())
                {
                    maxchar = tablelist[toffset].size();
                }
            }
            maxchar += 2;
            for (int j = 0; j < percolnum; j++)
            {
                n = 0;
                toffset = pc + j;
                // offset=tableshow[i].size();
                if (toffset < tablelist.size() && tablelist[toffset].size() > 0)
                {

                    tableshow[j].append("\033[1m\033[36m");
                    if (toffset < 9)
                    {
                        tableshow[j].push_back('0');
                        //  tableshow[j].push_back(0x20);
                    }
                    else if (toffset < 100)
                    {
                        // tableshow[j].push_back(0x20);
                    }

                    tableshow[j].append(std::to_string(toffset + 1));
                    tableshow[j].append("\033[0m ");
                    // tableshow[j].push_back('.');

                    if (toffset < tablelist.size())
                    {

                        for (; n < tablelist[toffset].size(); n++)
                        {
                            tableshow[j].push_back(tablelist[toffset][n]);
                        }
                    }
                }
                if (toffset < tablelist.size())
                {
                    for (; n < maxchar; n++)
                    {
                        tableshow[j].push_back(0x20);
                    }
                }
            }
        }
        for (int i = 0; i < tableshow.size(); i++)
        {
            if (tableshow[i].size() > 0)
            {
                std::cout << tableshow[i] << std::endl;
            }
        }
        std::cout << "Select number to update orm basefile,a to update all, x to quit.\n";
        bool isloop = false;
        while (true)
        {
            isloop = false;
            std::cout << "input table index number to update, e(x)it (r)eload dbconfig (a)ll :";
            command.clear();
            std::cin >> command;
            if (command == "x")
            {
                break;
            }
            if (command == "r")
            {

                break;
            }
            if (command == "a")
            {
                isloop = true;
            }
            for (int tn = 0; tn < tablelist.size(); tn++)
            {

                int indexnum = 0;
                if (isloop)
                {
                    indexnum = tn;
                }
                else
                {
                    indexnum = 0;
                    if (command[0] >= '0' && command[0] <= '9')
                    {

                        for (int i = 0; i < command.size(); i++)
                        {
                            if (command[i] == 0x20)
                            {
                                break;
                            }
                            if (command[i] >= '0' && command[i] <= '9')
                            {
                                indexnum = indexnum * 10 + (command[i] - '0');
                            }
                            if (i > 4)
                            {
                                break;
                            }
                        }
                        indexnum -= 1;
                    }
                    else
                    {
                        indexnum = tablelist.size() + 20;
                    }
                }
                command.clear();

                if (indexnum < tablelist.size())
                {
                    std::string realtablename, tablename;
                    // std::string fixtablepre="web_";
                    int offsetmodel, iszimu = 0;
                    realtablename = tablelist[indexnum];
                    offsetmodel = 0;
                    for (; offsetmodel < pretable.size(); offsetmodel++)
                    {
                        if (realtablename[offsetmodel] == pretable[offsetmodel])
                        {
                            continue;
                        }
                        break;
                    }

                    if (offsetmodel == pretable.size())
                    {

                        // 处理表前缀 分离模型名称和表名
                        tablename.reserve(realtablename.size());
                        if (realtablename[offsetmodel] > 96 && realtablename[offsetmodel] < 123)
                        {
                            tablename.push_back(realtablename[offsetmodel] - 32);
                        }
                        else
                        {
                            tablename.push_back(realtablename[offsetmodel]);
                            if (realtablename[offsetmodel] >= '0' && realtablename[offsetmodel] <= '9')
                            {
                                iszimu++;
                            }
                            else if (realtablename[offsetmodel] >= 'A' && realtablename[offsetmodel] <= 'Z')
                            {
                                iszimu++;
                            }
                            else if (realtablename[offsetmodel] >= 'a' && realtablename[offsetmodel] <= 'z')
                            {
                                iszimu++;
                            }
                        }
                        offsetmodel += 1;
                        for (int i = offsetmodel; i < realtablename.size(); i++)
                        {

                            if (realtablename[i] == '_')
                            {

                                for (; i < realtablename.size(); i++)
                                {
                                    if (realtablename[i] == '_')
                                    {
                                        continue;
                                    }
                                    break;
                                }
                                if (i < realtablename.size())
                                {
                                    if (realtablename[i] > 96 && realtablename[i] < 123)
                                    {
                                        tablename.push_back(realtablename[i] - 32);
                                    }
                                    else
                                    {
                                        tablename.push_back(realtablename[i]);
                                    }
                                    if (realtablename[i] >= '0' && realtablename[i] <= '9')
                                    {
                                        iszimu++;
                                    }
                                    else if (realtablename[i] >= 'A' && realtablename[i] <= 'Z')
                                    {
                                        iszimu++;
                                    }
                                    else if (realtablename[i] >= 'a' && realtablename[i] <= 'z')
                                    {
                                        iszimu++;
                                    }
                                    i++;
                                }
                            }

                            tablename.push_back(realtablename[i]);
                            if (realtablename[i] >= '0' && realtablename[i] <= '9')
                            {
                                iszimu++;
                            }
                            else if (realtablename[i] >= 'A' && realtablename[i] <= 'Z')
                            {
                                iszimu++;
                            }
                            else if (realtablename[i] >= 'a' && realtablename[i] <= 'z')
                            {
                                iszimu++;
                            }
                        }
                        if (iszimu > 1)
                        {
                            createtabletoorm(ormnowpath,controlpath, realtablename, tablename, rmstag, conn.get());
                            addhfiletoormfile(ormfilepath, tablename, rmstag);
                        }
                        loopnumcount = 0;
                    }
                    else
                    {
                        std::cout << " \033[1m\033[31m error! only create \033[0m\033[1m\033[32m " << pretable << " \033[0m prefix tables.\n";
                    }
                }

                if (isloop == false)
                {
                    break;
                }
            }

            loopnumcount++;
            if (loopnumcount > 10)
            {
                command.clear();
                break;
            }
        }

        if (command[0] == 'x')
        {
            break;
        }
        for (int i = 0; i < myconfig.size(); i++)
        {
            std::cout << " \033[1m\033[31m" << (i + 1) << "\033[0m " << myconfig[i].dbname << std::endl;
        }
        std::cout << "Select db index num:";
        command.clear();
        std::cin >> command;
        if (command[0] > '9' || command[0] < '0')
        {
            break;
        }
        unsigned char indexsdb = 0;

        for (int i = 0; i < command.size(); i++)
        {
            if (command[i] == 0x20)
            {
                break;
            }
            if (command[i] >= '0' && command[i] <= '9')
            {
                indexsdb = indexsdb * 10 + (command[i] - '0');
            }
            if (i > 2)
            {
                break;
            }
        }

        if (indexsdb > myconfig.size())
        {
            break;
        }
        if (indexsdb > 0)
        {
            indexsdb -= 1;
        }
        command.clear();
        std::string port = myconfig[indexsdb].port;
        unsigned int myport = atol(port.c_str());
        
        rmstag = myconfig[indexsdb].spname;
        // command=myconfig[indexsdb].dbname;
        pretable = myconfig[indexsdb].pretable;

        conn.reset(new MYSQL);
        mysql_init(conn.get()); 

        if(!mysql_real_connect(conn.get(), myconfig[indexsdb].host.c_str(),myconfig[indexsdb].user.c_str(),myconfig[indexsdb].password.c_str(),myconfig[indexsdb].dbname.c_str(),myport,(myconfig[indexsdb].unix_socket.size()>0?myconfig[indexsdb].unix_socket.c_str():NULL),0))
        {
            std::cout<<"mysql db link error 4!"<<std::endl;
            std::cout<<mysql_error(conn.get())<<std::endl;
            break;
        } 

        if (rmstag != "default")
        {
            controlpath=rootcontrolpath;
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
                fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
            controlrunpath = controlpath + "include";
            paths = controlrunpath;
            if (!fs::exists(paths))
            {
                fs::create_directories(paths);
                fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }

    
            ormnowpath = ormfilepath;
            ormnowpath.append(rmstag);
            ormnowpath.push_back('/');
            ormpath=ormnowpath;
            if (!fs::exists(ormpath))
            {
                fs::create_directories(ormpath);
                fs::permissions(ormpath, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
            ormnowpath = ormnowpath + "include";
            ormpath = ormnowpath;
            ormnowpath = ormfilepath;
            ormnowpath.append(rmstag);
            ormnowpath.push_back('/');

            if (!fs::exists(ormpath))
            {
                fs::create_directories(ormpath);
                fs::permissions(ormpath, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }

        }
    }
    
    return 0;
}