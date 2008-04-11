
#ifndef __CAIRO_DOCK_DOCK_FACTORY__
#define  __CAIRO_DOCK_DOCK_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"

/**
* Teste si le container est un dock.
* @param pContainer le container.
* @return TRUE ssi le container a ete declare comme un dock.
*/
#define CAIRO_DOCK_IS_DOCK(pContainer) (pContainer != NULL && pContainer->iType == CAIRO_DOCK_TYPE_DOCK)
/**
* Caste un container en dock.
* @param pContainer le container.
* @return le dock.
*/
#define CAIRO_DOCK_DOCK(pContainer) ((CairoDock *)pContainer)

/**
* Retourne le zoom max des icones contenues dans un conteneur donne.
* @param pContainer le container.
* @return le facteur d'echelle max.
*/
#define cairo_dock_get_max_scale(pContainer) (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK_DOCK (pContainer)->fRatio : 1)

/**
* Cree un nouveau dock principal.
* @param iWmHint indicateur du type de fenetre pour le WM.
* @param cDockName nom du dock, qui pourra etre utilise pour retrouver celui-ci rapidement.
* @param cRendererName nom de la fonction de rendu a applisuer au dock. si NULL, le rendu par defaut sera applique.
* @return le dock nouvellement alloué, a detruire avec #cairo_dock_destroy_dock
*/
CairoDock *cairo_dock_create_new_dock (GdkWindowTypeHint iWmHint, gchar *cDockName, gchar *cRendererName);
/**
* Desactive un dock : le rend inoperant, en detruisant tout ce qu'il contient, sauf sa liste d'icones.
* @param pDock le dock.
*/
void cairo_dock_deactivate_one_dock (CairoDock *pDock);
/**
* Detruit un dock et tout ce qu'il contient, y compris ses icones et lui-meme.
* @param pDock le dock.
*/
void cairo_dock_free_dock (CairoDock *pDock);
/**
* Diminue le nombre d'icones pointant sur un dock de 1. Si aucune icone ne pointe plus sur lui apres ca, le detruit ainsi que tous ses sous-docks, et libere la memoire qui lui etait allouee. Ne fais rien pour le dock principal, utiliser #cairo_dock_free_all_docks pour cela.
* @param pDock le dock a detruire.
* @param cDockName son nom.
* @param ReceivingDock un dock qui recuperera les icones, ou NULL pour detruire toutes les icones contenues dans le dock.
* @param cReceivingDockName le nom du dock qui recuperera les icones, ou NULL si aucun n'est fourni.
*/
void cairo_dock_destroy_dock (CairoDock *pDock, const gchar *cDockName, CairoDock *ReceivingDock, gchar *cReceivingDockName);

/**
* Incremente de 1 la reference d'un dock, c'est-a-dire le nombre d'icones pointant sur ce dock. Si le dock etait auparavant un dock principal, il devient un sous-dock, prenant du meme coup les parametres propres aux sous-docks.
* @param pDock un dock.
*/
void cairo_dock_reference_dock (CairoDock *pDock);

/**
* Cree un nouveau dock de type "sous-dock", et y insere la liste des icones fournie. La liste est appropriee par le dock, et ne doit donc _pas_ etre liberee apres cela. Chaque icone est chargee, et a donc juste besoin d'avoir un nom et un fichier d'image.
* @param pIconList une liste d'icones qui seront entierement chargees et inserees dans le dock.
* @param cDockName le nom desire pour le dock.
* @param iWindowTypeHint indicateur du type de fenetre pour le WM.
* @return le dock nouvellement alloue.
*/
CairoDock *cairo_dock_create_subdock_from_scratch_with_type (GList *pIconList, gchar *cDockName, GdkWindowTypeHint iWindowTypeHint);
#define cairo_dock_create_subdock_from_scratch(pIconList, cDockName) cairo_dock_create_subdock_from_scratch_with_type (pIconList, cDockName, GDK_WINDOW_TYPE_HINT_DOCK)
#define cairo_dock_create_subdock_for_class_appli(cClassName) cairo_dock_create_subdock_from_scratch_with_type (NULL, cClassName, GDK_WINDOW_TYPE_HINT_DOCK)

/**
* Charge un ensemble de fichiers .desktop definissant des icones, et construit l'arborescence des docks.
* Toutes les icones sont creees et placees dans leur conteneur repectif, qui est cree si necessaire. Cette fonction peut tres bien s'utiliser pour 
* A la fin du processus, chaque dock est calcule, et place a la position qui lui est assignee.
* Il faut fournir un dock pour avoir ujn contexte de dessin, car les icones sont crees avant leur conteneur.
* @param pMainDock un dock quelconque.
* @param cDirectory le repertoire contenant les fichiers .desktop.
*/
void cairo_dock_build_docks_tree_with_desktop_files (CairoDock *pMainDock, gchar *cDirectory);

/**
* Detruit tous les docks et toutes les icones contenues dedans, et libere la memoire qui leur etait allouee. Les applets sont stoppees au prealable, ainsi que la barre des taches.
*/
void cairo_dock_free_all_docks (void);



/**
* Recalcule la taille maximale du dock, si par exemple une icone a ete enlevee/rajoutee. Met a jour la taille des decorations si necessaire et adapte a la taille max imposee.
* Le dock est deplace de maniere a rester centre sur la meme position, et les coordonnees des icones des applis sont recalculees et renvoyees au WM.
* @param pDock le dock.
*/
void cairo_dock_update_dock_size (CairoDock *pDock);

/**
* Insere une icone dans le dock, a la position indiquee par le champ /a fOrder.
* Insere un separateur si necessaire, et reserve l'espace correspondant aux nouvelles dimensions du dock si necessaire.
* @param icon l'icone a inserer.
* @param pDock le dock dans lequel l'inserer.
* @param bUpdateSize TRUE pour recalculer la taille du dock apres insertion.
* @param bAnimated TRUE pour regler la taille de l'icone au minimum de facon a la faire grossir apres.
* @param bApplyRatio TRUE pour appliquer le facteur de taille propre au sous-dock.
* @param bInsertSeparator TRUE pour inserer un separateur si necessaire.
*/
void cairo_dock_insert_icon_in_dock (Icon *icon, CairoDock *pDock, gboolean bUpdateSize, gboolean bAnimated, gboolean bApplyRatio, gboolean bInsertSeparator);



/**
* Demande au WM d'empecher les autres fenetres d'empieter sur l'espace du dock.
* L'espace reserve est pris sur la taille minimale du dock, c'est-a-dire la taille de la zone de rappel si l'auto-hide est active,
* ou la taille du dock au repos sinon.
* @param pDock le dock.
* @param bReserve TRUE pour reserver l'espace, FALSE pour annuler la reservation.
*/
void cairo_dock_reserve_space_for_dock (CairoDock *pDock, gboolean bReserve);

/**
* Autorise un widget a accepter les glisse-deposes.
* @param pWidget un widget.
* @param pCallBack la fonction qui sera appelee lors d'une reception de donnee.
* @param data donnees passees en entree de la callback.
*/
void cairo_dock_allow_widget_to_receive_data (GtkWidget *pWidget, GCallback pCallBack, gpointer data);

void cairo_dock_notify_drop_data (gchar *cReceivedData, Icon *pPointedIcon, double fOrder, CairoDockContainer *pContainer);


#endif
