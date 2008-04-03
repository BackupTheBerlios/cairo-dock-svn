
#ifndef __CAIRO_DOCK_DBUS__
#define  __CAIRO_DOCK_DBUS__

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include "cairo-dock-struct.h"

typedef struct
{
	GObject parent;
	DBusGConnection *connection;
} dbusCallback;

typedef struct
{
	GObjectClass parent_class;
} dbusCallbackClass;

void cd_dbus_init(void);

gboolean cairo_dock_bdus_is_enabled (void);
DBusGProxy *cairo_dock_create_new_dbus_proxy (const char *name, const char *path, const char *interface);
gboolean cairo_dock_dbus_detect_application (const gchar *cName);


gboolean cd_dbus_callback_hello(dbusCallback *pDbusCallback, GError **error);
gboolean cd_dbus_callback_show_dialog(dbusCallback *pDbusCallback, gchar *message, GError **error);
gboolean cd_dbus_callback_show_desklet(dbusCallback *pDbusCallback, gboolean *widgetLayer, GError **error);
gboolean cd_dbus_callback_reboot(dbusCallback *pDbusCallback, GError **error);
#endif
