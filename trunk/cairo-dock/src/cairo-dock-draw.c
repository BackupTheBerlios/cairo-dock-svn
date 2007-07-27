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
#include "cairo-dock-draw.h"


extern GList* icons;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern gint g_iCurrentWidth;
extern gint g_iCurrentHeight;

extern float g_fMagnitude;

extern gboolean g_bAtBottom;
extern gboolean g_bAtTop;
extern gboolean g_bInside;

extern gint g_iWindowPositionX;
extern gint g_iWindowPositionY;
extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern int g_iIconGap;
extern int g_iLabelSize;
extern gboolean g_bRoundedBottomCorner;
extern gboolean g_bAutoHide;
extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern gdouble g_fGradientOffsetX;
extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];
extern int g_iNbStripes;
extern double g_fStripesWidth;

extern int g_iMaxDockWidth;
extern int g_iMaxDockHeight;
extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_iGapX;
extern int g_iGapY;
extern int g_iMaxIconHeight;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern double g_fVisibleZoneAlpha;
extern int g_iMinDockWidth;
extern double g_fAmplitude;
extern int g_iSinusoidWidth;

extern int g_iSidMoveDown;
extern int g_iSidMoveUp;
extern int g_iSidGrowUp;
extern int g_iSidShrinkDown;

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



double get_current_dock_width ()
{
	if (icons == NULL)
		return 2 * g_iDockRadius + g_iDockLineWidth;
	Icon *icon = get_last_icon();
	double fWidth = icon->fX + icon->fWidth * icon->fScale - get_first_icon()->fX + 2 * g_iDockRadius + g_iDockLineWidth;
	
	return fWidth;
}

double get_dock_offset_y ()
{
  return  g_iMaxDockHeight - g_iMaxIconHeight - g_iDockLineWidth;
}

double get_current_dock_offset_x ()
{
  return get_first_icon()->fX;
}

double get_current_dock_offset_y ()
{
  return  g_iMaxDockHeight - g_iMaxIconHeight - g_iDockLineWidth;
}


void cairo_dock_update_dock_size (GtkWidget *pWidget, int iMaxIconHeight, int iMinDockWidth)
{
	//g_print ("%s (%d, %d)\n", __func__, iMaxIconHeight, iMinDockWidth);
	g_iMaxDockHeight = (int) ((1 + g_fAmplitude) * iMaxIconHeight) + g_iLabelSize;
	g_iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (iMinDockWidth)) + 1;  // + 1 pour gerer les largeur impaire.
	cairo_dock_calculate_icons (pWidget, g_fMagnitude);
	
	if (! g_bAutoHide)
	{
		g_iVisibleZoneWidth = iMinDockWidth + 2 * (g_iDockRadius + g_iDockLineWidth);
		g_iVisibleZoneHeight = g_iMaxIconHeight + 2 * g_iDockLineWidth;
	}
	
	if (! g_bInside && g_bAutoHide)
		return;
	else if (g_bInside)
	{
		g_iWindowPositionX = (g_iScreenWidth - g_iMaxDockWidth) / 2 + g_iGapX;
		if (! g_bAutoHide)
			g_iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - g_iMaxDockHeight - g_iGapY : g_iScreenHeight - g_iGapY);
		else
			g_iWindowPositionY = (g_bDirectionUp ? g_iWindowPositionY : g_iVisibleZoneHeight - g_iMaxDockHeight + (g_iScreenHeight - g_iGapY));
		//g_print ("%s () -> %dx%d\n", __func__, g_iMaxDockWidth, g_iMaxDockHeight);
		gdk_window_move_resize (pWidget->window,
			g_iWindowPositionX,
			g_iWindowPositionY,
			g_iMaxDockWidth,
			g_iMaxDockHeight);
	}
	else
	{
		g_iWindowPositionX = (g_iScreenWidth - (g_iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth)) / 2 + g_iGapX;
		g_iWindowPositionY = g_iScreenHeight - g_iGapY - (g_bDirectionUp ? g_iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth : 0);
		//g_print ("%s () -> %dx%d\n", __func__, g_iMaxDockWidth, g_iMaxDockHeight);
		gdk_window_move_resize (pWidget->window,
			g_iWindowPositionX,
			g_iWindowPositionY,
			g_iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth,
			g_iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth);
	}
	
	
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


void render (GtkWidget *pWidget)
{
	//g_print ("%s ()\n", __func__);
	double fRadius = g_iDockRadius;
	double fLineWidth = g_iDockLineWidth;
	double fDockWidth = get_current_dock_width ();
	gint iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pWidget), &iWidth, &iHeight);
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
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
	if (!pCairoContext)
		return ;
	cairo_pattern_t* pPattern = NULL;

	/* set rendering-"fidelity" and clear canvas */
	cairo_set_tolerance (pCairoContext, 0.1);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\_________________ On dessine les rayures du fond.
	cairo_save (pCairoContext);
	if (g_iNbStripes > 0)
	{
		pPattern = cairo_pattern_create_linear (0.0f,
			0.0f,
			100.0f,
			(gdouble) iHeight);
		if (cairo_pattern_status (pPattern) == CAIRO_STATUS_SUCCESS)
		{
			cairo_matrix_t matrix;
			
			cairo_pattern_set_extend (pPattern, CAIRO_EXTEND_REPEAT);
			if (g_bHorizontalDock)
				cairo_matrix_init_translate (&matrix, g_fGradientOffsetX, 10.0f);
			else
				cairo_matrix_init_translate (&matrix, 10.0f, g_fGradientOffsetX);
			cairo_pattern_set_matrix (pPattern, &matrix);
			// This is CPU heavy!
			gdouble fStep;
			double fStripesGap = 1. / (g_iNbStripes);  // ecart entre 2 rayures foncees.
			for (fStep = 0.0f; fStep < 1.0f; fStep += fStripesGap)
			{
				cairo_pattern_add_color_stop_rgba (pPattern,
					fStep - g_fStripesWidth / 2,
					g_fStripesColorBright[0],
					g_fStripesColorBright[1],
					g_fStripesColorBright[2],
					g_fStripesColorBright[3]);
				cairo_pattern_add_color_stop_rgba (pPattern,
					fStep,
					g_fStripesColorDark[0],
					g_fStripesColorDark[1],
					g_fStripesColorDark[2],
					g_fStripesColorDark[3]);
				cairo_pattern_add_color_stop_rgba (pPattern,
					fStep + g_fStripesWidth / 2,
					g_fStripesColorBright[0],
					g_fStripesColorBright[1],
					g_fStripesColorBright[2],
					g_fStripesColorBright[3]);
			}
			
			cairo_set_source (pCairoContext, pPattern);
			cairo_pattern_destroy (pPattern);
		}
	}

	//\_________________ On dessine le cadre, en commencant par le coin haut gauche.
	int sens;
	double fDockOffsetY;
	double fDockOffsetX = get_current_dock_offset_x ();
	if (g_bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = iHeight - g_iMaxIconHeight - fLineWidth / 2;
	}
	else
	{
		sens = -1;
		fDockOffsetY = g_iMaxIconHeight + fLineWidth / 2;
	}
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	
	cairo_rel_line_to (pCairoContext, fDockWidth - (2 * fRadius + fLineWidth), 0);
	
	// Top Right.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens * (g_iMaxIconHeight - fRadius * (g_bRoundedBottomCorner ? 2 : 1)));
	
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
	cairo_rel_line_to (pCairoContext, 0, sens * (- g_iMaxIconHeight + fRadius * (g_bRoundedBottomCorner ? 2 : 1)));
	
	// Top Left.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (! g_bDirectionUp)
		cairo_move_to (pCairoContext, fDockOffsetX, iHeight - g_iMaxIconHeight - fLineWidth / 2);
	
	if (g_iNbStripes > 0)
		cairo_fill_preserve (pCairoContext);
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
	for (ic = icons; ic != NULL; ic = ic->next)
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

void cairo_dock_render_background (GtkWidget *pWidget)
{
	//g_print ("%s ()\n", __func__);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
	
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

void cairo_dock_render_blank (GtkWidget *pWidget)
{
	//g_print ("%s ()\n", __func__);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
	
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);

	cairo_destroy (pCairoContext);
}


gboolean grow_up2 (GtkWidget* pWidget)
{
	//g_print ("%s (%f)\n", __func__, g_fMagnitude);
	if (g_fMagnitude < 0.05)
		g_fMagnitude = 0.05;
	
	g_fMagnitude *= g_fGrowUpFactor;  // 1.4
	
	if (g_fMagnitude > 1.0)
		g_fMagnitude = 1.0;
				
	cairo_dock_calculate_icons (pWidget, g_fMagnitude);
	gtk_widget_queue_draw (pWidget);
	
	if (g_fMagnitude == 1)
	{
		g_iSidGrowUp = 0;
		return FALSE;
	}
	else
		return TRUE;
}

gboolean shrink_down2 (GtkWidget* pWidget)
{
	//g_print ("%s (%f)\n", __func__, g_fMagnitude);
	if (g_fMagnitude > 0.05)
		g_fMagnitude *= g_fShrinkDownFactor; //  0.6
	else
		g_fMagnitude = 0.0;
		
	cairo_dock_calculate_icons (pWidget, g_fMagnitude);
	gtk_widget_queue_draw (pWidget);
	
	if (g_fMagnitude < 0.05)
	{
		Icon *pBouncingIcon = cairo_dock_get_bouncing_icon ();
		Icon *pRemovingIcon = cairo_dock_get_removing_or_inserting_icon ();
		
		if (pBouncingIcon == NULL && pRemovingIcon == NULL)
		{
			g_fMagnitude = 0;
			g_iSidShrinkDown = 0;
			
			if (! g_bAutoHide && ! g_bInside)
			{
				//g_print ("on arrive en bas -> %dx%d\n", g_iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth, g_iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth);
				g_iWindowPositionX = (g_iScreenWidth - (g_iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth)) / 2 + g_iGapX;
				g_iWindowPositionY = g_iScreenHeight - g_iGapY - (g_bDirectionUp ? g_iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth : 0);
				gdk_window_move_resize (pWidget->window,
					g_iWindowPositionX,
					g_iWindowPositionY,
					g_iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth,
					g_iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth);
			}
			
			cairo_dock_calculate_icons (pWidget, 0);  // relance le grossissement si on est dedans.
			return FALSE;
		}
		
		//\______________ Au moins une icone est en cours d'animation suite a un clique, on continue le 'shrink_down'.
		if (pRemovingIcon != NULL)
		{
			//g_print ("au moins 1 icone en cours d'insertion/suppression (%f)\n", pRemovingIcon->fPersonnalScale);
			if (pRemovingIcon->fPersonnalScale == 0.05)
			{
				//g_print ("  fin\n");
				cairo_dock_remove_icon_from_dock (pRemovingIcon);
				cairo_dock_update_dock_size (pWidget, g_iMaxIconHeight, g_iMinDockWidth);
				cairo_dock_free_icon (pRemovingIcon);
			}
			else if (pRemovingIcon->fPersonnalScale == -0.05)
			{
				//g_print ("  fin\n");
				pRemovingIcon->fPersonnalScale = 0;
			}
		}
		
		g_fMagnitude = 0.001;  // on garde la magnitude > 0 de facon a ce qu'un motion_notify ne commence pas un 'grow_up'.
		return TRUE;
	}
	else
		return TRUE;
}



void cairo_dock_redraw_my_icon (Icon *icon)
{
	GdkRectangle rect = {(int) icon->fX, (int) icon->fY, (int) icon->fWidth * icon->fScale, (int) icon->fHeight * icon->fScale};
	gdk_window_invalidate_rect (g_pWidget->window, &rect, FALSE);
}


void cairo_dock_render_optimized (GtkWidget *pWidget, GdkRectangle *pArea)
{
	//g_print ("%s ()\n", __func__);
	double fLineWidth = g_iDockLineWidth;
	gint iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pWidget), &iWidth, &iHeight);
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
	if (!pCairoContext)
		return ;
	cairo_pattern_t* pPattern = NULL;

	/* set rendering-"fidelity" and clear canvas */
	cairo_set_tolerance (pCairoContext, 0.1);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\_________________ On dessine les rayures du fond.
	cairo_save (pCairoContext);
	if (g_iNbStripes > 0)
	{
		pPattern = cairo_pattern_create_linear (0.0f,
			0.0f,
			100.0f,
			(gdouble) iHeight);
		if (cairo_pattern_status (pPattern) == CAIRO_STATUS_SUCCESS)
		{
			cairo_matrix_t matrix;
			
			cairo_pattern_set_extend (pPattern, CAIRO_EXTEND_REPEAT);
			if (g_bHorizontalDock)
				cairo_matrix_init_translate (&matrix, g_fGradientOffsetX, 10.0f);
			else
				cairo_matrix_init_translate (&matrix, 10.0f, g_fGradientOffsetX);
			cairo_pattern_set_matrix (pPattern, &matrix);
			// This is CPU heavy!
			gdouble fStep;
			double fStripesGap = 1. / (g_iNbStripes);  // ecart entre 2 rayures foncees.
			for (fStep = 0.0f; fStep < 1.0f; fStep += fStripesGap)
			{
				cairo_pattern_add_color_stop_rgba (pPattern,
					fStep - g_fStripesWidth / 2,
					g_fStripesColorBright[0],
					g_fStripesColorBright[1],
					g_fStripesColorBright[2],
					g_fStripesColorBright[3]);
				cairo_pattern_add_color_stop_rgba (pPattern,
					fStep,
					g_fStripesColorDark[0],
					g_fStripesColorDark[1],
					g_fStripesColorDark[2],
					g_fStripesColorDark[3]);
				cairo_pattern_add_color_stop_rgba (pPattern,
					fStep + g_fStripesWidth / 2,
					g_fStripesColorBright[0],
					g_fStripesColorBright[1],
					g_fStripesColorBright[2],
					g_fStripesColorBright[3]);
			}
			
			cairo_set_source (pCairoContext, pPattern);
			cairo_pattern_destroy (pPattern);
		}
	}
	
	
	double fDockOffsetY;
	double fDockOffsetX = pArea->x;
	fDockOffsetY = iHeight - g_iMaxIconHeight - fLineWidth / 2;
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, g_iMaxIconHeight);
	cairo_fill_preserve (pCairoContext);
	
	
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
	for (ic = icons; ic != NULL; ic = ic->next)
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

