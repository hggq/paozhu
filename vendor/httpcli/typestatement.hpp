#pragma once
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

struct obj_reflect_type
{
    std::string name;
    std::string type;
    std::string value;
    std::vector<std::pair<std::string, std::string>> mapvalue;
    std::vector<std::string> listvalue;
    std::string inputalias;
    std::string outputalias;
    std::string varname;
    std::string namespacename;
    std::vector<obj_reflect_type> children;
    unsigned char typevalue;// 0 null 1 struct 2 class 3 func  10 float 11 double 12 char 13 unsigned char 14 long long
                            // 15 unsigned long long  16 int 17 unsigned  20 string 21 vector 22 map 23 array 30 other
    bool ispublic = true;   // 0 public 1 private
    // void display(int level = 0) { level = 1; }
};

class typejsonfiles
{
  public:
    std::vector<std::string> readallfile(const std::string &methodpath)
    {
        fs::path tagetpath = methodpath;
        std::vector<std::string> temp;
        if (fs::exists(tagetpath) && fs::is_directory(tagetpath))
        {

            for (const auto &entry : fs::directory_iterator(tagetpath))
            {
                auto filename = entry.path().filename().string();
                if (fs::is_regular_file(entry.status()))
                {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".h")
                    {
                        temp.emplace_back(filename.substr(0, filename.size() - 2));
                    }
                }
                else if (fs::is_directory(entry.status()))
                {
                    std::string ext;
                    if (methodpath.back() == '/')
                    {

                        ext.append(methodpath);
                        ext.append(filename);
                    }
                    else
                    {
                        ext.append(methodpath);
                        ext.append("/");
                        ext.append(filename);
                    }
                    std::vector<std::string> directory_temp = readallfile(ext);
                    if (directory_temp.size() > 0)
                    {
                        for (unsigned int j = 0; j < directory_temp.size(); j++)
                        {
                            ext.clear();
                            ext.append(filename);
                            ext.append("/");
                            ext.append(directory_temp[j]);
                            temp.emplace_back(ext);
                        }
                    }
                }
            }
        }
        return temp;
    }
    std::vector<struct obj_reflect_type> pickreflectfile(const std::string &methodpathfile)
    {
        std::vector<struct obj_reflect_type> temp;
        std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(methodpathfile.c_str(), "rb"), std::fclose);

        if (!fp.get())
        {
            return temp;
        }

        std::string filecontent;
        std::string namespacename;

        fseek(fp.get(), 0, SEEK_END);
        unsigned long long file_size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);

        filecontent.resize(file_size);
        file_size = fread(&filecontent[0], 1, file_size, fp.get());
        filecontent.resize(file_size);

        for (unsigned int i = 0; i < filecontent.size(); i++)
        {
            if (filecontent[i] == '/')
            {
                if ((i + 1) < filecontent.size() && filecontent[i + 1] == '*')
                {
                    i += 2;
                    for (; i < filecontent.size(); i++)
                    {
                        if (filecontent[i] == '*')
                        {
                            if ((i + 1) < filecontent.size() && filecontent[i + 1] == '/')
                            {
                                i += 2;
                                break;
                            }
                        }
                    }
                }
                else if ((i + 1) < filecontent.size() && filecontent[i + 1] == '/')
                {
                    for (; i < filecontent.size(); i++)
                    {
                        if (filecontent[i] == 0x0A)
                        {
                            i++;

                            if (i < filecontent.size() && filecontent[i] == 0x0D)
                            {
                                i++;
                            }

                            break;
                        }
                    }
                }

                continue;
            }

            // pick namespace
            if (filecontent[i] == 'n' && filecontent[i + 1] == 'a' && filecontent[i + 2] == 'm' &&
                filecontent[i + 3] == 'e' && filecontent[i + 4] == 's' && filecontent[i + 5] == 'p' &&
                filecontent[i + 6] == 'a' && filecontent[i + 7] == 'c' && filecontent[i + 8] == 'e' &&
                filecontent[i + 9] == ' ')
            {
                unsigned j = i + 10;
                for (; j < filecontent.size(); j++)
                {
                    if (filecontent[j] == ' ' || filecontent[j] == '\t')
                    {
                        continue;
                    }
                    break;
                }
                for (; j < filecontent.size(); j++)
                {
                    if (filecontent[j] == ' ' || filecontent[j] == '\t' || filecontent[j] == '{' ||
                        filecontent[j] == 0x0A || filecontent[j] == 0x0D)
                    {
                        break;
                    }
                    namespacename.push_back(filecontent[j]);
                }
                break;
            }
        }

        bool isbegin = true;
        for (unsigned int i = 0; i < filecontent.size(); i++)
        {
            if (isbegin == false)
            {
                if (filecontent[i] == 0x0A)
                {
                    isbegin = true;
                }
                continue;
            }

            // if (filecontent[i] > 0x2F)
            // {
            //     isbegin = false;
            //     continue;
            // }
            ////@reflect
            if (filecontent[i] == '/' && filecontent[i + 1] == '/' && filecontent[i + 2] == '@' &&
                filecontent[i + 3] == 'r' && filecontent[i + 4] == 'e' && filecontent[i + 5] == 'f' &&
                filecontent[i + 6] == 'l')
            {
                if (filecontent[i + 7] == 'e' && filecontent[i + 8] == 'c' && filecontent[i + 9] == 't' &&
                    filecontent[i + 10] == ' ' && filecontent[i + 11] == 'j' && filecontent[i + 12] == 's' &&
                    filecontent[i + 13] == 'o' && filecontent[i + 14] == 'n')
                {
                    std::string prename;
                    std::string tempname;
                    unsigned int j = i + 15;
                    for (; j < filecontent.size(); j++)
                    {
                        if (filecontent[j] == 0x0A)
                        {
                            j++;

                            if (j < filecontent.size() && filecontent[j] == 0x0D)
                            {
                                j++;
                            }

                            break;
                        }
                    }

                    auto [newoffset, a] = processclass(filecontent, j);
                    i                   = newoffset;

                    auto [objendoffset, varname] = processvarname(filecontent, i);
                    if (varname.size() > 0)
                    {
                        i = objendoffset;
                    }

                    a.varname       = varname;
                    a.namespacename = namespacename;

                    temp.push_back(a);
                    continue;
                }
            }
        }

        return temp;
    }

    std::tuple<unsigned int, obj_reflect_type> processclass(const std::string &filecontent, unsigned int offset = 0)
    {
        struct obj_reflect_type temp;
        std::string classname;
        for (; offset < filecontent.size(); offset++)
        {
            // struct
            if (filecontent[offset] == 's' && filecontent[offset + 1] == 't' && filecontent[offset + 2] == 'r' &&
                filecontent[offset + 3] == 'u' && filecontent[offset + 4] == 'c' && filecontent[offset + 5] == 't' &&
                filecontent[offset + 6] == ' ')
            {
                offset = offset + 7;
                for (; offset < filecontent.size(); offset++)
                {
                    // struct
                    bool isalpha = false;
                    if (filecontent[offset] > 0x2F && filecontent[offset] < 0x3A)
                    {
                        isalpha = true;
                    }
                    else if (filecontent[offset] > 0x40 && filecontent[offset] < 0x5B)
                    {
                        isalpha = true;
                    }
                    else if (filecontent[offset] > 0x60 && filecontent[offset] < 0x7B)
                    {
                        isalpha = true;
                    }
                    else if (filecontent[offset] == 0x5F)
                    {
                        isalpha = true;
                    }
                    if (isalpha)
                    {
                        classname.push_back(filecontent[offset]);
                    }
                    else
                    {
                        // think struct name at next line
                        break;
                    }
                }
                if (classname.size() > 0)
                {
                    if (filecontent[offset] != '{')
                    {
                        for (; offset < filecontent.size(); offset++)
                        {
                            if (filecontent[offset] == '{')
                            {
                                offset++;
                                break;
                            }
                        }
                    }
                    else
                    {
                        offset++;
                    }
                    // main content begin

                    temp.typevalue = 1;
                    temp.type      = "struct";
                    break;
                }
                else
                {
                    break;
                }
            }
            // class
            if (filecontent[offset] == 'c' && filecontent[offset + 1] == 'l' && filecontent[offset + 2] == 'a' &&
                filecontent[offset + 3] == 's' && filecontent[offset + 4] == 's' && filecontent[offset + 5] == ' ')
            {
                offset = offset + 6;
                for (; offset < filecontent.size(); offset++)
                {
                    // struct
                    bool isalpha = false;
                    if (filecontent[offset] > 0x2F && filecontent[offset] < 0x3A)
                    {
                        isalpha = true;
                    }
                    else if (filecontent[offset] > 0x40 && filecontent[offset] < 0x5B)
                    {
                        isalpha = true;
                    }
                    else if (filecontent[offset] > 0x60 && filecontent[offset] < 0x7B)
                    {
                        isalpha = true;
                    }
                    else if (filecontent[offset] == 0x5F)
                    {
                        isalpha = true;
                    }
                    if (isalpha)
                    {
                        classname.push_back(filecontent[offset]);
                    }
                    else
                    {
                        // think class name at next line
                        break;
                    }
                }
                if (classname.size() > 0)
                {
                    if (filecontent[offset] != '{')
                    {
                        for (; offset < filecontent.size(); offset++)
                        {
                            if (filecontent[offset] == '{')
                            {
                                offset++;
                                break;
                            }
                        }
                    }
                    else
                    {
                        offset++;
                    }
                    // main content begin

                    temp.typevalue = 2;
                    temp.type      = "class";
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        if (classname.size() > 0)
        {
            temp.name     = classname;
            bool ispublic = true;

            if (temp.typevalue == 2)
            {
                ispublic = false;
            }

            for (; offset < filecontent.size();)
            {
                // offset-=1;
                auto [newoffset, linecontent] = readlinecontent(filecontent, offset);
                // offset = newoffset;
                // process class or struct
                // struct
                bool isobj = false;
                if (linecontent.size() > 5 && linecontent[0] == 's' && linecontent[1] == 't' && linecontent[2] == 'r' &&
                    linecontent[3] == 'u' && linecontent[4] == 'c' && linecontent[5] == 't' && linecontent[6] == ' ')
                {
                    isobj = true;
                }
                // class
                if (linecontent.size() > 4 && linecontent[0] == 'c' && linecontent[1] == 'l' && linecontent[2] == 'a' &&
                    linecontent[3] == 's' && linecontent[4] == 's' && linecontent[5] == ' ')
                {
                    isobj = true;
                }
                // public:
                if (linecontent[0] == 'p' && linecontent[1] == 'u' && linecontent[2] == 'b' && linecontent[3] == 'l' &&
                    linecontent[4] == 'i' && linecontent[5] == 'c' && linecontent[6] == ':')
                {
                    ispublic = true;
                    offset   = newoffset;
                    continue;
                }
                else if (linecontent[0] == 'p' && linecontent[1] == 'r' && linecontent[2] == 'i' &&
                         linecontent[3] == 'v' && linecontent[4] == 'a' && linecontent[5] == 't' &&
                         linecontent[6] == 'e' && linecontent[7] == ':')
                {
                    // private
                    ispublic = false;
                    offset   = newoffset;
                    continue;
                }
                if (isobj)
                {
                    offset              = newoffset - linecontent.size() - 1;
                    auto [offsetnew, a] = processclass(filecontent, offset);
                    offset              = offsetnew;

                    // 内部定义对象名称 如果没有定义应该出错
                    auto [objendoffset, varname] = processvarname(filecontent, offset);
                    if (varname.size() > 0)
                    {
                        offset = objendoffset;
                    }
                    a.varname = varname;

                    a.ispublic = ispublic;
                    temp.children.push_back(a);
                }
                else
                {
                    auto [isfunc, a] = processclassline(linecontent);
                    if (isfunc < 0)
                    {
                        offset = newoffset + isfunc;
                        offset = processfunc(filecontent, offset);
                    }
                    else
                    {
                        a.ispublic = ispublic;
                        if (a.name.size() > 0)
                        {
                            temp.children.push_back(a);
                        }
                        unsigned a = isfunc;
                        if (isfunc < 0)
                        {
                            a = 0;
                        }
                        if (a < linecontent.size())
                        {

                            // offset=newoffset - ( linecontent.size() -   isfunc );
                            // for ( ; offset < filecontent.size(); offset++)
                            // {
                            //     if (filecontent[offset] != ' ' && filecontent[offset] != '\t')
                            //     {
                            //         break;
                            //     }
                            // }
                            // if (filecontent[offset] == '}')
                            // {
                            //     offset+=1;
                            // }
                            // this obj end;
                            break;
                        }
                        else
                        {
                            offset = newoffset;
                        }
                    }
                }
            }
        }

        return std::make_tuple(offset, temp);
    }
    std::tuple<unsigned int, std::string> processvarname(const std::string &filecontent, unsigned int offset = 0)
    {
        std::string varname;
        for (; offset < filecontent.size(); offset++)
        {
            if (filecontent[offset] != ' ' && filecontent[offset] != '\t')
            {
                break;
            }
        }
        if (filecontent[offset] == '}')
        {
            offset += 1;
            for (; offset < filecontent.size(); offset++)
            {
                if (filecontent[offset] != ' ' && filecontent[offset] != '\t')
                {
                    break;
                }
            }
        }
        for (; offset < filecontent.size(); offset++)
        {
            if (filecontent[offset] == ' ' || filecontent[offset] == '\t' || filecontent[offset] == ';')
            {
                offset += 1;
                break;
            }
            bool isalpha = false;
            if (filecontent[offset] > 0x2F && filecontent[offset] < 0x3A)
            {
                isalpha = true;
            }
            else if (filecontent[offset] > 0x40 && filecontent[offset] < 0x5B)
            {
                isalpha = true;
            }
            else if (filecontent[offset] > 0x60 && filecontent[offset] < 0x7B)
            {
                isalpha = true;
            }
            else if (filecontent[offset] == 0x5F)
            {
                isalpha = true;
            }

            if (isalpha)
            {
                varname.push_back(filecontent[offset]);
            }
            else
            {
                break;
            }
        }
        return std::make_tuple(offset, varname);
    }
    unsigned int processfunc(const std::string &filecontent, unsigned int offset = 0)
    {
        unsigned int levelnum = 0;
        bool isdefinefunc     = true;
        // unsigned int beginpos = offset;
        for (; offset < filecontent.size(); offset++)
        {
            if (isdefinefunc)
            {
                if (filecontent[offset] == ';')
                {
                    offset++;
                    break;
                }
            }
            if (filecontent[offset] == '/')
            {
                if ((offset + 1) < filecontent.size() && filecontent[offset + 1] == '*')
                {
                    offset += 2;
                    for (; offset < filecontent.size(); offset++)
                    {
                        if (filecontent[offset] == '*')
                        {
                            if ((offset + 1) < filecontent.size() && filecontent[offset + 1] == '/')
                            {
                                offset += 1;
                                break;
                            }
                        }
                    }
                }
                else if ((offset + 1) < filecontent.size() && filecontent[offset + 1] == '/')
                {
                    for (; offset < filecontent.size(); offset++)
                    {
                        if (filecontent[offset] == 0x0A)
                        {

                            if (offset < filecontent.size() && filecontent[offset] == 0x0D)
                            {
                                offset++;
                            }

                            break;
                        }
                    }
                }
                continue;
            }
            if (filecontent[offset] == '"')
            {
                offset++;
                for (; offset < filecontent.size(); offset++)
                {
                    if (filecontent[offset] == '"' && filecontent[offset - 1] != 0x5C)
                    {
                        // offset++;
                        break;
                    }
                }
                continue;
            }

            if (filecontent[offset] == '{')
            {
                isdefinefunc = false;
                if (filecontent[offset - 1] == 0x27 && filecontent[offset + 1] == 0x27)
                {
                }
                else
                {

                    levelnum += 1;
                }
            }
            if (filecontent[offset] == '}')
            {
                if (filecontent[offset - 1] == 0x27 && filecontent[offset + 1] == 0x27)
                {
                }
                else
                {
                    levelnum -= 1;
                }

                if (levelnum == 0)
                {
                    // end func
                    offset += 1;
                    break;
                }
            }
        }
        return offset;
    }
    std::tuple<int, obj_reflect_type> processclassline(const std::string &linecontent)
    {
        struct obj_reflect_type temp;
        unsigned int offset      = 0;
        unsigned int startoffset = 0;
        // unsigned int saveoffset  = 0;
        std::string fulltypename;
        std::string vartypename;
        for (; offset < linecontent.size(); offset++)
        {
            if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
            {
                continue;
            }
            break;
        }
        startoffset = offset;
        for (; offset < linecontent.size(); offset++)
        {
            if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
            {
                break;
            }
            vartypename.push_back(linecontent[offset]);
        }
        for (; offset < linecontent.size(); offset++)
        {
            if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
            {
                continue;
            }
            break;
        }
        if (vartypename.size() > 0)
        {
            if (vartypename[0] == '}')
            {
                // obj end;
                return std::make_tuple(startoffset, temp);
            }
        }
        if (vartypename == "const")
        {
            // fullvartypename="const ";
            vartypename.clear();
            for (; offset < linecontent.size(); offset++)
            {
                if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                {
                    break;
                }
                vartypename.push_back(linecontent[offset]);
            }
            for (; offset < linecontent.size(); offset++)
            {
                if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                {
                    continue;
                }
                break;
            }
            fulltypename.append(vartypename);
            if (vartypename == "unsigned")
            {
                vartypename.clear();
                for (; offset < linecontent.size(); offset++)
                {
                    if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                    {
                        break;
                    }
                    vartypename.push_back(linecontent[offset]);
                }
                for (; offset < linecontent.size(); offset++)
                {
                    if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                    {
                        continue;
                    }
                    break;
                }
                fulltypename.append(" ");
                fulltypename.append(vartypename);
                if (vartypename == "long")
                {
                    vartypename.clear();
                    for (; offset < linecontent.size(); offset++)
                    {
                        if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                        {
                            break;
                        }
                        vartypename.push_back(linecontent[offset]);
                    }
                    for (; offset < linecontent.size(); offset++)
                    {
                        if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                        {
                            continue;
                        }
                        break;
                    }
                    fulltypename.append(" ");
                    fulltypename.append(vartypename);
                }
            }
        }
        else
        {
            fulltypename.append(vartypename);
            if (vartypename == "unsigned")
            {
                vartypename.clear();
                for (; offset < linecontent.size(); offset++)
                {
                    if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                    {
                        break;
                    }
                    vartypename.push_back(linecontent[offset]);
                }
                for (; offset < linecontent.size(); offset++)
                {
                    if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                    {
                        continue;
                    }
                    break;
                }
                fulltypename.append(" ");
                fulltypename.append(vartypename);
                if (vartypename == "long")
                {
                    vartypename.clear();
                    for (; offset < linecontent.size(); offset++)
                    {
                        if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                        {
                            break;
                        }
                        vartypename.push_back(linecontent[offset]);
                    }
                    for (; offset < linecontent.size(); offset++)
                    {
                        if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                        {
                            continue;
                        }
                        break;
                    }
                    fulltypename.append(" ");
                    fulltypename.append(vartypename);
                }
            }
        }
        temp.type = fulltypename;
        vartypename.clear();
        bool isfunc = false;
        // startoffset = offset;
        for (; offset < linecontent.size(); offset++)
        {
            if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
            {
                break;
            }
            if (linecontent[offset] == '(')
            {
                isfunc = true;
                break;
            }
            if (linecontent[offset] == '=')
            {
                break;
            }
            if (linecontent[offset] == ';')
            {
                break;
            }
            vartypename.push_back(linecontent[offset]);
        }
        temp.name = vartypename;
        if (isfunc)
        {
            return std::make_tuple(startoffset - linecontent.size(), temp);
        }

        for (; offset < linecontent.size(); offset++)
        {
            if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
            {
                continue;
            }
            break;
        }

        std::string defaultvalue;
        if (linecontent[offset] == '=')
        {
            offset += 1;
            for (; offset < linecontent.size(); offset++)
            {
                if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                {
                    continue;
                }
                break;
            }
            for (; offset < linecontent.size(); offset++)
            {
                if (linecontent[offset] == ' ' || linecontent[offset] == '\t')
                {
                    break;
                }
                if (linecontent[offset] == ';')
                {
                    break;
                }
                defaultvalue.push_back(linecontent[offset]);
            }
        }
        temp.value = defaultvalue;
        if (linecontent[offset] == ';')
        {
            offset += 1;
        }
        isfunc = false;
        for (; offset < linecontent.size(); offset++)
        {
            if (linecontent[offset] == '/')
            {
                break;
            }
            if (linecontent[offset] == '}')
            {
                isfunc = true;
                break;
            }
        }
        if (isfunc)
        {
            startoffset = offset;
            return std::make_tuple(startoffset, temp);
        }
        return std::make_tuple(linecontent.size(), temp);
    }
    std::tuple<unsigned int, std::string> readlinecontent(const std::string &filecontent, unsigned int offset = 0)
    {
        std::string linecontent;
        for (; offset < filecontent.size(); offset++)
        {
            if (filecontent[offset] == '/')
            {
                if ((offset + 1) < filecontent.size() && filecontent[offset + 1] == '*')
                {
                    offset += 2;
                    for (; offset < filecontent.size(); offset++)
                    {
                        if (filecontent[offset] == '*')
                        {
                            if ((offset + 1) < filecontent.size() && filecontent[offset + 1] == '/')
                            {
                                offset += 1;
                                break;
                            }
                        }
                    }
                }
                else if ((offset + 1) < filecontent.size() && filecontent[offset + 1] == '/')
                {
                    for (; offset < filecontent.size(); offset++)
                    {
                        if (filecontent[offset] == 0x0A)
                        {

                            if (offset < filecontent.size() && filecontent[offset] == 0x0D)
                            {
                                offset++;
                            }

                            break;
                        }
                    }
                }

                continue;
            }

            bool isalpha = true;
            // if (filecontent[offset] > 0x2F && filecontent[offset] < 0x3A )
            // {
            //     isalpha=true;
            // }else if (filecontent[offset] > 0x40 && filecontent[offset] < 0x5B )
            // {
            //     isalpha=true;
            // }else if (filecontent[offset] > 0x60 && filecontent[offset] < 0x7B )
            // {
            //     isalpha=true;
            // }else if (filecontent[offset] == 0x5F )
            // {
            //     isalpha=true;
            // }
            if (filecontent[offset] == ' ' || filecontent[offset] == '\t')
            {
                isalpha = false;
            }
            if (isalpha)
            {
                for (; offset < filecontent.size(); offset++)
                {
                    if (filecontent[offset] == '/')
                    {
                        if ((offset + 1) < filecontent.size() && filecontent[offset + 1] == '/')
                        {
                            offset += 2;
                            for (; offset < filecontent.size(); offset++)
                            {
                                if (filecontent[offset] == 0x0A)
                                {
                                    offset++;

                                    if (offset < filecontent.size() && filecontent[offset] == 0x0D)
                                    {
                                        offset++;
                                    }

                                    break;
                                }
                            }
                            break;
                        }
                    }
                    else if (filecontent[offset] == 0x0A)
                    {
                        offset++;

                        if (offset < filecontent.size() && filecontent[offset] == 0x0D)
                        {
                            offset++;
                        }

                        break;
                    }
                    linecontent.push_back(filecontent[offset]);
                }
                break;
            }
        }

        return std::make_tuple(offset, linecontent);
    }
    void createhfile(const std::string &filename,
                     const std::vector<struct obj_reflect_type> &methodpathfile,
                     const std::string &headerfilename)
    {
        std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(filename.c_str(), "wb"), std::fclose);

        if (!fp.get())
        {
            return;
        }

        std::string namespacename;
        if (methodpathfile.size() > 0)
        {
            namespacename = methodpathfile[0].namespacename;
        }

        std::string header_content = R"(
#include <sstream>
#include <string>
#include <vector>
#include <map>
)";
        header_content += headerfilename;
        header_content += R"(
#include "json_reflect_headers.h"
#include "unicode.h"

)";

        if (namespacename.size() > 0)
        {
            header_content.append("\nnamespace ");
            header_content.append(namespacename);
            header_content.append("\n{\n");
        }
        fwrite(&header_content[0], 1, header_content.size(), fp.get());

        for (unsigned int i = 0; i < methodpathfile.size(); i++)
        {
            header_content.clear();
            header_content = createjsonobj(methodpathfile[i]);
            fwrite(&header_content[0], 1, header_content.size(), fp.get());
        }

        header_content.clear();
        if (namespacename.size() > 0)
        {

            header_content.append("\n}\n");
        }

        fwrite(&header_content[0], 1, header_content.size(), fp.get());
    }
    std::string createjsonobj(const struct obj_reflect_type &json_reflect_data)
    {
        std::string temp;
        std::string objtypename = json_reflect_data.name;
        // to_json
        temp.append(jsonobjfuncto_json(json_reflect_data, objtypename));
        temp.append(jsonobjfuncfrom_json(json_reflect_data, objtypename));
        return temp;
    }
    std::string jsonobjfuncto_json(const struct obj_reflect_type &json_reflect_data, const std::string &jsonobjtypename)
    {
        std::string temp;
        // to_json
        temp = "\n\nstd::string json_encode(const " + jsonobjtypename + " &json_reflectobj)\n\t{\n";
        temp += single_obj_to_json(json_reflect_data, jsonobjtypename, "json_reflectobj", true, 1);
        temp += "\n\t}\n";

        temp += "\n\nstd::string json_encode(const std::vector<" + jsonobjtypename + "> &json_reflectobj)";
        temp += R"(
        {
        std::stringstream _stream;
        _stream << "["; 
    
            for(unsigned int i=0;i<json_reflectobj.size();i++) 
            {
                if(i>0)
                {
                    _stream <<",";
                }
                _stream <<json_encode(json_reflectobj[i]);
            }
    
        _stream << "]";

        return _stream.str();
        }
            )";
        return temp;
    }
    std::string jsonobjfuncfrom_json(const struct obj_reflect_type &json_reflect_data,
                                     const std::string &jsonobjtypename)
    {
        std::string temp;
        // std::string objname=jsonobjtypename+".";
        // to_json
        temp = single_obj_fromjson(json_reflect_data, jsonobjtypename, 0);
        temp += array_obj_fromjson(jsonobjtypename);
        return temp;
    }
    std::string single_obj_fromjson(const struct obj_reflect_type &json_reflect_data,
                                    const std::string &jsonobjtypename,
                                    [[maybe_unused]] unsigned int levelnum = 0)
    {
        std::string tempcontent;
        std::string tempobjname = "json_reflectobj.";
        unsigned int ishas      = 0;
        std::string obj_name_array_tempname;

        tempcontent = R"(  
     unsigned int json_decode()";
        tempcontent += jsonobjtypename;
        tempcontent += R"( &json_reflectobj,const std::string &_json_data,unsigned int _offset)
     {           
             bool _isarray=false;
        for(;_offset<_json_data.size();_offset++)
        {
            if(_json_data[_offset]=='{')
            {
                break;
            }
            if(_json_data[_offset]=='[')
            {
                _isarray=true;
                break;
            }
        }

        if(_isarray)
        {
            for(;_offset<_json_data.size();_offset++)
            {
                if(_json_data[_offset]=='{')
                {
                    _isarray=false;
                    break;
                }   
            }
        }

        if(_isarray==false)
        {
            if(_json_data[_offset]=='{')
            {
                _offset++;
            }
            std::string _json_key_name,_json_value_name;
            for(;_offset<_json_data.size();_offset++)
            {

                //去除空格
                _offset=http::json_string_trim(_json_data,_offset);
                //如果是右侧括号表示这个对象已经结束
                if(_json_data[_offset]==0x7D)
                {
                    return _offset;
                }
                //直到引号
                if(_json_data[_offset]==0x22)
                {
                    unsigned int temp_offset=_offset;
                    _json_value_name.clear();
                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);

                    _offset=temp_offset;
                    if(_offset < _json_data.size() &&_json_data[_offset]==0x22)
                    {
                        _offset+=1;
                    }
                    //键名 后就是键值类型 循环去除空格
                    _offset=http::json_string_trim(_json_data,_offset);
                    if(_offset < _json_data.size() &&_json_data[_offset]!=':')
                    {
                        return _offset; 
                    }
                    _offset++;
                    _offset=http::json_string_trim(_json_data,_offset);
                    
                    if(_offset < _json_data.size() &&_json_data[_offset]=='{')
                    {   //还是一个对象，表示有嵌套对象
                        //1 内置 struct map<std::string,*>
                        //递归代码
                    )";
        ishas = 0;
        for (unsigned int i = 0; i < json_reflect_data.children.size(); i++)
        {
            if (json_reflect_data.children[i].type == "struct")
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].varname;
                tempcontent += R"(") == 0)
                            {)";
                std::string temp_obj_name_obj = tempobjname + json_reflect_data.children[i].varname;

                tempcontent += object_item_fromjson(json_reflect_data.children[i], temp_obj_name_obj, 1);

                tempcontent += R"(     
                            }
                            )";
            }
        }

        // 当没有item时候
        if (ishas == 0)
        {
            tempcontent += R"(
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == '}')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        )";
        }

        tempcontent += R"(                        

                        if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }
                        //直接下一个，不用处理键值
                        continue;   
                    }
                    else if(_json_data[_offset]=='[')
                    {   //表示有数组 
                        //////////////////////////////////////////////////////////////////////
                        //begin level1 []
                        //vector<std::string> vector<std::pair<std::string,*>>   vector<vector<int|long|float|double>>
                        //如果是非内置类型 直接使用json_decode<>

                        //递归代码     

                    )";

        ishas = 0;
        for (unsigned int i = 0; i < json_reflect_data.children.size(); i++)
        {

            if (json_reflect_data.children[i].type.size() > 12 &&
                json_reflect_data.children[i].type.starts_with("std::vector<std::string>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 30);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<unsigned long long>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 21);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<long long>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 22);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<double>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 10);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<float>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 11);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<unsigned int>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 23);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<int>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 24);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type == "std::vector<std::vector<std::string>>")
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 60);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<std::vector<double>>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 40);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<std::vector<float>>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 41);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<std::vector<unsigned int>>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 53);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<std::vector<int>>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 54);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::map<std::string,int>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 6);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::map<std::string,std::string>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 7);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::map<int,std::string>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 8);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::map<std::string,float>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 9);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                // tempcontent+=array_item_fromjson(json_reflect_data.children[i],jsonobjtypename,1);

                tempcontent += R"(
                                _offset=json_decode()";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(,_json_data,_offset);
                            )";

                tempcontent += R"(     
                            }
                            )";
            }
        }

        // 当没有item时候
        if (ishas == 0)
        {
            tempcontent += R"(
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == ']')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        )";
        }
        tempcontent += R"(
                        //直接下一个，不用处理键值
                        if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }
                        continue;    
                        //end level1[]
                        ////////////////////////////////////////////////////////////////////
                    }
                    else if(_json_data[_offset]==0x22)
                    {
                        //如果键值也是字符串
                        temp_offset=_offset;
                        _json_value_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                        _offset=temp_offset;
                        if(_json_data[_offset]==0x22)
                        {
                            if((_offset+1)<_json_data.size())
                            {
                                if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                                {
                                    _offset+=1;
                                }
                            }
                        }
                    }
                    else
                    {
                        //表示是数字 bool NULL
                        for(;_offset<_json_data.size();_offset++)
                        {
                            //结束条件
                            if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                            {
                                break;
                            }
                            _json_value_name.push_back(_json_data[_offset]);
                        }
                        //让前面循环退出或返回
                        if(_offset < _json_data.size() && _json_data[_offset]=='}')
                        {
                            _offset-=1;
                        }
                    }
                    ////////////////////////////////////////////////////////
                    // level1
                    //处理对象赋值
                    )";

        ishas = 0;
        for (unsigned int i = 0; i < json_reflect_data.children.size(); i++)
        {
            if (json_reflect_data.children[i].type.starts_with("unsigned"))
            {
                if (ishas > 0)
                {
                    tempcontent += "else ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                // tempcontent+=array_item_fromjson(json_reflect_data.children[i],jsonobjtypename,1);

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("int"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                // tempcontent+=array_item_fromjson(json_reflect_data.children[i],jsonobjtypename,1);

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stoi(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("long"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                // tempcontent+=array_item_fromjson(json_reflect_data.children[i],jsonobjtypename,1);

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stoll(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("float"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                // tempcontent+=array_item_fromjson(json_reflect_data.children[i],jsonobjtypename,1);

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stof(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("double"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                // tempcontent+=array_item_fromjson(json_reflect_data.children[i],jsonobjtypename,1);

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stod(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("std::string"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {

                                )";

                // tempcontent+=array_item_fromjson(json_reflect_data.children[i],jsonobjtypename,1);

                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=_json_value_name;)";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
        }

        tempcontent += R"(
                    ////////////////////////////////////////////////////////
                    //继续循环下一个键值
                    continue;
                }
            }
        }
        return _offset;
    }
)";
        return tempcontent;
    }

    std::string object_item_fromjson(const struct obj_reflect_type &json_reflect_data,
                                     const std::string &jsonobjtypename,
                                     [[maybe_unused]] unsigned int levelnum = 0)
    {
        std::string tempcontent;
        std::string tempobjname = jsonobjtypename + ".";
        unsigned ishas          = 0;

        std::string obj_name_array_tempname;
        std::string ptype_tempname;

        tempcontent = R"(  

            for(;_offset<_json_data.size();_offset++)
            {
                //去除空格
                _offset=http::json_string_trim(_json_data,_offset);
                //如果是右侧括号表示这个对象已经结束
                if(_json_data[_offset]==0x7D)
                {
                    _offset+=1;
                    break;
                }

                //直到引号
                if(_json_data[_offset]==0x22)
                {
                    unsigned int temp_offset=_offset;
                    _json_value_name.clear();
                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                    _offset=temp_offset;
                    if(_json_data[_offset]==0x22)
                    {
                        if((_offset+1)<_json_data.size())
                        {
                            if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                            {
                                _offset+=1;
                            }
                        }
                    }
                    //键名 后就是键值类型 循环去除空格
                    _offset=http::json_string_trim(_json_data,_offset);
                    if(_offset < _json_data.size() && _json_data[_offset]!=':')
                    {
                        return _offset; 
                    }
                    _offset++;
                    _offset=http::json_string_trim(_json_data,_offset);

                    if(_offset < _json_data.size() && _json_data[_offset]=='{')
                    {   //还是一个对象，表示有嵌套对象
                    )";

        ishas = 0;
        for (unsigned int i = 0; i < json_reflect_data.children.size(); i++)
        {
            if (json_reflect_data.children[i].type == "struct")
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].varname;
                tempcontent += R"(") == 0)
                            {)";
                std::string temp_obj_name_obj = tempobjname + json_reflect_data.children[i].varname;

                tempcontent += object_item_fromjson(json_reflect_data.children[i], temp_obj_name_obj, levelnum);

                tempcontent += R"(     
                            }
                            )";
            }
        }

        // 当没有item时候
        if (ishas == 0)
        {

            // 当没有item时候
            tempcontent += R"(
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == '}')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        )";
        }

        tempcontent += R"(
                        if( _offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }  
                        continue;    
                    }
                    else if(_json_data[_offset]=='[')
                    {   //表示有数组 
                    )";

        ishas = 0;
        for (unsigned int i = 0; i < json_reflect_data.children.size(); i++)
        {

            if (json_reflect_data.children[i].type.size() > 12 &&
                json_reflect_data.children[i].type.starts_with("std::vector<std::string>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 30);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<unsigned long long>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 21);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<long long>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 22);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<double>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 10);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<float>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 11);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<unsigned int>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 23);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<int>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 24);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type == "std::vector<std::vector<std::string>>")
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 60);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<std::vector<double>>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 40);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<std::vector<float>>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 41);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<std::vector<unsigned int>>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 53);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector<std::vector<int>>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 54);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::map<std::string,int>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 6);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::map<std::string,std::string>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 7);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::map<int,std::string>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 8);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::map<std::string,float>"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";
                obj_name_array_tempname = tempobjname + json_reflect_data.children[i].name;
                tempcontent += array_item_fromjson(json_reflect_data.children[i], obj_name_array_tempname, 1, 9);

                tempcontent += R"(     
                            }
                            )";
            }
            else if (json_reflect_data.children[i].type.size() > 12 &&
                     json_reflect_data.children[i].type.starts_with("std::vector"))
            {
                ishas = 1;
                tempcontent += R"(
                            if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                // tempcontent+=array_item_fromjson(json_reflect_data.children[i],jsonobjtypename,1,0);

                tempcontent += R"(
                                _offset=json_decode()";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(,_json_data,_offset);
                            )";

                tempcontent += R"(     
                            }
                            )";
            }
        }

        // 当没有item时候
        if (ishas == 0)
        {

            // 当没有item时候
            tempcontent += R"(
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == ']')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        )";
        }
        tempcontent += R"(  
                        if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }
                        //直接下一个，不用处理键值
                        continue;    
                    }
                    else if(_json_data[_offset]==0x22)
                    {
                        //如果键值也是字符串
                        temp_offset=_offset;
                        _json_value_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                        _offset=temp_offset;
                        if(_json_data[_offset]==0x22)
                        {
                            if((_offset+1)<_json_data.size())
                            {
                                if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                                {
                                    _offset+=1;
                                }
                            }
                        }
                    }
                    else
                    {
                        //表示是数字 bool NULL
                        for(;_offset<_json_data.size();_offset++)
                        {
                            //结束条件
                            if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                            {
                                break;
                            }
                            _json_value_name.push_back(_json_data[_offset]);
                        }
                        //让前面循环退出或返回 主要与数组不同这里是大括号
                        if(_offset < _json_data.size() && _json_data[_offset]=='}')
                        {
                            _offset-=1;
                        }
                    }
                    )";

        ishas = 0;
        for (unsigned int i = 0; i < json_reflect_data.children.size(); i++)
        {

            if (json_reflect_data.children[i].type.starts_with("unsigned"))
            {
                if (ishas > 0)
                {
                    tempcontent += "else ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("int"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stoi(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("long"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stoll(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("float"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stof(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("double"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {)";

                tempcontent += R"(
                                try{
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=std::stod(_json_value_name);
                                }catch (...) 
                                { 
                                    )";
                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=0;
                                }
                            )";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
            else if (json_reflect_data.children[i].type.starts_with("std::string"))
            {
                if (ishas > 0)
                {
                    tempcontent += "\t\t\telse ";
                }
                tempcontent += R"(if (strcasecmp(_json_key_name.c_str(), ")";
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(") == 0)
                            {

                                )";

                tempcontent += tempobjname;
                tempcontent += json_reflect_data.children[i].name;
                tempcontent += R"(=_json_value_name;)";

                tempcontent += R"(     
                            }
                            )";
                ishas += 1;
            }
        }

        tempcontent += R"(  
                    //直接下一个，不用处理键值
                    continue;   

                }
            }

            )";
        return tempcontent;
    }

    std::string array_item_fromjson(const struct obj_reflect_type &json_reflect_data,
                                    const std::string &jsonobjtypename,
                                    [[maybe_unused]] unsigned int levelnum = 0,
                                    unsigned int typenum                   = 0)
    {
        std::string tempcontent;
        std::string tempobjname = jsonobjtypename + ".";
        unsigned ishas          = 0;

        tempcontent = R"(  
            if(_json_data[_offset]==0x5B)
            {
                _offset+=1;
            }

            for(;_offset<_json_data.size();_offset++)
            {
                //去除空格
                _offset=http::json_string_trim(_json_data,_offset);
                //如果是右侧中括号表示这个对象已经结束
                if(_json_data[_offset]==0x5D)
                {
                    _offset+=1;
                    break;
                }
                //直到引号
                if(_json_data[_offset]=='{')
                {   //还是一个对象，表示有嵌套对象
                    //_offset=psy::json_decode(_json_data,_offset);
                    //直接下一个，不用处理键值
                    //需要动态生成 std::stol bool string
                    //json_reflectobj.children.push_back(_json_key_name,_json_value_name);
                    )";
        ishas       = 0;

        if (typenum == 6)
        {
            tempcontent += R"( 
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                _offset=http::json_string_trim(_json_data,_offset);
                                if (_json_data[_offset] == ']')
                                {
                                    break;
                                }
                                if(_json_data[_offset]==0x22)
                                {
                                    unsigned int t_offset=_offset;
                                    
                                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,t_offset);
                                    _offset=t_offset;
                                    _offset=http::json_string_trim(_json_data,_offset);
                                    if(_json_data[_offset]==0x22)
                                    {
                                        _offset+=1;
                                    }
                                    //键名 后就是键值类型 循环去除空格
                                    _offset=http::json_string_trim(_json_data,_offset);
                                    if(_offset < _json_data.size() && _json_data[_offset]!=':')
                                    {
                                        return _offset; 
                                    }
                                    _offset+=1;
                                    _json_value_name.clear();
                                    for(;_offset<_json_data.size();_offset++)
                                    {
                                        //结束条件
                                        if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                                        {
                                            break;
                                        }
                                        _json_value_name.push_back(_json_data[_offset]);
                                    }
                                    )";

            tempcontent += R"( 
                                     try{
                                        )";
            tempcontent += jsonobjtypename;
            tempcontent += R"([_json_key_name]=std::stoi(_json_value_name);
                                    }catch (...) 
                                    { 
                                        )";
            tempcontent += jsonobjtypename;
            tempcontent += R"([_json_key_name]=0;
                                    }
                                    )";
            tempcontent += R"( 
                                }
                                
                            }
                            )";
        }
        else if (typenum == 7)
        {
            tempcontent += R"( 
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                _offset=http::json_string_trim(_json_data,_offset);
                                if (_json_data[_offset] == ']')
                                {
                                    break;
                                }
                                if(_json_data[_offset]==0x22)
                                {
                                    unsigned int t_offset=_offset;
                                    
                                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,t_offset);
                                    _offset=t_offset;
                                    _offset=http::json_string_trim(_json_data,_offset);
                                    if(_json_data[_offset]==0x22)
                                    {
                                        _offset+=1;
                                    }
                                    //键名 后就是键值类型 循环去除空格
                                    _offset=http::json_string_trim(_json_data,_offset);
                                    if(_offset < _json_data.size() && _json_data[_offset]!=':')
                                    {
                                        return _offset; 
                                    }
                                    _offset+=1;
                                    _json_value_name.clear();
                                    for(;_offset<_json_data.size();_offset++)
                                    {
                                        //结束条件
                                        if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                                        {
                                            break;
                                        }
                                        _json_value_name.push_back(_json_data[_offset]);
                                    }
                                    
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"([_json_key_name]=_json_value_name;
                                    )";

            tempcontent += R"( 
                                }
                                
                            }
                            )";
        }
        else if (typenum == 8)
        {
            tempcontent += R"( 
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                _offset=http::json_string_trim(_json_data,_offset);
                                if (_json_data[_offset] == ']')
                                {
                                    break;
                                }
                                if(_json_data[_offset]==0x22)
                                {
                                    unsigned int t_offset=_offset;
                                    
                                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,t_offset);
                                    _offset=t_offset;
                                    _offset=http::json_string_trim(_json_data,_offset);
                                    if(_json_data[_offset]==0x22)
                                    {
                                        _offset+=1;
                                    }
                                    //键名 后就是键值类型 循环去除空格
                                    _offset=http::json_string_trim(_json_data,_offset);
                                    if(_offset < _json_data.size() && _json_data[_offset]!=':')
                                    {
                                        return _offset; 
                                    }
                                    _offset+=1;
                                    _json_value_name.clear();
                                    for(;_offset<_json_data.size();_offset++)
                                    {
                                        //结束条件
                                        if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                                        {
                                            break;
                                        }
                                        _json_value_name.push_back(_json_data[_offset]);
                                    }
                                    
                                    )";

            tempcontent += R"( 
                                     try{
                                        int a=)";
            tempcontent += "=std::stoi(_json_key_name)";
            tempcontent += jsonobjtypename;
            tempcontent += R"([a]=_json_value_name;
                                    }catch (...) 
                                    { 

                                    }
                                   
                                    )";
            tempcontent += R"( 
                                }
                                
                            }
                            )";
        }
        else if (typenum == 9)
        {
            tempcontent += R"( 
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                _offset=http::json_string_trim(_json_data,_offset);
                                if (_json_data[_offset] == ']')
                                {
                                    break;
                                }
                                if(_json_data[_offset]==0x22)
                                {
                                    unsigned int t_offset=_offset;
                                    
                                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,t_offset);
                                    _offset=t_offset;
                                    _offset=http::json_string_trim(_json_data,_offset);
                                    if(_json_data[_offset]==0x22)
                                    {
                                        _offset+=1;
                                    }
                                    //键名 后就是键值类型 循环去除空格
                                    _offset=http::json_string_trim(_json_data,_offset);
                                    if(_offset < _json_data.size() && _json_data[_offset]!=':')
                                    {
                                        return _offset; 
                                    }
                                    _offset+=1;
                                    _json_value_name.clear();
                                    for(;_offset<_json_data.size();_offset++)
                                    {
                                        //结束条件
                                        if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                                        {
                                            break;
                                        }
                                        _json_value_name.push_back(_json_data[_offset]);
                                    }
                                    
                                    )";

            tempcontent += R"( 
                                     try{
                                        )";
            tempcontent += jsonobjtypename;
            tempcontent += R"([_json_key_name]=std::stof(_json_value_name);
                                    }catch (...) 
                                    { 
                                         )";
            tempcontent += jsonobjtypename;
            tempcontent += R"([_json_key_name]=0.0;
                                    }
                                   
                                    )";
            tempcontent += R"( 
                                }
                                
                            }
                            )";
        }

        if (ishas == 0)
        {
            tempcontent += R"(
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == '}')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        )";
        }

        tempcontent += R"(  
                    if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                    {
                        _offset-=1;
                    }
                    continue;    
                }
                else if(_json_data[_offset]=='[')
                {   //表示有数组 
                    ///////////////////////////////////////////////////////
                )";

        ishas = 0;

        if (typenum > 39)
        {
            ishas                   = 1;
            unsigned int temptype_a = typenum - 30;

            if (typenum == 24)
            {
                tempcontent += R"( std::vector<int> a;
                                    )";
                tempcontent += jsonobjtypename;
                tempcontent += R"(.push_back(a);)";
            }
            if (typenum == 23)
            {

                tempcontent += R"( std::vector<unsigned int> a;
                                    )";
                tempcontent += jsonobjtypename;
                tempcontent += R"(.push_back(a);)";
            }

            if (typenum == 10)
            {
                tempcontent += R"( std::vector<double> a;
                                    )";
                tempcontent += jsonobjtypename;
                tempcontent += R"(.push_back(a);)";
            }

            if (typenum == 11)
            {

                tempcontent += R"( std::vector<float> a;
                                    )";
                tempcontent += jsonobjtypename;
                tempcontent += R"(.push_back(a);)";
            }

            if (typenum == 21)
            {
                tempcontent += R"( std::vector<unsigned long long> a;
                                    )";
                tempcontent += jsonobjtypename;
                tempcontent += R"(.push_back(a);)";
            }

            if (typenum == 22)
            {

                tempcontent += R"( std::vector<long long> a;
                                    )";
                tempcontent += jsonobjtypename;
                tempcontent += R"(.push_back(a);)";
            }

            if (temptype_a == 30)
            {
                tempcontent += R"( std::vector<std::string> a;
                                    )";
                tempcontent += jsonobjtypename;
                tempcontent += R"(.push_back(a);)";
            }

            tempcontent += R"(
                        unsigned int temp_size=)";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.size();
                        temp_size-=1;
                        )";

            std::string tempobjname_a = jsonobjtypename + "[temp_size]";
            tempcontent += array_item_fromjson(json_reflect_data, tempobjname_a, levelnum, temptype_a);
        }

        // 当没有item时候
        if (ishas == 0)
        {
            // 当没有item时候
            tempcontent += R"(
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == ']')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        )";
        }

        tempcontent += R"(  
                    ///////////////////////////////////////////////////////
                    //直接下一个，不用处理键值
                    if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                    {
                        _offset-=1;
                    }
                    continue;    
                }
                else if(_json_data[_offset]==0x22)
                {
                    //如果键值也是字符串
                    temp_offset=_offset;
                    _json_value_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                    _offset=temp_offset;
                    if(_json_data[_offset]==0x22)
                    {
                        if((_offset+1)<_json_data.size())
                        {
                            if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                            {
                                _offset+=1;
                            }
                        }
                        
                    }
                    //json_reflectobj.children.push_back(_json_key_name,_json_value_name);
                }
                else
                {
                    //表示是数字 bool NULL
                    for(;_offset<_json_data.size();_offset++)
                    {
                        //结束条件
                        if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                        {
                            break;
                        }
                        _json_value_name.push_back(_json_data[_offset]);
                    }
                    //让前面循环退出或返回 主要与对象不同这里是中括号
                    if(_offset < _json_data.size() && _json_data[_offset]==']')
                    {
                        _offset-=1;
                    }
                }
                // 数字  bool std::string 列表
                //直接下一个，不用处理键值
                /////////////////////////////
                )";

        if (typenum == 24)
        {
            tempcontent += R"(
                                try{
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(std::stoi(_json_value_name));
                                }catch (...) 
                                { 
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(0);
                                }
                            )";
        }
        if (typenum == 23)
        {
            tempcontent += R"(
                                try{
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(std::stoul(_json_value_name));
                                }catch (...) 
                                { 
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(0);
                                }
                            )";
        }

        if (typenum == 10)
        {
            tempcontent += R"(
                                try{
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(std::stod(_json_value_name));
                                }catch (...) 
                                { 
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(0);
                                }
                            )";
        }

        if (typenum == 11)
        {
            tempcontent += R"(
                                try{
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(std::stof(_json_value_name));
                                }catch (...) 
                                { 
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(0);
                                }
                            )";
        }

        if (typenum == 21)
        {
            tempcontent += R"(
                                try{
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(std::stoull(_json_value_name));
                                }catch (...) 
                                { 
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(0);
                                }
                            )";
        }

        if (typenum == 22)
        {
            tempcontent += R"(
                                try{
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(std::stoll(_json_value_name));
                                }catch (...) 
                                { 
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(0);
                                }
                            )";
        }

        if (typenum == 30)
        {
            tempcontent += R"(
                                    )";
            tempcontent += jsonobjtypename;
            tempcontent += R"(.push_back(_json_value_name);)";
        }

        tempcontent += R"(  
                /////////////////////////////
                continue;   
            }
        )";
        return tempcontent;
    }

    std::string array_obj_fromjson(const std::string &jsonobjtypename)
    {
        std::string tempcontent;
        tempcontent = R"(  
unsigned int json_decode(std::vector<)";
        tempcontent += jsonobjtypename;
        tempcontent += R"(> &json_reflectobj,const std::string &_json_data,unsigned int _offset)
      {          
             bool _isarray=false;
        for(;_offset<_json_data.size();_offset++)
        {
            if(_json_data[_offset]=='{')
            {
                break;
            }
            if(_json_data[_offset]=='[')
            {
                _isarray=true;
                break;
            }
        }

        if(_isarray)
        {
            if(_json_data[_offset]=='[')
            {
                _offset+=1;
            }
            for(;_offset<_json_data.size();_offset++)
            {
                _offset=http::json_string_trim(_json_data,_offset);
                //直接返回，这样可以防插入空的对象
                if(_json_data[_offset]==0x5D)
                {
                    return _offset;
                }else if(_json_data[_offset]=='{')
                {
                    )";
        tempcontent += jsonobjtypename;
        tempcontent += R"( temp;
                    _offset=json_decode(temp,_json_data,_offset);
                    json_reflectobj.push_back(temp);
                }

            }
            
        }
        else
        {
            )";
        tempcontent += jsonobjtypename;
        tempcontent += R"( temp;
            _offset=json_decode(temp,_json_data,_offset);
            json_reflectobj.push_back(temp);
            
        }

        return _offset; 
    }
         )";
        return tempcontent;
    }

    std::string single_obj_to_json(const struct obj_reflect_type &json_reflect_data,
                                   const std::string &jsonobjtypename,
                                   std::string objnameval = "",
                                   bool isheadcontent     = true,
                                   unsigned int leveobj   = 1)
    {
        std::string maincontent;
        if (isheadcontent)
        {
            maincontent = R"( 
    std::stringstream _stream;
    _stream << "{"; 
    )";
        }
        else
        {
            maincontent = R"( 
    _stream << "{"; 
    )";
        }

        std::string tempobjname = objnameval + ".";
        std::string tempdouhao  = "\t_stream <<\",\";\n\t";
        for (unsigned int i = 0; i < json_reflect_data.children.size(); i++)
        {
            if (json_reflect_data.children[i].name.size() > 0 && json_reflect_data.children[i].name[0] == '_')
            {
                continue;
            }
            if (i > 0)
            {
                maincontent += "\t_stream <<\",\";\n\t";
            }
            if (json_reflect_data.children[i].type == "struct")
            {
                std::string templevel = tempobjname + json_reflect_data.children[i].varname;
                maincontent += "_stream <<\"\\\"" + json_reflect_data.children[i].varname + "\\\":\";";
                unsigned int newlevelobj = leveobj + 1;
                maincontent +=
                    single_obj_to_json(json_reflect_data.children[i], jsonobjtypename, templevel, false, newlevelobj);
            }
            else
            {
                maincontent +=
                    single_obj_itemto_json(json_reflect_data.children[i], jsonobjtypename, tempobjname, leveobj);
            }
        }

        if (isheadcontent)
        {
            maincontent += R"(
    _stream << "}";

    return _stream.str();
            )";
        }
        else
        {
            maincontent += R"( 
            _stream << "}";
                )";
        }

        return maincontent;
    }
    std::string single_obj_itemto_json(const struct obj_reflect_type &json_reflect_data,
                                       const std::string &jsonobjtypename,
                                       std::string pre_name                   = "",
                                       [[maybe_unused]] unsigned int levelnum = 0)
    {
        std::string maincontent;
        std::string selfitem = "std::vector<" + jsonobjtypename + ">";
        if (json_reflect_data.name.size() > 0 && json_reflect_data.name[0] == '_')
        {
            return "";
        }
        if (json_reflect_data.type == "unsigned int")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":" << std::to_string()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"();
                )";
        }
        else if (json_reflect_data.type == "unsigned char")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":" << std::to_string()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"();
                )";
        }
        else if (json_reflect_data.type == "int")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":" << std::to_string()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"();
                )";
        }
        else if (json_reflect_data.type == "char")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":" << std::to_string()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"();
                )";
        }
        else if (json_reflect_data.type == "unsigned long long")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":" << std::to_string()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"();
                )";
        }
        else if (json_reflect_data.type == "long long")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":" << std::to_string()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"();
                )";
        }
        else if (json_reflect_data.type == "float")
        {

            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":";)";
            maincontent += R"(
        
        {
            std::string temp;
            std::stringstream _floatstream;
            _floatstream.precision(15);
            _floatstream <<)";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(;
            temp = _floatstream.str();
            unsigned int i = 0;
            for (; i < temp.length(); i++)
            {
                if (temp[i] == '.')
                {
                break;
                }
            }
            if (i == temp.length())
            {
                temp.append(".0");
            }
            _stream<<temp;
        }
        
        )";
        }
        else if (json_reflect_data.type == "double")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":";)";
            maincontent += R"(
        
        {
            std::string temp;
            std::stringstream _floatstream;
            _floatstream.precision(15);
            _floatstream <<)";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(;
            temp = _floatstream.str();
            unsigned int i = 0;
            for (; i < temp.length(); i++)
            {
                if (temp[i] == '.')
                {
                break;
                }
            }
            if (i == temp.length())
            {
                temp.append(".0");
            }
            _stream<<temp;
        }
        
)";
        }
        else if (json_reflect_data.type == "bool")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":" << ()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"( ? "true" : "false");
                )";
        }
        else if (json_reflect_data.type == "std::string")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":\"" << http::utf8_to_jsonstring()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"()<< "\"";
                )";
        }
        else if (json_reflect_data.type == selfitem)
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      for (unsigned int i = 0; i <)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<json_encode()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i]);
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type == "std::vector<int>" ||
                 json_reflect_data.type == "std::vector<unsigned int>" ||
                 json_reflect_data.type == "std::vector<float>" || json_reflect_data.type == "std::vector<double>" ||
                 json_reflect_data.type == "std::vector<long long>" ||
                 json_reflect_data.type == "std::vector<unsigned long long>")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      for (unsigned int i = 0; i <)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<)";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(;
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type == "std::vector<std::string>")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      for (unsigned int i = 0; i <)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<"\""<<http::utf8_to_jsonstring()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i])<<"\"";
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type == "std::vector<std::vector<std::string>>")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      for (unsigned int i = 0; i <)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream << "[";
         for (unsigned int j = 0; j <)";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i].size(); i++)
       {
        if (j > 0)
        {
          _stream << ",";
        }
        _stream <<"\""<<http::utf8_to_jsonstring()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i][j])<<"\"";
        }
        _stream << "]";
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type == "std::vector<std::vector<float>>")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      for (unsigned int i = 0; i <)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream << "[";
         for (unsigned int j = 0; j <)";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i].size(); i++)
       {
        if (j > 0)
        {
          _stream << ",";
        }
        _stream <<)";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i][j];
        }
        _stream << "]";
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type == "std::vector<std::vector<int>>")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      for (unsigned int i = 0; i <)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream << "[";
         for (unsigned int j = 0; j <)";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i].size(); i++)
       {
        if (j > 0)
        {
          _stream << ",";
        }
        _stream <<)";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i][j];
        }
        _stream << "]";
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type.size() > 3 && json_reflect_data.type[0] != 's' &&
                 json_reflect_data.type[1] != 't' && json_reflect_data.type[2] != 'd' &&
                 json_reflect_data.type[3] != ':')
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":" << )";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(.to_json();
                )";
        }
        else if (json_reflect_data.type.size() > 10 && json_reflect_data.type[0] == 's' &&
                 json_reflect_data.type[1] == 't' && json_reflect_data.type[2] == 'd' &&
                 json_reflect_data.type[3] == ':' && json_reflect_data.type[4] == ':' &&
                 json_reflect_data.type[5] == 'v' && json_reflect_data.type[6] == 'e' &&
                 json_reflect_data.type[7] == 'c' && json_reflect_data.type[8] == 't' &&
                 json_reflect_data.type[9] == 'o' && json_reflect_data.type[10] == 'r' &&
                 json_reflect_data.type[11] == '<')
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      for (unsigned int i = 0; i <)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"(.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<json_encode()";
            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"([i]);
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type == "std::map<int,std::string>")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      unsigned int count_i=0;     
      for (auto&[first,second]:)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"()
      {
        if (count_i > 0)
        {
          _stream << ",";
        }
        _stream <<"{\""<<std::to_string(first)<<"\",\""<<http::utf8_to_jsonstring(second)<<"\"}";
        count_i+=1;
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type == "std::map<std::string,std::string>")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
      unsigned int count_i=0;     
      for (auto&[first,second]:)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"()
      {
        if (count_i > 0)
        {
          _stream << ",";
        }
        _stream <<"{\""<<http::utf8_to_jsonstring(first)<<"\",\""<<http::utf8_to_jsonstring(second)<<"\"}";
        count_i+=1;
      }
      _stream << "]";
        )";
        }
        else if (json_reflect_data.type == "std::map<std::string,int>")
        {
            maincontent += R"(_stream << "\")";
            maincontent += json_reflect_data.name;
            maincontent += R"(\":[";
           unsigned int count_i=0;     
      for (auto&[first,second]:)";

            maincontent += pre_name;
            maincontent += json_reflect_data.name;
            maincontent += R"()
      {
        if (count_i > 0)
        {
          _stream << ",";
        }
        _stream <<"{\""<<http::utf8_to_jsonstring(first)<<"\","<<std::to_string(second)<<"}";
        count_i+=1;
      }
      _stream << "]";
        )";
        }
        return maincontent;
    }
    std::string get_jsonreflect_header(const std::vector<struct obj_reflect_type> &methodpathfile)
    {
        std::string temp;
        std::string namespacename;
        namespacename.clear();
        if (methodpathfile.size() > 0)
        {
            namespacename = methodpathfile[0].namespacename;
        }
        if (namespacename.size() > 0)
        {
            temp.append("\nnamespace ");
            temp.append(namespacename);
            temp.append("\n{\n");
        }
        for (unsigned int i = 0; i < methodpathfile.size(); i++)
        {

            temp += "\n\nstd::string json_encode(const " + methodpathfile[i].name + " &json_reflectobj);\n";
            temp += "\n\nstd::string json_encode(const std::vector<" + methodpathfile[i].name + "> &json_reflectobj);";
            temp += "\n\nunsigned int json_decode(" + methodpathfile[i].name +
                    " &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);";
            temp += "\n\nunsigned int json_decode(std::vector<" + methodpathfile[i].name +
                    "> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);";
        }
        if (namespacename.size() > 0)
        {
            temp.append("\n}\n");
        }
        return temp;
    }
    void createhjson_reflectfile(const std::string &filename,
                                 const std::string &content,
                                 const std::vector<std::string> hfiles)
    {
        std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(filename.c_str(), "wb"), std::fclose);

        if (!fp.get())
        {
            return;
        }

        std::string header_content = R"(#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <map>
)";

        for (unsigned int i = 0; i < hfiles.size(); i++)
        {
            header_content += "\n" + hfiles[i];
        }

        header_content += R"(
#include "unicode.h"

template<typename JSON_REF_OBJ_TEMP>
std::string json_encode([[maybe_unused]] const JSON_REF_OBJ_TEMP &json_reflectobj){ return ""; }

template<typename JSON_REF_OBJ_TEMP>
std::string json_encode([[maybe_unused]] const std::vector<JSON_REF_OBJ_TEMP> &json_reflectobj){ return ""; }

template<typename JSON_REF_OBJ_TEMP>
unsigned int json_decode([[maybe_unused]] JSON_REF_OBJ_TEMP &json_reflectobj,[[maybe_unused]] const std::string &_json_data,[[maybe_unused]] unsigned int _offset){ return 0; }

template<typename JSON_REF_OBJ_TEMP>
unsigned int json_decode([[maybe_unused]] std::vector<JSON_REF_OBJ_TEMP> &json_reflectobj,[[maybe_unused]] const std::string &_json_data,[[maybe_unused]] unsigned int _offset){ return 0; }

)";
        fwrite(&header_content[0], 1, header_content.size(), fp.get());

        fwrite(&content[0], 1, content.size(), fp.get());
    }

  public:
    std::vector<std::string> filelists;
};