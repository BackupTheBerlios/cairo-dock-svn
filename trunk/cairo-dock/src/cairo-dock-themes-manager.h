
#ifndef __CAIRO_DOCK_THEMES_MANAGER__
#define  __CAIRO_DOCK_THEMES_MANAGER__

#include <glib.h>
#include <gtk/gtk.h>


gchar *cairo_dock_edit_themes (gchar *cLanguage, GHashTable **hThemeTable);


gchar *cairo_dock_get_chosen_theme (gchar *cConfFile);


gchar *cairo_dock_get_theme_path (gchar *cThemeName, GHashTable *hThemeTable);


gchar *cairo_dock_load_theme (gchar *cThemePath);


gchar *cairo_dock_get_last_theme_name (gchar *cCairoDockDataDir);



gchar *cairo_dock_ask_initial_theme (void);

void cairo_dock_manage_themes (GtkWidget *pWidget);


#endif
