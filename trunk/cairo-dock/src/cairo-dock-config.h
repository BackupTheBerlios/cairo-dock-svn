
#ifndef __CAIRO_DOCK_CONFIG__
#define  __CAIRO_DOCK_CONFIG__

#include <glib.h>

#include "cairo-dock-struct.h"


guint cairo_dock_get_number_from_name (gchar *cName, const gchar **tNamesList);

gboolean cairo_dock_get_boolean_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gboolean bDefaultValue);
int cairo_dock_get_integer_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int iDefaultValue);
double cairo_dock_get_double_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double fDefaultValue);
gchar *cairo_dock_get_string_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, const gchar *cDefaultValue);
void cairo_dock_get_integer_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int *iValueBuffer, int iNbElements, int *iDefaultValues);
void cairo_dock_get_double_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double *fValueBuffer, int iNbElements, double *fDefaultValues);
gchar **cairo_dock_get_string_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gsize *length, gchar *cDefaultValues);
CairoDockAnimationType cairo_dock_get_animation_type_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, const gchar *cDefaultAnimation);

void cairo_dock_read_conf_file (gchar *cConfFilePath, CairoDock *pDock);

gboolean cairo_dock_edit_conf_file (GtkWidget *pWidget, gchar *conf_file, gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CairoDockConfigFunc pConfigFunc, gpointer data, GFunc pFreeUserDataFunc);


void cairo_dock_update_conf_file_with_position (gchar *cConfFilePath, int x, int y);

void cairo_dock_update_conf_file_with_translations (gchar *cConfFile, gchar *cTranslationsDir);


CairoDockDesktopEnv cairo_dock_guess_environment (void);


#endif
