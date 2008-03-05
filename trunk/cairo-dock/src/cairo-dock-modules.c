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
#include "cairo-dock-keyfile-manager.h"
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
extern gboolean g_bUseSeparator;
extern short g_iMajorVersion, g_iMinorVersion, g_iMicroVersion;
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
			cd_warning ("Attention : %s\n  no module will be available", erreur->message);
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
	g_free (pVisitCard);
}

static void cairo_dock_open_module (CairoDockModule *pCairoDockModule, GError **erreur)
{
	GModule *module = g_module_open (pCairoDockModule->cSoFilePath, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	if (!module)
	{
		g_set_error (erreur, 1, 1, "Attention : while opening module '%s' : (%s)", pCairoDockModule->cSoFilePath, g_module_error ());
		return ;
	}

	gboolean bSymbolFound;

	CairoDockModulePreInit function_pre_init = NULL;
	bSymbolFound = g_module_symbol (module, "pre_init", (gpointer) &function_pre_init);
	pCairoDockModule->pVisitCard = NULL;
	if (bSymbolFound && function_pre_init != NULL)
	{
		pCairoDockModule->pVisitCard = function_pre_init ();
	}
	if (pCairoDockModule->pVisitCard == NULL)
	{
		g_set_error (erreur, 1, 1, "Attention : this module ('%s') does not have any visit card, it may be broken or icompatible with cairo-dock\n", pCairoDockModule->cSoFilePath);
		return ;
	}
	else
	{
		CairoDockVisitCard *pVisitCard = pCairoDockModule->pVisitCard;
		if (pVisitCard->iMajorVersionNeeded > g_iMajorVersion || (pVisitCard->iMajorVersionNeeded == g_iMajorVersion && pVisitCard->iMinorVersionNeeded > g_iMinorVersion) || (pVisitCard->iMajorVersionNeeded == g_iMajorVersion && pVisitCard->iMinorVersionNeeded == g_iMinorVersion && pVisitCard->iMicroVersionNeeded > g_iMicroVersion))
		{
			g_set_error (erreur, 1, 1, "Attention : this module ('%s') needs at least Cairo-Dock v%d.%d.%d, but Cairo-Dock is in v%s\n  It will be ignored", pCairoDockModule->cSoFilePath, pVisitCard->iMajorVersionNeeded, pVisitCard->iMinorVersionNeeded, pVisitCard->iMicroVersionNeeded, CAIRO_DOCK_VERSION);
			cairo_dock_free_visit_card (pCairoDockModule->pVisitCard);
			pCairoDockModule->pVisitCard = NULL;
			return ;
		}
		if (pVisitCard->cDockVersionOnCompilation != NULL && strcmp (pVisitCard->cDockVersionOnCompilation, CAIRO_DOCK_VERSION) != 0)
		{
			g_set_error (erreur, 1, 1, "Attention : this module ('%s') was compiled with Cairo-Dock v%s, but Cairo-Dock is in v%s\n  It will be ignored", pCairoDockModule->cSoFilePath, pVisitCard->cDockVersionOnCompilation, CAIRO_DOCK_VERSION);
			cairo_dock_free_visit_card (pCairoDockModule->pVisitCard);
			pCairoDockModule->pVisitCard = NULL;
			return ;
		}

		if (pVisitCard->cModuleName == NULL)
			pVisitCard->cModuleName = cairo_dock_extract_default_module_name_from_path (pCairoDockModule->cSoFilePath);
	}

	CairoDockModuleInit function_init;
	bSymbolFound = g_module_symbol (module, "init", (gpointer) &function_init);
	if (! bSymbolFound)
	{
		g_set_error (erreur, 1, 1, "Attention : the module '%s' is not valid : (%s)", pCairoDockModule->cSoFilePath, g_module_error ());
		if (!g_module_close (module))
			g_warning ("%s: %s", pCairoDockModule->cSoFilePath, g_module_error ());
		return ;
	}


	CairoDockModuleStop function_stop;
	bSymbolFound = g_module_symbol (module, "stop", (gpointer) &function_stop);
	if (! bSymbolFound)
	{
		function_stop = NULL;
	}

	CairoDockModuleReload function_reload;
	bSymbolFound = g_module_symbol (module, "reload", (gpointer) &function_reload);
	if (! bSymbolFound)
	{
		function_reload = NULL;
	}


	pCairoDockModule->pModule = module;
	pCairoDockModule->initModule = function_init;
	pCairoDockModule->stopModule = function_stop;
	pCairoDockModule->reloadModule = function_reload;
}

static void cairo_dock_close_module (CairoDockModule *module)
{
	g_module_close (module->pModule);
	module->pModule = NULL;
	module->initModule = NULL;
	module->stopModule = NULL;
	module->reloadModule = NULL;

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
		return NULL;
	}


	if (pModuleTable != NULL && pCairoDockModule->initModule != NULL)
		g_hash_table_insert (pModuleTable, pCairoDockModule->pVisitCard->cModuleName, pCairoDockModule);

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
				cd_message ("Attention : %s\n", tmp_erreur->message);
				g_error_free (tmp_erreur);
				tmp_erreur = NULL;
			}
			g_free (cFilePath);
		}
	}
	while (1);
	g_dir_close (dir);
}


void cairo_dock_activate_modules_from_list (gchar **cActiveModuleList, CairoDock *pDock, double fTime)
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
		if (pModule != NULL)
		{
			pModule->fLastLoadingTime = fTime;
			if (! pModule->bActive)
			{
				Icon *pIcon = cairo_dock_activate_module (pModule, pDock, &erreur);
				if (erreur != NULL)
				{
					cd_warning ("Attention : %s", erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
				if (pIcon != NULL)
				{
					pIcon->fOrder = iOrder ++;
					if (CAIRO_DOCK_IS_DOCK (pModule->pContainer))
						cairo_dock_insert_icon_in_dock (pIcon, CAIRO_DOCK_DOCK (pModule->pContainer), ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
				}
			}
			else
			{
				cairo_dock_reload_module (pModule, FALSE);
			}
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

void cairo_dock_update_conf_file_with_available_modules_full (GKeyFile *pOpenedKeyFile, gchar *cConfFile, gchar *cGroupName, gchar *cKeyName)
{
	cairo_dock_update_conf_file_with_hash_table (pOpenedKeyFile, cConfFile, s_hModuleTable, cGroupName, cKeyName, NULL, (GHFunc) cairo_dock_write_one_module_name, FALSE, FALSE);  // ils sont classes par ordre dans le dock.
}



static void _cairo_dock_add_one_module_name_if_active (gchar *cModuleName, CairoDockModule *pModule, GSList **pListeModule)
{
	if (pModule->bActive)
	{
		if (g_slist_find (*pListeModule, cModuleName) == NULL)
			*pListeModule = g_slist_prepend (*pListeModule, cModuleName);
	}
}
void cairo_dock_update_conf_file_with_active_modules (GKeyFile *pOpenedKeyFile, gchar *cConfFile, GList *pIconList)  // garde l'ordre des icones.
{
	cd_message ("%s ()\n", __func__);
	GError *erreur = NULL;

	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = pOpenedKeyFile;
	if (pKeyFile == NULL)
	{
		pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, cConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_message ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return ;
		}
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
			pListeModule = g_slist_append (pListeModule, icon->pModule->pVisitCard->cModuleName);
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
	if (pOpenedKeyFile == NULL)
		g_key_file_free (pKeyFile);
}



void cairo_dock_free_module (CairoDockModule *module)
{
	if (module == NULL)
		return ;
	cd_message ("%s (%s)\n", __func__, module->pVisitCard->cModuleName);

	cairo_dock_deactivate_module (module);

	cairo_dock_close_module (module);

	cairo_dock_free_visit_card (module->pVisitCard);
	g_free (module->cSoFilePath);
	g_free (module->cConfFilePath);
	g_free (module);
}


GKeyFile *cairo_dock_pre_read_module_config (CairoDockModule *pModule, CairoDockMinimalAppletConfig *pMinimalConfig)
{
	g_return_val_if_fail (pModule->cConfFilePath != NULL, NULL);

	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, pModule->cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}

	gboolean bNeedsUpgrade = cairo_dock_conf_file_needs_update (pKeyFile, pModule->pVisitCard->cModuleVersion);
	if (bNeedsUpgrade)
	{
		cairo_dock_flush_conf_file (pKeyFile, pModule->cConfFilePath, pModule->pVisitCard->cShareDataDir);
		g_key_file_free (pKeyFile);
		pKeyFile = g_key_file_new ();

		g_key_file_load_from_file (pKeyFile, pModule->cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
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

	if (! g_key_file_has_group (pKeyFile, "Desklet"))  // cette applet ne peut pas se detacher.
	{
		pMinimalConfig->iDeskletWidth = -1;
		pMinimalConfig->iDeskletHeight = -1;
	}
	else
	{
		pMinimalConfig->bDeskletUseSize = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "use size", NULL, TRUE, NULL, NULL);
		pMinimalConfig->bDeskletAlwaysDrawIcon = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "always draw icon", NULL, FALSE, NULL, NULL);
		pMinimalConfig->iDeskletWidth = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "width", NULL, 92, NULL, NULL);
		pMinimalConfig->iDeskletHeight = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "height", NULL, 92, NULL, NULL);
		pMinimalConfig->iDeskletPositionX = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "x position", NULL, 0, NULL, NULL);
		pMinimalConfig->iDeskletPositionY = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "y position", NULL, 0, NULL, NULL);
		pMinimalConfig->bIsDetached = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "initially detached", NULL, FALSE, NULL, NULL);
		pMinimalConfig->bKeepBelow = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "keep below", NULL, FALSE, NULL, NULL);
		pMinimalConfig->bKeepAbove = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "keep above", NULL, FALSE, NULL, NULL);
		pMinimalConfig->bOnWidgetLayer = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "on widget layer", NULL, FALSE, NULL, NULL);
	}
	return pKeyFile;
}

Icon * cairo_dock_activate_module (CairoDockModule *module, CairoDock *pDock, GError **erreur)
{
	cd_message ("%s (%s)", __func__, module->pVisitCard->cModuleName);
	if (module == NULL)
	{
		g_set_error (erreur, 1, 1, "%s () : empty module !", __func__);
		return NULL;
	}

	if (module->bActive)
	{
		g_set_error (erreur, 1, 1, "%s () : module %s is already active !", __func__, module->pVisitCard->cModuleName);
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
	
	//\____________________ On cree le container de l'applet, ainsi que son icone.
	CairoDockContainer *pContainer = NULL;
	Icon *pIcon = NULL;
	GKeyFile *pKeyFile = NULL;
	CairoDockMinimalAppletConfig *pMinimalConfig = NULL;
	g_free (module->cConfFilePath);
	module->cConfFilePath = cairo_dock_check_conf_file_exists (module->pVisitCard->cUserDataDir, module->pVisitCard->cShareDataDir, module->pVisitCard->cConfFileName);
	if (module->cConfFilePath != NULL)
	{
		pMinimalConfig = g_new0 (CairoDockMinimalAppletConfig, 1);
		pKeyFile = cairo_dock_pre_read_module_config (module, pMinimalConfig);
		
		if (pMinimalConfig->iDesiredIconWidth > 0)  // le module a une icone, c'est donc une applet.
		{
			module->bCanDetach = pMinimalConfig->iDeskletWidth > 0;
			gchar *cDockName = CAIRO_DOCK_MAIN_DOCK_NAME;  // par defaut ...
			
			if (module->bCanDetach && pMinimalConfig->bIsDetached)
			{
				CairoDockDesklet *pDesklet = cairo_dock_create_desklet (NULL, NULL, pMinimalConfig->bOnWidgetLayer);
				pContainer = CAIRO_DOCK_CONTAINER (pDesklet);
				cairo_dock_place_desklet (pDesklet, pMinimalConfig);
				while (gtk_events_pending ())
					gtk_main_iteration ();
			}
			else
			{
				pContainer = CAIRO_DOCK_CONTAINER (pDock);
			}
			module->pContainer = pContainer;
			
			int iIconWidth, iIconHeight;
			if (CAIRO_DOCK_IS_DOCK (pContainer))
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
				module);
			pIcon->cParentDockName = g_strdup (cDockName);
			if (CAIRO_DOCK_IS_DESKLET (pContainer))
			{
				CairoDockDesklet *pDesklet = CAIRO_DOCK_DESKLET (pContainer);
				pDesklet->pIcon = pIcon;
				///gtk_widget_queue_draw (pContainer->pWidget);
			}
			cairo_dock_free_minimal_config (pMinimalConfig);
		}
	}

	//\____________________ On initialise le module.
	module->initModule (pKeyFile, pIcon, pContainer, module->cConfFilePath, &tmp_erreur);
	if (pKeyFile != NULL)
		g_key_file_free (pKeyFile);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}

	module->bActive = TRUE;
	return pIcon;
}


void cairo_dock_deactivate_module (CairoDockModule *module)
{
	if (module != NULL && module->bActive)
	{
		if (module->stopModule != NULL)
		{
			module->stopModule ();
		}
		module->bActive = FALSE;

		if (CAIRO_DOCK_IS_DESKLET (module->pContainer))
			cairo_dock_free_desklet (CAIRO_DOCK_DESKLET (module->pContainer));
	}
}


void cairo_dock_reload_module (CairoDockModule *module, gboolean bReloadAppletConf)
{
	GError *erreur = NULL;
	g_return_if_fail (module != NULL);
	CairoDockContainer *pActualContainer = module->pContainer;
	gboolean bContainerTypeChanged = FALSE;
	module->pContainer = NULL;
	cd_message ("%s (%s)", __func__, module->pVisitCard->cModuleName);

	//\______________ On tente de recharger le module.
	gboolean bModuleReloaded = FALSE;
	if (module->bActive && module->reloadModule != NULL)
	{
		Icon *pIcon = cairo_dock_find_icon_from_module (module, pActualContainer);

		GKeyFile *pKeyFile = NULL;
		CairoDockMinimalAppletConfig *pMinimalConfig = NULL;
		gboolean bToBeInserted = FALSE;
		CairoDockContainer *pNewContainer = NULL;
		if (bReloadAppletConf && module->cConfFilePath != NULL)
		{
			pMinimalConfig = g_new0 (CairoDockMinimalAppletConfig, 1);
			pKeyFile = cairo_dock_pre_read_module_config (module, pMinimalConfig);

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
					CairoDockDesklet *pDesklet;
					if (CAIRO_DOCK_IS_DOCK (pActualContainer))  // elle etait dans un dock.
					{
						cairo_dock_detach_icon_from_dock (pIcon, CAIRO_DOCK_DOCK (pActualContainer), g_bUseSeparator);
						cairo_dock_update_dock_size (CAIRO_DOCK_DOCK (pActualContainer));
						pDesklet = cairo_dock_create_desklet (pIcon, NULL, pMinimalConfig->bOnWidgetLayer);
						bContainerTypeChanged = TRUE;
					}
					else
					{
						pDesklet = CAIRO_DOCK_DESKLET (pActualContainer);
					}
					pNewContainer = CAIRO_DOCK_CONTAINER (pDesklet);
					cairo_dock_place_desklet (pDesklet, pMinimalConfig);
				}
				else  // l'applet est maintenant dans un dock.
				{
					CairoDock *pDock;
					if (CAIRO_DOCK_IS_DESKLET (pActualContainer))  // elle etait dans un desklet.
					{
						pDock = g_pMainDock;  // par defaut.
						cairo_dock_free_desklet (CAIRO_DOCK_DESKLET (pActualContainer));
						pActualContainer = NULL;
						bToBeInserted = TRUE;
						bContainerTypeChanged = TRUE;
					}
					else
					{
						pDock = CAIRO_DOCK_DOCK (pActualContainer);
					}
					pNewContainer = CAIRO_DOCK_CONTAINER (pDock);
				}
			}
		}
		else
			pNewContainer = pActualContainer;

		module->pContainer = pNewContainer;
		if (CAIRO_DOCK_IS_DOCK (pNewContainer))
		{
			cairo_dock_load_one_icon_from_scratch (pIcon, pNewContainer);

			if (bToBeInserted)
			{
				CairoDock *pDock = CAIRO_DOCK_DOCK (pNewContainer);
				cairo_dock_insert_icon_in_dock (pIcon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
				cairo_dock_start_animation (pIcon, pDock);
			}
			else if (bReloadAppletConf)
				cairo_dock_update_dock_size (CAIRO_DOCK_DOCK (pNewContainer));
		}

		bModuleReloaded = module->reloadModule (pKeyFile, module->cConfFilePath, pNewContainer);

		cairo_dock_free_minimal_config (pMinimalConfig);
		if (pKeyFile != NULL)
			g_key_file_free (pKeyFile);
	}

	//\______________ Si cela n'a pas marche, on applique la sequence stop + init.
	if (! bModuleReloaded)
	{
		//\______________ On recupere l'eventuelle icone du module.
		Icon *pOldIcon = cairo_dock_find_icon_from_module (module, pActualContainer);

		//\______________ On desactive le module et on retire son eventuelle icone du dock.
		gboolean bToBeRemoved = (pOldIcon != NULL && CAIRO_DOCK_IS_DOCK (pActualContainer));
		cairo_dock_deactivate_module (module);
		if (bToBeRemoved)
		{
			cd_message ("  enlevement de l'ancienne icone (%.1f)", pOldIcon->fOrder);
			pOldIcon->pModule = NULL;
			cairo_dock_remove_icon_from_dock (CAIRO_DOCK_DOCK (pActualContainer), pOldIcon);
		}

		//\______________ On le reactive.
		Icon *pNewIcon = cairo_dock_activate_module (module, g_pMainDock, &erreur);
		if (erreur != NULL)
		{
			module->bActive = FALSE;
			cairo_dock_update_conf_file_with_active_modules (NULL, g_cConfFile, g_pMainDock->icons);
			cd_warning ("Attention : %s", erreur->message);
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

			cd_message ("pNewIcon->fOrder <- %.1f", pNewIcon->fOrder);
			if (CAIRO_DOCK_IS_DOCK (module->pContainer))
			{
				cairo_dock_insert_icon_in_dock (pNewIcon, CAIRO_DOCK_DOCK (module->pContainer), ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);

				cairo_dock_redraw_my_icon (pNewIcon, module->pContainer);
			}
		}
		/**else if (pOldIcon != NULL)
		{
			cairo_dock_update_dock_size (pDock);
			gtk_widget_queue_draw (pDock->pWidget);
		}*/
		cairo_dock_free_icon (pOldIcon);

		if (bReloadAppletConf)
		{
			if (CAIRO_DOCK_IS_DOCK (module->pContainer))
				cairo_dock_update_dock_size (CAIRO_DOCK_DOCK (module->pContainer));
		}
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
	gchar *list[2] = {cModuleName, NULL};
	cairo_dock_activate_modules_from_list (list, g_pMainDock, 0);
	
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	if (CAIRO_DOCK_IS_DOCK (pModule->pContainer))
	{
		CairoDock *pDock = CAIRO_DOCK_DOCK (pModule->pContainer);
		cairo_dock_update_dock_size (pDock);
		gtk_widget_queue_draw (pDock->pWidget);
	}
	
	cairo_dock_update_conf_file_with_active_modules (NULL, g_cConfFile, g_pMainDock->icons);
}
void cairo_dock_deactivate_module_and_unload (gchar *cModuleName)
{
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	Icon *pIcon = cairo_dock_find_icon_from_module (pModule, pModule->pContainer);
	
	if (CAIRO_DOCK_IS_DOCK (pModule->pContainer))
	{
		CairoDock *pDock = CAIRO_DOCK_DOCK (pModule->pContainer);
		cairo_dock_remove_icon_from_dock (pDock, pIcon);  // desactive le module.
		cairo_dock_update_dock_size (pDock);
		gtk_widget_queue_draw (pDock->pWidget);
	}
	else
	{
		cairo_dock_deactivate_module (pModule);
	}
	cairo_dock_update_conf_file_with_active_modules (NULL, g_cConfFile, g_pMainDock->icons);
	cairo_dock_free_icon (pIcon);
}


static void _cairo_dock_configure_module_callback (gchar *cConfFile, gpointer *data)
{
	g_return_if_fail (data != NULL);
	CairoDockModule *module = data[0];
	gboolean bReloadAppletConf = GPOINTER_TO_INT (data[1]);
	cairo_dock_reload_module (module, bReloadAppletConf);
}
void cairo_dock_configure_module (GtkWindow *pParentWindow, CairoDockModule *module, CairoDock *pDock, GError **erreur)
{
	g_return_if_fail (module != NULL);
	//g_print ("%s (%s)\n", __func__, module->cConfFilePath);

	if (module->cConfFilePath == NULL)
		return;

	gchar *cTitle = g_strdup_printf (_("Configuration of %s"), module->pVisitCard->cModuleName);
	gpointer *data = g_new (gpointer, 2);
	data[0]= module;
	data[1] = GINT_TO_POINTER (TRUE);  // reload applet conf file.
	gboolean configuration_ok = cairo_dock_edit_conf_file (pParentWindow, module->cConfFilePath, cTitle, CAIRO_DOCK_MODULE_PANEL_WIDTH, CAIRO_DOCK_MODULE_PANEL_HEIGHT, 0, NULL, (CairoDockConfigFunc) _cairo_dock_configure_module_callback, data, (GFunc) g_free, module->pVisitCard->cGettextDomain);
	g_free (cTitle);
}



Icon *cairo_dock_find_icon_from_module (CairoDockModule *module, CairoDockContainer *pContainer)
{
	if (! module->bActive || pContainer == NULL)
		return NULL;

	if (CAIRO_DOCK_IS_DOCK (pContainer))
	{
		CairoDock *pDock = CAIRO_DOCK_DOCK (pContainer);
		Icon *icon;
		GList *ic;
		for (ic = pDock->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->pModule == module)
				return icon;
		}
	}
	else if (CAIRO_DOCK_IS_DESKLET (pContainer))
	{
		CairoDockDesklet *pDesklet = CAIRO_DOCK_DESKLET (pContainer);
		return pDesklet->pIcon;
	}
	return NULL;
}

CairoDockModule *cairo_dock_find_module_from_name (gchar *cModuleName)
{
	//g_print ("%s (%s)\n", __func__, cModuleName);
	g_return_val_if_fail (cModuleName != NULL, NULL);
	return g_hash_table_lookup (s_hModuleTable, cModuleName);
}




static void _cairo_dock_for_one_desklet (gchar *cModuleName, CairoDockModule *pModule, gpointer *data)
{
	if (CAIRO_DOCK_IS_DESKLET (pModule->pContainer))
	{
		CairoDockDesklet *pDesklet = CAIRO_DOCK_DESKLET (pModule->pContainer);
		CairoDockForeachDeskletFunc pCallback = data[0];
		gpointer user_data = data[1];
		
		pCallback (pDesklet, pModule, user_data);
	}
}
void cairo_dock_foreach_desklet (CairoDockForeachDeskletFunc pCallback, gpointer user_data)
{
	gpointer data[2] = {pCallback, user_data};
	g_hash_table_foreach (s_hModuleTable, (GHFunc) _cairo_dock_for_one_desklet, data);
}
