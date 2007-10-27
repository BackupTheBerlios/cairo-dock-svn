
#ifndef __CAIRO_DOCK_KEYFILE_MANAGER__
#define  __CAIRO_DOCK_KEYFILE_MANAGER__

#include <glib.h>
#include <cairo-dock-struct.h>


void cairo_dock_write_keys_to_file (GKeyFile *key_file, gchar *conf_file);
void cairo_dock_flush_conf_file (GKeyFile *pKeyFile, gchar *cConfFilePath, gchar *cShareDataDirPath);

void cairo_dock_replace_comments (GKeyFile *pOriginalKeyFile, GKeyFile *pReplacementKeyFile);
void cairo_dock_replace_key_values (GKeyFile *pOriginalKeyFile, GKeyFile *pReplacementKeyFile, gboolean bUseOriginalKeys, gchar iIdentifier);

void cairo_dock_write_one_name (gchar *cName, gpointer value, GString *pString);
void cairo_dock_write_one_name_description (gchar *cName, gchar *cDescriptionFilePath, GString *pString);
void cairo_dock_write_one_module_name (gchar *cName, CairoDockModule *pModule, GString *pString);
void cairo_dock_write_one_theme_name (gchar *cName, gchar *cThemePath, GString *pString);
void cairo_dock_write_one_renderer_name (gchar *cName, CairoDockRenderer *pRenderer, GString *pString);
void cairo_dock_update_conf_file_with_hash_table (gchar *cConfFile, GHashTable *pModuleTable, gchar *cGroupName, gchar *cKeyName, gchar *cNewUsefullComment, GHFunc pWritingFunc);

void cairo_dock_apply_translation_on_conf_file (gchar *cConfFilePath, gchar *cCommentsFilePath);
void cairo_dock_replace_values_in_conf_file (gchar *cConfFilePath, GKeyFile *pValidKeyFile, gboolean bUseFileKeys, gchar iIdentifier);
void cairo_dock_replace_keys_by_identifier (gchar *cConfFilePath, gchar *cReplacementConfFilePath, gchar iIdentifier);

GHashTable *cairo_dock_list_available_translations (gchar *cTranslationsDir, gchar *cFilePrefix, GError **erreur);
gchar *cairo_dock_get_conf_file_language (GKeyFile *pKeyFile);


#endif
