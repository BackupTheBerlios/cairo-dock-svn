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
#include "cairo-dock-callbacks.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-animations.h"

extern GHashTable *g_hDocksTable;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern int g_iMaxAuthorizedWidth;

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
extern double g_fLabelAlphaThreshold;

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
extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;

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


gboolean cairo_dock_move_up (CairoDock *pDock)
{
	int deltaY_possible;
	deltaY_possible = pDock->iWindowPositionY - (g_bDirectionUp ? g_iScreenHeight - pDock->iMaxDockHeight - pDock->iGapY : pDock->iGapY);
	//g_print ("%s (%dx%d -> %d)\n", __func__, pDock->iWindowPositionX, pDock->iWindowPositionY, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 0) || (! g_bDirectionUp && deltaY_possible < 0))  // alors on peut encore monter.
	{
			pDock->iWindowPositionY -= (int) (deltaY_possible * g_fMoveUpSpeed) + (g_bDirectionUp ? 1 : -1);
		//g_print ("  move to (%dx%d)\n", g_iWindowPositionX, g_iWindowPositionY);
		if (g_bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionY, pDock->iWindowPositionX);
		pDock->bAtBottom = FALSE;
		return TRUE;
	}
	else
	{
		pDock->bAtTop = TRUE;
		pDock->iSidMoveUp = 0;
		return FALSE;
	}
}

gboolean cairo_dock_move_down (CairoDock *pDock)
{
	if (pDock->fMagnitude > 0.1)  // on retarde le cachage du dock pour apercevoir les effets.
		return TRUE;
	int deltaY_possible = (g_bDirectionUp ? g_iScreenHeight - pDock->iGapY - g_iVisibleZoneHeight : pDock->iGapY + g_iVisibleZoneHeight - pDock->iMaxDockHeight) - pDock->iWindowPositionY;
	//g_print ("%s (%d)\n", __func__, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 4) || (! g_bDirectionUp && deltaY_possible < -4))  // alors on peut encore descendre.
	{
		pDock->iWindowPositionY += (int) (deltaY_possible * g_fMoveDownSpeed) + (g_bDirectionUp ? 1 : -1);  // 0.33
		if (g_bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionY, pDock->iWindowPositionX);
		pDock->bAtTop = FALSE;
		return TRUE;
	}
	else  // on se fixe en bas, et on montre la zone visible.
	{
		pDock->bAtBottom = TRUE;
		//pDock->iWindowPositionX = (g_iScreenWidth - g_iVisibleZoneWidth) / 2 + pDock->iGapX;
		//pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - g_iVisibleZoneHeight - pDock->iGapY : pDock->iGapY);
		cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_MIN_SIZE);
		//g_print ("%s () -> %dx%d\n", __func__, g_iVisibleZoneWidth, g_iVisibleZoneHeight);
		if (g_bHorizontalDock)
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionX,
				pDock->iWindowPositionY,
				g_iVisibleZoneWidth,
				g_iVisibleZoneHeight);
		else
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionY,
				pDock->iWindowPositionX,
				g_iVisibleZoneHeight,
				g_iVisibleZoneWidth);
		pDock->iSidMoveDown = 0;
		
		if ((g_bAutoHide && pDock->iRefCount == 0))
		{
			Icon *pBouncingIcon = cairo_dock_get_bouncing_icon (pDock->icons);
			if (pBouncingIcon != NULL)  // s'il y'a une icone en cours d'animation, on l'arrete.
			{
				pBouncingIcon->iCount = 0;
			}
			Icon *pRemovingIcon = cairo_dock_get_removing_or_inserting_icon (pDock->icons);
			if (pRemovingIcon != NULL)  // idem.
			{
				pRemovingIcon->fPersonnalScale = 0.001;
			}
		}
		
		return FALSE;
	}
}



gboolean cairo_dock_grow_up (CairoDock *pDock)
{
	//g_print ("%s (%f)\n", __func__, g_fMagnitude);
	if (pDock->fMagnitude < 0.05)
		pDock->fMagnitude = 0.05;
	
	pDock->fMagnitude *= g_fGrowUpFactor;  // 1.4
	
	if (pDock->fMagnitude > 1.0)
		pDock->fMagnitude = 1.0;
	
	gint iMouseX, iMouseY;
	if (g_bHorizontalDock)
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
	else
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseY, &iMouseX, NULL);
	
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

gboolean cairo_dock_shrink_down (CairoDock *pDock)
{
	//g_print ("%s (%f)\n", __func__, pDock->fMagnitude);
	if (pDock->fMagnitude > 0.05)
		pDock->fMagnitude *= g_fShrinkDownFactor; //  0.6
	else
		pDock->fMagnitude = 0.0;
		
	gint iMouseX, iMouseY;
	if (g_bHorizontalDock)
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
	else
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseY, &iMouseX, NULL);
	
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
			
			if (! (g_bAutoHide && pDock->iRefCount == 0) && ! pDock->bInside)
			{
				//g_print ("on arrive en bas -> %dx%d\n", g_iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth, g_iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth);
				cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_NORMAL_SIZE);
				if (g_bHorizontalDock)
					gdk_window_move_resize (pDock->pWidget->window,
						pDock->iWindowPositionX,
						pDock->iWindowPositionY,
						MIN (g_iMaxAuthorizedWidth, pDock->iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth),
						pDock->iMaxIconHeight + 2 * g_iDockLineWidth);
				else
					gdk_window_move_resize (pDock->pWidget->window,
						pDock->iWindowPositionY,
						pDock->iWindowPositionX,
						pDock->iMaxIconHeight + 2 * g_iDockLineWidth,
						MIN (g_iMaxAuthorizedWidth, pDock->iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth));
			}
			
			gint iMouseX, iMouseY;
			if (g_bHorizontalDock)
				gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
			else
				gdk_window_get_pointer (pDock->pWidget->window, &iMouseY, &iMouseX, NULL);
			
			cairo_dock_calculate_icons (pDock, iMouseX, iMouseY);  // relance le grossissement si on est dedans.
			if (! pDock->bInside && pDock->iRefCount > 0)
			{
					gdk_window_hide (pDock->pWidget->window);
					cairo_dock_hide_parent_docks (pDock);
			}
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
