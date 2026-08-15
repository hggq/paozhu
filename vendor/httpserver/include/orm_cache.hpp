#ifndef ORM_CACHE_HPP
#define ORM_CACHE_HPP

#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include <memory>
#include <ctime>

namespace orm
{

inline unsigned int orm_timeid()
{
    return static_cast<unsigned int>(std::time(nullptr));
}

template <typename BASE_T>
std::map<std::size_t, BASE_T> &get_static_model_cache()
{
    static std::map<std::size_t, BASE_T> instance;
    return instance;
}

template <typename BASE_MODEL>
class model_meta_cache
{
  private:
    model_meta_cache() {};
    ~model_meta_cache() {};
    model_meta_cache(const model_meta_cache &);
    model_meta_cache &operator=(const model_meta_cache &);

  public:
    struct data_cache_t
    {
        BASE_MODEL data;
        unsigned int exptime = 0;
    };

  public:
    void save(std::size_t hashid, const BASE_MODEL &data_list, int expnum = 0, bool cover_data = false)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        struct data_cache_t temp;
        temp.data = data_list;
        if (expnum != 0)
        {
            temp.exptime = orm_timeid() + expnum;
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

    void save(std::size_t hashid, BASE_MODEL &&data_list, int expnum = 0, bool cover_data = false)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        struct data_cache_t temp;
        temp.data = std::move(data_list);
        if (expnum != 0)
        {
            temp.exptime = orm_timeid() + expnum;
        }
        else
        {
            temp.exptime = 0;
        }
        std::unique_lock<std::mutex> lock(editlock);
        auto [_, success] = obj.insert({hashid, std::move(temp)});
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

    bool remove(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
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
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = orm_timeid();
        std::unique_lock<std::mutex> lock(editlock);
        for (auto iter = obj.begin(); iter != obj.end();)
        {
            // exptime==0 表示永不过期，保留；已过期则删除
            if (iter->second.exptime != 0 && iter->second.exptime < nowtime)
            {
                iter = obj.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    void clear()
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        std::unique_lock<std::mutex> lock(editlock);
        obj.clear();
    }

    int check(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = orm_timeid();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime == 0)
            {
                return 0;
            }
            return ((int)(iter->second.exptime - nowtime));
        }
        return -1;
    }

    int update(std::size_t hashid, int exptime = 0)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = orm_timeid() + exptime;
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

    const BASE_MODEL get(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = orm_timeid();
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
                throw std::runtime_error("This cache is vector loswer expected time");
            }
        }
        throw std::runtime_error("Not in this vector cache");
    }

    static model_meta_cache &getinstance()
    {
        static model_meta_cache instance;
        return instance;
    }

  public:
    std::mutex editlock;
};

}// namespace orm
#endif
