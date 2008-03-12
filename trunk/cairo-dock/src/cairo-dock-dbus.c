/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Adrien Pilleboue (for any bug report, please mail me to adrien.pilleboue@gmail.com)

******************************************************************************/
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include "cairo-dock.h"
#include "cairo-dock-dbus-spec.h"
#include "cairo-dock-dbus.h"

extern gchar *g_cConfFile;
extern CairoDock *g_pMainDock;

G_DEFINE_TYPE(dbusCallback, cd_dbus_callback, G_TYPE_OBJECT);

void cd_dbus_callback_class_init(dbusCallbackClass *class)
{
	// Nothing here
}

void cd_dbus_callback_init(dbusCallback *server)
{
	GError *error = NULL;
	DBusGProxy *driver_proxy;
	int request_ret;
	
	// Initialise the DBus connection
	server->connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (server->connection == NULL) {
		g_warning("Unable to connect to dbus: %s", error->message);
		g_error_free(error);
		return;
	}
	
	dbus_g_object_type_install_info(cd_dbus_callback_get_type(), &dbus_glib_cd_dbus_callback_object_info);
	
	// Register DBUS path
	dbus_g_connection_register_g_object(server->connection, "/org/cairodock/CairoDock", G_OBJECT(server));

	// Register the service name, the constants here are defined in dbus-glib-bindings.h
	driver_proxy = dbus_g_proxy_new_for_name(server->connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	if (!org_freedesktop_DBus_request_name (driver_proxy, "org.cairodock.CairoDock", 0, &request_ret, &error)) {
		g_warning("Unable to register service: %s", error->message);
		g_error_free(error);
	}
	
	g_object_unref(driver_proxy);
}

void cd_dbus_init(void)
{
	GMainLoop *main_loop;
	dbusCallback *server;

	cd_message("dbus : Lancement du service\n");

	g_type_init();

	server = g_object_new(cd_dbus_callback_get_type(), NULL);

	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	return 0;
}

gboolean cd_dbus_callback_hello(dbusCallback *pDbusCallback, GError **error)
{
	cairo_dock_show_general_message("Hello !",3000);
	return TRUE;
}

gboolean cd_dbus_callback_show_dialog(dbusCallback *pDbusCallback, gchar *message, GError **error)
{
	cairo_dock_show_general_message(message,3000);
	return TRUE;
}

gboolean cd_dbus_callback_reboot(dbusCallback *pDbusCallback, GError **error)
{
	cairo_dock_read_conf_file (g_cConfFile,g_pMainDock);
	return TRUE;
}
