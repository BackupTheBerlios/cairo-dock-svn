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
#include "cairo-dock-applications.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-config.h"

static gchar *s_tAnimationNames[CAIRO_DOCK_NB_ANIMATIONS + 1] = {"bounce", "rotate", "blink", "pulse", "random", NULL};
static gchar * s_cIconTypeNames[(CAIRO_DOCK_NB_TYPES+1)/2] = {"launchers", "applications", "applets"};

extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;
extern gchar *g_cLanguage;
extern gboolean g_bReverseVisibleImage;
extern gboolean g_bReserveSpace;

extern int g_iMaxAuthorizedWidth;
extern int g_iScrollAmount;
extern gboolean g_bResetScrollOnLeave;
extern double g_fScrollAcceleration;
extern gboolean g_bForceLoop;

extern int g_iSinusoidWidth;
extern double g_fAmplitude;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern int g_iIconGap;
extern gboolean g_bAutoHide;
extern double g_fVisibleZoneAlpha;
extern gboolean g_bDirectionUp;
extern gboolean g_bSameHorizontality;
extern double g_fSubDockSizeRatio;
extern gboolean bShowSubDockOnMouseOver;

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
extern gchar *g_cCurrentThemePath;
extern gchar *g_cConfFile;

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_fBackgroundImageWidth;
extern int g_fBackgroundImageHeight;

extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bRoundedBottomCorner;
extern double g_fLineColor[4];
extern gint g_iStringLineWidth;
extern double g_fStringColor[4];

extern gboolean g_bBackgroundImageRepeat;
extern double g_fBackgroundImageAlpha;
extern gchar *g_cBackgroundImageFile;

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
extern double g_fGrowUpFactor;
extern double g_fShrinkDownFactor;
extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;
extern double g_fRefreshInterval;

extern gboolean g_bShowAppli;
extern gboolean g_bUniquePid;
extern int g_iAppliMaxNameLength;
extern int g_iSidUpdateAppliList;

extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];

extern gchar *g_cSeparatorImage;
extern gboolean g_bRevolveSeparator;

extern GHashTable *g_hModuleTable;


int cairo_dock_get_number_from_name (gchar *cName, gchar **tNamesList)
{
	int i = 0;
	while (tNamesList[i] != NULL)
	{
		if (strcmp (cName,tNamesList[i]) == 0)
			return i;
		i ++;
	}
	return 0;
}


static gboolean cairo_dock_get_boolean_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gboolean bDefaultValue)
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
		if (erreur != NULL)
		{
			g_error_free (erreur);
			bValue = bDefaultValue;
		}
		g_free (cGroupNameUpperCase);
		
		g_key_file_set_boolean (pKeyFile, cGroupName, cKeyName, bValue);
		*bFlushConfFileNeeded = TRUE;
	}
	return bValue;
}
static int cairo_dock_get_integer_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int iDefaultValue)
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
			iValue = iDefaultValue;
		}
		g_free (cGroupNameUpperCase);
		
		g_key_file_set_integer (pKeyFile, cGroupName, cKeyName, iValue);
		*bFlushConfFileNeeded = TRUE;
	}
	return iValue;
}
static double cairo_dock_get_double_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double fDefaultValue)
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
			fValue = fDefaultValue;
		}
		g_free (cGroupNameUpperCase);
		
		g_key_file_set_double (pKeyFile, cGroupName, cKeyName, fValue);
		*bFlushConfFileNeeded = TRUE;
	}
	return fValue;
}
static gchar *cairo_dock_get_string_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gchar *cDefaultValue)
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
			cValue = g_strdup (cDefaultValue);
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
static void cairo_dock_get_integer_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int *iValueBuffer, int iNbElements, int *iDefaultValues)
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
static void cairo_dock_get_double_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double *fValueBuffer, int iNbElements, double *fDefaultValues)
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
static gchar **cairo_dock_get_string_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gsize *length, gchar *cDefaultValues)
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
			g_key_file_set_string_list (pKeyFile, cGroupName, cKeyName, cValuesList, *length);
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



void cairo_dock_read_conf_file (gchar *conf_file, CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	GError *erreur = NULL;
	gsize length;
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.

	
	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, conf_file, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	//\___________________ On recupere la position du dock.
	pDock->iGapX = cairo_dock_get_integer_key_value (pKeyFile, "Position", "x gap", &bFlushConfFileNeeded, 0);
	pDock->iGapY = cairo_dock_get_integer_key_value (pKeyFile, "Position", "y gap", &bFlushConfFileNeeded, 0);
	
	gchar *cScreenBorder = cairo_dock_get_string_key_value (pKeyFile, "Position", "screen border", &bFlushConfFileNeeded, "bottom");
	
	pDock->fAlign = cairo_dock_get_double_key_value (pKeyFile, "Position", "alignment", &bFlushConfFileNeeded, 0.5);
	
	g_bReserveSpace = cairo_dock_get_boolean_key_value (pKeyFile, "Position", "reserve space", &bFlushConfFileNeeded, FALSE);
	
	
	//\___________________ On recupere les parametres de la zone visible.
	gchar *cVisibleZoneImageFile = cairo_dock_get_string_key_value (pKeyFile, "Auto-Hide", "background image", &bFlushConfFileNeeded, NULL);
	
	g_iVisibleZoneWidth = cairo_dock_get_integer_key_value (pKeyFile, "Auto-Hide", "zone width", &bFlushConfFileNeeded, 150);
	g_iVisibleZoneHeight = cairo_dock_get_integer_key_value (pKeyFile, "Auto-Hide", "zone height", &bFlushConfFileNeeded, 25);
	
	g_fVisibleZoneAlpha = cairo_dock_get_double_key_value (pKeyFile, "Auto-Hide", "alpha", &bFlushConfFileNeeded, 0.5);
	
	g_bAutoHide = cairo_dock_get_boolean_key_value (pKeyFile, "Auto-Hide", "auto-hide", &bFlushConfFileNeeded, FALSE);
	
	g_bReverseVisibleImage = cairo_dock_get_boolean_key_value (pKeyFile, "Auto-Hide", "reverse visible image", &bFlushConfFileNeeded, TRUE);
	
	
	//\___________________ On recupere les parametres des etiquettes.
	g_bLabelForPointedIconOnly = cairo_dock_get_boolean_key_value (pKeyFile, "Labels", "pointed icon only", &bFlushConfFileNeeded, FALSE);
	
	g_fLabelAlphaThreshold = cairo_dock_get_double_key_value (pKeyFile, "Labels", "alpha threshold", &bFlushConfFileNeeded, 10.);
	
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
	gchar *cPreviousLanguage = g_cLanguage;
	g_cLanguage = cairo_dock_get_string_key_value (pKeyFile, "Cairo Dock", "language", &bFlushConfFileNeeded, "en");
	
	gchar **cIconsTypesList = cairo_dock_get_string_list_key_value (pKeyFile, "Cairo Dock", "icon's type order", &bFlushConfFileNeeded, &length, NULL);
	if (cIconsTypesList != NULL && length > 0)
	{
		int i, j;
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
	
	g_iMaxAuthorizedWidth = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "max autorized width", &bFlushConfFileNeeded, 0);
	
	g_bForceLoop = cairo_dock_get_boolean_key_value (pKeyFile, "Cairo Dock", "force loop", &bFlushConfFileNeeded, FALSE);
	
	g_iScrollAmount = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "scroll amount", &bFlushConfFileNeeded, FALSE);
	
	g_bResetScrollOnLeave = cairo_dock_get_boolean_key_value (pKeyFile, "Cairo Dock", "reset scroll", &bFlushConfFileNeeded, TRUE);
	
	g_fScrollAcceleration = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "reset scroll acceleration", &bFlushConfFileNeeded, 0.9);
	
	g_fAmplitude = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "amplitude", &bFlushConfFileNeeded, 1.0);
	
	g_iSinusoidWidth = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "sinusoid width", &bFlushConfFileNeeded, 250);
	g_iSinusoidWidth = MAX (1, g_iSinusoidWidth);
	
	g_iDockRadius = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "corner radius", &bFlushConfFileNeeded, 250);

	g_iDockLineWidth = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "line width", &bFlushConfFileNeeded, 1);
	
	double couleur[4] = {0., 0., 0.6, 0.4};
	cairo_dock_get_double_list_key_value (pKeyFile, "Cairo Dock", "line color", &bFlushConfFileNeeded, g_fLineColor, 4, couleur);
	
	g_bRoundedBottomCorner = cairo_dock_get_boolean_key_value (pKeyFile, "Cairo Dock", "rounded bottom corner", &bFlushConfFileNeeded, TRUE);
	
	g_iStringLineWidth = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "string width", &bFlushConfFileNeeded, 0);
	
	cairo_dock_get_double_list_key_value (pKeyFile, "Cairo Dock", "string color", &bFlushConfFileNeeded, g_fStringColor, 4, couleur);
	
	double fUserValue = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "unfold factor", &bFlushConfFileNeeded, 8.);
	g_fUnfoldAcceleration = 1 - pow (2, - fUserValue);
	
	g_fGrowUpFactor = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "grow up factor", &bFlushConfFileNeeded, 1.45);
	g_fGrowUpFactor = MAX (1.02, g_fGrowUpFactor);
	g_fShrinkDownFactor = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "shrink down factor", &bFlushConfFileNeeded, 0.65);
	g_fShrinkDownFactor = MAX (0, MIN (g_fShrinkDownFactor, 0.99));
	g_fMoveUpSpeed = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "move up speed", &bFlushConfFileNeeded, 0.35);
	g_fMoveUpSpeed = MAX (0.01, MIN (g_fMoveUpSpeed, 1));
	g_fMoveDownSpeed = cairo_dock_get_double_key_value (pKeyFile, "Cairo Dock", "move down speed", &bFlushConfFileNeeded, 0.35);
	g_fMoveDownSpeed = MAX (0.01, MIN (g_fMoveDownSpeed, 1));
	
	int iRefreshFrequency = cairo_dock_get_integer_key_value (pKeyFile, "Cairo Dock", "refresh frequency", &bFlushConfFileNeeded, 25);
	g_fRefreshInterval = 1000. / iRefreshFrequency;
	
	
	//\___________________ On recupere les parametres propres aux sous-docks.
	g_bSameHorizontality = cairo_dock_get_boolean_key_value (pKeyFile, "Sub-Docks", "same horizontality", &bFlushConfFileNeeded, TRUE);
	
	g_fSubDockSizeRatio = cairo_dock_get_double_key_value (pKeyFile, "Sub-Docks", "relative icon size", &bFlushConfFileNeeded, 0.8);
	
	bShowSubDockOnMouseOver = cairo_dock_get_boolean_key_value (pKeyFile, "Sub-Docks", "on mouse over", &bFlushConfFileNeeded, TRUE);
	
	//\___________________ On recupere les parametres du fond.
	g_fStripesSpeedFactor = cairo_dock_get_double_key_value (pKeyFile, "Background", "scroll speed factor", &bFlushConfFileNeeded, 1.0);
	g_fStripesSpeedFactor = MIN (1., g_fStripesSpeedFactor);
	
	double couleur2[4] = {.7, .9, .7, .4};
	cairo_dock_get_double_list_key_value (pKeyFile, "Background", "stripes color bright", &bFlushConfFileNeeded, g_fStripesColorBright, 4, couleur2);
	
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
				g_cDefaultIconDirectory[j] = g_strdup_printf ("%s%s", g_cCurrentThemePath, directoryList[i]+16);
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
	
	gchar *cAnimationName;
	cAnimationName = cairo_dock_get_string_key_value (pKeyFile, "Launchers", "animation type", &bFlushConfFileNeeded, s_tAnimationNames[0]);
	g_tAnimationType[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_number_from_name (cAnimationName, s_tAnimationNames);
	g_free (cAnimationName);
	
	g_tNbAnimationRounds[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_integer_key_value (pKeyFile, "Launchers", "number of animation rounds", &bFlushConfFileNeeded, 4);
	
	
	//\___________________ On recupere les parametres des aplications.
	g_bShowAppli = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "show applications", &bFlushConfFileNeeded, TRUE);
	
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Applications", "max icon size", &bFlushConfFileNeeded, 0);
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Applications", "min icon size", &bFlushConfFileNeeded, 0);
	
	cAnimationName = cairo_dock_get_string_key_value (pKeyFile, "Applications", "animation type", &bFlushConfFileNeeded, s_tAnimationNames[0]);
	g_tAnimationType[CAIRO_DOCK_APPLI] = cairo_dock_get_number_from_name (cAnimationName, s_tAnimationNames);
	g_free (cAnimationName);
	
	g_tNbAnimationRounds[CAIRO_DOCK_APPLI] = cairo_dock_get_integer_key_value (pKeyFile, "Applications", "number of animation rounds", &bFlushConfFileNeeded, 2);
	
	gboolean bUniquePidOld = g_bUniquePid;
	g_bUniquePid = cairo_dock_get_boolean_key_value (pKeyFile, "Applications", "unique PID", &bFlushConfFileNeeded, 2);
	
	g_iAppliMaxNameLength = cairo_dock_get_integer_key_value (pKeyFile, "Applications", "max name length", &bFlushConfFileNeeded, 15);
	
	
	//\___________________ On recupere les parametres des applets.
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET] = cairo_dock_get_integer_key_value (pKeyFile, "Applets", "max icon size", &bFlushConfFileNeeded, 0);
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET] = cairo_dock_get_integer_key_value (pKeyFile, "Applets", "min icon size", &bFlushConfFileNeeded, 0);
	
	cAnimationName = cairo_dock_get_string_key_value (pKeyFile, "Applets", "animation type", &bFlushConfFileNeeded, s_tAnimationNames[0]);
	g_tAnimationType[CAIRO_DOCK_APPLET] = cairo_dock_get_number_from_name (cAnimationName, s_tAnimationNames);
	g_free (cAnimationName);
	
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
	
	//\___________________ On (re)charge tout, car n'importe quel parametre peut avoir change.
	if (strcmp (cScreenBorder, "bottom") == 0)
	{
		pDock->bHorizontalDock = CAIRO_DOCK_HORIZONTAL;
		g_bDirectionUp = TRUE;
	}
	else if (strcmp (cScreenBorder, "top") == 0)
	{
		pDock->bHorizontalDock = CAIRO_DOCK_HORIZONTAL;
		g_bDirectionUp = FALSE;
	}
	else if (strcmp (cScreenBorder, "right") == 0)
	{
		pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
		g_bDirectionUp = TRUE;
	}
	else if (strcmp (cScreenBorder, "left") == 0)
	{
		pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
		g_bDirectionUp = FALSE;
	}
	g_free (cScreenBorder);
	
	GdkScreen *gdkscreen = gtk_window_get_screen (GTK_WINDOW (pDock->pWidget));  // on le fait ici, ca permet de remettre a jour le dock en le reconfigurant si l'on a change la resolution de l'ecran.
	g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] = gdk_screen_get_width (gdkscreen);
	g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] = gdk_screen_get_height (gdkscreen);
	g_iScreenWidth[CAIRO_DOCK_VERTICAL] = g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
	g_iScreenHeight[CAIRO_DOCK_VERTICAL] = g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
	
	if (g_iMaxAuthorizedWidth == 0)
		g_iMaxAuthorizedWidth = g_iScreenWidth[pDock->bHorizontalDock];
	
	
	cairo_dock_remove_all_applets (pDock);  // on est obliges d'arreter tous les applets.
	
	if (bUniquePidOld != g_bUniquePid || (g_iSidUpdateAppliList != 0 && ! g_bShowAppli))  // on ne veut plus voir les applis, il faut donc les enlever.
	{
		cairo_dock_remove_all_applis (pDock);
	}
	else  // il reste 2 types distincts dans la liste, on reordonne car l'ordre des types a pu changer.
	{
		pDock->icons = g_list_sort (pDock->icons, (GCompareFunc) cairo_dock_compare_icons_order);
	}
	
	g_fBackgroundImageWidth = 1e4;  // inutile de mettre a jour les decorations maintenant.
	g_fBackgroundImageHeight = 1e4;
	if (pDock->icons == NULL)
		cairo_dock_build_docks_tree_with_desktop_files (pDock, g_cCurrentThemePath);
	else
		cairo_dock_reload_buffers_in_all_dock (g_hDocksTable);
	
	
	if (g_iSidUpdateAppliList == 0 && g_bShowAppli)  // maintenant on veut voir les applis !
	{
		cairo_dock_show_all_applis (pDock);
	}
	
	
	cairo_dock_activate_modules_from_list (cActiveModuleList, g_hModuleTable, pDock);
	g_strfreev (cActiveModuleList);
	
	cairo_dock_reserve_space_for_dock (pDock, g_bReserveSpace);
	cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
	
	
	cairo_dock_load_visible_zone (pDock, cVisibleZoneImageFile, g_iVisibleZoneWidth, g_iVisibleZoneHeight, g_fVisibleZoneAlpha);
	g_free (cVisibleZoneImageFile);
	
	cairo_dock_load_background_decorations (pDock);
	
	
	cairo_dock_calculate_icons (pDock, 0, 0);
	gtk_widget_queue_draw (pDock->pWidget);
	
	
	if (pDock->bAtBottom)
	{
		int iNewWidth, iNewHeight;
		if (g_bAutoHide && pDock->iRefCount == 0)
		{
			cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_MIN_SIZE, &iNewWidth, &iNewHeight);
			pDock->fLateralFactor = g_fUnfoldAcceleration;
		}
		else
		{
			cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);
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
	
	if ((cPreviousLanguage != NULL && g_cLanguage != NULL && strcmp (cPreviousLanguage, g_cLanguage) != 0) || bFlushConfFileNeeded)
	{
		gchar *cCommand = g_strdup_printf ("/bin/cp %s/cairo-dock-%s.conf %s", CAIRO_DOCK_SHARE_DATA_DIR, g_cLanguage, conf_file);
		system (cCommand);
		g_free (cCommand);
		
		cairo_dock_replace_values_in_conf_file (conf_file, pKeyFile, TRUE);
		
		cairo_dock_update_conf_file_with_modules (conf_file, g_hModuleTable);
		cairo_dock_update_conf_file_with_translations (conf_file, CAIRO_DOCK_SHARE_DATA_DIR);
	}
	g_free (cPreviousLanguage);
	
	g_key_file_free (pKeyFile);
}


static void _cairo_dock_user_action_on_config (GtkDialog *pDialog, gint action, gpointer *user_data)
{
	GKeyFile *pKeyFile = user_data[0];
	GSList *pWidgetList = user_data[1];
	gchar *conf_file = user_data[2];
	GtkTextBuffer *pTextBuffer = user_data[3];
	CairoDockConfigFunc pConfigFunc = user_data[4];
	gpointer data = user_data[5];
	GFunc pFreeUserDataFunc = user_data[6];
	
	gtk_window_set_modal (GTK_WINDOW (pDialog), TRUE);  // pour prevenir tout interaction avec l'appli pendant sa re-configuration.
	if (action == GTK_RESPONSE_ACCEPT || action == GTK_RESPONSE_APPLY)
	{
		if (pWidgetList != NULL)
		{
			cairo_dock_update_keyfile_from_widget_list (pKeyFile, pWidgetList);
			cairo_dock_write_keys_to_file (pKeyFile, conf_file);
		}
		else
		{
			GtkTextIter start, end;
			gtk_text_buffer_get_iter_at_offset (pTextBuffer, &start, 0);
			gtk_text_buffer_get_iter_at_offset (pTextBuffer, &end, -1);
			
			gchar *cConfiguration = gtk_text_buffer_get_text (pTextBuffer, &start, &end, FALSE);
			
			gboolean write_ok = g_file_set_contents (conf_file, cConfiguration, -1, NULL);
			g_free (cConfiguration);
			if (! write_ok)
				g_print ("error while writing to %s\n", conf_file);
		}
		
		if (pConfigFunc != NULL)
			pConfigFunc (conf_file, data);
	}
	gtk_window_set_modal (GTK_WINDOW (pDialog), FALSE);
	
	if (action != GTK_RESPONSE_APPLY)
	{
		gtk_widget_destroy (GTK_WIDGET (pDialog));
		g_key_file_free (pKeyFile);
		cairo_dock_free_generated_widget_list (pWidgetList);
		g_free (conf_file);
		if (pFreeUserDataFunc != NULL)
			pFreeUserDataFunc (data, NULL);
		g_free (user_data);
	}
}
gboolean cairo_dock_edit_conf_file (GtkWidget *pWidget, gchar *conf_file, gchar *cTitle, int iWindowWidth, int iWindowHeight, gboolean bFullConfig, CairoDockConfigFunc pConfigFunc, gpointer data, GFunc pFreeUserDataFunc)
{
	GSList *pWidgetList = NULL;
	GtkTextBuffer *pTextBuffer = NULL;  // le buffer est lie au widget, donc au pDialog.
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, conf_file, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_key_file_remove_key (pKeyFile, "Desktop Entry", "X-Ubuntu-Gettext-Domain", NULL);
	
	GtkWidget *pDialog = cairo_dock_generate_advanced_ihm_from_keyfile (pKeyFile, cTitle, pWidget, &pWidgetList, (pConfigFunc != NULL), bFullConfig);
	if (pDialog == NULL || pWidgetList == NULL)
	{
		pDialog = cairo_dock_generate_basic_ihm_from_keyfile (conf_file, cTitle, pWidget, &pTextBuffer, (pConfigFunc != NULL));
	}
	g_return_val_if_fail (pDialog != NULL, FALSE);
	
	if (iWindowWidth != 0 && iWindowHeight != 0)
		gtk_window_resize (GTK_WINDOW (pDialog), iWindowWidth, iWindowHeight);
	gint iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pDialog), &iWidth, &iHeight);
	iWidth = MAX (iWidth, iWindowWidth);  // car la taille n'est pas encore effective.
	iHeight = MAX (iWidth, iWindowHeight);
	gtk_window_move (GTK_WINDOW (pDialog), (g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] - iWidth) / 2, (g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] - iHeight) / 2);
	
	if (pConfigFunc != NULL)  // alors on autorise la modification a la volee, avec un bouton "Appliquer". La fenetre doit donc laisser l'appli se derouler.
	{
		gpointer *user_data = g_new (gpointer, 7);
		user_data[0] = pKeyFile;
		user_data[1] = pWidgetList;
		user_data[2] = g_strdup (conf_file);
		user_data[3] = pTextBuffer;
		user_data[4] = pConfigFunc;
		user_data[5] = data;
		user_data[6] = pFreeUserDataFunc;
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
				cairo_dock_write_keys_to_file (pKeyFile, conf_file);
			}
			else
			{
				GtkTextIter start, end;
				gtk_text_buffer_get_iter_at_offset (pTextBuffer, &start, 0);
				gtk_text_buffer_get_iter_at_offset (pTextBuffer, &end, -1);
				
				gchar *cConfiguration = gtk_text_buffer_get_text (pTextBuffer, &start, &end, FALSE);
				
				gboolean write_ok = g_file_set_contents (conf_file, cConfiguration, -1, NULL);
				g_free (cConfiguration);
				if (! write_ok)
				{
					g_print ("error while writing to %s\n", conf_file);
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
		return config_ok;
	}
}



void cairo_dock_write_keys_to_file (GKeyFile *key_file, gchar *conf_file)
{
	GError *erreur = NULL;
	
	gchar *cDirectory = g_path_get_dirname (conf_file);
	if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE))
	{
		g_mkdir_with_parents (cDirectory, 7*8*8+7*8+5);
	}
	g_free (cDirectory);
	
	
	gsize length;
	gchar *new_conf_file = g_key_file_to_data (key_file, &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Error while fetching data : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	g_file_set_contents (conf_file, new_conf_file, length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Error while writing data : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
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


static void _cairo_dock_write_one_name (gchar *cName, gpointer value, GString *pString)
{
	g_string_append_printf (pString, "%s;", cName);
}
void cairo_dock_update_conf_file_with_hash_table (gchar *cConfFile, GHashTable *pModuleTable, gchar *cGroupName, gchar *cKeyName, int iNbAvailableChoices, gchar *cNewUsefullComment, gboolean bAllowNewChoice)
{
	//g_print ("%s (%s)\n", __func__, cConfFile);
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
	
	gchar *cUsefullComment = NULL;
	if (cNewUsefullComment == NULL)
	{
		gchar *cOldComment = g_key_file_get_comment (pKeyFile, cGroupName, cKeyName, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		//g_print ("cOldComment : %s\n", cOldComment);
		if (cOldComment != NULL)
		{
			cOldComment[strlen (cOldComment) - 1] = '\0';
			cUsefullComment = strchr (cOldComment, ']');
			if (cUsefullComment == NULL)
			{
				cUsefullComment = cOldComment;
				while (*cUsefullComment == ' ')
					cUsefullComment ++;
			}
			if (*cUsefullComment != '\0')
				cUsefullComment ++;  // on saute le caractere de type ou le crochet.
			else
				cUsefullComment = NULL;
			if (cUsefullComment != NULL)
				cUsefullComment = g_strdup (cUsefullComment);
			g_free (cOldComment);
		}
	}
	else
	{
		cUsefullComment = g_strdup (cNewUsefullComment);
	}
	
	GString *cComment = g_string_new ("");
	g_string_printf (cComment, "%c%d[", (bAllowNewChoice ? 'E' : 's'), iNbAvailableChoices);
	g_hash_table_foreach (pModuleTable, (GHFunc) _cairo_dock_write_one_name, cComment);
	if (cComment->str[cComment->len-1] == ';')  // peut etre faux si aucune valeur n'a ete ecrite.
		cComment->len --;
	g_string_append_printf (cComment, "] %s", (cUsefullComment != NULL ? cUsefullComment : ""));
	
	g_key_file_set_comment (pKeyFile, cGroupName, cKeyName, cComment->str, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_string_free (cComment, TRUE);
	
	cairo_dock_write_keys_to_file (pKeyFile, cConfFile);
	g_key_file_free (pKeyFile);
	g_free (cUsefullComment);
}

void cairo_dock_update_conf_file_with_modules (gchar *cConfFile, GHashTable *pModuleTable)
{
	cairo_dock_update_conf_file_with_hash_table (cConfFile, pModuleTable, "Applets", "active modules", 99, NULL, FALSE);  // "List of active plug-ins (applets and others)."
}

void cairo_dock_update_conf_file_with_translations (gchar *cConfFile, gchar *cTranslationsDir)
{
	GError *erreur = NULL;
	GHashTable *pTranslationTable = cairo_dock_list_available_translations (cTranslationsDir, "cairo-dock-", &erreur);
	
	cairo_dock_update_conf_file_with_hash_table (cConfFile, pTranslationTable, "Cairo Dock", "language", 1, NULL, FALSE);
	
	g_hash_table_destroy (pTranslationTable);
}


static void _cairo_dock_write_one_module_name_if_active (gchar *cModuleName, CairoDockModule *pModule, GSList **pListeModule)
{
	if (pModule->bActive)
	{
		if (g_slist_find (*pListeModule, cModuleName) == NULL)
			*pListeModule = g_slist_prepend (*pListeModule, cModuleName);
	}
}
void cairo_dock_update_conf_file_with_active_modules (gchar *cConfFile, GList *pIconList, GHashTable *pModuleTable)
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
	
	g_hash_table_foreach (pModuleTable, (GHFunc) _cairo_dock_write_one_module_name_if_active, &pListeModule);
	
	
	GSList *pSList;
	GString *cActiveModules = g_string_new ("");
	for (pSList = pListeModule; pSList != NULL; pSList = pSList->next)
	{
		g_string_append_printf (cActiveModules, "%s;", (gchar *) pSList->data);
	}
	g_slist_free (pListeModule);
	
	g_key_file_set_string (pKeyFile, "Applets", "active modules", cActiveModules->str);
	
	cairo_dock_write_keys_to_file (pKeyFile, cConfFile);
	
	g_string_free (cActiveModules, TRUE);
	g_key_file_free (pKeyFile);
}



void cairo_dock_apply_translation_on_conf_file (gchar *cConfFilePath, gchar *cTranslatedConfFilePath)
{
	GKeyFile *pConfKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pConfKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	
	GKeyFile *pTranslatedKeyFile = g_key_file_new ();
	
	g_key_file_load_from_file (pTranslatedKeyFile, cTranslatedConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		g_key_file_free (pConfKeyFile);
		return ;
	}
	
	cairo_dock_replace_comments (pConfKeyFile, pTranslatedKeyFile);
	
	cairo_dock_write_keys_to_file (pConfKeyFile, cConfFilePath);
	
	g_key_file_free (pConfKeyFile);
	g_key_file_free (pTranslatedKeyFile);
}

void cairo_dock_replace_values_in_conf_file (gchar *cConfFilePath, GKeyFile *pValidKeyFile, gboolean bUseFileKeys)
{
	GKeyFile *pConfKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pConfKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	cairo_dock_replace_key_values (pConfKeyFile, pValidKeyFile, bUseFileKeys);
	
	cairo_dock_write_keys_to_file (pConfKeyFile, cConfFilePath);
	
	g_key_file_free (pConfKeyFile);
}


GHashTable *cairo_dock_list_available_translations (gchar *cTranslationsDir, gchar *cFilePrefix, GError **erreur)
{
	g_return_val_if_fail (cFilePrefix != NULL, NULL);
	GError *tmp_erreur = NULL;
	GDir *dir = g_dir_open (cTranslationsDir, 0, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	GHashTable *pTranslationTable = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
	
	int iPrefixLength = strlen (cFilePrefix);
	const gchar* cFileName;
	gchar *cLanguage;
	gchar *cFilePath;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		if (g_str_has_suffix (cFileName, ".conf") && strncmp (cFileName, cFilePrefix, iPrefixLength) == 0)
		{
			cFilePath = g_strdup_printf ("%s/%s", cTranslationsDir, cFileName);
			
			cLanguage = g_strdup (cFileName + iPrefixLength);
			cLanguage[strlen (cLanguage) - 5] = '\0';
			
			g_hash_table_insert (pTranslationTable, cLanguage, cFilePath);
		}
	}
	while (1);
	g_dir_close (dir);
	
	return pTranslationTable;
}
