
#ifndef __CAIRO_DOCK_DIALOGS__
#define  __CAIRO_DOCK_DIALOGS__

#include <glib.h>

#include "cairo-dock-struct.h"

typedef struct 
{
	int iWidth;
	int iHeight;
	int iPositionX;
	int iPositionY;
	cairo_surface_t* pTextBuffer;
	int iTextWidth;
	int iTextHeight;
	gdouble fTextXOffset;
	gdouble fTextYOffset;
	GtkWidget *pWidget;
	} CairoDockDialog;


GtkWidget *cairo_dock_build_dialog (gchar *cText, Icon *pIcon, CairoDock *pDock);


#endif

