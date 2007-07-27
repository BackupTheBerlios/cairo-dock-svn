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
#include "cairo-dock-menu.h"


extern GList* icons;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern gint g_iCurrentWidth;
extern gint g_iCurrentHeight;

extern float g_fMagnitude;
extern double g_fAmplitude;
extern int g_iLabelSize;
extern gboolean g_bUseText;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bAutoHide;
extern int g_iIconGap;
extern int g_iMaxIconHeight;

extern gboolean g_bAtBottom;
extern gboolean g_bAtTop;
extern gboolean g_bInside;
extern gchar *g_cConfFile;
extern gchar *g_cCairoDockDataDir;

extern gint g_iWindowPositionX;
extern gint g_iWindowPositionY;

extern gdouble g_fGradientOffsetX;

extern int g_iMaxDockWidth;
extern int g_iMaxDockHeight;
extern int g_iMinDockWidth;
extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_iGapX;
extern int g_iGapY;
extern int g_iNbAnimationRounds;
extern gchar *g_cLabelPolice;

extern int g_iSidMoveDown;
extern int g_iSidMoveUp;
extern int g_iSidGrowUp;
extern int g_iSidShrinkDown;
extern gboolean g_bMenuVisible;
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
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	gboolean config_ok = cairo_dock_edit_conf_file (pWidget, g_cConfFile, "Configuration of Cairo-Dock");
	if (config_ok)
	{
		cairo_dock_read_conf_file (pWidget, g_cConfFile);
	}
}


static void cairo_dock_remove_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	gchar *question = g_strdup_printf ("You're about to remove this icon (%s) from the dock. Sure ?", icon->acName);
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
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
		icon->fPersonnalScale = 1.0;
		if (g_iSidShrinkDown == 0)
			g_iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pWidget);
		
	}
}

static void cairo_dock_create_launcher (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	//\___________________ On cree le texte qu'on va y mettre par defaut.
	gchar *cDesktopContent = g_strdup ("#!\n[Desktop Entry]\n\
#s Image's name or path :\n\
Icon = \n\
#s Launcher's name :\n\
Name[fr] = Nouveau Lanceur\n\
Name[en] = New Launcher\n\
#s Exec command :\n\
Exec = echo 'edit me !'\n\
#i Order you want for this launcher among the other launchers :\n\
Order = 99\n");
	
	//\___________________ On lui choisit un nom de fichier tel qu'il n'y ait pas de collision.
	gchar *cNewDesktopFileName = cairo_dock_generate_desktop_filename (g_cCairoDockDataDir);
	
	//\___________________ On ecrit tout dedans.
	gchar *cNewDesktopFilePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, cNewDesktopFileName);
	GError *erreur = NULL;
	gboolean bWritingOk = g_file_set_contents (cNewDesktopFilePath,
		cDesktopContent,
		-1,
		&erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : while writing new launcher desktop file : %s\n", erreur->message);
		g_error_free (erreur);
		g_free (cNewDesktopFileName);
		g_free (cNewDesktopFilePath);
		return ;
	}
	
	//\___________________ On ouvre automatiquement l'IHM pour permettre de modifier ses champs.
	gboolean config_ok = cairo_dock_edit_conf_file (pWidget, cNewDesktopFilePath, "Fill this launcher");
	if (config_ok)
	{
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cNewDesktopFileName, pCairoContext);
		
		cairo_dock_insert_icon_in_list (pNewIcon, pWidget, TRUE, TRUE);
		
		if (g_iSidShrinkDown == 0)
			g_iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pWidget);
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
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	GtkWidget *pFileChooserDialog = gtk_file_chooser_dialog_new ("Choose or Create a launcher",
		GTK_WINDOW (pWidget),
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
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
		gchar *cFilePath;
		Icon *pNewIcon;
		gchar *cDesktopFileName;
		GError *erreur = NULL;
		GSList *pSelectedFile;
		for (pSelectedFile = selected_files; pSelectedFile != NULL; pSelectedFile = pSelectedFile->next)
		{
			cFilePath = pSelectedFile->data;
			cairo_dock_add_desktop_file_from_path (cFilePath, CAIRO_DOCK_LAST_ORDER, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
				continue;
			}
			
			cDesktopFileName = g_path_get_basename (cFilePath);
			pNewIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFileName, pCairoContext);
			
			cairo_dock_insert_icon_in_list (pNewIcon, pWidget, FALSE, TRUE);
			
			g_free (cFilePath);
		}
		g_slist_free (selected_files);
		cairo_destroy (pCairoContext);
		
		cairo_dock_update_dock_size (pWidget, g_iMaxIconHeight, g_iMinDockWidth);
		
		if (g_iSidShrinkDown == 0)
			g_iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pWidget);
		gtk_widget_queue_draw (pWidget);
		
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
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, icon->acDesktopFileName);
	gboolean config_ok = cairo_dock_edit_conf_file (pWidget, cDesktopFilePath, "Modify this launcher");
	g_free (cDesktopFilePath);
	if (config_ok)
	{
		GError *erreur = NULL;
		cairo_dock_remove_icon_from_dock (icon);  // car sa position a pu changer.
		
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (icon->acDesktopFileName, pCairoContext);
		
		cairo_dock_insert_icon_in_list (pNewIcon, pWidget, TRUE, FALSE);
		
		cairo_dock_free_icon (icon);  // on le fait maintenant por plus de surete.
		cairo_destroy (pCairoContext);
		gtk_widget_queue_draw (pWidget);
	}
}


static void cairo_dock_initiate_config_module (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	GError *erreur = NULL;
	cairo_dock_configure_module (icon->pModule, pWidget, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
	}
}

static void cairo_dock_remove_module (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	gchar *question = g_strdup_printf ("You're about to remove this icon (%s) from the dock. Sure ?", icon->acName);
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		question);
	g_free (question);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
	{
		cairo_dock_remove_icon_from_dock (icon);  // desactive le module.
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, g_hModuleTable);
		//cairo_dock_free_icon (icon);
		cairo_dock_update_dock_size (pWidget, g_iMaxIconHeight, g_iMinDockWidth);
	}
}


static void cairo_dock_close_appli (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	if (icon->Xid > 0)
	{
		cairo_dock_close_xwindow (icon->Xid);
	}
}

static void cairo_dock_minimize_appli (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	if (icon->Xid > 0)
		cairo_dock_minimize_xwindow (icon->Xid);
}

static void cairo_dock_maximize_appli (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	if (icon->Xid > 0)
	{
		gboolean bIsMaximized = cairo_dock_window_is_maximized (icon->Xid);
		cairo_dock_maximize_xwindow (icon->Xid, ! bIsMaximized);
	}
}

static void cairo_dock_set_appli_fullscreen (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
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
	GtkWidget* pWidget = data[0];
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
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	Icon *pOtherIcon;
	GList *ic = icons;
	for (ic = icons; ic != NULL; ic = ic->next)
	{
		pOtherIcon = (Icon*) ic->data;
		if (pOtherIcon == icon)
		{
			if (ic->prev == NULL)
				return ;
			Icon *prev_icon = ic->prev->data;
			cairo_dock_swap_icons (icon, prev_icon);
		}
	}
}

static void cairo_dock_swap_with_next_icon (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	Icon *pOtherIcon;
	GList *ic = icons;
	for (ic = icons; ic != NULL; ic = ic->next)
	{
		pOtherIcon = (Icon*) ic->data;
		if (pOtherIcon == icon)
		{
			if (ic->next == NULL)
				return ;
			Icon *next_icon = ic->next->data;
			cairo_dock_swap_icons (icon, next_icon);
		}
	}
}

static void cairo_dock_swap_with_first_icon (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	Icon* pFirstIcon = cairo_dock_get_first_icon_of_type (icon->iType);
	if (pFirstIcon != NULL && pFirstIcon != icon)
		cairo_dock_swap_icons (icon, pFirstIcon);
}

static void cairo_dock_swap_with_last_icon (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget* pWidget = data[0];
	Icon *icon = data[1];
	
	Icon* pLastIcon = cairo_dock_get_last_icon_of_type (icon->iType);
	if (pLastIcon != NULL && pLastIcon != icon)
		cairo_dock_swap_icons (icon, pLastIcon);
}

static void cairo_dock_delete_menu (GtkMenuShell *menu, GtkWidget* pWidget)
{
	//g_print ("%s ()\n", __func__);
	g_bMenuVisible = FALSE;
	if (! g_bInside)
	{
		g_bInside = TRUE;
		on_leave_notify2 (pWidget,
			NULL,
			NULL);
	}
}


GtkWidget *cairo_dock_build_menu (GtkWidget *pWidget)
{
	static gpointer *data = NULL;
	
	GtkWidget *menu = gtk_menu_new ();
	g_signal_connect (G_OBJECT (menu),
		"deactivate",
		G_CALLBACK (cairo_dock_delete_menu),
		pWidget);
	
	Icon *icon = cairo_dock_get_pointed_icon ();
	
	if (data == NULL)
		data = g_new (gpointer, 2);
	data[0] = pWidget;
	data[1] = icon;
	
	GtkWidget *menu_item;
	menu_item = gtk_menu_item_new_with_label ("Configure Cairo-Dock");
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_edit_and_reload_conf_file), data);
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	
	
	if (icon == NULL)
		return menu;
	
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
		
		GtkWidget *submenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), submenu);
		
		menu_item = gtk_menu_item_new_with_label ("To the left");
		gtk_menu_shell_append  (GTK_MENU_SHELL (submenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_prev_icon), data);
		
		menu_item = gtk_menu_item_new_with_label ("To the right");
		gtk_menu_shell_append  (GTK_MENU_SHELL (submenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_next_icon), data);
		gtk_widget_show_all (menu);
		
		menu_item = gtk_menu_item_new_with_label ("To the beginning");
		gtk_menu_shell_append  (GTK_MENU_SHELL (submenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_first_icon), data);
		gtk_widget_show_all (menu);
		
		menu_item = gtk_menu_item_new_with_label ("To the end");
		gtk_menu_shell_append  (GTK_MENU_SHELL (submenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(cairo_dock_swap_with_last_icon), data);
	}
	
	
	return menu;
}

