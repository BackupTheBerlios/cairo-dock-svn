
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include "gtk/gtk.h"
#include "cairo-dock.h"

//\___________ structure containing the applet's configuration parameters.
typedef struct {
	} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
        CairoDockDialog *dialog;
	    GtkWidget *pGtkMozEmbed;
	} AppletData;


#endif