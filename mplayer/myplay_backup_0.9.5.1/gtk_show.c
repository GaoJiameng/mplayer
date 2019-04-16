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
#include "sungtk_interface.h"
#include "gtk_show.h"
#include "gtk_callback.h"


WINDOW w;
SunGtkCList *clist;

void *fun1(void *arg)     //1号线程  负责发命令给mplayer
{  
    get_pos_pthread();
    return NULL;  
}  

void *fun2(void *arg)     //2号线程   负责读mplayer的命令
{  
    pipe_read();
    return NULL;  
}  

void *fun3(void *arg)     //3号线程   显示gtk
{  
    show_gtk();  
    return NULL;
}  

gboolean set_label_name(gpointer data)
{
    gtk_label_set_text(GTK_LABEL(w.label_shrink_song) , (char *)data);
    gtk_label_set_text(GTK_LABEL(w.label_songname) , (char *)data);
	return TRUE;
}

gboolean set_bar(gpointer data)
{
    
    double time = atof((char *)data);
    // printf("show = %lf\n",time);
    time = time/100;
    add_bar(time , &w); 
    return TRUE;
}

void msg_dispose(char buf[])   //获取歌曲名称
{   
    // char *buf_src[3];
    // buf_src[0] = strtok(buf,"='");
	// buf_src[1] = strtok(NULL,"='");
    // buf_src[2] = strtok(buf_src[1],".");
    // strcpy(w.shmaddr_01, buf_src[2]);  

}

void percent_dispose(char buf[])   //获取进度信息
{   
    char *buf_src[3];
    buf_src[0] = strtok(buf,"=");
	buf_src[1] = strtok(NULL,"=");
    strcpy(w.shmaddr_02, buf_src[1]);

}

void time_dispose(char buf[])   //获取歌曲当前播放时间
{

}

void length_dispose(char buf[])   //获得歌的总长度
{
    
}



void *get_pos_pthread(void)         //写信息函数
{   
    
    while(1)
    {   
        if(w.count_pause%2 == 0)
        {   
            //printf("w.count_pause = %d\n",w.count_pause);
            char cmd[20]="get_percent_pos\n";  //获取进度信息
            if((write(w.fd,cmd, strlen(cmd)))!=strlen(cmd))
            {
                perror("write");
            }
            char cmd_03[20]="get_time_pos\n";  //获取歌曲当前播放时间
            if((write(w.fd,cmd_03, strlen(cmd_03)))!=strlen(cmd_03))
            {
                perror("write");
            }

            char cmd_01[20]="get_time_length\n";  //获得歌的总长度
            if((write(w.fd,cmd_01, strlen(cmd_01)))!=strlen(cmd_01))
            {
                perror("write");
            }
            // char cmd_02[20]="get_file_name\n";   //获取歌曲名称
            // if((write(w.fd,cmd_02, strlen(cmd_02)))!=strlen(cmd_02))
            // {
            //     perror("write");
            // }
            usleep(200*1000);
        }
        
    }
    return NULL;
}


void *pipe_read(void)     //读信息函数
{
    int size;
    char buf[100];
    char msg_buf[100];
    while(1)
    {
        memset(buf, 0 , 100) ;
        if((size = read(w.fd_01[0],buf,sizeof(buf))) == -1)//读取mplayer发过来的歌曲信息
        {
            perror("read");
            exit(1);        }

        buf[size]='\0';//使信息变成字符串，便于处理
       
        strcpy(msg_buf,buf);
        // if(strncmp(buf,"ANS_FILENAME",12) ==0) //获取歌曲名称
        // {
        //     buf[strlen(buf)-2]='\0';//多减一个去掉引号
        //     msg_dispose(buf);
        // }
        if(strncmp(buf,"ANS_PERCENT_POSITION", 20)==0) //获取进度信息
        {
            percent_dispose(buf);
        }
        else if(strncmp(buf,"ANS_TIME_POSITION", 17) ==0) //获取歌曲当前播放时间
        {
            time_dispose(buf);
        }
        else if(strncmp(buf,"ANS_LENGTH",10) ==0) //获得歌的总长度
        {
            length_dispose(buf);
        }
    }
    return NULL;
}




// void *pipe_write_pthread(void *arg)     //写管道线程
// {   
//     get_pos_pthread();
//     return NULL;
// }

// void *pipe_read_pthread(void *arg)     //读管道线程
// {   
//     pipe_read();
//     return NULL;
// }

gboolean callback_list_release(GtkWidget *widget, GdkEventButton *event, gpointer data)
{     
	int row = (int)data;
    char path_src[50];
	printf("%d\n", row);
	
	const char *text = sungtk_clist_get_row_data(clist, row);
    char cmd[128]="";  
    int len = sprintf(cmd,"loadfile /home/work/song/%s\n", text);
    printf("%s",cmd);
    write(w.fd,cmd, len); 
    //更换封面
    //cmd给图片空间image重新加载图片资源
    sscanf(text,"%[^.]",path_src);
    //char *buf[1] = strtok(text,".");
    printf("path_src =%s\n",path_src);
    char cmd_01[128]="";  
    sprintf(cmd_01,"/home/work/song_cover/%s.jpeg", path_src);
    printf("cmd_01=%s\n",cmd_01);
    loadImagePixbuf(w.bg_image,cmd_01, 220,220);
    loadImagePixbuf(w.shrink_image,cmd_01, 50,50);

    gtk_label_set_text(GTK_LABEL(w.label_shrink_song) , path_src);
    gtk_label_set_text(GTK_LABEL(w.label_songname) , path_src);
    //将image控件添加到table中
    gtk_table_attach_defaults(GTK_TABLE(w.table), w.bg_image, 1,7,1,7);
    gtk_table_attach_defaults(GTK_TABLE(w.table), w.shrink_image, 0,1,8,9);
	return TRUE;
}



gboolean deal_button_press( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
    //判断左键还是右键按下
    if(event->button == 1)//左键
    {
        printf("左键按下：x=%lf, y=%lf\n", event->x, event->y);
    }
    else if(event->button == 3)//右键
    {
        printf("右键按下：x=%lf, y=%lf\n", event->x, event->y);
    }

    if(event->type == GDK_BUTTON_PRESS)
    {
        printf("单击\n");
    }
    else if(event->type == GDK_2BUTTON_PRESS)
    {
        printf("双击\n");
    }
    return TRUE;
}




gboolean motion_mask( GtkWidget *widget, GdkEventMotion *event, gpointer data )
{
    printf("鼠标移动了：x = %lf, y=%lf\n", event->x, event->y);
    return TRUE;
}




// int read_catalog(void)
// {
//     system("ls /home/work/song/ > songlist.txt");
//     system("ls /home/work/song_cover/ > songcover.txt");
//     int i = 0,len = 0,j =0;
//     char lrc_buf[1024];
//     char str='\n’;
// 	FILE *fp;
// 	fp = fopen("songlist.txt","rb"); 
// 	if (fp == NULL)
// 	{
// 		perror("songlist.txt");
// 		return 0;
// 	}
// 	fseek(fp,0,SEEK_END);
// 	len = ftell(fp);
// 	rewind(fp);
// 	fread(lrc_buf,1,len,fp);
// 	j = fclose(fp);
// 	w.songlist[i]=strtok(lrc_buf,str);
// 	while(w.songlist[i]!=NULL)
// 	{
// 		i++;
// 		w.songlist[i]=strtok(NULL,str);
// 	}
//     printf("songlist_01 = %s\n",w.songlist[0]);
//     printf("songlist_06 = %s\n",w.songlist[6]);
// 	return i;

// }



void show_gtk(void)
{
    w.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_widget_set_size_request(w.window,1024,600);
        window_background(w.window, 1024, 600, "./background_picture/1.png");
        gtk_window_set_title(GTK_WINDOW(w.window),"mplayer");
        g_signal_connect(w.window, "destroy", G_CALLBACK(gtk_main_quit),NULL);
        w.table = gtk_table_new(9,16,TRUE);
        w.table_bar = gtk_table_new(28,1,TRUE);
        w.table_profile = gtk_table_new(2,1,TRUE);
        w.table_time = gtk_table_new(2,36,TRUE);
        gtk_container_add(GTK_CONTAINER(w.window), w.table);

        //自定义列表开始
        clist = sungtk_clist_new();
        sungtk_clist_set_row_height(clist, 60);
        sungtk_clist_set_col_width(clist, 420);
        sungtk_clist_set_text_size(clist, 15);
        sungtk_clist_set_select_row_signal(clist, "button-release-event", callback_list_release);
        gtk_table_attach_defaults(GTK_TABLE(w.table), clist->fixed, 9, 15, 2, 7);
        
        sungtk_clist_append(clist, buf[0]);
        sungtk_clist_append(clist, buf[1]);
        sungtk_clist_append(clist, buf[2]);
        sungtk_clist_append(clist, buf[3]);
        sungtk_clist_append(clist, buf[4]);
        sungtk_clist_append(clist, buf[5]);
        
        
        
        
        sungtk_clist_set_row_color(clist, 0, "#D9D9D9");
        sungtk_clist_set_row_color(clist, 1, "#D9D9D9");
        sungtk_clist_set_row_color(clist, 2, "#D9D9D9");
        // sungtk_clist_set_row_color(clist, 2, "#000000");
        sungtk_clist_set_row_color(clist, 3, "#D9D9D9");
        sungtk_clist_set_row_color(clist, 4, "#D9D9D9");
        sungtk_clist_set_row_color(clist, 5, "#D9D9D9");
        //自定义列表结束





        w.back = gtk_button_new();  
        w.next = gtk_button_new();  
        w.pause = gtk_button_new(); 
        w.random = gtk_button_new(); 
        w.switch_button = gtk_button_new();
        w.volume_plus = gtk_button_new(); 
        w.volume_minus = gtk_button_new();
        w.list = gtk_button_new();
        w.bar = gtk_progress_bar_new();
        w.label_songname = gtk_label_new("Song Name");
         w.label_singer = gtk_label_new("歌手：Various Artists      专辑：Unknow Album");
        w.label_shrink_song = gtk_label_new("Song Name");
        w.label_time = gtk_label_new("00:00");
        w.label_gang = gtk_label_new("/");
        w.label_time_01 = gtk_label_new("04:00");
        w.clist_song = gtk_label_new("clist_song");
        w.shutdown = gtk_button_new(); 

        


        //创建一个图片控件
        w.back_image = gtk_image_new_from_pixbuf(NULL);
        w.next_image = gtk_image_new_from_pixbuf(NULL);
        w.pause_image = gtk_image_new_from_pixbuf(NULL);
        w.random_image = gtk_image_new_from_pixbuf(NULL);
        w.switch_image = gtk_image_new_from_pixbuf(NULL);
        w.volume_plus_image = gtk_image_new_from_pixbuf(NULL);
        w.volume_minus_image = gtk_image_new_from_pixbuf(NULL);
        w.disc_image = gtk_image_new_from_pixbuf(NULL);
        w.bg_image = gtk_image_new_from_pixbuf(NULL);
        w.list_image = gtk_image_new_from_pixbuf(NULL);
        w.shrink_above_image = gtk_image_new_from_pixbuf(NULL);
        w.shrink_image = gtk_image_new_from_pixbuf(NULL);
        w.shutdown_image = gtk_image_new_from_pixbuf(NULL);
        //加载一张图片
        loadImagePixbuf(w.back_image,"./icon/back.png", 60,60);
        loadImagePixbuf(w.next_image,"./icon/next.png", 60,60);
        loadImagePixbuf(w.pause_image,"./icon/stop.png", 60,60);
        loadImagePixbuf(w.random_image,"./icon/circulation.png", 28,28);
        loadImagePixbuf(w.switch_image,"./icon/switch.png", 30,30);
        loadImagePixbuf(w.volume_plus_image,"./icon/plus.png", 35,35);
        loadImagePixbuf(w.volume_minus_image,"./icon/minus.png", 35,35);
        loadImagePixbuf(w.list_image,"./icon/list.png", 28,28);
        loadImagePixbuf(w.disc_image,"./song_cover/disc.png", 360,360);
        loadImagePixbuf(w.bg_image,"./song_cover/Symphony.jpeg", 220,220);
        loadImagePixbuf(w.shrink_above_image,"./song_cover/shrink_above.png", 50,50);
        loadImagePixbuf(w.shrink_image,"./song_cover/Symphony.jpeg", 50,50);
        loadImagePixbuf(w.shutdown_image,"./icon/shutdown.png", 30,30);
        //将图片控件放入table容器中
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.pause, 7,9,8,9);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.back, 5,6,8,9);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.next, 10,11,8,9);
        //gtk_table_attach_defaults(GTK_TABLE(w.table), w.random, 14,15,8,9);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.switch_button, 15,16,0,1);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.volume_plus, 14,15,8,9); 
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.volume_minus, 15,16,8,9); 
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.list, 13,14,8,9);   
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.disc_image, 1,7,1,7);
        //gtk_table_attach_defaults(GTK_TABLE(w.table), w.clist_song, 1,7,6,7);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.bg_image, 1,7,1,7);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.shrink_above_image, 0,1,8,9);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.shrink_image, 0,1,8,9);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.table_bar, 0,16,7,8);
        gtk_table_attach_defaults(GTK_TABLE(w.table_bar), w.bar, 0,1,20,23);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.table_profile, 9,15,1,2);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.table_time, 1,4,8,9);
        gtk_table_attach_defaults(GTK_TABLE(w.table_time), w.label_shrink_song, 0,36,0,1);
        gtk_table_attach_defaults(GTK_TABLE(w.table_time), w.label_time, 0,8,1,2);
        gtk_table_attach_defaults(GTK_TABLE(w.table_time), w.label_gang, 8,10,1,2);
        gtk_table_attach_defaults(GTK_TABLE(w.table_time), w.label_time_01, 10,18,1,2);
        gtk_table_attach_defaults(GTK_TABLE(w.table_profile), w.label_songname, 0,1,0,1);
        gtk_table_attach_defaults(GTK_TABLE(w.table_profile), w.label_singer, 0,1,1,2);
        gtk_table_attach_defaults(GTK_TABLE(w.table), w.shutdown, 0,1,0,1);


        gtk_label_set_text(GTK_LABEL(w.label_shrink_song) , "Symphony");
        gtk_label_set_text(GTK_LABEL(w.label_songname) , "Symphony");

        gtk_misc_set_alignment(GTK_MISC(w.label_songname), 0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(w.label_singer), 0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(w.clist_song), 0.5, 1);
        sungtk_widget_set_font_color(w.label_songname, "#000000", FALSE);
	    sungtk_widget_set_font_size(w.label_songname, 21, FALSE);
        sungtk_widget_set_font_color(w.label_singer, "#050505", FALSE);
	    sungtk_widget_set_font_size(w.label_singer, 10, FALSE);

        gtk_misc_set_alignment(GTK_MISC(w.label_shrink_song), 0, 1);
        gtk_misc_set_alignment(GTK_MISC(w.label_time), 0, 0);
        gtk_misc_set_alignment(GTK_MISC(w.label_gang), 0.5, 0);
        gtk_misc_set_alignment(GTK_MISC(w.label_time_01), 0, 0);
        sungtk_widget_set_font_color(w.label_shrink_song, "#6E6E6E", FALSE);
	    sungtk_widget_set_font_size(w.label_shrink_song, 14, FALSE);
        sungtk_widget_set_font_color(w.label_time, "#CDC1C5", FALSE);
	    sungtk_widget_set_font_size(w.label_time, 12, FALSE);
        sungtk_widget_set_font_color(w.label_gang, "#CDC1C5", FALSE);
	    sungtk_widget_set_font_size(w.label_gang, 12, FALSE);
        sungtk_widget_set_font_color(w.label_time_01, "#CDC1C5", FALSE);
	    sungtk_widget_set_font_size(w.label_time_01, 12, FALSE);





        gtk_button_set_image(GTK_BUTTON(w.back), w.back_image);
        gtk_button_set_relief(GTK_BUTTON(w.back), GTK_RELIEF_NONE);

        gtk_button_set_image(GTK_BUTTON(w.next), w.next_image);
        gtk_button_set_relief(GTK_BUTTON(w.next), GTK_RELIEF_NONE);

        gtk_button_set_image(GTK_BUTTON(w.pause), w.pause_image);
        gtk_button_set_relief(GTK_BUTTON(w.pause), GTK_RELIEF_NONE); 

        gtk_button_set_image(GTK_BUTTON(w.random), w.random_image);
        gtk_button_set_relief(GTK_BUTTON(w.random), GTK_RELIEF_NONE);

        gtk_button_set_image(GTK_BUTTON(w.switch_button), w.switch_image);
        gtk_button_set_relief(GTK_BUTTON(w.switch_button), GTK_RELIEF_NONE);

        gtk_button_set_image(GTK_BUTTON(w.volume_plus), w.volume_plus_image);
        gtk_button_set_relief(GTK_BUTTON(w.volume_plus), GTK_RELIEF_NONE);

        gtk_button_set_image(GTK_BUTTON(w.volume_minus), w.volume_minus_image);
        gtk_button_set_relief(GTK_BUTTON(w.volume_minus), GTK_RELIEF_NONE);

        gtk_button_set_image(GTK_BUTTON(w.list), w.list_image);
        gtk_button_set_relief(GTK_BUTTON(w.list), GTK_RELIEF_NONE);

        gtk_button_set_image(GTK_BUTTON(w.shutdown), w.shutdown_image);
        gtk_button_set_relief(GTK_BUTTON(w.shutdown), GTK_RELIEF_NONE);

        g_signal_connect(w.back,"clicked", G_CALLBACK(deal_button), &w);  
        g_signal_connect(w.next,"clicked", G_CALLBACK(deal_button), &w);  
        g_signal_connect(w.pause,"clicked", G_CALLBACK(deal_pause), &w);
        g_signal_connect(w.random,"clicked", G_CALLBACK(random_play), &w);
        g_signal_connect(w.switch_button,"clicked", G_CALLBACK(deal_switch), &w);
        g_signal_connect(w.volume_plus,"clicked", G_CALLBACK(deal_volume), &w);
        g_signal_connect(w.volume_minus,"clicked", G_CALLBACK(deal_volume), &w);
        //g_signal_connect(w.list,"clicked", G_CALLBACK(deal_show_list), &w);
        g_signal_connect(w.shutdown,"clicked", G_CALLBACK(gtk_main_quit), NULL);  


        //g_timeout_add(100,set_label_name,w.shmaddr_01);
        g_timeout_add(100,set_bar,w.shmaddr_02);
        
        gtk_widget_show_all(w.window);
        gtk_main();
}



