#include <stdlib.h>
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.quickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
	
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iPlayer = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "current-player", MY_XMMS);
	
	myConfig.enableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.timeDialogs 		= CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "time_dialogs", 3000);
	myConfig.extendedDesklet		= CD_CONFIG_GET_BOOLEAN ("Configuration", "extended_desklet");
	
	myConfig.enableAnim 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_anim");
	myConfig.changeAnimation 	= CD_CONFIG_GET_ANIMATION_WITH_DEFAULT ("Configuration", "change_animation", CAIRO_DOCK_ROTATE);
	
	myConfig.cDefaultIcon 		= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cPlayIcon 			= CD_CONFIG_GET_STRING ("Configuration", "play icon");
	myConfig.cPauseIcon 		= CD_CONFIG_GET_STRING ("Configuration", "pause icon");
	myConfig.cStopIcon 		= CD_CONFIG_GET_STRING ("Configuration", "stop icon");
	myConfig.cBrokenIcon 		= CD_CONFIG_GET_STRING ("Configuration", "broken icon");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	myConfig.defaultTitle = NULL;
	
	g_free (myConfig.cDefaultIcon);
	myConfig.cDefaultIcon = NULL;
	g_free (myConfig.cPlayIcon);
	myConfig.cPlayIcon = NULL;
	g_free (myConfig.cPauseIcon);
	myConfig.cPauseIcon = NULL;
	g_free (myConfig.cStopIcon);
	myConfig.cStopIcon = NULL;
	g_free (myConfig.cBrokenIcon);
	myConfig.cBrokenIcon = NULL;
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_surface_destroy (myData.pSurface);
	myData.pSurface = NULL;
	cairo_surface_destroy (myData.pStopSurface);
	myData.pStopSurface = NULL;
	cairo_surface_destroy (myData.pPlaySurface);
	myData.pPlaySurface = NULL;
	cairo_surface_destroy (myData.pPauseSurface);
	myData.pPauseSurface = NULL;
	cairo_surface_destroy (myData.pBrokenSurface);
	myData.pBrokenSurface = NULL;
	
	if (myData.pipeTimer != 0)
		g_source_remove(myData.pipeTimer);
	
	g_free (myData.playingTitle);
	myData.playingTitle = NULL;
CD_APPLET_RESET_DATA_END
