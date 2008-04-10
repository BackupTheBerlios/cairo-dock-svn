
#ifndef __CAIRO_DOCK_DBUS__
#define  __CAIRO_DOCK_DBUS__

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include "cairo-dock-struct.h"


/**
* Initialise le gestionnaire de bus dans le dock. Il recupere la connexion 'session' de DBus, y etablit un proxy, et les met a disposition de tout le monde.
*/
void cairo_dock_initialize_dbus_manager(void);

/**
* Retourne la connexion 'session' de DBus.
*@return la connexion au bus.
*/
DBusGConnection *cairo_dock_get_dbus_connection (void);
/**
* Enregistre un nouveau service sur le bus.
*@param cServiceName le nom du service.
*/
void cairo_dock_register_service_name (const gchar *cServiceName);
/**
* Dis si le bus est disponible dans le dock.
*@return TRUE ssi le bus a pu etre recupere precedemment.
*/
gboolean cairo_dock_bdus_is_enabled (void);

/**
* Cree un nouveau proxy pour la connexion 'session'.
*@param name un nom sur le bus.
*@param path le chemin associe.
*@param interface nom de l'interface associee.
*@return le proxy nouvellement cree.
*/
DBusGProxy *cairo_dock_create_new_dbus_proxy (const char *name, const char *path, const char *interface);
/**
* Detecte si une application est couramment lancee.
*@param cName nom de l'application.
*@return TRUE ssi l'application est lancee et possede un service sur le bus.
*/
gboolean cairo_dock_dbus_detect_application (const gchar *cName);


#endif
