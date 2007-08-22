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
#include "cairo-dock-callbacks.h"

static Icon *s_pIconClicked = NULL;  // pour savoir quand on deplace une icone a la souris.
static CairoDock *g_pLastPointedDock = NULL;  // pour savoir quand on passe d'un dock a un autre.

extern CairoDock *g_pMainDock;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern int g_iScrollAmount;
extern gboolean g_bResetScrollOnLeave;

extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bAutoHide;

extern gchar *g_cConfFile;

extern int g_iVisibleZoneHeight, g_iVisibleZoneWidth;
extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern double g_fRefreshInterval;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS];

extern gboolean g_bUseGlitz;


gboolean on_expose (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoDock *pDock)
{
	//g_print ("%s ((%d;%d) %dx%d)\n", __func__, pExpose->area.x, pExpose->area.y, pExpose->area.width, pExpose->area.height);
	
	if (pExpose->area.x + pExpose->area.y != 0)  // x et y sont >= 0.
	{
		if (! (g_bAutoHide && pDock->iRefCount == 0) || ! pDock->bAtBottom)
			cairo_dock_render_optimized (pDock, &pExpose->area);
		
		return FALSE;
	}
	
	if (!pDock->bAtBottom)
	{
		render (pDock);
	}
	else
	{
		if ((g_bAutoHide && pDock->iRefCount == 0))
		{
			if (pDock->bInside)
				cairo_dock_render_blank (pDock);
			else
				cairo_dock_render_background (pDock);
		}
		else
			render (pDock);
	}
	
	return FALSE;
}


gboolean on_motion_notify2 (GtkWidget* pWidget,
	GdkEventMotion* pMotion,
	CairoDock *pDock)
{
	static double fLastTime = 0;
	Icon *pLastPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	//g_print ("%s (%d,%d) (%d, %.2fms)\n", __func__, (int) pMotion->x, (int) pMotion->y, pMotion->is_hint, pMotion->time - fLastTime);
	
	//\_______________ On elague le flux des MotionNotify, sinon X en envoie autant que le permet le CPU !
	Icon *pPointedIcon;
	if (pMotion != NULL)
	{
		if (pDock->bAtBottom || ! pDock->bInside || pDock->iSidShrinkDown > 0 || pMotion->time - fLastTime < g_fRefreshInterval)  // si les icones sont en train de diminuer de taille (suite a un clic) on ne redimensionne pas les icones, le temps que l'animation se finisse.
		{
			gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);
			return FALSE;
		}
		
		//\_______________ On recalcule toutes les icones.
		if (g_bHorizontalDock)
			pPointedIcon = cairo_dock_calculate_icons (pDock, (int) pMotion->x, (int) pMotion->y);
		else
			pPointedIcon = cairo_dock_calculate_icons (pDock, (int) pMotion->y, (int) pMotion->x);
		gtk_widget_queue_draw (pWidget);
		fLastTime = pMotion->time;
		
		//gdk_event_request_motions (pMotion);  // ce sera pour GDK 2.12.
		gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);  // pour recevoir d'autres MotionNotify.
	}
	else  // cas d'un drag and drop.
	{
		int iX, iY;
		if (g_bHorizontalDock)
			gdk_window_get_pointer (pWidget->window, &iX, &iY, NULL);
		else
			gdk_window_get_pointer (pWidget->window, &iY, &iX, NULL);
		pPointedIcon = cairo_dock_calculate_icons (pDock, iX, iY);
		cairo_dock_mark_icons_as_avoiding_mouse (pDock, iX);
		gtk_widget_queue_draw (pWidget);
	}
	
	if (pPointedIcon != pLastPointedIcon || g_pLastPointedDock == NULL)
	{
		//g_print ("on change d'icone (-> %s)\n", (pPointedIcon != NULL ? pPointedIcon->acName : "rien"));
		if ((pDock == g_pLastPointedDock || g_pLastPointedDock == NULL) && pLastPointedIcon != NULL && pLastPointedIcon->pSubDock != NULL)
		{
			if (GTK_WIDGET_VISIBLE (pLastPointedIcon->pSubDock->pWidget))
			{
				//g_print ("on cache %s en changeant d'icône\n", pLastPointedIcon->acName);
				//gdk_window_hide (pLastPointedIcon->pSubDock->pWidget->window);
				//while (gtk_events_pending ())
				//	gtk_main_iteration ();
				pLastPointedIcon->pSubDock->iScrollOffset = 0;
				gtk_widget_hide (pLastPointedIcon->pSubDock->pWidget);
			}
			//else
			//	g_print ("pas encore visible !\n");
		}
		if (pPointedIcon != NULL && pPointedIcon->pSubDock != NULL)
		{
			//g_print ("on montre le dock fils\n");
			CairoDock *pSubDock = pPointedIcon->pSubDock;
			
			if (pSubDock->iSidShrinkDown != 0)
			{
				g_source_remove (pSubDock->iSidShrinkDown);
				pSubDock->iSidShrinkDown = 0;
				pSubDock->fMagnitude = 0.0;
				cairo_dock_shrink_down (pSubDock);
			}
			
			pSubDock->iGapX = pDock->iGapX + (pPointedIcon->fX + pPointedIcon->fWidth * pPointedIcon->fScale / 2 - pDock->iMaxDockWidth / 2);
			pSubDock->iGapY = pDock->iGapY + pDock->iMaxDockHeight;
			
			int iNewWidth, iNewHeight;
			cairo_dock_calculate_window_position_at_balance (pSubDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);
			
			if (g_bHorizontalDock)
				gdk_window_move (pSubDock->pWidget->window, pSubDock->iWindowPositionX, pSubDock->iWindowPositionY);
			else
				gdk_window_move (pSubDock->pWidget->window, pSubDock->iWindowPositionY, pSubDock->iWindowPositionX);
			//gdk_window_show (pSubDock->pWidget->window);
			g_pLastPointedDock = pDock;
			pSubDock->bAtBottom = FALSE;
			gtk_window_present (GTK_WINDOW (pSubDock->pWidget));
		}
		pLastPointedIcon = pPointedIcon;
		if (g_pLastPointedDock == NULL)
			 g_pLastPointedDock = pDock;
	}
	
	return FALSE;
}


void cairo_dock_leave_from_main_dock (CairoDock *pDock)
{
	g_print ("%s (iSidShrinkDown : %d)\n", __func__, pDock->iSidShrinkDown);
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
		g_source_remove (pDock->iSidGrowUp);
		pDock->iSidGrowUp = 0;
	}
	
	if (g_bAutoHide && pDock->iRefCount == 0)
	{
		if (pDock->iSidMoveDown == 0)  // on commence a descendre.
			pDock->iSidMoveDown = g_timeout_add (40, (GSourceFunc) cairo_dock_move_down, (gpointer) pDock);
	}
	else
	{
		pDock->bAtBottom = TRUE;
	}
	
	if (pDock->iSidShrinkDown == 0)  // on commence a faire diminuer la taille des icones.
		pDock->iSidShrinkDown = g_timeout_add (35, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
	
	g_pLastPointedDock = NULL;
	//g_print ("g_pLastPointedDock <- NULL\n");
}
gboolean on_leave_notify2 (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	if (pDock->bAtBottom || ! pDock->bInside)
		return FALSE;
	//g_print ("%s (main dock : %d)\n", __func__, pDock->bIsMainDock);
	
	while (gtk_events_pending ())  // on laisse le temps au signal d'entree dans le sous-dock d'etre traite.
		gtk_main_iteration ();
	
	if (! cairo_dock_hide_child_docks (pDock))  // on quitte si on entre dans un sous-dock, pour rester en position "haute".
		return FALSE;
	
	cairo_dock_leave_from_main_dock (pDock);
	
	return FALSE;
}


gboolean on_enter_notify2 (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDock *pDock)
{
	if (pDock->bAtTop || pDock->bInside || pDock->iSidMoveDown != 0)  // le 'iSidMoveDown != 0' est la pour empecher le dock de "vibrer" si l'utilisateur sort par en bas avec l'auto-hide active.
		return FALSE;
	//g_print ("%s (main dock : %d)\n", __func__, pDock->bIsMainDock);
	
	if (! pDock->bIsMainDock)
		gtk_window_present (GTK_WINDOW (pWidget));
	pDock->bInside = TRUE;
	
	int iNewWidth, iNewHeight;
	cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_MAX_SIZE, &iNewWidth, &iNewHeight);
	if ((g_bAutoHide && pDock->iRefCount == 0) && pDock->bAtBottom)
		pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - g_iVisibleZoneHeight - pDock->iGapY : g_iVisibleZoneHeight + pDock->iGapY - pDock->iMaxDockHeight);
	
	//g_print (" -> %dx%d\n", g_iWindowPositionX, g_iWindowPositionY);
	///if (pDock->bAtBottom || ! (g_bAutoHide && pDock->iRefCount == 0))
	if (g_bHorizontalDock)
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
		g_source_remove (pDock->iSidMoveDown);
		pDock->iSidMoveDown = 0;
	}
	/*if (g_iSidShrinkDown > 0)  // si on est en train de faire diminuer la tailler des icones, on arrete.
	{
		g_source_remove (g_iSidShrinkDown);
		g_iSidShrinkDown = 0;
	}*/
	
	if (g_bAutoHide && pDock->iRefCount == 0)
	{
		if (pDock->iSidMoveUp == 0)  // on commence a monter.
			pDock->iSidMoveUp = g_timeout_add (40, (GSourceFunc) cairo_dock_move_up, (gpointer) pDock);
	}
	else
	{
		pDock->bAtTop = TRUE;
		pDock->bAtBottom = FALSE;
	}
	if (pDock->iSidGrowUp == 0 && pDock->iSidShrinkDown == 0)  // on commence a faire grossir les icones.
		pDock->iSidGrowUp = g_timeout_add (35, (GSourceFunc) cairo_dock_grow_up, (gpointer) pDock);
	
	return FALSE;
}


void cairo_dock_update_gaps_with_window_position (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	int iWidth, iHeight;  // mieux que iCurrentWidth.
	if (g_bHorizontalDock)
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
	x = pDock->iWindowPositionX + iWidth / 2;
	y = (g_bDirectionUp ? pDock->iWindowPositionY + iHeight : pDock->iWindowPositionY);
	
	pDock->iGapX = x - g_iScreenWidth / 2;
	pDock->iGapY = (g_bDirectionUp ? g_iScreenHeight - y : y);
	
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

gboolean on_key_press (GtkWidget *pWidget,
			GdkEventKey *pKey,
			CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	iMoveByArrow ++;
	int iPossibleMove;
	
	int iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pDock->pWidget), &iWidth, &iHeight);  // mieux que iCurrentWidth.
	
	int x, y;  // position du centre bas du dock;
	x = pDock->iWindowPositionX +iWidth / 2;
	y = pDock->iWindowPositionY + iHeight - 1;
	if (pKey->type == GDK_KEY_PRESS)
	{
		GdkEventScroll dummyScroll;
		int iX, iY;
		switch (pKey->keyval)
		{
			case GDK_q :
				//if (pDock->bIsMainDock && pKey != NULL && pKey->state & GDK_CONTROL_MASK)  // CTRL + q quitte l'appli.
				//	gtk_main_quit ();
			break;
			
			case GDK_Down :
				iPossibleMove = MAX (0, g_iScreenHeight - 1 - y);
				iMoveByArrow = MIN (iMoveByArrow, iPossibleMove);
				if (iMoveByArrow > 0)
				{
					pDock->iWindowPositionY += iMoveByArrow;
					pDock->iGapY -= iMoveByArrow;
					gtk_window_move (GTK_WINDOW (pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
					if (pDock->bIsMainDock)
						cairo_dock_update_conf_file_with_position (g_cConfFile, pDock->iGapX, pDock->iGapY);
				}
			break;

			case GDK_Up :
				iPossibleMove = MAX (0, y - pDock->iMaxDockHeight);
				iMoveByArrow = MIN (iMoveByArrow, iPossibleMove);
				if (iMoveByArrow > 0)
				{
					pDock->iWindowPositionY -= iMoveByArrow;
					pDock->iGapY += iMoveByArrow;
					gtk_window_move (GTK_WINDOW (pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
					if (pDock->bIsMainDock)
						cairo_dock_update_conf_file_with_position (g_cConfFile, pDock->iGapX, pDock->iGapY);
				}
			break;
			
			case GDK_Left :
				iPossibleMove = MAX (0, x - pDock->iMinDockWidth / 2);
				iMoveByArrow = MIN (iMoveByArrow, iPossibleMove);
				if (iMoveByArrow > 0)
				{
					pDock->iWindowPositionX -= iMoveByArrow;
					pDock->iGapX -= iMoveByArrow;
					gtk_window_move (GTK_WINDOW (pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
					if (pDock->bIsMainDock)
						cairo_dock_update_conf_file_with_position (g_cConfFile, pDock->iGapX, pDock->iGapY);
				}
			break;
			
			case GDK_Right :
				iPossibleMove = MAX (0, g_iScreenWidth - (x + pDock->iMinDockWidth / 2 + g_iDockRadius + g_iDockLineWidth));
				iMoveByArrow = MIN (iMoveByArrow, iPossibleMove);
				if (iMoveByArrow > 0)
				{
					pDock->iWindowPositionX += iMoveByArrow;
					pDock->iGapX += iMoveByArrow;
					gtk_window_move (GTK_WINDOW (pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
					if (pDock->bIsMainDock)
						cairo_dock_update_conf_file_with_position (g_cConfFile, pDock->iGapX, pDock->iGapY);
				}
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
	
	return FALSE;
}


gboolean on_button_press2 (GtkWidget* pWidget,
				GdkEventButton* pButton,
				CairoDock *pDock)
{
	//g_print ("%s (%d/%d)\n", __func__, pButton->type, pButton->button);
	if (pButton->button == 1)  // clique gauche.
	{
		if (pButton->type == GDK_BUTTON_RELEASE)
		{
			if (s_pIconClicked != NULL)
				s_pIconClicked->iAnimationType = 0;
			
			Icon *icon = cairo_dock_get_pointed_icon (pDock->icons);
			if (icon != NULL && ! CAIRO_DOCK_IS_SEPARATOR (icon) && icon == s_pIconClicked)
			{
				icon->iAnimationType = g_tAnimationType[icon->iType];
				if (icon->iAnimationType == CAIRO_DOCK_RANDOM)
					icon->iAnimationType =  g_random_int_range (0, CAIRO_DOCK_NB_ANIMATIONS);  // [a;b[
				icon->iCount = g_tNbIterInOneRound[icon->iAnimationType] * g_tNbAnimationRounds[icon->iType] - 1;
				
				if (icon->acCommand != NULL)
				{
					g_spawn_command_line_async (icon->acCommand, NULL);
				}
				else if (icon->Xid != 0)
				{
					cairo_dock_show_appli (icon->Xid);
				}
				else if (icon->pModule != NULL && icon->pModule->actionModule != NULL)
				{
					icon->pModule->actionModule ();
				}
				
				if (icon->pSubDock != NULL)
					gtk_widget_hide (icon->pSubDock->pWidget);
				
				if (pDock->iSidShrinkDown == 0)
					pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);  // fera diminuer de taille les icones, et rebondir/tourner/clignoter celle qui est cliquee.
			}
			else if (s_pIconClicked != NULL && icon != NULL && icon != s_pIconClicked && icon->iType == s_pIconClicked->iType)
			{
				//g_print ("deplacement de %s\n", s_pIconClicked->acName);
				CairoDock *pOriginDock = cairo_dock_search_container_from_icon (s_pIconClicked);
				if (pDock != pOriginDock)
				{
					cairo_dock_remove_icon_from_dock (pOriginDock, s_pIconClicked);
					cairo_dock_update_dock_size (pOriginDock, pOriginDock->iMaxIconHeight, pOriginDock->iMinDockWidth);
					
					cairo_dock_update_icon_s_container_name (s_pIconClicked, icon->cParentDockName);
				}
				
				s_pIconClicked->iCount = 20;  // 2 rebonds.
				s_pIconClicked->iAnimationType = CAIRO_DOCK_BOUNCE;
				int iX, iY;
				if (g_bHorizontalDock)
				{
					iX = pButton->x;
					iY = pButton->y;
				}
				else
				{
					iX = pButton->y;
					iY = pButton->x;
				}
				
				if (iX > icon->fX + icon->fWidth * icon->fScale / 2)
					cairo_dock_move_icon_after_icon (pDock, s_pIconClicked, icon);
				else
				{
					Icon *prev_icon = cairo_dock_get_previous_icon (pDock->icons, icon);
					cairo_dock_move_icon_after_icon (pDock, s_pIconClicked, prev_icon);
				}
				
				if (pDock != pOriginDock)
					cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
				cairo_dock_calculate_icons (pDock, iX, iY);
				gtk_widget_queue_draw (pWidget);
				
				if (pDock->iSidShrinkDown == 0)  // on commence a faire diminuer la taille des icones.
					pDock->iSidShrinkDown = g_timeout_add (35, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
			}
			s_pIconClicked = NULL;
		}
		else if (pButton->type == GDK_BUTTON_PRESS)
		{
			s_pIconClicked = cairo_dock_get_pointed_icon (pDock->icons);
			s_pIconClicked->iAnimationType = CAIRO_DOCK_FOLLOW_MOUSE;
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
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
		if (g_bHorizontalDock)
			cairo_dock_calculate_icons (pDock, (int) pButton->x, (int) pButton->y);
		else
			cairo_dock_calculate_icons (pDock, (int) pButton->y, (int) pButton->x);
		gtk_widget_queue_draw (pWidget);
		/*gtk_window_begin_move_drag (GTK_WINDOW (pWidget),
			pButton->button,
			pButton->x_root,
			pButton->y_root,
			pButton->time);  // permet de déplacer la fenetre, marche avec Metacity, mais pas avec Beryl !*/
	}
	
	return FALSE;
}

gboolean on_scroll (GtkWidget* pWidget,
			GdkEventScroll* pScroll,
			CairoDock *pDock)
{
	static double fLastTime = 0;
	//g_print ("%s (%d)\n", __func__, pScroll->direction);
	if (pDock->bAtBottom || ! pDock->bInside || pDock->iSidShrinkDown > 0 || pScroll->time - fLastTime < g_fRefreshInterval)  // si les icones sont en train de diminuer de taille (suite a un clic) on ne redimensionne pas les icones, le temps que l'animation se finisse.
	{
		return FALSE;
	}
	
	Icon *pLastPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	Icon *pNeighborIcon;
	if (pScroll->direction == GDK_SCROLL_UP)
	{
		pNeighborIcon = cairo_dock_get_previous_icon (pDock->icons, pLastPointedIcon);
		if (pNeighborIcon == NULL)
			pNeighborIcon = cairo_dock_get_last_icon (pDock->icons);
		if (pNeighborIcon == NULL)
			return FALSE;
		pDock->iScrollOffset += ((pScroll->state & GDK_CONTROL_MASK) || g_iScrollAmount == 0 ? (pNeighborIcon->fWidth + (pLastPointedIcon != NULL ? pLastPointedIcon->fWidth : 0)) / 2 : g_iScrollAmount);
	}
	else if (pScroll->direction == GDK_SCROLL_DOWN)
	{
		pNeighborIcon = cairo_dock_get_next_icon (pDock->icons, pLastPointedIcon);
		if (pNeighborIcon == NULL)
			pNeighborIcon = cairo_dock_get_first_icon (pDock->icons);
		if (pNeighborIcon == NULL)
			return FALSE;
		pDock->iScrollOffset -= ((pScroll->state & GDK_CONTROL_MASK) || g_iScrollAmount == 0 ? (pNeighborIcon->fWidth + (pLastPointedIcon != NULL ? pLastPointedIcon->fWidth : 0)) / 2 : g_iScrollAmount);
	}
	else
	{
		return FALSE;
	}
	
	if (pDock->iScrollOffset >= pDock->iMinDockWidth)
		pDock->iScrollOffset -= pDock->iMinDockWidth;
	if (pDock->iScrollOffset < 0)
		pDock->iScrollOffset += pDock->iMinDockWidth;
	//g_print ("iScrollOffset <- %d, (%d;%d)\n", pDock->iScrollOffset, (int) pScroll->x, (int) pScroll->y);
	
	//pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, pDock->iMinDockWidth, pDock->iScrollOffset);
	cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
	
	//\_______________ On recalcule toutes les icones.
	Icon *pPointedIcon;
	if (g_bHorizontalDock)
		pPointedIcon = cairo_dock_calculate_icons (pDock, (int) pScroll->x, (int) pScroll->y);
	else
		pPointedIcon = cairo_dock_calculate_icons (pDock, (int) pScroll->y, (int) pScroll->x);
	gtk_widget_queue_draw (pWidget);
	fLastTime = pScroll->time;
	
	//\_______________ On montre les sous-docks.
	if (pPointedIcon != pLastPointedIcon || g_pLastPointedDock == NULL)
	{
		//g_print ("on change d'icone\n");
		if (pDock == g_pLastPointedDock && pLastPointedIcon != NULL && pLastPointedIcon->pSubDock != NULL)
		{
			if (GTK_WIDGET_VISIBLE (pLastPointedIcon->pSubDock->pWidget))
				gdk_window_hide (pLastPointedIcon->pSubDock->pWidget->window);
		}
		if (pPointedIcon != NULL && pPointedIcon->pSubDock != NULL)
		{
			//g_print ("on montre le dock fils\n");
			CairoDock *pSubDock = pPointedIcon->pSubDock;
			
			if (pSubDock->iSidShrinkDown != 0)
			{
				g_source_remove (pSubDock->iSidShrinkDown);
				pSubDock->iSidShrinkDown = 0;
				pSubDock->fMagnitude = 0.0;
				cairo_dock_shrink_down (pSubDock);
			}
			
			pSubDock->iGapX = pDock->iGapX + (pPointedIcon->fX + pPointedIcon->fWidth * pPointedIcon->fScale / 2 - pDock->iMaxDockWidth / 2);
			pSubDock->iGapY = pDock->iGapY + pDock->iMaxDockHeight;
			
			int iNewWidth, iNewHeight;
			cairo_dock_calculate_window_position_at_balance (pSubDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);
			
			if (g_bHorizontalDock)
				gdk_window_move (pSubDock->pWidget->window, pSubDock->iWindowPositionX, pSubDock->iWindowPositionY);
			else
				gdk_window_move (pSubDock->pWidget->window, pSubDock->iWindowPositionY, pSubDock->iWindowPositionX);
			g_pLastPointedDock = pDock;
			gtk_window_present (GTK_WINDOW (pSubDock->pWidget));
			//gtk_widget_show (pSubDock->pWidget);
		}
		pLastPointedIcon = pPointedIcon;
	}
	
	return FALSE;
}


gboolean on_configure (GtkWidget* pWidget,
			GdkEventConfigure* pEvent,
			CairoDock *pDock)
{
	//g_print ("%s (main dock : %d)\n", __func__, pDock->bIsMainDock);
	gint iNewWidth, iNewHeight;
	if (g_bHorizontalDock)
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
		if (g_bHorizontalDock)
			gdk_window_get_pointer (pWidget->window, &iX, &iY, NULL);
		else
			gdk_window_get_pointer (pWidget->window, &iY, &iX, NULL);
		if (iX < 0 || iX > pDock->iCurrentWidth)
			iX = 0;
		
		cairo_dock_calculate_icons (pDock, iX, iY);
		
		///if (gdk_window_is_visible (pWidget->window))
		///	gtk_window_present (GTK_WINDOW (pWidget));
		gtk_widget_queue_draw (pWidget);
#ifdef HAVE_GLITZ
		if (g_pGlitzDrawable)
			glitz_drawable_update_size (pDock->pGlitzDrawable,
				pEvent->width,
				pEvent->height);
#endif
	}

	return FALSE;
}


void on_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, CairoDock *pDock)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	
	cairo_dock_mark_icons_as_avoiding_mouse (pDock, -1e4);
	
	gchar *cReceivedData = (gchar *) selection_data->data;
	if (strncmp (cReceivedData, "file://", 7) == 0)
	{
		GError *erreur = NULL;
		//\_________________ On recupere l'URI du .desktop.
		gchar *cFilePath = g_strdup (cReceivedData + 7);  // on saute le "file://".
		gchar *str = strrchr (cFilePath, '\n');
		if (str != NULL)
			*(str-1) = '\0';  // on vire les retrours chariot.
		
		//\_________________ On calcule la position a laquelle on l'a lache.
		double fOrder = 0;
		CairoDock *pReceivingDock = pDock;
		GList *ic;
		Icon *icon, *next_icon, *prev_icon;
		for (ic = pDock->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->bPointed)
			{
				//g_print ("On pointe sur %s\n", icon->acName);
				if (icon->pSubDock != NULL)
				{
					pReceivingDock = icon->pSubDock;
				}
				else if (x > icon->fX + icon->fWidth * icon->fScale / 2)  // on est apres.
				{
					next_icon = (ic->next != NULL ? ic->next->data : NULL);
					fOrder = (next_icon != NULL ? (icon->fOrder + next_icon->fOrder) / 2 : icon->fOrder + 1);
				}
				else
				{
					prev_icon = (ic->prev != NULL ? ic->prev->data : NULL);
					fOrder = (prev_icon != NULL ? (icon->fOrder + prev_icon->fOrder) / 2 : icon->fOrder - 1);
				}
			}
		}
		
		//\_________________ On l'ajoute dans le repertoire .cairo-dock.
		const gchar *cDockName = cairo_dock_search_dock_name (pReceivingDock);
		gchar *cNewDesktopFilePath = cairo_dock_add_desktop_file_from_path (cFilePath, cDockName, fOrder, pDock, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return ;
		}
		
		//\_________________ On charge ce nouveau lanceur.
		if (cNewDesktopFilePath != NULL)
		{
			gchar *cDesktopFileName = g_path_get_basename (cNewDesktopFilePath);
			g_free (cNewDesktopFilePath);
			cairo_t* pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
			Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFileName, pCairoContext);
			g_free (cDesktopFileName);
			cairo_destroy (pCairoContext);
			
			if (pNewIcon != NULL)
				cairo_dock_insert_icon_in_dock (pNewIcon, pReceivingDock, TRUE, TRUE);
			
			if (pDock->iSidShrinkDown == 0)  // on lance l'animation.
				pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
		}
		
		g_free (cFilePath);
	}
}


void on_drag_motion (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, guint t, CairoDock *pDock)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	//\_________________ On simule les evenements souris habituels.
	on_enter_notify2 (pWidget, NULL, pDock);  // ne sera effectif que la 1ere fois.
	on_motion_notify2 (pWidget, NULL, pDock);
}


gboolean on_delete (GtkWidget *pWidget, GdkEvent *event, CairoDock *pDock)
{
	if (pDock->bIsMainDock)
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
	}
	else
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		"destroy this dock ?");
		int answer = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (answer == GTK_RESPONSE_YES)
		{
			const gchar *cDockName = cairo_dock_search_dock_name (pDock);
			gboolean bDestroyIcons = TRUE;
			dialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"Do you want to re-dispatch the icons contained inside this container into the dock (otherwise they will be destroyed) ?");
			int answer = gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			if (answer == GTK_RESPONSE_YES)
				bDestroyIcons = FALSE;
			cairo_dock_destroy_dock (pDock, cDockName, (bDestroyIcons ? NULL : g_pMainDock), (bDestroyIcons ? NULL : CAIRO_DOCK_MAIN_DOCK_NAME));
		}
	}
	return TRUE;
}
