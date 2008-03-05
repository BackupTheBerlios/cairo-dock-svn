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
extern gchar *g_cConfFile;
extern int g_iDockRadius;
extern double g_fDeskletColor[4];
extern double g_fDeskletColorInside[4];
extern gboolean g_bSticky;

static gboolean on_expose_desklet(GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoDockDesklet *pDesklet)
{
  //cd_debug ("%s ()", __func__);
  gint w = 0, h = 0;

  if (!pDesklet)
    return FALSE;

  cairo_t *pCairoContext = gdk_cairo_create (pWidget->window);
  if (cairo_status(pCairoContext) != CAIRO_STATUS_SUCCESS) {
    cairo_destroy (pCairoContext);
    return FALSE;
  }

  //erase the background
  cairo_set_source_rgba (pCairoContext, 0., 0., 0., 0.);
  cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
  cairo_paint (pCairoContext);
  cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
  cairo_save (pCairoContext);

	//set the color
	double fColor[4];
	int i;
	for (i = 0; i < 4; i ++)
	{
		fColor[i] = (g_fDeskletColorInside[i] * pDesklet->iGradationCount + g_fDeskletColor[i] * (CD_NB_ITER_FOR_GRADUATION - pDesklet->iGradationCount)) / CD_NB_ITER_FOR_GRADUATION;
	}
	 if (gtk_window_is_active (GTK_WINDOW (pDesklet->pWidget)))
		fColor[3] = MIN (1., fColor[3] * 1.25);
	cairo_set_source_rgba (pCairoContext, fColor[0], fColor[1], fColor[2], fColor[3]);
	cairo_save (pCairoContext);
	cairo_set_line_width (pCairoContext, g_iDockRadius);
	cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
  //draw a rounded square
  //gtk_window_get_size(GTK_WINDOW(pDesklet->pWidget), &w, &h);
  w = pDesklet->iWidth;
  h = pDesklet->iHeight;
  cairo_move_to (pCairoContext, g_iDockRadius>>1, g_iDockRadius>>1);
  cairo_rel_line_to (pCairoContext, w - (g_iDockRadius), 0);
  cairo_rel_line_to (pCairoContext, 0, h - (g_iDockRadius));
  cairo_rel_line_to (pCairoContext, -(w - (g_iDockRadius)) , 0);
  cairo_close_path (pCairoContext);
  cairo_stroke (pCairoContext);

  cairo_restore (pCairoContext);

  cairo_rectangle(pCairoContext, g_iDockRadius, g_iDockRadius, (w - (g_iDockRadius << 1)), (h - (g_iDockRadius << 1)));
  cairo_fill(pCairoContext);


	cairo_restore (pCairoContext);  // retour au debut.
	if (pDesklet->renderer != NULL)  // une fonction de dessin specifique a ete fournie.
	{
		//cairo_translate (pCairoContext, g_iDockRadius, g_iDockRadius);
		pDesklet->renderer (pCairoContext, pDesklet->pRendererData);
		cairo_destroy (pCairoContext);
	}
	else if (pDesklet->pIcon != NULL)  // sinon par defaut on dessine l'icone dans le desklet.
	{
		Icon *pIcon = pDesklet->pIcon;
		cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);

		if (pIcon->pIconBuffer != NULL)
		{
			//cd_debug ("  dessin de l'icone (%.2fx%.2f)", pIcon->fWidth, pIcon->fHeight);
			//cairo_translate (pCairoContext, g_iDockRadius, g_iDockRadius);
			cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
			cairo_paint (pCairoContext);
		}
		if (pIcon->pQuickInfoBuffer != NULL)
		{
			//cd_debug ("  dessin de l'info-rapide (%dx%d)", pIcon->iQuickInfoWidth, pIcon->iQuickInfoHeight);
			cairo_translate (pCairoContext,
				//-icon->fQuickInfoXOffset + icon->fWidth / 2,
				//icon->fHeight - icon->fQuickInfoYOffset);
				(- pIcon->iQuickInfoWidth + pIcon->fWidth) / 2 * pIcon->fScale,
				(pIcon->fHeight - pIcon->iQuickInfoHeight) * pIcon->fScale);

			cairo_set_source_surface (pCairoContext,
				pIcon->pQuickInfoBuffer,
				0,
				0);
			cairo_paint (pCairoContext);
		}
		cairo_destroy (pCairoContext);
	}
	else
		cairo_destroy (pCairoContext);

	return FALSE;
}


static gboolean _cairo_dock_write_desklet_size (CairoDockDesklet *pDesklet)
{
	if (pDesklet->pIcon != NULL && pDesklet->pIcon->pModule != NULL)
		cairo_dock_update_conf_file (pDesklet->pIcon->pModule->cConfFilePath,
			G_TYPE_INT, "Desklet", "width", pDesklet->iWidth,
			G_TYPE_INT, "Desklet", "height", pDesklet->iHeight,
			G_TYPE_INVALID);
	pDesklet->iSidWriteSize = 0;
	if (pDesklet->pIcon != NULL)
	{
		cairo_dock_reload_module (pDesklet->pIcon->pModule, FALSE);
		gtk_widget_queue_draw (pDesklet->pWidget);  // sinon on redessine que l'interieur.
	}
	return FALSE;
}
static gboolean _cairo_dock_write_desklet_position (CairoDockDesklet *pDesklet)
{
	if (pDesklet->pIcon != NULL && pDesklet->pIcon->pModule != NULL)
		cairo_dock_update_conf_file (pDesklet->pIcon->pModule->cConfFilePath,
			G_TYPE_INT, "Desklet", "x position", pDesklet->iWindowPositionX,
			G_TYPE_INT, "Desklet", "y position", pDesklet->iWindowPositionY,
			G_TYPE_INVALID);
	pDesklet->iSidWritePosition = 0;
	return FALSE;
}
static gboolean on_configure_desklet (GtkWidget* pWidget,
                                      GdkEventConfigure* pEvent,
                                      CairoDockDesklet *pDesklet)
{
	//cd_debug ("%s (%dx%d ; %d,%d)", __func__, pEvent->width, pEvent->height, (int) pEvent->x, (int) pEvent->y);
	if (pDesklet->iWidth != pEvent->width || pDesklet->iHeight != pEvent->height)
	{
		pDesklet->iWidth = pEvent->width;
		pDesklet->iHeight = pEvent->height;
		if (pDesklet->iSidWriteSize != 0)
		{
			g_source_remove (pDesklet->iSidWriteSize);
			pDesklet->iSidWriteSize = 0;
		}

		pDesklet->iSidWriteSize = g_timeout_add (100, (GSourceFunc) _cairo_dock_write_desklet_size, (gpointer) pDesklet);
	}

	if (pDesklet->iWindowPositionX != pEvent->x || pDesklet->iWindowPositionY != pEvent->y)
	{
		pDesklet->iWindowPositionX = pEvent->x;
		pDesklet->iWindowPositionY = pEvent->y;

		if (pDesklet->iSidWritePosition != 0)
		{
			g_source_remove (pDesklet->iSidWritePosition);
			pDesklet->iSidWritePosition = 0;
		}
		pDesklet->iSidWritePosition = g_timeout_add (100, (GSourceFunc) _cairo_dock_write_desklet_position, (gpointer) pDesklet);
	}

	return FALSE;
}


Icon *_cairo_dock_find_clicked_icon_in_desklet (CairoDockDesklet *pDesklet)
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
	CairoDockDesklet *pDesklet)
{
	if (pButton->button == 1)  // clic gauche.
	{
		if (pButton->type == GDK_BUTTON_PRESS)
		{
			pDesklet->diff_x = - pButton->x;  // pour le deplacement manuel.
			pDesklet->diff_y = - pButton->y;
			///pDesklet->moving = TRUE;  // on ne peut pas le mettre a TRUE ici, sinon au release, on saute toujours le lancement de la notification.
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
				Icon *pClickedIcon = _cairo_dock_find_clicked_icon_in_desklet (pDesklet);
				gpointer data[2] = {pClickedIcon, pDesklet};
				cairo_dock_notify (CAIRO_DOCK_CLICK_ICON, data);
			}
		}
		else if (pButton->type == GDK_2BUTTON_PRESS)
		{
			Icon *pClickedIcon = _cairo_dock_find_clicked_icon_in_desklet (pDesklet);
			gpointer data[2] = {pClickedIcon, pDesklet};
			cairo_dock_notify (CAIRO_DOCK_DOUBLE_CLICK_ICON, data);
		}
	}
	else if (pButton->button == 3 && pButton->type == GDK_BUTTON_PRESS)  // clique droit.
	{
		Icon *pClickedIcon = _cairo_dock_find_clicked_icon_in_desklet (pDesklet);
		GtkWidget *menu = cairo_dock_build_menu (pClickedIcon, CAIRO_DOCK_CONTAINER (pDesklet));  // genere un CAIRO_DOCK_BUILD_MENU.
		gtk_widget_show_all (menu);
		gtk_menu_popup (GTK_MENU (menu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
		pDesklet->bInside = FALSE;
	}
	else if (pButton->button == 2 && pButton->type == GDK_BUTTON_PRESS)  // clique milieu.
	{
		gpointer data[2] = {pDesklet->pIcon, pDesklet};
		cairo_dock_notify (CAIRO_DOCK_MIDDLE_CLICK_ICON, data);
	}
	return FALSE;
}

void on_drag_data_received_desklet (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, CairoDockDesklet *pDesklet)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	
	//\_________________ On recupere l'URI.
	gchar *cReceivedData = (gchar *) selection_data->data;
	g_return_if_fail (cReceivedData != NULL);
	int length = strlen (cReceivedData);
	if (cReceivedData[length-1] == '\n')
		cReceivedData[--length] = '\0';  // on vire le retour chariot final.
	if (cReceivedData[length-1] == '\r')
		cReceivedData[--length] = '\0';
	cd_message (">>> cReceivedData : %s", cReceivedData);
	
	pDesklet->diff_x = - x;
	pDesklet->diff_y = - y;
	
	double fOrder = 0;
	Icon *pClickedIcon = _cairo_dock_find_clicked_icon_in_desklet (pDesklet);
	gpointer data[4] = {cReceivedData, pClickedIcon, &fOrder, pDesklet};
	cairo_dock_notify (CAIRO_DOCK_DROP_DATA, data);
}

static gboolean on_motion_notify_desklet(GtkWidget *pWidget,
	GdkEventMotion* pMotion,
	CairoDockDesklet *pDesklet)
{
	if (pMotion->state & GDK_BUTTON1_MASK /**&& pDesklet->moving*/)
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
	CairoDockDesklet *pDesklet)
{
	if (pDesklet)
		gtk_widget_queue_draw(pDesklet->pWidget);
	return FALSE;
}

static gboolean _cairo_dock_desklet_gradation (CairoDockDesklet *pDesklet)
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
	CairoDockDesklet *pDesklet)
{
	cd_debug ("%s (%d)", __func__, pDesklet->bInside);
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
	CairoDockDesklet *pDesklet)
{
	cd_debug ("%s (%d)", __func__, pDesklet->bInside);
	int iMouseX, iMouseY;
	gdk_window_get_pointer (pWidget->window, &iMouseX, &iMouseY, NULL);
	if (iMouseX > 0 && iMouseX < pDesklet->iWidth && iMouseY > 0 && iMouseY < pDesklet->iHeight)  // en fait on est dans un widget fils, donc on ne fait rien.
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



CairoDockDesklet *cairo_dock_create_desklet (Icon *pIcon, GtkWidget *pInteractiveWidget, gboolean bOnWidgetLayer)
{
  cd_message ("%s ()", __func__);
  CairoDockDesklet *pDesklet = g_new0(CairoDockDesklet, 1);
  pDesklet->iType = CAIRO_DOCK_TYPE_DESKLET;
  pDesklet->bIsHorizontal = TRUE;
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
  gtk_container_set_border_width(GTK_CONTAINER(pWindow), g_iDockRadius/2);  /// re-utiliser la formule des dialgues...
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

void cairo_dock_place_desklet (CairoDockDesklet *pDesklet, CairoDockMinimalAppletConfig *pMinimalConfig)
{
	cd_message ("%s (%dx%d ; (%d,%d) ; %d,%d,%d)", __func__, pMinimalConfig->iDeskletWidth, pMinimalConfig->iDeskletHeight, pMinimalConfig->iDeskletPositionX, pMinimalConfig->iDeskletPositionY, pMinimalConfig->bKeepBelow, pMinimalConfig->bKeepAbove, pMinimalConfig->bOnWidgetLayer);
	if (pMinimalConfig->bDeskletUseSize)
		gdk_window_resize (pDesklet->pWidget->window,
			pMinimalConfig->iDeskletWidth,
			pMinimalConfig->iDeskletHeight);

	gdk_window_move(pDesklet->pWidget->window,
		pMinimalConfig->iDeskletPositionX,
		pMinimalConfig->iDeskletPositionY);

	gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->bKeepBelow);
	gtk_window_set_keep_above (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->bKeepAbove);

	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	if (pMinimalConfig->bOnWidgetLayer)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_UTILITY");  // le hide-show le fait deconner completement, il perd son skip_task_bar ! au moins sous KDE.
	else
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
}


void cairo_dock_free_desklet (CairoDockDesklet *pDesklet)
{
	if (pDesklet == NULL)
		return;

	GtkWidget *pInteractiveWidget = gtk_bin_get_child (GTK_BIN (pDesklet->pWidget));
	if (pInteractiveWidget != NULL)
		cairo_dock_steal_widget_from_its_container (pInteractiveWidget);

	gtk_widget_destroy (pDesklet->pWidget);
	pDesklet->pWidget = NULL;

	g_free(pDesklet);
}

void cairo_dock_hide_desklet (CairoDockDesklet *pDesklet)
{
	if (pDesklet)
		gtk_widget_hide (pDesklet->pWidget);
}

void cairo_dock_show_desklet (CairoDockDesklet *pDesklet)
{
	if (pDesklet)
		gtk_window_present(GTK_WINDOW(pDesklet->pWidget));
}


void cairo_dock_add_interactive_widget_to_desklet (GtkWidget *pInteractiveWidget, CairoDockDesklet *pDesklet)
{
	g_return_if_fail (pDesklet != NULL);
	gtk_container_add (GTK_CONTAINER (pDesklet->pWidget), pInteractiveWidget);
}



static void _cairo_dock_set_one_desklet_visible (CairoDockDesklet *pDesklet, CairoDockModule *pModule, gpointer data)
{
	gboolean bOnWidgetLayerToo = GPOINTER_TO_INT (data);
	if (bOnWidgetLayerToo)
	{
		Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
	}
	
	gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), FALSE);
	gtk_window_set_keep_above (GTK_WINDOW (pDesklet->pWidget), TRUE);
	
	cairo_dock_show_desklet (pDesklet);
}
void cairo_dock_set_all_desklets_visible (gboolean bOnWidgetLayerToo)
{
	cairo_dock_foreach_desklet (_cairo_dock_set_one_desklet_visible, GINT_TO_POINTER (bOnWidgetLayerToo));
}

static void _cairo_dock_set_one_desklet_visibility_to_default (CairoDockDesklet *pDesklet, CairoDockModule *pModule, CairoDockMinimalAppletConfig *pMinimalConfig)
{
	GKeyFile *pKeyFile = cairo_dock_pre_read_module_config (pModule, pMinimalConfig);
	g_key_file_free (pKeyFile);
	
	gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->bKeepBelow);
	gtk_window_set_keep_above (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->bKeepAbove);
	
	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	if (pMinimalConfig->bOnWidgetLayer)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_UTILITY");
	else
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
}
void cairo_dock_set_desklets_visibility_to_default (void)
{
	CairoDockMinimalAppletConfig pMinimalConfig;
	cairo_dock_foreach_desklet (_cairo_dock_set_one_desklet_visibility_to_default, &pMinimalConfig);
}
