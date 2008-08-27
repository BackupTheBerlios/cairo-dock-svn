
#ifndef __CAIRO_DOCK_DARW_OPENGL__
#define  __CAIRO_DOCK_DARW_OPENGL__

#include <glib.h>

#include "cairo-dock-struct.h"

void cairo_dock_render_one_icon_opengl (Icon *icon, CairoDock *pDock, double fRatio, double fDockMagnitude, gboolean bUseText);

GLuint cairo_dock_create_texture_from_surface (cairo_surface_t *pImageSurface);

void cairo_dock_init_capsule_display (GLuint iChromeTexture);

void cairo_dock_init_square_display (void);

void cairo_dock_init_cube_display (void);



#endif
