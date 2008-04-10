/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Adrien Pilleboue (for any bug report, please mail me to adrien.pilleboue@gmail.com)

******************************************************************************/
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include "cairo-dock.h"
//#include "cairo-dock-dbus-spec.h"
#include "cairo-dock-dbus.h"

static DBusGConnection *s_pDBusConnexion = NULL;
static DBusGProxy *s_pDBusProxy = NULL;

extern gchar *g_cConfFile;
extern CairoDock *g_pMainDock;

//G_DEFINE_TYPE(dbusCallback, cd_dbus_callback, G_TYPE_OBJECT);


void cairo_dock_initialize_dbus_manager (void)
{
	g_return_if_fail (s_pDBusConnexion == NULL);
	cd_message ("");
	GError *erreur = NULL;
	s_pDBusConnexion = dbus_g_bus_get (DBUS_BUS_SESSION, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		s_pDBusConnexion = NULL;
	}
	if (s_pDBusConnexion == NULL)
		return ;
	
	s_pDBusProxy = dbus_g_proxy_new_for_name(s_pDBusConnexion,
		DBUS_SERVICE_DBUS,
		DBUS_PATH_DBUS,
		DBUS_INTERFACE_DBUS);
}



DBusGConnection *cairo_dock_get_dbus_connection (void)
{
	return s_pDBusConnexion;
}

void cairo_dock_register_service_name (const gchar *cServiceName)
{
	if (s_pDBusProxy == NULL)
		return ;
	GError *erreur = NULL;
	int request_ret;
	if (!org_freedesktop_DBus_request_name (s_pDBusProxy, cServiceName, 0, &request_ret, &erreur))
	{
		cd_warning ("Unable to register service: %s", erreur->message);
		g_error_free (erreur);
	}
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
