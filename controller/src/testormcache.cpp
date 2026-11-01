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
    long long aid  = client.get["aid"].to_int();
    if (aid == 0)
    {
        aid = 13;
    }

    // use_cache() 不带参数即走框架默认档（ORM_CACHE_DEFAULT_TTL 秒）；fetch_one 读写都走 data 那一格
    articles.where("isopen", 1).where("aid", aid).use_cache().fetch_one();
    client << "<br />title:" << articles.getTitle();
    if (articles.is_hit_cache)
    {
        client << "<p>  this visit to cache </p>";
    }
    else if (articles.effect() > 0)
    {
        client << "<p>  read from db, saved with the framework default ttl </p>";
    }
    else
    {
        client << "<p>  no row, nothing saved </p>";
    }
    return "";
}

//@urlpath(null,testormcacheb)
std::string testormcacheb(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " test orm cache B style ";
    auto articles = orm::cms::Article();
    long long aid  = client.get["aid"].to_int();
    if (aid == 0)
    {
        aid = 13;
    }

    articles.where("isopen", 1).where("aid", aid).use_cache(60).fetch_one();
    client << "<br />title:" << articles.getTitle();
    // is_hit_cache 才是命中旗标；isuse_cache 看的是装填格子剩下的状态
    if (articles.is_hit_cache)
    {
        client << "<p>  this visit to cache </p>";
    }
    else if (articles.effect() > 0)
    {
        client << "<p>  read from db, saved 60 second </p>";
    }
    else
    {
        client << "<p>  no row, nothing saved </p>";
    }
    return "";
}

//@urlpath(null,testormcachec)
std::string testormcachec(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " test orm cache fetchRow style ";
    auto articles = orm::cms::Article();
    long long aid  = client.get["aid"].to_int();
    if (aid == 0)
    {
        aid = 13;
    }

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

    // fetch_row 不设 is_hit_cache：真正读库回写会把 exptime 归零，命中则留下 60
    if (vallists.empty())
    {
        client << "<p>  no row, nothing saved </p>";
    }
    else if (articles.isuse_cache(true))
    {
        client << "<p>  read from db, saved 60 second </p>";
    }
    else
    {
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

    if (cached && !cached->empty())
    {
        client << "<b>HIT</b> cache key=<code>" << key << "</code>"
               << "<br />title:" << (*cached)[0].title;
        return "";
    }

    // --- 3. miss：走 DB。具名 key 存的是 record，所以用 fetch()；
    //        fetch_one() 只填 data，record 一直是空的 ---
    orm::cms::Article u;
    u.where("aid", orm::wq::eq, aid).limit(1).fetch();

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
           << "<br />title:" << u.record[0].title;

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
//     save(hash, data, ttl, cover)   存；cover 为 true 才覆盖旧内容，否则只续期
//     remove(hash)                                删一个
//     clear()                                     清全部
//     check(hash)          → int                 查剩余秒数 / -1 不存在（到期未扫走时给负数）
//     update(hash, ttl)    → int                 刷新 TTL；-1 不存在，1 已刷新
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
        // 命中（ttl 是还剩几秒，0 = 正好到点）
        auto data = record_cache.try_get(hash);
        if (data && !(*data).empty())
        {
            client << "<b>SOURCE HIT</b> (orm_cache.hpp try_get)"
                   << "<br />ttl=" << ttl << "s"
                   << "<br />title:" << (*data)[0].title;
            return "";
        }
    }

    // miss：走 DB。具名 key 存的是 record，所以用 fetch()；fetch_one() 只填 data
    orm::cms::Article u;
    u.where("aid", orm::wq::eq, aid).limit(1).fetch();

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

    // 存缓存（源头 save，hash 预计算）；第四个参数才覆盖旧值，缺省时只续期
    record_cache.save(hash, u.record, 60, true);

    client << "<b>MISS → DB → SOURCE SAVE</b> cache key=<code>" << key << "</code>"
           << "<br />title:" << u.record[0].title;

    return "";
}

// ===========================================================================
// testormcache_f — 失败读不再钉缓存（文本 record 面写回闸门的在线演示）
//
//   被演示的改动：三模板里 fetch() / async_fetch() / fetch_append() /
//   async_fetch_append() 的写回从 if (iscache) 变成
//     if (iscache && read_is_clean)
//   其中 read_is_clean = select_conn->error_msg.empty() && !select_conn->isclose，
//   在连接归还池之前取样。
//
//   触发方式（不用拔网线，也不用让数据库出故障）：mysql 文本协议里单行的包超过
//   0xFFFFFF 时驱动显式报错并排空到行集终结包，同时把已经交付给回调的前几行原样
//   返回 —— 于是这次读「有行 + error_msg 非空」，正好是闸门要拦的形状：检测式
//   if (fetch_count == 0 && !error_msg.empty()) 因为 fetch_count>0 不成立，既不
//   latch iserror 也不早退，执行流一路走到写回点。
//     改前：这一格被半截 record 钉住 60s，之后同一条语句直接命中缓存 —— 返回同样
//           的半截行集、error_msg 为空，看不出是缓存答的。
//     改后：写回被跳过，格子保持挂挡（isuse_cache(true) 为 false），同一条语句下一
//           次读仍然落库。
//
//   依赖：[cms] 是 mysql；isopen=1 的行至少 2 条；服务端 max_allowed_packet 容得下
//   这一行（容不下时第一趟在检测式就早退，页面会打印"触发条件没成立"）。
//   pg / sqlite 没有这枚大包护栏，它们的闸门只能在真断流时看见。
//
//   Z 组是甲案的对照面：一条干净的 0 行读必须仍然进缓存，用来证明闸门没有把
//   "空结果也是答案"一起拦掉。
// ===========================================================================

//@urlpath(null,testormcache_f)
std::string testormcache_f(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " test orm cache writeback gate ";

    // record 面的剩余秒数只能从源头取；模型上的 check_cache() 看的是 meta 那一格
    auto &record_cache = orm::model_meta_cache<std::vector<orm::cms::article_info::meta>>::getinstance();

    // 先取结果集里最小的一条 aid：第一行给短串，第二行拿去撑爆包
    auto finder = orm::cms::Article();
    finder.where("isopen", 1);
    unsigned int total = finder.count();
    // 别名必须是真列名，否则列位置映射不上，聚合值进不了 record
    finder.select("MIN(aid) AS aid").fetch();
    long long minaid = finder.record.empty() ? 0 : finder.record[0].aid;

    // 这一行要 > 0xFFFFFF 才拆得出包，服务端容不下就先看这个数字
    auto masetting = orm::cms::Article();
    masetting.select("@@max_allowed_packet AS aid").limit(1).fetch();
    unsigned int maxpacket = masetting.record.empty() ? 0 : masetting.record[0].aid;
    client << "<br />@@max_allowed_packet:" << std::to_string(maxpacket);

    if (total < 2 || minaid <= 0)
    {
        client << "<p> 触发需要至少 2 条 isopen=1 的行（现在 count=" << std::to_string(total)
               << " minaid=" << std::to_string(minaid) << "），T1/T2 跳过 </p>";
    }
    else
    {
        std::string bigselect =
            "aid, IF(aid = " + std::to_string(minaid) + ", 'short', REPEAT('a', 17000000)) AS title";

        // --- T1：挂上缓存读一次，让第二行的包撞破 0xFFFFFF ---
        auto first = orm::cms::Article();
        first.select(bigselect).where("isopen", 1).asc("aid").limit(2).use_cache(60).fetch();
        std::size_t sqlhash = std::hash<std::string>{}(first.get_query());

        client << "<br />sql:" << first.get_query();
        client << "<br />T1 effect:" << std::to_string(first.effect());
        client << "<br />T1 record size:" << std::to_string(first.record.size());
        if (!first.record.empty())
        {
            client << "<br />T1 title:" << first.record[0].title;
        }
        client << "<br />T1 iserror:" << (first.iserror ? "true" : "false");
        client << "<br />T1 error_msg:" << first.error_msg;
        client << "<br />T1 is_hit_cache:" << (first.is_hit_cache ? "true" : "false");
        // 写回被拦下 ⇒ iscache 与 exptime 都留在挂挡值；写回跑过 ⇒ 两者一起归零
        client << "<br />T1 isuse_cache(true):" << (first.isuse_cache(true) ? "true" : "false");

        // --- 那一格现在到底有没有内容 ---
        client << "<br />key:" << std::to_string(sqlhash);
        client << "<br />check(hash):" << std::to_string(record_cache.check(sqlhash)) << " (-1 = 不存在)";
        auto probe   = orm::cms::Article();
        bool  pinned = probe.get_record_cache(sqlhash);
        client << "<br />get_record_cache:" << (pinned ? "true" : "false")
               << " size:" << std::to_string(probe.record.size());

        // --- T2：同一条语句再挂一次，看缓存答不答 ---
        auto second = orm::cms::Article();
        second.select(bigselect).where("isopen", 1).asc("aid").limit(2).use_cache(60).fetch();
        client << "<br />T2 effect:" << std::to_string(second.effect());
        client << "<br />T2 is_hit_cache:" << (second.is_hit_cache ? "true" : "false");
        client << "<br />T2 error_msg:" << second.error_msg;

        if (first.is_hit_cache)
        {
            client << "<p> 第一趟就命中：进程里还留着升级前钉住的那格，等它过期或重启实例再看，本轮读数不作数 </p>";
        }
        else if (first.iserror || first.error_msg.empty() || first.effect() == 0)
        {
            client << "<p> 触发条件没成立：第一趟一行都没读到，本臂不判任何东西 —— 看上面那行 "
                      "@@max_allowed_packet，它容不下这一行（单行 payload 要越过 0xFFFFFF）时服务端会直接回错误包 </p>";
        }
        else if (pinned || second.is_hit_cache)
        {
            client << "<p> 半截结果被钉进缓存了（改动前的行为）：T2 由缓存作答，它的 error_msg 是空的 </p>";
        }
        else
        {
            client << "<p> 闸门生效：带错的读没有写回，T2 仍然落库 </p>";
        }

        // 收尾清掉这一格，下一次访问仍从冷格开始
        orm::cms::Article().remove_cache(sqlhash);
    }

    // --- Z：甲案对照，一条干净的空结果读必须仍然进缓存 ---
    auto zero = orm::cms::Article();
    zero.where("aid", 999999999).use_cache(60).fetch();
    std::size_t zerohash = std::hash<std::string>{}(zero.get_query());
    client << "<br />Z sql:" << zero.get_query();
    client << "<br />Z effect:" << std::to_string(zero.effect());
    client << "<br />Z iserror:" << (zero.iserror ? "true" : "false");
    client << "<br />Z error_msg:" << zero.error_msg;
    client << "<br />Z check(hash):" << std::to_string(record_cache.check(zerohash));
    auto zeroprobe   = orm::cms::Article();
    bool zero_cached = zeroprobe.get_record_cache(zerohash);
    client << "<br />Z get_record_cache:" << (zero_cached ? "true" : "false")
           << " size:" << std::to_string(zeroprobe.record.size());

    // 再读一次：命中才证明空集合真的进了缓存并由缓存作答（文本命中不填 effect_num，所以不看 effect()）
    auto again = orm::cms::Article();
    again.where("aid", 999999999).use_cache(60).fetch();
    client << "<br />Z2 is_hit_cache:" << (again.is_hit_cache ? "true" : "false");

    if (zero.iserror || !zero.error_msg.empty())
    {
        client << "<p> Z 组这次读本身就没跑干净，对照不作数 </p>";
    }
    else if (zero_cached && zeroprobe.record.empty() && again.is_hit_cache)
    {
        client << "<p> 甲案仍然成立：干净的空结果进了缓存，并由缓存作答 </p>";
    }
    else
    {
        client << "<p> 甲案被拦掉了：干净的空结果没进缓存 —— 这是回归，不是预期 </p>";
    }
    orm::cms::Article().remove_cache(zerohash);
    return "";
}

}// namespace http
