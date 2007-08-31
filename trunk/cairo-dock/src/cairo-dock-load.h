
#ifndef __CAIRO_DOCK_LOAD__
#define  __CAIRO_DOCK_LOAD__

#include <glib.h>

#include "cairo-dock-struct.h"


void cairo_dock_calculate_contrainted_icon_size (double *fImageWidth, double *fImageHeight, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fIconWidthSaturationFactor, double *fIconHeightSaturationFactor);



void cairo_dock_fill_one_icon_buffer (Icon *icon, cairo_t* pSourceContext, gdouble fMaxScale, gboolean bHorizontalDock);

void cairo_dock_fill_one_text_buffer (Icon *icon, cairo_t* pSourceContext, int iLabelSize, gchar *cLabelPolice, gboolean bHorizontalDock);

void cairo_dock_reload_buffers_in_all_dock (GHashTable *hDocksTable, double fMaxScale, int iLabelSize, gchar *cLabelPolice);


cairo_surface_t *cairo_dock_load_image (cairo_t *pSourceContext, gchar *cImageFile, double *fImageWidth, double *fImageHeight, double fRotationAngle, double fAlpha, gboolean bReapeatAsPattern);

void cairo_dock_load_visible_zone (CairoDock *pDock, gchar *cVisibleZoneImageFile, int iVisibleZoneWidth, int iVisibleZoneHeight, double fVisibleZoneAlpha);

cairo_surface_t *cairo_dock_load_stripes (cairo_t* pSourceContext, int iStripesWidth, int iStripesHeight, double fRotationAngle);

void cairo_dock_update_background_decorations_if_necessary (CairoDock *pDock, int iNewMaxDockWidth, int iNewMaxIconHeight, double fRotationAngle);

void cairo_dock_load_background_decorations (CairoDock *pDock);


#endif

