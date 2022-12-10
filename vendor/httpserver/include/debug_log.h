#ifndef PROJECT_DEBUG_LOG_H
#define PROJECT_DEBUG_LOG_H

#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <memory>
#include <string_view>
#include "request.h"

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
        debug_log &operator<<(http::OBJ_VALUE &a)
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
            std::string temp = a;
            if (temp[0] == '/')
            {

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
            if (issave == 1)
            {
                try
                {
                    FILE *fp = fopen(file_log_pathname.c_str(), "a+");
                    if (fp)
                    {
                        fwrite(&output[0], 1, output.size(), fp);
                        char enter[] = {'\r', '\n'};
                        fwrite(enter, 1, sizeof(enter), fp);
                        fclose(fp);
                    }
                }
                catch(const std::exception& e)
                {
                    issave = 0;
                    output.clear();
                }
                
                
            }
            else if (issave == 2)
            {
                try
                {
                    FILE *fp = fopen(file_runtime_pathname.c_str(), "a+");
                    if (fp)
                    {
                        fwrite(&output[0], 1, output.size(), fp);
                        char enter[] = {'\r', '\n'};
                        fwrite(enter, 1, sizeof(enter), fp);
                        fclose(fp);
                    }
                }
                catch(const std::exception& e)
                {
                    issave = 0;
                    output.clear();
                }
                
                
            }
            else if (issave == 3)
            {
                try
                {
                    FILE *fp = fopen(file_error_pathname.c_str(), "a+");
                    if (fp)
                    {
                        fwrite(&output[0], 1, output.size(), fp);
                        char enter[] = {'\r', '\n'};
                        fwrite(enter, 1, sizeof(enter), fp);
                        fclose(fp);
                    }
                }
                catch(const std::exception& e)
                {
                    issave = 0;
                    output.clear();
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
        static debug_log *instance()
        {
            if (singleton == NULL)
                singleton = new debug_log();
            return singleton;
        }
        debug_log &get()
        {
            return *this;
        }
        static debug_log *singleton;
        static debug_log *setdebug_logapi(debug_log *p)
        {
            singleton = p;
            return singleton;
        }
        class ctxmarCollector {
            public:
                ~ctxmarCollector() {
                        delete debug_log::singleton;
                        debug_log::singleton = 0;
                }
         };
         static ctxmarCollector gc;
    };
#define LOG_OUT (debug_log::instance()->get()) << (long long)0xFFFFFFFA << __FILE__ << __FUNCTION__ << __LINE__
#define LOG_SAVE (debug_log::instance()->get()) << (long long)0xFFFFFFFB
#define LOG_ERROR (debug_log::instance()->get()) << (long long)0xFFFFFFFC << __FILE__ << __FUNCTION__ << __LINE__
#define LOG_END (long long)0xFFFFFFF0
}
#endif