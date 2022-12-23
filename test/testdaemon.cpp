#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include "server.h"
#include "init_daemon.hpp"

static sigjmp_buf env_startacs;
static void sig_child(int signo);

int main(int argc, char *argv[])
{
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
      http::httpserver httpmy;
 
      std::string argv_str;
      if (argc > 1)
      {
        // server.conf filepath or confpath
        argv_str.append(argv[1]);
      }
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
