// pzredis 用到但 bench 不需要的最小 stub
#include <string_view>
namespace http {
bool str2uint64_strict(std::string_view s, unsigned long long &out, unsigned int) {
    if (s.empty()) return false;
    unsigned long long v = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
        v = v * 10 + (c - '0');
    }
    out = v;
    return true;
}
} // namespace http
