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

static DBusGConnection *s_pDBusConnexion = NULL;
static DBusGProxy *s_pDBusProxy = NULL;

extern gchar *g_cConfFile;
extern CairoDock *g_pMainDock;

gboolean dbus_deskletVisible = FALSE;
Window dbus_xLastActiveWindow;

G_DEFINE_TYPE(dbusCallback, cd_dbus_callback, G_TYPE_OBJECT);

void cd_dbus_callback_class_init(dbusCallbackClass *class)
{
	// Nothing here
	cd_message("");
}

void cd_dbus_callback_init(dbusCallback *server)
{
	g_return_if_fail (s_pDBusConnexion == NULL);
	
	// Initialise the DBus connection
	cd_message ("Connexion au bus ... ");
	GError *erreur = NULL;
	s_pDBusConnexion = dbus_g_bus_get (DBUS_BUS_SESSION, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		s_pDBusConnexion = NULL;
	}
	if (s_pDBusConnexion == NULL)
		return ;
	server->connection = s_pDBusConnexion;
	
	dbus_g_object_type_install_info(cd_dbus_callback_get_type(), &dbus_glib_cd_dbus_callback_object_info);
	
	// Register DBUS path
	dbus_g_connection_register_g_object(server->connection, "/org/cairodock/CairoDock", G_OBJECT(server));

	// Register the service name, the constants here are defined in dbus-glib-bindings.h
	s_pDBusProxy = dbus_g_proxy_new_for_name(server->connection,
		DBUS_SERVICE_DBUS,
		DBUS_PATH_DBUS,
		DBUS_INTERFACE_DBUS);

	int request_ret;
	if (!org_freedesktop_DBus_request_name (s_pDBusProxy, "org.cairodock.CairoDock", 0, &request_ret, &erreur))
	{
		cd_warning ("Unable to register service: %s", erreur->message);
		g_error_free (erreur);
	}
}


static gpointer _cairo_dock_threaded_dbus_init (gpointer data)
{
	cd_message("");
	GError *erreur = NULL;
	
	dbusCallback *server = g_object_new(cd_dbus_callback_get_type(), NULL);  // -> appelle cd_dbus_callback_class_init() et cd_dbus_callback_init().
	
	cd_message ("*** fin du thread dbus");
	return NULL;
}
void cd_dbus_init(void)
{
	g_type_init();
	cd_message("dbus : Lancement du service");
	
	/**GError *erreur = NULL;
	GThread* pThread = g_thread_create ((GThreadFunc) _cairo_dock_threaded_dbus_init,
		NULL,
		FALSE,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}*/
	_cairo_dock_threaded_dbus_init (NULL);  // est-ce utile de le threader ? des fois j'ai l'impression que l'init prend du temps...
}

gboolean cd_dbus_callback_hello(dbusCallback *pDbusCallback, GError **error)
{
	cairo_dock_show_general_message("Hello !",3000);
	return TRUE;
}

gboolean cd_dbus_callback_show_desklet(dbusCallback *pDbusCallback, gboolean *widgetLayer, GError **error)
{
	if (dbus_deskletVisible)
	{
		cairo_dock_set_desklets_visibility_to_default ();
		cairo_dock_show_appli (dbus_xLastActiveWindow);
	}
	else
	{
		dbus_xLastActiveWindow = cairo_dock_get_active_window ();
		cairo_dock_set_all_desklets_visible (widgetLayer != NULL ? *widgetLayer : FALSE);
	}
	dbus_deskletVisible = !dbus_deskletVisible;
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



gboolean cairo_dock_bdus_is_enabled (void)
{
	return (s_pDBusConnexion != NULL);
}

DBusGProxy *cairo_dock_create_new_dbus_proxy (const char *name, const char *path, const char *interface)
{
	if (s_pDBusConnexion)
		return dbus_g_proxy_new_for_name (
			s_pDBusConnexion,
			name,
			path,
			interface);
	else
		return NULL;
}

gboolean cairo_dock_dbus_detect_application (const gchar *cName)
{
	cd_message ("");
	if (! s_pDBusConnexion)
		return FALSE;
	
	gchar **name_list = NULL;
	gboolean bPresent = FALSE;
	if(dbus_g_proxy_call (s_pDBusProxy, "ListNames", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRV,
		&name_list,
		G_TYPE_INVALID))
	{
		cd_message ("detection du service %s ...", cName);
		int i;
		for (i = 0; name_list[i] != NULL; i ++)
		{
			if (strcmp (name_list[i], cName) == 0)
			{
				bPresent = TRUE;
				break;
			}
		}
	}
	g_strfreev (name_list);
	return bPresent;
}
