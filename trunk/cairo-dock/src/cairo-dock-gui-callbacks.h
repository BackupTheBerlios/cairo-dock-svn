
#ifndef __CAIRO_DOCK_GUI_CALLBACKS__
#define  __CAIRO_DOCK_GUI_CALLBACKS__

#include <gtk/gtk.h>
#include "cairo-dock-gui-manager.h"

G_BEGIN_DECLS


void on_click_category_button (GtkButton *button, gpointer *data);
void on_click_all_button (GtkButton *button, gpointer *data);
void on_click_back_button (GtkButton *button, gpointer *data);

void on_click_group_button (GtkButton *button, CairoDockGroupDescription *pGroupDescription);
void on_enter_group_button (GtkButton *button, CairoDockGroupDescription *pGroupDescription);
void on_leave_group_button (GtkButton *button, gpointer *data);


void on_click_apply (GtkButton *button, GtkWidget *pWindow);
void on_click_ok (GtkButton *button, GtkWidget *pWindow);
void on_click_quit (GtkButton *button, GtkWidget *pWindow);

void on_click_activate_given_group (GtkToggleButton *button, CairoDockGroupDescription *pGroupDescription);
void on_click_activate_current_group (GtkToggleButton *button, gpointer *data);


void on_click_normal_apply (GtkButton *button, GtkWidget *pWindow);
void on_click_normal_ok (GtkButton *button, GtkWidget *pWindow);
void on_click_normal_quit (GtkButton *button, GtkWidget *pWindow);

gboolean on_delete_main_gui (GtkWidget *pWidget, GdkEvent *event, GMainLoop *pBlockingLoop);
gboolean on_delete_normal_gui (GtkWidget *pWidget, GdkEvent *event, GMainLoop *pBlockingLoop);


G_END_DECLS
#endif
