/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

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
#include "cairo-dock-load.h"
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
extern int g_iMaxAuthorizedWidth;
extern gboolean g_bForceLoop;

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
	
	pDock->pWidget = pWindow;
	
	gtk_widget_add_events (pWindow,
		GDK_BUTTON_PRESS_MASK |
		GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK);
	
	g_signal_connect (G_OBJECT (pWindow),
		"delete-event",
		G_CALLBACK (on_delete),
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
		"scroll-event",
		G_CALLBACK (on_scroll),
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
	gtk_widget_show_all (pWindow);
	
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
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, iMinDockWidth, pDock->iScrollOffset);
	pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * iMaxIconHeight) + g_iLabelSize + g_iDockLineWidth;
	pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, iMinDockWidth)) + 1;  // + 1 pour gerer les largeurs impaires.
	int iNewMaxWidth = (g_bForceLoop && pDock->iRefCount == 0 ? pDock->iMaxDockWidth / 2 : MIN (g_iMaxAuthorizedWidth, pDock->iMaxDockWidth));
	
	if (! pDock->bInside && (g_bAutoHide && pDock->iRefCount == 0))
		return;
	else if (pDock->bInside)
	{
		int iNewWidth, iNewHeight;
		cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_MAX_SIZE, &iNewWidth, &iNewHeight);  // inutile de recalculer Y mais bon...
		//g_print ("%s () -> %dx%d\n", __func__, g_iMaxDockWidth, g_iMaxDockHeight);
		if (g_bHorizontalDock)
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionX,
				pDock->iWindowPositionY,
				iNewWidth,
				iNewHeight);
		else
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionY,
				pDock->iWindowPositionX,
				iNewHeight,
				iNewWidth);
	}
	else
	{
		int iNewWidth, iNewHeight;
		cairo_dock_calculate_window_position_at_balance (pDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);  // inutile de recalculer Y mais bon...
		//g_print ("%s () -> %dx%d\n", __func__, g_iMaxDockWidth, g_iMaxDockHeight);
		if (g_bHorizontalDock)
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionX,
				pDock->iWindowPositionY,
				iNewWidth,
				iNewHeight);
		else
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionY,
				pDock->iWindowPositionX,
				iNewHeight,
				iNewWidth);
	}
	
	cairo_dock_update_background_decorations_if_necessary (pDock->pWidget, pDock->iMaxDockWidth, pDock->iMaxIconHeight, (g_bHorizontalDock ? 0 : (g_bDirectionUp ? -G_PI/2 : G_PI/2)));
}



void cairo_dock_insert_icon_in_dock (Icon *icon, CairoDock *pDock, gboolean bUpdateSize, gboolean bAnimated)
{
	g_return_if_fail (icon != NULL);
	
	//\______________ On regarde si on doit inserer un separateur.
	gboolean bSeparatorNeeded = FALSE;
	if (! CAIRO_DOCK_IS_SEPARATOR (icon))
	{
		Icon *pSameTypeIcon = cairo_dock_get_first_icon_of_type (pDock->icons, icon->iType);
		if (pSameTypeIcon == NULL && pDock->icons != NULL)
			bSeparatorNeeded = TRUE;
	}
	
	//\______________ On insere l'icone a sa place dans la liste.
	if (icon->fOrder == CAIRO_DOCK_LAST_ORDER)
	{
		Icon *pLastIcon = cairo_dock_get_last_launcher (pDock->icons);
		if (pLastIcon != NULL)
			icon->fOrder = pLastIcon->fOrder + 1;
		else
			icon->fOrder = 1;
	}
	
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon,
		(GCompareFunc) cairo_dock_compare_icons_order);
	
	pDock->iMinDockWidth += g_iIconGap + icon->fWidth;
	pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, icon->fHeight);
	
	//\______________ On insere un separateur si necessaire.
	if (bSeparatorNeeded)
	{
		if (g_tIconTypeOrder[icon->iType] + 1 < CAIRO_DOCK_NB_TYPES)
		{
			Icon *pNextIcon = cairo_dock_get_next_icon (pDock->icons, icon);
			if (pNextIcon != NULL && ((pNextIcon->iType - icon->iType) % 2 == 0))
			{
				int iSeparatorType = g_tIconTypeOrder[icon->iType] + 1;
				//g_print ("insertion de %s -> iSeparatorType : %d\n", icon->acName, iSeparatorType);
				
				cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
				Icon *pSeparatorIcon = cairo_dock_create_separator_icon (pSourceContext, iSeparatorType, pDock);
				if (pSeparatorIcon != NULL)
				{
					pDock->icons = g_list_insert_sorted (pDock->icons,
						pSeparatorIcon,
						(GCompareFunc) cairo_dock_compare_icons_order);
					pDock->iMinDockWidth += g_iIconGap + pSeparatorIcon->fWidth;
					pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, pSeparatorIcon->fHeight);
				}
				cairo_destroy (pSourceContext);
			}
		}
		if (g_tIconTypeOrder[icon->iType] - 1 > 0)
		{
			Icon *pPrevIcon = cairo_dock_get_previous_icon (pDock->icons, icon);
			if (pPrevIcon != NULL && ((pPrevIcon->iType - icon->iType) % 2 == 0))
			{
				int iSeparatorType = g_tIconTypeOrder[icon->iType] - 1;
				//g_print ("insertion de %s -> iSeparatorType : %d\n", icon->acName, iSeparatorType);
				
				cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
				Icon *pSeparatorIcon = cairo_dock_create_separator_icon (pSourceContext, iSeparatorType, pDock);
				if (pSeparatorIcon != NULL)
				{
					pDock->icons = g_list_insert_sorted (pDock->icons,
						pSeparatorIcon,
						(GCompareFunc) cairo_dock_compare_icons_order);
					pDock->iMinDockWidth += g_iIconGap + pSeparatorIcon->fWidth;
					pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, pSeparatorIcon->fHeight);
				}
				cairo_destroy (pSourceContext);
			}
		}
	}
	
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, pDock->iMinDockWidth, pDock->iScrollOffset);
	
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
		//pDock->iCurrentWidth = pDock->iMinDockWidth + 2 * g_iDockRadius + g_iDockLineWidth;
		//pDock->iCurrentHeight = pDock->iMaxIconHeight + 2 * g_iDockLineWidth;
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
		pDock->iCurrentWidth = pDock->iMaxDockWidth;
		pDock->iCurrentHeight = pDock->iMaxDockHeight;
		//render (pDock);
		gtk_window_present (GTK_WINDOW (pDock->pWidget));
		gtk_widget_queue_draw (pDock->pWidget);
		while (gtk_events_pending ())
			gtk_main_iteration ();
		if (pDock->iRefCount > 0)
			gtk_widget_hide (pDock->pWidget);
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
				pParentDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_MENU, icon->cParentDockName);
				pParentDock->iRefCount --;
			}
			
			cairo_dock_insert_icon_in_dock (icon, pParentDock, FALSE, FALSE);
		}
	} while (1);
	g_dir_close (dir);
	
	g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_update_child_dock_size, NULL);  // on mettra a jour la taille du dock principal apres y avoir insere les applis/applets, car pour l'instant les docks fils n'en ont pas.
}


static gboolean _cairo_dock_free_one_dock (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (pDock->iSidMoveDown != 0)
		g_source_remove (pDock->iSidMoveDown);
	if (pDock->iSidMoveUp != 0)
		g_source_remove (pDock->iSidMoveUp);
	if (pDock->iSidGrowUp != 0)
		g_source_remove (pDock->iSidGrowUp);
	if (pDock->iSidShrinkDown != 0)
		g_source_remove (pDock->iSidShrinkDown);
	if (pDock->bIsMainDock && g_iSidUpdateAppliList != 0)
	{
		g_source_remove (g_iSidUpdateAppliList);
		g_iSidUpdateAppliList = 0;
	}
	
	gtk_widget_destroy (pDock->pWidget);
	pDock->pWidget = NULL;
	
	g_list_foreach (pDock->icons, (GFunc) cairo_dock_free_icon, NULL);
	g_list_free (pDock->icons);
	pDock->icons = NULL;
	
	g_free (pDock);
	return TRUE;
}
void cairo_dock_free_all_docks (CairoDock *pMainDock)
{
	cairo_dock_remove_all_applets (pMainDock);
	
	if (g_iSidUpdateAppliList != 0)
		cairo_dock_remove_all_applis (pMainDock);
	
	g_hash_table_foreach_remove (g_hDocksTable, (GHRFunc) _cairo_dock_free_one_dock, NULL);
}

void cairo_dock_destroy_dock (CairoDock *pDock, gchar *cDockName, CairoDock *ReceivingDock, gchar *cReceivingDockName)
{
	pDock->iRefCount --;  // peut-etre qu'il faudrait en faire une operation atomique...
	if (pDock->iRefCount > 0)
		return ;
	
	if (pDock->iSidMoveDown != 0)
		g_source_remove (pDock->iSidMoveDown);
	if (pDock->iSidMoveUp != 0)
		g_source_remove (pDock->iSidMoveUp);
	if (pDock->iSidGrowUp != 0)
		g_source_remove (pDock->iSidGrowUp);
	if (pDock->iSidShrinkDown != 0)
		g_source_remove (pDock->iSidShrinkDown);
	if (pDock->bIsMainDock && g_iSidUpdateAppliList != 0)
	{
		g_source_remove (g_iSidUpdateAppliList);
		g_iSidUpdateAppliList = 0;
	}
	
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
		
		if (icon->pSubDock != NULL && ReceivingDock == NULL)
		{
			cairo_dock_destroy_dock (icon->pSubDock, icon->acName, NULL, NULL);
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
			
			cairo_dock_insert_icon_in_dock (icon, ReceivingDock, FALSE, TRUE);
		}
		
		g_free (cDesktopFilePath);
	}
	if (ReceivingDock != NULL)
		cairo_dock_update_dock_size (ReceivingDock, ReceivingDock->iMaxIconHeight, ReceivingDock->iMinDockWidth);
	
	g_list_free (pDock->icons);
	pDock->icons = NULL;
	
	g_hash_table_remove (g_hDocksTable, cDockName);
	
	g_free (pDock);
}
