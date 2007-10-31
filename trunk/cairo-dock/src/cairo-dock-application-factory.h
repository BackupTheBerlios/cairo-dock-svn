
#ifndef __CAIRO_DOCK_APPLICATION_FACTORY__
#define  __CAIRO_DOCK_APPLICATION_FACTORY__

#include <glib.h>
#include <X11/Xlib.h>

#include "cairo-dock-struct.h"


void cairo_dock_initialize_application_factory (Display *pXDisplay);

void cairo_dock_unregister_pid (Icon *icon);


cairo_surface_t *cairo_dock_create_surface_from_xwindow (Window Xid, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);


Icon * cairo_dock_create_icon_from_xwindow (cairo_t *pSourceContext, Window Xid, CairoDock *pDock);


#endif
