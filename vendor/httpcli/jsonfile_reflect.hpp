/*
 *  @author 黄自权 huangziquan
 *  @dest generate json_encode/json_decode struct from a .json sample file
 *
 *  从 libs 目录下的 .json 样例文件解析结构，生成:
 *    <base>.h   : namespace json 下的 struct 定义
 *  然后复用 typestatement.hpp 里的 createhfile / get_jsonreflect_header
 *  生成 <base>.cpp 与并入 common/json_reflect_headers.h。
 *
 *  通用数据模型复用 obj_reflect_type。
 */
#pragma once
#include <string>
#include <vector>
#include <utility>
#include <cstdio>
#include <cctype>
#include <memory>
#include <filesystem>

#include "typestatement.hpp"

namespace jsonfilereflect
{

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// 通用 JSON 值树
// ---------------------------------------------------------------------------
struct jsonfile_value
{
    enum kind_t
    {
        NUL,
        BOOLEAN,
        NUMBER,
        STRING,
        OBJECT,
        ARRAY
    };
    kind_t kind    = NUL;
    bool is_double = false;
    std::string str;                                              // STRING 的解码后内容(仅键使用)
    std::vector<std::pair<std::string, jsonfile_value>> members; // OBJECT (保序)
    std::vector<jsonfile_value> elements;                        // ARRAY
};

// ---------------------------------------------------------------------------
// 最小 JSON 解析器 (递归下降)
// 字符串严格按引号+转义扫描, 因此 )" 反斜杠 emoji 等不会破坏解析。
// ---------------------------------------------------------------------------
class jsonfile_parser
{
  public:
    explicit jsonfile_parser(const std::string &data) : _data(data), _pos(0), _n(data.size())
    {
    }

    jsonfile_value parse()
    {
        return parse_value();
    }

  private:
    const std::string &_data;
    std::size_t _pos;
    std::size_t _n;

    void skipws()
    {
        while (_pos < _n)
        {
            char c = _data[_pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            {
                _pos++;
                continue;
            }
            break;
        }
    }

    void append_utf8(std::string &out, unsigned int cp)
    {
        if (cp < 0x80)
        {
            out.push_back(static_cast<char>(cp));
        }
        else if (cp < 0x800)
        {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
        else
        {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }

    std::string parse_string_value()
    {
        std::string out;
        _pos++; // skip opening quote
        while (_pos < _n)
        {
            char c = _data[_pos];
            if (c == '\\')
            {
                _pos++;
                if (_pos >= _n)
                {
                    break;
                }
                char e = _data[_pos];
                switch (e)
                {
                case '"':
                    out.push_back('"');
                    break;
                case '\\':
                    out.push_back('\\');
                    break;
                case '/':
                    out.push_back('/');
                    break;
                case 'n':
                    out.push_back('\n');
                    break;
                case 't':
                    out.push_back('\t');
                    break;
                case 'r':
                    out.push_back('\r');
                    break;
                case 'b':
                    out.push_back('\b');
                    break;
                case 'f':
                    out.push_back('\f');
                    break;
                case 'u':
                {
                    unsigned int cp = 0;
                    for (int k = 0; k < 4 && (_pos + 1) < _n; k++)
                    {
                        _pos++;
                        char h  = _data[_pos];
                        cp    <<= 4;
                        if (h >= '0' && h <= '9')
                            cp |= static_cast<unsigned int>(h - '0');
                        else if (h >= 'a' && h <= 'f')
                            cp |= static_cast<unsigned int>(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F')
                            cp |= static_cast<unsigned int>(h - 'A' + 10);
                    }
                    append_utf8(out, cp);
                    break;
                }
                default:
                    out.push_back(e);
                    break;
                }
                _pos++;
            }
            else if (c == '"')
            {
                _pos++;
                break;
            }
            else
            {
                out.push_back(c);
                _pos++;
            }
        }
        return out;
    }

    jsonfile_value parse_object()
    {
        jsonfile_value v;
        v.kind = jsonfile_value::OBJECT;
        _pos++; // skip {
        while (_pos < _n)
        {
            std::size_t prev = _pos;
            skipws();
            if (_pos >= _n)
            {
                break;
            }
            if (_data[_pos] == '}')
            {
                _pos++;
                break;
            }
            if (_data[_pos] == ',')
            {
                _pos++;
                continue;
            }
            if (_data[_pos] == '"')
            {
                std::string key = parse_string_value();
                skipws();
                if (_pos < _n && _data[_pos] == ':')
                {
                    _pos++;
                }
                jsonfile_value val = parse_value();
                v.members.emplace_back(std::move(key), std::move(val));
            }
            else
            {
                _pos++;
            }
            if (_pos == prev)
            {
                _pos++; // 防止死循环
            }
        }
        return v;
    }

    jsonfile_value parse_array()
    {
        jsonfile_value v;
        v.kind = jsonfile_value::ARRAY;
        _pos++; // skip [
        while (_pos < _n)
        {
            std::size_t prev = _pos;
            skipws();
            if (_pos >= _n)
            {
                break;
            }
            if (_data[_pos] == ']')
            {
                _pos++;
                break;
            }
            if (_data[_pos] == ',')
            {
                _pos++;
                continue;
            }
            jsonfile_value e = parse_value();
            v.elements.push_back(std::move(e));
            if (_pos == prev)
            {
                _pos++; // 防止死循环
            }
        }
        return v;
    }

    jsonfile_value parse_value()
    {
        skipws();
        jsonfile_value v;
        if (_pos >= _n)
        {
            return v;
        }
        char c = _data[_pos];
        if (c == '{')
        {
            return parse_object();
        }
        if (c == '[')
        {
            return parse_array();
        }
        if (c == '"')
        {
            v.kind = jsonfile_value::STRING;
            v.str  = parse_string_value();
            return v;
        }
        if (c == 't' || c == 'f')
        {
            v.kind = jsonfile_value::BOOLEAN;
            while (_pos < _n && std::isalpha(static_cast<unsigned char>(_data[_pos])))
            {
                _pos++;
            }
            return v;
        }
        if (c == 'n')
        {
            v.kind = jsonfile_value::NUL;
            while (_pos < _n && std::isalpha(static_cast<unsigned char>(_data[_pos])))
            {
                _pos++;
            }
            return v;
        }
        // number
        v.kind = jsonfile_value::NUMBER;
        while (_pos < _n)
        {
            char d = _data[_pos];
            if ((d >= '0' && d <= '9') || d == '-' || d == '+' || d == '.' || d == 'e' || d == 'E')
            {
                if (d == '.' || d == 'e' || d == 'E')
                {
                    v.is_double = true;
                }
                _pos++;
            }
            else
            {
                break;
            }
        }
        return v;
    }
};

// ---------------------------------------------------------------------------
// JSON 值树 -> std::vector<obj_reflect_type> (后序: 依赖类型在前, root 在后)
// ---------------------------------------------------------------------------
class jsonfile_builder
{
  public:
    std::vector<obj_reflect_type> build(const jsonfile_value &root, const std::string &basename,
                                        const std::string &ns)
    {
        _ns = ns;
        std::vector<obj_reflect_type> topo;
        if (root.kind == jsonfile_value::OBJECT)
        {
            obj_reflect_type st = build_struct(root, basename, basename, topo);
            st.varname          = "";
            topo.push_back(st);
        }
        else if (root.kind == jsonfile_value::ARRAY)
        {
            // 根为数组: 生成元素结构体命名为 basename, 由其 vector 重载服务
            jsonfile_value merged = merge_object_elements(root);
            obj_reflect_type st   = build_struct(merged, basename, basename, topo);
            st.varname            = "";
            topo.push_back(st);
        }
        return topo;
    }

  private:
    std::string _ns;

    static bool is_numeric_type(const std::string &type)
    {
        if (type == "bool" || type == "struct" || type == "rawjson")
        {
            return false;
        }
        if (type.starts_with("std::"))
        {
            return false;
        }
        return true; // long long / double / int ...
    }

    // 合并对象数组中所有元素的键(取首次出现的类型), 生成一个并集对象
    jsonfile_value merge_object_elements(const jsonfile_value &arr)
    {
        jsonfile_value merged;
        merged.kind = jsonfile_value::OBJECT;
        for (const auto &e : arr.elements)
        {
            if (e.kind != jsonfile_value::OBJECT)
            {
                continue;
            }
            for (const auto &m : e.members)
            {
                bool found = false;
                for (const auto &exist : merged.members)
                {
                    if (exist.first == m.first)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    merged.members.emplace_back(m.first, m.second);
                }
            }
        }
        return merged;
    }

    obj_reflect_type build_struct(const jsonfile_value &obj, const std::string &structname,
                                  const std::string &pathprefix, std::vector<obj_reflect_type> &topo)
    {
        obj_reflect_type st;
        st.name          = structname;
        st.type          = "struct";
        st.typevalue     = 1;
        st.namespacename = _ns;

        for (const auto &member : obj.members)
        {
            const std::string &key      = member.first;
            const jsonfile_value &val    = member.second;
            obj_reflect_type child;
            child.namespacename = _ns;
            child.name          = key;
            child.varname       = key;

            switch (val.kind)
            {
            case jsonfile_value::NUL:
                child.type = "rawjson";
                break;
            case jsonfile_value::BOOLEAN:
                child.type = "bool";
                break;
            case jsonfile_value::NUMBER:
                child.type = val.is_double ? "double" : "long long";
                break;
            case jsonfile_value::STRING:
                child.type = "std::string";
                break;
            case jsonfile_value::OBJECT:
            {
                std::string childstruct = pathprefix + "_" + key + "_t";
                obj_reflect_type inlinest =
                    build_struct(val, childstruct, pathprefix + "_" + key, topo);
                inlinest.type    = "struct";
                inlinest.name    = childstruct;
                inlinest.varname = key;
                child            = inlinest;
                break;
            }
            case jsonfile_value::ARRAY:
                child = classify_array(val, key, pathprefix, topo);
                break;
            }
            st.children.push_back(child);
        }
        return st;
    }

    obj_reflect_type classify_array(const jsonfile_value &arr, const std::string &key,
                                    const std::string &pathprefix, std::vector<obj_reflect_type> &topo)
    {
        obj_reflect_type child;
        child.namespacename = _ns;
        child.name          = key;
        child.varname       = key;

        if (arr.elements.empty())
        {
            child.type = "rawjson";
            return child;
        }

        bool allobj = true, allstr = true, allnum = true, allbool = true, allarr = true;
        for (const auto &e : arr.elements)
        {
            allobj  = allobj && (e.kind == jsonfile_value::OBJECT);
            allstr  = allstr && (e.kind == jsonfile_value::STRING);
            allnum  = allnum && (e.kind == jsonfile_value::NUMBER);
            allbool = allbool && (e.kind == jsonfile_value::BOOLEAN);
            allarr  = allarr && (e.kind == jsonfile_value::ARRAY);
        }

        if (allobj)
        {
            std::string elemname = pathprefix + "_" + key + "_t";
            jsonfile_value merged = merge_object_elements(arr);
            obj_reflect_type elemst =
                build_struct(merged, elemname, pathprefix + "_" + key, topo);
            elemst.varname       = "";
            elemst.type          = "struct";
            elemst.name          = elemname;
            elemst.namespacename = _ns;
            topo.push_back(elemst);
            child.type = "std::vector<" + elemname + ">";
            return child;
        }
        if (allstr)
        {
            child.type = "std::vector<std::string>";
            return child;
        }
        if (allnum)
        {
            bool anyd = false;
            for (const auto &e : arr.elements)
            {
                if (e.is_double)
                {
                    anyd = true;
                }
            }
            child.type = anyd ? "std::vector<double>" : "std::vector<long long>";
            return child;
        }
        if (allarr)
        {
            bool ok = true, sawstr = false, sawnum = false, anyd = false;
            for (const auto &e : arr.elements)
            {
                if (e.elements.empty())
                {
                    ok = false;
                    break;
                }
                for (const auto &x : e.elements)
                {
                    if (x.kind == jsonfile_value::STRING)
                    {
                        sawstr = true;
                    }
                    else if (x.kind == jsonfile_value::NUMBER)
                    {
                        sawnum = true;
                        if (x.is_double)
                        {
                            anyd = true;
                        }
                    }
                    else
                    {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                {
                    break;
                }
            }
            if (ok && sawstr && !sawnum)
            {
                child.type = "std::vector<std::vector<std::string>>";
                return child;
            }
            if (ok && sawnum && !sawstr)
            {
                child.type = anyd ? "std::vector<std::vector<double>>" : "std::vector<std::vector<int>>";
                return child;
            }
        }

        // bool 数组 / 混合类型 / 过深嵌套 -> 原始 JSON 透传
        child.type = "rawjson";
        return child;
    }
};

// ---------------------------------------------------------------------------
// 生成 <base>.h : namespace json 下的 struct 定义(不含 //@reflect 标记)
// ---------------------------------------------------------------------------
class jsonfile_hwriter
{
  public:
    void write(const std::string &filename, const std::vector<obj_reflect_type> &topo,
               const std::string &relname, const std::string &ns)
    {
        std::string guard = make_guard(relname);
        std::string out;
        out += "#ifndef " + guard + "\n#define " + guard + "\n";
        out += "#include <string>\n#include <vector>\n#include <map>\n\n";
        out += "namespace " + ns + "\n{\n";
        for (const auto &st : topo)
        {
            out += emit_struct(st);
            out += "\n";
        }
        out += "}// namespace " + ns + "\n#endif\n";

        std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(filename.c_str(), "wb"), std::fclose);
        if (!fp.get())
        {
            return;
        }
        fwrite(&out[0], 1, out.size(), fp.get());
    }

  private:
    static std::string make_guard(const std::string &relname)
    {
        std::string guard = "PAOZHU_JSON_";
        for (char c : relname)
        {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
            {
                guard.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
            }
            else
            {
                guard.push_back('_');
            }
        }
        guard += "_H";
        return guard;
    }

    static bool is_numeric_type(const std::string &type)
    {
        if (type == "bool" || type == "struct" || type == "rawjson")
        {
            return false;
        }
        if (type.starts_with("std::"))
        {
            return false;
        }
        return true;
    }

    std::string emit_struct(const obj_reflect_type &st)
    {
        std::string out;
        out += "struct " + st.name + "\n{\n";
        for (const auto &c : st.children)
        {
            out += emit_member(c, 1);
        }
        out += "};\n";
        return out;
    }

    std::string emit_member(const obj_reflect_type &child, int indent)
    {
        std::string ind(static_cast<std::size_t>(indent) * 4, ' ');
        if (child.type == "struct")
        {
            std::string out = ind + "struct " + child.name + "\n" + ind + "{\n";
            for (const auto &gc : child.children)
            {
                out += emit_member(gc, indent + 1);
            }
            out += ind + "} " + child.varname + ";\n";
            return out;
        }

        std::string decl_type = child.type;
        std::string def;
        if (child.type == "rawjson")
        {
            decl_type = "std::string";
            def       = " = \"null\"";
        }
        else if (child.type == "bool")
        {
            def = " = false";
        }
        else if (is_numeric_type(child.type))
        {
            def = " = 0";
        }
        return ind + decl_type + " " + child.name + def + ";\n";
    }
};

// ---------------------------------------------------------------------------
// 递归扫描目录下所有 .json, 返回相对路径(不含 .json 扩展名)
// ---------------------------------------------------------------------------
inline std::vector<std::string> readalljsonfile(const std::string &methodpath)
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
                if (ext == ".json")
                {
                    temp.emplace_back(filename.substr(0, filename.size() - 5));
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
                std::vector<std::string> directory_temp = readalljsonfile(ext);
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
    return temp;
}

// 读取文件内容
inline std::string read_json_file(const std::string &pathfile)
{
    std::string filecontent;
    std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(pathfile.c_str(), "rb"), std::fclose);
    if (!fp.get())
    {
        return filecontent;
    }
    fseek(fp.get(), 0, SEEK_END);
    long ftellsize = ftell(fp.get());
    fseek(fp.get(), 0, SEEK_SET);
    if (ftellsize <= 0)
    {
        return filecontent;
    }
    filecontent.resize(static_cast<std::size_t>(ftellsize));
    std::size_t rd = fread(&filecontent[0], 1, static_cast<std::size_t>(ftellsize), fp.get());
    filecontent.resize(rd);
    return filecontent;
}

} // namespace jsonfilereflect
