/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.iScrollVariation = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "scroll variation", 5);
	myConfig.fInitialGamma = CD_CONFIG_GET_DOUBLE ("Configuration", "initial gamma");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN	
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myData.pDialog)
	{
		cairo_dock_dialog_unreference (myData.pDialog);  // detruit aussi le widget interactif.
	}
	else
	{
		gtk_widget_destroy (myData.pWidget);
	}
CD_APPLET_RESET_DATA_END
