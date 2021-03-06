
#ifndef __CAIRO_DOCK_TASKBAR__
#define  __CAIRO_DOCK_TASKBAR__

#include <glib.h>

#include "cairo-dock-struct.h"
#include "cairo-dock-config.h"

typedef struct _CairoConfigTaskBar CairoConfigTaskBar;

#ifndef _INTERNAL_MODULE_
extern CairoConfigTaskBar myTaskBar;
#endif
G_BEGIN_DECLS

struct _CairoConfigTaskBar {
	gboolean bShowAppli;
	///gboolean bUniquePid;
	gboolean bGroupAppliByClass;
	gint iAppliMaxNameLength;
	gboolean bMinimizeOnClick;
	gboolean bCloseAppliOnMiddleClick;
	gboolean bHideVisibleApplis;
	gdouble fVisibleAppliAlpha;
	gboolean bAppliOnCurrentDesktopOnly;
	gboolean bDemandsAttentionWithDialog;
	gint iDialogDuration;
	gchar *cAnimationOnDemandsAttention;
	gchar *cAnimationOnActiveWindow;
	gboolean bOverWriteXIcons;
	gboolean bShowThumbnail;
	gboolean bMixLauncherAppli;
	gchar *cOverwriteException;
	gchar *cGroupException;
	gchar *cForceDemandsAttention;
	gboolean bDrawIndicatorOnAppli;
	} ;


DEFINE_PRE_INIT (TaskBar);

G_END_DECLS
#endif
