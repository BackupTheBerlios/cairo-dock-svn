/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include "cairo-dock-notifications.h"

static GPtrArray *pNotificationsTab = NULL;


void cairo_dock_register_notification (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction, gboolean bRunFirst)
{
	if (pNotificationsTab == NULL)
		pNotificationsTab = g_ptr_array_sized_new (CAIRO_DOCK_NB_NOTIFICATIONS);
	
	if (pNotificationsTab->len < iNotifType + 1)
		g_ptr_array_set_size (pNotificationsTab, iNotifType + 1);
	
	GSList *pFunctionListForNotification = g_ptr_array_index (pNotificationsTab, iNotifType);
	if (bRunFirst)
		pNotificationsTab->pdata[iNotifType] = g_slist_prepend (pFunctionListForNotification, pFunction);
	else
		pNotificationsTab->pdata[iNotifType] = g_slist_append (pFunctionListForNotification, pFunction);
}

void cairo_dock_remove_notification_func (CairoDockNotificationType iNotifType, CairoDockNotificationFunc pFunction)
{
	if (pNotificationsTab != NULL)
	{
		GSList *pFunctionListForNotification = g_ptr_array_index (pNotificationsTab, iNotifType);
		pNotificationsTab->pdata[iNotifType] = g_slist_remove (pFunctionListForNotification, pFunction);
	}
}

gboolean cairo_dock_notify (CairoDockNotificationType iNotifType, gpointer data)
{
	gboolean bNotificationCaught = FALSE;
	
	if (pNotificationsTab != NULL)
	{
		GSList *pFunctionListForNotification = g_ptr_array_index (pNotificationsTab, iNotifType);
		if (pFunctionListForNotification != NULL)
			bNotificationCaught = TRUE;
		
		gboolean bStop = FALSE;
		CairoDockNotificationFunc pFunction;
		GSList *pElementList = pFunctionListForNotification;
		while (pElementList != NULL && ! bStop)
		{
			pFunction = pElementList->data;
			
			bStop = pFunction (data);
			
			pElementList = pElementList->next;
		}
	}
	
	return bNotificationCaught;
}
