/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cairo-dock-log.h>
#include <cairo-dock-surface-factory.h>
#include <cairo-dock-dock-factory.h>
#include <cairo-dock-draw.h>
#include <cairo-dock-graph.h>

extern int g_iDockRadius;
extern double g_fAmplitude;
extern CairoDockLabelDescription g_quickInfoTextDescription;

void cairo_dock_draw_graph (cairo_t *pCairoContext, CairoDockGraph *pGraph)
{
	g_return_if_fail (pGraph != NULL);
	
	cairo_set_source_rgba (pCairoContext, 0., 0., 0., 0.);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	if (pGraph->pBackgroundSurface != NULL)
	{
		cairo_set_source_surface (pCairoContext, pGraph->pBackgroundSurface, 0., 0.);
		cairo_paint (pCairoContext);
	}
	
	if (pGraph->iNbValues > 1)
	{
		double fMargin = pGraph->iRadius * (1. - sqrt(2)/2);
		double fWidth = pGraph->fWidth - 2*fMargin;
		double fHeight = pGraph->fHeight - 2*fMargin;
		
		cairo_pattern_t *pGradationPattern = NULL;
		if (pGraph->fLowColor[0] != pGraph->fHighColor[0] || pGraph->fLowColor[1] != pGraph->fHighColor[1] || pGraph->fLowColor[2] != pGraph->fHighColor[2])
		{
			pGradationPattern = cairo_pattern_create_linear (0.,
				0.,
				0.,
				fHeight);
			g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);	
			
			cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
			cairo_pattern_add_color_stop_rgba (pGradationPattern,
				0.,
				pGraph->fHighColor[0],
				pGraph->fHighColor[1],
				pGraph->fHighColor[2],
				1.);
			cairo_pattern_add_color_stop_rgba (pGradationPattern,
				1.,
				pGraph->fLowColor[0],
				pGraph->fLowColor[1],
				pGraph->fLowColor[2],
				1.);
			cairo_set_source (pCairoContext, pGradationPattern);
		}
		else
		{
			cairo_set_source_rgb (pCairoContext,
				pGraph->fLowColor[0],
				pGraph->fLowColor[1],
				pGraph->fLowColor[2]);
		}
		
		cairo_set_line_width (pCairoContext, 1);
		cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
		int j = pGraph->iCurrentIndex + 1;
		if (j >= pGraph->iNbValues)
			j -= pGraph->iNbValues;
		cairo_move_to (pCairoContext, fMargin, fMargin + (1 - pGraph->pTabValues[j]) * fHeight);
		int i;
		for (i = 1; i < pGraph->iNbValues; i ++)
		{
			j = pGraph->iCurrentIndex + i + 1;
			if (j >= pGraph->iNbValues)
				j -= pGraph->iNbValues;
			cairo_line_to (pCairoContext,
				fMargin + i * fWidth / pGraph->iNbValues,
				(1 - pGraph->pTabValues[j]) * fHeight);
		}
		cairo_stroke (pCairoContext);
		
		if (pGradationPattern != NULL)
			cairo_pattern_destroy (pGradationPattern);
	}
}

void cairo_dock_render_graph (cairo_t *pSourceContext, CairoContainer *pContainer, Icon *pIcon, CairoDockGraph *pGraph)
{
	cairo_save (pSourceContext);
	
	cairo_dock_draw_graph (pSourceContext, pGraph);
	
	cairo_restore (pSourceContext);
	
	// On cree le reflet.
	if (CAIRO_DOCK_IS_DOCK (pContainer) && CAIRO_DOCK (pContainer)->bUseReflect)
	{
		double fMaxScale = cairo_dock_get_max_scale (pContainer);
		
		cairo_surface_destroy (pIcon->pReflectionBuffer);  // pour aller plus vite on decide de ne creer que le minimum.
		pIcon->pReflectionBuffer = NULL;
		if (pIcon->pFullIconBuffer != NULL)
		{
			cairo_surface_destroy (pIcon->pFullIconBuffer);
			pIcon->pFullIconBuffer = NULL;
		}
		
		pIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (pIcon->pIconBuffer,
			pSourceContext,
			(pContainer->bIsHorizontal ? pIcon->fWidth : pIcon->fHeight) * fMaxScale,
			(pContainer->bIsHorizontal ? pIcon->fHeight : pIcon->fWidth) * fMaxScale,
			pContainer->bIsHorizontal,
			fMaxScale,
			pContainer->bDirectionUp);
	}
	
	cairo_dock_redraw_my_icon (pIcon, pContainer);
}

static cairo_surface_t *_cairo_dock_create_graph_background (cairo_t *pSourceContext, double fWidth, double fHeight, int iRadius, gdouble *pBackGroundColor)
{
	cairo_surface_t *pBackgroundSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fWidth,
		fHeight);
	cairo_t *pCairoContext = cairo_create (pBackgroundSurface);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgba (pCairoContext,
		pBackGroundColor[0],
		pBackGroundColor[1],
		pBackGroundColor[2],
		pBackGroundColor[3]);
	
	double fRadius = iRadius;
	cairo_set_line_width (pCairoContext, fRadius);
	cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
	cairo_move_to (pCairoContext, .5*fRadius, .5*fRadius);
	cairo_rel_line_to (pCairoContext, fWidth - (fRadius), 0);
	cairo_rel_line_to (pCairoContext, 0, fHeight - (fRadius));
	cairo_rel_line_to (pCairoContext, -(fWidth - (fRadius)) ,0);
	cairo_close_path (pCairoContext);
	cairo_stroke (pCairoContext);
	
	cairo_rectangle (pCairoContext, fRadius, fRadius, (fWidth - 2*fRadius), (fHeight - 2*fRadius));
	cairo_fill (pCairoContext);
	
	
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	double fMargin = fRadius * (1. - sqrt(2)/2);
	cairo_set_source_rgb (pCairoContext, g_quickInfoTextDescription.fBackgroundColor[0], g_quickInfoTextDescription.fBackgroundColor[1], g_quickInfoTextDescription.fBackgroundColor[2]);  // meme couleur que le fond des info-rapides.
	cairo_set_line_width (pCairoContext, 1.);
	cairo_move_to (pCairoContext, fMargin, fMargin);
	cairo_rel_line_to (pCairoContext, 0., fHeight - 2*fMargin);
	cairo_rel_line_to (pCairoContext, fWidth - 2*fMargin, 0.);
	cairo_stroke (pCairoContext);
	
	
	cairo_destroy (pCairoContext);
	return  pBackgroundSurface;
}

CairoDockGraph* cairo_dock_create_graph (cairo_t *pSourceContext, int iNbValues, CairoDockTypeGraph iType, double fWidth, double fHeight, gdouble *pLowColor, gdouble *pHighColor, gdouble *pBackGroundColor)
{
	g_return_val_if_fail (iNbValues > 0, NULL);
	CairoDockGraph *pGraph = g_new0 (CairoDockGraph, 1);
	pGraph->iNbValues = iNbValues;
	pGraph->pTabValues = g_new0 (double, iNbValues);
	pGraph->iType = iType;
	
	if (pLowColor != NULL)
		memcpy (pGraph->fLowColor, pLowColor, sizeof (pGraph->fLowColor));
	if (pHighColor != NULL)
		memcpy (pGraph->fHighColor, pHighColor, sizeof (pGraph->fHighColor));
	if (pBackGroundColor != NULL)
		memcpy (pGraph->fBackGroundColor, pBackGroundColor, sizeof (pGraph->fBackGroundColor));
	
	pGraph->fWidth = fWidth;
	pGraph->fHeight = fHeight;
	pGraph->iRadius = g_iDockRadius;  // memes arrondis que le dock et les desklets.
	pGraph->pBackgroundSurface = _cairo_dock_create_graph_background (pSourceContext, fWidth, fHeight, g_iDockRadius, pGraph->fBackGroundColor);
	
	return pGraph;
}

void cairo_dock_reload_graph (cairo_t *pSourceContext, CairoDockGraph *pGraph, int iWidth, int iHeight)
{
	if (pGraph->pBackgroundSurface != NULL)
		cairo_surface_destroy (pGraph->pBackgroundSurface);
	pGraph->fWidth = iWidth;
	pGraph->fHeight = iHeight;
	pGraph->iRadius = g_iDockRadius;
	pGraph->pBackgroundSurface = _cairo_dock_create_graph_background (pSourceContext, pGraph->fWidth, pGraph->fHeight, pGraph->iRadius, pGraph->fBackGroundColor);
}


void cairo_dock_free_graph (CairoDockGraph *pGraph)
{
	cd_debug ("");
	if (pGraph == NULL)
		return ;
	g_free (pGraph->pTabValues);
	if (pGraph->pBackgroundSurface != NULL)
		cairo_surface_destroy (pGraph->pBackgroundSurface);
	g_free (pGraph);
}


void cairo_dock_update_graph (CairoDockGraph *pGraph, double fNewValue)
{
	g_return_if_fail (pGraph != NULL && pGraph->iNbValues > 0);
	fNewValue = MIN (MAX (fNewValue, 0.), 1.);
	pGraph->iCurrentIndex += 1;
	if (pGraph->iCurrentIndex >= pGraph->iNbValues)
		pGraph->iCurrentIndex -= pGraph->iNbValues;
	
	pGraph->pTabValues[pGraph->iCurrentIndex] = fNewValue;
}


void cairo_dock_add_watermark_on_graph (cairo_t *pSourceContext, CairoDockGraph *pGraph, gchar *cImagePath, double fAlpha)
{
	g_return_if_fail (pGraph != NULL && pGraph->pBackgroundSurface != NULL && cImagePath != NULL);
	
	cairo_surface_t *pWatermarkSurface = cairo_dock_create_surface_for_icon (cImagePath, pSourceContext, pGraph->fWidth/2, pGraph->fHeight/2);
	
	cairo_t *pCairoContext = cairo_create (pGraph->pBackgroundSurface);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	cairo_set_source_surface (pCairoContext, pWatermarkSurface, pGraph->fWidth/4, pGraph->fHeight/4);
	cairo_paint_with_alpha (pCairoContext, fAlpha);
	
	cairo_destroy (pCairoContext);
	
	cairo_surface_destroy (pWatermarkSurface);
}
