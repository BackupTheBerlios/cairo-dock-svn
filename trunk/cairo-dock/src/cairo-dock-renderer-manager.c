/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-keyfile-utilities.h"
#include "cairo-dock-default-view.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-log.h"
#include "cairo-dock-gui-factory.h"
#include "cairo-dock-renderer-manager.h"

extern GHashTable *g_hDocksTable;

extern gchar *g_cMainDockDefaultRendererName;
extern gchar *g_cSubDockDefaultRendererName;

static GHashTable *s_hRendererTable = NULL;  // table des fonctions de rendus de dock.
static GHashTable *s_hDeskletRendererTable = NULL;  // table des fonctions de rendus des desklets.


CairoDockRenderer *cairo_dock_get_renderer (const gchar *cRendererName, gboolean bForMainDock)
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

void cairo_dock_register_renderer (const gchar *cRendererName, CairoDockRenderer *pRenderer)
{
	cd_message ("%s (%s)", __func__, cRendererName);
	g_hash_table_insert (s_hRendererTable, g_strdup (cRendererName), pRenderer);
}

void cairo_dock_remove_renderer (const gchar *cRendererName)
{
	g_hash_table_remove (s_hRendererTable, cRendererName);
}


CairoDeskletRenderer *cairo_dock_get_desklet_renderer (const gchar *cRendererName)
{
	if (cRendererName != NULL)
		return g_hash_table_lookup (s_hDeskletRendererTable, cRendererName);
	else
		return NULL;
}

void cairo_dock_register_desklet_renderer (const gchar *cRendererName, CairoDeskletRenderer *pRenderer)
{
	cd_message ("%s (%s)", __func__, cRendererName);
	g_hash_table_insert (s_hDeskletRendererTable, g_strdup (cRendererName), pRenderer);
}

void cairo_dock_remove_desklet_renderer (const gchar *cRendererName)
{
	g_hash_table_remove (s_hDeskletRendererTable, cRendererName);
}

void cairo_dock_predefine_desklet_renderer_config (CairoDeskletRenderer *pRenderer, const gchar *cConfigName, CairoDeskletRendererConfig *pConfig)
{
	g_return_if_fail (cConfigName != NULL && pConfig != NULL);
	CairoDeskletRendererPreDefinedConfig *pPreDefinedConfig = g_new (CairoDeskletRendererPreDefinedConfig, 1);
	pPreDefinedConfig->cName = g_strdup (cConfigName);
	pPreDefinedConfig->pConfig = pConfig;
	pRenderer->pPreDefinedConfigList = g_list_prepend (pRenderer->pPreDefinedConfigList, pPreDefinedConfig);
}

CairoDeskletRendererConfig *cairo_dock_get_desklet_renderer_predefined_config (const gchar *cRendererName, const gchar *cConfigName)
{
	CairoDeskletRenderer *pRenderer = cairo_dock_get_desklet_renderer (cRendererName);
	g_return_val_if_fail (pRenderer != NULL && cConfigName != NULL, NULL);
	
	GList *c;
	CairoDeskletRendererPreDefinedConfig *pPreDefinedConfig;
	for (c = pRenderer->pPreDefinedConfigList; c != NULL; c = c->next)
	{
		pPreDefinedConfig = c->data;
		if (strcmp (pPreDefinedConfig->cName, cConfigName) == 0)
			return pPreDefinedConfig->pConfig;
	}
	return NULL;
}


void cairo_dock_initialize_renderer_manager (void)
{
	g_return_if_fail (s_hRendererTable == NULL);
	cd_message ("");
	
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


void cairo_dock_set_renderer (CairoDock *pDock, const gchar *cRendererName)
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


void cairo_dock_set_desklet_renderer (CairoDesklet *pDesklet, CairoDeskletRenderer *pRenderer, cairo_t *pSourceContext, gboolean bLoadIcons, CairoDeskletRendererConfig *pConfig)
{
	g_return_if_fail (pDesklet != NULL);
	cd_debug ("%s (%x)", __func__, pRenderer);
	
	if (pDesklet->pRenderer != NULL && pDesklet->pRenderer->free_data != NULL)
	{
		pDesklet->pRenderer->free_data (pDesklet);
		pDesklet->pRendererData = NULL;
	}
	
	pDesklet->pRenderer = pRenderer;
	
	if (pRenderer != NULL)
	{
		cairo_t *pCairoContext = (pSourceContext != NULL ? pSourceContext : cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDesklet)));
		
		if (pRenderer->configure != NULL)
			pDesklet->pRendererData = pRenderer->configure (pDesklet, pCairoContext, pConfig);
		
		if (bLoadIcons && pRenderer->load_icons != NULL)
			pRenderer->load_icons (pDesklet, pCairoContext);
		
		if (pRenderer->load_data != NULL)
			pRenderer->load_data (pDesklet, pCairoContext);
		
		if (pSourceContext == NULL)
			cairo_destroy (pCairoContext);
	}
}

void cairo_dock_set_desklet_renderer_by_name (CairoDesklet *pDesklet, const gchar *cRendererName, cairo_t *pSourceContext, gboolean bLoadIcons, CairoDeskletRendererConfig *pConfig)
{
	cd_message ("%s (%s, %d)", __func__, cRendererName, bLoadIcons);
	CairoDeskletRenderer *pRenderer = (cRendererName != NULL ? cairo_dock_get_desklet_renderer (cRendererName) : NULL);
	
	cairo_t *pCairoContext = (pSourceContext != NULL ? pSourceContext : cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDesklet)));
	
	cairo_dock_set_desklet_renderer (pDesklet, pRenderer, pCairoContext, bLoadIcons, pConfig);
	
	if (pSourceContext == NULL)
		cairo_destroy (pCairoContext);
}



void cairo_dock_update_renderer_list_for_gui (void)
{
	cairo_dock_build_renderer_list_for_gui (s_hRendererTable);
}
