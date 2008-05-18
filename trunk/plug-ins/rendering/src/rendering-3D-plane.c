/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "rendering-3D-plane.h"

extern double my_fInclinationOnHorizon;
extern CDSpeparatorType my_iDrawSeparator3D;
extern cairo_surface_t *my_pFlatSeparatorSurface[2];
extern double my_fSeparatorColor[4];


void cd_rendering_calculate_max_dock_size_3D_plane (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	pDock->iDecorationsHeight = (pDock->iMaxIconHeight + g_fReflectSize + 2 * g_iFrameMargin) / sqrt (1 + my_fInclinationOnHorizon * my_fInclinationOnHorizon);
	
	double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pDock->iDecorationsHeight, my_fInclinationOnHorizon, g_iDockRadius, g_iDockLineWidth);
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., fExtraWidth));
	///pDock->iMaxDockWidth = MIN (pDock->iMaxDockWidth, g_iMaxAuthorizedWidth);
	
	pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * pDock->iMaxIconHeight + g_fReflectSize) + g_iLabelSize + g_iDockLineWidth + g_iFrameMargin;
	
	pDock->iDecorationsWidth = pDock->iMaxDockWidth;
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth + fExtraWidth;
	pDock->iMinDockHeight = g_iDockLineWidth + g_iFrameMargin + g_fReflectSize + pDock->iMaxIconHeight;
}
void cd_rendering_calculate_max_dock_size_3D_plane2 (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	double h = 200;
	double fInclinationOnHorizon = h  / (pDock->fFlatDockWidth / 2);
	double hi = g_fReflectSize + g_iFrameMargin;
	
	pDock->iDecorationsHeight = hi + (pDock->iMaxIconHeight + g_iFrameMargin) / sqrt (1 + fInclinationOnHorizon * fInclinationOnHorizon);
	
	double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pDock->iDecorationsHeight, fInclinationOnHorizon, g_iDockRadius, g_iDockLineWidth);
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., fExtraWidth));
	///pDock->iMaxDockWidth = MIN (pDock->iMaxDockWidth, g_iMaxAuthorizedWidth);
	
	pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * pDock->iMaxIconHeight + g_fReflectSize) + g_iLabelSize + g_iDockLineWidth + g_iFrameMargin;
	
	pDock->iDecorationsWidth = pDock->iMaxDockWidth;
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth + fExtraWidth;
	pDock->iMinDockHeight = g_iDockLineWidth + g_iFrameMargin + g_fReflectSize + pDock->iMaxIconHeight;
}

void cd_rendering_calculate_construction_parameters_3D_plane (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY)
{
	icon->fDrawX = icon->fX;
	icon->fDrawY = icon->fY + fReflectionOffsetY;
	icon->fWidthFactor = 1.;
	icon->fHeightFactor = 1.;
	icon->fDeltaYReflection = 0.;
	icon->fOrientation = 0.;
	if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= iCurrentWidth)
	{
		icon->fAlpha = 1;
	}
	else
	{
		icon->fAlpha = .25;
	}
}


cairo_surface_t *cd_rendering_create_flat_separator_surface (cairo_t *pSourceContext, int iWidth, int iHeight)
{
	cairo_pattern_t *pStripesPattern = cairo_pattern_create_linear (0.0f,
		iHeight,
		0.,
		0.);
	g_return_val_if_fail (cairo_pattern_status (pStripesPattern) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_pattern_set_extend (pStripesPattern, CAIRO_EXTEND_REPEAT);
	
	double fStep = 1.;
	double y = 0, h0 = (fStep * (1 + sqrt (1 + 4. * iHeight / fStep)) / 2) - 1, hk = h0;
	int k = 0;
	for (k = 0; k < h0 / fStep; k ++)
	{
		//g_print ("step : %f ; y = %.2f\n", 1.*hk / iHeight, y);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			0.,
			0.,
			0.,
			0.);
		y += 1.*hk / iHeight;
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			0.,
			0.,
			0.,
			0.);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			my_fSeparatorColor[0],
			my_fSeparatorColor[1],
			my_fSeparatorColor[2],
			my_fSeparatorColor[3]);
		y += 1.*hk / iHeight;
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			my_fSeparatorColor[0],
			my_fSeparatorColor[1],
			my_fSeparatorColor[2],
			my_fSeparatorColor[3]);
		hk -= fStep;
	}
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		iWidth,
		iHeight);
	cairo_t *pImageContext = cairo_create (pNewSurface);
	cairo_set_source (pImageContext, pStripesPattern);
	cairo_paint (pImageContext);
	
	cairo_pattern_destroy (pStripesPattern);
	cairo_destroy (pImageContext);
	
	return pNewSurface;
}

static void cd_rendering_draw_3D_separator_horizontal (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bIncludeEdges)
{
	//g_print ("%s ()\n", __func__);
	int sens;
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - pDock->iDecorationsHeight - g_iDockLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iDecorationsHeight + g_iDockLineWidth;
	}
	
	//double h = 200;
	//double fInclinationOnHorizon = h  / (pDock->fFlatDockWidth / 2);
	
	double h = my_fInclinationOnHorizon * (pDock->fFlatDockWidth / 2);
	double fInclination = fabs (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / (pDock->iCurrentWidth / 2);
	///double fInclination = my_fInclinationOnHorizon * fabs ((icon->fDrawX + icon->fWidth * icon->fScale / 2) / pDock->iCurrentWidth - .5) * 2;
	//g_print ("fInclination : %f\n", fInclination);
	double fEpsilon = .1 * icon->fWidth;
	double fDeltaX = pDock->iDecorationsHeight * fInclination;
	double fHeight = pDock->iDecorationsHeight + (bIncludeEdges ? 2 * g_iDockLineWidth : 0);
	if (fDeltaX + 2 * fEpsilon > icon->fWidth)
	{
		fDeltaX = icon->fWidth - 2 * fEpsilon - 1;
		fHeight = fDeltaX / fInclination;
	}
	fDockOffsetY += sens * (pDock->iDecorationsHeight - fHeight) / 2;
	double fBigWidth = icon->fWidth - fDeltaX, fLittleWidth = icon->fWidth - fDeltaX - 2 * fEpsilon;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	if (icon->fDrawX + icon->fWidth * icon->fScale / 2 > pDock->iCurrentWidth / 2)  // on est a droite.
	{
		fDockOffsetX = icon->fDrawX + fEpsilon + icon->fWidth * (icon->fScale - 1) / 2;
		cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		//g_print ("droite : fDockOffsetX : %.2f; fDeltaX:%.2f; fEpsilon:%.2f\n", fDockOffsetX, fDeltaX, fEpsilon);
		
		cairo_rel_line_to (pCairoContext, fLittleWidth/pDock->fRatio, 0);
		cairo_rel_line_to (pCairoContext, (fDeltaX + fEpsilon)/pDock->fRatio, sens * fHeight/pDock->fRatio);
		cairo_rel_line_to (pCairoContext, - fBigWidth/pDock->fRatio, 0);
		cairo_rel_line_to (pCairoContext, (- fDeltaX + fEpsilon)/pDock->fRatio, - sens * fHeight/pDock->fRatio);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, 1, -1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], - fEpsilon, 0);
		}
	}
	else  // a gauche.
	{
		fDockOffsetX = icon->fDrawX + fDeltaX + fEpsilon + icon->fWidth * (icon->fScale - 1) / 2;
		cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		//g_print ("gauche : fDockOffsetX : %.2f; fDeltaX:%.2f; fEpsilon:%.2f\n", fDockOffsetX, fDeltaX, fEpsilon);
		
		cairo_rel_line_to (pCairoContext, fLittleWidth, 0);
		cairo_rel_line_to (pCairoContext, - fDeltaX + fEpsilon, sens * fHeight);
		cairo_rel_line_to (pCairoContext, - fBigWidth, 0);
		cairo_rel_line_to (pCairoContext, fDeltaX + fEpsilon, - sens * fHeight);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, 1, -1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], - fDeltaX - fEpsilon, 0);
		}
	}
}
static void cd_rendering_draw_3D_separator_vertical (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bIncludeEdges)
{
	int sens;
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - pDock->iDecorationsHeight - g_iDockLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iDecorationsHeight + g_iDockLineWidth;
	}
	
	double fInclination = my_fInclinationOnHorizon * fabs ((icon->fDrawX + icon->fWidth * icon->fScale / 2) / pDock->iCurrentWidth - .5) * 2;
	//g_print ("fInclination : %f\n", fInclination);
	double fEpsilon = .1 * icon->fWidth;
	double fDeltaX = pDock->iDecorationsHeight * fInclination;
	double fHeight = pDock->iDecorationsHeight;
	if (fDeltaX + 2 * fEpsilon > icon->fWidth)
	{
		fDeltaX = icon->fWidth - 2 * fEpsilon - 1;
		fHeight = fDeltaX / fInclination;
	}
	fDockOffsetY += sens * (pDock->iDecorationsHeight - fHeight) / 2;
	double fBigWidth = icon->fWidth - fDeltaX, fLittleWidth = icon->fWidth - fDeltaX - 2 * fEpsilon;
	
	if (icon->fDrawX + icon->fWidth * icon->fScale / 2 > pDock->iCurrentWidth / 2)  // on est a droite.
	{
		fDockOffsetX = icon->fDrawX + fEpsilon + icon->fWidth * (icon->fScale - 1) / 2;
		cairo_translate (pCairoContext, fDockOffsetY, fDockOffsetX);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		//g_print ("droite : fDockOffsetX : %.2f; fDeltaX:%.2f; fEpsilon:%.2f\n", fDockOffsetX, fDeltaX, fEpsilon);
		
		cairo_rel_line_to (pCairoContext, 0, fLittleWidth/pDock->fRatio);
		cairo_rel_line_to (pCairoContext, sens * fHeight/pDock->fRatio, (fDeltaX + fEpsilon)/pDock->fRatio);
		cairo_rel_line_to (pCairoContext, 0, - fBigWidth/pDock->fRatio);
		cairo_rel_line_to (pCairoContext,  - sens * fHeight/pDock->fRatio, (- fDeltaX + fEpsilon)/pDock->fRatio);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, - fEpsilon);
		}
	}
	else  // a gauche.
	{
		fDockOffsetX = icon->fDrawX + fDeltaX + fEpsilon + icon->fWidth * (icon->fScale - 1) / 2;
		cairo_translate (pCairoContext, fDockOffsetY, fDockOffsetX);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		//g_print ("gauche : fDockOffsetX : %.2f; fDeltaX:%.2f; fEpsilon:%.2f\n", fDockOffsetX, fDeltaX, fEpsilon);
		
		cairo_rel_line_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, - fDeltaX + fEpsilon);
		cairo_rel_line_to (pCairoContext, 0, - fBigWidth);
		cairo_rel_line_to (pCairoContext, - sens * fHeight, fDeltaX + fEpsilon);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, - fDeltaX - fEpsilon);
		}
	}
}
static void cd_rendering_draw_3D_separator_edge_horizontal (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	int sens;
	double fDockOffsetY;
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = g_iDockLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = - g_iDockLineWidth;
	}
	
	double h = my_fInclinationOnHorizon * (pDock->fFlatDockWidth / 2);
	double fInclination = fabs (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / (pDock->iCurrentWidth / 2);
	///double fInclination = my_fInclinationOnHorizon * fabs ((icon->fDrawX + icon->fWidth * icon->fScale / 2) / pDock->iCurrentWidth - .5) * 2;
	//g_print ("fInclination : %f\n", fInclination);
	double fEpsilon = .1 * icon->fWidth;
	double fDeltaX = pDock->iDecorationsHeight * fInclination;
	double fHeight = pDock->iDecorationsHeight + 2 * g_iDockLineWidth;
	if (fDeltaX + 2 * fEpsilon > icon->fWidth)
	{
		fDeltaX = icon->fWidth - 2 * fEpsilon - 1;
		fHeight = fDeltaX / fInclination;
	}
	double fBigWidth = icon->fWidth - fDeltaX, fLittleWidth = icon->fWidth - fDeltaX - 2 * fEpsilon;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	if (icon->fDrawX + icon->fWidth * icon->fScale / 2 > pDock->iCurrentWidth / 2)  // on est a droite.
	{
		cairo_translate (pCairoContext, 0, fDockOffsetY);  // coin haut droit.
		
		cairo_move_to (pCairoContext, fLittleWidth/pDock->fRatio, 0);
		cairo_rel_line_to (pCairoContext, (fDeltaX + fEpsilon)/pDock->fRatio, sens * fHeight/pDock->fRatio);
		
		cairo_move_to (pCairoContext, 0, 0);
		cairo_rel_line_to (pCairoContext, - (- fDeltaX + fEpsilon)/pDock->fRatio, sens * fHeight/pDock->fRatio);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, 1, -1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], - fEpsilon, 0);
		}
	}
	else  // a gauche.
	{
		cairo_translate (pCairoContext, 0, fDockOffsetY);  // coin haut droit.
		
		cairo_move_to (pCairoContext, fLittleWidth, 0);
		cairo_rel_line_to (pCairoContext, - fDeltaX + fEpsilon, sens * fHeight);
		
		cairo_move_to (pCairoContext, 0, 0);
		cairo_rel_line_to (pCairoContext, - (fDeltaX + fEpsilon), sens * fHeight);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, 1, -1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], - fDeltaX - fEpsilon, 0);
		}
	}
}

static void cd_rendering_draw_3D_separator_edge_vertical (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock)
{
	int sens;
	double fDockOffsetY;
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = - 0*g_iDockLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = + 0*g_iDockLineWidth;
	}
	
	double fInclination = my_fInclinationOnHorizon * fabs ((icon->fDrawX + icon->fWidth * icon->fScale / 2) / pDock->iCurrentWidth - .5) * 2;
	//g_print ("fInclination : %f\n", fInclination);
	double fEpsilon = .1 * icon->fWidth;
	double fDeltaX = pDock->iDecorationsHeight * fInclination;
	double fHeight = pDock->iDecorationsHeight + 2 * g_iDockLineWidth;
	if (fDeltaX + 2 * fEpsilon > icon->fWidth)
	{
		fDeltaX = icon->fWidth - 2 * fEpsilon - 1;
		fHeight = fDeltaX / fInclination;
	}
	double fBigWidth = icon->fWidth - fDeltaX, fLittleWidth = icon->fWidth - fDeltaX - 2 * fEpsilon;
	
	if (icon->fDrawX + icon->fWidth * icon->fScale / 2 > pDock->iCurrentWidth / 2)  // on est a droite.
	{
		cairo_translate (pCairoContext, fDockOffsetY, 0);
		
		cairo_move_to (pCairoContext, 0, fLittleWidth/pDock->fRatio);
		cairo_rel_line_to (pCairoContext, sens * fHeight/pDock->fRatio, (fDeltaX + fEpsilon)/pDock->fRatio);
		
		cairo_move_to (pCairoContext, 0, 0);
		cairo_rel_line_to (pCairoContext,  sens * fHeight/pDock->fRatio, - (- fDeltaX + fEpsilon)/pDock->fRatio);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, - fEpsilon);
		}
	}
	else  // a gauche.
	{
		cairo_translate (pCairoContext, fDockOffsetY, 0);
		
		cairo_move_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, - fDeltaX + fEpsilon);
		
		cairo_move_to (pCairoContext, 0, 0);
		cairo_rel_line_to (pCairoContext, sens * fHeight, - (fDeltaX + fEpsilon));
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, - fDeltaX - fEpsilon);
		}
	}
}

static void cd_rendering_draw_3D_separator (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bHorizontal)
{
	//g_print ("%s ()\n", __func__);
	if (bHorizontal)
		cd_rendering_draw_3D_separator_horizontal (icon, pCairoContext, pDock, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR));
	else
		cd_rendering_draw_3D_separator_vertical (icon, pCairoContext, pDock, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR));
	
	if (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_DEST_OUT);
		cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 1.0);
		cairo_fill (pCairoContext);
		
		if (bHorizontal)
			cd_rendering_draw_3D_separator_edge_horizontal (icon, pCairoContext, pDock);
		else
			cd_rendering_draw_3D_separator_edge_vertical (icon, pCairoContext, pDock);
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
		cairo_set_line_width (pCairoContext, g_iDockLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
	{
		cairo_fill (pCairoContext);
	}
}


void cd_rendering_render_3D_plane (CairoDock *pDock)
{
	//\____________________ On cree le contexte du dessin.
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_set_tolerance (pCairoContext, 0.5);  // avec moins que 0.5 on ne voit pas la difference.
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	//\____________________ On trace le cadre.
	double fChangeAxes = 0.5 * (pDock->iCurrentWidth - pDock->iMaxDockWidth);
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	double fRadius = (pDock->iDecorationsHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDock->iDecorationsHeight + fLineWidth) / 2 - 1);
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + 0 - fMargin : fRadius + fLineWidth / 2);  // fChangeAxes
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iDecorationsHeight + fLineWidth;
	}
	
	cairo_save (pCairoContext);
	
        cairo_dock_draw_frame (pCairoContext, fRadius, 1, fDockWidth, pDock->iDecorationsHeight, fDockOffsetX, fDockOffsetY, sens, my_fInclinationOnHorizon, pDock->bHorizontalDock);  // fLineWidth
	
	//\____________________ On dessine les decorations dedans.
	fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY);
	
	//\____________________ On dessine le cadre.
	if (fLineWidth > 0)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
		cairo_new_path (pCairoContext);
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine la ficelle qui les joint.
	if (g_iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, g_iStringLineWidth, FALSE, (my_pFlatSeparatorSurface != NULL));
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	fRatio = pDock->fRatio;
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement != NULL)
	{
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		do
		{
			icon = ic->data;
			
			cairo_save (pCairoContext);
			
			if (CAIRO_DOCK_IS_SEPARATOR (icon) && icon->acFileName == NULL && (my_pFlatSeparatorSurface[0] != NULL || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR))
				cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock);
			else
				cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE, pDock->iCurrentWidth, pDock->bDirectionUp);
			
			cairo_restore (pCairoContext);
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
	
	
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}



void cd_rendering_render_optimized_3D_plane (CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ((%d;%d) x (%d;%d) / (%dx%d))\n", __func__, pArea->x, pArea->y, pArea->width, pArea->height, pDock->iCurrentWidth, pDock->iCurrentHeight);
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	int iWidth = pDock->iCurrentWidth;
	int iHeight = pDock->iCurrentHeight;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	cairo_rectangle (pCairoContext,
		pArea->x,
		pArea->y,
		pArea->width,
		pArea->height);
	cairo_clip (pCairoContext);
	cairo_set_tolerance (pCairoContext, 0.5);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\____________________ On dessine les decorations du fond sur la portion de fenetre.
	cairo_save (pCairoContext);
	
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bHorizontalDock)
	{
		fDockOffsetX = pArea->x;
		fDockOffsetY = (pDock->bDirectionUp ? iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	}
	else
	{
		fDockOffsetX = (pDock->bDirectionUp ? iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
		fDockOffsetY = pArea->y;
	}
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	if (pDock->bHorizontalDock)
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, pDock->iDecorationsHeight);
	else
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pDock->iDecorationsHeight, pArea->height);
	
	fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY);
	
	
	//\____________________ On dessine la partie du cadre qui va bien.
	cairo_new_path (pCairoContext);
	
	if (pDock->bHorizontalDock)
	{
		cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY - fLineWidth / 2);
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
		
		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, fDockOffsetX, (pDock->bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth));
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	}
	else
	{
		cairo_move_to (pCairoContext, fDockOffsetX - fLineWidth / 2, fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
		
		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, (pDock->bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth), fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	}
	cairo_stroke (pCairoContext);
	
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine les icones impactees.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement != NULL)
	{
		double fXMin = (pDock->bHorizontalDock ? pArea->x : pArea->y), fXMax = (pDock->bHorizontalDock ? pArea->x + pArea->width : pArea->y + pArea->height);
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
		fRatio = pDock->fRatio;
		double fXLeft, fXRight;
		
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		do
		{
			icon = ic->data;
			
			fXLeft = icon->fDrawX + icon->fScale + 1;
			fXRight = icon->fDrawX + (icon->fWidth - 1) * icon->fScale * icon->fWidthFactor - 1;
			
			if (fXLeft <= fXMax && floor (fXRight) > fXMin)
			{
				cairo_save (pCairoContext);
				
				if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->iCurrentWidth)
				{
					icon->fAlpha = 1;
				}
				else
				{
					icon->fAlpha = .25;
				}
				
				if (icon->iAnimationType == CAIRO_DOCK_AVOID_MOUSE)
				{
					icon->fAlpha = 0.4;
				}
				
				if (CAIRO_DOCK_IS_SEPARATOR (icon) && icon->acFileName == NULL && (my_pFlatSeparatorSurface[0] != NULL || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR))
					cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock);
				else
					cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE, pDock->iCurrentWidth, pDock->bDirectionUp);
				cairo_restore (pCairoContext);
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
	
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}


Icon *cd_rendering_calculate_icons_3D_plane (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	CairoDockMousePositionType iMousePositionType = cairo_dock_check_if_mouse_inside_linear (pDock);
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	cairo_dock_mark_avoiding_mouse_icons_linear (pDock);
	
	double fReflectionOffsetY = (pDock->bDirectionUp ? -1 : 1) * g_fReflectSize;
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cd_rendering_calculate_construction_parameters_3D_plane (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, fReflectionOffsetY);
		cairo_dock_manage_animations (icon, pDock);
	}
	
	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
}

void cd_rendering_register_3D_plane_renderer (void)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-3D-plane-view", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-3D-plane.png", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_3D_plane;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_3D_plane;
	pRenderer->render = cd_rendering_render_3D_plane;
	pRenderer->render_optimized = cd_rendering_render_optimized_3D_plane;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pRenderer->bUseReflect = TRUE;
	
	cairo_dock_register_renderer (MY_APPLET_3D_PLANE_VIEW_NAME, pRenderer);
}

