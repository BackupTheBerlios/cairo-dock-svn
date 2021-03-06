/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-clipboard.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("Clipper",
	1, 6, 3,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet keeps a trace of the clipboard and mouse selection,\n"
	"so that you can recall them quickly. It's a clone of the well-know Klipper.\n"
	"It supports clipboard and mouse selection, predefined actions, and persistent items.\n"
	"Left-click to popup the clipboard and mouse selection history,\n"
	"Drop text on the icon to create persistent items, and middle-clck to recall them."),
	"Fabrice Rey (Fabounet)")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (myIcon->acFileName == NULL)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
	}
	
	GtkClipboard *pClipBoard;
	if (myConfig.iItemType & CD_CLIPPER_CLIPBOARD)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		myData.iSidClipboardOwnerChange = g_signal_connect (G_OBJECT (pClipBoard), "owner-change", G_CALLBACK(cd_clipper_selection_owner_changed), NULL);
	}
	
	if (myConfig.iItemType & CD_CLIPPER_PRIMARY)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		myData.iSidPrimaryOwnerChange = g_signal_connect (G_OBJECT (pClipBoard), "owner-change", G_CALLBACK(cd_clipper_selection_owner_changed), NULL);
	}
	
	//_on_text_received (NULL, "http://test.fr", NULL);
	//_on_text_received (NULL, "http://truc.fr", NULL);
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	
	GtkClipboard *pClipBoard;
	if (myData.iSidClipboardOwnerChange != 0)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		g_signal_handler_disconnect (pClipBoard, myData.iSidClipboardOwnerChange);
	}
	if (myData.iSidPrimaryOwnerChange != 0)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		g_signal_handler_disconnect (pClipBoard, myData.iSidPrimaryOwnerChange);
	}
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		gtk_widget_queue_draw (myDesklet->pWidget);
	}
	
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myIcon->acFileName == NULL)
		{
			CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
		}
		
		g_list_foreach (myData.pActions, (GFunc) cd_clipper_free_action, NULL);
		g_list_free (myData.pActions);
		myData.pActions = NULL;
		myData.bActionsLoaded = FALSE;
		
		GtkClipboard *pClipBoard;
		if (myConfig.iItemType & CD_CLIPPER_CLIPBOARD)
		{
			if (myData.iSidClipboardOwnerChange == 0)
			{
				pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
				myData.iSidClipboardOwnerChange = g_signal_connect (G_OBJECT (pClipBoard), "owner-change", G_CALLBACK(cd_clipper_selection_owner_changed), NULL);
			}
		}
		else
		{
			if (myData.iSidClipboardOwnerChange != 0)
			{
				pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
				g_signal_handler_disconnect (pClipBoard, myData.iSidClipboardOwnerChange);
				myData.iSidClipboardOwnerChange = 0;
			}
		}
		
		if (myConfig.iItemType & CD_CLIPPER_PRIMARY)
		{
			if (myData.iSidPrimaryOwnerChange == 0)
			{
				pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
				myData.iSidPrimaryOwnerChange = g_signal_connect (G_OBJECT (pClipBoard), "owner-change", G_CALLBACK(cd_clipper_selection_owner_changed), NULL);
			}
		}
		else
		{
			if (myData.iSidPrimaryOwnerChange != 0)
			{
				pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
				g_signal_handler_disconnect (pClipBoard, myData.iSidPrimaryOwnerChange);
				myData.iSidPrimaryOwnerChange = 0;
			}
		}
	}
CD_APPLET_RELOAD_END
