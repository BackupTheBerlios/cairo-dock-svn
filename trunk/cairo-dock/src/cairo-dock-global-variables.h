#ifndef __CAIRO_DOCK_GLOBAL_VARIABLES_H__
#define __CAIRO_DOCK_GLOBAL_VARIABLES_H__

#include <glib.h>
#include <cairo.h>

extern GList* icons;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern gint g_iCurrentWidth;
extern gint g_iCurrentHeight;

extern gchar *g_cCairoDockModuleDataDir;
extern gchar *g_cDefaultFileBrowser;

extern float g_fMagnitude;

extern gboolean g_bAtBottom;
extern gboolean g_bAtTop;
extern gboolean g_bInside;

extern gint g_iWindowPositionX;
extern gint g_iWindowPositionY;
extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern int g_iIconGap;
extern int g_iLabelSize;
extern gboolean g_bRoundedBottomCorner;
extern gboolean g_bAutoHide;

extern gdouble g_fGradientOffsetX;
extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];

extern int g_iMaxDockWidth;
extern int g_iMaxDockHeight;
extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_iGapX;
extern int g_iGapY;
extern int g_iMaxIconHeight;
extern gchar *g_cCairoDockDataDir;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern double g_fVisibleZoneAlpha;
extern int g_iNbStripes;
extern int g_iMinDockWidth;
extern double g_fAmplitude;
extern int g_iSinusoidWidth;
extern int g_iLabelWeight;
extern int g_iLabelStyle;

extern int g_iSidMoveDown;
extern int g_iSidMoveUp;
extern int g_iSidGrowUp;
extern int g_iSidShrinkDown;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;
extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern GHashTable *g_hAppliTable;
extern GHashTable *g_hXWindowTable;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];

#endif
