#ifndef PROJECT_DEBUG_LOG_H
#define PROJECT_DEBUG_LOG_H

#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <memory>
#include <string_view>
#include "request.h"
#include "datetime.h"
namespace http
{

    class debug_log
    {
    public:
        unsigned char issave = 0;
        bool debug = true;
        std::string output;
        std::string file_log_pathname;
        std::string file_runtime_pathname;
        std::string file_error_pathname;
        std::mutex writemutex;
        std::string cache_list1;
        std::string cache_list2;
        std::string cache_list3;
        unsigned long long cache_list1_timeid=0;
        unsigned long long cache_list2_timeid=0;
        unsigned long long cache_list3_timeid=0;
        void setDebug(bool a)
        {
            debug=a;
        }
        void setLogfile(std::string logpath)
        {
            if (logpath.size() > 0 && logpath.back() != '/')
            {
                logpath.push_back('/');
            }
            file_log_pathname = logpath + "debug.log";
            file_runtime_pathname = logpath + "access.log";
            file_error_pathname = logpath + "error.log";
        }
        debug_log &operator<<(http::obj_val &a)
        {
            this->output.append(a.to_string());
            return *this;
        }
        debug_log &operator<<(const std::string &a)
        {
            if (a[0] == '/')
            {
                std::string temp;
                unsigned int str_size = a.size() - 1;
                unsigned int posn = 0;
                for (; str_size > 0; str_size--)
                {
                    if (a[str_size] == '/')
                    {
                        posn += 1;
                        if (posn == 3)
                        {
                            break;
                        }
                    }
                }
                this->output.append(a.substr(str_size));
                this->output.append(" ");
            }
            else
            {
                this->output.append(a);
                this->output.append(" ");
            }
            return *this;
        }
        debug_log &operator<<(std::string &&a)
        {
            this->output.append(a);
            this->output.append(" ");
            return *this;
        }
        debug_log &operator<<(std::string_view a)
        {
            this->output.append(a);
            this->output.append(" ");
            return *this;
        }
        debug_log &operator<<(const char *a)
        {
            
            if (a[0] == '/')
            {
                std::string temp(a);
                unsigned int str_size = temp.size() - 1;
                unsigned int posn = 0;
                for (; str_size > 0; str_size--)
                {
                    if (temp[str_size] == '/')
                    {
                        posn += 1;
                        if (posn == 3)
                        {
                            break;
                        }
                    }
                }
                this->output.append(temp.substr(str_size));
                this->output.append(" ");
            }
            else
            {
                this->output.append(a);
                this->output.append(" ");
            }
            return *this;
        }

        debug_log &operator<<(short a)
        {
            this->output.append(std::to_string(a));
            this->output.append(" ");
            return *this;
        }
        debug_log &operator<<(unsigned short a)
        {
            this->output.append(std::to_string(a));
            this->output.append(" ");
            return *this;
        }
        debug_log &operator<<(long long a)
        {
            if (a == 0xFFFFFFF0)
            {
                if (issave == 1)
                {
                    cache_list1.append(this->output);
                    cache_list1.append("\r\n");
                    
                }
                else if (issave == 2)
                {
                    cache_list2.append(this->output);
                    cache_list2.append("\r\n");    
                }
                else if (issave == 3)
                {
                    cache_list3.append(this->output);
                    cache_list3.append("\r\n");    
                }
                this->output.clear();
                out_str();
                issave = 0;
                return *this;
            }
            if (a == 0xFFFFFFFA)
            {
                if (debug)
                {
                    issave = 0;
                }
                else
                {
                    issave = 1;
                }
            }
            else if (a == 0xFFFFFFFB)
            {
                issave = 2;
            }
            else if (a == 0xFFFFFFFC)
            {
                issave = 3;
            }
            else
            {
                this->output.append(std::to_string(a));
                this->output.append(" ");
            }

            return *this;
        }
        debug_log &operator<<(unsigned long long a)
        {
            if (a == 0xFFFFFFF0)
            {
                if (issave == 1)
                {
                    cache_list1.append(this->output);
                    cache_list1.append("\r\n");
                    
                }
                else if (issave == 2)
                {
                    cache_list2.append(this->output);
                    cache_list2.append("\r\n");    
                }
                else if (issave == 3)
                {
                    cache_list3.append(this->output);
                    cache_list3.append("\r\n");    
                }
                this->output.clear();
                out_str();
                issave = 0;
                return *this;
            }
            if (a == 0xFFFFFFFA)
            {
                if (debug)
                {
                    issave = 0;
                }
                else
                {
                    issave = 1;
                }
            }
            else if (a == 0xFFFFFFFB)
            {
                issave = 2;
            }
            else if (a == 0xFFFFFFFC)
            {
                issave = 3;
            }
            else
            {
                this->output.append(std::to_string(a));
                this->output.append(" ");
            }

            return *this;
        }
        debug_log &operator<<(int a)
        {
            this->output.append(std::to_string(a));
            this->output.append(" ");
            return *this;
        }
        debug_log &operator<<(unsigned int a)
        {
            this->output.append(std::to_string(a));
            this->output.append(" ");
            return *this;
        }
        debug_log &operator<<(float a)
        {
            this->output.append(std::to_string(a));
            this->output.append(" ");
            return *this;
        }
        debug_log &operator<<(double a)
        {

            this->output.append(std::to_string(a));
            this->output.append(" ");
            return *this;
        }

        debug_log &out_str()
        {
            issave = 0;

            if (issave == 1)
            {
                if(cache_list1.size()>300||((timeid()-cache_list1_timeid)>90))
                {
                    try
                    {
                        std::unique_lock<std::mutex> lock(writemutex);
                        FILE *fp = fopen(file_log_pathname.c_str(), "a+");
                        if (fp)
                        {
                            fwrite(&cache_list1[0], 1, cache_list1.size(), fp);
                            char enter[] = {'\r', '\n'};
                            fwrite(enter, 1, sizeof(enter), fp);
                            fclose(fp);
                        }

                        cache_list1.clear();
                        cache_list1_timeid=timeid();
                    }
                    catch(const std::exception& e)
                    {
                        issave = 0;
                        output.clear();
                    }
                
                }   
            }
            else if (issave == 2)
            {
                if(cache_list2.size()>300||((timeid()-cache_list2_timeid)>90))
                {
                    try
                    {
                        std::unique_lock<std::mutex> lock(writemutex);
                        FILE *fp = fopen(file_runtime_pathname.c_str(), "a+");
                        if (fp)
                        {
                            fwrite(&cache_list2[0], 1, cache_list2.size(), fp);
                            char enter[] = {'\r', '\n'};
                            fwrite(enter, 1, sizeof(enter), fp);
                            fclose(fp);
                        }
                        cache_list2.clear();
                        cache_list2_timeid=timeid();
                    }
                    catch(const std::exception& e)
                    {
                        issave = 0;
                        output.clear();
                    }
                }
                
            }
            else if (issave == 3)
            {

                if(cache_list3.size()>300||((timeid()-cache_list3_timeid)>90))
                {
                    try
                    {
                        std::unique_lock<std::mutex> lock(writemutex);
                        FILE *fp = fopen(file_error_pathname.c_str(), "a+");
                        if (fp)
                        {
                            fwrite(&cache_list3[0], 1, cache_list3.size(), fp);
                            char enter[] = {'\r', '\n'};
                            fwrite(enter, 1, sizeof(enter), fp);
                            fclose(fp);
                        }
                        cache_list3.clear();
                        cache_list3_timeid=timeid();
                    }
                    catch(const std::exception& e)
                    {
                        issave = 0;
                        output.clear();
                    }
                }

            }
            else
            {
                if (debug)
                {
                    std::cout << output << std::endl;
                }
            }
            issave = 0;
            output.clear();
            
            return *this;
        }
        static debug_log &instance()
        {
            static debug_log singleton;
            return singleton;
        }
        debug_log &get()
        {
            return *this;
        }
        // static debug_log *singleton;
        // static debug_log *setdebug_logapi(debug_log *p)
        // {
        //     singleton = p;
        //     return singleton;
        // }
        // class ctxmarCollector {
        //     public:
        //         ~ctxmarCollector() {
        //                 delete debug_log::singleton;
        //                 debug_log::singleton = 0;
        //         }
        //  };
        //  static ctxmarCollector gc;
    };
#define LOG_OUT (debug_log::instance()) << (long long)0xFFFFFFFA << __FILE__ << __FUNCTION__ << __LINE__
#define LOG_SAVE (debug_log::instance()) << (long long)0xFFFFFFFB
#define LOG_ERROR (debug_log::instance()) << (long long)0xFFFFFFFC << __FILE__ << __FUNCTION__ << __LINE__
#define LOG_END (long long)0xFFFFFFF0
}
#endif