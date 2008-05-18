/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <cairo.h>
#include <pango/pango.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#ifdef HAVE_GLITZ
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-applications-manager.h"
#include "cairo-dock-load.h"
#include "cairo-dock-config.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-file-manager.h"
#include "cairo-dock-X-utilities.h"
#include "cairo-dock-log.h"
#include "cairo-dock-keyfile-utilities.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-dock-manager.h"

extern CairoDock *g_pMainDock;
extern gchar *g_cConfFile;
extern gchar *g_cCurrentThemePath;
extern gboolean g_bSameHorizontality;

static GHashTable *s_hDocksTable = NULL;  // table des docks existant.


void cairo_dock_initialize_dock_manager (void)
{
	cd_message ("");
	if (s_hDocksTable == NULL)
		s_hDocksTable = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,
			NULL);  // donc on peut utiliser g_hash_table_remove plutot que g_hash_table_steal.
}

CairoDock *cairo_dock_register_dock (const gchar *cDockName, CairoDock *pDock)
{
	g_return_val_if_fail (cDockName != NULL, NULL);
	
	CairoDock *pExistingDock = g_hash_table_lookup (s_hDocksTable, cDockName);
	if (pExistingDock != NULL)
	{
		return pExistingDock;
	}
	
	if (g_hash_table_size (s_hDocksTable) == 0)  // c'est le 1er. On pourrait aussi se baser sur son nom ...
	{
		pDock->bIsMainDock = TRUE;
		g_pMainDock = pDock;
	}
	
	g_hash_table_insert (s_hDocksTable, g_strdup (cDockName), pDock);
	return pDock;
}

void cairo_dock_unregister_dock (const gchar *cDockName)
{
	g_return_if_fail (cDockName != NULL);
	g_hash_table_remove (s_hDocksTable, cDockName);
}

static gboolean _cairo_dock_free_one_dock (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	cairo_dock_free_dock (pDock);
	return TRUE;
}
void cairo_dock_reset_docks_table (void)
{
	g_hash_table_foreach_remove (s_hDocksTable, (GHRFunc) _cairo_dock_free_one_dock, NULL);
	g_pMainDock = NULL;
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

	g_hash_table_find (s_hDocksTable, (GHRFunc)_cairo_dock_search_dock_name_from_subdock, data);
	return cDockName;
}

CairoDock *cairo_dock_search_dock_from_name (const gchar *cDockName)
{
	if (cDockName == NULL)
		return NULL;
	return g_hash_table_lookup (s_hDocksTable, cDockName);
}

static gboolean _cairo_dock_search_icon_from_subdock (gchar *cDockName, CairoDock *pDock, gpointer *data)
{
	if (pDock == data[0])
		return FALSE;
	Icon **pIconFound = data[1];
	CairoDock **pDockFound = data[2];
	Icon *icon = cairo_dock_get_icon_with_subdock (pDock->icons, data[0]);
	if (icon != NULL)
	{
		*pIconFound = icon;
		if (pDockFound != NULL)
			*pDockFound = pDock;
		return TRUE;
	}
	else
		return FALSE;
}
Icon *cairo_dock_search_icon_pointing_on_dock (CairoDock *pDock, CairoDock **pParentDock)  // pParentDock peut etre NULL.
{
	if (pDock->bIsMainDock)  // par definition. On n'utilise pas iRefCount, car si on est en train de detruire un dock, sa reference est deja decrementee. note pour moi-meme : pas terrible ca...
		return NULL;
	Icon *pPointingIcon = NULL;
	gpointer data[3] = {pDock, &pPointingIcon, pParentDock};
	g_hash_table_find (s_hDocksTable, (GHRFunc)_cairo_dock_search_icon_from_subdock, data);
	return pPointingIcon;
}

static gboolean _cairo_dock_search_icon_in_a_dock (gchar *cDockName, CairoDock *pDock, Icon *icon)
{
	return (g_list_find (pDock->icons, icon) != NULL);
}
CairoContainer *cairo_dock_search_container_from_icon (Icon *icon)
{
	g_return_val_if_fail (icon != NULL, NULL);
	if (CAIRO_DOCK_IS_APPLET (icon))
		return icon->pModule->pContainer;
	
	if (icon->cParentDockName != NULL)
		return g_hash_table_lookup (s_hDocksTable, icon->cParentDockName);
	else  /// pas tres utile ...
		return g_hash_table_find (s_hDocksTable, (GHRFunc) _cairo_dock_search_icon_in_a_dock, icon);
}


void cairo_dock_update_conf_file_with_containers_full (GKeyFile *pKeyFile, gchar *cDesktopFilePath, gchar *cGroupName, gchar *cKeyName)
{
	cairo_dock_update_conf_file_with_hash_table (pKeyFile, cDesktopFilePath, s_hDocksTable, cGroupName, cKeyName, NULL, (GHFunc)cairo_dock_write_one_name, FALSE, FALSE);
}

static void _cairo_dock_get_one_decoration_size (gchar *cDockName, CairoDock *pDock, int *data)
{
	if (pDock->iDecorationsWidth > data[0])
		data[0] = pDock->iDecorationsWidth;
	if (pDock->iDecorationsHeight > data[1])
		data[1] = pDock->iDecorationsHeight;
}
void cairo_dock_search_max_decorations_size (int *iWidth, int *iHeight)
{
	int data[2] = {0, 0};
	g_hash_table_foreach (s_hDocksTable, (GHFunc) _cairo_dock_get_one_decoration_size, &data);
	cd_message ("  decorations max : %dx%d", data[0], data[1]);
	*iWidth = data[0];
	*iHeight = data[1];
}


static gboolean _cairo_dock_hide_dock_if_parent (gchar *cDockName, CairoDock *pDock, CairoDock *pChildDock)
{
	if (pDock->bInside)
		return FALSE;

	Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	if (pPointedIcon == NULL || pPointedIcon->pSubDock != pChildDock)
		pPointedIcon = cairo_dock_get_icon_with_subdock (pDock->icons, pChildDock);

	if (pPointedIcon != NULL)
	{
		cd_message (" il faut cacher ce dock parent");
		if (pDock->iRefCount == 0)
		{
			cairo_dock_leave_from_main_dock (pDock);
		}
		else
		{
			if (pDock->iScrollOffset != 0)  // on remet systematiquement a 0 l'offset pour les containers.
			{
				pDock->iScrollOffset = 0;
				pDock->iMouseX = pDock->iCurrentWidth / 2;  // utile ?
				pDock->iMouseY = 0;
				pDock->calculate_icons (pDock);
				pDock->render (pDock);  // peut-etre qu'il faudrait faire un redraw.
			}

			cd_message ("on cache %s par parente", cDockName);
			gtk_widget_hide (pDock->pWidget);
			cairo_dock_hide_parent_dock (pDock);
		}
		return TRUE;
	}
	return FALSE;
}
void cairo_dock_hide_parent_dock (CairoDock *pDock)
{
	 g_hash_table_find (s_hDocksTable, (GHRFunc)_cairo_dock_hide_dock_if_parent, pDock);
}

gboolean cairo_dock_hide_child_docks (CairoDock *pDock)
{
	GList* ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->pSubDock != NULL && GTK_WIDGET_VISIBLE (icon->pSubDock->pWidget))
		{
			if (icon->pSubDock->bInside)
			{
				cd_message ("on est dans le sous-dock, donc on ne le cache pas");
				pDock->bInside = FALSE;
				pDock->bAtTop = FALSE;
				return FALSE;
			}
			else if (icon->pSubDock->iSidLeaveDemand == 0)  // si on sort du dock sans passer par le sous-dock, par exemple en sortant par le bas.
			{
				cd_message ("on cache %s par filiation", icon->acName);
				icon->pSubDock->iScrollOffset = 0;
				icon->pSubDock->fFoldingFactor = 0;
				gtk_widget_hide (icon->pSubDock->pWidget);
			}
		}
	}
	return TRUE;
}


void cairo_dock_reload_buffers_in_all_docks (void)
{
	g_hash_table_foreach (s_hDocksTable, (GHFunc) cairo_dock_reload_buffers_in_dock, GINT_TO_POINTER (FALSE));
}


void cairo_dock_rename_dock (const gchar *cDockName, CairoDock *pDock, const gchar *cNewName)
{
	g_return_if_fail (cDockName != NULL && cNewName != NULL);
	if (pDock == NULL)
		pDock = g_hash_table_lookup (s_hDocksTable, cDockName);
	
	g_hash_table_remove (s_hDocksTable, cDockName);  // libere la cle, mais pas la valeur puisque la GDestroyFunc est a NULL.
	g_hash_table_insert (s_hDocksTable, g_strdup (cNewName), pDock);
	
	GList* ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		g_free (icon->cParentDockName);
		icon->cParentDockName = g_strdup (cNewName);
	}
}

static void _cairo_dock_reset_one_dock_view (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	cairo_dock_set_renderer (pDock, NULL);  // on met NULL plutot que CAIRO_DOCK_DEFAULT_RENDERER_NAME pour ne pas ecraser le nom de la vue.
}
void cairo_dock_reset_all_views (void)
{
	//g_print ("%s ()\n", __func__);
	g_hash_table_foreach (s_hDocksTable, (GHFunc) _cairo_dock_reset_one_dock_view, NULL);
}

static void _cairo_dock_set_one_dock_view_to_default (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	//g_print ("%s (%s)\n", __func__, cDockName);
	cairo_dock_set_default_renderer (pDock);
	cairo_dock_update_dock_size (pDock);
}
void cairo_dock_set_all_views_to_default (void)
{
	//g_print ("%s ()\n", __func__);
	g_hash_table_foreach (s_hDocksTable, (GHFunc) _cairo_dock_set_one_dock_view_to_default, NULL);
}



void cairo_dock_write_root_dock_gaps (CairoDock *pDock)
{
	if (pDock->iRefCount > 0)
		return;
	cairo_dock_prevent_dock_from_out_of_screen (pDock);
	if (pDock->bIsMainDock)
	{
		cairo_dock_update_conf_file_with_position (g_cConfFile, pDock->iGapX, pDock->iGapY);
	}
	else
	{
		const gchar *cDockName = cairo_dock_search_dock_name (pDock);
		gchar *cConfFilePath = g_strdup_printf ("%s/%s.conf", g_cCurrentThemePath, cDockName);
		if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))
		{
			gchar *cCommand = g_strdup_printf ("cp %s/%s %s", CAIRO_DOCK_SHARE_DATA_DIR, CAIRO_DOCK_MAIN_DOCK_CONF_FILE, cConfFilePath);
			system (cCommand);
			g_free (cCommand);
		}
		
		cairo_dock_update_conf_file_with_position (cConfFilePath, pDock->iGapX, pDock->iGapY);
		g_free (cConfFilePath);
	}
}

void cairo_dock_get_root_dock_position (const gchar *cDockName, CairoDock *pDock)
{
	g_return_if_fail (cDockName != NULL && pDock != NULL);
	if (pDock->iRefCount > 0)
		return;
	
	gchar *cConfFilePath = (pDock->bIsMainDock ? g_cConfFile : g_strdup_printf ("%s/%s.conf", g_cCurrentThemePath, cDockName));
	if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))
	{
		pDock->bHorizontalDock = g_pMainDock->bHorizontalDock;
		pDock->bDirectionUp = g_pMainDock->bDirectionUp;
		pDock->fAlign = g_pMainDock->fAlign;
		
		if (! pDock->bIsMainDock)
			g_free (cConfFilePath);
		return ;
	}
	
	GKeyFile *pKeyFile = g_key_file_new ();
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	else
	{
		gboolean bFlushConfFileNeeded = FALSE;
		pDock->iGapX = cairo_dock_get_integer_key_value (pKeyFile, "Position", "x gap", &bFlushConfFileNeeded, 0, NULL, NULL);
		pDock->iGapY = cairo_dock_get_integer_key_value (pKeyFile, "Position", "y gap", &bFlushConfFileNeeded, 0, NULL, NULL);
		
		CairoDockPositionType iScreenBorder = cairo_dock_get_integer_key_value (pKeyFile, "Position", "screen border", &bFlushConfFileNeeded, 0, NULL, NULL);
		if (iScreenBorder < 0 || iScreenBorder >= CAIRO_DOCK_NB_POSITIONS)
			iScreenBorder = 0;
		
		switch (iScreenBorder)
		{
			case CAIRO_DOCK_BOTTOM :
				pDock->bHorizontalDock = CAIRO_DOCK_HORIZONTAL;
				pDock->bDirectionUp = TRUE;
			break;
			case CAIRO_DOCK_TOP :
				pDock->bHorizontalDock = CAIRO_DOCK_HORIZONTAL;
				pDock->bDirectionUp = FALSE;
			break;
			case CAIRO_DOCK_LEFT :
				pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
				pDock->bDirectionUp = FALSE;
			break;
			case CAIRO_DOCK_RIGHT :
				pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
				pDock->bDirectionUp = TRUE;
			break;
		}
		
		pDock->fAlign = cairo_dock_get_double_key_value (pKeyFile, "Position", "alignment", &bFlushConfFileNeeded, 0.5, NULL, NULL);
		
		pDock->bAutoHide = cairo_dock_get_boolean_key_value (pKeyFile, "Position", "auto-hide", &bFlushConfFileNeeded, FALSE, "Auto-Hide", "auto-hide");
		
		g_key_file_free (pKeyFile);
	}
	
	if (! pDock->bIsMainDock)
		g_free (cConfFilePath);
}

void cairo_dock_remove_root_dock_config (const gchar *cDockName)
{
	gchar *cConfFilePath = g_strdup_printf ("%s/%s.conf", g_cCurrentThemePath, cDockName);
	if (g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))
	{
		g_remove (cConfFilePath);
	}
	g_free (cConfFilePath);
}


static gboolean s_bTemporaryAutoHide = FALSE;

static void _cairo_dock_quick_hide_one_root_dock (const gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (pDock->iRefCount == 0)
	{
		pDock->bAtBottom = FALSE;  // car on a deja quitte le dock lors de la fermeture du menu, donc le "leave-notify" serait ignore.
		pDock->bAutoHideInitialValue = pDock->bAutoHide;
		pDock->bAutoHide = TRUE;
		pDock->bEntranceDisabled = TRUE;
		cairo_dock_emit_leave_signal (pDock);
	}
}
void cairo_dock_activate_temporary_auto_hide (void)
{
	if (! s_bTemporaryAutoHide)
	{
		s_bTemporaryAutoHide = TRUE;
		g_hash_table_foreach (s_hDocksTable, (GHFunc) _cairo_dock_quick_hide_one_root_dock, NULL);
	}
}

static void _cairo_dock_stop_quick_hide_one_root_dock (const gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (pDock->iRefCount == 0)
	{
		pDock->bAutoHide = pDock->bAutoHideInitialValue;
		pDock->bAtBottom = TRUE;
		
		if (! pDock->bInside && ! pDock->bAutoHide)  // on le fait re-apparaitre.
		{
			pDock->fFoldingFactor = 0;
			
			int iNewWidth, iNewHeight;
			cairo_dock_get_window_position_and_geometry_at_balance (pDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);
			
			if (pDock->bHorizontalDock)
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
	}
}
void cairo_dock_deactivate_temporary_auto_hide (void)
{
	cd_message ("");
	if (s_bTemporaryAutoHide)
	{
		s_bTemporaryAutoHide = FALSE;
		g_hash_table_foreach (s_hDocksTable, (GHFunc) _cairo_dock_stop_quick_hide_one_root_dock, NULL);
	}
}

void cairo_dock_allow_entrance (CairoDock *pDock)
{
	pDock->bEntranceDisabled = FALSE;
}

void cairo_dock_disable_entrance (CairoDock *pDock)
{
	pDock->bEntranceDisabled = TRUE;
}

gboolean cairo_dock_entrance_is_allowed (CairoDock *pDock)
{
	return (! pDock->bEntranceDisabled);
}

gboolean cairo_dock_quick_hide_is_activated (void)
{
	return s_bTemporaryAutoHide;
}


gboolean cairo_dock_window_hovers_dock (GtkAllocation *pWindowGeometry, CairoDock *pDock)
{
	if (pWindowGeometry->width != 0 && pWindowGeometry->height != 0)
	{
		int iDockX = pDock->iWindowPositionX, iDockY = pDock->iWindowPositionY;
		int iDockWidth = pDock->iCurrentWidth, iDockHeight = pDock->iCurrentHeight;
		cd_message ("dock : (%d;%d) %dx%d", iDockX, iDockY, iDockWidth, iDockHeight);
		if ((pWindowGeometry->x <= iDockX + iDockWidth && pWindowGeometry->x + pWindowGeometry->width >= iDockX) || (pWindowGeometry->y >= iDockY + iDockHeight && pWindowGeometry->y + pWindowGeometry->height >= iDockY))
		{
			cd_message (" empiete sur le dock");
			return TRUE;
		}
	}
	else
	{
		cd_warning (" on ne peut pas dire ou elle est sur l'ecran, on va supposer qu'elle recouvre le dock");
		return TRUE;
	}
	return FALSE;
}

void cairo_dock_synchronize_sub_docks_position (CairoDock *pDock, gboolean bReloadBuffersIfNecessary)
{
	GList* ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->pSubDock != NULL)
		{
			if (icon->pSubDock->bDirectionUp != pDock->bDirectionUp || (icon->pSubDock->bDirectionUp != ((!g_bSameHorizontality) ^ pDock->bHorizontalDock)))
			{
				icon->pSubDock->bDirectionUp = pDock->bDirectionUp;
				icon->pSubDock->bHorizontalDock = (!g_bSameHorizontality) ^ pDock->bHorizontalDock;
				if (bReloadBuffersIfNecessary)
					cairo_dock_reload_reflects_in_dock (icon->pSubDock);
				cairo_dock_synchronize_sub_docks_position (icon->pSubDock, bReloadBuffersIfNecessary);
			}
		}
	}
}
