
#ifndef __CAIRO_DOCK_DRAW__
#define  __CAIRO_DOCK_DRAW__

#include <glib.h>

#include "cairo-dock-struct.h"


double get_current_dock_width (GList *pIconList);

double get_current_dock_offset_x (GList *pIconList);

double get_current_dock_offset_y (CairoDock *pDock);



cairo_t * cairo_dock_create_context_from_window (GdkWindow* pWindow);

void render (CairoDock *pDock);

void cairo_dock_render_background (CairoDock *pDock);

void cairo_dock_render_blank (CairoDock *pDock);



gboolean grow_up2 (CairoDock *pDock);

gboolean shrink_down2 (CairoDock *pDock);



void cairo_dock_redraw_my_icon (Icon *icon, GtkWidget *pWidget);

void cairo_dock_render_optimized (CairoDock *pDock, GdkRectangle *pArea);


void cairo_dock_hide_parent_docks (CairoDock *pDock);

#endif

