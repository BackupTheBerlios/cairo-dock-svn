#include "cairo-dock-struct.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

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

void cd_xml_open_file(gchar *filePath,gchar *mainNodeName,xmlDocPtr *xmlDoc,xmlNodePtr *node);

Gauge *init_cd_Gauge(cairo_t *pSourceContext, gchar *themeName, int iWidth, int iHeight);
GaugeImage *init_cd_GaugeImage(gchar *sImagePath);

void make_cd_Gauge(cairo_t *pSourceContext, CairoDock *pDock, Icon *pIcon, Gauge *pGauge, double dValue);
void make_cd_Gauge_multiValue(cairo_t *pSourceContext, CairoDock *pDock, Icon *pIcon, Gauge *pGauge, GList *valueList);
void draw_cd_Gauge_needle(cairo_t *pSourceContext, Gauge *pGauge, GaugeIndicator *pGaugeIndicator, double dValue);
void draw_cd_Gauge_image(cairo_t *pSourceContext, Gauge *pGauge, GaugeIndicator *pGaugeIndicator, double dValue);
void draw_cd_GaugeImage(cairo_t *pSourceContext, GaugeImage *pGaugeImage, int iWidth, int iHeight);

void free_cd_GaugeImage(GaugeImage *pGaugeImage);
void free_cd_GaugeIndicator(GaugeIndicator *pGaugeIndicator);
void free_cd_Gauge(Gauge *pGauge);

gchar *cairo_dock_get_gauge_key_value(gchar *cAppletConfFilePath, GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gchar *cDefaultThemeName);
