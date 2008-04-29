/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include <pango/pango.h>

#include "cairo-dock-struct.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-log.h"

extern int g_tIconAuthorizedWidth[CAIRO_DOCK_NB_TYPES];
extern int g_tIconAuthorizedHeight[CAIRO_DOCK_NB_TYPES];
extern double g_fAmplitude;
extern double g_fReflectSize;
extern double g_fAlbedo;
extern gboolean g_bDirectionUp;

extern int g_iLabelWeight;
extern int g_iLabelStyle;
extern int g_iDockRadius;


void cairo_dock_calculate_contrainted_size (double *fImageWidth, double *fImageHeight, int iWidthConstraint, int iHeightConstraint, double *fZoomWidth, double *fZoomHeight)
{
	if (iWidthConstraint != 0)
	{
		*fZoomWidth = 1. * iWidthConstraint / (*fImageWidth);
		*fImageWidth = (double) iWidthConstraint;
	}
	else
		*fZoomWidth = 1.;
	if (iHeightConstraint != 0)
	{
		*fZoomHeight = 1. * iHeightConstraint / (*fImageHeight);
		*fImageHeight = (double) iHeightConstraint;
	}
	else
		*fZoomHeight = 1.;
}

void cairo_dock_calculate_size_constant_ratio (double *fImageWidth, double *fImageHeight, int iWidthConstraint, int iHeightConstraint, double *fZoom)
{
	if (iWidthConstraint != 0 && iHeightConstraint != 0)
		*fZoom = MIN (iWidthConstraint / (*fImageWidth), iHeightConstraint / (*fImageHeight));
	else if (iWidthConstraint != 0)
		*fZoom = iWidthConstraint / (*fImageWidth);
	else if (iHeightConstraint != 0)
		*fZoom = iHeightConstraint / (*fImageHeight);
	else
		*fZoom = 1.;
	*fImageWidth = (*fImageWidth) * (*fZoom);
	*fImageHeight = (*fImageHeight) * (*fZoom);
}


cairo_surface_t *cairo_dock_create_surface_from_xicon_buffer (gulong *pXIconBuffer, int iBufferNbElements, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);

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
	gint pixel, alpha, red, green, blue;
	float fAlphaFactor;
	gint *pPixelBuffer = (gint *) &pXIconBuffer[iBestIndex+2];  // on va ecrire le resultat du filtre directement dans le tableau fourni en entree. C'est ok car sizeof(gulong) >= sizeof(gint), donc le tableau de pixels est plus petit que le buffer fourni en entree. merci a Hannemann pour ses tests et ses screenshots ! :-)
	for (i = 0; i < (int) (*fHeight) * (*fWidth); i ++)
	{
		pixel = (gint) pXIconBuffer[iBestIndex+2+i];
		alpha = (pixel & 0xFF000000) >> 24;
		red   = (pixel & 0x00FF0000) >> 16;
		green = (pixel & 0x0000FF00) >> 8;
		blue  = (pixel & 0x000000FF);
		fAlphaFactor = (float) alpha / 255;
		red *= fAlphaFactor;
		green *= fAlphaFactor;
		blue *= fAlphaFactor;
		pPixelBuffer[i] = (pixel & 0xFF000000) + (red << 16) + (green << 8) + blue;
	}

	//\____________________ On cree la surface a partir du tampon.
	int iStride = (int) (*fWidth) * sizeof (gint);  // nbre d'octets entre le debut de 2 lignes.
	cairo_surface_t *surface_ini = cairo_image_surface_create_for_data ((guchar *)pPixelBuffer,
		CAIRO_FORMAT_ARGB32,
		(int) pXIconBuffer[iBestIndex],
		(int) pXIconBuffer[iBestIndex+1],
		(int) iStride);

	double fIconWidthSaturationFactor, fIconHeightSaturationFactor;
	cairo_dock_calculate_contrainted_size (fWidth,
		fHeight,
		g_tIconAuthorizedWidth[CAIRO_DOCK_APPLI],
		g_tIconAuthorizedHeight[CAIRO_DOCK_APPLI],
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


cairo_surface_t *cairo_dock_create_surface_from_pixbuf (GdkPixbuf *pixbuf, cairo_t *pSourceContext, double fMaxScale, int iWidthConstraint, int iHeightConstraint, gboolean bKeepRatio, double *fImageWidth, double *fImageHeight)
{
	*fImageWidth = gdk_pixbuf_get_width (pixbuf);
	*fImageHeight = gdk_pixbuf_get_height (pixbuf);

	double fIconWidthSaturationFactor = 1., fIconHeightSaturationFactor = 1.;
	if (bKeepRatio)
	{
		cairo_dock_calculate_size_constant_ratio (fImageWidth,
			fImageHeight,
			iWidthConstraint,
			iHeightConstraint,
			&fIconWidthSaturationFactor);
		fIconHeightSaturationFactor = fIconWidthSaturationFactor;
	}
	else
	{
		cairo_dock_calculate_contrainted_size (fImageWidth,
			fImageHeight,
			iWidthConstraint,
			iHeightConstraint,
			&fIconWidthSaturationFactor,
			&fIconHeightSaturationFactor);
	}

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


cairo_surface_t *cairo_dock_create_surface_from_image (gchar *cImagePath, cairo_t* pSourceContext, double fMaxScale, int iWidthConstraint, int iHeightConstraint, double *fImageWidth, double *fImageHeight, gboolean bKeepRatio)
{
	//g_print ("%s (%s, %dx%dx%.2f)\n", __func__, cImagePath, iWidthConstraint, iHeightConstraint, fMaxScale);
	g_return_val_if_fail (cImagePath != NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	GError *erreur = NULL;
	RsvgDimensionData rsvg_dimension_data;
	RsvgHandle *rsvg_handle = NULL;
	cairo_surface_t* surface_ini;
	cairo_surface_t* pNewSurface = NULL;
	cairo_t* pCairoContext = NULL;
	double fIconWidthSaturationFactor = 1.;
	double fIconHeightSaturationFactor = 1.;
	
	//\_______________ On cherche a determiner le type de l'image. En effet, les SVG et les PNG sont charges differemment des autres.
	gboolean bIsSVG = FALSE, bIsPNG = FALSE, bIsXPM = FALSE;
	FILE *fd = fopen (cImagePath, "r");
	if (fd != NULL)
	{
		char buffer[6];
		if (fgets (buffer, 5, fd) != NULL)
		{
			if (strncmp (buffer+2, "SVG", 3) == 0)
				bIsSVG = TRUE;
			else if (strncmp (buffer+1, "PNG", 3) == 0)
				bIsPNG = TRUE;
			else if (strncmp (buffer+2, "XPM", 3) == 0)
				bIsXPM = TRUE;
		}
		fclose (fd);
	}
	if (! bIsSVG && ! bIsPNG && ! bIsXPM)  // sinon en desespoir de cause on se base sur l'extension.
	{
		cd_debug ("  on se base sur l'extension en desespoir de cause.");
		if (g_str_has_suffix (cImagePath, ".svg"))
			bIsSVG = TRUE;
		else if (g_str_has_suffix (cImagePath, ".png"))
			bIsPNG = TRUE;
	}
	
	if (bIsSVG)
	{
		rsvg_handle = rsvg_handle_new_from_file (cImagePath, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			return NULL;
		}
		else
		{
			g_return_val_if_fail (rsvg_handle != NULL, NULL);
			rsvg_handle_get_dimensions (rsvg_handle, &rsvg_dimension_data);
			*fImageWidth = (gdouble) rsvg_dimension_data.width;
			*fImageHeight = (gdouble) rsvg_dimension_data.height;
			//g_print ("%.2fx%.2f\n", *fImageWidth, *fImageHeight);
			if (bKeepRatio)
			{
				cairo_dock_calculate_size_constant_ratio (fImageWidth,
					fImageHeight,
					iWidthConstraint,
					iHeightConstraint,
					&fIconWidthSaturationFactor);
				fIconHeightSaturationFactor = fIconWidthSaturationFactor;
			}
			else
			{
				cairo_dock_calculate_contrainted_size (fImageWidth,
					fImageHeight,
					iWidthConstraint,
					iHeightConstraint,
					&fIconWidthSaturationFactor,
					&fIconHeightSaturationFactor);
			}
			
			pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				ceil ((*fImageWidth) * fMaxScale),
				ceil ((*fImageHeight) * fMaxScale));

			pCairoContext = cairo_create (pNewSurface);
			cairo_scale (pCairoContext, fMaxScale * fIconWidthSaturationFactor, fMaxScale * fIconHeightSaturationFactor);

			rsvg_handle_render_cairo (rsvg_handle, pCairoContext);
			g_object_unref (rsvg_handle);
		}
	}
	else if (bIsPNG)
	{
		surface_ini = cairo_image_surface_create_from_png (cImagePath);
		if (cairo_surface_status (surface_ini) == CAIRO_STATUS_SUCCESS)
		{
			*fImageWidth = (double) cairo_image_surface_get_width (surface_ini);
			*fImageHeight = (double) cairo_image_surface_get_height (surface_ini);
			
			if (bKeepRatio)
			{
				cairo_dock_calculate_size_constant_ratio (fImageWidth,
					fImageHeight,
					iWidthConstraint,
					iHeightConstraint,
					&fIconWidthSaturationFactor);
				fIconHeightSaturationFactor = fIconWidthSaturationFactor;
			}
			else
			{
				cairo_dock_calculate_contrainted_size (fImageWidth,
					fImageHeight,
					iWidthConstraint,
					iHeightConstraint,
					&fIconWidthSaturationFactor,
					&fIconHeightSaturationFactor);
			}
			
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
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (cImagePath, &erreur);  // semble se baser sur l'extension pour definir le type !
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			return NULL;
		}

		pNewSurface = cairo_dock_create_surface_from_pixbuf (pixbuf,
			pSourceContext,
			fMaxScale,
			iWidthConstraint,
			iHeightConstraint,
			FALSE,
			fImageWidth,
			fImageHeight);
	}
	cairo_destroy (pCairoContext);
	
	return pNewSurface;
}

cairo_surface_t *cairo_dock_create_surface_for_icon (gchar *cImagePath, cairo_t* pSourceContext, double fImageWidth, double fImageHeight)
{
	double fImageWidth_ = fImageWidth, fImageHeight_ = fImageHeight;
	return cairo_dock_create_surface_from_image (cImagePath,
		pSourceContext,
		1.,
		fImageWidth,
		fImageHeight,
		&fImageWidth_,
		&fImageHeight_,
		FALSE);
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


static cairo_surface_t * cairo_dock_create_reflection_surface_horizontal (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, double fMaxScale)
{
	g_return_val_if_fail (pSurface != NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);

	//\_______________ On cree la surface d'une fraction hauteur de l'image originale.
	double fReflectHeight = g_fReflectSize * fMaxScale;
	if (fReflectHeight == 0 || g_fAlbedo == 0)
		return NULL;
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fImageWidth,
		fReflectHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);

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

static cairo_surface_t * cairo_dock_create_reflection_surface_vertical (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, double fMaxScale)
{
	g_return_val_if_fail (pSurface != NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);

	//\_______________ On cree la surface d'une fraction hauteur de l'image originale.
	double fReflectWidth = g_fReflectSize * fMaxScale;
	if (fReflectWidth == 0 || g_fAlbedo == 0)
		return NULL;
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fReflectWidth,
		fImageHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);

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

cairo_surface_t * cairo_dock_create_reflection_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, gboolean bHorizontalDock, double fMaxScale)
{
	if (bHorizontalDock)
		return cairo_dock_create_reflection_surface_horizontal (pSurface, pSourceContext, fImageWidth, fImageHeight,fMaxScale);
	else
		return cairo_dock_create_reflection_surface_vertical (pSurface, pSourceContext, fImageWidth, fImageHeight, fMaxScale);
}


cairo_surface_t * cairo_dock_create_icon_surface_with_reflection_horizontal (cairo_surface_t *pIconSurface, cairo_surface_t *pReflectionSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, double fMaxScale)
{
	g_return_val_if_fail (pIconSurface != NULL && pReflectionSurface!= NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);

	//\_______________ On cree la surface de telle facon qu'elle contienne les 2 surfaces.
	double fReflectHeight = g_fReflectSize * fMaxScale;
	if (fReflectHeight == 0 || g_fAlbedo == 0)
		return NULL;
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fImageWidth,
		fImageHeight + fReflectHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);

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
cairo_surface_t * cairo_dock_create_icon_surface_with_reflection_vertical (cairo_surface_t *pIconSurface, cairo_surface_t *pReflectionSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, double fMaxScale)
{
	g_return_val_if_fail (pIconSurface != NULL && pReflectionSurface!= NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);

	//\_______________ On cree la surface de telle facon qu'elle contienne les 2 surfaces.
	double fReflectWidth = g_fReflectSize * fMaxScale;
	if (fReflectWidth == 0 || g_fAlbedo == 0)
		return NULL;
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fImageWidth + fReflectWidth,
		fImageHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);

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

cairo_surface_t * cairo_dock_create_icon_surface_with_reflection (cairo_surface_t *pIconSurface, cairo_surface_t *pReflectionSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, gboolean bHorizontalDock, double fMaxScale)
{
	if (bHorizontalDock)
		return cairo_dock_create_icon_surface_with_reflection_horizontal (pIconSurface, pReflectionSurface, pSourceContext, fImageWidth, fImageHeight, fMaxScale);
	else
		return cairo_dock_create_icon_surface_with_reflection_vertical (pIconSurface, pReflectionSurface, pSourceContext, fImageWidth, fImageHeight, fMaxScale);
}


cairo_surface_t *cairo_dock_create_surface_from_text (gchar *cText, cairo_t* pSourceContext, int iLabelSize, gchar *cLabelPolice, int iLabelWeight, double *fBackgroundColor, double fMaxScale, int *iTextWidth, int *iTextHeight, double *fTextXOffset, double *fTextYOffset)
{
	g_return_val_if_fail (cText != NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	
	//\_________________ On ecrit le texte dans un calque Pango.
	PangoLayout *pLayout = pango_cairo_create_layout (pSourceContext);
	
	PangoFontDescription *pDesc = pango_font_description_new ();
	pango_font_description_set_absolute_size (pDesc, fMaxScale * iLabelSize * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, cLabelPolice);
	pango_font_description_set_weight (pDesc, iLabelWeight);
	pango_font_description_set_style (pDesc, g_iLabelStyle);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_font_description_free (pDesc);
	
	pango_layout_set_text (pLayout, cText, -1);
	
	//\_________________ On recupere la taille effective du calque.
	PangoRectangle ink, log;
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	
	*iTextWidth = ink.width + 2;
	*iTextHeight = ink.height + 2 + 1;  // +1 car certaines polices "debordent".
	
	//\_________________ On dessine le calque dans une surface cairo.
	cairo_surface_t* pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		*iTextWidth, *iTextHeight);
	cairo_t* pCairoContext = cairo_create (pNewSurface);
	
	if (fBackgroundColor != NULL && fBackgroundColor[3] > 0)  // non transparent.
	{
		cairo_save (pCairoContext);
		double fRadius = fMaxScale * MIN (.5 * g_iDockRadius, 5.);  // bon compromis.
		double fLineWidth = 1.;
		double fFrameWidth = *iTextWidth - 2 * fRadius - fLineWidth;
		double fFrameHeight = *iTextHeight - fLineWidth;
		double fDockOffsetX = fRadius + fLineWidth/2;
		double fDockOffsetY = 0.;
		cairo_dock_draw_frame (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight, fDockOffsetX, fDockOffsetY, 1, 0., CAIRO_DOCK_HORIZONTAL);
		cairo_set_source_rgba (pCairoContext, fBackgroundColor[0], fBackgroundColor[1], fBackgroundColor[2], fBackgroundColor[3]);
		cairo_fill_preserve (pCairoContext);
		cairo_restore(pCairoContext);
	}
	
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
	
	/* set_device_offset is buggy, doesn't work for positive offsets. so we use explicit offsets... so unfortunate.
	cairo_surface_set_device_offset (pNewSurface,
					 log.width / 2. - ink.x,
					 log.height     - ink.y);*/
	*fTextXOffset = (log.width / 2. - ink.x) / fMaxScale;
	*fTextYOffset = - (iLabelSize - (log.height - ink.y)) / fMaxScale ;  // en tenant compte de l'ecart du bas du texte.
	//*fTextYOffset = - (ink.y) / fMaxScale;  // pour tenir compte de l'ecart du bas du texte.
	
	*iTextWidth = *iTextWidth / fMaxScale;
	*iTextHeight = *iTextHeight / fMaxScale;
	
	g_object_unref (pLayout);
	return pNewSurface;
}


cairo_surface_t * cairo_dock_duplicate_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fWidth, double fHeight, double fDesiredWidth, double fDesiredHeight)
{
	g_return_val_if_fail (pSurface != NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);

	//\_______________ On cree la surface de la taille desiree.
	if (fDesiredWidth == 0)
		fDesiredWidth = fWidth;
	if (fDesiredHeight == 0)
		fDesiredHeight = fHeight;
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fDesiredWidth,
		fDesiredHeight);
	cairo_t *pCairoContext = cairo_create (pNewSurface);

	//\_______________ On plaque la surface originale dessus.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_scale (pCairoContext,
		fDesiredWidth / fWidth,
		fDesiredHeight / fHeight);
	
	cairo_set_source_surface (pCairoContext, pSurface, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_destroy (pCairoContext);
	
	return pNewSurface;
}
