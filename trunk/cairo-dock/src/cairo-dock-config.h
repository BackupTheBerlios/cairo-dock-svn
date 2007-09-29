
#ifndef __CAIRO_DOCK_CONFIG__
#define  __CAIRO_DOCK_CONFIG__

#include <glib.h>

#include "cairo-dock-struct.h"


guint cairo_dock_get_number_from_name (gchar *cName, gchar **tNamesList);
const gchar **cairo_dock_get_animations_names (void);
CairoDockAnimationType cairo_dock_get_animation_type_from_name (gchar *cAnimationName);

gboolean cairo_dock_get_boolean_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gboolean bDefaultValue);
int cairo_dock_get_integer_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int iDefaultValue);
double cairo_dock_get_double_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double fDefaultValue);
gchar *cairo_dock_get_string_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gchar *cDefaultValue);
void cairo_dock_get_integer_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int *iValueBuffer, int iNbElements, int *iDefaultValues);
void cairo_dock_get_double_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double *fValueBuffer, int iNbElements, double *fDefaultValues);
gchar **cairo_dock_get_string_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gsize *length, gchar *cDefaultValues);



void cairo_dock_read_conf_file (gchar *cConfFilePath, CairoDock *pDock);

void cairo_dock_flush_conf_file (GKeyFile *pKeyFile, gchar *cConfFilePath, gchar *cShareDataDirPath);

gboolean cairo_dock_edit_conf_file (GtkWidget *pWidget, gchar *conf_file, gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CairoDockConfigFunc pConfigFunc, gpointer data, GFunc pFreeUserDataFunc);


void cairo_dock_write_keys_to_file (GKeyFile *key_file, gchar *conf_file);

void cairo_dock_update_conf_file_with_position (gchar *cConfFilePath, int x, int y);


void cairo_dock_write_one_name (gchar *cName, gpointer value, GString *pString);
void cairo_dock_write_one_name_description (gchar *cName, gchar *cDescriptionFilePath, GString *pString);
void cairo_dock_write_one_module_name (gchar *cName, CairoDockModule *pModule, GString *pString);
void cairo_dock_write_one_theme_name (gchar *cName, gchar *cThemePath, GString *pString);
void cairo_dock_update_conf_file_with_hash_table (gchar *cConfFile, GHashTable *pModuleTable, gchar *cGroupName, gchar *cKeyName, gchar *cNewUsefullComment, GHFunc pWritingFunc);
void cairo_dock_update_conf_file_with_modules (gchar *cConfFile, GHashTable *pModuleTable);

void cairo_dock_update_conf_file_with_translations (gchar *cConfFile, gchar *cTranslationsDir);

void cairo_dock_update_conf_file_with_active_modules (gchar *cConfFile, GList *pIconList, GHashTable *pModuleTable);


void cairo_dock_apply_translation_on_conf_file (gchar *cConfFilePath, gchar *cCommentsFilePath);
void cairo_dock_replace_values_in_conf_file (gchar *cConfFilePath, GKeyFile *pValidKeyFile, gboolean bUseFileKeys, gchar iIdentifier);
void cairo_dock_replace_keys_by_identifier (gchar *cConfFilePath, gchar *cReplacementConfFilePath, gchar iIdentifier);

GHashTable *cairo_dock_list_available_translations (gchar *cTranslationsDir, gchar *cFilePrefix, GError **erreur);


CairoDockDesktopEnv cairo_dock_guess_environment (void);

#endif
