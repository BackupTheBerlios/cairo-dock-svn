
#ifndef __CAIRO_DOCK_MENU__
#define  __CAIRO_DOCK_MENU__

#include <gtk/gtk.h>

#include "cairo-dock-struct.h"


gboolean cairo_dock_notification_remove_icon (gpointer *data);


GtkWidget *cairo_dock_build_menu (Icon *icon, CairoDock *pDock);


gboolean cairo_dock_notification_build_menu (gpointer *data);


//int cairo_dock_ask_question (CairoDock *pDock, const gchar *cQuestion);


#endif
