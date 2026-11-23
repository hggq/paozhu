#include "httppeer.h"
#include "test_echoraw.h"
#include <fstream>

namespace http
{

//@urlpath(null,echoraw)
asio::awaitable<std::string> arena_echo(std::shared_ptr<httppeer> peer)
{
    // Echo the raw request body byte-for-byte (8gbit profile). raw_body covers
    // both the in-memory fast path and the >2MB spilled file; chunked framing is
    // decoded by the parser into rawcontent. An empty body echoes as 200 empty.
    peer->type("application/octet-stream");
    if (!peer->rawcontent.empty())
    {
        // 内存快路径（≤2MB）：正文就在 rawcontent，没有临时文件。
        peer->output = std::move(peer->rawcontent);
        co_return "";
    }
    // spill 路径（>2MB）：正文落在临时文件，读回后删掉。
    const std::string path = peer->files["rawcontent"]["tempfile"].to_string();
    if (!path.empty())
    {
        std::ifstream in(path, std::ios::binary);
        peer->output.assign((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
        in.close();
        std::remove(path.c_str());
    }
    co_return "";
}

//@urlpath(null,spec)
asio::awaitable<std::string> arena_spec(std::shared_ptr<httppeer> peer)
{
    peer->status(200);
    if (peer->method == static_cast<unsigned char>(HEAD_METHOD::HEAD) ||
        peer->method == static_cast<unsigned char>(HEAD_METHOD::GET))
    {
        peer->type("text/plain");
        peer->output = "spec ok";
    }
    else if (peer->method == static_cast<unsigned char>(HEAD_METHOD::POST))
    {
        peer->type("application/octet-stream");
        peer->output = std::move(peer->rawcontent);
    }
    else if (peer->method == static_cast<unsigned char>(HEAD_METHOD::OPTIONS))
    {
        peer->set_header("Allow", "GET, HEAD, POST, OPTIONS");
        peer->type("text/plain");
        peer->output.clear();
    }
    co_return "";
}

}// namespace http