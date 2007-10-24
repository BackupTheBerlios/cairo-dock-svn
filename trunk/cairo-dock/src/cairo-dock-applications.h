
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
void cairo_dock_move_xwindow_to_nth_desktop (Window Xid, int iDesktopNumber, int iDesktopViewportX, int iDesktopViewportY);

gboolean cairo_dock_window_is_maximized (Window Xid);
gboolean cairo_dock_window_is_fullscreen (Window Xid);
Window cairo_dock_get_active_window (void);
void cairo_dock_get_current_desktop (int *iDesktopNumber, int *iDesktopViewportX, int *iDesktopViewportY);


void cairo_dock_set_root_window_mask (void);
gboolean cairo_dock_update_applis_list (CairoDock *pDock);

int cairo_dock_xerror_handler (Display * pDisplay, XErrorEvent *pXError);

void cairo_dock_show_all_applis (CairoDock *pDock);


void cairo_dock_set_strut_partial (int Xid, int left, int right, int top, int bottom, int left_start_y, int left_end_y, int right_start_y, int right_end_y, int top_start_x, int top_end_x, int bottom_start_x, int bottom_end_x);

void cairo_dock_set_window_type_hint (int Xid, gchar *cWindowTypeName);


void cairo_dock_set_one_icon_geometry_for_window_manager (Icon *icon, CairoDock *pDock);
void cairo_dock_set_icons_geometry_for_window_manager (CairoDock *pDock);


#endif
