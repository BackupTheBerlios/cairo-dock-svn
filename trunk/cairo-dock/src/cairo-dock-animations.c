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
#include "cairo-dock-animations.h"

extern double g_fScrollAcceleration;
extern gboolean g_bResetScrollOnLeave;

extern int g_iScreenHeight[2];

extern gboolean g_bAutoHide;
extern gboolean g_bDirectionUp;

extern int g_iVisibleZoneHeight;

extern double g_fUnfoldAcceleration;
extern double g_fGrowUpFactor;
extern double g_fShrinkDownFactor;
extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;


gboolean cairo_dock_move_up (CairoDock *pDock)
{
	int deltaY_possible;
	deltaY_possible = pDock->iWindowPositionY - (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - pDock->iMaxDockHeight - pDock->iGapY : pDock->iGapY);
	//g_print ("%s (%dx%d -> %d)\n", __func__, pDock->iWindowPositionX, pDock->iWindowPositionY, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 0) || (! g_bDirectionUp && deltaY_possible < 0))  // alors on peut encore monter.
	{
		pDock->iWindowPositionY -= (int) (deltaY_possible * g_fMoveUpSpeed) + (g_bDirectionUp ? 1 : -1);
		//g_print ("  move to (%dx%d)\n", g_iWindowPositionX, g_iWindowPositionY);
		if (pDock->bHorizontalDock)
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
	//g_print ("%s ()\n", __func__);
	if (pDock->fMagnitude > 0.0)  // on retarde le cachage du dock pour apercevoir les effets.
		return TRUE;
	int deltaY_possible = (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - pDock->iGapY - g_iVisibleZoneHeight : pDock->iGapY + g_iVisibleZoneHeight - pDock->iMaxDockHeight) - pDock->iWindowPositionY;
	//g_print ("%s (%d)\n", __func__, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 8) || (! g_bDirectionUp && deltaY_possible < -8))  // alors on peut encore descendre.
	{
		pDock->iWindowPositionY += (int) (deltaY_possible * g_fMoveDownSpeed) + (g_bDirectionUp ? 1 : -1);  // 0.33
		if (pDock->bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionY, pDock->iWindowPositionX);
		pDock->bAtTop = FALSE;
		return TRUE;
	}
	else  // on se fixe en bas, et on montre la zone visible.
	{
		//g_print ("  on se fixe en bas\n");
		pDock->bAtBottom = TRUE;
		int iNewWidth, iNewHeight;
		cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_MIN_SIZE, &iNewWidth, &iNewHeight);
		if (pDock->bHorizontalDock)
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionX,
				pDock->iWindowPositionY,
				iNewWidth,
				iNewHeight);
		else
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionY,
				pDock->iWindowPositionX,
				iNewHeight,
				iNewWidth);
		pDock->iSidMoveDown = 0;
		
		if (g_bAutoHide && pDock->iRefCount == 0)
		{
			//g_print ("on arrete les animations\n");
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
			pDock->iScrollOffset = 0;
			
			pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, pDock->iMinDockWidth, pDock->iScrollOffset);
			pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->iMinDockWidth)) + 1;
			pDock->fLateralFactor = g_fUnfoldAcceleration;
		}
		
		gtk_widget_queue_draw (pDock->pWidget);
		
		return FALSE;
	}
}



gboolean cairo_dock_grow_up (CairoDock *pDock)
{
	//g_print ("%s (%f ; %f ; %d)\n", __func__, pDock->fMagnitude, pDock->fLateralFactor, pDock->iSidShrinkDown);
	if (pDock->iSidShrinkDown != 0)
		return TRUE;  // on se met en attente de fin d'animation.
	
	pDock->fMagnitude *= g_fGrowUpFactor;  // 1.4
	if (pDock->fMagnitude < 0.05)
		pDock->fMagnitude = 0.05;
	if (pDock->fMagnitude > 1.0)
		pDock->fMagnitude = 1.0;
	
	pDock->fLateralFactor *= pDock->fLateralFactor;
	///pDock->fLateralFactor = (pDock->fLateralFactor != 0 ? pow (1.5, - 1. / pDock->fLateralFactor) : 0);  // f(x)-x < 0 pour a > exp(exp(-1)) ~ 1.445.
	if (pDock->fLateralFactor < 0.03)
		pDock->fLateralFactor = 0;
	
	gint iMouseX, iMouseY;
	if (pDock->bHorizontalDock)
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
	else
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseY, &iMouseX, NULL);
	
	cairo_dock_calculate_icons (pDock, iMouseX, iMouseY);
	gtk_widget_queue_draw (pDock->pWidget);
	
	if (pDock->fMagnitude == 1 && pDock->fLateralFactor == 0)
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
	if (pDock->bHorizontalDock)
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
	else
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseY, &iMouseX, NULL);
	
	if (pDock->iScrollOffset != 0 && g_bResetScrollOnLeave)
	{
		if (pDock->iScrollOffset < pDock->iMinDockWidth / 2)
			pDock->iScrollOffset = pDock->iScrollOffset * g_fScrollAcceleration;
		else
			pDock->iScrollOffset += ceil ((pDock->iMinDockWidth - pDock->iScrollOffset) * (1 - g_fScrollAcceleration));
		if (pDock->iScrollOffset < 5 || pDock->iMinDockWidth - pDock->iScrollOffset < 5)
			pDock->iScrollOffset = 0;
		pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, pDock->iMinDockWidth, pDock->iScrollOffset);
		pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->iMinDockWidth)) + 1;
	}
	
	cairo_dock_calculate_icons (pDock, iMouseX, iMouseY);
	gtk_widget_queue_draw (pDock->pWidget);
	
	if (pDock->fMagnitude < 0.05)
	{
		Icon *pBouncingIcon = cairo_dock_get_bouncing_icon (pDock->icons);
		Icon *pRemovingIcon = cairo_dock_get_removing_or_inserting_icon (pDock->icons);
		
		if (pBouncingIcon == NULL && pRemovingIcon == NULL && (! g_bResetScrollOnLeave || pDock->iScrollOffset == 0))
		{
			pDock->fMagnitude = 0;
			int iNewWidth, iNewHeight;
			
			if (! (g_bAutoHide && pDock->iRefCount == 0) && ! pDock->bInside)
			{
				cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);
				if (pDock->bHorizontalDock)
					gdk_window_move_resize (pDock->pWidget->window,
						pDock->iWindowPositionX,
						pDock->iWindowPositionY,
						iNewWidth,
						iNewHeight);
				else
					gdk_window_move_resize (pDock->pWidget->window,
						pDock->iWindowPositionY,
						pDock->iWindowPositionX,
						iNewHeight,
						iNewWidth);
			}
			
			gint iMouseX, iMouseY;
			if (pDock->bHorizontalDock)
				gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
			else
				gdk_window_get_pointer (pDock->pWidget->window, &iMouseY, &iMouseX, NULL);
			
			cairo_dock_calculate_icons (pDock, iMouseX, iMouseY);  // relance le grossissement si on est dedans.
			if (! pDock->bInside && pDock->iRefCount > 0)
			{
				g_print ("on cache ce sous-dock en sortant par lui\n");
				//gdk_window_hide (pDock->pWidget->window);
				gtk_widget_hide (pDock->pWidget);
				cairo_dock_hide_parent_docks (pDock);
			}
			//g_print ("fin du shrink down\n");
			pDock->iSidShrinkDown = 0;
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
