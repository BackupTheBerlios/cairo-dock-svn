
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("logout"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_DESKTOP,
	N_("A very simple applet that adds an icon to log out from your session\n"
	"Left click to log out, middle click to shutdown\n"
	"You can invert this order if you prefer to shutdown on left-click."),
	"Fabounet (Fabrice Rey)")


CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME || g_iDesktopEnv == CAIRO_DOCK_XFCE)  // on prend le controle de l'icone de la fenetre.
		CD_APPLET_MANAGE_APPLICATION ("x-session-manager", TRUE);
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	CD_APPLET_MANAGE_APPLICATION ("x-session-manager", FALSE);  // on relache le controle de l'icone de la fenetre.
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	}
CD_APPLET_RELOAD_END
