/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


******************************************************************************/
#include <math.h>
#include <string.h>
#include <cairo.h>
#include <pango/pango.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif


#include "cairo-dock-icons.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-draw.h"


extern gint g_iScreenWidth;
extern gint g_iScreenHeight;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern int g_iIconGap;
extern int g_iLabelSize;
extern gboolean g_bRoundedBottomCorner;
extern gboolean g_bAutoHide;
extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];
extern int g_iNbStripes;
extern double g_fStripesWidth;
extern cairo_surface_t *g_pStripesBuffer;
extern double g_fStripesSpeedFactor;

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern cairo_surface_t *g_pVisibleZoneSurfaceAlpha;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern double g_fVisibleZoneAlpha;
extern double g_fAmplitude;
extern int g_iSinusoidWidth;

extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bLabelForPointedIconOnly;

extern double g_fGrowUpFactor;
extern double g_fShrinkDownFactor;

extern Icon *g_pLastFixedIconLeft;
extern Icon *g_pLastFixedIconRight;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];

extern GtkWidget *g_pWidget;

#ifdef HAVE_GLITZ
extern gboolean g_bUseGlitz;
extern glitz_drawable_format_t *gDrawFormat;
extern glitz_drawable_t* g_pGlitzDrawable;
extern glitz_format_t* g_pGlitzFormat;
#endif // HAVE_GLITZ



double get_current_dock_width (GList *pIconList)
{
	if (pIconList == NULL)
		return 2 * g_iDockRadius + g_iDockLineWidth;
	Icon *pLastIcon = get_last_icon (pIconList);
	Icon *pFirstIcon = get_first_icon (pIconList);
	double fWidth = pLastIcon->fX + pLastIcon->fWidth * pLastIcon->fScale - pFirstIcon->fX + 2 * g_iDockRadius + g_iDockLineWidth;
	
	return fWidth;
}

double get_current_dock_offset_x (GList *pIconList)
{
	Icon *pFirstIcon = get_first_icon (pIconList);
	return (pFirstIcon != NULL ? pFirstIcon->fX : g_iDockRadius + 1. * g_iDockLineWidth / 2);
}

double get_current_dock_offset_y (CairoDock *pDock)
{
	return  pDock->iMaxDockHeight - pDock->iMaxIconHeight - g_iDockLineWidth;
}



cairo_t * cairo_dock_create_context_from_window (GdkWindow* pWindow)
{
#ifdef HAVE_GLITZ
	if (g_pGlitzDrawable)
	{
		glitz_surface_t* pGlitzSurface;
		cairo_surface_t* pCairoSurface;
		cairo_t* pCairoContext;

		pGlitzSurface = glitz_surface_create (g_pGlitzDrawable,
						      g_pGlitzFormat,
						      g_iCurrentWidth,
						      g_iCurrentHeight,
						      0,
						      NULL);

		if (gDrawFormat->doublebuffer)
			glitz_surface_attach (pGlitzSurface,
					      g_pGlitzDrawable,
					      GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
		else
			glitz_surface_attach (pGlitzSurface,
					      g_pGlitzDrawable,
					      GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);

		pCairoSurface = cairo_glitz_surface_create (pGlitzSurface);
		pCairoContext = cairo_create (pCairoSurface);

		cairo_surface_destroy (pCairoSurface);
		glitz_surface_destroy (pGlitzSurface);

		return pCairoContext;
	}
#endif // HAVE_GLITZ

	return gdk_cairo_create (pWindow);
}


void render (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	double fRadius = g_iDockRadius;
	double fLineWidth = g_iDockLineWidth;
	double fDockWidth = get_current_dock_width (pDock->icons);
	gint iWidth, iHeight;
	///gtk_window_get_size (GTK_WINDOW (pWidget), &iWidth, &iHeight);
	iWidth = pDock->iCurrentWidth; iHeight = pDock->iCurrentHeight;
	/*if (fDockWidth > iWidth)
	{
		g_iMaxDockWidth = ceil (fDockWidth);
		g_iWindowPositionX = g_iGapX + (g_iScreenWidth - fDockWidth) / 2;
		gdk_window_move_resize (pWidget->window,
			g_iWindowPositionX,
			g_iWindowPositionY,
			g_iMaxDockWidth,
			iHeight);
	}*/
	
	//\_________________ On cree le contexte du dessin.
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
	g_return_if_fail (pCairoContext != NULL);
	
	cairo_set_tolerance (pCairoContext, 0.5);  // avec moins que 0.5 on ne voit pas la difference.
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	

	//\_________________ On trace un cadre, en commencant par le coin haut gauche.
	cairo_save (pCairoContext);
	int sens;
	double fDockOffsetY;
	double fDockOffsetX = get_current_dock_offset_x (pDock->icons);
	if (g_bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = iHeight - pDock->iMaxIconHeight - fLineWidth / 2;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iMaxIconHeight + fLineWidth / 2;
	}
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	
	cairo_rel_line_to (pCairoContext, fDockWidth - (2 * fRadius + fLineWidth), 0);
	
	// Top Right.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens * (pDock->iMaxIconHeight - fRadius * (g_bRoundedBottomCorner ? 2 : 1)));
	
	// Bottom Right.
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			0, sens * fRadius,
			-fRadius, sens * fRadius);
	
	cairo_rel_line_to (pCairoContext, -fDockWidth + fLineWidth + (g_bRoundedBottomCorner ? 2 * fRadius : 0), 0);
	
	// Bottom Left
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			-fRadius, 0,
			-fRadius, -sens * fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens * (- pDock->iMaxIconHeight + fRadius * (g_bRoundedBottomCorner ? 2 : 1)));
	
	// Top Left.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (! g_bDirectionUp)
		cairo_move_to (pCairoContext, fDockOffsetX, iHeight - pDock->iMaxIconHeight - fLineWidth / 2);
	
	
	//\_________________ On dessine les rayures du fond.
	if (g_pStripesBuffer != NULL)
	{
		cairo_save (pCairoContext);
		
		cairo_translate (pCairoContext, - (pDock->fGradientOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth, iHeight - pDock->iMaxIconHeight + fLineWidth / 2);
		cairo_set_source_surface (pCairoContext, g_pStripesBuffer, 0., 0.);
		
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	else if (g_pVisibleZoneSurface != NULL)
	{
		cairo_save (pCairoContext);
		
		cairo_translate (pCairoContext, - (pDock->fGradientOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth / 2, iHeight - pDock->iMaxIconHeight + fLineWidth / 2);
		cairo_scale (pCairoContext, 1. * iWidth / g_fVisibleZoneImageWidth, 1. * pDock->iMaxIconHeight / g_fVisibleZoneImageHeight);
		cairo_set_source_surface (pCairoContext, (pDock->bInside ? g_pVisibleZoneSurface : g_pVisibleZoneSurfaceAlpha), 0., 0.);
		
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	//\_________________ On dessine le cadre.
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	cairo_stroke (pCairoContext);
	cairo_restore (pCairoContext);
	
	//\_________________ On dessine les icones et les etiquettes.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	Icon* icon;
	GList* ic;
	double fWidthFactor = 1.;
	int c;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = (Icon*) ic->data;
		cairo_save (pCairoContext);
		
		//\_____________________ On dessine les icones en les zoomant du facteur d'echelle pre-calcule.
		cairo_translate (pCairoContext, icon->fX, icon->fY);
		cairo_save (pCairoContext);
		if (icon->pIconBuffer != NULL)
		{
			if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_ROTATE)
			{
				c = icon->iCount;
				if ((c/5) & 1)
				{
					fWidthFactor = ((c/10) & 1 ? 1. : -1.) * (c%5) / 5;
				}
				else
				{
					fWidthFactor = ((c/10) & 1 ? 1. : -1.) * ((c%5) - 5) / 5;
				}
				if (fWidthFactor == 0)
					fWidthFactor = 0.01;
				cairo_translate (pCairoContext, (1 - fWidthFactor) / 2 * icon->fWidth * icon->fScale, 0.);
				cairo_scale (pCairoContext, fWidthFactor * icon->fScale / (1 + g_fAmplitude), icon->fScale / (1 + g_fAmplitude));
				icon->iCount --;
			}
			else
			{
				cairo_scale (pCairoContext, icon->fScale / (1 + g_fAmplitude), icon->fScale / (1 + g_fAmplitude));
			}
			
			cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
			if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_BLINK)
			{
				c = icon->iCount;
				double alpha;
				if ( (c/10) & 1)
					alpha = 1. * (c%10) / 10;
				else
					alpha = 1. * (9 - (c%10)) / 10;
				cairo_paint_with_alpha (pCairoContext, alpha * alpha);
				icon->iCount --;
			}
			else
			{
				cairo_paint (pCairoContext);
			}
		}
		cairo_restore (pCairoContext);
		
		//\_____________________ On dessine les etiquettes, avec un alpha proportionnel a leur facteur d'echelle.
		if (g_bUseText && icon->pTextBuffer != NULL && icon->fScale > 1.0 && (! g_bLabelForPointedIconOnly || icon->bPointed))
		{
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				-icon->fTextXOffset + icon->fWidth * icon->fScale * 0.5,
				(g_bDirectionUp ? -icon->fTextYOffset : icon->fHeight * icon->fScale));
			cairo_paint_with_alpha (pCairoContext, ((icon->fScale - 1) / g_fAmplitude) * ((icon->fScale - 1) / g_fAmplitude));
		}
		// Made this tighter [KL]
		cairo_restore (pCairoContext);
	}
	
	cairo_destroy (pCairoContext);
	
#ifdef HAVE_GLITZ
	if (gDrawFormat && gDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (g_pGlitzDrawable);
#endif
}

void cairo_dock_render_background (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
	
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_move_to (pCairoContext, 0, 0);
	if (g_pVisibleZoneSurface != NULL)
	{
		cairo_scale (pCairoContext, 1. * g_iVisibleZoneWidth / g_fVisibleZoneImageWidth, 1. * g_iVisibleZoneHeight / g_fVisibleZoneImageHeight);
		cairo_set_source_surface (pCairoContext, g_pVisibleZoneSurface, 0.0, 0.0);
		cairo_paint_with_alpha (pCairoContext, g_fVisibleZoneAlpha);
	}
	cairo_destroy (pCairoContext);
}

void cairo_dock_render_blank (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
	
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);

	cairo_destroy (pCairoContext);
}


gboolean grow_up2 (CairoDock *pDock)
{
	//g_print ("%s (%f)\n", __func__, g_fMagnitude);
	if (pDock->fMagnitude < 0.05)
		pDock->fMagnitude = 0.05;
	
	pDock->fMagnitude *= g_fGrowUpFactor;  // 1.4
	
	if (pDock->fMagnitude > 1.0)
		pDock->fMagnitude = 1.0;
	
	gint iMouseX, iMouseY;
	gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
	
	cairo_dock_calculate_icons (pDock, iMouseX, iMouseY);
	gtk_widget_queue_draw (pDock->pWidget);
	
	if (pDock->fMagnitude == 1)
	{
		pDock->iSidGrowUp = 0;
		return FALSE;
	}
	else
		return TRUE;
}

gboolean shrink_down2 (CairoDock *pDock)
{
	//g_print ("%s (%f)\n", __func__, g_fMagnitude);
	if (pDock->fMagnitude > 0.05)
		pDock->fMagnitude *= g_fShrinkDownFactor; //  0.6
	else
		pDock->fMagnitude = 0.0;
		
	gint iMouseX, iMouseY;
	gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
	
	cairo_dock_calculate_icons (pDock, iMouseX, iMouseY);
	gtk_widget_queue_draw (pDock->pWidget);
	
	if (pDock->fMagnitude < 0.05)
	{
		Icon *pBouncingIcon = cairo_dock_get_bouncing_icon (pDock->icons);
		Icon *pRemovingIcon = cairo_dock_get_removing_or_inserting_icon (pDock->icons);
		
		if (pBouncingIcon == NULL && pRemovingIcon == NULL)
		{
			pDock->fMagnitude = 0;
			pDock->iSidShrinkDown = 0;
			
			if (! g_bAutoHide && ! pDock->bInside)
			{
				//g_print ("on arrive en bas -> %dx%d\n", g_iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth, g_iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth);
				pDock->iWindowPositionX = (g_iScreenWidth - (pDock->iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth)) / 2 + pDock->iGapX;
				pDock->iWindowPositionY = g_iScreenHeight - pDock->iGapY - (g_bDirectionUp ? pDock->iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth : 0);
				gdk_window_move_resize (pDock->pWidget->window,
					pDock->iWindowPositionX,
					pDock->iWindowPositionY,
					pDock->iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth,
					pDock->iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth);
			}
			
			gint iMouseX, iMouseY;
			gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
			
			cairo_dock_calculate_icons (pDock, iMouseX, iMouseY);  // relance le grossissement si on est dedans.
			return FALSE;
		}
		
		//\______________ Au moins une icone est en cours d'animation suite a un clique, on continue le 'shrink_down'.
		if (pRemovingIcon != NULL)
		{
			//g_print ("au moins 1 icone en cours d'insertion/suppression (%f)\n", pRemovingIcon->fPersonnalScale);
			if (pRemovingIcon->fPersonnalScale == 0.05)
			{
				//g_print ("  fin\n");
				cairo_dock_remove_icon_from_dock (pDock, pRemovingIcon);
				cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
				cairo_dock_free_icon (pRemovingIcon);
			}
			else if (pRemovingIcon->fPersonnalScale == -0.05)
			{
				//g_print ("  fin\n");
				pRemovingIcon->fPersonnalScale = 0;
			}
		}
		
		pDock->fMagnitude = 0.001;  // on garde la magnitude > 0 de facon a ce qu'un motion_notify ne commence pas un 'grow_up'.
		return TRUE;
	}
	else
		return TRUE;
}



void cairo_dock_redraw_my_icon (Icon *icon, GtkWidget *pWidget)
{
	GdkRectangle rect = {(int) icon->fX, (int) icon->fY, (int) icon->fWidth * icon->fScale, (int) icon->fHeight * icon->fScale};
	gdk_window_invalidate_rect (pWidget->window, &rect, FALSE);
}


void cairo_dock_render_optimized (CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ()\n", __func__);
	double fLineWidth = g_iDockLineWidth;
	gint iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pDock->pWidget), &iWidth, &iHeight);
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
	g_return_if_fail (pCairoContext != NULL);

	/* set rendering-"fidelity" and clear canvas */
	cairo_set_tolerance (pCairoContext, 0.5);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\_________________ On dessine les rayures du fond sur la portion de fenetre.
	cairo_save (pCairoContext);
	
	double fDockOffsetY;
	double fDockOffsetX = pArea->x;
	fDockOffsetY = iHeight - pDock->iMaxIconHeight - fLineWidth / 2;
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, pDock->iMaxIconHeight);
	if (g_pStripesBuffer != NULL)
	{
		cairo_save (pCairoContext);
		
		cairo_translate (pCairoContext, - (pDock->fGradientOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth, iHeight - pDock->iMaxIconHeight + fLineWidth / 2);
		cairo_set_source_surface (pCairoContext, g_pStripesBuffer, 0., 0.);
		
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	else if (g_pVisibleZoneSurface != NULL)
	{
		cairo_save (pCairoContext);
		
		cairo_translate (pCairoContext, - (pDock->fGradientOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth / 2, iHeight - pDock->iMaxIconHeight + fLineWidth / 2);
		cairo_scale (pCairoContext, 1. * iWidth / g_fVisibleZoneImageWidth, 1. * pDock->iMaxIconHeight / g_fVisibleZoneImageHeight);
		cairo_set_source_surface (pCairoContext, (pDock->bInside ? g_pVisibleZoneSurface : g_pVisibleZoneSurfaceAlpha), 0., 0.);
		
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	//\_________________ On dessine la partie du cadre qui va bien.
	cairo_new_path (pCairoContext);
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	cairo_rel_line_to (pCairoContext, pArea->width, 0);
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	cairo_stroke (pCairoContext);
	
	cairo_new_path (pCairoContext);
	
	cairo_move_to (pCairoContext, fDockOffsetX, iHeight - fLineWidth / 2);
	cairo_rel_line_to (pCairoContext, pArea->width, 0);
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	cairo_stroke (pCairoContext);
	
	cairo_restore (pCairoContext);
	
	
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	GList *ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		if (floor (icon->fX + icon->fWidth * icon->fScale) > pArea->x)  // on entre dans la zone.
		{
			do
			{
				icon = ic->data;
				
				//g_print ("redessin de %s (%d->%d)\n", icon->acName, (int) icon->fX, (int) (icon->fX + icon->fWidth * icon->fScale));
				cairo_save (pCairoContext);
				cairo_translate (pCairoContext, icon->fX, icon->fY);
				cairo_scale (pCairoContext, icon->fScale / (1 + g_fAmplitude), icon->fScale / (1 + g_fAmplitude));
				cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
				cairo_paint (pCairoContext);
				cairo_restore (pCairoContext);
				
				ic = ic->next;
			} while (ceil (icon->fX) < pArea->x + pArea->width && ic != NULL);
			break ;
		}
		
	}
	
	cairo_destroy (pCairoContext);
}

