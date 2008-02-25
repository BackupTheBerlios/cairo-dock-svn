/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include "math.h"
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-read-data.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;
gboolean my_bRotateIconsOnEllipse = TRUE;


void cd_weather_draw_in_desklet (cairo_t *pCairoContext, gpointer data)
{
	if (myConfig.bDesklet3D)
	{
		int iNumIcon=0, iNbIcons = g_list_length (myData.pDeskletIconList);
		cd_debug ("%d icones a dessiner", iNbIcons);
		double fTheta = G_PI/2, fDeltaTheta = 2 * G_PI / iNbIcons;
		
		int iEllipseHeight = MIN (myIcon->fHeight, myDesklet->iHeight - 2 * (g_iLabelSize + g_fReflectSize) - 1);
		double fInclinationOnHorizon = atan2 (myDesklet->iHeight, myDesklet->iWidth/4);
		
		int iFrameHeight = iEllipseHeight + 0*2 * g_iFrameMargin + g_fReflectSize;
		double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (iFrameHeight, fInclinationOnHorizon, g_iDockRadius, g_iDockLineWidth);
		double a = MAX (myDesklet->iWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : myData.iMaxIconWidth/2), iEllipseHeight)/2, b = MIN (myDesklet->iWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : myData.iMaxIconWidth/2), iEllipseHeight)/2;
		double c = sqrt (a * a - b * b);
		double e = c / a;
		gboolean bFlip = (iEllipseHeight > myDesklet->iWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : myData.iMaxIconWidth/2));
		Icon *pIcon;
		GList *ic;
		GList *pFirstDrawnElement = (myData.pFirstDrawnElement != NULL ? myData.pFirstDrawnElement : myData.pDeskletIconList);
		if (pFirstDrawnElement != NULL)
		{
			ic = pFirstDrawnElement;
			do
			{
				pIcon = ic->data;
				
				if (fTheta > G_PI && fTheta < 2*G_PI)  // arriere-plan.
				{
					pIcon->fScale = (1 + fabs (cos (fTheta))) / 2;
					pIcon->fAlpha = (1 + fabs (cos (fTheta))) / 2;
				}
				else
				{
					pIcon->fScale = 1.;
					pIcon->fAlpha = 1.;
				}
				pIcon->fDrawX = myDesklet->iWidth / 2 + a * cos (fTheta) - pIcon->fWidth/2 * pIcon->fScale;
				pIcon->fDrawY = myDesklet->iHeight / 2 + b * sin (fTheta) - pIcon->fHeight * pIcon->fScale;
				
				fTheta += fDeltaTheta;
				ic = cairo_dock_get_next_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
		}
		
		//\____________________ On trace le cadre.
		cairo_translate (pCairoContext, 0, g_iLabelSize);
		double fLineWidth = g_iDockLineWidth;
		double fMargin = 0*g_iFrameMargin;
		
		double fDockWidth = myDesklet->iWidth - fExtraWidth;
		int sens=1;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = fExtraWidth / 2;
		fDockOffsetY = (myDesklet->iHeight - iEllipseHeight) / 2;
		
		cairo_save (pCairoContext);
		cairo_dock_draw_frame (pCairoContext, g_iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclinationOnHorizon, myDesklet->bIsHorizontal);
		
		//\____________________ On dessine les decorations dedans.
		cairo_save (pCairoContext);
		double fColor[4];
		int i;
		for (i = 0; i < 4; i ++)
		{
			fColor[i] = (g_fDeskletColorInside[i] * myDesklet->iGradationCount + g_fDeskletColor[i] * (10 - myDesklet->iGradationCount)) / 10;
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
		if (pFirstDrawnElement != NULL)
		{
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					if (pIcon->fDrawY + pIcon->fHeight < myDesklet->iHeight / 2 && pIcon->fDrawX + pIcon->fWidth/2 > myDesklet->iWidth / 2)  // arriere-plan droite.
						cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_next_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
			
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					if (pIcon->fDrawY + pIcon->fHeight < myDesklet->iHeight / 2 && pIcon->fDrawX + pIcon->fWidth/2 <= myDesklet->iWidth / 2)  // arriere-plan gauche.
						cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_previous_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
		}
		
		cairo_save (pCairoContext);
		myIcon->fDrawY = myDesklet->iHeight/2 - myIcon->fHeight;
		cairo_dock_render_one_icon_in_desklet (myIcon, pCairoContext, TRUE, FALSE, myDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		if (pFirstDrawnElement != NULL)
		{
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					if (pIcon->fDrawY + pIcon->fHeight >= myDesklet->iHeight / 2 && pIcon->fDrawX + pIcon->fWidth/2 > myDesklet->iWidth / 2)  // avant-plan droite.
						cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_next_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
			
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					if (pIcon->fDrawY + pIcon->fHeight >= myDesklet->iHeight / 2 && pIcon->fDrawX + pIcon->fWidth/2 <= myDesklet->iWidth / 2)  // avant-plan gauche.
						cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_previous_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
		}
	}
	else
	{
		cd_debug (" icone en (%.2f;%.2f)", myIcon->fDrawX, myIcon->fDrawY);
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon_in_desklet (myIcon, pCairoContext, FALSE, FALSE, myDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		int iNumIcon=0, iNbIcons = g_list_length (myData.pDeskletIconList);
		cd_debug ("%d icones a dessiner", iNbIcons);
		double fTheta = G_PI/2, fDeltaTheta = 2 * G_PI / iNbIcons;
		
		double a = MAX (myIcon->fWidth, myIcon->fHeight)/2 + .1*myDesklet->iWidth, b = MIN (myIcon->fWidth, myIcon->fHeight)/2 + .1*myDesklet->iHeight;
		double c = sqrt (a * a - b * b);
		double e = c / a;
		gboolean bFlip = (myIcon->fHeight > myIcon->fWidth);
		Icon *pIcon;
		GList *ic;
		GList *pFirstDrawnElement = (myData.pFirstDrawnElement != NULL ? myData.pFirstDrawnElement : myData.pDeskletIconList);
		if (pFirstDrawnElement != NULL)
		{
			ic = pFirstDrawnElement;
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					pIcon->fDrawX = myIcon->fDrawX + myIcon->fWidth / 2 + (bFlip ? b : a) * cos (fTheta) - pIcon->fWidth/2;
					pIcon->fDrawY = myIcon->fDrawY + myIcon->fHeight / 2 + (bFlip ? a : b) * sin (fTheta) - pIcon->fHeight/2;
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				fTheta += fDeltaTheta;
				ic = cairo_dock_get_next_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
		}
	}
}