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
