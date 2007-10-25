/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-launcher-factory.h"

extern CairoDock *g_pMainDock;
extern double g_fSubDockSizeRatio;

extern double g_fAmplitude;
extern int g_iIconGap;

extern int g_iLabelSize;
extern gboolean g_bTextAlwaysHorizontal;
extern gchar *g_cLabelPolice;

extern gchar *g_cConfFile;
extern gchar *g_cCurrentLaunchersPath;
extern gchar **g_cDefaultIconDirectory;
extern GtkIconTheme *g_pIconTheme;

extern gboolean g_bDirectionUp;
extern gboolean g_bSameHorizontality;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUseGlitz;


gchar *cairo_dock_search_icon_s_path (gchar *cFileName)
{
	g_return_val_if_fail (cFileName != NULL, NULL);
	GString *sIconPath = g_string_new ("");
	gchar *cSuffixTab[4] = {".svg", ".png", ".xpm", NULL};
	gboolean bAddSuffix, bFileFound;
	int i, j;
	
	//\_______________________ On construit le chemin de l'icone a afficher.
	if (*cFileName == '~')
	{
		g_string_printf (sIconPath, "%s%s", g_getenv ("HOME"), cFileName+1);
	}
	else if (*cFileName == '/')
	{
		g_string_printf (sIconPath, "%s", cFileName);
	}
	else
	{
		//\_______________________ On determine si le suffixe est present ou non.
		bAddSuffix = FALSE;
		j = 0;
		while (cSuffixTab[j] != NULL && ! g_str_has_suffix (cFileName, cSuffixTab[j]))
			j ++;
		
		if (cSuffixTab[j] == NULL)
			bAddSuffix = TRUE;
		
		//\_______________________ On parcourt les repertoires disponibles, en testant tous les suffixes connus.
		i = 0;
		bFileFound = FALSE;
		if (g_cDefaultIconDirectory != NULL)
		{
			while (g_cDefaultIconDirectory[i] != NULL && ! bFileFound)
			{
				j = 0;
				while (! bFileFound && (cSuffixTab[j] != NULL || ! bAddSuffix))
				{
					g_string_printf (sIconPath, "%s/%s", g_cDefaultIconDirectory[i], cFileName);
					if (bAddSuffix)
						g_string_append_printf (sIconPath, "%s", cSuffixTab[j]);
					
					if ( g_file_test (sIconPath->str, G_FILE_TEST_EXISTS) )
						bFileFound = TRUE;
					
					j ++;
					if (! bAddSuffix)
						break;
				}
				i ++;
			}
		}
		
		if (! bFileFound)
		{
			g_string_printf (sIconPath, "%s", cFileName);
			if (! bAddSuffix)
			{
				gchar *str = strrchr (sIconPath->str, '.');
				if (str != NULL)
					*str = '\0';
			}
			//g_print ("on recherche %s dans le theme d'icones\n", sIconPath->str);
			GtkIconInfo* pIconInfo = gtk_icon_theme_lookup_icon  (g_pIconTheme,
				sIconPath->str,
				64,
				GTK_ICON_LOOKUP_FORCE_SVG);
			if (pIconInfo == NULL)
			{
				GtkIconTheme *pDefaultIconTheme = gtk_icon_theme_get_default ();
				if (g_pIconTheme != pDefaultIconTheme)
				{
					pIconInfo = gtk_icon_theme_lookup_icon  (pDefaultIconTheme,
						sIconPath->str,
						64,
						GTK_ICON_LOOKUP_FORCE_SVG);
				}
			}
			
			if (pIconInfo != NULL)
			{
				g_string_printf (sIconPath, "%s", gtk_icon_info_get_filename (pIconInfo));
				gtk_icon_info_free (pIconInfo);
			}
			else
				g_string_printf (sIconPath, cFileName);
		}
	}
	
	gchar *cIconPath = sIconPath->str;
	g_string_free (sIconPath, FALSE);
	return cIconPath;
}



cairo_surface_t *cairo_dock_create_surface_from_pixbuf (GdkPixbuf *pixbuf, cairo_t *pSourceContext, double fMaxScale, gboolean bConstraintSize, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fImageWidth, double *fImageHeight)
{
	*fImageWidth = gdk_pixbuf_get_width (pixbuf);
	*fImageHeight = gdk_pixbuf_get_height (pixbuf);
	
	double fIconWidthSaturationFactor = 1., fIconHeightSaturationFactor = 1.;
	if (bConstraintSize)
		cairo_dock_calculate_contrainted_icon_size (fImageWidth, 
			fImageHeight,
			iMinIconAuthorizedWidth,
			iMinIconAuthorizedHeight,
			iMaxIconAuthorizedWidth,
			iMaxIconAuthorizedHeight,
			&fIconWidthSaturationFactor,
			&fIconHeightSaturationFactor);
	
	GdkPixbuf *pPixbufWithAlpha = pixbuf;
	if (! gdk_pixbuf_get_has_alpha (pixbuf))  // on lui rajoute un canal alpha s'il n'en a pas.
	{
		//g_print ("  ajout d'un canal alpha\n");
		pPixbufWithAlpha = gdk_pixbuf_add_alpha (pixbuf, TRUE, 255, 255, 255);  // TRUE <=> les pixels blancs deviennent transparents.
	}
	
	//\____________________ On pre-multiplie chaque composante par le alpha (necessaire pour libcairo).
	int iNbChannels = gdk_pixbuf_get_n_channels (pPixbufWithAlpha);
	int iRowstride = gdk_pixbuf_get_rowstride (pPixbufWithAlpha);
	guchar *p, *pixels = gdk_pixbuf_get_pixels (pPixbufWithAlpha);
	
	int w = gdk_pixbuf_get_width (pPixbufWithAlpha);
	int h = gdk_pixbuf_get_height (pPixbufWithAlpha);
	int x, y;
	int red, green, blue;
	float fAlphaFactor;
	for (y = 0; y < h; y ++)
	{
		for (x = 0; x < w; x ++)
		{
			p = pixels + y * iRowstride + x * iNbChannels;
			fAlphaFactor = (float) p[3] / 255;
			red = p[0] * fAlphaFactor;
			green = p[1] * fAlphaFactor;
			blue = p[2] * fAlphaFactor;
			p[0] = blue;
			p[1] = green;
			p[2] = red;
		}
	}
	
	///guchar *pixels = gdk_pixbuf_get_pixels (pPixbufWithAlpha);
	cairo_surface_t *surface_ini = cairo_image_surface_create_for_data (pixels,
		CAIRO_FORMAT_ARGB32,
		gdk_pixbuf_get_width (pPixbufWithAlpha),
		gdk_pixbuf_get_height (pPixbufWithAlpha),
		gdk_pixbuf_get_rowstride (pPixbufWithAlpha));
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		ceil ((*fImageWidth) * fMaxScale),
		ceil ((*fImageHeight) * fMaxScale));
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	
	cairo_scale (pCairoContext, fMaxScale * fIconWidthSaturationFactor, fMaxScale * fIconHeightSaturationFactor);
	cairo_set_source_surface (pCairoContext, surface_ini, 0, 0);
	cairo_paint (pCairoContext);
	cairo_surface_destroy (surface_ini);
	
	if (pPixbufWithAlpha != pixbuf)
		g_object_unref (pPixbufWithAlpha);
	return pNewSurface;
}


void cairo_dock_load_icon_info_from_desktop_file (const gchar *cDesktopFileName, Icon *icon)
{
	gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, cDesktopFileName);
	
	GError *erreur = NULL;
	GKeyFile* keyfile = g_key_file_new();
	g_key_file_load_from_file (keyfile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	g_free(icon->acDesktopFileName);
	icon->acDesktopFileName = g_strdup (cDesktopFileName);
	
	g_free(icon->acFileName);
	icon->acFileName = g_key_file_get_string (keyfile, "Desktop Entry", "Icon", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	g_free(icon->acName);
	icon->acName = g_key_file_get_locale_string (keyfile, "Desktop Entry", "Name", NULL, NULL);
	if (icon->acName != NULL && strcmp (icon->acName, "") == 0)
	{
		g_free (icon->acName);
		icon->acName = NULL;
	}
	
	if (icon->acName == NULL)
	{
		icon->acName = g_key_file_get_string (keyfile, "Desktop Entry", "Name", &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
	}
	if (icon->acName != NULL && strcmp (icon->acName, "") == 0)
	{
		g_free (icon->acName);
		icon->acName = NULL;
	}
	
	g_free(icon->acCommand);
	icon->acCommand = g_key_file_get_string (keyfile, "Desktop Entry", "Exec", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	if (icon->acCommand != NULL && strcmp (icon->acCommand, "") == 0)
	{
		g_free (icon->acCommand);
		icon->acCommand = NULL;
	}
	
	icon->fOrder = g_key_file_get_double (keyfile, "Desktop Entry", "Order", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	icon->cBaseURI = g_key_file_get_string (keyfile, "Desktop Entry", "Base URI", &erreur);
	if (erreur != NULL)
	{
		icon->cBaseURI = NULL;
		g_error_free (erreur);
		erreur = NULL;
	}
	if (icon->cBaseURI != NULL && strcmp (icon->cBaseURI, "") == 0)
	{
		g_free (icon->cBaseURI);
		icon->cBaseURI = NULL;
	}
	
	icon->iVolumeID = g_key_file_get_boolean (keyfile, "Desktop Entry", "Is mounting point", &erreur);
	if (erreur != NULL)
	{
		icon->iVolumeID = FALSE;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	gboolean bIsContainer = g_key_file_get_boolean (keyfile, "Desktop Entry", "Is container", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		bIsContainer = FALSE;
	}
	if (bIsContainer && icon->acName != NULL)
	{
		CairoDock *pChildDock = cairo_dock_search_dock_from_name (icon->acName);
		if (pChildDock == NULL)
		{
			g_print ("le dock fils (%s) n'existe pas, on le cree\n", icon->acName);
			pChildDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_DOCK, icon->acName);
		}
		cairo_dock_reference_dock (pChildDock);
		icon->pSubDock = pChildDock;
	}
	
	icon->cParentDockName = g_key_file_get_string (keyfile, "Desktop Entry", "Container", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		icon->cParentDockName = NULL;
	}
	if (icon->cParentDockName == NULL || strcmp (icon->cParentDockName, "") == 0)
	{
		g_free (icon->cParentDockName);
		icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);
	}
	
	icon->iType = CAIRO_DOCK_LAUNCHER;
	
	g_free (cDesktopFilePath);
}



Icon * cairo_dock_create_icon_from_desktop_file (const gchar *cDesktopFileName, cairo_t *pSourceContext)
{
	Icon *icon = g_new0 (Icon, 1);
	
	cairo_dock_load_icon_info_from_desktop_file (cDesktopFileName, icon);
	g_return_val_if_fail (icon->acDesktopFileName != NULL, NULL);
	
	cairo_dock_fill_one_icon_buffer (icon, pSourceContext, 1 + g_fAmplitude, g_pMainDock->bHorizontalDock);
	
	cairo_dock_fill_one_text_buffer (icon,
		pSourceContext,
		g_iLabelSize,
		g_cLabelPolice,
		(g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : g_pMainDock->bHorizontalDock));
	
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
	if (pParentDock == NULL)
	{
		g_print ("le dock parent (%s) n'existe pas, on le cree\n", icon->cParentDockName);
		pParentDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_DOCK, icon->cParentDockName);
	}
	
	return icon;
}
