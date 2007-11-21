#ifndef __CAIRO_DOCK_GLOBAL_VARIABLES_H__
#define __CAIRO_DOCK_GLOBAL_VARIABLES_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <cairo.h>

extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;
extern gchar *g_cLanguage;

extern gint g_iScreenWidth[2];
extern gint g_iScreenHeight[2];

extern gchar *g_cCairoDockModuleDataDir;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern int g_iIconGap;
extern int g_iLabelSize;
extern gboolean g_bRoundedBottomCorner;
extern gboolean g_bAutoHide;

extern double g_fReflectSize;
extern double g_fAlbedo;

extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;

extern GtkIconTheme *g_pIconTheme;
extern gchar *g_cCairoDockDataDir;
extern gchar *g_cCurrentThemePath;
extern gchar *g_cCurrentLaunchersPath;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern double g_fVisibleZoneAlpha;
extern int g_iNbStripes;
extern double g_fAmplitude;
extern int g_iSinusoidWidth;
extern int g_iLabelWeight;
extern int g_iLabelStyle;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;
extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];

#endif
