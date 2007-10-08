
#ifndef __CAIRO_DOCK_ICONS__
#define  __CAIRO_DOCK_ICONS__

#include <glib.h>

#include "cairo-dock-struct.h"


#define CAIRO_DOCK_IS_APPLI(icon) (icon != NULL && icon->iType == CAIRO_DOCK_APPLI)
#define CAIRO_DOCK_IS_LAUNCHER(icon) (icon != NULL && icon->iType == CAIRO_DOCK_LAUNCHER)
#define CAIRO_DOCK_IS_SEPARATOR(icon) (icon != NULL && icon->iType & 1)
#define CAIRO_DOCK_IS_APPLET(icon) (icon != NULL && icon->iType  == CAIRO_DOCK_APPLET)

#define CAIRO_DOCK_IS_NORMAL_LAUNCHER(icon) (CAIRO_DOCK_IS_LAUNCHER (icon) && icon->acDesktopFileName != NULL)
#define CAIRO_DOCK_IS_URI_LAUNCHER(icon) (CAIRO_DOCK_IS_LAUNCHER (icon) && icon->cBaseURI != NULL)
#define CAIRO_DOCK_IS_VALID_APPLI(icon) (CAIRO_DOCK_IS_APPLI (icon) && icon->Xid != 0)
#define CAIRO_DOCK_IS_VALID_APPLET(icon) (CAIRO_DOCK_IS_APPLET (icon) && icon->pModule != NULL)


void cairo_dock_free_icon (Icon *icon);

int cairo_dock_compare_icons_order (Icon *icon1, Icon *icon2);


Icon *cairo_dock_get_first_icon (GList *pIconList);
Icon *cairo_dock_get_last_icon (GList *pIconList);
Icon *cairo_dock_get_first_drawn_icon (CairoDock *pDock);
Icon *cairo_dock_get_last_drawn_icon (CairoDock *pDock);
Icon *cairo_dock_get_first_icon_of_type (GList *pIconList, CairoDockIconType iType);
Icon *cairo_dock_get_last_icon_of_type (GList *pIconList, CairoDockIconType iType);
Icon *cairo_dock_get_pointed_icon (GList *pIconList);
Icon *cairo_dock_get_bouncing_icon (GList *pIconList);
Icon *cairo_dock_get_removing_or_inserting_icon (GList *pIconList);
Icon *cairo_dock_get_animated_icon (GList *pIconList);
Icon *cairo_dock_get_next_icon (GList *pIconList, Icon *pIcon);
Icon *cairo_dock_get_previous_icon (GList *pIconList, Icon *pIcon);
#define cairo_dock_get_next_element(ic, list) (ic->next == NULL ? list : ic->next)
#define cairo_dock_get_previous_element(ic, list) (ic->prev == NULL ? g_list_last (list) : ic->prev)
Icon *cairo_dock_get_icon_with_command (GList *pIconList, gchar *cCommand);
Icon *cairo_dock_get_icon_with_base_uri (GList *pIconList, gchar *cBaseURI);
Icon *cairo_dock_get_icon_with_subdock (GList *pIconList, CairoDock *pSubDock);
Icon *cairo_dock_get_icon_with_module (GList *pIconList, CairoDockModule *pModule);
Icon *cairo_dock_get_icon_with_class (GList *pIconList, gchar *cClass);

#define cairo_dock_none_clicked(pIconList) (cairo_dock_get_bouncing_icon (pIconList) == NULL)
#define cairo_dock_none_removed_or_inserted(pIconList) (cairo_dock_get_removing_or_inserting_icon (pIconList) == NULL)
#define cairo_dock_none_animated(pIconList) (cairo_dock_get_animated_icon (pIconList) == NULL)

#define cairo_dock_get_first_launcher(pIconList) cairo_dock_get_first_icon_of_type (pIconList, CAIRO_DOCK_LAUNCHER)
#define cairo_dock_get_last_launcher(pIconList) cairo_dock_get_last_icon_of_type (pIconList, CAIRO_DOCK_LAUNCHER)
#define cairo_dock_get_first_appli(pIconList) cairo_dock_get_first_icon_of_type (pIconList, CAIRO_DOCK_APPLI)
#define cairo_dock_get_last_appli(pIconList) cairo_dock_get_last_icon_of_type (pIconList, CAIRO_DOCK_APPLI)


void cairo_dock_swap_icons (CairoDock *pDock, Icon *icon1, Icon *icon2);
void cairo_dock_move_icon_after_icon (CairoDock *pDock, Icon *icon1, Icon *icon2);

void cairo_dock_detach_icon_from_dock (Icon *icon, CairoDock *pDock, gboolean bCheckUnusedSeparator);
void cairo_dock_remove_one_icon_from_dock (CairoDock *pDock, Icon *icon);
void cairo_dock_remove_icon_from_dock (CairoDock *pDock, Icon *icon);
void cairo_dock_remove_icons_of_type (CairoDock *pDock, CairoDockIconType iType);

void cairo_dock_remove_separator (CairoDock *pDock, CairoDockIconType iType);

void cairo_dock_remove_all_separators (CairoDock *pDock);

void cairo_dock_remove_all_applis (CairoDock *pDock);

void cairo_dock_remove_all_applets (CairoDock *pDock);


GList * cairo_dock_calculate_icons_positions_at_rest (GList *pIconList, int iMinDockWidth, int iXOffset);

Icon * cairo_dock_calculate_icons_with_position (GList *pIconList, GList *pFirstDrawnElement, int x_abs, gdouble fMagnitude, int iMinDockWidth, int iWidth, int iHeight, int iMouseY, double fAlign, double fLateralFactor);

Icon *cairo_dock_calculate_icons (CairoDock *pDock, int iMouseX, int iMouseY);

double cairo_dock_calculate_max_dock_width (CairoDock *pDock, GList *pFirstDrawnElement, int iFlatDockWidth);

void cairo_dock_calculate_max_dock_size_generic (CairoDock *pDock);
void cairo_dock_calculate_max_dock_size_caroussel (CairoDock *pDock);


void cairo_dock_mark_icons_as_avoiding_mouse (CairoDock *pDock, CairoDockIconType iType, double fMargin);
void cairo_dock_stop_marking_icons (CairoDock *pDock, CairoDockIconType iType);

void cairo_dock_update_icon_s_container_name (Icon *icon, gchar *cNewParentDockName);

#endif

