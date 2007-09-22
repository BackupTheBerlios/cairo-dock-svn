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
#include "cairo-dock-config.h"
#include "cairo-dock-applet-factory.h"


extern double g_fAmplitude;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

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



Icon *cairo_dock_create_icon_for_applet (CairoDock *pDock, int iWidth, int iHeight, gchar *cName, gchar *cIconFileName)
{
	Icon *icon = g_new0 (Icon, 1);
	icon->iType = CAIRO_DOCK_APPLET;
	
	icon->acName = g_strdup (cName);
	icon->acFileName = g_strdup (cIconFileName);  // NULL si cIconFileName = NULL.
	
	icon->fWidth =iWidth;
	icon->fHeight =iHeight;
	cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock);
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, icon);
	
	if (cIconFileName == NULL)
		icon->pIconBuffer = cairo_dock_create_applet_surface (pSourceContext, 1 + g_fAmplitude, &icon->fWidth, &icon->fHeight);
	else
		cairo_dock_fill_one_icon_buffer (icon, pSourceContext, 1 + g_fAmplitude, pDock->bHorizontalDock);
	
	cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
	
	cairo_destroy (pSourceContext);
	return icon;
}


GKeyFile *cairo_dock_read_header_applet_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gboolean *bFlushConfFileNeeded)
{
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	*iWidth = cairo_dock_get_integer_key_value (pKeyFile, "ICON", "width", bFlushConfFileNeeded, 48);
	
	*iHeight = cairo_dock_get_integer_key_value (pKeyFile, "ICON", "height", bFlushConfFileNeeded, 48);
	
	*cName = cairo_dock_get_string_key_value (pKeyFile, "ICON", "name", bFlushConfFileNeeded, NULL);
	
	return pKeyFile;
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
