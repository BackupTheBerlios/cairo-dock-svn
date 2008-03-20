/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-default-view.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-log.h"
#include "cairo-dock-renderer-manager.h"

extern GHashTable *g_hDocksTable;

extern gchar *g_cMainDockDefaultRendererName;
extern gchar *g_cSubDockDefaultRendererName;

static GHashTable *s_hRendererTable = NULL;  // table des fonctions de rendus de dock.
static GHashTable *s_hDeskletRendererTable = NULL;  // table des fonctions de rendus des desklets.


CairoDockRenderer *cairo_dock_get_renderer (gchar *cRendererName, gboolean bForMainDock)
{
	//g_print ("%s (%s, %d)\n", __func__, cRendererName, bForMainDock);
	CairoDockRenderer *pRenderer = NULL;
	if (cRendererName != NULL)
		pRenderer = g_hash_table_lookup (s_hRendererTable, cRendererName);
	
	if (pRenderer == NULL)
	{
		const gchar *cDefaultRendererName = (bForMainDock ? g_cMainDockDefaultRendererName : g_cSubDockDefaultRendererName);
		//g_print ("  cDefaultRendererName : %s\n", cDefaultRendererName);
		if (cDefaultRendererName != NULL)
			pRenderer = g_hash_table_lookup (s_hRendererTable, cDefaultRendererName);
	}
	
	if (pRenderer == NULL)
		pRenderer = g_hash_table_lookup (s_hRendererTable, CAIRO_DOCK_DEFAULT_RENDERER_NAME);
	
	return pRenderer;
}

void cairo_dock_register_renderer (gchar *cRendererName, CairoDockRenderer *pRenderer)
{
	cd_message ("%s (%s)", __func__, cRendererName);
	g_hash_table_insert (s_hRendererTable, g_strdup (cRendererName), pRenderer);
}

void cairo_dock_remove_renderer (gchar *cRendererName)
{
	g_hash_table_remove (s_hRendererTable, cRendererName);
}


CairoDockDeskletRenderer *cairo_dock_get_desklet_renderer (gchar *cRendererName)
{
	if (cRendererName != NULL)
		return g_hash_table_lookup (s_hDeskletRendererTable, cRendererName);
	else
		return NULL;
}

void cairo_dock_register_desklet_renderer (gchar *cRendererName, CairoDockDeskletRenderer *pRenderer)
{
	cd_message ("%s (%s)", __func__, cRendererName);
	g_hash_table_insert (s_hDeskletRendererTable, g_strdup (cRendererName), pRenderer);
}

void cairo_dock_remove_desklet_renderer (gchar *cRendererName)
{
	g_hash_table_remove (s_hDeskletRendererTable, cRendererName);
}



void cairo_dock_initialize_renderer_manager (void)
{
	g_return_if_fail (s_hRendererTable == NULL);
	
	s_hRendererTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		g_free);
	
	s_hDeskletRendererTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		g_free);
	
	cairo_dock_register_default_renderer ();
}


void cairo_dock_set_renderer (CairoDock *pDock, gchar *cRendererName)
{
	g_return_if_fail (pDock != NULL);
	cd_message ("%s (%s)", __func__, cRendererName);
	CairoDockRenderer *pRenderer = cairo_dock_get_renderer (cRendererName, (pDock->iRefCount == 0));
	
	pDock->calculate_max_dock_size = pRenderer->calculate_max_dock_size;
	pDock->calculate_icons = pRenderer->calculate_icons;
	pDock->render = pRenderer->render;
	pDock->render_optimized = pRenderer->render_optimized;
	pDock->set_subdock_position = pRenderer->set_subdock_position;
	pDock->bUseReflect = pRenderer->bUseReflect;
	if (cRendererName != NULL)  // NULL n'ecrase pas le nom de l'ancienne vue.
		pDock->cRendererName = g_strdup (cRendererName);
}

void cairo_dock_set_default_renderer (CairoDock *pDock)
{
	g_return_if_fail (pDock != NULL);
	cairo_dock_set_renderer (pDock, (pDock->cRendererName != NULL ? pDock->cRendererName : NULL));  // NULL => laissera le champ cRendererName nul plutot que de mettre le nom de la vue par defaut.
}


void cairo_dock_set_desklet_renderer (CairoDockDesklet *pDesklet, CairoDockDeskletRenderer *pRenderer, cairo_t *pSourceContext, gpointer *pConfig)
{
	g_return_if_fail (pDesklet != NULL);
	cd_message ("%s (%x)", __func__, pRenderer);
	
	if (pDesklet->pRenderer != NULL && pDesklet->pRenderer->free_data != NULL)
	{
		pDesklet->pRenderer->free_data (pDesklet);
		pDesklet->pRendererData = NULL;
	}
	
	pDesklet->pRenderer = pRenderer;
	if (pRenderer != NULL && pRenderer->load_data != NULL)
	{
		cairo_t *pCairoContext = (pSourceContext != NULL ? pSourceContext : cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDesklet)));
		
		pDesklet->pRendererData = pRenderer->load_data (pDesklet, pCairoContext, pConfig);
		
		if (pSourceContext == NULL)
			cairo_destroy (pCairoContext);
	}
}

void cairo_dock_set_desklet_renderer_by_name (CairoDockDesklet *pDesklet, gchar *cRendererName, cairo_t *pSourceContext, gboolean bLoadIcons, gpointer *pConfig)
{
	cd_message ("%s (%s, %d)", __func__, cRendererName, bLoadIcons);
	g_return_if_fail (cRendererName != NULL);
	CairoDockDeskletRenderer *pRenderer = cairo_dock_get_desklet_renderer (cRendererName);
	
	cairo_t *pCairoContext = (pSourceContext != NULL ? pSourceContext : cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDesklet)));
	
	cairo_dock_set_desklet_renderer (pDesklet, pRenderer, pCairoContext, pConfig);
	
	if (bLoadIcons && pRenderer != NULL && pRenderer->load_icons != NULL )
		pRenderer->load_icons (pDesklet, pCairoContext);
	
	if (pSourceContext == NULL)
		cairo_destroy (pCairoContext);
}



#define _cairo_dock_update_conf_file_with_renderers(pOpenedKeyFile, cConfFile, cGroupName, cKeyName, bAddEmptyRenderer) cairo_dock_update_conf_file_with_hash_table (pOpenedKeyFile, cConfFile, s_hRendererTable, cGroupName, cKeyName, NULL, (GHFunc) cairo_dock_write_one_renderer_name, FALSE, bAddEmptyRenderer)

void cairo_dock_update_conf_file_with_renderers (GKeyFile *pOpenedKeyFile, gchar *cConfFile, gchar *cGroupName, gchar *cKeyName)
{
	_cairo_dock_update_conf_file_with_renderers(pOpenedKeyFile, cConfFile, cGroupName, cKeyName, TRUE);
}

void cairo_dock_update_main_conf_file_with_renderers (GKeyFile *pOpenedKeyFile, gchar *cConfFile)
{
	_cairo_dock_update_conf_file_with_renderers (pOpenedKeyFile, cConfFile, "Views", "main dock view", FALSE);
	_cairo_dock_update_conf_file_with_renderers (pOpenedKeyFile, cConfFile, "Views", "sub-dock view", FALSE);
}

void cairo_dock_update_launcher_conf_file_with_renderers (GKeyFile *pOpenedKeyFile, gchar *cConfFile)
{
	_cairo_dock_update_conf_file_with_renderers (pOpenedKeyFile, cConfFile, "Desktop Entry", "Renderer", TRUE);
}

void cairo_dock_update_easy_conf_file_with_renderers (GKeyFile *pOpenedKeyFile, gchar *cConfFile)
{
	//g_print ("%s (%s)\n", __func__, cConfFile);
	_cairo_dock_update_conf_file_with_renderers (pOpenedKeyFile, cConfFile, "Personnalisation", "main dock view", FALSE);
	_cairo_dock_update_conf_file_with_renderers (pOpenedKeyFile, cConfFile, "Personnalisation", "sub-dock view", FALSE);
}

static void _cairo_dock_reset_one_dock_view (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	cairo_dock_set_renderer (pDock, NULL);  // on met NULL plutot que CAIRO_DOCK_DEFAULT_RENDERER_NAME pour ne pas ecraser le nom de la vue.
}
void cairo_dock_reset_all_views (void)
{
	//g_print ("%s ()\n", __func__);
	g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_reset_one_dock_view, NULL);
}

static void _cairo_dock_set_one_dock_view_to_default (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	//g_print ("%s (%s)\n", __func__, cDockName);
	cairo_dock_set_default_renderer (pDock);
	cairo_dock_update_dock_size (pDock);
}
void cairo_dock_set_all_views_to_default (void)
{
	//g_print ("%s ()\n", __func__);
	g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_set_one_dock_view_to_default, NULL);
}
