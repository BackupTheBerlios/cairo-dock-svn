
#ifndef __CAIRO_DOCK_RENDERER_MANAGER__
#define  __CAIRO_DOCK_RENDERER_MANAGER__

#include <glib.h>

#include "cairo-dock-struct.h"


CairoDockRenderer *cairo_dock_get_renderer (const gchar *cRendererName, gboolean bForMainDock);
void cairo_dock_register_renderer (const gchar *cRendererName, CairoDockRenderer *pRenderer);
void cairo_dock_remove_renderer (const gchar *cRendererName);

CairoDeskletRenderer *cairo_dock_get_desklet_renderer (const gchar *cRendererName);
void cairo_dock_register_desklet_renderer (const gchar *cRendererName, CairoDeskletRenderer *pRenderer);
void cairo_dock_remove_desklet_renderer (const gchar *cRendererName);
void cairo_dock_predefine_desklet_renderer_config (CairoDeskletRenderer *pRenderer, const gchar *cConfigName, CairoDeskletRendererConfig 
*pConfig);
CairoDeskletRendererConfig *cairo_dock_get_desklet_renderer_predefined_config (const gchar *cRendererName, const gchar *cConfigName);

void cairo_dock_initialize_renderer_manager (void);

void cairo_dock_set_renderer (CairoDock *pDock, const gchar *cRendererName);
void cairo_dock_set_default_renderer (CairoDock *pDock);

void cairo_dock_set_desklet_renderer (CairoDesklet *pDesklet, CairoDeskletRenderer *pRenderer, cairo_t *pSourceContext, gboolean bLoadIcons, CairoDeskletRendererConfig *pConfig);
void cairo_dock_set_desklet_renderer_by_name (CairoDesklet *pDesklet, const gchar *cRendererName, cairo_t *pSourceContext, gboolean bLoadIcons, CairoDeskletRendererConfig *pConfig);


void cairo_dock_update_renderer_list_for_gui (void);


#endif
