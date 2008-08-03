
#ifndef __CAIRO_DOCK_GAUGE__
#define  __CAIRO_DOCK_GAUGE__

#include "cairo-dock-struct.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
G_BEGIN_DECLS

typedef struct
{
	RsvgHandle *svgNeedle;
	cairo_surface_t *cairoSurface;
	int sizeX;
	int sizeY;
} GaugeImage;

typedef struct
{
	double posX;
	double posY;
	double posStart;
	double posStop;
	double direction;
	int nbImage;
	GList *imageList;
	GList *imageNeedle;
} GaugeIndicator;

typedef struct
{
	gchar *themeName;
	int sizeX;
	int sizeY;
	GaugeImage *imageBackground;
	GaugeImage *imageForeground;
	GList *indicatorList;
} Gauge;

void cairo_dock_xml_open_file (gchar *filePath, const gchar *mainNodeName,xmlDocPtr *xmlDoc,xmlNodePtr *node);

Gauge *cairo_dock_load_gauge(cairo_t *pSourceContext, gchar *themeName, int iWidth, int iHeight);
GaugeImage *cairo_dock_init_gauge_image (gchar *cImagePath);
void cairo_dock_load_gauge_image (cairo_t *pSourceContext, GaugeImage *pGaugeImage, int iWidth, int iHeight);

void cairo_dock_render_gauge(cairo_t *pSourceContext, CairoContainer *pContainer, Icon *pIcon, Gauge *pGauge, double fValue);
void cairo_dock_render_gauge_multi_value(cairo_t *pSourceContext, CairoContainer *pContainer, Icon *pIcon, Gauge *pGauge, GList *valueList);
void draw_cd_Gauge_needle(cairo_t *pSourceContext, Gauge *pGauge, GaugeIndicator *pGaugeIndicator, double fValue);
void draw_cd_Gauge_image(cairo_t *pSourceContext, GaugeIndicator *pGaugeIndicator, double fValue);

void cairo_dock_reload_gauge (cairo_t *pSourceContext, Gauge *pGauge, int iWidth, int iHeight);

void cairo_dock_free_gauge(Gauge *pGauge);

gchar *cairo_dock_get_gauge_key_value(gchar *cAppletConfFilePath, GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gchar *cDefaultThemeName);


G_END_DECLS
#endif
