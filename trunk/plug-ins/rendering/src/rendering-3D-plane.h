
#ifndef __RENDERING_3D_PLANE2_VIEW__
#define  __RENDERING_3D_PLANE2_VIEW__

#include "cairo-dock.h"

#define MY_APPLET_3D_PLANE_VIEW_NAME "3D plane"


typedef enum {
	CD_NORMAL_SEPARATOR = 0,
	CD_FLAT_SEPARATOR,
	CD_PHYSICAL_SEPARATOR,
	CD_NB_SEPARATORS
	} CDSpeparatorType;

void cd_rendering_calculate_max_dock_size_3D_plane (CairoDock *pDock);

void cd_rendering_calculate_construction_parameters_3D_plane (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY);

cairo_surface_t *cd_rendering_create_flat_separator_surface (cairo_t *pSourceContext, int iWidth, int iHeight);


cairo_surface_t *cd_rendering_create_flat_separator_surface (cairo_t *pSourceContext, int iWidth, int iHeight);


void cd_rendering_render_3D_plane (cairo_t *pCairoContext, CairoDock *pDock);


void cd_rendering_render_optimized_3D_plane (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea);


Icon *cd_rendering_calculate_icons_3D_plane (CairoDock *pDock);


void cd_rendering_register_3D_plane_renderer (void);


#endif
