/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include "cairo-dock-notifications.h"

static GPtrArray *s_pNotificationsTab = NULL;


void cairo_dock_register_notification (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gboolean bRunFirst)
{
	if (s_pNotificationsTab == NULL)
	{
		s_pNotificationsTab = g_ptr_array_new ();  // un 'g_ptr_array_sized_new' ne met pas a 0 les pointeurs, et laisse 'len' a 0.
		g_ptr_array_set_size (s_pNotificationsTab, CAIRO_DOCK_NB_NOTIFICATIONS);
	}
	
	g_return_if_fail (iNotifType < s_pNotificationsTab->len);
	
	GSList *pFunctionListForNotification = g_ptr_array_index (s_pNotificationsTab, iNotifType);
	if (bRunFirst)
		s_pNotificationsTab->pdata[iNotifType] = g_slist_prepend (pFunctionListForNotification, pFunction);
	else
		s_pNotificationsTab->pdata[iNotifType] = g_slist_append (pFunctionListForNotification, pFunction);
}

void cairo_dock_remove_notification_func (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction)
{
	if (s_pNotificationsTab != NULL)
	{
		GSList *pFunctionListForNotification = g_ptr_array_index (s_pNotificationsTab, iNotifType);
		s_pNotificationsTab->pdata[iNotifType] = g_slist_remove (pFunctionListForNotification, pFunction);
	}
}

gboolean cairo_dock_notify (CairoDockNotificationType iNotifType, gpointer data)
{
	if (s_pNotificationsTab != NULL)
	{
		g_return_val_if_fail (iNotifType < s_pNotificationsTab->len, FALSE);
		
		GSList *pFunctionListForNotification = g_ptr_array_index (s_pNotificationsTab, iNotifType);
		if (pFunctionListForNotification == NULL)
			return FALSE;
		
		gboolean bStop = FALSE;
		CairoDockNotificationFunc pFunction;
		GSList *pElementList = pFunctionListForNotification;
		while (pElementList != NULL && ! bStop)
		{
			pFunction = pElementList->data;
			
			bStop = pFunction (data);
			
			pElementList = pElementList->next;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


static void cairo_dock_register_notifications (gboolean bRunFirst, int iFirstNotifType, va_list args)
{
	CairoDockNotificationType iNotifType = iFirstNotifType;
	CairoDockNotificationFunc pFunction;
	
	while (iNotifType != -1)
	{
		g_print ("%s () : %d\n", __func__, iNotifType);
		
		pFunction= va_arg (args, CairoDockNotificationFunc);
		if (pFunction == NULL)  // ne devrait pas arriver.
			break;
		
		cairo_dock_register_notification (iNotifType, pFunction, bRunFirst);
		
		iNotifType = va_arg (args, CairoDockNotificationType);
	}
}

void cairo_dock_register_first_notifications (int iFirstNotifType, ...)
{
	va_list args;
	va_start (args, iFirstNotifType);
	cairo_dock_register_notifications (CAIRO_DOCK_RUN_FIRST, iFirstNotifType, args);
	va_end (args);
}

void cairo_dock_register_last_notifications (int iFirstNotifType, ...)
{
	va_list args;
	va_start (args, iFirstNotifType);
	cairo_dock_register_notifications (CAIRO_DOCK_RUN_AFTER, iFirstNotifType, args);
	va_end (args);
}

void cairo_dock_remove_notification_funcs (int iFirstNotifType, ...)
{
	va_list args;
	va_start (args, iFirstNotifType);
	
	CairoDockNotificationType iNotifType = iFirstNotifType;
	CairoDockNotificationFunc pFunction;
	while (iNotifType != -1)
	{
		pFunction= va_arg (args, CairoDockNotificationFunc);
		if (pFunction == NULL)  // ne devrait pas arriver.
			break;
		
		cairo_dock_remove_notification_func (iNotifType, pFunction);
		
		iNotifType = va_arg (args, CairoDockNotificationType);
	}
	
	va_end (args);
}
