
#ifndef __CAIRO_DOCK_INDICATORS__
#define  __CAIRO_DOCK_INDICATORS__

#include <glib.h>

#include "cairo-dock-struct.h"
#include "cairo-dock-config.h"


typedef struct _CairoConfigIndicators CairoConfigIndicators;
#ifndef _INTERNAL_MODULE_
extern CairoConfigIndicators myIndicators;
#endif
G_BEGIN_DECLS

struct _CairoConfigIndicators {
	// active indicator.
	gchar *cActiveIndicatorImagePath;
	gdouble fActiveColor[4];
	gint iActiveLineWidth;
	gint iActiveCornerRadius;
	gboolean bActiveIndicatorAbove;
	// launched indicator.
	gchar *cIndicatorImagePath;
	gboolean bIndicatorAbove;
	gdouble fIndicatorRatio;
	gboolean bLinkIndicatorWithIcon;
	gint iIndicatorDeltaY;
	gboolean bRotateWithDock;
	// grouped indicator.
	gchar *cClassIndicatorImagePath;
	gboolean bZoomClassIndicator;
	};


DEFINE_PRE_INIT (Indicators);

G_END_DECLS
#endif
