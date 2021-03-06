/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <stdlib.h>
#include <glib/gstdio.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-load-icons.h"
#include "applet-stack.h"


CD_APPLET_DEFINITION ("stack",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_DESKTOP,
	N_("This applet allows you to build a stack of files, just like the Stacks applet of MacOS X.\n"
	"To add file into your stacks, you just have to drag and drop it on the Stacks icon and you're done.\n"
	"You can drop any file, or web URL, or even some piece of text\n"
	"You can quickly copy the path/url/text to the clipboard, or open it."),
	"ChAnGFu (Rémy Robertson)")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	cd_stack_check_local (myApplet, CD_APPLET_MY_KEY_FILE);
	cd_stack_build_icons (myApplet);
	
	if (myDock)
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK_FUNC, CAIRO_DOCK_RUN_FIRST, myApplet);  // on se met en premier pour pas que le dock essaye de lancer nos icones, car ce ne sont pas toutes des lanceurs, donc on va le faire nous-memes.
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	
	if (! g_file_test (myApplet->cConfFilePath, G_FILE_TEST_EXISTS) && myConfig.cStackDir)  // on a efface notre instance, on efface donc aussi le repertoire.
	{
		gchar *cCommand = g_strdup_printf ("rm -rf '%s'", myConfig.cStackDir);
		g_print ("Stack : %s\n", myConfig.cStackDir);
		int r = system (cCommand);
		g_free (cCommand);
	}
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myDock)
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		
		cd_stack_build_icons (myApplet);  // pour les mimes.
	}
	else if (myDesklet) {
		CD_APPLET_SET_DESKLET_RENDERER ("Tree");  // on recharge juste les surfaces/textures des icones.
	}
CD_APPLET_RELOAD_END
