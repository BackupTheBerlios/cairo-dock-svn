#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (_D("This is the netspeed applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	cd_netspeed(myIcon);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
		CD_APPLET_ADD_SUB_MENU ("netspeed", pSubMenu, CD_APPLET_MY_MENU)
		if (myData.interfaceFound == 0) {
	    		CD_APPLET_ADD_IN_MENU (_D("Waiting"), cd_netspeed_wait, pSubMenu)
	  	}
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)  /// proposer de reverifier la presence d'une 'wireless extension' si no n'en a pas eu a u moment donne.
	
CD_APPLET_ON_BUILD_MENU_END