#ifndef __HTTP_PZ_CACHE_H
#define __HTTP_PZ_CACHE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif// defined(_MSC_VER) && (_MSC_VER >= 1200)
#include <string>
#include <map>
#include <mutex>

namespace http
{
template <typename BASETYPE_T>
std::map<std::string, BASETYPE_T> &get_pz_cache()
{
    static std::map<std::string, BASETYPE_T> instance;
    return instance;
}

template <typename BASE_TYPE>
class pzcache
{
  private:
    pzcache(){};
    ~pzcache(){};
    pzcache(const pzcache &);
    pzcache &operator=(const pzcache &);

  public:
    struct data_cache_t
    {
        BASE_TYPE data;
        unsigned int exptime = 0;
    };

  public:
    void save(const std::string &hashid, const BASE_TYPE &data_list, int expnum = 0, bool cover_data = false)
    {
        std::map<std::string, data_cache_t> &obj = get_pz_cache<data_cache_t>();
        struct data_cache_t temp;
        temp.data = data_list;
        if (expnum != 0)
        {
            temp.exptime = http::timeid() + expnum;
        }
        else
        {
            temp.exptime = 0;
        }
        std::unique_lock<std::mutex> lock(editlock);
        auto [_, success] = obj.insert({hashid, temp});
        if (!success)
        {
            if (cover_data)
            {
                obj[hashid] = temp;
            }
            else
            {
                obj[hashid].exptime = temp.exptime;
            }
        }
    }
    void save(const std::string &hashid, BASE_TYPE &&data_list, int expnum = 0, bool cover_data = false)
    {
        std::map<std::string, data_cache_t> &obj = get_pz_cache<data_cache_t>();
        struct data_cache_t temp;
        temp.data = std::move(data_list);
        if (expnum != 0)
        {
            temp.exptime = http::timeid() + expnum;
        }
        else
        {
            temp.exptime = 0;
        }
        std::unique_lock<std::mutex> lock(editlock);
        auto [_, success] = obj.emplace(hashid, std::move(temp));
        if (!success)
        {
            if (cover_data)
            {
                obj[hashid] = temp;
            }
            else
            {
                obj[hashid].exptime = temp.exptime;
            }
        }
    }

    bool remove(const std::string &hashid)
    {
        std::map<std::string, data_cache_t> &obj = get_pz_cache<data_cache_t>();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            obj.erase(iter++);
            return true;
        }
        return false;
    }
    void remove_exptime()
    {
        std::map<std::string, data_cache_t> &obj = get_pz_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid();
        std::unique_lock<std::mutex> lock(editlock);
        for (auto iter = obj.begin(); iter != obj.end();)
        {
            if (iter->second.exptime == 0)
            {
                continue;
            }
            if (iter->second.exptime < nowtime)
            {
                obj.erase(iter++);
            }
        }
    }
    void clear()
    {
        std::map<std::string, data_cache_t> &obj = get_pz_cache<data_cache_t>();
        std::unique_lock<std::mutex> lock(editlock);
        obj.clear();
    }
    int check(const std::string &hashid)
    {
        std::map<std::string, data_cache_t> &obj = get_pz_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime == 0)
            {
                return 0;
            }
            int temp = (int)(iter->second.exptime - nowtime);
            if (temp == -1)
            {
                return -2;
            }
            return temp;
        }
        return -1;
    }

    int update(const std::string &hashid, int exptime = 0)
    {
        std::map<std::string, data_cache_t> &obj = get_pz_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid() + exptime;
        if (exptime == 0)
        {
            nowtime = 0;
        }
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime == 0)
            {
                iter->second.exptime = nowtime;
                return 0;
            }
            iter->second.exptime = nowtime;
            return 1;
        }
        return -1;
    }

    const BASE_TYPE &get(const std::string &hashid)
    {
        std::map<std::string, data_cache_t> &obj = get_pz_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime == 0)
            {
                return iter->second.data;
            }

            if (iter->second.exptime >= nowtime)
            {
                return iter->second.data;
            }
            else
            {
                obj.erase(iter++);
            }
        }
        throw "Not in Cache";
    }
    static pzcache &conn()
    {
        static pzcache instance;
        return instance;
    }

  public:
    std::mutex editlock;
};
}// namespace http
#endif