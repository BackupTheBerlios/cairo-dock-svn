/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "cairo-dock-config.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-desktop-file-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-menu.h"

#define CAIRO_DOCK_CONF_PANEL_WIDTH 800
#define CAIRO_DOCK_CONF_PANEL_HEIGHT 600
#define CAIRO_DOCK_LAUNCHER_PANEL_WIDTH 600
#define CAIRO_DOCK_LAUNCHER_PANEL_HEIGHT 350

extern CairoDock *g_pMainDock;
extern double g_fSubDockSizeRatio;

extern gboolean g_bAutoHide;
extern gchar *g_cConfFile;
extern gchar *g_cEasyConfFile;
extern gchar *g_cCurrentLaunchersPath;


static void cairo_dock_edit_and_reload_conf (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	cairo_dock_build_easy_conf_file (g_cConfFile, g_cEasyConfFile);
	
	if (cairo_dock_use_full_conf_file ())
		cairo_dock_edit_conf_file_full (GTK_WINDOW (pDock->pWidget), g_cConfFile, "Configuration of Cairo-Dock", CAIRO_DOCK_CONF_PANEL_WIDTH, CAIRO_DOCK_CONF_PANEL_HEIGHT, '\0', NULL, (CairoDockConfigFunc) cairo_dock_read_conf_file, g_pMainDock, NULL, cairo_dock_read_easy_conf_file, g_cEasyConfFile, "Well, maybe not ...", "I want more !");
	else
		cairo_dock_edit_conf_file_full (GTK_WINDOW (pDock->pWidget), g_cEasyConfFile, "Configuration of Cairo-Dock", CAIRO_DOCK_CONF_PANEL_WIDTH, CAIRO_DOCK_CONF_PANEL_HEIGHT, '\0', NULL, (CairoDockConfigFunc) cairo_dock_read_easy_conf_file, g_pMainDock, NULL, (CairoDockConfigFunc) cairo_dock_read_conf_file, g_cConfFile, "I want more !", "Well, maybe not ...");
	
}

static void cairo_dock_initiate_theme_management (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	gboolean bRefreshGUI;
	do
	{
		bRefreshGUI = cairo_dock_manage_themes (pDock->pWidget);
	} while (bRefreshGUI);
}

static void cairo_dock_about (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	gchar *cTitle = g_strdup_printf ("\nCairo-Dock (2007)\n version %s", CAIRO_DOCK_VERSION);
	GtkWidget *pDialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		cTitle);
	g_free (cTitle);
	
	gchar *cImagePath = g_strdup_printf ("%s/cairo-dock.svg", CAIRO_DOCK_SHARE_DATA_DIR);
	GtkWidget *pImage = gtk_image_new_from_file (cImagePath);
#if GTK_MINOR_VERSION >= 10
	gtk_message_dialog_set_image (GTK_MESSAGE_DIALOG (pDialog), pImage);
#endif
	GtkWidget *pLabel = gtk_label_new (NULL);
	gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
	gchar *cAboutText = g_strdup_printf ("<b>Original idea/first development :</b>\n  Mac Slow\n\
<b>Main developer :</b>\n  Fabounet (Fabrice Rey)\n\
<b>Applets :</b>\n  Fabounet\n  Necropotame\n\
<b>Themes :</b>\n  Fabounet\n  Chilperik\n  Djoole\n  Glattering\n  Vilraleur\n  Lord Northam\n\
<b>Patchs :</b>\n  Robrob\n  Tshirtman\n\
<b>Translations :</b>\n  Fabounet\n  Ppmt\n\
<b>Suggestions/Comments/Bêta-Testers :</b>\n  AuraHxC\n  Chilperik\n  Cybergoll\n  Damster\n  Djoole\n  Glattering\n  Mav\n  Necropotame\n  Ppmt\n  Sombrero\n  Vilraleur");  // Djoole <=> (Julien Barrau)
	gtk_label_set_markup (GTK_LABEL (pLabel), cAboutText);
	g_free (cAboutText);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pDialog)->vbox), pLabel);
	
	gtk_widget_show_all (pDialog);
	gtk_window_set_position (GTK_WINDOW (pDialog), GTK_WIN_POS_CENTER_ALWAYS);  // un GTK_WIN_POS_CENTER simple ne marche pas, probablement parceque la fenetre n'est pas encore realisee. le 'always' ne pose pas de probleme, puisqu'on ne peut pas redimensionner le dialogue.
	gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);
}

static void cairo_dock_update (GtkMenuItem *menu_item, gpointer *data)
{
	system ("xterm -e cairo-dock-update.sh &");
}

static void cairo_dock_help (GtkMenuItem *menu_item, gpointer *data)
{
	GError *erreur = NULL;
	g_spawn_command_line_async ("firefox http://doc.ubuntu-fr.org/cairo-dock", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n  you can consult the wiki at http://doc.ubuntu-fr.org/cairo-dock\n", erreur->message);
		g_error_free (erreur);
	}
}

static void cairo_dock_quick_hide (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[1];
	//g_print ("%s ()\n", __func__);
	pDock->bMenuVisible = FALSE;
	cairo_dock_activate_temporary_auto_hide (g_pMainDock);
}

static void cairo_dock_quit (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[1];
	on_delete (pDock->pWidget, NULL, pDock);
}


gboolean cairo_dock_notification_remove_icon (gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	if (icon->pSubDock != NULL)
	{
		gboolean bDestroyIcons = TRUE;
		if (! CAIRO_DOCK_IS_URI_LAUNCHER (icon) && icon->pSubDock->icons != NULL)  // alors on propose de repartir les icones de son sous-dock dans le dock principal.
		{
			int answer = cairo_dock_ask_question_and_wait (_("Do you want to re-dispatch the icons contained inside this container into the dock ?\n (otherwise they will be destroyed)"), icon, pDock);
			g_return_val_if_fail (answer != GTK_RESPONSE_NONE, CAIRO_DOCK_LET_PASS_NOTIFICATION);
			if (answer == GTK_RESPONSE_YES)
				bDestroyIcons = FALSE;
		}
		cairo_dock_destroy_dock (icon->pSubDock, icon->acName, (bDestroyIcons ? NULL : g_pMainDock), (bDestroyIcons ? NULL : CAIRO_DOCK_MAIN_DOCK_NAME));
		icon->pSubDock = NULL;
	}
	
	icon->fPersonnalScale = 1.0;
	if (pDock->iSidShrinkDown == 0)
		pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
	
	cairo_dock_mark_theme_as_modified (TRUE);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;  // on l'intercepte car on ne peut plus garantir la validite de l'icone apres cela.
}
static void cairo_dock_remove_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	gchar *question = g_strdup_printf (_("You're about to remove this icon (%s) from the dock. Sure ?"), icon->acName);
	///int answer = cairo_dock_ask_question (pDock, question);
	int answer = cairo_dock_ask_question_and_wait (question, icon, pDock);
	g_free (question);
	if (answer == GTK_RESPONSE_YES)
	{
		cairo_dock_notify (CAIRO_DOCK_REMOVE_ICON, data);
	}
}

static void _cairo_dock_create_launcher (GtkMenuItem *menu_item, gpointer *data, gboolean bIsContainer)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	//\___________________ On determine l'ordre d'insertion suivant l'endroit du clique.
	GError *erreur = NULL;
	double fOrder;
	if (CAIRO_DOCK_IS_LAUNCHER (icon))
	{
		if (pDock->iMouseX < icon->fDrawX + icon->fWidth * icon->fScale / 2)  // a gauche.
		{
			Icon *prev_icon = cairo_dock_get_previous_icon (pDock->icons, icon);
			fOrder = (prev_icon != NULL ? (icon->fOrder + prev_icon->fOrder) / 2 : icon->fOrder - 1);
		}
		else
		{
			Icon *next_icon = cairo_dock_get_next_icon (pDock->icons, icon);
			fOrder = (next_icon != NULL ? (icon->fOrder + next_icon->fOrder) / 2 : icon->fOrder + 1);
		}
	}
	else
		fOrder = CAIRO_DOCK_LAST_ORDER;
	
	//\___________________ On cree un fichier de lanceur avec des valeurs par defaut.
	const gchar *cDockName = cairo_dock_search_dock_name (pDock);
	gchar *cNewDesktopFileName;
	if (bIsContainer)
		cNewDesktopFileName = cairo_dock_add_desktop_file_for_container (cDockName, fOrder, pDock, &erreur);
	else
		cNewDesktopFileName = cairo_dock_add_desktop_file_from_uri (NULL, cDockName, fOrder, pDock, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while trying to create a new launcher : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	//\___________________ On ouvre automatiquement l'IHM pour permettre de modifier ses champs.
	gchar *cNewDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, cNewDesktopFileName);
	cairo_dock_update_launcher_desktop_file (cNewDesktopFilePath, bIsContainer);
	
	gboolean config_ok = cairo_dock_edit_conf_file (GTK_WINDOW (pDock->pWidget), cNewDesktopFilePath, _("Fill this launcher"), CAIRO_DOCK_LAUNCHER_PANEL_WIDTH, CAIRO_DOCK_LAUNCHER_PANEL_HEIGHT, 0, NULL, NULL, NULL, NULL);
	if (config_ok)
	{
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pDock);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cNewDesktopFileName, pCairoContext);
		
		CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
		cairo_dock_insert_icon_in_dock (pNewIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
		
		if (pDock->iSidShrinkDown == 0)
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
		cairo_dock_mark_theme_as_modified (TRUE);
	}
	else
	{
		g_remove (cNewDesktopFilePath);
	}
	
	g_free (cNewDesktopFileName);
	g_free (cNewDesktopFilePath);
}

static void cairo_dock_add_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	GtkWidget *pFileChooserDialog = gtk_file_chooser_dialog_new (_("Choose or Create a launcher"),
		GTK_WINDOW (pDock->pWidget),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_NEW,
		1,
		GTK_STOCK_OK,
		GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);
	
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (pFileChooserDialog), "/usr/share/app-install/");
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (pFileChooserDialog), TRUE);
	gtk_widget_show (pFileChooserDialog);
	
	int answer = gtk_dialog_run (GTK_DIALOG (pFileChooserDialog));
	if (answer == GTK_RESPONSE_OK)
	{
		GSList* selected_files = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (pFileChooserDialog));
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pDock);
		gchar *cFilePath;
		Icon *pNewIcon;
		gchar *cDesktopFileName;
		const gchar *cDockName;
		GError *erreur = NULL;
		GSList *pSelectedFile;
		for (pSelectedFile = selected_files; pSelectedFile != NULL; pSelectedFile = pSelectedFile->next)
		{
			cFilePath = pSelectedFile->data;
			cDockName = cairo_dock_search_dock_name (pDock);
			cDesktopFileName = cairo_dock_add_desktop_file_from_uri (cFilePath, cDockName, CAIRO_DOCK_LAST_ORDER, pDock, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
				continue;
			}
			
			pNewIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFileName, pCairoContext);
			g_free (cDesktopFileName);
			
			cairo_dock_insert_icon_in_dock (pNewIcon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
			
			cairo_dock_mark_theme_as_modified (TRUE);
			g_free (cFilePath);
		}
		g_slist_free (selected_files);
		cairo_destroy (pCairoContext);
		
		cairo_dock_update_dock_size (pDock);
		
		if (pDock->iSidShrinkDown == 0)
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
		gtk_widget_queue_draw (pDock->pWidget);
		
		gtk_widget_destroy (pFileChooserDialog);
	}
	else if (answer == 1)
	{
		gtk_widget_destroy (pFileChooserDialog);  // dans ce cas on ferme le selecteur avant.
		_cairo_dock_create_launcher (menu_item, data, FALSE);
	}
	else
	{
		gtk_widget_destroy (pFileChooserDialog);
	}
}

static void cairo_dock_add_container (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	_cairo_dock_create_launcher (menu_item, data, TRUE);
}

static void cairo_dock_modify_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon->acDesktopFileName);
	
	cairo_dock_update_launcher_desktop_file (cDesktopFilePath, icon->pSubDock != NULL);
	
	gboolean config_ok = cairo_dock_edit_conf_file (GTK_WINDOW (pDock->pWidget), cDesktopFilePath, _("Modify this launcher"), CAIRO_DOCK_LAUNCHER_PANEL_WIDTH, CAIRO_DOCK_LAUNCHER_PANEL_HEIGHT, 0, NULL, NULL, NULL, NULL);
	g_free (cDesktopFilePath);
	
	if (! pDock->bInside)
	{
		//g_print ("on force a quitter\n");
		pDock->bInside = TRUE;
		pDock->bAtBottom = FALSE;
		on_leave_notify2 (pDock->pWidget,
			NULL,
			pDock);
	}
	
	if (config_ok)
	{
		GError *erreur = NULL;
		cairo_dock_detach_icon_from_dock (icon, pDock, TRUE);  // il va falloir la recreer, car tous ses parametres peuvent avoir change; neanmoins, on ne souhaite pas detruire son .desktop.
		///cairo_dock_remove_icon_from_dock (pDock, icon);
		
		//\_____________ On recree l'icone de zero.
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (icon->acDesktopFileName, pCairoContext);
		
		//\_____________ On redistribue les icones du sous-dock si l'icone n'est plus un container.
		if (icon->pSubDock != NULL)
		{
			if (pNewIcon->pSubDock == NULL)  // ce n'est plus un container.
			{
				gboolean bDestroyIcons = TRUE;
				///int answer = cairo_dock_ask_question (pDock, "Do you want to re-dispatch the icons contained inside this container into the dock (otherwise they will be destroyed) ?");
				int answer = GTK_RESPONSE_NONE;
				while (answer == GTK_RESPONSE_NONE)
					answer = cairo_dock_ask_question_and_wait (_("Do you want to re-dispatch the icons contained inside this container into the dock ?\n (otherwise they will be destroyed)"), icon, pDock);
				if (answer == GTK_RESPONSE_YES)
					bDestroyIcons = FALSE;
				cairo_dock_destroy_dock (icon->pSubDock, icon->acName, (bDestroyIcons ? NULL : g_pMainDock), (bDestroyIcons ? NULL : CAIRO_DOCK_MAIN_DOCK_NAME));
				icon->pSubDock = NULL;
			}
			else if (pNewIcon->pSubDock != icon->pSubDock)  // ca n'est plus le meme container.
			{
				cairo_dock_destroy_dock (icon->pSubDock, icon->acName, pNewIcon->pSubDock, pNewIcon->acName);
			}
			icon->pSubDock = NULL;
		}
		
		//\_____________ On cherche le dock auquel elle appartient maintenant.
		CairoDock *pNewContainer = cairo_dock_search_dock_from_name (pNewIcon->cParentDockName);
		g_return_if_fail (pNewContainer != NULL);
		
		if (pDock != pNewContainer && pNewIcon->fOrder > g_list_length (pNewContainer->icons) + 1)
			pNewIcon->fOrder = CAIRO_DOCK_LAST_ORDER;
		
		cairo_dock_insert_icon_in_dock (pNewIcon, pNewContainer, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);  // on n'empeche pas les bouclages.
		
		if (pDock != pNewContainer)
			cairo_dock_update_dock_size (pDock);
		
		cairo_dock_free_icon (icon);  // on ne le fait que maintenant pour plus de surete.
		cairo_destroy (pCairoContext);
		gtk_widget_queue_draw (pDock->pWidget);
		if (pNewContainer != pDock)
			gtk_widget_queue_draw (pNewContainer->pWidget);
		cairo_dock_mark_theme_as_modified (TRUE);
	}
}


static void cairo_dock_initiate_config_module_from_module (GtkMenuItem *menu_item, CairoDockModule *pModule)
{
	GError *erreur = NULL;
	cairo_dock_configure_module (GTK_WINDOW (g_pMainDock->pWidget), pModule, g_pMainDock, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
	}
}
static void cairo_dock_initiate_config_module (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	GError *erreur = NULL;
	cairo_dock_configure_module (GTK_WINDOW (pDock->pWidget), icon->pModule, pDock, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
	}
}

static void cairo_dock_remove_module (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	gchar *question = g_strdup_printf (_("You're about to remove this module (%s) from the dock. Sure ?"), icon->acName);
	///int answer = cairo_dock_ask_question (pDock, question);
	int answer = cairo_dock_ask_question_and_wait (question, icon, pDock);
	if (answer == GTK_RESPONSE_YES)
	{
		cairo_dock_remove_icon_from_dock (pDock, icon);  // desactive le module.
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, pDock->icons);
		cairo_dock_update_dock_size (pDock);
		gtk_widget_queue_draw (pDock->pWidget);
		cairo_dock_free_icon (icon);
	}
}


static void cairo_dock_close_appli (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	
	if (CAIRO_DOCK_IS_VALID_APPLI (icon))
		cairo_dock_close_xwindow (icon->Xid);
}

static void cairo_dock_minimize_appli (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (icon->Xid > 0)
		cairo_dock_minimize_xwindow (icon->Xid);
}

static void cairo_dock_maximize_appli (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (icon->Xid > 0)
	{
		gboolean bIsMaximized = cairo_dock_window_is_maximized (icon->Xid);
		cairo_dock_maximize_xwindow (icon->Xid, ! bIsMaximized);
	}
}

static void cairo_dock_set_appli_fullscreen (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (icon->Xid > 0)
	{
		gboolean bIsFullScreen = cairo_dock_window_is_fullscreen (icon->Xid);
		cairo_dock_set_xwindow_fullscreen (icon->Xid, ! bIsFullScreen);
	}
}

static void cairo_dock_move_appli_to_current_desktop (GtkMenuItem *menu_item, gpointer *data)
{
	//g_print ("%s ()\n", __func__);
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (icon->Xid > 0)
	{
		int iCurrentDesktop, iDesktopViewportX, iDesktopViewportY;
		cairo_dock_get_current_desktop (&iCurrentDesktop, &iDesktopViewportX, &iDesktopViewportY);
		
		cairo_dock_move_xwindow_to_nth_desktop (icon->Xid, iCurrentDesktop, 0, 0);  // on ne veut pas decaler son viewport par rapport a nous.
	}
}


static void cairo_dock_swap_with_prev_icon (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	Icon *prev_icon = cairo_dock_get_previous_icon (pDock->icons, icon);
	cairo_dock_swap_icons (pDock, icon, prev_icon);
}

static void cairo_dock_swap_with_next_icon (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	Icon *next_icon = cairo_dock_get_next_icon (pDock->icons, icon);
	cairo_dock_swap_icons (pDock, icon, next_icon);
}

static void cairo_dock_move_icon_to_beginning (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	cairo_dock_move_icon_after_icon (pDock, icon, NULL);
}

static void cairo_dock_move_icon_to_end (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	Icon* pLastIcon = cairo_dock_get_last_icon_of_type (pDock->icons, icon->iType);
	if (pLastIcon != NULL && pLastIcon != icon)
		cairo_dock_move_icon_after_icon (pDock, icon, pLastIcon);
}

static void cairo_dock_delete_menu (GtkMenuShell *menu, CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	pDock->bMenuVisible = FALSE;
	if (! pDock->bInside)
	{
		g_print ("on force a quitter\n");
		pDock->bInside = TRUE;
		pDock->bAtBottom = FALSE;
		///cairo_dock_disable_entrance ();  // trop violent, il faudrait trouver un autre truc.
		on_leave_notify2 (pDock->pWidget,
			NULL,
			pDock);
	}
}


static void _cairo_dock_insert_module_in_menu (gchar *cModuleName, CairoDockModule *pModule, GtkWidget *pModuleSubMenu)
{
	if (pModule->cConfFilePath != NULL)
	{
		GtkWidget *menu_item;
		menu_item = gtk_menu_item_new_with_label (cModuleName);
		gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_initiate_config_module_from_module), pModule);
	}
}
GtkWidget *cairo_dock_build_menu (Icon *icon, CairoDock *pDock)
{
	static gpointer *data = NULL;
	
	GtkWidget *menu = gtk_menu_new ();
	g_signal_connect (G_OBJECT (menu),
		"deactivate",
		G_CALLBACK (cairo_dock_delete_menu),
		pDock);
	
	if (data == NULL)
		data = g_new (gpointer, 3);
	data[0] = icon;
	data[1] = pDock;
	data[2] = menu;
	
	GtkWidget *menu_item, *image;
	menu_item = gtk_image_menu_item_new_with_label ("Cairo-Dock");
	gchar *cIconPath = g_strdup_printf ("%s/cairo-dock-icon.svg", CAIRO_DOCK_SHARE_DATA_DIR);
	image = gtk_image_new_from_file (cIconPath);
	g_free (cIconPath);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	GtkWidget *pSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenu);
	
	menu_item = gtk_image_menu_item_new_with_label (_("Configure"));
	image = gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	GtkWidget *pConfSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pConfSubMenu);
	
	menu_item = gtk_menu_item_new_with_label (_("All"));
	gtk_menu_shell_append  (GTK_MENU_SHELL (pConfSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_edit_and_reload_conf), data);
	
	menu_item = gtk_image_menu_item_new_with_label (_("Modules"));
	image = gtk_image_new_from_stock (GTK_STOCK_CONNECT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (pConfSubMenu), menu_item);
	GtkWidget *pModuleSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pModuleSubMenu);
	
	cairo_dock_foreach_module ((GHFunc)_cairo_dock_insert_module_in_menu, pModuleSubMenu);
	
	
	menu_item = gtk_image_menu_item_new_with_label (_("Manage themes"));
	image = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_initiate_theme_management), data);
	
	menu_item = gtk_image_menu_item_new_with_label (_("Check updates"));
	image = gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_update), data);
	
	menu_item = gtk_image_menu_item_new_with_label (_("About"));
	image = gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_about), data);
	
	menu_item = gtk_image_menu_item_new_with_label (_("Help"));
	image = gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_help), data);
	
	if (! g_bAutoHide)
	{
		menu_item = gtk_image_menu_item_new_with_label (_("Quick-Hide"));
		image = gtk_image_new_from_stock (GTK_STOCK_GOTO_BOTTOM, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_quick_hide), data);
	}
	
	menu_item = gtk_image_menu_item_new_with_label (_("Quit"));
	image = gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_quit), data);
	
	
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	
	cairo_dock_notify (CAIRO_DOCK_BUILD_MENU, data);
	
	return menu;
}


gboolean cairo_dock_notification_build_menu (gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	GtkWidget *menu = data[2];
	
	GtkWidget *menu_item, *image;
	if (icon == NULL)
	{
		menu_item = gtk_image_menu_item_new_with_label (_("Add a launcher"));
		image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_launcher), data);
		
		menu_item = gtk_image_menu_item_new_with_label (_("Add a sub-dock"));
		image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_container), data);
		
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	if (CAIRO_DOCK_IS_SEPARATOR (icon))
	{
		menu_item = gtk_image_menu_item_new_with_label (_("Add a launcher"));
		image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_launcher), data);
		
		menu_item = gtk_image_menu_item_new_with_label (_("Add a sub-dock"));
		image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_container), data);
	}
	else
	{
		if (CAIRO_DOCK_IS_LAUNCHER (icon))
		{
			menu_item = gtk_image_menu_item_new_with_label (_("Add a launcher"));
			image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_launcher), data);
			
			menu_item = gtk_image_menu_item_new_with_label (_("Add a sub-dock"));
			image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_container), data);
			
			menu_item = gtk_image_menu_item_new_with_label (_("Remove this launcher"));
			image = gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_remove_launcher), data);
			
			menu_item = gtk_image_menu_item_new_with_label (_("Modify this launcher"));
			image = gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_modify_launcher), data);
		}
		else if (CAIRO_DOCK_IS_VALID_APPLI (icon))
		{
			menu_item = gtk_image_menu_item_new_with_label (_("Close"));
			image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_close_appli), data);
			
			menu_item = gtk_image_menu_item_new_with_label (_("Minimize"));
			image = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_minimize_appli), data);
			
			gboolean bIsMaximized = cairo_dock_window_is_maximized (icon->Xid);
			menu_item = gtk_image_menu_item_new_with_label (bIsMaximized ? _("Unmaximize") : _("Maximize"));
			image = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_maximize_appli), data);
			
			
			menu_item = gtk_menu_item_new_with_label (_("Other actions"));
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			GtkWidget *pSubMenuOtherActions = gtk_menu_new ();
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenuOtherActions);
			
			gboolean bIsFullScreen = cairo_dock_window_is_fullscreen (icon->Xid);
			menu_item = gtk_menu_item_new_with_label (bIsFullScreen ? _("Not Fullscreen") : _("Fullscreen"));
			gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenuOtherActions), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_set_appli_fullscreen), data);
			
			menu_item = gtk_menu_item_new_with_label (_("Move to this desktop"));
			gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenuOtherActions), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_move_appli_to_current_desktop), data);
		}
		else if (CAIRO_DOCK_IS_VALID_APPLET (icon))  // on regarde si pModule != NULL de facon a le faire que pour l'icone qui detient effectivement le module.
		{
			menu_item = gtk_image_menu_item_new_with_label (_("Configure this module"));
			image = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_initiate_config_module), data);
			
			menu_item = gtk_image_menu_item_new_with_label (_("Remove this module"));
			image = gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_remove_module), data);
		}
		menu_item = gtk_image_menu_item_new_with_label (_("Move this icon"));
		
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		image = gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		GtkWidget *pSubMenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenu);
		
		menu_item = gtk_image_menu_item_new_with_label (_("To the left"));
		image = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_prev_icon), data);
		
		menu_item = gtk_image_menu_item_new_with_label (_("To the right"));
		image = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_next_icon), data);
		gtk_widget_show_all (menu);
		
		menu_item = gtk_image_menu_item_new_with_label (_("To the beginning"));
		image = gtk_image_new_from_stock (GTK_STOCK_GOTO_FIRST, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_move_icon_to_beginning), data);
		gtk_widget_show_all (menu);
		
		menu_item = gtk_image_menu_item_new_with_label (_("To the end"));
		image = gtk_image_new_from_stock (GTK_STOCK_GOTO_LAST, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_move_icon_to_end), data);
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
