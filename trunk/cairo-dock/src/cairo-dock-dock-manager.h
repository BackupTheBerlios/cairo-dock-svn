
#ifndef __CAIRO_DOCK_DOCK_MANAGER__
#define  __CAIRO_DOCK_DOCK_MANAGER__

#include <glib.h>

#include "cairo-dock-struct.h"


/**
* Initialise la classe des docks. N'a aucun effet la 2eme fois.
*/
void cairo_dock_initialize_dock_manager (void);

/**
* Enregistre un dock dans la table des docks.
* @param cDockName nom du dock.
* @param pDock le dock.
* @return le dock portant ce nom s'il en existait deja un, sinon le dock qui a ete insere.
*/
CairoDock *cairo_dock_register_dock (const gchar *cDockName, CairoDock *pDock);
/**
* Desenregistre un dock de la table des docks.
* @param cDockName le nom du dock.
*/
void cairo_dock_unregister_dock (const gchar *cDockName);
/**
* Vide la table des docks, en detruisant tous les docks et leurs icones.
*/
void cairo_dock_reset_docks_table (void);


/**
* Cherche le nom d'un dock, en parcourant la table des docks jusqu'a trouver celui passe en entree.
* @param pDock le dock.
* @return le nom du dock, ou NULL si ce dock n'existe pas.
*/
const gchar *cairo_dock_search_dock_name (CairoDock *pDock);
/**
* Cherche un dock etant donne son nom.
* @param cDockName le nom du dock.
* @return le dock qui a ete enregistre sous ce nom, ou NULL si aucun ne correspond.
*/
CairoDock *cairo_dock_search_dock_from_name (const gchar *cDockName);
/**
* Cherche l'icone pointant sur un dock. Si plusieurs icones pointent sur ce dock, la premiere sera renvoyee.
* @param pDock le dock.
* @param pParentDock si non NULL, sera renseigne avec le dock contenant l'icone.
* @return l'icone pointant sur le dock.
*/
Icon *cairo_dock_search_icon_pointing_on_dock (CairoDock *pDock, CairoDock **pParentDock);
/**
* Cherche le container contenant l'icone donnee, en parcourant la liste des icones de tous les docks jusqu'a trouver celle passee en entree, ou en renvoyant son desklet dans le cas d'une applet.
* @param icon l'icone.
* @return le container contenant cette icone, ou NULL si aucun n'a ete trouve.
*/
CairoDockContainer *cairo_dock_search_container_from_icon (Icon *icon);



/**
* Met a jour un fichier .desktop avec la liste des docks dans le champ "Container".
* @param pKeyFile fichier de conf ouvert.
* @param cDesktopFilePath chemin du fichier de conf.
*/
void cairo_dock_update_conf_file_with_containers (GKeyFile *pKeyFile, gchar *cDesktopFilePath);

void cairo_dock_search_max_decorations_size (int *iWidth, int *iHeight);

/**
*Cache recursivement tous les dock peres d'un dock.
*@param pDock le dock fils.
*/
void cairo_dock_hide_parent_dock (CairoDock *pDock);
/**
*Cache recursivement tous les sous-docks fils d'un dock donne.
*@param pDock le dock parent.
*/
gboolean cairo_dock_hide_child_docks (CairoDock *pDock);
/**
*Recharge les buffers de toutes les icones de tous les docks.
*/
void cairo_dock_reload_buffers_in_all_docks (void);
/**
* Renomme un dock. Met a jour le nom du container de ses icones.
*@param cDockName nom du dock.
*@param pDock le dock.
*@param cNewName son nouveau nom.
*/
void cairo_dock_rename_dock (const gchar *cDockName, CairoDock *pDock, const gchar *cNewName);

void cairo_dock_reset_all_views (void);
void cairo_dock_set_all_views_to_default (void);


#endif
