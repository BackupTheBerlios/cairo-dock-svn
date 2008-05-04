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

#include "cairo-dock-draw.h"
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
#include "cairo-dock-config.h"

#define CAIRO_DOCK_TYPE_CONF_FILE_FILE ".cairo-dock-conf-file"

static const gchar * s_cIconTypeNames[(CAIRO_DOCK_NB_TYPES+1)/2] = {"launchers", "applications", "applets"};

extern CairoDock *g_pMainDock;
extern gboolean g_bReverseVisibleImage;
extern gboolean g_bReserveSpace;
extern gchar *g_cMainDockDefaultRendererName;
extern gchar *g_cSubDockDefaultRendererName;
extern gchar *g_cCurrentThemePath;
extern gchar *g_cEasyConfFile;
extern gchar *g_cCairoDockDataDir;

extern int g_iMaxAuthorizedWidth;
extern int g_iScrollAmount;
extern gboolean g_bResetScrollOnLeave;
extern double g_fScrollAcceleration;

extern int g_iSinusoidWidth;
extern double g_fAmplitude;
extern int g_iIconGap;
extern double g_fReflectSize;
extern double g_fAlbedo;
extern gboolean g_bDynamicReflection;
extern double g_fAlphaAtRest;

extern double g_fVisibleZoneAlpha;
extern gboolean g_bSameHorizontality;
extern double g_fSubDockSizeRatio;
extern gboolean g_bAnimateSubDock;
extern int g_iLeaveSubDockDelay;
extern int g_iShowSubDockDelay;
extern gboolean bShowSubDockOnClick;

extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern int g_iLabelWeight;
extern int g_iLabelStyle;
extern gboolean g_bLabelForPointedIconOnly;
extern double g_fLabelAlphaThreshold;
extern gboolean g_bTextAlwaysHorizontal;
extern double g_fLabelBackgroundColor[4];
extern gboolean g_bUseBackgroundForLabel;

extern gpointer *g_pDefaultIconDirectory;
static gboolean s_bUserTheme = FALSE;
extern gchar *g_cCurrentLaunchersPath;
extern gchar *g_cConfFile;

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_fBackgroundImageWidth;
extern int g_fBackgroundImageHeight;

extern int g_iDockRadius;
extern gint g_iFrameMargin;
extern int g_iDockLineWidth;
extern gboolean g_bRoundedBottomCorner;
extern double g_fLineColor[4];
extern gint g_iStringLineWidth;
extern double g_fStringColor[4];

extern gboolean g_bBackgroundImageRepeat;
extern double g_fBackgroundImageAlpha;
extern gchar *g_cBackgroundImageFile;
extern gboolean g_bDecorationsFollowMouse;

extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];
extern cairo_surface_t *g_pStripesBuffer;
extern int g_iNbStripes;
extern double g_fStripesSpeedFactor;
extern double g_fStripesWidth;
extern double g_fStripesAngle;

extern int g_iScreenWidth[2];
extern int g_iScreenHeight[2];

extern gboolean g_bAnimateOnAutoHide;
extern double g_fUnfoldAcceleration;
extern int g_iGrowUpInterval;
extern int g_iShrinkDownInterval;
extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;
extern double g_fRefreshInterval;

extern gboolean g_bShowAppli;
extern gboolean g_bUniquePid;
extern gboolean g_bGroupAppliByClass;
extern int g_iAppliMaxNameLength;
extern gboolean g_bMinimizeOnClick;
extern gboolean g_bCloseAppliOnMiddleClick;
extern gboolean g_bAutoHideOnFullScreen;
extern gboolean g_bDemandsAttentionWithDialog;
extern gboolean g_bDemandsAttentionWithAnimation;
extern gboolean g_bAnimateOnActiveWindow;
extern double g_fVisibleAppliAlpha;
extern gboolean g_bHideVisibleApplis;
extern gboolean g_bAppliOnCurrentDesktopOnly;

extern int g_tIconAuthorizedWidth[CAIRO_DOCK_NB_TYPES];
extern int g_tIconAuthorizedHeight[CAIRO_DOCK_NB_TYPES];
extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUseSeparator;
extern gchar *g_cSeparatorImage;
extern gboolean g_bRevolveSeparator;
extern gboolean g_bConstantSeparatorSize;

extern int g_iDialogButtonWidth;
extern int g_iDialogButtonHeight;
extern double g_fDialogColor[4];
extern int g_iDialogIconSize;
extern int g_iDialogMessageSize;
extern gchar *g_cDialogMessagePolice;
extern int g_iDialogMessageWeight;
extern int g_iDialogMessageStyle;
extern double g_fDialogTextColor[4];

extern double g_fDeskletColor[4];
extern double g_fDeskletColorInside[4];

extern CairoDockFMSortType g_iFileSortType;
extern gboolean g_bShowHiddenFiles;
extern gchar *g_cRaiseDockShortcut;
extern cairo_surface_t *g_pIndicatorSurface[2];
extern gboolean g_bMixLauncherAppli;
extern gboolean g_bOverWriteXIcons;
extern double g_fIndicatorWidth, g_fIndicatorHeight;
extern int g_iIndicatorDeltaY;
extern gboolean g_bLinkIndicatorWithIcon;

static gchar **g_cUseXIconAppliList = NULL;
static gboolean s_bLoading = FALSE;

/**
*Recupere une cle booleene d'un fichier de cles.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param bDefaultValue valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
@Returns la valeur booleene de la cle.
*/
gboolean cairo_dock_get_boolean_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gboolean bDefaultValue, gchar *cDefaultGroupName, gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	gboolean bValue = g_key_file_get_boolean (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning ("Attention : %s", erreur->message);
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
/**
*Recupere une cle entiere d'un fichier de cles.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param iDefaultValue valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
@Returns la valeur entiere de la cle.
*/
int cairo_dock_get_integer_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int iDefaultValue, gchar *cDefaultGroupName, gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	int iValue = g_key_file_get_integer (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning ("Attention : %s", erreur->message);
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
/**
*Recupere une cle flottante d'un fichier de cles.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param fDefaultValue valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
@Returns la valeur flottante de la cle.
*/
double cairo_dock_get_double_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double fDefaultValue, gchar *cDefaultGroupName, gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	double fValue = g_key_file_get_double (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning ("Attention : %s", erreur->message);
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
/**
*Recupere une cle d'un fichier de cles sous la forme d'une chaine de caractere.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param cDefaultValue valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
@Returns la chaine de caractere nouvellement allouee correspondante a la cle.
*/
gchar *cairo_dock_get_string_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, const gchar *cDefaultValue, gchar *cDefaultGroupName, gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	gchar *cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning ("Attention : %s", erreur->message);
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
/**
*Recupere une cle d'un fichier de cles sous la forme d'un tableau d'entiers.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param iValueBuffer tableau qui sera rempli.
*@param iNbElements nombre d'elements a recuperer; c'est le nombre d'elements du tableau passe en entree.
*@param iDefaultValues valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*/
void cairo_dock_get_integer_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int *iValueBuffer, int iNbElements, int *iDefaultValues, gchar *cDefaultGroupName, gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	gsize length = 0;
	if (iDefaultValues != NULL)
		memcpy (iValueBuffer, iDefaultValues, iNbElements * sizeof (int));

	int *iValuesList = g_key_file_get_integer_list (pKeyFile, cGroupName, cKeyName, &length, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning ("Attention : %s", erreur->message);
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
/**
*Recupere une cle d'un fichier de cles sous la forme d'un tableau de doubles.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param fValueBuffer tableau qui sera rempli.
*@param iNbElements nombre d'elements a recuperer; c'est le nombre d'elements du tableau passe en entree.
*@param fDefaultValues valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*/
void cairo_dock_get_double_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double *fValueBuffer, int iNbElements, double *fDefaultValues, gchar *cDefaultGroupName, gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	gsize length = 0;
	if (fDefaultValues != NULL)
		memcpy (fValueBuffer, fDefaultValues, iNbElements * sizeof (double));

	double *fValuesList = g_key_file_get_double_list (pKeyFile, cGroupName, cKeyName, &length, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning ("Attention : %s", erreur->message);
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
/**
*Recupere une cle d'un fichier de cles sous la forme d'un tableau de chaines de caracteres.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param length nombre de chaines de caracteres recuperees.
*@param cDefaultValues valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
@Returns un tableau de chaines de caracteres; a liberer avec g_strfreev().
*/
gchar **cairo_dock_get_string_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gsize *length, gchar *cDefaultValues, gchar *cDefaultGroupName, gchar *cDefaultKeyName)
{
	GError *erreur = NULL;
	*length = 0;
	gchar **cValuesList = g_key_file_get_string_list (pKeyFile, cGroupName, cKeyName, length, &erreur);
	if (erreur != NULL)
	{
		if (bFlushConfFileNeeded != NULL)
			cd_warning ("Attention : %s", erreur->message);
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
/**
*Recupere une cle d'un fichier de cles sous la forme d'un tableau de chaines de caracteres.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param iDefaultAnimation valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@Returns le type de l'animation correspondante a la cle.
*/
CairoDockAnimationType cairo_dock_get_animation_type_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, CairoDockAnimationType iDefaultAnimation, gchar *cDefaultGroupName, gchar *cDefaultKeyName)
{
	CairoDockAnimationType iAnimationType = cairo_dock_get_integer_key_value (pKeyFile, cGroupName, cKeyName, bFlushConfFileNeeded, iDefaultAnimation, cDefaultGroupName, cDefaultKeyName);
	if (iAnimationType < 0 || iAnimationType >= CAIRO_DOCK_NB_ANIMATIONS)
		iAnimationType = 0;
	return iAnimationType;
}


void cairo_dock_read_conf_file (gchar *cConfFilePath, CairoDock *pDock)
{
	//g_print ("%s (%s)\n", __func__, cConfFilePath);
	GError *erreur = NULL;
	gsize length;
	gboolean bFlushConfFileNeeded = FALSE, bFlushConfFileNeeded2 = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.

	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		return ;
	}
	
	s_bLoading = TRUE;
	
	//\___________________ On recupere la position du dock.
	pDock->iGapX = cairo_dock_get_integer_key_value (pKeyFile, "Position", "x gap", &bFlushConfFileNeeded, 0, NULL, NULL);
	pDock->iGapY = cairo_dock_get_integer_key_value (pKeyFile, "Position", "y gap", &bFlushConfFileNeeded, 0, NULL, NULL);

	CairoDockPositionType iScreenBorder = cairo_dock_get_integer_key_value (pKeyFile, "Position", "screen border", &bFlushConfFileNeeded, 0, NULL, NULL);
	if (iScreenBorder < 0 || iScreenBorder >= CAIRO_DOCK_NB_POSITIONS)
		iScreenBorder = 0;

	pDock->fAlign = cairo_dock_get_double_key_value (pKeyFile, "Position", "alignment", &bFlushConfFileNeeded, 0.5, NULL, NULL);

	g_bReserveSpace = cairo_dock_get_boolean_key_value (pKeyFile, "Position", "reserve space", &bFlushConfFileNeeded, FALSE, NULL, NULL);

	cairo_dock_deactivate_temporary_auto_hide ();
	pDock->bAutoHide = cairo_dock_get_boolean_key_value (pKeyFile, "Position", "auto-hide", &bFlushConfFileNeeded, FALSE, "Auto-Hide", "auto-hide");

	//\___________________ On recupere les parametres de la zone visible.
	gchar *cVisibleZoneImageFile = cairo_dock_get_string_key_value (pKeyFile, "Background", "callback image", &bFlushConfFileNeeded, NULL, "Auto-Hide", "background image");

	g_iVisibleZoneWidth = cairo_dock_get_integer_key_value (pKeyFile, "Background", "zone width", &bFlushConfFileNeeded, 150, "Auto-Hide", NULL);
	if (g_iVisibleZoneWidth < 10) g_iVisibleZoneWidth = 10;
	g_iVisibleZoneHeight = cairo_dock_get_integer_key_value (pKeyFile, "Background", "zone height", &bFlushConfFileNeeded, 25, "Auto-Hide", NULL);
	if (g_iVisibleZoneHeight < 1) g_iVisibleZoneHeight = 1;

	g_fVisibleZoneAlpha = cairo_dock_get_double_key_value (pKeyFile, "Background", "alpha", &bFlushConfFileNeeded, 0.5, "Auto-Hide", NULL);

	g_bReverseVisibleImage = cairo_dock_get_boolean_key_value (pKeyFile, "Background", "reverse visible image", &bFlushConfFileNeeded, TRUE, "Auto-Hide", NULL);


	//\___________________ On recupere les parametres des etiquettes.
	g_bLabelForPointedIconOnly = cairo_dock_get_boolean_key_value (pKeyFile, "System", "pointed icon only", &bFlushConfFileNeeded, FALSE, "Labels", NULL);

	g_fLabelAlphaThreshold = cairo_dock_get_double_key_value (pKeyFile, "System", "alpha threshold", &bFlushConfFileNeeded, 10., "Labels", NULL);

	g_bTextAlwaysHorizontal = cairo_dock_get_boolean_key_value (pKeyFile, "System", "always horizontal", &bFlushConfFileNeeded, FALSE, "Labels", NULL);


	g_free (g_cLabelPolice);
	g_cLabelPolice = cairo_dock_get_string_key_value (pKeyFile, "Icons", "police", &bFlushConfFileNeeded, "sans", "Labels", NULL);

	g_iLabelSize = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "size", &bFlushConfFileNeeded, 14, "Labels", NULL);

	g_iLabelWeight = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "weight", &bFlushConfFileNeeded, 5, "Labels", NULL);
	g_iLabelWeight = ((PANGO_WEIGHT_HEAVY - PANGO_WEIGHT_ULTRALIGHT) * g_iLabelWeight + 9 * PANGO_WEIGHT_ULTRALIGHT - PANGO_WEIGHT_HEAVY) / 8;  // on se ramene aux intervalles definit par Pango.

	gboolean bLabelStyleItalic = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "italic", &bFlushConfFileNeeded, FALSE, "Labels", NULL);
	if (bLabelStyleItalic)
		g_iLabelStyle = PANGO_STYLE_ITALIC;
	else
		g_iLabelStyle = PANGO_STYLE_NORMAL;


	if (g_cLabelPolice == NULL)
		g_iLabelSize = 0;

	if (g_iLabelSize == 0)
	{
		g_free (g_cLabelPolice);
		g_cLabelPolice = NULL;
	}

	double couleur_label[4] = {0., 0., 0., 0.5};
	cairo_dock_get_double_list_key_value (pKeyFile, "Icons", "text background color", &bFlushConfFileNeeded, g_fLabelBackgroundColor, 4, couleur_label, NULL, NULL);

	g_bUseBackgroundForLabel = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "background for label", &bFlushConfFileNeeded, FALSE, NULL, NULL);


	//\___________________ On recupere les parametres du dock en lui-meme.
	gchar **cIconsTypesList = cairo_dock_get_string_list_key_value (pKeyFile, "Icons", "icon's type order", &bFlushConfFileNeeded, &length, NULL, "Cairo Dock", NULL);
	if (cIconsTypesList != NULL && length > 0)
	{
		unsigned int i, j;
		for (i = 0; i < length; i ++)
		{
			for (j = 0; j < ((CAIRO_DOCK_NB_TYPES + 1) / 2); j ++)
			{
				if (strcmp (cIconsTypesList[i], s_cIconTypeNames[j]) == 0)
				{
					g_tIconTypeOrder[2*j] = 2 * i;
				}
			}
		}
	}
	g_strfreev (cIconsTypesList);

	g_free (g_cMainDockDefaultRendererName);
	g_cMainDockDefaultRendererName = cairo_dock_get_string_key_value (pKeyFile, "Views", "main dock view", &bFlushConfFileNeeded, CAIRO_DOCK_DEFAULT_RENDERER_NAME, "Cairo Dock", NULL);
	if (g_cMainDockDefaultRendererName == NULL)
		g_cMainDockDefaultRendererName = g_strdup (CAIRO_DOCK_DEFAULT_RENDERER_NAME);
	cd_message ("g_cMainDockDefaultRendererName <- %s", g_cMainDockDefaultRendererName);

	double fUserValue = cairo_dock_get_double_key_value (pKeyFile, "System", "unfold factor", &bFlushConfFileNeeded, 8., "Cairo Dock", NULL);
	g_fUnfoldAcceleration = 1 - pow (2, - fUserValue);
	
	g_bAnimateOnAutoHide = cairo_dock_get_boolean_key_value (pKeyFile, "System", "animate on auto-hide", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	
	int iNbSteps = cairo_dock_get_integer_key_value (pKeyFile, "System", "grow up steps", &bFlushConfFileNeeded, 8, "Cairo Dock", NULL);
	iNbSteps = MAX (iNbSteps, 1);
	g_iGrowUpInterval = MAX (1, CAIRO_DOCK_NB_MAX_ITERATIONS / iNbSteps);
	iNbSteps = cairo_dock_get_integer_key_value (pKeyFile, "System", "shrink down steps", &bFlushConfFileNeeded, 10, "Cairo Dock", NULL);
	iNbSteps = MAX (iNbSteps, 1);
	g_iShrinkDownInterval = MAX (1, CAIRO_DOCK_NB_MAX_ITERATIONS / iNbSteps);
	g_fMoveUpSpeed = cairo_dock_get_double_key_value (pKeyFile, "System", "move up speed", &bFlushConfFileNeeded, 0.35, "Cairo Dock", NULL);
	g_fMoveUpSpeed = MAX (0.01, MIN (g_fMoveUpSpeed, 1));
	g_fMoveDownSpeed = cairo_dock_get_double_key_value (pKeyFile, "System", "move down speed", &bFlushConfFileNeeded, 0.35, "Cairo Dock", NULL);
	g_fMoveDownSpeed = MAX (0.01, MIN (g_fMoveDownSpeed, 1));

	int iRefreshFrequency = cairo_dock_get_integer_key_value (pKeyFile, "System", "refresh frequency", &bFlushConfFileNeeded, 25, "Cairo Dock", NULL);
	g_fRefreshInterval = 1000. / iRefreshFrequency;


	//\___________________ On recupere les parametres propres aux sous-docks.
	g_free (g_cSubDockDefaultRendererName);
	g_cSubDockDefaultRendererName = cairo_dock_get_string_key_value (pKeyFile, "Views", "sub-dock view", &bFlushConfFileNeeded, CAIRO_DOCK_DEFAULT_RENDERER_NAME, "Sub-Docks", NULL);
	if (g_cSubDockDefaultRendererName == NULL)
		g_cSubDockDefaultRendererName = g_strdup (CAIRO_DOCK_DEFAULT_RENDERER_NAME);

	g_bSameHorizontality = cairo_dock_get_boolean_key_value (pKeyFile, "Views", "same horizontality", &bFlushConfFileNeeded, TRUE, "Sub-Docks", NULL);

	g_fSubDockSizeRatio = cairo_dock_get_double_key_value (pKeyFile, "Views", "relative icon size", &bFlushConfFileNeeded, 0.8, "Sub-Docks", NULL);

	g_bAnimateSubDock = cairo_dock_get_boolean_key_value (pKeyFile, "System", "animate subdocks", &bFlushConfFileNeeded, TRUE, "Sub-Docks", NULL);

	g_iLeaveSubDockDelay = cairo_dock_get_integer_key_value (pKeyFile, "System", "leaving delay", &bFlushConfFileNeeded, 330, "Sub-Docks", NULL);
	g_iShowSubDockDelay = cairo_dock_get_integer_key_value (pKeyFile, "System", "show delay", &bFlushConfFileNeeded, 300, "Sub-Docks", NULL);

	bShowSubDockOnClick = cairo_dock_get_boolean_key_value (pKeyFile, "System", "show on click", &bFlushConfFileNeeded, FALSE, "Sub-Docks", NULL);


	//\___________________ On recupere les parametres du fond.
	g_iDockRadius = cairo_dock_get_integer_key_value (pKeyFile, "Background", "corner radius", &bFlushConfFileNeeded, 12, NULL, NULL);

	g_iDockLineWidth = cairo_dock_get_integer_key_value (pKeyFile, "Background", "line width", &bFlushConfFileNeeded, 2, NULL, NULL);

	g_iFrameMargin = cairo_dock_get_integer_key_value (pKeyFile, "Background", "frame margin", &bFlushConfFileNeeded, 2, NULL, NULL);

	double couleur[4] = {0., 0., 0.6, 0.4};
	cairo_dock_get_double_list_key_value (pKeyFile, "Background", "line color", &bFlushConfFileNeeded, g_fLineColor, 4, couleur, NULL, NULL);

	g_bRoundedBottomCorner = cairo_dock_get_boolean_key_value (pKeyFile, "Background", "rounded bottom corner", &bFlushConfFileNeeded, TRUE, NULL, NULL);

	g_iMaxAuthorizedWidth = cairo_dock_get_integer_key_value (pKeyFile, "Position", "max autorized width", &bFlushConfFileNeeded, 0, "Background", NULL);

	double couleur2[4] = {.7, .9, .7, .4};
	cairo_dock_get_double_list_key_value (pKeyFile, "Background", "stripes color bright", &bFlushConfFileNeeded, g_fStripesColorBright, 4, couleur2, NULL, NULL);

	g_free (g_cBackgroundImageFile);
	g_cBackgroundImageFile = cairo_dock_get_string_key_value (pKeyFile, "Background", "background image", &bFlushConfFileNeeded, NULL, NULL, NULL);

	g_fBackgroundImageAlpha = cairo_dock_get_double_key_value (pKeyFile, "Background", "image alpha", &bFlushConfFileNeeded, 0.5, NULL, NULL);

	g_bBackgroundImageRepeat = cairo_dock_get_boolean_key_value (pKeyFile, "Background", "repeat image", &bFlushConfFileNeeded, FALSE, NULL, NULL);

	g_iNbStripes = cairo_dock_get_integer_key_value (pKeyFile, "Background", "number of stripes", &bFlushConfFileNeeded, 10, NULL, NULL);

	g_fStripesWidth = cairo_dock_get_double_key_value (pKeyFile, "Background", "stripes width", &bFlushConfFileNeeded, 0.02, NULL, NULL);
	if (g_iNbStripes > 0 && g_fStripesWidth > 1. / g_iNbStripes)
	{
		cd_warning ("Attention : the stripes' width is greater than the space between them.");
		g_fStripesWidth = 0.99 / g_iNbStripes;
	}

	double couleur3[4] = {.7, .7, 1., .7};
	cairo_dock_get_double_list_key_value (pKeyFile, "Background", "stripes color dark", &bFlushConfFileNeeded, g_fStripesColorDark, 4, couleur3, NULL, NULL);

	g_fStripesAngle = cairo_dock_get_double_key_value (pKeyFile, "Background", "stripes angle", &bFlushConfFileNeeded, 30., NULL, NULL);

	g_fStripesSpeedFactor = cairo_dock_get_double_key_value (pKeyFile, "System", "scroll speed factor", &bFlushConfFileNeeded, 1.0, "Background", NULL);
	g_fStripesSpeedFactor = MIN (1., g_fStripesSpeedFactor);

	g_bDecorationsFollowMouse = cairo_dock_get_boolean_key_value (pKeyFile, "System", "decorations enslaved", &bFlushConfFileNeeded, TRUE, "Background", NULL);


	//\___________________ On recupere les parametres des icones.
	double fFieldDepth = cairo_dock_get_double_key_value (pKeyFile, "Icons", "field depth", &bFlushConfFileNeeded, 0.7, NULL, NULL);

	g_bDynamicReflection = cairo_dock_get_boolean_key_value (pKeyFile, "System", "dynamic reflection", &bFlushConfFileNeeded, FALSE, NULL, NULL);

	g_fAlbedo = cairo_dock_get_double_key_value (pKeyFile, "Icons", "albedo", &bFlushConfFileNeeded, .6, NULL, NULL);

	g_fAmplitude = cairo_dock_get_double_key_value (pKeyFile, "Icons", "amplitude", &bFlushConfFileNeeded, 1.0, NULL, NULL);

	g_iSinusoidWidth = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "sinusoid width", &bFlushConfFileNeeded, 250, NULL, NULL);
	g_iSinusoidWidth = MAX (1, g_iSinusoidWidth);

	g_iIconGap = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "icon gap", &bFlushConfFileNeeded, 0, NULL, NULL);

	g_iStringLineWidth = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "string width", &bFlushConfFileNeeded, 0, NULL, NULL);

	cairo_dock_get_double_list_key_value (pKeyFile, "Icons", "string color", &bFlushConfFileNeeded, g_fStringColor, 4, couleur, NULL, NULL);

	g_fAlphaAtRest = cairo_dock_get_double_key_value (pKeyFile, "Icons", "alpha at rest", &bFlushConfFileNeeded, 1., NULL, NULL);

	g_iScrollAmount = cairo_dock_get_integer_key_value (pKeyFile, "System", "scroll amount", &bFlushConfFileNeeded, FALSE, "Icons", NULL);

	g_bResetScrollOnLeave = cairo_dock_get_boolean_key_value (pKeyFile, "System", "reset scroll", &bFlushConfFileNeeded, TRUE, "Icons", NULL);

	g_fScrollAcceleration = cairo_dock_get_double_key_value (pKeyFile, "System", "reset scroll acceleration", &bFlushConfFileNeeded, 0.9, "Icons", NULL);


	//\___________________ On recupere les parametres des lanceurs.
	if (g_pDefaultIconDirectory != NULL)  // g_strfreev (g_cDefaultIconDirectory);
	{
		gpointer data;
		int i;
		for (i = 0; (g_pDefaultIconDirectory[2*i] != NULL || g_pDefaultIconDirectory[2*i+1] != NULL); i ++)
		{
			if (g_pDefaultIconDirectory[2*i] != NULL)
				g_free (g_pDefaultIconDirectory[2*i]);
			else
				g_object_unref (g_pDefaultIconDirectory[2*i+1]);
		}
	}
	gchar **directoryList = cairo_dock_get_string_list_key_value (pKeyFile, "Icons", "default icon directory", &bFlushConfFileNeeded, &length, NULL, "Launchers", NULL);


	if (directoryList == NULL)
	{
		g_pDefaultIconDirectory = NULL;
	}
	else
	{
		g_pDefaultIconDirectory = g_new0 (gpointer, 2 * length + 2);  // +2 pour les NULL final.
		int i = 0, j = 0;
		while (directoryList[i] != NULL)
		{
			if (directoryList[i][0] == '~')
			{
				g_pDefaultIconDirectory[j] = g_strdup_printf ("%s%s", getenv ("HOME"), directoryList[i]+1);
			}
			else if (directoryList[i][0] == '/')
			{
				g_pDefaultIconDirectory[j] = g_strdup (directoryList[i]);
			}
			else if (strncmp (directoryList[i], "_ThemeDirectory_", 16) == 0)
			{
				g_pDefaultIconDirectory[j] = g_strdup_printf ("%s%s", g_cCurrentLaunchersPath, directoryList[i]+16);
			}
			else
			{
				cd_message ("theme %s\n", directoryList[i]);
				g_pDefaultIconDirectory[j+1] = gtk_icon_theme_new ();
				gtk_icon_theme_set_custom_theme (g_pDefaultIconDirectory[j+1], directoryList[i]);
			}
			//g_print ("+ %s\n", g_cDefaultIconDirectory[j]);
			i ++;
			j += 2;
		}
	}
	g_strfreev (directoryList);

	g_tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "launcher width", &bFlushConfFileNeeded, 48, "Launchers", "max icon size");

	g_tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "launcher height", &bFlushConfFileNeeded, 48, "Launchers", "max icon size");

	g_tAnimationType[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_animation_type_key_value (pKeyFile, "Icons", "launcher animation", &bFlushConfFileNeeded, CAIRO_DOCK_BOUNCE, "Launchers", "animation type");

	g_tNbAnimationRounds[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "launcher number of rounds", &bFlushConfFileNeeded, 3, "Launchers", "number of animation rounds");


	//\___________________ On recupere les parametres des applications.
	g_bShowAppli = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "show applications", &bFlushConfFileNeeded, TRUE, "Applications", NULL);

	g_tIconAuthorizedWidth[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "appli width", &bFlushConfFileNeeded, 48, "Applications", "max icon size");

	g_tIconAuthorizedHeight[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "appli height", &bFlushConfFileNeeded, 48, "Applications", "max icon size");

	g_tAnimationType[CAIRO_DOCK_APPLI] = cairo_dock_get_animation_type_key_value (pKeyFile, "Icons", "appli animation", &bFlushConfFileNeeded, CAIRO_DOCK_ROTATE, "Applications", "animation type");

	g_tNbAnimationRounds[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "appli number of rounds", &bFlushConfFileNeeded, 2, "Applications", "number of animation rounds");

	gboolean bUniquePidOld = g_bUniquePid;
	g_bUniquePid = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "unique PID", &bFlushConfFileNeeded, FALSE, "Applications", NULL);

	gboolean bGroupAppliByClassOld = g_bGroupAppliByClass;
	g_bGroupAppliByClass = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "group by class", &bFlushConfFileNeeded, FALSE, "Applications", NULL);

	g_iAppliMaxNameLength = cairo_dock_get_integer_key_value (pKeyFile, "TaskBar", "max name length", &bFlushConfFileNeeded, 15, "Applications", NULL);

	g_bMinimizeOnClick = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "minimize on click", &bFlushConfFileNeeded, TRUE, "Applications", NULL);

	g_bCloseAppliOnMiddleClick = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "close on middle click", &bFlushConfFileNeeded, TRUE, "Applications", NULL);

	g_bAutoHideOnFullScreen = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "auto quick hide", &bFlushConfFileNeeded, FALSE, "Applications", NULL);

	g_bDemandsAttentionWithDialog = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "demands attention with dialog", &bFlushConfFileNeeded, TRUE, "Applications", NULL);
	g_bDemandsAttentionWithAnimation = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "demands attention with animation", &bFlushConfFileNeeded, FALSE, "Applications", NULL);

	g_bAnimateOnActiveWindow = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "animate on active window", &bFlushConfFileNeeded, TRUE, "Applications", NULL);

	gboolean bHideVisibleApplisOld = g_bHideVisibleApplis;
	g_bHideVisibleApplis = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "hide visible", &bFlushConfFileNeeded, FALSE, "Applications", NULL);

	g_fVisibleAppliAlpha = cairo_dock_get_double_key_value (pKeyFile, "TaskBar", "visibility alpha", &bFlushConfFileNeeded, .7, "Applications", NULL);  // >0 <=> les fenetres minimisees sont transparentes.
	if (g_bHideVisibleApplis && g_fVisibleAppliAlpha < 0)
		g_fVisibleAppliAlpha = 0.;  // on inhibe ce parametre, puisqu'il ne sert alors a rien.

	gboolean bAppliOnCurrentDesktopOnlyOld = g_bAppliOnCurrentDesktopOnly;
	g_bAppliOnCurrentDesktopOnly = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "current desktop only", &bFlushConfFileNeeded, FALSE, "Applications", NULL);

	gchar *cIndicatorImageName = cairo_dock_get_string_key_value (pKeyFile, "Icons", "indicator image", &bFlushConfFileNeeded, NULL, NULL, NULL);
	gchar *cIndicatorImagePath;
	if (cIndicatorImageName != NULL)
	{
		cIndicatorImagePath = cairo_dock_generate_file_path (cIndicatorImageName);
		g_free (cIndicatorImageName);
	}
	else
	{
		cIndicatorImagePath = g_strdup_printf ("%s/%s", CAIRO_DOCK_SHARE_DATA_DIR, CAIRO_DOCK_DEFAULT_INDICATOR_NAME);
	}
	
	double fIndicatorRatio = cairo_dock_get_double_key_value (pKeyFile, "Icons", "indicator ratio", &bFlushConfFileNeeded, 1., NULL, NULL);
	
	g_bLinkIndicatorWithIcon = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "link indicator", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	
	g_iIndicatorDeltaY = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "indicator deltaY", &bFlushConfFileNeeded, 2, NULL, NULL);
	
	gboolean bMixLauncherAppliOld = g_bMixLauncherAppli;
	g_bMixLauncherAppli = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "mix launcher appli", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	
	length = 0;
	guint i, j;
	/**gchar **cUseXIconAppliListNew = cairo_dock_get_string_list_key_value (pKeyFile, "TaskBar", "use xicon", &bFlushConfFileNeeded, &length, NULL, NULL, NULL);
	if (cUseXIconAppliListNew != NULL)
	{
		for (i = 0; cUseXIconAppliListNew[i] != NULL; i ++)
		{
			cairo_dock_set_class_use_xicon (cUseXIconAppliListNew[i], TRUE);
		}
	}
	if (g_cUseXIconAppliList != NULL)
	{
		for (i = 0; g_cUseXIconAppliList[i] != NULL; i ++)
		{
			if (cUseXIconAppliListNew != NULL)
			{
				for (j = 0; cUseXIconAppliListNew[j] != NULL; j ++)
				{
					if (strcmp (g_cUseXIconAppliList[i], cUseXIconAppliListNew[j]) == 0)
						break ;
				}
			}
			if (cUseXIconAppliListNew == NULL || cUseXIconAppliListNew[j] == NULL)  // pas trouve.
				cairo_dock_set_class_use_xicon (g_cUseXIconAppliList[i], FALSE);
		}
		g_strfreev (g_cUseXIconAppliList);
	}
	g_cUseXIconAppliList = cUseXIconAppliListNew;*/
	
	gboolean bOverWriteXIconsOld = g_bOverWriteXIcons;
	g_bOverWriteXIcons = cairo_dock_get_boolean_key_value (pKeyFile, "TaskBar", "overwrite xicon", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	
	
	//\___________________ On recupere les parametres des applets.
	g_tIconAuthorizedWidth[CAIRO_DOCK_APPLET] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "applet width", &bFlushConfFileNeeded, 48, "Applets", "max icon size");

	g_tIconAuthorizedHeight[CAIRO_DOCK_APPLET] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "applet height", &bFlushConfFileNeeded, 48, "Applets", "max icon size");

	g_tAnimationType[CAIRO_DOCK_APPLET] = cairo_dock_get_animation_type_key_value (pKeyFile, "Icons", "applet animation", &bFlushConfFileNeeded, CAIRO_DOCK_ROTATE, "Applets", "animation type");

	g_tNbAnimationRounds[CAIRO_DOCK_APPLET] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "applet number of rounds", &bFlushConfFileNeeded, 2, "Applets", "number of animation rounds");
	
	int iNbModules = cairo_dock_get_nb_modules ();
	gchar **cActiveModuleList = g_new0 (gchar *, iNbModules + 1), **cActiveModuleList_n;  // +1 pour le NULL terminal.
	GString *sKeyName = g_string_new ("");
	guint iNbActiveModules=0;
	for (i = 0; i < CAIRO_DOCK_NB_CATEGORY; i ++)
	{
		g_string_printf (sKeyName, "%s_%d", "modules", i);
		cActiveModuleList_n = cairo_dock_get_string_list_key_value (pKeyFile, "Applets", sKeyName->str, &bFlushConfFileNeeded, &length, NULL, "Applets", "active modules");
		
		if (cActiveModuleList_n != NULL)
		{
			int j = 0;
			while (cActiveModuleList_n[j] != NULL && iNbActiveModules < iNbModules)
			{
				cActiveModuleList[iNbActiveModules] = cActiveModuleList_n[j];
				iNbActiveModules ++;
				j ++;
			}
			g_free (cActiveModuleList_n);
		}
	}
	g_string_free (sKeyName, TRUE);

	//\___________________ On recupere les parametres des separateurs.
	g_tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR12] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "separator width", &bFlushConfFileNeeded, 48, "Separators", "min icon size");
	g_tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR23] = g_tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR12];

	g_tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR12] = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "separator height", &bFlushConfFileNeeded, 48, "Separators", "min icon size");
	g_tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR23] = g_tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR12];

	gboolean bUseSeparatorOld = g_bUseSeparator;
	g_bUseSeparator = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "use separator", &bFlushConfFileNeeded, TRUE, "Separators", NULL);

	g_free (g_cSeparatorImage);
	g_cSeparatorImage = cairo_dock_get_string_key_value (pKeyFile, "Icons", "separator image", &bFlushConfFileNeeded, NULL, "Separators", NULL);

	g_bRevolveSeparator = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "revolve separator image", &bFlushConfFileNeeded, TRUE, "Separators", NULL);

	g_bConstantSeparatorSize = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "force size", &bFlushConfFileNeeded, TRUE, "Separators", NULL);


	//\___________________ On recupere les parametres des dialogues.
	gchar *cButtonOkImage = cairo_dock_get_string_key_value (pKeyFile, "Dialogs", "button_ok image", &bFlushConfFileNeeded, NULL, NULL, NULL);
	gchar *cButtonCancelImage = cairo_dock_get_string_key_value (pKeyFile, "Dialogs", "button_cancel image", &bFlushConfFileNeeded, NULL, NULL, NULL);

	g_iDialogButtonWidth = cairo_dock_get_integer_key_value (pKeyFile, "Dialogs", "button width", &bFlushConfFileNeeded, 48, NULL, NULL);
	g_iDialogButtonHeight = cairo_dock_get_integer_key_value (pKeyFile, "Dialogs", "button height", &bFlushConfFileNeeded, 32, NULL, NULL);

	double couleur_bulle[4] = {1.0, 1.0, 1.0, 0.7};
	cairo_dock_get_double_list_key_value (pKeyFile, "Dialogs", "background color", &bFlushConfFileNeeded, g_fDialogColor, 4, couleur_bulle, NULL, NULL);

	g_iDialogIconSize = cairo_dock_get_integer_key_value (pKeyFile, "Dialogs", "icon size", &bFlushConfFileNeeded, 48, NULL, NULL);

	g_free (g_cDialogMessagePolice);
	if (cairo_dock_get_boolean_key_value (pKeyFile, "Dialogs", "homogeneous text", &bFlushConfFileNeeded, TRUE, NULL, NULL))
	{
		g_iDialogMessageSize = g_iLabelSize;
		g_cDialogMessagePolice = g_strdup (g_cLabelPolice);
		g_iDialogMessageWeight = g_iLabelWeight;
		g_iDialogMessageStyle = g_iLabelStyle;
	}
	else
	{
		g_cDialogMessagePolice = cairo_dock_get_string_key_value (pKeyFile, "Dialogs", "message police", &bFlushConfFileNeeded, "sans", NULL, NULL);
		g_iDialogMessageSize = cairo_dock_get_integer_key_value (pKeyFile, "Dialogs", "message size", &bFlushConfFileNeeded, 14, NULL, NULL);
		g_iDialogMessageWeight = cairo_dock_get_integer_key_value (pKeyFile, "Dialogs", "message weight", &bFlushConfFileNeeded, 5, NULL, NULL);
		g_iDialogMessageWeight = ((PANGO_WEIGHT_HEAVY - PANGO_WEIGHT_ULTRALIGHT) * g_iLabelWeight + 9 * PANGO_WEIGHT_ULTRALIGHT - PANGO_WEIGHT_HEAVY) / 8;  // on se ramene aux intervalles definit par Pango.
		if (cairo_dock_get_boolean_key_value (pKeyFile, "Dialogs", "message italic", &bFlushConfFileNeeded, FALSE, NULL, NULL))
			g_iDialogMessageStyle = PANGO_STYLE_ITALIC;
		else
			g_iDialogMessageStyle = PANGO_STYLE_NORMAL;
	}
	
	double couleur_dtext[4] = {0., 0., 0., 1.};
	cairo_dock_get_double_list_key_value (pKeyFile, "Dialogs", "text color", &bFlushConfFileNeeded, g_fDialogTextColor, 4, couleur_dtext, NULL, NULL);
	
	double couleur_desklett[4] = {1.0, 1.0, 1.0, 0.2};
	cairo_dock_get_double_list_key_value (pKeyFile, "Desklets", "background color", &bFlushConfFileNeeded, g_fDeskletColor, 4, couleur_desklett, NULL, NULL);
	couleur_desklett[3] = .6;
	cairo_dock_get_double_list_key_value (pKeyFile, "Desklets", "background color inside", &bFlushConfFileNeeded, g_fDeskletColorInside, 4, couleur_desklett, NULL, NULL);
	
	g_iFileSortType = cairo_dock_get_integer_key_value (pKeyFile, "System", "sort files", &bFlushConfFileNeeded, CAIRO_DOCK_FM_SORT_BY_NAME, NULL, NULL);
	g_bShowHiddenFiles = cairo_dock_get_boolean_key_value (pKeyFile, "System", "show hidden files", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	
	if (g_cRaiseDockShortcut != NULL)
	{
		cd_keybinder_unbind (g_cRaiseDockShortcut, (CDBindkeyHandler) cairo_dock_raise_from_keyboard);
		g_free (g_cRaiseDockShortcut);
	}
	g_cRaiseDockShortcut = cairo_dock_get_string_key_value (pKeyFile, "Position", "raise shortcut", &bFlushConfFileNeeded, NULL, NULL, NULL);
	
	
	//\___________________ On (re)charge tout, car n'importe quel parametre peut avoir change.
	switch (iScreenBorder)
	{
		case CAIRO_DOCK_BOTTOM :
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

	if (g_iMaxAuthorizedWidth == 0)
		g_iMaxAuthorizedWidth = g_iScreenWidth[pDock->bHorizontalDock];

	cairo_dock_load_dialog_buttons (CAIRO_CONTAINER (pDock), cButtonOkImage, cButtonCancelImage);
	g_free (cButtonOkImage);
	g_free (cButtonCancelImage);
	
	cairo_surface_destroy (g_pIndicatorSurface[0]);
	cairo_surface_destroy (g_pIndicatorSurface[1]);
	g_pIndicatorSurface[0] = NULL;
	g_pIndicatorSurface[1] = NULL;
	if (g_bShowAppli)
	{
		double fLauncherWidth = (g_tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] != 0 ? g_tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] : 48);
		double fLauncherHeight = (g_tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] != 0 ? g_tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] : 48);
		
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
		
		double fMasxScale = (g_bLinkIndicatorWithIcon ? (1 + g_fAmplitude) : 1 + 0);
		g_pIndicatorSurface[CAIRO_DOCK_HORIZONTAL] = cairo_dock_create_surface_from_image (
			cIndicatorImagePath,
			pCairoContext,
			fMasxScale,
			fLauncherWidth * fIndicatorRatio,
			fLauncherHeight * fIndicatorRatio,
			&g_fIndicatorWidth,
			&g_fIndicatorHeight,
			TRUE);
		//g_print ("g_pIndicatorSurface : %.2fx%.2f\n", g_fIndicatorWidth, g_fIndicatorHeight);
		g_pIndicatorSurface[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (
			g_pIndicatorSurface[CAIRO_DOCK_HORIZONTAL],
			pCairoContext, 
			g_fIndicatorWidth * fMasxScale,
			g_fIndicatorHeight * fMasxScale,
			- G_PI / 2);
		
		cairo_destroy (pCairoContext);
	}
	g_free (cIndicatorImagePath);

	g_fReflectSize = 0;
	for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
	{
		if (g_tIconAuthorizedHeight[i] > 0)
			g_fReflectSize = MAX (g_fReflectSize, g_tIconAuthorizedHeight[i]);
	}
	if (g_fReflectSize == 0)  // on n'a pas trouve de hauteur, on va essayer avec la largeur.
	{
		for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
		{
			if (g_tIconAuthorizedWidth[i] > 0)
				g_fReflectSize = MAX (g_fReflectSize, g_tIconAuthorizedWidth[i]);
		}
		if (g_fReflectSize > 0)
			g_fReflectSize = MIN (48, g_fReflectSize);
		else
			g_fReflectSize = 48;
	}
	g_fReflectSize *= fFieldDepth;
	cd_debug ("  g_fReflectSize : %.2f pixels\n", g_fReflectSize);
	
	if (bUniquePidOld != g_bUniquePid || bGroupAppliByClassOld != g_bGroupAppliByClass || bHideVisibleApplisOld != g_bHideVisibleApplis || bAppliOnCurrentDesktopOnlyOld != g_bAppliOnCurrentDesktopOnly || (bMixLauncherAppliOld != g_bMixLauncherAppli) || (bOverWriteXIconsOld != g_bOverWriteXIcons) || (cairo_dock_application_manager_is_running () && ! g_bShowAppli))  // on ne veut plus voir les applis, il faut donc les enlever.
	{
		cairo_dock_stop_application_manager ();
	}

	pDock->icons = g_list_sort (pDock->icons, (GCompareFunc) cairo_dock_compare_icons_order);  // on reordonne car l'ordre des types a pu changer.

	if (bUseSeparatorOld && ! g_bUseSeparator)
		cairo_dock_remove_all_separators (pDock);
		
	g_fBackgroundImageWidth = 1e4;  // inutile de mettre a jour les decorations maintenant.
	g_fBackgroundImageHeight = 1e4;
	if (pDock->icons == NULL)
	{
		pDock->fFlatDockWidth = - g_iIconGap;  // car on ne le connaissais pas encore au moment de la creation du dock.
		cairo_dock_build_docks_tree_with_desktop_files (pDock, g_cCurrentLaunchersPath);
	}
	else
		cairo_dock_reload_buffers_in_all_docks ();  // tout sauf les applets, qui seront rechargees en bloc juste apres.


	if (! cairo_dock_application_manager_is_running () && g_bShowAppli)  // maintenant on veut voir les applis !
	{
		cairo_dock_start_application_manager (pDock);  // va inserer le separateur si necessaire.
	}

	if (g_bUseSeparator && ! bUseSeparatorOld)
	{
		cairo_dock_insert_separators_in_dock (pDock);
	}
	
	GTimeVal time_val;
	g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
	double fTime = time_val.tv_sec + time_val.tv_usec * 1e-6;
	cairo_dock_activate_modules_from_list (cActiveModuleList, pDock, fTime);
	cairo_dock_deactivate_old_modules (fTime);
	g_strfreev (cActiveModuleList);

	cairo_dock_set_all_views_to_default ();
	
	g_bReserveSpace = g_bReserveSpace && (g_cRaiseDockShortcut == NULL);
	cairo_dock_reserve_space_for_dock (pDock, g_bReserveSpace);


	cairo_dock_load_visible_zone (pDock, cVisibleZoneImageFile, g_iVisibleZoneWidth, g_iVisibleZoneHeight, g_fVisibleZoneAlpha);
	g_free (cVisibleZoneImageFile);

	cairo_dock_load_background_decorations (pDock);


	pDock->iMouseX = 0;  // on se place hors du dock initialement.
	pDock->iMouseY = 0;
	pDock->calculate_max_dock_size (pDock);
	pDock->calculate_icons (pDock);
	gtk_widget_queue_draw (pDock->pWidget);  // le 'gdk_window_move_resize' ci-dessous ne provoquera pas le redessin si la taille n'a pas change.

	if (pDock->bAtBottom && pDock->bAutoHide)
	{
		cairo_dock_place_root_dock (pDock);  // semble necessaire dans le cas de l'auto-hide.
	}

	//\___________________ On ecrit si necessaire.
	if (! bFlushConfFileNeeded)
		bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile, CAIRO_DOCK_VERSION);
	if (bFlushConfFileNeeded)
	{
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, CAIRO_DOCK_SHARE_DATA_DIR);
		g_key_file_free (pKeyFile);
		pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
		
		cairo_dock_update_conf_file_with_available_modules2 (pKeyFile, cConfFilePath);
	}

	cairo_dock_update_renderer_list_for_gui ();
	
	if (g_cRaiseDockShortcut != NULL)
		cd_keybinder_bind (g_cRaiseDockShortcut, (CDBindkeyHandler) cairo_dock_raise_from_keyboard, (gpointer)NULL);
	
	//\___________________ On applique les modifs au fichier de conf easy.
	cairo_dock_copy_to_easy_conf_file (pKeyFile, g_cEasyConfFile);

	g_key_file_free (pKeyFile);

	cairo_dock_mark_theme_as_modified (TRUE);
	
	s_bLoading = FALSE;
}

gboolean cairo_dock_is_loading (void)
{
	return s_bLoading;
}


static void _cairo_dock_user_action_on_config (GtkDialog *pDialog, gint action, gpointer *user_data);  // declaree en amont, car les 2 s'appellent mutuellement.

static gboolean cairo_dock_edit_conf_file_core (GtkWindow *pWindow, gchar *cConfFilePath, const gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CairoDockConfigFunc pConfigFunc, gpointer data, GFunc pFreeUserDataFunc, CairoDockConfigFunc pConfigFunc2, gchar *cConfFilePath2, gchar *cButtonConvert, gchar *cButtonRevert, gchar *cGettextDomain)
{
	cd_message ("%s (%s; %s)", __func__, cConfFilePath, cConfFilePath2);
	GSList *pWidgetList = NULL;
	GtkTextBuffer *pTextBuffer = NULL;  // le buffer est lie au widget, donc au pDialog.
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	g_key_file_remove_key (pKeyFile, "Desktop Entry", "X-Ubuntu-Gettext-Domain", NULL);  // salete de traducteur automatique.
	
	GPtrArray *pDataGarbage = g_ptr_array_new ();
	GtkWidget *pDialog = cairo_dock_generate_advanced_ihm_from_keyfile (pKeyFile, cTitle, pWindow, &pWidgetList, (pConfigFunc != NULL), iIdentifier, cPresentedGroup, (pConfigFunc2 != NULL), cButtonConvert, cGettextDomain, pDataGarbage);
	if (pDialog == NULL || pWidgetList == NULL)
	{
		pDialog = cairo_dock_generate_basic_ihm_from_keyfile (cConfFilePath, cTitle, pWindow, &pTextBuffer, (pConfigFunc != NULL), (pConfigFunc2 != NULL), cButtonConvert, NULL);
	}
	g_return_val_if_fail (pDialog != NULL, FALSE);
	
	if (iWindowWidth != 0 && iWindowHeight != 0)
		gtk_window_resize (GTK_WINDOW (pDialog), iWindowWidth, iWindowHeight);
	gint iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pDialog), &iWidth, &iHeight);
	iWidth = MAX (iWidth, iWindowWidth);  // car la taille n'est peut-etre pas encore effective.
	iHeight = MAX (iHeight, iWindowHeight);
	
	if (g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] <= 0 || g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] <= 0)  // peut arriver avec le panneau de choix du theme initial.
	{
		GdkScreen *gdkscreen = gdk_screen_get_default ();
		g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] = gdk_screen_get_width (gdkscreen);  // on n'a besoin que de ca ici, le reste sera recupere au moment voulu.
		g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] = gdk_screen_get_height (gdkscreen);
	}
	gtk_window_move (GTK_WINDOW (pDialog), (g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] - iWidth) / 2, (g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] - iHeight) / 2);
	
	if (pConfigFunc != NULL)  // alors on autorise la modification a la volee, avec un bouton "Appliquer". La fenetre doit donc laisser l'appli se derouler.
	{
		gpointer *user_data = g_new (gpointer, 17);
		user_data[0] = pKeyFile;
		user_data[1] = pWidgetList;
		user_data[2] = cConfFilePath;
		user_data[3] = pTextBuffer;
		user_data[4] = pConfigFunc;
		user_data[5] = data;
		user_data[6] = pFreeUserDataFunc;
		user_data[7] = cConfFilePath2;
		user_data[8] = pConfigFunc2;
		user_data[9] = pWindow;
		user_data[10] = cTitle;
		user_data[11] = GINT_TO_POINTER (iWindowWidth);
		user_data[12] = GINT_TO_POINTER (iWindowHeight);
		user_data[13] = GINT_TO_POINTER ((int) iIdentifier);
		user_data[14] = cButtonConvert;
		user_data[15] = cButtonRevert;
		user_data[16] = pDataGarbage;
		
		g_signal_connect (pDialog, "response", G_CALLBACK (_cairo_dock_user_action_on_config), user_data);
		return FALSE;
	}
	else  // sinon on bloque l'appli jusqu'a ce que l'utilisateur valide ou annule.
	{
		gboolean config_ok;
		gint action = gtk_dialog_run (GTK_DIALOG (pDialog));
		if (action == GTK_RESPONSE_ACCEPT)
		{
			config_ok = TRUE;
			if (pWidgetList != NULL)
			{
				cairo_dock_update_keyfile_from_widget_list (pKeyFile, pWidgetList);
				cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
			}
			else
			{
				GtkTextIter start, end;
				gtk_text_buffer_get_iter_at_offset (pTextBuffer, &start, 0);
				gtk_text_buffer_get_iter_at_offset (pTextBuffer, &end, -1);
				
				gchar *cConfiguration = gtk_text_buffer_get_text (pTextBuffer, &start, &end, FALSE);
				
				gboolean write_ok = g_file_set_contents (cConfFilePath, cConfiguration, -1, NULL);
				g_free (cConfiguration);
				if (! write_ok)
				{
					cd_warning ("error while writing to %s", cConfFilePath);
					config_ok = FALSE;
				}
			}
		}
		else
		{
			config_ok = FALSE;
		}
		
		g_key_file_free (pKeyFile);
		cairo_dock_free_generated_widget_list (pWidgetList);
		gtk_widget_destroy (GTK_WIDGET (pDialog));
		g_ptr_array_foreach (pDataGarbage, (GFunc) g_free, NULL);
		g_ptr_array_free (pDataGarbage, TRUE);
		g_free (cConfFilePath);
		g_free (cConfFilePath2);
		///g_free (cTitle);
		g_free (cButtonConvert);
		g_free (cButtonRevert);
		return config_ok;
	}
}

static void _cairo_dock_user_action_on_config (GtkDialog *pDialog, gint action, gpointer *user_data)
{
	GKeyFile *pKeyFile = user_data[0];
	GSList *pWidgetList = user_data[1];
	gchar *cConfFilePath = user_data[2];
	GtkTextBuffer *pTextBuffer = user_data[3];
	CairoDockConfigFunc pConfigFunc = user_data[4];
	gpointer data = user_data[5];
	GFunc pFreeUserDataFunc = user_data[6];
	gchar *cConfFilePath2 = user_data[7];
	CairoDockConfigFunc pConfigFunc2 = user_data[8];
	GtkWindow *pWindow = user_data[9];
	gchar *cTitle = user_data[10];
	int iWindowWidth = GPOINTER_TO_INT (user_data[11]);
	int iWindowHeight = GPOINTER_TO_INT (user_data[12]);
	gchar iIdentifier = GPOINTER_TO_INT (user_data[13]);
	gchar *cButtonConvert = user_data[14];
	gchar *cButtonRevert = user_data[15];
	GPtrArray *pDataGarbage = user_data[16];

	if (action == GTK_RESPONSE_ACCEPT || action == GTK_RESPONSE_APPLY)
	{
		gtk_window_set_modal (GTK_WINDOW (pDialog), TRUE);  // pour prevenir tout interaction avec l'appli pendant sa re-configuration.

		if (pWidgetList != NULL)
		{
			cairo_dock_update_keyfile_from_widget_list (pKeyFile, pWidgetList);
			cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
		}
		else
		{
			GtkTextIter start, end;
			gtk_text_buffer_get_iter_at_offset (pTextBuffer, &start, 0);
			gtk_text_buffer_get_iter_at_offset (pTextBuffer, &end, -1);

			gchar *cConfiguration = gtk_text_buffer_get_text (pTextBuffer, &start, &end, FALSE);

			gboolean write_ok = g_file_set_contents (cConfFilePath, cConfiguration, -1, NULL);
			g_free (cConfiguration);
			if (! write_ok)
				cd_warning ("error while writing to %s", cConfFilePath);
		}

		if (pConfigFunc != NULL)
			pConfigFunc (cConfFilePath, data);

		gtk_window_set_modal (GTK_WINDOW (pDialog), FALSE);
	}

	if (action == GTK_RESPONSE_ACCEPT || action == GTK_RESPONSE_REJECT || action == GTK_RESPONSE_NONE)
	{
		cairo_dock_mark_prefered_conf_file (cConfFilePath);

		gtk_widget_destroy (GTK_WIDGET (pDialog));
		g_key_file_free (pKeyFile);
		cairo_dock_free_generated_widget_list (pWidgetList);
		g_ptr_array_foreach (pDataGarbage, (GFunc) g_free, NULL);
		g_ptr_array_free (pDataGarbage, TRUE);
		g_free (cConfFilePath);
		g_free (cConfFilePath2);
		g_free (cTitle);
		g_free (cButtonConvert);
		g_free (cButtonRevert);
		if (pFreeUserDataFunc != NULL)
			pFreeUserDataFunc (data, NULL);
		g_free (user_data);
	}
	else if (action == GTK_RESPONSE_HELP)  // demande de bascule d'un fichier de conf a un autre.
	{
		gtk_widget_destroy (GTK_WIDGET (pDialog));
		g_key_file_free (pKeyFile);
		cairo_dock_free_generated_widget_list (pWidgetList);
		g_ptr_array_foreach (pDataGarbage, (GFunc) g_free, NULL);
		g_ptr_array_free (pDataGarbage, TRUE);
		g_free (user_data);

		cairo_dock_edit_conf_file_core (pWindow, cConfFilePath2, cTitle, iWindowWidth, iWindowHeight, iIdentifier, NULL, pConfigFunc2, data, pFreeUserDataFunc, pConfigFunc, cConfFilePath, cButtonRevert, cButtonConvert, NULL);
	}
}


/**
*Lis un fichier de conf, construit l'IHM appropriee, et la presente a l'utilisateur.
*@param pWidget
*@param cConfFilePath
*@param cTitle
*@param iWindowWidth
*@param iWindowHeight
*@param iIdentifier
*@param cPresentedGroup
*@param pConfigFunc
*@param data
*@param pFreeUserDataFunc
*@param pConfigFunc2
*@param cConfFilePath2
*@param cButtonConvert
*@param cButtonRevert
*@param cGettextDomain
@Returns TRUE si l'utilisateur a ferme le panneau de conf en appuyant sur OK, FALSE sinon.
*/
gboolean cairo_dock_edit_conf_file_full (GtkWindow *pWindow, gchar *cConfFilePath, const gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CairoDockConfigFunc pConfigFunc, gpointer data, GFunc pFreeUserDataFunc, CairoDockConfigFunc pConfigFunc2, gchar *cConfFilePath2, gchar *cButtonConvert, gchar *cButtonRevert, gchar *cGettextDomain)
{
	return cairo_dock_edit_conf_file_core (pWindow, g_strdup (cConfFilePath), g_strdup (cTitle), iWindowWidth, iWindowHeight, iIdentifier, cPresentedGroup, pConfigFunc, data, pFreeUserDataFunc, pConfigFunc2, g_strdup (cConfFilePath2), g_strdup (cButtonConvert), g_strdup (cButtonRevert), cGettextDomain);
}


void cairo_dock_update_conf_file (gchar *cConfFilePath, GType iFirstDataType, ...)  // type, groupe, nom, valeur, etc. finir par G_TYPE_INVALID.
{
	cd_message ("%s (%s)", __func__, cConfFilePath);
	va_list args;
	va_start (args, iFirstDataType);

	GKeyFile *pKeyFile = g_key_file_new ();
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		//va_end (args);
		//return ;
	}

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



CairoDockDesktopEnv cairo_dock_guess_environment (void)
{
	const gchar * cEnv = g_getenv ("GNOME_DESKTOP_SESSION_ID");
	CairoDockDesktopEnv iDesktopEnv;
	if (cEnv == NULL || *cEnv == '\0')
	{
		cEnv = g_getenv ("KDE_FULL_SESSION");
		if (cEnv == NULL || *cEnv == '\0')
		{
			if (! cairo_dock_property_is_present_on_root ("_DT_SAVE_MODE"))
				iDesktopEnv = CAIRO_DOCK_UNKNOWN_ENV;
			else
				iDesktopEnv = CAIRO_DOCK_XFCE;
		}
		else
			iDesktopEnv = CAIRO_DOCK_KDE;
	}
	else
		iDesktopEnv = CAIRO_DOCK_GNOME;
	
	return iDesktopEnv;
}


static void cairo_dock_copy_value_to_keyfile (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, GKeyFile *pKeyFile2, gchar *cGroupName2, gchar *cKeyName2)
{
	GError *erreur = NULL;
	gchar *cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	else
	{
		g_key_file_set_string (pKeyFile2, cGroupName2, cKeyName2, cValue);
	}
	g_free (cValue);
}
void cairo_dock_copy_easy_conf_file (gchar *cEasyConfFilePath, GKeyFile *pMainKeyFile)
{
	//g_print ("%s (%s)\n", __func__, cEasyConfFilePath);

	//\___________________ On ouvre le fichier de conf.
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cEasyConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}

	//\___________________ On recupere les parametres systeme.
	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "screen border", pMainKeyFile, "Position", "screen border");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "auto-hide", pMainKeyFile, "Position", "auto-hide");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "show applications", pMainKeyFile, "TaskBar", "show applications");

	GString *sKeyName = g_string_new ("");
	int i;
	for (i = 0; i < CAIRO_DOCK_NB_CATEGORY; i ++)
	{
		g_string_printf (sKeyName, "%s_%d", "modules", i);
		cairo_dock_copy_value_to_keyfile (pKeyFile, "System", sKeyName->str, pMainKeyFile, "Applets", sKeyName->str);
	}
	g_string_free (sKeyName, TRUE);

	//\___________________ On recupere les parametres de personnalisation.
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "callback image", pMainKeyFile, "Background", "callback image");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "background image", pMainKeyFile, "Background", "background image");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "separator image", pMainKeyFile, "Icons", "separator image");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "main dock view", pMainKeyFile, "Views", "main dock view");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "sub-dock view", pMainKeyFile, "Views", "sub-dock view");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Icons", "launcher width");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Icons", "launcher height");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Icons", "appli width");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Icons", "appli height");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Icons", "applet width");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Icons", "applet height");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Icons", "separator width");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Icons", "separator height");

	gboolean bFlushConfFileNeeded;
	double fValue = cairo_dock_get_double_key_value (pKeyFile, "Personnalisation", "zoom", &bFlushConfFileNeeded, 2.0, NULL, NULL);
	g_key_file_set_double (pMainKeyFile, "Icons", "amplitude", fValue - 1);

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon gap", pMainKeyFile, "Icons", "icon gap");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "launcher animation", pMainKeyFile, "Icons", "launcher animation");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "appli animation", pMainKeyFile, "Icons", "appli animation");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "corner radius", pMainKeyFile, "Background", "corner radius");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "line color", pMainKeyFile, "Background", "line color");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "police", pMainKeyFile, "Icons", "police");

	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "font size", pMainKeyFile, "Icons", "size");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "desklet bg color", pMainKeyFile, "Desklets", "background color");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "desklet bg color inside", pMainKeyFile, "Desklets", "background color inside");
	
	g_key_file_free (pKeyFile);
}

void cairo_dock_copy_to_easy_conf_file (GKeyFile *pMainKeyFile, gchar *cEasyConfFilePath)
{
	//\___________________ On recupere le patron.
	gchar *cEasyConfTemplate = g_strdup_printf ("%s/%s", CAIRO_DOCK_SHARE_DATA_DIR, CAIRO_DOCK_EASY_CONF_FILE);

	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cEasyConfTemplate, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	g_free (cEasyConfTemplate);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}

	//\___________________ On ecrit les parametres systeme.
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Position", "screen border", pKeyFile, "System", "screen border");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Position", "auto-hide", pKeyFile, "System", "auto-hide");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "TaskBar", "show applications", pKeyFile, "System", "show applications");

	GString *sKeyName = g_string_new ("");
	int i;
	for (i = 0; i < CAIRO_DOCK_NB_CATEGORY; i ++)
	{
		g_string_printf (sKeyName, "%s_%d", "modules", i);
		cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Applets", sKeyName->str, pKeyFile, "System", sKeyName->str);
	}
	g_string_free (sKeyName, TRUE);

	//\___________________ On ecrit les parametres de personnalisation.
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Background", "callback image", pKeyFile, "Personnalisation", "callback image");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Background", "background image", pKeyFile, "Personnalisation", "background image");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Icons", "separator image", pKeyFile, "Personnalisation", "separator image");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Views", "main dock view", pKeyFile, "Personnalisation", "main dock view");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Views", "sub-dock view", pKeyFile, "Personnalisation", "sub-dock view");

	g_key_file_set_integer (pKeyFile, "Personnalisation", "icon size", MAX (g_tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER], g_tIconAuthorizedWidth[CAIRO_DOCK_APPLI]));

	g_key_file_set_double (pKeyFile, "Personnalisation", "zoom", 1 + g_fAmplitude);

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Icons", "icon gap", pKeyFile, "Personnalisation", "icon gap");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Icons", "launcher animation", pKeyFile, "Personnalisation", "launcher animation");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Icons", "appli animation", pKeyFile, "Personnalisation", "appli animation");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Background", "corner radius", pKeyFile, "Personnalisation", "corner radius");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Background", "line color", pKeyFile, "Personnalisation", "line color");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Icons", "police", pKeyFile, "Personnalisation", "police");

	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Icons", "size", pKeyFile, "Personnalisation", "font size");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Desklets", "background color", pKeyFile, "Personnalisation", "desklet bg color");
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Desklets", "background color inside", pKeyFile, "Personnalisation", "desklet bg color inside");
	
	//\___________________ On ecrit tout.
	cairo_dock_write_keys_to_file (pKeyFile, cEasyConfFilePath);
	
	//\___________________ On complete.
	cairo_dock_update_easy_conf_file_with_available_modules2 (pKeyFile, cEasyConfFilePath);

	//cairo_dock_update_easy_conf_file_with_renderers (pKeyFile, cEasyConfFilePath);
	g_key_file_free (pKeyFile);
}

void cairo_dock_build_easy_conf_file (gchar *cMainConfFilePath, gchar *cEasyConfFilePath)
{
	GKeyFile *pMainKeyFile = g_key_file_new ();
	GError *erreur = NULL;
	g_key_file_load_from_file (pMainKeyFile, cMainConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}

	cairo_dock_copy_to_easy_conf_file (pMainKeyFile, cEasyConfFilePath);
	g_key_file_free (pMainKeyFile);
}

void cairo_dock_read_easy_conf_file (gchar *cEasyConfFilePath, gpointer data)
{
	//\___________________ On ouvre le fichier de conf principal.
	GKeyFile *pMainKeyFile = g_key_file_new ();
	GError *erreur = NULL;
	g_key_file_load_from_file (pMainKeyFile, g_cConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}

	//\___________________ On remplit les champs fournis par le fichier de conf easy.
	cairo_dock_copy_easy_conf_file (cEasyConfFilePath, pMainKeyFile);

	//\___________________ On ecrit tout.
	cairo_dock_write_keys_to_file (pMainKeyFile, g_cConfFile);
	g_key_file_free (pMainKeyFile);

	//\___________________ On recharge l'appli.
	cairo_dock_read_conf_file (g_cConfFile, data);
}



gboolean cairo_dock_use_full_conf_file (void)
{
	gchar *cFile = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_TYPE_CONF_FILE_FILE);
	gsize length = 0;
	gchar *cContent = NULL;
	g_file_get_contents (cFile,
		&cContent,
		&length,
		NULL);
	g_free (cFile);

	gboolean bFullConfFile = (length > 0 && strcmp (cContent, CAIRO_DOCK_CONF_FILE) == 0);
	g_free (cContent);
	return bFullConfFile;
}
void cairo_dock_mark_prefered_conf_file (gchar *cConfFilePath)
{
	gchar *cConfFileName = g_path_get_basename (cConfFilePath);
	g_return_if_fail (cConfFileName != NULL);
	if (strcmp (cConfFileName, CAIRO_DOCK_CONF_FILE) == 0 || strcmp (cConfFileName, CAIRO_DOCK_EASY_CONF_FILE) == 0)
	{
		gchar *cFile = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_TYPE_CONF_FILE_FILE);

		g_file_set_contents (cFile,
			cConfFileName,
			-1,
			NULL);

		g_free (cFile);
	}
	g_free (cConfFileName);
}


void cairo_dock_get_version_from_string (gchar *cVersionString, int *iMajorVersion, int *iMinorVersion, int *iMicroVersion)
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
