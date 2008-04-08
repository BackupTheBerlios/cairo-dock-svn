
#ifndef __CAIRO_DOCK_DBUS__
#define  __CAIRO_DOCK_DBUS__

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include "cairo-dock-struct.h"


void cairo_dock_initialize_dbus_manager(void);


DBusGConnection *cairo_dock_get_dbus_connection (void);
void cairo_dock_register_service_name (const gchar *cServiceName);
gboolean cairo_dock_bdus_is_enabled (void);

DBusGProxy *cairo_dock_create_new_dbus_proxy (const char *name, const char *path, const char *interface);
gboolean cairo_dock_dbus_detect_application (const gchar *cName);


#endif
