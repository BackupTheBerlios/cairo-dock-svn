/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <cairo.h>
#include <pango/pango.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-draw.h"
#include "cairo-dock-config.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-dock-factory.h"


extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern int g_iIconGap;
extern int g_iLabelSize;
extern gboolean g_bRoundedBottomCorner;
extern gboolean g_bAutoHide;

extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern gchar *g_cCairoDockDataDir;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern double g_fVisibleZoneAlpha;
extern int g_iNbStripes;
extern double g_fAmplitude;
extern int g_iSinusoidWidth;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;
extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern GHashTable *g_hAppliTable;
extern gboolean g_bUniquePid;
extern GHashTable *g_hXWindowTable;
extern int g_iSidUpdateAppliList;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];
extern gchar *g_cConfFile;
extern GHashTable *g_hModuleTable;


extern gboolean g_bKeepAbove;
extern gboolean g_bSkipPager;
extern gboolean g_bSkipTaskbar;
extern gboolean g_bSticky;


static void
on_alpha_screen_changed (GtkWidget* pWidget,
			GdkScreen* pOldScreen,
			GtkWidget* pLabel)
{
	GdkScreen* pScreen = gtk_widget_get_screen (pWidget);
	GdkColormap* pColormap = gdk_screen_get_rgba_colormap (pScreen);
	
	if (!pColormap)
		pColormap = gdk_screen_get_rgb_colormap (pScreen);
		
	gtk_widget_set_colormap (pWidget, pColormap);
}
CairoDock *cairo_dock_create_new_dock (int iWmHint, gchar *cDockName)
{
	//g_print ("%s ()\n", __func__);
	CairoDock *pDock = g_new0 (CairoDock, 1);
	pDock->bAtBottom = TRUE;
	pDock->iRefCount = 1;
	
	GtkWidget* pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	if (g_bSticky)
		gtk_window_stick (GTK_WINDOW (pWindow));
	gtk_window_set_keep_above (GTK_WINDOW (pWindow), g_bKeepAbove);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (pWindow), g_bSkipPager);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (pWindow), g_bSkipTaskbar);
	gtk_window_set_gravity (GTK_WINDOW (pWindow), GDK_GRAVITY_STATIC);
	
	gtk_window_set_type_hint (GTK_WINDOW (pWindow), iWmHint);
	on_alpha_screen_changed (pWindow, NULL, NULL);
	
	gtk_widget_set_app_paintable (pWindow, TRUE);
	gtk_window_set_decorated (GTK_WINDOW (pWindow), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (pWindow), TRUE);
	gtk_window_set_title (GTK_WINDOW (pWindow), "cairo-dock");
	
	gtk_widget_show_all (pWindow);
	
	pDock->pWidget = pWindow;
	
	gtk_widget_add_events (pWindow,
		GDK_BUTTON_PRESS_MASK |
		GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK);
	
	if (strcmp (cDockName, CAIRO_DOCK_MAIN_DOCK_NAME) == 0)
		g_signal_connect (G_OBJECT (pWindow),
			"destroy",
			G_CALLBACK (gtk_main_quit),
			pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"expose-event",
		G_CALLBACK (on_expose),
		pDock);
//#ifdef HAVE_GLITZ	
	g_signal_connect (G_OBJECT (pWindow),
		"configure-event",
		G_CALLBACK (on_configure),
		pDock);
//#endif
	g_signal_connect (G_OBJECT (pWindow),
		"key-press-event",
		G_CALLBACK (on_key_press),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"key-release-event",
		G_CALLBACK (on_key_release),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"button-press-event",
		G_CALLBACK (on_button_press2),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"button-release-event",
		G_CALLBACK (on_button_release),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"motion-notify-event",
		G_CALLBACK (on_motion_notify2),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"enter-notify-event",
		G_CALLBACK (on_enter_notify2),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"leave-notify-event",
		G_CALLBACK (on_leave_notify2),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"drag_data_received",
		G_CALLBACK (on_drag_data_received),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"drag_motion",
		G_CALLBACK (on_drag_motion),
		pDock);
	
	GtkTargetEntry *pTargetEntry = g_new0 (GtkTargetEntry, 1);
	pTargetEntry[0].target = g_strdup ("text/uri-list");
	pTargetEntry[0].flags = (GtkTargetFlags) 0;
	pTargetEntry[0].info = 0;
	gtk_drag_dest_set (pWindow,
		GTK_DEST_DEFAULT_DROP | GTK_DEST_DEFAULT_MOTION,  // GTK_DEST_DEFAULT_HIGHLIGHT ne rend pas joli je trouve.
		pTargetEntry,
		1,
		GDK_ACTION_COPY);
	
	g_hash_table_insert (g_hDocksTable, g_strdup (cDockName), pDock);
	
	return pDock;
}


static gboolean _cairo_dock_search_dock_name_from_subdock (gchar *cDockName, CairoDock *pDock, gpointer *data)
{
	if (pDock == data[0])
	{
		* ((gchar **) data[1]) = cDockName;
		return TRUE;
	}
	else
		return FALSE;
}
const gchar *cairo_dock_search_dock_name (CairoDock *pDock)
{
	gchar *cDockName = NULL;
	gpointer data[2] = {pDock, &cDockName};
	
	g_hash_table_find (g_hDocksTable, (GHRFunc)_cairo_dock_search_dock_name_from_subdock, data);
	return cDockName;
}

CairoDock *cairo_dock_search_dock_from_name (gchar *cDockName)
{
	return g_hash_table_lookup (g_hDocksTable, cDockName);
}



void cairo_dock_update_dock_size (CairoDock *pDock, int iMaxIconHeight, int iMinDockWidth)
{
	//g_print ("%s (%d, %d)\n", __func__, iMaxIconHeight, iMinDockWidth);
	pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * iMaxIconHeight) + g_iLabelSize;
	pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock->icons, iMinDockWidth)) + 1;  // + 1 pour gerer les largeurs impaires.
	cairo_dock_calculate_icons (pDock, 0, 0);
	
	if (! g_bAutoHide)
	{
		g_iVisibleZoneWidth = iMinDockWidth + 2 * (g_iDockRadius + g_iDockLineWidth);
		g_iVisibleZoneHeight = pDock->iMaxIconHeight + 2 * g_iDockLineWidth;
	}
	
	if (! pDock->bInside && g_bAutoHide)
		return;
	else if (pDock->bInside)
	{
		pDock->iWindowPositionX = (g_iScreenWidth - pDock->iMaxDockWidth) / 2 + pDock->iGapX;
		if (! g_bAutoHide)
			pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - pDock->iMaxDockHeight - pDock->iGapY : g_iScreenHeight - pDock->iGapY);
		else
			pDock->iWindowPositionY = (g_bDirectionUp ? pDock->iWindowPositionY : g_iVisibleZoneHeight - pDock->iMaxDockHeight + (g_iScreenHeight - pDock->iGapY));
		//g_print ("%s () -> %dx%d\n", __func__, g_iMaxDockWidth, g_iMaxDockHeight);
		gdk_window_move_resize (pDock->pWidget->window,
			pDock->iWindowPositionX,
			pDock->iWindowPositionY,
			pDock->iMaxDockWidth,
			pDock->iMaxDockHeight);
	}
	else
	{
		pDock->iWindowPositionX = (g_iScreenWidth - (iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth)) / 2 + pDock->iGapX;
		pDock->iWindowPositionY = g_iScreenHeight - pDock->iGapY - (g_bDirectionUp ? iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth : 0);
		//g_print ("%s () -> %dx%d\n", __func__, g_iMaxDockWidth, g_iMaxDockHeight);
		gdk_window_move_resize (pDock->pWidget->window,
			pDock->iWindowPositionX,
			pDock->iWindowPositionY,
			iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth,
			iMaxIconHeight + g_iLabelSize + 2 * g_iDockLineWidth);
	}
}



void cairo_dock_insert_icon_in_dock (Icon *icon, CairoDock *pDock, gboolean bUpdateSize, gboolean bAnimated)
{
	g_return_if_fail (icon != NULL);
	
	//\______________ On regarde si on doit inserer un separateur.
	if (! CAIRO_DOCK_IS_SEPARATOR (icon))
	{
		Icon *pSameTypeIcon = cairo_dock_get_first_icon_of_type (pDock->icons, icon->iType);
		if (pSameTypeIcon == NULL && pDock->icons != NULL)
		{
			cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
			int iSeparatorType = -1;
			if (g_tIconTypeOrder[icon->iType] > 1)  // on l'insere avant nous de preference.
				iSeparatorType = icon->iType - 1;
			else if (g_tIconTypeOrder[icon->iType] + 1 < CAIRO_DOCK_NB_TYPES)
				iSeparatorType = icon->iType + 1;
			//g_print ("iSeparatorType : %d\n", iSeparatorType);
			if (iSeparatorType != -1)
			{
				Icon *pSeparatorIcon = cairo_dock_create_separator_icon (pSourceContext, iSeparatorType, pDock);
				pDock->icons = g_list_insert_sorted (pDock->icons,
					pSeparatorIcon,
					(GCompareFunc) cairo_dock_compare_icons_order);
				pDock->iMinDockWidth += g_iIconGap + pSeparatorIcon->fWidth;
				pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, pSeparatorIcon->fHeight);
			}
			cairo_destroy (pSourceContext);
		}
	}
	
	//\______________ On insere l'icone a sa place dans la liste.
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon,
		(GCompareFunc) cairo_dock_compare_icons_order);
	
	pDock->iMinDockWidth += g_iIconGap + icon->fWidth;
	pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, icon->fHeight);
	
	//\______________ On effectue les actions demandees.
	if (bAnimated)
		icon->fPersonnalScale = - 0.95;
	
	if (bUpdateSize)
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
}

void _cairo_dock_update_child_dock_size (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (! pDock->bIsMainDock)
	{
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
	}
}
void cairo_dock_build_docks_tree_with_desktop_files (CairoDock *pMainDock, gchar *cDirectory)
{
	GDir *dir = g_dir_open (cDirectory, 0, NULL);
	g_return_if_fail (dir != NULL);
	
	Icon* icon;
	const gchar *cFileName;
	CairoDock *pParentDock, *pChildDock;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pMainDock->pWidget->window);
	
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		if (g_str_has_suffix (cFileName, ".desktop"))
		{
			icon = cairo_dock_create_icon_from_desktop_file (cFileName, pCairoContext);
			
			g_return_if_fail (icon->cParentDockName != NULL);
			
			pParentDock = g_hash_table_lookup (g_hDocksTable, icon->cParentDockName);
			if (pParentDock == NULL)
			{
				g_print ("le dock parent (%s) n'existe pas, on le cree\n", icon->cParentDockName);
				pParentDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_NORMAL, icon->cParentDockName);
				pParentDock->iRefCount --;
			}
			
			/*if (GPOINTER_TO_INT (icon->pSubDock) != 0)
			{
				pChildDock = g_hash_table_lookup (g_hDocksTable, icon->acName);
				if (pChildDock == NULL)
				{
					g_print ("le dock fils (%s) n'existe pas, on le cree\n", icon->acName);
					pChildDock= cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_NORMAL, icon->acName);
				}
				icon->pSubDock = pChildDock;
			}*/
			
			cairo_dock_insert_icon_in_dock (icon, pParentDock, FALSE, FALSE);
		}
	} while (1);
	g_dir_close (dir);
	
	g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_update_child_dock_size, NULL);  // on mettra a jour la taille du dock principal apres y avoir insere les applis/applets, car pour l'instant les docks fils n'en ont pas.
}


void cairo_dock_destroy_dock (CairoDock *pDock, gchar *cDockName, CairoDock *ReceivingDock, gchar *cReceivingDockName)
{
	pDock->iRefCount --;  // peut-etre qu'il faudrait en faire une operation atomique...
	if (pDock->iRefCount > 0)
		return ;
	gtk_widget_destroy (pDock->pWidget);
	pDock->pWidget = NULL;
	
	Icon *icon;
	GList *ic;
	gchar *cDesktopFilePath;
	GKeyFile *pKeyFile;
	GError *erreur = NULL;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		if (icon->pSubDock != NULL)
		{
			cairo_dock_destroy_dock (icon->pSubDock, icon->acName, ReceivingDock, cReceivingDockName);
			icon->pSubDock = NULL;
		}
		
		cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, icon->acDesktopFileName);
		
		if (ReceivingDock == NULL || cReceivingDockName == NULL)  // alors on les jete.
		{
			g_remove (cDesktopFilePath);
			
			cairo_dock_free_icon (icon);
		}
		else  // on les re-attribue au dock receveur.
		{
			pKeyFile = g_key_file_new();
			g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			else
			{
				g_free (icon->cParentDockName);
				icon->cParentDockName = g_strdup (cReceivingDockName);
				g_key_file_set_string (pKeyFile, "Desktop Entry", "Container", icon->cParentDockName);
				cairo_dock_write_keys_to_file (pKeyFile, cDesktopFilePath);
			}
			g_key_file_free (pKeyFile);
			
			cairo_dock_insert_icon_in_dock (icon, ReceivingDock, FALSE, FALSE);
		}
		
		g_free (cDesktopFilePath);
	}
	g_list_free (pDock->icons);
	pDock->icons = NULL;
	
	g_hash_table_remove (g_hDocksTable, cDockName);
	
	g_free (pDock);
}
