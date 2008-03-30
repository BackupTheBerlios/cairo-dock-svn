#include "cairo-dock-struct.h"

typedef struct
{
	RsvgHandle *svgNeedle;
	cairo_surface_t *cairoSurface;
	int sizeX;
	int sizeY;
} GaugeImage;

typedef struct
{
	gchar *themeName;
	gchar *themeType;
	int sizeX;
	int sizeY;
	double centerX;
	double centerY;
	double posStart;
	double posStop;
	GaugeImage *imageBackground;
	GaugeImage *imageForeground;
	int nbImage;
	GList *imageList;
} Gauge;

Gauge *init_cd_Gauge(cairo_t *pSourceContext, gchar *themeName, int iWidth, int iHeight);
GaugeImage *init_cd_GaugeImage(gchar *sImagePath);
void make_cd_Gauge(cairo_t *pSourceContext, CairoDock *pDock, Icon *pIcon, Gauge *pGauge, double dValue);

void draw_cd_Gauge_rotate(cairo_t *pSourceContext, Gauge *pGauge, double dValue);
void draw_cd_Gauge_image(cairo_t *pSourceContext, Gauge *pGauge, double dValue);
void draw_cd_GaugeImage(cairo_t *pSourceContext, GaugeImage *pGaugeImage, int iWidth, int iHeight);

void free_cd_Gauge(Gauge *pGauge);
void free_cd_GaugeImage(GaugeImage *pGaugeImage);

gchar *cairo_dock_get_gauge_key_value(gchar *cAppletConfFilePath, GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gchar *cDefaultThemeName);
