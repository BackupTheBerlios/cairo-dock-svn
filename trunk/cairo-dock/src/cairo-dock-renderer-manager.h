
#ifndef __CAIRO_DOCK_RENDERER_MANAGER__
#define  __CAIRO_DOCK_RENDERER_MANAGER__

#include <glib.h>

#include "cairo-dock-struct.h"


CairoDockRenderer *cairo_dock_get_renderer (gchar *cRendererName, gboolean bForMainDock);
void cairo_dock_register_renderer (gchar *cRendererName, CairoDockRenderer *pRenderer);
void cairo_dock_remove_renderer (gchar *cRendererName);

void cairo_dock_initialize_renderer_manager (void);

void cairo_dock_set_renderer (CairoDock *pDock, gchar *cRendererName);
void cairo_dock_set_default_renderer (CairoDock *pDock);

#define _cairo_dock_update_conf_file_with_renderers(cConfFile, cGroupName, cKeyName) cairo_dock_update_conf_file_with_hash_table (cConfFile, s_hRendererTable, cGroupName, cKeyName, NULL, (GHFunc) cairo_dock_write_one_renderer_name)
void cairo_dock_update_conf_file_with_renderers (gchar *cConfFile);
void cairo_dock_update_launcher_conf_file_with_renderers (gchar *cConfFile);


void cairo_dock_reset_all_views (void);
void cairo_dock_set_all_views_to_default (void);


#endif
