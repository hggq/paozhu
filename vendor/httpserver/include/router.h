#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H

#include <map>
#include <memory>
#include <string>

#include <asio/awaitable.hpp>

#include "httppeer.h"

namespace http
{

struct co_route_result
{
    bool matched   = false;// a coroutine handler matched peer->sendfilename
    bool need_sync = true; // request still needs the sync thread pool handling
};

class router
{
  public:
    // Match peer->pathinfos against the registered method tables (alias domain first),
    // fill restful url params into peer->get and return the method path (404/home fallback).
    std::string resolve(std::shared_ptr<httppeer> peer);

    // Sync routing used by the thread pool: site pre hooks -> handler chain -> site after hooks.
    void dispatch(std::shared_ptr<httppeer> peer);

    // Coroutine routing used by server.cpp against _co_http_regmethod_table.
    asio::awaitable<co_route_result> co_dispatch(std::shared_ptr<httppeer> peer);

  private:
    std::map<std::string, std::map<std::string, regmethold_t>>::iterator find_domain_methods(std::shared_ptr<httppeer> &peer);

    // Run the pre/regfun chain within one method table, returns the leftover method name
    // when the chain redirected to a method not present in this table (empty otherwise).
    std::string run_chain(std::map<std::string, regmethold_t> &method_table,
                          std::map<std::string, regmethold_t>::iterator method_iter,
                          std::string regmethold_path,
                          std::shared_ptr<httppeer> &peer);
};

router &get_router();

}// namespace http
#endif
