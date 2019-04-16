#ifndef _GTK_SHOW_H_
#define _GTK_SHOW_H_
#include "gtk_callback.h"

extern WINDOW w;
extern SunGtkCList *clist;

extern gboolean callback_list_release(GtkWidget *widget, GdkEventButton *event, gpointer data);
extern gboolean deal_button_press( GtkWidget *widget, GdkEventButton *event, gpointer data );
extern gboolean motion_mask( GtkWidget *widget, GdkEventMotion *event, gpointer data );
extern void show_gtk(void);
extern int read_catalog(void);
// extern void *pipe_write_pthread(void *arg);
// extern void *pipe_read_pthread(void *arg);
extern void *pipe_read(void);
extern void *get_pos_pthread(void);
extern void *fun1(void *arg);
extern void *fun2(void *arg);
extern void *fun3(void *arg);

#endif