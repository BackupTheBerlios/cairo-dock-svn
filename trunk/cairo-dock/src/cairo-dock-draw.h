
#ifndef __CAIRO_DOCK_DRAW__
#define  __CAIRO_DOCK_DRAW__

#include <glib.h>

#include "cairo-dock-struct.h"


double get_current_dock_width ();

double get_dock_offset_y ();

double get_current_dock_offset_x ();

double get_current_dock_offset_y ();

void cairo_dock_update_dock_size (GtkWidget *pWidget, int iMaxIconHeight, int iMinDockWidth);


cairo_t * cairo_dock_create_context_from_window (GdkWindow* pWindow);

void render (GtkWidget *pWidget);

void cairo_dock_render_background (GtkWidget *pWidget);

void cairo_dock_render_blank (GtkWidget *pWidget);



gboolean grow_up2 (GtkWidget* pWidget);

gboolean shrink_down2 (GtkWidget* pWidget);



void cairo_dock_redraw_my_icon (Icon *icon);

void cairo_dock_render_optimized (GtkWidget *pWidget, GdkRectangle *pArea);


#endif

