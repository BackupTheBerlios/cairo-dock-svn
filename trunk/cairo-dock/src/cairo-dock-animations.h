
#ifndef __CAIRO_DOCK_ANIMATIONS__
#define  __CAIRO_DOCK_ANIMATIONS__

#include <glib.h>

#include "cairo-dock-struct.h"


gboolean cairo_dock_move_up (CairoDock *pDock);

gboolean cairo_dock_move_down (CairoDock *pDock);


gfloat cairo_dock_calculate_magnitude (gint iMagnitudeIndex);

gboolean cairo_dock_grow_up (CairoDock *pDock);

gboolean cairo_dock_shrink_down (CairoDock *pDock);


void cairo_dock_arm_animation (Icon *icon, CairoDockAnimationType iAnimationType, int iNbRounds);
void cairo_dock_start_animation (Icon *icon, CairoDock *pDock);

#endif
