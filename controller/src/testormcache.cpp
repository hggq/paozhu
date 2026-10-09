#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testormcache.h"
namespace http
{

//@urlpath(null,testormcache)
std::string testormcache(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " test orm cache ";
    auto articles = orm::cms::Article();
    int aid       = 5;

    articles.where("isopen", 1).where("aid", aid).limit(1).use_cache().fetch_one();
    client << "<br />title:" << articles.getTitle();
    if (articles.isuse_cache())
    {
        articles.save_cache(60);//cache 60 second
        client << "<p>  this visit to cache </p>";
    }
    return "";
}

//@urlpath(null,testormcacheb)
std::string testormcacheb(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " test orm cache B style ";
    auto articles = orm::cms::Article();
    int aid       = 5;

    articles.where("isopen", 1).where("aid", aid).limit(1).use_cache(60).fetch_one();
    client << "<br />title:" << articles.getTitle();
    if (articles.isuse_cache(true))
    {
        //is has cache data
        client << "<p>  this visit to cache </p>";
    }
    return "";
}

//@urlpath(null,testormcachec)
std::string testormcachec(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " test orm cache fetchRow style ";
    auto articles = orm::cms::Article();
    int aid       = 5;

    auto [colnames, colnamemaps, vallists] = articles.where("isopen", 1).where("aid", aid).limit(1).use_cache(60).fetch_row();

    client << "<br />Record size:" << std::to_string(vallists.size());
    client << "<br />Colname size:" << std::to_string(colnames.size());

    if (vallists.size() > 0)
    {
        if (vallists[0].size() > 4)
        {
            client << "<br />title:" << vallists[0][4];
        }
    }
    for (std::size_t i = 0; i < vallists.size(); i++)
    {
        for (std::size_t j = 0; j < colnames.size(); j++)
        {
            if (j < vallists[i].size())
            {
                client << "<p>" << colnames[j] << ":" << vallists[i][j] << "</p>";
            }
            if (j > 3)
            {
                break;
            }
        }
    }

    if (articles.isuse_cache(true))
    {
        //is has cache data
        client << "<p>  this visit to cache </p>";
    }
    return "";
}

// ===========================================================================
// testormcache_d — 具名 key 缓存（业务自己传 key，稳定、可精确失效）
//
//   推荐路径（新）：
//     model_meta_cache<T>::getinstance().try_get(hash)  ← orm_cache.hpp 源头
//     不 throw，miss/过期返回 std::nullopt
//
//   对比老 use_cache(sqlstring) 路径：
//     - key 稳定（WHERE 字面量不影响）
//     - 失效精确（按具名 key 删，不是靠 sqlstring 哈希碰巧命中）
//     - 缓存命中 / 空结果 / DB 错误可干净区分（optional 语义）
// ===========================================================================

//@urlpath(null,testormcache_d/:aid)
std::string testormcache_d(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    long long aid    = peer->get["aid"].to_int();

    // --- 1. 构造稳定具名 key（业务语义，不是 sqlstring）---
    std::string key  = "article:by_id:" + std::to_string(aid);
    std::size_t hash = std::hash<std::string>{}(key);

    // --- 2. 先按具名 key 查缓存（orm_cache.hpp 源头 try_get，不依赖 opsql 生成）---
    auto &record_cache = orm::model_meta_cache<std::vector<orm::cms::article_info::meta>>::getinstance();
    auto cached = record_cache.try_get(hash);   // ← 源头 API，any opsql 都能用

    if (cached)
    {
        client << "<b>HIT</b> cache key=<code>" << key << "</code>"
               << "<br />title:" << (*cached)[0].title;
        return "";
    }

    // --- 3. miss：走 DB ---
    orm::cms::Article u;
    u.where("aid", orm::wq::eq, aid).limit(1).fetch_one();

    if (u.iserror)
    {
        client << "DB error: " << u.error_msg;
        return "";
    }
    if (u.record.empty())
    {
        client << "no such article aid=" << aid;
        return "";
    }

    // --- 4. 存缓存（具名 key）---
    u.save_cache(key, u.record, 60);   // string key → 内部 std::hash
    // 或传预计算 hash：
    // record_cache.save(hash, u.record, 60);

    client << "<b>MISS → DB → FILL</b> cache key=<code>" << key << "</code>"
           << "<br />title:" << u.getTitle();

    return "";
}

//@urlpath(null,testormcache_d_invalidate/:aid)
std::string testormcache_d_invalidate(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    long long aid     = peer->get["aid"].to_int();
    std::string key   = "article:by_id:" + std::to_string(aid);
    std::size_t hash  = std::hash<std::string>{}(key);

    // --- 写操作后，按具名 key 精确清缓存 ---
    // （ORM 不自动失效，业务自己知道清什么）
    orm::cms::Article().remove_cache(hash);
    client << "invalidated cache key=<code>" << key << "</code>";

    // 可选：整表清（粗粒度，写多的场景慎用）
    // orm::cms::Article().clear_cache();

    return "";
}

// ===========================================================================
// testormcache_e — orm_cache.hpp 源头 API 演示
//
//   本函数只依赖 orm_cache.hpp 里的 model_meta_cache<T> 模板，
//   不依赖任何 opsql 生成代码——rebuild 后始终可用。
//
//   源头 API（orm_cache.hpp）：
//     try_get(hash)        → std::optional<T>   首选，不 throw
//     get(hash)            → T                  旧接口，miss throw
//     save(hash, data, ttl)                      存
//     remove(hash)                                删一个
//     clear()                                     清全部
//     check(hash)          → int                 查剩余秒数 / -1 不存在 / 0 永久
//     update(hash, ttl)    → int                 刷新 TTL
// ===========================================================================

//@urlpath(null,testormcache_e/:aid)
std::string testormcache_e(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    long long aid    = peer->get["aid"].to_int();

    std::string key  = "article:by_id:" + std::to_string(aid);
    std::size_t hash = std::hash<std::string>{}(key);

    // 查剩余 TTL（check 源头 API）
    auto &record_cache = orm::model_meta_cache<std::vector<orm::cms::article_info::meta>>::getinstance();
    int ttl = record_cache.check(hash);

    if (ttl >= 0)
    {
        // 命中（ttl==0 永不过期，ttl>0 还有 ttl 秒）
        auto data = record_cache.try_get(hash);
        if (data && !(*data).empty())
        {
            client << "<b>SOURCE HIT</b> (orm_cache.hpp try_get)"
                   << "<br />ttl=" << (ttl == 0 ? "forever" : std::to_string(ttl) + "s")
                   << "<br />title:" << (*data)[0].title;
            return "";
        }
    }

    // miss：走 DB
    orm::cms::Article u;
    u.where("aid", orm::wq::eq, aid).limit(1).fetch_one();

    if (u.iserror)
    {
        client << "DB error: " << u.error_msg;
        return "";
    }
    if (u.record.empty())
    {
        client << "no such article aid=" << aid;
        return "";
    }

    // 存缓存（源头 save，hash 预计算）
    record_cache.save(hash, u.record, 60);

    client << "<b>MISS → DB → SOURCE SAVE</b> cache key=<code>" << key << "</code>"
           << "<br />title:" << u.getTitle();

    return "";
}

}// namespace http
