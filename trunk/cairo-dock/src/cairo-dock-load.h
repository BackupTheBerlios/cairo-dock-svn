
#ifndef __CAIRO_DOCK_LOAD__
#define  __CAIRO_DOCK_LOAD__

#include <glib.h>

#include "cairo-dock-struct.h"


gchar *cairo_dock_generate_file_path (gchar *cImageFile);

cairo_surface_t *cairo_dock_load_image (cairo_t *pSourceContext, gchar *cImageFile, double *fImageWidth, double *fImageHeight, double fRotationAngle, double fAlpha, gboolean bReapeatAsPattern);
cairo_surface_t *cairo_dock_load_image_for_icon (cairo_t *pSourceContext, gchar *cImageFile, double fImageWidth, double fImageHeight);
#define cairo_dock_load_image_for_square_icon(pSourceContext, cImageFile, fImageSize) cairo_dock_load_image_for_icon (pSourceContext, cImageFile, fImageSize, fImageSize)

void cairo_dock_fill_one_icon_buffer (Icon *icon, cairo_t* pSourceContext, gdouble fMaxScale, gboolean bHorizontalDock, gboolean bApplySizeRestriction);

void cairo_dock_fill_one_text_buffer (Icon *icon, cairo_t* pSourceContext, int iLabelSize, gchar *cLabelPolice, gboolean bHorizontalDock);

void cairo_dock_fill_one_quick_info_buffer (Icon *icon, cairo_t* pSourceContext, int iLabelSize, gchar *cLabelPolice, int iLabelWeight, double fMaxScale);


void cairo_dock_fill_icon_buffers (Icon *icon, cairo_t *pSourceContext, double fMaxScale, gboolean bHorizontalDock, gboolean bApplySizeRestriction);

void cairo_dock_load_one_icon_from_scratch (Icon *pIcon, CairoDock *pDock, CairoDockDesklet *pDesklet);

void cairo_dock_reload_buffers_in_dock (gchar *cDockName, CairoDock *pDock, gpointer data);
#define cairo_dock_load_buffers_in_one_dock(pDock) cairo_dock_reload_buffers_in_dock (NULL, pDock, NULL)
void cairo_dock_reload_buffers_in_all_docks (GHashTable *hDocksTable);



void cairo_dock_load_visible_zone (CairoDock *pDock, gchar *cVisibleZoneImageFile, int iVisibleZoneWidth, int iVisibleZoneHeight, double fVisibleZoneAlpha);

cairo_surface_t *cairo_dock_load_stripes (cairo_t* pSourceContext, int iStripesWidth, int iStripesHeight, double fRotationAngle);

void cairo_dock_update_background_decorations_if_necessary (CairoDock *pDock, int iNewDecorationsWidth, int iNewDecorationsHeight);

void cairo_dock_load_background_decorations (CairoDock *pDock);


#endif
