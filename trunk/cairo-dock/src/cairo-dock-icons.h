
#ifndef __CAIRO_DOCK_ICONS__
#define  __CAIRO_DOCK_ICONS__

#include <glib.h>

#include "cairo-dock-struct.h"


#define CAIRO_DOCK_IS_APPLI(icon) (icon->iType == CAIRO_DOCK_APPLI)
#define CAIRO_DOCK_IS_LAUNCHER(icon) (icon->iType == CAIRO_DOCK_LAUNCHER && icon->acDesktopFileName != NULL)
#define CAIRO_DOCK_IS_SEPARATOR(icon) (icon->iType & 1)
#define CAIRO_DOCK_IS_APPLET(icon) (icon->iType  == CAIRO_DOCK_APPLET && icon->pModule != NULL)
#define CAIRO_DOCK_IS_VALID_APPLI(icon) (CAIRO_DOCK_IS_APPLI (icon) && icon->Xid != 0)

void cairo_dock_free_icon (Icon *icon);

int cairo_dock_compare_icons_order (Icon *icon1, Icon *icon2);


Icon* cairo_dock_get_first_icon (GList *pIconList);
Icon* cairo_dock_get_last_icon (GList *pIconList);
Icon* cairo_dock_get_first_icon_of_type (GList *pIconList, CairoDockIconType iType);
Icon* cairo_dock_get_last_icon_of_type (GList *pIconList, CairoDockIconType iType);
Icon* cairo_dock_get_pointed_icon (GList *pIconList);
Icon *cairo_dock_get_bouncing_icon (GList *pIconList);
Icon *cairo_dock_get_removing_or_inserting_icon (GList *pIconList);
Icon *cairo_dock_get_animated_icon (GList *pIconList);
Icon *cairo_dock_get_next_icon (GList *pIconList, Icon *pIcon);
Icon *cairo_dock_get_previous_icon (GList *pIconList, Icon *pIcon);

#define cairo_dock_none_clicked(pIconList) (cairo_dock_get_bouncing_icon (pIconList) == NULL)
#define cairo_dock_none_removed_or_inserted(pIconList) (cairo_dock_get_removing_or_inserting_icon (pIconList) == NULL)
#define cairo_dock_none_animated(pIconList) (cairo_dock_get_animated_icon (pIconList) == NULL)

#define cairo_dock_get_first_launcher(pIconList) cairo_dock_get_first_icon_of_type (pIconList, CAIRO_DOCK_LAUNCHER)
#define cairo_dock_get_last_launcher(pIconList) cairo_dock_get_last_icon_of_type (pIconList, CAIRO_DOCK_LAUNCHER)
#define cairo_dock_get_first_appli(pIconList) cairo_dock_get_first_icon_of_type (pIconList, CAIRO_DOCK_APPLI)
#define cairo_dock_get_last_appli(pIconList) cairo_dock_get_last_icon_of_type (pIconList, CAIRO_DOCK_APPLI)


void cairo_dock_swap_icons (CairoDock *pDock, Icon *icon1, Icon *icon2);
void cairo_dock_move_icon_after_icon (CairoDock *pDock, Icon *icon1, Icon *icon2);


void cairo_dock_remove_one_icon_from_dock (CairoDock *pDock, Icon *icon);
void cairo_dock_remove_icon_from_dock (CairoDock *pDock, Icon *icon);
void cairo_dock_remove_icons_of_type (CairoDock *pDock, CairoDockIconType iType);

void cairo_dock_remove_separator (CairoDock *pDock, CairoDockIconType iType);

void cairo_dock_remove_all_separators (CairoDock *pDock);

void cairo_dock_remove_all_applis (CairoDock *pDock);

void cairo_dock_remove_all_applets (CairoDock *pDock);


Icon * cairo_dock_calculate_icons_with_position (GList *pIconList, int x_abs, gdouble fMagnitude, int iMinDockWidth, int iWidth, int iHeight);

Icon *cairo_dock_calculate_icons (CairoDock *pDock, int iMouseX, int iMouseY);

double cairo_dock_calculate_max_dock_width (GList *pIconList, int iFlatDockWidth);


#endif

