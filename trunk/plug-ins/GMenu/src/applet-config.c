/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bHasIcons = CD_CONFIG_GET_BOOLEAN ("Configuration", "has icons");
	myConfig.bShowRecent = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show recent", TRUE);
	myConfig.cMenuShortkey = CD_CONFIG_GET_STRING ("Configuration", "menu shortkey");
	myConfig.cQuickLaunchShortkey = CD_CONFIG_GET_STRING ("Configuration", "quick launch shortkey");
	myConfig.cConfigureMenuCommand = CD_CONFIG_GET_STRING ("Configuration", "config menu");
	myConfig.cRecentRootDirFilter = CD_CONFIG_GET_STRING ("Configuration", "recent root dir");
	if (myConfig.cRecentRootDirFilter != NULL && *myConfig.cRecentRootDirFilter == '/')
	{
		gchar *tmp = myConfig.cRecentRootDirFilter;
		myConfig.cRecentRootDirFilter = g_strconcat ("file://", myConfig.cRecentRootDirFilter, NULL);
		g_free (tmp);
	}
	myConfig.iRecentAge = CD_CONFIG_GET_INTEGER ("Configuration", "recent age");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cConfigureMenuCommand);
	if (myConfig.cMenuShortkey)
		cd_keybinder_unbind (myConfig.cMenuShortkey, (CDBindkeyHandler) cd_menu_on_shortkey_menu);
	g_free (myConfig.cMenuShortkey);
	
	if (myConfig.cQuickLaunchShortkey)
		cd_keybinder_unbind (myConfig.cQuickLaunchShortkey, (CDBindkeyHandler) cd_menu_on_shortkey_quick_launch);
	g_free (myConfig.cQuickLaunchShortkey);
	
	g_free (myConfig.cRecentRootDirFilter);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.pMenu)
		gtk_widget_destroy (myData.pMenu);  // detruit aussi pRecentMenuItem.
	
	if (myData.dir_hash)
		g_hash_table_destroy (myData.dir_hash);
	
	GList *l;
	for (l = myData.possible_executables; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.possible_executables);
	
	for (l = myData.completion_items; l; l = l->next)
		g_free (l->data);
	g_list_free (myData.completion_items);
	
	if (myData.completion)
		g_completion_free (myData.completion);
	
	if (!cairo_dock_dialog_unreference (myData.pQuickLaunchDialog))
		cairo_dock_dialog_unreference (myData.pQuickLaunchDialog);
CD_APPLET_RESET_DATA_END
