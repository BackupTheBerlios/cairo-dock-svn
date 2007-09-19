
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
	int iAimedX;
	int iAimedY;
	gboolean bRight;
	int iGapFromDock;
	cairo_surface_t* pTextBuffer;
	int iTextWidth;
	int iTextHeight;
	gdouble fTextXOffset;
	gdouble fTextYOffset;
	GtkWidget *pWidget;
	int iSidTimer;
	} CairoDockDialog;


void cairo_dock_free_dialog (CairoDockDialog *pDialog);


CairoDockDialog *cairo_dock_build_dialog (gchar *cText, Icon *pIcon, CairoDock *pDock);


void cairo_dock_dialog_calculate_aimed_point (Icon *pIcon, CairoDock *pDock, int *iX, int *iY, gboolean *bRight, int *iGapFromDock);


void cairo_dock_show_temporary_dialog (gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength);

#endif

