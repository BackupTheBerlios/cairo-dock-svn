/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
** Login : <chris.chapuis@gmail.com>
** Started on  Thu Apr 03 18:21:35 2008 CHAPUIS Christophe
** $Id$
**
** Author(s):
**  - Christophe CHAPUIS <chris.chapuis@gmail.com>
**
** Copyright (C) 2008 CHAPUIS Christophe
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

#include "applet-struct.h"
#include "applet-widget.h"

#include <gtk/gtk.h>



void _cd_weblets_set_crop_position (CairoDockModuleInstance *myApplet)
{
	GtkAdjustment *pGtkAdjustmentH = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW (myData.pGtkMozEmbed));
	gtk_adjustment_set_value(pGtkAdjustmentH, 0);
	gtk_adjustment_set_value(pGtkAdjustmentH, myConfig.iPosScrollX);
	GtkAdjustment *pGtkAdjustmentV = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW (myData.pGtkMozEmbed));
	gtk_adjustment_set_value(pGtkAdjustmentV, 0);
	gtk_adjustment_set_value(pGtkAdjustmentV, myConfig.iPosScrollY);
}

// hide/show the scrollbars
static void show_hide_scrollbars(CairoDockModuleInstance *myApplet)
{
	// First, set the position
	_cd_weblets_set_crop_position (myApplet);
	
	// Then, hide or show the scrollbars
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (myData.pGtkMozEmbed), myConfig.bShowScrollbars?GTK_POLICY_AUTOMATIC:GTK_POLICY_NEVER, myConfig.bShowScrollbars?GTK_POLICY_AUTOMATIC:GTK_POLICY_NEVER);
}


CairoDialog *cd_weblets_build_dialog(CairoDockModuleInstance *myApplet)
{
	CairoDialogAttribute attr;
	memset (&attr, 0, sizeof (CairoDialogAttribute));
	attr.cText = D_ ("Weblets");
	attr.pInteractiveWidget = myData.pGtkMozEmbed;
	return cairo_dock_build_dialog (&attr, myIcon, myContainer);
}

/* Will be called when loading of the page is finished*/
void load_finished_cb(WebKitWebView *pWebKitView, WebKitWebFrame* widget
, CairoDockModuleInstance *myApplet)
{
	g_print ("weblets : (re)load finished\n");
	// update scrollbars status
	show_hide_scrollbars(myApplet);
}

/* Build the embedded widget */
void weblet_build_and_show(CairoDockModuleInstance *myApplet)
{
	myData.pGtkMozEmbed = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (myData.pGtkMozEmbed), myConfig.bShowScrollbars?GTK_POLICY_AUTOMATIC:GTK_POLICY_NEVER, myConfig.bShowScrollbars?GTK_POLICY_AUTOMATIC:GTK_POLICY_NEVER);
	
	myData.pWebKitView = WEBKIT_WEB_VIEW (webkit_web_view_new ());
	gtk_container_add (GTK_CONTAINER (myData.pGtkMozEmbed), GTK_WIDGET (myData.pWebKitView));
	gtk_signal_connect(GTK_OBJECT(myData.pWebKitView),
		"load_finished",
		GTK_SIGNAL_FUNC(load_finished_cb),
		myApplet);
	gtk_widget_show_all (myData.pGtkMozEmbed);
				 					 
	if (myDock)
	{
		gtk_widget_set (GTK_WIDGET (myData.pWebKitView), "width-request", 600, "height-request", 400, NULL);
		myData.dialog = cd_weblets_build_dialog(myApplet);
	}
	else
	{
		cairo_dock_add_interactive_widget_to_desklet_full (myData.pGtkMozEmbed, myDesklet, myConfig.iRightMargin);
		cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
}

gboolean cd_weblets_refresh_page (CairoDockModuleInstance *myApplet)
{
	cd_message( "weblets: refreshing page.\n" );

	// load the page
	if(myData.pGtkMozEmbed)
	{
		cd_message( " >> weblets: refresh !\n" );
		if (myConfig.cURI_to_load == NULL)
		{
			g_free (myConfig.cURI_to_load);
			myConfig.cURI_to_load = g_strdup ("http://www.google.com");
		}
		else
		{
			if (g_strstr_len (myConfig.cURI_to_load, -1, "://") == NULL)  // pas de protocole defini, on prend http par defaut.
			{
				gchar *tmp = myConfig.cURI_to_load;
				myConfig.cURI_to_load = g_strconcat ("http://", (strncmp (myConfig.cURI_to_load, "www.", 4) ? "www." : ""), myConfig.cURI_to_load, NULL);
				g_free (tmp);
			}
		}
		
		webkit_web_view_open(WEBKIT_WEB_VIEW(myData.pWebKitView), myConfig.cURI_to_load?myConfig.cURI_to_load:"http://www.google.com");
	}
	/* available since rev. 30985, from fev. 2008 */
	webkit_web_view_set_transparent(myData.pWebKitView, myConfig.bIsTransparent);

	return TRUE;
}

