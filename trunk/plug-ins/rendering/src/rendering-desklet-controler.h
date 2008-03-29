
#ifndef __RENDERING_DESKLET_CONTROLER__
#define  __RENDERING_DESKLET_CONTROLER__

#include "cairo-dock.h"

#define MY_APPLET_CONTROLER_DESKLET_RENDERER_NAME "Controler"


typedef struct {
	gboolean b3D;
	gboolean bCircular;
	double fGapBetweenIcons;
	gint iEllipseHeight;
	gdouble fInclinationOnHorizon;
	gint iFrameHeight;
	gdouble fExtraWidth;
	} CDControlerParameters;


CDControlerParameters *rendering_load_controler (CairoDockDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_free_controler_data (CairoDockDesklet *pDesklet);

void rendering_load_icons_for_controler (CairoDockDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_controler_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_controler_desklet_renderer (void);


#endif
