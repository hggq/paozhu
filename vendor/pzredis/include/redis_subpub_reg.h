#ifndef PZ_REDIS_SUBPUB_REG_H
#define PZ_REDIS_SUBPUB_REG_H

#ifdef ENABLE_REDIS_CLIENT
/*
 * Redis pubsub 业务订阅客户端注册表
 *  - key 随便起（不重就行）
 *  - value = 工厂 → shared_ptr<redis_subpub_client>
 *  - 业务侧通过 common/redis_regmethod.hpp 注入
 *  - server.cpp 启动时遍历注册表，每项 co_spawn 一个长期运行协程
 */
#include <functional>
#include <map>
#include <memory>

#include "redis_subpub.h"

namespace pz
{
namespace redis
{

typedef std::map<std::string, std::function<std::shared_ptr<redis_subpub_client>()>>
    REDIS_SUBPUB_REG;

REDIS_SUBPUB_REG &get_redis_subpub_reg();

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS_CLIENT

#endif
