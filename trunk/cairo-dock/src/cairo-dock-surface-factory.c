/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#include "cairo-dock-struct.h"
#include "cairo-dock-surface-factory.h"

extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern double g_fAmplitude;
extern double g_fReflectSize;
extern double g_fAlbedo;
extern gboolean g_bDirectionUp;


void cairo_dock_calculate_contrainted_icon_size (double *fImageWidth, double *fImageHeight, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fIconWidthSaturationFactor, double *fIconHeightSaturationFactor)
{
	*fIconWidthSaturationFactor = 1;
	*fIconHeightSaturationFactor = 1;
	
	if (iMaxIconAuthorizedWidth > 0 && (*fImageWidth) > iMaxIconAuthorizedWidth)
	{
		*fIconWidthSaturationFactor = 1. * iMaxIconAuthorizedWidth / (*fImageWidth);
		*fImageWidth = (double) iMaxIconAuthorizedWidth;
	}
	if (iMaxIconAuthorizedHeight > 0 && (*fImageHeight) > iMaxIconAuthorizedHeight)
	{
		*fIconHeightSaturationFactor = 1. * iMaxIconAuthorizedHeight / (*fImageHeight);
		*fImageHeight = (double) iMaxIconAuthorizedHeight;
	}
	if (iMinIconAuthorizedWidth > 0 && (*fImageWidth) < iMinIconAuthorizedWidth)
	{
		*fIconWidthSaturationFactor = 1. * iMinIconAuthorizedWidth / (*fImageWidth);
		*fImageWidth = (double) iMinIconAuthorizedWidth;
	}
	if (iMinIconAuthorizedHeight > 0 && (*fImageHeight) < iMinIconAuthorizedHeight)
	{
		*fIconHeightSaturationFactor = 1. * iMinIconAuthorizedHeight / (*fImageHeight);
		*fImageHeight = (double) iMinIconAuthorizedHeight;
	}
}


cairo_surface_t *cairo_dock_create_surface_from_xicon_buffer (gulong *pXIconBuffer, int iBufferNbElements, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	//g_print ("%s (%d)\n", __func__, iBufferNbElements);
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	int iNbChannels = 4;
	
	//\____________________ On recupere la plus grosse des icones presentes dans le tampon (meilleur rendu).
	int iIndex = 0, iBestIndex = 0;
	while (iIndex + 2 < iBufferNbElements)
	{
		if (pXIconBuffer[iIndex] > pXIconBuffer[iBestIndex])
			iBestIndex = iIndex;
		iIndex += 2 + pXIconBuffer[iIndex] * pXIconBuffer[iIndex+1];
	}
	
	//\____________________ On pre-multiplie chaque composante par le alpha (necessaire pour libcairo).
	*fWidth = (double) pXIconBuffer[iBestIndex];
	*fHeight = (double) pXIconBuffer[iBestIndex+1];
	
	int i;
	int alpha, red, green, blue;
	float fAlphaFactor;
	for (i = 0; i < (int) (*fHeight) * (*fWidth); i ++)
	{
		alpha = (pXIconBuffer[iBestIndex+2+i] & 0xFF000000) >> 24;
		red = (pXIconBuffer[iBestIndex+2+i] & 0x00FF0000) >> 16;
		green = (pXIconBuffer[iBestIndex+2+i] & 0x0000FF00) >> 8;
		blue = pXIconBuffer[iBestIndex+2+i] & 0x000000FF;
		fAlphaFactor = (float) alpha / 255;
		red *= fAlphaFactor;
		green *= fAlphaFactor;
		blue *= fAlphaFactor;
		pXIconBuffer[iBestIndex+2+i] = (pXIconBuffer[iBestIndex+2+i] & 0xFF000000) + (red << 16) + (green << 8) + blue;
	}
	
	//\____________________ On cree la surface a partir du tampon.
	cairo_surface_t *surface_ini = cairo_image_surface_create_for_data ((guchar *)&pXIconBuffer[iBestIndex+2],
		CAIRO_FORMAT_ARGB32,
		(int) pXIconBuffer[iBestIndex],
		(int) pXIconBuffer[iBestIndex+1],
		(int) pXIconBuffer[iBestIndex] * iNbChannels);
	
	double fIconWidthSaturationFactor, fIconHeightSaturationFactor;
	cairo_dock_calculate_contrainted_icon_size (fWidth,
		fHeight,
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI],
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI],
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI],
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI],
		&fIconWidthSaturationFactor,
		&fIconHeightSaturationFactor);
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		ceil (*fWidth * fMaxScale),
		ceil (*fHeight * fMaxScale));
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	
	cairo_scale (pCairoContext, fMaxScale * fIconWidthSaturationFactor, fMaxScale * fIconHeightSaturationFactor);
	cairo_set_source_surface (pCairoContext, surface_ini, 0, 0);
	cairo_paint (pCairoContext);
	
	cairo_surface_destroy (surface_ini);
	cairo_destroy (pCairoContext);
	
	return pNewSurface;
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


cairo_surface_t *cairo_dock_create_surface_from_image (gchar *cImagePath, cairo_t* pSourceContext, double fMaxScale, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fImageWidth, double *fImageHeight, double fRotationAngle, double fAlpha, gboolean bReapeatAsPattern)
{
	//g_print ("%s (%s)\n", __func__, cImagePath);
	g_return_val_if_fail (cImagePath != NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
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
		
		pNewSurface = cairo_dock_create_surface_from_pixbuf (pixbuf,
			pSourceContext,
			fMaxScale,
			TRUE,
			iMinIconAuthorizedWidth,
			iMinIconAuthorizedHeight,
			iMaxIconAuthorizedWidth,
			iMaxIconAuthorizedHeight,
			fImageWidth,
			fImageHeight);
	}
	cairo_destroy (pCairoContext);
	
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
		pCairoContext = cairo_create (pNewSurfaceFilled);
		
		cairo_pattern_t* pPattern = cairo_pattern_create_for_surface (pNewSurface);
		cairo_pattern_set_extend (pPattern, CAIRO_EXTEND_REPEAT);
		
		cairo_set_source (pCairoContext, pPattern);
		cairo_paint (pCairoContext);
		cairo_destroy (pCairoContext);
		
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceFilled;
	}
	
	if (fAlpha < 1)
	{
		cairo_surface_t *pNewSurfaceAlpha = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			*fImageWidth * fMaxScale,
			*fImageHeight * fMaxScale);
		pCairoContext = cairo_create (pNewSurfaceAlpha);
		
		cairo_set_source_surface (pCairoContext, pNewSurface, 0, 0);
		cairo_paint_with_alpha (pCairoContext, fAlpha);
		cairo_destroy (pCairoContext);
		
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceAlpha;
	}
	
	if (fRotationAngle != 0)
	{
		cairo_surface_t *pNewSurfaceRotated = cairo_dock_rotate_surface (pNewSurface, pSourceContext, *fImageWidth * fMaxScale, *fImageHeight * fMaxScale, fRotationAngle);
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceRotated;
	}
	
	return pNewSurface;
}

cairo_surface_t * cairo_dock_rotate_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, double fRotationAngle)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
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



static cairo_surface_t * cairo_dock_create_reflection_surface_horizontal (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight)
{
	g_return_val_if_fail (pSurface != NULL && pSourceContext != NULL, NULL);
	
	//\_______________ On cree la surface d'une fraction hauteur de l'image originale.
	double fReflectHeight = g_fReflectSize * (1 + g_fAmplitude);
	if (fReflectHeight == 0)
		return NULL;
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fImageWidth,
		fReflectHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	
	/**cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);*/
	
	
	//\_______________ On dessine l'image originale inversee.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_save (pCairoContext);
	cairo_translate (pCairoContext, 0, fImageHeight);
	cairo_scale (pCairoContext, 1., -1.);
	
	cairo_set_source_surface (pCairoContext, pSurface, 0, (g_bDirectionUp ? 0 : fImageHeight - fReflectHeight));
	cairo_paint (pCairoContext);
	cairo_destroy (pCairoContext);
	
	
	//\_______________ On re-dessine avec un degrade en transparence.
	cairo_surface_t *pNewSurfaceGradated = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fImageWidth,
		fReflectHeight);
	pCairoContext = cairo_create (pNewSurfaceGradated);
	cairo_set_source_surface (pCairoContext, pNewSurface, 0, 0);
	
	cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
		0.,
		0.,
		fReflectHeight);  // de haut en bas.
	g_return_val_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		0.,
		0.,
		0.,
		0.,
		(g_bDirectionUp ? g_fAlbedo : 0.));
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		1.,
		0.,
		0.,
		0.,
		(g_bDirectionUp ? 0 : g_fAlbedo));
	
	cairo_translate (pCairoContext, 0, 0);
	cairo_mask (pCairoContext, pGradationPattern);
	
	cairo_pattern_destroy (pGradationPattern);
	cairo_destroy (pCairoContext);
	cairo_surface_destroy (pNewSurface);
	return pNewSurfaceGradated;
}

static cairo_surface_t * cairo_dock_create_reflection_surface_vertical (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight)
{
	g_return_val_if_fail (pSurface != NULL && pSourceContext != NULL, NULL);
	
	//\_______________ On cree la surface d'une fraction hauteur de l'image originale.
	double fReflectWidth = g_fReflectSize * (1 + g_fAmplitude);
	if (fReflectWidth == 0)
		return NULL;
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fReflectWidth,
		fImageHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	
	/**cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);*/
	
	
	//\_______________ On dessine l'image originale inversee.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_save (pCairoContext);
	cairo_translate (pCairoContext, fImageWidth, 0);
	cairo_scale (pCairoContext, -1., 1.);
	
	cairo_set_source_surface (pCairoContext, pSurface, (g_bDirectionUp ? 0. : fImageHeight - fReflectWidth), 0.);
	cairo_paint (pCairoContext);
	cairo_destroy (pCairoContext);
	
	
	//\_______________ On re-dessine avec un degrade en transparence.
	cairo_surface_t *pNewSurfaceGradated = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fReflectWidth,
		fImageHeight);
	pCairoContext = cairo_create (pNewSurfaceGradated);
	cairo_set_source_surface (pCairoContext, pNewSurface, 0, 0);
	
	cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
		0.,
		fReflectWidth,
		0.);  // de gauche a droite.
	g_return_val_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_REPEAT);
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		0.,
		0.,
		0.,
		0.,
		(g_bDirectionUp ? g_fAlbedo : 0.));
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		1.,
		0.,
		0.,
		0.,
		(g_bDirectionUp ? 0. : g_fAlbedo));
	
	cairo_translate (pCairoContext, 0, 0);
	cairo_mask (pCairoContext, pGradationPattern);
	
	cairo_pattern_destroy (pGradationPattern);
	cairo_destroy (pCairoContext);
	cairo_surface_destroy (pNewSurface);
	return pNewSurfaceGradated;
}

cairo_surface_t * cairo_dock_create_reflection_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, gboolean bHorizontalDock)
{
	if (bHorizontalDock)
		return cairo_dock_create_reflection_surface_horizontal (pSurface, pSourceContext, fImageWidth, fImageHeight);
	else
		return cairo_dock_create_reflection_surface_vertical (pSurface, pSourceContext, fImageWidth, fImageHeight);
}


cairo_surface_t * cairo_dock_create_icon_surface_with_reflection_horizontal (cairo_surface_t *pIconSurface, cairo_surface_t *pReflectionSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight)
{
	g_return_val_if_fail (pIconSurface != NULL && pReflectionSurface!= NULL && pSourceContext != NULL, NULL);
	
	//\_______________ On cree la surface de telle facon qu'elle contienne les 2 surfaces.
	double fReflectHeight = g_fReflectSize * (1 + g_fAmplitude);
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fImageWidth,
		fImageHeight + fReflectHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	
	/**cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);*/
	
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface (pCairoContext, pIconSurface, 0, (g_bDirectionUp ? 0. : fReflectHeight));
	cairo_paint (pCairoContext);
	
	if (pReflectionSurface != NULL)
	{
		cairo_set_source_surface (pCairoContext, pReflectionSurface, 0, (g_bDirectionUp ? fImageHeight : 0));
		cairo_paint (pCairoContext);
	}
	
	cairo_destroy (pCairoContext);
	return pNewSurface;
}
cairo_surface_t * cairo_dock_create_icon_surface_with_reflection_vertical (cairo_surface_t *pIconSurface, cairo_surface_t *pReflectionSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight)
{
	g_return_val_if_fail (pIconSurface != NULL && pReflectionSurface!= NULL && pSourceContext != NULL, NULL);
	
	//\_______________ On cree la surface de telle facon qu'elle contienne les 2 surfaces.
	double fReflectWidth = g_fReflectSize * (1 + g_fAmplitude);
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fImageWidth + fReflectWidth,
		fImageHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	
	/**cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);*/
	
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface (pCairoContext, pIconSurface, (g_bDirectionUp ? 0. : fReflectWidth), 0);
	cairo_paint (pCairoContext);
	
	if (pReflectionSurface != NULL)
	{
		cairo_set_source_surface (pCairoContext, pReflectionSurface, (g_bDirectionUp ? fImageWidth : 0), 0);
		cairo_paint (pCairoContext);
	}
	
	cairo_destroy (pCairoContext);
	return pNewSurface;
}

cairo_surface_t * cairo_dock_create_icon_surface_with_reflection (cairo_surface_t *pIconSurface, cairo_surface_t *pReflectionSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, gboolean bHorizontalDock)
{
	if (bHorizontalDock)
		return cairo_dock_create_icon_surface_with_reflection_horizontal (pIconSurface, pReflectionSurface, pSourceContext, fImageWidth, fImageHeight);
	else
		return cairo_dock_create_icon_surface_with_reflection_vertical (pIconSurface, pReflectionSurface, pSourceContext, fImageWidth, fImageHeight);
}
