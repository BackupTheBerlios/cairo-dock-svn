/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

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
#include "cairo-dock-callbacks.h"
#include "cairo-dock-draw.h"

extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern gint g_iMaxAuthorizedWidth;
extern gboolean g_bResetScrollOnLeave;
extern gboolean g_bForceLoop;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern int g_iIconGap;

extern gboolean g_bRoundedBottomCorner;
extern gboolean g_bAutoHide;
extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];
extern int g_iNbStripes;
extern double g_fStripesWidth;
extern double g_fStripesSpeedFactor;
extern double g_fBackgroundImageWidth, g_fBackgroundImageHeight;
extern cairo_surface_t *g_pBackgroundSurface;
extern cairo_surface_t *g_pBackgroundSurfaceFull;

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern double g_fVisibleZoneAlpha;
extern double g_fAmplitude;
extern int g_iSinusoidWidth;

extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bLabelForPointedIconOnly;
extern double g_fLabelAlphaThreshold;

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



double cairo_dock_get_current_dock_width (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return 2 * g_iDockRadius + g_iDockLineWidth;
	
	Icon *pLastIcon = cairo_dock_get_last_drawn_icon (pDock);
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	double fWidth = pLastIcon->fX + pLastIcon->fWidth * pLastIcon->fScale - pFirstIcon->fX + 2 * g_iDockRadius + g_iDockLineWidth;
	
	return fWidth;
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

static void _cairo_dock_draw_frame_horizontal (CairoDock *pDock, cairo_t *pCairoContext, double fRadius, double fLineWidth, double fDockWidth, double fDockOffsetX, double fDockOffsetY, int sens)
{
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	
	cairo_rel_line_to (pCairoContext, fDockWidth - (2 * fRadius + fLineWidth), 0);
	// Top Right.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens * (pDock->iMaxIconHeight + fLineWidth - fRadius * (g_bRoundedBottomCorner ? 2 : 1)));
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
	cairo_rel_line_to (pCairoContext, 0, sens * (- pDock->iMaxIconHeight - fLineWidth + fRadius * (g_bRoundedBottomCorner ? 2 : 1)));
	// Top Left.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
	if (! g_bDirectionUp)
		cairo_move_to (pCairoContext, fDockOffsetX, pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth / 2);
}
static void _cairo_dock_draw_frame_vertical (CairoDock *pDock, cairo_t *pCairoContext, double fRadius, double fLineWidth, double fDockWidth, double fDockOffsetX, double fDockOffsetY, int sens)
{
	cairo_move_to (pCairoContext, fDockOffsetY, fDockOffsetX);
	
	cairo_rel_line_to (pCairoContext, 0, fDockWidth - (2 * fRadius + fLineWidth));
	// Top Right.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, fRadius,
		sens * fRadius, fRadius);
	cairo_rel_line_to (pCairoContext, sens * (pDock->iMaxIconHeight + fLineWidth - fRadius * (g_bRoundedBottomCorner ? 2 : 1)), 0);
	// Bottom Right.
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			sens * fRadius, 0,
			sens * fRadius, -fRadius);
	
	cairo_rel_line_to (pCairoContext, 0, -fDockWidth + fLineWidth + (g_bRoundedBottomCorner ? 2 * fRadius : 0));
	// Bottom Left
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			0, -fRadius,
			-sens * fRadius, -fRadius);
	cairo_rel_line_to (pCairoContext, sens * (- pDock->iMaxIconHeight - fLineWidth + fRadius * (g_bRoundedBottomCorner ? 2 : 1)), 0);
	// Top Left.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-sens * fRadius, 0,
		-sens * fRadius, fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
	if (! g_bDirectionUp)
		cairo_move_to (pCairoContext, pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth / 2, fDockOffsetX);
}
static void cairo_dock_render_one_icon (Icon *icon, cairo_t *pCairoContext, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, gboolean bLoop, gboolean bInside)
{
	//\_____________________ On dessine les icones en les zoomant du facteur d'echelle pre-calcule.
	double fDeltaLeft = (iMaxDockWidth - iCurrentWidth) / 2, fDeltaRight = fDeltaLeft;
	double fX, fY, fAlpha, fTheta;
	fX = icon->fX + (iCurrentWidth - iMaxDockWidth) / 2;
	//g_print ("icon->fX : %.2f -> %.2f\n", icon->fX, fX);
	if ((fX > 0 && fX + icon->fWidth * icon->fScale < iCurrentWidth))
	{
		fAlpha = 1;
		fY = icon->fY;
		if (! bLoop)
			fTheta = 0;
		else if (fX < iCurrentWidth / 2)
			fTheta = (fX + icon->fWidth * icon->fScale / 2 - iCurrentWidth / 2) / iCurrentWidth * G_PI;
		else
			fTheta = (fX + icon->fWidth * icon->fScale / 2 - iCurrentWidth / 2) / iCurrentWidth * G_PI;
		//g_print ("fX = %.2f (iCurrentWidth=%d) -> fTheta = %.2f\n", fX, iCurrentWidth, fTheta);
	}
	else
	{
		double a, b;
		if (! bInside)
		{
			fAlpha = 0.25;
			fY = icon->fY;
			if (! bLoop)
				fTheta = 0;
			else if (fX < iCurrentWidth / 2)
				fTheta = (fX + icon->fWidth * icon->fScale / 2 - iCurrentWidth / 2) / iCurrentWidth * G_PI;
			else
				fTheta = (fX + icon->fWidth * icon->fScale / 2 - iCurrentWidth / 2) / iCurrentWidth * G_PI;
		}
		else if (fX <= 0)
		{
			a = iCurrentHeight - icon->fHeight * icon->fScale;
			b = (iCurrentWidth - icon->fWidth * icon->fScale) / 2;
			fTheta = (fX / fDeltaLeft - 1) * G_PI / 2;
			fX = b * (1 + sin (fTheta));
			fY = (g_bDirectionUp ? a * (1 + cos (fTheta)) : - a * cos (fTheta) + g_iDockLineWidth);
			fAlpha = MAX (0.2, MIN (0.8, sin (fTheta) * sin (fTheta)));
			//g_print ("  theta = %.2fdeg -> fX = %.2f; fY = %.2f; alpha = %.2f\n", fTheta / G_PI*180, fX, fY, fAlpha);
		}
		else
		{
			a = iCurrentHeight - icon->fHeight * icon->fScale;
			b = (iCurrentWidth - icon->fWidth * icon->fScale) / 2;
			fTheta = ((fX + icon->fWidth * icon->fScale - iCurrentWidth) / fDeltaRight + 1) * G_PI / 2;
			fX = b * (1 + sin (fTheta));
			fY = (g_bDirectionUp ? a * (1 + cos (fTheta)) : - a * cos (fTheta) + g_iDockLineWidth);
			fAlpha = MAX (0.2, MIN (0.8, sin (fTheta) * sin (fTheta)));
			//g_print ("  theta = %.2fdeg -> fX = %.2f; fY = %.2f; alpha = %.2f\n", fTheta / G_PI*180, fX, fY, fAlpha);
		}
	}
	if (g_bHorizontalDock)
	{
		cairo_translate (pCairoContext, fX, fY);
	}
	else
	{
		cairo_translate (pCairoContext, fY, fX);
	}
	
	cairo_save (pCairoContext);
	if (icon->pIconBuffer != NULL)
	{
		double fWidthFactor = 1.;
		if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_ROTATE)
		{
			int c = icon->iCount;
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
		
		fWidthFactor = (G_PI / 2 - fabs (fTheta)) * 2 / G_PI;
		if (fWidthFactor >= 0 && fWidthFactor < 0.05)
			fWidthFactor = 0.05;
		else if (fWidthFactor < 0 && fWidthFactor > -0.05)
			fWidthFactor = -0.05;
		if (g_bHorizontalDock)
		{
			cairo_translate (pCairoContext, (1 - fWidthFactor) / 2 * icon->fWidth * icon->fScale, 0.);
			cairo_scale (pCairoContext, fWidthFactor * icon->fScale / (1 + g_fAmplitude), icon->fScale / (1 + g_fAmplitude));
		}
		else
		{
			cairo_translate (pCairoContext, 0., (1 - fWidthFactor) / 2 * icon->fHeight * icon->fScale);
			cairo_scale (pCairoContext, icon->fScale / (1 + g_fAmplitude), fWidthFactor * icon->fScale / (1 + g_fAmplitude));
		}
		//cairo_scale (pCairoContext, icon->fScale / (1 + g_fAmplitude), icon->fScale / (1 + g_fAmplitude));
		
		cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
		if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_BLINK)
		{
			int c = icon->iCount;
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
			if (fAlpha == 1)
				cairo_paint (pCairoContext);
			else
				cairo_paint_with_alpha (pCairoContext, fAlpha);
		}
	}
	cairo_restore (pCairoContext);
	
	//\_____________________ On dessine les etiquettes, avec un alpha proportionnel a leur facteur d'echelle.
	if (g_bUseText && icon->pTextBuffer != NULL && icon->fScale > 1.01 && (! g_bLabelForPointedIconOnly || icon->bPointed))  // 1.01 car sin(pi) = 1+epsilon :-/
	{
		if (g_bHorizontalDock)
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				-icon->fTextXOffset + icon->fWidth * icon->fScale * 0.5,
				g_bDirectionUp ? -g_iLabelSize : icon->fHeight * icon->fScale + icon->fTextYOffset);
		else
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				g_bDirectionUp ? -g_iLabelSize : icon->fHeight * icon->fScale + icon->fTextYOffset,
				-icon->fTextXOffset + icon->fWidth * icon->fScale * 0.5);
		double fMagnitude = (icon->fScale - 1) / g_fAmplitude;
		//cairo_paint_with_alpha (pCairoContext, (g_bLabelForPointedIconOnly ? 1.0 : pow (fMagnitude, 3)));
		cairo_paint_with_alpha (pCairoContext, (g_bLabelForPointedIconOnly ? 1.0 : (fMagnitude > 1. - 1. / g_fLabelAlphaThreshold ? 1.0 : 1. / (1. - fMagnitude) / g_fLabelAlphaThreshold)));
	}
}

void render (CairoDock *pDock)
{
	double fRadius = g_iDockRadius;
	double fLineWidth = g_iDockLineWidth;
	double fDockWidth = cairo_dock_get_current_dock_width (pDock);
	//g_print ("%s (%.2f)\n", __func__, fDockWidth);
	
	//\_________________ On determine des parametres de construction.
	int sens;
	double fDockOffsetX, fDockOffsetY;
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	//g_print ("1ere icone : %s, %.2f, %.2f\n", pFirstIcon->acName, pFirstIcon->fXAtRest, pFirstIcon->fX);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX : g_iDockRadius + 1. * g_iDockLineWidth / 2);
	fDockOffsetX = fDockOffsetX - (pDock->iMaxDockWidth - pDock->iCurrentWidth) / 2;
	if (fDockOffsetX - (fRadius + fLineWidth / 2) < 0)
		fDockOffsetX = fRadius + fLineWidth / 2;
	if (fDockOffsetX + fDockWidth - (fRadius + fLineWidth / 2) > pDock->iCurrentWidth)
		fDockWidth = pDock->iCurrentWidth - fDockOffsetX + (fRadius + fLineWidth / 2);
	//g_print ("fDockOffsetX : %.2f\n", fDockOffsetX);
	
	if (g_bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - pDock->iMaxIconHeight - 1.5 * fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iMaxIconHeight + 1.5 * fLineWidth;
	}
	
	//\_________________ On cree le contexte du dessin.
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
	g_return_if_fail (pCairoContext != NULL);
	
	cairo_set_tolerance (pCairoContext, 0.5);  // avec moins que 0.5 on ne voit pas la difference.
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\_________________ On trace un cadre, en commencant par le coin haut gauche.
	cairo_save (pCairoContext);
	if (g_bHorizontalDock)
		_cairo_dock_draw_frame_horizontal (pDock, pCairoContext, fRadius, fLineWidth, fDockWidth, fDockOffsetX, fDockOffsetY, sens);
	else
		_cairo_dock_draw_frame_vertical (pDock, pCairoContext, fRadius, fLineWidth, fDockWidth, fDockOffsetX, fDockOffsetY, sens);
	
	//\_________________ On dessine les decorations du fond.
	if (g_pBackgroundSurfaceFull != NULL)
	{
		cairo_save (pCairoContext);
		
		if (g_bHorizontalDock)
			cairo_translate (pCairoContext, (- pDock->fDecorationsOffsetX - pDock->iCurrentWidth / 2) / g_fStripesSpeedFactor - pDock->iCurrentWidth * 0.5, (g_bDirectionUp ? pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth : fLineWidth));
		else
			cairo_translate (pCairoContext, (g_bDirectionUp ? pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth : fLineWidth), (- pDock->fDecorationsOffsetX - pDock->iCurrentWidth / 2) / g_fStripesSpeedFactor - pDock->iCurrentWidth * 0.5);
		
		cairo_set_source_surface (pCairoContext, g_pBackgroundSurfaceFull, 0., 0.);
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	else if (g_pBackgroundSurface != NULL)
	{
		cairo_save (pCairoContext);
		
		double fDeltaX = (- pDock->fDecorationsOffsetX - pDock->iCurrentWidth / 2) / g_fStripesSpeedFactor;
		if (g_bHorizontalDock)
		{
			cairo_translate (pCairoContext, (fDeltaX + pDock->iCurrentWidth / 2 - pDock->iCurrentWidth / 2), (g_bDirectionUp ? pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth: fLineWidth));
			cairo_scale (pCairoContext, 1. * pDock->iCurrentWidth / g_fBackgroundImageWidth, 1. * pDock->iMaxIconHeight / g_fBackgroundImageHeight);
		}
		else
		{
			cairo_translate (pCairoContext, (g_bDirectionUp ? pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth: fLineWidth), (fDeltaX + pDock->iCurrentWidth / 2 - pDock->iCurrentWidth / 2));
			cairo_scale (pCairoContext, 1. * pDock->iMaxIconHeight / g_fBackgroundImageHeight, 1. * pDock->iCurrentWidth / g_fBackgroundImageWidth);
		}
		
		//g_print ("(%dx%d) / (%dx%d)\n", pDock->iCurrentWidth, pDock->iMaxIconHeight, (int) g_fBackgroundImageWidth, (int) g_fBackgroundImageHeight);
		
		cairo_set_source_surface (pCairoContext, (pDock->bInside ? g_pBackgroundSurface : g_pBackgroundSurface), 0., 0.);
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
	
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	ic = pFirstDrawnElement;
	if (ic == NULL)
	{
		cairo_destroy (pCairoContext);
		return ;
	}
	//for (ic = pDock->icons; ic != NULL; ic = ic->next)
	//g_print ("--------------------\n");
	do
	{
		icon = (Icon*) ic->data;
		cairo_save (pCairoContext);
		
		//g_print ("redessin a gauche de %s\n", icon->acName);
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, (pDock->iRefCount == 0 && 1. * pDock->iCurrentWidth / pDock->iMaxDockWidth < .6 && pDock->bInside), pDock->bInside);
		
		cairo_restore (pCairoContext);
		ic = ic->next;
		if (ic == NULL)
			ic = pDock->icons;
	}
	while (icon->fX + icon->fWidth * icon->fScale - (pDock->iMaxDockWidth - pDock->iCurrentWidth) / 2 < 0 && ic != pFirstDrawnElement);
	
	GList *pMiddleElement = ic;
	ic = pFirstDrawnElement->prev;
	if (ic == NULL)
		ic = g_list_last (pDock->icons);
	do
	{
		icon = (Icon*) ic->data;
		cairo_save (pCairoContext);
		
		//g_print ("redessin a droite de %s\n", icon->acName);
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, (pDock->iRefCount == 0 && 1. * pDock->iCurrentWidth / pDock->iMaxDockWidth < .6 && pDock->bInside), pDock->bInside);
		
		cairo_restore (pCairoContext);
		if (ic == pMiddleElement)
			break;
		ic = ic->prev;
		if (ic == NULL)
			ic = g_list_last (pDock->icons);
	}
	while (TRUE);
	
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
		///cairo_scale (pCairoContext, 1. * g_iVisibleZoneWidth / g_fVisibleZoneImageWidth, 1. * g_iVisibleZoneHeight / g_fVisibleZoneImageHeight);
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



void cairo_dock_redraw_my_icon (Icon *icon, GtkWidget *pWidget)
{
	GdkRectangle rect = {(int) round (icon->fX - (g_pMainDock->iMaxDockWidth - g_pMainDock->iCurrentWidth) / 2), (int) icon->fY, (int) round (icon->fWidth * icon->fScale), (int) icon->fHeight * icon->fScale};
	if (! g_bHorizontalDock)
	{
		rect.x = (int) icon->fY;
		rect.y = (int) round (icon->fX - (g_pMainDock->iMaxDockWidth - g_pMainDock->iCurrentWidth) / 2);
		rect.width = (int) icon->fHeight * icon->fScale;
		rect.height = (int) round (icon->fWidth * icon->fScale);
	}
	gdk_window_invalidate_rect (pWidget->window, &rect, FALSE);
}


void cairo_dock_render_optimized (CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ((%d;%d) x (%d;%d) / (%dx%d))\n", __func__, pArea->x, pArea->y, pArea->width, pArea->height, pDock->iCurrentWidth, pDock->iCurrentHeight);
	double fLineWidth = g_iDockLineWidth;
	gint iWidth, iHeight;
	iWidth = pDock->iCurrentWidth;
	iHeight = pDock->iCurrentHeight;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
	g_return_if_fail (pCairoContext != NULL);

	/* set rendering-"fidelity" and clear canvas */
	cairo_set_tolerance (pCairoContext, 0.5);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\_________________ On dessine les rayures du fond sur la portion de fenetre.
	cairo_save (pCairoContext);
	
	double fDockOffsetX, fDockOffsetY;
	if (g_bHorizontalDock)
	{
		fDockOffsetX = pArea->x;
		fDockOffsetY = (g_bDirectionUp ? iHeight - pDock->iMaxIconHeight - fLineWidth : fLineWidth);
	}
	else
	{
		fDockOffsetX = (g_bDirectionUp ? iHeight - pDock->iMaxIconHeight - fLineWidth : fLineWidth);
		fDockOffsetY = pArea->y;
	}
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	if (g_bHorizontalDock)
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, pDock->iMaxIconHeight);
	else
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pDock->iMaxIconHeight, pArea->height);
	if (g_pBackgroundSurfaceFull != NULL)
	{
		cairo_save (pCairoContext);
		
		//cairo_translate (pCairoContext, - (pDock->fDecorationsOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth, fDockOffsetY);
		if (g_bHorizontalDock)
			cairo_translate (pCairoContext, (- pDock->fDecorationsOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth * 0.5, (g_bDirectionUp ? iHeight - pDock->iMaxIconHeight - fLineWidth: fLineWidth));
		else
			cairo_translate (pCairoContext, (g_bDirectionUp ? iHeight - pDock->iMaxIconHeight - fLineWidth: fLineWidth), (- pDock->fDecorationsOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth * 0.5);
		cairo_set_source_surface (pCairoContext, g_pBackgroundSurfaceFull, 0., 0.);
		
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	else if (g_pBackgroundSurface != NULL)
	{
		cairo_save (pCairoContext);
		
		if (g_bHorizontalDock)
		{
			cairo_translate (pCairoContext, - (pDock->fDecorationsOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth / 2, fDockOffsetY);
			cairo_scale (pCairoContext, 1. * iWidth / g_fBackgroundImageWidth, 1. * pDock->iMaxIconHeight / g_fBackgroundImageHeight);
		}
		else
		{
			cairo_translate (pCairoContext, 0, - (pDock->fDecorationsOffsetX - iWidth / 2) / g_fStripesSpeedFactor - iWidth / 2);
			cairo_scale (pCairoContext, 1. * pDock->iMaxIconHeight / g_fBackgroundImageHeight, 1. * iWidth / g_fBackgroundImageWidth);
		}
		
		cairo_set_source_surface (pCairoContext, (pDock->bInside ? g_pBackgroundSurface : g_pBackgroundSurface), 0., 0.);
		
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	//\_________________ On dessine la partie du cadre qui va bien.
	cairo_new_path (pCairoContext);
	
	if (g_bHorizontalDock)
	{
		cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY - fLineWidth / 2);
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
		
		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, fDockOffsetX, (g_bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iMaxIconHeight + 1.5 * fLineWidth));
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
		cairo_move_to (pCairoContext, (g_bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iMaxIconHeight + 1.5 * fLineWidth), fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	}
	cairo_stroke (pCairoContext);
	
	cairo_restore (pCairoContext);
	
	
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	double fX, fXLimit = (g_bHorizontalDock ? pArea->x + pArea->width : pArea->y + pArea->height);
	GList *ic;
	Icon *icon;
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	ic = pFirstDrawnElement;
	do
	{
		if (ic == NULL)
			break;
		icon = ic->data;
		
		fX = icon->fX - (pDock->iMaxDockWidth - pDock->iCurrentWidth) / 2;
		if (floor (fX + icon->fWidth * icon->fScale) > pArea->x)  // on entre dans la zone.
		{
			do
			{
				icon = ic->data;
				cairo_save (pCairoContext);
				//g_print ("redessin de %s (%.2f->%.2f)\n", icon->acName, fX, (fX + icon->fWidth * icon->fScale));
				/*fX = icon->fX - (pDock->iMaxDockWidth - pDock->iCurrentWidth) / 2;
				if (g_bHorizontalDock)
					cairo_translate (pCairoContext, fX, icon->fY);
				else
					cairo_translate (pCairoContext, icon->fY, fX);
				cairo_scale (pCairoContext, icon->fScale / (1 + g_fAmplitude), icon->fScale / (1 + g_fAmplitude));
				cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
				cairo_paint (pCairoContext);
				cairo_restore (pCairoContext);*/
				cairo_dock_render_one_icon (icon, pCairoContext, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, (pDock->iRefCount == 0 && 1. * pDock->iCurrentWidth / pDock->iMaxDockWidth < .6 && pDock->bInside), pDock->bInside);
				
				ic = ic->next;
				if (ic == NULL)
					ic = pDock->icons;
			} while (ceil (fX + icon->fWidth * icon->fScale) < fXLimit && ic != pFirstDrawnElement);
			break ;
		}
		
		ic = ic->next;
		if (ic == NULL)
			ic = pDock->icons;
	}
	while (ic != pFirstDrawnElement);
	
	cairo_destroy (pCairoContext);
}


static gboolean _cairo_dock_hide_dock (gchar *cDockName, CairoDock *pDock, CairoDock *pChildDock)
{
	Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	if (pPointedIcon != NULL && pPointedIcon->pSubDock == pChildDock && ! pDock->bInside)
	{
		if (pDock->iRefCount == 0)  // pDock->bIsMainDock
		{
			cairo_dock_leave_from_main_dock (pDock);
		}
		else
		{
			if (pDock->iScrollOffset != 0)  // on remet systematiquement a 0 l'offset pour les containers.
			{
				pDock->iScrollOffset = 0;
				cairo_dock_calculate_icons (pDock, pDock->iCurrentWidth / 2, 0);
				render (pDock);
			}
			
			gdk_window_hide (pDock->pWidget->window);
			cairo_dock_hide_parent_docks (pDock);
		}
		return TRUE;
	}
	return FALSE;
}
void cairo_dock_hide_parent_docks (CairoDock *pDock)
{
	 g_hash_table_find (g_hDocksTable, (GHRFunc)_cairo_dock_hide_dock, pDock);
}



void cairo_dock_calculate_window_position_at_balance (CairoDock *pDock, CairoDockSizeType iSizeType, int *iNewWidth, int *iNewHeight)
{
	if (iSizeType == CAIRO_DOCK_MAX_SIZE)
	{
		*iNewWidth = (g_bForceLoop && pDock->iRefCount == 0 ? pDock->iMaxDockWidth / 2 : MIN (g_iMaxAuthorizedWidth, pDock->iMaxDockWidth));
		*iNewHeight = pDock->iMaxDockHeight;
		pDock->iWindowPositionX = (g_iScreenWidth - *iNewWidth) / 2 + pDock->iGapX;
		pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - (*iNewHeight) - pDock->iGapY : pDock->iGapY);
	}
	else if (iSizeType == CAIRO_DOCK_NORMAL_SIZE)
	{
		*iNewWidth = MIN (g_iMaxAuthorizedWidth, pDock->iMinDockWidth  + 2 * g_iDockRadius + g_iDockLineWidth);
		if (g_bForceLoop && *iNewWidth > pDock->iMaxDockWidth / 2)
			*iNewWidth = pDock->iMaxDockWidth / 2;
		*iNewHeight = pDock->iMaxIconHeight + 2 * g_iDockLineWidth;
		pDock->iWindowPositionX = (g_iScreenWidth - *iNewWidth) / 2 + pDock->iGapX;
		pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - (*iNewHeight) - pDock->iGapY : pDock->iGapY);
	}
	else
	{
		*iNewWidth = g_iVisibleZoneWidth;
		*iNewHeight = g_iVisibleZoneHeight;
		pDock->iWindowPositionX = (g_iScreenWidth - *iNewWidth) / 2 + pDock->iGapX;
		pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - (*iNewHeight) - pDock->iGapY : pDock->iGapY);
	}
}
