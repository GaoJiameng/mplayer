#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "sungtk_interface.h"
//#include "layout.h"
//#include "callback.h"
#include "gtk_show.h"


int main(int argc , char *argv[])
{
    pid_t pid = 0;
    int i = 0;
    w.count_pause = 2;
    //创建无名管道（父子进程间共享的内存）  
    if(pipe(w.fd_01) < 0)  
    {  
        perror("pipe");  
    }  
    





    srand((unsigned)time(NULL));                                //创建随机池
    for(i=0 ; i<2 ; i++)                                        //创建两个子进程
    {
        pid = fork();               
        if(pid < 0)                                              //创建子进程失败
        {
            perror("fork:");
            _exit(-1);
        }
        else if(pid == 0)                                       //限制子进程再次创建子进程
        {
            break;
        }
    }

    if(i == 0)                                                   //一号子进程  负责屏幕显示
    {
        pthread_t tid1 = 0;  
        pthread_t tid2 = 0;
        pthread_t tid3 = 0;
        gtk_init(&argc, &argv);
        shmaddr(&w);
        //借助addr虚拟地址  往物理共享内存写入数据  
        //strcpy(shmaddr, "hello share m");  
        mkfifo("fifo_cmd", 0777);
        mkfifo("fifo_mplayer", 0777);                           //创建fifo_mplayer命名管道
        w.fd = open("fifo_cmd", O_WRONLY);
        if(w.fd < 0)
        {
            perror("open");
            _exit(-1);
        } 
        // len = read_catalog();

        //2、创建线程  
        pthread_create(&tid1, NULL,fun1, NULL);             //1号线程   负责发命令给mplayer
        pthread_create(&tid2, NULL,fun2, NULL);             //2号线程   负责读mplayer的命令
        pthread_create(&tid3, NULL,fun3, NULL);             //3号线程   显示gtk
    
        //3、等待线程结束  
        void *ret=NULL;  
        pthread_join(tid1, &ret);       
        pthread_join(tid2, &ret);  
        pthread_join(tid3, &ret); 

    }

   else if(i == 1)                                             //二号子进程  负责启动mplayer
    {
        mkfifo("fifo_cmd", 0777);                               //创建fifo_cmd命名管道
        dup2(w.fd_01[1],1);
        execlp("mplayer","mplayer", "-slave", "-quiet", "-idle", "-input",\
        "file=./fifo_cmd", "/home/work/song/Symphony.mp3", NULL);//"-ac", "mad", 
        close(w.fd);
    }


    else                                                         //父进程  负责回收子进程资源
    {
         while(1)
        {   
            pid_t pid = waitpid(-1, NULL, WNOHANG);
            if(pid >0)
            {
                printf("子进程%d退出了\n", pid);
            }
            else if(pid == 0)
            {
                continue;
            }
            else if(pid == -1)
            {
                break;
            }   
        }
    }
    

return 0;
}
