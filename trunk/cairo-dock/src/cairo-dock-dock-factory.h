
#ifndef __CAIRO_DOCK_DOCK_FACTORY__
#define  __CAIRO_DOCK_DOCK_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"

void cairo_dock_set_colormap_for_window (GtkWidget *pWidget);

CairoDock *cairo_dock_create_new_dock (int iWmHint, gchar *cDockName);


const gchar *cairo_dock_search_dock_name (CairoDock *pDock);
CairoDock *cairo_dock_search_dock_from_name (gchar *cDockName);
Icon *cairo_dock_search_icon_pointing_on_dock (CairoDock *pDock, CairoDock **pParentDock);
CairoDock *cairo_dock_search_container_from_icon (Icon *icon);


void cairo_dock_reserve_space_for_dock (CairoDock *pDock, gboolean bReserve);
void cairo_dock_update_dock_size (CairoDock *pDock, int iMaxIconHeight, int iMinDockWidth);


void cairo_dock_insert_icon_in_dock (Icon *icon, CairoDock *pDock, gboolean bUpdateSize, gboolean bAnimated, gboolean bApplyRatio);

void cairo_dock_build_docks_tree_with_desktop_files (CairoDock *pMainDock, gchar *cDirectory);


void cairo_dock_free_all_docks (CairoDock *pMainDock);
void cairo_dock_destroy_dock (CairoDock *pDock, gchar *cDockName, CairoDock *ReceivingDock, gchar *cReceivingDockName);


void cairo_dock_reference_dock (CairoDock *pChildDock);

CairoDock *cairo_dock_create_subdock_from_scratch (GList *pIconList, gchar *cDockName);

#endif
