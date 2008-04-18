
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "delay");
	if (myConfig.iCheckInterval < 100)  // avant c'etait en ms...
		myConfig.iCheckInterval *= 1000;
	
	myConfig.cInterface = CD_CONFIG_GET_STRING ("Configuration", "interface");
	if (myConfig.cInterface == NULL)
		myConfig.cInterface = g_strdup ("eth0");
	myConfig.iStringLen = strlen (myConfig.cInterface);
	
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");

	//On charge le theme :
	myConfig.cThemePath = cairo_dock_get_gauge_key_value(CD_APPLET_MY_CONF_FILE, pKeyFile, "Configuration", "theme", &bFlushConfFileNeeded, "turbo-night");
	cd_message("gauge (netspeed) : Theme(%s)\n",myConfig.cThemePath);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cThemePath);
	g_free (myConfig.cInterface);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	
	//Adieu la jauge...
	free_cd_Gauge(myData.pGauge);
	
	g_timer_destroy (myData.pClock);
CD_APPLET_RESET_DATA_END

