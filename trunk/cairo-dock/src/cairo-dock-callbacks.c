/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <cairo.h>
#include <pango/pango.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include </usr/include/X11/Xlib.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-menu.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-applications.h"
#include "cairo-dock-desktop-file-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-config.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-callbacks.h"

static Icon *s_pIconClicked = NULL;  // pour savoir quand on deplace une icone a la souris. Dangereux si l'icone se fait effacer en cours ...
static CairoDock *s_pLastPointedDock = NULL;  // pour savoir quand on passe d'un dock a un autre.
static int s_iSidNonStopScrolling = 0;
///static int s_iInternMovingIconType = -1;
///static double s_fInternAvoidingMouseMargin = 0.;
static int s_iSidShowSubDockDemand = 0;

extern CairoDock *g_pMainDock;
extern double g_fSubDockSizeRatio;
extern gboolean g_bAnimateSubDock;
extern double g_fUnfoldAcceleration;
extern int g_iLeaveSubDockDelay;
extern int g_iShowSubDockDelay;

extern gint g_iScreenWidth[2];
extern gint g_iScreenHeight[2];
extern int g_iScrollAmount;
extern gboolean g_bResetScrollOnLeave;
extern gboolean g_bDecorationsFollowMouse;
extern cairo_surface_t *g_pBackgroundSurfaceFull[2];

extern gboolean g_bSameHorizontality;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bAutoHide;

extern gchar *g_cConfFile;

extern int g_iVisibleZoneHeight, g_iVisibleZoneWidth;
extern gboolean g_bDirectionUp;

extern double g_fRefreshInterval;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS];

extern gboolean g_bUseGlitz;


gboolean on_expose (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoDock *pDock)
{
	//g_print ("%s ((%d;%d) %dx%d) (%d)\n", __func__, pExpose->area.x, pExpose->area.y, pExpose->area.width, pExpose->area.height, pDock->bAtBottom);
	
	if (pExpose->area.x + pExpose->area.y != 0)  // x et y sont >= 0.
	{
		if (! (g_bAutoHide && pDock->iRefCount == 0) || ! pDock->bAtBottom)
			cairo_dock_render_optimized (pDock, &pExpose->area);
		
		return FALSE;
	}
	
	if (!pDock->bAtBottom)
	{
		pDock->render (pDock);
		//cairo_dock_render (pDock);
	}
	else
	{
		if (g_bAutoHide && pDock->iRefCount == 0)
		{
			if (pDock->bInside)
			{
				cairo_dock_render_blank (pDock);
			}
			else
				cairo_dock_render_background (pDock);
		}
		else
			pDock->render (pDock);
			//cairo_dock_render (pDock);
	}
	
	return FALSE;
}



void cairo_dock_set_subdock_position_linear (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	///int iX = iMouseX + (-iMouseX + (pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2)) / 2;
	int iX = pPointedIcon->fXAtRest - (pDock->iFlatDockWidth - pDock->iMaxDockWidth) / 2 + pPointedIcon->fWidth / 2;
	//int iX = iMouseX + (iMouseX < pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2 ? (g_bDirectionUp ? 1 : 0) : (g_bDirectionUp ? 0 : -1)) * pPointedIcon->fWidth * pPointedIcon->fScale / 2;
	if (pSubDock->bHorizontalDock == pDock->bHorizontalDock)
	{
		pSubDock->fAlign = 0.5;
		pSubDock->iGapX = iX + pDock->iWindowPositionX - g_iScreenWidth[pDock->bHorizontalDock] / 2;  // les sous-dock ont un alignement egal a 0.5.  // pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2
		pSubDock->iGapY = pDock->iGapY + pDock->iMaxDockHeight;
	}
	else
	{
		pSubDock->fAlign = (g_bDirectionUp ? 1 : 0);
		pSubDock->iGapX = (pDock->iGapY + pDock->iMaxDockHeight) * (g_bDirectionUp ? -1 : 1);
		if (g_bDirectionUp)
			pSubDock->iGapY = g_iScreenWidth[pDock->bHorizontalDock] - (iX + pDock->iWindowPositionX) - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 1.
		else
			pSubDock->iGapY = iX + pDock->iWindowPositionX - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 0.
	}
}
void cairo_dock_set_subdock_position_caroussel (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	int iMouseX = pDock->iMouseX;
	int iX = iMouseX + (-iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2) / 2;
	//int iX = iMouseX + (iMouseX < pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2 ? (g_bDirectionUp ? 1 : 0) : (g_bDirectionUp ? 0 : -1)) * pPointedIcon->fWidth * pPointedIcon->fScale / 2;
	if (pSubDock->bHorizontalDock == pDock->bHorizontalDock)
	{
		pSubDock->fAlign = 0.5;
		pSubDock->iGapX = iX + pDock->iWindowPositionX - g_iScreenWidth[pDock->bHorizontalDock] / 2;  // les sous-dock ont un alignement egal a 0.5.  // pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2
		pSubDock->iGapY = pDock->iGapY + pDock->iMaxDockHeight;
	}
	else
	{
		pSubDock->fAlign = (g_bDirectionUp ? 1 : 0);
		pSubDock->iGapX = (pDock->iGapY + pDock->iMaxDockHeight) * (g_bDirectionUp ? -1 : 1);
		if (g_bDirectionUp)
			pSubDock->iGapY = g_iScreenWidth[pDock->bHorizontalDock] - (iX + pDock->iWindowPositionX) - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 1.
		else
			pSubDock->iGapY = iX + pDock->iWindowPositionX - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 0.
	}
}
static void cairo_dock_show_subdock (Icon *pPointedIcon, gboolean bUpdate, CairoDock *pDock)
{
	//g_print ("on montre le dock fils\n");
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	g_return_if_fail (pSubDock != NULL);
	
	if (pSubDock->iSidShrinkDown != 0)
	{
		g_source_remove (pSubDock->iSidShrinkDown);
		pSubDock->iSidShrinkDown = 0;
		pSubDock->iMagnitudeIndex = 0;
		cairo_dock_shrink_down (pSubDock);
	}
	
	if (bUpdate)
	{
		gboolean bIsLoop = (pDock->iRefCount == 0 && 1. * pDock->iCurrentWidth / pDock->iMaxDockWidth < .6 && pDock->bInside);
		cairo_dock_calculate_construction_parameters (pPointedIcon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, bIsLoop, pDock->bInside);  // c'est un peu un hack pourri, l'idee c'est de recalculer la position exacte de l'icone pointee pour pouvoir placer le sous-dock precisement, car sa derniere position connue est decalee d'un coup de molette par rapport a la nouvelle, ce qui fait beaucoup.
	}
	
	///cairo_dock_set_subdock_position_generic (pPointedIcon, pDock);
	pSubDock->set_subdock_position (pPointedIcon, pDock);
	
	pSubDock->fFoldingFactor = g_fUnfoldAcceleration;
	pSubDock->bAtBottom = FALSE;
	int iNewWidth, iNewHeight;
	if (! g_bAnimateSubDock || g_fUnfoldAcceleration == 0)
	{
		cairo_dock_get_window_position_and_geometry_at_balance (pSubDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);
		
		if (pSubDock->bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pSubDock->pWidget), pSubDock->iWindowPositionX, pSubDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pSubDock->pWidget), pSubDock->iWindowPositionY, pSubDock->iWindowPositionX);
		
		gtk_window_present (GTK_WINDOW (pSubDock->pWidget));
	}
	else
	{
		cairo_dock_get_window_position_and_geometry_at_balance (pSubDock, CAIRO_DOCK_MAX_SIZE, &iNewWidth, &iNewHeight);
		
		gtk_window_present (GTK_WINDOW (pSubDock->pWidget));
		if (pSubDock->bHorizontalDock)
			gdk_window_move_resize (pSubDock->pWidget->window,
				pSubDock->iWindowPositionX,
				pSubDock->iWindowPositionY,
				iNewWidth,
				iNewHeight);
		else
			gdk_window_move_resize (pSubDock->pWidget->window,
				pSubDock->iWindowPositionY,
				pSubDock->iWindowPositionX,
				iNewHeight,
				iNewWidth);
		
		if (pSubDock->iSidGrowUp == 0)  // on commence a faire grossir les icones.  //  && pDock->iSidShrinkDown == 0
			pSubDock->iSidGrowUp = g_timeout_add (40, (GSourceFunc) cairo_dock_grow_up, (gpointer) pSubDock);
	}
	//g_print ("  -> Gap %d;%d -> W(%d;%d) (%d)\n", pSubDock->iGapX, pSubDock->iGapY, pSubDock->iWindowPositionX, pSubDock->iWindowPositionY, pSubDock->bHorizontalDock);
}
gboolean _cairo_dock_show_sub_dock_delayed (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	Icon *icon = cairo_dock_get_pointed_icon (pDock->icons);
	if (icon != NULL && icon->pSubDock != NULL)
		cairo_dock_show_subdock (icon, FALSE, pDock);
	return FALSE;
}
gboolean on_motion_notify2 (GtkWidget* pWidget,
	GdkEventMotion* pMotion,
	CairoDock *pDock)
{
	static double fLastTime = 0;
	Icon *pLastPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	int iLastMouseX = pDock->iMouseX;
	//g_print ("%s (%d,%d) (%d, %.2fms, bAtBottom:%d; iSidShrinkDown:%d)\n", __func__, (int) pMotion->x, (int) pMotion->y, pMotion->is_hint, pMotion->time - fLastTime, pDock->bAtBottom, pDock->iSidShrinkDown);
	
	//\_______________ On elague le flux des MotionNotify, sinon X en envoie autant que le permet le CPU !
	Icon *pPointedIcon;
	if (pMotion != NULL)
	{
		//if (g_iShowSubDockDelay > 0)
		//	pDock->iMouseX = (pDock->bHorizontalDock ? pMotion->x : pMotion->y);
		if (pDock->bHorizontalDock)
		{
			pDock->iMouseX = (int) pMotion->x;
			pDock->iMouseY = (int) pMotion->y;
		}
		else
		{
			pDock->iMouseX = (int) pMotion->y;
			pDock->iMouseY = (int) pMotion->x;
		}
		
		if (pDock->iSidShrinkDown > 0 || pMotion->time - fLastTime < g_fRefreshInterval)  // si les icones sont en train de diminuer de taille (suite a un clic) on on laisse l'animation se finir, sinon elle va trop vite.  // || ! pDock->bInside || pDock->bAtBottom
		{
			gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);
			return FALSE;
		}
		
		//\_______________ On recalcule toutes les icones.
		//pPointedIcon = cairo_dock_apply_wave_effect (pDock);
		pPointedIcon = pDock->calculate_icons (pDock);
		///if (s_iInternMovingIconType != -1)
		///	cairo_dock_mark_icons_as_avoiding_mouse (pDock, s_iInternMovingIconType, .5);
		
		gtk_widget_queue_draw (pWidget);
		fLastTime = pMotion->time;
		
		//gdk_event_request_motions (pMotion);  // ce sera pour GDK 2.12.
		gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);  // pour recevoir d'autres MotionNotify.
	}
	else  // cas d'un drag and drop.
	{
		if (pDock->bHorizontalDock)
			gdk_window_get_pointer (pWidget->window, &pDock->iMouseX, &pDock->iMouseY, NULL);
		else
			gdk_window_get_pointer (pWidget->window, &pDock->iMouseY, &pDock->iMouseX, NULL);
		//pPointedIcon = cairo_dock_apply_wave_effect (pDock);
		pPointedIcon = pDock->calculate_icons (pDock);
		pDock->iAvoidingMouseIconType = CAIRO_DOCK_LAUNCHER;
		pDock->fAvoidingMouseMargin = .25;
		///s_iInternMovingIconType = CAIRO_DOCK_LAUNCHER;
		///s_fInternAvoidingMouseMargin = .25;
		///cairo_dock_mark_icons_as_avoiding_mouse (pDock, s_iInternMovingIconType, .25);
		
		gtk_widget_queue_draw (pWidget);
	}
	
	if (g_bDecorationsFollowMouse)
	{
		pDock->fDecorationsOffsetX = pDock->iMouseX - pDock->iCurrentWidth / 2;
		//g_print ("fDecorationsOffsetX <- %.2f\n", pDock->fDecorationsOffsetX);
	}
	else
	{
		if (pDock->iMouseX > iLastMouseX)
		{
			pDock->fDecorationsOffsetX += 10;
			if (pDock->fDecorationsOffsetX > pDock->iCurrentWidth / 2)
			{
				if (g_pBackgroundSurfaceFull[0] != NULL)
					pDock->fDecorationsOffsetX -= pDock->iCurrentWidth;
				else
					pDock->fDecorationsOffsetX = pDock->iCurrentWidth / 2;
			}
		}
		else
		{
			pDock->fDecorationsOffsetX -= 10;
			if (pDock->fDecorationsOffsetX < - pDock->iCurrentWidth / 2)
			{
				if (g_pBackgroundSurfaceFull[0] != NULL)
					pDock->fDecorationsOffsetX += pDock->iCurrentWidth;
				else
					pDock->fDecorationsOffsetX = - pDock->iCurrentWidth / 2;
			}
		}
		//pDock->iMouseX = iX;
	}
	
	if (pPointedIcon != pLastPointedIcon || s_pLastPointedDock == NULL)
	{
		//g_print ("on change d'icone (-> %s)\n", (pPointedIcon != NULL ? pPointedIcon->acName : "rien"));
		if (s_iSidShowSubDockDemand != 0)
		{
			g_source_remove (s_iSidShowSubDockDemand);
			s_iSidShowSubDockDemand = 0;
		}
		cairo_dock_replace_all_dialogs ();
		if ((pDock == s_pLastPointedDock || s_pLastPointedDock == NULL) && pLastPointedIcon != NULL && pLastPointedIcon->pSubDock != NULL)
		{
			CairoDock *pSubDock = pLastPointedIcon->pSubDock;
			if (GTK_WIDGET_VISIBLE (pSubDock->pWidget))
			{
				//g_print ("on cache %s en changeant d'icône\n", pLastPointedIcon->acName);
				if (pSubDock->iSidGrowUp != 0)
				{
					g_source_remove (pSubDock->iSidGrowUp);
					pSubDock->iSidGrowUp = 0;
				}
				if (pSubDock->iSidShrinkDown != 0)
				{
					g_source_remove (pSubDock->iSidShrinkDown);
					pSubDock->iSidShrinkDown = 0;
				}
				pSubDock->iScrollOffset = 0;
				pSubDock->fFoldingFactor = 0;
				gtk_widget_hide (pSubDock->pWidget);
			}
			//else
			//	g_print ("pas encore visible !\n");
		}
		if (pPointedIcon != NULL && pPointedIcon->pSubDock != NULL && pPointedIcon->pSubDock != s_pLastPointedDock)
		{
			if (g_iShowSubDockDelay > 0)
			{
				//pDock->iMouseX = iX;
				s_iSidShowSubDockDemand = g_timeout_add (g_iShowSubDockDelay, (GSourceFunc) _cairo_dock_show_sub_dock_delayed, pDock);
			}
			else
				cairo_dock_show_subdock (pPointedIcon, FALSE, pDock);
			s_pLastPointedDock = pDock;
		}
		pLastPointedIcon = pPointedIcon;
		if (s_pLastPointedDock == NULL)
		{
			//g_print ("pLastPointedDock n'est plus null\n");
			s_pLastPointedDock = pDock;
		}
	}
	
	return FALSE;
}

gboolean _cairo_dock_emit_leave_signal (CairoDock *pDock)
{
	static gboolean bReturn;
	//g_print ("demande de quitter\n");
	g_signal_emit_by_name (pDock->pWidget, "leave-notify-event", NULL, &bReturn);
	return FALSE;
}

void cairo_dock_leave_from_main_dock (CairoDock *pDock)
{
	//g_print ("%s (iSidShrinkDown : %d)\n", __func__, pDock->iSidShrinkDown);
	///s_iInternMovingIconType = -1;
	pDock->iAvoidingMouseIconType = -1;
	pDock->fAvoidingMouseMargin = 0;
	pDock->bInside = FALSE;
	pDock->bAtTop = FALSE;
	if (pDock->bMenuVisible)
	{
		return ;
	}
	if (pDock->iSidMoveUp > 0)  // si on est en train de monter, on arrete.
	{
		g_source_remove (pDock->iSidMoveUp);
		pDock->iSidMoveUp = 0;
	}
	if (pDock->iSidGrowUp > 0)  // si on est en train de faire grossir les icones, on arrete.
	{
		pDock->fFoldingFactor = 0;
		g_source_remove (pDock->iSidGrowUp);
		pDock->iSidGrowUp = 0;
	}
	
	if (g_bAutoHide && pDock->iRefCount == 0)
	{
		pDock->fFoldingFactor = (g_fUnfoldAcceleration != 0. ? 0.03 : 0.);
		if (pDock->iSidMoveDown == 0)  // on commence a descendre.
			pDock->iSidMoveDown = g_timeout_add (40, (GSourceFunc) cairo_dock_move_down, (gpointer) pDock);
	}
	else
	{
		pDock->fFoldingFactor = 0;
		pDock->bAtBottom = TRUE;
	}
	
	pDock->fDecorationsOffsetX = 0;
	if (pDock->iSidShrinkDown == 0)  // on commence a faire diminuer la taille des icones.
		pDock->iSidShrinkDown = g_timeout_add (40, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
	
	//s_pLastPointedDock = NULL;
	//g_print ("s_pLastPointedDock <- NULL\n");
}
gboolean on_leave_notify2 (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDock *pDock)
{
	//g_print ("%s (bInside:%d)\n", __func__, pDock->bInside);
	if (pDock->bAtBottom )  // || ! pDock->bInside
		return FALSE;
	//g_print ("%s (main dock : %d)\n", __func__, pDock->bIsMainDock);
	
	if (pEvent != NULL && pDock->iRefCount > 0)  // on ne le fait que pour les containers.
	{
		//g_print ("  on retarde la sortie\n");
		if (pDock->iSidLeaveDemand == 0)
			pDock->iSidLeaveDemand = g_timeout_add (g_iLeaveSubDockDelay, (GSourceFunc) _cairo_dock_emit_leave_signal, (gpointer) pDock);
		return FALSE;
	}
	pDock->iSidLeaveDemand = 0;
	
	if (s_iSidNonStopScrolling > 0)
	{
		g_source_remove (s_iSidNonStopScrolling);
		s_iSidNonStopScrolling = 0;
	}
	
	while (gtk_events_pending ())  // on laisse le temps au signal d'entree dans le sous-dock d'etre traite.
		gtk_main_iteration ();
	
	if (s_iSidShowSubDockDemand != 0)
	{
		g_source_remove (s_iSidShowSubDockDemand);
		s_iSidShowSubDockDemand = 0;
	}
	if (! cairo_dock_hide_child_docks (pDock))  // on quitte si on entre dans un sous-dock, pour rester en position "haute".
		return FALSE;
	
	cairo_dock_leave_from_main_dock (pDock);
	
	return FALSE;
}


gboolean on_enter_notify2 (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDock *pDock)
{
	//g_print ("%s (bAtTop:%d; bInside:%d; iSidMoveDown:%d; iMagnitudeIndex:%d)\n", __func__, pDock->bAtTop, pDock->bInside, pDock->iSidMoveDown, pDock->iMagnitudeIndex);
	s_pLastPointedDock = NULL;  // ajoute le 04/10/07 pour permettre aux sous-docks d'apparaitre si on entre en pointant tout de suite sur l'icone.
	if (pDock->iSidLeaveDemand != 0)
	{
		g_source_remove (pDock->iSidLeaveDemand);
		pDock->iSidLeaveDemand = 0;
	}
	if (pDock->bAtTop || pDock->bInside || (pDock->iSidMoveDown != 0))  // le 'iSidMoveDown != 0' est la pour empecher le dock de "vibrer" si l'utilisateur sort par en bas avec l'auto-hide active.
	{
		return FALSE;
	}
	//g_print ("%s (main dock : %d)\n", __func__, pDock->bIsMainDock);
	
	pDock->fDecorationsOffsetX = 0;
	if (! pDock->bIsMainDock)
		gtk_window_present (GTK_WINDOW (pWidget));
	pDock->bInside = TRUE;
	
	if (s_pIconClicked != NULL)  // on pourrait le faire a chaque motion aussi.
	{
		pDock->iAvoidingMouseIconType = s_pIconClicked->iType;
		pDock->fAvoidingMouseMargin = .5;
	}
	
	
	int iNewWidth, iNewHeight;
	int iActualPositionY = pDock->iWindowPositionY;
	cairo_dock_get_window_position_and_geometry_at_balance (pDock, CAIRO_DOCK_MAX_SIZE, &iNewWidth, &iNewHeight);
	if ((g_bAutoHide && pDock->iRefCount == 0) && pDock->bAtBottom)
		pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - g_iVisibleZoneHeight - pDock->iGapY : g_iVisibleZoneHeight + pDock->iGapY - pDock->iMaxDockHeight);
	
	if (pDock->bHorizontalDock)
		gdk_window_move_resize (pWidget->window,
			pDock->iWindowPositionX,
			pDock->iWindowPositionY,
			iNewWidth,
			iNewHeight);
	else
		gdk_window_move_resize (pWidget->window,
			pDock->iWindowPositionY,
			pDock->iWindowPositionX,
			iNewHeight,
			iNewWidth);
	//gtk_widget_queue_draw (pWidget);
	
	if (pDock->iSidMoveDown > 0)  // si on est en train de descendre, on arrete.
	{
		//g_print ("  on est en train de descendre, on arrete\n");
		g_source_remove (pDock->iSidMoveDown);
		pDock->iSidMoveDown = 0;
	}
	/*if (g_iSidShrinkDown > 0)  // si on est en train de faire diminuer la tailler des icones, on arrete.
	{
		g_source_remove (g_iSidShrinkDown);
		g_iSidShrinkDown = 0;
	}*/
	
	cairo_dock_replace_all_dialogs ();
	
	if (g_bAutoHide && pDock->iRefCount == 0)
	{
		//g_print ("  on commence a monter\n");
		if (pDock->iSidMoveUp == 0)  // on commence a monter.
			pDock->iSidMoveUp = g_timeout_add (40, (GSourceFunc) cairo_dock_move_up, (gpointer) pDock);
	}
	else
	{
		pDock->bAtTop = TRUE;
		pDock->bAtBottom = FALSE;
	}
	if (pDock->iSidGrowUp == 0 && pDock->iSidShrinkDown == 0)  // on commence a faire grossir les icones.
	{
		pDock->iSidGrowUp = g_timeout_add (40, (GSourceFunc) cairo_dock_grow_up, (gpointer) pDock);
	}
	
	return FALSE;
}


void cairo_dock_update_gaps_with_window_position (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	int iWidth, iHeight;  // mieux que iCurrentWidth.
	if (pDock->bHorizontalDock)
	{
		gtk_window_get_size (GTK_WINDOW (pDock->pWidget), &iWidth, &iHeight);
		gtk_window_get_position (GTK_WINDOW (pDock->pWidget), &pDock->iWindowPositionX, &pDock->iWindowPositionY);
	}
	else
	{
		gtk_window_get_size (GTK_WINDOW (pDock->pWidget),  &iHeight, &iWidth);
		gtk_window_get_position (GTK_WINDOW (pDock->pWidget), &pDock->iWindowPositionY, &pDock->iWindowPositionX);
	}
	
	int x, y;  // position du point invariant du dock.
	x = pDock->iWindowPositionX + iWidth * pDock->fAlign;
	y = (g_bDirectionUp ? pDock->iWindowPositionY + iHeight : pDock->iWindowPositionY);
	
	pDock->iGapX = x - g_iScreenWidth[pDock->bHorizontalDock] * pDock->fAlign;
	pDock->iGapY = (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - y : y);
	
	if (pDock->bIsMainDock)
		cairo_dock_update_conf_file_with_position (g_cConfFile, pDock->iGapX, pDock->iGapY);
}

static int iMoveByArrow = 0;
gboolean on_key_release (GtkWidget *pWidget,
			GdkEventKey *pKey,
			CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	iMoveByArrow = 0;
	if (pKey->state & GDK_MOD1_MASK)  // On relache la touche ALT, typiquement apres avoir fait un ALT + clique gauche + deplacement.
	{
		cairo_dock_update_gaps_with_window_position (pDock);
	}
	return FALSE;
}


static int _move_up_by_arrow (int iMoveByArrow, CairoDock *pDock)
{
	int iPossibleMove = MAX (0, pDock->iWindowPositionY);
	int iEffectiveMove = MIN (iMoveByArrow, iPossibleMove);
	//g_print ("%s () : iPossibleMove=%d->%d\n", __func__, iPossibleMove, iEffectiveMove);
	if (iEffectiveMove > 0)
	{
		pDock->iWindowPositionY -= iEffectiveMove;
		pDock->iGapY += (g_bDirectionUp ? iEffectiveMove : -iEffectiveMove);
	}
	return iEffectiveMove;
}
static int _move_down_by_arrow (int iMoveByArrow, CairoDock *pDock)
{
	int iPossibleMove = MAX (0, g_iScreenHeight[pDock->bHorizontalDock] - (pDock->iWindowPositionY + pDock->iCurrentHeight));
	int iEffectiveMove = MIN (iMoveByArrow, iPossibleMove);
	//g_print ("%s () : iPossibleMove=%d->%d\n", __func__, iPossibleMove, iEffectiveMove);
	if (iEffectiveMove > 0)
	{
		pDock->iWindowPositionY += iEffectiveMove;
		pDock->iGapY += (g_bDirectionUp ? -iEffectiveMove : iEffectiveMove);
	}
	return iEffectiveMove;
}
static int _move_left_by_arrow (int iMoveByArrow, CairoDock *pDock)
{
	int iPossibleMove = MAX (0, pDock->iWindowPositionX);
	int iEffectiveMove = MIN (iMoveByArrow, iPossibleMove);
	if (iEffectiveMove > 0)
	{
		pDock->iWindowPositionX -= iEffectiveMove;
		pDock->iGapX -= iEffectiveMove;
	}
	return iEffectiveMove;
}
static int _move_right_by_arrow (int iMoveByArrow, CairoDock *pDock)
{
	int iPossibleMove = MAX (0, g_iScreenWidth[pDock->bHorizontalDock] - (pDock->iWindowPositionX + pDock->iCurrentWidth));
	int iEffectiveMove = MIN (iMoveByArrow, iPossibleMove);
	if (iEffectiveMove > 0)
	{
		pDock->iWindowPositionX += iEffectiveMove;
		pDock->iGapX += iEffectiveMove;
	}
	return iEffectiveMove;
}
gboolean on_key_press (GtkWidget *pWidget,
			GdkEventKey *pKey,
			CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	if (pKey->type == GDK_KEY_PRESS)
	{
		GdkEventScroll dummyScroll;
		int iX, iY;
		switch (pKey->keyval)
		{
			case GDK_Down :
				if (pKey->state & GDK_CONTROL_MASK)
					iMoveByArrow = (pDock->bHorizontalDock ? _move_down_by_arrow (++iMoveByArrow, pDock) : _move_right_by_arrow (++iMoveByArrow, pDock));
			break;

			case GDK_Up :
				if (pKey->state & GDK_CONTROL_MASK)
					iMoveByArrow = (pDock->bHorizontalDock ? _move_up_by_arrow (++iMoveByArrow, pDock) : _move_left_by_arrow (++iMoveByArrow, pDock));
			break;
			
			case GDK_Left :
				if (pKey->state & GDK_CONTROL_MASK)
					iMoveByArrow = (pDock->bHorizontalDock ? _move_left_by_arrow (++iMoveByArrow, pDock) : _move_up_by_arrow (++iMoveByArrow, pDock));
			break;
			
			case GDK_Right :
				if (pKey->state & GDK_CONTROL_MASK)
					iMoveByArrow = (pDock->bHorizontalDock ? _move_right_by_arrow (++iMoveByArrow, pDock) : _move_down_by_arrow (++iMoveByArrow, pDock));
			break;
			
			case GDK_Page_Up :
				dummyScroll.direction = GDK_SCROLL_UP;
				gdk_window_get_pointer (pWidget->window, &iX, &iY, NULL);
				dummyScroll.x = iX;
				dummyScroll.y = iY;
				dummyScroll.time = pKey->time;
				dummyScroll.state = pKey->state;
				on_scroll (pWidget,
					&dummyScroll,
					pDock);
			break;
			
			case GDK_Page_Down:
				dummyScroll.direction = GDK_SCROLL_DOWN;
				gdk_window_get_pointer (pWidget->window, &iX, &iY, NULL);
				dummyScroll.x = iX;
				dummyScroll.y = iY;
				dummyScroll.time = pKey->time;
				dummyScroll.state = pKey->state;
				on_scroll (pWidget,
					&dummyScroll,
					pDock);
			break;
		}
	}
	
	if (iMoveByArrow > 0)
	{
		if (pDock->bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionY, pDock->iWindowPositionX);
		if (pDock->bIsMainDock)
			cairo_dock_update_conf_file_with_position (g_cConfFile, pDock->iGapX, pDock->iGapY);
	}
	
	return FALSE;
}


gboolean cairo_dock_notification_click_icon (gpointer *data)
{
	Icon *icon = data[0];
	
	if (CAIRO_DOCK_IS_LAUNCHER (icon))
	{
		if (icon->acCommand != NULL)
		{
			g_spawn_command_line_async (icon->acCommand, NULL);
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		}
		else
		{
			icon->iCount = 0;
		}
	}
	else if (CAIRO_DOCK_IS_VALID_APPLI (icon))
	{
		cairo_dock_show_appli (icon->Xid);
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	else
	{
		g_print ("No known action\n");
		icon->iCount = 0;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
gboolean cairo_dock_notification_double_click_icon (gpointer *data)
{
	Icon *icon = data[0];
	if (icon->pSubDock != NULL)
		return cairo_dock_notification_click_icon (data);
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
gboolean on_button_press2 (GtkWidget* pWidget,
				GdkEventButton* pButton,
				CairoDock *pDock)
{
	//g_print ("%s (%d/%d)\n", __func__, pButton->type, pButton->button);
	if (pButton->button == 1)  // clique gauche.
	{
		Icon *icon = cairo_dock_get_pointed_icon (pDock->icons);
		if (pButton->type == GDK_BUTTON_RELEASE)
		{
			if (s_pIconClicked != NULL)
			{
				g_print ("release sur %s\n", s_pIconClicked->acName);
				s_pIconClicked->iAnimationType = 0;  // stoppe les animations de suivi du curseur.
				//cairo_dock_mark_icons_as_avoiding_mouse (pDock, -1e4, -1);
				cairo_dock_stop_marking_icons (pDock, pDock->iAvoidingMouseIconType);
				pDock->iAvoidingMouseIconType = -1;
			}
			if (icon != NULL && ! CAIRO_DOCK_IS_SEPARATOR (icon) && icon == s_pIconClicked)
			{
				cairo_dock_arm_animation (icon, -1, -1);
				
				gpointer data[2] = {icon, pDock};
				cairo_dock_notify (CAIRO_DOCK_CLICK_ICON, data);
				
				cairo_dock_start_animation (icon, pDock);
			}
			else if (s_pIconClicked != NULL && icon != NULL && icon != s_pIconClicked)  //  && icon->iType == s_pIconClicked->iType
			{
				g_print ("deplacement de %s\n", s_pIconClicked->acName);
				CairoDock *pOriginDock = cairo_dock_search_container_from_icon (s_pIconClicked);
				if (pDock != pOriginDock)
				{
					cairo_dock_detach_icon_from_dock (s_pIconClicked, pOriginDock, TRUE);  // plutot que 'cairo_dock_remove_icon_from_dock', afin de ne pas la fermer.
					///cairo_dock_remove_icon_from_dock (pOriginDock, s_pIconClicked);
					cairo_dock_update_dock_size (pOriginDock);
					
					///s_pIconClicked->fWidth /= (pOriginDock->iRefCount == 0 ? 1. : g_fSubDockSizeRatio);
					///s_pIconClicked->fHeight /= (pOriginDock->iRefCount == 0 ? 1. : g_fSubDockSizeRatio);
					cairo_dock_update_icon_s_container_name (s_pIconClicked, icon->cParentDockName);
					if (pOriginDock->iRefCount > 0 && ! g_bSameHorizontality)
					{
						cairo_t* pSourceContext = cairo_dock_create_context_from_window (pDock);
						cairo_dock_fill_one_text_buffer (s_pIconClicked, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : g_pMainDock->bHorizontalDock));
						cairo_destroy (pSourceContext);
					}
					
					cairo_dock_insert_icon_in_dock (s_pIconClicked, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
				}
				
				int iX, iY;
				if (pDock->bHorizontalDock)
				{
					iX = pButton->x;
					iY = pButton->y;
				}
				else
				{
					iX = pButton->y;
					iY = pButton->x;
				}
				
				Icon *prev_icon, *next_icon;
				if (iX > icon->fX + icon->fWidth * icon->fScale / 2)
				{
					prev_icon = icon;
					next_icon = cairo_dock_get_next_icon (pDock->icons, icon);
				}
				else
				{
					prev_icon = cairo_dock_get_previous_icon (pDock->icons, icon);
					next_icon = icon;
				}
				if ((prev_icon == NULL || prev_icon->iType != s_pIconClicked->iType) && (next_icon == NULL || next_icon->iType != s_pIconClicked->iType))
				{
					s_pIconClicked = NULL;
					return FALSE;
				}
				//g_print ("deplacement de %s\n", s_pIconClicked->acName);
				if (prev_icon != NULL && prev_icon->iType != s_pIconClicked->iType)
					prev_icon = NULL;
				s_pIconClicked->iAnimationType = CAIRO_DOCK_BOUNCE;
				s_pIconClicked->iCount = 2 * g_tNbIterInOneRound[icon->iAnimationType] - 1;  // 2 rebonds.
				cairo_dock_move_icon_after_icon (pDock, s_pIconClicked, prev_icon);
				
				cairo_dock_update_dock_size (pDock);
				pDock->iMouseX = iX;  // utile ?
				pDock->iMouseY = iY;
				//cairo_dock_apply_wave_effect (pDock);
				pDock->calculate_icons (pDock);
				gtk_widget_queue_draw (pWidget);
				
				if (pDock->iSidShrinkDown == 0)  // on lance l'animation.
					pDock->iSidShrinkDown = g_timeout_add (40, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
			}
			s_pIconClicked = NULL;
		}
		else if (pButton->type == GDK_BUTTON_PRESS)
		{
			s_pIconClicked = icon;
			if (s_pIconClicked != NULL)
			{
				g_print ("s_pIconClicked <- %s\n", s_pIconClicked->acName);
				s_pIconClicked->iAnimationType = CAIRO_DOCK_FOLLOW_MOUSE;
				///s_iInternMovingIconType = s_pIconClicked->iType;
				pDock->iAvoidingMouseIconType = s_pIconClicked->iType;  // on pourrait le faire lors du 'motion' aussi.
				pDock->fAvoidingMouseMargin = .5;
				
			}
		}
		else if (pButton->type == GDK_2BUTTON_PRESS)
		{
			gpointer data[2] = {icon, pDock};
			cairo_dock_notify (CAIRO_DOCK_DOUBLE_CLICK_ICON, data);
		}
	}
	else if (pButton->button == 3 && pButton->type == GDK_BUTTON_PRESS)  // clique droit.
	{
		pDock->bMenuVisible = TRUE;
		GtkWidget *menu = cairo_dock_build_menu (pDock);
		
		gtk_widget_show_all (menu);
		
		gtk_menu_popup (GTK_MENU (menu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
	}
	else if (pButton->button == 2 && pButton->type == GDK_BUTTON_PRESS)  // clique milieu.
	{
		pDock->iScrollOffset = 0;
		cairo_dock_update_dock_size (pDock);
		if (pDock->bHorizontalDock)  // utile ?
		{
			pDock->iMouseX = (int) pButton->x;
			pDock->iMouseY = (int) pButton->y;
		}
		else
		{
			pDock->iMouseX = (int) pButton->y;
			pDock->iMouseY = (int) pButton->x;
		}
		//cairo_dock_apply_wave_effect (pDock);
		pDock->calculate_icons (pDock);
		gtk_widget_queue_draw (pWidget);
	}
	
	return FALSE;
}


static gboolean _cairo_dock_autoscroll (gpointer *data)
{
	GdkEventScroll* pScroll = data[0];
	CairoDock *pDock = data[1];
	gboolean bAutoScroll = GPOINTER_TO_INT (data[2]);
	
	//g_print ("%s (%d, %.2f)\n", __func__, pDock->iSidShrinkDown, pDock->fMagnitude);
	if (pDock->iSidShrinkDown != 0 || pDock->iMagnitudeIndex == 0)
		return FALSE;
	
	Icon *pLastPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	Icon *pNeighborIcon;
	if (pScroll->direction == GDK_SCROLL_UP)
	{
		pNeighborIcon = cairo_dock_get_previous_icon (pDock->icons, pLastPointedIcon);
		if (pNeighborIcon == NULL)
			pNeighborIcon = cairo_dock_get_last_icon (pDock->icons);
		pDock->iScrollOffset += (bAutoScroll ? 10 : ((pScroll->state & GDK_CONTROL_MASK) || g_iScrollAmount == 0 ? (pNeighborIcon->fWidth + (pLastPointedIcon != NULL ? pLastPointedIcon->fWidth : 0)) / 2 : g_iScrollAmount));
	}
	else if (pScroll->direction == GDK_SCROLL_DOWN)
	{
		pNeighborIcon = cairo_dock_get_next_icon (pDock->icons, pLastPointedIcon);
		if (pNeighborIcon == NULL)
			pNeighborIcon = cairo_dock_get_first_icon (pDock->icons);
		pDock->iScrollOffset -= (bAutoScroll ? 10 : ((pScroll->state & GDK_CONTROL_MASK) || g_iScrollAmount == 0 ? (pNeighborIcon->fWidth + (pLastPointedIcon != NULL ? pLastPointedIcon->fWidth : 0)) / 2 : g_iScrollAmount));
	}
	else
	{
		//g_print ("stop\n");
		return FALSE;
	}
	
	if (pDock->iScrollOffset >= pDock->iFlatDockWidth)
		pDock->iScrollOffset -= pDock->iFlatDockWidth;
	if (pDock->iScrollOffset < 0)
		pDock->iScrollOffset += pDock->iFlatDockWidth;
	//g_print ("iScrollOffset <- %d, (%d;%d) (%x)\n", pDock->iScrollOffset, (int) pScroll->x, (int) pScroll->y, pDock->icons);
	
	cairo_dock_update_dock_size (pDock);
	
	//\_______________ On recalcule toutes les icones.
	Icon *pPointedIcon;
	int iX, iY;
	if (bAutoScroll)
	{
		if (pDock->bHorizontalDock)
			gdk_window_get_pointer (pDock->pWidget->window, &iX, &iY, NULL);
		else
			gdk_window_get_pointer (pDock->pWidget->window, &iY, &iX, NULL);
	}
	else
	{
		if (pDock->bHorizontalDock)
		{
			iX = pScroll->x;
			iY = pScroll->y;
		}
		else
		{
			iX = pScroll->y;
			iY = pScroll->x;
		}
	}
	pDock->iMouseX = iX;
	pDock->iMouseY = iY;
	//pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	pPointedIcon = pDock->calculate_icons (pDock);
	gtk_widget_queue_draw (pDock->pWidget);
	
	//\_______________ On montre les sous-docks.
	if (pPointedIcon != pLastPointedIcon || s_pLastPointedDock == NULL)
	{
		//g_print ("on change d'icone\n");
		if (pDock == s_pLastPointedDock && pLastPointedIcon != NULL && pLastPointedIcon->pSubDock != NULL)
		{
			if (GTK_WIDGET_VISIBLE (pLastPointedIcon->pSubDock->pWidget))
				gdk_window_hide (pLastPointedIcon->pSubDock->pWidget->window);
		}
		if (pPointedIcon != NULL && pPointedIcon->pSubDock != NULL)
		{
			if (g_iShowSubDockDelay > 0)
			{
				//pDock->iMouseX = iX;
				s_iSidShowSubDockDemand = g_timeout_add (g_iShowSubDockDelay, (GSourceFunc) _cairo_dock_show_sub_dock_delayed, pDock);
			}
			else
				cairo_dock_show_subdock (pPointedIcon, TRUE, pDock);
			s_pLastPointedDock = pDock;
		}
		pLastPointedIcon = pPointedIcon;
	}
	
	return TRUE;
}
gboolean on_scroll (GtkWidget* pWidget,
			GdkEventScroll* pScroll,
			CairoDock *pDock)
{
	static double fLastTime = 0;
	static int iNbSimultaneousScroll = 0;
	static GdkEventScroll scrollBuffer;
	static gpointer data[3] = {&scrollBuffer, NULL, NULL};
	if (pDock->icons == NULL)
		return FALSE;
	
	//g_print ("%s (%d)\n", __func__, pScroll->direction);
	if (pScroll->time - fLastTime < g_fRefreshInterval && s_iSidNonStopScrolling == 0)
		iNbSimultaneousScroll ++;
	else
		iNbSimultaneousScroll = 0;
	if (iNbSimultaneousScroll == 2 && s_iSidNonStopScrolling == 0)
	{
		g_print ("on a scrolle comme un bourrinos\n");
		iNbSimultaneousScroll = -999;
		data[1] = pDock;
		data[2] = GINT_TO_POINTER (1);
		memcpy (&scrollBuffer, pScroll, sizeof (GdkEventScroll));
		s_iSidNonStopScrolling = g_timeout_add (g_fRefreshInterval, (GSourceFunc)_cairo_dock_autoscroll, data);
		return FALSE;
	}
	
	//g_print ("%d / %d\n", pScroll->direction, scrollBuffer.direction);
	if (s_iSidNonStopScrolling != 0 && pScroll->direction != scrollBuffer.direction)
	{
		//g_print ("on arrete\n");
		g_source_remove (s_iSidNonStopScrolling);
		s_iSidNonStopScrolling = 0;
		iNbSimultaneousScroll = 0;
		return FALSE;
	}
	
	if (pDock->bAtBottom || ! pDock->bInside || pDock->iSidShrinkDown > 0 || pScroll->time - fLastTime < g_fRefreshInterval)  // si les icones sont en train de diminuer de taille (suite a un clic) on ne redimensionne pas les icones, le temps que l'animation se finisse.
	{
		return FALSE;
	}
	
	fLastTime = pScroll->time;
	gpointer user_data[3] = {pScroll, pDock, GINT_TO_POINTER (0)};
	_cairo_dock_autoscroll (user_data);
	
	
	return FALSE;
}


gboolean on_configure (GtkWidget* pWidget,
					GdkEventConfigure* pEvent,
					CairoDock *pDock)
{
	//g_print ("%s (main dock : %d) : (%d;%d) (%dx%d)\n", __func__, pDock->bIsMainDock, pEvent->x, pEvent->y, pEvent->width, pEvent->height);
	gint iNewWidth, iNewHeight;
	if (pDock->bHorizontalDock)
	{
		iNewWidth = pEvent->width;
		iNewHeight = pEvent->height;
	}
	else
	{
		iNewWidth = pEvent->height;
		iNewHeight = pEvent->width;
	}
	
	if (iNewWidth != pDock->iCurrentWidth || iNewHeight != pDock->iCurrentHeight)
	{
		//g_print ("-> %dx%d\n", iNewWidth, iNewHeight);
		pDock->iCurrentWidth = iNewWidth;
		pDock->iCurrentHeight = iNewHeight;
		
		int iX, iY;
		if (pDock->bHorizontalDock)
			gdk_window_get_pointer (pWidget->window, &pDock->iMouseX, &pDock->iMouseY, NULL);
		else
			gdk_window_get_pointer (pWidget->window, &pDock->iMouseY, &pDock->iMouseX, NULL);
		if (pDock->iMouseX < 0 || pDock->iMouseX > pDock->iCurrentWidth)  // utile ?
			pDock->iMouseX = 0;
		
		//cairo_dock_apply_wave_effect (pDock);
		pDock->calculate_icons (pDock);
		///if (gdk_window_is_visible (pWidget->window))
		///	gtk_window_present (GTK_WINDOW (pWidget));
		gtk_widget_queue_draw (pWidget);
#ifdef HAVE_GLITZ
		if (pDock->pGlitzDrawable)
		{
			glitz_drawable_update_size (pDock->pGlitzDrawable,
				pEvent->width,
				pEvent->height);
		}
#endif
	}

	return FALSE;
}


void on_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, CairoDock *pDock)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	
	//\_________________ On arrete l'animation.
	//cairo_dock_mark_icons_as_avoiding_mouse (pDock, -1e4, -1);
	cairo_dock_stop_marking_icons (pDock, pDock->iAvoidingMouseIconType);
	pDock->iAvoidingMouseIconType = -1;
	
	//\_________________ On recupere l'URI.
	gchar *cReceivedData = (gchar *) selection_data->data;
	gchar *str = strrchr (cReceivedData, '\n');
	if (str != NULL)
		*(str-1) = '\0';  // on vire les retrours chariot.
	//g_print ("cReceivedData : %s\n", cReceivedData);
	
	//\_________________ On calcule la position a laquelle on l'a lache.
	double fOrder = 0;
	CairoDock *pReceivingDock = pDock;
	Icon *pPointedIcon = NULL, *pNeighboorIcon = NULL;
	GList *ic;
	Icon *icon, *next_icon, *prev_icon;
	int iDropX = (pDock->bHorizontalDock ? x : y);
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->bPointed)
		{
			//g_print ("On pointe sur %s\n", icon->acName);
			pPointedIcon = icon;
			double fMargin;
			if (g_str_has_suffix (cReceivedData, ".desktop"))  // si c'est un .desktop, on l'ajoute.
				fMargin = 0.5;  // on ne sera jamais dessus.
			else  // sinon on le lance si on est sur l'icone, et on l'ajoute autrement.
				fMargin = 0.25;
			
			if (iDropX > icon->fX + icon->fWidth * icon->fScale * (1 - fMargin))  // on est apres.
			{
				pNeighboorIcon = (ic->next != NULL ? ic->next->data : NULL);
				fOrder = (pNeighboorIcon != NULL ? (icon->fOrder + pNeighboorIcon->fOrder) / 2 : icon->fOrder + 1);
			}
			else if (iDropX < icon->fX + icon->fWidth * icon->fScale * fMargin)  // on est avant.
			{
				pNeighboorIcon = (ic->prev != NULL ? ic->prev->data : NULL);
				fOrder = (pNeighboorIcon != NULL ? (icon->fOrder + pNeighboorIcon->fOrder) / 2 : icon->fOrder - 1);
			}
			else  // on est dessus.
			{
				fOrder = CAIRO_DOCK_LAST_ORDER;
			}
		}
	}
	gpointer data[4] = {cReceivedData, pPointedIcon, &fOrder, pDock};
	cairo_dock_notify (CAIRO_DOCK_DROP_DATA, data);
}

gboolean cairo_dock_notification_drop_data (gpointer *data)
{
	gchar *cReceivedData = data[0];
	Icon *icon = data[1];
	double fOrder = *((double *) data[2]);
	CairoDock *pDock = data[3];
	
	CairoDock *pReceivingDock = pDock;
	if (fOrder == CAIRO_DOCK_LAST_ORDER)  // on a lache dessus.
	{
		if (icon->pSubDock != NULL)  // on essaiera de l'ajouter au sous-dock.
		{
			pReceivingDock = icon->pSubDock;
		}
		else if (icon->acDesktopFileName != NULL)  // on le lance avec les infos du lanceurs.
		{
			gchar *cCommand = g_strdup_printf ("%s '%s'", icon->acCommand, cReceivedData);
			g_spawn_command_line_async (cCommand, NULL);
			g_free (cCommand);
			icon->iAnimationType = CAIRO_DOCK_BLINK;
			icon->iCount = g_tNbIterInOneRound[icon->iAnimationType] * 2 - 1;  // 2 clignotements.
			if (pDock->iSidShrinkDown == 0)  // on lance l'animation.
				pDock->iSidShrinkDown = g_timeout_add (40, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		}
	}
	
	//\_________________ On l'ajoute dans le repertoire .cairo-dock.
	GError *erreur = NULL;
	const gchar *cDockName = cairo_dock_search_dock_name (pReceivingDock);
	gchar *cNewDesktopFileName = cairo_dock_add_desktop_file_from_uri (cReceivedData, cDockName, fOrder, pDock, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	//\_________________ On charge ce nouveau lanceur.
	if (cNewDesktopFileName != NULL)
	{
		cairo_dock_mark_theme_as_modified (TRUE);
		
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pDock);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cNewDesktopFileName, pCairoContext);
		g_free (cNewDesktopFileName);
		cairo_destroy (pCairoContext);
		
		if (pNewIcon != NULL)
			cairo_dock_insert_icon_in_dock (pNewIcon, pReceivingDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
		
		if (pDock->iSidShrinkDown == 0)  // on lance l'animation.
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


void on_drag_motion (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, guint t, CairoDock *pDock)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	//\_________________ On simule les evenements souris habituels.
	on_enter_notify2 (pWidget, NULL, pDock);  // ne sera effectif que la 1ere fois a chaque entree dans un dock.
	on_motion_notify2 (pWidget, NULL, pDock);
}


gboolean on_delete (GtkWidget *pWidget, GdkEvent *event, CairoDock *pDock)
{
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		"Quit Cairo-Dock ?");
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
		gtk_main_quit ();
	return FALSE;
}
