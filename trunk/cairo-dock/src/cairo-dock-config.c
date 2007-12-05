/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
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
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-gui-factory.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-menu.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-config.h"

#define CAIRO_DOCK_TYPE_CONF_FILE_FILE ".cairo-dock-conf-file"

static const gchar * s_cIconTypeNames[(CAIRO_DOCK_NB_TYPES+1)/2] = {"launchers", "applications", "applets"};

extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;
extern gchar *g_cLanguage;
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

extern gboolean g_bAutoHide;
extern double g_fVisibleZoneAlpha;
extern gboolean g_bDirectionUp;
extern gboolean g_bSameHorizontality;
extern double g_fSubDockSizeRatio;
extern gboolean g_bAnimateSubDock;
extern int g_iLeaveSubDockDelay;
extern int g_iShowSubDockDelay;

extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern int g_iLabelWeight;
extern int g_iLabelStyle;
extern gboolean g_bLabelForPointedIconOnly;
extern double g_fLabelAlphaThreshold;
extern gboolean g_bTextAlwaysHorizontal;

extern gchar **g_cDefaultIconDirectory;
extern GtkIconTheme *g_pIconTheme;
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
extern gboolean g_bDemandsAttentionWithDialog;
extern gboolean g_bDemandsAttentionWithAnimation;
extern gboolean g_bAnimateOnActiveWindow;

extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];

extern gchar *g_cSeparatorImage;
extern gboolean g_bRevolveSeparator;
extern gboolean g_bConstantSeparatorSize;

extern int g_iDialogButtonWidth;
extern int g_iDialogButtonHeight;
extern double g_fDialogAlpha;
extern int g_iDialogIconSize;


gboolean cairo_dock_get_boolean_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gboolean bDefaultValue)
{
	GError *erreur = NULL;
	gboolean bValue = g_key_file_get_boolean (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
				bValue = bDefaultValue;
			}
			else
				g_print (" (recuperee)\n");
		}
		
		g_key_file_set_boolean (pKeyFile, cGroupName, cKeyName, bValue);
		*bFlushConfFileNeeded = TRUE;
	}
	return bValue;
}
int cairo_dock_get_integer_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int iDefaultValue)
{
	GError *erreur = NULL;
	int iValue = g_key_file_get_integer (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
				iValue = iDefaultValue;
			}
			else
				g_print (" (recuperee)\n");
		}
		g_free (cGroupNameUpperCase);
		
		g_key_file_set_integer (pKeyFile, cGroupName, cKeyName, iValue);
		*bFlushConfFileNeeded = TRUE;
	}
	return iValue;
}
double cairo_dock_get_double_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double fDefaultValue)
{
	GError *erreur = NULL;
	double fValue = g_key_file_get_double (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
				fValue = fDefaultValue;
			}
			else
				g_print (" (recuperee)\n");
		}
		g_free (cGroupNameUpperCase);
		
		g_key_file_set_double (pKeyFile, cGroupName, cKeyName, fValue);
		*bFlushConfFileNeeded = TRUE;
	}
	return fValue;
}
gchar *cairo_dock_get_string_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, const gchar *cDefaultValue)
{
	GError *erreur = NULL;
	gchar *cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
				cValue = g_strdup (cDefaultValue);
			}
			else
				g_print (" (recuperee)\n");
		}
		g_free (cGroupNameUpperCase);
		
		g_key_file_set_string (pKeyFile, cGroupName, cKeyName, (cValue != NULL ? cValue : ""));
		*bFlushConfFileNeeded = TRUE;
	}
	if (cValue != NULL && *cValue == '\0')
	{
		g_free (cValue);
		cValue = NULL;
	}
	return cValue;
}
void cairo_dock_get_integer_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int *iValueBuffer, int iNbElements, int *iDefaultValues)
{
	GError *erreur = NULL;
	gsize length = 0;
	if (iDefaultValues != NULL)
		memcpy (iValueBuffer, iDefaultValues, iNbElements * sizeof (int));
	
	int *iValuesList = g_key_file_get_integer_list (pKeyFile, cGroupName, cKeyName, &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
			}
			else
			{
				g_print (" (recuperee)\n");
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
		*bFlushConfFileNeeded = TRUE;
	}
	else
	{
		if (length > 0)
			memcpy (iValueBuffer, iValuesList, MIN (iNbElements, length) * sizeof (int));
	}
	g_free (iValuesList);
}
void cairo_dock_get_double_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double *fValueBuffer, int iNbElements, double *fDefaultValues)
{
	GError *erreur = NULL;
	gsize length = 0;
	if (fDefaultValues != NULL)
		memcpy (fValueBuffer, fDefaultValues, iNbElements * sizeof (double));
	
	double *fValuesList = g_key_file_get_double_list (pKeyFile, cGroupName, cKeyName, &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
			}
			else
			{
				g_print (" (recuperee)\n");
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
		*bFlushConfFileNeeded = TRUE;
	}
	else
	{
		if (length > 0)
			memcpy (fValueBuffer, fValuesList, MIN (iNbElements, length) * sizeof (double));
	}
	g_free (fValuesList);
}
gchar **cairo_dock_get_string_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gsize *length, gchar *cDefaultValues)
{
	GError *erreur = NULL;
	*length = 0;
	gchar **cValuesList = g_key_file_get_string_list (pKeyFile, cGroupName, cKeyName, length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		
		gchar* cGroupNameUpperCase = g_ascii_strup (cGroupName, -1);
		cValuesList = g_key_file_get_string_list (pKeyFile, cGroupNameUpperCase, cKeyName, length, &erreur);
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
		g_free (cGroupNameUpperCase);
		
		if (*length > 0)
			g_key_file_set_string_list (pKeyFile, cGroupName, cKeyName, (const gchar **)cValuesList, *length);
		else
			g_key_file_set_string (pKeyFile, cGroupName, cKeyName, "");
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

CairoDockAnimationType cairo_dock_get_animation_type_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, CairoDockAnimationType iDefaultAnimation)
{
	/*gchar *cAnimationName = cairo_dock_get_string_key_value (pKeyFile, cGroupName, cKeyName, bFlushConfFileNeeded, cDefaultAnimation);
	int iAnimationType = cairo_dock_get_animation_type_from_name (cAnimationName);  // cAnimationName peut etre NULL.
	g_free(cAnimationName);*/
	CairoDockAnimationType iAnimationType = cairo_dock_get_integer_key_value (pKeyFile, cGroupName, cKeyName, bFlushConfFileNeeded, iDefaultAnimation);
	if (iAnimationType < 0 || iAnimationType >= CAIRO_DOCK_NB_ANIMATIONS)
		iAnimationType = 0;
	return iAnimationType;
}

void cairo_dock_read_conf_file (gchar *cConfFilePath, CairoDock *pDock)
{
	//g_print ("%s (%s)\n", __func__, cConfFilePath);
	GError *erreur = NULL;
	gsize length;
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	//\___________________ On recupere la position du dock.
	pDock->iGapX = cairo_dock_get_integer_key_value (pKeyFile, "Position", "x gap", &bFlushConfFileNeeded, 0);
	pDock->iGapY = cairo_dock_get_integer_key_value (pKeyFile, "Position", "y gap", &bFlushConfFileNeeded, 0);
	
	///gchar *cScreenBorder = cairo_dock_get_string_key_value (pKeyFile, "Position", "screen border", &bFlushConfFileNeeded, "bottom");
	CairoDockPositionType iScreenBorder = cairo_dock_get_integer_key_value (pKeyFile, "Position", "screen border", &bFlushConfFileNeeded, 0);
	if (iScreenBorder < 0 || iScreenBorder >= CAIRO_DOCK_NB_POSITIONS)
		iScreenBorder = 0;
	
	pDock->fAlign = cairo_dock_get_double_key_value (pKeyFile, "Position", "alignment", &bFlushConfFileNeeded, 0.5);
	
	g_bReserveSpace = cairo_dock_get_boolean_key_value (pKeyFile, "Position", "reserve space", &bFlushConfFileNeeded, FALSE);
	
	
	//\___________________ On recupere les parametres de la zone visible.
	gchar *cVisibleZoneImageFile = cairo_dock_get_string_key_value (pKeyFile, "Auto-Hide", "background image", &bFlushConfFileNeeded, NULL);
	
	g_iVisibleZoneWidth = cairo_dock_get_integer_key_value (pKeyFile, "Auto-Hide", "zone width", &bFlushConfFileNeeded, 150);
	g_iVisibleZoneHeight = cairo_dock_get_integer_key_value (pKeyFile, "Auto-Hide", "zone height", &bFlushConfFileNeeded, 25);
	
	g_fVisibleZoneAlpha = cairo_dock_get_double_key_value (pKeyFile, "Auto-Hide", "alpha", &bFlushConfFileNeeded, 0.5);
	
	cairo_dock_deactivate_temporary_auto_hide ();
	g_bAutoHide = cairo_dock_get_boolean_key_value (pKeyFile, "Auto-Hide", "auto-hide", &bFlushConfFileNeeded, FALSE);
	
	g_bReverseVisibleImage = cairo_dock_get_boolean_key_value (pKeyFile, "Auto-Hide", "reverse visible image", &bFlushConfFileNeeded, TRUE);
	
	
	//\___________________ On recupere les parametres des etiquettes.
	g_bLabelForPointedIconOnly = cairo_dock_get_boolean_key_value (pKeyFile, "Labels", "pointed icon only", &bFlushConfFileNeeded, FALSE);
	
	g_fLabelAlphaThreshold = cairo_dock_get_double_key_value (pKeyFile, "Labels", "alpha threshold", &bFlushConfFileNeeded, 10.);
	
	g_free (g_cLabelPolice);
	g_cLabelPolice = cairo_dock_get_string_key_value (pKeyFile, "Labels", "police", &bFlushConfFileNeeded, "sans");
	
	g_iLabelSize = cairo_dock_get_integer_key_value (pKeyFile, "Labels", "size", &bFlushConfFileNeeded, 14);
	
	g_iLabelWeight = cairo_dock_get_integer_key_value (pKeyFile, "Labels", "weight", &bFlushConfFileNeeded, 5);
	g_iLabelWeight = ((PANGO_WEIGHT_HEAVY - PANGO_WEIGHT_ULTRALIGHT) * g_iLabelWeight + 9 * PANGO_WEIGHT_ULTRALIGHT - PANGO_WEIGHT_HEAVY) / 8;  // on se ramene aux intervalles definit par Pango.
	
	gboolean bLabelStyleItalic = cairo_dock_get_boolean_key_value (pKeyFile, "Labels", "italic", &bFlushConfFileNeeded, FALSE);
	if (bLabelStyleItalic)
		g_iLabelStyle = PANGO_STYLE_ITALIC;
	else
		g_iLabelStyle = PANGO_STYLE_NORMAL;
	
	g_bTextAlwaysHorizontal = cairo_dock_get_boolean_key_value (pKeyFile, "Labels", "always horizontal", &bFlushConfFileNeeded, FALSE);
	
	if (g_cLabelPolice == NULL)
		g_iLabelSize = 0;
	
	if (g_iLabelSize == 0)
	{
		g_free (g_cLabelPolice);
		g_cLabelPolice = NULL;
	}
	
	
	//\___________________ On recupere les parametres du dock en lui-meme.
	g_free (g_cLanguage);
	g_cLanguage = cairo_dock_get_string_key_value (pKeyFile, "Cairo Dock", "language", &bFlushConfFileNeeded, "en");
	
	gchar **cIconsTypesList = cairo_dock_get_string_list_key_value (pKeyFile, "Cairo Dock", "icon's type order", &bFlushConfFileNeeded, &length, NULL);
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
	g_cMainDockDefaultRendererName = cairo_dock_get_string_key_value (pKeyFile, "Cairo Dock", "main dock view", &bFlushConfFileNeeded, CAIRO_DOCK_DEFAULT_RENDERER_NAME);
	
	double fUserValue = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "unfold factor", &bFlushConfFileNeeded, 8.);
	g_fUnfoldAcceleration = 1 - pow (2, - fUserValue);
	
	int iNbSteps = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "grow up steps", &bFlushConfFileNeeded, 8);
	iNbSteps = MAX (iNbSteps, 1);
	g_iGrowUpInterval = MAX (1, CAIRO_DOCK_NB_MAX_ITERATIONS / iNbSteps);
	iNbSteps = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "shrink down steps", &bFlushConfFileNeeded, 10);
	iNbSteps = MAX (iNbSteps, 1);
	g_iShrinkDownInterval = MAX (1, CAIRO_DOCK_NB_MAX_ITERATIONS / iNbSteps);
	g_fMoveUpSpeed = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "move up speed", &bFlushConfFileNeeded, 0.35);
	g_fMoveUpSpeed = MAX (0.01, MIN (g_fMoveUpSpeed, 1));
	g_fMoveDownSpeed = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "move down speed", &bFlushConfFileNeeded, 0.35);
	g_fMoveDownSpeed = MAX (0.01, MIN (g_fMoveDownSpeed, 1));
	
	int iRefreshFrequency = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "refresh frequency", &bFlushConfFileNeeded, 25);
	g_fRefreshInterval = 1000. / iRefreshFrequency;
	
	
	//\___________________ On recupere les parametres propres aux sous-docks.
	g_free (g_cSubDockDefaultRendererName);
	g_cSubDockDefaultRendererName = cairo_dock_get_string_key_value (pKeyFile, "Sub-Docks", "sub-dock view", &bFlushConfFileNeeded, CAIRO_DOCK_DEFAULT_RENDERER_NAME);
	
	g_bSameHorizontality = cairo_dock_get_boolean_key_value (pKeyFile, "Sub-Docks", "same horizontality", &bFlushConfFileNeeded, TRUE);
	
	g_fSubDockSizeRatio = cairo_dock_get_double_key_value (pKeyFile, "Sub-Docks", "relative icon size", &bFlushConfFileNeeded, 0.8);
	
	g_bAnimateSubDock = cairo_dock_get_boolean_key_value (pKeyFile, "Sub-Docks", "animate subdocks", &bFlushConfFileNeeded, TRUE);
	
	g_iLeaveSubDockDelay = cairo_dock_get_integer_key_value (pKeyFile, "Sub-Docks", "leaving delay", &bFlushConfFileNeeded, 250);
	g_iShowSubDockDelay = cairo_dock_get_integer_key_value (pKeyFile, "Sub-Docks", "show delay", &bFlushConfFileNeeded, 300);
	
	
	//\___________________ On recupere les parametres du fond.
	g_iDockRadius = cairo_dock_get_integer_key_value (pKeyFile, "Background", "corner radius", &bFlushConfFileNeeded, 12);

	g_iDockLineWidth = cairo_dock_get_integer_key_value (pKeyFile, "Background", "line width", &bFlushConfFileNeeded, 2);
	
	g_iFrameMargin = cairo_dock_get_integer_key_value (pKeyFile, "Background", "frame margin", &bFlushConfFileNeeded, 2);
	
	double couleur[4] = {0., 0., 0.6, 0.4};
	cairo_dock_get_double_list_key_value (pKeyFile, "Background", "line color", &bFlushConfFileNeeded, g_fLineColor, 4, couleur);
	
	g_bRoundedBottomCorner = cairo_dock_get_boolean_key_value (pKeyFile, "Background", "rounded bottom corner", &bFlushConfFileNeeded, TRUE);
	
	g_iMaxAuthorizedWidth = cairo_dock_get_integer_key_value (pKeyFile, "Background", "max autorized width", &bFlushConfFileNeeded, 0);
	
	double couleur2[4] = {.7, .9, .7, .4};
	cairo_dock_get_double_list_key_value (pKeyFile, "Background", "stripes color bright", &bFlushConfFileNeeded, g_fStripesColorBright, 4, couleur2);
	
	g_free (g_cBackgroundImageFile);
	g_cBackgroundImageFile = cairo_dock_get_string_key_value (pKeyFile, "Background", "background image", &bFlushConfFileNeeded, NULL);
	
	g_fBackgroundImageAlpha = cairo_dock_get_double_key_value (pKeyFile, "Background", "image alpha", &bFlushConfFileNeeded, 0.5);
	
	g_bBackgroundImageRepeat = cairo_dock_get_boolean_key_value (pKeyFile, "Background", "repeat image", &bFlushConfFileNeeded, FALSE);
	
	g_iNbStripes = cairo_dock_get_integer_key_value (pKeyFile, "Background", "number of stripes", &bFlushConfFileNeeded, 10);
	
	g_fStripesWidth = cairo_dock_get_double_key_value (pKeyFile, "Background", "stripes width", &bFlushConfFileNeeded, 0.02);
	if (g_iNbStripes > 0 && g_fStripesWidth > 1. / g_iNbStripes)
	{
		g_print ("Attention : the stripes' width is greater than the space between them.\n");
		g_fStripesWidth = 0.99 / g_iNbStripes;
	}
	
	double couleur3[4] = {.7, .7, 1., .7};
	cairo_dock_get_double_list_key_value (pKeyFile, "Background", "stripes color dark", &bFlushConfFileNeeded, g_fStripesColorDark, 4, couleur3);
	
	g_fStripesAngle = cairo_dock_get_double_key_value (pKeyFile, "Background", "stripes angle", &bFlushConfFileNeeded, 30.);
	
	g_fStripesSpeedFactor = cairo_dock_get_double_key_value (pKeyFile, "Background", "scroll speed factor", &bFlushConfFileNeeded, 1.0);
	g_fStripesSpeedFactor = MIN (1., g_fStripesSpeedFactor);
	
	g_bDecorationsFollowMouse = cairo_dock_get_boolean_key_value (pKeyFile, "Background", "decorations enslaved", &bFlushConfFileNeeded, TRUE);
	
	
	//\___________________ On recupere les parametres des icones.
	double fFieldDepth = cairo_dock_get_double_key_value (pKeyFile, "Icons", "field depth", &bFlushConfFileNeeded, 0.7);
	
	g_bDynamicReflection = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "dynamic reflection", &bFlushConfFileNeeded, FALSE);
	
	g_fAlbedo = cairo_dock_get_double_key_value (pKeyFile, "Icons", "albedo", &bFlushConfFileNeeded, .6);
	
	g_fAmplitude = cairo_dock_get_double_key_value (pKeyFile, "Icons", "amplitude", &bFlushConfFileNeeded, 1.0);
	
	g_iSinusoidWidth = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "sinusoid width", &bFlushConfFileNeeded, 250);
	g_iSinusoidWidth = MAX (1, g_iSinusoidWidth);
	
	g_iIconGap = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "icon gap", &bFlushConfFileNeeded, 0);
	
	g_iStringLineWidth = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "string width", &bFlushConfFileNeeded, 0);
	
	cairo_dock_get_double_list_key_value (pKeyFile, "Icons", "string color", &bFlushConfFileNeeded, g_fStringColor, 4, couleur);
	
	g_fAlphaAtRest = cairo_dock_get_double_key_value (pKeyFile, "Icons", "alpha at rest", &bFlushConfFileNeeded, 1.);
	
	g_iScrollAmount = cairo_dock_get_integer_key_value (pKeyFile, "Icons", "scroll amount", &bFlushConfFileNeeded, FALSE);
	
	g_bResetScrollOnLeave = cairo_dock_get_boolean_key_value (pKeyFile, "Icons", "reset scroll", &bFlushConfFileNeeded, TRUE);
	
	g_fScrollAcceleration = cairo_dock_get_double_key_value (pKeyFile, "Icons", "reset scroll acceleration", &bFlushConfFileNeeded, 0.9);
	
	
	//\___________________ On recupere les parametres des lanceurs.
	gchar *cIconThemeName = cairo_dock_get_string_key_value (pKeyFile, "Launchers", "icon theme", &bFlushConfFileNeeded, NULL);
	if (s_bUserTheme)
		g_object_unref (g_pIconTheme);
	
	if (cIconThemeName != NULL)
	{
		g_pIconTheme = gtk_icon_theme_new ();
		gtk_icon_theme_set_custom_theme (g_pIconTheme, cIconThemeName);
		g_free (cIconThemeName);
		s_bUserTheme = TRUE;
	}
	else
	{
		g_pIconTheme = gtk_icon_theme_get_default ();
		s_bUserTheme = FALSE;
	}
	
	gchar **directoryList = cairo_dock_get_string_list_key_value (pKeyFile, "Launchers", "default icon directory", &bFlushConfFileNeeded, &length, NULL);
	g_strfreev (g_cDefaultIconDirectory);
	
	if (directoryList == NULL)
	{
		g_cDefaultIconDirectory = NULL;
	}
	else
	{
		g_cDefaultIconDirectory = g_new0 (gchar *, length + 1);  // +1 pour le NULL final.
		int i = 0, j = 0;
		while (directoryList[i] != NULL)
		{
			if (directoryList[i][0] == '~')
			{
				g_cDefaultIconDirectory[j] = g_strdup_printf ("%s%s", getenv ("HOME"), directoryList[i]+1);
			}
			else if (directoryList[i][0] == '/')
			{
				g_cDefaultIconDirectory[j] = g_strdup (directoryList[i]);
			}
			else if (strncmp (directoryList[i], "_ThemeDirectory_", 16) == 0)
			{
				g_cDefaultIconDirectory[j] = g_strdup_printf ("%s%s", g_cCurrentLaunchersPath, directoryList[i]+16);
			}
			else
			{
				g_print ("Attention : invalid directory name (%s), will be ignored\n", directoryList[i]);
				j --;
			}
			//g_print ("+ %s\n", g_cDefaultIconDirectory[j]);
			i ++;
			j ++;
		}
	}
	g_strfreev (directoryList);
	
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_integer_key_value (pKeyFile, "Launchers", "max icon size", &bFlushConfFileNeeded, 0);
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_integer_key_value (pKeyFile, "Launchers", "min icon size", &bFlushConfFileNeeded, 0);
	
	g_tAnimationType[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_animation_type_key_value (pKeyFile, "Launchers", "animation type", &bFlushConfFileNeeded, CAIRO_DOCK_BOUNCE);
	
	g_tNbAnimationRounds[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_integer_key_value (pKeyFile, "Launchers", "number of animation rounds", &bFlushConfFileNeeded, 4);
	
	
	//\___________________ On recupere les parametres des aplications.
	g_bShowAppli = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "show applications", &bFlushConfFileNeeded, TRUE);
	
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Applications", "max icon size", &bFlushConfFileNeeded, 0);
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Applications", "min icon size", &bFlushConfFileNeeded, 0);
	
	g_tAnimationType[CAIRO_DOCK_APPLI] = cairo_dock_get_animation_type_key_value (pKeyFile, "Applications", "animation type", &bFlushConfFileNeeded, CAIRO_DOCK_ROTATE);
	
	g_tNbAnimationRounds[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Applications", "number of animation rounds", &bFlushConfFileNeeded, 2);
	
	gboolean bUniquePidOld = g_bUniquePid;
	g_bUniquePid = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "unique PID", &bFlushConfFileNeeded, FALSE);
	
	gboolean bGroupAppliByClassOld = g_bGroupAppliByClass;
	g_bGroupAppliByClass = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "group by class", &bFlushConfFileNeeded, FALSE);
	
	g_iAppliMaxNameLength = cairo_dock_get_integer_key_value (pKeyFile, "Applications", "max name length", &bFlushConfFileNeeded, 15);
	
	g_bMinimizeOnClick = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "minimize on click", &bFlushConfFileNeeded, TRUE);
	
	g_bDemandsAttentionWithDialog = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "demands attention with dialog", &bFlushConfFileNeeded, TRUE);
	g_bDemandsAttentionWithAnimation = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "demands attention with animation", &bFlushConfFileNeeded, FALSE);
	
	g_bAnimateOnActiveWindow = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "animate on active window", &bFlushConfFileNeeded, TRUE);
	
	
	//\___________________ On recupere les parametres des applets.
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET] = cairo_dock_get_integer_key_value (pKeyFile, "Applets", "max icon size", &bFlushConfFileNeeded, 0);
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET] = cairo_dock_get_integer_key_value (pKeyFile, "Applets", "min icon size", &bFlushConfFileNeeded, 0);
	
	g_tAnimationType[CAIRO_DOCK_APPLET] = cairo_dock_get_animation_type_key_value (pKeyFile, "Applets", "animation type", &bFlushConfFileNeeded, CAIRO_DOCK_BLINK);
	
	g_tNbAnimationRounds[CAIRO_DOCK_APPLET] = cairo_dock_get_integer_key_value (pKeyFile, "Applets", "number of animation rounds", &bFlushConfFileNeeded, 1);
	
	gchar **cActiveModuleList = cairo_dock_get_string_list_key_value (pKeyFile, "Applets", "active modules", &bFlushConfFileNeeded, &length, NULL);
	
	
	//\___________________ On recupere les parametres des separateurs.
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12] = cairo_dock_get_integer_key_value (pKeyFile, "Separators", "max icon size", &bFlushConfFileNeeded, 0);
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR23] = g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12];
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12] = cairo_dock_get_integer_key_value (pKeyFile, "Separators", "min icon size", &bFlushConfFileNeeded, 0);
	g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR23] = g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12];
	
	g_free (g_cSeparatorImage);
	g_cSeparatorImage = cairo_dock_get_string_key_value (pKeyFile, "Separators", "separator image", &bFlushConfFileNeeded, NULL);
	
	g_bRevolveSeparator = cairo_dock_get_boolean_key_value (pKeyFile, "Separators", "revolve separator image", &bFlushConfFileNeeded, TRUE);
	
	g_bConstantSeparatorSize = cairo_dock_get_boolean_key_value (pKeyFile, "Separators", "force size", &bFlushConfFileNeeded, TRUE);
	
	
	//\___________________ On recupere les parametres des dialogues.
	gchar *cButtonOkImage = cairo_dock_get_string_key_value (pKeyFile, "Dialogs", "button_ok image", &bFlushConfFileNeeded, NULL);
	gchar *cButtonCancelImage = cairo_dock_get_string_key_value (pKeyFile, "Dialogs", "button_cancel image", &bFlushConfFileNeeded, NULL);
	
	g_iDialogButtonWidth = cairo_dock_get_integer_key_value (pKeyFile, "Dialogs", "button width", &bFlushConfFileNeeded, 48);
	g_iDialogButtonHeight = cairo_dock_get_integer_key_value (pKeyFile, "Dialogs", "button height", &bFlushConfFileNeeded, 32);
	
	g_fDialogAlpha = cairo_dock_get_double_key_value (pKeyFile, "Dialogs", "alpha", &bFlushConfFileNeeded, .6);
	
	g_iDialogIconSize = cairo_dock_get_integer_key_value (pKeyFile, "Dialogs", "icon size", &bFlushConfFileNeeded, 48);
	
	
	//\___________________ On (re)charge tout, car n'importe quel parametre peut avoir change.
	switch (iScreenBorder)
	{
		case CAIRO_DOCK_BOTTOM :
			pDock->bHorizontalDock = CAIRO_DOCK_HORIZONTAL;
			g_bDirectionUp = TRUE;
		break;
		case CAIRO_DOCK_TOP :
			pDock->bHorizontalDock = CAIRO_DOCK_HORIZONTAL;
		g_bDirectionUp = FALSE;
		break;
		case CAIRO_DOCK_LEFT :
			pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
		g_bDirectionUp = FALSE;
		break;
		case CAIRO_DOCK_RIGHT :
			pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
		g_bDirectionUp = TRUE;
		break;
	}
	
	cairo_dock_update_screen_geometry (pDock);
	
	if (g_iMaxAuthorizedWidth == 0)
		g_iMaxAuthorizedWidth = g_iScreenWidth[pDock->bHorizontalDock];
	
	cairo_dock_load_dialog_buttons (pDock, cButtonOkImage, cButtonCancelImage);
	g_free (cButtonOkImage);
	g_free (cButtonCancelImage);
	
	g_fReflectSize = 0;
	guint i;
	for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
	{
		if (g_tMinIconAuthorizedSize[i] > 0)
			g_fReflectSize = MAX (g_fReflectSize, g_tMinIconAuthorizedSize[i]);
	}
	if (g_fReflectSize == 0)  // on n'a pas trouve de taille minimale, on va essayer avec la taille max.
	{
		for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
		{
			if (g_tMaxIconAuthorizedSize[i] > 0)
				g_fReflectSize = MAX (g_fReflectSize, g_tMaxIconAuthorizedSize[i]);
		}
		if (g_fReflectSize > 0)
			g_fReflectSize = MIN (48, g_fReflectSize);
		else
			g_fReflectSize = 48;
	}
	g_fReflectSize *= fFieldDepth;
	g_print ("  g_fReflectSize : %.2f pixels\n", g_fReflectSize);
	
	cairo_dock_remove_all_applets (pDock);  // on est obliges d'arreter tous les applets (c.a.d. les modules ayant une icone dans le dock).
	
	if (bUniquePidOld != g_bUniquePid || bGroupAppliByClassOld != g_bGroupAppliByClass || (cairo_dock_application_manager_is_running () && ! g_bShowAppli))  // on ne veut plus voir les applis, il faut donc les enlever.
	{
		cairo_dock_stop_application_manager (pDock);
	}
	else  // il reste 2 types distincts dans la liste, on reordonne car l'ordre des types a pu changer.
	{
		pDock->icons = g_list_sort (pDock->icons, (GCompareFunc) cairo_dock_compare_icons_order);
	}
	
	g_fBackgroundImageWidth = 1e4;  // inutile de mettre a jour les decorations maintenant.
	g_fBackgroundImageHeight = 1e4;
	if (pDock->icons == NULL)
	{
		pDock->iFlatDockWidth = - g_iIconGap;  // car on ne le connaissais pas encore au moment de la creation du dock.
		cairo_dock_build_docks_tree_with_desktop_files (pDock, g_cCurrentLaunchersPath);
	}
	else
		cairo_dock_reload_buffers_in_all_docks (g_hDocksTable);
	
	
	if (! cairo_dock_application_manager_is_running () && g_bShowAppli)  // maintenant on veut voir les applis !
	{
		cairo_dock_start_application_manager (pDock);
	}
	
	
	cairo_dock_activate_modules_from_list (cActiveModuleList, pDock);
	g_strfreev (cActiveModuleList);
	
	cairo_dock_set_all_views_to_default ();  // met a jour les tailles des docks.
	
	cairo_dock_reserve_space_for_dock (pDock, g_bReserveSpace);
	///cairo_dock_update_dock_size (pDock);
	
	
	cairo_dock_load_visible_zone (pDock, cVisibleZoneImageFile, g_iVisibleZoneWidth, g_iVisibleZoneHeight, g_fVisibleZoneAlpha);
	g_free (cVisibleZoneImageFile);
	
	cairo_dock_load_background_decorations (pDock);
	
	
	pDock->iMouseX = 0;  // on se place hors du dock initialement.
	pDock->iMouseY = 0;
	pDock->calculate_icons (pDock);
	gtk_widget_queue_draw (pDock->pWidget);  //   // le 'gdk_window_move_resize' ci-dessous ne provoquera pas le redessin si la taille n'a pas change.
	
	if (pDock->bAtBottom)
	{
		int iNewWidth, iNewHeight;
		if (g_bAutoHide && pDock->iRefCount == 0)
		{
			cairo_dock_get_window_position_and_geometry_at_balance (pDock, CAIRO_DOCK_MIN_SIZE, &iNewWidth, &iNewHeight);
			pDock->fFoldingFactor = g_fUnfoldAcceleration;
		}
		else
		{
			pDock->fFoldingFactor = 0;
			cairo_dock_get_window_position_and_geometry_at_balance (pDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);
		}
		
		//g_print ("on commence en bas a %dx%d (%d;%d)\n", g_iVisibleZoneWidth, g_iVisibleZoneHeight, pDock->iWindowPositionX, pDock->iWindowPositionY);
		if (pDock->bHorizontalDock)
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionX,
				pDock->iWindowPositionY,
				iNewWidth,
				iNewHeight);
		else
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionY,
				pDock->iWindowPositionX,
				iNewHeight,
				iNewWidth);
	}
	
	//\___________________ On ecrit si necessaire.
	if (! bFlushConfFileNeeded)
		bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile);
	if (bFlushConfFileNeeded)
	{
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, CAIRO_DOCK_SHARE_DATA_DIR);
		
		cairo_dock_update_conf_file_with_available_modules (cConfFilePath);
		cairo_dock_update_conf_file_with_translations (cConfFilePath, CAIRO_DOCK_SHARE_DATA_DIR);
	}
	
	cairo_dock_update_conf_file_with_renderers (cConfFilePath);
	
	//\___________________ On applique les modifs au fichier de conf easy.
	cairo_dock_copy_to_easy_conf_file (pKeyFile, g_cEasyConfFile);
	
	g_key_file_free (pKeyFile);
	
	cairo_dock_mark_theme_as_modified (TRUE);
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
	GtkWidget *pWidget = user_data[9];
	gchar *cTitle = user_data[10];
	int iWindowWidth = GPOINTER_TO_INT (user_data[11]);
	int iWindowHeight = GPOINTER_TO_INT (user_data[12]);
	gchar iIdentifier = GPOINTER_TO_INT (user_data[13]);
	gchar *cButtonConvert = user_data[14];
	gchar *cButtonRevert = user_data[15];
	
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
				g_print ("error while writing to %s\n", cConfFilePath);
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
		g_free (cConfFilePath);
		g_free (cConfFilePath2);
		g_free (cTitle);
		g_free (cButtonConvert);
		g_free (cButtonRevert);
		if (pFreeUserDataFunc != NULL)
			pFreeUserDataFunc (data, NULL);
		g_free (user_data);
	}
	else if (action == GTK_RESPONSE_HELP)
	{
		gtk_widget_destroy (GTK_WIDGET (pDialog));
		g_key_file_free (pKeyFile);
		cairo_dock_free_generated_widget_list (pWidgetList);
		g_free (user_data);
		
		cairo_dock_edit_conf_file_core (pWidget, cConfFilePath2, cTitle, iWindowWidth, iWindowHeight, iIdentifier, NULL, pConfigFunc2, data, pFreeUserDataFunc, pConfigFunc, cConfFilePath, cButtonRevert, cButtonConvert);
	}
}


gboolean cairo_dock_edit_conf_file_core (GtkWidget *pWidget, gchar *cConfFilePath, gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CairoDockConfigFunc pConfigFunc, gpointer data, GFunc pFreeUserDataFunc, CairoDockConfigFunc pConfigFunc2, gchar *cConfFilePath2, gchar *cButtonConvert, gchar *cButtonRevert)
{
	g_print ("%s (%s; %s)\n", __func__, cConfFilePath, cConfFilePath2);
	GSList *pWidgetList = NULL;
	GtkTextBuffer *pTextBuffer = NULL;  // le buffer est lie au widget, donc au pDialog.
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	g_key_file_remove_key (pKeyFile, "Desktop Entry", "X-Ubuntu-Gettext-Domain", NULL);  // salete de traducteur automatique.
	
	GtkWidget *pDialog = cairo_dock_generate_advanced_ihm_from_keyfile (pKeyFile, cTitle, pWidget, &pWidgetList, (pConfigFunc != NULL), iIdentifier, cPresentedGroup, (pConfigFunc2 != NULL), cButtonConvert);
	if (pDialog == NULL || pWidgetList == NULL)
	{
		pDialog = cairo_dock_generate_basic_ihm_from_keyfile (cConfFilePath, cTitle, pWidget, &pTextBuffer, (pConfigFunc != NULL), (pConfigFunc2 != NULL), cButtonConvert);
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
		gpointer *user_data = g_new (gpointer, 16);
		user_data[0] = pKeyFile;
		user_data[1] = pWidgetList;
		user_data[2] = cConfFilePath;
		user_data[3] = pTextBuffer;
		user_data[4] = pConfigFunc;
		user_data[5] = data;
		user_data[6] = pFreeUserDataFunc;
		user_data[7] = cConfFilePath2;
		user_data[8] = pConfigFunc2;
		user_data[9] = pWidget;
		user_data[10] = cTitle;
		user_data[11] = GINT_TO_POINTER (iWindowWidth);
		user_data[12] = GINT_TO_POINTER (iWindowHeight);
		user_data[13] = GINT_TO_POINTER ((int) iIdentifier);
		user_data[14] = cButtonConvert;
		user_data[15] = cButtonRevert;
		
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
					g_print ("error while writing to %s\n", cConfFilePath);
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
		g_free (cConfFilePath);
		g_free (cConfFilePath2);
		g_free (cTitle);
		g_free (cButtonConvert);
		g_free (cButtonRevert);
		return config_ok;
	}
}

gboolean cairo_dock_edit_conf_file_full (GtkWidget *pWidget, gchar *cConfFilePath, gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CairoDockConfigFunc pConfigFunc, gpointer data, GFunc pFreeUserDataFunc, CairoDockConfigFunc pConfigFunc2, gchar *cConfFilePath2, gchar *cButtonConvert, gchar *cButtonRevert)
{
	return cairo_dock_edit_conf_file_core (pWidget, g_strdup (cConfFilePath), g_strdup (cTitle), iWindowWidth, iWindowHeight, iIdentifier, cPresentedGroup, pConfigFunc, data, pFreeUserDataFunc, pConfigFunc2, g_strdup (cConfFilePath2), g_strdup (cButtonConvert), g_strdup (cButtonRevert));
}



void cairo_dock_update_conf_file_with_position (gchar *cConfFilePath, int x, int y)
{
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	g_key_file_set_integer (pKeyFile, "Position", "x gap", x);
	g_key_file_set_integer (pKeyFile, "Position", "y gap", y);
	
	cairo_dock_write_keys_to_file (pKeyFile, g_cConfFile);
	g_key_file_free (pKeyFile);
}

void cairo_dock_update_conf_file_with_translations_full (gchar *cConfFile, gchar *cTranslationsDir, gchar *cGroupName, gchar *cKeyName)
{
	GError *erreur = NULL;
	GHashTable *pTranslationTable = cairo_dock_list_available_translations (cTranslationsDir, "cairo-dock-", &erreur);
	
	cairo_dock_update_conf_file_with_hash_table (cConfFile, pTranslationTable, cGroupName, cKeyName, NULL, (GHFunc) cairo_dock_write_one_name);
	
	g_hash_table_destroy (pTranslationTable);
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
			iDesktopEnv = CAIRO_DOCK_UNKNOWN_ENV;
		}
		else
			iDesktopEnv = CAIRO_DOCK_KDE;
	}
	else
	{
		iDesktopEnv = CAIRO_DOCK_GNOME;
	}
	return iDesktopEnv;
}


static void cairo_dock_copy_value_to_keyfile (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, GKeyFile *pKeyFile2, gchar *cGroupName2, gchar *cKeyName2)
{
	GError *erreur = NULL;
	gchar *cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	//\___________________ On recupere les parametres systeme.
	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "language", pMainKeyFile, "Cairo Dock", "language");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "screen border", pMainKeyFile, "Position", "screen border");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "auto-hide", pMainKeyFile, "Auto-Hide", "auto-hide");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "show applications", pMainKeyFile, "Applications", "show applications");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "active modules", pMainKeyFile, "Applets", "active modules");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "System", "active modules", pMainKeyFile, "Applets", "active modules");
	
	//\___________________ On recupere les parametres de personnalisation.
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "callback image", pMainKeyFile, "Auto-Hide", "background image");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "background image", pMainKeyFile, "Background", "background image");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "separator image", pMainKeyFile, "Separators", "separator image");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "main dock view", pMainKeyFile, "Cairo Dock", "main dock view");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "sub-dock view", pMainKeyFile, "Sub-Docks", "sub-dock view");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Launchers", "min icon size");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Launchers", "max icon size");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Applications", "min icon size");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Applications", "max icon size");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Applets", "max icon size");
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon size", pMainKeyFile, "Launchers", "max icon size");
	
	gboolean bFlushConfFileNeeded;
	double fValue = cairo_dock_get_double_key_value (pKeyFile, "Personnalisation", "zoom", &bFlushConfFileNeeded, 2.0);
	g_key_file_set_double (pMainKeyFile, "Icons", "amplitude", fValue - 1);
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "icon gap", pMainKeyFile, "Icons", "icon gap");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "launcher animation", pMainKeyFile, "Launchers", "animation type");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "appli animation", pMainKeyFile, "Applications", "animation type");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "corner radius", pMainKeyFile, "Background", "corner radius");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "line color", pMainKeyFile, "Background", "line color");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "police", pMainKeyFile, "Labels", "police");
	
	cairo_dock_copy_value_to_keyfile (pKeyFile, "Personnalisation", "font size", pMainKeyFile, "Labels", "size");
	
	g_key_file_free (pKeyFile);
}

void cairo_dock_copy_to_easy_conf_file (GKeyFile *pMainKeyFile, gchar *cEasyConfFilePath)
{
	//\___________________ On recupere le patron.
	gchar *cEasyConfTemplate = cairo_dock_get_translated_conf_file_path (CAIRO_DOCK_EASY_CONF_FILE, CAIRO_DOCK_SHARE_DATA_DIR);
	
	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cEasyConfTemplate, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	g_free (cEasyConfTemplate);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	//\___________________ On ecrit les parametres systeme.
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Cairo Dock", "language", pKeyFile, "System", "language");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Position", "screen border", pKeyFile, "System", "screen border");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Auto-Hide", "auto-hide", pKeyFile, "System", "auto-hide");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Applications", "show applications", pKeyFile, "System", "show applications");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Applets", "active modules", pKeyFile, "System", "active modules");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Applets", "active modules", pKeyFile, "System", "active modules");
	
	//\___________________ On ecrit les parametres de personnalisation.
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Auto-Hide", "background image", pKeyFile, "Personnalisation", "callback image");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Background", "background image", pKeyFile, "Personnalisation", "background image");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Separators", "separator image", pKeyFile, "Personnalisation", "separator image");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Cairo Dock", "main dock view", pKeyFile, "Personnalisation", "main dock view");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Sub-Docks", "sub-dock view", pKeyFile, "Personnalisation", "sub-dock view");
	
	g_key_file_set_integer (pKeyFile, "Personnalisation", "icon size", MAX (g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER], g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI]));
	
	g_key_file_set_double (pKeyFile, "Personnalisation", "zoom", 1 + g_fAmplitude);
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Icons", "icon gap", pKeyFile, "Personnalisation", "icon gap");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Launchers", "animation type", pKeyFile, "Personnalisation", "launcher animation");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Applications", "animation type", pKeyFile, "Personnalisation", "appli animation");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Background", "corner radius", pKeyFile, "Personnalisation", "corner radius");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Background", "line color", pKeyFile, "Personnalisation", "line color");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Labels", "police", pKeyFile, "Personnalisation", "police");
	
	cairo_dock_copy_value_to_keyfile (pMainKeyFile, "Labels", "size", pKeyFile, "Personnalisation", "font size");
	
	//\___________________ On ecrit tout.
	cairo_dock_write_keys_to_file (pKeyFile, cEasyConfFilePath);
	g_key_file_free (pKeyFile);
	
	//\___________________ On complete.
	cairo_dock_update_easy_conf_file_with_available_modules (cEasyConfFilePath);
	cairo_dock_update_easy_conf_file_with_translations (cEasyConfFilePath, CAIRO_DOCK_SHARE_DATA_DIR);
	
	cairo_dock_update_easy_conf_file_with_renderers (cEasyConfFilePath);
}

void cairo_dock_build_easy_conf_file (gchar *cMainConfFilePath, gchar *cEasyConfFilePath)
{
	GKeyFile *pMainKeyFile = g_key_file_new ();
	GError *erreur = NULL;
	g_key_file_load_from_file (pMainKeyFile, cMainConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
		g_print ("Attention : %s\n", erreur->message);
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
