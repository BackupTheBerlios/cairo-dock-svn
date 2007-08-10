/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


******************************************************************************/
#include <math.h>
#include <string.h>
#include <glib.h>
#include <cairo.h>
#include <pango/pango.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <glib/gstdio.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif


#include "cairo-dock-config.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-applications.h"
#include "cairo-dock-desktop-file-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-menu.h"

extern CairoDock *g_pMainDock;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;

extern double g_fAmplitude;
extern int g_iLabelSize;
extern gboolean g_bUseText;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern int g_iIconGap;

extern gchar *g_cConfFile;
extern gchar *g_cCairoDockDataDir;

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_iNbAnimationRounds;
extern gchar *g_cLabelPolice;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern int g_iNbStripes;

extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];
extern GHashTable *g_hModuleTable;

#ifdef HAVE_GLITZ
extern gboolean g_bUseGlitz;
extern glitz_drawable_format_t *gDrawFormat;
extern glitz_drawable_t* g_pGlitzDrawable;
extern glitz_format_t* g_pGlitzFormat;
#endif // HAVE_GLITZ


static void cairo_dock_edit_and_reload_conf_file (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	gboolean config_ok = cairo_dock_edit_conf_file (pDock->pWidget, g_cConfFile, "Configuration of Cairo-Dock", 400, 600);
	if (config_ok)
	{
		cairo_dock_read_conf_file (g_cConfFile, g_pMainDock);
	}
}

static void cairo_dock_about (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	gchar *cTitle = g_strdup_printf ("\nCairo-Dock (2007)\n version %s", CAIRO_DOCK_VERSION);
	GtkWidget *pDialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		cTitle);
	g_free (cTitle);
	
	gchar *cImagePath = g_strdup_printf ("%s/cairo-dock.svg", CAIRO_DOCK_SHARE_DATA_DIR);
	GtkWidget *pImage = gtk_image_new_from_file (cImagePath);
	gtk_message_dialog_set_image (GTK_MESSAGE_DIALOG (pDialog), pImage);
	
	GtkWidget *pLabel = gtk_label_new (NULL);
	gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
	gchar *cAboutText = g_strdup_printf ("<b>Original idea/first development :</b>\n  Mac Slow\n<b>Main developer :</b>\n  Fabounet (Fabrice Rey)\n\n<b>Themes :</b>\n  Fabounet\n  Chilperik\n\n<b>Translations :</b>\n\n<b>Contributors :</b>\n");
	gtk_label_set_markup (GTK_LABEL (pLabel), cAboutText);
	g_free (cAboutText);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pDialog)->vbox), pLabel);
	
	gtk_widget_show_all (pDialog);
	gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);
}

static void cairo_dock_help (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	system ("firefox http://doc.ubuntu-fr.org/gnome_dock");
}


static void cairo_dock_remove_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	gchar *question = g_strdup_printf ("You're about to remove this icon (%s) from the dock. Sure ?", icon->acName);
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		question);
	g_free (question);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
	{
		if (icon->acDesktopFileName != NULL)  // normallement impossible.
		{
			gchar *icon_path = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, icon->acDesktopFileName);
			g_remove (icon_path);
			g_free (icon_path);
		}
		
		if (icon->pSubDock != NULL)
		{
			gboolean bDestroyIcons = TRUE;
			dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"Do you want to re-dispatch the icons contained inside this container into the dock (otherwise they will be destroyed) ?");
			int answer = gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			if (answer == GTK_RESPONSE_YES)
				bDestroyIcons = FALSE;
			cairo_dock_destroy_dock (icon->pSubDock, icon->acName, (bDestroyIcons ? NULL : g_pMainDock), (bDestroyIcons ? NULL : CAIRO_DOCK_MAIN_DOCK_NAME));
			icon->pSubDock = NULL;
		}
		
		icon->fPersonnalScale = 1.0;
		if (pDock->iSidShrinkDown == 0)
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pDock);
	}
}

static void cairo_dock_create_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	//\___________________ On cree un fichier de lanceur avec des valeurs par defaut.
	GError *erreur = NULL;
	const gchar *cDockName = cairo_dock_search_dock_name (pDock);
	gchar *cNewDesktopFilePath = cairo_dock_add_desktop_file_from_path (NULL, cDockName, CAIRO_DOCK_LAST_ORDER, pDock, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while trying to create a new launcher : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	gchar *cNewDesktopFileName = g_path_get_basename (cNewDesktopFilePath);
	
	//\___________________ On ouvre automatiquement l'IHM pour permettre de modifier ses champs.
	gboolean config_ok = cairo_dock_edit_conf_file (pDock->pWidget, cNewDesktopFilePath, "Fill this launcher", 0, 0);
	if (config_ok)
	{
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cNewDesktopFileName, pCairoContext);
		
		cairo_dock_insert_icon_in_dock (pNewIcon, pDock, TRUE, TRUE);
		
		if (pDock->iSidShrinkDown == 0)
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pDock);
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
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	GtkWidget *pFileChooserDialog = gtk_file_chooser_dialog_new ("Choose or Create a launcher",
		GTK_WINDOW (pDock->pWidget),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_NEW,
		1,
		GTK_STOCK_OK,
		GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);
	
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (pFileChooserDialog),"/usr/share/app-install/");
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (pFileChooserDialog), TRUE);
	gtk_widget_show (pFileChooserDialog);
	
	int answer = gtk_dialog_run (GTK_DIALOG (pFileChooserDialog));
	if (answer == GTK_RESPONSE_OK)
	{
		GSList* selected_files = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (pFileChooserDialog));
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
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
			cairo_dock_add_desktop_file_from_path (cFilePath, cDockName, CAIRO_DOCK_LAST_ORDER, pDock, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
				continue;
			}
			
			cDesktopFileName = g_path_get_basename (cFilePath);
			pNewIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFileName, pCairoContext);
			
			cairo_dock_insert_icon_in_dock (pNewIcon, pDock, FALSE, TRUE);
			
			g_free (cFilePath);
		}
		g_slist_free (selected_files);
		cairo_destroy (pCairoContext);
		
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
		
		if (pDock->iSidShrinkDown == 0)
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pDock);
		gtk_widget_queue_draw (pDock->pWidget);
		
		gtk_widget_destroy (pFileChooserDialog);
	}
	else if (answer == 1)
	{
		gtk_widget_destroy (pFileChooserDialog);  // dans ce cas on ferme le selecteur avant.
		cairo_dock_create_launcher (menu_item, data);
	}
	else
	{
		gtk_widget_destroy (pFileChooserDialog);
	}
}

static void cairo_dock_modify_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, icon->acDesktopFileName);
	gboolean config_ok = cairo_dock_edit_conf_file (pDock->pWidget, cDesktopFilePath, "Modify this launcher", 300, 400);
	g_free (cDesktopFilePath);
	if (config_ok)
	{
		GError *erreur = NULL;
		cairo_dock_remove_icon_from_dock (pDock, icon);  // car sa position a pu changer.
		
		//\_____________ On recree l'icone.
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (icon->acDesktopFileName, pCairoContext);
		
		//\_____________ On redistribue les icones du sous-dock si l'icone n'est plus un container.
		if (icon->pSubDock != NULL)
		{
			if (pNewIcon->pSubDock == NULL)  // ce n'est plus un container.
			{
				gboolean bDestroyIcons = TRUE;
				GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					"Do you want to re-dispatch the icons contained inside this container into the dock (otherwise they will be destroyed) ?");
				int answer = gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);
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
		if (pNewContainer == NULL)
		{
			pNewContainer = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_MENU, pNewIcon->cParentDockName);
			pNewContainer->iRefCount --;
		}
		
		cairo_dock_insert_icon_in_dock (pNewIcon, pNewContainer, TRUE, FALSE);  // pour l'instant on n'empeche pas les bouclages.
		
		if (pDock != pNewContainer)
			cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
		
		cairo_dock_free_icon (icon);  // on le fait maintenant pour plus de surete.
		cairo_destroy (pCairoContext);
		gtk_widget_queue_draw (pDock->pWidget);
		if (pNewContainer != pDock)
			gtk_widget_queue_draw (pNewContainer->pWidget);
	}
}


static void cairo_dock_initiate_config_module (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	GError *erreur = NULL;
	cairo_dock_configure_module (icon->pModule, pDock, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
	}
}

static void cairo_dock_remove_module (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	gchar *question = g_strdup_printf ("You're about to remove this icon (%s) from the dock. Sure ?", icon->acName);
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pDock->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		question);
	g_free (question);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
	{
		cairo_dock_remove_icon_from_dock (pDock, icon);  // desactive le module.
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, pDock->icons, g_hModuleTable);
		//cairo_dock_free_icon (icon);
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
	}
}


static void cairo_dock_close_appli (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	if (icon->Xid > 0)
	{
		cairo_dock_close_xwindow (icon->Xid);
	}
}

static void cairo_dock_minimize_appli (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	if (icon->Xid > 0)
		cairo_dock_minimize_xwindow (icon->Xid);
}

static void cairo_dock_maximize_appli (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	if (icon->Xid > 0)
	{
		gboolean bIsMaximized = cairo_dock_window_is_maximized (icon->Xid);
		cairo_dock_maximize_xwindow (icon->Xid, ! bIsMaximized);
	}
}

static void cairo_dock_set_appli_fullscreen (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	if (icon->Xid > 0)
	{
		gboolean bIsFullScreen = cairo_dock_window_is_fullscreen (icon->Xid);
		cairo_dock_set_xwindow_fullscreen (icon->Xid, ! bIsFullScreen);
	}
}

static void cairo_dock_move_appli_to_current_desktop (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	if (icon->Xid > 0)
	{
		int iCurrentDesktop = cairo_dock_get_current_desktop ();
		g_print ("iCurrentDesktop : %d\n", iCurrentDesktop);
		cairo_dock_move_xwindow_to_nth_desktop (icon->Xid, iCurrentDesktop);
	}
}


static void cairo_dock_swap_with_prev_icon (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	Icon *pOtherIcon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pOtherIcon = (Icon*) ic->data;
		if (pOtherIcon == icon)
		{
			if (ic->prev == NULL)
				return ;
			Icon *prev_icon = ic->prev->data;
			cairo_dock_swap_icons (pDock, icon, prev_icon);
		}
	}
}

static void cairo_dock_swap_with_next_icon (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	Icon *pOtherIcon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pOtherIcon = (Icon*) ic->data;
		if (pOtherIcon == icon)
		{
			if (ic->next == NULL)
				return ;
			Icon *next_icon = ic->next->data;
			cairo_dock_swap_icons (pDock, icon, next_icon);
		}
	}
}

static void cairo_dock_swap_with_first_icon (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	Icon* pFirstIcon = cairo_dock_get_first_icon_of_type (pDock->icons, icon->iType);
	if (pFirstIcon != NULL && pFirstIcon != icon)
		cairo_dock_swap_icons (pDock, icon, pFirstIcon);
}

static void cairo_dock_swap_with_last_icon (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDock *pDock = data[0];
	Icon *icon = data[1];
	
	Icon* pLastIcon = cairo_dock_get_last_icon_of_type (pDock->icons, icon->iType);
	if (pLastIcon != NULL && pLastIcon != icon)
		cairo_dock_swap_icons (pDock, icon, pLastIcon);
}

static void cairo_dock_delete_menu (GtkMenuShell *menu, CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	pDock->bMenuVisible = FALSE;
	if (! pDock->bInside)
	{
		pDock->bInside = TRUE;
		on_leave_notify2 (pDock->pWidget,
			NULL,
			pDock);
	}
}


GtkWidget *cairo_dock_build_menu (CairoDock *pDock)
{
	static gpointer *data = NULL;
	
	GtkWidget *menu = gtk_menu_new ();
	g_signal_connect (G_OBJECT (menu),
		"deactivate",
		G_CALLBACK (cairo_dock_delete_menu),
		pDock);
	
	Icon *icon = cairo_dock_get_pointed_icon (pDock->icons);
	
	if (data == NULL)
		data = g_new (gpointer, 2);
	data[0] = pDock;
	data[1] = icon;
	
	GtkWidget *menu_item;
	menu_item = gtk_menu_item_new_with_label ("Cairo-Dock");
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	GtkWidget *pSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenu);
	
	menu_item = gtk_menu_item_new_with_label ("Configure Cairo-Dock");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_edit_and_reload_conf_file), data);
	
	menu_item = gtk_menu_item_new_with_label ("About");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_about), data);
	
	menu_item = gtk_menu_item_new_with_label ("Help");
	gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_help), data);
	
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	
	if (icon == NULL)
	{
		menu_item = gtk_menu_item_new_with_label ("Add a launcher");
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_launcher), data);
		return menu;
	}
	
	if (CAIRO_DOCK_IS_SEPARATOR (icon))
	{
		menu_item = gtk_menu_item_new_with_label ("Add a launcher");
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_launcher), data);
	}
	else
	{
		if (CAIRO_DOCK_IS_LAUNCHER (icon))
		{
			menu_item = gtk_menu_item_new_with_label ("Add a launcher");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_add_launcher), data);
			
			menu_item = gtk_menu_item_new_with_label ("Remove this launcher");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_remove_launcher), data);
			
			menu_item = gtk_menu_item_new_with_label ("Modify this launcher");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_modify_launcher), data);
		}
		else if (CAIRO_DOCK_IS_VALID_APPLI (icon))
		{
			menu_item = gtk_menu_item_new_with_label ("Close");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_close_appli), data);
			
			menu_item = gtk_menu_item_new_with_label ("Minimize");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_minimize_appli), data);
			
			gboolean bIsMaximized = cairo_dock_window_is_maximized (icon->Xid);
			menu_item = gtk_menu_item_new_with_label (bIsMaximized ? "Unmaximize" : "Maximize");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_maximize_appli), data);
			
			
			menu_item = gtk_menu_item_new_with_label ("Other actions");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			GtkWidget *pSubMenuOtherActions = gtk_menu_new ();
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenuOtherActions);
			
			gboolean bIsFullScreen = cairo_dock_window_is_fullscreen (icon->Xid);
			menu_item = gtk_menu_item_new_with_label (bIsFullScreen ? "Not Fullscreen" : "Fullscreen");
			gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenuOtherActions), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_set_appli_fullscreen), data);
			
			menu_item = gtk_menu_item_new_with_label ("Move to this desktop");
			gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenuOtherActions), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_move_appli_to_current_desktop), data);
		}
		else if (CAIRO_DOCK_IS_APPLET (icon))
		{
			menu_item = gtk_menu_item_new_with_label ("Configure this module");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_initiate_config_module), data);
			menu_item = gtk_menu_item_new_with_label ("Remove this module");
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_remove_module), data);
			
			if (icon->pMenu != NULL)
			{
				menu_item = gtk_menu_item_new_with_label (icon->pModule->cModuleName);
				gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
				
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), icon->pMenu);
			}
			/*if (icon->pMenuEntryList != NULL)
			{
				menu_item = gtk_menu_item_new_with_label (icon->pModule->cModuleName);
				gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
				
				GtkWidget *pModuleSubmenu = gtk_menu_new ();
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pModuleSubmenu);
				GList *pMenuEntry;
				CairoDockMenuEntry *pEntryData;
				for (pMenuEntry = icon->pMenuEntryList; pMenuEntry != NULL; pMenuEntry = pMenuEntry->next)
				{
					pEntryData = pMenuEntry->data;
					menu_item = gtk_menu_item_new_with_label (pEntryData->cLabel);
					gtk_menu_shell_append  (GTK_MENU_SHELL (pModuleSubmenu), menu_item);
					g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(pEntryData->pCallback), data);
				}
			}*/
			
		}
		menu_item = gtk_menu_item_new_with_label ("Move this icon");
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
		
		pSubMenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), pSubMenu);
		
		menu_item = gtk_menu_item_new_with_label ("To the left");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_prev_icon), data);
		
		menu_item = gtk_menu_item_new_with_label ("To the right");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_next_icon), data);
		gtk_widget_show_all (menu);
		
		menu_item = gtk_menu_item_new_with_label ("To the beginning");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_first_icon), data);
		gtk_widget_show_all (menu);
		
		menu_item = gtk_menu_item_new_with_label ("To the end");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pSubMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_last_icon), data);
	}
	
	
	return menu;
}
