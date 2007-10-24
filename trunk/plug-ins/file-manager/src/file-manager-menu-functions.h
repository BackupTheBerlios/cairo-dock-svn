
#ifndef __FILE_MANAGER_MENU_FUNC__
#define  __FILE_MANAGER_MENU_FUNC__


#include <cairo-dock.h>


void file_manager_about (GtkMenuItem *menu_item, gpointer *data);


gboolean file_manager_notification_remove_icon (gpointer *data);


gboolean file_manager_notification_build_menu (gpointer *data);


gboolean file_manager_notification_drop_data (gpointer *data);


gboolean file_manager_notification_click_icon (gpointer *data);


#endif
