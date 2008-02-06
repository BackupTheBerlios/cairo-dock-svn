/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
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

#include "cairo-dock-draw.h"
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
#include "cairo-dock-dock-factory.h"

extern int g_iWmHint;
extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;
extern gboolean g_bSameHorizontality;
extern double g_fSubDockSizeRatio;
extern gboolean g_bReserveSpace;
extern gchar *g_cMainDockDefaultRendererName;
extern gchar *g_cSubDockDefaultRendererName;

extern int g_iMaxAuthorizedWidth;
extern gint g_iDockLineWidth;
extern int g_iIconGap;
extern double g_fAmplitude;

extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

extern gboolean g_bUseSeparator;
extern gboolean g_bAutoHide;

extern gchar *g_cCurrentLaunchersPath;

extern gboolean g_bDirectionUp;

extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];
extern gchar *g_cConfFile;

extern gboolean g_bKeepAbove;
extern gboolean g_bSkipPager;
extern gboolean g_bSkipTaskbar;
extern gboolean g_bSticky;

extern gboolean g_bUseGlitz;


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
CairoDock *cairo_dock_create_new_dock (GdkWindowTypeHint iWmHint, gchar *cDockName, gchar *cRendererName)
{
	static pouet = 0;
	//g_print ("%s ()\n", __func__);
	g_return_val_if_fail (cDockName != NULL, NULL);
	CairoDock *pExistingDock = g_hash_table_lookup (g_hDocksTable, cDockName);
	if (pExistingDock != NULL)
		return pExistingDock;
	
	//\__________________ On cree le dock.
	CairoDock *pDock = g_new0 (CairoDock, 1);
	pDock->bAtBottom = TRUE;
	pDock->iRefCount = 0;  // c'est un dock racine par defaut.
	pDock->iAvoidingMouseIconType = -1;
	pDock->fFlatDockWidth = - g_iIconGap;
	if (g_pMainDock != NULL)
	{
		pDock->bHorizontalDock = g_pMainDock->bHorizontalDock;
		pDock->fAlign = g_pMainDock->fAlign;
	}
	pDock->iMouseX = -1; // utile ?
	pDock->iMouseY = -1;
	pDock->fMagnitudeMax = 1.;
	
	//\__________________ On cree la fenetre GTK.
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
	gtk_window_set_title (GTK_WINDOW (pWindow), "cairo-dock");  // GTK renseigne la classe avec la meme valeur.
	
	cairo_dock_set_renderer (pDock, cRendererName);
	
	//\__________________ On connecte les evenements a la fenetre.
	gtk_widget_add_events (pWindow,
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		GDK_KEY_PRESS_MASK |
		//GDK_STRUCTURE_MASK | GDK_PROPERTY_CHANGE_MASK |
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
	
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
	
	/*g_signal_connect (G_OBJECT (pWindow),
		"selection_request_event",
		G_CALLBACK (on_selection_request_event),
		pDock);
	g_signal_connect (G_OBJECT (pWindow),
		"selection_notify_event",
		G_CALLBACK (on_selection_notify_event),
		pDock);*/
	
	GtkTargetEntry *pTargetEntry = g_new0 (GtkTargetEntry, 6);
	pTargetEntry[0].target = "text/*";
	pTargetEntry[0].flags = (GtkTargetFlags) 0;
	pTargetEntry[0].info = 0;
	pTargetEntry[1].target = "text/uri-list";
	pTargetEntry[2].target = "text/plain";
	pTargetEntry[3].target = "text/plain;charset=UTF-8";
	pTargetEntry[4].target = "text/directory";
	pTargetEntry[5].target = "text/html";
	gtk_drag_dest_set (pWindow,
		GTK_DEST_DEFAULT_DROP | GTK_DEST_DEFAULT_MOTION,  // GTK_DEST_DEFAULT_HIGHLIGHT ne rend pas joli je trouve.
		pTargetEntry,
		6,
		GDK_ACTION_COPY);
	g_free (pTargetEntry);
	
	g_hash_table_insert (g_hDocksTable, g_strdup (cDockName), pDock);
	gtk_window_get_size (GTK_WINDOW (pWindow), &pDock->iCurrentWidth, &pDock->iCurrentHeight);  // ca n'est que la taille initiale allouee par GTK.
	gtk_widget_show_all (pWindow);
	
	/*if (!pouet)
	{
		pouet = 1;
		GdkAtom selection = gdk_atom_intern ("_NET_SYSTEM_TRAY_S0", FALSE);
		gboolean bOwnerOK = gtk_selection_owner_set (pWindow,
			selection,
			GDK_CURRENT_TIME);
		g_print ("bOwnerOK : %d\n", bOwnerOK);
		
		gtk_selection_add_target (pWindow,
			selection,
			GDK_SELECTION_TYPE_ATOM,
			1);
		gtk_selection_add_target (pWindow,
			selection,
			GDK_SELECTION_TYPE_BITMAP,
			2);
		gtk_selection_add_target (pWindow,
			selection,
			GDK_SELECTION_TYPE_DRAWABLE,
			3);
		gtk_selection_add_target (pWindow,
			selection,
			GDK_SELECTION_TYPE_INTEGER,
			4);
		gtk_selection_add_target (pWindow,
			selection,
			GDK_SELECTION_TYPE_STRING,
			4);
		gtk_selection_add_target (pWindow,
			selection,
			GDK_SELECTION_TYPE_WINDOW,
			4);
		gtk_selection_add_target (pWindow,
			selection,
			GDK_SELECTION_TYPE_PIXMAP,
			4);
		
		GdkAtom message_type = gdk_atom_intern ("_NET_SYSTEM_TRAY_OPCODE", False);
		gtk_selection_add_target (pWindow,
			selection,
			message_type,
			5);
		message_type = gdk_atom_intern ("_NET_SYSTEM_TRAY_MESSAGE_DATA", False );
		gtk_selection_add_target (pWindow,
			selection,
			message_type,
			5);
		
		g_signal_connect (G_OBJECT (pWindow),
			"selection_get",
			G_CALLBACK (on_selection_get),
			pDock);
		g_signal_connect (G_OBJECT (pWindow),
			"selection_received",
			G_CALLBACK (on_selection_received),
			pDock);
		g_signal_connect (G_OBJECT (pWindow),
			"selection_clear_event",
			G_CALLBACK (on_selection_clear_event),
			pDock);
	}*/
	
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
	g_return_val_if_fail (cDockName != NULL, NULL);
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
	if (pDock->bIsMainDock)  // par definition. On n'utilise pas iRefCount, car si on est en train de detruire un dock, sa reference est deja decrementee.
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
	g_return_val_if_fail (icon != NULL, NULL);
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
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_DOCK");  // gtk_window_set_type_hint ne marche que sur une fenetre avant de la rendre visible !
	else if (g_iWmHint == GDK_WINDOW_TYPE_HINT_NORMAL)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");  // idem.
	else if (g_iWmHint == GDK_WINDOW_TYPE_HINT_TOOLBAR)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_TOOLBAR");  // idem.
}


void cairo_dock_update_dock_size (CairoDock *pDock)  // iMaxIconHeight et fFlatDockWidth doivent avoir ete mis a jour au prealable.
{
	//g_print ("%s (bInside : %d ; iSidShrinkDown : %d)\n", __func__, pDock->bInside, pDock->iSidShrinkDown);
	pDock->calculate_max_dock_size (pDock);
	
	if (! pDock->bInside && (g_bAutoHide && pDock->iRefCount == 0))
		return;
	else if (GTK_WIDGET_VISIBLE (pDock->pWidget))
	{
		int iNewWidth, iNewHeight;
		cairo_dock_get_window_position_and_geometry_at_balance (pDock, (pDock->bInside || pDock->iSidShrinkDown > 0 ? CAIRO_DOCK_MAX_SIZE : CAIRO_DOCK_NORMAL_SIZE), &iNewWidth, &iNewHeight);  // inutile de recalculer Y mais bon...
		
		if (pDock->bHorizontalDock)
		{
			if (pDock->iCurrentWidth != iNewWidth || pDock->iCurrentHeight != iNewHeight)
				gdk_window_move_resize (pDock->pWidget->window,
					pDock->iWindowPositionX,
					pDock->iWindowPositionY,
					iNewWidth,
					iNewHeight);
		}
		else
		{
			if (pDock->iCurrentWidth != iNewHeight || pDock->iCurrentHeight != iNewWidth)
				gdk_window_move_resize (pDock->pWidget->window,
					pDock->iWindowPositionY,
					pDock->iWindowPositionX,
					iNewHeight,
					iNewWidth);
		}
	}
	
	cairo_dock_set_icons_geometry_for_window_manager (pDock);
	
	cairo_dock_update_background_decorations_if_necessary (pDock, pDock->iDecorationsWidth, pDock->iDecorationsHeight);
}



void cairo_dock_insert_icon_in_dock (Icon *icon, CairoDock *pDock, gboolean bUpdateSize, gboolean bAnimated, gboolean bApplyRatio, gboolean bInsertSeparator)
{
	g_return_if_fail (icon != NULL);
	if (g_list_find (pDock->icons, icon) != NULL)  // elle est deja dans ce dock.
		return ;
	
	int iPreviousMinWidth = pDock->fFlatDockWidth;
	int iPreviousMaxIconHeight = pDock->iMaxIconHeight;
	
	//\______________ On regarde si on doit inserer un separateur.
	gboolean bSeparatorNeeded = FALSE;
	if (bInsertSeparator && ! CAIRO_DOCK_IS_SEPARATOR (icon))
	{
		Icon *pSameTypeIcon = cairo_dock_get_first_icon_of_type (pDock->icons, icon->iType);
		if (pSameTypeIcon == NULL && pDock->icons != NULL)
		{
			bSeparatorNeeded = TRUE;
			g_print ("separateur necessaire\n");
		}
	}
	
	//\______________ On insere l'icone a sa place dans la liste.
	if (icon->fOrder == CAIRO_DOCK_LAST_ORDER)
	{
		Icon *pLastIcon = cairo_dock_get_last_icon_of_type (pDock->icons, icon->iType);
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
	}
	//g_print (" +size <- %.2fx%.2f\n", icon->fWidth, icon->fHeight);
	
	if (! g_bSameHorizontality)
	{
		cairo_t* pSourceContext = cairo_dock_create_context_from_window (pDock);
		cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
		cairo_destroy (pSourceContext);
	}
	
	pDock->fFlatDockWidth += g_iIconGap + icon->fWidth;
	pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, icon->fHeight);
	
	//\______________ On insere un separateur si necessaire.
	if (bSeparatorNeeded)
	{
		int iOrder = cairo_dock_get_group_order (icon);
		if (iOrder + 1 < CAIRO_DOCK_NB_TYPES)
		{
			Icon *pNextIcon = cairo_dock_get_next_icon (pDock->icons, icon);
			if (pNextIcon != NULL && ((pNextIcon->iType - icon->iType) % 2 == 0))
			{
				int iSeparatorType = iOrder + 1;
				g_print ("insertion de %s -> iSeparatorType : %d\n", icon->acName, iSeparatorType);
				
				cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock);
				Icon *pSeparatorIcon = cairo_dock_create_separator_icon (pSourceContext, iSeparatorType, pDock, bApplyRatio);
				if (pSeparatorIcon != NULL)
				{
					pDock->icons = g_list_insert_sorted (pDock->icons,
						pSeparatorIcon,
						(GCompareFunc) cairo_dock_compare_icons_order);
					pDock->fFlatDockWidth += g_iIconGap + pSeparatorIcon->fWidth;
					pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, pSeparatorIcon->fHeight);
				}
				cairo_destroy (pSourceContext);
			}
		}
		if (iOrder > 1)
		{
			Icon *pPrevIcon = cairo_dock_get_previous_icon (pDock->icons, icon);
			if (pPrevIcon != NULL && ((pPrevIcon->iType - icon->iType) % 2 == 0))
			{
				int iSeparatorType = iOrder - 1;
				g_print ("insertion de %s -> iSeparatorType : %d\n", icon->acName, iSeparatorType);
				
				cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock);
				Icon *pSeparatorIcon = cairo_dock_create_separator_icon (pSourceContext, iSeparatorType, pDock, bApplyRatio);
				if (pSeparatorIcon != NULL)
				{
					pDock->icons = g_list_insert_sorted (pDock->icons,
						pSeparatorIcon,
						(GCompareFunc) cairo_dock_compare_icons_order);
					pDock->fFlatDockWidth += g_iIconGap + pSeparatorIcon->fWidth;
					pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, pSeparatorIcon->fHeight);
				}
				cairo_destroy (pSourceContext);
			}
		}
	}
	
	//\______________ On effectue les actions demandees.
	if (bAnimated)
		icon->fPersonnalScale = - 0.95;
	
	if (bUpdateSize)
		cairo_dock_update_dock_size (pDock);
	
	if (pDock->bIsMainDock && g_bReserveSpace && ! g_bAutoHide && ! pDock->bInside && (pDock->fFlatDockWidth != iPreviousMinWidth || pDock->iMaxIconHeight != iPreviousMaxIconHeight))
		cairo_dock_reserve_space_for_dock (pDock, TRUE);
}


static void _cairo_dock_update_child_dock_size (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (! pDock->bIsMainDock)
	{
		g_print ("  %s (%s)\n", __func__, cDockName);
		cairo_dock_update_dock_size (pDock);
		///pDock->iMouseX = -1; // utile ?
		///pDock->iMouseY = -1;
		pDock->calculate_icons (pDock);
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
	g_print ("%s (%s)\n", __func__, cDirectory);
	GDir *dir = g_dir_open (cDirectory, 0, NULL);
	g_return_if_fail (dir != NULL);
	
	Icon* icon;
	const gchar *cFileName;
	CairoDock *pParentDock;
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
				cairo_dock_insert_icon_in_dock (icon, pParentDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, ! CAIRO_DOCK_INSERT_SEPARATOR);
		}
	} while (1);
	g_dir_close (dir);
	
	//g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_update_child_dock_size, NULL);  // on mettra a jour la taille du dock principal apres y avoir insere les applis/applets, car pour l'instant les docks fils n'en ont pas.
}


static void _cairo_dock_fm_remove_monitor_on_one_icon (Icon *icon, gpointer data)
{
	if (CAIRO_DOCK_IS_URI_LAUNCHER (icon))
		cairo_dock_fm_remove_monitor (icon);
}
static void _cairo_dock_deactivate_one_dock (CairoDock *pDock)
{
	if (pDock->iSidMoveDown != 0)
		g_source_remove (pDock->iSidMoveDown);
	if (pDock->iSidMoveUp != 0)
		g_source_remove (pDock->iSidMoveUp);
	if (pDock->iSidGrowUp != 0)
		g_source_remove (pDock->iSidGrowUp);
	if (pDock->iSidShrinkDown != 0)
		g_source_remove (pDock->iSidShrinkDown);
	if (pDock->iSidLeaveDemand != 0)
		g_source_remove (pDock->iSidLeaveDemand);
	if (pDock->bIsMainDock && cairo_dock_application_manager_is_running ())
	{
		cairo_dock_pause_application_manager ();  // precaution au cas ou.
	}
	
	g_list_foreach (pDock->icons, (GFunc) _cairo_dock_fm_remove_monitor_on_one_icon, NULL);
	
	Icon *pPointedIcon;
	while ((pPointedIcon = cairo_dock_search_icon_pointing_on_dock (pDock, NULL)) != NULL)
	{
		pPointedIcon->pSubDock = NULL;
	}
	
	gtk_widget_destroy (pDock->pWidget);
	pDock->pWidget = NULL;
	
	g_free (pDock->cRendererName);
	pDock->cRendererName = NULL;
}
static gboolean _cairo_dock_free_one_dock (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	_cairo_dock_deactivate_one_dock (pDock);
	
	GList *pIconsList = pDock->icons;
	pDock->icons = NULL;
	
	g_list_foreach (pIconsList, (GFunc) cairo_dock_free_icon, NULL);
	g_list_free (pIconsList);
	
	g_free (pDock);
	return TRUE;
}
void cairo_dock_free_all_docks (CairoDock *pMainDock)
{
	if (pMainDock == NULL)
		return ;
	
	cairo_dock_deactivate_all_modules ();  // y compris les modules qui n'ont pas d'icone.
	
	cairo_dock_pause_application_manager ();
	
	g_hash_table_foreach_remove (g_hDocksTable, (GHRFunc) _cairo_dock_free_one_dock, NULL);
	g_pMainDock = NULL;
}


void cairo_dock_destroy_dock (CairoDock *pDock, const gchar *cDockName, CairoDock *ReceivingDock, gchar *cReceivingDockName)
{
	//g_print ("%s (%s, %d)\n", __func__, cDockName, pDock->iRefCount);
	g_return_if_fail (pDock != NULL && cDockName != NULL);
	if (pDock->bIsMainDock)  // utiliser cairo_dock_free_all_docks ().
		return;
	pDock->iRefCount --;  // peut-etre qu'il faudrait en faire une operation atomique...
	if (pDock->iRefCount > 0)
		return ;
	
	_cairo_dock_deactivate_one_dock (pDock);
	
	GList *pIconsList = pDock->icons;
	pDock->icons = NULL;
	Icon *icon;
	GList *ic;
	gchar *cDesktopFilePath;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
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
			cairo_dock_insert_icon_in_dock (icon, ReceivingDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
		}
	}
	if (ReceivingDock != NULL)
		cairo_dock_update_dock_size (ReceivingDock);
	
	g_list_free (pIconsList);
	
	g_hash_table_remove (g_hDocksTable, cDockName);
	
	g_free (pDock);
}


void cairo_dock_reference_dock (CairoDock *pDock)
{
	pDock->iRefCount ++;  // peut-etre qu'il faudrait en faire une operation atomique...
	if (pDock->iRefCount == 1)  // il devient un sous-dock.
	{
		pDock->bHorizontalDock = (g_bSameHorizontality ? g_pMainDock->bHorizontalDock : ! g_pMainDock->bHorizontalDock);
		
		Icon *icon;
		GList *ic;
		pDock->fFlatDockWidth = -g_iIconGap;
		for (ic = pDock->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			icon->fWidth *= g_fSubDockSizeRatio;
			icon->fHeight *= g_fSubDockSizeRatio;
			pDock->fFlatDockWidth += icon->fWidth + g_iIconGap;
			
			if (! g_bSameHorizontality)
			{
				cairo_t* pSourceContext = cairo_dock_create_context_from_window (pDock);
				cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
				cairo_destroy (pSourceContext);
			}
		}
		pDock->iMaxIconHeight *= g_fSubDockSizeRatio;
		
		cairo_dock_set_default_renderer (pDock);
		
		gtk_widget_hide (pDock->pWidget);
	}
}


CairoDock *cairo_dock_create_subdock_from_scratch_with_type (GList *pIconList, gchar *cDockName, GdkWindowTypeHint iWindowTypeHint)
{
	CairoDock *pSubDock = cairo_dock_create_new_dock (iWindowTypeHint, cDockName, NULL);
	cairo_dock_reference_dock (pSubDock);  // on le fait tout de suite pour avoir la bonne reference avant le 'load'.
	
	pSubDock->icons = pIconList;
	if (pIconList != NULL)
		cairo_dock_load_buffers_in_one_dock (pSubDock);
	
	/*while (gtk_events_pending ())
		gtk_main_iteration ();
	gtk_widget_hide (pSubDock->pWidget);*/
	
	return pSubDock;
}



void cairo_dock_allow_widget_to_receive_data (GtkWidget *pWidget, GCallback pCallBack)
{
	GtkTargetEntry *pTargetEntry = g_new0 (GtkTargetEntry, 6);
	pTargetEntry[0].target = "text/*";
	pTargetEntry[0].flags = (GtkTargetFlags) 0;
	pTargetEntry[0].info = 0;
	pTargetEntry[1].target = "text/uri-list";
	pTargetEntry[2].target = "text/plain";
	pTargetEntry[3].target = "text/plain;charset=UTF-8";
	pTargetEntry[4].target = "text/directory";
	pTargetEntry[5].target = "text/html";
	gtk_drag_dest_set (pWidget,
		GTK_DEST_DEFAULT_DROP | GTK_DEST_DEFAULT_MOTION,  // GTK_DEST_DEFAULT_HIGHLIGHT ne rend pas joli je trouve.
		pTargetEntry,
		6,
		GDK_ACTION_COPY);
	g_free (pTargetEntry);
	
	g_signal_connect (G_OBJECT (pWidget),
		"drag_data_received",
		pCallBack,
		NULL);
}
