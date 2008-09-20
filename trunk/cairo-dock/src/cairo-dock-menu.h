
#ifndef __CAIRO_DOCK_MENU__
#define  __CAIRO_DOCK_MENU__

#include <gtk/gtk.h>

#include "cairo-dock-struct.h"
G_BEGIN_DECLS


gboolean cairo_dock_notification_remove_icon (gpointer *data);


GtkWidget *cairo_dock_build_menu (Icon *icon, CairoContainer *pContainer);


gboolean cairo_dock_notification_build_menu (gpointer *data);

void cairo_dock_delete_menu (GtkMenuShell *menu, CairoDock *pDock);

G_END_DECLS
#endif
