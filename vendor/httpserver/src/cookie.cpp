#include <string>
#include <vector>
#include <map>
#include "datetime.h"
#include "urlcode.h"
#include "cookie.h"

namespace http
{

cookie::cookie() {}
cookie *cookie::set(std::string key,
                    std::string val,
                    unsigned long exptime,
                    std::string path,
                    std::string domain,
                    bool secure,
                    bool httponly,
                    std::string issamesite)
{
    _val[key] = val;
    if (exptime != 0)
    {
        _expires[key] = exptime;
    }
    if (!domain.empty())
    {
        _domain[key] = domain;
    }
    if (!path.empty())
    {
        _path[key] = path;
    }
    if (secure)
    {
        _secure[key] = 1;
    }
    if (httponly)
    {
        _httponly[key] = 1;
    }
    curkey    = key;
    _set[key] = true;
    if (!issamesite.empty())
    {

        _samesite[key] = issamesite;
    }
    return this;
}
cookie *cookie::opt(std::string key)
{
    curkey = key;
    return this;
}
cookie *cookie::domain(std::string d)
{
    if (curkey.empty())
    {
        return this;
    }
    _domain[curkey] = d;
    return this;
}
cookie *cookie::expires(unsigned long e)
{
    if (curkey.empty())
    {
        return this;
    }
    _expires[curkey] = e;
    _set[curkey]     = true;
    return this;
}
cookie *cookie::path(std::string p)
{
    if (curkey.empty())
    {
        return this;
    }
    _path[curkey] = p;
    return this;
}
cookie *cookie::secure(unsigned char p)
{
    if (curkey.empty())
    {
        return this;
    }
    _set[curkey]    = true;
    _secure[curkey] = p;
    return this;
}
cookie *cookie::httponly(unsigned char p)
{
    if (curkey.empty())
    {
        return this;
    }
    _set[curkey]      = true;
    _httponly[curkey] = p;
    return this;
}
cookie *cookie::samesite(std::string p)
{
    if (curkey.empty())
    {
        return this;
    }
    _set[curkey]      = true;
    _samesite[curkey] = p;
    return this;
}
std::string cookie::get(std::string key) { return _val[key]; }
std::string &cookie::operator[](std::string key) { return _val[key]; }

std::map<std::string, std::string>::const_iterator cookie::begin() const { return _val.begin(); }

std::map<std::string, std::string>::const_iterator cookie::end() const { return _val.end(); }

std::map<std::string, std::string>::iterator cookie::begin() { return _val.begin(); }

std::map<std::string, std::string>::iterator cookie::end() { return _val.end(); }

std::map<std::string, std::string> cookie::getAll() { return _val; }
std::string cookie::makeheader(std::string key)
{
    std::string tempc;
    if (_val.find(key) != _val.end())
    {
        tempc.append("Set-cookie: ");
        tempc.append(http::url_encode(key.data(), key.size()));
        tempc.push_back('=');
        tempc.append(http::url_encode(_val[key].data(), _val[key].size()));

        long long timeexp = 0;
        if (_expires.find(key) != _expires.end())
        {
            timeexp = _expires[key];

            if (timeexp > 0 && timeexp < 63072000)
            {
                timeexp = http::timeid() + timeexp;
            }
            // if (timeexp > 0)
            // {

            tempc.append("; Expires=");
            tempc.append(http::get_gmttime(timeexp));
            //}
        }

        return tempc;
    }
    else
    {
        return tempc;
    }
}
void cookie::clear()
{
    _val.clear();
    _domain.clear();
    _path.clear();
    _expires.clear();
    _secure.clear();
    _httponly.clear();
    _set.clear();
    _samesite.clear();
}
bool cookie::check(std::string key)
{
    if (_val.find(key) != _val.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::string cookie::getDomain(std::string key)
{
    if (_domain.find(key) != _domain.end())
    {
        return _domain[key];
    }
    else
    {
        return "";
    }
}
std::string cookie::getPath(std::string key)
{
    if (_path.find(key) != _path.end())
    {
        return _path[key];
    }
    else
    {
        return "";
    }
}
unsigned long cookie::getExpires(std::string key)
{
    if (_expires.find(key) != _expires.end())
    {
        return _expires[key];
    }
    else
    {
        return 0;
    }
}
unsigned char cookie::getSecure(std::string key)
{
    if (_secure.find(key) != _secure.end())
    {
        return _secure[key];
    }
    else
    {
        return 0;
    }
}
unsigned char cookie::getHttponly(std::string key)
{
    if (_httponly.find(key) != _httponly.end())
    {
        return _httponly[key];
    }
    else
    {
        return 0;
    }
}
std::string cookie::getSamesite(std::string key)
{
    if (_samesite.find(key) != _samesite.end())
    {
        return _samesite[key];
    }
    else
    {
        return "";
    }
}

} // namespace http