
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


Icon* cairo_dock_get_first_icon (void);
Icon* cairo_dock_get_last_icon (void);
Icon* cairo_dock_get_first_icon_of_type (CairoDockIconType iType);
Icon* cairo_dock_get_last_icon_of_type (CairoDockIconType iType);
Icon* cairo_dock_get_pointed_icon (void);
Icon *cairo_dock_get_bouncing_icon (void);
Icon *cairo_dock_get_removing_or_inserting_icon (void);

#define get_first_icon cairo_dock_get_first_icon
#define get_last_icon cairo_dock_get_last_icon
#define cairo_dock_none_clicked (cairo_dock_get_bouncing_icon () == NULL)
#define none_clicked2 cairo_dock_none_clicked
#define cairo_dock_none_removed_or_inserted (cairo_dock_get_removing_or_inserting_icon () == NULL)

#define cairo_dock_get_first_launcher(...) cairo_dock_get_first_icon_of_type (CAIRO_DOCK_LAUNCHER)
#define cairo_dock_get_last_launcher(...) cairo_dock_get_last_icon_of_type (CAIRO_DOCK_LAUNCHER)
#define cairo_dock_get_first_appli(...) cairo_dock_get_first_icon_of_type (CAIRO_DOCK_APPLI)
#define cairo_dock_get_last_appli(...) cairo_dock_get_last_icon_of_type (CAIRO_DOCK_APPLI)


void cairo_dock_swap_icons (Icon *icon1, Icon *icon2);


void cairo_dock_remove_one_icon_from_dock (Icon *icon);
void cairo_dock_remove_icon_from_dock (Icon *icon);
void cairo_dock_remove_icons_of_type (CairoDockIconType iType);

void cairo_dock_remove_separator (CairoDockIconType iType);

void cairo_dock_remove_all_separators (void);

void cairo_dock_remove_all_applis (GtkWidget *pWidget);

void cairo_dock_remove_all_applets (GtkWidget *pWidget);


Icon * cairo_dock_calculate_icons_with_position (int x_abs, gdouble fMagnitude, int iWidth, int iHeight);

void cairo_dock_calculate_icons (GtkWidget* pWidget, gdouble fMagnitude);

double cairo_dock_calculate_max_dock_width (int iFlatDockWidth);

#endif

