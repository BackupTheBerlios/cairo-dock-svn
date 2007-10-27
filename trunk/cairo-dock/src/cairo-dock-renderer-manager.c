/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-renderer-manager.h"

extern gchar *g_cMainDockDefaultRendererName;
extern gchar *g_cSubDockDefaultRendererName;

static GHashTable *s_hRendererTable = NULL;  // table des modules de rendus.


CairoDockRenderer *cairo_dock_get_renderer (gchar *cRendererName)
{
	if (cRendererName != NULL)
		return g_hash_table_lookup (s_hRendererTable, cRendererName);
	else
		return g_hash_table_lookup (s_hRendererTable, CAIRO_DOCK_DEFAULT_RENDERER_NAME);
}

void cairo_dock_add_renderer (gchar *cRendererName, CairoDockRenderer *pRenderer)
{
	g_hash_table_insert (s_hRendererTable, g_strdup (cRendererName), pRenderer);
}


void cairo_dock_initialize_renderer_manager (void)
{
	g_return_if_fail (s_hRendererTable == NULL);
	
	s_hRendererTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		g_free);
	
	CairoDockRenderer *pDefaultRenderer = g_new0 (CairoDockRenderer, 1);
	pDefaultRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-basic-view", CAIRO_DOCK_SHARE_DATA_DIR);
	pDefaultRenderer->calculate_max_dock_size = cairo_dock_calculate_max_dock_size_linear;
	pDefaultRenderer->calculate_icons = cairo_dock_apply_wave_effect;
	pDefaultRenderer->render = cairo_dock_render_linear;
	pDefaultRenderer->render_optimized = cairo_dock_render_optimized_linear;
	pDefaultRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	
	cairo_dock_add_renderer (CAIRO_DOCK_DEFAULT_RENDERER_NAME, pDefaultRenderer);
}


void cairo_dock_set_renderer (CairoDock *pDock, gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = cairo_dock_get_renderer (cRendererName);
	if (pRenderer == NULL)
		pRenderer = cairo_dock_get_renderer (CAIRO_DOCK_DEFAULT_RENDERER_NAME);
	
	pDock->calculate_max_dock_size = pRenderer->calculate_max_dock_size;
	pDock->calculate_icons = pRenderer->calculate_icons;
	pDock->render = pRenderer->render;
	pDock->render_optimized = pRenderer->render_optimized;
	pDock->set_subdock_position = pRenderer->set_subdock_position;
}



void cairo_dock_update_conf_file_with_renderers (gchar *cConfFile)
{
	cairo_dock_update_conf_file_with_hash_table (cConfFile, s_hRendererTable, "Cairo Dock", "main dock view", NULL, (GHFunc) cairo_dock_write_one_renderer_name);
	cairo_dock_update_conf_file_with_hash_table (cConfFile, s_hRendererTable, "Sub-Docks", "sub-dock view", NULL, (GHFunc) cairo_dock_write_one_renderer_name);
}
