#include "test_framework.hpp"

#include "httppeer.h"
#include "http2_parse.h"

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

void parse_query(http::http2parse &parser, const std::shared_ptr<http::httppeer> &peer,
                 const std::string &target)
{
    peer->stream_id = 3;
    parser.path_process(":path", target, peer);
}

void parse_form(http::http2parse &parser, http::HTTP2_POST_DATA_T &post,
                const std::string &body)
{
    post.peer             = std::make_shared<http::httppeer>();
    post.peer->stream_id  = 5;
    post.peer->rawcontent = body;
    post.stream_id        = 5;
    parser.post_www_form_urlencoded(post);
}
} // namespace

TEST_CASE(http2_query_views_and_decoding)
{
    const std::string query = "a=hello+world&b=%2F&flag&arr%5Bx%5D=y&eq=b=c&&empty=&skip==z";
    const std::string target = "/q?" + query;
    http::http2parse parser;
    auto peer = std::make_shared<http::httppeer>();

    parse_query(parser, peer, target);

    REQUIRE_EQ(parser.error, 0u);
    REQUIRE_EQ(peer->url, target);
    REQUIRE_EQ(peer->querystring, query);
    REQUIRE(points_into(peer->querystring, peer->url));
    REQUIRE_EQ(peer->urlpath, "/q");
    REQUIRE_EQ(peer->get["a"].to_string(), "hello world");
    REQUIRE_EQ(peer->get["b"].to_string(), "/");
    REQUIRE_EQ(peer->get["flag"].to_string(), "");
    REQUIRE_EQ(peer->get["arr"]["x"].to_string(), "y");
    REQUIRE_EQ(peer->get["eq"].to_string(), "b=c");
    REQUIRE_EQ(peer->get["empty"].to_string(), "");
    REQUIRE_EQ(peer->get["skip"].to_string(), "z");
}

TEST_CASE(http2_query_empty_and_repeated_separators)
{
    const std::vector<std::string> targets = {"/q", "/q?", "/q?&", "/q?&&", "/q?a=1&", "/q?a===&b==c=d"};
    for (const auto &target : targets)
    {
        http::http2parse parser;
        auto peer = std::make_shared<http::httppeer>();
        parse_query(parser, peer, target);
        REQUIRE_EQ(parser.error, 0u);
        REQUIRE_EQ(peer->url, target);
        const auto query_pos = target.find('?');
        const std::string expected_query = query_pos == std::string::npos
                                               ? ""
                                               : target.substr(query_pos + 1);
        REQUIRE_EQ(peer->querystring, expected_query);
        if (!expected_query.empty())
        {
            REQUIRE(points_into(peer->querystring, peer->url));
        }
    }

    http::http2parse parser;
    auto peer = std::make_shared<http::httppeer>();
    parse_query(parser, peer, "/q?a===&b==c=d");
    REQUIRE_EQ(peer->get["a"].to_string(), "");
    REQUIRE_EQ(peer->get["b"].to_string(), "c=d");
}

TEST_CASE(http2_query_key_limits_and_error_codes)
{
    const std::string key48(48, 'A');
    const std::string key49(49, 'A');
    std::string encoded73;
    std::string encoded74;
    for (int i = 0; i < 24; ++i)
    {
        encoded73 += "%41";
        encoded74 += "%41";
    }
    encoded73 += "A";
    encoded74 += "AA";

    struct QueryCase
    {
        std::string target;
        unsigned int error;
    };
    const std::vector<QueryCase> cases = {
        {"/q?" + key48 + "=v", 0},
        {"/q?" + encoded73 + "=v", 0},
        {"/q?" + encoded74 + "=v", 40174},
        {"/q?" + encoded74 + "&x=1", 40174},
        {"/q?" + encoded74, 40174},
        {"/q?a=1&" + encoded74, 40174},
        {"/q?" + key49 + "&x=1", 40173},
        {"/q?" + key49 + "=v", 40175},
        {"/q?a=1&" + key49, 40176},
        {"/q?" + key49, 40177},
    };
    for (const auto &item : cases)
    {
        http::http2parse parser;
        auto peer = std::make_shared<http::httppeer>();
        parse_query(parser, peer, item.target);
        REQUIRE_EQ(parser.error, item.error);
    }
}

TEST_CASE(http2_urlencoded_form_decoding)
{
    const std::string body = "a=hello+world&b=%2F&flag&arr%5Bx%5D=y&eq=b=c&&empty=&skip==z";
    http::http2parse parser;
    http::HTTP2_POST_DATA_T post;

    parse_form(parser, post, body);

    REQUIRE_EQ(parser.error, 0u);
    REQUIRE_EQ(post.peer->post["a"].to_string(), "hello world");
    REQUIRE_EQ(post.peer->post["b"].to_string(), "/");
    REQUIRE_EQ(post.peer->post["flag"].to_string(), "");
    REQUIRE_EQ(post.peer->post["arr"]["x"].to_string(), "y");
    REQUIRE_EQ(post.peer->post["eq"].to_string(), "b=c");
    REQUIRE_EQ(post.peer->post["empty"].to_string(), "");
    REQUIRE_EQ(post.peer->post["skip"].to_string(), "=z");
}

TEST_CASE(http2_urlencoded_form_empty_and_repeated_separators)
{
    for (const std::string body : {"", "&", "&&", "a=1&", "a==&b==c=d"})
    {
        http::http2parse parser;
        http::HTTP2_POST_DATA_T post;
        parse_form(parser, post, body);
        REQUIRE_EQ(parser.error, 0u);
    }

    http::http2parse parser;
    http::HTTP2_POST_DATA_T post;
    parse_form(parser, post, "a==&b==c=d");
    REQUIRE_EQ(post.peer->post["a"].to_string(), "=");
    REQUIRE_EQ(post.peer->post["b"].to_string(), "=c=d");
}

TEST_CASE(http2_urlencoded_form_key_limits_and_error_codes)
{
    const std::string key72(72, 'A');
    const std::string key73(73, 'A');
    std::string encoded73;
    std::string encoded74;
    for (int i = 0; i < 24; ++i)
    {
        encoded73 += "%41";
        encoded74 += "%41";
    }
    encoded73 += "A";
    encoded74 += "AA";

    struct FormCase
    {
        std::string body;
        unsigned int error;
    };
    const std::vector<FormCase> cases = {
        {key72 + "=v", 0},
        {encoded73 + "=v", 0},
        {encoded74 + "=v", 40197},
        {encoded74 + "&x=1", 40197},
        {encoded74, 40197},
        {"a=1&" + encoded74, 40197},
        {key73 + "&x=1", 40196},
        {key73 + "=v", 40198},
        {"a=1&" + key73, 40199},
        {key73, 40200},
    };
    for (const auto &item : cases)
    {
        http::http2parse parser;
        http::HTTP2_POST_DATA_T post;
        parse_form(parser, post, item.body);
        REQUIRE_EQ(parser.error, item.error);
        if (item.error != 0)
        {
            REQUIRE(!parser.error_is_conn);
            REQUIRE_EQ(parser.error_stream_id, 5u);
            REQUIRE_EQ(parser.h2_error_code, 1u);
        }
    }
}

TEST_MAIN()
