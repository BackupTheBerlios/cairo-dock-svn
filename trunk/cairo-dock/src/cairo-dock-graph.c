/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>

#include <cairo-dock-log.h>
#include <cairo-dock-graph.h>

extern double g_fAmplitude;


void cairo_dock_draw_graph (cairo_t *pCairoContext, CairoDockGraph *pGraph, double fWidth, double fHeight)
{
	g_return_if_fail (pGraph != NULL);
	
	cairo_save (pCairoContext);
	cairo_set_source_rgba (pCairoContext, 0., 0., 0., 0.);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	cairo_set_source_rgba (pCairoContext,
		pGraph->fBackGroundColor[0],
		pGraph->fBackGroundColor[1],
		pGraph->fBackGroundColor[2],
		pGraph->fBackGroundColor[3]);
	cairo_paint (pCairoContext);
	
	cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
		0.,
		0.,
		fHeight);
	g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);	
	
	cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		0.,
		pGraph->fLowColor[0],
		pGraph->fLowColor[1],
		pGraph->fLowColor[2],
		1.);
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		1.,
		pGraph->fHighColor[0],
		pGraph->fHighColor[1],
		pGraph->fHighColor[2],
		1.);
	cairo_set_source (pCairoContext, pGradationPattern);
	
	if (pGraph->iNbValues > 1)
	{
		int j = pGraph->iCurrentIndex + 1;
		if (j >= pGraph->iNbValues)
			j -= pGraph->iNbValues;
		cairo_move_to (pCairoContext, 0, pGraph->pTabValues[j] * fHeight);
		int i;
		for (i = 1; i < pGraph->iNbValues; i ++)
		{
			j = pGraph->iCurrentIndex + i + 1;
			if (j >= pGraph->iNbValues)
				j -= pGraph->iNbValues;
			cairo_line_to (pCairoContext,
				i * fWidth / pGraph->iNbValues,
				pGraph->pTabValues[j] * fHeight);
		}
		cairo_stroke (pCairoContext);
	}
	
	cairo_pattern_destroy (pGradationPattern);
	cairo_restore (pCairoContext);
}


CairoDockGraph* cairo_dock_create_graph (int iNbValues, CairoDockTypeGraph iType, gdouble *pLowColor, gdouble *pHighColor, gdouble *pBackGroundColor)
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
	return pGraph;
}

void cairo_dock_free_graph (CairoDockGraph *pGraph)
{
	if (pGraph == NULL)
		return ;
	g_free (pGraph->pTabValues);
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
