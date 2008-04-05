
#ifndef __CAIRO_DOCK_GUI_FACTORY__
#define  __CAIRO_DOCK_GUI_FACTORY__

#include <gtk/gtk.h>


GtkWidget *cairo_dock_generate_advanced_ihm_from_keyfile (GKeyFile *pKeyFile, gchar *cTitle, GtkWindow *pParentWindow, GSList **pWidgetList, gboolean bApplyButtonPresent, gchar iIdentifier, gchar *cPresentedGroup, gboolean bSwitchButtonPresent, gchar *cButtonConvert, gchar *cGettextDomain, GPtrArray *pDataGarbage);

gboolean cairo_dock_is_advanced_keyfile (GKeyFile *pKeyFile);

GtkWidget *cairo_dock_generate_basic_ihm_from_keyfile (gchar *cConfFilePath, gchar *cTitle, GtkWindow *pParentWindow, GtkTextBuffer **pTextBuffer, gboolean bApplyButtonPresent, gboolean bSwitchButtonPresent, gchar *cButtonConvert, gchar *cGettextDomain);


void cairo_dock_update_keyfile_from_widget_list (GKeyFile *pKeyFile, GSList *pWidgetList);


void cairo_dock_free_generated_widget_list (GSList *pWidgetList);


#endif
