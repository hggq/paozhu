#ifndef ORM_CACHE_HPP
#define ORM_CACHE_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <map>
#include <memory>
#include <thread>
#include <ctime>
#include <vector>

namespace orm
{

inline unsigned int orm_timeid()
{
    return static_cast<unsigned int>(std::time(nullptr));
}

template <typename BASE_T>
std::map<std::size_t, BASE_T> &get_static_model_cache()
{
    // 清理线程是 detach 的，可能在 main 返回后再醒一次；缓存故意不析构，
    // 以免它在静态析构之后访问已销毁的容器
    static std::map<std::size_t, BASE_T> *instance = new std::map<std::size_t, BASE_T>();
    return *instance;
}

// ===========================================================================
// 全局缓存清理守护线程
//
// 每个 model_meta_cache<T> 单例在首次 getinstance() 时自注册到
// orm_cache_registry，后台线程每 ORM_CACHE_SCAN_INTERVAL 秒调一次
// remove_exptime()，业务零配置。
// ===========================================================================

#ifndef ORM_CACHE_SCAN_INTERVAL
#define ORM_CACHE_SCAN_INTERVAL 30   // 秒，可编译期 -D 覆盖
#endif

#ifndef ORM_CACHE_DEFAULT_TTL
#define ORM_CACHE_DEFAULT_TTL 120    // 秒，可编译期 -D 覆盖
#endif

#ifndef ORM_CACHE_MAX_TTL
#define ORM_CACHE_MAX_TTL 3600       // 秒，可编译期 -D 覆盖
#endif

// 到期时刻只在这一处算：过期归框架决定，业务传的秒数只是"想短一点"的请求，
// 不给档走默认档、给了档钳到上界，所以不再有任何条目是清不掉的永久条目。
inline unsigned int orm_cache_deadline(int expnum)
{
    if (expnum <= 0)
    {
        return orm_timeid() + ORM_CACHE_DEFAULT_TTL;
    }
    if (expnum > ORM_CACHE_MAX_TTL)
    {
        expnum = ORM_CACHE_MAX_TTL;
    }
    return orm_timeid() + static_cast<unsigned int>(expnum);
}

// 类型擦除的 registry：每个 model_meta_cache<T> 注册一个 lambda 进去
// 下面这几个守护线程会用到的单例同样不析构，理由同 get_static_model_cache()
inline std::vector<std::function<void()>> &orm_cache_registry()
{
    static std::vector<std::function<void()>> *v = new std::vector<std::function<void()>>();
    return *v;
}

inline std::mutex &orm_cache_registry_mutex()
{
    static std::mutex *m = new std::mutex();
    return *m;
}

inline std::atomic<bool> &orm_cache_daemon_running()
{
    static std::atomic<bool> *running = new std::atomic<bool>{false};
    return *running;
}

inline std::condition_variable &orm_cache_daemon_cv()
{
    static std::condition_variable *cv = new std::condition_variable();
    return *cv;
}

// 启动守护线程；可多次调（幂等，第二次 no-op）
inline void orm_cache_start_daemon()
{
    bool expected = false;
    if (!orm_cache_daemon_running().compare_exchange_strong(expected, true))
    {
        return;   // 已在跑
    }
    std::thread([]() {
        std::mutex &m = orm_cache_registry_mutex();
        std::condition_variable &cv = orm_cache_daemon_cv();
        while (orm_cache_daemon_running().load())
        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait_for(lk, std::chrono::seconds(ORM_CACHE_SCAN_INTERVAL), []() {
                return !orm_cache_daemon_running().load();
            });
            if (!orm_cache_daemon_running().load()) break;

            // 拷贝一份 registry 再跑，避免持锁期间调 remove_exptime（它自己要加 editlock）
            auto tasks = orm_cache_registry();
            lk.unlock();
            for (auto &fn : tasks)
            {
                try { fn(); } catch (...) {}   // 单个缓存异常不影响其他
            }
        }
    }).detach();
}

inline void orm_cache_stop_daemon()
{
    orm_cache_daemon_running().store(false);
    orm_cache_daemon_cv().notify_all();
}

template <typename BASE_MODEL>
class model_meta_cache
{
  private:
    model_meta_cache()
    {
        // 首次构造 → 注册到全局 registry + 确保守护线程已启动
        std::lock_guard<std::mutex> lk(orm_cache_registry_mutex());
        static bool registered = false;
        if (!registered)
        {
            orm_cache_registry().push_back([this]() { remove_exptime(); });
            registered = true;
        }
        orm_cache_start_daemon();   // 幂等
    }
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
        temp.data    = data_list;
        temp.exptime = orm_cache_deadline(expnum);
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
        temp.data    = std::move(data_list);
        temp.exptime = orm_cache_deadline(expnum);
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter == obj.end())
        {
            obj.emplace(hashid, std::move(temp));
        }
        else if (cover_data)
        {
            iter->second = std::move(temp);
        }
        else
        {
            iter->second.exptime = temp.exptime;
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

    // 清所有已过期的键；到期时刻一律出自 orm_cache_deadline()，没有豁免的条目
    void remove_exptime()
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = orm_timeid();
        std::unique_lock<std::mutex> lock(editlock);
        for (auto iter = obj.begin(); iter != obj.end();)
        {
            if (iter->second.exptime < nowtime)
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

    // 剩余秒数；-1 = 键不存在。到期时刻只可能来自 orm_cache_deadline()，
    // 已过期但还没被扫走的键会给出负数，调用方按 miss 处理即可。
    int check(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = orm_timeid();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            return ((int)(iter->second.exptime - nowtime));
        }
        return -1;
    }

    // 给已存在的键换一次到期时刻；-1 = 键不存在，1 = 已刷新
    int update(std::size_t hashid, int exptime = 0)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = orm_cache_deadline(exptime);
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            iter->second.exptime = nowtime;
            return 1;
        }
        return -1;
    }

    // 首选：不 throw，miss/过期返回 std::nullopt
    std::optional<BASE_MODEL> try_get(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = orm_timeid();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime >= nowtime)
            {
                return iter->second.data;
            }
            obj.erase(iter++);   // 过期顺手清
        }
        return std::nullopt;
    }

    // 旧接口：保留 throw 语义；内部复用 try_get
    // Deprecated → 新代码用 try_get()
    const BASE_MODEL get(std::size_t hashid)
    {
        auto hit = try_get(hashid);
        if (!hit)
        {
            throw std::runtime_error("Not in this vector cache");
        }
        return *hit;
    }

    static model_meta_cache &getinstance()
    {
        // 首次调用触发构造函数 → 自注册 + 启守护线程；实例不析构，理由同上
        static model_meta_cache *instance = new model_meta_cache;
        return *instance;
    }

  public:
    std::mutex editlock;
};

}// namespace orm
#endif
