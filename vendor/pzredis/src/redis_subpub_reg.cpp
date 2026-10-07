/*
 * Redis pubsub 注册表 singleton
 */
#include "redis_subpub_reg.h"

#ifdef ENABLE_REDIS_CLIENT

namespace pz
{
namespace redis
{

REDIS_SUBPUB_REG &get_redis_subpub_reg()
{
    static REDIS_SUBPUB_REG instance;
    return instance;
}

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS_CLIENT
