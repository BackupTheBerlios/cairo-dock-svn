
#ifndef __CAIRO_DOCK_LAUNCHER_FACTORY__
#define  __CAIRO_DOCK_LAUNCHER_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"


gchar *cairo_dock_search_image_path (gchar *cFileName);


cairo_surface_t *cairo_dock_create_surface_from_image (gchar *cImagePath, cairo_t* pSourceContext, double fMaxScale, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fImageWidth, double *fImageHeight, double fRotationAngle, double fAlpha, gboolean bReapeatAsPattern);


void cairo_dock_load_desktop_file_information (gchar *cDesktopFileName, Icon *icon);


Icon * cairo_dock_create_icon_from_desktop_file (gchar *cDesktopFileName, cairo_t *pSourceContext, gboolean bHorizontalDock);


#endif

