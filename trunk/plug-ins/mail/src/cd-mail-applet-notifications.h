
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>

CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

CD_APPLET_ON_MIDDLE_CLICK_H

void cd_mail_force_update (void);

void cd_mail_read_folder_data(CDMailAccount *pMailAccount);

void cd_mail_update_status( gpointer *data );

#endif
