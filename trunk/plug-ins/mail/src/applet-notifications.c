/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"


extern void config_add_btn_clicked_cb(GtkWidget *w, XfceMailwatch *mailwatch);
extern gboolean config_do_edit_window_2 (GtkWidget *w, XfceMailwatch *mailwatch, XfceMailwatchMailbox *mailbox);

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (_D("This is the mail applet\n made by Christophe Chapuis for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN

    // spawn the selected program
    if( myConfig.cMailApplication )
    {
		cd_message (">>> cd_mail: spawning %s\n", myConfig.cMailApplication);
		GError *erreur = NULL;
		g_spawn_command_line_async(myConfig.cMailApplication, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : when trying to execute '%s' : %s", myConfig.cMailApplication, erreur->message);
			g_error_free (erreur);
			//gchar *cTipMessage = g_strdup_printf ("A problem occured\nIf '%s' is not your usual file browser, you can change it in the conf panel of this module", myConfig.cDefaultBrowser);
			cairo_dock_show_temporary_dialog (_D("A problem occured\nIf '%s' is not your usual mail application,\nyou can change it in the conf panel of this module"), myIcon, myDock, 5000, myConfig.cMailApplication);
			//g_free (cTipMessage);
		}
    }

CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN

    xfce_mailwatch_force_update(myData.mailwatch);

CD_APPLET_ON_MIDDLE_CLICK_END

static void _cd_mail_add_account (GtkMenuItem *menu_item, gpointer *data)
{
    // display a dialog window to select the informations to show
    config_add_btn_clicked_cb(GTK_WIDGET(myContainer->pWidget), myData.mailwatch);

    GKeyFile *pKeyFile = g_key_file_new();

    if( g_key_file_load_from_file(pKeyFile,myIcon->pModule->cConfFilePath,
					     G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS,
					     NULL))
    {
        xfce_mailwatch_save_config(myData.mailwatch, pKeyFile);
        cairo_dock_write_keys_to_file (pKeyFile, myIcon->pModule->cConfFilePath);
    }
    g_key_file_free(pKeyFile);
}

static void _cd_mail_remove_account (GtkMenuItem *menu_item, gpointer *data)
{
    XfceMailwatchMailbox *mailbox = (XfceMailwatchMailbox*)( data );

    GKeyFile *pKeyFile = g_key_file_new();

    if( g_key_file_load_from_file(pKeyFile,myIcon->pModule->cConfFilePath,
					     G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS,
					     NULL))
    {
        cd_mailwatch_remove_account (myData.mailwatch, mailbox);
        xfce_mailwatch_save_config(myData.mailwatch, pKeyFile);
        cairo_dock_write_keys_to_file (pKeyFile, myIcon->pModule->cConfFilePath);
    }
    g_key_file_free(pKeyFile);

    xfce_mailwatch_force_update(myData.mailwatch);
}

static void _cd_mail_modify_account(GtkMenuItem *menu_item, gpointer *data)
{
    XfceMailwatchMailbox *mailbox = (XfceMailwatchMailbox*)( data );

    GKeyFile *pKeyFile = g_key_file_new();

    if( g_key_file_load_from_file(pKeyFile,myIcon->pModule->cConfFilePath,
					     G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS,
					     NULL))
    {
        if( config_do_edit_window_2(GTK_WIDGET(myContainer->pWidget), myData.mailwatch, mailbox) )
        {
            xfce_mailwatch_save_config(myData.mailwatch, pKeyFile);
            cairo_dock_write_keys_to_file (pKeyFile, myIcon->pModule->cConfFilePath);
        }
    }
    g_key_file_free(pKeyFile);

    xfce_mailwatch_force_update(myData.mailwatch);
}

CD_APPLET_ON_BUILD_MENU_BEGIN

    GList *list_names = NULL, *list_data = NULL, *l = NULL, *l2 = NULL;
    guint i;

	CD_APPLET_ADD_SUB_MENU ("mail", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_("Add a new mail account"), _cd_mail_add_account, pSubMenu)

        cd_mailwatch_get_mailboxes_infos( myData.mailwatch, &list_names, &list_data );
        if( list_names && list_data )
        {
            CD_APPLET_ADD_SUB_MENU (_("Remove a mail account"), pRemoveAccountSubMenu, pSubMenu)

            /* add a "remove account" item for each mailbox */
            for(l = list_names, l2 = list_data; l && l2; l = l->next, l2 = l2->next) {
                CD_APPLET_ADD_IN_MENU_WITH_DATA (l->data, _cd_mail_remove_account, pRemoveAccountSubMenu, l2->data)
            }
            CD_APPLET_ADD_SUB_MENU (_("Modify a mail account"), pModifyAccountSubMenu, pSubMenu)

            /* add a "modify account" item for each mailbox */
            for(l = list_names, l2 = list_data; l && l2; l = l->next, l2 = l2->next) {
                CD_APPLET_ADD_IN_MENU_WITH_DATA (l->data, _cd_mail_modify_account, pModifyAccountSubMenu, l2->data)
            }
            g_list_free( list_names );
            g_list_free( list_data );
        }

		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

void
mailwatch_new_messages_changed_cb(XfceMailwatch *mailwatch, gpointer arg, gpointer user_data)
{
    myData.iNbUnreadMails = GPOINTER_TO_UINT( arg );

    cd_message( "mailwatch_new_messages_changed_cb: %d new messages !", myData.iNbUnreadMails );
	if (myData.iNbUnreadMails <= 0)
	{
	    cairo_dock_remove_dialog_if_any (myIcon);
        cairo_dock_show_temporary_dialog (_("No unread mail in your mailboxes"), myIcon, myContainer, 1000);

	    if( myData.pNoMailSurface )
	    {
            CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pNoMailSurface)
	    }
	}
	else
	{
        GString *ttip_str = g_string_sized_new(32);
        gchar **mailbox_names = NULL;
        guint *new_message_counts = NULL;
        gint i;

        g_string_append_printf(ttip_str, "You have %d new mail%s:",myData.iNbUnreadMails,myData.iNbUnreadMails>1?"s":"");

        xfce_mailwatch_get_new_message_breakdown(myData.mailwatch,
                &mailbox_names, &new_message_counts);
        for(i = 0; mailbox_names[i]; i++) {
            if(new_message_counts[i] > 0) {
                g_string_append_printf(ttip_str, "\n    %d in %s",
                        new_message_counts[i], mailbox_names[i]);
            }
        }

        g_strfreev(mailbox_names);
        g_free(new_message_counts);

	    cairo_dock_remove_dialog_if_any (myIcon);
        cairo_dock_show_temporary_dialog (ttip_str->str, myIcon, myContainer, 5000);

        g_string_free(ttip_str, TRUE);

	    if( myData.pHasMailSurface )
	    {
            CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pHasMailSurface)
	    }
	}
    cd_message( "mailwatch_new_messages_changed_cb: Leaving." );

    CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d mail%s", myData.iNbUnreadMails,myData.iNbUnreadMails>1?"s":"")

    CD_APPLET_REDRAW_MY_ICON
}

