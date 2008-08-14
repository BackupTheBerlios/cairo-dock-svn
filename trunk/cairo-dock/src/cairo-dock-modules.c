/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include <cairo.h>

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-load.h"
#include "cairo-dock-config.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-keyfile-utilities.h"
#include "cairo-dock-log.h"
#include "cairo-dock-applet-facility.h"
#include "cairo-dock-applet-factory.h"
#include "cairo-dock-desklet.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-modules.h"

#define CAIRO_DOCK_MODULE_PANEL_WIDTH 700
#define CAIRO_DOCK_MODULE_PANEL_HEIGHT 450

extern CairoDock *g_pMainDock;
extern gchar *g_cConfFile;
extern gchar *g_cCurrentThemePath;
extern gboolean g_bUseSeparator;
extern short g_iMajorVersion, g_iMinorVersion, g_iMicroVersion;
extern int g_iWmHint;

static GHashTable *s_hModuleTable = NULL;
static int s_iMaxOrder = 0;

void cairo_dock_initialize_module_manager (gchar *cModuleDirPath)
{
	if (s_hModuleTable == NULL)
		s_hModuleTable = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			NULL,  // la cle est le nom du module, et pointe directement sur le champ 'cModuleName' du module.
			(GDestroyNotify) cairo_dock_free_module);

	if (cModuleDirPath != NULL && g_file_test (cModuleDirPath, G_FILE_TEST_IS_DIR))
	{
		GError *erreur = NULL;
		cairo_dock_preload_module_from_directory (cModuleDirPath, s_hModuleTable, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("%s\n  no module will be available", erreur->message);
			g_error_free (erreur);
		}
	}
}


static gchar *cairo_dock_extract_default_module_name_from_path (gchar *cSoFilePath)
{
	gchar *ptr = g_strrstr (cSoFilePath, "/");
	if (ptr == NULL)
		ptr = cSoFilePath;
	else
		ptr ++;
	if (strncmp (ptr, "lib", 3) == 0)
		ptr += 3;

	if (strncmp (ptr, "cd-", 3) == 0)
		ptr += 3;
	else if (strncmp (ptr, "cd_", 3) == 0)
		ptr += 3;

	gchar *cModuleName = g_strdup (ptr);

	ptr = g_strrstr (cModuleName, ".so");
	if (ptr != NULL)
		*ptr = '\0';

	//ptr = cModuleName;
	//while ((ptr = g_strrstr (ptr, "-")) != NULL)
	//	*ptr = '_';

	return cModuleName;
}

gchar *cairo_dock_check_module_conf_file (CairoDockVisitCard *pVisitCard)
{
	if (pVisitCard->cConfFileName == NULL)
		return NULL;
	
	gchar *cUserDataDirPath = g_strdup_printf ("%s/plug-ins/%s", g_cCurrentThemePath, pVisitCard->cUserDataDir);
	if (! g_file_test (cUserDataDirPath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		cd_message ("directory %s doesn't exist, it will be added.", cUserDataDirPath);
		
		gchar *command = g_strdup_printf ("mkdir -p %s", cUserDataDirPath);
		system (command);
		g_free (command);
	}
	
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", cUserDataDirPath, pVisitCard->cConfFileName);
	if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))
	{
		cd_message ("no conf file %s, we will take the default one", cConfFilePath);
		gchar *command = g_strdup_printf ("cp %s/%s %s", pVisitCard->cShareDataDir, pVisitCard->cConfFileName, cConfFilePath);
		system (command);
		g_free (command);
	}
	
	if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))  // la copie ne s'est pas bien passee.
	{
		cd_warning ("couldn't copy %s/%s in %s; check permissions and file's existence", pVisitCard->cShareDataDir, pVisitCard->cConfFileName, cUserDataDirPath);
		g_free (cUserDataDirPath);
		g_free (cConfFilePath);
		return NULL;
	}
	
	g_free (cUserDataDirPath);
	return cConfFilePath;
}

void cairo_dock_free_visit_card (CairoDockVisitCard *pVisitCard)
{
	g_free (pVisitCard->cReadmeFilePath);
	g_free (pVisitCard->cPreviewFilePath);
	g_free (pVisitCard->cGettextDomain);
	g_free (pVisitCard->cDockVersionOnCompilation);
	g_free (pVisitCard->cModuleName);
	g_free (pVisitCard->cUserDataDir);
	g_free (pVisitCard->cShareDataDir);
	g_free (pVisitCard->cConfFileName);
	g_free (pVisitCard->cModuleVersion);
	g_free (pVisitCard->cIconFilePath);
	g_free (pVisitCard);
}

static void cairo_dock_open_module (CairoDockModule *pCairoDockModule, GError **erreur)
{
	//\__________________ On ouvre le .so.
	GModule *module = g_module_open (pCairoDockModule->cSoFilePath, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	if (!module)
	{
		g_set_error (erreur, 1, 1, "while opening module '%s' : (%s)", pCairoDockModule->cSoFilePath, g_module_error ());
		return ;
	}

	//\__________________ On identifie le module.
	gboolean bSymbolFound;
	CairoDockModulePreInit function_pre_init = NULL;
	bSymbolFound = g_module_symbol (module, "pre_init", (gpointer) &function_pre_init);
	if (bSymbolFound && function_pre_init != NULL)
	{
		pCairoDockModule->pVisitCard = g_new0 (CairoDockVisitCard, 1);
		pCairoDockModule->pInterface = g_new0 (CairoDockModuleInterface, 1);
		function_pre_init (pCairoDockModule->pVisitCard, pCairoDockModule->pInterface);
	}
	else
	{
		pCairoDockModule->pVisitCard = NULL;
		g_set_error (erreur, 1, 1, "this module ('%s') does not have any visit card, it may be broken or icompatible with cairo-dock", pCairoDockModule->cSoFilePath);
		return ;
	}
	
	//\__________________ On verifie sa compatibilite.
	CairoDockVisitCard *pVisitCard = pCairoDockModule->pVisitCard;
	if (pVisitCard->iMajorVersionNeeded > g_iMajorVersion || (pVisitCard->iMajorVersionNeeded == g_iMajorVersion && pVisitCard->iMinorVersionNeeded > g_iMinorVersion) || (pVisitCard->iMajorVersionNeeded == g_iMajorVersion && pVisitCard->iMinorVersionNeeded == g_iMinorVersion && pVisitCard->iMicroVersionNeeded > g_iMicroVersion))
	{
		g_set_error (erreur, 1, 1, "this module ('%s') needs at least Cairo-Dock v%d.%d.%d, but Cairo-Dock is in v%s\n  It will be ignored", pCairoDockModule->cSoFilePath, pVisitCard->iMajorVersionNeeded, pVisitCard->iMinorVersionNeeded, pVisitCard->iMicroVersionNeeded, CAIRO_DOCK_VERSION);
		cairo_dock_free_visit_card (pCairoDockModule->pVisitCard);
		pCairoDockModule->pVisitCard = NULL;
		return ;
	}
	if (pVisitCard->cDockVersionOnCompilation != NULL && strcmp (pVisitCard->cDockVersionOnCompilation, CAIRO_DOCK_VERSION) != 0)
	{
		g_set_error (erreur, 1, 1, "this module ('%s') was compiled with Cairo-Dock v%s, but Cairo-Dock is in v%s\n  It will be ignored", pCairoDockModule->cSoFilePath, pVisitCard->cDockVersionOnCompilation, CAIRO_DOCK_VERSION);
		cairo_dock_free_visit_card (pCairoDockModule->pVisitCard);
		pCairoDockModule->pVisitCard = NULL;
		return ;
	}

	if (pVisitCard->cModuleName == NULL)
		pVisitCard->cModuleName = cairo_dock_extract_default_module_name_from_path (pCairoDockModule->cSoFilePath);
}

static void cairo_dock_close_module (CairoDockModule *module)
{
	g_module_close (module->pModule);
	
	g_free (module->pInterface);
	
	cairo_dock_free_visit_card (module->pVisitCard);
	
	g_free (module->cConfFilePath);
	module->cConfFilePath = NULL;
}



CairoDockModule * cairo_dock_load_module (gchar *cSoFilePath, GHashTable *pModuleTable, GError **erreur)  // cSoFilePath vers un fichier de la forme 'libtruc.so'. Le module est rajoute en debut de la liste si il n'est pas deja dedans. La liste peut neanmoins etre NULL.
{
	//g_print ("%s (%s)\n", __func__, cSoFilePath);
	if (cSoFilePath == NULL)  // g_module_open () plante si 'cSoFilePath' est NULL.
	{
		g_set_error (erreur, 1, 1, "%s () : no such module", __func__);
		return NULL;
	}

	CairoDockModule *pCairoDockModule = g_new0 (CairoDockModule, 1);
	pCairoDockModule->cSoFilePath = g_strdup (cSoFilePath);

	GError *tmp_erreur = NULL;
	cairo_dock_open_module (pCairoDockModule, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		g_free (pCairoDockModule);
		return NULL;
	}

	if (pModuleTable != NULL && pCairoDockModule->pInterface->initModule != NULL)
		g_hash_table_insert (pModuleTable, pCairoDockModule->pVisitCard->cModuleName, pCairoDockModule);

	return pCairoDockModule;
}


void cairo_dock_preload_module_from_directory (gchar *cModuleDirPath, GHashTable *pModuleTable, GError **erreur)
{
	cd_message ("%s (%s)", __func__, cModuleDirPath);
	GError *tmp_erreur = NULL;
	GDir *dir = g_dir_open (cModuleDirPath, 0, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return ;
	}

	const gchar *cFileName;
	GString *sFilePath = g_string_new ("");
	CairoDockModule *pModule;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		if (g_str_has_suffix (cFileName, ".so"))
		{
			g_string_printf (sFilePath, "%s/%s", cModuleDirPath, cFileName);
			pModule = cairo_dock_load_module (sFilePath->str, pModuleTable, &tmp_erreur);
			if (tmp_erreur != NULL)
			{
				cd_warning (tmp_erreur->message);
				g_error_free (tmp_erreur);
				tmp_erreur = NULL;
			}
		}
	}
	while (1);
	g_string_free (sFilePath, TRUE);
	g_dir_close (dir);
}



void cairo_dock_activate_modules_from_list (gchar **cActiveModuleList, double fTime)
{
	if (cActiveModuleList == NULL)
		return ;

	GError *erreur = NULL;
	gchar *cModuleName;
	CairoDockModule *pModule;
	int i = 0;
	while (cActiveModuleList[i] != NULL)
	{
		cModuleName = cActiveModuleList[i];
		pModule = g_hash_table_lookup (s_hModuleTable, cModuleName);
		if (pModule == NULL)
		{
			cd_warning ("No such module (%s)", cModuleName);
			i ++;
			continue ;
		}
		
		pModule->fLastLoadingTime = fTime;
		///if (! pModule->bActive)
		if (pModule->pInstancesList == NULL)
		{
			cairo_dock_activate_module (pModule, &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
		}
		else
		{
			cairo_dock_reload_module (pModule, FALSE);
		}
		i ++;
	}
}

static void _cairo_dock_deactivate_one_old_module (gchar *cModuleName, CairoDockModule *pModule, double *fTime)
{
	if (pModule->fLastLoadingTime < *fTime)
		cairo_dock_deactivate_module (pModule);
}
void cairo_dock_deactivate_old_modules (double fTime)
{
	g_hash_table_foreach (s_hModuleTable, (GHFunc) _cairo_dock_deactivate_one_old_module, &fTime);
}



void cairo_dock_free_module (CairoDockModule *module)
{
	if (module == NULL)
		return ;
	cd_debug ("%s (%s)", __func__, module->pVisitCard->cModuleName);

	cairo_dock_deactivate_module (module);

	cairo_dock_close_module (module);

	cairo_dock_free_visit_card (module->pVisitCard);
	g_free (module->cSoFilePath);
	g_free (module->cConfFilePath);
	g_free (module);
}


GKeyFile *cairo_dock_pre_read_module_instance_config (CairoDockModuleInstance *pInstance, CairoDockMinimalAppletConfig *pMinimalConfig)
{
	g_return_val_if_fail (pInstance != NULL && pInstance->cConfFilePath != NULL, NULL);
	gchar *cInstanceConfFilePath = pInstance->cConfFilePath;
	CairoDockModule *pModule = pInstance->pModule;
	
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cInstanceConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return NULL;
	}

	gboolean bNeedsUpgrade = cairo_dock_conf_file_needs_update (pKeyFile, pModule->pVisitCard->cModuleVersion);
	if (bNeedsUpgrade)
	{
		cairo_dock_flush_conf_file (pKeyFile, cInstanceConfFilePath, pModule->pVisitCard->cShareDataDir, pModule->pVisitCard->cConfFileName);
		g_key_file_free (pKeyFile);
		pKeyFile = g_key_file_new ();

		g_key_file_load_from_file (pKeyFile, cInstanceConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
			return NULL;
		}
	}

	if (! g_key_file_has_group (pKeyFile, "Icon"))  // ce module n'a pas d'icone, ce n'est donc pas une applet.
	{
		pMinimalConfig->iDesiredIconWidth = -1;
		pMinimalConfig->iDesiredIconHeight = -1;
		return pKeyFile;
	}
	
	pMinimalConfig->iDesiredIconWidth = cairo_dock_get_integer_key_value (pKeyFile, "Icon", "width", NULL, 48, NULL, NULL);
	pMinimalConfig->iDesiredIconHeight = cairo_dock_get_integer_key_value (pKeyFile, "Icon", "height", NULL, 48, NULL, NULL);
	pMinimalConfig->cLabel = cairo_dock_get_string_key_value (pKeyFile, "Icon", "name", NULL, NULL, NULL, NULL);
	pMinimalConfig->cIconFileName = cairo_dock_get_string_key_value (pKeyFile, "Icon", "icon", NULL, NULL, NULL, NULL);
	pMinimalConfig->fOrder = cairo_dock_get_double_key_value (pKeyFile, "Icon", "order", NULL, CAIRO_DOCK_LAST_ORDER, NULL, NULL);
	if (pMinimalConfig->fOrder == 0 || pMinimalConfig->fOrder == CAIRO_DOCK_LAST_ORDER)
	{
		pMinimalConfig->fOrder = ++ s_iMaxOrder;
		g_key_file_set_double (pKeyFile, "Icon", "order", pMinimalConfig->fOrder);
		cairo_dock_write_keys_to_file (pKeyFile, cInstanceConfFilePath);
	}
	else
	{
		s_iMaxOrder = MAX (s_iMaxOrder, pMinimalConfig->fOrder);
	}
	pMinimalConfig->cDockName = cairo_dock_get_string_key_value (pKeyFile, "Icon", "dock name", NULL, NULL, NULL, NULL);
	
	if (! g_key_file_has_group (pKeyFile, "Desklet"))  // cette applet ne peut pas se detacher.
	{
		pMinimalConfig->iDeskletWidth = -1;
		pMinimalConfig->iDeskletHeight = -1;
	}
	else
	{
		pMinimalConfig->bDeskletUseSize = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "use size", NULL, TRUE, NULL, NULL);
		pMinimalConfig->iDeskletWidth = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "width", NULL, 92, NULL, NULL);
		pMinimalConfig->iDeskletHeight = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "height", NULL, 92, NULL, NULL);
		pMinimalConfig->iDeskletPositionX = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "x position", NULL, 0, NULL, NULL);
		pMinimalConfig->iDeskletPositionY = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "y position", NULL, 0, NULL, NULL);
		pMinimalConfig->bIsDetached = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "initially detached", NULL, FALSE, NULL, NULL);
		pMinimalConfig->bKeepBelow = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "keep below", NULL, FALSE, NULL, NULL);
		pMinimalConfig->bKeepAbove = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "keep above", NULL, FALSE, NULL, NULL);
		pMinimalConfig->bOnWidgetLayer = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "on widget layer", NULL, FALSE, NULL, NULL);
		pMinimalConfig->bPositionLocked = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "locked", NULL, FALSE, NULL, NULL);
	}
	return pKeyFile;
}

void cairo_dock_activate_module (CairoDockModule *module, GError **erreur)
{
	cd_message ("%s (%s)", __func__, module->pVisitCard->cModuleName);
	if (module == NULL)
	{
		g_set_error (erreur, 1, 1, "%s () : empty module !", __func__);
		return ;
	}

	///if (module->bActive)
	if (module->pInstancesList != NULL)
	{
		g_set_error (erreur, 1, 1, "%s () : module %s is already active !", __func__, module->pVisitCard->cModuleName);
		return ;
	}

	GError *tmp_erreur = NULL;
	if (module->pModule == NULL)  // normalement impossible.
	{
		cairo_dock_open_module (module, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return ;
		}
	}
	
	g_free (module->cConfFilePath);
	module->cConfFilePath = cairo_dock_check_module_conf_file (module->pVisitCard);
	
	gchar *cInstanceFilePath = NULL;
	int j = 0;
	do
	{
		if (j == 0)
			cInstanceFilePath = g_strdup (module->cConfFilePath);
		else
			cInstanceFilePath = g_strdup_printf ("%s-%d",  module->cConfFilePath, j);
		
		if (! g_file_test (cInstanceFilePath, G_FILE_TEST_EXISTS))
		{
			g_free (cInstanceFilePath);
			break ;
		}
		
		cairo_dock_instanciate_module (module, cInstanceFilePath);  // prend possession de 'cInstanceFilePath'.
		
		j ++;
	} while (1);
	
	if (j == 0)
	{
		g_set_error (erreur, 1, 1, "%s () : no instance of module %s could be created", __func__, module->pVisitCard->cModuleName);
		return ;
	}
}


void cairo_dock_deactivate_module (CairoDockModule *module)
{
	g_return_if_fail (module != NULL);
	g_list_foreach (module->pInstancesList, (GFunc) cairo_dock_stop_module_instance, NULL);
	g_list_foreach (module->pInstancesList, (GFunc) cairo_dock_free_module_instance, NULL);
	g_list_free (module->pInstancesList);
	module->pInstancesList = NULL;
}


void cairo_dock_reload_module_instance (CairoDockModuleInstance *pInstance, gboolean bReloadAppletConf)
{
	g_return_if_fail (pInstance != NULL);
	CairoDockModule *module = pInstance->pModule;
	cd_message ("%s (%s, %d)", __func__, module->pVisitCard->cModuleName, bReloadAppletConf);
	
	GError *erreur = NULL;
	CairoContainer *pActualContainer = pInstance->pContainer;
	pInstance->pContainer = NULL;
	
	//\______________ On tente de recharger le module.
	gboolean bModuleReloaded = FALSE;
	//if (module->bActive && module->reloadModule != NULL)
	if (module->pInstancesList != NULL && module->pInterface->reloadModule != NULL)
	{
		Icon *pIcon = pInstance->pIcon;

		GKeyFile *pKeyFile = NULL;
		CairoDockMinimalAppletConfig *pMinimalConfig = NULL;
		gboolean bToBeInserted = FALSE;
		gboolean bNeedFreeDesklet = FALSE;
		CairoContainer *pNewContainer = NULL;
		if (bReloadAppletConf && pInstance->cConfFilePath != NULL)
		{
			pMinimalConfig = g_new0 (CairoDockMinimalAppletConfig, 1);
			pKeyFile = cairo_dock_pre_read_module_instance_config (pInstance, pMinimalConfig);

			if (pMinimalConfig->iDesiredIconWidth > -1)  // c'est une applet.
			{
				if (pIcon != NULL)
				{
					g_free (pIcon->acName);
					pIcon->acName = pMinimalConfig->cLabel;
					pMinimalConfig->cLabel = NULL;  // astuce.
					g_free (pIcon->acFileName);
					pIcon->acFileName = pMinimalConfig->cIconFileName;
					pMinimalConfig->cIconFileName = NULL;
				}

				if (pMinimalConfig->bIsDetached)  // l'applet est maintenant dans un desklet.
				{
					CairoDesklet *pDesklet;
					if (CAIRO_DOCK_IS_DOCK (pActualContainer))  // elle etait dans un dock.
					{
						cd_message ("le container a change (%s -> desklet)", pIcon->cParentDockName);
						gchar *cOldDockName = g_strdup (pIcon->cParentDockName);
						cairo_dock_detach_icon_from_dock (pIcon, CAIRO_DOCK (pActualContainer), g_bUseSeparator);
						if (CAIRO_DOCK (pActualContainer)->icons == NULL)
							cairo_dock_destroy_dock (CAIRO_DOCK (pActualContainer), cOldDockName, NULL, NULL);
						else
							cairo_dock_update_dock_size (CAIRO_DOCK (pActualContainer));
						g_free (cOldDockName);
						pDesklet = cairo_dock_create_desklet (pIcon, NULL, pMinimalConfig->bOnWidgetLayer);
					}
					else
					{
						pDesklet = CAIRO_DESKLET (pActualContainer);
					}
					pNewContainer = CAIRO_CONTAINER (pDesklet);
					cairo_dock_place_desklet (pDesklet, pMinimalConfig);
				}
				else  // l'applet est maintenant dans un dock.
				{
					gchar *cDockName = (pMinimalConfig->cDockName != NULL ? pMinimalConfig->cDockName : CAIRO_DOCK_MAIN_DOCK_NAME);
					CairoDock *pDock = cairo_dock_search_dock_from_name (cDockName);
					if (pDock == NULL)  // c'est un nouveau dock.
					{
						pDock = cairo_dock_create_new_dock (g_iWmHint, cDockName, NULL);
						///cairo_dock_place_root_dock (pDock);
					}
					
					if (CAIRO_DOCK_IS_DESKLET (pActualContainer))  // elle etait dans un desklet.
					{
						bNeedFreeDesklet = TRUE;  // le desklet sera detruit apres le reload.
						cairo_dock_steal_interactive_widget_from_desklet (CAIRO_DESKLET (pActualContainer));
						///cairo_dock_free_desklet (CAIRO_DESKLET (pActualContainer));
						///pActualContainer = NULL;
						bToBeInserted = TRUE;  // l'icone sera inseree dans le dock avant le reload.
					}
					else  // elle etait deja dans un dock.
					{
						if (pActualContainer != CAIRO_CONTAINER (pDock))  // le dock a change.
						{
							cd_message ("le dock a change (%s -> %s)", pIcon->cParentDockName, cDockName);
							gchar *cOldDockName = g_strdup (pIcon->cParentDockName);
							cairo_dock_detach_icon_from_dock (pIcon, CAIRO_DOCK (pActualContainer), g_bUseSeparator);
							if (CAIRO_DOCK (pActualContainer)->icons == NULL)
							{
								cairo_dock_destroy_dock (CAIRO_DOCK (pActualContainer), cOldDockName, NULL, NULL);
								pActualContainer = NULL;
							}
							else
								cairo_dock_update_dock_size (CAIRO_DOCK (pActualContainer));
							g_free (cOldDockName);
							bToBeInserted = TRUE;  // l'icone sera inseree dans le dock avant le reload.
						}
					}
					pNewContainer = CAIRO_CONTAINER (pDock);
				}
			}
		}
		else
			pNewContainer = pActualContainer;

		pInstance->pContainer = pNewContainer;
		if (CAIRO_DOCK_IS_DOCK (pNewContainer))
		{
			pInstance->pDock = CAIRO_DOCK (pNewContainer);
			pInstance->pDesklet = NULL;
		}
		else
		{
			pInstance->pDock = NULL;
			pInstance->pDesklet = CAIRO_DESKLET (pNewContainer);
		}
		if (CAIRO_DOCK_IS_DOCK (pNewContainer) && pIcon != NULL)
		{
			if (pMinimalConfig != NULL)
			{
				pIcon->fWidth = pMinimalConfig->iDesiredIconWidth;
				pIcon->fHeight = pMinimalConfig->iDesiredIconHeight;
			}
			cairo_dock_load_one_icon_from_scratch (pIcon, pNewContainer);

			if (bToBeInserted)
			{
				CairoDock *pDock = CAIRO_DOCK (pNewContainer);
				cairo_dock_insert_icon_in_dock (pIcon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
				pIcon->cParentDockName = g_strdup (pMinimalConfig->cDockName);
				cairo_dock_start_animation (pIcon, pDock);
			}
			else
			{
				pIcon->fWidth *= CAIRO_DOCK (pActualContainer)->fRatio;
				pIcon->fHeight *= CAIRO_DOCK (pActualContainer)->fRatio;
				
				if (bReloadAppletConf)
					cairo_dock_update_dock_size (CAIRO_DOCK (pNewContainer));
			}
		}
		
		///\_______________________ On recharge l'instance.
		if (pKeyFile != NULL)
		{
			cairo_dock_read_module_config (pKeyFile, pInstance);
		}
		gboolean bCanReload = TRUE;
		if (pInstance->pDrawContext != NULL)
			cairo_destroy (pInstance->pDrawContext);
		if (pInstance->pDock)
		{
			if (pInstance->pIcon->pIconBuffer == NULL)
			{
				cd_warning ("invalid applet's icon buffer");
				pInstance->pDrawContext = NULL;
			}
			else
				pInstance->pDrawContext = cairo_create (pInstance->pIcon->pIconBuffer);
			if (cairo_status (pInstance->pDrawContext) != CAIRO_STATUS_SUCCESS)
			{
				cd_warning ("couldn't initialize drawing context, applet won't be reloaded !");
				bCanReload = FALSE;
			}
		}
		else
			pInstance->pDrawContext = NULL;
		if (bCanReload)
			bModuleReloaded = module->pInterface->reloadModule (pInstance, pActualContainer, pKeyFile);
		
		if (pNewContainer != pActualContainer && CAIRO_DOCK_IS_DOCK (pNewContainer) && CAIRO_DOCK_IS_DOCK (pActualContainer) && pIcon != NULL)
		{
			cairo_dock_synchronize_one_sub_dock_position (pIcon, CAIRO_DOCK (pNewContainer), TRUE);
		}
		
		cairo_dock_free_minimal_config (pMinimalConfig);
		if (pKeyFile != NULL)
			g_key_file_free (pKeyFile);
		
		if (bNeedFreeDesklet)
			cairo_dock_free_desklet (CAIRO_DESKLET (pActualContainer));
	}
}

void cairo_dock_reload_module (CairoDockModule *pModule, gboolean bReloadAppletConf)
{
	GList *pElement;
	CairoDockModuleInstance *pInstance;
	for (pElement = pModule->pInstancesList; pElement != NULL; pElement = pElement->next)
	{
		pInstance = pElement->data;
		cairo_dock_reload_module_instance (pInstance, bReloadAppletConf);
	}
}


static void _cairo_dock_deactivate_one_module (gchar *cModuleName, CairoDockModule *pModule, gpointer data)
{
	cairo_dock_deactivate_module (pModule);
}
void cairo_dock_deactivate_all_modules (void)
{
	g_hash_table_foreach (s_hModuleTable, (GHFunc) _cairo_dock_deactivate_one_module, NULL);
}


void cairo_dock_activate_module_and_load (gchar *cModuleName)
{
	if (g_pMainDock == NULL)
		return ;
	gchar *list[2] = {cModuleName, NULL};
	cairo_dock_activate_modules_from_list (list, 0);
	
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	g_return_if_fail (pModule != NULL);
	GList *pElement;
	CairoDockModuleInstance *pInstance;
	for (pElement = pModule->pInstancesList; pElement != NULL; pElement = pElement->next)
	{
		pInstance = pElement->data;
		if (pInstance->pDock)
		{
			cairo_dock_update_dock_size (pInstance->pDock);
			gtk_widget_queue_draw (pInstance->pDock->pWidget);
		}
	}
	
	cairo_dock_update_conf_file_with_active_modules2 (NULL, g_cConfFile);
}

void cairo_dock_deactivate_module_instance_and_unload (CairoDockModuleInstance *pInstance)
{
	g_return_if_fail (pInstance != NULL);
	cd_message ("%s (%s)", __func__, pInstance->cConfFilePath);
	
	Icon *pIcon = pInstance->pIcon;  // l'instance va etre detruite.
	CairoDock *pDock = pInstance->pDock;
	if (pDock)
	{
		cairo_dock_remove_icon_from_dock (pDock, pInstance->pIcon);  // desinstancie le module.
		cairo_dock_update_dock_size (pDock);
		gtk_widget_queue_draw (pDock->pWidget);
	}
	else
	{
		cairo_dock_deinstanciate_module (pInstance);
		pIcon->pModuleInstance = NULL;
	}
	cairo_dock_free_icon (pIcon);
}

void cairo_dock_deactivate_module_and_unload (gchar *cModuleName)
{
	if (g_pMainDock == NULL)
		return ;
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	g_return_if_fail (pModule != NULL);
	
	GList *pElement = pModule->pInstancesList, *pNextElement;
	CairoDockModuleInstance *pInstance;
	cd_debug ("%d instance(s) a arreter", g_list_length (pModule->pInstancesList));
	//for (pElement = pModule->pInstancesList; pElement != NULL; pElement = pElement->next)
	while (pElement != NULL)
	{
		pInstance = pElement->data;
		pNextElement = pElement->next;
		cairo_dock_deactivate_module_instance_and_unload (pInstance);
		pElement = pNextElement;
	}
	
	cairo_dock_update_conf_file_with_active_modules2 (NULL, g_cConfFile);
}


static void _cairo_dock_configure_module_instance_callback (gchar *cConfFile, gpointer *data)
{
	g_return_if_fail (data != NULL);
	CairoDockModuleInstance *pModuleInstance = data[0];
	gboolean bReloadAppletConf = GPOINTER_TO_INT (data[1]);
	cairo_dock_reload_module_instance (pModuleInstance, bReloadAppletConf);
}
void cairo_dock_configure_module_instance (GtkWindow *pParentWindow, CairoDockModuleInstance *pModuleInstance, GError **erreur)
{
	g_return_if_fail (pModuleInstance != NULL);
	
	cd_message ("%s (%s)", __func__, pModuleInstance->cConfFilePath);
	
	cairo_dock_update_applet_conf_file_with_containers (NULL, pModuleInstance->cConfFilePath);
	
	gchar *cTitle = g_strdup_printf (_("Configuration of %s"), pModuleInstance->pModule->pVisitCard->cModuleName);
	gpointer *data = g_new (gpointer, 2);
	data[0]= pModuleInstance;
	data[1] = GINT_TO_POINTER (TRUE);  // TRUE <=> reload applet conf file.
	gboolean configuration_ok = cairo_dock_edit_conf_file (pParentWindow, pModuleInstance->cConfFilePath, cTitle, CAIRO_DOCK_MODULE_PANEL_WIDTH, CAIRO_DOCK_MODULE_PANEL_HEIGHT, 0, NULL, (CairoDockConfigFunc) _cairo_dock_configure_module_instance_callback, data, (GFunc) g_free, pModuleInstance->pModule->pVisitCard->cGettextDomain);
	g_free (cTitle);
}

void cairo_dock_configure_inactive_module (GtkWindow *pParentWindow, CairoDockModule *pModule)
{
	if (pModule->cConfFilePath == NULL)  // on n'est pas encore passe par la dans le cas ou le plug-in n'a pas ete active; mais on veut pouvoir configurer un plug-in meme lorsqu'il est inactif.
	{
		pModule->cConfFilePath = cairo_dock_check_module_conf_file (pModule->pVisitCard);
	}
	if (pModule->cConfFilePath == NULL)
	{
		cd_warning ("couldn't load a conf file for this module => can't configure it.");
		return;
	}
	cd_message ("%s (%s)", __func__, pModule->cConfFilePath);
	
	cairo_dock_update_applet_conf_file_with_containers (NULL, pModule->cConfFilePath);
	
	gchar *cTitle = g_strdup_printf (_("Configuration of %s"), pModule->pVisitCard->cModuleName);
	gboolean configuration_ok = cairo_dock_edit_conf_file (pParentWindow, pModule->cConfFilePath, cTitle, CAIRO_DOCK_MODULE_PANEL_WIDTH, CAIRO_DOCK_MODULE_PANEL_HEIGHT, 0, NULL, (CairoDockConfigFunc) NULL, NULL, (GFunc) NULL, pModule->pVisitCard->cGettextDomain);
	g_free (cTitle);
}

void cairo_dock_configure_module (GtkWindow *pParentWindow, const gchar *cModuleName)
{
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	g_return_if_fail (pModule != NULL);
	
	GError *erreur = NULL;
	if (pModule->pInstancesList == NULL)  // module encore inactif.
	{
		cairo_dock_configure_inactive_module (pParentWindow, pModule);
	}
	else
	{
		cairo_dock_configure_module_instance (pParentWindow, pModule->pInstancesList->data, &erreur);  // on choisit de configurer la 1re instance.
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
		}
	}
}

CairoDockModule *cairo_dock_find_module_from_name (const gchar *cModuleName)
{
	//g_print ("%s (%s)\n", __func__, cModuleName);
	g_return_val_if_fail (cModuleName != NULL, NULL);
	return g_hash_table_lookup (s_hModuleTable, cModuleName);
}



static gboolean _cairo_dock_for_one_desklet (gchar *cModuleName, CairoDockModule *pModule, gpointer *data)
{
	GList *pElement;
	CairoDockModuleInstance *pInstance;
	for (pElement = pModule->pInstancesList; pElement != NULL; pElement = pElement->next)
	{
		pInstance = pElement->data;
		if (pInstance->pDesklet)
		{
			CairoDockForeachDeskletFunc pCallback = data[0];
			gpointer user_data = data[1];
			
			
			if (pCallback (pInstance->pDesklet, pInstance, user_data))
			{
				data[2] = pInstance;
				return TRUE;
			}
		}
		
	}
	
	return FALSE;
}
CairoDockModuleInstance *cairo_dock_foreach_desklet (CairoDockForeachDeskletFunc pCallback, gpointer user_data)
{
	gpointer data[3] = {pCallback, user_data, NULL};
	g_hash_table_find (s_hModuleTable, (GHRFunc) _cairo_dock_for_one_desklet, data);
	return data[2];
}



static void _cairo_dock_write_one_module_by_category (gchar *cModuleName, CairoDockModule *pModule, gpointer *data)
{
	GString **pStrings = data[0];
	gboolean bActiveOnly = GPOINTER_TO_INT (data[1]);
	///if (! bActiveOnly || pModule->bActive)
	if (! bActiveOnly || pModule->pInstancesList != NULL)
	{
		GString *sModuleInCategory = pStrings[pModule->pVisitCard->iCategory];
		if (bActiveOnly)
			cairo_dock_write_one_name (cModuleName, pModule, sModuleInCategory);
		else
			cairo_dock_write_one_module_name (cModuleName, pModule, sModuleInCategory);
	}
}
GString **cairo_dock_list_modules_by_category (gboolean bActiveOnly)
{
	GString **pStrings = g_new (GString *, CAIRO_DOCK_NB_CATEGORY);
	int i;
	for (i = 0; i < CAIRO_DOCK_NB_CATEGORY; i ++)
		pStrings[i] = g_string_new ("");
	gpointer data[2] = {pStrings, GINT_TO_POINTER (bActiveOnly)};
	
	g_hash_table_foreach (s_hModuleTable, (GHFunc) _cairo_dock_write_one_module_by_category, data);
	
	for (i = 0; i < CAIRO_DOCK_NB_CATEGORY; i ++)
	{
		if (pStrings[i]->len > 0)
			pStrings[i]->str[pStrings[i]->len-1] = '\0';
	}
	
	return pStrings;
}

void cairo_dock_update_conf_file_with_modules_full (GKeyFile *pOpenedKeyFile, gchar *cConfFile, gchar *cGroupName, gchar *cKeyNameBase, gboolean bActiveOnly)
{
	cd_debug ("%s (%s ; %d)", __func__, cConfFile, bActiveOnly);
	GKeyFile *pKeyFile = pOpenedKeyFile;
	if (pKeyFile == NULL)
	{
		pKeyFile = g_key_file_new ();
		GError *erreur = NULL;
		g_key_file_load_from_file (pKeyFile, cConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
			return ;
		}
	}
	
	GString **pStrings = cairo_dock_list_modules_by_category (bActiveOnly);
	GString *sKeyName = g_string_new (cKeyNameBase);
	int i;
	for (i = 0; i < CAIRO_DOCK_NB_CATEGORY; i ++)
	{
		g_string_printf (sKeyName, "%s_%d", cKeyNameBase, i);
		if (bActiveOnly)
			g_key_file_set_string (pKeyFile, cGroupName, sKeyName->str, pStrings[i]->str);
		else
			cairo_dock_update_conf_file_with_list (pKeyFile, NULL, pStrings[i]->str, cGroupName, sKeyName->str, NULL);
		//g_print ("%s <- %s\n", sKeyName->str, pStrings[i]->str);
		g_string_free (pStrings[i], TRUE);
	}
	cairo_dock_write_keys_to_file (pKeyFile, cConfFile);
	
	g_free (pStrings);
	g_string_free (sKeyName, TRUE);
	if (pOpenedKeyFile == NULL)
		g_key_file_free (pKeyFile);
}


int cairo_dock_get_nb_modules (void)
{
	return g_hash_table_size (s_hModuleTable);
}


void cairo_dock_update_module_instance_order (CairoDockModuleInstance *pModuleInstance, double fOrder)
{
	cd_message ("%s <- %.2f", pModuleInstance->pModule->pVisitCard->cModuleName, fOrder);
	cairo_dock_update_conf_file (pModuleInstance->cConfFilePath,
		G_TYPE_DOUBLE, "Icon", "order", fOrder,
		G_TYPE_INVALID);
}


/**
* Cree une nouvelle instance d'un module. Cree l'icone et le container associe, et les place ou il faut.
*/
CairoDockModuleInstance *cairo_dock_instanciate_module (CairoDockModule *pModule, gchar *cConfFilePath)  // prend possession de 'cConfFilePah'.
{
	g_return_val_if_fail (pModule != NULL && cConfFilePath != NULL, NULL);
	cd_message ("%s (%s)", __func__, cConfFilePath);
	
	//\____________________ On cree une instance du module.
	///CairoDockModuleInstance *pInstance = g_new0 (CairoDockModuleInstance, 1);
	CairoDockModuleInstance *pInstance = calloc (1, sizeof (CairoDockModuleInstance) + pModule->pVisitCard->iSizeOfConfig + pModule->pVisitCard->iSizeOfData);
	pInstance->pModule = pModule;
	pInstance->cConfFilePath = cConfFilePath;
	/*/if (pModule->pVisitCard->iSizeOfConfig > 0)
		pInstance->pConfig = g_new0 (gpointer, pModule->pVisitCard->iSizeOfConfig);
	if (pModule->pVisitCard->iSizeOfData > 0)
		pInstance->pData = g_new0 (gpointer, pModule->pVisitCard->iSizeOfData);*/
	pModule->pInstancesList = g_list_prepend (pModule->pInstancesList, pInstance);
	
	//\____________________ On cree le container de l'instance, ainsi que son icone.
	CairoContainer *pContainer = NULL;
	CairoDock *pDock = NULL;
	CairoDesklet *pDesklet = NULL;
	Icon *pIcon = NULL;
	GKeyFile *pKeyFile = NULL;
	CairoDockMinimalAppletConfig *pMinimalConfig = g_new0 (CairoDockMinimalAppletConfig, 1);
	pKeyFile = cairo_dock_pre_read_module_instance_config (pInstance, pMinimalConfig);
	
	if (pMinimalConfig->iDesiredIconWidth > 0)  // le module a une icone, c'est donc une applet.
	{
		pInstance->bCanDetach = pMinimalConfig->iDeskletWidth > 0;
		pModule->bCanDetach = pInstance->bCanDetach;  // pas encore clair ...
		gchar *cDockName = (pMinimalConfig->cDockName != NULL ? pMinimalConfig->cDockName : CAIRO_DOCK_MAIN_DOCK_NAME);
		
		if (pModule->bCanDetach && pMinimalConfig->bIsDetached)
		{
			pDesklet = cairo_dock_create_desklet (NULL, NULL, pMinimalConfig->bOnWidgetLayer);
			cairo_dock_place_desklet (pDesklet, pMinimalConfig);
			while (gtk_events_pending ())
				gtk_main_iteration ();
			pContainer = CAIRO_CONTAINER (pDesklet);
		}
		else
		{
			pDock = cairo_dock_search_dock_from_name (cDockName);
			if (pDock == NULL)
			{
				pDock = cairo_dock_create_new_dock (g_iWmHint, cDockName, NULL);
				///cairo_dock_place_root_dock (pDock);
			}
			pContainer = CAIRO_CONTAINER (pDock);
		}
		
		int iIconWidth, iIconHeight;
		if (pDock)
		{
			iIconWidth = pMinimalConfig->iDesiredIconWidth;
			iIconHeight = pMinimalConfig->iDesiredIconHeight;
		}
		else  // l'applet creera la surface elle-meme, car on ne sait ni la taille qu'elle voudra lui donner, ni meme si elle l'utilisera !
		{
			iIconWidth = -1;
			iIconHeight = -1;
		}
		pIcon = cairo_dock_create_icon_for_applet (pContainer,
			iIconWidth,
			iIconHeight,
			pMinimalConfig->cLabel,
			pMinimalConfig->cIconFileName,
			pInstance);
		pIcon->fOrder = pMinimalConfig->fOrder;
		pIcon->cParentDockName = g_strdup (cDockName);
		if (pDesklet)
		{
			pDesklet->pIcon = pIcon;
			///gtk_widget_queue_draw (pContainer->pWidget);
		}
		cairo_dock_free_minimal_config (pMinimalConfig);
	}

	//\____________________ On initialise l'instance.
	if (pDock)
	{
		pIcon->fWidth *= pDock->fRatio;
		pIcon->fHeight *= pDock->fRatio;
	}
	
	pInstance->pIcon = pIcon;
	pInstance->pDock = pDock;
	pInstance->pDesklet = pDesklet;
	pInstance->pContainer = pContainer;
	
	cairo_dock_read_module_config (pKeyFile, pInstance);
	
	gboolean bCanInit = TRUE;
	if (pDock)
	{
		if (pIcon != NULL)
		{
			if (pIcon->pIconBuffer == NULL)
			{
				cd_warning ("icon's buffer is NULL, applet won't be able to draw to it !");
				pInstance->pDrawContext = NULL;
			}
			else
				pInstance->pDrawContext = cairo_create (pIcon->pIconBuffer);
			if (cairo_status (pInstance->pDrawContext) != CAIRO_STATUS_SUCCESS)
			{
				cd_warning ("couldn't initialize drawing context, applet won't be able to draw its stuff !");
				bCanInit = FALSE;
			}
		}
	}
	else
		pInstance->pDrawContext = NULL;
	if (bCanInit)
		pModule->pInterface->initModule (pInstance, pKeyFile);
	
	if (pDock)
	{
		pIcon->fWidth /= pDock->fRatio;
		pIcon->fHeight /= pDock->fRatio;
		cairo_dock_insert_icon_in_dock (pIcon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
	}
	if (pKeyFile != NULL)
		g_key_file_free (pKeyFile);
	return pInstance;
}

/**
* Detruit une instance de module et libere les resources associees.
*/
void cairo_dock_free_module_instance (CairoDockModuleInstance *pInstance)
{
	g_free (pInstance->cConfFilePath);
	/**g_free (pInstance->pConfig);
	g_free (pInstance->pData);*/
	g_free (pInstance);
}

/**
* Stoppe une instance d'un module.
*/
void cairo_dock_stop_module_instance (CairoDockModuleInstance *pInstance)
{
	if (pInstance->pModule->pInterface->stopModule != NULL)
		pInstance->pModule->pInterface->stopModule (pInstance);
	
	if (pInstance->pModule->pInterface->reset_data != NULL)
		pInstance->pModule->pInterface->reset_data (pInstance);
	
	if (pInstance->pModule->pInterface->reset_config != NULL)
		pInstance->pModule->pInterface->reset_config (pInstance);
	
	if (pInstance->pDesklet)
		cairo_dock_free_desklet (pInstance->pDesklet);
	if (pInstance->pDrawContext != NULL)
		cairo_destroy (pInstance->pDrawContext);
}
/**
* Stoppe une instance d'un module, et la detruit.
*/
void cairo_dock_deinstanciate_module (CairoDockModuleInstance *pInstance)
{
	cairo_dock_stop_module_instance (pInstance);
	
	pInstance->pModule->pInstancesList = g_list_remove (pInstance->pModule->pInstancesList, pInstance);
	
	cairo_dock_free_module_instance (pInstance);
}

/**
* Stoppe une instance d'un module, et la supprime.
*/
void cairo_dock_remove_module_instance (CairoDockModuleInstance *pInstance)
{
	cd_message ("%s (%s)", __func__, pInstance->cConfFilePath);
	//\_________________ Si c'est la derniere instance, on desactive le module.
	if (pInstance->pModule->pInstancesList->next == NULL)
	{
		cairo_dock_deactivate_module_and_unload (pInstance->pModule->pVisitCard->cModuleName);
		return ;
	}
	
	//\_________________ On efface le fichier de conf de cette instance.
	cd_debug ("on efface %s", pInstance->cConfFilePath);
	g_remove (pInstance->cConfFilePath);
	
	//\_________________ On supprime cette instance (on le fait maintenant, pour que son fichier de conf n'existe plus lors du 'stop'.
	gchar *cConfFilePath = pInstance->cConfFilePath;
	pInstance->cConfFilePath = NULL;
	CairoDockModule *pModule = pInstance->pModule;
	cairo_dock_deactivate_module_instance_and_unload (pInstance);  // pInstance n'est plus.
	
	//\_________________ Si c'est pas la derniere instance, la derniere instance prend sa place.
	int iNbInstances = g_list_length (pModule->pInstancesList)+1;  // nombre d'instances avant suppression.
	gchar *str = strrchr (cConfFilePath, '-');
	if (str == NULL || atoi (str+1) != iNbInstances-1)
	{
		gchar *cLastInstanceFilePath = g_strdup_printf ("%s-%d", pModule->cConfFilePath, iNbInstances-1);
		
		CairoDockModuleInstance *pOneInstance;
		GList *pElement;
		for (pElement = pModule->pInstancesList; pElement != NULL; pElement = pElement->next)
		{
			pOneInstance = pElement->data;
			if (strcmp (pOneInstance->cConfFilePath, cLastInstanceFilePath) == 0)
			{
				gchar *cCommand = g_strdup_printf ("mv '%s' '%s'", cLastInstanceFilePath, cConfFilePath);
				system (cCommand);
				g_free (cCommand);
				
				g_free (pOneInstance->cConfFilePath);
				pOneInstance->cConfFilePath = cConfFilePath;
				cConfFilePath = NULL;
				break ;
			}
		}
		
		g_free (cLastInstanceFilePath);
	}
	g_free (cConfFilePath);
}

void cairo_dock_add_module_instance (CairoDockModule *pModule)
{
	if (pModule->pInstancesList == NULL)
	{
		cd_warning ("This module has not been instanciated yet");
		
		return ;
	}
	int iNbInstances = g_list_length (pModule->pInstancesList);
	gchar *cInstanceFilePath = g_strdup_printf ("%s-%d", pModule->cConfFilePath, iNbInstances);
	if (! g_file_test (cInstanceFilePath, G_FILE_TEST_EXISTS))
	{
		gchar *cCommand = g_strdup_printf ("cp %s/%s %s", pModule->pVisitCard->cShareDataDir, pModule->pVisitCard->cConfFileName, cInstanceFilePath);
		cd_debug (cCommand);
		system (cCommand);
		g_free (cCommand);
	}
	
	CairoDockModuleInstance *pNewInstance = cairo_dock_instanciate_module (pModule, cInstanceFilePath);  // prend le 'cInstanceFilePath'.
	
	if (pNewInstance != NULL && pNewInstance->pDock)
	{
		cairo_dock_update_dock_size (pNewInstance->pDock);
	}
}


void cairo_dock_read_module_config (GKeyFile *pKeyFile, CairoDockModuleInstance *pInstance)
{
	CairoDockModuleInterface *pInterface = pInstance->pModule->pInterface;
	CairoDockVisitCard *pVisitCard = pInstance->pModule->pVisitCard;
	
	if (pInterface->read_conf_file == NULL)
		return ;
	
	if (pInterface->reset_config != NULL)
		pInterface->reset_config (pInstance);
	if (pVisitCard->iSizeOfConfig != 0)
		memset (((gpointer)pInstance)+sizeof(CairoDockModuleInstance), 0, pVisitCard->iSizeOfConfig);
	
	gboolean bFlushConfFileNeeded = pInterface->read_conf_file (pInstance, pKeyFile);
	
	
	if (! bFlushConfFileNeeded)
		bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile, pVisitCard->cModuleVersion);
	if (bFlushConfFileNeeded)
		cairo_dock_flush_conf_file (pKeyFile, pInstance->cConfFilePath, pVisitCard->cShareDataDir, pVisitCard->cConfFileName);
}
