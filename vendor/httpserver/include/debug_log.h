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
        // ---- 跨线程共享（inline static，全进程一份）----
        // 配置只在启动时由主线程经 setDebug()/setLogfile() 写入（server.cpp 唯一调用点）。
        // 若留在实例里，thread_local 化后工作线程各拿一份空路径/默认开关，日志落盘直接失效；
        // writemutex 每线程一把等于没有锁，fopen/fwrite 的跨线程互斥同样必须上收到类级。
        inline static bool debug = true;
        inline static std::mutex writemutex;
        inline static std::string file_log_pathname;
        inline static std::string file_runtime_pathname;
        inline static std::string file_error_pathname;

        // ---- 线程私有（thread_local 实例持有）----
        // output/issave/cache_list* 沿「LOG_ERROR << a << b << LOG_END」一条链连续读写。
        // 原进程级单例被两条线程各拼各的：append 扩容先 new 后 delete，
        // 一方扩容 free 掉另一方正拿着的旧缓冲 → ASan use-after-free
        //（现场：dispatch_parked → http2_send_rst_stream，T28/T29 同一行 debug_log.h）。
        // 拼接状态按线程切开后，这条竞争整类消失。
        unsigned char issave = 0;
        std::string output;
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
            // 每线程一份拼接状态：LOG 链的 append 从此只碰本线程的对象，
            // 跨线程撕 output 的 use-after-free 整类消失。
            // 用指针 + 不析构：thread_local 值对象会在线程退出时先死，
            // 收尾/join 之后任何一行日志都会命中已销毁实例；指针本身平凡析构，
            // 对象随进程存活。代价是每线程一份缓冲永不归还，上界=线程数×实例大小。
            thread_local debug_log *singleton = new debug_log();
            return *singleton;
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