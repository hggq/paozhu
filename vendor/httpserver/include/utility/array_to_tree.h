#ifndef HTTP_ARRAY_TO_TREE_H
#define HTTP_ARRAY_TO_TREE_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <type_traits>

namespace http
{
template <typename deps_json_type>
concept ishas_isuse_member = requires(deps_json_type m) { m._is_use; };

template <typename deps_json_type>
concept ishas_islevel_member = requires(deps_json_type m) { m._level; };

template <typename deps_json_type>
    requires ishas_isuse_member<deps_json_type>
void array_to_tree(std::vector<deps_json_type> &targetdata, std::vector<deps_json_type> &sourcedata)
{
    if (targetdata.size() == 0)
    {
        for (unsigned int i = 0; i < sourcedata.size(); i++)
        {
            if (sourcedata[i].parentid == 0 && sourcedata[i]._is_use == false)
            {
                targetdata.push_back(sourcedata[i]);
                sourcedata[i]._is_use = true;
            }
        }
    }
    for (unsigned int i = 0; i < sourcedata.size(); i++)
    {
        if (sourcedata[i]._is_use == false)
        {
            for (unsigned int j = 0; j < targetdata.size(); j++)
            {
                if (targetdata[j].id == sourcedata[i].parentid)
                {
                    targetdata[j].children.push_back(sourcedata[i]);
                    sourcedata[i]._is_use = true;
                    array_to_tree(targetdata[j].children, sourcedata);
                }
            }
        }
    }
}

template <typename deps_json_type>

void array_to_tree(std::vector<deps_json_type> &targetdata, std::vector<deps_json_type> &sourcedata)
{

    if (targetdata.size() == 0)
    {
        for (unsigned int i = 0; i < sourcedata.size(); i++)
        {
            if (sourcedata[i].parentid == 0)
            {
                targetdata.push_back(sourcedata[i]);
            }
        }
    }

    for (unsigned int i = 0; i < sourcedata.size(); i++)
    {
        if (sourcedata[i].parentid > 0)
        {
            for (unsigned int j = 0; j < targetdata.size(); j++)
            {
                if (targetdata[j].id == sourcedata[i].parentid)
                {
                    targetdata[j].children.push_back(sourcedata[i]);
                    array_to_tree(targetdata[j].children, sourcedata);
                }
            }
        }
    }
}

template <typename deps_json_type>
    requires ishas_islevel_member<deps_json_type>
void tree_to_array(std::vector<deps_json_type> &targetdata,
                   std::vector<deps_json_type> &sourcedata,
                   unsigned int parendid = 0,
                   unsigned int level    = 0)
{

    for (unsigned int i = 0; i < sourcedata.size(); i++)
    {
        if (sourcedata[i].parentid == parendid)
        {
            if (sourcedata[i].children.size() > 0)
            {
                deps_json_type temp = sourcedata[i];
                temp.children.clear();
                temp._level = level;
                targetdata.push_back(temp);
                tree_to_array(targetdata, sourcedata[i].children, sourcedata[i].id, level + 1);
            }
            else
            {
                sourcedata[i]._level = level;
                targetdata.push_back(sourcedata[i]);
            }
        }
    }
}

template <typename deps_json_type>
void tree_to_array(std::vector<deps_json_type> &targetdata,
                   std::vector<deps_json_type> &sourcedata,
                   unsigned int parendid = 0,
                   unsigned int level    = 0)
{

    for (unsigned int i = 0; i < sourcedata.size(); i++)
    {
        if (sourcedata[i].parentid == parendid)
        {
            if (sourcedata[i].children.size() > 0)
            {
                deps_json_type temp = sourcedata[i];
                temp.children.clear();
                targetdata.push_back(temp);
                tree_to_array(targetdata, sourcedata[i].children, sourcedata[i].id, level + 1);
            }
            else
            {
                targetdata.push_back(sourcedata[i]);
            }
        }
    }
}

template <typename deps_json_type> std::string tree_to_json(std::vector<deps_json_type> &sourcedata)
{
    std::stringstream stream;
    stream << "[";
    for (unsigned int i = 0; i < sourcedata.size(); i++)
    {
        if (i > 0)
        {
            stream << ",";
        }
        stream << sourcedata[i].to_json();
    }
    stream << "]";
    return stream.str();
}
}// namespace http
#endif