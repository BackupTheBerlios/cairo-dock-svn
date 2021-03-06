/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define __USE_POSIX
#include <signal.h>

#include <cairo.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#ifdef HAVE_XEXTEND
#include <X11/extensions/Xcomposite.h>
//#include <X11/extensions/Xdamage.h>
#endif

#include "cairo-dock-icons.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-load.h"
#include "cairo-dock-application-factory.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dock-facility.h"
#include "cairo-dock-container.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-log.h"
#include "cairo-dock-X-utilities.h"
#include "cairo-dock-config.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-class-manager.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-internal-system.h"
#include "cairo-dock-internal-taskbar.h"
#include "cairo-dock-internal-position.h"
#include "cairo-dock-internal-icons.h"
#include "cairo-dock-internal-accessibility.h"
#include "cairo-dock-applications-manager.h"

#define CAIRO_DOCK_TASKBAR_CHECK_INTERVAL 200

extern CairoDock *g_pMainDock;

extern int g_iXScreenWidth[2], g_iXScreenHeight[2];

extern int g_iNbDesktops;
extern int g_iNbViewportX,g_iNbViewportY ;
//extern int g_iDamageEvent;

static GHashTable *s_hXWindowTable = NULL;  // table des fenetres X affichees dans le dock.
static Display *s_XDisplay = NULL;
static int s_iSidUpdateAppliList = 0;
static int s_iTime = 0;  // on peut aller jusqu'a 2^31, soit 17 ans a 4Hz.
static Window s_iCurrentActiveWindow = 0;
static Atom s_aNetClientList;
static Atom s_aNetActiveWindow;
static Atom s_aNetCurrentDesktop;
static Atom s_aNetDesktopViewport;
static Atom s_aNetDesktopGeometry;
static Atom s_aNetWmState;
static Atom s_aNetWmDesktop;
static Atom s_aRootMapID;
static Atom s_aNetNbDesktops;
static Atom s_aXKlavierState;


void cairo_dock_close_xwindow (Window Xid)
{
	//g_print ("%s (%d)\n", __func__, Xid);
	g_return_if_fail (Xid > 0);
	
	XEvent xClientMessage;
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_CLOSE_WINDOW", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = cairo_dock_get_xwindow_timestamp (Xid);  // timestamp
	xClientMessage.xclient.data.l[1] = 2;  // 2 <=> pagers and other Clients that represent direct user actions.
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;

	Window root = DefaultRootWindow (s_XDisplay);
	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}

void cairo_dock_kill_xwindow (Window Xid)
{
	g_return_if_fail (Xid > 0);
	XKillClient (s_XDisplay, Xid);
}

void cairo_dock_show_xwindow (Window Xid)
{
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;
	Window root = DefaultRootWindow (s_XDisplay);

	//\______________ On se deplace sur le bureau de la fenetre a afficher (autrement Metacity deplacera la fenetre sur le bureau actuel).
	int iDesktopNumber = cairo_dock_get_xwindow_desktop (Xid);
	cairo_dock_set_current_desktop (iDesktopNumber);

	//\______________ On active la fenetre.
	//XMapRaised (s_XDisplay, Xid);  // on la mappe, pour les cas ou elle etait en zone de notification. Malheuresement, la zone de notif de gnome est bugguee, et reduit la fenetre aussitot qu'on l'a mappee :-(
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = s_aNetActiveWindow;
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = 2;  // source indication
	xClientMessage.xclient.data.l[1] = 0;  // timestamp
	xClientMessage.xclient.data.l[2] = 0;  // requestor's currently active window, 0 if none
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;

	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);

	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}





void cairo_dock_initialize_application_manager (Display *pDisplay)
{
	s_XDisplay = pDisplay;

	s_hXWindowTable = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	
	s_aNetClientList		= XInternAtom (s_XDisplay, "_NET_CLIENT_LIST_STACKING", False);
	s_aNetActiveWindow		= XInternAtom (s_XDisplay, "_NET_ACTIVE_WINDOW", False);
	s_aNetCurrentDesktop	= XInternAtom (s_XDisplay, "_NET_CURRENT_DESKTOP", False);
	s_aNetDesktopViewport	= XInternAtom (s_XDisplay, "_NET_DESKTOP_VIEWPORT", False);
	s_aNetDesktopGeometry	= XInternAtom (s_XDisplay, "_NET_DESKTOP_GEOMETRY", False);
	s_aNetWmState			= XInternAtom (s_XDisplay, "_NET_WM_STATE", False);
	s_aNetWmDesktop			= XInternAtom (s_XDisplay, "_NET_WM_DESKTOP", False);
	s_aRootMapID			= XInternAtom (s_XDisplay, "_XROOTPMAP_ID", False);
	s_aNetNbDesktops		= XInternAtom (s_XDisplay, "_NET_NUMBER_OF_DESKTOPS", False);
	s_aXKlavierState		= XInternAtom (s_XDisplay, "XKLAVIER_STATE", False);
}

void cairo_dock_register_appli (Icon *icon)
{
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cd_debug ("%s (%ld ; %s)\n", __func__, icon->Xid, icon->acName);
		Window *pXid = g_new (Window, 1);
			*pXid = icon->Xid;
		g_hash_table_insert (s_hXWindowTable, pXid, icon);
		
		cairo_dock_add_appli_to_class (icon);
	}
}

void cairo_dock_blacklist_appli (Window Xid)
{
	if (Xid > 0)
	{
		cd_debug ("%s (%ld)\n", __func__, Xid);
		Window *pXid = g_new (Window, 1);
			*pXid = Xid;
		g_hash_table_insert (s_hXWindowTable, pXid, g_new0 (Icon, 1));  // NULL
	}
}

void cairo_dock_unregister_appli (Icon *icon)
{
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cd_message ("%s (%ld ; %s)", __func__, icon->Xid, icon->acName);
		if (icon->iLastCheckTime != -1)
			g_hash_table_remove (s_hXWindowTable, &icon->Xid);
		
		//cairo_dock_unregister_pid (icon);  // on n'efface pas sa classe ici car on peut en avoir besoin encore.
		
		if (icon->iBackingPixmap != 0)
		{
			XFreePixmap (s_XDisplay, icon->iBackingPixmap);
			icon->iBackingPixmap = 0;
		}
		
		cairo_dock_remove_appli_from_class (icon);
		cairo_dock_update_Xid_on_inhibators (icon->Xid, icon->cClass);
		
		icon->Xid = 0;  // hop, elle n'est plus une appli.
	}
}

static gboolean _cairo_dock_delete_one_appli (Window *pXid, Icon *pIcon, gpointer data)
{
	if (pIcon == NULL)
		return TRUE;
	if (pIcon->Xid == 0)
	{
		g_free (pIcon);
		return TRUE;
	}
	
	CairoDock *pDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	if (pDock != NULL)
	{
		gchar *cParentDockName = pIcon->cParentDockName;
		pIcon->cParentDockName = NULL;  // astuce.
		cairo_dock_detach_icon_from_dock (pIcon, pDock, myIcons.bUseSeparator);
		if (! pDock->bIsMainDock)  // la taille du main dock est mis a jour 1 fois a la fin.
		{
			if (pDock->icons == NULL)  // le dock degage, le fake aussi.
			{
				CairoDock *pFakeClassParentDock = NULL;
				Icon *pFakeClassIcon = cairo_dock_search_icon_pointing_on_dock (pDock, &pFakeClassParentDock);
				if (pFakeClassIcon != NULL && ! CAIRO_DOCK_IS_APPLI (pFakeClassIcon) && ! CAIRO_DOCK_IS_APPLET (pFakeClassIcon) && ! CAIRO_DOCK_IS_NORMAL_LAUNCHER (pFakeClassIcon) && pFakeClassIcon->cClass != NULL && pFakeClassIcon->acName != NULL && strcmp (pFakeClassIcon->cClass, pFakeClassIcon->acName) == 0)
				{
					cairo_dock_detach_icon_from_dock (pFakeClassIcon, pFakeClassParentDock, myIcons.bUseSeparator);
					cairo_dock_free_icon (pFakeClassIcon);
					if (! pFakeClassParentDock->bIsMainDock)
						cairo_dock_update_dock_size (pFakeClassParentDock);
				}
				
				cairo_dock_destroy_dock (pDock, cParentDockName, NULL, NULL);
			}
			else
				cairo_dock_update_dock_size (pDock);
		}
		g_free (cParentDockName);
	}
	
	cairo_dock_free_icon_buffers (pIcon);  // on ne veut pas passer dans le 'unregister' ni la gestion de la classe.
	//cairo_dock_unregister_pid (pIcon);
	g_free (pIcon);
	return TRUE;
}
void cairo_dock_reset_appli_table (void)
{
	g_hash_table_foreach_remove (s_hXWindowTable, (GHRFunc) _cairo_dock_delete_one_appli, NULL);
	cairo_dock_update_dock_size (g_pMainDock);
}


static gboolean _cairo_dock_window_is_on_our_way (Window *Xid, Icon *icon, int *data)
{
	if (icon != NULL && cairo_dock_xwindow_is_on_this_desktop (*Xid, data[0]))
	{
		gboolean bIsFullScreen, bIsHidden, bIsMaximized;
		cairo_dock_xwindow_is_fullscreen_or_hidden_or_maximized (*Xid, &bIsFullScreen, &bIsHidden, &bIsMaximized, NULL);
		if ((data[1] && bIsMaximized && ! bIsHidden) || (data[2] && bIsFullScreen))
		{
			cd_debug ("%s est genante (%d, %d) (%d;%d %dx%d)", icon->acName, bIsMaximized, bIsFullScreen, icon->windowGeometry.x, icon->windowGeometry.y, icon->windowGeometry.width, icon->windowGeometry.height);
			if (cairo_dock_window_hovers_dock (&icon->windowGeometry, g_pMainDock))
			{
				cd_debug (" et en plus elle empiete sur notre dock");
				return TRUE;
			}
		}
	}
	return FALSE;
}
Icon * cairo_dock_search_window_on_our_way (gboolean bMaximizedWindow, gboolean bFullScreenWindow)
{
	cd_debug ("%s (%d, %d)", __func__, bMaximizedWindow, bFullScreenWindow);
	if (! bMaximizedWindow && ! bFullScreenWindow)
		return NULL;
	int iDesktopNumber;
	iDesktopNumber = cairo_dock_get_current_desktop ();
	int data[3] = {iDesktopNumber, bMaximizedWindow, bFullScreenWindow};
	return g_hash_table_find (s_hXWindowTable, (GHRFunc) _cairo_dock_window_is_on_our_way, data);
}
static void _cairo_dock_hide_show_windows_on_other_desktops (Window *Xid, Icon *icon, int *pCurrentDesktop)
{
	g_return_if_fail (Xid != NULL && pCurrentDesktop != NULL);

	if (CAIRO_DOCK_IS_APPLI (icon) && (! myTaskBar.bHideVisibleApplis || icon->bIsHidden))
	{
		cd_debug ("%s (%d)", __func__, *Xid);
		CairoDock *pParentDock;

		if (cairo_dock_xwindow_is_on_this_desktop (*Xid, pCurrentDesktop[0]))
		{
			cd_debug (" => est sur le bureau actuel.");
			CairoDock *pMainDock = GINT_TO_POINTER (pCurrentDesktop[1]);
			pParentDock = cairo_dock_insert_appli_in_dock (icon, pMainDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
		}
		else
		{
			cd_debug (" => n'est pas sur le bureau actuel.");
			pParentDock = cairo_dock_detach_appli (icon);
		}
		if (pParentDock != NULL)
			gtk_widget_queue_draw (pParentDock->pWidget);
	}
}
gboolean cairo_dock_unstack_Xevents (CairoDock *pDock)
{
	static XEvent event;
	static gboolean bHackMeToo = FALSE;
	g_return_val_if_fail (pDock != NULL, FALSE);
	
	s_iTime ++;
	long event_mask = 0xFFFFFFFF;  // on les recupere tous, ca vide la pile au fur et a mesure plutot que tout a la fin.
	Window Xid;
	Window root = DefaultRootWindow (s_XDisplay);
	Icon *icon;
	if (bHackMeToo)
	{
		//g_print ("HACK ME\n");
		bHackMeToo = FALSE;
		if (pDock->bHorizontalDock)
			gdk_window_get_pointer (pDock->pWidget->window, &pDock->iMouseX, &pDock->iMouseY, NULL);
		else
			gdk_window_get_pointer (pDock->pWidget->window, &pDock->iMouseY, &pDock->iMouseX, NULL);
		cairo_dock_calculate_dock_icons (pDock);  // pour faire retrecir le dock si on n'est pas dedans, merci X de nous faire sortir du dock alors que la souris est toujours dedans :-/
	}
	
	while (XCheckMaskEvent (s_XDisplay, event_mask, &event))
	{
		icon = NULL;
		Xid = event.xany.window;
		//g_print ("  type : %d; atom : %s; window : %d\n", event.type, gdk_x11_get_xatom_name (event.xproperty.atom), Xid);
		//if (event.type == ClientMessage)
		//cd_message ("\n\n\n >>>>>>>>>>>< event.type : %d\n\n", event.type);
		if (event.type == PropertyNotify)
		{
			//g_print ("  type : %d; atom : %s; window : %d\n", event.xproperty.type, gdk_x11_get_xatom_name (event.xproperty.atom), Xid);
			if (Xid == root)  // PropertyNotify sur root
			{
				if (event.xproperty.atom == s_aNetClientList)
				{
					cairo_dock_update_applis_list (pDock, s_iTime);
					cairo_dock_notify (CAIRO_DOCK_WINDOW_CONFIGURED, NULL);
				}
				else if (event.xproperty.atom == s_aNetActiveWindow)
				{
					Window XActiveWindow = cairo_dock_get_active_xwindow ();
					//g_print ("%d devient active (%d)\n", XActiveWindow, root);
					if (s_iCurrentActiveWindow != XActiveWindow)
					{
						icon = g_hash_table_lookup (s_hXWindowTable, &XActiveWindow);
						CairoDock *pParentDock = NULL;
						if (CAIRO_DOCK_IS_APPLI (icon))
						{
							cd_message ("%s devient active", icon->acName);
							pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
							if (pParentDock == NULL)  // elle est inhibee.
							{
								cairo_dock_update_activity_on_inhibators (icon->cClass, icon->Xid);
							}
							else
							{
								cairo_dock_animate_icon_on_active (icon, pParentDock);
							}
							if (icon->bIsDemandingAttention)  // on force ici, car il semble qu'on ne recoive pas toujours le retour a la normale.
								cairo_dock_appli_stops_demanding_attention (icon);
						}
						
						gboolean bHackMe = FALSE;
						Icon *pLastActiveIcon = g_hash_table_lookup (s_hXWindowTable, &s_iCurrentActiveWindow);
						if (CAIRO_DOCK_IS_APPLI (pLastActiveIcon))
						{
							CairoDock *pLastActiveParentDock = cairo_dock_search_dock_from_name (pLastActiveIcon->cParentDockName);
							if (pLastActiveParentDock != NULL)
							{
								if (! pLastActiveParentDock->bIsShrinkingDown)
									cairo_dock_redraw_icon (pLastActiveIcon, CAIRO_CONTAINER (pLastActiveParentDock));
							}
							else
							{
								cairo_dock_update_inactivity_on_inhibators (pLastActiveIcon->cClass, pLastActiveIcon->Xid);
							}
						}
						else
							bHackMe = TRUE;
						s_iCurrentActiveWindow = XActiveWindow;
						cairo_dock_notify (CAIRO_DOCK_WINDOW_ACTIVATED, &XActiveWindow);
						if (bHackMe)  // si on active une fenetre n'ayant pas de focus clavier, on n'aura pas d'evenement kbd_changed, pourtant en interne le clavier changera. du coup si apres on revient sur une fenetre qui a un focus clavier, il risque de ne pas y avoir de changement de clavier, et donc encore une fois pas d'evenement ! pour palier a ce, on considere que les fenetres avec focus clavier sont celles presentes en barre des taches. On decide de generer un evenement lorsqu'on revient sur une fenetre avec focus, a partir d'une fenetre sans focus (mettre a jour le clavier pour une fenetre sans focus n'a pas grand interet, autant le laisser inchange).
							cairo_dock_notify (CAIRO_DOCK_KBD_STATE_CHANGED, &XActiveWindow);
					}
				}
				else if (event.xproperty.atom == s_aNetCurrentDesktop || event.xproperty.atom == s_aNetDesktopViewport)
				{
					cd_message ("on change de bureau");
					if (myTaskBar.bAppliOnCurrentDesktopOnly)
					{
						int iDesktopNumber = cairo_dock_get_current_desktop ();
						int data[2] = {iDesktopNumber, GPOINTER_TO_INT (pDock)};
						g_hash_table_foreach (s_hXWindowTable, (GHFunc) _cairo_dock_hide_show_windows_on_other_desktops, data);
					}
					if (cairo_dock_quick_hide_is_activated () && (myAccessibility.bAutoHideOnFullScreen || myAccessibility.bAutoHideOnMaximized))
					{
						if (cairo_dock_search_window_on_our_way (myAccessibility.bAutoHideOnMaximized, myAccessibility.bAutoHideOnFullScreen) == NULL)
						{
							cd_message (" => plus aucune fenetre genante");
							cairo_dock_deactivate_temporary_auto_hide ();
						}
					}
					else if (! cairo_dock_quick_hide_is_activated () && (myAccessibility.bAutoHideOnFullScreen || myAccessibility.bAutoHideOnMaximized))
					{
						if (cairo_dock_search_window_on_our_way (myAccessibility.bAutoHideOnMaximized, myAccessibility.bAutoHideOnFullScreen) != NULL)
						{
							cd_message (" => une fenetre est genante");
							cairo_dock_activate_temporary_auto_hide ();
						}
					}
					cd_message ("bureau change");
					cairo_dock_notify (CAIRO_DOCK_DESKTOP_CHANGED);  // , NULL
					
					if (! pDock->bIsShrinkingDown && ! pDock->bIsGrowingUp)
					{
						if (pDock->bHorizontalDock)
							gdk_window_get_pointer (pDock->pWidget->window, &pDock->iMouseX, &pDock->iMouseY, NULL);
						else
							gdk_window_get_pointer (pDock->pWidget->window, &pDock->iMouseY, &pDock->iMouseX, NULL);
						//g_print ("on met a jour de force\n");
						cairo_dock_calculate_dock_icons (pDock);  // pour faire retrecir le dock si on n'est pas dedans, merci X de nous faire sortir du dock alors que la souris est toujours dedans :-/
						if (pDock->bInside)
							bHackMeToo = TRUE;
						//g_print (">>> %d;%d, %dx%d\n", pDock->iMouseX, pDock->iMouseY,pDock->iCurrentWidth,  pDock->iCurrentHeight);
					}
				}
				else if (event.xproperty.atom == s_aNetNbDesktops)
				{
					cd_message ("changement du nombre de bureaux virtuels");
					
					g_iNbDesktops = cairo_dock_get_nb_desktops ();  // concretement ca ne change rien pour nous.
					
					cairo_dock_notify (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED);  // , NULL
				}
				else if (event.xproperty.atom == s_aNetDesktopGeometry)
				{
					cd_message ("geometrie du bureau alteree");
					
					cairo_dock_get_nb_viewports (&g_iNbViewportX, &g_iNbViewportY);
					
					if (cairo_dock_update_screen_geometry ())  // modification de la resolution.
					{
						cd_message ("resolution alteree");
						cairo_dock_reposition_root_docks (FALSE);  // main dock compris.
						/*if (myPosition.bUseXinerama)
							cairo_dock_get_screen_offsets (myPosition.iNumScreen);
						cairo_dock_update_dock_size (pDock);  // la taille max du dock a change, on recalcule son ratio.
						cairo_dock_set_window_position_at_balance (pDock, pDock->iCurrentWidth, pDock->iCurrentHeight);
						g_print (" -> le dock se place en %d;%d", pDock->iWindowPositionX, pDock->iWindowPositionY);
						gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);*/
					}
					cairo_dock_notify (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED);  // , NULL
				}
				else if (event.xproperty.atom == s_aRootMapID)
				{
					cd_message ("changement du fond d'ecran");
					if (mySystem.bUseFakeTransparency)
						cairo_dock_load_desktop_background_surface ();
					else
						cairo_dock_invalidate_desktop_bg_surface ();
					cairo_dock_notify (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED);  // , NULL
				}
				else if (event.xproperty.atom == s_aXKlavierState)
				{
					cairo_dock_notify (CAIRO_DOCK_KBD_STATE_CHANGED, NULL);
				}
			}
			else  // PropertyNotify sur une fenetre
			{
				if (event.xproperty.atom == s_aNetWmState)
				{
					gboolean bIsFullScreen, bIsHidden, bIsMaximized, bDemandsAttention;
					cairo_dock_xwindow_is_fullscreen_or_hidden_or_maximized (Xid, &bIsFullScreen, &bIsHidden, &bIsMaximized, &bDemandsAttention);
					cd_debug ("changement d'etat de %d => {%d ; %d ; %d ; %d}", Xid, bIsFullScreen, bIsHidden, bIsMaximized, bDemandsAttention);
					
					icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
					if (bDemandsAttention && (myTaskBar.bDemandsAttentionWithDialog || myTaskBar.cAnimationOnDemandsAttention))
					{
						if (CAIRO_DOCK_IS_APPLI (icon))  // elle peut demander l'attention plusieurs fois de suite.
						{
							cd_debug ("%s demande votre attention %s !", icon->acName, icon->bIsDemandingAttention?"encore une fois":"");
							cairo_dock_appli_demands_attention (icon);
						}
					}
					else if (! bDemandsAttention)
					{
						if (CAIRO_DOCK_IS_APPLI (icon) && icon->bIsDemandingAttention)
						{
							cd_debug ("%s se tait", icon->acName);
							cairo_dock_appli_stops_demanding_attention (icon);  // ca c'est plus une precaution qu'autre chose.
						}
					}
					if (myAccessibility.bAutoHideOnMaximized || myAccessibility.bAutoHideOnFullScreen)
					{
						if ( ((bIsMaximized && ! bIsHidden && myAccessibility.bAutoHideOnMaximized) || (bIsFullScreen && myAccessibility.bAutoHideOnFullScreen)) && ! cairo_dock_quick_hide_is_activated ())
						{
							cd_message (" => %s devient genante", CAIRO_DOCK_IS_APPLI (icon) ? icon->acName : "une fenetre");
							if (CAIRO_DOCK_IS_APPLI (icon) && cairo_dock_window_hovers_dock (&icon->windowGeometry, g_pMainDock))
								cairo_dock_activate_temporary_auto_hide ();
						}
						else if ((! bIsMaximized || ! myAccessibility.bAutoHideOnMaximized || bIsHidden) && (! bIsFullScreen || ! myAccessibility.bAutoHideOnFullScreen) && cairo_dock_quick_hide_is_activated ())
						{
							if (cairo_dock_search_window_on_our_way (myAccessibility.bAutoHideOnMaximized, myAccessibility.bAutoHideOnFullScreen) == NULL)
							{
								cd_message (" => plus aucune fenetre genante");
								cairo_dock_deactivate_temporary_auto_hide ();
							}
						}
					}
					
					if (CAIRO_DOCK_IS_APPLI (icon) && icon->fPersonnalScale <= 0)  // pour une icone en cours de supression, on ne fait rien.
					{
						CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
						///if (pParentDock == NULL)
						///	pParentDock = pDock;
						
						if (bIsHidden != icon->bIsHidden)
						{
							cd_message ("  changement de visibilite -> %d", bIsHidden);
							icon->bIsHidden = bIsHidden;
							cairo_dock_update_visibility_on_inhibators (icon->cClass, icon->Xid, icon->bIsHidden);
							
							/*if (cairo_dock_quick_hide_is_activated ())
							{
								if (cairo_dock_search_window_on_our_way (myAccessibility.bAutoHideOnMaximized, myAccessibility.bAutoHideOnFullScreen) == NULL)
									cairo_dock_deactivate_temporary_auto_hide ();
							}*/
							#ifdef HAVE_XEXTEND
							if (myTaskBar.bShowThumbnail && (pParentDock != NULL || myTaskBar.bHideVisibleApplis))  // on recupere la miniature ou au contraire on remet l'icone.
							{
								if (! icon->bIsHidden)  // fenetre mappee => BackingPixmap disponible.
								{
									if (icon->iBackingPixmap != 0)
										XFreePixmap (s_XDisplay, icon->iBackingPixmap);
									if (myTaskBar.bShowThumbnail)
										icon->iBackingPixmap = XCompositeNameWindowPixmap (s_XDisplay, Xid);
									else
										icon->iBackingPixmap = 0;
									cd_message ("new backing pixmap (bis) : %d", icon->iBackingPixmap);
								}
								// on redessine avec ou sans la miniature.
								cairo_dock_reload_one_icon_buffer_in_dock (icon, pParentDock ? pParentDock : g_pMainDock);
								if (pParentDock)
									cairo_dock_redraw_icon (icon, CAIRO_CONTAINER (pParentDock));
							}
							#endif
							if (myTaskBar.bHideVisibleApplis)  // on insere/detache l'icone selon la visibilite de la fenetre.
							{
								if (bIsHidden)  // se cache => on insere son icone.
								{
									cd_message (" => se cache");
									if (! myTaskBar.bAppliOnCurrentDesktopOnly || cairo_dock_xwindow_is_on_current_desktop (Xid))
									{
										pParentDock = cairo_dock_insert_appli_in_dock (icon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);  /// ! CAIRO_DOCK_ANIMATE_ICON
										if (pParentDock != NULL)
											gtk_widget_queue_draw (pParentDock->pWidget);
									}
								}
								else  // se montre => on detache l'icone.
								{
									cd_message (" => re-apparait");
									///pParentDock = cairo_dock_detach_appli (icon);
									///if (pParentDock != NULL)
									///	gtk_widget_queue_draw (pParentDock->pWidget);
									pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
									if (pParentDock)
									{
										cairo_dock_stop_icon_animation (icon);
										icon->fPersonnalScale = 1.0;
										cairo_dock_notify (CAIRO_DOCK_REMOVE_ICON, icon, pParentDock);
										cairo_dock_launch_animation (CAIRO_CONTAINER (pParentDock));
									}
								}
							}
							else if (myTaskBar.fVisibleAppliAlpha != 0)
							{
								icon->fAlpha = 1;  // on triche un peu.
								if (pParentDock != NULL)
									cairo_dock_redraw_icon (icon, CAIRO_CONTAINER (pParentDock));
							}
						}
					}
				}
				else if (event.xproperty.atom == s_aNetWmDesktop)  // cela ne gere pas les changements de viewports, qui eux se font en changeant les coordonnees. Il faut donc recueillir les ConfigureNotify, qui donnent les redimensionnements et les deplacements.
				{
					cd_message ("changement de bureau pour %d", Xid);
					icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
					if (CAIRO_DOCK_IS_APPLI (icon))
					{
						icon->iNumDesktop = cairo_dock_get_xwindow_desktop (Xid);
						if (myTaskBar.bAppliOnCurrentDesktopOnly)
						{
							int iDesktopNumber = cairo_dock_get_current_desktop ();
	
							int data[2] = {iDesktopNumber, GPOINTER_TO_INT (pDock)};
							_cairo_dock_hide_show_windows_on_other_desktops (&Xid, icon, data);
						}
					}
				}
				else if (event.xproperty.atom == s_aXKlavierState)
				{
					cairo_dock_notify (CAIRO_DOCK_KBD_STATE_CHANGED, &Xid);
				}
				else
				{
					icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
					if (CAIRO_DOCK_IS_APPLI (icon) && icon->fPersonnalScale <= 0)  // pour une icone en cours de supression, on ne fait rien.
					{
						CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
						if (pParentDock == NULL)
							pParentDock = pDock;
						cairo_dock_Xproperty_changed (icon, event.xproperty.atom, event.xproperty.state, pParentDock);
					}
				}
			}
		}
		else if (event.type == ConfigureNotify)
		{
			//g_print ("  type : %d; (%d;%d) %dx%d window : %d\n", event.xconfigure.type, event.xconfigure.x, event.xconfigure.y, event.xconfigure.width, event.xconfigure.height, Xid);
			icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
			if (CAIRO_DOCK_IS_APPLI (icon))
			{
				#ifdef HAVE_XEXTEND
				if (event.xconfigure.width != icon->windowGeometry.width || event.xconfigure.height != icon->windowGeometry.height)
				{
					if (icon->iBackingPixmap != 0)
						XFreePixmap (s_XDisplay, icon->iBackingPixmap);
					if (myTaskBar.bShowThumbnail)
						icon->iBackingPixmap = XCompositeNameWindowPixmap (s_XDisplay, Xid);
					cd_message ("new backing pixmap : %d", icon->iBackingPixmap);
				}
				#endif
				memcpy (&icon->windowGeometry, &event.xconfigure.x, sizeof (GtkAllocation));
			}
			
			if (myTaskBar.bAppliOnCurrentDesktopOnly)
			{
				if (CAIRO_DOCK_IS_APPLI (icon) && icon->fPersonnalScale <= 0)  // pour une icone en cours de supression, on ne fait rien.
				{
					if (event.xconfigure.x + event.xconfigure.width <= 0 || event.xconfigure.x >= g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] || event.xconfigure.y + event.xconfigure.height <= 0 || event.xconfigure.y >= g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL])  // en fait il faudrait faire ca modulo le nombre de viewports * la largeur d'un bureau, car avec une fenetre a droite, elle peut revenir sur le bureau par la gauche si elle est tres large...
					{
						CairoDock *pParentDock = cairo_dock_detach_appli (icon);
						if (pParentDock)
							gtk_widget_queue_draw (pParentDock->pWidget);
					}
					else  // elle est sur le bureau.
					{
						cd_message ("cette fenetre s'est deplacee sur le bureau courant (%d;%d)", event.xconfigure.x, event.xconfigure.y);
						gboolean bInsideDock = (icon->cParentDockName != NULL);  // jamais verifie mais ca devrait etre bon.
						if (! bInsideDock)
							cairo_dock_insert_appli_in_dock (icon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
					}
				}
			}
			cairo_dock_notify (CAIRO_DOCK_WINDOW_CONFIGURED, &event.xconfigure);
		}
		/*else if (event.type == g_iDamageEvent + XDamageNotify)
		{
			XDamageNotifyEvent *e = (XDamageNotifyEvent *) &event;
			g_print ("window %s has been damaged (%d;%d %dx%d)\n", e->drawable, e->area.x, e->area.y, e->area.width, e->area.height);
			// e->drawable is the window ID of the damaged window
			// e->geometry is the geometry of the damaged window	
			// e->area     is the bounding rect for the damaged area	
			// e->damage   is the damage handle returned by XDamageCreate()
	
			// Subtract all the damage, repairing the window.
			XDamageSubtract (s_XDisplay, e->damage, None, None);
		}
		else
			g_print ("  type : %d (%d); window : %d\n", event.type, XDamageNotify, Xid);*/
	}
	if (XEventsQueued (s_XDisplay, QueuedAlready) != 0)
		XSync (s_XDisplay, True);  // True <=> discard.
	//g_print ("XEventsQueued : %d\n", XEventsQueued (s_XDisplay, QueuedAfterFlush));  // QueuedAlready, QueuedAfterReading, QueuedAfterFlush
	
	return TRUE;
}


static gboolean _cairo_dock_remove_old_applis (Window *Xid, Icon *icon, gpointer iTimePtr)
{
	if (icon == NULL)
		return FALSE;
	gint iTime = GPOINTER_TO_INT (iTimePtr);
	
	//g_print ("%s (%s, %f / %f)\n", __func__, icon->acName, icon->fLastCheckTime, *fTime);
	if (icon->iLastCheckTime >= 0 && icon->iLastCheckTime < iTime && icon->fPersonnalScale <= 0)
	{
		cd_message ("cette fenetre (%ld, %s) est trop vieille (%d / %d)", *Xid, icon->acName, icon->iLastCheckTime, iTime);
		if (CAIRO_DOCK_IS_APPLI (icon))
		{
			if (cairo_dock_quick_hide_is_activated () && (myAccessibility.bAutoHideOnFullScreen || myAccessibility.bAutoHideOnMaximized))
			{
				if (cairo_dock_search_window_on_our_way (myAccessibility.bAutoHideOnMaximized, myAccessibility.bAutoHideOnFullScreen) == NULL)
				{
					cd_message (" => plus aucune fenetre genante");
					cairo_dock_deactivate_temporary_auto_hide ();
				}
			}
			
			CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
			if (pParentDock != NULL)
			{
				if (! pParentDock->bInside && (pParentDock->bAutoHide || pParentDock->iRefCount != 0) && pParentDock->bAtBottom)
					icon->fPersonnalScale = 0.05;
				else
				{
					cairo_dock_stop_icon_animation (icon);
					icon->fPersonnalScale = 1.0;
					cairo_dock_notify (CAIRO_DOCK_REMOVE_ICON, icon, pParentDock);
				}
				//g_print ("icon->fPersonnalScale <- %.2f\n", icon->fPersonnalScale);
				
				icon->iLastCheckTime = -1;  // inutile de chercher a la desenregistrer par la suite, puisque ce sera fait ici. Cela sert aussi a bien supprimer l'icone en fin d'animation.
				///cairo_dock_unregister_appli (icon);
				cairo_dock_remove_appli_from_class (icon);  // elle reste une icone d'appli, et de la meme classe, mais devient invisible aux autres icones de sa classe. Inutile de tester les inhibiteurs, puisqu'elle est dans un dock.
				
				//cairo_dock_start_icon_animation (icon, pParentDock);
				cairo_dock_launch_animation (CAIRO_CONTAINER (pParentDock));
			}
			else
			{
				cd_message ("  pas dans un container, on la detruit donc immediatement");
				cairo_dock_update_name_on_inhibators (icon->cClass, *Xid, NULL);
				icon->iLastCheckTime = -1;  // pour ne pas la desenregistrer de la HashTable lors du 'free' puisqu'on est en train de parcourir la table.
				cairo_dock_free_icon (icon);
				/// redessiner les inhibiteurs...
			}
		}
		else
		{
			g_free (icon);
		}
		return TRUE;
	}
	return FALSE;
}
void cairo_dock_update_applis_list (CairoDock *pDock, gint iTime)
{
	gulong i, iNbWindows = 0;
	Window *pXWindowsList = cairo_dock_get_windows_list (&iNbWindows);
	
	Window Xid;
	Icon *icon;
	int iStackOrder = 0;
	gpointer pOriginalXid;
	gboolean bAppliAlreadyRegistered;
	gboolean bUpdateMainDockSize = FALSE;
	CairoDock *pParentDock;
	cairo_t *pCairoContext = NULL;
	
	cd_debug ("%s (%d)", __func__, iNbWindows);
	for (i = 0; i < iNbWindows; i ++)
	{
		Xid = pXWindowsList[i];

		bAppliAlreadyRegistered = g_hash_table_lookup_extended (s_hXWindowTable, &Xid, &pOriginalXid, (gpointer *) &icon);
		if (! bAppliAlreadyRegistered)
		{
			cd_message (" cette fenetre (%ld) de la pile n'est pas dans la liste", Xid);
			if (pCairoContext == NULL)
				pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
			if (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS)
				icon = cairo_dock_create_icon_from_xwindow (pCairoContext, Xid, pDock);
			else
				cd_warning ("couldn't create a cairo context => this window (%ld) will not have an icon", Xid);
			if (icon != NULL)
			{
				icon->iLastCheckTime = iTime;
				icon->iStackOrder = iStackOrder ++;
				if (/*(icon->bIsHidden || ! myTaskBar.bHideVisibleApplis) && */(! myTaskBar.bAppliOnCurrentDesktopOnly || cairo_dock_xwindow_is_on_current_desktop (Xid)))
				{
					cd_message (" insertion de %s ... (%d)", icon->acName, icon->iLastCheckTime);
					pParentDock = cairo_dock_insert_appli_in_dock (icon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
					if (pParentDock != NULL)
					{
						if (pParentDock->bIsMainDock)
							bUpdateMainDockSize = TRUE;
						else
							cairo_dock_update_dock_size (pParentDock);
					}
				}
				else if (myTaskBar.bMixLauncherAppli)  // on met tout de meme l'indicateur sur le lanceur.
				{
					cairo_dock_prevent_inhibated_class (icon);
				}
				if ((myAccessibility.bAutoHideOnMaximized && icon->bIsMaximized) || (myAccessibility.bAutoHideOnFullScreen && icon->bIsFullScreen))
				{
					if (! cairo_dock_quick_hide_is_activated ())
					{
						int iDesktopNumber = cairo_dock_get_current_desktop ();
						if (cairo_dock_xwindow_is_on_this_desktop (Xid, iDesktopNumber) && cairo_dock_window_hovers_dock (&icon->windowGeometry, pDock))
						{
							cd_message (" cette nouvelle fenetre empiete sur notre dock");
							cairo_dock_activate_temporary_auto_hide ();
						}
					}
				}
			}
			else
				cairo_dock_blacklist_appli (Xid);
		}
		else if (icon != NULL)
		{
			icon->iLastCheckTime = iTime;
			if (CAIRO_DOCK_IS_APPLI (icon))
				icon->iStackOrder = iStackOrder ++;
		}
	}
	if (pCairoContext != NULL)
		cairo_destroy (pCairoContext);
	
	g_hash_table_foreach_remove (s_hXWindowTable, (GHRFunc) _cairo_dock_remove_old_applis, GINT_TO_POINTER (iTime));
	
	if (bUpdateMainDockSize)
		cairo_dock_update_dock_size (pDock);

	XFree (pXWindowsList);
}


void cairo_dock_start_application_manager (CairoDock *pDock)
{
	g_return_if_fail (s_iSidUpdateAppliList == 0);
	
	cairo_dock_set_overwrite_exceptions (myTaskBar.cOverwriteException);
	cairo_dock_set_group_exceptions (myTaskBar.cGroupException);
	
	//\__________________ On recupere l'ensemble des fenetres presentes.
	Window root = DefaultRootWindow (s_XDisplay);
	cairo_dock_set_xwindow_mask (root, PropertyChangeMask /*| StructureNotifyMask | SubstructureNotifyMask | ResizeRedirectMask | SubstructureRedirectMask*/);

	gulong i, iNbWindows = 0;
	Window *pXWindowsList = cairo_dock_get_windows_list (&iNbWindows);

	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);

	int iDesktopNumber = cairo_dock_get_current_desktop ();
	
	//\__________________ On cree les icones de toutes ces applis.
	CairoDock *pParentDock;
	gboolean bUpdateMainDockSize = FALSE;
	int iStackOrder = 0;
	Window Xid;
	Icon *pIcon;
	for (i = 0; i < iNbWindows; i ++)
	{
		Xid = pXWindowsList[i];
		pIcon = cairo_dock_create_icon_from_xwindow (pCairoContext, Xid, pDock);
		
		if (pIcon != NULL)
		{
			pIcon->iStackOrder = iStackOrder ++;
			if (/*(pIcon->bIsHidden || ! myTaskBar.bHideVisibleApplis) && */(! myTaskBar.bAppliOnCurrentDesktopOnly || cairo_dock_xwindow_is_on_current_desktop (Xid)))
			{
				pParentDock = cairo_dock_insert_appli_in_dock (pIcon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
				//g_print (">>>>>>>>>>>> Xid : %d\n", Xid);
				if (pParentDock != NULL)
				{
					if (pParentDock->bIsMainDock)
						bUpdateMainDockSize = TRUE;
					else
						cairo_dock_update_dock_size (pParentDock);
				}
			}
			else if (myTaskBar.bMixLauncherAppli)  // on met tout de meme l'indicateur sur le lanceur.
			{
				cairo_dock_prevent_inhibated_class (pIcon);
			}
			if ((myAccessibility.bAutoHideOnMaximized && pIcon->bIsMaximized) || (myAccessibility.bAutoHideOnFullScreen && pIcon->bIsFullScreen))
			{
				if (! cairo_dock_quick_hide_is_activated () && cairo_dock_xwindow_is_on_this_desktop (pIcon->Xid, iDesktopNumber))
				{
					if (cairo_dock_window_hovers_dock (&pIcon->windowGeometry, pDock))
					{
						cd_message (" elle empiete sur notre dock");
						cairo_dock_activate_temporary_auto_hide ();
					}
				}
			}
		}
		else
			cairo_dock_blacklist_appli (Xid);
	}
	cairo_destroy (pCairoContext);
	if (pXWindowsList != NULL)
		XFree (pXWindowsList);
	
	if (bUpdateMainDockSize)
		cairo_dock_update_dock_size (pDock);
	
	//\__________________ On lance le gestionnaire d'evenements X.
	s_iSidUpdateAppliList = g_timeout_add (CAIRO_DOCK_TASKBAR_CHECK_INTERVAL, (GSourceFunc) cairo_dock_unstack_Xevents, (gpointer) pDock);  // un g_idle_add () consomme 90% de CPU ! :-/
	
	if (s_iCurrentActiveWindow == 0)
		s_iCurrentActiveWindow = cairo_dock_get_active_xwindow ();
}

void cairo_dock_pause_application_manager (void)
{
	if (s_iSidUpdateAppliList != 0)
	{
		Window root = DefaultRootWindow (s_XDisplay);
		cairo_dock_set_xwindow_mask (root, 0);
		
		XSync (s_XDisplay, True);  // True <=> discard.
		
		g_source_remove (s_iSidUpdateAppliList);
		s_iSidUpdateAppliList = 0;
	}
}

void cairo_dock_stop_application_manager (void)
{
	cairo_dock_pause_application_manager ();
	
	cairo_dock_remove_all_applis_from_class_table ();  // enleve aussi les indicateurs.
	
	cairo_dock_reset_appli_table ();  // libere les icones des applis.
}

gboolean cairo_dock_application_manager_is_running (void)
{
	return (s_iSidUpdateAppliList != 0);
}


GList *cairo_dock_get_current_applis_list (void)
{
	return g_hash_table_get_values (s_hXWindowTable);
}

Window cairo_dock_get_current_active_window (void)
{
	return s_iCurrentActiveWindow;
}

Icon *cairo_dock_get_current_active_icon (void)
{
	Icon *pIcon = g_hash_table_lookup (s_hXWindowTable, &s_iCurrentActiveWindow);
	if (CAIRO_DOCK_IS_APPLI (pIcon))
		return pIcon;
	else
		return NULL;
}

Icon *cairo_dock_get_icon_with_Xid (Window Xid)
{
	Icon *pIcon = g_hash_table_lookup (s_hXWindowTable, &Xid);
	if (CAIRO_DOCK_IS_APPLI (pIcon))
		return pIcon;
	else
		return NULL;
}


static void _cairo_dock_for_one_appli (Window *Xid, Icon *icon, gpointer *data)
{
	if (! CAIRO_DOCK_IS_APPLI (icon))
		return ;
	CairoDockForeachIconFunc pFunction = data[0];
	gpointer pUserData = data[1];
	gboolean bOutsideDockOnly =  GPOINTER_TO_INT (data[2]);
	
	if ((bOutsideDockOnly && icon->cParentDockName == NULL) || ! bOutsideDockOnly)
	{
		CairoDock *pParentDock = NULL;
		if (icon->cParentDockName != NULL)
			pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
		else
			pParentDock = g_pMainDock;
		pFunction (icon, CAIRO_CONTAINER (pParentDock), pUserData);
	}
}
void cairo_dock_foreach_applis (CairoDockForeachIconFunc pFunction, gboolean bOutsideDockOnly, gpointer pUserData)
{
	gpointer data[3] = {pFunction, pUserData, GINT_TO_POINTER (bOutsideDockOnly)};
	g_hash_table_foreach (s_hXWindowTable, (GHFunc) _cairo_dock_for_one_appli, data);
}



CairoDock *cairo_dock_insert_appli_in_dock (Icon *icon, CairoDock *pMainDock, gboolean bUpdateSize, gboolean bAnimate)
{
	cd_message ("%s (%s, %d)", __func__, icon->acName, icon->Xid);
	
	//\_________________ On gere ses eventuels inhibiteurs.
	if (myTaskBar.bMixLauncherAppli && cairo_dock_prevent_inhibated_class (icon))
	{
		cd_message (" -> se fait inhiber");
		return NULL;
	}
	
	//\_________________ On gere les filtres.
	if (!icon->bIsHidden && myTaskBar.bHideVisibleApplis)
		return NULL;
	
	//\_________________ On determine dans quel dock l'inserer.
	CairoDock *pParentDock = cairo_dock_manage_appli_class (icon, pMainDock);  // renseigne cParentDockName.
	g_return_val_if_fail (pParentDock != NULL, NULL);

	//\_________________ On l'insere dans son dock parent en animant ce dernier eventuellement.
	cairo_dock_insert_icon_in_dock (icon, pParentDock, bUpdateSize, bAnimate);
	cd_message (" insertion de %s complete (%.2f %.2fx%.2f) dans %s", icon->acName, icon->fPersonnalScale, icon->fWidth, icon->fHeight, icon->cParentDockName);

	if (bAnimate && cairo_dock_animation_will_be_visible (pParentDock))
	{
		///cairo_dock_notify (CAIRO_DOCK_INSERT_ICON, icon, pParentDock);
		//cairo_dock_start_icon_animation (icon, pParentDock);
		cairo_dock_launch_animation (CAIRO_CONTAINER (pParentDock));
	}
	else
	{
		icon->fPersonnalScale = 0;
		icon->fScale = 1.;
	}

	return pParentDock;
}

CairoDock * cairo_dock_detach_appli (Icon *pIcon)
{
	cd_debug ("%s (%s)", __func__, pIcon->acName);
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	if (pParentDock == NULL)
		return NULL;
	
	cairo_dock_detach_icon_from_dock (pIcon, pParentDock, TRUE);
	
	if (pIcon->cClass != NULL && pParentDock == cairo_dock_search_dock_from_name (pIcon->cClass))
	{
		gboolean bEmptyClassSubDock = cairo_dock_check_class_subdock_is_empty (pParentDock, pIcon->cClass);
		if (bEmptyClassSubDock)
			return NULL;
	}
	cairo_dock_update_dock_size (pParentDock);
	return pParentDock;
}

void cairo_dock_animate_icon_on_active (Icon *icon, CairoDock *pParentDock)
{
	g_return_if_fail (pParentDock != NULL);
	if (icon->fPersonnalScale == 0)  // sinon on laisse l'animation actuelle.
	{
		if (myTaskBar.cAnimationOnActiveWindow)
		{
			if (cairo_dock_animation_will_be_visible (pParentDock) && ! pParentDock->bInside && icon->iAnimationState == CAIRO_DOCK_STATE_REST)
			cairo_dock_request_icon_animation (icon, pParentDock, myTaskBar.cAnimationOnActiveWindow, 1);
		}
		else if (! pParentDock->bIsShrinkingDown)
		{
			cairo_dock_redraw_icon (icon, CAIRO_CONTAINER (pParentDock));
		}
	}
}

void  cairo_dock_set_one_icon_geometry_for_window_manager (Icon *icon, CairoDock *pDock)
{
	int iX, iY, iWidth, iHeight;
	iX = pDock->iWindowPositionX + icon->fXAtRest + (pDock->iCurrentWidth - pDock->fFlatDockWidth) / 2;
	iY = pDock->iWindowPositionY + icon->fDrawY - icon->fHeight * myIcons.fAmplitude * pDock->fMagnitudeMax;  // il faudrait un fYAtRest ...
	iWidth = icon->fWidth;
	iHeight = icon->fHeight * (1. + 2*myIcons.fAmplitude * pDock->fMagnitudeMax);  // on elargit en haut et en bas, pour gerer les cas ou l'icone grossirait vers le haut ou vers le bas.

	if (pDock->bHorizontalDock)
		cairo_dock_set_xicon_geometry (icon->Xid, iX, iY, iWidth, iHeight);
	else
		cairo_dock_set_xicon_geometry (icon->Xid, iY, iX, iHeight, iWidth);
}

void cairo_dock_set_icons_geometry_for_window_manager (CairoDock *pDock)
{
	if (s_iSidUpdateAppliList <= 0)
		return ;
	//g_print ("%s ()\n", __func__);

	Icon *icon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (CAIRO_DOCK_IS_APPLI (icon))
		{
			cairo_dock_set_one_icon_geometry_for_window_manager (icon, pDock);
		}
	}
}
