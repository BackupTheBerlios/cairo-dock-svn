/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
** Login : <ctaf42@gmail.com>
** Started on  Sun Jan 27 18:35:38 2008 Cedric GESTES
** $Id$
**
** Author(s)
**  - Cedric GESTES <ctaf42@gmail.com>
**  - Fabrice REY
**
** Copyright (C) 2008 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gdk/gdkx.h>
#include "cairo-dock-draw.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-config.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-log.h"
#include "cairo-dock-menu.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-X-utilities.h"
#include "cairo-dock-desklet.h"

extern CairoDock *g_pMainDock;
extern int g_iScreenWidth[2], g_iScreenHeight[2];
extern gchar *g_cConfFile;
extern int g_iDockRadius;
extern double g_fDeskletColor[4];
extern double g_fDeskletColorInside[4];
extern gboolean g_bSticky;

static gboolean on_expose_desklet(GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoDesklet *pDesklet)
{
	//cd_debug ("%s ()", __func__);
	if (!pDesklet)
	return FALSE;
	gint w = 0, h = 0;
	
	cairo_t *pCairoContext;
	//set the color
	double fColor[4];
	int i;
	for (i = 0; i < 4; i ++)
	{
		fColor[i] = (g_fDeskletColorInside[i] * pDesklet->iGradationCount + g_fDeskletColor[i] * (CD_NB_ITER_FOR_GRADUATION - pDesklet->iGradationCount)) / CD_NB_ITER_FOR_GRADUATION;
	}
	if (gtk_window_is_active (GTK_WINDOW (pDesklet->pWidget)))
		fColor[3] = MIN (1., fColor[3] * 1.25);
	
	
	gboolean bRenderOptimized = (pExpose->area.x > 0 || pExpose->area.y > 0);
	if (bRenderOptimized)
	{
		pCairoContext = cairo_dock_create_drawing_context_on_area (CAIRO_CONTAINER (pDesklet), &pExpose->area, fColor);
	}
	else
	{
		pCairoContext = cairo_dock_create_drawing_context (CAIRO_CONTAINER (pDesklet));
		
		if (fColor[3] != 0)
		{
			cairo_save (pCairoContext);
			cairo_set_source_rgba (pCairoContext, fColor[0], fColor[1], fColor[2], fColor[3]);
			cairo_set_line_width (pCairoContext, g_iDockRadius);
			cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
			//draw a rounded square
			w = pDesklet->iWidth;
			h = pDesklet->iHeight;
			cairo_move_to (pCairoContext, .5*g_iDockRadius, .5*g_iDockRadius);
			cairo_rel_line_to (pCairoContext, w - (g_iDockRadius), 0);
			cairo_rel_line_to (pCairoContext, 0, h - (g_iDockRadius));
			cairo_rel_line_to (pCairoContext, -(w - (g_iDockRadius)) , 0);
			cairo_close_path (pCairoContext);
			cairo_stroke (pCairoContext);
			
			cairo_rectangle(pCairoContext, g_iDockRadius, g_iDockRadius, (w - 2*g_iDockRadius), (h - 2*g_iDockRadius));
			cairo_fill(pCairoContext);
			cairo_restore (pCairoContext);  // retour au debut.
		}
	}
	
	if (pDesklet->pRenderer != NULL)  // un moteur de rendu specifique a ete fourni.
	{
		if (pDesklet->pRenderer->render != NULL)
			pDesklet->pRenderer->render (pCairoContext, pDesklet, bRenderOptimized);
	}
	
	cairo_destroy (pCairoContext);
	return FALSE;
}


static gboolean _cairo_dock_write_desklet_size (CairoDesklet *pDesklet)
{
	if (pDesklet->pIcon != NULL && pDesklet->pIcon->pModuleInstance != NULL)
		cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
			G_TYPE_INT, "Desklet", "width", pDesklet->iWidth,
			G_TYPE_INT, "Desklet", "height", pDesklet->iHeight,
			G_TYPE_INVALID);
	pDesklet->iSidWriteSize = 0;
	if (pDesklet->pIcon != NULL)
	{
		cairo_dock_reload_module_instance (pDesklet->pIcon->pModuleInstance, FALSE);
		gtk_widget_queue_draw (pDesklet->pWidget);  // sinon on ne redessine que l'interieur.
	}
	return FALSE;
}
static gboolean _cairo_dock_write_desklet_position (CairoDesklet *pDesklet)
{
	if (pDesklet->pIcon != NULL && pDesklet->pIcon->pModuleInstance != NULL)
	{
		int iRelativePositionX = (pDesklet->iWindowPositionX + pDesklet->iWidth/2 <= g_iScreenWidth[CAIRO_DOCK_HORIZONTAL]/2 ? pDesklet->iWindowPositionX : g_iScreenWidth[CAIRO_DOCK_HORIZONTAL]/2 - pDesklet->iWindowPositionX);
		int iRelativePositionY = (pDesklet->iWindowPositionY + pDesklet->iHeight/2 <= g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]/2 ? pDesklet->iWindowPositionY : g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]/2 - pDesklet->iWindowPositionY);
		cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
			G_TYPE_INT, "Desklet", "x position", iRelativePositionX,
			G_TYPE_INT, "Desklet", "y position", iRelativePositionY,
			G_TYPE_INVALID);
	}
	pDesklet->iSidWritePosition = 0;
	return FALSE;
}
static gboolean on_configure_desklet (GtkWidget* pWidget,
	GdkEventConfigure* pEvent,
	CairoDesklet *pDesklet)
{
	//cd_debug ("%s (%dx%d ; %d,%d)", __func__, pEvent->width, pEvent->height, (int) pEvent->x, (int) pEvent->y);
	if (pDesklet->iWidth != pEvent->width || pDesklet->iHeight != pEvent->height)
	{
		pDesklet->iWidth = pEvent->width;
		pDesklet->iHeight = pEvent->height;
		
		if (pDesklet->iSidWriteSize != 0)
		{
			g_source_remove (pDesklet->iSidWriteSize);
		}
		pDesklet->iSidWriteSize = g_timeout_add (500, (GSourceFunc) _cairo_dock_write_desklet_size, (gpointer) pDesklet);
	}

	if (pDesklet->iWindowPositionX != pEvent->x || pDesklet->iWindowPositionY != pEvent->y)
	{
		pDesklet->iWindowPositionX = pEvent->x;
		pDesklet->iWindowPositionY = pEvent->y;

		if (pDesklet->iSidWritePosition != 0)
		{
			g_source_remove (pDesklet->iSidWritePosition);
		}
		pDesklet->iSidWritePosition = g_timeout_add (500, (GSourceFunc) _cairo_dock_write_desklet_position, (gpointer) pDesklet);
	}

	return FALSE;
}

gboolean on_scroll_desklet (GtkWidget* pWidget,
	GdkEventScroll* pScroll,
	CairoDesklet *pDesklet)
{
	if (pScroll->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK))
	{
		Icon *icon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
		if (icon != NULL)
		{
			gpointer data[3] = {icon, pDesklet, GINT_TO_POINTER (pScroll->direction)};
			cairo_dock_notify (CAIRO_DOCK_SCROLL_ICON, data);
		}
	}
	return FALSE;
}


Icon *cairo_dock_find_clicked_icon_in_desklet (CairoDesklet *pDesklet)
{
	int iMouseX = - (int) pDesklet->diff_x;
	int iMouseY = - (int) pDesklet->diff_y;
	cd_debug (" clic en (%d;%d)", iMouseX, iMouseY);
	
	Icon *icon = pDesklet->pIcon;
	if (icon->fDrawX < iMouseX && icon->fDrawX + icon->fWidth * icon->fScale > iMouseX && icon->fDrawY < iMouseY && icon->fDrawY + icon->fHeight * icon->fScale > iMouseY)
	{
		return icon;
	}
	
	if (pDesklet->icons != NULL)
	{
		GList* ic;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->fDrawX < iMouseX && icon->fDrawX + icon->fWidth * icon->fScale > iMouseX && icon->fDrawY < iMouseY && icon->fDrawY + icon->fHeight * icon->fScale > iMouseY)
			{
				return icon;
			}
		}
	}
	return NULL;
}

static gboolean on_button_press_desklet(GtkWidget *widget,
	GdkEventButton *pButton,
	CairoDesklet *pDesklet)
{
	if (pButton->button == 1)  // clic gauche.
	{
		if (pButton->type == GDK_BUTTON_PRESS)
		{
			pDesklet->diff_x = - pButton->x;  // pour le deplacement manuel.
			pDesklet->diff_y = - pButton->y;
			cd_debug ("diff : %d;%d", pDesklet->diff_x, pDesklet->diff_y);
		}
		else if (pButton->type == GDK_BUTTON_RELEASE)
		{
			cd_debug ("GDK_BUTTON_RELEASE");
			if (pDesklet->moving)
			{
				pDesklet->moving = FALSE;
			}
			else
			{
				Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
				gpointer data[2] = {pClickedIcon, pDesklet};
				cairo_dock_notify (CAIRO_DOCK_CLICK_ICON, data);
			}
		}
		else if (pButton->type == GDK_2BUTTON_PRESS)
		{
			Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
			gpointer data[2] = {pClickedIcon, pDesklet};
			cairo_dock_notify (CAIRO_DOCK_DOUBLE_CLICK_ICON, data);
		}
	}
	else if (pButton->button == 3 && pButton->type == GDK_BUTTON_PRESS)  // clique droit.
	{
		Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
		GtkWidget *menu = cairo_dock_build_menu (pClickedIcon, CAIRO_CONTAINER (pDesklet));  // genere un CAIRO_DOCK_BUILD_MENU.
		gtk_widget_show_all (menu);
		gtk_menu_popup (GTK_MENU (menu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
		pDesklet->bInside = FALSE;
		pDesklet->iGradationCount = 0;  // on force le fond a redevenir transparent.
		gtk_widget_queue_draw (pDesklet->pWidget);
	}
	else if (pButton->button == 2 && pButton->type == GDK_BUTTON_PRESS)  // clique milieu.
	{
		gpointer data[2] = {pDesklet->pIcon, pDesklet};
		cairo_dock_notify (CAIRO_DOCK_MIDDLE_CLICK_ICON, data);
	}
	return FALSE;
}

void on_drag_data_received_desklet (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, CairoDesklet *pDesklet)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	
	//\_________________ On recupere l'URI.
	gchar *cReceivedData = (gchar *) selection_data->data;
	g_return_if_fail (cReceivedData != NULL);
	
	pDesklet->diff_x = - x;
	pDesklet->diff_y = - y;
	Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
	cairo_dock_notify_drop_data (cReceivedData, pClickedIcon, 0, CAIRO_CONTAINER (pDesklet));
}

static gboolean on_motion_notify_desklet(GtkWidget *pWidget,
	GdkEventMotion* pMotion,
	CairoDesklet *pDesklet)
{
	if (pMotion->state & GDK_BUTTON1_MASK && ! pDesklet->bPositionLocked)
	{
		cd_debug ("root : %d;%d", (int) pMotion->x_root, (int) pMotion->y_root);
		pDesklet->moving = TRUE;
		gtk_window_move (GTK_WINDOW (pWidget),
			pMotion->x_root + pDesklet->diff_x,
			pMotion->y_root + pDesklet->diff_y);
	}
	else  // le 'press-button' est local au sous-widget clique, alors que le 'motion-notify' est global a la fenetre; c'est donc par lui qu'on peut avoir a coup sur les coordonnees du curseur (juste avant le clic).
	{
		pDesklet->diff_x = -pMotion->x;
		pDesklet->diff_y = -pMotion->y;
	}
	gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);  // pour recevoir d'autres MotionNotify.
	return FALSE;
}


static gboolean cd_desklet_on_focus_in_out(GtkWidget *widget,
	GdkEventFocus *event,
	CairoDesklet *pDesklet)
{
	if (pDesklet)
		gtk_widget_queue_draw(pDesklet->pWidget);
	return FALSE;
}

static gboolean _cairo_dock_desklet_gradation (CairoDesklet *pDesklet)
{
	pDesklet->iGradationCount += (pDesklet->bInside ? 1 : -1);
	gtk_widget_queue_draw (pDesklet->pWidget);
	
	if (pDesklet->iGradationCount <= 0 || pDesklet->iGradationCount >= CD_NB_ITER_FOR_GRADUATION)
	{
		if (pDesklet->iGradationCount < 0)
			pDesklet->iGradationCount = 0;
		else if (pDesklet->iGradationCount > CD_NB_ITER_FOR_GRADUATION)
			pDesklet->iGradationCount = CD_NB_ITER_FOR_GRADUATION;
		pDesklet->iSidGradationOnEnter = 0;
		return FALSE;
	}
	return TRUE;
}
static gboolean on_enter_desklet (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDesklet *pDesklet)
{
	//g_print ("%s (%d)\n", __func__, pDesklet->bInside);
	if (! pDesklet->bInside)  // avant on etait dehors, on redessine donc.
	{
		pDesklet->bInside = TRUE;
		if (pDesklet->iSidGradationOnEnter == 0)
		{
			pDesklet->iSidGradationOnEnter = g_timeout_add (50, (GSourceFunc) _cairo_dock_desklet_gradation, (gpointer) pDesklet);
		}
	}
	return FALSE;
}
static gboolean on_leave_desklet (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDesklet *pDesklet)
{
	//g_print ("%s (%d)\n", __func__, pDesklet->bInside);
	int iMouseX, iMouseY;
	gdk_window_get_pointer (pWidget->window, &iMouseX, &iMouseY, NULL);
	if (gtk_bin_get_child (GTK_BIN (pDesklet->pWidget)) != NULL && iMouseX > 0 && iMouseX < pDesklet->iWidth && iMouseY > 0 && iMouseY < pDesklet->iHeight)  // en fait on est dans un widget fils, donc on ne fait rien.
	{
		return FALSE;
	}

	pDesklet->bInside = FALSE;
	if (pDesklet->iSidGradationOnEnter == 0)
	{
		pDesklet->iSidGradationOnEnter = g_timeout_add (50, (GSourceFunc) _cairo_dock_desklet_gradation, (gpointer) pDesklet);
	}
	return FALSE;
}

gboolean on_delete_desklet (GtkWidget *pWidget, GdkEvent *event, CairoDesklet *pDesklet)
{
	if (pDesklet->pIcon->pModuleInstance != NULL)
	{
		cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
			G_TYPE_BOOLEAN, "Desklet", "initially detached", FALSE,
			G_TYPE_INVALID);

		cairo_dock_reload_module_instance (pDesklet->pIcon->pModuleInstance, TRUE);
	}
	return TRUE;
}


CairoDesklet *cairo_dock_create_desklet (Icon *pIcon, GtkWidget *pInteractiveWidget, gboolean bOnWidgetLayer)
{
  cd_message ("%s ()", __func__);
  CairoDesklet *pDesklet = g_new0(CairoDesklet, 1);
  pDesklet->iType = CAIRO_DOCK_TYPE_DESKLET;
  pDesklet->bIsHorizontal = TRUE;
  pDesklet->bDirectionUp = TRUE;
  GtkWidget* pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if (bOnWidgetLayer)
		gtk_window_set_type_hint (GTK_WINDOW (pWindow), GDK_WINDOW_TYPE_HINT_UTILITY);
  pDesklet->pWidget = pWindow;
  pDesklet->pIcon = pIcon;

  if (g_bSticky)
	  gtk_window_stick(GTK_WINDOW(pWindow));
  gtk_window_set_skip_pager_hint(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pWindow), TRUE);
  cairo_dock_set_colormap_for_window(pWindow);
  gtk_widget_set_app_paintable(pWindow, TRUE);
  gtk_window_set_decorated(GTK_WINDOW(pWindow), FALSE);
  gtk_window_set_resizable(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_title(GTK_WINDOW(pWindow), "cairo-dock-desklet");  /// distinguer titre et classe ?...
  gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_FOCUS_CHANGE_MASK);
  //the border is were cairo paint
  gtk_container_set_border_width(GTK_CONTAINER(pWindow), g_iDockRadius/2);  /// re-utiliser la formule des dialogues...
  gtk_window_set_default_size(GTK_WINDOW(pWindow), 2*g_iDockRadius+1, 2*g_iDockRadius+1);

	g_signal_connect (G_OBJECT (pWindow),
		"expose-event",
		G_CALLBACK (on_expose_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"configure-event",
		G_CALLBACK (on_configure_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"motion-notify-event",
		G_CALLBACK (on_motion_notify_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"button-press-event",
		G_CALLBACK (on_button_press_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"button-release-event",
		G_CALLBACK (on_button_press_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"focus-in-event",
		G_CALLBACK (cd_desklet_on_focus_in_out),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"focus-out-event",
		G_CALLBACK (cd_desklet_on_focus_in_out),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"enter-notify-event",
		G_CALLBACK (on_enter_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"leave-notify-event",
		G_CALLBACK (on_leave_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"delete-event",
		G_CALLBACK (on_delete_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"scroll-event",
		G_CALLBACK (on_scroll_desklet),
		pDesklet);
	cairo_dock_allow_widget_to_receive_data (pWindow, G_CALLBACK (on_drag_data_received_desklet), pDesklet);

  //user widget
  if (pInteractiveWidget != NULL)
  {
    cd_message ("ref = %d", pInteractiveWidget->object.parent_instance.ref_count);
    gtk_container_add (GTK_CONTAINER (pDesklet->pWidget), pInteractiveWidget);
    cd_message ("pack -> ref = %d", pInteractiveWidget->object.parent_instance.ref_count);
  }

  gtk_widget_show_all(pWindow);

  return pDesklet;
}

void cairo_dock_place_desklet (CairoDesklet *pDesklet, CairoDockMinimalAppletConfig *pMinimalConfig)
{
	cd_message ("%s (%dx%d ; (%d,%d) ; %d,%d,%d)", __func__, pMinimalConfig->iDeskletWidth, pMinimalConfig->iDeskletHeight, pMinimalConfig->iDeskletPositionX, pMinimalConfig->iDeskletPositionY, pMinimalConfig->bKeepBelow, pMinimalConfig->bKeepAbove, pMinimalConfig->bOnWidgetLayer);
	if (pMinimalConfig->bDeskletUseSize)
		gdk_window_resize (pDesklet->pWidget->window,
			pMinimalConfig->iDeskletWidth,
			pMinimalConfig->iDeskletHeight);

	gdk_window_move(pDesklet->pWidget->window,
		(pMinimalConfig->iDeskletPositionX < 0 ?
			g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] - pMinimalConfig->iDeskletPositionX :
			pMinimalConfig->iDeskletPositionX),
		(pMinimalConfig->iDeskletPositionY < 0 ?
			g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] - pMinimalConfig->iDeskletPositionY :
			pMinimalConfig->iDeskletPositionY));

	gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->bKeepBelow);
	gtk_window_set_keep_above (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->bKeepAbove);

	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	if (pMinimalConfig->bOnWidgetLayer)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_UTILITY");  // le hide-show le fait deconner completement, il perd son skip_task_bar ! au moins sous KDE.
	else
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
	
	pDesklet->bPositionLocked = pMinimalConfig->bPositionLocked;
}


void cairo_dock_steal_interactive_widget_from_desklet (CairoDesklet *pDesklet)
{
	if (pDesklet == NULL)
		return;

	GtkWidget *pInteractiveWidget = gtk_bin_get_child (GTK_BIN (pDesklet->pWidget));
	if (pInteractiveWidget != NULL)
		cairo_dock_steal_widget_from_its_container (pInteractiveWidget);
}

void cairo_dock_free_desklet (CairoDesklet *pDesklet)
{
	if (pDesklet == NULL)
		return;

	cairo_dock_steal_interactive_widget_from_desklet (pDesklet);

	gtk_widget_destroy (pDesklet->pWidget);
	pDesklet->pWidget = NULL;
	
	if (pDesklet->pRenderer != NULL)
	{
		if (pDesklet->pRenderer->free_data != NULL)
		{
			pDesklet->pRenderer->free_data (pDesklet);
			pDesklet->pRendererData = NULL;
		}
	}
	
	if (pDesklet->icons != NULL)
	{
		g_list_foreach (pDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (pDesklet->icons);
		pDesklet->icons = NULL;
	}
	
	g_free(pDesklet);
}

void cairo_dock_hide_desklet (CairoDesklet *pDesklet)
{
	if (pDesklet)
		gtk_widget_hide (pDesklet->pWidget);
}

void cairo_dock_show_desklet (CairoDesklet *pDesklet)
{
	if (pDesklet)
		gtk_window_present(GTK_WINDOW(pDesklet->pWidget));
}


void cairo_dock_add_interactive_widget_to_desklet (GtkWidget *pInteractiveWidget, CairoDesklet *pDesklet)
{
	g_return_if_fail (pDesklet != NULL);
	gtk_container_add (GTK_CONTAINER (pDesklet->pWidget), pInteractiveWidget);
}



static gboolean _cairo_dock_set_one_desklet_visible (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, gpointer data)
{
	gboolean bOnWidgetLayerToo = GPOINTER_TO_INT (data);
	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	gboolean bIsOnWidgetLayer = cairo_dock_window_is_utility (Xid);
	if (bOnWidgetLayerToo || ! bIsOnWidgetLayer)
	{
		cd_debug ("%s (%d)", __func__, Xid);
		
		if (bIsOnWidgetLayer)  // on le passe sur la couche visible.
			cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
		
		gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), FALSE);
		
		cairo_dock_show_desklet (pDesklet);
	}
	return FALSE;
}
void cairo_dock_set_all_desklets_visible (gboolean bOnWidgetLayerToo)
{
	cd_debug ("%s (%d)", __func__, bOnWidgetLayerToo);
	cairo_dock_foreach_desklet (_cairo_dock_set_one_desklet_visible, GINT_TO_POINTER (bOnWidgetLayerToo));
}

static gboolean _cairo_dock_set_one_desklet_visibility_to_default (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, CairoDockMinimalAppletConfig *pMinimalConfig)
{
	GKeyFile *pKeyFile = cairo_dock_pre_read_module_instance_config (pInstance, pMinimalConfig);
	g_key_file_free (pKeyFile);
	
	gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->bKeepBelow);
	gtk_window_set_keep_above (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->bKeepAbove);
	
	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	if (pMinimalConfig->bOnWidgetLayer)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_UTILITY");
	else
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
	return FALSE;
}
void cairo_dock_set_desklets_visibility_to_default (void)
{
	CairoDockMinimalAppletConfig pMinimalConfig;
	cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cairo_dock_set_one_desklet_visibility_to_default, &pMinimalConfig);
}

static gboolean _cairo_dock_test_one_desklet_Xid (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, Window *pXid)
{
	return (GDK_WINDOW_XID (pDesklet->pWidget->window) == *pXid);
}
CairoDesklet *cairo_dock_get_desklet_by_Xid (Window Xid)
{
	CairoDockModuleInstance *pInstance = cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cairo_dock_test_one_desklet_Xid, &Xid);
	return (pInstance != NULL ? pInstance->pDesklet : NULL);
}
