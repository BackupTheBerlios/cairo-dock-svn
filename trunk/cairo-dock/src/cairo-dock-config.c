/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


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
#include "cairo-dock-config.h"


static gchar *s_tAnimationNames[CAIRO_DOCK_NB_ANIMATIONS + 1] = {"bounce", "rotate", "blink", "random", NULL};

extern GList* icons;

extern int g_iSinusoidWidth;
extern double g_fAmplitude;

extern gint g_iWindowPositionX;
extern gint g_iWindowPositionY;
extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern int g_iIconGap;
extern gboolean g_bAutoHide;
extern double g_fVisibleZoneAlpha;
extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern int g_iLabelWeight;
extern int g_iLabelStyle;
extern gboolean g_bLabelForPointedIconOnly;

extern gchar **g_cDefaultIconDirectory;
extern gchar *g_cCairoDockDataDir;
extern gchar *g_cConfFile;

extern int g_iMaxDockWidth;
extern int g_iMaxDockHeight;
extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_iGapX;
extern int g_iGapY;
extern gchar *g_cCairoDockBackgroundFileName;
extern int g_iMaxIconHeight;
extern int g_iMinDockWidth;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bRoundedBottomCorner;
extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];
extern double g_fLineColor[4];
extern int g_iNbStripes;
extern double g_fStripesWidth;
extern gchar *g_cDefaultFileBrowser;

extern gboolean g_bAtBottom;
extern int g_iScreenWidth;
extern int g_iScreenHeight;

extern double g_fGrowUpFactor;
extern double g_fShrinkDownFactor;
extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;

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
extern GHashTable *g_hModuleTable;

#ifdef HAVE_GLITZ
extern gboolean g_bUseGlitz;
#endif // HAVE_GLITZ


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

void cairo_dock_read_conf_file (GtkWidget *pWidget, gchar *conf_file)
{
	//g_print ("%s ()\n", __func__);
	GError *erreur = NULL;
	gsize length;
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.

	
	//\___________________ On ouvre le fichier de conf.
	GKeyFile *fconf = g_key_file_new ();
	
	g_key_file_load_from_file (fconf, conf_file, G_KEY_FILE_KEEP_COMMENTS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	//\___________________ On recupere la position du dock.
	g_iGapX = g_key_file_get_integer (fconf, "POSITION", "x gap", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iGapX = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "POSITION", "x gap", g_iGapX);
		bFlushConfFileNeeded = TRUE;
	}
	g_iGapY = g_key_file_get_integer (fconf, "POSITION", "y gap", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iGapY = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "POSITION", "y gap", g_iGapY);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_bDirectionUp = g_key_file_get_boolean (fconf, "POSITION", "unroll upward", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_bDirectionUp = TRUE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "POSITION", "unroll upward", g_bDirectionUp);
		bFlushConfFileNeeded = TRUE;
	}
	
	
	//\___________________ On recupere les parametres de la zone visible.
	g_cCairoDockBackgroundFileName = g_key_file_get_string (fconf, "ZONE", "background image", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_cCairoDockBackgroundFileName = NULL;
		g_key_file_set_string (fconf, "ZONE", "background image", "");
		bFlushConfFileNeeded = TRUE;
	}
	else if (g_cCairoDockBackgroundFileName != NULL && strcmp (g_cCairoDockBackgroundFileName, "") == 0)
	{
		g_free (g_cCairoDockBackgroundFileName);
		g_cCairoDockBackgroundFileName = NULL;
	}
	
	g_iVisibleZoneWidth = g_key_file_get_integer (fconf, "ZONE", "zone width", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iVisibleZoneWidth = 150;  // valeur par defaut.
		g_key_file_set_integer (fconf, "ZONE", "zone width", g_iVisibleZoneWidth);
		bFlushConfFileNeeded = TRUE;
	}
	g_iVisibleZoneHeight = g_key_file_get_integer (fconf, "ZONE", "zone height", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iVisibleZoneHeight = 25;  // valeur par defaut.
		g_key_file_set_integer (fconf, "ZONE", "zone height", g_iVisibleZoneHeight);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_fVisibleZoneAlpha = g_key_file_get_double (fconf, "ZONE", "alpha", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fVisibleZoneAlpha = 0.5;  // valeur par defaut.
		g_key_file_set_double (fconf, "ZONE", "alpha", g_fVisibleZoneAlpha);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_bAutoHide = g_key_file_get_boolean (fconf, "ZONE", "auto-hide", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_bAutoHide = TRUE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "ZONE", "auto-hide", g_bAutoHide);
		bFlushConfFileNeeded = TRUE;
	}
	
	
	//\___________________ On recupere les parametres des etiquettes.
	g_bUseText = g_key_file_get_boolean (fconf, "LABELS", "with labels", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_bUseText = TRUE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "LABELS", "with labels", g_bUseText);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_cLabelPolice = g_key_file_get_string (fconf, "LABELS", "police", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_cLabelPolice = g_strdup ("sans");
		g_key_file_set_string (fconf, "LABELS", "police", g_cLabelPolice);
		bFlushConfFileNeeded = TRUE;
	}
	else if (g_cLabelPolice != NULL && strcmp (g_cLabelPolice, "") == 0)
	{
		g_free (g_cLabelPolice);
		g_cLabelPolice = NULL;
	}
	
	g_iLabelSize = MAX (1, g_key_file_get_integer (fconf, "LABELS", "size", &erreur));
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iLabelSize = 14;  // valeur par defaut.
		g_key_file_set_integer (fconf, "LABELS", "size", g_iLabelSize);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_iLabelWeight = g_key_file_get_integer (fconf, "LABELS", "weight", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iLabelWeight = 5;  // valeur par defaut.
		g_key_file_set_integer (fconf, "LABELS", "weight", g_iLabelWeight);
		bFlushConfFileNeeded = TRUE;
	}
	g_iLabelWeight = MIN (9, MAX (1, g_iLabelWeight));
	g_iLabelWeight = ((PANGO_WEIGHT_HEAVY - PANGO_WEIGHT_ULTRALIGHT) * g_iLabelWeight + 9 * PANGO_WEIGHT_ULTRALIGHT - PANGO_WEIGHT_HEAVY) / 8;  // on se ramene aux intervalles definit par Pango.
	
	gboolean bLabelStyleItalic = g_key_file_get_boolean (fconf, "LABELS", "italic", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		bLabelStyleItalic = FALSE;
		g_iLabelStyle = PANGO_STYLE_NORMAL;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "LABELS", "italic", (g_iLabelStyle != PANGO_STYLE_NORMAL));
		bFlushConfFileNeeded = TRUE;
	}
	if (bLabelStyleItalic)
		g_iLabelStyle = PANGO_STYLE_ITALIC;
	else
		g_iLabelStyle = PANGO_STYLE_NORMAL;
	
	g_bLabelForPointedIconOnly = g_key_file_get_boolean (fconf, "LABELS", "pointed icon only", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_bLabelForPointedIconOnly = FALSE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "LABELS", "pointed icon only", g_bLabelForPointedIconOnly);
		bFlushConfFileNeeded = TRUE;
	}
	
	
	if (g_cLabelPolice == NULL)
		g_bUseText = FALSE;
	
	if (! g_bUseText)
	{
		g_iLabelSize = 0;
		g_free (g_cLabelPolice);
		g_cLabelPolice = NULL;
	}
	
	
	//\___________________ On recupere les parametres des lanceurs.
	gchar **directoryList = g_key_file_get_string_list (fconf, "LAUNCHERS", "default icon directory", &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		directoryList = NULL;
		length = 0;
		g_key_file_set_string (fconf, "LAUNCHERS", "default icon directory", "");
		bFlushConfFileNeeded = TRUE;
	}
	g_strfreev (g_cDefaultIconDirectory);
	g_cDefaultIconDirectory = g_new0 (gchar *, length + 2);
	g_cDefaultIconDirectory[0] = g_strdup (g_cCairoDockDataDir);
	if (directoryList != NULL && length > 0)
		memcpy (&g_cDefaultIconDirectory[1], directoryList, length * sizeof (gchar *));
	g_free (directoryList);
	
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER] = g_key_file_get_integer (fconf, "LAUNCHERS", "max icon size", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER] = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "LAUNCHERS", "max icon size", g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER]);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_LAUNCHER] = g_key_file_get_integer (fconf, "LAUNCHERS", "min icon size", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tMinIconAuthorizedSize[CAIRO_DOCK_LAUNCHER] = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "LAUNCHERS", "min icon size", g_tMinIconAuthorizedSize[CAIRO_DOCK_LAUNCHER]);
		bFlushConfFileNeeded = TRUE;
	}
	
	gchar *cAnimationName;
	cAnimationName = g_key_file_get_string (fconf, "LAUNCHERS", "animation type", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		cAnimationName = g_strdup (s_tAnimationNames[0]);  // valeur par defaut.
		g_key_file_set_string (fconf, "LAUNCHERS", "animation type", cAnimationName);
		bFlushConfFileNeeded = TRUE;
	}
	g_tAnimationType[CAIRO_DOCK_LAUNCHER] = cairo_dock_get_number_from_name (cAnimationName, s_tAnimationNames);
	g_free (cAnimationName);
	
	g_tNbAnimationRounds[CAIRO_DOCK_LAUNCHER] = g_key_file_get_integer (fconf, "LAUNCHERS", "number of animation rounds", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tNbAnimationRounds[CAIRO_DOCK_LAUNCHER] = 4;  // valeur par defaut.
		g_key_file_set_integer (fconf, "LAUNCHERS", "number of animation rounds", g_tNbAnimationRounds[CAIRO_DOCK_LAUNCHER]);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_cDefaultFileBrowser = g_key_file_get_string (fconf, "LAUNCHERS", "default file browser", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_cDefaultFileBrowser = g_strdup ("nautilus");  // valeur par defaut.
		g_key_file_set_string (fconf, "LAUNCHERS", "default file browser", g_cDefaultFileBrowser);
		bFlushConfFileNeeded = TRUE;
	}
	
	
	//\___________________ On recupere les parametres du dock en lui-meme.
	g_fAmplitude = g_key_file_get_double (fconf, "CAIRO DOCK", "amplitude", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fAmplitude = 1.0;  // valeur par defaut.
		g_key_file_set_double (fconf, "CAIRO DOCK", "amplitude", g_fAmplitude);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_iSinusoidWidth = g_key_file_get_integer (fconf, "CAIRO DOCK", "sinusoid width", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iSinusoidWidth = 250;  // valeur par defaut.
		g_key_file_set_integer (fconf, "CAIRO DOCK", "sinusoid width", g_iSinusoidWidth);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_iDockRadius = g_key_file_get_integer (fconf, "CAIRO DOCK", "corner radius", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iDockRadius = 10;  // valeur par defaut.
		g_key_file_set_integer (fconf, "CAIRO DOCK", "corner radius", g_iDockRadius);
		bFlushConfFileNeeded = TRUE;
	}

	g_iDockLineWidth = g_key_file_get_integer (fconf, "CAIRO DOCK", "line width", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iDockLineWidth = 1;  // valeur par defaut.
		g_key_file_set_integer (fconf, "CAIRO DOCK", "line width", g_iDockLineWidth);
		bFlushConfFileNeeded = TRUE;
	}
	
	length = 0;
	gdouble *couleur = g_key_file_get_double_list (fconf, "CAIRO DOCK", "line color", &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fLineColor[0] = 0.0;  // valeur par defaut.
		g_fLineColor[1] = 0.0;  // valeur par defaut.
		g_fLineColor[2] = 0.6;  // valeur par defaut.
		g_fLineColor[3] = 0.4;  // valeur par defaut.
		g_key_file_set_double_list (fconf, "CAIRO DOCK", "line color", g_fLineColor, 4);
		bFlushConfFileNeeded = TRUE;
	}
	else
	{
		if (length > 0)
			memcpy (&g_fLineColor, couleur, MAX (4, length) * sizeof (double));
	}
	g_free (couleur);
	
	g_bRoundedBottomCorner = g_key_file_get_boolean (fconf, "CAIRO DOCK", "rounded bottom corner", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_bRoundedBottomCorner = TRUE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "CAIRO DOCK", "rounded bottom corner", g_bRoundedBottomCorner);
		bFlushConfFileNeeded = TRUE;
	}
	
	
	g_fGrowUpFactor = g_key_file_get_double (fconf, "CAIRO DOCK", "grow up factor", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fGrowUpFactor = 1.45;  // valeur par defaut.
		g_key_file_set_double (fconf, "CAIRO DOCK", "grow up factor", g_fGrowUpFactor);
		bFlushConfFileNeeded = TRUE;
	}
	g_fGrowUpFactor = MAX (1.02, g_fGrowUpFactor);
	g_fShrinkDownFactor = g_key_file_get_double (fconf, "CAIRO DOCK", "shrink down factor", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fShrinkDownFactor = 0.65;  // valeur par defaut.
		g_key_file_set_double (fconf, "CAIRO DOCK", "shrink down factor", g_fShrinkDownFactor);
		bFlushConfFileNeeded = TRUE;
	}
	g_fShrinkDownFactor = MAX (0, MIN (g_fShrinkDownFactor, 0.99));
	g_fMoveUpSpeed = g_key_file_get_double (fconf, "CAIRO DOCK", "move up speed", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fMoveUpSpeed = 0.35;  // valeur par defaut.
		g_key_file_set_double (fconf, "CAIRO DOCK", "move up speed", g_fMoveUpSpeed);
		bFlushConfFileNeeded = TRUE;
	}
	g_fMoveUpSpeed = MAX (0.01, MIN (g_fMoveUpSpeed, 1));
	g_fMoveDownSpeed = g_key_file_get_double (fconf, "CAIRO DOCK", "move down speed", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fMoveDownSpeed = 0.25;  // valeur par defaut.
		g_key_file_set_double (fconf, "CAIRO DOCK", "move down speed", g_fMoveDownSpeed);
		bFlushConfFileNeeded = TRUE;
	}
	g_fMoveDownSpeed = MAX (0.01, MIN (g_fMoveDownSpeed, 1));
	
	
	//\___________________ On recupere les parametres des rayures.
	g_iNbStripes = g_key_file_get_integer (fconf, "STRIPES", "number of stripes", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iNbStripes = 10;  // valeur par defaut.
		g_key_file_set_integer (fconf, "STRIPES", "number of stripes", g_iNbStripes);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_fStripesWidth = g_key_file_get_double (fconf, "STRIPES", "stripes width", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fStripesWidth = 0.02;  // valeur par defaut.
		g_key_file_set_double (fconf, "STRIPES", "stripes width", g_fStripesWidth);
		bFlushConfFileNeeded = TRUE;
	}
	
	length = 0;
	couleur = g_key_file_get_double_list (fconf, "STRIPES", "stripes color bright", &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fStripesColorBright[0] = 0.7;  // valeur par defaut.
		g_fStripesColorBright[1] = 0.9;  // valeur par defaut.
		g_fStripesColorBright[2] = 0.7;  // valeur par defaut.
		g_fStripesColorBright[3] = 0.4;  // valeur par defaut.
		g_key_file_set_double_list (fconf, "STRIPES", "stripes color bright", g_fStripesColorBright, 4);
		bFlushConfFileNeeded = TRUE;
	}
	else
	{
		if (length > 0)
			memcpy (&g_fStripesColorBright, couleur, MAX (4, length) * sizeof (double));
	}
	g_free (couleur);
	
	length = 0;
	couleur = g_key_file_get_double_list (fconf, "STRIPES", "stripes color dark", &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_fStripesColorDark[0] = 0.7;  // valeur par defaut.
		g_fStripesColorDark[1] = 0.7;  // valeur par defaut.
		g_fStripesColorDark[2] = 1.0;  // valeur par defaut.
		g_fStripesColorDark[3] = 0.7;  // valeur par defaut.
		g_key_file_set_double_list (fconf, "STRIPES", "stripes color dark", g_fStripesColorDark, 4);
		bFlushConfFileNeeded = TRUE;
	}
	else
	{
		if (length > 0)
			memcpy (&g_fStripesColorDark, couleur, MAX (4, length) * sizeof (double));
	}
	g_free (couleur);
	
	
	//\___________________ On recupere les parametres des aplications.
	g_bShowAppli = g_key_file_get_boolean (fconf, "APPLICATIONS", "show applications", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_bShowAppli = FALSE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "APPLICATIONS", "show applications", g_bShowAppli);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI] = g_key_file_get_integer (fconf, "APPLICATIONS", "max icon size", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI] = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "APPLICATIONS", "max icon size", g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI]);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI] = g_key_file_get_integer (fconf, "APPLICATIONS", "min icon size", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI] = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "APPLICATIONS", "min icon size", g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI]);
		bFlushConfFileNeeded = TRUE;
	}
	
	cAnimationName = g_key_file_get_string (fconf, "APPLICATIONS", "animation type", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		cAnimationName = g_strdup (s_tAnimationNames[0]);  // valeur par defaut.
		g_key_file_set_string (fconf, "APPLICATIONS", "animation type", cAnimationName);
		bFlushConfFileNeeded = TRUE;
	}
	g_tAnimationType[CAIRO_DOCK_APPLI] = cairo_dock_get_number_from_name (cAnimationName, s_tAnimationNames);
	g_free (cAnimationName);
	
	g_tNbAnimationRounds[CAIRO_DOCK_APPLI] = g_key_file_get_integer (fconf, "APPLICATIONS", "number of animation rounds", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tNbAnimationRounds[CAIRO_DOCK_APPLI] = 2;  // valeur par defaut.
		g_key_file_set_integer (fconf, "APPLICATIONS", "number of animation rounds", g_tNbAnimationRounds[CAIRO_DOCK_APPLI]);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_bUniquePid = g_key_file_get_boolean (fconf, "APPLICATIONS", "unique PID", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_bUniquePid = FALSE;  // valeur par defaut.
		g_key_file_set_boolean (fconf, "APPLICATIONS", "unique PID", g_iAppliMaxNameLength);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_iAppliMaxNameLength = g_key_file_get_integer (fconf, "APPLICATIONS", "max name length", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_iAppliMaxNameLength = 15;  // valeur par defaut.
		g_key_file_set_integer (fconf, "APPLICATIONS", "max name length", g_iAppliMaxNameLength);
		bFlushConfFileNeeded = TRUE;
	}
	
	
	//\___________________ On recupere les parametres des applets.
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET] = g_key_file_get_integer (fconf, "APPLETS", "max icon size", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET] = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "APPLETS", "max icon size", g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET]);
		bFlushConfFileNeeded = TRUE;
	}
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET] = g_key_file_get_integer (fconf, "APPLETS", "min icon size", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET] = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "APPLETS", "min icon size", g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET]);
		bFlushConfFileNeeded = TRUE;
	}
	
	cAnimationName = g_key_file_get_string (fconf, "APPLETS", "animation type", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		cAnimationName = g_strdup (s_tAnimationNames[0]);  // valeur par defaut.
		g_key_file_set_string (fconf, "APPLETS", "animation type", cAnimationName);
		bFlushConfFileNeeded = TRUE;
	}
	g_tAnimationType[CAIRO_DOCK_APPLET] = cairo_dock_get_number_from_name (cAnimationName, s_tAnimationNames);
	g_free (cAnimationName);
	
	g_tNbAnimationRounds[CAIRO_DOCK_APPLET] = g_key_file_get_integer (fconf, "APPLETS", "number of animation rounds", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tNbAnimationRounds[CAIRO_DOCK_APPLET] = 1;  // valeur par defaut.
		g_key_file_set_integer (fconf, "APPLETS", "number of animation rounds", g_tNbAnimationRounds[CAIRO_DOCK_APPLET]);
		bFlushConfFileNeeded = TRUE;
	}
	
	gchar **cActiveModuleList = g_key_file_get_string_list (fconf, "APPLETS", "active modules", &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		cActiveModuleList = NULL;  // valeur par defaut.
		g_key_file_set_string (fconf, "APPLETS", "active modules", "");
		bFlushConfFileNeeded = TRUE;
	}
	
	
	//\___________________ On recupere les parametres des separateurs.
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12] = g_key_file_get_integer (fconf, "SEPARATORS", "max icon size", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12] = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "SEPARATORS", "max icon size", g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12]);
		bFlushConfFileNeeded = TRUE;
	}
	g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR23] = g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12];
	
	g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12] = g_key_file_get_integer (fconf, "SEPARATORS", "min icon size", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET] = 0;  // valeur par defaut.
		g_key_file_set_integer (fconf, "SEPARATORS", "min icon size", g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12]);
		bFlushConfFileNeeded = TRUE;
	}
	g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR23] = g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12];
	
	g_free (g_cSeparatorImage);
	g_cSeparatorImage = g_key_file_get_string (fconf, "SEPARATORS", "separator image", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		g_cSeparatorImage = NULL;  // valeur par defaut.
		g_key_file_set_string (fconf, "SEPARATORS", "separator image", "");
		bFlushConfFileNeeded = TRUE;
	}
	if (g_cSeparatorImage != NULL && strcmp (g_cSeparatorImage, "") == 0)
	{
		g_free (g_cSeparatorImage);
		g_cSeparatorImage = NULL;
	}
	
	
	if (bFlushConfFileNeeded)
	{
		cairo_dock_write_keys_to_file (fconf, g_cConfFile);
	}
	
	
	//\___________________ On (re)charge tout, car n'importe quel parametre peut avoir change.
	cairo_dock_remove_all_applets (pWidget);  // on est obliges d'arreter tous les applets, 
	
	if (g_iSidUpdateAppliList != 0 && ! g_bShowAppli)  // on ne veut plus voir les applis, il faut donc les enlever.
	{
		cairo_dock_remove_all_applis (pWidget);
	}
	
	
	if (icons == NULL)
		cairo_dock_init_list_with_desktop_files (pWidget, g_cCairoDockDataDir);
	else
		cairo_dock_reload_buffers (pWidget, 1 + g_fAmplitude, g_iLabelSize, g_bUseText);
	
	
	if (g_iSidUpdateAppliList == 0 && g_bShowAppli)  // maintenant on veut voir les applis !
	{
		cairo_dock_show_all_applis (pWidget);
	}
	
	
	//cairo_dock_load_module_from_directory (CAIRO_DOCK_MODULES_DIR, pWidget, NULL, g_tActiveModuleList);
	cairo_dock_activate_modules_from_list (cActiveModuleList, g_hModuleTable, pWidget);
	g_strfreev (cActiveModuleList);
	
	cairo_dock_update_dock_size (pWidget, g_iMaxIconHeight, g_iMinDockWidth);
	
	if (!g_bHorizontalDock)
	{
		double tmp = g_iMaxDockWidth;
		g_iMaxDockWidth = g_iMaxDockHeight;
		g_iMaxDockHeight = tmp;
		tmp = g_iVisibleZoneWidth;
		g_iVisibleZoneWidth = g_iVisibleZoneHeight;
		g_iVisibleZoneHeight = tmp;
	}
	
	cairo_dock_load_background_image (GTK_WINDOW (pWidget), g_cCairoDockBackgroundFileName, g_iVisibleZoneWidth, g_iVisibleZoneHeight);
	if (g_bAtBottom)
	{
		//g_print ("on commence en bas a %dx%d\n", g_iVisibleZoneWidth, g_iVisibleZoneHeight);
		if (g_bHorizontalDock)
		{
			g_iWindowPositionX = g_iGapX + (g_iScreenWidth - g_iVisibleZoneWidth) / 2;
			g_iWindowPositionY = g_iScreenHeight - g_iGapY - (g_bDirectionUp ? g_iVisibleZoneHeight : 0);
		}
		else
		{
			g_iWindowPositionX = (g_bDirectionUp ? g_iGapX : g_iScreenWidth - g_iGapX);
			g_iWindowPositionY = g_iScreenHeight / 2 + g_iGapY;
		}
		gdk_window_move_resize (pWidget->window,
			g_iWindowPositionX,
			g_iWindowPositionY,
			g_iVisibleZoneWidth,
			g_iVisibleZoneHeight);
	}
	
	g_key_file_free (fconf);
}


gboolean cairo_dock_edit_conf_file (GtkWidget *pWidget, gchar *conf_file, gchar *cTitle)
{
	GSList *pWidgetList = NULL;
	GtkTextBuffer *pTextBuffer = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, conf_file, G_KEY_FILE_KEEP_COMMENTS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	GtkWidget *pDialog = cairo_dock_generate_advanced_ihm_from_keyfile (pKeyFile, cTitle, pWidget, &pWidgetList);
	if (pDialog == NULL || pWidgetList == NULL)
	{
		pDialog = cairo_dock_generate_basic_ihm_from_keyfile (conf_file, cTitle, pWidget, &pTextBuffer);
	}
	g_return_val_if_fail (pDialog != NULL, FALSE);
	
	gint action = gtk_dialog_run (GTK_DIALOG (pDialog));
	gboolean config_ok = TRUE;
	if (action == GTK_RESPONSE_ACCEPT)
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
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	g_key_file_set_integer (pKeyFile, "POSITION", "x gap", x);
	g_key_file_set_integer (pKeyFile, "POSITION", "y gap", y);
	
	cairo_dock_write_keys_to_file (pKeyFile, g_cConfFile);
	g_key_file_free (pKeyFile);
}


static void _cairo_dock_write_one_name (gchar *cName, gpointer value, GString *pString)
{
	g_string_append_printf (pString, "%s;", cName);
}
void cairo_dock_update_conf_file_with_hash_table (gchar *cConfFile, GHashTable *pModuleTable, gchar *cGroupName, gchar *cKeyName, int iNbAvailableChoices, gchar *cUsefullComment)
{
	g_print ("%s (%s)\n", __func__, cConfFile);
	GError *erreur = NULL;
	
	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	
	g_key_file_load_from_file (pKeyFile, cConfFile, G_KEY_FILE_KEEP_COMMENTS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	g_key_file_remove_comment (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	GString *cComment = g_string_new ("");
	g_string_printf (cComment, "s%d[", iNbAvailableChoices);
	g_hash_table_foreach (pModuleTable, (GHFunc) _cairo_dock_write_one_name, cComment);
	cComment->len --;
	g_string_append_printf (cComment, "]\n %s", cUsefullComment);
	
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
}
void cairo_dock_update_conf_file_with_modules (gchar *cConfFile, GHashTable *pModuleTable)
{
	cairo_dock_update_conf_file_with_hash_table (cConfFile, pModuleTable, "APPLETS", "active modules", 99, "List of active plug-ins (applets and others).");
}


static void _cairo_dock_write_one_module_name_if_active (gchar *cModuleName, CairoDockModule *pModule, GSList **pListeModule)
{
	if (pModule->bActive)
	{
		if (g_slist_find (*pListeModule, cModuleName) == NULL)
			*pListeModule = g_slist_prepend (*pListeModule, cModuleName);
	}
}
void cairo_dock_update_conf_file_with_active_modules (gchar *cConfFile, GHashTable *pModuleTable)
{
	g_print ("%s ()\n", __func__);
	GError *erreur = NULL;
	
	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	
	g_key_file_load_from_file (pKeyFile, cConfFile, G_KEY_FILE_KEEP_COMMENTS, &erreur);
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
	for (pList = icons; pList != NULL; pList = pList->next)
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
	
	g_key_file_set_string (pKeyFile, "APPLETS", "active modules", cActiveModules->str);
	
	cairo_dock_write_keys_to_file (pKeyFile, cConfFile);
	
	g_string_free (cActiveModules, TRUE);
	g_key_file_free (pKeyFile);
}

