#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include "server.h"
#include "init_daemon.hpp"

static sigjmp_buf env_startacs;
static void sig_child(int signo);
namespace fs = std::filesystem;
int main(int argc, char *argv[])
{
  std::string argv_str;
  if (argc > 1)
  {
    // server.conf filepath or confpath
    argv_str.append(argv[1]);
    fs::path conf_path = argv_str;
    if (fs::is_regular_file(conf_path))
    {
        
    }
    else
    {
       if(argv_str.back()=='/')
       {
        argv_str=argv_str+"server.conf";
       }
       else
       {
        argv_str=argv_str+"/server.conf";
       }
       conf_path = argv_str;
       if (fs::is_regular_file(conf_path))
       {

       }
       else
       {
          // 启动期找不到配置是一件必须让人看见的事：以前走 std::cout + return 0，
          // 前台看着像"正常退出"，守护进程则连父进程都留不下来，只有日志里没有一行。
          std::cerr << "paozhu: server.conf not found, tried: " << argv[1] << " and "
                    << argv_str << "\n";
          return 1;
       }

    }
  }
  else
  {
 
    fs::path conf_path = fs::current_path();
    argv_str=conf_path.string()+"/conf/server.conf";
    conf_path=argv_str;
    if (fs::is_regular_file(conf_path))
    {

    }
    else
    {
      argv_str="/usr/local/etc/paozhu/server.conf";
      conf_path=argv_str;
      if (fs::is_regular_file(conf_path))
      {
        
      }
      else
      {
        // 这里的候选只有两条，比配置加载器（serverconfig::init_path）少一条
        // /etc/paozhu/conf/server.conf：装在那儿的部署在守护模式下走不到加载器就先退出了。
        std::cerr << "paozhu: server.conf not found, tried: " << argv_str
                  << " and /usr/local/etc/paozhu/server.conf; pass the server.conf path as argv[1]\n";
        return 1;
      }
      
    }
  }
  init_daemon();
  pid_t pid;//, subpid = 0;
  signal(SIGCHLD, sig_child);
  if (sigsetjmp(env_startacs, 1) == 0) // 设置记号
  {
    printf("setjmp ok.....\n");
  }
  else
  {
    printf("longjmp ok.....\n");
  }

  pid = fork();
  printf("fork id %d \n", pid);
  if (pid < 0)
  {
    perror("fork error:");
    exit(1);
  }
  else if (pid == 0)
  {

    try
    {
      http::httpserver &httpmy=http::get_server_app();
      httpmy.run(argv_str);
    }
    catch (std::exception &e)
    {
      std::printf("Exception: %s\n", e.what());
    }
     exit(0);
  }
  else
  {

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        //Future features are added here
    }
    return 0;
  }
}
static void sig_child(int signo)
{
  /*pid_t */ int pid;
  int stat;
  // 处理僵尸进程

  switch (signo)
  {
  case SIGCHLD:

    pid = wait(&stat);
    printf("SIGCHLD...farter id %d..%d\n", getpid(), pid);
    siglongjmp(env_startacs, 1); // jump setjmp begin
    break;
  }
  exit(0);
}
