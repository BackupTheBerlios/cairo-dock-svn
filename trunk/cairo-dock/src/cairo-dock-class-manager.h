
#ifndef __CAIRO_DOCK_CLASS_MANAGER__
#define  __CAIRO_DOCK_CLASS_MANAGER__

#include <X11/Xlib.h>

#include "cairo-dock-struct.h"

/**
* Initialise le gestionnaire de classes. Ne fait rien la 2eme fois.
*/
void cairo_dock_initialize_class_manager (void);

/**
* Libere une classe d'appli, en enlevant au passage tous les indicateurs des inhibiteurs de cette classe.
* @param pClassAppli la classe d'appli.
*/
void cairo_dock_free_class_appli (CairoDockClassAppli *pClassAppli);
/**
* Fournit la liste de toutes les applis connues du dock appartenant a cette classe.
* @param cClass la classe.
* @return la liste des applis de cettte classe.
*/
const GList *cairo_dock_list_existing_appli_with_class (const gchar *cClass);

/**
* Enregistre une icone d'appli dans sa classe. Ne fais rien aux inhibiteurs.
* @param pIcon l'icone de l'appli.
* @return TRUE si l'enregistrement s'est effectue correctement ou si l'appli etait deja enregistree, FALSE sinon.
*/
gboolean cairo_dock_add_appli_to_class (Icon *pIcon);
/**
* Desenregistre une icone d'appli de sa classe. Ne fais rien aux inhibiteurs.
* @param pIcon l'icone de l'appli.
* @return TRUE si le desenregistrement s'est effectue correctement ou si elle n'etait pas enregistree, FALSE sinon.
*/
gboolean cairo_dock_remove_appli_from_class (Icon *pIcon);
/**
* Force les applis d'une classe a utiliser ou non les icones fournies par X. Dans le cas ou elles ne les utilisent pas, elle utiliseront les memes icones que leur lanceur correspondant s'il existe. Recharge leur buffer en consequence, mais ne force pas le redessin.
* @param cClass la classe.
* @param bUseXIcon TRUE pour utiliser l'icone fournie par X, FALSE sinon.
* @return TRUE si l'etat a change, FALSE sinon.
*/
gboolean cairo_dock_set_class_use_xicon (const gchar *cClass, gboolean bUseXIcon);
/**
* Ajoute un inhibiteur a une classe, et lui fait prendre immediatement le controle de la 1ere appli de cette classe trouvee, la detachant du dock. Rajoute l'indicateur si necessaire, et redessine le dock d'ou l'appli a ete enlevee, mais ne redessine pas l'icone inhibitrice.
* @param cClass la classe.
* @param pInhibatorIcon l'inhibiteur.
* @return TRUE si l'inhibiteur a bien ete rajoute a la classe.
*/
gboolean cairo_dock_inhibate_class (const gchar *cClass, Icon *pInhibatorIcon);

/**
* Dis si une classe donnee est inhibee par un inhibiteur, libre ou non.
* @return TRUE ssi les applis de cette classe sont inhibees.
*/
gboolean cairo_dock_class_is_inhibated (const gchar *cClass);
/**
* Dis si une classe donnee utilise les icones fournies par X.
* @return TRUE ssi les applis de cette classe utilisent les icones de X.
*/
gboolean cairo_dock_class_is_using_xicon (const gchar *cClass);
/**
* Dis si une appli doit etre inhibee ou pas. Si un inhibiteur libre a ete trouve, il en prendra le controle, et TRUE sera renvoye. Un indicateur lui sera rajoute (ainsi qu'a l'icone du sous-dock si necessaire), et la geometrie de l'icone pour le WM lui est mise, mais il ne sera pas redessine. Dans le cas contraire, FALSE sera renvoye, et l'appli pourra etre inseree dans le dock.
* @param pIcon l'icone d'appli.
* @return TRUE si l'appli a ete inhibee.
*/
gboolean cairo_dock_prevent_inhibated_class (Icon *pIcon);


gboolean cairo_dock_remove_icon_from_class (Icon *pInhibatorIcon);
void cairo_dock_deinhibate_class (const gchar *cClass, Icon *pInhibatorIcon);
void cairo_dock_update_Xid_on_inhibators (Window Xid, const gchar *cClass);
void cairo_dock_remove_all_applis_from_class_table (void);
void cairo_dock_reset_class_table (void);

cairo_surface_t *cairo_dock_create_surface_from_class (gchar *cClass, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);

void cairo_dock_update_visibility_on_inhibators (gchar *cClass, Window Xid, gboolean bIsHidden);
void cairo_dock_update_activity_on_inhibators (gchar *cClass, Window Xid);


#endif
