/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>
#include <pango/pango.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-application-factory.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-applet-factory.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-load.h"

extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;
extern double g_fSubDockSizeRatio;
extern gboolean g_bSameHorizontality;

extern int g_iSinusoidWidth;
extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern gint g_iFrameMargin;
extern double g_fAmplitude;
extern int g_iIconGap;
extern double g_fAlbedo;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern gboolean g_bReverseVisibleImage;

extern int g_iLabelWeight;
extern int g_iLabelStyle;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;
extern gchar *g_cCurrentThemePath;

extern int g_iDockRadius;
extern int g_iDockLineWidth;

extern gchar *g_cBackgroundImageFile;
extern double g_fBackgroundImageAlpha;
extern cairo_surface_t *g_pBackgroundSurface[2];
extern cairo_surface_t *g_pBackgroundSurfaceFull[2];
extern double g_fBackgroundImageWidth, g_fBackgroundImageHeight;
extern gboolean g_bBackgroundImageRepeat;
extern int g_iNbStripes;
extern double g_fStripesAngle;
extern double g_fStripesWidth;
extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];

extern gboolean g_bDirectionUp;

extern unsigned int g_iAppliMaxNameLength;

extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUseGlitz;


gchar *cairo_dock_generate_file_path (gchar *cImageFile)
{
	g_return_val_if_fail (cImageFile != NULL, NULL);
	gchar *cImagePath;
	if (*cImageFile == '~')
	{
		cImagePath = g_strdup_printf ("%s%s", getenv("HOME"), cImageFile + 1);
	}
	else if (*cImageFile == '/')
	{
		cImagePath = g_strdup (cImageFile);
	}
	else
	{
		cImagePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, cImageFile);
	}
	return cImagePath;
}

cairo_surface_t *cairo_dock_load_image (cairo_t *pSourceContext, gchar *cImageFile, double *fImageWidth, double *fImageHeight, double fRotationAngle, double fAlpha, gboolean bReapeatAsPattern)
{
	//g_print ("%s (%dx%d)\n", __func__, image_width, image_height);
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	cairo_surface_t *pNewSurface = NULL;
	
	if (cImageFile != NULL)
	{
		gchar *cImagePath = cairo_dock_generate_file_path (cImageFile);
		
		pNewSurface = cairo_dock_create_surface_from_image (cImagePath,
			pSourceContext,
			1.,
			(int) (*fImageWidth),
			(int) (*fImageHeight),
			(int) (*fImageWidth),
			(int) (*fImageHeight),
			fImageWidth,
			fImageHeight,
			fRotationAngle,
			fAlpha,
			bReapeatAsPattern);
		
		g_free (cImagePath);
	}
	
	return pNewSurface;
}



void cairo_dock_fill_one_icon_buffer (Icon *icon, cairo_t* pSourceContext, gdouble fMaxScale, gboolean bHorizontalDock)
{
	//g_print ("%s (%.2f, %s)\n", __func__, fMaxScale, icon->acFileName);
	cairo_surface_destroy (icon->pIconBuffer);
	icon->pIconBuffer = NULL;
	cairo_surface_destroy (icon->pReflectionBuffer);
	icon->pReflectionBuffer = NULL;
	
	if (CAIRO_DOCK_IS_LAUNCHER (icon))
	{
		//\_______________________ On recherche une icone.
		gchar *cIconPath = cairo_dock_search_icon_s_path (icon->acFileName);
		
		//\_______________________ On cree la surface cairo a afficher.
		if (cIconPath != NULL && strlen (cIconPath) > 0)
		{
			icon->pIconBuffer = cairo_dock_create_surface_from_image (cIconPath,
				pSourceContext,
				fMaxScale,
				g_tMinIconAuthorizedSize[CAIRO_DOCK_LAUNCHER],
				g_tMinIconAuthorizedSize[CAIRO_DOCK_LAUNCHER],
				g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER],
				g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER],
				(bHorizontalDock ? &icon->fWidth : &icon->fHeight),
				(bHorizontalDock ? &icon->fHeight : &icon->fWidth),
				0,
				1,
				FALSE);
		}
		
		g_free (cIconPath);
	}
	else if (CAIRO_DOCK_IS_VALID_APPLI (icon))  // c'est l'icône d'une appli valide.
	{
		icon->pIconBuffer = cairo_dock_create_surface_from_xwindow (icon->Xid, pSourceContext, fMaxScale, &icon->fWidth, &icon->fHeight);
	}
	else if (CAIRO_DOCK_IS_APPLET (icon))  // c'est l'icône d'une applet.
	{
		icon->pIconBuffer = cairo_dock_create_applet_surface (icon->acFileName, pSourceContext, fMaxScale, &icon->fWidth, &icon->fHeight);
	}
	else  // c'est une icone de separation.
	{
		icon->pIconBuffer = cairo_dock_create_separator_surface (pSourceContext, fMaxScale, bHorizontalDock, &icon->fWidth, &icon->fHeight);
	}
	
	if (icon->pIconBuffer == NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/%s", CAIRO_DOCK_SHARE_DATA_DIR, CAIRO_DOCK_DEFAULT_ICON_NAME);
		icon->pIconBuffer = cairo_dock_create_surface_from_image (cIconPath,
			pSourceContext,
			fMaxScale,
			g_tMinIconAuthorizedSize[icon->iType],
			g_tMinIconAuthorizedSize[icon->iType],
			g_tMaxIconAuthorizedSize[icon->iType],
			g_tMaxIconAuthorizedSize[icon->iType],
			(bHorizontalDock ? &icon->fWidth : &icon->fHeight),
			(bHorizontalDock ? &icon->fHeight : &icon->fWidth),
			0,
			1,
			FALSE);
		g_free (cIconPath);
	}
	
	if (g_fAlbedo > 0 && icon->pIconBuffer != NULL)
	{
		icon->pReflectionBuffer = cairo_dock_create_reflection_surface (icon->pIconBuffer, 
			pSourceContext,
			(bHorizontalDock ? icon->fWidth : icon->fHeight) * fMaxScale,
			(bHorizontalDock ? icon->fHeight : icon->fWidth) * fMaxScale,
			bHorizontalDock);
	}
}

void cairo_dock_fill_one_text_buffer (Icon *icon, cairo_t* pSourceContext, int iLabelSize, gchar *cLabelPolice, gboolean bHorizontalDock)
{
	//g_print ("%s (%s, %d)\n", __func__, cLabelPolice, iLabelSize);
	cairo_surface_destroy (icon->pTextBuffer);
	icon->pTextBuffer = NULL;
	if (icon->acName == NULL || (iLabelSize == 0))
		return ;
	
	PangoFontDescription *pDesc;
	PangoLayout *pLayout;
	
	pLayout = pango_cairo_create_layout (pSourceContext);
	
	pDesc = pango_font_description_new ();
	pango_font_description_set_absolute_size (pDesc, iLabelSize * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, cLabelPolice);
	pango_font_description_set_weight (pDesc, g_iLabelWeight);
	pango_font_description_set_style (pDesc, g_iLabelStyle);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_font_description_free (pDesc);
	
	
	if (CAIRO_DOCK_IS_APPLI (icon) && g_iAppliMaxNameLength > 0)
	{
		//g_print ("troncature de %s\n", icon->acName);
		gsize bytes_read, bytes_written;
		GError *erreur = NULL;
		gchar *cUtf8Name = g_locale_to_utf8 (icon->acName,
			-1,
			&bytes_read,
			&bytes_written,
			&erreur);  // inutile sur Ubuntu, qui est nativement UTF8, mais sur les autres on ne sait pas.
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		if (cUtf8Name == NULL)  // une erreur s'est produite, on tente avec la chaine brute.
			cUtf8Name = g_strdup (icon->acName);
		
		gchar *cTruncatedName = NULL;
		const gchar *cEndValidChain = NULL;
		if (g_utf8_validate (cUtf8Name, -1, &cEndValidChain))
		{
			if (g_utf8_strlen (cUtf8Name, -1) > g_iAppliMaxNameLength)
			{
				cTruncatedName = g_new0 (gchar, 8 * (g_iAppliMaxNameLength + 4));  // 8 octets par caractere.
				g_utf8_strncpy (cTruncatedName, cUtf8Name, g_iAppliMaxNameLength);
				
				gchar *cTruncature = g_utf8_offset_to_pointer (cTruncatedName, g_iAppliMaxNameLength);
				*cTruncature = '.';
				*(cTruncature+1) = '.';
				*(cTruncature+2) = '.';
			}
		}
		else
		{
			if (strlen (icon->acName) > g_iAppliMaxNameLength)
			{
				cTruncatedName = g_new0 (gchar, g_iAppliMaxNameLength + 4);
				strncpy (cTruncatedName, icon->acName, g_iAppliMaxNameLength);
				
				cTruncatedName[g_iAppliMaxNameLength] = '.';
				cTruncatedName[g_iAppliMaxNameLength+1] = '.';
				cTruncatedName[g_iAppliMaxNameLength+2] = '.';
			}
		}
		g_free (cUtf8Name);
		
		pango_layout_set_text (pLayout, (cTruncatedName != NULL ? cTruncatedName : icon->acName), -1);
		//g_print (" -> etiquette : %s\n", cTruncatedName);
		g_free (cTruncatedName);
	}
	else
		pango_layout_set_text (pLayout, icon->acName, -1);
	
	
	PangoRectangle ink, log;
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	
	icon->iTextWidth = ink.width + 2;
	icon->iTextHeight = ink.height + 2 + 1;  // le +1 est la pour palier aux erreurs d'arrondis qui apparaissent avec certaines polices.
	cairo_surface_t* pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		icon->iTextWidth, icon->iTextHeight);
	cairo_t* pCairoContext = cairo_create (pNewSurface);
	cairo_translate (pCairoContext, -ink.x, -ink.y+1);  // meme remarque.
	
	cairo_push_group (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0.2, 0.2, 0.2);
	int i;
	for (i = 0; i < 4; i++)
	{
		cairo_move_to (pCairoContext, i&2, 2*(i&1));
		pango_cairo_show_layout (pCairoContext, pLayout);
	}
	cairo_pop_group_to_source (pCairoContext);
	cairo_paint_with_alpha (pCairoContext, .75);
	
	cairo_set_source_rgb (pCairoContext, 1., 1., 1.);
	cairo_move_to (pCairoContext, 1., 1.);
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	cairo_destroy (pCairoContext);
	
	/* set_device_offset is buggy, doesn't work for positive
	 * offsets.  so we use explicit offsets... so unfortunate.
	cairo_surface_set_device_offset (pNewSurface, 
					 log.width / 2. - ink.x,
					 log.height     - ink.y);*/
	icon->fTextXOffset = log.width / 2. - ink.x;
	//icon->fTextYOffset = log.height     - ink.y;
	icon->fTextYOffset = iLabelSize - (log.height + 1) + ink.y ;  // en tenant compte de l'ecart du bas du texte.
	//g_print ("%s -> %.2f (%d;%d)\n", icon->acName, icon->fTextYOffset, log.height, ink.y);
	
	double fRotationAngle = (bHorizontalDock ? 0 : (g_bDirectionUp ? -G_PI/2 : G_PI/2));
	cairo_surface_t *pNewSurfaceRotated = cairo_dock_rotate_surface (pNewSurface, pSourceContext, icon->iTextWidth, icon->iTextHeight, fRotationAngle);
	if (pNewSurfaceRotated != NULL)
	{
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceRotated;
	}
	
	icon->pTextBuffer = pNewSurface;
	
	g_object_unref (pLayout);
}

void cairo_dock_load_one_icon_from_scratch (Icon *pIcon, CairoDock *pDock)
{
	g_return_if_fail (pIcon != NULL);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	
	cairo_dock_fill_one_icon_buffer (pIcon, pCairoContext, 1 + g_fAmplitude, pDock->bHorizontalDock);
	
	cairo_dock_fill_one_text_buffer (pIcon, pCairoContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
	
	cairo_destroy (pCairoContext);
}

void cairo_dock_reload_buffers_in_dock (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (pDock->iRefCount > 0)
		pDock->bHorizontalDock = (g_bSameHorizontality ? g_pMainDock->bHorizontalDock : ! g_pMainDock->bHorizontalDock);
	else
		pDock->bHorizontalDock = g_pMainDock->bHorizontalDock;
	
	pDock->iFlatDockWidth = - g_iIconGap;
	pDock->iMaxIconHeight = 0;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	double fMaxScale = 1 + g_fAmplitude;
	
	Icon* icon;
	GList* ic;
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		cairo_dock_fill_one_icon_buffer (icon, pCairoContext, fMaxScale, pDock->bHorizontalDock);
		icon->fWidth *= fRatio;
		icon->fHeight *= fRatio;
		pDock->iFlatDockWidth += g_iIconGap + icon->fWidth;
		pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, icon->fHeight);
		
		cairo_dock_fill_one_text_buffer (icon, pCairoContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
	}
	cairo_destroy (pCairoContext);
	
	if (! pDock->bIsMainDock)  // on le fait pas pour le dock principal, ce sera fait a la fin de la fonction 'read_conf'.
	{
		cairo_dock_update_dock_size (pDock);
	}
}
void cairo_dock_reload_buffers_in_all_docks (GHashTable *hDocksTable)
{
	g_hash_table_foreach (hDocksTable, (GHFunc) cairo_dock_reload_buffers_in_dock, NULL);
}



void cairo_dock_load_visible_zone (CairoDock *pDock, gchar *cVisibleZoneImageFile, int iVisibleZoneWidth, int iVisibleZoneHeight, double fVisibleZoneAlpha)
{
	double fVisibleZoneWidth = iVisibleZoneWidth, fVisibleZoneHeight = iVisibleZoneHeight;
	cairo_surface_destroy (g_pVisibleZoneSurface);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_pVisibleZoneSurface = cairo_dock_load_image (pCairoContext,
		cVisibleZoneImageFile,
		&fVisibleZoneWidth,
		&fVisibleZoneHeight,
		(pDock->bHorizontalDock ? (! g_bDirectionUp && g_bReverseVisibleImage ? G_PI : 0) : (g_bDirectionUp ? -G_PI/2 : G_PI/2)),
		fVisibleZoneAlpha,
		FALSE);
	cairo_destroy (pCairoContext);
}

cairo_surface_t *cairo_dock_load_stripes (cairo_t* pSourceContext, int iStripesWidth, int iStripesHeight, double fRotationAngle)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	cairo_pattern_t *pStripesPattern;
	double fWidth = (g_iNbStripes > 0 ? 200. : iStripesWidth);
	if (fabs (g_fStripesAngle) != 90)
		pStripesPattern = cairo_pattern_create_linear (0.0f,
			0.0f,
			fWidth,
			fWidth * tan (g_fStripesAngle * G_PI/180.));
	else
		pStripesPattern = cairo_pattern_create_linear (0.0f,
			0.0f,
			0.,
			(g_fStripesAngle == 90) ? iStripesHeight : - iStripesHeight);
	g_return_val_if_fail (cairo_pattern_status (pStripesPattern) == CAIRO_STATUS_SUCCESS, NULL);
	
	
	cairo_pattern_set_extend (pStripesPattern, CAIRO_EXTEND_REPEAT);
	
	if (g_iNbStripes > 0)
	{
		gdouble fStep;
		double fStripesGap = 1. / (g_iNbStripes);  // ecart entre 2 rayures foncees.
		for (fStep = 0.0f; fStep < 1.0f; fStep += fStripesGap)
		{
			cairo_pattern_add_color_stop_rgba (pStripesPattern,
				fStep - g_fStripesWidth / 2,
				g_fStripesColorBright[0],
				g_fStripesColorBright[1],
				g_fStripesColorBright[2],
				g_fStripesColorBright[3]);
			cairo_pattern_add_color_stop_rgba (pStripesPattern,
				fStep,
				g_fStripesColorDark[0],
				g_fStripesColorDark[1],
				g_fStripesColorDark[2],
				g_fStripesColorDark[3]);
			cairo_pattern_add_color_stop_rgba (pStripesPattern,
				fStep + g_fStripesWidth / 2,
				g_fStripesColorBright[0],
				g_fStripesColorBright[1],
				g_fStripesColorBright[2],
				g_fStripesColorBright[3]);
		}
	}
	else
	{
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			0.,
			g_fStripesColorDark[0],
			g_fStripesColorDark[1],
			g_fStripesColorDark[2],
			g_fStripesColorDark[3]);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			1.,
			g_fStripesColorBright[0],
			g_fStripesColorBright[1],
			g_fStripesColorBright[2],
			g_fStripesColorBright[3]);
	}
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		iStripesWidth,
		iStripesHeight);
	cairo_t *pImageContext = cairo_create (pNewSurface);
	cairo_set_source (pImageContext, pStripesPattern);
	cairo_paint (pImageContext);
	
	cairo_pattern_destroy (pStripesPattern);
	cairo_destroy (pImageContext);
	
	if (fRotationAngle != 0)
	{
		cairo_surface_t *pNewSurfaceRotated = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			iStripesHeight,
			iStripesWidth);
		pImageContext = cairo_create (pNewSurfaceRotated);
		
		if (fRotationAngle < 0)
		{
			cairo_move_to (pImageContext, iStripesWidth, 0);
			cairo_rotate (pImageContext, fRotationAngle);
			cairo_translate (pImageContext, - iStripesWidth, 0);
		}
		else
		{
			cairo_move_to (pImageContext, 0, 0);
			cairo_rotate (pImageContext, fRotationAngle);
			cairo_translate (pImageContext, 0, - iStripesHeight);
		}
		cairo_set_source_surface (pImageContext, pNewSurface, 0, 0);
		
		cairo_paint (pImageContext);
		cairo_surface_destroy (pNewSurface);
		cairo_destroy (pImageContext);
		pNewSurface = pNewSurfaceRotated;
	}
	
	return pNewSurface;
}



void cairo_dock_update_background_decorations_if_necessary (CairoDock *pDock, int iNewDecorationsWidth, int iNewDecorationsHeight)
{
	//g_print ("%s (%dx%d)\n", __func__, iNewDecorationsWidth, iNewDecorationsHeight);
	if (2 * iNewDecorationsWidth > g_fBackgroundImageWidth || iNewDecorationsHeight > g_fBackgroundImageHeight)
	{
		cairo_surface_destroy (g_pBackgroundSurface[0]);
		g_pBackgroundSurface[0] = NULL;
		cairo_surface_destroy (g_pBackgroundSurface[1]);
		g_pBackgroundSurface[1] = NULL;
		cairo_surface_destroy (g_pBackgroundSurfaceFull[0]);
		g_pBackgroundSurfaceFull[0] = NULL;
		cairo_surface_destroy (g_pBackgroundSurfaceFull[1]);
		g_pBackgroundSurfaceFull[1] = NULL;
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
		
		if (g_cBackgroundImageFile != NULL)
		{
			if (g_bBackgroundImageRepeat)
			{
				g_fBackgroundImageWidth = 2 * iNewDecorationsWidth;
				g_fBackgroundImageHeight = iNewDecorationsHeight;
				g_pBackgroundSurfaceFull[CAIRO_DOCK_HORIZONTAL] = cairo_dock_load_image (pCairoContext,
					g_cBackgroundImageFile,
					&g_fBackgroundImageWidth,
					&g_fBackgroundImageHeight,
					0,  // (pDock->bHorizontalDock ? 0 : (g_bDirectionUp ? -G_PI/2 : G_PI/2)),
					g_fBackgroundImageAlpha,
					g_bBackgroundImageRepeat);
				
				g_pBackgroundSurfaceFull[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (g_pBackgroundSurfaceFull[CAIRO_DOCK_HORIZONTAL], pCairoContext, g_fBackgroundImageWidth, g_fBackgroundImageHeight, (g_bDirectionUp ? -G_PI/2 : G_PI/2));
			}
			else if (g_fBackgroundImageWidth == 0 || g_fBackgroundImageHeight == 0)
			{
				g_fBackgroundImageWidth = 0;
				g_fBackgroundImageHeight = iNewDecorationsHeight;
				g_pBackgroundSurface[CAIRO_DOCK_HORIZONTAL] = cairo_dock_load_image (pCairoContext,
					g_cBackgroundImageFile,
					&g_fBackgroundImageWidth,
					&g_fBackgroundImageHeight,
					0,  // (pDock->bHorizontalDock ? 0 : (g_bDirectionUp ? -G_PI/2 : G_PI/2)),
					g_fBackgroundImageAlpha,
					g_bBackgroundImageRepeat);
				
				g_pBackgroundSurface[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (g_pBackgroundSurface[CAIRO_DOCK_HORIZONTAL], pCairoContext, g_fBackgroundImageWidth, g_fBackgroundImageHeight, (g_bDirectionUp ? -G_PI/2 : G_PI/2));
			}
		}
		else
		{
			g_fBackgroundImageWidth = 2 * iNewDecorationsWidth;
			g_fBackgroundImageHeight = iNewDecorationsHeight;
			g_pBackgroundSurfaceFull[CAIRO_DOCK_HORIZONTAL] = cairo_dock_load_stripes (pCairoContext, g_fBackgroundImageWidth, g_fBackgroundImageHeight, 0);  // (pDock->bHorizontalDock ? 0 : (g_bDirectionUp ? -G_PI/2 : G_PI/2))
			
			g_pBackgroundSurfaceFull[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (g_pBackgroundSurfaceFull[CAIRO_DOCK_HORIZONTAL], pCairoContext, g_fBackgroundImageWidth, g_fBackgroundImageHeight, (g_bDirectionUp ? -G_PI/2 : G_PI/2));
		}
		
		cairo_destroy (pCairoContext);
	}
}

static void _cairo_dock_search_max_decorations_size (gchar *cDockName, CairoDock *pDock, int *data)
{
	if (pDock->iDecorationsWidth > data[0])
		data[0] = pDock->iDecorationsWidth;
	if (pDock->iDecorationsHeight > data[1])
		data[1] = pDock->iDecorationsHeight;
}
void cairo_dock_load_background_decorations (CairoDock *pDock)
{
	int data[2] = {0, 0};
	g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_search_max_decorations_size, &data);
	
	g_fBackgroundImageWidth = 0;
	g_fBackgroundImageHeight = 0;
	cairo_dock_update_background_decorations_if_necessary (pDock, data[0], data[1]);
}
