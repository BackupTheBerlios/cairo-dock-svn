
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

void cairo_dock_update_conf_file_with_renderers (gchar *cConfFile);
void cairo_dock_update_launcher_conf_file_with_renderers (gchar *cConfFile);
void cairo_dock_update_easy_conf_file_with_renderers (gchar *cConfFile);


void cairo_dock_reset_all_views (void);
void cairo_dock_set_all_views_to_default (void);

int cairo_dock_get_number_of_renderers (void);


#endif
