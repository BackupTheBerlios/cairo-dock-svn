#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-infopipe.h"

AppletConfig myConfig;
AppletData myData;

CD_APPLET_DEFINITION ("xmms", 1, 4, 7)

static void _load_surfaces (void) {
	gchar *cUserImagePath;
	GString *sImagePath = g_string_new ("");
	//Chargement de l'image "default"
	if (myData.pSurface != NULL) {
		cairo_surface_destroy (myData.pSurface);
	}
	if (myConfig.cDefaultIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cDefaultIcon);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/xmms.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "stop"
	if (myData.pStopSurface != NULL) {
		cairo_surface_destroy (myData.pStopSurface);
	}
	if (myConfig.cStopIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cStopIcon);
		myData.pStopSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/stop.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pStopSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "pause"
	if (myData.pPauseSurface != NULL) {
		cairo_surface_destroy (myData.pPauseSurface);
	}
	if (myConfig.cPauseIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cPauseIcon);
		myData.pPauseSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pPauseSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "play"
	if (myData.pPlaySurface != NULL) {
		cairo_surface_destroy (myData.pPlaySurface);
	}
	if (myConfig.cPlayIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cPlayIcon);
		myData.pPlaySurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/play.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pPlaySurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "broken"
	if (myData.pBrokenSurface != NULL) {
		cairo_surface_destroy (myData.pBrokenSurface);
	}
	if (myConfig.cBrokenIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cBrokenIcon);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	g_string_free (sImagePath, TRUE);
}

CD_APPLET_INIT_BEGIN (erreur)
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	_load_surfaces();
	cd_xmms_update_title();
	g_timeout_add (1000, (GSourceFunc) cd_xmms_read_pipe, (gpointer) NULL);
	
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	
	//\_________________ On libere toutes nos ressources.
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
	  _load_surfaces();
		cd_xmms_update_title();
	}
	else {
		cd_xmms_update_title();
	}
CD_APPLET_RELOAD_END