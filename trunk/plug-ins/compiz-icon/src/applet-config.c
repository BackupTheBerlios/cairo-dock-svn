/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	
	myConfig.iWM = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "WM", COMPIZ_FUSION);
	myConfig.lBinding = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "binding", FALSE);
	myConfig.iRendering = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "irendering", FALSE);
	myConfig.selfDecorator = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "decorator", FALSE);
	myConfig.protectDecorator = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "protect", TRUE);
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_KEY_FILE, CD_APPLET_MY_CONF_FILE, "Configuration", "renderer");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cRenderer);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myIcon->pSubDock != NULL) {
		cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
		myIcon->pSubDock = NULL;
	}
	
CD_APPLET_RESET_DATA_END
