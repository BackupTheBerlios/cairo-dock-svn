/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>
#include <glib/gi18n.h>

#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern gchar **my_cTrashDirectoryList;
extern gchar *my_cBrowser;
extern int my_iNbTrash;


CD_APPLET_ABOUT (_D("This is the dustbin applet for Cairo-Dock\n made by Fabrice Rey (fabounet_03@yahoo.fr)"))


CD_APPLET_ON_CLICK_BEGIN
	g_print ("_Note_ : You can manage many Trash directories with this applet.\n Right click on its icon to see which Trash directories are already being monitored.\n");
	cd_dustbin_show_trash (NULL, "trash:/");  // my_cTrashDirectoryList[0]
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Dustbin", pModuleSubMenu, CD_APPLET_MY_MENU)
	int i = 0;
	if (my_cTrashDirectoryList != NULL)
	{
		while (my_cTrashDirectoryList[i] != NULL)
			i ++;
	}
	my_iNbTrash = i;
	
	GString *sLabel = g_string_new ("");
	
	CD_APPLET_ADD_SUB_MENU (_D("Show Trash"), pShowSubMenu, pModuleSubMenu)
	i = 0;
	if (my_cTrashDirectoryList != NULL)
	{
		while (my_cTrashDirectoryList[i] != NULL)
		{
			g_string_printf (sLabel, _D("Show %s"), my_cTrashDirectoryList[i]);
			
			CD_APPLET_ADD_IN_MENU_WITH_DATA (sLabel->str, cd_dustbin_show_trash, pShowSubMenu, my_cTrashDirectoryList[i])
			i ++;
		}
	}
	CD_APPLET_ADD_IN_MENU (_D("Show All"), cd_dustbin_show_trash, pShowSubMenu)
	
	CD_APPLET_ADD_SUB_MENU (_D("Delete Trash"), pDeleteSubMenu, pModuleSubMenu)
	i = 0;
	if (my_cTrashDirectoryList != NULL)
	{
		while (my_cTrashDirectoryList[i] != NULL)
		{
			g_string_printf (sLabel, _D("Delete %s"), my_cTrashDirectoryList[i]);
			
			CD_APPLET_ADD_IN_MENU_WITH_DATA (sLabel->str, cd_dustbin_delete_trash, pDeleteSubMenu, my_cTrashDirectoryList[i])
			
			i ++;
		}
	}
	CD_APPLET_ADD_IN_MENU (_D("Delete All"), cd_dustbin_delete_trash, pDeleteSubMenu)
	
	g_string_free (sLabel, TRUE);
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pModuleSubMenu)
CD_APPLET_ON_BUILD_MENU_END


void cd_dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	gchar *question;
	if (cDirectory != NULL)
		question = g_strdup_printf (_D("You're about to delete all files in %s. Sure ?"), cDirectory);
	else if (my_cTrashDirectoryList != NULL)
		question = g_strdup_printf (_D("You're about to delete all files all dustbins. Sure ?"));
	else
		return;
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		question);
	g_free (question);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
	{
		GString *sCommand = g_string_new ("rm -rf ");
		if (cDirectory != NULL)
		{
			g_string_append_printf (sCommand, "%s/*", cDirectory);
		}
		else
		{
			int i = 0;
			while (my_cTrashDirectoryList[i] != NULL)
			{
				g_string_append_printf (sCommand, "%s ", my_cTrashDirectoryList[i]);
				i ++;
			}
		}
		//g_print (">>> %s\n", sCommand->str);
		GError *erreur = NULL;
		g_spawn_command_line_async (sCommand->str, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : when trying to execute '%s' : %s\n", sCommand->str, erreur->message);
			g_error_free (erreur);
		}
		g_string_free (sCommand, TRUE);
	}
}


void cd_dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	GString *sCommand = g_string_new (my_cBrowser);
	if (cDirectory != NULL)
	{
		g_string_append_printf (sCommand, " %s", cDirectory);
	}
	else
	{
		int i = 0;
		while (my_cTrashDirectoryList[i] != NULL)
		{
			g_string_append_printf (sCommand, " %s", my_cTrashDirectoryList[i]);
			i ++;
		}
	}
	//g_print (">>> %s\n", sCommand->str);
	GError *erreur = NULL;
	g_spawn_command_line_async (sCommand->str, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : when trying to execute '%s' : %s\n", sCommand->str, erreur->message);
		g_error_free (erreur);
		//gchar *cTipMessage = g_strdup_printf ("A problem occured\nIf '%s' is not your usual file browser, you can change it in the conf panel of this module", my_cBrowser);
		cairo_dock_show_temporary_dialog (_D("A problem occured\nIf '%s' is not your usual file browser,\nyou can change it in the conf panel of this module"), myIcon, myDock, 5000, my_cBrowser);
		//g_free (cTipMessage);
	}
	g_string_free (sCommand, TRUE);
}