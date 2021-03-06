/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
** Login : <ctaf42@gmail.com>
** Started on  Fri Nov 30 05:31:31 2007 GESTES Cedric
** $Id$
**
** Copyright (C) 2007 GESTES Cedric
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
#include <signal.h>

#include "systray-menu-functions.h"
#include "systray-init.h"
#include "systray-struct.h"
#include "cd-tray.h"



CairoDialog *cd_systray_build_dialog (void)
{
	CairoDialogAttribute attr;
	memset (&attr, 0, sizeof (CairoDialogAttribute));
	attr.pInteractiveWidget = myData.tray->widget;
	return cairo_dock_build_dialog (&attr, myIcon, myContainer);
}
static void systray_build_new_dialog();
void systray_on_keybinding_pull (const char *keystring, gpointer user_data);

void systray_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	if (myData.tray)
	{
		if (myDesklet)
			cairo_dock_show_desklet(myDesklet);
		else if (myData.dialog)
			cairo_dock_unhide_dialog(myData.dialog);
	}
	else
	{
		systray_build_and_show ();
	}
}

void systray_apply_settings()
{
  cd_keybinder_bind(myConfig.shortcut, (CDBindkeyHandler)systray_on_keybinding_pull, 0);
}

void systray_build_and_show (void)
{
	myData.tray = tray_init(g_pMainDock->pWidget);
	gtk_widget_show (myData.tray->widget);

	systray_apply_settings();

	if (myDock)
	{
		myData.dialog = cd_systray_build_dialog ();
		//myData.dialog = cairo_dock_build_dialog (NULL, myIcon, myContainer, NULL, myData.tray->widget, GTK_BUTTONS_NONE, NULL, NULL, NULL);
		gtk_window_set_resizable(GTK_WINDOW(myData.dialog->pWidget), FALSE);
		//	gtk_window_resize(GTK_WINDOW(myData.dialog->pWidget), 2*g_iDockRadius, 2*g_iDockRadius);
	}
	else
	{
		cairo_dock_add_interactive_widget_to_desklet (myData.tray->widget, myDesklet);
		cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		gtk_window_set_resizable(GTK_WINDOW(myDesklet->pWidget), FALSE);
//		gtk_window_resize(GTK_WINDOW(myDesklet->pWidget), 2*g_iDockRadius, 2*g_iDockRadius);
		//cairo_dock_set_xwindow_type_hint (GDK_WINDOW_XID (myDesklet->pWidget->window), "_NET_WM_WINDOW_TYPE_DOCK");
	}
}


CD_APPLET_ON_CLICK_BEGIN
{
	if (! myData.tray)
		systray_build_and_show ();
	else if (myDesklet)
		cairo_dock_show_desklet (myDesklet);
	else if (myData.dialog)
		cairo_dock_unhide_dialog(myData.dialog);
}
CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
	if (myData.tray)
	{
		if (myData.dialog)
			cairo_dock_hide_dialog (myData.dialog);
	}
}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
{
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END


