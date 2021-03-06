/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "cairo-dock-modules.h"
#include "cairo-dock-load.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dock-facility.h"
#include "cairo-dock-log.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-draw-opengl.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-internal-taskbar.h"
#include "cairo-dock-internal-indicators.h"
#include "cairo-dock-container.h"
#define _INTERNAL_MODULE_
#include "cairo-dock-internal-icons.h"

CairoConfigIcons myIcons;
extern CairoDock *g_pMainDock;
extern gchar *g_cCurrentThemePath;
extern gchar *g_cCurrentLaunchersPath;
extern gboolean g_bUseOpenGL;
extern double g_fBackgroundImageWidth, g_fBackgroundImageHeight;

static const gchar * s_cIconTypeNames[(CAIRO_DOCK_NB_TYPES+1)/2] = {"launchers", "applications", "applets"};

static gboolean get_config (GKeyFile *pKeyFile, CairoConfigIcons *pIcons)
{
	gboolean bFlushConfFileNeeded = FALSE;
	
	//\___________________ Ordre des icones.
	int i;
	for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
		pIcons->tIconTypeOrder[i] = i;
	gsize length=0;
	
	int iIconsTypesList[3] = {0,0,0};
	int iDefaultValues[3] = {0,0,0};
	cairo_dock_get_integer_list_key_value (pKeyFile, "Icons", "icon's type order", &bFlushConfFileNeeded, iIconsTypesList, 3, NULL, "Cairo Dock", NULL);
	if (iIconsTypesList[0] == 0 && iIconsTypesList[1] == 0)  // old format.
	{
		g_print ("icon's type order : old format\n");
		gchar **cIconsTypesList = cairo_dock_get_string_list_key_value (pKeyFile, "Icons", "icon's type order", &bFlushConfFileNeeded, &length, NULL, "Cairo Dock", NULL);
		
		if (cIconsTypesList != NULL && length > 0)
		{
			g_print (" conversion ...\n");
			unsigned int i, j;
			for (i = 0; i < length; i ++)
			{
				g_print (" %d) %s\n", i, cIconsTypesList[i]);
				for (j = 0; j < ((CAIRO_DOCK_NB_TYPES + 1) / 2); j ++)
				{
					if (strcmp (cIconsTypesList[i], s_cIconTypeNames[j]) == 0)
					{
						g_print ("   => %d\n", j);
						pIcons->tIconTypeOrder[2*j] = 2 * i;
					}
				}
			}
		}
		g_strfreev (cIconsTypesList);
		
		iIconsTypesList[0] = pIcons->tIconTypeOrder[2*0]/2;
		iIconsTypesList[1] = pIcons->tIconTypeOrder[2*1]/2;
		iIconsTypesList[2] = pIcons->tIconTypeOrder[2*2]/2;
		g_print ("mise a jour avec {%d;%d;%d}\n", iIconsTypesList[0], iIconsTypesList[1], iIconsTypesList[2]);
		g_key_file_set_integer_list (pKeyFile, "Icons", "icon's type order", iIconsTypesList, 3);
		bFlushConfFileNeeded = TRUE;
	}
	for (i = 0; i < 3; i ++)
		pIcons->tIconTypeOrder[2*i] = 2*iIconsTypesList[i];
	
	pIcons->bMixAppletsAndLaunchers = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "mix applets with launchers", &bFlushConfFileNeeded, FALSE , NULL, NULL);
	if (pIcons->bMixAppletsAndLaunchers)
		pIcons->tIconTypeOrder[CAIRO_DOCK_APPLET] = pIcons->tIconTypeOrder[CAIRO_DOCK_LAUNCHER];
	
	//\___________________ Reflets.
	pIcons->fFieldDepth = cairo_dock_get_double_key_value (pKeyFile, "Icons", "field depth", &bFlushConfFileNeeded, 0.7, NULL, NULL);

	pIcons->fAlbedo = cairo_dock_get_double_key_value (pKeyFile, "Icons", "albedo", &bFlushConfFileNeeded, .6, NULL, NULL);

	double fMaxScale = cairo_dock_get_double_key_value (pKeyFile, "Icons", "zoom max", &bFlushConfFileNeeded, 0., NULL, NULL);
	if (fMaxScale == 0)
	{
		pIcons->fAmplitude = g_key_file_get_double (pKeyFile, "Icons", "amplitude", NULL);
		fMaxScale = 1 + pIcons->fAmplitude;
		g_key_file_set_double (pKeyFile, "Icons", "zoom max", fMaxScale);
	}
	else
		pIcons->fAmplitude = fMaxScale - 1;

	pIcons->iSinusoidWidth = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "sinusoid width", &bFlushConfFileNeeded, 250, NULL, NULL);
	pIcons->iSinusoidWidth = MAX (1, pIcons->iSinusoidWidth);

	pIcons->iIconGap = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "icon gap", &bFlushConfFileNeeded, 0, NULL, NULL);

	//\___________________ Ficelle.
	pIcons->iStringLineWidth = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "string width", &bFlushConfFileNeeded, 0, NULL, NULL);

	gdouble couleur[4];
	cairo_dock_get_double_list_key_value (pKeyFile, "Icons", "string color", &bFlushConfFileNeeded, pIcons->fStringColor, 4, couleur, NULL, NULL);

	pIcons->fAlphaAtRest = cairo_dock_get_double_key_value (pKeyFile, "Icons", "alpha at rest", &bFlushConfFileNeeded, 1., NULL, NULL);
	
	//\___________________ Themes.
	pIcons->bUseLocalIcons = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "local icons", &bFlushConfFileNeeded, TRUE , NULL, NULL);
	pIcons->pDirectoryList = cairo_dock_get_string_list_key_value (pKeyFile, "Icons", "default icon directory", &bFlushConfFileNeeded, &length, NULL, "Launchers", NULL);
	
	pIcons->pDefaultIconDirectory = g_new0 (gpointer, 2 * length + 2 + 2);  // +2 pour le theme local et +2 pour les NULL final.
	int j = 0;
	
	if (pIcons->bUseLocalIcons)
	{
		pIcons->pDefaultIconDirectory[j] = g_strdup_printf ("%s/%s", g_cCurrentThemePath, CAIRO_DOCK_LOCAL_ICONS_DIR);
		cd_message (" utilisation du repertoire local %s", pIcons->pDefaultIconDirectory[j]);
		j += 2;
	}
	
	if (pIcons->pDirectoryList != NULL && pIcons->pDirectoryList[0] != NULL)
	{
		for (i = 0; pIcons->pDirectoryList[i] != NULL; i ++)
		{
			if (pIcons->pDirectoryList[i][0] == '\0')
				continue;
			if (pIcons->pDirectoryList[i][0] == '~')
			{
				pIcons->pDefaultIconDirectory[j] = g_strdup_printf ("%s%s", getenv ("HOME"), pIcons->pDirectoryList[i]+1);
				cd_message (" utilisation du repertoire %s", pIcons->pDefaultIconDirectory[j]);
				j += 2;
			}
			else if (pIcons->pDirectoryList[i][0] == '/')
			{
				pIcons->pDefaultIconDirectory[j] = g_strdup (pIcons->pDirectoryList[i]);
				cd_message (" utilisation du repertoire %s", pIcons->pDefaultIconDirectory[j]);
				j += 2;
			}
			else if (strncmp (pIcons->pDirectoryList[i], "_LocalTheme_", 12) == 0)
			{
				if (bFlushConfFileNeeded)
				{
					g_key_file_set_string_list (pKeyFile,
						"Icons",
						"default icon directory",
						pIcons->pDirectoryList,
						length-1);
				}
				else
				{
					gchar *cCommand = g_strdup_printf ("sed -i \"s/_LocalTheme_;*//g\" '%s/%s'", g_cCurrentThemePath, CAIRO_DOCK_CONF_FILE);
					cd_message ("%s", cCommand);
					int r = system (cCommand);
					g_free (cCommand);
					g_free (pIcons->pDirectoryList[i]);
					pIcons->pDirectoryList[i] = NULL;
					if (pIcons->pDirectoryList[i+1] != NULL)
					{
						pIcons->pDirectoryList[i] = pIcons->pDirectoryList[i+1];
						pIcons->pDirectoryList[i+1] = NULL;
					}
				}
			}
			else if (strncmp (pIcons->pDirectoryList[i], "_ThemeDirectory_", 16) == 0)
			{
				cd_warning ("Cairo-Dock's local icons are now located in the 'icons' folder, they will be moved there");
				gchar *cCommand = g_strdup_printf ("cd '%s' && mv *.svg *.png *.xpm *.jpg *.bmp *.gif '%s/%s' > /dev/null", g_cCurrentLaunchersPath, g_cCurrentThemePath, CAIRO_DOCK_LOCAL_ICONS_DIR);
				cd_message ("%s", cCommand);
				int r = system (cCommand);
				g_free (cCommand);
				cCommand = g_strdup_printf ("sed -i \"s/_ThemeDirectory_;*//g\" '%s/%s'", g_cCurrentThemePath, CAIRO_DOCK_CONF_FILE);
				cd_message ("%s", cCommand);
				r = system (cCommand);
				g_free (cCommand);
			}
			else
			{
				cd_message (" utilisation du theme %s", pIcons->pDirectoryList[i]);
				pIcons->pDefaultIconDirectory[j+1] = gtk_icon_theme_new ();
				gtk_icon_theme_set_custom_theme (pIcons->pDefaultIconDirectory[j+1], pIcons->pDirectoryList[i]);
				j += 2;
			}
		}
	}
	
	gchar *cLauncherBackgroundImageName = cairo_dock_get_string_key_value (pKeyFile, "Icons", "icons bg", &bFlushConfFileNeeded, NULL, NULL, NULL);
	if (cLauncherBackgroundImageName != NULL)
	{
		pIcons->cBackgroundImagePath = cairo_dock_generate_file_path (cLauncherBackgroundImageName);
		g_free (cLauncherBackgroundImageName);
		pIcons->bBgForApplets = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "bg for applets", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	}

	//\___________________ Parametres des lanceurs.
	cairo_dock_get_size_key_value_helper (pKeyFile, "Icons", "launcher ", bFlushConfFileNeeded, pIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER], pIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER]);
	if (pIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] == 0)
		pIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] = 48;
	if (pIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] == 0)
		pIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] = 48;
	
	//\___________________ Parametres des applis.
	cairo_dock_get_size_key_value_helper (pKeyFile, "Icons", "appli ", bFlushConfFileNeeded, pIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLI], pIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLI]);
	if (pIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLI] == 0)
		pIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLET] = pIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER];
	if (pIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLI] == 0)
		pIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLET] = pIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER];
	
	//\___________________ Parametres des applets.
	cairo_dock_get_size_key_value_helper (pKeyFile, "Icons", "applet ", bFlushConfFileNeeded, pIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLET], pIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLET]);
	if (pIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLET] == 0)
		pIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLET] = pIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER];
	if (pIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLET] == 0)
		pIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLET] = pIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER];
	
	//\___________________ Parametres des separateurs.
	cairo_dock_get_size_key_value_helper (pKeyFile, "Icons", "separator ", bFlushConfFileNeeded, pIcons->tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR12], pIcons->tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR12]);
	pIcons->tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR23] = pIcons->tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR12];
	pIcons->tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR23] = pIcons->tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR12];

	pIcons->bUseSeparator = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "use separator", &bFlushConfFileNeeded, TRUE, "Separators", NULL);

	pIcons->cSeparatorImage = cairo_dock_get_string_key_value (pKeyFile, "Icons", "separator image", &bFlushConfFileNeeded, NULL, "Separators", NULL);

	pIcons->bRevolveSeparator = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "revolve separator image", &bFlushConfFileNeeded, TRUE, "Separators", NULL);

	pIcons->bConstantSeparatorSize = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "force size", &bFlushConfFileNeeded, TRUE, "Separators", NULL);
	
	
	pIcons->fReflectSize = 0;
	for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
	{
		if (pIcons->tIconAuthorizedHeight[i] > 0)
			pIcons->fReflectSize = MAX (pIcons->fReflectSize, pIcons->tIconAuthorizedHeight[i]);
	}
	if (pIcons->fReflectSize == 0)  // on n'a pas trouve de hauteur, on va essayer avec la largeur.
	{
		for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
		{
			if (pIcons->tIconAuthorizedWidth[i] > 0)
				pIcons->fReflectSize = MAX (pIcons->fReflectSize, pIcons->tIconAuthorizedWidth[i]);
		}
		if (pIcons->fReflectSize > 0)
			pIcons->fReflectSize = MIN (48, pIcons->fReflectSize);
		else
			pIcons->fReflectSize = 48;
	}
	pIcons->fReflectSize *= pIcons->fFieldDepth;
	
	return bFlushConfFileNeeded;
}


static void reset_config (CairoConfigIcons *pIcons)
{
	if (pIcons->pDefaultIconDirectory != NULL)
	{
		gpointer data;
		int i;
		for (i = 0; (pIcons->pDefaultIconDirectory[2*i] != NULL || pIcons->pDefaultIconDirectory[2*i+1] != NULL); i ++)
		{
			if (pIcons->pDefaultIconDirectory[2*i] != NULL)
				g_free (pIcons->pDefaultIconDirectory[2*i]);
			else
				g_object_unref (pIcons->pDefaultIconDirectory[2*i+1]);
		}
	}
	g_free (pIcons->cSeparatorImage);
	g_free (pIcons->cBackgroundImagePath);
	g_strfreev (pIcons->pDirectoryList);
}


static void reload (CairoConfigIcons *pPrevIcons, CairoConfigIcons *pIcons)
{
	CairoDock *pDock = g_pMainDock;
	double fMaxScale = cairo_dock_get_max_scale (pDock);
	cairo_t* pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	gboolean bInsertSeparators = FALSE;
	
	gboolean bGroupOrderChanged;
	if (pPrevIcons->tIconTypeOrder[CAIRO_DOCK_LAUNCHER] != pIcons->tIconTypeOrder[CAIRO_DOCK_LAUNCHER] ||
		pPrevIcons->tIconTypeOrder[CAIRO_DOCK_APPLI] != pIcons->tIconTypeOrder[CAIRO_DOCK_APPLI] ||
		pPrevIcons->tIconTypeOrder[CAIRO_DOCK_APPLET] != pIcons->tIconTypeOrder[CAIRO_DOCK_APPLET] ||
		pPrevIcons->bMixAppletsAndLaunchers != pIcons->bMixAppletsAndLaunchers)
		bGroupOrderChanged = TRUE;
	else
		bGroupOrderChanged = FALSE;
	
	if (bGroupOrderChanged)
	{
		bInsertSeparators = TRUE;  // on enleve les separateurs avant de re-ordonner.
		cairo_dock_remove_automatic_separators (pDock);
		pDock->icons = g_list_sort (pDock->icons, (GCompareFunc) cairo_dock_compare_icons_order);
	}
	
	if ((pPrevIcons->bUseSeparator && ! pIcons->bUseSeparator) ||
		pPrevIcons->cSeparatorImage != pIcons->cSeparatorImage ||
		pPrevIcons->tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR12] != pIcons->tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR12] ||
		pPrevIcons->tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR12] != pIcons->tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR12] ||
		pPrevIcons->fAmplitude != pIcons->fAmplitude)
	{
		bInsertSeparators = TRUE;
		cairo_dock_remove_automatic_separators (pDock);
	}
	
	gboolean bThemeChanged = (pPrevIcons->bUseLocalIcons != pIcons->bUseLocalIcons);
	if ((pIcons->pDirectoryList == NULL && pPrevIcons->pDirectoryList != NULL) || (pIcons->pDirectoryList != NULL && pPrevIcons->pDirectoryList == NULL))
	{
		bThemeChanged = TRUE;
	}
	else if (pIcons->pDirectoryList != NULL && pPrevIcons->pDirectoryList != NULL)
	{
		int i;
		for (i = 0; pIcons->pDirectoryList[i] != NULL || pPrevIcons->pDirectoryList[i] != NULL; i ++)
		{
			if (cairo_dock_strings_differ (pIcons->pDirectoryList[i], pPrevIcons->pDirectoryList[i]))
			{
				bThemeChanged = TRUE;
				break ;
			}
		}
	}
	
	gboolean bIconBackgroundImagesChanged = FALSE;
	// if background images are different, reload them and trigger the reload of all icons
	if (cairo_dock_strings_differ (pPrevIcons->cBackgroundImagePath, pIcons->cBackgroundImagePath) ||
		pPrevIcons->fAmplitude != pIcons->fAmplitude)
	{
		bIconBackgroundImagesChanged = TRUE;
		cairo_dock_load_icons_background_surface (pIcons->cBackgroundImagePath, pCairoContext, fMaxScale);
	}
	if (pPrevIcons->bBgForApplets != pIcons->bBgForApplets && pIcons->cBackgroundImagePath != NULL)
	{
		bIconBackgroundImagesChanged = TRUE;  // on pourrait faire plus fin en ne rechargeant que les applets mais bon.
	}
	
	if (pPrevIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] != pIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] ||
		pPrevIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] != pIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] ||
		pPrevIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLI] != pIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLI] ||
		pPrevIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLI] != pIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLI] ||
		pPrevIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLET] != pIcons->tIconAuthorizedWidth[CAIRO_DOCK_APPLET] ||
		pPrevIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLET] != pIcons->tIconAuthorizedHeight[CAIRO_DOCK_APPLET] ||
		pPrevIcons->fAmplitude != pIcons->fAmplitude ||
		(!g_bUseOpenGL && pPrevIcons->fFieldDepth != pIcons->fFieldDepth) ||
		(!g_bUseOpenGL && pPrevIcons->fAlbedo != pIcons->fAlbedo) ||
		bThemeChanged ||
		bIconBackgroundImagesChanged)  // oui on ne fait pas dans la finesse.
	{
		g_fBackgroundImageWidth = 0.;  // pour mettre a jour les decorations.
		g_fBackgroundImageHeight = 0.;
		cairo_dock_reload_buffers_in_all_docks (TRUE);  // TRUE <=> y compris les applets.
	}
	
	cairo_dock_create_icon_pbuffer ();
	
	if (bInsertSeparators)
	{
		cairo_dock_insert_separators_in_dock (pDock);
	}
	
	if (pPrevIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] != pIcons->tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] ||
		pPrevIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] != pIcons->tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] ||
		pPrevIcons->fAmplitude != pIcons->fAmplitude)
	{
		cairo_dock_load_active_window_indicator (pCairoContext,
			myIndicators.cActiveIndicatorImagePath,
			fMaxScale,
			myIndicators.iActiveCornerRadius,
			myIndicators.iActiveLineWidth,
			myIndicators.fActiveColor);
		if (myTaskBar.bShowAppli && myTaskBar.bGroupAppliByClass)
			cairo_dock_load_class_indicator (myIndicators.cClassIndicatorImagePath,
				pCairoContext,
				fMaxScale);
		if (myTaskBar.bShowAppli && myTaskBar.bMixLauncherAppli)
			cairo_dock_load_task_indicator (myIndicators.cIndicatorImagePath,
				pCairoContext,
				fMaxScale,
				myIndicators.fIndicatorRatio);
	}
	
	g_fBackgroundImageWidth = g_fBackgroundImageHeight = 0.;
	cairo_dock_set_all_views_to_default (0);  // met a jour la taille (decorations incluses) de tous les docks.
	cairo_dock_calculate_dock_icons (pDock);
	cairo_dock_redraw_root_docks (FALSE);  // main dock inclus.
	
	cairo_destroy (pCairoContext);
}


DEFINE_PRE_INIT (Icons)
{
	static const gchar *cDependencies[3] = {"Animated icons", N_("It provides many animations to your icons."), NULL};
	pModule->cModuleName = "Icons";
	pModule->cTitle = N_("Icons");
	pModule->cIcon = CAIRO_DOCK_SHARE_DATA_DIR"/icon-icons.svg";
	pModule->cDescription = N_("All about icons :\n size, reflection, icon theme, ...");
	pModule->iCategory = CAIRO_DOCK_CATEGORY_THEME;
	pModule->iSizeOfConfig = sizeof (CairoConfigIcons);
	pModule->iSizeOfData = 0;
	
	pModule->reload = (CairoDockInternalModuleReloadFunc) reload;
	pModule->get_config = (CairoDockInternalModuleGetConfigFunc) get_config;
	pModule->reset_config = (CairoDockInternalModuleResetConfigFunc) reset_config;
	pModule->reset_data = NULL;
	
	pModule->pConfig = (CairoInternalModuleConfigPtr) &myIcons;
	pModule->pData = NULL;
}
