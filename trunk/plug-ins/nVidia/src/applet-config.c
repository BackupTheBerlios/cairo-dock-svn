/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iDrawTemp = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "temp type", 1);
	myConfig.iLowerLimit = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "llt", 50);
	myConfig.iUpperLimit = MAX (myConfig.iLowerLimit+1, CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "ult", 110));
	myConfig.iAlertLimit = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "alt", 100);
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "delay", 10);
	myConfig.bCardName = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "card", TRUE) && (myConfig.iDrawTemp != MY_APPLET_TEMP_ON_NAME);
	myConfig.bAlert = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "alert", TRUE);
	myConfig.cGThemePath = cairo_dock_get_gauge_key_value (CD_APPLET_MY_CONF_FILE, pKeyFile, "Configuration", "theme", &bFlushConfFileNeeded, "radium");
	cd_message ("gauge : Theme '%s'",myConfig.cGThemePath);
	myConfig.cBrokenUserImage = CD_CONFIG_GET_STRING ("Configuration", "broken");
	
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cGThemePath);
	g_free (myConfig.cBrokenUserImage);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	free_cd_Gauge (myData.pGauge);
	g_free (myData.pGPUData.cGPUName);
	g_free (myData.pGPUData.cDriverVersion);
	
CD_APPLET_RESET_DATA_END