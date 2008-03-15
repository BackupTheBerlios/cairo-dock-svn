
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib.h>

#include "mailwatch.h"
#include "cairo-dock.h"

typedef struct {
    gchar *cNoMailUserImage;
    gchar *cHasMailUserImage;
	} AppletConfig;

typedef struct {
	XfceMailwatch *mailwatch;

	cairo_surface_t *pNoMailSurface;
	cairo_surface_t *pHasMailSurface;
	guint iNbUnreadMails;
	} AppletData;


#endif