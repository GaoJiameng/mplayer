#ifndef _GTK_CALLBACK_H_
#define _GTK_CALLBACK_H_
#include <gtk/gtk.h>

typedef struct 
{
    GtkWidget *window;              //主窗口
    GtkWidget *table;               //网格布局控件
    GtkWidget *bar;                 //创建进度条
    GtkWidget *table_bar;
    GtkWidget *table_profile;
    GtkWidget *table_time;
    GtkWidget *back;                //上一首按钮
    GtkWidget *next;                //下一首按钮
    GtkWidget *pause;               //暂停按钮
    GtkWidget *random;              //随机播放按钮
    GtkWidget *switch_button;       //切换背景按钮
    GtkWidget *volume_plus;              //音量按钮
    GtkWidget *volume_minus;
    GtkWidget *list;                //list按钮
    GtkWidget *clist_song;          //歌曲分栏列表
    GtkWidget *label_songname;      //歌名
    GtkWidget *label_singer;        //歌手
    GtkWidget *label_shrink_song;        
    GtkWidget *label_time; 
    GtkWidget *label_gang;
    GtkWidget *label_time_01; 
           
    GtkWidget *shutdown;          //关闭按钮
    GtkWidget *back_image;          //上一首按钮图标
    GtkWidget *next_image;          //下一首按钮图标
    GtkWidget *pause_image;         //暂停按钮图标
    GtkWidget *random_image;        //随机播放按钮图标
    GtkWidget *switch_image;        //切换背景按钮图标
    GtkWidget *volume_plus_image;        //音量按钮图标
    GtkWidget *volume_minus_image;
    GtkWidget *bg_image;            //歌曲图片
    GtkWidget *shrink_image;        //缩略图图片
    GtkWidget *disc_image;          //唱片图片
    GtkWidget *shrink_above_image;  //缩略图蒙版图片
    GtkWidget *list_image;          //list图片
    GtkWidget *bg_clist_image;      //列表背景图片
    GtkWidget *shutdown_image;    //关闭按钮图标
    int fd;                         //返回open函数状态
    int fd_01[2];
    int fd_02;
    int fd_sl;
    int fd_sc;
    char *songlist[18];
    char *songcover[18];
    char *shmaddr_01;
    char *shmaddr_02;
    char *shmaddr_03;
    char *shmaddr_04;
    int count_pause;
    char path_src[50];
    
}WINDOW;

typedef struct student{
	int num;
	char name[500];
	struct student *next;
}STU;


extern char *buf[];
extern char *arr[];
// extern int full_count_pause;
extern void window_background(GtkWidget *widget, int w, int h, const gchar *path);
extern void loadImagePixbuf( GtkWidget *image,char *fileName,int w, int h);
extern void add_bar(double  time , gpointer user_data);
extern void deal_pause(GtkButton *button, gpointer   user_data);
extern void deal_button(GtkButton *button, gpointer   user_data);  
extern void deal_switch(GtkButton *button,gpointer user_data);
extern void random_play(GtkButton *button, gpointer   user_data); 
extern void deal_volume(GtkButton *button, gpointer   user_data);
extern void shmaddr(gpointer user_data);
// extern void change_play(gpointer   user_data);


#endif