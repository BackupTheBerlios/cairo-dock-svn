
#ifndef __RENDERING_DESKLET_MEDIAPLAYER__
#define  __RENDERING_DESKLET_MEDIAPLAYER__

#include "cairo.h"

#define MY_APPLET_MEDIAPLAYER_DESKLET_RENDERER_NAME "Mediaplayer"


typedef struct {
	gchar *cArtist;
	gchar *cTitle;
	cairo_surface_t *pArtistSurface;
	cairo_surface_t *pTitleSurface;
	gboolean bControlButton;
	
	gdouble fDeskletWidth;
	
	gdouble fArtistWidth;
	gdouble fArtistHeight;
	gdouble fArtistXOffset;
	gdouble fArtistYOffset;
	
	gdouble fTitleWidth;
	gdouble fTitleHeight;
	gdouble fTitleXOffset;
	gdouble fTitleYOffset;
	
	gint iNbIcons;
	gint iIconsLimit;
	gdouble fBandWidth;
	gdouble fIconBandOffset;
	
	Icon *pClickedIcon;
} CDMediaplayerParameters;


CDMediaplayerParameters *rendering_configure_mediaplayer (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_free_mediaplayer_data (CairoDesklet *pDesklet);

void rendering_load_icons_for_mediaplayer (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_load_mediaplayer_data (CairoDesklet *pDesklet, cairo_t *pSourceContext);

void rendering_draw_mediaplayer_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_mediaplayer_desklet_renderer (void);

void rendering_update_text_for_mediaplayer(CairoDesklet *pDesklet, gpointer *pNewData);

#endif
