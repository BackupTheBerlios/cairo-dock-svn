
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <gtk/gtk.h>
#include <cairo-dock.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <webkit/webkit.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	   gchar *cURI_to_load;
	   gboolean bShowScrollbars;
	   gboolean bIsTransparent;
	   gint iPosScrollX;
	   gint iPosScrollY;
	   guint iReloadTimeout;
	   gchar **cListURI;
	   gint iRightMargin;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
      CairoDialog *dialog;
	    GtkWidget *pGtkMozEmbed;
	    WebKitWebView *pWebKitView;
	    CairoDockTask *pRefreshTimer;
	} ;


#endif
