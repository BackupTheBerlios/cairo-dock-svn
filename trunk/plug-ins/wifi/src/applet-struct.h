
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *defaultTitle;
	gboolean enableSSQ;
} AppletConfig;

typedef struct {
  cairo_surface_t *pDefault;
	cairo_surface_t *p2Surface;
	cairo_surface_t *p4Surface;
	cairo_surface_t *p6Surface;
	cairo_surface_t *p8Surface;
	cairo_surface_t *p1Surface;
	
	int checkTimer;
} AppletData;


#endif