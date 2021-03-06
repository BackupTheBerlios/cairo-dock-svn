
#ifndef __CAIRO_DOCK_DESKTOP_FILE_FACTORY__
#define  __CAIRO_DOCK_DESKTOP_FILE_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"
G_BEGIN_DECLS

/**
*@file cairo-dock-desktop-file-factory.h This class handles the creation of desktop files, which are group/key pair files used by Cairo-Dock to store information about icons : launchers and files, separators, sub-docks.
*/


/** Replace the %20 by normal spaces into the string. The string is directly modified.
*@param cString the string (it can't be a constant string)
*/
void cairo_dock_remove_html_spaces (gchar *cString);

gchar *cairo_dock_add_desktop_file_from_uri_full (const gchar *cURI, const gchar *cDockName, double fOrder, CairoDockNewLauncherType iNewLauncherType, CairoDock *pDock, GError **erreur);

/** Create and add a desktop file for a launcher. It can be either a launcher defined by a common desktop file, or a launcher of a file/folder.
*@param cURI URI of a file defining the launcher.
*@param cDockName name of the dock the separator will be added.
*@param fOrder order of the icon inside the dock.
*@param pDock the dock that will hold the icon.
*@param erreur an error filled if something goes wrong.
*/
#define cairo_dock_add_desktop_file_for_launcher(cURI, cDockName, fOrder, pDock, erreur) cairo_dock_add_desktop_file_from_uri_full (cURI, cDockName, fOrder, CAIRO_DOCK_LAUNCHER_FROM_DESKTOP_FILE, pDock, erreur)
#define cairo_dock_add_desktop_file_from_uri cairo_dock_add_desktop_file_for_launcher

/** Create and add a desktop file for a sub-dock.
*@param cDockName name of the dock the separator will be added.
*@param fOrder order of the icon inside the dock.
*@param pDock the dock that will hold the icon.
*@param erreur an error filled if something goes wrong.
*/
#define cairo_dock_add_desktop_file_for_container(cDockName, fOrder, pDock, erreur) cairo_dock_add_desktop_file_from_uri_full (NULL, cDockName, fOrder, CAIRO_DOCK_LAUNCHER_FOR_CONTAINER, pDock, erreur)

/** Create and add a desktop file for a separator.
*@param cDockName name of the dock the separator will be added.
*@param fOrder order of the icon inside the dock.
*@param pDock the dock that will hold the icon.
*@param erreur an error filled if something goes wrong.
*/
#define cairo_dock_add_desktop_file_for_separator(cDockName, fOrder, pDock, erreur) cairo_dock_add_desktop_file_from_uri_full (NULL, cDockName, fOrder, CAIRO_DOCK_LAUNCHER_FOR_SEPARATOR, pDock, erreur)


gchar *cairo_dock_generate_desktop_filename (const gchar *cBaseName, gchar *cCairoDockDataDir);


void cairo_dock_update_launcher_desktop_file (gchar *cDesktopFilePath, CairoDockNewLauncherType iLauncherType);


gchar *cairo_dock_get_launcher_template_conf_file (CairoDockNewLauncherType iNewLauncherType);


G_END_DECLS
#endif

