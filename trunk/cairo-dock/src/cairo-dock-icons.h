
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

/**
*Libere toutes les ressources liees a une icone, ainsi que cette derniere. Le sous-dock pointee par elle n'est pas dereferencee, cela doit etre fait au prealable.
*@param icon l'icone a liberer.
*/
void cairo_dock_free_icon (Icon *icon);
/**
*Compare 2 icones grace a la relation d'ordre sur le couple (position du type , ordre).
*@param icon1 une icone.
*@param icon2 une autre icone.
*@Returns -1 si icone1 < icone2, 1 si icone1 > icone2, 0 si icone1 = icone2 (au sens de la relation d'ordre).
*/

int cairo_dock_compare_icons_order (Icon *icon1, Icon *icon2);
/**
*Trie une liste en se basant sur la relation d'ordre sur le couple (position du type , ordre).
*@param pIconList la liste d'icones.
*@Returns la liste triee. Les elements sont les memes que ceux de la liste initiale, seul leur ordre a change.
*/
GList *cairo_dock_sort_icons_by_order (GList *pIconList);
/**
*Trie une liste en se basant sur la relation d'ordre alphanumerique sur le nom des icones.
*@param pIconList la liste d'icones.
*@Returns la liste triee. Les elements sont les memes que ceux de la liste initiale, seul leur ordre a change. Les ordres des icones sont mis a jour pour refleter le nouvel ordre global.
*/
GList *cairo_dock_sort_icons_by_name (GList *pIconList);

/**
*Renvoie la 1ere icone d'une liste d'icones.
*@param pIconList la liste d'icones.
*@Returns la 1ere icone, ou NULL si la liste est vide.
*/

Icon *cairo_dock_get_first_icon (GList *pIconList);
/**
*Renvoie la derniere icone d'une liste d'icones.
*@param pIconList la liste d'icones.
*@Returns la derniere icone, ou NULL si la liste est vide.
*/
Icon *cairo_dock_get_last_icon (GList *pIconList);
/**
*Renvoie la 1ere icone a etre dessinee d'une liste d'icones (qui n'est pas forcement la 1ere icone de la liste, si l'utilisateur a scrolle).
*@param pIconList la liste d'icones.
*@Returns la 1ere icone a etre dessinee, ou NULL si la liste est vide.
*/
Icon *cairo_dock_get_first_drawn_icon (CairoDock *pDock);
/**
*Renvoie la derniere icone a etre dessinee d'une liste d'icones (qui n'est pas forcement la derniere icone de la liste, si l'utilisateur a scrolle).
*@param pIconList la liste d'icones.
*@Returns la derniere icone a etre dessinee, ou NULL si la liste est vide.
*/
Icon *cairo_dock_get_last_drawn_icon (CairoDock *pDock);
/**
*Renvoie la 1ere icone du type donne.
*@param pIconList la liste d'icones.
*@param iType le type d'icone recherche.
*@Returns la 1ere icone trouvee ayant ce type, ou NULL si aucune icone n'est trouvee.
*/
Icon *cairo_dock_get_first_icon_of_type (GList *pIconList, CairoDockIconType iType);
/**
*Renvoie la derniere icone du type donne.
*@param pIconList la liste d'icones.
*@param iType le type d'icone recherche.
*@Returns la derniere icone trouvee ayant ce type, ou NULL si aucune icone n'est trouvee.
*/
Icon *cairo_dock_get_last_icon_of_type (GList *pIconList, CairoDockIconType iType);
/**
*Renvoie l'icone actuellement pointee parmi une liste d'icones.
*@param pIconList la liste d'icones.
*@Returns l'icone dont le champ bPointed a TRUE, ou NULL si aucune icone n'est pointee.
*/
Icon *cairo_dock_get_pointed_icon (GList *pIconList);
/**
*Renvoie l'icone actuellement en cours d'animation due a un clique parmi une liste d'icones.
*@param pIconList la liste d'icones.
*@Returns la 1ere icone dont le champ iCount est > 0 ou NULL si aucune icone n'est en cours d'animation.
*/
Icon *cairo_dock_get_bouncing_icon (GList *pIconList);
/**
*Renvoie l'icone actuellement en cours d'insertion ou de suppression parmi une liste d'icones.
*@param pIconList la liste d'icones.
*@Returns la 1ere icone dont le champ fPersonnalScale est non nul ou NULL si aucune icone n'est en cours d'insertion / suppression.
*/
Icon *cairo_dock_get_removing_or_inserting_icon (GList *pIconList);
/**
*Renvoie l'icone actuellement en cours d'animation (due a un clique ou a une insertion / suppression) parmi une liste d'icones.
*@param pIconList la liste d'icones.
*@Returns la 1ere icone dont le champ iCount est > 0 ou le champ fPersonnalScale est non nul, ou NULL si aucune icone n'est en cours d'animation.
*/
Icon *cairo_dock_get_animated_icon (GList *pIconList);
Icon *cairo_dock_get_removing_or_inserting_icon (GList *pIconList);
/**
*Renvoie l'icone suivante dans la liste d'icones. Cout en O(n).
*@param pIconList la liste d'icones.
*@Returns l'icone dont le voisin de gauche est pIcon, ou NULL si pIcon est la derniere icone de la liste.
*/
Icon *cairo_dock_get_next_icon (GList *pIconList, Icon *pIcon);
/**
*Renvoie l'icone precedente dans la liste d'icones. Cout en O(n).
*@param pIconList la liste d'icones.
*@Returns l'icone dont le voisin de droite est pIcon, ou NULL si pIcon est la 1ere icone de la liste.
*/
Icon *cairo_dock_get_previous_icon (GList *pIconList, Icon *pIcon);
/**
*Renvoie le prochain element dans la liste, en bouclant si necessaire.
*@param ic l'element courant.
*@param list la liste d'icones.
*@Returns l'element suivant de la liste bouclee.
*/
#define cairo_dock_get_next_element(ic, list) (ic->next == NULL ? list : ic->next)
/**
*Renvoie l'element precedent dans la liste, en bouclant si necessaire.
*@param ic l'element courant.
*@param list la liste d'icones.
*@Returns l'element precedent de la liste bouclee.
*/
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
#define cairo_dock_remove_all_applis(pDock) cairo_dock_remove_icons_of_type (pDock, CAIRO_DOCK_APPLI)
#define cairo_dock_remove_all_applets(pDock) cairo_dock_remove_icons_of_type (pDock, CAIRO_DOCK_APPLET)

void cairo_dock_remove_separator (CairoDock *pDock, CairoDockIconType iType);

void cairo_dock_remove_all_separators (CairoDock *pDock);



GList * cairo_dock_calculate_icons_positions_at_rest_linear (GList *pIconList, int iMinDockWidth, int iXOffset);

Icon * cairo_dock_calculate_wave_with_position_linear (GList *pIconList, GList *pFirstDrawnElement, int x_abs, gdouble fMagnitude, int iMinDockWidth, int iWidth, int iHeight, double fAlign, double fLateralFactor);

Icon *cairo_dock_apply_wave_effect (CairoDock *pDock);

CairoDockMousePositionType cairo_dock_check_if_mouse_inside_linear (CairoDock *pDock);

void cairo_dock_manage_mouse_position (CairoDock *pDock, CairoDockMousePositionType iMousePositionType);


double cairo_dock_calculate_max_dock_width (CairoDock *pDock, GList *pFirstDrawnElement, int iFlatDockWidth, double fWidthConstraintFactor, double fExtraWidth);


void cairo_dock_mark_icons_as_avoiding_mouse (CairoDock *pDock, CairoDockIconType iType, double fMargin);
void cairo_dock_mark_avoiding_mouse_icons_linear (CairoDock *pDock);
void cairo_dock_stop_marking_icons (CairoDock *pDock);


void cairo_dock_update_icon_s_container_name (Icon *icon, gchar *cNewParentDockName);

#endif

