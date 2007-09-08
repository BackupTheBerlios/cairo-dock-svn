
#ifndef __CAIRO_DOCK_NOTIFICATIONS__
#define  __CAIRO_DOCK_NOTIFICATIONS__

#include <glib.h>


typedef gboolean (* CairoDockNotificationFunc) (gpointer data);

typedef enum {
	CAIRO_DOCK_REMOVE_ICON=0,  // data : {Icon, CairoDock}
	CAIRO_DOCK_CLICK_ICON,  // data : Icon
	CAIRO_DOCK_ADD_ICON,  // data : {Icon, CairoDock}
	CAIRO_DOCK_MODIFY_ICON,  // data : {Icon, CairoDock}
	CAIRO_DOCK_BUILD_MENU,  // data : {CairoDock, Icon}
	CAIRO_DOCK_NB_NOTIFICATIONS
	} CairoDockNotificationType;

void cairo_dock_register_notification (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gboolean bRunFirst);

void cairo_dock_remove_notification_func (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction);

gboolean cairo_dock_notify (CairoDockNotificationType iNotifType, gpointer data);

#endif
