    /*name: init_deamon.c
     *function:创建一个守护进程
     */
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h> 
    #include <signal.h> 
    #include <sys/param.h> 
    #include <sys/types.h> 
    #include <sys/stat.h> 
    void init_daemon(void) 
    { 
        int pid; 
        int i;
        pid=fork();
        if(pid>0) 
        {
                exit(0);//是父进程，结束父进程 
        }else if(pid< 0){ 
            perror("fail to fork1");
            exit(1);//fork失败，退出
        }
        //是第一子进程，后台继续执行
        setsid();//第一子进程成为新的会话组长和进程组长 
         /* 处理SIGUP信号*/
        if(signal(SIGCHLD,SIG_IGN) == SIG_ERR){
            printf("Cant signal in init_daemon.");
            exit(1);
        }
        //并与控制终端分离 
        pid=fork();
        if(pid>0) 
        {
           exit(0);//是第一子进程，结束第一子进程       
        }
        else if(pid< 0) 
            exit(1);//fork失败，退出 
        //是第二子进程，继续 
        //第二子进程不再是会话组长 
     
        chdir("/tmp");//改变工作目录到/
     
        for(i=0;i< getdtablesize();++i)//关闭打开的文件描述符 
            close(i); 
        umask(022);//重设文件创建掩模 
        return; 
    }