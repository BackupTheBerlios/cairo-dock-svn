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
#include "cairo-dock-default-view.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-renderer-manager.h"

extern GHashTable *g_hDocksTable;

extern gchar *g_cMainDockDefaultRendererName;
extern gchar *g_cSubDockDefaultRendererName;

static GHashTable *s_hRendererTable = NULL;  // table des modules de rendus.


CairoDockRenderer *cairo_dock_get_renderer (gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = NULL;
	if (cRendererName != NULL)
		pRenderer = g_hash_table_lookup (s_hRendererTable, cRendererName);
	
	if (pRenderer == NULL)
		pRenderer = g_hash_table_lookup (s_hRendererTable, CAIRO_DOCK_DEFAULT_RENDERER_NAME);
	
	return pRenderer;
}

void cairo_dock_register_renderer (gchar *cRendererName, CairoDockRenderer *pRenderer)
{
	g_hash_table_insert (s_hRendererTable, g_strdup (cRendererName), pRenderer);
}

void cairo_dock_remove_renderer (gchar *cRendererName)
{
	g_hash_table_remove (s_hRendererTable, cRendererName);
}


void cairo_dock_initialize_renderer_manager (void)
{
	g_return_if_fail (s_hRendererTable == NULL);
	
	s_hRendererTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		g_free);
	
	cairo_dock_register_default_renderer ();
}


void cairo_dock_set_renderer (CairoDock *pDock, gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = cairo_dock_get_renderer (cRendererName);
	
	pDock->calculate_max_dock_size = pRenderer->calculate_max_dock_size;
	pDock->calculate_icons = pRenderer->calculate_icons;
	pDock->render = pRenderer->render;
	pDock->render_optimized = pRenderer->render_optimized;
	pDock->set_subdock_position = pRenderer->set_subdock_position;
	pDock->bUseReflect = pRenderer->bUseReflect;
}



void cairo_dock_update_conf_file_with_renderers (gchar *cConfFile)
{
	cairo_dock_update_conf_file_with_hash_table (cConfFile, s_hRendererTable, "Cairo Dock", "main dock view", NULL, (GHFunc) cairo_dock_write_one_renderer_name);
	cairo_dock_update_conf_file_with_hash_table (cConfFile, s_hRendererTable, "Sub-Docks", "sub-dock view", NULL, (GHFunc) cairo_dock_write_one_renderer_name);
}



static void _cairo_dock_reset_one_dock_view (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	cairo_dock_set_renderer (pDock, CAIRO_DOCK_DEFAULT_RENDERER_NAME);
}
void cairo_dock_reset_all_views (void)
{
	g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_reset_one_dock_view, NULL);
}

static void _cairo_dock_set_one_dock_view_to_default (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	cairo_dock_set_default_renderer (pDock);
	//pDock->calculate_max_dock_size (pDock);
	cairo_dock_update_dock_size (pDock);
}
void cairo_dock_set_all_views_to_default (void)
{
	//g_print ("%s ()\n", __func__);
	g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_set_one_dock_view_to_default, NULL);
}
