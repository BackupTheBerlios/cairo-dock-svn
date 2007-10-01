
#ifndef __CAIRO_DOCK_DRAW__
#define  __CAIRO_DOCK_DRAW__

#include <glib.h>

#include "cairo-dock-struct.h"


double cairo_dock_get_current_dock_width (CairoDock *pDock);



cairo_t * cairo_dock_create_context_from_window (CairoDock *pDock);


void cairo_dock_calculate_construction_parameters (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, gboolean bLoop, gboolean bInside);

void cairo_dock_render (CairoDock *pDock);

void cairo_dock_render_background (CairoDock *pDock);

void cairo_dock_render_blank (CairoDock *pDock);



void cairo_dock_redraw_my_icon (Icon *icon, CairoDock *pDock);

void cairo_dock_render_optimized (CairoDock *pDock, GdkRectangle *pArea);


void cairo_dock_hide_parent_docks (CairoDock *pDock);
gboolean cairo_dock_hide_child_docks (CairoDock *pDock);


void cairo_dock_calculate_window_position_at_balance (CairoDock *pDock, CairoDockSizeType iSizeType, int *iNewWidth, int *iNewHeight);

#endif

