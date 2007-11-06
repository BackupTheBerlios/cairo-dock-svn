
#ifndef __CAIRO_DOCK_DIALOGS__
#define  __CAIRO_DOCK_DIALOGS__

#include <glib.h>

#include "cairo-dock-struct.h"


gboolean cairo_dock_dialog_reference (Icon *pIcon);
void cairo_dock_dialog_unreference (Icon *pIcon);

CairoDockDialog *cairo_dock_isolate_dialog (Icon *pIcon);
void cairo_dock_free_dialog (CairoDockDialog *pDialog);


CairoDockDialog *cairo_dock_build_dialog (gchar *cText, Icon *pIcon, CairoDock *pDock);


void cairo_dock_dialog_calculate_aimed_point (Icon *pIcon, CairoDock *pDock, int *iX, int *iY, gboolean *bRight, gboolean *bIsPerpendicular, gboolean *bDirectionUp);

void cairo_dock_dialog_find_optimal_placement  (CairoDockDialog *pDialog, Icon *pIcon, CairoDock *pDock);

void cairo_dock_place_dialog (CairoDockDialog *pDialog, Icon *pIcon, CairoDock *pDock);


void cairo_dock_show_temporary_dialog (gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength);

void cairo_dock_replace_all_dialogs (void);

void cairo_dock_remove_dialog_if_any (Icon *icon);


#endif
