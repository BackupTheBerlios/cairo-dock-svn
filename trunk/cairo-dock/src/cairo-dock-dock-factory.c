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
#include "cairo-dock-applications.h"
#include "cairo-dock-load.h"
#include "cairo-dock-config.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-dock-factory.h"

extern int g_iWmHint;
extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;
extern gboolean g_bSameHorizontality;
extern double g_fSubDockSizeRatio;
extern gboolean g_bReserveSpace;

extern int g_iMaxAuthorizedWidth;
extern gboolean g_bForceLoop;
extern gint g_iDockLineWidth;
extern int g_iIconGap;
extern double g_fAmplitude;

extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

extern gboolean g_bAutoHide;

extern gchar *g_cCurrentLaunchersPath;

extern gboolean g_bDirectionUp;

extern int g_iSidUpdateAppliList;
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];
extern gchar *g_cConfFile;
extern GHashTable *g_hModuleTable;

extern gboolean g_bKeepAbove;
extern gboolean g_bSkipPager;
extern gboolean g_bSkipTaskbar;
extern gboolean g_bSticky;

extern gboolean g_bUseGlitz;


void cairo_dock_set_colormap_for_window (GtkWidget *pWidget)
{
	GdkScreen* pScreen = gtk_widget_get_screen (pWidget);
	GdkColormap* pColormap = gdk_screen_get_rgba_colormap (pScreen);
	if (!pColormap)
		pColormap = gdk_screen_get_rgb_colormap (pScreen);
		
	gtk_widget_set_colormap (pWidget, pColormap);
}

static void _cairo_dock_set_colormap (CairoDock *pDock)
{
	//g_print ("%s f%d)\n", __func__, g_bUseGlitz);
	GdkColormap* pColormap;
#ifdef HAVE_GLITZ
	if (g_bUseGlitz)
	{
		glitz_drawable_format_t templ, *format;
		unsigned long	    mask = GLITZ_FORMAT_DOUBLEBUFFER_MASK;
		XVisualInfo		    *vinfo = NULL;
		int			    screen = 0;
		GdkVisual		    *visual;
		GdkDisplay		    *gdkdisplay;
		Display		    *xdisplay;
		
		templ.doublebuffer = 1;
		gdkdisplay = gtk_widget_get_display (pDock->pWidget);
		xdisplay   = gdk_x11_display_get_xdisplay (gdkdisplay);
		
		int i = 0;
		do
		{
			format = glitz_glx_find_window_format (xdisplay,
				screen,
				mask,
				&templ,
				i++);
			if (format)
			{
				vinfo = glitz_glx_get_visual_info_from_format (xdisplay,
					screen,
					format);
				if (vinfo->depth == 32)
				{
					pDock->pDrawFormat = format;
					break;
				}
				else if (!pDock->pDrawFormat)
				{
					pDock->pDrawFormat = format;
				}
			}
		} while (format);
		
		if (! pDock->pDrawFormat)
		{
			g_print ("Attention : no double buffered GLX visual\n");
		}
		else
		{
			vinfo = glitz_glx_get_visual_info_from_format (xdisplay,
				screen,
				pDock->pDrawFormat);
	
			visual = gdkx_visual_get (vinfo->visualid);
			pColormap = gdk_colormap_new (visual, TRUE);
	
			gtk_widget_set_colormap (pDock->pWidget, pColormap);
			gtk_widget_set_double_buffered (pDock->pWidget, FALSE);
			return ;
		}
	}
#endif
	
	cairo_dock_set_colormap_for_window (pDock->pWidget);
}
CairoDock *cairo_dock_create_new_dock (int iWmHint, gchar *cDockName)
{
	//g_print ("%s ()\n", __func__);
	CairoDock *pDock = g_new0 (CairoDock, 1);
	pDock->bAtBottom = TRUE;
	pDock->iRefCount = 0;  // c'est un dock racine par defaut.
	pDock->iAvoidingMouseIconType = -1;
	if (g_pMainDock != NULL)
	{
		pDock->bHorizontalDock = g_pMainDock->bHorizontalDock;
		pDock->fAlign = g_pMainDock->fAlign;
	}
	
	GtkWidget* pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	pDock->pWidget = pWindow;
	
	if (g_bSticky)
		gtk_window_stick (GTK_WINDOW (pWindow));
	gtk_window_set_keep_above (GTK_WINDOW (pWindow), g_bKeepAbove);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (pWindow), g_bSkipPager);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (pWindow), g_bSkipTaskbar);
	gtk_window_set_gravity (GTK_WINDOW (pWindow), GDK_GRAVITY_STATIC);
	
	gtk_window_set_type_hint (GTK_WINDOW (pWindow), iWmHint);
	
	
	_cairo_dock_set_colormap (pDock);
	
	gtk_widget_set_app_paintable (pWindow, TRUE);
	gtk_window_set_decorated (GTK_WINDOW (pWindow), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (pWindow), TRUE);
	gtk_window_set_title (GTK_WINDOW (pWindow), "cairo-dock");
	
	pDock->calculate_max_dock_size = cairo_dock_calculate_max_dock_size_linear;
	pDock->calculate_icons = cairo_dock_apply_wave_effect;
	pDock->render = cairo_dock_render_linear;
	pDock->set_subdock_position = cairo_dock_set_subdock_position_linear;
	
	
	gtk_widget_add_events (pWindow,
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | 
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
	g_signal_connect (G_OBJECT (pWindow),
		"configure-event",
		G_CALLBACK (on_configure),
		pDock);
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
		G_CALLBACK (on_button_press2),
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
	gtk_window_get_size (GTK_WINDOW (pWindow), &pDock->iCurrentWidth, &pDock->iCurrentHeight);  // ca n'est que la taille initiale allouee par GTK.
	gtk_widget_show_all (pWindow);
	
	
#ifdef HAVE_GLITZ
	if (g_bUseGlitz && pDock->pDrawFormat != NULL)
	{
		glitz_format_t templ;
		GdkDisplay	   *gdkdisplay;
		Display	   *XDisplay;
		Window	   xid;

		gdkdisplay = gdk_display_get_default ();
		XDisplay   = gdk_x11_display_get_xdisplay (gdkdisplay);
		xid = gdk_x11_drawable_get_xid (GDK_DRAWABLE (pWindow->window));
		pDock->pGlitzDrawable = glitz_glx_create_drawable_for_window (XDisplay,
			0,
			pDock->pDrawFormat,
			xid,
			pDock->iCurrentWidth,
			pDock->iCurrentHeight);
		if (! pDock->pGlitzDrawable)
		{
			g_print ("Attention : failed to create glitz drawable\n");
		}
		else
		{
			templ.color        = pDock->pDrawFormat->color;
			templ.color.fourcc = GLITZ_FOURCC_RGB;
			pDock->pGlitzFormat = glitz_find_format (pDock->pGlitzDrawable,
				GLITZ_FORMAT_RED_SIZE_MASK   |
				GLITZ_FORMAT_GREEN_SIZE_MASK |
				GLITZ_FORMAT_BLUE_SIZE_MASK  |
				GLITZ_FORMAT_ALPHA_SIZE_MASK |
				GLITZ_FORMAT_FOURCC_MASK,
				&templ,
				0);
			if (! pDock->pGlitzFormat)
			{
				g_print ("couldn't find glitz surface format\n");
			}
		}
	}
#endif
	
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
	if (pDock->iRefCount == 0)  // inutile de chercher dans ce cas-la.
		return NULL;
	Icon *pPointingIcon = NULL;
	gpointer data[3] = {pDock, &pPointingIcon, pParentDock};
	g_hash_table_find (g_hDocksTable, (GHRFunc)_cairo_dock_search_icon_from_subdock, data);
	return pPointingIcon;
}

static gboolean _cairo_dock_search_icon_in_a_dock (gchar *cDockName, CairoDock *pDock, Icon *icon)
{
	return (g_list_find (pDock->icons, icon) != NULL);
}
CairoDock *cairo_dock_search_container_from_icon (Icon *icon)
{
	if (icon->cParentDockName != NULL)
		return g_hash_table_lookup (g_hDocksTable, icon->cParentDockName);
	else
		return g_hash_table_find (g_hDocksTable, (GHRFunc) _cairo_dock_search_icon_in_a_dock, icon);
}



void cairo_dock_reserve_space_for_dock (CairoDock *pDock, gboolean bReserve)
{
	int Xid = GDK_WINDOW_XID (pDock->pWidget->window);
	int left=0, right=0, top=0, bottom=0;
	int left_start_y=0, left_end_y=0, right_start_y=0, right_end_y=0, top_start_x=0, top_end_x=0, bottom_start_x=0, bottom_end_x=0;
	int iHeight, iWidth;
	
	if (bReserve)
	{
		int iWindowPositionX = pDock->iWindowPositionX, iWindowPositionY = pDock->iWindowPositionY;
		cairo_dock_get_window_position_and_geometry_at_balance (pDock, (g_bAutoHide ? CAIRO_DOCK_MIN_SIZE : CAIRO_DOCK_NORMAL_SIZE), &iWidth, &iHeight);
		if (g_bDirectionUp)
		{
			if (pDock->bHorizontalDock)
			{
				bottom = iHeight + pDock->iGapY;
				bottom_start_x = pDock->iWindowPositionX;
				bottom_end_x = pDock->iWindowPositionX + iWidth;
				
			}
			else
			{
				right = iHeight + pDock->iGapY;
				right_start_y = pDock->iWindowPositionX;
				right_end_y = pDock->iWindowPositionX + iWidth;
			}
		}
		else
		{
			
			if (pDock->bHorizontalDock)
			{
				top = iHeight + pDock->iGapY;
				top_start_x = pDock->iWindowPositionX;
				top_end_x = pDock->iWindowPositionX + iWidth;
			}
			else
			{
				left = iHeight + pDock->iGapY;
				left_start_y = pDock->iWindowPositionX;
				left_end_y = pDock->iWindowPositionX + iWidth;
			}
		}
		pDock->iWindowPositionX = iWindowPositionX;
		pDock->iWindowPositionY = iWindowPositionY;
	}
	
	cairo_dock_set_strut_partial (Xid, left, right, top, bottom, left_start_y, left_end_y, right_start_y, right_end_y, top_start_x, top_end_x, bottom_start_x, bottom_end_x);
	
	if ((bReserve && ! g_bDirectionUp) || (g_iWmHint == GDK_WINDOW_TYPE_HINT_DOCK))  // merci a Robrob pour le patch !
		cairo_dock_set_window_type_hint (Xid, "_NET_WM_WINDOW_TYPE_DOCK");  // gtk_window_set_type_hint ne marche que sur une fenetre avant de la rendre visible !
	else if (g_iWmHint == GDK_WINDOW_TYPE_HINT_NORMAL)
		cairo_dock_set_window_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");  // idem.
	else if (g_iWmHint == GDK_WINDOW_TYPE_HINT_TOOLBAR)
		cairo_dock_set_window_type_hint (Xid, "_NET_WM_WINDOW_TYPE_TOOLBAR");  // idem.
}

void cairo_dock_update_dock_size (CairoDock *pDock, int iMaxIconHeight, int iMinDockWidth)
{
	//g_print ("%s (%d, %d)\n", __func__, iMaxIconHeight, iMinDockWidth);
	//pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, iMinDockWidth, pDock->iScrollOffset);
	//pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * iMaxIconHeight) + g_iLabelSize + g_iDockLineWidth;
	//pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, iMinDockWidth)) + 1;  // + 1 pour gerer les largeurs impaires.
	///cairo_dock_calculate_max_dock_size_generic (pDock);
	pDock->calculate_max_dock_size (pDock);
	int iNewMaxWidth = (g_bForceLoop && pDock->iRefCount == 0 ? pDock->iMaxDockWidth / 2 : MIN (g_iMaxAuthorizedWidth, pDock->iMaxDockWidth));
	
	if (! pDock->bInside && (g_bAutoHide && pDock->iRefCount == 0))
		return;
	else
	{
		int iNewWidth, iNewHeight;
		cairo_dock_get_window_position_and_geometry_at_balance (pDock, (pDock->bInside ? CAIRO_DOCK_MAX_SIZE : CAIRO_DOCK_NORMAL_SIZE), &iNewWidth, &iNewHeight);  // inutile de recalculer Y mais bon...
		//g_print ("%s () -> %dx%d\n", __func__, g_iMaxDockWidth, g_iMaxDockHeight);
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
	
	cairo_dock_update_background_decorations_if_necessary (pDock, pDock->iMaxDockWidth, pDock->iMaxIconHeight, (pDock->bHorizontalDock ? 0 : (g_bDirectionUp ? -G_PI/2 : G_PI/2)));
}



void cairo_dock_insert_icon_in_dock (Icon *icon, CairoDock *pDock, gboolean bUpdateSize, gboolean bAnimated, gboolean bApplyRatio)
{
	g_return_if_fail (icon != NULL);
	int iPreviousMinWidth = pDock->iMinDockWidth;
	int iPreviousMaxIconHeight = pDock->iMaxIconHeight;
	
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
	
	if (bApplyRatio && pDock->iRefCount > 0)
	{
		icon->fWidth *= g_fSubDockSizeRatio;
		icon->fHeight *= g_fSubDockSizeRatio;
		
		if (! g_bSameHorizontality)
		{
			cairo_t* pSourceContext = cairo_dock_create_context_from_window (pDock);
			cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
			cairo_destroy (pSourceContext);
		}
	}
	
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
				
				cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock);
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
				
				cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock);
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
	
	///pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, pDock->iMinDockWidth, pDock->iScrollOffset);
	
	//\______________ On effectue les actions demandees.
	if (bAnimated)
		icon->fPersonnalScale = - 0.95;
	
	if (bUpdateSize)
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
	
	if (pDock->bIsMainDock && g_bReserveSpace && ! g_bAutoHide && ! pDock->bInside && pDock->iMinDockWidth != iPreviousMinWidth && pDock->iMaxIconHeight != iPreviousMaxIconHeight)
		cairo_dock_reserve_space_for_dock (pDock, TRUE);
}


static void _cairo_dock_update_child_dock_size (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (! pDock->bIsMainDock)
	{
		cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
		pDock->iMouseX = 0; // utile ?
		pDock->iMouseY = 0;
		pDock->calculate_icons (pDock);
		//cairo_dock_apply_wave_effect (pDock);
		gtk_window_present (GTK_WINDOW (pDock->pWidget));
		while (gtk_events_pending ())
			gtk_main_iteration ();
		if (pDock->iRefCount > 0)
			gtk_widget_hide (pDock->pWidget);
		else
			gtk_window_move (GTK_WINDOW (pDock->pWidget), 500, 500);  // sinon ils n'apparaisesent pas.
	}
}
void cairo_dock_build_docks_tree_with_desktop_files (CairoDock *pMainDock, gchar *cDirectory)
{
	//g_print ("%s (%s)\n", __func__, cDirectory);
	GDir *dir = g_dir_open (cDirectory, 0, NULL);
	g_return_if_fail (dir != NULL);
	
	Icon* icon;
	const gchar *cFileName;
	CairoDock *pParentDock, *pChildDock;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pMainDock);
	
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
			
			if (pParentDock != NULL)  // a priori toujours vrai.
				cairo_dock_insert_icon_in_dock (icon, pParentDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
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
	//g_print ("%s (%s, %d)\n", __func__, cDockName, pDock->iRefCount);
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
		
		if (ReceivingDock == NULL || cReceivingDockName == NULL)  // alors on les jete.
		{
			if (icon->acDesktopFileName != NULL)
			{
				cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon->acDesktopFileName);
				g_remove (cDesktopFilePath);
				g_free (cDesktopFilePath);
			}
			cairo_dock_free_icon (icon);
		}
		else  // on les re-attribue au dock receveur.
		{
			cairo_dock_update_icon_s_container_name (icon, cReceivingDockName);
			
			if (pDock->iRefCount > 0)
			{
				icon->fWidth /= g_fSubDockSizeRatio;
				icon->fHeight /= g_fSubDockSizeRatio;
			}
			cairo_dock_insert_icon_in_dock (icon, ReceivingDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
		}
	}
	if (ReceivingDock != NULL)
		cairo_dock_update_dock_size (ReceivingDock, ReceivingDock->iMaxIconHeight, ReceivingDock->iMinDockWidth);
	
	g_list_free (pDock->icons);
	pDock->icons = NULL;
	
	g_hash_table_remove (g_hDocksTable, cDockName);
	
	g_free (pDock);
}



void cairo_dock_reference_dock (CairoDock *pChildDock)
{
	pChildDock->iRefCount ++;  // peut-etre qu'il faudrait en faire une operation atomique...
	if (pChildDock->iRefCount == 1)  // il devient un sous-dock.
	{
		pChildDock->bHorizontalDock = (g_bSameHorizontality ? g_pMainDock->bHorizontalDock : ! g_pMainDock->bHorizontalDock);
		
		Icon *icon;
		GList *ic;
		pChildDock->iMinDockWidth = -g_iIconGap;
		for (ic = pChildDock->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			icon->fWidth *= g_fSubDockSizeRatio;
			icon->fHeight *= g_fSubDockSizeRatio;
			pChildDock->iMinDockWidth += icon->fWidth + g_iIconGap;
			
			if (! g_bSameHorizontality)
			{
				cairo_t* pSourceContext = cairo_dock_create_context_from_window (pChildDock);
				cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pChildDock->bHorizontalDock));
				cairo_destroy (pSourceContext);
			}
		}
		pChildDock->iMaxIconHeight *= g_fSubDockSizeRatio;
	}
}


CairoDock *cairo_dock_create_subdock_from_scratch (GList *pIconList, gchar *cDockName)
{
	CairoDock *pSubDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_MENU, cDockName);
	cairo_dock_reference_dock (pSubDock);  // on le fait tout de suite pour avoir la bonne reference avant le 'load'.
	
	pSubDock->icons = pIconList;
	if (pIconList != NULL)
		cairo_dock_load_buffers_in_one_dock (pSubDock);
	
	while (gtk_events_pending ())
		gtk_main_iteration ();
	gtk_widget_hide (pSubDock->pWidget);
	
	return pSubDock;
}

