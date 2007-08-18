
#ifndef __CAIRO_DOCK_KEYFILE_MANAGER__
#define  __CAIRO_DOCK_KEYFILE_MANAGER__

#include <gtk/gtk.h>


GtkWidget *cairo_dock_generate_advanced_ihm_from_keyfile (GKeyFile *pKeyFile, gchar *cTitle, GtkWidget *pParentWidget, GSList **pWidgetList, gboolean bApplyButtonPresent, gboolean bFullConfig);

gboolean cairo_dock_is_advanced_keyfile (GKeyFile *pKeyFile);


GtkWidget *cairo_dock_generate_basic_ihm_from_keyfile (gchar *cConfFilePath, gchar *cTitle, GtkWidget *pParentWidget, GtkTextBuffer **pTextBuffer, gboolean bApplyButtonPresent);


void cairo_dock_update_keyfile_from_widget_list (GKeyFile *pKeyFile, GSList *pWidgetList);


void cairo_dock_free_generated_widget_list (GSList *pWidgetList);


void cairo_dock_replace_comments (GKeyFile *pOriginalKeyFile, GKeyFile *pReplacementKeyFile);
void cairo_dock_replace_key_values (GKeyFile *pOriginalKeyFile, GKeyFile *pReplacementKeyFile);

#endif

