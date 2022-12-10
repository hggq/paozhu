#include <string>
#include <vector>
#include <map>
#include "datetime.h"
#include "urlcode.h"
#include "cookie.h"

namespace http
{
 
        Cookie::Cookie()
        {
        }
        Cookie * Cookie::set(std::string key, std::string val, unsigned long exptime,std::string path, std::string domain, bool secure , bool httponly, std::string issamesite )
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
            curkey = key;
            _set[key] = true;
            if (!issamesite.empty())
            {

                _samesite[key] = issamesite;
            }
            return this;
        }
        Cookie *Cookie::opt(std::string key)
        {
            curkey = key;
            return this;
        }
        Cookie *Cookie::domain(std::string d)
        {
            if (curkey.empty())
            {
                return this;
            }
            _domain[curkey] = d;
            return this;
        }
        Cookie *Cookie::expires(unsigned long e)
        {
            if (curkey.empty())
            {
                return this;
            }
            _expires[curkey] = e;
            _set[curkey] = true;
            return this;
        }
        Cookie *Cookie::path(std::string p)
        {
            if (curkey.empty())
            {
                return this;
            }
            _path[curkey] = p;
            return this;
        }
        Cookie *Cookie::secure(unsigned char p)
        {
            if (curkey.empty())
            {
                return this;
            }
            _set[curkey] = true;
            _secure[curkey] = p;
            return this;
        }
        Cookie *Cookie::httponly(unsigned char p)
        {
            if (curkey.empty())
            {
                return this;
            }
            _set[curkey] = true;
            _httponly[curkey] = p;
            return this;
        }
        Cookie *Cookie::samesite(std::string p)
        {
            if (curkey.empty())
            {
                return this;
            }
            _set[curkey] = true;
            _samesite[curkey] = p;
            return this;
        }
        std::string Cookie::get(std::string key)
        {
            return _val[key];
        }
        std::string &Cookie::operator[](std::string key)
        {
            return _val[key];
        }

        std::map<std::string, std::string>::const_iterator Cookie::begin() const
        {
            return _val.begin();
        }

        std::map<std::string, std::string>::const_iterator Cookie::end() const
        {
            return _val.end();
        }

        std::map<std::string, std::string>::iterator Cookie::begin()
        {
            return _val.begin();
        }

        std::map<std::string, std::string>::iterator Cookie::end()
        {
            return _val.end();
        }

        std::map<std::string, std::string> Cookie::getAll()
        {
            return _val;
        }
        std::string Cookie::makeheader(std::string key)
        {
            std::string tempc;
            if (_val.find(key) != _val.end())
            {
                tempc.append("Set-Cookie: ");
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
                        tempc.append(http::getGmtTime(timeexp));
                    //}
                }

                return tempc;
            }
            else
            {
                return tempc;
            }
        }
        void Cookie::clear()
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
        bool Cookie::check(std::string key)
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

        std::string Cookie::getDomain(std::string key)
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
        std::string Cookie::getPath(std::string key)
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
        unsigned long Cookie::getExpires(std::string key)
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
        unsigned char Cookie::getSecure(std::string key)
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
        unsigned char Cookie::getHttponly(std::string key)
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
        std::string Cookie::getSamesite(std::string key)
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

}