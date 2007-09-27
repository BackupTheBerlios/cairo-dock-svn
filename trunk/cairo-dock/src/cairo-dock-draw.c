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
extern double g_fSubDockSizeRatio;
extern gboolean g_bAutoHide;
extern gboolean g_bTextAlwaysHorizontal;

extern gint g_iScreenWidth[2];
extern gint g_iScreenHeight[2];
extern gint g_iMaxAuthorizedWidth;
extern gboolean g_bForceLoop;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern gint g_iStringLineWidth;
extern double g_fStringColor[4];
extern int g_iIconGap;

extern gboolean g_bRoundedBottomCorner;
extern gboolean g_bDirectionUp;
extern double g_fStripesSpeedFactor;
extern double g_fBackgroundImageWidth, g_fBackgroundImageHeight;
extern cairo_surface_t *g_pBackgroundSurface[2];
extern cairo_surface_t *g_pBackgroundSurfaceFull[2];

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneAlpha;
extern double g_fAmplitude;

extern int g_iLabelSize;
extern gboolean g_bLabelForPointedIconOnly;
extern double g_fLabelAlphaThreshold;

extern int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS];

extern gboolean g_bUseGlitz;


double cairo_dock_get_current_dock_width (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return 2 * g_iDockRadius + g_iDockLineWidth;
	
	Icon *pLastIcon = cairo_dock_get_last_drawn_icon (pDock);
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	double fWidth = pLastIcon->fX - pFirstIcon->fX + 2 * g_iDockRadius + g_iDockLineWidth + pLastIcon->fWidth * pLastIcon->fScale;
	
	return fWidth;
}


cairo_t * cairo_dock_create_context_from_window (CairoDock *pDock)
{
#ifdef HAVE_GLITZ
	if (pDock->pGlitzDrawable)
	{
		//g_print ("creation d'un contexte lie a une surface glitz\n");
		glitz_surface_t* pGlitzSurface;
		cairo_surface_t* pCairoSurface;
		cairo_t* pCairoContext;
		
		pGlitzSurface = glitz_surface_create (pDock->pGlitzDrawable,
			pDock->pGlitzFormat,
			pDock->iCurrentWidth,
			pDock->iCurrentHeight,
			0,
			NULL);
		
		if (pDock->pDrawFormat->doublebuffer)
			glitz_surface_attach (pGlitzSurface,
				pDock->pGlitzDrawable,
				GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
		else
			glitz_surface_attach (pGlitzSurface,
				pDock->pGlitzDrawable,
				GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
		
		pCairoSurface = cairo_glitz_surface_create (pGlitzSurface);
		pCairoContext = cairo_create (pCairoSurface);
		
		cairo_surface_destroy (pCairoSurface);
		glitz_surface_destroy (pGlitzSurface);

		return pCairoContext;
	}
#endif // HAVE_GLITZ
	return gdk_cairo_create (pDock->pWidget->window);
}

static void _cairo_dock_draw_frame_horizontal (CairoDock *pDock, cairo_t *pCairoContext, double fRadius, double fLineWidth, double fDockWidth, double fDockOffsetX, double fDockOffsetY, int sens, gboolean bIsLoop)
{
	double fDeltaXForLoop = (bIsLoop ? (pDock->iMaxIconHeight + fLineWidth - 2 * fRadius) * tan (30.*G_PI/180) : 0.);
	double fDeltaCornerForLoop = fRadius / (pDock->iMaxIconHeight + fLineWidth - 2 * fRadius) * fDeltaXForLoop;
	cairo_move_to (pCairoContext, fDockOffsetX + fDeltaXForLoop + fDeltaCornerForLoop / 2, fDockOffsetY);
	
	cairo_rel_line_to (pCairoContext, fDockWidth - (2 * fRadius + fLineWidth + 2 * fDeltaXForLoop + fDeltaCornerForLoop), 0);
	// Top Right.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius - fDeltaCornerForLoop, 0,
		fRadius, sens * fRadius);
	cairo_rel_line_to (pCairoContext, fDeltaXForLoop, sens * (pDock->iMaxIconHeight + fLineWidth - fRadius * (g_bRoundedBottomCorner ? 2 : 1)));
	// Bottom Right.
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			fDeltaCornerForLoop, sens * fRadius,
			-fRadius, sens * fRadius);
	
	cairo_rel_line_to (pCairoContext, -fDockWidth + fLineWidth + (g_bRoundedBottomCorner ? 2 * fRadius : 0) + fDeltaCornerForLoop, 0);
	// Bottom Left
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			-(fRadius + fDeltaCornerForLoop), 0,
			-fRadius, -sens * fRadius);
	cairo_rel_line_to (pCairoContext, fDeltaXForLoop, sens * (- pDock->iMaxIconHeight - fLineWidth + fRadius * (g_bRoundedBottomCorner ? 2 : 1)));
	// Top Left.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fDeltaCornerForLoop, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
}
static void _cairo_dock_draw_frame_vertical (CairoDock *pDock, cairo_t *pCairoContext, double fRadius, double fLineWidth, double fDockWidth, double fDockOffsetX, double fDockOffsetY, int sens, gboolean bIsLoop)
{
	double fDeltaXForLoop = (bIsLoop ? (pDock->iMaxIconHeight + fLineWidth - 2 * fRadius) * tan (30.*G_PI/180) : 0.);
	double fDeltaCornerForLoop = fRadius / (pDock->iMaxIconHeight + fLineWidth - 2 * fRadius) * fDeltaXForLoop;
	cairo_move_to (pCairoContext, fDockOffsetY, fDockOffsetX + fDeltaXForLoop + fDeltaCornerForLoop / 2);
	
	cairo_rel_line_to (pCairoContext, 0, fDockWidth - (2 * fRadius + fLineWidth + 2 * fDeltaXForLoop + fDeltaCornerForLoop));
	// Top Right.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, fRadius - fDeltaCornerForLoop,
		sens * fRadius, fRadius);
	cairo_rel_line_to (pCairoContext, sens * (pDock->iMaxIconHeight + fLineWidth - fRadius * (g_bRoundedBottomCorner ? 2 : 1)), fDeltaXForLoop);
	// Bottom Right.
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			sens * fRadius, fDeltaCornerForLoop,
			sens * fRadius, -fRadius);
	
	cairo_rel_line_to (pCairoContext, 0, -fDockWidth + fLineWidth + (g_bRoundedBottomCorner ? 2 * fRadius : 0) + fDeltaCornerForLoop);
	// Bottom Left
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			0, -(fRadius + fDeltaCornerForLoop),
			-sens * fRadius, -fRadius);
	cairo_rel_line_to (pCairoContext, sens * (- pDock->iMaxIconHeight - fLineWidth + fRadius * (g_bRoundedBottomCorner ? 2 : 1)), fDeltaXForLoop);
	// Top Left.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-sens * fRadius, fDeltaCornerForLoop,
		-sens * fRadius, fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
}
void cairo_dock_calculate_construction_parameters (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, gboolean bLoop, gboolean bInside, double fLateralFactor, double fAlign)
{
	//\_____________________ On calcule leur position : en ligne droite sur l'avant-plan ou sur une ellipse en arriere-plan.
	double fDeltaLeft = (iMaxDockWidth - iCurrentWidth - icon->fWidth * icon->fScale) / 2, fDeltaRight = fDeltaLeft;
	double fX, fY, fAlpha, fTheta;
	fX = icon->fX + (iCurrentWidth - iMaxDockWidth) / 2;
	//fX = fAlign * iCurrentWidth + (fX - fAlign * iCurrentWidth) * (1 - fLateralFactor);
	//g_print ("(%s) icon->fX : %.2f -> %.2f\n", icon->acName, icon->fX, fX);
	if (fX >= 0 && fX + icon->fWidth * icon->fScale <= iCurrentWidth)
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
		double a, b;  // parametres de la demi-ellipse.
		if (! bInside || ! bLoop)
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
			fTheta = ((fX + icon->fWidth * icon->fScale / 2) / fDeltaLeft - 1) * G_PI / 2;
			fX = b * (1 + sin (fTheta));
			fY = (g_bDirectionUp ? a * (1 + MIN (0, cos (fTheta))) : - a * cos (fTheta) + g_iDockLineWidth);
			fAlpha = MAX (0.25, MIN (0.75, sin (fTheta) * sin (fTheta)));
			//g_print ("  theta = %.2fdeg -> fX = %.2f; fY = %.2f; alpha = %.2f\n", fTheta / G_PI*180, fX, fY, fAlpha);
		}
		else
		{
			a = iCurrentHeight - icon->fHeight * icon->fScale;
			b = (iCurrentWidth - icon->fWidth * icon->fScale) / 2;
			fTheta = ((fX + icon->fWidth * icon->fScale / 2 - iCurrentWidth) / fDeltaLeft + 1) * G_PI / 2;
			fX = b * (1 + sin (fTheta));
			fY = (g_bDirectionUp ? a * (1 + MIN (0, cos (fTheta))) : - a * cos (fTheta) + g_iDockLineWidth);
			fAlpha = MAX (0.25, MIN (0.75, sin (fTheta) * sin (fTheta)));
			//g_print ("  theta = %.2fdeg -> fX = %.2f; fY = %.2f; alpha = %.2f\n", fTheta / G_PI*180, fX, fY, fAlpha);
		}
	}
	
	//\_____________________ On gere l'animation de rebond.
	if (icon->iAnimationType == CAIRO_DOCK_BOUNCE && icon->iCount > 0)
	{
		int n = g_tNbIterInOneRound[CAIRO_DOCK_BOUNCE];  // nbre d'iteration pour une montree+descente.
		int k = n - (icon->iCount % n);
		
		double fPossibleDeltaY = (g_bDirectionUp ? fY : iCurrentHeight - (fY + icon->fHeight * icon->fScale));
		
		fY += (g_bDirectionUp ? -1. : 1.) * k / (n/2) * fPossibleDeltaY * (2 - 1.*k/(n/2));
		icon->iCount --;  // c'est une loi de type acceleration dans le champ de pesanteur. 'g' et 'v0' n'interviennent pas directement, car s'expriment en fonction de 'fPossibleDeltaY' et 'n'.
	}
	
	//\_____________________ On gere l'animation de rotation sur elle-meme.
	double fWidthFactor = 1.;
	if (icon->iAnimationType == CAIRO_DOCK_ROTATE && icon->iCount > 0)
	{
		int c = icon->iCount;
		int n = g_tNbIterInOneRound[CAIRO_DOCK_ROTATE] / 4;  // nbre d'iteration pour 1/2 tour.
		if ((c/n) & 1)
		{
			fWidthFactor *= ((c/(2*n)) & 1 ? 1. : -1.) * (c%n) / n;
		}
		else
		{
			fWidthFactor *= ((c/(2*n)) & 1 ? 1. : -1.) * ((c%n) - n) / n;
		}
		icon->iCount --;
	}
	
	fWidthFactor *= (G_PI / 2 - fabs (fTheta)) * 2 / G_PI;
	if (fWidthFactor >= 0 && fWidthFactor < 0.05)
		fWidthFactor = 0.05;
	else if (fWidthFactor < 0 && fWidthFactor > -0.05)
		fWidthFactor = -0.05;
	
	fX += (1 - fWidthFactor) / 2 * icon->fWidth * icon->fScale;
	
	//\_____________________ On gere l'animation de l'icone qui suit ou evite le curseur.
	if (icon->iAnimationType == CAIRO_DOCK_FOLLOW_MOUSE)
	{
		fX = icon->fDrawX + (iCurrentWidth - iMaxDockWidth) / 2 - icon->fWidth * icon->fScale / 2;
		fY = icon->fDrawY -icon->fHeight * icon->fScale / 2 ;
		fAlpha = 0.4;
	}
	else if (icon->iAnimationType == CAIRO_DOCK_AVOID_MOUSE)
	{
		fAlpha = 0.4;
		fX += icon->fWidth / 2 * (icon->fScale - 1) / g_fAmplitude * (icon->fPhase < G_PI/2 ? -1 : 1);
	}
	
	//\_____________________ On gere l'animation d'ondelette.
	if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_PULSE)
	{
		fAlpha = 1. * (icon->iCount % g_tNbIterInOneRound[CAIRO_DOCK_PULSE]) / g_tNbIterInOneRound[CAIRO_DOCK_PULSE];
		icon->iCount --;
	}
	
	//\_____________________ On gere l'animation de clignotement.
	if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_BLINK)
	{
		int c = icon->iCount;
		int n = g_tNbIterInOneRound[CAIRO_DOCK_BLINK] / 2;  // nbre d'iteration pour une inversion d'alpha.
		if ( (c/n) & 1)
			fAlpha *= 1. * (c%n) / n;
		else
			fAlpha *= 1. * (n - 1 - (c%n)) / n;
		fAlpha *= fAlpha;
		icon->iCount --;
	}
	
	icon->fDrawX = fX;  // fAlign * iCurrentWidth + (fX - fAlign * iCurrentWidth) * (1 - fLateralFactor);
	icon->fDrawY = fY;
	icon->fWidthFactor = fWidthFactor;  // son signe nous renseigne sur la position de l'icone (avant-plan ou arriere-plan).
	icon->fAlpha = fAlpha;
}
static void cairo_dock_render_one_icon (Icon *icon, cairo_t *pCairoContext, gboolean bHorizontalDock, double fRatio)
{
	//\_____________________ On dessine l'icone en fonction de son placement, son angle, et sa transparence.
	//cairo_push_group (pCairoContext);
	//g_print ("%s (%.2f;%.2f)\n", __func__, icon->fDrawX, icon->fDrawY);
	if (bHorizontalDock)
	{
		cairo_translate (pCairoContext, icon->fDrawX, icon->fDrawY);
		cairo_save (pCairoContext);
		cairo_scale (pCairoContext, fRatio * icon->fWidthFactor * icon->fScale / (1 + g_fAmplitude), fRatio * icon->fScale / (1 + g_fAmplitude));
	}
	else
	{
		cairo_translate (pCairoContext, icon->fDrawY, icon->fDrawX);
		cairo_save (pCairoContext);
		cairo_scale (pCairoContext, fRatio * icon->fScale / (1 + g_fAmplitude), fRatio * icon->fWidthFactor * icon->fScale / (1 + g_fAmplitude));
	}
	
	if (icon->iAnimationType == CAIRO_DOCK_PULSE)
	{
		if (icon->fAlpha > 0)
		{
			cairo_save (pCairoContext);
			double fScaleFactor = 1 + (1 - icon->fAlpha);
			if (bHorizontalDock)
				cairo_translate (pCairoContext, icon->fWidth * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fHeight * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
			else
				cairo_translate (pCairoContext, icon->fHeight * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fWidth * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
			cairo_scale (pCairoContext, fScaleFactor, fScaleFactor);
			if (icon->pIconBuffer != NULL)
				cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
			cairo_paint_with_alpha (pCairoContext, icon->fAlpha);
			cairo_restore (pCairoContext);
		}
		icon->fAlpha = .8;
	}
	
	if (icon->pIconBuffer != NULL)
		cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
	//cairo_pop_group (pCairoContext);
	
	if (icon->fAlpha == 1)
		cairo_paint (pCairoContext);
	else
		cairo_paint_with_alpha (pCairoContext, icon->fAlpha);
	cairo_restore (pCairoContext);
	
	//\_____________________ On dessine les etiquettes, avec un alpha proportionnel au facteur d'echelle de leur icone.
	if (icon->pTextBuffer != NULL && icon->fScale > 1.01 && (! g_bLabelForPointedIconOnly || icon->bPointed))  // 1.01 car sin(pi) = 1+epsilon :-/
	{
		if (! bHorizontalDock && g_bTextAlwaysHorizontal)
		{
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				0,
				0);
		}
		else if (bHorizontalDock)
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				-icon->fTextXOffset + icon->fWidthFactor * icon->fWidth * icon->fScale * 0.5,
				g_bDirectionUp ? -g_iLabelSize : icon->fHeight * icon->fScale + icon->fTextYOffset);
		else
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				g_bDirectionUp ? -g_iLabelSize : icon->fHeight * icon->fScale + icon->fTextYOffset,
				-icon->fTextXOffset + icon->fWidthFactor * icon->fWidth * icon->fScale * 0.5);
		double fMagnitude = (icon->fScale - 1) / g_fAmplitude;
		//cairo_paint_with_alpha (pCairoContext, (g_bLabelForPointedIconOnly ? 1.0 : pow (fMagnitude, 3)));
		cairo_paint_with_alpha (pCairoContext, (g_bLabelForPointedIconOnly ? 1.0 : (fMagnitude > 1. - 1. / g_fLabelAlphaThreshold ? 1.0 : fRatio / (1. - fMagnitude) / g_fLabelAlphaThreshold)));
	}
}

void cairo_dock_render (CairoDock *pDock)
{
	double fChangeAxes = 0.5 * (pDock->iCurrentWidth - pDock->iMaxDockWidth);
	double fLineWidth = g_iDockLineWidth;
	double fRadius = (pDock->iMaxIconHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDock->iMaxIconHeight + fLineWidth) / 2 - 1);
	double fDockWidth = cairo_dock_get_current_dock_width (pDock);
	gboolean bIsLoop = pDock->iRefCount == 0 && 1. * pDock->iCurrentWidth / pDock->iMaxDockWidth < .6 && pDock->bInside;
	//g_print ("%s (%.2f) %d / %d -> %d\n", __func__, fDockWidth, pDock->iCurrentWidth, pDock->iMaxDockWidth, bIsLoop);
	
	//\____________________ On determine des parametres de construction.
	int sens;
	double fDockOffsetX, fDockOffsetY;
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	//g_print ("1ere icone : %s, %.2f, %.2f\n", pFirstIcon->acName, pFirstIcon->fXAtRest, pFirstIcon->fX);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + fChangeAxes : g_iDockRadius + fLineWidth / 2);
	if (fDockOffsetX - (fRadius + fLineWidth / 2) < 0)
		fDockOffsetX = fRadius + fLineWidth / 2;
	if (fDockOffsetX + fDockWidth - (fRadius + fLineWidth / 2) > pDock->iCurrentWidth)
		fDockWidth = MAX (pDock->iCurrentWidth - fDockOffsetX + (fRadius + fLineWidth / 2), 2 * fRadius + fLineWidth);
	//g_print ("fDockOffsetX : %.2f ; fDockWidth : %.2f ; pFirstIcon->fX : %.2f\n", fDockOffsetX, fDockWidth, (pFirstIcon != NULL ? pFirstIcon->fX : 0));
	
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
	
	//\____________________ On cree le contexte du dessin.
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_set_tolerance (pCairoContext, 0.5);  // avec moins que 0.5 on ne voit pas la difference.
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\____________________ On trace un cadre, en commencant par le coin haut gauche.
	cairo_save (pCairoContext);
	if (pDock->bHorizontalDock)
		_cairo_dock_draw_frame_horizontal (pDock, pCairoContext, fRadius, fLineWidth, fDockWidth, fDockOffsetX, fDockOffsetY, sens, bIsLoop);
	else
		_cairo_dock_draw_frame_vertical (pDock, pCairoContext, fRadius, fLineWidth, fDockWidth, fDockOffsetX, fDockOffsetY, sens, bIsLoop);
	
	//\____________________ On dessine les decorations du fond.
	if (g_pBackgroundSurfaceFull[pDock->bHorizontalDock] != NULL)
	{
		cairo_save (pCairoContext);
		
		if (pDock->bHorizontalDock)
			cairo_translate (pCairoContext, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor - pDock->iCurrentWidth * 0.5, (g_bDirectionUp ? pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth : fLineWidth));
		else
			cairo_translate (pCairoContext, (g_bDirectionUp ? pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth : fLineWidth), pDock->fDecorationsOffsetX * g_fStripesSpeedFactor - pDock->iCurrentWidth * 0.5);
		
		cairo_set_source_surface (pCairoContext, g_pBackgroundSurfaceFull[pDock->bHorizontalDock], 0., 0.);
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	else if (g_pBackgroundSurface[pDock->bHorizontalDock] != NULL)
	{
		cairo_save (pCairoContext);
		
		if (pDock->bHorizontalDock)
		{
			cairo_translate (pCairoContext, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor, (g_bDirectionUp ? pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth: fLineWidth));
			cairo_scale (pCairoContext, 1. * pDock->iCurrentWidth / g_fBackgroundImageWidth, 1. * pDock->iMaxIconHeight / g_fBackgroundImageHeight);
		}
		else
		{
			cairo_translate (pCairoContext, (g_bDirectionUp ? pDock->iCurrentHeight - pDock->iMaxIconHeight - fLineWidth: fLineWidth), pDock->fDecorationsOffsetX * g_fStripesSpeedFactor);
			cairo_scale (pCairoContext, 1. * pDock->iMaxIconHeight / g_fBackgroundImageHeight, 1. * pDock->iCurrentWidth / g_fBackgroundImageWidth);
		}
		
		//g_print ("(%dx%d) / (%dx%d)\n", pDock->iCurrentWidth, pDock->iMaxIconHeight, (int) g_fBackgroundImageWidth, (int) g_fBackgroundImageHeight);
		
		cairo_set_source_surface (pCairoContext, g_pBackgroundSurface[pDock->bHorizontalDock], 0., 0.);
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	//\____________________ On dessine le cadre.
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	cairo_stroke (pCairoContext);
	cairo_restore (pCairoContext);
	
	//\____________________ On pre-calcule les position/etirements/alpha des icones.
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cairo_dock_calculate_construction_parameters (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, bIsLoop, pDock->bInside, pDock->fLateralFactor, pDock->fAlign);
	}
	
	//\____________________ On dessine la ficelle qui les joint.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL)
	{
		cairo_destroy (pCairoContext);
		return ;
	}
	
	if (g_iStringLineWidth > 0)
	{
		cairo_save (pCairoContext);
		Icon *prev_icon = NULL, *next_icon;
		if (bIsLoop)
		{
			ic = cairo_dock_get_previous_element (pFirstDrawnElement, pDock->icons);
			prev_icon = ic->data;
		}
		ic = pFirstDrawnElement;
		icon = ic->data;
		double x, y, fCurvature = 0.3;
		x = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor / 2;
		y = icon->fDrawY + icon->fHeight * icon->fScale / 2;
		GList *next_ic;
		double x1, x2, x3;
		double y1, y2, y3;
		double dx, dy;
		if (pDock->bHorizontalDock)
			cairo_move_to (pCairoContext, x, y);
		else
			cairo_move_to (pCairoContext, y, x);
		do
		{
			if (prev_icon != NULL)
			{
				x1 = prev_icon->fDrawX + prev_icon->fWidth * prev_icon->fScale * prev_icon->fWidthFactor / 2;
				y1 = prev_icon->fDrawY + prev_icon->fHeight * prev_icon->fScale / 2;
			}
			else
			{
				x1 = x;
				y1 = y;
			}
			prev_icon = icon;
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
			if (ic == pFirstDrawnElement && ! bIsLoop)
				break;
			icon = ic->data;
			x2 = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor / 2;
			y2 = icon->fDrawY + icon->fHeight * icon->fScale / 2;
			
			dx = x2 - x;
			dy = y2 - y;
			
			next_ic = cairo_dock_get_next_element (ic, pDock->icons);
			next_icon = (next_ic == pFirstDrawnElement && ! bIsLoop ? NULL : next_ic->data);
			if (next_icon != NULL)
			{
				x3 = next_icon->fDrawX + next_icon->fWidth * next_icon->fScale * next_icon->fWidthFactor / 2;
				y3 = next_icon->fDrawY + next_icon->fHeight * next_icon->fScale / 2;
			}
			else
			{
				x3 = x2;
				y3 = y2;
			}
			
			if (pDock->bHorizontalDock)
				cairo_rel_curve_to (pCairoContext,
					(fabs ((x - x1) / (y - y1)) > .35 ? dx * fCurvature : 0),
					(fabs ((x - x1) / (y - y1)) > .35 ? dx * fCurvature * (y - y1) / (x - x1) : 0),
					(fabs ((x3 - x2) / (y3 - y2)) > .35 ? dx * (1 - fCurvature) : dx),
					(fabs ((x3 - x2) / (y3 - y2)) > .35 ? MAX (0, MIN (dy, dy - dx * fCurvature * (y3 - y2) / (x3 - x2))) : dy),
					dx,
					dy);
			else
				cairo_rel_curve_to (pCairoContext,
					(fabs ((x - x1) / (y - y1)) > .35 ? dx * fCurvature * (y - y1) / (x - x1) : 0),
					(fabs ((x - x1) / (y - y1)) > .35 ? dx * fCurvature : 0),
					(fabs ((x3 - x2) / (y3 - y2)) > .35 ? MAX (0, MIN (dy, dy - dx * fCurvature * (y3 - y2) / (x3 - x2))) : dy),
					(fabs ((x3 - x2) / (y3 - y2)) > .35 ? dx * (1 - fCurvature) : dx),
					dy,
					dx);
			x = x2;
			y = y2;
		}
		while (ic != pFirstDrawnElement);
		cairo_set_line_width (pCairoContext, g_iStringLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fStringColor[0], g_fStringColor[1], g_fStringColor[2], g_fStringColor[3]);
		cairo_stroke (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	//g_print ("--------------------\n");
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	ic = pFirstDrawnElement;
	do
	{
		icon = (Icon*) ic->data;
		cairo_save (pCairoContext);
		
		//g_print ("redessin a gauche de %s\n", icon->acName);
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio);
		
		cairo_restore (pCairoContext);
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	}
	while (icon->fX + icon->fWidth * icon->fScale + fChangeAxes < 0 && ic != pFirstDrawnElement);
	
	GList *pMiddleElement = ic;
	ic = pFirstDrawnElement->prev;
	if (ic == NULL)
		ic = g_list_last (pDock->icons);
	do
	{
		icon = (Icon*) ic->data;
		cairo_save (pCairoContext);
		
		//g_print ("redessin a droite de %s\n", icon->acName);
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio);
		
		cairo_restore (pCairoContext);
		if (ic == pMiddleElement)
			break;
		ic = cairo_dock_get_previous_element (ic, pDock->icons);
	}
	while (TRUE);
	
	cairo_destroy (pCairoContext);
	
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}

void cairo_dock_render_background (CairoDock *pDock)
{
	//g_print ("%s (%.2f)\n", __func__, g_fVisibleZoneAlpha);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_move_to (pCairoContext, 0, 0);
	if (g_pVisibleZoneSurface != NULL)
	{
		cairo_set_source_surface (pCairoContext, g_pVisibleZoneSurface, 0.0, 0.0);
		cairo_paint_with_alpha (pCairoContext, g_fVisibleZoneAlpha);
	}
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}

void cairo_dock_render_blank (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}



void cairo_dock_redraw_my_icon (Icon *icon, CairoDock *pDock)
{
	if (pDock->bAtBottom && (pDock->iRefCount > 0 || g_bAutoHide))
		return ;
	GdkRectangle rect = {(int) round (icon->fDrawX + MIN (0, icon->fWidth * icon->fScale * icon->fWidthFactor)), (int) icon->fDrawY, (int) round (icon->fWidth * icon->fScale * fabs (icon->fWidthFactor)), (int) icon->fHeight * icon->fScale};
	if (! pDock->bHorizontalDock)
	{
		rect.x = (int) icon->fDrawY;
		rect.y = (int) round (icon->fDrawX + MIN (0, icon->fWidth * icon->fScale * icon->fWidthFactor));
		rect.width = (int) icon->fHeight * icon->fScale;
		rect.height = (int) round (icon->fWidth * icon->fScale * fabs (icon->fWidthFactor));
	}
	//g_print ("rect (%d;%d) (%dx%d)\n", rect.x, rect.y, rect.width, rect.height);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		gtk_widget_queue_draw (pDock->pWidget);
	else
#endif
	gdk_window_invalidate_rect (pDock->pWidget->window, &rect, FALSE);
}


void cairo_dock_render_optimized (CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ((%d;%d) x (%d;%d) / (%dx%d))\n", __func__, pArea->x, pArea->y, pArea->width, pArea->height, pDock->iCurrentWidth, pDock->iCurrentHeight);
	double fLineWidth = g_iDockLineWidth;
	int iWidth = pDock->iCurrentWidth;
	int iHeight = pDock->iCurrentHeight;
	gboolean bIsLoop = pDock->iRefCount == 0 && 1. * pDock->iCurrentWidth / pDock->iMaxDockWidth < .6 && pDock->bInside;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	/* set rendering-"fidelity" and clear canvas */
	cairo_set_tolerance (pCairoContext, 0.5);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\____________________ On dessine les rayures du fond sur la portion de fenetre.
	cairo_save (pCairoContext);
	
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bHorizontalDock)
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
	if (pDock->bHorizontalDock)
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, pDock->iMaxIconHeight);
	else
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pDock->iMaxIconHeight, pArea->height);
	if (g_pBackgroundSurfaceFull[pDock->bHorizontalDock] != NULL)
	{
		cairo_save (pCairoContext);
		
		if (pDock->bHorizontalDock)
			cairo_translate (pCairoContext, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor - iWidth * 0.5, (g_bDirectionUp ? iHeight - pDock->iMaxIconHeight - fLineWidth: fLineWidth));
		else
			cairo_translate (pCairoContext, (g_bDirectionUp ? iHeight - pDock->iMaxIconHeight - fLineWidth: fLineWidth), pDock->fDecorationsOffsetX * g_fStripesSpeedFactor - iWidth * 0.5);
		cairo_set_source_surface (pCairoContext, g_pBackgroundSurfaceFull[pDock->bHorizontalDock], 0., 0.);
		
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	else if (g_pBackgroundSurface[pDock->bHorizontalDock] != NULL)
	{
		cairo_save (pCairoContext);
		
		if (pDock->bHorizontalDock)
		{
			cairo_translate (pCairoContext, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor - iWidth / 2, fDockOffsetY);
			cairo_scale (pCairoContext, 1. * iWidth / g_fBackgroundImageWidth, 1. * pDock->iMaxIconHeight / g_fBackgroundImageHeight);
		}
		else
		{
			cairo_translate (pCairoContext, 0, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor - iWidth / 2);
			cairo_scale (pCairoContext, 1. * pDock->iMaxIconHeight / g_fBackgroundImageHeight, 1. * iWidth / g_fBackgroundImageWidth);
		}
		
		cairo_set_source_surface (pCairoContext, g_pBackgroundSurface[pDock->bHorizontalDock], 0., 0.);
		
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
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
	
	//\____________________ On dessine les icones impactees.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	double fXMin = (pDock->bHorizontalDock ? pArea->x : pArea->y), fXMax = (pDock->bHorizontalDock ? pArea->x + pArea->width : pArea->y + pArea->height);
	GList *ic;
	Icon *icon;
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	ic = pFirstDrawnElement;
	
	if (ic != NULL)
	{
		double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
		double fXLeft, fXRight;  // il faut tenir compte de l'inversion si l'icone est en arriere-plan.
		do
		{
			icon = (Icon*) ic->data;
			
			if (icon->fWidthFactor < 0)
			{
				fXLeft = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor;
				fXRight = icon->fDrawX;
			}
			else
			{
				fXLeft = icon->fDrawX;
				fXRight = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor;
			}
			
			//g_print ("test a gauche de %s (%.2f -> %.2f)\n", icon->acName, fXLeft, fXRight);
			if (fXLeft <= fXMax && floor (fXRight) > fXMin)
			{
				cairo_save (pCairoContext);
				
				//g_print ("  redessin a gauche de %s\n", icon->acName);
				cairo_dock_calculate_construction_parameters (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, bIsLoop, pDock->bInside, pDock->fLateralFactor, pDock->fAlign);
				cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio);
				
				cairo_restore (pCairoContext);
			}
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
			
			if (icon->fWidthFactor < 0)
			{
				fXLeft = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor;
				fXRight = icon->fDrawX;
			}
			else
			{
				fXLeft = icon->fDrawX;
				fXRight = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor;
			}
			
			//g_print ("test a droite de %s (%.2f -> %.2f)\n", icon->acName, fXLeft, fXRight);
			if (fXLeft <= fXMax && floor (fXRight) > fXMin)
			{
				cairo_save (pCairoContext);
				
				//g_print ("  redessin a droite de %s\n", icon->acName);
				cairo_dock_calculate_construction_parameters (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, bIsLoop, pDock->bInside, pDock->fLateralFactor, pDock->fAlign);
				cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio);
				
				cairo_restore (pCairoContext);
			}
			if (ic == pMiddleElement)
				break;
			ic = ic->prev;
			if (ic == NULL)
				ic = g_list_last (pDock->icons);
		}
		while (TRUE);
	}
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}


static gboolean _cairo_dock_hide_dock (gchar *cDockName, CairoDock *pDock, CairoDock *pChildDock)
{
	if (pDock->bInside)
		return FALSE;
	
	Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	if (pPointedIcon == NULL || pPointedIcon->pSubDock != pChildDock)
		pPointedIcon = cairo_dock_get_icon_with_subdock (pDock->icons, pChildDock);
	
	if (pPointedIcon != NULL)
	{
		//g_print (" il faut cacher ce dock parent\n");
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
				cairo_dock_render (pDock);
			}
			
			//g_print ("on cache %s par parente\n", cDockName);
			gtk_widget_hide (pDock->pWidget);
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

gboolean cairo_dock_hide_child_docks (CairoDock *pDock)
{
	GList* ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->pSubDock != NULL && GTK_WIDGET_VISIBLE (icon->pSubDock->pWidget))
		{
			if (icon->pSubDock->bInside)
			{
				//g_print ("on est dans le sous-dock, donc on ne le cache pas\n");
				pDock->bInside = FALSE;
				pDock->bAtTop = FALSE;
				return FALSE;
			}
			else  // si on sort du dock sans passer par le sous-dock, par exemple en sortant par le bas.
			{
				//g_print ("on cache %s en sortant du dock principal\n", pPointedIcon->acName);
				//while (gtk_events_pending ())
				//	gtk_main_iteration ();
				//g_print ("on cache %s par filiation\n", icon->acName);
				icon->pSubDock->iScrollOffset = 0;
				icon->pSubDock->fLateralFactor = 0;
				gtk_widget_hide (icon->pSubDock->pWidget);
			}
		}
	}
	return TRUE;
}


void cairo_dock_calculate_window_position_at_balance (CairoDock *pDock, CairoDockSizeType iSizeType, int *iNewWidth, int *iNewHeight)
{
	if (iSizeType == CAIRO_DOCK_MAX_SIZE)
	{
		*iNewWidth = (g_bForceLoop && pDock->iRefCount == 0 ? pDock->iMaxDockWidth / 2 : MIN (g_iMaxAuthorizedWidth, pDock->iMaxDockWidth));
		*iNewHeight = pDock->iMaxDockHeight;
		pDock->iWindowPositionX = (g_iScreenWidth[pDock->bHorizontalDock] - *iNewWidth) * pDock->fAlign + pDock->iGapX;
		pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - (*iNewHeight) - pDock->iGapY : pDock->iGapY);
	}
	else if (iSizeType == CAIRO_DOCK_NORMAL_SIZE)
	{
		*iNewWidth = MIN (g_iMaxAuthorizedWidth, pDock->iMinDockWidth  + 2 * g_iDockRadius + g_iDockLineWidth);
		if (g_bForceLoop && *iNewWidth > pDock->iMaxDockWidth / 2)
			*iNewWidth = pDock->iMaxDockWidth / 2;
		*iNewHeight = pDock->iMaxIconHeight + 2 * g_iDockLineWidth;
		pDock->iWindowPositionX = (g_iScreenWidth[pDock->bHorizontalDock] - *iNewWidth) * pDock->fAlign + pDock->iGapX;
		pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - (*iNewHeight) - pDock->iGapY : pDock->iGapY);
	}
	else
	{
		*iNewWidth = g_iVisibleZoneWidth;
		*iNewHeight = g_iVisibleZoneHeight;
		pDock->iWindowPositionX = (g_iScreenWidth[pDock->bHorizontalDock] - *iNewWidth) * pDock->fAlign + pDock->iGapX;
		pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - (*iNewHeight) - pDock->iGapY : pDock->iGapY);
	}
	
	if (pDock->iWindowPositionX < 0)
		pDock->iWindowPositionX = 0;
	else if (pDock->iWindowPositionX > g_iScreenWidth[pDock->bHorizontalDock])
		pDock->iWindowPositionX = g_iScreenWidth[pDock->bHorizontalDock];
	
	if (pDock->iWindowPositionY < 0)
		pDock->iWindowPositionY = 0;
	else if (pDock->iWindowPositionY > g_iScreenHeight[pDock->bHorizontalDock])
		pDock->iWindowPositionY = g_iScreenHeight[pDock->bHorizontalDock];
}
