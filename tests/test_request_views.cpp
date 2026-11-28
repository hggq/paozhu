#include "test_framework.hpp"

#include "func.h"
#include "httppeer.h"
#include "rpc_parse.h"
#include "serverconfig.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace
{
bool points_into(std::string_view view, const std::string &owner)
{
    const auto begin = reinterpret_cast<std::uintptr_t>(owner.data());
    const auto end   = begin + owner.size();
    const auto ptr   = reinterpret_cast<std::uintptr_t>(view.data());
    return ptr >= begin && ptr <= end && view.size() <= end - ptr;
}

void require_header_views(const http::httppeer &peer)
{
    for (const auto &[name, value] : peer.header)
    {
        REQUIRE(points_into(name, peer.rawheader));
        REQUIRE(points_into(value, peer.rawheader));
    }
}

void require_path_views(const std::string &urlpath, const std::vector<std::string_view> &pathinfos)
{
    for (const auto &segment : pathinfos)
    {
        REQUIRE(points_into(segment, urlpath));
    }
}

void require_urlpath(std::string_view raw, const std::string &expected)
{
    std::string urlpath;
    std::vector<std::string_view> pathinfos;
    REQUIRE(http::build_urlpath(raw, urlpath, pathinfos));
    REQUIRE_EQ(urlpath, expected);
    require_path_views(urlpath, pathinfos);

    std::vector<std::string_view> expected_segments;
    for (std::size_t pos = 1; pos < expected.size();)
    {
        const auto slash = expected.find('/', pos);
        expected_segments.emplace_back(expected.data() + pos, (slash == std::string::npos ? expected.size() : slash) - pos);
        if (slash == std::string::npos)
        {
            break;
        }
        pos = slash + 1;
    }
    REQUIRE_EQ(pathinfos.size(), expected_segments.size());
    for (std::size_t i = 0; i < pathinfos.size() && i < expected_segments.size(); ++i)
    {
        REQUIRE_EQ(pathinfos[i], expected_segments[i]);
    }
}
} // namespace

TEST_CASE(request_headers_http1_append_and_views)
{
    http::httppeer peer;
    peer.begin_request_headers();
    peer.add_request_header("Host", "example.com");
    peer.add_request_header("X-MiXeD", "first");

    // The public header map is published only after the complete block is parsed.
    REQUIRE(peer.header.empty());

    // Duplicate names keep the last value.
    peer.add_request_header("X-MIXED", "last");
    peer.add_request_header("Cookie", "sid=abc; theme=dark");
    peer.finalize_request_headers();

    REQUIRE_EQ(peer.get_header("host"), "example.com");
    REQUIRE_EQ(peer.get_header("X-MIXED"), "last");
    REQUIRE(peer.header.find("cookie") == peer.header.end());
    REQUIRE_EQ(peer.header.size(), std::size_t{2});
    REQUIRE(peer.rawheader.find("host: example.com\r\n") != std::string::npos);
    REQUIRE(peer.rawheader.find("cookie: sid=abc; theme=dark\r\n") != std::string::npos);
    REQUIRE(peer.rawheader.find("x-mixed: first\r\n") != std::string::npos);
    REQUIRE(peer.rawheader.find("x-mixed: last\r\n") != std::string::npos);
    require_header_views(peer);
}

TEST_CASE(request_headers_finalize_preserves_h2_fields_and_values)
{
    http::httppeer peer;
    peer.begin_request_headers();
    peer.add_request_header(":method", "GET");
    peer.add_request_header(":path", "/a?x=1");
    peer.add_request_header(":authority", "example.com");
    peer.add_request_header("X-Empty", "");
    peer.add_request_header("X-Spaced", "  value: tail ");
    peer.add_request_header("Cookie", "a=1");
    REQUIRE(peer.header.empty());

    peer.finalize_request_headers();

    REQUIRE_EQ(peer.get_header(":method"), "GET");
    REQUIRE_EQ(peer.get_header(":path"), "/a?x=1");
    REQUIRE_EQ(peer.get_header(":authority"), "example.com");
    REQUIRE_EQ(peer.get_header("x-empty"), "");
    REQUIRE_EQ(peer.get_header("x-spaced"), "  value: tail ");
    REQUIRE_EQ(peer.header.size(), std::size_t{5});
    REQUIRE(peer.header.find("cookie") == peer.header.end());
    require_header_views(peer);
}

TEST_CASE(request_cors_origin_before_host_is_finalized)
{
    auto &config = http::getserversysconfig();
    config.sitehostinfos.clear();
    config.sitehostinfos.emplace_back();
    auto &site = config.sitehostinfos.back();
    site.cors_origin_allowed.insert("http://example.com");

    http::httppeer peer;
    peer.httpv = 1;
    peer.begin_request_headers();
    peer.cors_origin_process("http://example.com");
    peer.add_request_header("Origin", "http://example.com");
    peer.add_request_header("Host", "example.com");
    peer.host = "example.com";
    peer.finalize_request_headers();

    REQUIRE_EQ(peer.send_header["Access-Control-Allow-Origin"], "http://example.com");
    REQUIRE_EQ(peer.send_header["Vary"], "Origin");
}

TEST_CASE(request_headers_clear_and_reuse)
{
    http::httppeer peer;
    peer.begin_request_headers();
    peer.add_request_header("X-Old", std::string(4096, 'o'));
    peer.finalize_request_headers();
    REQUIRE_EQ(peer.header.size(), std::size_t{1});
    require_header_views(peer);

    peer.clear();
    REQUIRE(peer.header.empty());
    REQUIRE(peer.rawheader.empty());
    REQUIRE(peer.pathinfos.empty());

    // Never dereference views saved from the old request: clear() invalidates them.
    // Rebuild and verify every new view belongs to the new rawheader.
    peer.begin_request_headers();
    peer.add_request_header("X-New", "new-value");
    peer.finalize_request_headers();
    REQUIRE_EQ(peer.get_header("x-new"), "new-value");
    REQUIRE(peer.get_header("x-old").empty());
    REQUIRE_EQ(peer.header.size(), std::size_t{1});
    require_header_views(peer);

    std::string urlpath;
    REQUIRE(http::build_urlpath("/new/path", urlpath, peer.pathinfos));
    REQUIRE_EQ(urlpath, "/new/path");
    require_path_views(urlpath, peer.pathinfos);
}

TEST_CASE(urlpath_decode_normalize_and_views)
{
    require_urlpath("/a/%41%7e/%2e/b//c/", "/a/A~/b/c");
    require_urlpath("/a/%2e%2e/b", "/b");
    require_urlpath("/%2E/%2e%2e/keep", "/keep");
    require_urlpath("a//b/./c", "/a/b/c");
    require_urlpath("", "/");
    require_urlpath("/", "/");
    require_urlpath("/a+b/%2B", "/a+b/+");
    require_urlpath("/bad%ZZ/%/%2", "/bad%ZZ/%/%2");
    require_urlpath("/a/../../b", "/b");
}

TEST_CASE(urlpath_rejects_unsafe_segments)
{
    const std::vector<std::string> unsafe = {
        "/%00", "/%2F", "/%2f", "/%5C", "/%5c", "/%0A", "/%0a", "/%7F", "/%7f"};
    for (const auto &raw : unsafe)
    {
        std::string urlpath = "unchanged";
        std::vector<std::string_view> pathinfos;
        REQUIRE(!http::build_urlpath(raw, urlpath, pathinfos));
    }

    for (unsigned char c = 0; c < 0x20; ++c)
    {
        std::string raw = "/safe";
        raw.push_back(static_cast<char>(c));
        raw += "/tail";
        std::string urlpath;
        std::vector<std::string_view> pathinfos;
        REQUIRE(!http::build_urlpath(raw, urlpath, pathinfos));
    }
    for (unsigned char c : {0x5c, 0x7f})
    {
        std::string raw = "/safe";
        raw.push_back(static_cast<char>(c));
        raw += "/tail";
        std::string urlpath;
        std::vector<std::string_view> pathinfos;
        REQUIRE(!http::build_urlpath(raw, urlpath, pathinfos));
    }

    std::string urlpath;
    std::vector<std::string_view> pathinfos;
    REQUIRE(!http::build_urlpath("/" + std::string(256, 'x'), urlpath, pathinfos));

    // The limit is measured in decoded bytes; 255 remains valid.
    REQUIRE(http::build_urlpath("/" + std::string(255, 'x'), urlpath, pathinfos));
    REQUIRE_EQ(urlpath.size(), std::size_t{256});
    REQUIRE_EQ(pathinfos.size(), std::size_t{1});
    REQUIRE_EQ(pathinfos[0].size(), std::size_t{255});
    require_path_views(urlpath, pathinfos);
}

TEST_CASE(urlpath_deep_parent_segments_do_not_escape_root)
{
    require_urlpath("/../../../../etc/passwd", "/etc/passwd");
    require_urlpath("/a/b/../../../../../../safe", "/safe");
    require_urlpath("/%2e%2e/%2e%2e/%2e%2e/root", "/root");
}

TEST_CASE(request_headers_many_appends_keep_finalized_views)
{
    http::httppeer peer;
    constexpr std::size_t count = 2048;
    peer.begin_request_headers();
    for (std::size_t i = 0; i < count; ++i)
    {
        peer.add_request_header("X-Header-" + std::to_string(i), "value-" + std::to_string(i));
    }
    peer.add_request_header("Cookie", "bulk=1");
    peer.finalize_request_headers();

    REQUIRE_EQ(peer.header.size(), count);
    REQUIRE(peer.header.find("cookie") == peer.header.end());
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto name = "x-header-" + std::to_string(i);
        REQUIRE_EQ(peer.get_header(name), "value-" + std::to_string(i));
    }
    require_header_views(peer);
}

TEST_CASE(querystring_view_tracks_url_and_clear_invalidates_it)
{
    http::httppeer peer;
    peer.url = "/a/%75ser/../info?x=%41&empty=";
    peer.querystring = std::string_view(peer.url).substr(peer.url.find('?') + 1);

    REQUIRE_EQ(peer.querystring, "x=%41&empty=");
    REQUIRE(points_into(peer.querystring, peer.url));
    REQUIRE_EQ(peer.querystring.data(), peer.url.data() + peer.url.find('?') + 1);

    peer.clear();
    REQUIRE(peer.querystring.empty());
    REQUIRE(peer.querystring.data() == nullptr);
    REQUIRE(peer.url.empty());

    peer.url = "/next";
    REQUIRE(peer.querystring.empty());
}

TEST_CASE(rpc_query_view_tracks_owned_url_across_request_line_chunks)
{
    auto peer = std::make_shared<http::httppeer>();
    http::rpc_parse parser;
    parser.peer = peer;

    const std::string first  = "rpc /a/%75se";
    const std::string second = "r/../info?x=%41&empty=\n";
    parser.process(reinterpret_cast<const unsigned char *>(first.data()), first.size());
    REQUIRE(!parser.iserror);
    parser.process_append(reinterpret_cast<const unsigned char *>(second.data()), second.size());

    REQUIRE(!parser.iserror);
    REQUIRE_EQ(peer->url, "/a/%75ser/../info?x=%41&empty=");
    REQUIRE_EQ(peer->urlpath, "/a/info");
    REQUIRE_EQ(peer->querystring, "x=%41&empty=");
    REQUIRE(points_into(peer->querystring, peer->url));
    REQUIRE_EQ(peer->querystring.data(), peer->url.data() + peer->url.find('?') + 1);
    REQUIRE_EQ(peer->get["x"].to_string(), "%41");
    require_path_views(peer->urlpath, peer->pathinfos);

    parser.reset();
    parser.peer = peer;
    const std::string no_query = "rpc /next\n";
    parser.process(reinterpret_cast<const unsigned char *>(no_query.data()), no_query.size());
    REQUIRE(!parser.iserror);
    REQUIRE_EQ(peer->url, "/next");
    REQUIRE(peer->querystring.empty());
}

TEST_MAIN()
