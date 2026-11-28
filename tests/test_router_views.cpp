#include <cstdlib>
#include <iostream>
#include <new>
#include <string>

#include "router.h"
#include "func.h"
#include "serverconfig.h"

namespace
{
std::size_t allocations = 0;
bool count_allocations = false;
int failures = 0;

void check(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "FAIL " << message << '\n';
        ++failures;
    }
}

unsigned int add_route(const char *path, std::vector<std::string> names = {})
{
    auto handler = [](std::shared_ptr<http::httppeer>) -> std::string { return ""; };
    http::reg_methold_mid_t entry;
    entry.reg_kind = http::fn_kind::sync;
    entry.regfun = handler;
    unsigned int idx = http::reg_add("", path, std::move(entry));
    if (!names.empty())
        http::reg_urlpath(idx, std::move(names));
    return idx;
}
}

void *operator new(std::size_t size)
{
    if (count_allocations)
        ++allocations;
    if (void *ptr = std::malloc(size == 0 ? 1 : size))
        return ptr;
    throw std::bad_alloc();
}

void *operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void *ptr) noexcept { std::free(ptr); }
void operator delete[](void *ptr) noexcept { std::free(ptr); }
void operator delete(void *ptr, std::size_t) noexcept { std::free(ptr); }
void operator delete[](void *ptr, std::size_t) noexcept { std::free(ptr); }

int main()
{
    auto &config = http::getserversysconfig();
    config.configfile = "conf/server.conf";
    if (!config.loadserverglobalconfig())
    {
        std::cerr << "FAIL load server config\n";
        return 1;
    }
    http::router_init_sites();
    const unsigned int exact = add_route("/exact");
    const unsigned int about = add_route("/about");
    const unsigned int other = add_route("/about.php");
    const unsigned int prefix = add_route("/user/info", {"", "", "userid"});
    const unsigned int root = add_route("/");

    auto &router = http::get_router();
    check(router.lookup_key_in_slots("/exact", 0) == static_cast<int>(exact), "exact lookup");
    check(router.lookup_stripped_key_in_slots("/about.html", 0) == static_cast<int>(about),
          "extension stripping uses exact stem");
    check(router.lookup_stripped_key_in_slots("/about.css", 0) == static_cast<int>(about),
          "extension stripping matches registered stem");
    check(router.lookup_stripped_key_in_slots("/missing.css", 0) == -1, "unknown stripped route");
    check(router.lookup_key_in_slots("/", 0) == static_cast<int>(root), "root route remains reachable");

    auto peer = std::make_shared<http::httppeer>();
    peer->url = "/user/info/42?raw=%2F";
    check(http::build_urlpath("/user/info/42", peer->urlpath, peer->pathinfos), "build test path");
    peer->get["userid"] = "";
    std::string matched;
    matched.reserve(128);
    allocations = 0;
    count_allocations = true;
    int idx = router.resolve_idx(peer, matched, -1, true);
    count_allocations = false;
    check(idx == static_cast<int>(prefix), "prefix lookup");
    check(peer->get["userid"].to_string() == "42", "prefix argument view");
    check(peer->url == "/user/info/42?raw=%2F", "lookup preserves raw target");
    check(peer->urlpath == "/user/info/42", "lookup preserves normalized path");
    check(allocations == 0, "prefix lookup allocates no temporary strings");

    matched.clear();
    allocations = 0;
    count_allocations = true;
    idx = router.resolve_idx(peer, matched, static_cast<int>(about));
    count_allocations = false;
    check(idx == static_cast<int>(about), "prefetched index wins");
    check(matched == "/about", "prefetched index uses registered path");
    check(allocations == 0, "prefetched resolution is allocation free");
    check(router.lookup_key_in_slots("/about.php", 0) == static_cast<int>(other),
          "exact route still wins over stripped stem");

    auto saved_site = config.sitehostinfos[0];
    auto &site     = config.sitehostinfos[0];
    site.isrewrite          = true;
    site.rewrite404         = 2;
    site.rewrite_404_action = "fallback";
    site.action_404_lists.clear();
    peer->host_index = 0;
    peer->sitepath   = "/tmp/paozhu-router-view-no-such-directory";
    peer->url        = "/a/%252F?x=1";
    check(http::build_urlpath("/a/%252F", peer->urlpath, peer->pathinfos), "build encoded percent path");
    peer->sendfiletype = 0;
    check(peer->get_fileinfo() == 3, "dynamic fallback rewrite selected");
    check(peer->url == "/a/%252F?x=1", "fallback preserves raw target");
    check(peer->urlpath == "/fallback/a/%2F", "fallback does not decode normalized path twice");
    check(peer->pathinfos.size() == 3, "fallback rebuilds path views");
    if (peer->pathinfos.size() == 3)
    {
        check(peer->pathinfos[0] == "fallback" && peer->pathinfos[1] == "a" &&
                  peer->pathinfos[2] == "%2F",
              "fallback path views retain literal percent encoding");
    }
    config.sitehostinfos[0] = saved_site;

    std::cout << "router view tests " << (failures == 0 ? "PASS" : "FAIL") << '\n';
    return failures == 0 ? 0 : 1;
}
