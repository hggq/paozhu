#include "http_header.h"
#include "unicode.h"
#include "httppeer.h"
namespace http
{
std::map<unsigned int, std::string> http_status_static_table = {
    {101, "Switching Protocols"},
    {200, "OK"},
    {202, "Accepted"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {304, "Not Modified"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Server Error"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
};
std::string make_header_etag(unsigned long long file_size, unsigned long long file_mix)
{
    std::string etag;
    etag.append(to_hexstring(file_size));
    etag.push_back('-');
    etag.append(to_hexstring(file_mix));
    return etag;
}

// bool make_file_mime(std::string &hh_data, const std::string &filename)
// {
//     return true;
// }

}// namespace http