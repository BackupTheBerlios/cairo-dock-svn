
#ifndef __CAIRO_DOCK_MODULES__
#define  __CAIRO_DOCK_MODULES__

#include <glib.h>

#include "cairo-dock-struct.h"


void cairo_dock_initialize_module_manager (gchar *cModuleDirPath);

void cairo_dock_free_visit_card (CairoDockVisitCard *pVisitCard);

CairoDockModule * cairo_dock_load_module (gchar *cSoFilePath, GHashTable *pModuleTable, GError **erreur);

void cairo_dock_preload_module_from_directory (gchar *cModuleDirPath, GHashTable *pModuleTable, GError **erreur);



void cairo_dock_activate_modules_from_list (gchar **cActiveModuleList, CairoDock *pDock);

void cairo_dock_update_conf_file_with_available_modules_full (gchar *cConfFile, gchar *cGroupName, gchar *cKeyName);
#define cairo_dock_update_conf_file_with_available_modules(cConfFile) cairo_dock_update_conf_file_with_available_modules_full (cConfFile, "Applets", "active modules")
#define cairo_dock_update_easy_conf_file_with_available_modules(cConfFile) cairo_dock_update_conf_file_with_available_modules_full (cConfFile, "System", "active modules")

void cairo_dock_update_conf_file_with_active_modules (gchar *cConfFile, GList *pIconList);

void cairo_dock_foreach_module (GHFunc pFunction, gpointer data);



void cairo_dock_free_module (CairoDockModule *module);

Icon * cairo_dock_activate_module (CairoDockModule *module, CairoDock *pDock, GError **erreur);

void cairo_dock_deactivate_module (CairoDockModule *module);

void cairo_dock_stop_deactivate_all_modules (void);

void cairo_dock_reload_module (gchar *cConfFile, gpointer *data);
void cairo_dock_configure_module (GtkWindow *pParentWindow, CairoDockModule *module, CairoDock *pDock, GError **erreur);


Icon *cairo_dock_find_icon_from_module (CairoDockModule *module, GList *pIconList);

CairoDockModule *cairo_dock_find_module_from_name (gchar *cModuleName);


#endif
