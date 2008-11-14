
#ifndef __CAIRO_DOCK_MODULES__
#define  __CAIRO_DOCK_MODULES__

#include <glib.h>

#include "cairo-dock-struct.h"
G_BEGIN_DECLS


void cairo_dock_initialize_module_manager (gchar *cModuleDirPath);

/**
*Verifie que le fichier de conf d'un plug-in est bien present dans le repertoire utilisateur du plug-in, sinon le copie a partir du fichier de conf fournit lors de l'installation. Cree au besoin le repertoire utilisateur du plug-in.
*@param cUserDataDirName le nom du repertoire utilisateur du plug-in.
*@param cShareDataDir le chemin du repertoire d'installation du plug-in.
*@param cConfFileName : le nom du fichier de conf fournit a l'installation.
*@return Le chemin du fichier de conf en espace utilisateur, ou NULL si le fichier n'a pu etre ni trouve, ni cree.
*/
gchar *cairo_dock_check_conf_file_exists (CairoDockVisitCard *pVisitCard);

void cairo_dock_free_visit_card (CairoDockVisitCard *pVisitCard);

CairoDockModule * cairo_dock_load_module (gchar *cSoFilePath, GHashTable *pModuleTable, GError **erreur);

void cairo_dock_preload_module_from_directory (gchar *cModuleDirPath, GHashTable *pModuleTable, GError **erreur);



void cairo_dock_activate_modules_from_list (gchar **cActiveModuleList, double fTime);

void cairo_dock_deactivate_old_modules (double fTime);


void cairo_dock_free_module (CairoDockModule *module);

GKeyFile *cairo_dock_pre_read_module_instance_config (CairoDockModuleInstance *pInstance, CairoDockMinimalAppletConfig *pMinimalConfig);

void cairo_dock_activate_module (CairoDockModule *module, GError **erreur);

void cairo_dock_deactivate_module (CairoDockModule *module);

void cairo_dock_reload_module_instance (CairoDockModuleInstance *pInstance, gboolean bReloadAppletConf);
void cairo_dock_reload_module (CairoDockModule *module, gboolean bReloadAppletConf);


void cairo_dock_deactivate_all_modules (void);

void cairo_dock_activate_module_and_load (gchar *cModuleName);
void cairo_dock_deactivate_module_instance_and_unload (CairoDockModuleInstance *pInstance);
void cairo_dock_deactivate_module_and_unload (gchar *cModuleName);

void cairo_dock_configure_module_instance (GtkWindow *pParentWindow, CairoDockModuleInstance *pModuleInstance, GError **erreur);
void cairo_dock_configure_inactive_module (GtkWindow *pParentWindow, CairoDockModule *pModule);
void cairo_dock_configure_module (GtkWindow *pParentWindow, const gchar *cModuleName);


CairoDockModule *cairo_dock_find_module_from_name (const gchar *cModuleName);

CairoDockModuleInstance *cairo_dock_foreach_desklet (CairoDockForeachDeskletFunc pCallback, gpointer user_data);


GString **cairo_dock_list_modules_by_category (gboolean bActiveOnly);
void cairo_dock_update_conf_file_with_modules_full (GKeyFile *pOpenedKeyFile, gchar *cConfFile, gchar *cGroupName, gchar *cKeyNameBase, gboolean bActiveOnly);
#define cairo_dock_update_conf_file_with_available_modules2(pOpenedKeyFile, cConfFile) cairo_dock_update_conf_file_with_modules_full (pOpenedKeyFile, cConfFile, "Applets", "modules", FALSE)
#define cairo_dock_update_easy_conf_file_with_available_modules2(pOpenedKeyFile, cConfFile) cairo_dock_update_conf_file_with_modules_full (pOpenedKeyFile, cConfFile, "System", "modules", FALSE)

#define cairo_dock_update_conf_file_with_active_modules2(pOpenedKeyFile, cConfFile) cairo_dock_update_conf_file_with_modules_full (pOpenedKeyFile, cConfFile, "Applets", "modules", TRUE)


int cairo_dock_get_nb_modules (void);

void cairo_dock_update_module_instance_order (CairoDockModuleInstance *pModuleInstance, double fOrder);


CairoDockModuleInstance *cairo_dock_instanciate_module (CairoDockModule *pModule, gchar *cConfFilePah);
void cairo_dock_free_module_instance (CairoDockModuleInstance *pInstance);
void cairo_dock_stop_module_instance (CairoDockModuleInstance *pInstance);
void cairo_dock_deinstanciate_module (CairoDockModuleInstance *pInstance);

void cairo_dock_remove_module_instance (CairoDockModuleInstance *pInstance);
void cairo_dock_add_module_instance (CairoDockModule *pModule);

void cairo_dock_read_module_config (GKeyFile *pKeyFile, CairoDockModuleInstance *pInstance);


G_END_DECLS
#endif
