
#ifndef __CAIRO_DOCK_MENU__
#define  __CAIRO_DOCK_MENU__

#include <gtk/gtk.h>

#include "cairo-dock-struct.h"


GtkWidget *cairo_dock_build_menu (CairoDock *pDock);


gboolean cairo_dock_notification_build_menu (gpointer *data);


#endif
