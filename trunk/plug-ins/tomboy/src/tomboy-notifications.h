#ifndef __TOMBOY_NOTIFICATIONS__
#define  __TOMBOY_NOTIFICATIONS__

#include <cairo-dock.h>


CD_APPLET_ON_CLICK_H
CD_APPLET_ON_BUILD_MENU_H
CD_APPLET_ON_MIDDLE_CLICK_H

gboolean cd_tomboy_on_change_icon (gpointer pUserData, Icon *pPointedIcon, CairoDock *pDock, gboolean *bStartAnimation);


#endif
