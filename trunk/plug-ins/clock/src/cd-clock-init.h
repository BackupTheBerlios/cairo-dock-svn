
#ifndef __CD_CLOCK_INIT__
#define  __CD_CLOCK_INIT__


#include <cairo-dock.h>


Icon *cd_clock_init (GtkWidget *pWidget, gchar **cConfFilePath, GError **erreur);


void cd_clock_stop (void);


gboolean cd_clock_action (void);


#endif

