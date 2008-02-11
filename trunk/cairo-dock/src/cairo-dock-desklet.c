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

#include "cairo-dock-draw.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-config.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-log.h"
#include "cairo-dock-menu.h"
#include "cairo-dock-desklet.h"

extern CairoDock *g_pMainDock;
extern gchar *g_cConfFile;
extern int g_iDockRadius;
extern double g_fDialogColor[4];
extern gboolean g_bSticky;


static gboolean on_expose_desklet(GtkWidget *pWidget,
                                     GdkEventExpose *pExpose,
                                     CairoDockDesklet *pDesklet)
{
  //cd_message ("%s ()\n", __func__);
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
	if (gtk_window_is_active(GTK_WINDOW(pDesklet->pWidget)))  /// je verrais bien un changement au survol (utiliser un flag bIsInside et les enter & leave events.) On pourrait meme avoir les 3 etats.
	///eviter a tous prix les flags qui servent a rien qd on peut determiner l'etat autrement, flag a mettre a js = source d'erreur
		cairo_set_source_rgba (pCairoContext, g_fDialogColor[0], g_fDialogColor[1], g_fDialogColor[2], MIN (1., g_fDialogColor[3] * 1.25));
	else if (pDesklet->bInside)
		cairo_set_source_rgba (pCairoContext, g_fDialogColor[0], g_fDialogColor[1], g_fDialogColor[2], g_fDialogColor[3]);
	else
		cairo_set_source_rgba (pCairoContext, g_fDialogColor[0], g_fDialogColor[1], g_fDialogColor[2], g_fDialogColor[3] * .75);

  cairo_set_line_width (pCairoContext, 2*g_iDockRadius);
  cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_ROUND);

  //draw a rounded square
  //gtk_window_get_size(GTK_WINDOW(pDesklet->pWidget), &w, &h);
  w = pDesklet->iWidth;
  h = pDesklet->iHeight;
  cairo_move_to (pCairoContext, g_iDockRadius, g_iDockRadius);
  cairo_rel_line_to (pCairoContext, w - (g_iDockRadius << 1), 0);
  cairo_rel_line_to (pCairoContext, 0, h - (g_iDockRadius << 1));
  cairo_rel_line_to (pCairoContext, -(w - (g_iDockRadius << 1)) , 0);
  cairo_set_line_join (pCairoContext, CAIRO_LINE_JOIN_ROUND);
  cairo_close_path (pCairoContext);
  cairo_stroke (pCairoContext);

  /*   cairo_set_source_rgba (pCairoContext, 1., 1., 1., 1.); */
  cairo_rectangle(pCairoContext, g_iDockRadius, g_iDockRadius, (w - (g_iDockRadius << 1)), (h - (g_iDockRadius << 1)));
  cairo_fill(pCairoContext);


	if (pDesklet->renderer != NULL)  // une fonction de dessin specifique a ete fournie.
	{
		cairo_destroy (pCairoContext);
		pDesklet->renderer (pDesklet);
	}
	else if (pDesklet->pIcon != NULL)
	{
		cairo_restore (pCairoContext);
		Icon *pIcon = pDesklet->pIcon;

		if (pIcon->pIconBuffer != NULL)
		{
			//cd_message ("  dessin de l'icone (%.2fx%.2f)\n", pIcon->fWidth, pIcon->fHeight);
			cairo_translate (pCairoContext, g_iDockRadius, g_iDockRadius);
			cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
			cairo_paint (pCairoContext);
		}
		if (pIcon->pQuickInfoBuffer != NULL)
		{
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
		cairo_dock_reload_module (pDesklet->pIcon->pModule, g_pMainDock, TRUE);
		//gtk_widget_queue_draw (pDesklet->pWidget);
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
	cd_message ("%s (%dx%d ; %d,%d)\n", __func__, pEvent->width, pEvent->height, (int) pEvent->x, (int) pEvent->y);
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
			cd_message ("diff : %d;%d\n", pDesklet->diff_x, pDesklet->diff_y);
		}
		else if (pButton->type == GDK_BUTTON_RELEASE)
		{
			cd_message ("GDK_BUTTON_RELEASE\n");
			if (pDesklet->moving)
			{
				pDesklet->moving = FALSE;
				/*if (pDesklet->pIcon != NULL && pDesklet->pIcon->pModule != NULL)
					cairo_dock_update_conf_file (pDesklet->pIcon->pModule->cConfFilePath,
						G_TYPE_INT, "Desklet", "x position", pDesklet->iWindowPositionX,
						G_TYPE_INT, "Desklet", "y position", pDesklet->iWindowPositionY,
						G_TYPE_INT, "Desklet", "width", pDesklet->iWidth,
						G_TYPE_INT, "Desklet", "height", pDesklet->iHeight,
						G_TYPE_INVALID);*/
			}
			else
			{
				gpointer data[2] = {pDesklet->pIcon, pDesklet};
				cairo_dock_notify (CAIRO_DOCK_CLICK_ICON, data);
			}
		}
		else if (pButton->type == GDK_2BUTTON_PRESS)
		{
			gpointer data[2] = {pDesklet->pIcon, pDesklet};
			cairo_dock_notify (CAIRO_DOCK_DOUBLE_CLICK_ICON, data);
		}
	}
	else if (pButton->button == 3 && pButton->type == GDK_BUTTON_PRESS)  // clique droit.
	{
		GtkWidget *menu = cairo_dock_build_menu (pDesklet->pIcon, pDesklet);  // genere un CAIRO_DOCK_BUILD_MENU.
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
		gpointer data[2] = {pDesklet->pIcon, pDesklet};
		cairo_dock_notify (CAIRO_DOCK_MIDDLE_CLICK_ICON, data);
	}
	return FALSE;
}

static gboolean on_motion_notify_desklet(GtkWidget *pWidget,
	GdkEventMotion* pMotion,
	CairoDockDesklet *pDesklet)
{
	if (pMotion->state & GDK_BUTTON1_MASK)
	{
		cd_message ("root : %d;%d\n", pMotion->x_root, pMotion->y_root);
		gtk_window_move (GTK_WINDOW (pWidget),
			pMotion->x_root + pDesklet->diff_x,
			pMotion->y_root + pDesklet->diff_y);
		pDesklet->moving = TRUE;
		return TRUE;
	}
	return FALSE;
}


static void on_button_press_desklet_nbt(GtkButton *button, CairoDockDesklet *pDesklet)
{
	GtkWidget *menu = cairo_dock_build_menu (pDesklet->pIcon, pDesklet);  // genere un CAIRO_DOCK_BUILD_MENU.
	gtk_widget_show_all (menu);
	gtk_menu_popup (GTK_MENU (menu),
		NULL,
		NULL,
		NULL,
		NULL,
		1,
		gtk_get_current_event_time ());
}



static gboolean cd_desklet_on_focus_in_out(GtkWidget *widget,
	GdkEventFocus *event,
	CairoDockDesklet *pDesklet)
{
	if (pDesklet)
		gtk_widget_queue_draw(pDesklet->pWidget);
	return FALSE;
}

gboolean on_enter_desklet (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDockDesklet *pDesklet)
{
	if (! pDesklet->bInside)  // avant on etait dehors, on redessine donc.
	{
		pDesklet->bInside = TRUE;
		gtk_widget_queue_draw (pWidget);
	}
	return FALSE;
}

static gboolean on_leave_desklet (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDockDesklet *pDesklet)
{
	int iMouseX, iMouseY;
	gdk_window_get_pointer (pWidget->window, &iMouseX, &iMouseY, NULL);
	if (iMouseX > 0 && iMouseX < pDesklet->iWidth && iMouseY > 0 && iMouseY < pDesklet->iHeight)  // en fait on est dans un widget fils, donc on ne fait rien.
	{
		return FALSE;
	}

	pDesklet->bInside = FALSE;
	gtk_widget_queue_draw (pWidget);
	return FALSE;
}



CairoDockDesklet *cairo_dock_create_desklet (Icon *pIcon, GtkWidget *pInteractiveWidget)
{
  cd_message ("%s ()\n", __func__);
  GtkWidget* vbox, *hbox, *btn;
  CairoDockDesklet *pDesklet = g_new0(CairoDockDesklet, 1);
  pDesklet->iType = CAIRO_DOCK_DESKLET;
  GtkWidget* pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  pDesklet->pWidget = pWindow;

  if (g_bSticky)
	  gtk_window_stick(GTK_WINDOW(pWindow));
  gtk_window_set_skip_pager_hint(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pWindow), TRUE);
  cairo_dock_set_colormap_for_window(pWindow);
  gtk_widget_set_app_paintable(pWindow, TRUE);
  gtk_window_set_decorated(GTK_WINDOW(pWindow), FALSE);
  gtk_window_set_resizable(GTK_WINDOW(pWindow), TRUE);
  gtk_window_set_title(GTK_WINDOW(pWindow), "cairo-dock-desklet");  /// distinguer titre et classe ?
  gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_FOCUS_CHANGE_MASK);
  //the border is were cairo paint
  gtk_container_set_border_width(GTK_CONTAINER(pWindow), 2*g_iDockRadius);  /// 10
  gtk_window_set_default_size(GTK_WINDOW(pWindow), 4*g_iDockRadius+1, 4*g_iDockRadius+1);
  hbox = gtk_hbox_new(0, 0);
  gtk_container_add(GTK_CONTAINER(pWindow), hbox);

  vbox = gtk_vbox_new(0, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

  btn = gtk_button_new_with_label("-");
  gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (on_button_press_desklet_nbt), pDesklet);  // Nuclear, Bacteriologic, Thermic ? :-)

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

  //user widget
  if (pInteractiveWidget != NULL)
  {
    cd_message ("ref = %d\n", pInteractiveWidget->object.parent_instance.ref_count);
    if (gtk_widget_get_parent (pInteractiveWidget) != NULL)
      {
        gtk_object_ref((gpointer)pInteractiveWidget);
        gtk_widget_unparent(pInteractiveWidget);
      }
    else
      gtk_object_ref((gpointer)pInteractiveWidget);
    gtk_box_pack_start(GTK_BOX(hbox), pInteractiveWidget, TRUE, TRUE, 0);
    gtk_object_unref((gpointer)pInteractiveWidget);
    cd_message ("pack -> ref = %d\n", pInteractiveWidget->object.parent_instance.ref_count);
  }

  gtk_widget_show_all(pWindow);

  return pDesklet;
}


void cairo_dock_place_desklet (CairoDockDesklet *pDesklet, int iWidth, int iHeight, int iPositionX, int iPositionY, gboolean bKeepBelow, gboolean bKeepAbove, gboolean bOnWidgetLayer)
{
	cd_message ("%s (%dx%d ; (%d,%d) ; %d,%d,%d)\n", __func__, iWidth, iHeight, iPositionX, iPositionY, bKeepBelow, bKeepAbove, bOnWidgetLayer);
	gdk_window_move_resize (pDesklet->pWidget->window,
		iPositionX,
		iPositionY,
		iWidth,
		iHeight);

	gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), bKeepBelow);
	gtk_window_set_keep_above( GTK_WINDOW (pDesklet->pWidget), bKeepAbove);

	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	if (bOnWidgetLayer)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_UTILITY");  // le hide-show le fait deconner completement, il perd son skip_task_bar ! au moins sous KDE.
	else
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
}

void cairo_dock_free_desklet (CairoDockDesklet *pDesklet)
{
	if (pDesklet == NULL)
		return;

	gtk_widget_destroy (pDesklet->pWidget);  // detruit aussi l'eventuel widget interactif.
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
	GtkWidget* hbox = gtk_bin_get_child (GTK_BIN (pDesklet->pWidget));
	GtkWidget* vbox = gtk_bin_get_child (GTK_BIN (hbox));
	gtk_box_pack_start (GTK_BOX (vbox),
		pInteractiveWidget,
		TRUE,
		TRUE,
		0);
}
