/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <cairo.h>

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-load.h"
#include "cairo-dock-config.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-modules.h"

#define CAIRO_DOCK_MODULE_PANEL_WIDTH 650
#define CAIRO_DOCK_MODULE_PANEL_HEIGHT 450

extern gchar *g_cConfFile;

static GHashTable *s_hModuleTable = NULL;


void cairo_dock_initialize_module_manager (gchar *cModuleDirPath)
{
	g_return_if_fail (s_hModuleTable == NULL);
	
	s_hModuleTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		NULL,  // la cle est le nom du module, et pointe directement sur le champ 'cModuleName' du module.
		(GDestroyNotify) cairo_dock_free_module);
	
	if (cModuleDirPath != NULL)
	{
		GError *erreur = NULL;
		cairo_dock_preload_module_from_directory (cModuleDirPath, s_hModuleTable, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n  no module will be available\n", erreur->message);
			g_error_free (erreur);
		}
	}
}


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
	GModule *module = g_module_open (pCairoDockModule->cSoFilePath, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	if (!module)
	{
		g_set_error (erreur, 1, 1, "Attention : while opening module '%s' : (%s)", pCairoDockModule->cSoFilePath, g_module_error ());
		return ;
	}
	
	g_free (pCairoDockModule->cReadmeFilePath);
	pCairoDockModule->cReadmeFilePath = NULL;
	gboolean bSymbolFound;
	
	CairoDockModulePreInit function_pre_init;
	gchar *cPreInitFuncName = g_strdup_printf ("%s_pre_init", pCairoDockModule->cModuleName);
	bSymbolFound = g_module_symbol (module, cPreInitFuncName, (gpointer) &function_pre_init) || g_module_symbol (module, "pre_init", (gpointer) &function_pre_init);
	if (! bSymbolFound)
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
	bSymbolFound = g_module_symbol (module, cInitFuncName, (gpointer) &function_init) || g_module_symbol (module, "init", (gpointer) &function_init);
	if (! bSymbolFound)
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
	bSymbolFound = g_module_symbol (module, cStopFuncName, (gpointer) &function_stop) || g_module_symbol (module, "stop", (gpointer) &function_stop);
	if (! bSymbolFound)
	{
		function_stop = NULL;
	}
	g_free (cStopFuncName);
	
	
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



void cairo_dock_activate_modules_from_list (gchar **cActiveModuleList, CairoDock *pDock)
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
		pModule = g_hash_table_lookup (s_hModuleTable, cModuleName);
		if (pModule != NULL && ! pModule->bActive)  // les modules qui n'ont pas d'icones ne sont pas desactives lors de la configuration du dock, et donc peuvent etre deja actifs.
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

void cairo_dock_update_conf_file_with_available_modules_full (gchar *cConfFile, gchar *cGroupName, gchar *cKeyName)
{
	cairo_dock_update_conf_file_with_hash_table (cConfFile, s_hModuleTable, cGroupName, cKeyName, NULL, (GHFunc) cairo_dock_write_one_module_name);
}



static void _cairo_dock_add_one_module_name_if_active (gchar *cModuleName, CairoDockModule *pModule, GSList **pListeModule)
{
	if (pModule->bActive)
	{
		if (g_slist_find (*pListeModule, cModuleName) == NULL)
			*pListeModule = g_slist_prepend (*pListeModule, cModuleName);
	}
}
void cairo_dock_update_conf_file_with_active_modules (gchar *cConfFile, GList *pIconList)  // garde l'ordre des icones.
{
	g_print ("%s ()\n", __func__);
	GError *erreur = NULL;
	
	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	//\___________________ On dresse la liste des modules actifs, en conservant leur ordre dans le dock.
	GSList *pListeModule = NULL;
	Icon *icon;
	gboolean bInside = FALSE;
	GList *pList = NULL;
	for (pList = pIconList; pList != NULL; pList = pList->next)
	{
		icon = pList->data;
		if (CAIRO_DOCK_IS_APPLET (icon))
		{
			bInside = TRUE;
			pListeModule = g_slist_append (pListeModule, icon->pModule->cModuleName);
		}
		else if (bInside)
			break ;
	}
	g_hash_table_foreach (s_hModuleTable, (GHFunc) _cairo_dock_add_one_module_name_if_active, &pListeModule);  // on complete.
	
	//\___________________ On ecrit tout ca dans le fichier de conf.
	GSList *pSList;
	GString *cActiveModules = g_string_new ("");
	for (pSList = pListeModule; pSList != NULL; pSList = pSList->next)
	{
		g_string_append_printf (cActiveModules, "%s;", (gchar *) pSList->data);
	}
	
	g_key_file_set_string (pKeyFile, "Applets", "active modules", cActiveModules->str);
	cairo_dock_write_keys_to_file (pKeyFile, cConfFile);
	
	g_slist_free (pListeModule);
	g_string_free (cActiveModules, TRUE);
	g_key_file_free (pKeyFile);
}

void cairo_dock_foreach_module (GHFunc pFunction, gpointer data)
{
	g_hash_table_foreach (s_hModuleTable, pFunction, data);
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
	
	//\______________ On recupere l'eventuelle icone du module.
	Icon *pOldIcon = cairo_dock_find_icon_from_module (module, pDock->icons);
	
	//\______________ On desactive le module et on retire son eventuelle icone du dock.
	cairo_dock_deactivate_module (module);
	if (pOldIcon != NULL)
	{
		g_print ("  enlevement de l'ancienne icone (%.1f)\n", pOldIcon->fOrder);
		pOldIcon->pModule = NULL;
		cairo_dock_remove_icon_from_dock (pDock, pOldIcon);
	}
	
	//\______________ On le reactive.
	Icon *pNewIcon = cairo_dock_activate_module (module, pDock, &erreur);
	if (erreur != NULL)
 	{
		module->bActive = FALSE;
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, pDock->icons);
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
	}
	
	//\______________ On insere sa nouvelle eventuelle icone en reprenant quelques parametres de l'ancienne.
	if (pNewIcon != NULL)
	{
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
		
		g_print ("pNewIcon->fOrder <- %.1f\n", pNewIcon->fOrder);
		cairo_dock_insert_icon_in_dock (pNewIcon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
		
		cairo_dock_redraw_my_icon (pNewIcon, pDock);
	}
	else if (pOldIcon != NULL)
	{
		cairo_dock_update_dock_size (pDock);
		gtk_widget_queue_draw (pDock->pWidget);
	}
	cairo_dock_free_icon (pOldIcon);
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
	gboolean configuration_ok = cairo_dock_edit_conf_file (NULL, module->cConfFilePath, cTitle, CAIRO_DOCK_MODULE_PANEL_WIDTH, CAIRO_DOCK_MODULE_PANEL_HEIGHT, 0, NULL, (CairoDockConfigFunc) cairo_dock_reload_module, user_data, (GFunc) g_free);
	g_free (cTitle);
}



Icon *cairo_dock_find_icon_from_module (CairoDockModule *module, GList *pIconList)
{
	if (! module->bActive)
		return NULL;
	
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

