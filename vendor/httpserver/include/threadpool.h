#ifndef FRAME_THREADPOOLS_H
#define FRAME_THREADPOOLS_H

#include <cstdio>
#include <cstddef>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <ctime>
#include <iostream>
#include <string>
#include <asio.hpp>
// #include <asio/ssl.hpp>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <map>
#include <string>
#include <sys/time.h>
#include <atomic>
#include <queue>
#include <memory>
#include <map>
#include <thread>
#include <mutex>

#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "httppeer.h"
// #include "threadlocalvariable.h"

namespace http
{

  struct threadinfo_t
  {
    unsigned int index;
    std::thread::id id;
    std::thread thread;
    bool stop = false;
    bool busy = false;
    bool close = false;
    unsigned int timelimit; // 0为不限制
    unsigned long long begin;
    unsigned long long end;
    char ip[65] = {0};
    char url[65] = {0};
  };
  struct thread_config
  {
    int timeout = 60;
    std::thread::id id;
  };

  class ThreadPool
  {
  public:
    ThreadPool(size_t);

    void threadloop(int);
    bool live_end(std::thread::id);
    bool live_add(std::thread::id);
    bool addthread(size_t);
    bool addclient(std::shared_ptr<httppeer>);
    void http_clientrun(std::shared_ptr<httppeer>);
    void timetasks_run(std::shared_ptr<httppeer>);
    void http_websocketsrun(std::shared_ptr<httppeer>);
    bool fixthread();
    unsigned int getpoolthreadnum();
    std::string printthreads(bool);
    unsigned int getlivenum() { return livethreadcount.load(); };
    unsigned int gettasknum()
    {
      return 0;
      clienttasks.size();
    };
    unsigned int getmixthreads() { return mixthreads.load(); };

    ~ThreadPool();
    std::string name; // 测试共享
  private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()>> tasks;
    std::queue<std::shared_ptr<httppeer>> clienttasks;
    // synchronization
    std::mutex queue_mutex, livemtx;
    std::condition_variable condition;
    bool stop;
    bool isclose_add = true;
    std::map<std::thread::id, struct threadinfo_t> threadlist;
    std::atomic<unsigned int> pooltotalnum, mixthreads;
    std::atomic<unsigned int> livethreadcount;
  };

}
#endif