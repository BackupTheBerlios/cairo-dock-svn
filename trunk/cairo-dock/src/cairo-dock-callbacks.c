/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


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
#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-applications.h"
#include "cairo-dock-desktop-file-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-config.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-callbacks.h"


extern gint g_iScreenWidth;
extern gint g_iScreenHeight;

extern double g_fAmplitude;
extern int g_iLabelSize;
extern gboolean g_bUseText;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bAutoHide;
extern int g_iIconGap;

extern gchar *g_cConfFile;
extern gchar *g_cCairoDockDataDir;


extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern gchar *g_cLabelPolice;


extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern int g_iNbStripes;

extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;
extern double g_fRefreshInterval;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];

#ifdef HAVE_GLITZ
extern gboolean g_bUseGlitz;
extern glitz_drawable_format_t *gDrawFormat;
extern glitz_drawable_t* g_pGlitzDrawable;
extern glitz_format_t* g_pGlitzFormat;
#endif // HAVE_GLITZ


gboolean move_up2 (CairoDock *pDock)
{
	int deltaY_possible;
	if (g_bHorizontalDock)
		deltaY_possible = (g_bDirectionUp ? pDock->iWindowPositionY - (g_iScreenHeight - pDock->iMaxDockHeight - pDock->iGapY) : pDock->iWindowPositionY - g_iScreenHeight + pDock->iGapY + g_iVisibleZoneHeight);
	else
		deltaY_possible = (g_bDirectionUp ? pDock->iGapX - pDock->iWindowPositionX : - g_iScreenWidth + pDock->iGapX + pDock->iMaxDockWidth + pDock->iWindowPositionX);
	//g_print ("%s (%dx%d -> %d)\n", __func__, g_iWindowPositionX, g_iWindowPositionY, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 0) || (! g_bDirectionUp && deltaY_possible < 0))  // alors on peut encore monter.
	{
		if (g_bHorizontalDock)
			pDock->iWindowPositionY -= (int) (deltaY_possible * g_fMoveUpSpeed) + (g_bDirectionUp ? 1 : -1); // 0.5
		else
			pDock->iWindowPositionX += (int) (deltaY_possible * g_fMoveUpSpeed) + (g_bDirectionUp ? 1 : -1);
		//g_print ("  move to (%dx%d)\n", g_iWindowPositionX, g_iWindowPositionY);
		gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
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


gboolean move_down2 (CairoDock *pDock)
{
	if (pDock->fMagnitude > 0.1)  // on retarde le cachage du dock pour apercevoir les effets.
		return TRUE;
	int deltaY_possible = g_iScreenHeight - pDock->iGapY - (g_bDirectionUp ? g_iVisibleZoneHeight : pDock->iMaxDockHeight) - pDock->iWindowPositionY;
	//g_print ("%s (%d)\n", __func__, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 4) || (! g_bDirectionUp && deltaY_possible < -4))  // alors on peut encore descendre.
	{
		pDock->iWindowPositionY += (int) (deltaY_possible * g_fMoveDownSpeed) + (g_bDirectionUp ? 1 : -1);  // 0.33
		gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
		pDock->bAtTop = FALSE;
		return TRUE;
	}
	else  // on se fixe en bas, et on montre la zone visible.
	{
		pDock->bAtBottom = TRUE;
		pDock->iWindowPositionX = (g_iScreenWidth - g_iVisibleZoneWidth) / 2 + pDock->iGapX;
		pDock->iWindowPositionY = g_iScreenHeight - g_iVisibleZoneHeight - pDock->iGapY;
		//g_print ("%s () -> %dx%d\n", __func__, g_iVisibleZoneWidth, g_iVisibleZoneHeight);
		gdk_window_move_resize (pDock->pWidget->window,
			pDock->iWindowPositionX,
			pDock->iWindowPositionY,
			g_iVisibleZoneWidth,
			g_iVisibleZoneHeight);
		pDock->iSidMoveDown = 0;
		
		if (g_bAutoHide)
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


gboolean on_expose (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoDock *pDock)
{
	//g_print ("%s ((%d;%d) %dx%d)\n", __func__, pExpose->area.x, pExpose->area.y, pExpose->area.width, pExpose->area.height);
	
	if (pExpose->area.x + pExpose->area.y != 0)  // x et y sont >= 0.
	{
		if (! g_bAutoHide || ! pDock->bAtBottom)
			cairo_dock_render_optimized (pDock, &pExpose->area);
		
		return FALSE;
	}
	
	if (!pDock->bAtBottom)
	{
		render (pDock);
		
	}
	else
	{
		if (g_bAutoHide)
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
	static double fLastTime = 9;
	static Icon *pLastPointedIcon = NULL;
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
		pPointedIcon = cairo_dock_calculate_icons (pDock, (int) pMotion->x, (int) pMotion->y);
		gtk_widget_queue_draw (pWidget);
		fLastTime = pMotion->time;
		
		//gdk_event_request_motions (pMotion);  // ce sera pour GDK 2.12.
		gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);  // pour recevoir d'autres MotionNotify.
	}
	else  // cas d'un drag and drop.
	{
		int iX, iY;
		gdk_window_get_pointer (pWidget->window, &iX, &iY, NULL);
		pPointedIcon = cairo_dock_calculate_icons (pDock, iX, iY);
		gtk_widget_queue_draw (pWidget);
	}
	
	
	if (pPointedIcon != pLastPointedIcon)
	{
		if (pLastPointedIcon != NULL && pLastPointedIcon->pSubDock != NULL)
			gtk_widget_hide (pLastPointedIcon->pSubDock->pWidget);
		if (pPointedIcon->pSubDock != NULL)
			gtk_widget_show (pPointedIcon->pSubDock->pWidget);
		pLastPointedIcon = pPointedIcon;
	}
	
	return FALSE;
}


gboolean on_leave_notify2 (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDock *pDock)
{
	if (pDock->bAtBottom || ! pDock->bInside)
		return FALSE;
	//g_print ("%s ()\n", __func__);
	
	pDock->bInside = FALSE;
	if (pDock->bMenuVisible)
	{
		pDock->bAtTop = FALSE;
		return FALSE;
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
	
	if (g_bAutoHide)
	{
		if (pDock->iSidMoveDown == 0)  // on commence a descendre.
			pDock->iSidMoveDown = g_timeout_add (40, (GSourceFunc) move_down2, (gpointer) pDock);
	}
	else
	{
		pDock->bAtTop = FALSE;
		pDock->bAtBottom = TRUE;
	}
	
	if (pDock->iSidShrinkDown == 0)  // on commence a faire diminuer la taille des icones.
		pDock->iSidShrinkDown = g_timeout_add (35, (GSourceFunc) shrink_down2, (gpointer) pDock);
	
	return FALSE;
	
}


gboolean on_enter_notify2 (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDock *pDock)
{
	if (pDock->bAtTop || pDock->bInside)
		return FALSE;
	//g_print ("%s (%d)\n", __func__, g_iWindowPositionY);
	
	gtk_window_present (GTK_WINDOW (pWidget));
	pDock->bInside = TRUE;
	
	
	if (g_bHorizontalDock)
	{
		pDock->iWindowPositionX = (g_iScreenWidth - pDock->iMaxDockWidth) / 2 + pDock->iGapX;
		if (! g_bAutoHide)
			pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - pDock->iMaxDockHeight - pDock->iGapY : g_iScreenHeight - pDock->iGapY);
		else
			pDock->iWindowPositionY = (g_bDirectionUp ? pDock->iWindowPositionY : g_iVisibleZoneHeight - pDock->iMaxDockHeight + (g_iScreenHeight - pDock->iGapY));
	}
	else
	{
		pDock->iWindowPositionY = (g_iScreenHeight - pDock->iMaxDockHeight) / 2 + pDock->iGapY;
		if (! g_bAutoHide)
			pDock->iWindowPositionX = (g_bDirectionUp ? pDock->iWindowPositionX : g_iScreenWidth - pDock->iGapX - pDock->iMaxDockWidth);
		else
			pDock->iWindowPositionX = (g_bDirectionUp ? pDock->iGapX - pDock->iMaxDockWidth : g_iScreenWidth - pDock->iGapX - pDock->iMaxDockWidth);
	}
	//g_print (" -> %dx%d\n", g_iWindowPositionX, g_iWindowPositionY);
		
	gdk_window_move_resize (pWidget->window,
		pDock->iWindowPositionX,
		pDock->iWindowPositionY,
		pDock->iMaxDockWidth,
		pDock->iMaxDockHeight);
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
	
	if (g_bAutoHide)
	{
		if (pDock->iSidMoveUp == 0)  // on commence a monter.
			pDock->iSidMoveUp = g_timeout_add (40, (GSourceFunc) move_up2, (gpointer) pDock);
	}
	else
	{
		pDock->bAtTop = TRUE;
		pDock->bAtBottom = FALSE;
	}
	if (pDock->iSidGrowUp == 0 && pDock->iSidShrinkDown == 0)  // on commence a faire grossir les icones.
		pDock->iSidGrowUp = g_timeout_add (35, (GSourceFunc) grow_up2, (gpointer) pDock);
	
	return FALSE;
}


void cairo_dock_update_gaps_with_window_position (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	int iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pDock->pWidget), &iWidth, &iHeight);
	gtk_window_get_position (GTK_WINDOW (pDock->pWidget), &pDock->iWindowPositionX, &pDock->iWindowPositionY);
	
	int x, y;  // position du centre bas du dock;
	x = pDock->iWindowPositionX + iWidth / 2;
	y = pDock->iWindowPositionY + (g_bDirectionUp ? iHeight : (g_bAutoHide ? g_iVisibleZoneHeight : 0));
	
	pDock->iGapX = x - g_iScreenWidth / 2;
	pDock->iGapY = g_iScreenHeight - y;
	
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
	gtk_window_get_size (GTK_WINDOW (pDock->pWidget), &iWidth, &iHeight);
	int x, y;  // position du centre bas du dock;
	x = pDock->iWindowPositionX +iWidth / 2;
	y = pDock->iWindowPositionY + iHeight;
	if (pKey->type == GDK_KEY_PRESS)
	{
		switch (pKey->keyval)
		{
			case GDK_q :
				if (pKey != NULL && pKey->state & GDK_CONTROL_MASK)  // CTRL + q quitte l'appli.
					gtk_main_quit ();
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
		}
	}
	
	return FALSE;
}


gboolean on_button_press2 (GtkWidget* pWidget,
				GdkEventButton* pButton,
				CairoDock *pDock)
{
	//g_print ("%s (%d/%d)\n", __func__, pButton->type, pButton->button);
	if (pButton->type == GDK_BUTTON_PRESS)  // simple clique.
	{
		if (pButton->button == 1)  // clique gauche.
		{
			Icon *icon = cairo_dock_get_pointed_icon (pDock->icons);
			if (icon != NULL && ! CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				icon->iAnimationType = g_tAnimationType[icon->iType];
				do
				{
					switch (icon->iAnimationType)
					{
						case CAIRO_DOCK_BOUNCE:
							icon->iCount = 10;  // 5 tours pour monter et 5 pour descendre.
						break;
						case CAIRO_DOCK_ROTATE:
							icon->iCount = 20;  // 5 iterations par quart de tour.
						break;
						case CAIRO_DOCK_BLINK:
							icon->iCount = 20;  // 10 iterations par inversion d'alpha.
						break;
						case CAIRO_DOCK_RANDOM:
							icon->iCount = 0;
							icon->iAnimationType =  g_random_int_range (0, CAIRO_DOCK_NB_ANIMATIONS);
						break;
						default :
							icon->iCount = 10;
					}
				} while (icon->iCount == 0);
				icon->iCount = icon->iCount * g_tNbAnimationRounds[icon->iType] - 1;
				
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
				
				if (pDock->iSidShrinkDown == 0)
					pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pDock);  // fera diminuer de taille les icones, et rebondir/tourner/clignoter celle qui est cliquee.
			}
		}
		else if (pButton->button == 3)  // clique droit.
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
		else if (pButton->button == 2)  // clique milieu.
		{
			gtk_window_begin_move_drag (GTK_WINDOW (pWidget),
				pButton->button,
				pButton->x_root,
				pButton->y_root,
				pButton->time);  // permet de déplacer la fenetre, marche avec Metacity, mais pas avec Beryl !
		}
	}
	
	return FALSE;
}
gboolean on_button_release (GtkWidget* pWidget,
				GdkEventButton* pButton,
				CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	if (pButton->button == 2)  // fin d'un drag and drop, sauf que ca ne marche ni avec Beryl, ni avec Metacity, car ils ne laissent pas passer le signal de relache du bouton :-/
	{
		cairo_dock_update_gaps_with_window_position (pDock);
	}
	return FALSE;
}


gboolean on_configure (GtkWidget* pWidget,
			GdkEventConfigure* pEvent,
			CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	gint iNewWidth = pEvent->width;
	gint iNewHeight = pEvent->height;

	if (iNewWidth != pDock->iCurrentWidth || iNewHeight != pDock->iCurrentHeight)
	{
		//g_print ("-> %dx%d\n", iNewWidth, iNewHeight);
		pDock->iCurrentWidth = iNewWidth;
		pDock->iCurrentHeight = iNewHeight;
		
		cairo_dock_calculate_icons (pDock, iNewWidth / 2, 0);
		if (!pDock->bAtBottom)
		{
			render (pDock);
		}
		else
		{
			if (g_bAutoHide)
				cairo_dock_render_background (pDock);
			else
				render (pDock);
		}


#ifdef HAVE_GLITZ
		if (g_pGlitzDrawable)
			glitz_drawable_update_size (g_pGlitzDrawable,
						    g_iCurrentWidth,
						    g_iCurrentHeight);
#endif
	}

	return FALSE;
}


void on_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, CairoDock *pDock)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	
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
		gchar *cDesktopFileName = g_path_get_basename (cNewDesktopFilePath);
		g_free (cNewDesktopFilePath);
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFileName, pCairoContext);
		g_free (cDesktopFileName);
		cairo_destroy (pCairoContext);
		
		if (pNewIcon != NULL)
			cairo_dock_insert_icon_in_dock (pNewIcon, pReceivingDock, TRUE, TRUE);
		
		if (pDock->iSidShrinkDown == 0)  // on lance l'animation.
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pDock);
		
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

