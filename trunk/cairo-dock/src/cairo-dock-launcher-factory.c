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
#include "cairo-dock-icons.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-launcher-factory.h"

extern CairoDock *g_pMainDock;
extern double g_fSubDockSizeRatio;

extern double g_fAmplitude;
extern int g_iIconGap;

extern int g_iLabelSize;
extern gboolean g_bTextAlwaysHorizontal;
extern gchar *g_cLabelPolice;

extern gchar *g_cConfFile;
extern gchar *g_cCurrentThemePath;
extern gchar **g_cDefaultIconDirectory;
extern GtkIconTheme *g_pIconTheme;

extern gboolean g_bDirectionUp;
extern gboolean g_bSameHorizontality;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUseGlitz;
extern CairoDockLoadDirectoryFunc cairo_dock_load_directory_func;


gchar *cairo_dock_search_image_path (gchar *cFileName)
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


cairo_surface_t *cairo_dock_create_surface_from_image (gchar *cImagePath, cairo_t* pSourceContext, double fMaxScale, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fImageWidth, double *fImageHeight, double fRotationAngle, double fAlpha, gboolean bReapeatAsPattern)
{
	//g_print ("%s (%s  : %d)\n", __func__, cImagePath, strlen (cImagePath));
	g_return_val_if_fail (cImagePath != NULL, NULL);
	GError *erreur = NULL;
	RsvgDimensionData rsvg_dimension_data;
	RsvgHandle *rsvg_handle;
	cairo_surface_t* surface_ini;
	cairo_surface_t* pNewSurface = NULL;
	cairo_t* pCairoContext = NULL;
	
	double fIconWidthSaturationFactor = 1.;
	double fIconHeightSaturationFactor = 1.;
	
	if (g_str_has_suffix (cImagePath, ".svg"))
	{
		rsvg_handle = rsvg_handle_new_from_file (cImagePath, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return NULL;
		}
		else
		{
			rsvg_handle_get_dimensions (rsvg_handle, &rsvg_dimension_data);
			*fImageWidth = (gdouble) rsvg_dimension_data.width;
			*fImageHeight = (gdouble) rsvg_dimension_data.height;
			//g_print ("%.2fx%.2f\n", *fImageWidth, *fImageHeight);
			if (! bReapeatAsPattern)
				cairo_dock_calculate_contrainted_icon_size (fImageWidth,
					fImageHeight,
					iMinIconAuthorizedWidth,
					iMinIconAuthorizedHeight,
					iMaxIconAuthorizedWidth,
					iMaxIconAuthorizedHeight,
					&fIconWidthSaturationFactor,
					&fIconHeightSaturationFactor);
			//g_print ("-> x%.2f ; x%.2f\n", fIconWidthSaturationFactor, fIconHeightSaturationFactor);
			pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				ceil ((*fImageWidth) * fMaxScale),
				ceil ((*fImageHeight) * fMaxScale));
			
			pCairoContext = cairo_create (pNewSurface);
			cairo_scale (pCairoContext, fMaxScale * fIconWidthSaturationFactor, fMaxScale * fIconHeightSaturationFactor);
			
			rsvg_handle_render_cairo (rsvg_handle, pCairoContext);
		}
	}
	else if (g_str_has_suffix (cImagePath, ".png"))
	{
		surface_ini = cairo_image_surface_create_from_png (cImagePath);
		if (cairo_surface_status (surface_ini) == CAIRO_STATUS_SUCCESS)
		{
			*fImageWidth = (double) cairo_image_surface_get_width (surface_ini);
			*fImageHeight = (double) cairo_image_surface_get_height (surface_ini);
			
			if (! bReapeatAsPattern)
				cairo_dock_calculate_contrainted_icon_size (fImageWidth,
					fImageHeight,
					iMinIconAuthorizedWidth,
					iMinIconAuthorizedHeight,
					iMaxIconAuthorizedWidth,
					iMaxIconAuthorizedHeight,
					&fIconWidthSaturationFactor,
					&fIconHeightSaturationFactor);
			
			pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				ceil ((*fImageWidth) * fMaxScale),
				ceil ((*fImageHeight) * fMaxScale));
			pCairoContext = cairo_create (pNewSurface);
			
			cairo_scale (pCairoContext, fMaxScale * fIconWidthSaturationFactor, fMaxScale * fIconHeightSaturationFactor);
			
			cairo_set_source_surface (pCairoContext, surface_ini, 0, 0);
			cairo_paint (pCairoContext);
			cairo_surface_destroy (surface_ini);
		}
	}
	else  // le code suivant permet de charger tout type d'image, mais en fait c'est un peu idiot d'utiliser des icones n'ayant pas de transparence.
	{
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (cImagePath, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return NULL;
		}
		*fImageWidth = gdk_pixbuf_get_width (pixbuf);
		*fImageHeight = gdk_pixbuf_get_height (pixbuf);
		
		if (! bReapeatAsPattern)
			cairo_dock_calculate_contrainted_icon_size (fImageWidth, 
				fImageHeight,
				iMinIconAuthorizedWidth,
				iMinIconAuthorizedHeight,
				iMaxIconAuthorizedWidth,
				iMaxIconAuthorizedHeight,
				&fIconWidthSaturationFactor,
				&fIconHeightSaturationFactor);
			
		if (!gdk_pixbuf_get_has_alpha (pixbuf))  // on lui rajoute un canal alpha si elle n'en a pas.
		{
			GdkPixbuf *pixbuf2 = gdk_pixbuf_add_alpha (pixbuf, TRUE, 255, 255, 255);  // TRUE <=> les pixels blancs deviennent transparents.906.00x299.00
			gdk_pixbuf_unref (pixbuf);
			pixbuf = pixbuf2;
		}
		
		guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
		surface_ini = cairo_image_surface_create_for_data (pixels,
			CAIRO_FORMAT_ARGB32,
			gdk_pixbuf_get_width (pixbuf),
			gdk_pixbuf_get_height (pixbuf),
			gdk_pixbuf_get_rowstride (pixbuf));
		
		pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			ceil ((*fImageWidth) * fMaxScale),
			ceil ((*fImageHeight) * fMaxScale));
		pCairoContext = cairo_create (pNewSurface);
		
		cairo_scale (pCairoContext, fMaxScale * fIconWidthSaturationFactor, fMaxScale * fIconHeightSaturationFactor);
		cairo_set_source_surface (pCairoContext, surface_ini, 0, 0);
		cairo_paint (pCairoContext);
		cairo_surface_destroy (surface_ini);
	}
	
	if (bReapeatAsPattern)
	{
		cairo_dock_calculate_contrainted_icon_size (fImageWidth,
			fImageHeight,
			iMinIconAuthorizedWidth,
			iMinIconAuthorizedHeight,
			iMaxIconAuthorizedWidth,
			iMaxIconAuthorizedHeight,
			&fIconWidthSaturationFactor,
			&fIconHeightSaturationFactor);
		
		cairo_surface_t *pNewSurfaceFilled = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			*fImageWidth* fMaxScale,
			*fImageHeight* fMaxScale);
		cairo_destroy (pCairoContext);
		pCairoContext = cairo_create (pNewSurfaceFilled);
		
		
		cairo_pattern_t* pPattern = cairo_pattern_create_for_surface (pNewSurface);
		cairo_pattern_set_extend (pPattern, CAIRO_EXTEND_REPEAT);
		
		cairo_set_source (pCairoContext, pPattern);
		cairo_paint (pCairoContext);
		
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceFilled;
	}
	
	if (fAlpha < 1)
	{
		cairo_surface_t *pNewSurfaceAlpha = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			*fImageWidth * fMaxScale,
			*fImageHeight * fMaxScale);
		cairo_destroy (pCairoContext);
		pCairoContext = cairo_create (pNewSurfaceAlpha);
		
		cairo_set_source_surface (pCairoContext, pNewSurface, 0, 0);
		cairo_paint_with_alpha (pCairoContext, fAlpha);
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceAlpha;
	}
	
	if (fRotationAngle != 0)
	{
		cairo_surface_t *pNewSurfaceRotated = cairo_dock_rotate_surface (pNewSurface, pSourceContext, *fImageWidth * fMaxScale, *fImageHeight * fMaxScale, fRotationAngle);
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceRotated;
		/*if (fabs (fRotationAngle) > G_PI / 2)
		{
			pNewSurfaceRotated = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				*fImageWidth * fMaxScale,
				*fImageHeight * fMaxScale);
			cairo_destroy (pCairoContext);
			pCairoContext = cairo_create (pNewSurfaceRotated);
			
			cairo_translate (pCairoContext, 0, *fImageHeight * fMaxScale);
			cairo_scale (pCairoContext, 1, -1);
		}
		else
		{
			pNewSurfaceRotated = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				*fImageHeight * fMaxScale,
				*fImageWidth* fMaxScale);
			cairo_destroy (pCairoContext);
			pCairoContext = cairo_create (pNewSurfaceRotated);
			
			if (fRotationAngle < 0)
			{
				cairo_move_to (pCairoContext, *fImageHeight * fMaxScale, 0);
				cairo_rotate (pCairoContext, fRotationAngle);
				cairo_translate (pCairoContext, - (*fImageWidth) * fMaxScale, 0);
			}
			else
			{
				cairo_move_to (pCairoContext, 0, 0);
				cairo_rotate (pCairoContext, fRotationAngle);
				cairo_translate (pCairoContext, 0, - *fImageHeight * fMaxScale);
			}
		}
		cairo_set_source_surface (pCairoContext, pNewSurface, 0, 0);
		
		cairo_paint (pCairoContext);
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceRotated;*/
	}
	
	cairo_destroy (pCairoContext);
	return pNewSurface;
}

cairo_surface_t * cairo_dock_rotate_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, double fRotationAngle)
{
	if (fRotationAngle != 0)
	{
		cairo_surface_t *pNewSurfaceRotated;
		cairo_t *pCairoContext;
		if (fabs (fRotationAngle) > G_PI / 2)
		{
			pNewSurfaceRotated = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				fImageWidth,
				fImageHeight);
			pCairoContext = cairo_create (pNewSurfaceRotated);
			
			cairo_translate (pCairoContext, 0, fImageHeight);
			cairo_scale (pCairoContext, 1, -1);
		}
		else
		{
			pNewSurfaceRotated = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				fImageHeight,
				fImageWidth);
			pCairoContext = cairo_create (pNewSurfaceRotated);
			
			if (fRotationAngle < 0)
			{
				cairo_move_to (pCairoContext, fImageHeight, 0);
				cairo_rotate (pCairoContext, fRotationAngle);
				cairo_translate (pCairoContext, - fImageWidth, 0);
			}
			else
			{
				cairo_move_to (pCairoContext, 0, 0);
				cairo_rotate (pCairoContext, fRotationAngle);
				cairo_translate (pCairoContext, 0, - fImageHeight);
			}
		}
		cairo_set_source_surface (pCairoContext, pSurface, 0, 0);
		cairo_paint (pCairoContext);
		
		cairo_destroy (pCairoContext);
		return pNewSurfaceRotated;
	}
	else
	{
		return NULL;
	}
}



void cairo_dock_load_icon_info_from_desktop_file (gchar *cDesktopFileName, Icon *icon)
{
	gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, cDesktopFileName);
	
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
	if (icon->acCommand == NULL || strcmp (icon->acCommand, "") == 0)
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
	
	icon->bIsURI = g_key_file_get_boolean (keyfile, "Desktop Entry", "Is URI", &erreur);
	if (erreur != NULL)
	{
		icon->bIsURI = FALSE;
		g_error_free (erreur);
		erreur = NULL;
	}
	icon->bIsMountingPoint = g_key_file_get_boolean (keyfile, "Desktop Entry", "Is mounting point", &erreur);
	if (erreur != NULL)
	{
		icon->bIsMountingPoint = FALSE;
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
			if (icon->bIsURI)
			{
				g_print ("toto\n");
				if (cairo_dock_load_directory_func != NULL)
					cairo_dock_load_directory_func (icon);
			}
			else
			{
				pChildDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_MENU, icon->acName);
				cairo_dock_reference_dock (pChildDock);
				icon->pSubDock = pChildDock;
			}
		}
		else
		{
			cairo_dock_reference_dock (pChildDock);
			icon->pSubDock = pChildDock;
		}
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



Icon * cairo_dock_create_icon_from_desktop_file (gchar *cDesktopFileName, cairo_t *pSourceContext)
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
		pParentDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_MENU, icon->cParentDockName);
	}
	
	return icon;
}
