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
#include <cairo-dock-renderer-manager.h>
#include <cairo-dock-draw-opengl.h>
#include <cairo-dock-draw.h>
#include <cairo-dock-internal-labels.h>
#include <cairo-dock-internal-icons.h>
#include <cairo-dock-internal-background.h>
#include "cairo-dock-container.h"
#include <cairo-dock-graph2.h>

extern gboolean g_bUseOpenGL;


void cairo_dock_render_graph2 (CairoDockGraph2 *pGraph, cairo_t *pCairoContext)
{
	g_return_if_fail (pGraph != NULL && pCairoContext != NULL);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	CairoDataRenderer *pRenderer = CAIRO_DATA_RENDERER (pGauge);
	CairoDataToRenderer *pData = cairo_data_renderer_get_data (pRenderer);
	
	if (pGraph->pBackgroundSurface != NULL)
	{
		cairo_set_source_surface (pCairoContext, pGraph->pBackgroundSurface, 0., 0.);
		cairo_paint (pCairoContext);
	}
	
	int iNbDrawings = pData->iNbValues / pRenderer->iRank;
	if (iNbDrawings == 0)
		return;
	
	double fMargin = pGraph->fMargin;
	double fWidth = pRenderer->iWidth - 2*fMargin;
	double fHeight = pRenderer->iHeight - 2*fMargin;
	fHeight /= pRenderer->iRank;
	
	double fValue;
	cairo_pattern_t *pGradationPattern;
	int i;
	for (i = 0; i < pData->iNbValues; i ++)
	{
		if (! pGraph->bMixGraphs)
			cairo_translate (pCairoContext,
				0.,
				i * fHeight);
		pGradationPattern = pGraph->pGradationPatterns[i];
		if (pGradationPattern != NULL)
			cairo_set_source (pCairoContext, pGradationPattern);
		else
			cairo_set_source_rgb (pCairoContext,
				pGraph->fLowColor[3*i+0],
				pGraph->fLowColor[3*i+1],
				pGraph->fLowColor[3*i+2]);
		
		if (pGraph->iType == CAIRO_DOCK_GRAPH2_LINE || pGraph->iType == CAIRO_DOCK_GRAPH2_PLAIN)
		{
			cairo_set_line_width (pCairoContext, 1);
			cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
			fValue = cairo_data_renderer_get_normalized_current_value (pRenderer, i);
			cairo_move_to (pCairoContext, fMargin + fWidth, fMargin + (1 - fValue) * fHeight);
			int t, n = pData->iMemorySize - 1;
			for (t = 1; t < pData->iMemorySize; t ++)
			{
				fValue = cairo_data_renderer_get_normalized_value (pRenderer, i, t);
				cairo_line_to (pCairoContext,
					fMargin + (n - i) * fWidth / n,
					fMargin + (1 - fValue) * fHeight);
			}
			if (pGraph->iType == CAIRO_DOCK_GRAPH2_PLAIN)
			{
				cairo_line_to (pCairoContext,
					fMargin,
					fMargin + fHeight);
				cairo_line_to (pCairoContext,
					fMargin + fWidth,
					fMargin + fHeight);
				cairo_close_path (pCairoContext);
				cairo_fill_preserve (pCairoContext);
			}
			cairo_stroke (pCairoContext);
		}
		else if (pGraph->iType == CAIRO_DOCK_GRAPH2_BAR)
		{
			double fBarWidth = fWidth / pData->iNbValues / 4;
			cairo_set_line_width (pCairoContext, fBarWidth);
			int t, n = pData->iMemorySize - 1;
			for (t = 0; t < pData->iMemorySize; t ++)
			{
				fValue = cairo_data_renderer_get_normalized_value (pRenderer, i, t);
				cairo_move_to (pCairoContext,
					fMargin + (n - i) * fWidth / n,
					fMargin + fHeight);
				cairo_rel_line_to (pCairoContext,
					0.,
					- fValue * fHeight);
				cairo_stroke (pCairoContext);
			}
		}
		else
		{
			cairo_set_line_width (pCairoContext, 1);
			cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
			fValue = cairo_data_renderer_get_normalized_current_value (pRenderer, i);
			double angle, radius = MIN (fWidth, fHeight)/2;
			angle = 2*G_PI*(-.5/pData->iMemorySize);
			cairo_move_to (pCairoContext,
				fMargin + fWidth/2 + radius * (fValue * cos (angle)),
				fMargin + fHeight/2 + radius * (fValue * sin (angle)));
			angle = 2*G_PI*(.5/pGraph->iNbValues);
			cairo_line_to (pCairoContext,
				fMargin + fWidth/2 + radius * (pTabValues[j] * cos (angle)),
				fMargin + fHeight/2 + radius * (pTabValues[j] * sin (angle)));
			int t;
			for (t = 1; t < pData->iMemorySize; t ++)
			{
				fValue = cairo_data_renderer_get_normalized_value (pRenderer, i, t);
				angle = 2*G_PI*((t-.5)/pData->iMemorySize);
				cairo_line_to (pCairoContext,
					fMargin + fWidth/2 + radius * (fValue * cos (angle)),
					fMargin + fHeight/2 + radius * (fValue * sin (angle)));
				angle = 2*G_PI*((t+.5)/pData->iMemorySize);
				cairo_line_to (pCairoContext,
					fMargin + fWidth/2 + radius * (fValue * cos (angle)),
					fMargin + fHeight/2 + radius * (fValue * sin (angle)));
			}
			if (pGraph->iType == CAIRO_DOCK_GRAPH2_CIRCLE_PLAIN)
			{
				cairo_close_path (pCairoContext);
				cairo_fill_preserve (pCairoContext);
			}
			cairo_stroke (pCairoContext);
		}
	}
}


static cairo_surface_t *_cairo_dock_create_graph2_background (cairo_t *pSourceContext, double fWidth, double fHeight, int iRadius, double fMargin, gdouble *pBackGroundColor, CairoDockTypeGraph2 iType)
{
	// on cree la surface.
	cairo_surface_t *pBackgroundSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		fWidth,
		fHeight);
	cairo_t *pCairoContext = cairo_create (pBackgroundSurface);
	
	// on trace le fond : un rectangle au coin arrondi.
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
	cairo_stroke (pCairoContext);  // on trace d'abord les contours arrondis.
	
	cairo_rectangle (pCairoContext, fRadius, fRadius, (fWidth - 2*fRadius), (fHeight - 2*fRadius));
	cairo_fill (pCairoContext);  // puis on rempli l'interieur.
	
	// on trace les axes.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgb (pCairoContext,
		myLabels.quickInfoTextDescription.fBackgroundColor[0],
		myLabels.quickInfoTextDescription.fBackgroundColor[1],
		myLabels.quickInfoTextDescription.fBackgroundColor[2]);  // meme couleur que le fond des info-rapides, mais opaque.
	cairo_set_line_width (pCairoContext, 1.);
	if (iType == CAIRO_DOCK_GRAPH2_CIRCLE || iType == CAIRO_DOCK_GRAPH2_CIRCLE_PLAIN)
	{
		cairo_arc (pCairoContext,
			fWidth/2,
			fHeight/2,
			MIN (fWidth, fHeight)/2 - fMargin,
			0.,
			360.);
	}
	else
	{
		cairo_move_to (pCairoContext, fMargin, fMargin);
		cairo_rel_line_to (pCairoContext, 0., fHeight - 2*fMargin);
		cairo_rel_line_to (pCairoContext, fWidth - 2*fMargin, 0.);
	}
	cairo_stroke (pCairoContext);
	
	cairo_destroy (pCairoContext);
	return  pBackgroundSurface;
}
static cairo_pattern_t *_cairo_dock_create_graph2_pattern (CairoDockGraph2 *pGraph, gdouble *fLowColor, gdouble *fHighColor, double fOffsetY)
{
	cairo_pattern_t *pGradationPattern = NULL;
	if (fLowColor[0] != fHighColor[0] || fLowColor[1] != fHighColor[1] || fLowColor[2] != fHighColor[2])  // un degrade existe.
	{
		double fMargin = pGraph->fMargin;
		double fWidth = pGraph->fWidth - 2*fMargin;
		double fHeight = pGraph->fHeight - 2*fMargin;
		fHeight /= pGraph->dataRenderer.iRank;
		
		if (pGraph->iType == CAIRO_DOCK_GRAPH2_CIRCLE || pGraph->iType == CAIRO_DOCK_GRAPH2_CIRCLE_PLAIN)
		{
			double radius = MIN (fWidth, fHeight)/2;
			pGradationPattern = cairo_pattern_create_radial (fMargin + radius,
				fMargin + radius + fOffsetY,
				0.,
				fMargin + radius,
				fMargin + radius + fOffsetY,
				radius);
		}
		else
			pGradationPattern = cairo_pattern_create_linear (0.,
				fMargin + fHeight + fOffsetY,
				0.,
				fMargin + fOffsetY);
		g_return_val_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS, NULL);	
		
		cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			0.,
			fLowColor[0],
			fLowColor[1],
			fLowColor[2],
			1.);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			1.,
			fHighColor[0],
			fHighColor[1],
			fHighColor[2],
			1.);
	}
	return pGradationPattern;
}

void cairo_dock_load_graph2 (CairoDockGraph2 *pGraph, cairo_t *pSourceContext, CairoContainer *pContainer, CairoGraph2Attribute *pAttribute)
{
	int iWidth = pGauge->dataRenderer.iWidth, iHeight = pGauge->dataRenderer.iHeight;
	if (iWidth == 0 || iHeight == 0)
		return ;
	
	pGraph->iType = pAttribute->iType;
	pGraph->dataRenderer.iRank = (pAttribute->bMixGraphs ? pGraph->dataRenderer.iNbValues : 1);
	
	pGraph->fHighColor = g_new0 (double, 3 * pGauge->dataRenderer.iNbValues);
	if (pAttribute->fHighColor != NULL)
		memcpy (pGraph->fHighColor, pAttribute->fHighColor, 3 * pGauge->dataRenderer.iNbValues * sizeof (double));
	pGraph->fLowColor = g_new0 (double, 3 * pGauge->dataRenderer.iNbValues);
	if (pAttribute->fLowColor != NULL)
		memcpy (pGraph->fLowColor, pAttribute->fLowColor, 3 * pGauge->dataRenderer.iNbValues * sizeof (double));
	pGraph->fBackGroundColor = g_new0 (double, 4);
	int i;
	for (i = 0; i < pGraph->pDataRenderer.iNbValues; i ++)
	{
		pGraph->pGradationPatterns[i] = _cairo_dock_create_graph2_pattern (pGraph,
			&pGraph->fLowColor[3*i],
			&pGraph->fHighColor[3*i],
			0.);
	}
	
	pGraph->iRadius = pAttribute->iRadius;
	pGraph->fMargin = pGraph->iRadius * (1. - sqrt(2)/2);
	if (pAttribute->fBackGroundColor != NULL)
		memcpy (pGraph->fBackGroundColor, pAttribute->fBackGroundColor, 4 * sizeof (double));
	pGraph->pBackgroundSurface = _cairo_dock_create_graph2_background (pSourceContext,
		iWidth,
		iHeight,
		pGraph->iRadius,
		pGraph->fMargin,
		pGraph->fBackGroundColor,
		pGraph->iType);
}

void cairo_dock_reload_graph2 (CairoDockGraph2 *pGraph, cairo_t *pSourceContext)
{
	int iWidth = pGauge->dataRenderer.iWidth, iHeight = pGauge->dataRenderer.iHeight;
	if (pGraph->pBackgroundSurface != NULL)
		cairo_surface_destroy (pGraph->pBackgroundSurface);
	pGraph->pBackgroundSurface = _cairo_dock_create_graph2_background (pSourceContext, iWidth, iHeight, pGraph->iRadius, pGraph->fMargin, pGraph->fBackGroundColor, pGraph->iType);
	
	for (i = 0; i < pGraph->pDataRenderer.iNbValues; i ++)
	{
		if (pGraph->pGradationPatterns[i] != NULL)
			cairo_pattern_destroy (pGraph->pGradationPattern[i]);
		pGraph->pGradationPatterns[i] = _cairo_dock_create_graph2_pattern (pGraph, &pGraph->fLowColor[3*i], &pGraph->fHighColor[3*i], 0.);
	}
}


void cairo_dock_free_graph2 (CairoDockGraph2 *pGraph)
{
	cd_debug ("");
	if (pGraph == NULL)
		return ;
	if (pGraph->pBackgroundSurface != NULL)
		cairo_surface_destroy (pGraph->pBackgroundSurface);
	int i;
	for (i = 0; i < pGraph->pDataRenderer.iNbValues; i ++)
	{
		if (pGraph->pGradationPattern[i] != NULL)
			cairo_pattern_destroy (pGraph->pGradationPattern[i]);
	}
	
	g_free (pGraph->fHighColor);
	g_free (pGraph->fLowColor);
	g_free (pGraph->fBackGroundColor);
	g_free (pGraph);
}



void cairo_dock_add_watermark_on_graph2 (cairo_t *pSourceContext, CairoDockGraph2 *pGraph, gchar *cImagePath, double fAlpha)
{
	/*g_return_if_fail (pGraph != NULL && pGraph->pBackgroundSurface != NULL && cImagePath != NULL);
	
	cairo_surface_t *pWatermarkSurface = cairo_dock_create_surface_for_icon (cImagePath, pSourceContext, pGraph->fWidth/2, pGraph->fHeight/2);
	
	cairo_t *pCairoContext = cairo_create (pGraph->pBackgroundSurface);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	cairo_set_source_surface (pCairoContext, pWatermarkSurface, pGraph->fWidth/4, pGraph->fHeight/4);
	cairo_paint_with_alpha (pCairoContext, fAlpha);
	
	cairo_destroy (pCairoContext);
	
	cairo_surface_destroy (pWatermarkSurface);*/
}


  //////////////////////////////////////////
 /////////////// RENDERER /////////////////
//////////////////////////////////////////
CairoDockGraph2 *cairo_dock_new_graph2 (void)
{
	CairoDockGraph2 *pGraph = g_new0 (CairoDockGraph2, 1);
	pGauge->dataRenderer.interface.new				= cairo_dock_new_graph2;
	pGauge->dataRenderer.interface.load				= cairo_dock_load_graph2;
	pGauge->dataRenderer.interface.render			= cairo_dock_render_graph2;
	pGauge->dataRenderer.interface.render_opengl	= NULL;
	pGauge->dataRenderer.interface.free				= cairo_dock_free_graph2;
	pGauge->dataRenderer.interface.reload			= cairo_dock_reload_graph2;
	return pGraph;
}