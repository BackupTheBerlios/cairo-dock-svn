/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include <ctype.h>
#include <stdlib.h>

#include <cairo-dock.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-xgamma.h"
#include "applet-struct.h"
#include "applet-init.h"

static gboolean s_bVideoExtensionChecked = FALSE;


CD_APPLET_DEFINITION ("Xgamma",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_CONTROLER,
	N_("Setup the luminosity of your screen directly from your dock.\n"
	"Scroll up/down to increase/decrease the luminosity\n"
	"Left-click to open a dialog to setup the luminosity\n"
	"Middle-click to set it up for each color.\n"
	"You can also define a luminosity value that will be applied automatically on startup."),
	"Fabounet (Fabrice Rey)")


CD_APPLET_INIT_BEGIN
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	
	if (! s_bVideoExtensionChecked)
	{
		s_bVideoExtensionChecked = TRUE;
		
		const Display *dpy = cairo_dock_get_Xdisplay ();
		if (dpy == NULL)
		{
			cd_warning ("Xgamma : unable to get X display");
			return ;
		}
		
		//#ifdef XF86VidModeQueryVersion
		int MajorVersion, MinorVersion;
		if (!XF86VidModeQueryVersion(dpy, &MajorVersion, &MinorVersion))
		{
			cd_warning ("Xgamma : unable to query video extension version");
			return ;
		}
		//#endif
		
		//#ifdef XF86VidModeQueryExtension
		int EventBase, ErrorBase;
		if (!XF86VidModeQueryExtension(dpy, &EventBase, &ErrorBase))
		{
			cd_warning ("Xgamma : unable to query video extension information");
			return ;
		}
		//#endif
		
		myData.bVideoExtensionOK = TRUE;
		
		if (myConfig.fInitialGamma != 0)
		{
			cd_message ("Applying luminosity as defined in config (gamma=%.2f)...", myConfig.fInitialGamma);
			xgamma_get_gamma (&myData.Xgamma);
			myConfig.fInitialGamma = MIN (GAMMA_MAX, MAX (myConfig.fInitialGamma, GAMMA_MIN));
			myData.Xgamma.red = myConfig.fInitialGamma;
			myData.Xgamma.blue = myConfig.fInitialGamma;
			myData.Xgamma.green = myConfig.fInitialGamma;
			xgamma_set_gamma (&myData.Xgamma);
		}
	}
	
	if (myDesklet)  // on cree le widget pour avoir qqch a afficher dans le desklet.
	{
		xgamma_build_and_show_widget ();
		CD_APPLET_SET_STATIC_DESKLET;
	}
	else
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (! myData.pWidget)
		{
			if (myDesklet)  // on cree le widget pour avoir qqch a afficher dans le desklet.
				xgamma_build_and_show_widget ();
		}
		else if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
		{
			if (myDesklet)  // il faut passer du dialogue au desklet.
			{
				myData.pWidget = cairo_dock_steal_widget_from_its_container (myData.pWidget);
				cairo_dock_dialog_unreference (myData.pDialog);
				myData.pDialog = NULL;
				cairo_dock_add_interactive_widget_to_desklet (myData.pWidget, myDesklet);
				cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
				CD_APPLET_SET_STATIC_DESKLET;
			}
			else  // il faut passer du desklet au dialogue
			{
				myData.pDialog = xgamma_build_dialog ();
				cairo_dock_hide_dialog (myData.pDialog);
			}
		}
	}
	
	if (myDock)
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
CD_APPLET_RELOAD_END
