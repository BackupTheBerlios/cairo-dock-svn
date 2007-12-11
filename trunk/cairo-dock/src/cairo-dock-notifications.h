
#ifndef __CAIRO_DOCK_NOTIFICATIONS__
#define  __CAIRO_DOCK_NOTIFICATIONS__

#include <glib.h>


typedef gboolean (* CairoDockNotificationFunc) (gpointer *data);

typedef enum {
	/// notification appellee lorsque l'utilisateur supprime un lanceur via le menu. data : {Icon, CairoDock}
	CAIRO_DOCK_REMOVE_ICON=0,
	/// notification appellee lorsque l'utilisateur clique sur une icone; l'animation est preparee juste avant, et lancee juste apres. data : Icon
	CAIRO_DOCK_CLICK_ICON,
	/// notification appellee lorsque l'utilisateur double clique sur une icone. data : Icon
	CAIRO_DOCK_DOUBLE_CLICK_ICON,
	/// data : Icon
	CAIRO_DOCK_MIDDLE_CLICK_ICON,
	/// data : {Icon, CairoDock}
	CAIRO_DOCK_ADD_ICON,
	/// data : {Icon, CairoDock}
	CAIRO_DOCK_MODIFY_ICON,
	/// data : {Icon, CairoDock, GtkMenu}
	CAIRO_DOCK_BUILD_MENU,
	/// data : {gchar, Icon, double, CairoDock}
	CAIRO_DOCK_DROP_DATA,
	/// data : NULL
	CAIRO_DOCK_DESKTOP_CHANGED,
	///
	CAIRO_DOCK_NB_NOTIFICATIONS
	} CairoDockNotificationType;

#define CAIRO_DOCK_RUN_FIRST TRUE
#define CAIRO_DOCK_RUN_AFTER FALSE

#define CAIRO_DOCK_INTERCEPT_NOTIFICATION TRUE
#define CAIRO_DOCK_LET_PASS_NOTIFICATION FALSE

/**
*Enregistre une action a appeler lors d'une notification.
*@param iNotifType type de la notification.
*@param pFunction fonction notifiee.
*@param bRunFirst CAIRO_DOCK_RUN_FIRST pour etre enregistre en 1er, CAIRO_DOCK_RUN_AFTER pour etre enregistreé en dernier.
*/
void cairo_dock_register_notification (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gboolean bRunFirst);

/**
*Enleve une fonction de la liste des fonctions appelees par une notification donnee.
*@param iNotifType type de la notification.
*@param pFunction fonction notifiee.
*/
void cairo_dock_remove_notification_func (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction);
/**
*Enleve une fonction de la liste des fonctions appelees par une notification donnee.
*@param iNotifType type de la notification.
*@param data donnees passees a la fonction notifiee.
@Returns TRUE si la notification a ete utilisee par quelqu'un, FALSE si aucune fonction n'est enregistree pour elle.
*/
gboolean cairo_dock_notify (CairoDockNotificationType iNotifType, gpointer data);

#endif
