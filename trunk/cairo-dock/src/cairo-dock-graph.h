
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
	} CairoDockGraph;


void cairo_dock_draw_graph (cairo_t *pCairoContext, CairoDockGraph *pGraph, double fWidth, double fHeight);


CairoDockGraph* cairo_dock_create_graph (int iNbValues, CairoDockTypeGraph iType, gdouble *fHighColor, gdouble *fLowColor, gdouble *fBackGroundColor);

void cairo_dock_free_graph (CairoDockGraph *pGraph);


void cairo_dock_update_graph (CairoDockGraph *pGraph, double fNewValue);


G_END_DECLS
#endif
