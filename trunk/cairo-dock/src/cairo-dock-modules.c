/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <cairo.h>

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-load.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-modules.h"

extern gchar *g_cCairoDockDataDir;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;
extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern GHashTable *g_hAppliTable;
extern GHashTable *g_hXWindowTable;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];


gchar *cairo_dock_extract_module_name_from_path (gchar *cSoFilePath)
{
	gchar *ptr = g_strrstr (cSoFilePath, "/");
	if (ptr == NULL)
		ptr = cSoFilePath;
	else
		ptr ++;
	if (strncmp (ptr, "lib", 3) == 0)
		ptr += 3;
	
	gchar *cModuleName = g_strdup (ptr);
	
	ptr = g_strrstr (cModuleName, ".so");
	if (ptr != NULL)
		*ptr = '\0';
	
	ptr = cModuleName;
	while ((ptr = g_strrstr (ptr, "-")) != NULL)
	{
		*ptr = '_';
	}
	
	return cModuleName;
}

static void cairo_dock_open_module (CairoDockModule *pCairoDockModule, GError **erreur)
{
	GModule *module = g_module_open (pCairoDockModule->cSoFilePath, G_MODULE_BIND_LAZY);
	if (!module)
	{
		g_set_error (erreur, 1, 1, "Attention : while opening module '%s' : (%s)", pCairoDockModule->cSoFilePath, g_module_error ());
		return ;
	}
	
	
	CairoDockModuleInit function_init;
	gchar *cInitFuncName = g_strdup_printf ("%s_init", pCairoDockModule->cModuleName);
	if (!g_module_symbol (module, cInitFuncName, (gpointer) &function_init))
	{
		g_set_error (erreur, 1, 1, "Attention : the module '%s' is not valid : (%s)", pCairoDockModule->cSoFilePath, g_module_error ());
		g_free (cInitFuncName);
		if (!g_module_close (module))
			g_warning ("%s: %s", pCairoDockModule->cSoFilePath, g_module_error ());
		return ;
	}
	g_free (cInitFuncName);
	
	
	CairoDockModuleStop function_stop;
	gchar *cStopFuncName = g_strdup_printf ("%s_stop", pCairoDockModule->cModuleName);
	if (!g_module_symbol (module, cStopFuncName, (gpointer) &function_stop))
	{
		function_stop = NULL;
	}
	g_free (cStopFuncName);
	
	
	CairoDockModuleConfig function_config;
	gchar *cConfigFuncName = g_strdup_printf ("%s_config", pCairoDockModule->cModuleName);
	if (!g_module_symbol (module, cConfigFuncName, (gpointer) &function_config))
	{
		function_config = NULL;
	}
	g_free (cConfigFuncName);
	
	
	CairoDockModuleAction function_action;
	gchar *cActionFuncName = g_strdup_printf ("%s_action", pCairoDockModule->cModuleName);
	if (!g_module_symbol (module, cActionFuncName, (gpointer) &function_action))
	{
		function_action = NULL;
	}
	g_free (cActionFuncName);
	
	
	pCairoDockModule->pModule = module;
	pCairoDockModule->initModule = function_init;
	pCairoDockModule->stopModule = function_stop;
	pCairoDockModule->configModule = function_config;
	pCairoDockModule->actionModule = function_action;
}


CairoDockModule * cairo_dock_load_module (gchar *cSoFilePath, GHashTable *pModuleTable, GError **erreur)  // cFilePath vers un fichier de la forme 'libtruc.so'. Le module est rajoute en debut de la liste si il n'est pas deja dedans. La liste peut neanmoins etre NULL.
{
	//g_print ("%s (%s)\n", __func__, cSoFilePath);
	if (cSoFilePath == NULL)  // g_module_open () plante si 'cFilePath' est NULL.
	{
		g_set_error (erreur, 1, 1, "%s () : no such module", __func__);
		return NULL;
	}
	
	
	gchar *cModuleName = cairo_dock_extract_module_name_from_path (cSoFilePath);
	if (pModuleTable != NULL && g_hash_table_lookup (pModuleTable, cModuleName) != NULL)
	{
		g_set_error (erreur, 1, 1, "%s () : this module has already been loaded", __func__);
		return NULL;
	}
	
	
	CairoDockModule *pCairoDockModule = g_new0 (CairoDockModule, 1);
	pCairoDockModule->cModuleName = cModuleName;
	pCairoDockModule->cSoFilePath = g_strdup (cSoFilePath);
	
	GError *tmp_erreur = NULL;
	cairo_dock_open_module (pCairoDockModule, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	
	if (pModuleTable != NULL)
		g_hash_table_insert (pModuleTable, cModuleName, pCairoDockModule);
	
	return pCairoDockModule;
}


void cairo_dock_preload_module_from_directory (gchar *cModuleDirPath, GHashTable *pModuleTable, GError **erreur)
{
	//g_print ("%s (%s)\n", __func__, cModuleDirPath);
	GError *tmp_erreur = NULL;
	GDir *dir = g_dir_open (cModuleDirPath, 0, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return ;
	}
	
	const gchar *cFileName;
	gchar *cFilePath;
	CairoDockModule *pModule;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		cFilePath = g_strdup_printf ("%s/%s", cModuleDirPath, cFileName);
		if (g_str_has_suffix (cFilePath, ".so"))
		{
			pModule = cairo_dock_load_module (cFilePath, pModuleTable, &tmp_erreur);
			if (tmp_erreur != NULL)
			{
				g_print ("Attention : %s", tmp_erreur->message);
				g_error_free (tmp_erreur);
				tmp_erreur = NULL;
			}
			g_free (cFilePath);
		}
	}
	while (1);
	g_dir_close (dir);
}


void cairo_dock_activate_modules_from_list (gchar **cActiveModuleList, GHashTable *pModuleTable, CairoDock *pDock)
{
	g_return_if_fail (cActiveModuleList != NULL);
	GError *erreur = NULL;
	gchar *cModuleName;
	CairoDockModule *pModule;
	int i = 0, iOrder = 0;
	while (cActiveModuleList[i] != NULL)
	{
		cModuleName = cActiveModuleList[i];
		//g_print (" + %s\n", cModuleName);
		pModule = g_hash_table_lookup (pModuleTable, cModuleName);
		if (pModule != NULL)
		{
			Icon *pIcon = cairo_dock_activate_module (pModule, pDock->pWidget, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			if (pIcon != NULL)
			{
				pIcon->pModule = pModule;
				pIcon->fOrder = iOrder ++;
				cairo_dock_insert_icon_in_dock (pIcon, pDock, FALSE, FALSE);
			}
		}
		i ++;
	}
}



void cairo_dock_free_module (CairoDockModule *module)
{
	if (module == NULL)
		return ;
	
	cairo_dock_deactivate_module (module);
	
	g_free (module->cModuleName);
	g_free (module->cSoFilePath);
	g_free (module);
}

Icon * cairo_dock_activate_module (CairoDockModule *module, GtkWidget *pWidget, GError **erreur)
{
	if (module == NULL)
	{
		g_set_error (erreur, 1, 1, "%s () : empty module !", __func__);
		return NULL;
	}
	
	if (module->bActive)
	{
		g_set_error (erreur, 1, 1, "%s () : this module is already active !", __func__);
		return NULL;
	}
	
	GError *tmp_erreur = NULL;
	if (module->pModule == NULL)
	{
		cairo_dock_open_module (module, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
	}
	
	
	Icon *icon = module->initModule (pWidget, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	module->bActive = TRUE;
	return icon;
}

void cairo_dock_deactivate_module (CairoDockModule *module)
{
	g_return_if_fail (module != NULL);
	
	if (module->stopModule != NULL && module->bActive)
	{
		module->stopModule ();
	}
	
	module->bActive = FALSE;
	
	g_module_close (module->pModule);
	module->pModule = NULL;
	module->initModule = NULL;
	module->stopModule = NULL;
	module->configModule = NULL;
}

void cairo_dock_configure_module (CairoDockModule *module, CairoDock *pDock, GError **erreur)
{
	g_return_if_fail (module != NULL);
	
	if (module->configModule == NULL)
		return;
	
	
	gboolean configuration_ok = module->configModule ();
	
	if (configuration_ok && module->bActive)  // si le module etait actif, on le re-active avec sa nouvelle configuration.
	{
		if (module->stopModule != NULL)
		{
			module->stopModule ();
		}
		
		GError *tmp_erreur = NULL;
		Icon *pNewIcon = module->initModule (pDock->pWidget, &tmp_erreur);
		if (pNewIcon != NULL)
			pNewIcon->pModule = module;
		if (tmp_erreur != NULL)
		{
			module->bActive = FALSE;
			g_propagate_error (erreur, tmp_erreur);
			return ;
		}
		
		Icon *pOldIcon = cairo_dock_find_icon_from_module (module, pDock->icons);
		if (pOldIcon != NULL)
		{
			pOldIcon->pModule = NULL;
			if (pNewIcon != NULL)
				pNewIcon->fOrder = pOldIcon->fOrder;
			cairo_dock_remove_icon_from_dock (pDock, pOldIcon);
			cairo_dock_free_icon (pOldIcon);
		}
		
		if (pNewIcon != NULL)
		{
			cairo_dock_insert_icon_in_dock (pNewIcon, pDock, TRUE, FALSE);
			cairo_dock_redraw_my_icon (pNewIcon, pDock->pWidget);
		}
	}
}



Icon *cairo_dock_find_icon_from_module (CairoDockModule *module, GList *pIconList)
{
	Icon *icon;
	GList *ic;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->pModule == module)
			return icon;
	}
	return NULL;
}

