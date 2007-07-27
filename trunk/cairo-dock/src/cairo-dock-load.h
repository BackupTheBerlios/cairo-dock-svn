
#ifndef __CAIRO_DOCK_LOAD__
#define  __CAIRO_DOCK_LOAD__

#include <glib.h>

#include "cairo-dock-struct.h"


void cairo_dock_calculate_contrainted_icon_size (double *fImageWidth, double *fImageHeight, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fIconWidthSaturationFactor, double *fIconHeightSaturationFactor);



void cairo_dock_insert_icon_in_list (Icon *icon, GtkWidget *pWidget, gboolean bUpdateSize, gboolean bAnimated);



void cairo_dock_fill_one_icon_buffer (Icon *icon, cairo_t* pSourceContext, gdouble fMaxScale);

void cairo_dock_fill_one_text_buffer (Icon *icon, cairo_t* pSourceContext, gboolean bUseText, int iLabelSize, gchar *cLabelPolice);

void cairo_dock_reload_buffers (GtkWidget *pWidget, double fMaxScale, int iLabelSize, gboolean bUseText);



void cairo_dock_load_background_image (GtkWindow *pWindow, gchar *image_filename, int image_width, int image_height);



void cairo_dock_init_list_with_desktop_files (GtkWidget *pWidget, gchar *cDirectory);


#endif

