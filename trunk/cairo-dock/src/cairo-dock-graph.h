
#ifndef __CAIRO_DOCK_GRAPH__
#define  __CAIRO_DOCK_GRAPH__

#include <gtk/gtk.h>
G_BEGIN_DECLS

#include <cairo-dock-struct.h>

typedef enum {
	CAIRO_DOCK_GRAPH_LINE=0,
	CAIRO_DOCK_GRAPH_LINE_FILL,
	CAIRO_DOCK_GRAPH_BAR
	} CairoDockTypeGraph;

typedef struct _CairoDockGraph {
	gint iNbValues;
	gdouble *pTabValues;
	gint iCurrentIndex;
	gdouble fHighColor[3];
	gdouble fLowColor[3];
	gdouble fBackGroundColor[4];
	CairoDockTypeGraph iType;
	cairo_surface_t *pBackgroundSurface;
	gdouble fWidth;
	gdouble fHeight;
	gint iRadius;
	gdouble fMargin;
	} CairoDockGraph;


void cairo_dock_draw_graph (cairo_t *pCairoContext, CairoDockGraph *pGraph);

void cairo_dock_update_graph (CairoDockGraph *pGraph, double fNewValue);

void cairo_dock_render_graph (cairo_t *pSourceContext, CairoContainer *pContainer, Icon *pIcon, CairoDockGraph *pGraph);


CairoDockGraph* cairo_dock_create_graph (cairo_t *pSourceContext, int iNbValues, CairoDockTypeGraph iType, double fWidth, double fHeight, gdouble *pLowColor, gdouble *pHighColor, gdouble *pBackGroundColor);

void cairo_dock_reload_graph (cairo_t *pSourceContext, CairoDockGraph *pGraph, int iWidth, int iHeight);

void cairo_dock_free_graph (CairoDockGraph *pGraph);



void cairo_dock_add_watermark_on_graph (cairo_t *pSourceContext, CairoDockGraph *pGraph, gchar *cImagePath, double fAlpha);


G_END_DECLS
#endif
