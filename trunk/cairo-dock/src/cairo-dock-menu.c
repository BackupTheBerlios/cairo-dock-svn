/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gdk/gdkx.h>

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
#include "cairo-dock-file-manager.h"
#include "cairo-dock-log.h"
#include "cairo-dock-desklet.h"
#include "cairo-dock-applet-facility.h"
#include "cairo-dock-X-utilities.h"
#include "cairo-dock-menu.h"

#define CAIRO_DOCK_CONF_PANEL_WIDTH 800
#define CAIRO_DOCK_CONF_PANEL_HEIGHT 600
#define CAIRO_DOCK_LAUNCHER_PANEL_WIDTH 600
#define CAIRO_DOCK_LAUNCHER_PANEL_HEIGHT 350

extern CairoDock *g_pMainDock;
extern double g_fSubDockSizeRatio;

extern gboolean g_bUseSeparator;
extern gboolean g_bAutoHide;
extern gchar *g_cConfFile;
extern gchar *g_cEasyConfFile;
extern gchar *g_cCurrentLaunchersPath;

extern int g_iNbDesktops;
extern int g_iNbViewportX,g_iNbViewportY ;
extern int g_iScreenWidth[2], g_iScreenHeight[2];

static void cairo_dock_edit_and_reload_conf (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];

	cairo_dock_build_easy_conf_file (g_cConfFile, g_cEasyConfFile);

	if (cairo_dock_use_full_conf_file ())
		cairo_dock_edit_conf_file_full (GTK_WINDOW (pDock->pWidget), g_cConfFile, "Configuration of Cairo-Dock", CAIRO_DOCK_CONF_PANEL_WIDTH, CAIRO_DOCK_CONF_PANEL_HEIGHT, '\0', NULL, (CairoDockConfigFunc) cairo_dock_read_conf_file, g_pMainDock, NULL, cairo_dock_read_easy_conf_file, g_cEasyConfFile, _("Well, maybe not ..."), _("Do you want to know more ?"), NULL);
	else
		cairo_dock_edit_conf_file_full (GTK_WINDOW (pDock->pWidget), g_cEasyConfFile, "Configuration of Cairo-Dock", CAIRO_DOCK_CONF_PANEL_WIDTH, CAIRO_DOCK_CONF_PANEL_HEIGHT, '\0', NULL, (CairoDockConfigFunc) cairo_dock_read_easy_conf_file, g_pMainDock, NULL, (CairoDockConfigFunc) cairo_dock_read_conf_file, g_cConfFile, _("Do you want to know more ?"), _("Well, maybe not ..."), NULL);

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


static void _cairo_dock_add_about_page (GtkWidget *pNoteBook, const gchar *cPageLabel, const gchar *cAboutText)
{
	GtkWidget *pVBox, *pScrolledWindow;
	GtkWidget *pPageLabel, *pAboutLabel;
	
	pPageLabel = gtk_label_new (cPageLabel);
	pVBox = gtk_vbox_new (FALSE, 0);
	pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pVBox);
	gtk_notebook_append_page (GTK_NOTEBOOK (pNoteBook), pScrolledWindow, pPageLabel);
	
	pAboutLabel = gtk_label_new (NULL);
	gtk_label_set_use_markup (GTK_LABEL (pAboutLabel), TRUE);
	gtk_box_pack_start (GTK_BOX (pVBox),
		pAboutLabel,
		FALSE,
		FALSE,
		0);
	gtk_label_set_markup (GTK_LABEL (pAboutLabel), cAboutText);
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
	g_free (cImagePath);
#if GTK_MINOR_VERSION >= 12
	gtk_message_dialog_set_image (GTK_MESSAGE_DIALOG (pDialog), pImage);
#endif
	GtkWidget *pNoteBook = gtk_notebook_new ();
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (pNoteBook), TRUE);
	gtk_notebook_popup_enable (GTK_NOTEBOOK (pNoteBook));
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), pNoteBook);
	
	_cairo_dock_add_about_page (pNoteBook,
		_("Development"),
		"<b>Main developer :</b>\n  Fabounet (Fabrice Rey)\n\
<b>Original idea/first development :</b>\n  Mac Slow\n\
<b>Applets :</b>\n  Fabounet\n  Necropotame\n  Ctaf\n\
<b>Themes :</b>\n  Fabounet\n  Chilperik\n  Djoole\n  Glattering\n  Vilraleur\n  Lord Northam\n\
<b>Patchs :</b>\n  Robrob\n  Tshirtman\n  Ctaf\n\
<b>Translations :</b>\n  Fabounet\n  Ppmt \n  Jiro Kawada");
	
	_cairo_dock_add_about_page (pNoteBook,
		_("Support"),
		"<b>Installation scripts and repository :</b>\n  Mav\n\
<b>Site (cairo-dock.org) :</b>\n  Tdey\n  Necropotame\n\
<b>Suggestions/Comments/Bêta-Testers :</b>\n  AuraHxC\n  Chilperik\n  Cybergoll\n  Damster\n  Djoole\n  Glattering\n  Mav\n  Necropotame\n  Nochka85\n  Ppmt\n  Sombrero\n  Vilraleur");
	
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
	g_spawn_command_line_async ("firefox http://www.cairo-dock.org", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n  you can consult the wiki at http://www.cairo-dock.org\n", erreur->message);
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
	cd_debug ("%s", icon->acName);
	
	if (icon->pSubDock != NULL)
	{
		gboolean bDestroyIcons = TRUE;
		if (! CAIRO_DOCK_IS_URI_LAUNCHER (icon) && icon->pSubDock->icons != NULL)  // alors on propose de repartir les icones de son sous-dock dans le dock principal.
		{
			int answer = cairo_dock_ask_question_and_wait (_("Do you want to re-dispatch the icons contained inside this container into the dock ?\n (otherwise they will be destroyed)"), icon, CAIRO_DOCK_CONTAINER (pDock));
			g_return_val_if_fail (answer != GTK_RESPONSE_NONE, CAIRO_DOCK_LET_PASS_NOTIFICATION);
			if (answer == GTK_RESPONSE_YES)
				bDestroyIcons = FALSE;
		}
		cairo_dock_destroy_dock (icon->pSubDock, icon->acName, (bDestroyIcons ? NULL : g_pMainDock), (bDestroyIcons ? NULL : CAIRO_DOCK_MAIN_DOCK_NAME));
		icon->pSubDock = NULL;
	}
	
	icon->fPersonnalScale = 1.0;
	cairo_dock_start_animation (icon, pDock);
	
	cairo_dock_mark_theme_as_modified (TRUE);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;  // on l'intercepte car on ne peut plus garantir la validite de l'icone apres cela.
}
static void cairo_dock_remove_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];

	gchar *question = g_strdup_printf (_("You're about to remove this icon (%s) from the dock. Sure ?"), icon->acName);
	int answer = cairo_dock_ask_question_and_wait (question, icon, CAIRO_DOCK_CONTAINER (pDock));
	g_free (question);
	if (answer == GTK_RESPONSE_YES)
	{
		cairo_dock_notify (CAIRO_DOCK_REMOVE_ICON, data);
	}
}

static void _cairo_dock_create_launcher (GtkMenuItem *menu_item, gpointer *data, CairoDockNewLauncherType iLauncherType)
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
	if (iLauncherType == 1)  // conteneur.
		cNewDesktopFileName = cairo_dock_add_desktop_file_for_container (cDockName, fOrder, pDock, &erreur);
	else if (iLauncherType == 0)  // lanceur vide.
		cNewDesktopFileName = cairo_dock_add_desktop_file_from_uri (NULL, cDockName, fOrder, pDock, &erreur);
	else if (iLauncherType == 2)  // separateur.
		cNewDesktopFileName = cairo_dock_add_desktop_file_for_separator (cDockName, fOrder, pDock, &erreur);
	else
		return ;
	if (erreur != NULL)
	{
		cd_message ("Attention : while trying to create a new launcher : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}

	//\___________________ On ouvre automatiquement l'IHM pour permettre de modifier ses champs.
	gchar *cNewDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, cNewDesktopFileName);
	cairo_dock_update_launcher_desktop_file (cNewDesktopFilePath, iLauncherType);

	gboolean config_ok;
	if (iLauncherType != CAIRO_DOCK_LAUNCHER_FOR_SEPARATOR)  // inutile pour un separateur.
		config_ok = cairo_dock_edit_conf_file (GTK_WINDOW (pDock->pWidget), cNewDesktopFilePath, _("Fill this launcher"), CAIRO_DOCK_LAUNCHER_PANEL_WIDTH, CAIRO_DOCK_LAUNCHER_PANEL_HEIGHT, 0, NULL, NULL, NULL, NULL, NULL);
	else
		config_ok = TRUE;
	if (config_ok)
	{
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDock));
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cNewDesktopFileName, pCairoContext);

		if (iLauncherType = CAIRO_DOCK_LAUNCHER_FOR_SEPARATOR)
			pNewIcon->iType = icon->iType;  // pour une futur insertion de separateurs dans les applis ou les applets.

		CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
		cairo_dock_insert_icon_in_dock (pNewIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);

		if (pDock->iSidShrinkDown == 0)
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
		cairo_dock_mark_theme_as_modified (TRUE);
	}
	else
	{
		g_remove (cNewDesktopFilePath);
	}

	g_free (cNewDesktopFilePath);
	g_free (cNewDesktopFileName);
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
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDock));
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
				cd_message ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
				continue;
			}

			pNewIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFileName, pCairoContext);
			g_free (cDesktopFileName);

			cairo_dock_insert_icon_in_dock (pNewIcon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);

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
		_cairo_dock_create_launcher (menu_item, data, CAIRO_DOCK_LAUNCHER_FROM_DESKTOP_FILE);
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

	_cairo_dock_create_launcher (menu_item, data, CAIRO_DOCK_LAUNCHER_FOR_CONTAINER);
}

static void cairo_dock_add_separator (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (icon != NULL)
		_cairo_dock_create_launcher (menu_item, data, CAIRO_DOCK_LAUNCHER_FOR_SEPARATOR);
}

static void cairo_dock_modify_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];

	gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon->acDesktopFileName);

	cairo_dock_update_launcher_desktop_file (cDesktopFilePath, CAIRO_DOCK_IS_SEPARATOR (icon) ? CAIRO_DOCK_LAUNCHER_FOR_SEPARATOR : icon->pSubDock != NULL ? CAIRO_DOCK_LAUNCHER_FOR_CONTAINER : CAIRO_DOCK_LAUNCHER_FROM_DESKTOP_FILE);

	gboolean config_ok = cairo_dock_edit_conf_file (GTK_WINDOW (pDock->pWidget), cDesktopFilePath, _("Modify this launcher"), CAIRO_DOCK_LAUNCHER_PANEL_WIDTH, CAIRO_DOCK_LAUNCHER_PANEL_HEIGHT, 0, NULL, NULL, NULL, NULL, NULL);
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
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDock));
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

		cairo_dock_insert_icon_in_dock (pNewIcon, pNewContainer, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);  // on n'empeche pas les bouclages.

		if (pDock != pNewContainer)
			cairo_dock_update_dock_size (pDock);

		if (pNewIcon->pSubDock != NULL)
		{
			if (icon->pSubDock == NULL || (pNewIcon->pSubDock->cRendererName != NULL && icon->pSubDock->cRendererName == NULL) || (pNewIcon->pSubDock->cRendererName == NULL && icon->pSubDock->cRendererName != NULL) || (pNewIcon->pSubDock->cRendererName != NULL && icon->pSubDock->cRendererName != NULL && strcmp (pNewIcon->pSubDock->cRendererName, icon->pSubDock->cRendererName)))
				cairo_dock_update_dock_size (pNewIcon->pSubDock);
		}

		cairo_dock_free_icon (icon);  // on ne le fait que maintenant pour plus de surete.
		cairo_destroy (pCairoContext);
		pDock->calculate_icons (pDock);
		gtk_widget_queue_draw (pDock->pWidget);
		if (pNewContainer != pDock)
		{
			pNewContainer->calculate_icons (pNewContainer);
			gtk_widget_queue_draw (pNewContainer->pWidget);
		}
		cairo_dock_mark_theme_as_modified (TRUE);
	}
}

static void _cairo_dock_show_file_properties (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	//g_print ("%s (%s)\n", __func__, icon->acName);

	guint64 iSize = 0;
	time_t iLastModificationTime = 0;
	gchar *cMimeType = NULL;
	int iUID=0, iGID=0, iPermissionsMask=0;
	if (cairo_dock_fm_get_file_properties (icon->acCommand, &iSize, &iLastModificationTime, &cMimeType, &iUID, &iGID, &iPermissionsMask))
	{
		GtkWidget *pDialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			"Properties :");

		GString *sInfo = g_string_new ("");
		g_string_printf (sInfo, "<b>%s</b>", icon->acName);

		GtkWidget *pLabel= gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);

		GtkWidget *pFrame = gtk_frame_new (NULL);
		gtk_container_set_border_width (GTK_CONTAINER (pFrame), 3);
		gtk_frame_set_label_widget (GTK_FRAME (pFrame), pLabel);
		gtk_frame_set_shadow_type (GTK_FRAME (pFrame), GTK_SHADOW_OUT);
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pDialog)->vbox), pFrame);

		GtkWidget *pVBox = gtk_vbox_new (FALSE, 3);
		gtk_container_add (GTK_CONTAINER (pFrame), pVBox);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		g_string_printf (sInfo, "<u>Size</u> : %d bytes", iSize);
		if (iSize > 1024*1024)
			g_string_append_printf (sInfo, " (%.1f Mo)", 1. * iSize / 1024 / 1024);
		else if (iSize > 1024)
			g_string_append_printf (sInfo, " (%.1f Ko)", 1. * iSize / 1024);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		struct tm epoch_tm;
		localtime_r (&iLastModificationTime, &epoch_tm);  // et non pas gmtime_r.
		gchar *cTimeChain = g_new0 (gchar, 100);
		strftime (cTimeChain, 100, "%F, %T", &epoch_tm);
		g_string_printf (sInfo, "<u>Last Modification</u> : %s", cTimeChain);
		g_free (cTimeChain);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		if (cMimeType != NULL)
		{
			pLabel = gtk_label_new (NULL);
			gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
			g_string_printf (sInfo, "<u>Mime Type</u> : %s", cMimeType);
			gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
			gtk_container_add (GTK_CONTAINER (pVBox), pLabel);
		}

		GtkWidget *pSeparator = gtk_hseparator_new ();
		gtk_container_add (GTK_CONTAINER (pVBox), pSeparator);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		g_string_printf (sInfo, "<u>User ID</u> : %d / <u>Group ID</u> : %d", iUID, iGID);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		int iOwnerPermissions = iPermissionsMask >> 6;  // 8*8.
		int iGroupPermissions = (iPermissionsMask - (iOwnerPermissions << 6)) >> 3;
		int iOthersPermissions = (iPermissionsMask % 8);
		g_string_printf (sInfo, "<u>Permissions</u> : %d / %d / %d", iOwnerPermissions, iGroupPermissions, iOthersPermissions);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		gtk_widget_show_all (GTK_DIALOG (pDialog)->vbox);
		gtk_window_set_position (GTK_WINDOW (pDialog), GTK_WIN_POS_CENTER_ALWAYS);
		int answer = gtk_dialog_run (GTK_DIALOG (pDialog));
		gtk_widget_destroy (pDialog);

		g_string_free (sInfo, TRUE);
		g_free (cMimeType);
	}
}

static void _cairo_dock_mount_unmount (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	cd_message ("%s (%s)\n", __func__, icon->acName);

	gboolean bIsMounted = FALSE;
	gchar *cActivationURI = cairo_dock_fm_is_mounted (icon->acCommand, &bIsMounted);
	cd_message ("  cActivationURI : %s; bIsMounted : %d\n", cActivationURI, bIsMounted);
	g_free (cActivationURI);

	if (! bIsMounted)
	{
		cairo_dock_fm_mount (icon, pDock);
	}
	else
		cairo_dock_fm_unmount (icon, pDock);
}

static void _cairo_dock_delete_file (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	cd_message ("%s (%s)\n", __func__, icon->acName);

	gchar *question = g_strdup_printf (_("You're about to delete this file\n  (%s)\nfrom your hard-disk. Sure ?"), icon->acCommand);
	int answer = cairo_dock_ask_question_and_wait (question, icon, CAIRO_DOCK_CONTAINER (pDock));
	g_free (question);
	if (answer == GTK_RESPONSE_YES)
	{
		gboolean bSuccess = cairo_dock_fm_delete_file (icon->acCommand);
		if (! bSuccess)
		{
			cd_message ("Attention : couldn't delete this file.\nCheck that you have writing rights on this file.\n");
			gchar *cMessage = g_strdup_printf (_("Attention : couldn't delete this file.\nCheck that you have writing rights on it."));
			cairo_dock_show_temporary_dialog_with_default_icon (cMessage, icon, CAIRO_DOCK_CONTAINER (pDock), 4000);
			g_free (cMessage);
		}
		cairo_dock_remove_icon_from_dock (pDock, icon);
		cairo_dock_update_dock_size (pDock);

		if (icon->acDesktopFileName != NULL)
		{
			gchar *icon_path = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon->acDesktopFileName);
			g_remove (icon_path);
			g_free (icon_path);
		}

		cairo_dock_free_icon (icon);
	}
}

static void _cairo_dock_rename_file (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	cd_message ("%s (%s)\n", __func__, icon->acName);

	gchar *cNewName = cairo_dock_show_demand_and_wait (_("Rename to :"), icon, CAIRO_DOCK_CONTAINER (pDock), icon->acName);
	if (cNewName != NULL && *cNewName != '\0')
	{
		gboolean bSuccess = cairo_dock_fm_rename_file (icon->acCommand, cNewName);
		if (! bSuccess)
		{
			cd_message ("Attention : couldn't rename this file.\nCheck that you have writing rights, and that the new name does not already exist.\n");
			gchar *cMessage = g_strdup_printf (_("Attention : couldn't rename %s.\nCheck that you have writing rights,\n and that the new name does not already exist."), icon->acCommand);
			cairo_dock_show_temporary_dialog (cMessage, icon, CAIRO_DOCK_CONTAINER (pDock), 5000);
			g_free (cMessage);
		}
	}
	g_free (cNewName);
	//gtk_widget_destroy (pDialog);  // il faut le faire ici et pas avant, pour garder la GtkEntry.
}




static void cairo_dock_initiate_config_module_from_module (GtkMenuItem *menu_item, CairoDockModule *pModule)
{
	GError *erreur = NULL;
	cairo_dock_configure_module (GTK_WINDOW (g_pMainDock->pWidget), pModule, g_pMainDock, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
	}
}
static void cairo_dock_initiate_config_module (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDockContainer *pContainer= data[1];
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
		icon = (CAIRO_DOCK_DESKLET (pContainer))->pIcon;  // l'icone cliquee du desklet n'est pas forcement celle qui contient le module !
	
	GError *erreur = NULL;
	cairo_dock_configure_module (GTK_WINDOW (pContainer->pWidget), icon->pModule, g_pMainDock, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
	}
}

static void cairo_dock_detach_module (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDockContainer *pContainer= data[1];
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
		icon = (CAIRO_DOCK_DESKLET (pContainer))->pIcon;  // l'icone cliquee du desklet n'est pas forcement celle qui contient le module !

	if (icon->pModule != NULL)
	{
		cairo_dock_update_conf_file (icon->pModule->cConfFilePath,
			G_TYPE_BOOLEAN, "Desklet", "initially detached", CAIRO_DOCK_IS_DOCK (pContainer),
			G_TYPE_INVALID);

		cairo_dock_reload_module (icon->pModule, TRUE);
	}
}

static void cairo_dock_remove_module (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDockContainer *pContainer= data[1];
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
		icon = (CAIRO_DOCK_DESKLET (pContainer))->pIcon;  // l'icone cliquee du desklet n'est pas forcement celle qui contient le module !

	gchar *question = g_strdup_printf (_("You're about to remove this module (%s) from the dock. Sure ?"), icon->pModule->pVisitCard->cModuleName);
	int answer = cairo_dock_ask_question_and_wait (question, icon, CAIRO_DOCK_CONTAINER (pContainer));
	if (answer == GTK_RESPONSE_YES)
	{
		cairo_dock_deactivate_module_and_unload (icon->pModule->pVisitCard->cModuleName);
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
		int iCurrentDesktop = cairo_dock_get_current_desktop ();
		cairo_dock_move_xwindow_to_nth_desktop (icon->Xid, iCurrentDesktop, 0, 0);  // on ne veut pas decaler son viewport par rapport a nous.
	}
}

static void cairo_dock_move_appli_to_desktop (GtkMenuItem *menu_item, gpointer *user_data)
{
	gpointer *data = user_data[0];
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	int iDesktopNumber = GPOINTER_TO_INT (user_data[1]);
	int iViewPortNumberY = GPOINTER_TO_INT (user_data[2]);
	int iViewPortNumberX = GPOINTER_TO_INT (user_data[3]);
	g_print ("%s (%d;%d;%d)\n", __func__, iDesktopNumber, iViewPortNumberX, iViewPortNumberY);
	if (icon->Xid > 0)
	{
		int iCurrentDesktopNumber = cairo_dock_get_window_desktop (icon->Xid);
		
		int iCurrentViewPortX, iCurrentViewPortY;
		cairo_dock_get_current_viewport (&iCurrentViewPortX, &iCurrentViewPortY);
		g_print (" current_viewport : %d;%d\n", iCurrentViewPortX, iCurrentViewPortY);
		
		cairo_dock_move_xwindow_to_nth_desktop (icon->Xid, iDesktopNumber, iViewPortNumberX * g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] - iCurrentViewPortX, iViewPortNumberY * g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] - iCurrentViewPortY);
	}
}

static void cairo_dock_change_window_above (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (icon->Xid > 0)
	{
		gboolean bIsAbove=FALSE, bIsBelow=FALSE;
		cairo_dock_window_is_above_or_below (icon->Xid, &bIsAbove, &bIsBelow);
		cairo_dock_set_xwindow_above (icon->Xid, ! bIsAbove);
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

static void _cairo_dock_keep_window_in_state (gpointer *data, gboolean bAbove, gboolean bBelow)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];

	gtk_window_set_keep_below(GTK_WINDOW(pDock->pWidget), bBelow);
	gtk_window_set_keep_above(GTK_WINDOW(pDock->pWidget), bAbove);
	if (CAIRO_DOCK_IS_VALID_APPLET (icon))
		cairo_dock_update_conf_file (icon->pModule->cConfFilePath,
			G_TYPE_BOOLEAN, "Desklet", "keep below", bBelow,
			G_TYPE_BOOLEAN, "Desklet", "keep above", bAbove,
			G_TYPE_INVALID);
}
static void cairo_dock_keep_above(GtkCheckMenuItem *menu_item, gpointer *data)
{
	cd_debug ("");
	if (gtk_check_menu_item_get_active (menu_item))
		_cairo_dock_keep_window_in_state (data, TRUE, FALSE);
}

static void cairo_dock_keep_normal(GtkCheckMenuItem *menu_item, gpointer *data)
{
	cd_debug ("");
	if (gtk_check_menu_item_get_active (menu_item))
		_cairo_dock_keep_window_in_state (data, FALSE, FALSE);
}

static void cairo_dock_keep_below(GtkCheckMenuItem *menu_item, gpointer *data)
{
	cd_debug ("");
	if (gtk_check_menu_item_get_active (menu_item))
		_cairo_dock_keep_window_in_state (data, FALSE, TRUE);
}

//for compiz fusion "widget layer"
//set behaviour in compiz to: (name=cairo-dock & type=utility)
static void cairo_dock_keep_on_widget_layer (GtkMenuItem *menu_item, gpointer *data)
{
	Icon *icon = data[0];
	CairoDockDesklet *pDesklet = data[1];

	cairo_dock_hide_desklet (pDesklet);
	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);

	gboolean bOnCompizWidgetLayer = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menu_item));
	cd_debug (" bOnCompizWidgetLayer : %d", bOnCompizWidgetLayer);
	if (bOnCompizWidgetLayer)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_UTILITY");
		//gtk_window_set_type_hint(GTK_WINDOW(pDock->pWidget), GDK_WINDOW_TYPE_HINT_UTILITY);
	else
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
		//gtk_window_set_type_hint(GTK_WINDOW(pDock->pWidget), GDK_WINDOW_TYPE_HINT_NORMAL);
	cairo_dock_show_desklet (pDesklet);

	if (CAIRO_DOCK_IS_VALID_APPLET (icon))
		cairo_dock_update_conf_file (icon->pModule->cConfFilePath,
			G_TYPE_BOOLEAN, "Desklet", "on widget layer", bOnCompizWidgetLayer,
			G_TYPE_INVALID);
}

static void cairo_dock_delete_menu (GtkMenuShell *menu, CairoDock *pDock)
{
	cd_debug ("");
	pDock->bMenuVisible = FALSE;
	if (CAIRO_DOCK_IS_DOCK (pDock) && ! pDock->bInside)
	{
		cd_message ("on force a quitter\n");
		pDock->bInside = TRUE;
		pDock->bAtBottom = FALSE;
		///cairo_dock_disable_entrance ();  // trop violent, il faudrait trouver un autre truc.
		on_leave_notify2 (pDock->pWidget,
			NULL,
			pDock);
	}
}


#define _add_entry_in_menu(cLabel, gtkStock, pSubMenu, pCallBack) CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, gtkStock, pSubMenu, pCallBack, data)

GtkWidget *cairo_dock_build_menu (Icon *icon, CairoDockContainer *pContainer)
{
	static gpointer *data = NULL;

	//\_________________________ On construit le menu et les donnees passees en callback.
	GtkWidget *menu = gtk_menu_new ();
	
	if (CAIRO_DOCK_IS_DOCK (pContainer))
		g_signal_connect (G_OBJECT (menu),
			"deactivate",
			G_CALLBACK (cairo_dock_delete_menu),
			pContainer);

	if (data == NULL)
		data = g_new (gpointer, 3);
	data[0] = icon;
	data[1] = pContainer;
	data[2] = menu;

	//\_________________________ On ajoute le sous-menu Cairo-Dock, toujours present.
	GtkWidget *menu_item, *image;
	menu_item = gtk_image_menu_item_new_with_label ("Cairo-Dock");
	gchar *cIconPath = g_strdup_printf ("%s/cairo-dock-icon.svg", CAIRO_DOCK_SHARE_DATA_DIR);
	image = gtk_image_new_from_file (cIconPath);
	g_free (cIconPath);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);

	GtkWidget *pSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenu);

	_add_entry_in_menu (_("Configure"), GTK_STOCK_PREFERENCES, cairo_dock_edit_and_reload_conf, pSubMenu);

	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (pSubMenu), menu_item);

	_add_entry_in_menu (_("Manage themes"), GTK_STOCK_EXECUTE, cairo_dock_initiate_theme_management, pSubMenu);

	_add_entry_in_menu (_("Check updates"), GTK_STOCK_REFRESH, cairo_dock_update, pSubMenu);

	_add_entry_in_menu (_("About"), GTK_STOCK_ABOUT, cairo_dock_about, pSubMenu);

	_add_entry_in_menu (_("Help"), GTK_STOCK_HELP, cairo_dock_help, pSubMenu);

	if (! g_bAutoHide)
	{
		_add_entry_in_menu (_("Quick-Hide"), GTK_STOCK_GOTO_BOTTOM, cairo_dock_quick_hide, pSubMenu);
	}

	_add_entry_in_menu (_("Quit"), GTK_STOCK_QUIT, cairo_dock_quit, pSubMenu);

	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);

	//\_________________________ On passe la main a ceux qui veulent y rajouter des choses.
	cairo_dock_notify (CAIRO_DOCK_BUILD_MENU, data);

	return menu;
}


gboolean cairo_dock_notification_build_menu (gpointer *data)
{
	Icon *icon = data[0];
	CairoDockContainer *pContainer = data[1];
	GtkWidget *menu = data[2];
	GtkWidget *menu_item, *image;
	static gpointer *pDesktopData = NULL;

	//\_________________________ On construit un sous-menu pour deplacer l'icone.
	if (CAIRO_DOCK_IS_DOCK (pContainer) && icon != NULL && ! CAIRO_DOCK_IS_AUTOMATIC_SEPARATOR (icon))
	{
		menu_item = gtk_image_menu_item_new_with_label (_("Move this icon"));

		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		image = gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
		GtkWidget *pSubMenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenu);

		_add_entry_in_menu (_("To the left"), GTK_STOCK_GO_BACK, cairo_dock_swap_with_prev_icon, pSubMenu);
		
		_add_entry_in_menu (_("To the right"), GTK_STOCK_GO_FORWARD, cairo_dock_swap_with_next_icon, pSubMenu);

		_add_entry_in_menu (_("To the beginning"), GTK_STOCK_GOTO_FIRST, cairo_dock_move_icon_to_beginning, pSubMenu);
		
		_add_entry_in_menu (_("To the end"), GTK_STOCK_GOTO_LAST, cairo_dock_move_icon_to_end, pSubMenu);

		menu_item = gtk_separator_menu_item_new ();
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	}

	//\_________________________ Si pas d'icone dans un dock, on s'arrete la.
	if (CAIRO_DOCK_IS_DOCK (pContainer) && (icon == NULL || CAIRO_DOCK_IS_AUTOMATIC_SEPARATOR (icon)))
	{
		_add_entry_in_menu (_("Add a launcher"), GTK_STOCK_ADD, cairo_dock_add_launcher, menu);
		
		_add_entry_in_menu (_("Add a sub-dock"), GTK_STOCK_ADD, cairo_dock_add_container, menu);
		
		if (icon != NULL)
		{
			_add_entry_in_menu (_("Add a separator"), GTK_STOCK_ADD, cairo_dock_add_separator, menu);
		}

		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}

	//\_________________________ On rajoute des actions suivant le type de l'icone.
	if (CAIRO_DOCK_IS_LAUNCHER (icon) || CAIRO_DOCK_IS_USER_SEPARATOR (icon))
	{
		//\_________________________ On rajoute les actions sur les icones de fichiers.
		if (CAIRO_DOCK_IS_URI_LAUNCHER (icon))
		{
			if (icon->iVolumeID > 0)
			{
				gboolean bIsMounted = FALSE;
				gchar *cActivationURI = cairo_dock_fm_is_mounted  (icon->acCommand, &bIsMounted);
				cd_message ("  cActivationURI : %s; bIsMounted : %d\n", cActivationURI, bIsMounted);
				g_free (cActivationURI);

				menu_item = gtk_menu_item_new_with_label (bIsMounted ? "Unmount" : "Mount");
				gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
				g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_cairo_dock_mount_unmount), data);
			}
			else
			{
				menu_item = gtk_menu_item_new_with_label ("Delete this file");
				gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
				g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_cairo_dock_delete_file), data);

				menu_item = gtk_menu_item_new_with_label ("Rename this file");
				gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
				g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_cairo_dock_rename_file), data);

				menu_item = gtk_menu_item_new_with_label ("Properties");
				gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
				g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_cairo_dock_show_file_properties), data);
			}
			if (icon->acDesktopFileName == NULL)  // un lanceur sans fichier .desktop, on est donc dans un sous-dock ou l'on ne peut pas faire d'autre action.
				return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		}
		
		//\_________________________ On rajoute des actions de modifications sur le dock.
		if (CAIRO_DOCK_IS_DOCK (pContainer))
		{
			_add_entry_in_menu (_("Add a launcher"), GTK_STOCK_ADD, cairo_dock_add_launcher, menu);
	
			_add_entry_in_menu (_("Add a sub-dock"), GTK_STOCK_ADD, cairo_dock_add_container, menu);
			
			if (icon != NULL)
			{
				_add_entry_in_menu (_("Add a separator"), GTK_STOCK_ADD, cairo_dock_add_separator, menu);
			}
			
			menu_item = gtk_separator_menu_item_new ();
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	
			_add_entry_in_menu (_("Remove this launcher"), GTK_STOCK_REMOVE, cairo_dock_remove_launcher, menu);
	
			_add_entry_in_menu (_("Modify this launcher"), GTK_STOCK_EDIT, cairo_dock_modify_launcher, menu);
		}
	}
	if (CAIRO_DOCK_IS_VALID_APPLI (icon))
	{
		//\_________________________ On rajoute les actions supplementaires sur les icones d'applis.
		menu_item = gtk_menu_item_new_with_label (_("Other actions"));
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		GtkWidget *pSubMenuOtherActions = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenuOtherActions);
		
		gboolean bIsFullScreen = cairo_dock_window_is_fullscreen (icon->Xid);
		_add_entry_in_menu (bIsFullScreen ? _("Not Fullscreen") : _("Fullscreen"), bIsFullScreen ? GTK_STOCK_LEAVE_FULLSCREEN : GTK_STOCK_FULLSCREEN, cairo_dock_set_appli_fullscreen, pSubMenuOtherActions);
		
		_add_entry_in_menu (_("Move to this desktop"), GTK_STOCK_JUMP_TO, cairo_dock_move_appli_to_current_desktop, pSubMenuOtherActions);
		
		gboolean bIsAbove=FALSE, bIsBelow=FALSE;
		cairo_dock_window_is_above_or_below (icon->Xid, &bIsAbove, &bIsBelow);
		_add_entry_in_menu (bIsAbove ? _("Don't keep above") : _("Keep above"), bIsAbove ? GTK_STOCK_GOTO_BOTTOM : GTK_STOCK_GOTO_TOP, cairo_dock_change_window_above, pSubMenuOtherActions);
		g_print ("g_iNbDesktops : %d ; g_iNbViewportX : %d ; g_iNbViewportY : %d\n", g_iNbDesktops, g_iNbViewportX, g_iNbViewportY);
		if (g_iNbDesktops > 1 || g_iNbViewportX > 1 || g_iNbViewportY > 1)
		{
			int i, j, k, iDesktopCode;
			const gchar *cLabel;
			if (g_iNbDesktops > 1 && (g_iNbViewportX > 1 || g_iNbViewportY > 1))
				cLabel = _("Move to desktop %d - face %d");
			else if (g_iNbDesktops > 1)
				cLabel = _("Move to desktop %d");
			else
				cLabel = _("Move to face %d");
			GString *sDesktop = g_string_new ("");
			g_free (pDesktopData);
			pDesktopData = g_new0 (gpointer, 4 * g_iNbDesktops * g_iNbViewportX * g_iNbViewportY);
			gpointer *user_data;
			
			for (i = 0; i < g_iNbDesktops; i ++)  // on range par bureau.
			{
				for (j = 0; j < g_iNbViewportY; j ++)  // puis par rangee.
				{
					for (k = 0; k < g_iNbViewportX; k ++)
					{
						if (g_iNbDesktops > 1 && (g_iNbViewportX > 1 || g_iNbViewportY > 1))
							g_string_printf (sDesktop, cLabel, i+1, j*g_iNbViewportX+k+1);
						else if (g_iNbDesktops > 1)
							g_string_printf (sDesktop, cLabel, i+1);
						else
							g_string_printf (sDesktop, cLabel, j*g_iNbViewportX+k+1);
						iDesktopCode = i * g_iNbViewportY * g_iNbViewportX + j * g_iNbViewportY + k;
						user_data = &pDesktopData[4*iDesktopCode];
						user_data[0] = data;
						user_data[1] = GINT_TO_POINTER (i);
						user_data[2] = GINT_TO_POINTER (j);
						user_data[3] = GINT_TO_POINTER (k);
						
						CD_APPLET_ADD_IN_MENU_WITH_STOCK (sDesktop->str, NULL, cairo_dock_move_appli_to_desktop, pSubMenuOtherActions, user_data);
					}
				}
			}
			g_string_free (sDesktop, TRUE);
		}
		
		//\_________________________ On rajoute les actions courantes sur les icones d'applis.
		menu_item = gtk_separator_menu_item_new ();
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		gboolean bIsMaximized = cairo_dock_window_is_maximized (icon->Xid);
		_add_entry_in_menu (bIsMaximized ? _("Unmaximize") : _("Maximize"), GTK_STOCK_GO_UP, cairo_dock_maximize_appli, menu);

		_add_entry_in_menu (_("Minimize"), GTK_STOCK_GO_DOWN, cairo_dock_minimize_appli, menu);

		_add_entry_in_menu (_("Close"), GTK_STOCK_CLOSE, cairo_dock_close_appli, menu);
	}
	else if (CAIRO_DOCK_IS_VALID_APPLET (icon) || CAIRO_DOCK_IS_DESKLET (pContainer))  // on regarde si pModule != NULL de facon a le faire que pour l'icone qui detient effectivement le module.
	{
		//\_________________________ On rajoute les actions propres a un module.
		_add_entry_in_menu (_("Configure this module"), GTK_STOCK_PROPERTIES, cairo_dock_initiate_config_module, menu);

		if ((CAIRO_DOCK_IS_VALID_APPLET (icon) && icon->pModule->bCanDetach) || (CAIRO_DOCK_IS_DESKLET (pContainer) && CAIRO_DOCK_DESKLET (pContainer)->pIcon->pModule->bCanDetach))
		{
			_add_entry_in_menu (CAIRO_DOCK_IS_DOCK (pContainer) ? _("Detach this module") : _("Return to dock"), CAIRO_DOCK_IS_DOCK (pContainer) ? GTK_STOCK_DISCONNECT : GTK_STOCK_CONNECT, cairo_dock_detach_module, menu);
		}

		_add_entry_in_menu (_("Remove this module"), GTK_STOCK_REMOVE, cairo_dock_remove_module, menu);
	}

	//\_________________________ On rajoute les actions de positionnement d'un desklet.
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
	{
		menu_item = gtk_separator_menu_item_new ();
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		GSList *group = NULL;

		//GdkWindowState iState = gdk_window_get_state (pContainer->pWidget->window);  // bugue.
		gboolean bIsAbove=FALSE, bIsBelow=FALSE;
		Window Xid = GDK_WINDOW_XID (pContainer->pWidget->window);
		cd_debug ("Xid : %d", Xid);
		cairo_dock_window_is_above_or_below (Xid, &bIsAbove, &bIsBelow);
		cd_debug (" -> %d;%d", bIsAbove, bIsBelow);

		menu_item = gtk_radio_menu_item_new_with_label(group, _("Always on top"));
		group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
		if (bIsAbove)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);
		g_signal_connect(G_OBJECT(menu_item), "toggled", G_CALLBACK(cairo_dock_keep_above), data);

		menu_item = gtk_radio_menu_item_new_with_label(group, _("Normal"));
		group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
		if (! bIsAbove && ! bIsBelow)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);
		g_signal_connect(G_OBJECT(menu_item), "toggled", G_CALLBACK(cairo_dock_keep_normal), data);

		menu_item = gtk_radio_menu_item_new_with_label(group, _("Always below"));
		group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
		if (bIsBelow)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);
		g_signal_connect(G_OBJECT(menu_item), "toggled", G_CALLBACK(cairo_dock_keep_below), data);

		menu_item = gtk_separator_menu_item_new ();
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		menu_item = gtk_check_menu_item_new_with_label("Compiz Fusion Widget");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
		if (cairo_dock_window_is_utility (Xid))  // gtk_window_get_type_hint me renvoie toujours 0 !
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);
		g_signal_connect(G_OBJECT(menu_item), "toggled", G_CALLBACK(cairo_dock_keep_on_widget_layer), data);
		
		GtkTooltips *pToolTipsGroup = gtk_tooltips_new ();
		gtk_tooltips_set_tip (GTK_TOOLTIPS (pToolTipsGroup),
			menu_item,
			_("set behaviour in Compiz to: (name=cairo-dock & type=utility)"),
			"pouet");
	}

	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
