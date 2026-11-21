/*name: init_deamon.c
 *function:创建一个守护进程
 */
#ifndef _MSC_VER
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
void init_daemon(void)
{
#ifndef _MSC_VER
    int pid;
    pid = fork();
    if (pid > 0)
    {
        exit(0);//是父进程，结束父进程
    }
    else if (pid < 0)
    {
        perror("fail to fork1");
        exit(1);//fork失败，退出
    }
    //是第一子进程，后台继续执行
    setsid();//第一子进程成为新的会话组长和进程组长
             /* 处理SIGUP信号*/
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
    {
        printf("Cant signal in init_daemon.");
        exit(1);
    }
    //并与控制终端分离
    pid = fork();
    if (pid > 0)
    {
        exit(0);//是第一子进程，结束第一子进程
    }
    else if (pid < 0)
        exit(1);//fork失败，退出
    //是第二子进程，继续
    //第二子进程不再是会话组长

    // After both forks + setsid, close ALL inherited fds except 0/1/2.
    // This prevents fd exhaustion when launchd/shell leaves random fds
    // open, and keeps ORM connection pools, TLS sockets, etc. as the
    // only fds the daemon uses. Redirect 0/1/2 to /dev/null so asio
    // and OpenSSL sanity checks on standard fds won't misbehave.
    // 两次 fork + setsid 完成后，关掉所有继承 fd ≥ 3。
    // 防止 launchd/shell 遗留 fd 耗尽上限，ORM/TLS 连接等从零开始分配。
    // 0/1/2 重定向到 /dev/null，确保 asio/OpenSSL 标准 fd 检查正常。
    {
        int devnull = open("/dev/null", O_RDWR);
        if (devnull >= 0)
        {
            dup2(devnull, 0);
            dup2(devnull, 1);
            dup2(devnull, 2);
            if (devnull > 2) close(devnull);
        }
        for (int i = 3; i < getdtablesize(); ++i)
            close(i);
    }
    umask(022);//重设文件创建掩模
#endif
    return;
}
