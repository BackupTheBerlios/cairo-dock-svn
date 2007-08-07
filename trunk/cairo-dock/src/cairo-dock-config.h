
#ifndef __CAIRO_DOCK_CONFIG__
#define  __CAIRO_DOCK_CONFIG__

#include <glib.h>

#include "cairo-dock-struct.h"


int cairo_dock_get_number_from_name (gchar *cName, gchar **tNamesList);

void cairo_dock_read_conf_file (gchar *conf_file, CairoDock *pDock);


gboolean cairo_dock_edit_conf_file (GtkWidget *pWidget, gchar *conf_file, gchar *cTitle);


void cairo_dock_write_keys_to_file (GKeyFile *key_file, gchar *conf_file);

void cairo_dock_update_conf_file_with_position (gchar *cConfFilePath, int x, int y);

void cairo_dock_update_conf_file_with_hash_table (gchar *cConfFile, GHashTable *pModuleTable, gchar *cGroupName, gchar *cKeyName, int iNbAvailableChoicess, gchar *cUsefullComment);
void cairo_dock_update_conf_file_with_modules (gchar *cConfFile, GHashTable *pModuleTable);

void cairo_dock_update_conf_file_with_translations (gchar *cConfFile, gchar *cTranslationsDir);

void cairo_dock_update_conf_file_with_active_modules (gchar *cConfFile, GList *pIconList, GHashTable *pModuleTable);


void cairo_dock_apply_translation_on_conf_file (gchar *cConfFilePath, gchar *cCommentsFilePath);

GHashTable *cairo_dock_list_available_translations (gchar *cTranslationsDir, gchar *cFilePrefix, GError **erreur);

#endif
