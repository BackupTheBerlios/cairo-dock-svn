/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <cairo.h>
#include <pango/pango.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-load.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-applet-factory.h"


extern double g_fAmplitude;
extern int g_iLabelSize;
extern gboolean g_bUseText;
extern gchar *g_cLabelPolice;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUseGlitz;


cairo_surface_t *cairo_dock_create_applet_surface (cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	double fIconWidthSaturationFactor, fIconHeightSaturationFactor;
	cairo_dock_calculate_contrainted_icon_size (fWidth,
		fHeight,
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET],
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET],
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET],
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET],
		&fIconWidthSaturationFactor, &fIconHeightSaturationFactor);
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		ceil (*fWidth * fMaxScale),
		ceil (*fHeight * fMaxScale));
	return pNewSurface;
}



Icon *cairo_dock_create_icon_for_applet (CairoDock *pDock, int iWidth, int iHeight, gchar *cName, GtkWidget *pMenu)
{
	Icon *icon = g_new0 (Icon, 1);
	icon->iType = CAIRO_DOCK_APPLET;
	
	icon->acName = g_strdup (cName);
	icon->pMenu = pMenu;
	
	icon->fWidth =iWidth;
	icon->fHeight =iHeight;
	cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock);
	
	icon->pIconBuffer = cairo_dock_create_applet_surface (pSourceContext, 1 + g_fAmplitude, &icon->fWidth, &icon->fHeight);
	cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_bUseText, g_iLabelSize, g_cLabelPolice, pDock->bHorizontalDock);
	
	cairo_destroy (pSourceContext);
	return icon;
}


gboolean cairo_dock_read_header_applet_conf_file (GKeyFile *pKeyFile, int *iWidth, int *iHeight, gchar **cName)
{
	gboolean bFlushConfFileNeeded = FALSE;
	GError *erreur = NULL;
	
	*iWidth = g_key_file_get_integer (pKeyFile, "ICON", "width", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		*iWidth = 48;  // valeur par defaut.
		g_key_file_set_integer (pKeyFile, "ICON", "width", *iWidth);
		bFlushConfFileNeeded = TRUE;
	}
	
	*iHeight = g_key_file_get_integer (pKeyFile, "ICON", "height", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		*iHeight = 48;  // valeur par defaut.
		g_key_file_set_integer (pKeyFile, "ICON", "height", *iHeight);
		bFlushConfFileNeeded = TRUE;
	}
	
	*cName = g_key_file_get_locale_string (pKeyFile, "ICON", "name", NULL, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		*cName = NULL;  // valeur par defaut.
		g_key_file_set_string (pKeyFile, "ICON", "name", "");
		bFlushConfFileNeeded = TRUE;
	}
	if (*cName != NULL && strcmp (*cName, "") == 0)
	{
		g_free (*cName);
		*cName = NULL;
	}
	
	return bFlushConfFileNeeded;
}



GHashTable *cairo_dock_list_themes (gchar *cThemesDir, GHashTable *hProvidedTable, GError **erreur)
{
	GError *tmp_erreur = NULL;
	GDir *dir = g_dir_open (cThemesDir, 0, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	GHashTable *pThemeTable = (hProvidedTable != NULL ? hProvidedTable : g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free));
	
	const gchar* cThemeName;
	gchar *cThemePath;
	do
	{
		cThemeName = g_dir_read_name (dir);
		if (cThemeName == NULL)
			break ;
		
		cThemePath = g_strdup_printf ("%s/%s", cThemesDir, cThemeName);
		
		if (g_file_test (cThemePath, G_FILE_TEST_IS_DIR))
			g_hash_table_insert (pThemeTable, g_strdup (cThemeName), cThemePath);
	}
	while (1);
	g_dir_close (dir);
	
	return pThemeTable;
}
