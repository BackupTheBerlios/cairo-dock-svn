
#ifndef __RENDERING_DIAPO_SIMPLE_VIEW__
#define  __RENDERING_DIAPO_SIMPLE_VIEW__

#include "cairo-dock.h"


void cd_rendering_calculate_max_dock_size_diapo_simple (CairoDock *pDock);


void cd_rendering_render_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock);


Icon *cd_rendering_calculate_icons_diapo_simple (CairoDock *pDock);


void cd_rendering_register_diapo_simple_renderer (const gchar *cRendererName);


guint cairo_dock_rendering_diapo_simple_guess_grid(GList *pIconList, guint *nRowX, guint *nRowY);


Icon *  cairo_dock_calculate_icons_position_for_diapo_simple(CairoDock* pDock, guint nRowsX, guint nRowsY, gint Mx, gint My);


void cairo_dock_calculate_wave_with_position_diapo_simple(GList *pIconList, gint Mx, gint My, guint nRowsX); 


void cairo_dock_rendering_diapo_simple_get_gridXY_from_index(guint nRowsX, guint index, guint* gridX, guint* gridY);


guint cairo_dock_rendering_diapo_simple_get_index_from_gridXY(guint nRowsX, guint gridX, guint gridY);




static void cairo_dock_draw_frame_horizontal_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock);

static void cairo_dock_draw_frame_vertical_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock);

void cairo_dock_draw_frame_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock);

void cairo_dock_render_decorations_in_frame_for_diapo_simple (cairo_t *pCairoContext, CairoDock *pDock, double fAlpha);


void cd_rendering_render_diapo_simple_opengl (CairoDock *pDock);

GLfloat *cd_rendering_generate_path_for_diapo_simple_opengl (CairoDock *pDock, int *iNbPoints);

GLfloat *cd_rendering_generate_path_for_diapo_simple_opengl_without_arrow (CairoDock *pDock, double fAlpha, GLfloat **colors, int *iNbPoints);

GLfloat *cd_rendering_generate_arrow_path_for_diapo_simple_opengl (CairoDock *pDock, double fAlpha, GLfloat *color);


#endif
