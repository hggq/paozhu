#include <iostream>
#include <map>
#include <set>
#include <string>
#include "http_mime.h"

namespace http
{

std::map<std::string, std::string> mime_map = {
    {"txt", "text/plain"},
    {"html", "text/html"},
    {"htm", "text/html"},
    {"xml", "text/xml"},
    {"json", "application/json"},
    {"css", "text/css"},
    {"js", "application/javascript"},
    {"md", "text/x-markdown"},
    {"map", "text/plain"},
    {"tsx", "application/javascript"},
    {"jsx", "application/javascript"},
    {"mjs", "application/javascript"},
    {"log", "text/plain"},
    {"csv", "text/csv"},
    {"sass", "text/x-sass"},
    {"scss", "text/x-scss"},

    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"gif", "image/gif"},
    {"png", "image/png"},
    {"bmp", "image/bmp"},
    {"webp", "image/webp"},
    {"svg", "image/svg+xml"},
    {"avi", "video/x-msvideo"},
    {"mid", "audio/midi"},

    {"ttf", "font/ttf"},
    {"otf", "font/otf"},
    {"woff", "font/woff"},
    {"woff2", "font/font/woff2"},

    {"epub", "application/epub+zip"},
    {"xhtml", "application/xhtml+xml"},

    {"pdf", "application/pdf"},
    {"zip", "application/x-zip-compressed"},
    {"rar", "application/octet-stream"},
    {"gz", "application/gzip"},
    {"bz", "application/x-bzip"},
    {"bz2", "application/x-bzip2"},
    {"iso", "application/octet-stream"},
    {"dmg", "application/octet-stream"},
    {"7z", "application/x-7z-compressed"},
    {"tar", "application/x-tar"},

    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {"doc", "application/msword"},

    {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {"xls", "application/vnd.ms-excel"},

    {"ppt", "application/vnd.ms-powerpoint"},
    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},

    {"mp4", "video/mp4"},
    {"mp3", "audio/mp3"},
    {"m4a", "audio/mp4"},
    {"ico", "image/x-icon"},
    {"m3u8", "application/vnd.apple.mpegurl"}

};

std::set<std::string> mime_compress = {{"txt"},
                                       {"html"},
                                       {"htm"},
                                       {"xml"},
                                       {"json"},
                                       {"css"},
                                       {"sass"},
                                       {"scss"},
                                       {"js"},
                                       {"md"},
                                       {"map"},
                                       {"tsx"},
                                       {"jsx"},
                                       {"mjs"},
                                       {"log"},
                                       {"svg"},
                                       {"csv"}};

}// namespace http