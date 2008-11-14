
#ifndef __CAIRO_DOCK_NOTIFICATIONS__
#define  __CAIRO_DOCK_NOTIFICATIONS__

#include <glib.h>
G_BEGIN_DECLS


typedef gboolean (* CairoDockNotificationFunc) (gpointer *data, gpointer pUserData);

typedef struct {
	CairoDockNotificationFunc pFunction;
	gpointer pUserData;
	} CairoDockNotificationRecord;

typedef enum {
	/// notification appellee lorsque l'utilisateur supprime un lanceur via le menu. data : {Icon, CairoDock}
	CAIRO_DOCK_REMOVE_ICON=0,
	/// notification appellee lorsque l'utilisateur clique sur une icone; l'animation est preparee juste avant, et lancee juste apres. data : {Icon, CairoDock, iState}
	CAIRO_DOCK_CLICK_ICON,
	/// notification appellee lorsque l'utilisateur double clique sur une icone. data : {Icon, CairoDock}
	CAIRO_DOCK_DOUBLE_CLICK_ICON,
	/// notification appellee lorsque l'utilisateur clique-milieu  sur une icone. data : {Icon, CairoDock}
	CAIRO_DOCK_MIDDLE_CLICK_ICON,
	/// data : {Icon, CairoDock, GtkMenu}
	CAIRO_DOCK_BUILD_MENU,
	/// data : {gchar*, Icon, double*, CairoDock}
	CAIRO_DOCK_DROP_DATA,
	/// notification appellee lors d'un changement de bureau ou de viewport. data : NULL
	CAIRO_DOCK_DESKTOP_CHANGED,
	/// notification appellee lorsqu'une fenetre est redimensionnee ou deplacee, ou lorsque l'ordre des fenetres change. data : XConfigureEvent ou NULL.
	CAIRO_DOCK_WINDOW_CONFIGURED,
	/// notification appellee lorsque la geometrie du bureau a change (nombre de viewports/bureaux, dimensions). data : NULL
	CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED,
	/// notification appellee lorsque la fenetre active a change. data : Xid
	CAIRO_DOCK_WINDOW_ACTIVATED,
	/// notification appellee lorsque l'utilisateur scrolle sur une icone en maintenant la touche SHIFT ou CTRL enfoncee. data : {Icon, CairoDock, iDirection}
	CAIRO_DOCK_SCROLL_ICON,
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
void cairo_dock_register_notification (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gboolean bRunFirst, gpointer pUserData);

/**
*Enleve une fonction de la liste des fonctions appelees par une notification donnee.
*@param iNotifType type de la notification.
*@param pFunction fonction notifiee.
*/
void cairo_dock_remove_notification_func (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gpointer pUserData);

GList *cairo_dock_get_notifications_list (CairoDockNotificationType iNotifType);
/**
*Appelle toutes les fonctions enregistrees pour une notification donnee.
*@param iNotifType type de la notification.
*@param data donnees passees a la fonction notifiee.
@return TRUE si la notification a ete utilisee par quelqu'un, FALSE si aucune fonction n'est enregistree pour elle.
*/
gboolean cairo_dock_notify (CairoDockNotificationType iNotifType, gpointer data);

typedef gboolean (* CairoDockNotificationFunc2) (gpointer pUserData, ...);
#define cairo_dock_notify2(iNotifType, ...) {\
	GSList *pNotificationRecordList = cairo_dock_get_notifications_list (iNotifType);\
	if (pNotificationRecordList == NULL)\
		FALSE;\
	gboolean bStop = FALSE;\
	CairoDockNotificationFunc2 pFunction;\
	CairoDockNotificationRecord *pNotificationRecord;\
	GSList *pElement = pNotificationRecordList;\
	while (pElement != NULL && ! bStop) {\
		pNotificationRecord = pElement->data;\
		bStop = pNotificationRecord->pFunction (pNotificationRecord->pUserData, ##__VA_ARGS__);\
		pElement = pElement->next; }\
	TRUE; }

/**
*Enregistre une liste de fonctions devant etre notifiees en premier. La liste est une liste de couples (CairoDockNotificationType, CairoDockNotificationFunc), et doit etre clot par -1.
*@param iFirstNotifType type de la 1ere notification.
*@param ... 1ere fonction notifiee, puis triplet de (notification, fonction, user_data), termine par -1.
*/
void cairo_dock_register_first_notifications (int iFirstNotifType, ...);
/**
*Enregistre une liste de fonctions devant etre notifiees en dernier. La liste est une liste de couples (CairoDockNotificationType, CairoDockNotificationFunc), et doit etre clot par -1.
*@param iFirstNotifType type de la 1ere notification.
*@param ... 1ere fonction notifiee, puis triplet de (notification, fonction, user_data), termine par -1.
*/
void cairo_dock_register_last_notifications (int iFirstNotifType, ...);
/**
*Enleve une liste de fonctions notifiees. La liste est une liste de couples (CairoDockNotificationType, CairoDockNotificationFunc), et doit etre clot par -1.
*@param iFirstNotifType type de la 1ere notification.
*@param ... 1ere fonction notifiee, puis couple de (notification, fonction), termine par -1.
*/
void cairo_dock_remove_notification_funcs (int iFirstNotifType, ...);


G_END_DECLS
#endif
