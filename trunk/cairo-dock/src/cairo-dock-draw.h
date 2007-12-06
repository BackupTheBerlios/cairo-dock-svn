
#ifndef __CAIRO_DOCK_DRAW__
#define  __CAIRO_DOCK_DRAW__

#include <glib.h>

#include "cairo-dock-struct.h"


double cairo_dock_get_current_dock_width_linear (CairoDock *pDock);


cairo_t * cairo_dock_create_context_from_window (CairoDock *pDock);


void cairo_dock_draw_frame_horizontal (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination);
void cairo_dock_draw_frame_vertical (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination);
void cairo_dock_draw_frame (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination, gboolean bHorizontal);


void cairo_dock_draw_string (cairo_t *pCairoContext, CairoDock *pDock, double fStringLineWidth, gboolean bIsLoop);


void cairo_dock_render_decorations_in_frame (cairo_t *pCairoContext, CairoDock *pDock, double fOffsetY);




void cairo_dock_manage_animations (Icon *icon, CairoDock *pDock);


void cairo_dock_render_one_icon (Icon *icon, cairo_t *pCairoContext, gboolean bHorizontalDock, double fRatio, double fDockMagnitude, gboolean bUseReflect, int iDecorationsHeight, double fInclinationOnHorizon);
void cairo_dock_render_icons_linear (cairo_t *pCairoContext, CairoDock *pDock, double fRatio);



void cairo_dock_render_background (CairoDock *pDock);

void cairo_dock_render_blank (CairoDock *pDock);



void cairo_dock_redraw_my_icon (Icon *icon, CairoDock *pDock);



void cairo_dock_hide_parent_docks (CairoDock *pDock);
gboolean cairo_dock_hide_child_docks (CairoDock *pDock);


void cairo_dock_set_window_position_at_balance (CairoDock *pDock, int iNewWidth, int iNewHeight);
void cairo_dock_get_window_position_and_geometry_at_balance (CairoDock *pDock, CairoDockSizeType iSizeType, int *iNewWidth, int *iNewHeight);

double cairo_dock_calculate_extra_width_for_trapeze (double fFrameHeight, double fInclination, double fRadius, double fLineWidth);


#endif
