
#ifndef __CAIRO_DOCK_ANIMATIONS__
#define  __CAIRO_DOCK_ANIMATIONS__

#include <glib.h>

#include "cairo-dock-struct.h"


gboolean cairo_dock_move_up (CairoDock *pDock);

gboolean cairo_dock_move_down (CairoDock *pDock);


gboolean cairo_dock_grow_up (CairoDock *pDock);

gboolean cairo_dock_shrink_down (CairoDock *pDock);


#endif
