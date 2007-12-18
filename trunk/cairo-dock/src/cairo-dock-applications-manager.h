
#ifndef __CAIRO_DOCK_APPLICATION_MANAGER__
#define  __CAIRO_DOCK_APPLICATION_MANAGER__

#include <glib.h>
#include <X11/Xlib.h>

#include "cairo-dock-struct.h"

int cairo_dock_xerror_handler (Display * pDisplay, XErrorEvent *pXError);
void cairo_dock_initialize_application_manager (void);
const Display *cairo_dock_get_Xdisplay (void);

void cairo_dock_register_appli (Icon *icon);
void cairo_dock_blacklist_appli (Window Xid);
void cairo_dock_unregister_appli (Icon *icon);


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
void cairo_dock_window_is_fullscreen_or_hidden (Window Xid, gboolean *bIsFullScreen, gboolean *bIsHidden);
Window cairo_dock_get_active_window (void);
void cairo_dock_get_window_desktop_and_position (int Xid, int *iDesktopNumber, int *iGlobalPositionX, int *iGlobalPositionY);
void cairo_dock_get_current_desktop (int *iDesktopNumber, int *iDesktopViewportX, int *iDesktopViewportY);
gboolean cairo_dock_window_is_on_current_desktop (int Xid);


gboolean cairo_dock_unstack_Xevents (CairoDock *pDock);
void cairo_dock_set_window_mask (Window Xid, long iMask);
Window *cairo_dock_get_windows_list (gulong *iNbWindows);
void cairo_dock_update_applis_list (CairoDock *pDock, double fTime);
void cairo_dock_start_application_manager (CairoDock *pDock);

void cairo_dock_pause_application_manager (void);

void cairo_dock_stop_application_manager (CairoDock *pDock);

gboolean cairo_dock_application_manager_is_running (void);


void cairo_dock_set_strut_partial (int Xid, int left, int right, int top, int bottom, int left_start_y, int left_end_y, int right_start_y, int right_end_y, int top_start_x, int top_end_x, int bottom_start_x, int bottom_end_x);

void cairo_dock_set_xwindow_type_hint (int Xid, gchar *cWindowTypeName);


void cairo_dock_set_one_icon_geometry_for_window_manager (Icon *icon, CairoDock *pDock);
void cairo_dock_set_icons_geometry_for_window_manager (CairoDock *pDock);


gboolean cairo_dock_update_screen_geometry (CairoDock *pDock);


#endif
