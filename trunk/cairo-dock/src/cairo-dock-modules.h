
#ifndef __CAIRO_DOCK_MODULES__
#define  __CAIRO_DOCK_MODULES__

#include <glib.h>

#include "cairo-dock-struct.h"


void cairo_dock_initialize_module_manager (gchar *cModuleDirPath);

void cairo_dock_free_visit_card (CairoDockVisitCard *pVisitCard);

CairoDockModule * cairo_dock_load_module (gchar *cSoFilePath, GHashTable *pModuleTable, GError **erreur);

void cairo_dock_preload_module_from_directory (gchar *cModuleDirPath, GHashTable *pModuleTable, GError **erreur);



void cairo_dock_activate_modules_from_list (gchar **cActiveModuleList, CairoDock *pDock, double fTime);

void cairo_dock_deactivate_old_modules (double fTime);


void cairo_dock_free_module (CairoDockModule *module);

GKeyFile *cairo_dock_pre_read_module_config (CairoDockModule *pModule, CairoDockMinimalAppletConfig *pMinimalConfig);

Icon * cairo_dock_activate_module (CairoDockModule *module, CairoDock *pDock, GError **erreur);

void cairo_dock_deactivate_module (CairoDockModule *module);

void cairo_dock_reload_module (CairoDockModule *module, gboolean bReloadAppletConf);


void cairo_dock_deactivate_all_modules (void);

void cairo_dock_activate_module_and_load (gchar *cModuleName);
void cairo_dock_deactivate_module_and_unload (gchar *cModuleName);

void cairo_dock_configure_module (GtkWindow *pParentWindow, CairoDockModule *module, GError **erreur);


Icon *cairo_dock_find_icon_from_module (CairoDockModule *module, CairoDockContainer *pContainer);

CairoDockModule *cairo_dock_find_module_from_name (gchar *cModuleName);

CairoDockModule *cairo_dock_foreach_desklet (CairoDockForeachDeskletFunc pCallback, gpointer user_data);


GString **cairo_dock_list_modules_by_category (gboolean bActiveOnly);
void cairo_dock_update_conf_file_with_modules_full (GKeyFile *pOpenedKeyFile, gchar *cConfFile, gchar *cGroupName, gchar *cKeyNameBase, gboolean bActiveOnly);
#define cairo_dock_update_conf_file_with_available_modules2(pOpenedKeyFile, cConfFile) cairo_dock_update_conf_file_with_modules_full (pOpenedKeyFile, cConfFile, "Applets", "modules", FALSE)
#define cairo_dock_update_easy_conf_file_with_available_modules2(pOpenedKeyFile, cConfFile) cairo_dock_update_conf_file_with_modules_full (pOpenedKeyFile, cConfFile, "System", "modules", FALSE)

#define cairo_dock_update_conf_file_with_active_modules2(pOpenedKeyFile, cConfFile) cairo_dock_update_conf_file_with_modules_full (pOpenedKeyFile, cConfFile, "Applets", "modules", TRUE)


int cairo_dock_get_nb_modules (void);

void cairo_dock_update_module_order (CairoDockModule *pModule, double fOrder);


#endif
