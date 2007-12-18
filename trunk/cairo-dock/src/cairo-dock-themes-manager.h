
#ifndef __CAIRO_DOCK_THEMES_MANAGER__
#define  __CAIRO_DOCK_THEMES_MANAGER__

#include <glib.h>
#include <gtk/gtk.h>


gchar *cairo_dock_edit_themes (GHashTable **hThemeTable);


gchar *cairo_dock_get_chosen_theme (gchar *cConfFile, gboolean *bUseThemeBehaviour, gboolean *bUseThemeLaunchers);


gchar *cairo_dock_get_theme_path (gchar *cThemeName, GHashTable *hThemeTable);


void cairo_dock_load_theme (gchar *cThemePath);


void cairo_dock_mark_theme_as_modified (gboolean bModified);
gboolean cairo_dock_theme_need_save (void);


int cairo_dock_ask_initial_theme (void);

gboolean cairo_dock_manage_themes (GtkWidget *pWidget);


#endif
