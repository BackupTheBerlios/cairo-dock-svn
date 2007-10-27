
#ifndef __CAIRO_DOCK_RENDERER_MANAGER__
#define  __CAIRO_DOCK_RENDERER_MANAGER__

#include <glib.h>

#include "cairo-dock-struct.h"


CairoDockRenderer *cairo_dock_get_renderer (gchar *cRendererName);
void cairo_dock_add_renderer (gchar *cRendererName, CairoDockRenderer *pRenderer);

void cairo_dock_initialize_renderer_manager (void);

void cairo_dock_set_renderer (CairoDock *pDock, gchar *cRendererName);
#define cairo_dock_set_default_renderer(pDock) cairo_dock_set_renderer (pDock, (pDock->iRefCount > 0 ? g_cSubDockDefaultRendererName : g_cMainDockDefaultRendererName));


void cairo_dock_update_conf_file_with_renderers (gchar *cConfFile);


#endif
