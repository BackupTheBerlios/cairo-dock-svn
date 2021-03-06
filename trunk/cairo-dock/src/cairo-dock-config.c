/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <math.h>
#include <string.h>

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_LIBCRYPT
/* libC crypt */
#include <crypt.h>

static char DES_crypt_key[64] =
{
    1,0,0,1,1,1,0,0, 1,0,1,1,1,0,1,1, 1,1,0,1,0,1,0,1, 1,1,0,0,0,0,0,1,
    0,0,0,1,0,1,1,0, 1,1,1,0,1,1,1,0, 1,1,1,0,0,1,0,0, 1,0,1,0,1,0,1,1
}; 
#endif

#include "cairo-dock-draw.h"
#include "cairo-dock-draw-opengl.h"
#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-keyfile-utilities.h"
#include "cairo-dock-gui-factory.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-menu.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-X-utilities.h"
#include "cairo-dock-log.h"
#include "cairo-dock-keybinder.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-class-manager.h"
#include "cairo-dock-emblem.h"
#include "cairo-dock-gauge.h"
#include "cairo-dock-desklet.h"
#include "cairo-dock-internal-position.h"
#include "cairo-dock-internal-accessibility.h"
#include "cairo-dock-internal-system.h"
#include "cairo-dock-internal-taskbar.h"
#include "cairo-dock-internal-dialogs.h"
#include "cairo-dock-internal-indicators.h"
#include "cairo-dock-internal-views.h"
#include "cairo-dock-internal-labels.h"
#include "cairo-dock-internal-desklets.h"
#include "cairo-dock-internal-icons.h"
#include "cairo-dock-internal-background.h"
#include "cairo-dock-container.h"
#include "cairo-dock-dock-facility.h"
#include "cairo-dock-config.h"

extern CairoDock *g_pMainDock;
extern gchar *g_cCurrentThemePath;
extern gchar *g_cCairoDockDataDir;
extern gchar *g_cCurrentLaunchersPath;
extern double g_fBackgroundImageWidth;
extern double g_fBackgroundImageHeight;
extern cairo_surface_t *g_pDesktopBgSurface;

static gboolean s_bLoading = FALSE;


gboolean cairo_dock_get_boolean_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, gboolean bDefaultValue, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	gboolean bValue = g_key_file_get_boolean (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning (erreur->message);
		g_error_free (erreur);
		erreur = NULL;

		gchar* cGroupNameUpperCase = g_ascii_strup (cGroupName, -1);
		bValue = g_key_file_get_boolean (pKeyFile, cGroupNameUpperCase, cKeyName, &erreur);
		g_free (cGroupNameUpperCase);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
			bValue = g_key_file_get_boolean (pKeyFile, "Cairo Dock", cKeyName, &erreur);
			if (erreur != NULL)
			{
				g_error_free (erreur);
				erreur = NULL;
				bValue = g_key_file_get_boolean (pKeyFile, (cDefaultGroupName != NULL ? cDefaultGroupName : cGroupName), (cDefaultKeyName != NULL ? cDefaultKeyName : cKeyName), &erreur);
				if (erreur != NULL)
				{
					g_error_free (erreur);
					bValue = bDefaultValue;
				}
				else
					cd_message (" (recuperee)");
			}
			else
				cd_message (" (recuperee)");
		}

		g_key_file_set_boolean (pKeyFile, cGroupName, cKeyName, bValue);
		if (bFlushConfFileNeeded != NULL)
			*bFlushConfFileNeeded = TRUE;
	}
	return bValue;
}

int cairo_dock_get_integer_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, int iDefaultValue, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	int iValue = g_key_file_get_integer (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning (erreur->message);
		g_error_free (erreur);
		erreur = NULL;

		gchar* cGroupNameUpperCase = g_ascii_strup (cGroupName, -1);
		iValue = g_key_file_get_integer (pKeyFile, cGroupNameUpperCase, cKeyName, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
			iValue = g_key_file_get_integer (pKeyFile, "Cairo Dock", cKeyName, &erreur);
			if (erreur != NULL)
			{
				g_error_free (erreur);
				erreur = NULL;
				iValue = g_key_file_get_integer (pKeyFile, (cDefaultGroupName != NULL ? cDefaultGroupName : cGroupName), (cDefaultKeyName != NULL ? cDefaultKeyName : cKeyName), &erreur);
				if (erreur != NULL)
				{
					g_error_free (erreur);
					iValue = iDefaultValue;
				}
				else
					cd_message (" (recuperee)");
			}
			else
				cd_message (" (recuperee)");
		}
		g_free (cGroupNameUpperCase);

		g_key_file_set_integer (pKeyFile, cGroupName, cKeyName, iValue);
		if (bFlushConfFileNeeded != NULL)
			*bFlushConfFileNeeded = TRUE;
	}
	return iValue;
}

double cairo_dock_get_double_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, double fDefaultValue, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	double fValue = g_key_file_get_double (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning (erreur->message);
		g_error_free (erreur);
		erreur = NULL;

		gchar* cGroupNameUpperCase = g_ascii_strup (cGroupName, -1);
		fValue = g_key_file_get_double (pKeyFile, cGroupNameUpperCase, cKeyName, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
			fValue = g_key_file_get_double (pKeyFile, "Cairo Dock", cKeyName, &erreur);
			if (erreur != NULL)
			{
				g_error_free (erreur);
				erreur = NULL;
				fValue = g_key_file_get_double (pKeyFile, (cDefaultGroupName != NULL ? cDefaultGroupName : cGroupName), (cDefaultKeyName != NULL ? cDefaultKeyName : cKeyName), &erreur);
				if (erreur != NULL)
				{
					g_error_free (erreur);
					fValue = fDefaultValue;
				}
				else
					cd_message (" (recuperee)");
			}
			else
				cd_message (" (recuperee)");
		}
		g_free (cGroupNameUpperCase);

		g_key_file_set_double (pKeyFile, cGroupName, cKeyName, fValue);
		if (bFlushConfFileNeeded != NULL)
			*bFlushConfFileNeeded = TRUE;
	}
	return fValue;
}

gchar *cairo_dock_get_string_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, const gchar *cDefaultValue, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	gchar *cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning (erreur->message);
		g_error_free (erreur);
		erreur = NULL;

		gchar* cGroupNameUpperCase = g_ascii_strup (cGroupName, -1);
		cValue = g_key_file_get_string (pKeyFile, cGroupNameUpperCase, cKeyName, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
			cValue = g_key_file_get_string (pKeyFile, "Cairo Dock", cKeyName, &erreur);
			if (erreur != NULL)
			{
				g_error_free (erreur);
				erreur = NULL;
				cValue = g_key_file_get_string (pKeyFile, (cDefaultGroupName != NULL ? cDefaultGroupName : cGroupName), (cDefaultKeyName != NULL ? cDefaultKeyName : cKeyName), &erreur);
				if (erreur != NULL)
				{
					g_error_free (erreur);
					cValue = g_strdup (cDefaultValue);
				}
				else
					cd_message (" (recuperee)");
			}
			else
				cd_message (" (recuperee)");
		}
		g_free (cGroupNameUpperCase);

		g_key_file_set_string (pKeyFile, cGroupName, cKeyName, (cValue != NULL ? cValue : ""));
		if (bFlushConfFileNeeded != NULL)
			*bFlushConfFileNeeded = TRUE;
	}
	if (cValue != NULL && *cValue == '\0')
	{
		g_free (cValue);
		cValue = NULL;
	}
	return cValue;
}

void cairo_dock_get_integer_list_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, int *iValueBuffer, guint iNbElements, int *iDefaultValues, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	gsize length = 0;
	if (iDefaultValues != NULL)
		memcpy (iValueBuffer, iDefaultValues, iNbElements * sizeof (int));

	int *iValuesList = g_key_file_get_integer_list (pKeyFile, cGroupName, cKeyName, &length, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning (erreur->message);
		g_error_free (erreur);
		erreur = NULL;

		gchar* cGroupNameUpperCase = g_ascii_strup (cGroupName, -1);
		iValuesList = g_key_file_get_integer_list (pKeyFile, cGroupNameUpperCase, cKeyName, &length, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
			iValuesList = g_key_file_get_integer_list (pKeyFile, "Cairo Dock", cKeyName, &length, &erreur);
			if (erreur != NULL)
			{
				g_error_free (erreur);
				erreur = NULL;
				iValuesList = g_key_file_get_integer_list (pKeyFile, (cDefaultGroupName != NULL ? cDefaultGroupName : cGroupName), (cDefaultKeyName != NULL ? cDefaultKeyName : cKeyName), &length, &erreur);
				if (erreur != NULL)
				{
					g_error_free (erreur);
				}
				else
				{
					cd_message (" (recuperee)");
					if (length > 0)
						memcpy (iValueBuffer, iValuesList, MIN (iNbElements, length) * sizeof (int));
				}
			}
			else
			{
				cd_message (" (recuperee)");
				if (length > 0)
					memcpy (iValueBuffer, iValuesList, MIN (iNbElements, length) * sizeof (int));
			}
		}
		else
		{
			if (length > 0)
				memcpy (iValueBuffer, iValuesList, MIN (iNbElements, length) * sizeof (int));
		}
		g_free (cGroupNameUpperCase);
		
		if (iDefaultValues != NULL)  // on ne modifie les valeurs actuelles que si on a explicitement passe des valeurs par defaut en entree; sinon on considere que l'on va traiter le cas en aval.
			g_key_file_set_integer_list (pKeyFile, cGroupName, cKeyName, iValueBuffer, iNbElements);
		if (bFlushConfFileNeeded != NULL)
			*bFlushConfFileNeeded = TRUE;
	}
	else
	{
		if (length > 0)
			memcpy (iValueBuffer, iValuesList, MIN (iNbElements, length) * sizeof (int));
	}
	g_free (iValuesList);
}

void cairo_dock_get_double_list_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, double *fValueBuffer, guint iNbElements, double *fDefaultValues, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	gsize length = 0;
	if (fDefaultValues != NULL)
		memcpy (fValueBuffer, fDefaultValues, iNbElements * sizeof (double));

	double *fValuesList = g_key_file_get_double_list (pKeyFile, cGroupName, cKeyName, &length, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning (erreur->message);
		g_error_free (erreur);
		erreur = NULL;

		gchar* cGroupNameUpperCase = g_ascii_strup (cGroupName, -1);
		fValuesList = g_key_file_get_double_list (pKeyFile, cGroupNameUpperCase, cKeyName, &length, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
			fValuesList = g_key_file_get_double_list (pKeyFile, "Cairo Dock", cKeyName, &length, &erreur);
			if (erreur != NULL)
			{
				g_error_free (erreur);
				erreur = NULL;
				fValuesList = g_key_file_get_double_list (pKeyFile, (cDefaultGroupName != NULL ? cDefaultGroupName : cGroupName), (cDefaultKeyName != NULL ? cDefaultKeyName : cKeyName), &length, &erreur);
				if (erreur != NULL)
				{
					g_error_free (erreur);
				}
				else
				{
					cd_message (" (recuperee)");
					if (length > 0)
						memcpy (fValueBuffer, fValuesList, MIN (iNbElements, length) * sizeof (double));
				}
			}
			else
			{
				cd_message (" (recuperee)");
				if (length > 0)
					memcpy (fValueBuffer, fValuesList, MIN (iNbElements, length) * sizeof (double));
			}
		}
		else
		{
			if (length > 0)
				memcpy (fValueBuffer, fValuesList, MIN (iNbElements, length) * sizeof (double));
		}
		g_free (cGroupNameUpperCase);

		g_key_file_set_double_list (pKeyFile, cGroupName, cKeyName, fValueBuffer, iNbElements);
		if (bFlushConfFileNeeded != NULL)
			*bFlushConfFileNeeded = TRUE;
	}
	else
	{
		if (length > 0)
			memcpy (fValueBuffer, fValuesList, MIN (iNbElements, length) * sizeof (double));
	}
	g_free (fValuesList);
}

gchar **cairo_dock_get_string_list_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, gsize *length, const gchar *cDefaultValues, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	*length = 0;
	gchar **cValuesList = g_key_file_get_string_list (pKeyFile, cGroupName, cKeyName, length, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning (erreur->message);
		g_error_free (erreur);
		erreur = NULL;

		gchar* cGroupNameUpperCase = g_ascii_strup (cGroupName, -1);
		cValuesList = g_key_file_get_string_list (pKeyFile, cGroupNameUpperCase, cKeyName, length, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
			cValuesList = g_key_file_get_string_list (pKeyFile, (cDefaultGroupName != NULL ? cDefaultGroupName : cGroupName), (cDefaultKeyName != NULL ? cDefaultKeyName : cKeyName), length, &erreur);
			if (erreur != NULL)
			{
				g_error_free (erreur);
				cValuesList = g_strsplit (cDefaultValues, ";", -1);  // "" -> NULL.
				int i = 0;
				if (cValuesList != NULL)
				{
					while (cValuesList[i] != NULL)
						i ++;
				}
				*length = i;
			}
		}
		g_free (cGroupNameUpperCase);

		if (*length > 0)
			g_key_file_set_string_list (pKeyFile, cGroupName, cKeyName, (const gchar **)cValuesList, *length);
		else
			g_key_file_set_string (pKeyFile, cGroupName, cKeyName, "");
		if (bFlushConfFileNeeded != NULL)
			*bFlushConfFileNeeded = TRUE;
	}
	if (cValuesList != NULL && (cValuesList[0] == NULL || (*(cValuesList[0]) == '\0' && *length == 1)))
	{
		g_strfreev (cValuesList);
		cValuesList = NULL;
		*length = 0;
	}
	return cValuesList;
}

gchar *cairo_dock_get_file_path_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName, const gchar *cDefaultDir, const gchar *cDefaultFileName)
{
	gchar *cFileName = cairo_dock_get_string_key_value (pKeyFile, cGroupName, cKeyName, bFlushConfFileNeeded, NULL, cDefaultGroupName, cDefaultKeyName);
	gchar *cFilePath = NULL;
	if (cFileName != NULL)
		cFilePath = cairo_dock_generate_file_path (cFileName);
	else if (cDefaultFileName != NULL && cDefaultDir != NULL)
		cFilePath = g_strdup_printf ("%s/%s", cDefaultDir, cDefaultFileName);
	return cFilePath;
}
void cairo_dock_get_size_key_value (GKeyFile *pKeyFile, const gchar *cGroupName, const gchar *cKeyName, gboolean *bFlushConfFileNeeded, gint iDefaultSize, const gchar *cDefaultGroupName, const gchar *cDefaultKeyName, int *iWidth, int *iHeight)
{
	int iSize[2];
	int iDefaultValues[2] = {iDefaultSize, iDefaultSize};
	cairo_dock_get_integer_list_key_value (pKeyFile, cGroupName, cKeyName, bFlushConfFileNeeded, iSize, 2, iDefaultValues, cDefaultGroupName, cDefaultKeyName);
	*iWidth = iSize[0];
	*iHeight = iSize[1];
}


void cairo_dock_read_conf_file (gchar *cConfFilePath, CairoDock *pDock)
{
	//g_print ("%s (%s)\n", __func__, cConfFilePath);
	GError *erreur = NULL;
	gsize length;
	gboolean bFlushConfFileNeeded = FALSE, bFlushConfFileNeeded2 = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.

	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	if (pKeyFile == NULL)
		return ;
	
	s_bLoading = TRUE;
	
	//\___________________ On garde une trace de certains parametres.
	gchar *cRaiseDockShortcutOld = myAccessibility.cRaiseDockShortcut;
	myAccessibility.cRaiseDockShortcut = NULL;
	gboolean bPopUpOld = myAccessibility.bPopUp;  // FALSE initialement.
	gboolean bUseFakeTransparencyOld = mySystem.bUseFakeTransparency;  // FALSE initialement.
	///gboolean bUniquePidOld = myTaskBar.bUniquePid;  // FALSE initialement.
	gboolean bGroupAppliByClassOld = myTaskBar.bGroupAppliByClass;  // FALSE initialement.
	gboolean bHideVisibleApplisOld = myTaskBar.bHideVisibleApplis;
	gboolean bAppliOnCurrentDesktopOnlyOld = myTaskBar.bAppliOnCurrentDesktopOnly;
	gboolean bMixLauncherAppliOld = myTaskBar.bMixLauncherAppli;
	gboolean bOverWriteXIconsOld = myTaskBar.bOverWriteXIcons;  // TRUE initialement.
	gboolean bShowThumbnailOld = myTaskBar.bShowThumbnail;
	gchar *cDeskletDecorationsNameOld = myDesklets.cDeskletDecorationsName;
	myDesklets.cDeskletDecorationsName = NULL;
	gboolean bMixAppletsAndLaunchersOld = (myIcons.tIconTypeOrder[CAIRO_DOCK_APPLET] == myIcons.tIconTypeOrder[CAIRO_DOCK_LAUNCHER]);
	gboolean bUseSeparatorOld = myIcons.bUseSeparator;  // TRUE initialement.
	CairoDockIconType tIconTypeOrderOld[CAIRO_DOCK_NB_TYPES];
	memcpy (tIconTypeOrderOld, myIcons.tIconTypeOrder, sizeof (tIconTypeOrderOld));
	
	//\___________________ On recupere la conf de tous les modules.
	bFlushConfFileNeeded = cairo_dock_get_global_config (pKeyFile);
	
	//\___________________ Post-initialisation.
	pDock->iGapX = myPosition.iGapX;
	pDock->iGapY = myPosition.iGapY;
	
	pDock->fAlign = myPosition.fAlign;
	pDock->bAutoHide = myAccessibility.bAutoHide;
	
	gboolean bGroupOrderChanged;
	if (tIconTypeOrderOld[CAIRO_DOCK_LAUNCHER] != myIcons.tIconTypeOrder[CAIRO_DOCK_LAUNCHER] ||
		tIconTypeOrderOld[CAIRO_DOCK_APPLI] != myIcons.tIconTypeOrder[CAIRO_DOCK_APPLI] ||
		tIconTypeOrderOld[CAIRO_DOCK_APPLET] != myIcons.tIconTypeOrder[CAIRO_DOCK_APPLET])
		bGroupOrderChanged = TRUE;
	else
		bGroupOrderChanged = FALSE;
	
	if (myDialogs.bHomogeneous)
	{
		myDialogs.dialogTextDescription.iSize = myLabels.iconTextDescription.iSize;
		if (myDialogs.dialogTextDescription.iSize == 0)
			myDialogs.dialogTextDescription.iSize = 14;
		myDialogs.dialogTextDescription.cFont = g_strdup (myLabels.iconTextDescription.cFont);
		myDialogs.dialogTextDescription.iWeight = myLabels.iconTextDescription.iWeight;
		myDialogs.dialogTextDescription.iStyle = myLabels.iconTextDescription.iStyle;
	}
	
	///cairo_dock_updated_emblem_conf_file (pKeyFile, &bFlushConfFileNeeded);
	
	//\___________________ On (re)charge tout, car n'importe quel parametre peut avoir change.
	if (myPosition.bUseXinerama)
		cairo_dock_get_screen_offsets (myPosition.iNumScreen, &pDock->iScreenOffsetX, &pDock->iScreenOffsetY);
	else
		pDock->iScreenOffsetX = pDock->iScreenOffsetY = 0;
	
	switch (myPosition.iScreenBorder)
	{
		case CAIRO_DOCK_BOTTOM :
		default :
			pDock->bHorizontalDock = CAIRO_DOCK_HORIZONTAL;
			pDock->bDirectionUp = TRUE;
		break;
		case CAIRO_DOCK_TOP :
			pDock->bHorizontalDock = CAIRO_DOCK_HORIZONTAL;
			pDock->bDirectionUp = FALSE;
		break;
		case CAIRO_DOCK_RIGHT :
			pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
			pDock->bDirectionUp = TRUE;
		break;
		case CAIRO_DOCK_LEFT :
			pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
			pDock->bDirectionUp = FALSE;
		break;
	}
	
	cairo_t* pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	double fMaxScale = cairo_dock_get_max_scale (pDock);
	
	if (mySystem.bUseFakeTransparency && g_pDesktopBgSurface == NULL)
		cairo_dock_load_desktop_background_surface ();
	
	//cairo_dock_load_dialog_buttons (CAIRO_CONTAINER (pDock), myDialogs.cButtonOkImage, myDialogs.cButtonCancelImage);
	cairo_dock_unload_dialog_buttons ();
	
	cairo_dock_load_task_indicator (myTaskBar.bShowAppli && myTaskBar.bMixLauncherAppli ? myIndicators.cIndicatorImagePath : NULL, pCairoContext, fMaxScale, myIndicators.fIndicatorRatio);
	
	cairo_dock_load_icons_background_surface (myIcons.cBackgroundImagePath, pCairoContext, fMaxScale);

	cairo_dock_load_active_window_indicator (pCairoContext, myIndicators.cActiveIndicatorImagePath, fMaxScale, myIndicators.iActiveCornerRadius, myIndicators.iActiveLineWidth, myIndicators.fActiveColor);
	
	cairo_dock_load_class_indicator (myTaskBar.bShowAppli && myTaskBar.bGroupAppliByClass ? myIndicators.cClassIndicatorImagePath : NULL, pCairoContext, fMaxScale);
	
	cairo_dock_load_desklet_buttons (pCairoContext);
	
	cairo_dock_load_visible_zone (pDock, myBackground.cVisibleZoneImageFile, myAccessibility.iVisibleZoneWidth, myAccessibility.iVisibleZoneHeight, myBackground.fVisibleZoneAlpha);
	
	
	if (/**bUniquePidOld != myTaskBar.bUniquePid ||*/
		bGroupAppliByClassOld != myTaskBar.bGroupAppliByClass ||
		bHideVisibleApplisOld != myTaskBar.bHideVisibleApplis ||
		bAppliOnCurrentDesktopOnlyOld != myTaskBar.bAppliOnCurrentDesktopOnly ||
		bMixLauncherAppliOld != myTaskBar.bMixLauncherAppli ||
		bOverWriteXIconsOld != myTaskBar.bOverWriteXIcons ||
		bShowThumbnailOld != myTaskBar.bShowThumbnail ||
		(cairo_dock_application_manager_is_running () && ! myTaskBar.bShowAppli))  // on ne veut plus voir les applis, il faut donc les enlever.
	{
		cairo_dock_stop_application_manager ();
	}
	
	if (bGroupOrderChanged || myIcons.bMixAppletsAndLaunchers != bMixAppletsAndLaunchersOld)
		pDock->icons = g_list_sort (pDock->icons, (GCompareFunc) cairo_dock_compare_icons_order);

	if ((bUseSeparatorOld && ! myIcons.bUseSeparator) || (! bMixAppletsAndLaunchersOld && myIcons.bMixAppletsAndLaunchers) || bGroupOrderChanged)
		cairo_dock_remove_automatic_separators (pDock);
		
	g_fBackgroundImageWidth = 1e4;  // inutile de mettre a jour les decorations maintenant.
	g_fBackgroundImageHeight = 1e4;
	if (pDock->icons == NULL)
	{
		pDock->fFlatDockWidth = - myIcons.iIconGap;  // car on ne le connaissait pas encore au moment de la creation du dock.
		cairo_dock_build_docks_tree_with_desktop_files (pDock, g_cCurrentLaunchersPath);
	}
	else
	{
		cairo_dock_synchronize_sub_docks_position (pDock, FALSE);
		cairo_dock_reload_buffers_in_all_docks (FALSE);  // tout sauf les applets, qui seront rechargees en bloc juste apres.
	}


	if (! cairo_dock_application_manager_is_running () && myTaskBar.bShowAppli)  // maintenant on veut voir les applis !
	{
		cairo_dock_start_application_manager (pDock);  // va inserer le separateur si necessaire.
	}

	if ((myIcons.bUseSeparator && ! bUseSeparatorOld) || (bMixAppletsAndLaunchersOld != myIcons.bMixAppletsAndLaunchers) || bGroupOrderChanged)
	{
		cairo_dock_insert_separators_in_dock (pDock);
	}
	
	cairo_dock_create_icon_pbuffer ();
	
	GTimeVal time_val;
	g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
	double fTime = time_val.tv_sec + time_val.tv_usec * 1e-6;
	cairo_dock_activate_modules_from_list (mySystem.cActiveModuleList, fTime);
	cairo_dock_deactivate_old_modules (fTime);

	cairo_dock_set_all_views_to_default (0);  // met a jour la taille de tous les docks, maintenant qu'ils sont tous remplis.
	cairo_dock_redraw_root_docks (TRUE);  // TRUE <=> sauf le main dock.
	
	if (myAccessibility.cRaiseDockShortcut != NULL)
	{
		if (cRaiseDockShortcutOld == NULL || strcmp (myAccessibility.cRaiseDockShortcut, cRaiseDockShortcutOld) != 0)
		{
			if (cRaiseDockShortcutOld != NULL)
				cd_keybinder_unbind (cRaiseDockShortcutOld, (CDBindkeyHandler) cairo_dock_raise_from_keyboard);
			if (! cd_keybinder_bind (myAccessibility.cRaiseDockShortcut, (CDBindkeyHandler) cairo_dock_raise_from_keyboard, NULL))
			{
				g_free (myAccessibility.cRaiseDockShortcut);
				myAccessibility.cRaiseDockShortcut = NULL;
			}
		}
	}
	else
	{
		if (cRaiseDockShortcutOld != NULL)
		{
			cd_keybinder_unbind (cRaiseDockShortcutOld, (CDBindkeyHandler) cairo_dock_raise_from_keyboard);
			cairo_dock_place_root_dock (pDock);
			gtk_widget_show (pDock->pWidget);
		}
	}
	g_free (cRaiseDockShortcutOld);
	
	myAccessibility.bReserveSpace = myAccessibility.bReserveSpace && (myAccessibility.cRaiseDockShortcut == NULL);
	cairo_dock_reserve_space_for_all_root_docks (myAccessibility.bReserveSpace);

	cairo_dock_load_background_decorations (pDock);

	cairo_dock_place_root_dock (pDock);
	if (mySystem.bUseFakeTransparency && ! bUseFakeTransparencyOld)
		gtk_window_set_keep_below (GTK_WINDOW (pDock->pWidget), TRUE);  // le main dock ayant ete cree avant, il n'a pas herite de ce parametre.
	else if (! mySystem.bUseFakeTransparency && bUseFakeTransparencyOld)
		gtk_window_set_keep_below (GTK_WINDOW (pDock->pWidget), FALSE);
	
	pDock->iMouseX = 0;  // on se place hors du dock initialement.
	pDock->iMouseY = 0;
	cairo_dock_calculate_dock_icons (pDock);
	gtk_widget_queue_draw (pDock->pWidget);  // le 'gdk_window_move_resize' ci-dessous ne provoquera pas le redessin si la taille n'a pas change.
	
	
	if (cDeskletDecorationsNameOld == NULL && myDesklets.cDeskletDecorationsName != NULL)  // chargement initial, on charge juste ceux qui n'ont pas encore leur deco et qui ont atteint leur taille definitive.
	{
		cairo_dock_reload_desklets_decorations (FALSE, pCairoContext);
	}
	else if (cDeskletDecorationsNameOld != NULL && (myDesklets.cDeskletDecorationsName == NULL || strcmp (cDeskletDecorationsNameOld, myDesklets.cDeskletDecorationsName) != 0))  // le theme par defaut a change, on recharge les desklets qui utilisent le theme "default".
	{
		cairo_dock_reload_desklets_decorations (TRUE, pCairoContext);
	}
	else if (myDesklets.cDeskletDecorationsName != NULL && strcmp (myDesklets.cDeskletDecorationsName, "personnal") == 0)  // on a configure le theme personnel, il peut avoir change.
	{
		cairo_dock_reload_desklets_decorations (TRUE, pCairoContext);
	}
	else  // on charge juste ceux qui n'ont pas encore leur deco et qui ont atteint leur taille definitive.
	{
		cairo_dock_reload_desklets_decorations (FALSE, pCairoContext);
	}
	g_free (cDeskletDecorationsNameOld);
	
	cairo_dock_update_renderer_list_for_gui ();
	cairo_dock_update_desklet_decorations_list_for_gui ();
	cairo_dock_update_desklet_decorations_list_for_applet_gui ();
	cairo_dock_update_animations_list_for_gui ();
	cairo_dock_update_dialog_decorator_list_for_gui ();
	
	if (myAccessibility.bPopUp)
	{
		cairo_dock_start_polling_screen_edge (pDock);
		if (! bPopUpOld)
			gtk_window_set_keep_below (GTK_WINDOW (pDock->pWidget), TRUE);  // le main dock ayant ete cree avant, il n'a pas herite de ce parametre.
	}
	else
	{
		cairo_dock_stop_polling_screen_edge ();
		if (bPopUpOld)
			cairo_dock_set_root_docks_on_top_layer ();
	}
	
	
	//\___________________ On ecrit si necessaire.
	if (! bFlushConfFileNeeded)
		bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile, CAIRO_DOCK_VERSION);
	if (bFlushConfFileNeeded)
	{
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, CAIRO_DOCK_SHARE_DATA_DIR, CAIRO_DOCK_CONF_FILE);
	}
	
	cairo_destroy (pCairoContext);
	
	g_key_file_free (pKeyFile);

	cairo_dock_mark_theme_as_modified (TRUE);  // force a FALSE apres coup dans le cas d'un chargement de theme.
	
	s_bLoading = FALSE;
}

gboolean cairo_dock_is_loading (void)
{
	return s_bLoading;
}


void cairo_dock_update_conf_file (const gchar *cConfFilePath, GType iFirstDataType, ...)  // type, groupe, cle, valeur, etc. finir par G_TYPE_INVALID.
{
	cd_message ("%s (%s)", __func__, cConfFilePath);
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	g_return_if_fail (pKeyFile != NULL);
	
	va_list args;
	va_start (args, iFirstDataType);
	
	GType iType = iFirstDataType;
	gboolean bValue;
	gint iValue;
	double fValue;
	gchar *cValue;
	gchar *cGroupName, *cGroupKey;
	while (iType != G_TYPE_INVALID)
	{
		cGroupName = va_arg (args, gchar *);
		cGroupKey = va_arg (args, gchar *);

		switch (iType)
		{
			case G_TYPE_BOOLEAN :
				bValue = va_arg (args, gboolean);
				g_key_file_set_boolean (pKeyFile, cGroupName, cGroupKey, bValue);
			break ;
			case G_TYPE_INT :
				iValue = va_arg (args, gint);
				g_key_file_set_integer (pKeyFile, cGroupName, cGroupKey, iValue);
			break ;
			case G_TYPE_DOUBLE :
				fValue = va_arg (args, gdouble);
				g_key_file_set_double (pKeyFile, cGroupName, cGroupKey, fValue);
			break ;
			case G_TYPE_STRING :
				cValue = va_arg (args, gchar *);
				g_key_file_set_string (pKeyFile, cGroupName, cGroupKey, cValue);
			break ;
			default :
			break ;
		}

		iType = va_arg (args, GType);
	}

	cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
	g_key_file_free (pKeyFile);

	va_end (args);
}


void cairo_dock_update_conf_file_with_position (const gchar *cConfFilePath, int x, int y)
{
	//g_print ("%s (%s ; %d;%d)\n", __func__, cConfFilePath, x, y);
	cairo_dock_update_conf_file (cConfFilePath,
		G_TYPE_INT, "Position", "x gap", x,
		G_TYPE_INT, "Position", "y gap", y,
		G_TYPE_INVALID);
}


void cairo_dock_get_version_from_string (const gchar *cVersionString, int *iMajorVersion, int *iMinorVersion, int *iMicroVersion)
{
	gchar **cVersions = g_strsplit (cVersionString, ".", -1);
	if (cVersions[0] != NULL)
	{
		*iMajorVersion = atoi (cVersions[0]);
		if (cVersions[1] != NULL)
		{
			*iMinorVersion = atoi (cVersions[1]);
			if (cVersions[2] != NULL)
				*iMicroVersion = atoi (cVersions[2]);
		}
	}
	g_strfreev (cVersions);
}


void cairo_dock_decrypt_string( const guchar *cEncryptedString,  guchar **cDecryptedString )
{
	if( !cEncryptedString || *cEncryptedString == '\0' )
	{
		*cDecryptedString = g_strdup( "" );
		return;
	}
#ifdef HAVE_LIBCRYPT
	guchar *input = g_strdup(cEncryptedString);
	guchar *shifted_input = input;
	guchar **output = cDecryptedString; 

  guchar *current_output = NULL;
  if( output && input && strlen(input)>0 )
  {
    *output = g_malloc( (strlen(input)+1)/3+1 );
    current_output = *output;
  }
  else
  {
  	if( input )
  	{
  		g_free(input);
		}
  	return;
	}

  guchar *last_char_in_input = input + strlen(input);
//  g_print( "Password (before decrypt): %s\n", input );

  for( ; shifted_input < last_char_in_input; shifted_input += 16+8, current_output += 8 )
  {
    guint block[8];
    guchar txt[64];
    gint i = 0, j = 0;
    guchar current_letter = 0;
    
    memset( txt, 0, 64 );

    shifted_input[16+8-1] = 0; // cut the string

    sscanf( shifted_input, "%X-%X-%X-%X-%X-%X-%X-%X",
    &block[0], &block[1], &block[2], &block[3], &block[4], &block[5], &block[6], &block[7] );

    // process the eight first characters of "input"
    for( i = 0; i < 8 ; i++ )
      for ( j = 0; j < 8; j++ )
        txt[i*8+j] = block[i] >> j & 1;
    
    setkey( DES_crypt_key );
    encrypt( txt, 1 );  // decrypt

    for ( i = 0; i < 8; i++ )
    {
      current_output[i] = 0;
      for ( j = 0; j < 8; j++ )
      {
        current_output[i] |= txt[i*8+j] << j;
      }
    }
  }

  *current_output = 0;

//  g_print( "Password (after decrypt): %s\n", *output );

	g_free( input );

#else
	*cDecryptedString = g_strdup( cEncryptedString );
#endif
}

void cairo_dock_encrypt_string( const guchar *cDecryptedString,  guchar **cEncryptedString )
{
	if( !cDecryptedString || strlen(cDecryptedString) == 0 )
	{
		*cEncryptedString = g_strdup( "" );
		return;
	}
	
#ifdef HAVE_LIBCRYPT
	const guchar *input = cDecryptedString;
	guchar **output = cEncryptedString;
	guint input_length = 0;

  guchar *current_output = NULL;
  if( output && input && strlen(input)>0 )
  {
  	// for each block of 8 characters, we need 24 bytes.
  	guint nbBlocks = strlen(input)/8+1;
  	
    *output = g_malloc( nbBlocks*24+1 );
    current_output = *output;
  }
  else
  {
  	return;
	}

  const guchar *last_char_in_input = input + strlen(input);

//  g_print( "Password (before encrypt): %s\n", input );

  for( ; input < last_char_in_input; input += 8, current_output += 16+8 )
  {
    guchar txt[64];
    guint i = 0, j = 0;
    guchar current_letter = 0;
    
    memset( txt, 0, 64 );
    
    // process the eight first characters of "input"
    for( i = 0; i < strlen(input) && i < 8 ; i++ )
      for ( j = 0; j < 8; j++ )
        txt[i*8+j] = input[i] >> j & 1;
    
    setkey( DES_crypt_key );
    encrypt( txt, 0 );  // encrypt

    for ( i = 0; i < 8; i++ )
    {
      current_letter = 0;
      for ( j = 0; j < 8; j++ )
      {
        current_letter |= txt[i*8+j] << j;
      }
      snprintf( current_output + i*3, 4, "%02X-", current_letter );
    }
  }

  *(current_output-1) = 0;

//  g_print( "Password (after encrypt): %s\n", *output );
#else
	*cEncryptedString = g_strdup( cDecryptedString );
#endif
}
