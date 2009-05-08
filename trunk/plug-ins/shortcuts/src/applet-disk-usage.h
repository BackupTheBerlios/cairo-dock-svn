
#ifndef __APPLET_DISK_USAGE__
#define  __APPLET_DISK_USAGE__


#include <cairo-dock.h>


void cd_shortcuts_get_disk_usage (CairoDockModuleInstance *myApplet);

gboolean cd_shortcuts_update_disk_usage (CairoDockModuleInstance *myApplet);


void cd_shortcuts_stop_disk_measure (CairoDockModuleInstance *myApplet);

void cd_shortcuts_launch_disk_measure (CairoDockModuleInstance *myApplet);


gchar *cd_shortcuts_get_fs_type (const gchar *cMountPath);


#endif