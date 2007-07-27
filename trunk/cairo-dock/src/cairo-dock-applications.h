
#ifndef __CAIRO_DOCK_APPLIS__
#define  __CAIRO_DOCK_APPLIS__

#include <glib.h>
#include <gdk/gdkx.h>

#include "cairo-dock-struct.h"


Window *cairo_dock_get_windows_list (gulong *iNbWindows);


gulong cairo_dock_get_xwindow_timestamp (Window Xid);
void cairo_dock_set_xwindow_timestamp (Window Xid, gulong iTimeStamp);


void cairo_dock_close_xwindow (Window Xid);


void cairo_dock_show_appli (Window Xid);


void cairo_dock_minimize_xwindow (Window Xid);
void cairo_dock_maximize_xwindow (Window Xid, gboolean bMaximize);
void cairo_dock_set_xwindow_fullscreen (Window Xid, gboolean bFullScreen);
void cairo_dock_move_xwindow_to_nth_desktop (Window Xid, int iDesktopNumber);

gboolean cairo_dock_window_is_maximized (Window Xid);
gboolean cairo_dock_window_is_fullscreen (Window Xid);
int cairo_dock_get_current_desktop (void);



void cairo_dock_set_root_window_mask (void);
gboolean cairo_dock_update_applis_list (GtkWidget *pWidget);

int cairo_dock_xerror_handler (Display * pDisplay, XErrorEvent *pXError);

void cairo_dock_show_all_applis (GtkWidget *pWidget);


#endif

