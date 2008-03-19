
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN
	reset_config ();
	
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
	
	myConfig.cDefault = CD_CONFIG_GET_STRING ("Configuration", "d icon");
	
	myConfig.c20Surface = CD_CONFIG_GET_STRING ("Configuration", "vl icon");
	myConfig.c40Surface = CD_CONFIG_GET_STRING ("Configuration", "l icon");
	myConfig.c60Surface = CD_CONFIG_GET_STRING ("Configuration", "m icon");
	myConfig.c80Surface	= CD_CONFIG_GET_STRING ("Configuration", "g icon");
	myConfig.c100Surface = CD_CONFIG_GET_STRING ("Configuration", "e icon");
	
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "signal_type", 1);
CD_APPLET_CONFIG_END


void reset_config (void) {
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	g_free (myConfig.c20Surface);
	myConfig.c20Surface = NULL;
	g_free (myConfig.c40Surface);
	myConfig.c40Surface = NULL;
	g_free (myConfig.c60Surface);
	myConfig.c60Surface = NULL;
	g_free (myConfig.c80Surface);
	myConfig.c80Surface = NULL;
	g_free (myConfig.c100Surface);
	myConfig.c100Surface = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void) {
	
	g_source_remove (myData.checkTimer);
	myData.checkTimer = 0;
	
	int i;
	for (i = 0; i < WIFI_NB_QUALITY; i ++)
	{
		cairo_surface_destroy (myData.pSurfaces[i]);
		myData.pSurfaces[i] = NULL;
	}
	
	memset (&myData, 0, sizeof (AppletData));
}
