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
#include "cairo-dock-config.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-modules.h"

extern GHashTable *g_hModuleTable;
extern gchar *g_cConfFile;


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
	
	g_free (pCairoDockModule->cReadmeFilePath);
	pCairoDockModule->cReadmeFilePath = NULL;
	
	CairoDockModulePreInit function_pre_init;
	gchar *cPreInitFuncName = g_strdup_printf ("%s_pre_init", pCairoDockModule->cModuleName);
	if (!g_module_symbol (module, cPreInitFuncName, (gpointer) &function_pre_init))
	{
		function_pre_init = NULL;
	}
	else
	{
		pCairoDockModule->cReadmeFilePath = function_pre_init ();
	}
	g_free (cPreInitFuncName);
	
	
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
	
	pCairoDockModule->pModule = module;
	pCairoDockModule->initModule = function_init;
	pCairoDockModule->stopModule = function_stop;
}

static void cairo_dock_close_module (CairoDockModule *pCairoDockModule)
{
	g_module_close (pCairoDockModule->pModule);
	pCairoDockModule->pModule = NULL;
	pCairoDockModule->initModule = NULL;
	pCairoDockModule->stopModule = NULL;
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
	if (cActiveModuleList == NULL)
		return ;
	GError *erreur = NULL;
	gchar *cModuleName;
	CairoDockModule *pModule;
	int i = 0, iOrder = 0;
	while (cActiveModuleList[i] != NULL)
	{
		cModuleName = cActiveModuleList[i];
		//g_print (" + %s\n", cModuleName);
		pModule = g_hash_table_lookup (pModuleTable, cModuleName);
		if (pModule != NULL && ! pModule->bActive)  // les modules qui n'ont pas d'icones ne sont pas desactives lors de la configuuration du dock, et donc peuvent etre deja actives.
		{
			Icon *pIcon = cairo_dock_activate_module (pModule, pDock, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			if (pIcon != NULL)
			{
				pIcon->fOrder = iOrder ++;
				cairo_dock_insert_icon_in_dock (pIcon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
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
	
	cairo_dock_close_module (module);
	
	g_free (module->cModuleName);
	g_free (module->cSoFilePath);
	g_free (module);
}

Icon * cairo_dock_activate_module (CairoDockModule *module, CairoDock *pDock, GError **erreur)
{
	if (module == NULL)
	{
		g_set_error (erreur, 1, 1, "%s () : empty module !", __func__);
		return NULL;
	}
	
	if (module->bActive)
	{
		g_set_error (erreur, 1, 1, "%s () : module %s is already active !", __func__, module->cModuleName);
		return NULL;
	}
	
	GError *tmp_erreur = NULL;
	if (module->pModule == NULL)  // normalement impossible.
	{
		cairo_dock_open_module (module, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
	}
	
	g_free (module->cConfFilePath);
	module->cConfFilePath = NULL;
	Icon *icon = module->initModule (pDock, &module->cConfFilePath, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	if (icon != NULL)
	{
		icon->pModule = module;
		icon->iType = CAIRO_DOCK_APPLET;
		icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);
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
	g_free (module->cConfFilePath);
	module->cConfFilePath = NULL;
}


void cairo_dock_reload_module (gchar *cConfFile, gpointer *data)
{
	g_return_if_fail (data != NULL);
	CairoDockModule *module = data[0];
	CairoDock *pDock = data[1];
	GError *erreur = NULL;
	if (! module->bActive)
		return;
	
	cairo_dock_deactivate_module (module);
	Icon *pOldIcon = cairo_dock_find_icon_from_module (module, pDock->icons);
	if (pOldIcon != NULL)
	{
		//g_print ("  enlevement de l'ancienne icone\n");
		pOldIcon->pModule = NULL;
		cairo_dock_remove_icon_from_dock (pDock, pOldIcon);
	}
	
	
	Icon *pNewIcon = cairo_dock_activate_module (module, pDock, &erreur);
	if (erreur != NULL)
 	{
		module->bActive = FALSE;
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, pDock->icons, g_hModuleTable);
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
	}
	
	
	if (pNewIcon != NULL)
	{
		cairo_dock_insert_icon_in_dock (pNewIcon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
		
		if (pOldIcon != NULL)
		{
			pNewIcon->fOrder = pOldIcon->fOrder;
			pNewIcon->fX = pOldIcon->fX;
			pNewIcon->fY = pOldIcon->fY;
			pNewIcon->fScale = pOldIcon->fScale;
			pNewIcon->fDrawX = pOldIcon->fDrawX;
			pNewIcon->fDrawY = pOldIcon->fDrawY;
			pNewIcon->fWidthFactor = pOldIcon->fWidthFactor;
			pNewIcon->fAlpha = pOldIcon->fAlpha;
		}
		cairo_dock_redraw_my_icon (pNewIcon, pDock);
	}
	else if (pOldIcon != NULL)
	{
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
		gtk_widget_queue_draw (pDock->pWidget);
	}
	cairo_dock_free_icon (pOldIcon);
	
	//gtk_widget_queue_draw (pDock->pWidget);
}

void cairo_dock_configure_module (CairoDockModule *module, CairoDock *pDock, GError **erreur)
{
	g_return_if_fail (module != NULL);
	
	if (module->cConfFilePath == NULL)
		return;
	
	gchar *cTitle = g_strdup_printf ("Configuration of %s", module->cModuleName);
	gpointer *user_data = g_new (gpointer, 2);
	user_data[0]= module;
	user_data[1] = pDock;
	gboolean configuration_ok = cairo_dock_edit_conf_file (NULL, module->cConfFilePath, cTitle, 450, 450, 0, NULL, (CairoDockConfigFunc) cairo_dock_reload_module, user_data, (GFunc) g_free);
	g_free (cTitle);
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

