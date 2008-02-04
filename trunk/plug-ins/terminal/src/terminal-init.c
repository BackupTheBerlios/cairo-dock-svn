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

#include "stdlib.h"

#include "cairo-dock-desklet.h"

#include "terminal-config.h"
#include "terminal-menu-functions.h"
#include "terminal-struct.h"
#include "terminal-init.h"

AppletConfig myConfig;
AppletData myData;

CD_APPLET_DEFINITION ("terminal", 1, 4, 7)


CD_APPLET_INIT_BEGIN (erreur)
{
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
{
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	//\_________________ On libere toutes nos ressources.
	reset_config ();
	reset_data ();
}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
{
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.dialog && myConfig.bIsInitiallyDetached)  // il faut le detacher.
		{
			myData.tab = cairo_dock_steal_widget_from_dialog (myData.dialog);
			cairo_dock_dialog_unreference (myData.dialog);
			myData.dialog = NULL;
			myData.desklet = cd_desklet_new(0, myData.tab, 0, 0);
			gtk_window_set_keep_above(GTK_WINDOW(myData.desklet->pWidget), myConfig.always_on_top);
		}
		if (myData.desklet && ! myConfig.bIsInitiallyDetached)
		{
			myData.tab = cd_desklet_steal_widget_from_desklet (myData.desklet);
			cd_desklet_free(myData.desklet);
			myData.desklet = NULL;
			myData.dialog = cairo_dock_build_dialog (_D("Terminal"), myIcon, myDock, NULL, myData.tab, GTK_BUTTONS_NONE, NULL, NULL, NULL);
		}
		term_tab_apply_settings();
	}
}
CD_APPLET_RELOAD_END
