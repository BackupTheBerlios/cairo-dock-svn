/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include "math.h"
#include <cairo-dock.h>

#include "rendering-desklet-caroussel.h"

#define CAROUSSEL_RATIO_ICON_DESKLET .5


static gboolean _caroussel_rotate (CairoDockDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return FALSE;
	pCaroussel->iRotationCount += (pCaroussel->iRotationDirection == GDK_SCROLL_UP ? 1 : -1);
	pCaroussel->fRotationAngle += (pCaroussel->iRotationDirection == GDK_SCROLL_UP ? 1 : -1) * pCaroussel->fDeltaTheta / 10;
	if (pCaroussel->fRotationAngle >= 2*G_PI)
		pCaroussel->fRotationAngle -= 2*G_PI;
	else if (pCaroussel->fRotationAngle < 0)
		pCaroussel->fRotationAngle += 2*G_PI;
	gtk_widget_queue_draw (pDesklet->pWidget);
	if (abs (pCaroussel->iRotationCount) >= 10 || pCaroussel->iRotationCount == 0)
	{
		pCaroussel->iRotationCount = 0;
		pCaroussel->iSidRotation = 0;
		return FALSE;
	}
	else
		return TRUE;
}
static gboolean on_scroll_desklet (GtkWidget* pWidget,
	GdkEventScroll* pScroll,
	CairoDockDesklet *pDesklet)
{
	if (pDesklet->icons != NULL && (pScroll->direction == GDK_SCROLL_DOWN || pScroll->direction == GDK_SCROLL_UP))
	{
		CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
		if (pCaroussel == NULL)
			return FALSE;
		
		if (pCaroussel->iSidRotation == 0)
		{
			pCaroussel->iRotationDirection = pScroll->direction;
			pCaroussel->iSidRotation = g_timeout_add (50, (GSourceFunc) _caroussel_rotate, (gpointer) pCaroussel);
		}
		else
		{
			if (pScroll->direction == pCaroussel->iRotationDirection)
			{
				pCaroussel->iRotationCount = 0;
			}
			else
			{
				pCaroussel->iRotationDirection = pScroll->direction;
			}
		}
		_caroussel_rotate (NULL);
	}
	return FALSE;
}
CDCarousselParameters *rendering_load_caroussel (CairoDockDesklet *pDesklet, cairo_t *pSourceContext, gboolean bRotateIconsOnEllipse, gboolean b3D)
{
	g_print ("%s ()\n", __func__);
	GList *pIconsList = pDesklet->icons;
	if (pIconsList == NULL)
		return NULL;
	
	CDCarousselParameters *pCaroussel = g_new0 (CDCarousselParameters, 1);
	double fCentralSphereWidth, fCentralSphereHeight;
	if (b3D)
	{
		fCentralSphereWidth = MAX (1, MIN (pDesklet->iWidth, pDesklet->iHeight) * CAROUSSEL_RATIO_ICON_DESKLET);
		fCentralSphereHeight = fCentralSphereWidth;
	}
	else
	{
		fCentralSphereWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
		fCentralSphereHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
	}
	
	int iNbIcons = g_list_length (pIconsList);
	pCaroussel->fDeltaTheta = 2 * G_PI / iNbIcons;
	
	pCaroussel->iEllipseHeight = MIN (fCentralSphereHeight, pDesklet->iHeight - 2 * (g_iLabelSize + g_fReflectSize) - 1);
	pCaroussel->fInclinationOnHorizon = atan2 (pDesklet->iHeight, pDesklet->iWidth/4);
	pCaroussel->iFrameHeight = pCaroussel->iEllipseHeight + 0*2 * g_iFrameMargin + g_fReflectSize;
	pCaroussel->fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pCaroussel->iFrameHeight, pCaroussel->fInclinationOnHorizon, g_iDockRadius, g_iDockLineWidth);
	
	int iMaxIconWidth = 0;
	Icon *icon;
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		iMaxIconWidth = MAX (iMaxIconWidth, icon->fWidth);
	}
	pCaroussel->a = MAX (pDesklet->iWidth - pCaroussel->fExtraWidth - (bRotateIconsOnEllipse ? 0 : iMaxIconWidth/2), pCaroussel->iEllipseHeight)/2;
	pCaroussel->b = MIN (pDesklet->iWidth - pCaroussel->fExtraWidth - (bRotateIconsOnEllipse ? 0 : iMaxIconWidth/2), pCaroussel->iEllipseHeight)/2;  // c = sqrt (a * a - b * b) ; e = c / a.
	
	pCaroussel->bRotateIconsOnEllipse = bRotateIconsOnEllipse;
	pCaroussel->b3D = b3D;
	
	gulong iOnScrollCallbackID = g_signal_handler_find (pDesklet->pWidget,
		G_SIGNAL_MATCH_FUNC,
		0,
		0,
		NULL,
		on_scroll_desklet,
		NULL);
	if (iOnScrollCallbackID == 0)
		g_signal_connect (G_OBJECT (pDesklet->pWidget),
			"scroll-event",
			G_CALLBACK (on_scroll_desklet),
			pDesklet);
	
	return pCaroussel;
}


void rendering_free_caroussel_parameters (CDCarousselParameters *pCaroussel, gboolean bFree)
{
	if (pCaroussel == NULL)
		return ;
	
	if (bFree)
		g_free (pCaroussel);
	else
		memset (pCaroussel, 0, sizeof (CDCarousselParameters));
}


void rendering_load_icons_for_caroussel_desklet (CairoDockDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	if (pDesklet->pIcon != NULL)
	{
		if (pCaroussel->b3D)
		{
			pDesklet->pIcon->fWidth = MAX (1, MIN (pDesklet->iWidth, pDesklet->iHeight) * CAROUSSEL_RATIO_ICON_DESKLET);
			pDesklet->pIcon->fHeight = pDesklet->pIcon->fWidth;
		}
		else
		{
			pDesklet->pIcon->fWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
			pDesklet->pIcon->fHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
		}
		
		pDesklet->pIcon->fDrawX = (pDesklet->iWidth - pDesklet->pIcon->fWidth) / 2;
		pDesklet->pIcon->fDrawY = (pDesklet->iHeight - pDesklet->pIcon->fHeight) / 2 + (pCaroussel->b3D ? g_iLabelSize : 0);
		pDesklet->pIcon->fScale = 1.;
		pDesklet->pIcon->fAlpha = 1.;
		pDesklet->pIcon->fWidthFactor = 1.;
		pDesklet->pIcon->fHeightFactor = 1.;
		cairo_dock_load_one_icon_from_scratch (pDesklet->pIcon, CAIRO_DOCK_CONTAINER (pDesklet));
	}
	GList* ic;
	Icon *icon;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDesklet));
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (pCaroussel->b3D)
		{
			icon->fWidth = 0;
			icon->fHeight = 0;
		}
		else
		{
			icon->fWidth = MAX (1, .2 * pDesklet->iWidth - g_iLabelSize);
			icon->fHeight = MAX (1, .2 * pDesklet->iHeight - g_iLabelSize);
		}
		cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, CAIRO_DOCK_HORIZONTAL, pCaroussel->b3D);  // en 3D on charge les reflets.
	}
	cairo_destroy (pCairoContext);
}



void rendering_draw_caroussel_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	double fTheta = G_PI/2 + pCaroussel->fRotationAngle, fDeltaTheta = pCaroussel->fDeltaTheta;
	
	int iEllipseHeight = pCaroussel->iEllipseHeight;
	double fInclinationOnHorizon = pCaroussel->fInclinationOnHorizon;
	
	int iFrameHeight = pCaroussel->iFrameHeight;
	double fExtraWidth = pCaroussel->fExtraWidth;
	double a = pCaroussel->a, b = pCaroussel->b;
	
	Icon *pIcon;
	GList *ic;
	if (pCaroussel->b3D)
	{
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			
			if (fTheta > G_PI && fTheta < 2*G_PI)  // arriere-plan.
			{
				pIcon->fScale = (1 + .5 * fabs (fTheta - 3 * G_PI / 2) / (G_PI / 2)) / 1.5;
				pIcon->fAlpha = pIcon->fScale;
			}
			else
			{
				pIcon->fScale = 1.;
				pIcon->fAlpha = 1.;
			}
			pIcon->fDrawX = pDesklet->iWidth / 2 + a * cos (fTheta) - pIcon->fWidth/2 * 1;
			pIcon->fDrawY = pDesklet->iHeight / 2 + b * sin (fTheta) - pIcon->fHeight * pIcon->fScale + g_iLabelSize;
			
			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
		
		//\____________________ On trace le cadre.
		double fLineWidth = g_iDockLineWidth;
		double fMargin = 0*g_iFrameMargin;
		
		double fDockWidth = pDesklet->iWidth - fExtraWidth;
		int sens=1;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = fExtraWidth / 2;
		fDockOffsetY = (pDesklet->iHeight - iEllipseHeight) / 2 + g_iLabelSize;
		
		cairo_save (pCairoContext);
		cairo_dock_draw_frame (pCairoContext, g_iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclinationOnHorizon, pDesklet->bIsHorizontal);
		
		//\____________________ On dessine les decorations dedans.
		cairo_save (pCairoContext);
		double fColor[4];
		int i;
		for (i = 0; i < 4; i ++)
		{
			fColor[i] = (g_fDeskletColorInside[i] * pDesklet->iGradationCount + g_fDeskletColor[i] * (CD_NB_ITER_FOR_GRADUATION - pDesklet->iGradationCount)) / CD_NB_ITER_FOR_GRADUATION;
		}
		cairo_set_source_rgba (pCairoContext, fColor[0], fColor[1], fColor[2], .75);
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
		
		//\____________________ On dessine le cadre.
		if (fLineWidth > 0)
		{
			cairo_set_line_width (pCairoContext, fLineWidth);
			cairo_set_source_rgba (pCairoContext, fColor[0], fColor[1], fColor[2], 1.);
			cairo_stroke (pCairoContext);
		}
		cairo_restore (pCairoContext);
		
		//\____________________ On dessine les icones dans l'ordre qui va bien.
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight < pDesklet->iHeight / 2 + g_iLabelSize && pIcon->fDrawX + pIcon->fWidth/2 > pDesklet->iWidth / 2)  // arriere-plan droite.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight < pDesklet->iHeight / 2 + g_iLabelSize && pIcon->fDrawX + pIcon->fWidth/2 <= pDesklet->iWidth / 2)  // arriere-plan gauche.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
		
		cairo_save (pCairoContext);
		pDesklet->pIcon->fDrawY = pDesklet->iHeight/2 - pDesklet->pIcon->fHeight + g_iLabelSize;
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, TRUE, FALSE, pDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight >= pDesklet->iHeight / 2 + g_iLabelSize && pIcon->fDrawX + pIcon->fWidth/2 > pDesklet->iWidth / 2)  // avant-plan droite.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
			
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight >= pDesklet->iHeight / 2 + g_iLabelSize && pIcon->fDrawX + pIcon->fWidth/2 <= pDesklet->iWidth / 2)  // avant-plan gauche.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
	}
	else
	{
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, FALSE, FALSE, pDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		gboolean bFlip = (pDesklet->pIcon->fHeight > pDesklet->pIcon->fWidth);
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				pIcon->fDrawX = pDesklet->pIcon->fDrawX + pDesklet->pIcon->fWidth / 2 + (bFlip ? b : a) * cos (fTheta) - pIcon->fWidth/2;
				pIcon->fDrawY = pDesklet->pIcon->fDrawY + pDesklet->pIcon->fHeight / 2 + (bFlip ? a : b) * sin (fTheta) - pIcon->fHeight/2 + g_iLabelSize;
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
	}
}