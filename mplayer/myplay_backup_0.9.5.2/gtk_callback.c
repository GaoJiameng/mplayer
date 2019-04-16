#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include "sungtk_interface.h"
#include "gtk_callback.h"


char *buf[]={"Airwaves.mp3","Fading.mp3","From_Here.mp3","I_hate_u_i_love_u.mp3",\
    "I_dont_wanna_see_u_anymore.mp3","Symphony.mp3"};  
char *arr[]={"Airwaves.jpeg","Fading.jpeg","From_Here.jpeg","I_hate_u_i_love_u.jpeg",\
    "I_dont_wanna_see_u_anymore.jpeg","Symphony.jpeg"}; 




void shmaddr(gpointer user_data)
{
    WINDOW *p = (WINDOW *)user_data;
    //创建一个唯一的键值  
    key_t key = ftok("/", 2019);  
    //根据key创建一个物理共享内存标识符  
    int shmId = shmget(key, 128, IPC_CREAT|0600);  
    //根据物理内存标识符 映射 虚拟内存  NULL自动分配  0可读可写  
    p->shmaddr_01 =NULL;  
    p->shmaddr_01 = shmat(shmId, NULL, 0);  
    if(p->shmaddr_01 == NULL)  
    {  
        perror("shmat");  
        return;  
    }  

    
    //创建一个唯一的键值  
    key_t key_01 = ftok("/", 2020);  
    //根据key创建一个物理共享内存标识符  
    int shmId_01 = shmget(key_01, 128, IPC_CREAT|0600);  
    //根据物理内存标识符 映射 虚拟内存  NULL自动分配  0可读可写  
    p->shmaddr_02 =NULL;  
    p->shmaddr_02 = shmat(shmId_01, NULL, 0);  
    if(p->shmaddr_02 == NULL)  
    {  
        perror("shmat");  
        return;  
    }  

    //借助addr虚拟地址  往物理共享内存写入数据  
    //strcpy(shmaddr, "hello share m");  
}



void window_background(GtkWidget *widget, int w, int h, const gchar *path)                  //设置播放器背景
{
    gtk_widget_set_app_paintable(widget, TRUE);		//允许窗口可以绘图
    gtk_widget_realize(widget);	

    /* 更改背景图时，图片会重叠
    * 这时要手动调用下面的函数，让窗口绘图区域失效，产生窗口重绘制事件（即 expose 事件）。
    */
    gtk_widget_queue_draw(widget);

    GdkPixbuf *src_pixbuf = gdk_pixbuf_new_from_file(path, NULL);	// 创建图片资源对象
    // w, h是指定图片的宽度和高度
    GdkPixbuf *dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, w, h, GDK_INTERP_BILINEAR);

    GdkPixmap *pixmap = NULL;

    /* 创建pixmap图像; 
    * NULL：不需要蒙版; 
    * 123： 0~255，透明到不透明
    */
    gdk_pixbuf_render_pixmap_and_mask(dst_pixbuf, &pixmap, NULL, 128);
    // 通过pixmap给widget设置一张背景图，最后一个参数必须为: FASLE
    gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);

    // 释放资源
    g_object_unref(src_pixbuf);
    g_object_unref(dst_pixbuf);
    g_object_unref(pixmap);
}

void loadImagePixbuf( GtkWidget *image,char *fileName,int w, int h)                     //加载图片（可以调整图片大小）
{
    //清掉原始图片控件中的数据
    gtk_image_clear(GTK_IMAGE(image));

    //获取图片资源
    GdkPixbuf *src = gdk_pixbuf_new_from_file(fileName,NULL);
    //修改图片资源
    GdkPixbuf *dst = gdk_pixbuf_scale_simple(src,w,h, GDK_INTERP_BILINEAR);
    //将图片资源设置到图片控件中
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), dst);
    //释放上面的图片资源
    g_object_unref(src);
    g_object_unref(dst);
}



void add_bar(double time , gpointer user_data)  //user_data==bar
{
    WINDOW *p = (WINDOW *)user_data;
    //获得进度条的进度百分比
    double lf =  gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(p->bar));
    lf = time;
    // printf("call = %lf\n",time);
    if(lf >= 0.990000)
    {
         gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(p->bar),0.000000);
         deal_button(NULL,p);
    }
    else
    {
       //设置进度条的百分比1.0
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(p->bar),lf);
    }
    return;  
}




void deal_pause(GtkButton *button, gpointer   user_data)                      //暂停按钮回调函数
{
    WINDOW *p = (WINDOW *)user_data;
    p->count_pause++;
    if(p->count_pause%2 == 1)
    {
        loadImagePixbuf(p->pause_image,"/home/work/icon/pause.png", 60,60);   //暂停
        gtk_table_attach_defaults(GTK_TABLE(p->table), p->pause, 7,9,8,9);
    }
    else if (p->count_pause%2 == 0)
    {
        loadImagePixbuf(p->pause_image,"/home/work/icon/stop.png", 60,60);    //播放
        gtk_table_attach_defaults(GTK_TABLE(p->table), p->pause, 7,9,8,9);
    }
    
    char cmd_pause[10]="pause\n";  
    write(p->fd,cmd_pause, strlen(cmd_pause));  
    return;  
}

void deal_button(GtkButton *button, gpointer   user_data)                    //按下 上一首或下一首按钮的回调函数
{
    WINDOW *p = (WINDOW *)user_data; 
    char songname[50]; 
    int n =  6 ; //sizeof(buf)/sizeof(buf[0]);  
    static int index = 0;  

    if(button == GTK_BUTTON(p->back))  
    {  
        index--;  
        if(index == -1)  
            index = n-1;  
    }  
    else if(button == GTK_BUTTON(p->next))  
    {  
        index++;  
        if(index == n)  
            index = 0;  
        // if (full_count_pause%2 == 0) 
        // {
        //     change_play(p);
        // }   
    }  
    else
    {
        index++;  
        if(index == n)  
            index = 0; 
    }
    if (p->count_pause%2 == 1)
    {
        loadImagePixbuf(p->pause_image,"./icon/stop.png", 60,60);    //播放
        gtk_table_attach_defaults(GTK_TABLE(p->table), p->pause, 7,9,8,9);
        p->count_pause++;
    }
    //根据文件名 拼凑路径
    char path[64]="";
    sprintf(path,"/home/work/song_cover/%s", arr[index]);
    //path给图片空间image重新加载图片资源
    loadImagePixbuf(p->bg_image,path, 220,220);
    loadImagePixbuf(p->shrink_image,path, 50,50);

    sscanf(arr[index],"%[^.]",songname);
    //printf("songname = %s\n",songname);
    gtk_label_set_text(GTK_LABEL(p->label_shrink_song) , songname);
    gtk_label_set_text(GTK_LABEL(p->label_songname) , songname);
    
    //将image控件添加到table中
    gtk_table_attach_defaults(GTK_TABLE(p->table), p->bg_image, 1,7,1,7);
    gtk_table_attach_defaults(GTK_TABLE(p->table), p->shrink_image, 0,1,8,9);

    char cmd[128]="";  
    int len = sprintf(cmd,"loadfile /home/work/song/%s\n", buf[index]);  
    write(p->fd,cmd, len);  
    return;  

}




void deal_switch(GtkButton *button,gpointer user_data)//user_data=&window               //切换播放器背景
{
    //在指针变量使用中  &p  加一个*   *p 减一个*    所以&与*同时出现可以抵消
    //int arr[5]={1,2,3,4,5};
    //arr == &arr[0] == &*(arr+0) == arr+0==arr
    //需求：定义有一个数组  数组有24个元素 每个元素为char *
    char *arr[] = {"1.png","2.png","3.png","4.png","5.png","6.png"};
    int n = 6 ;//sizeof(arr)/sizeof(arr[0]);
    static int index = 0;
    WINDOW *p = (WINDOW *)(user_data);
    index++;
    if(index == n)
    {
        index = 0;
    }
    //根据文件名 拼凑路径
    char path[64]="";
    sprintf(path,"/home/work/background_picture/%s", arr[index]);

    //根嗄path给图片空间image重新加载图片资源
    //loadImagePixbuf(p->image,path, 1024,600);

    //将image控件添加到table中
    //gtk_table_attach_defaults(GTK_TABLE(p->table),p->image, 1,6,1,5);
    window_background(p->window, 1024, 600, path);
}


void random_play(GtkButton *button, gpointer   user_data)                           //随机播放函数
{
    int a = 0 ;
    a = (rand()%5)+1;
    static int count_random = 3;
    WINDOW *p = (WINDOW *)user_data;
    if(count_random%2 == 1)
    {
        loadImagePixbuf(p->random_image,"/home/work/icon/random.png", 28,28);
        gtk_table_attach_defaults(GTK_TABLE(p->table), p->random, 15,16,8,9);
    }
    else if (count_random%2 == 0)
    {
        loadImagePixbuf(p->random_image,"/home/work/icon/circulation.png", 28,28);
        gtk_table_attach_defaults(GTK_TABLE(p->table),p->random, 15,16,8,9);
    }
    count_random++;
 

    return;  
}


void deal_volume(GtkButton *button, gpointer   user_data)
{
    //给主窗口添加鼠标事件
    //GDK_BUTTON_PRESS_MASK鼠标单击事件
    //GDK_BUTTON_MOTION_MASK鼠标移动事件
    // gtk_widget_add_events(w.window, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK);

    //链接鼠标事件的信号
    // g_signal_connect(w.window,"button-press-event", G_CALLBACK(deal_button_press), NULL);
        
    //鼠标移动事件
    // g_signal_connect(w.window, "motion-notify-event", G_CALLBACK(motion_mask), NULL);

    WINDOW *p = (WINDOW *)user_data;  
    static int volume = 90;
    char cmd[128]=""; 
    if(volume<0)
        volume = 0;
    if(volume>100)
        volume = 100;

    

    if(button == GTK_BUTTON(p->volume_plus))  
    {    
        volume +=10;
        int len_01 = sprintf(cmd,"volume %d 1\n", volume);
        write(p->fd,cmd, len_01); 
    }  
    else if(button == GTK_BUTTON(p->volume_minus))  
    {  
        volume -=10;
        int len_02 = sprintf(cmd,"volume %d 1\n", volume);
        write(p->fd,cmd, len_02); 
    }  
    else
    {
        int len_03 = sprintf(cmd,"volume %d 1\n", volume);
        write(p->fd,cmd, len_03); 
    }
    
    
 
    return;  


}






   

