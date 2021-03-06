
#ifndef __APPLET_READ_DATA__
#define  __APPLET_READ_DATA__

#include <cairo-dock.h>


gchar *cd_weather_get_location_data (gchar *cLocation);

GList *cd_weather_parse_location_data  (gchar *cDataFilePath, GError **erreur);


void cd_weather_get_distant_data (CairoDockModuleInstance *myApplet);


#endif
