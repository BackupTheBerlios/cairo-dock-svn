
#ifndef __CAIRO_DOCK_APPLICATION_MANAGER__
#define  __CAIRO_DOCK_APPLICATION_MANAGER__

#include <X11/Xlib.h>

#include "cairo-dock-struct.h"

void cairo_dock_initialize_application_manager (Display *pDisplay);

void cairo_dock_register_appli (Icon *icon);
void cairo_dock_blacklist_appli (Window Xid);
void cairo_dock_unregister_appli (Icon *icon);

void cairo_dock_set_icons_geometry_for_window_manager (CairoDock *pDock);;

void cairo_dock_close_xwindow (Window Xid);
void cairo_dock_show_appli (Window Xid);
void cairo_dock_minimize_xwindow (Window Xid);
void cairo_dock_maximize_xwindow (Window Xid, gboolean bMaximize);
void cairo_dock_set_xwindow_fullscreen (Window Xid, gboolean bFullScreen);
void cairo_dock_set_xwindow_above (Window Xid, gboolean bAbove);
void cairo_dock_move_xwindow_to_nth_desktop (Window Xid, int iDesktopNumber, int iDesktopViewportX, int iDesktopViewportY);


gboolean cairo_dock_window_is_maximized (Window Xid);
gboolean cairo_dock_window_is_fullscreen (Window Xid);
void cairo_dock_window_is_above_or_below (Window Xid, gboolean *bIsAbove, gboolean *bIsBelow);
void cairo_dock_window_is_fullscreen_or_hidden (Window Xid, gboolean *bIsFullScreen, gboolean *bIsHidden);
Window cairo_dock_get_active_window (void);

int cairo_dock_get_window_desktop (int Xid);
void cairo_dock_get_window_geometry (int Xid, int *iGlobalPositionX, int *iGlobalPositionY, int *iWidthExtent, int *iHeightExtent);
void cairo_dock_get_window_position_on_its_viewport (int Xid, int *iRelativePositionX, int *iRelativePositionY);


gboolean cairo_dock_window_is_on_this_desktop (int Xid, int iDesktopNumber);
gboolean cairo_dock_window_is_on_current_desktop (int Xid);


gboolean cairo_dock_unstack_Xevents (CairoDock *pDock);
void cairo_dock_set_window_mask (Window Xid, long iMask);
Window *cairo_dock_get_windows_list (gulong *iNbWindows);
CairoDock *cairo_dock_insert_appli_in_dock (Icon *icon, CairoDock *pMainDock, gboolean bUpdateSize, gboolean bAnimate);
void cairo_dock_update_applis_list (CairoDock *pDock, double fTime);
void cairo_dock_start_application_manager (CairoDock *pDock);

void cairo_dock_pause_application_manager (void);

void cairo_dock_stop_application_manager (CairoDock *pDock);

gboolean cairo_dock_application_manager_is_running (void);


void cairo_dock_free_class_appli (CairoDockClassAppli *pClassAppli);
GList *cairo_dock_list_existing_appli_with_class (const gchar *cClass);

gboolean cairo_dock_add_appli_to_class (Icon *pIcon);
gboolean cairo_dock_remove_appli_from_class (Icon *pIcon);
gboolean cairo_dock_set_class_use_xicon (const gchar *cClass, gboolean bUseXIcon);
gboolean cairo_dock_inhibate_class (const gchar *cClass, Icon *pInhibatorIcon);

gboolean cairo_dock_class_is_inhibated (const gchar *cClass);
gboolean cairo_dock_class_is_using_xicon (const gchar *cClass);
gboolean cairo_dock_prevent_inhibated_class (Icon *pIcon);

gboolean cairo_dock_remove_icon_from_class (Icon *pInhibatorIcon);
void cairo_dock_deinhibate_class (const gchar *cClass, Icon *pInhibatorIcon);
void cairo_dock_udpate_Xid_on_inhibators (Window Xid, const gchar *cClass);


#endif
