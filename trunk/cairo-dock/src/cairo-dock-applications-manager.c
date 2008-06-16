/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <cairo.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <signal.h>

#include "cairo-dock-icons.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-load.h"
#include "cairo-dock-application-factory.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-log.h"
#include "cairo-dock-X-utilities.h"
#include "cairo-dock-config.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-class-manager.h"
#include "cairo-dock-applications-manager.h"

#define CAIRO_DOCK_TASKBAR_CHECK_INTERVAL 250

extern int kill (__pid_t pid, int sig);

extern CairoDock *g_pMainDock;
extern double g_fAmplitude;
extern gboolean g_bUseSeparator;

extern int g_iScreenWidth[2], g_iScreenHeight[2];

extern gboolean g_bUniquePid;
extern gboolean g_bAnimateOnActiveWindow;
extern gboolean g_bAutoHideOnFullScreen;
extern gboolean g_bAutoHideOnMaximized;
extern gboolean g_bHideVisibleApplis;
extern double g_fVisibleAppliAlpha;
extern gboolean g_bAppliOnCurrentDesktopOnly;
extern gboolean g_bGroupAppliByClass;
extern int g_iNbDesktops;
extern int g_iNbViewportX,g_iNbViewportY ;
extern gboolean g_bMixLauncherAppli;
extern gboolean g_bUseFakeTransparency;

static GHashTable *s_hXWindowTable = NULL;  // table des fenetres X affichees dans le dock.
static Display *s_XDisplay = NULL;
static int s_iSidUpdateAppliList = 0;
static Atom s_aNetClientList;
static Atom s_aNetActiveWindow;
static Atom s_aNetCurrentDesktop;
static Atom s_aNetDesktopViewport;
static Atom s_aNetDesktopGeometry;
static Atom s_aNetWmState;
static Atom s_aNetWmFullScreen;
static Atom s_aNetWmAbove;
static Atom s_aNetWmBelow;
static Atom s_aNetWmHidden;
static Atom s_aNetWmDesktop;
static Atom s_aNetWmMaximizedHoriz;
static Atom s_aNetWmMaximizedVert;
static Atom s_aRootMapID;
static Atom s_aNetNbDesktops;

void cairo_dock_initialize_application_manager (Display *pDisplay)
{
	s_XDisplay = pDisplay;

	s_hXWindowTable = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	
	s_aNetClientList = XInternAtom (s_XDisplay, "_NET_CLIENT_LIST", False);
	s_aNetActiveWindow = XInternAtom (s_XDisplay, "_NET_ACTIVE_WINDOW", False);
	s_aNetCurrentDesktop = XInternAtom (s_XDisplay, "_NET_CURRENT_DESKTOP", False);
	s_aNetDesktopViewport = XInternAtom (s_XDisplay, "_NET_DESKTOP_VIEWPORT", False);
	s_aNetDesktopGeometry = XInternAtom (s_XDisplay, "_NET_DESKTOP_GEOMETRY", False);
	s_aNetWmState = XInternAtom (s_XDisplay, "_NET_WM_STATE", False);
	s_aNetWmFullScreen = XInternAtom (s_XDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	s_aNetWmAbove = XInternAtom (s_XDisplay, "_NET_WM_STATE_ABOVE", False);
	s_aNetWmBelow = XInternAtom (s_XDisplay, "_NET_WM_STATE_BELOW", False);
	s_aNetWmHidden = XInternAtom (s_XDisplay, "_NET_WM_STATE_HIDDEN", False);
	s_aNetWmDesktop = XInternAtom (s_XDisplay, "_NET_WM_DESKTOP", False);
	s_aNetWmMaximizedHoriz = XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	s_aNetWmMaximizedVert = XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	s_aRootMapID = XInternAtom (s_XDisplay, "_XROOTPMAP_ID", False);
	s_aNetNbDesktops = XInternAtom (s_XDisplay, "_NET_NUMBER_OF_DESKTOPS", False);
}


void cairo_dock_register_appli (Icon *icon)
{
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
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
		Window *pXid = g_new (Window, 1);
			*pXid = Xid;
		g_hash_table_insert (s_hXWindowTable, pXid, NULL);
	}
}

void cairo_dock_unregister_appli (Icon *icon)
{
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cd_message ("%s (%s)", __func__, icon->acName);
		if (icon->fLastCheckTime != -1)
			g_hash_table_remove (s_hXWindowTable, &icon->Xid);
		
		cairo_dock_unregister_pid (icon);  // on n'efface pas sa classe ici car on peut en avoir besoin encore.
		
		cairo_dock_remove_appli_from_class (icon);
		cairo_dock_update_Xid_on_inhibators (icon->Xid, icon->cClass);
		
		icon->Xid = 0;  // hop, elle n'est plus une appli.
	}
}

static gboolean _cairo_dock_delete_one_appli (Window *pXid, Icon *pIcon, gpointer data)
{
	if (pIcon == NULL)
		return TRUE;
	
	CairoDock *pDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	if (pDock != NULL)
	{
		gchar *cParentDockName = pIcon->cParentDockName;
		pIcon->cParentDockName = NULL;  // astuce.
		cairo_dock_detach_icon_from_dock (pIcon, pDock, g_bUseSeparator);
		if (pDock->icons == NULL)
			cairo_dock_destroy_dock (pDock, cParentDockName, NULL, NULL);
		else if (! pDock->bIsMainDock)
			cairo_dock_update_dock_size (pDock);
		g_free (cParentDockName);
	}
	
	cairo_dock_free_icon_buffers (pIcon);  // on ne veut pas passer dans le 'unregister' ni la gestion de la classe.
	cairo_dock_unregister_pid (pIcon);
	g_free (pIcon);
	return TRUE;
}
void cairo_dock_reset_appli_table (void)
{
	g_hash_table_foreach_remove (s_hXWindowTable, (GHRFunc) _cairo_dock_delete_one_appli, NULL);
	cairo_dock_update_dock_size (g_pMainDock);
}


void  cairo_dock_set_one_icon_geometry_for_window_manager (Icon *icon, CairoDock *pDock)
{
	int iX, iY, iWidth, iHeight;
	iX = pDock->iWindowPositionX + icon->fXAtRest + (pDock->iCurrentWidth - pDock->fFlatDockWidth) / 2;
	iY = pDock->iWindowPositionY + icon->fDrawY - icon->fHeight * g_fAmplitude;  // il faudrait un fYAtRest ...
	iWidth = icon->fWidth;
	iHeight = icon->fHeight * (1. + 2. * g_fAmplitude);  // on elargit en haut et en bas, pour gerer les cas ou l'icone grossirait vers le haut ou vers le bas.

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


void cairo_dock_close_xwindow (Window Xid)
{
	//g_print ("%s (%d)\n", __func__, Xid);
	g_return_if_fail (Xid > 0);
	
	if (g_bUniquePid)
	{
		gulong *pPidBuffer = NULL;
		Atom aReturnedType = 0;
		int aReturnedFormat = 0;
		unsigned long iLeftBytes, iBufferNbElements = 0;
		Atom aNetWmPid = XInternAtom (s_XDisplay, "_NET_WM_PID", False);
		iBufferNbElements = 0;
		XGetWindowProperty (s_XDisplay, Xid, aNetWmPid, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pPidBuffer);
		if (iBufferNbElements > 0)
		{
			cd_message ("kill (%ld)", pPidBuffer[0]);
			kill (pPidBuffer[0], 1);  // 1 : HUP, 2 : INT, 3 : QUIT, 15 : TERM.
		}
		XFree (pPidBuffer);
	}
	else
	{
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
	int iDesktopNumber = cairo_dock_get_window_desktop (Xid);
	cairo_dock_set_current_desktop (iDesktopNumber);

	//\______________ On active la fenetre.
	//XMapRaised (s_XDisplay, Xid);  // on la mappe, pour les cas ou elle etait en zone de notification. Malheuresement, la zone de notif de gnome est bugguee, et reduit la fenetre aussitot qu'on l'a mappee :-(
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_ACTIVE_WINDOW", False);
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

void cairo_dock_minimize_xwindow (Window Xid)
{
	g_return_if_fail (Xid > 0);
	XIconifyWindow (s_XDisplay, Xid, DefaultScreen (s_XDisplay));
	//Window root = DefaultRootWindow (s_XDisplay);
	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}

static void _cairo_dock_change_window_state (Window Xid, gulong iNewValue, Atom iProperty1, Atom iProperty2)
{
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;

	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = s_aNetWmState;
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = iNewValue;
	xClientMessage.xclient.data.l[1] = iProperty1;
	xClientMessage.xclient.data.l[2] = iProperty2;
	xClientMessage.xclient.data.l[3] = 2;
	xClientMessage.xclient.data.l[4] = 0;

	Window root = DefaultRootWindow (s_XDisplay);
	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);

	cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}
void cairo_dock_maximize_xwindow (Window Xid, gboolean bMaximize)
{
	_cairo_dock_change_window_state (Xid, bMaximize, s_aNetWmMaximizedVert, s_aNetWmMaximizedHoriz);
}

void cairo_dock_set_xwindow_fullscreen (Window Xid, gboolean bFullScreen)
{
	_cairo_dock_change_window_state (Xid, bFullScreen, s_aNetWmFullScreen, 0);
}

void cairo_dock_set_xwindow_above (Window Xid, gboolean bAbove)
{
	_cairo_dock_change_window_state (Xid, bAbove, s_aNetWmAbove, 0);
}


void cairo_dock_move_xwindow_to_nth_desktop (Window Xid, int iDesktopNumber, int iDesktopViewportX, int iDesktopViewportY)
{
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;

	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_WM_DESKTOP", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = iDesktopNumber;
	xClientMessage.xclient.data.l[1] = 2;
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;

	Window root = DefaultRootWindow (s_XDisplay);
	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
	
	int iRelativePositionX, iRelativePositionY;
	cairo_dock_get_window_position_on_its_viewport (Xid, &iRelativePositionX, &iRelativePositionY);

	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_MOVERESIZE_WINDOW", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = StaticGravity | (1 << 8) | (1 << 9) | (0 << 10) | (0 << 11);
	xClientMessage.xclient.data.l[1] = iDesktopViewportX + iRelativePositionX;  // coordonnees dans le referentiel du viewport desire.
	xClientMessage.xclient.data.l[2] = iDesktopViewportY + iRelativePositionY;
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;
	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);

	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}


gboolean cairo_dock_window_is_maximized (Window Xid)
{
	g_return_val_if_fail (Xid > 0, FALSE);

	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	int iIsMaximized = 0;
	if (iBufferNbElements > 0)
	{
		int i;
		for (i = 0; i < iBufferNbElements && iIsMaximized < 2; i ++)
		{
			if (pXStateBuffer[i] == s_aNetWmMaximizedVert)
				iIsMaximized ++;
			if (pXStateBuffer[i] == s_aNetWmMaximizedHoriz)
				iIsMaximized ++;
		}
	}
	XFree (pXStateBuffer);

	return (iIsMaximized == 2);
}

static gboolean _cairo_dock_window_is_in_state (Window Xid, Atom iState)
{
	g_return_val_if_fail (Xid > 0, FALSE);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);

	gboolean bIsInState = FALSE;
	if (iBufferNbElements > 0)
	{
		int i;
		for (i = 0; i < iBufferNbElements; i ++)
		{
			if (pXStateBuffer[i] == iState)
			{
				bIsInState = TRUE;
				break;
			}
		}
	}

	XFree (pXStateBuffer);
	return bIsInState;
}

gboolean cairo_dock_window_is_fullscreen (Window Xid)
{
	return _cairo_dock_window_is_in_state (Xid, s_aNetWmFullScreen);
}
void cairo_dock_window_is_above_or_below (Window Xid, gboolean *bIsAbove, gboolean *bIsBelow)
{
	g_return_if_fail (Xid > 0);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);

	if (iBufferNbElements > 0)
	{
		int i;
		//g_print ("iBufferNbElements : %d (%d;%d)\n", iBufferNbElements, s_aNetWmAbove, s_aNetWmBelow);
		for (i = 0; i < iBufferNbElements; i ++)
		{
			//g_print (" - %d\n", pXStateBuffer[i]);
			if (pXStateBuffer[i] == s_aNetWmAbove)
			{
				*bIsAbove = TRUE;
				*bIsBelow = FALSE;
				break;
			}
			else if (pXStateBuffer[i] == s_aNetWmBelow)
			{
				*bIsAbove = FALSE;
				*bIsBelow = TRUE;
				break;
			}
		}
	}

	XFree (pXStateBuffer);
}

void cairo_dock_window_is_fullscreen_or_hidden_or_maximized (Window Xid, gboolean *bIsFullScreen, gboolean *bIsHidden, gboolean *bIsMaximized)
{
	g_return_if_fail (Xid > 0);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);

	*bIsFullScreen = FALSE;
	*bIsHidden = FALSE;
	*bIsMaximized = FALSE;
	if (iBufferNbElements > 0)
	{
		int i, iNbMaximizedDimensions = 0;
		for (i = 0; i < iBufferNbElements; i ++)
		{
			if (pXStateBuffer[i] == s_aNetWmFullScreen)
			{
				cd_message (  "s_aNetWmFullScreen");
				*bIsFullScreen = TRUE;
				break ;
			}
			else if (pXStateBuffer[i] == s_aNetWmHidden)
			{
				cd_message (  "s_aNetWmHidden");
				*bIsHidden = TRUE;
				//break ;
			}
			else if (pXStateBuffer[i] == s_aNetWmMaximizedVert)
			{
				iNbMaximizedDimensions ++;
				if (iNbMaximizedDimensions == 2)
				{
					*bIsMaximized = TRUE;
					//break;
				}
			}
			else if (pXStateBuffer[i] == s_aNetWmMaximizedHoriz)
			{
				iNbMaximizedDimensions ++;
				if (iNbMaximizedDimensions == 2)
				{
					*bIsMaximized = TRUE;
					//break;
				}
			}
		}
	}
	
	XFree (pXStateBuffer);
}

Window cairo_dock_get_active_window (void)
{
	Atom aNetActiveWindow = XInternAtom (s_XDisplay, "_NET_ACTIVE_WINDOW", False);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXBuffer = NULL;
	Window root = DefaultRootWindow (s_XDisplay);
	XGetWindowProperty (s_XDisplay, root, aNetActiveWindow, 0, G_MAXULONG, False, XA_WINDOW, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXBuffer);

	Window xActiveWindow = (iBufferNbElements > 0 && pXBuffer != NULL ? pXBuffer[0] : 0);
	XFree (pXBuffer);
	return xActiveWindow;
}


int cairo_dock_get_window_desktop (int Xid)
{
	int iDesktopNumber;
	gulong iLeftBytes, iBufferNbElements = 0;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	gulong *pBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmDesktop, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pBuffer);
	if (iBufferNbElements > 0)
		iDesktopNumber = *pBuffer;
	else
		iDesktopNumber = 0;
	
	XFree (pBuffer);
	return iDesktopNumber;
}

void cairo_dock_get_window_geometry (int Xid, int *iGlobalPositionX, int *iGlobalPositionY, int *iWidthExtent, int *iHeightExtent)  // renvoie les coordonnees du coin haut gauche dans le referentiel du viewport actuel. // sous KDE, x et y sont toujours nuls ! (meme avec XGetWindowAttributes).
{
	Window root_return;
	int x_return=1, y_return=1;
	unsigned int width_return, height_return, border_width_return, depth_return;
	XGetGeometry (s_XDisplay, Xid,
		&root_return,
		&x_return, &y_return,
		&width_return, &height_return,
		&border_width_return, &depth_return);  // renvoie les coordonnees du coin haut gauche dans le referentiel du viewport actuel.
	
	*iGlobalPositionX = x_return;  // on pourrait tenir compte de border_width_return...
	*iGlobalPositionY = y_return;  // idem.
	*iWidthExtent = width_return;  // idem.
	*iHeightExtent = height_return;  // idem.
	//g_print ("%s () -> %d;%d %dx%d / %d,%d\n", __func__, x_return, y_return, *iWidthExtent, *iHeightExtent, border_width_return, depth_return);
}

void cairo_dock_get_window_position_on_its_viewport (int Xid, int *iRelativePositionX, int *iRelativePositionY)
{
	int iGlobalPositionX, iGlobalPositionY, iWidthExtent, iHeightExtent;
	cairo_dock_get_window_geometry (Xid, &iGlobalPositionX, &iGlobalPositionY, &iWidthExtent, &iHeightExtent);
	
	while (iGlobalPositionX < 0)  // on passe au referentiel du viewport de la fenetre; inutile de connaitre sa position, puisqu'ils ont tous la meme taille.
		iGlobalPositionX += g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
	while (iGlobalPositionX >= g_iScreenWidth[CAIRO_DOCK_HORIZONTAL])
		iGlobalPositionX -= g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
	while (iGlobalPositionY < 0)
		iGlobalPositionY += g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
	while (iGlobalPositionY >= g_iScreenHeight[CAIRO_DOCK_HORIZONTAL])
		iGlobalPositionY -= g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
	
	*iRelativePositionX = iGlobalPositionX;
	*iRelativePositionY = iGlobalPositionY;
	cd_debug ("position relative : (%d;%d) taille : %dx%d", *iRelativePositionX, *iRelativePositionY, iWidthExtent, iHeightExtent);
}


gboolean cairo_dock_window_is_on_this_desktop (int Xid, int iDesktopNumber)
{
	cd_message ("", __func__);
	int iWindowDesktopNumber, iGlobalPositionX, iGlobalPositionY, iWidthExtent, iHeightExtent;  // coordonnees du coin haut gauche dans le referentiel du viewport actuel.
	iWindowDesktopNumber = cairo_dock_get_window_desktop (Xid);
	cairo_dock_get_window_geometry (Xid, &iGlobalPositionX, &iGlobalPositionY, &iWidthExtent, &iHeightExtent);

	cd_message (" -> %d/%d ; (%d ; %d)", iWindowDesktopNumber, iDesktopNumber, iGlobalPositionX, iGlobalPositionY);
	return ( (iWindowDesktopNumber == iDesktopNumber || iWindowDesktopNumber == -1) &&
		iGlobalPositionX + iWidthExtent >= 0 &&
		iGlobalPositionX < g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] &&
		iGlobalPositionY + iHeightExtent >= 0 &&
		iGlobalPositionY < g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] );  // -1 <=> 0xFFFFFFFF en unsigned.
}

gboolean cairo_dock_window_is_on_current_desktop (int Xid)
{
	cd_message ("", __func__);
	int iDesktopNumber, iDesktopViewportX, iDesktopViewportY;
	iDesktopNumber = cairo_dock_get_current_desktop ();

	return cairo_dock_window_is_on_this_desktop (Xid, iDesktopNumber);
}


void cairo_dock_animate_icon_on_active (Icon *icon, CairoDock *pParentDock)
{
	if ((icon->iCount == 0 || icon->iCount > 1e5) && icon->fPersonnalScale == 0)  // sinon on laisse l'animation actuelle.
	{
		if (cairo_dock_animation_will_be_visible (pParentDock) && ! pParentDock->bInside && g_bAnimateOnActiveWindow)
		{
			cairo_dock_arm_animation (icon, CAIRO_DOCK_WOBBLY, 1);  // un clignotement. il faut choisir une animation qui ne necessite pas que la fenetre du dock soit de taille maximale.
			cairo_dock_start_animation (icon, pParentDock);
		}
	}
}

static gboolean _cairo_dock_window_is_on_our_way (Window *Xid, Icon *icon, int *data)
{
	if (icon != NULL && cairo_dock_window_is_on_this_desktop (*Xid, data[0]))
	{
		gboolean bIsFullScreen, bIsHidden, bIsMaximized;
		cairo_dock_window_is_fullscreen_or_hidden_or_maximized (*Xid, &bIsFullScreen, &bIsHidden, &bIsMaximized);
		if ((data[1] && bIsMaximized & ! bIsHidden) || (data[2] && bIsFullScreen))
		{
			cd_message ("%s est genante (%d, %d) (%d;%d %dx%d)", icon->acName, bIsMaximized, bIsFullScreen, icon->windowGeometry.x, icon->windowGeometry.y, icon->windowGeometry.width, icon->windowGeometry.height);
			if (cairo_dock_window_hovers_dock (&icon->windowGeometry, g_pMainDock))
			{
				cd_message (" et en plus elle empiete sur notre dock");
				return TRUE;
			}
		}
	}
	return FALSE;
}
Icon * cairo_dock_search_window_on_our_way (gboolean bMaximizedWindow, gboolean bFullScreenWindow)
{
	cd_message ("%s (%d, %d)", __func__, bMaximizedWindow, bFullScreenWindow);
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

	if (icon != NULL && (! g_bHideVisibleApplis || icon->bIsHidden))
	{
		cd_message ("%s (%d)", __func__, *Xid);
		CairoDock *pParentDock;

		if (cairo_dock_window_is_on_this_desktop (*Xid, pCurrentDesktop[0]))
		{
			cd_message (" => est sur le bureau actuel.");
			CairoDock *pMainDock = GINT_TO_POINTER (pCurrentDesktop[1]);
			pParentDock = cairo_dock_insert_appli_in_dock (icon, pMainDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
		}
		else
		{
			cd_message (" => n'est pas sur le bureau actuel.");
			CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
			if (pParentDock != NULL)
			{
				cairo_dock_detach_icon_from_dock (icon, pParentDock, TRUE);
				cairo_dock_update_dock_size (pParentDock);
			}
		}
		if (pParentDock != NULL)
			gtk_widget_queue_draw (pParentDock->pWidget);
	}
}
gboolean cairo_dock_unstack_Xevents (CairoDock *pDock)
{
	static XEvent event;
	static int iLastActiveWindow = 0;
	static gboolean bInProgress = FALSE;
	
	g_return_val_if_fail (pDock != NULL, FALSE);
	if (bInProgress)
		return TRUE;
	bInProgress = TRUE;

	long event_mask = 0xFFFFFFFF;  // on les recupere tous, ca vide la pile au fur et a mesure plutot que tout a la fin.
	Window Xid;
	Window root = DefaultRootWindow (s_XDisplay);
	Icon *icon;
	while (XCheckMaskEvent (s_XDisplay, event_mask, &event))
	{
		icon = NULL;
		Xid = event.xany.window;
		//if (event.type == ClientMessage)
		//	cd_message ("\n\n\n >>>>>>>>>>>< event.type : %d\n\n", event.type);
		if (event.type == PropertyNotify)
		{
			//g_print ("  type : %d; atom : %s; window : %d\n", event.xproperty.type, gdk_x11_get_xatom_name (event.xproperty.atom), Xid);
			if (Xid == root)
			{
				g_print ("  type : %d; atom : %s; window : %d\n", event.xproperty.type, gdk_x11_get_xatom_name (event.xproperty.atom), Xid);
				if (event.xproperty.atom == s_aNetClientList)
				{
					GTimeVal time_val;
					g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
					cairo_dock_update_applis_list (pDock, time_val.tv_sec + time_val.tv_usec * 1e-6);
					cairo_dock_notify (CAIRO_DOCK_WINDOW_CONFIGURED, NULL);
				}
				else if (event.xproperty.atom == s_aNetActiveWindow)
				{
					Window XActiveWindow = cairo_dock_get_active_window ();
					if (iLastActiveWindow != XActiveWindow)
					{
						iLastActiveWindow = XActiveWindow;
						icon = g_hash_table_lookup (s_hXWindowTable, &XActiveWindow);
						if (icon != NULL)
						{
							cd_message ("%s devient active", icon->acName);
							CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
							if (pParentDock == NULL)  // elle est inhibee.
							{
								cairo_dock_update_activity_on_inhibators (icon->cClass, icon->Xid);
							}
							else
							{
								cairo_dock_animate_icon_on_active (icon, pParentDock);
							}
						}
						cairo_dock_notify (CAIRO_DOCK_WINDOW_ACTIVATED, &XActiveWindow);
					}
				}
				else if (event.xproperty.atom == s_aNetCurrentDesktop || event.xproperty.atom == s_aNetDesktopViewport)
				{
					cd_message ("on change de bureau");
					if (g_bAppliOnCurrentDesktopOnly)
					{
						int iDesktopNumber = cairo_dock_get_current_desktop ();
						int data[2] = {iDesktopNumber, GPOINTER_TO_INT (pDock)};
						g_hash_table_foreach (s_hXWindowTable, (GHFunc) _cairo_dock_hide_show_windows_on_other_desktops, data);
					}
					if (cairo_dock_quick_hide_is_activated () && (g_bAutoHideOnFullScreen || g_bAutoHideOnMaximized))
					{
						if (cairo_dock_search_window_on_our_way (g_bAutoHideOnMaximized, g_bAutoHideOnFullScreen) == NULL)
						{
							cd_message (" => plus aucune fenetre genante");
							cairo_dock_deactivate_temporary_auto_hide ();
						}
					}
					else if (! cairo_dock_quick_hide_is_activated () && (g_bAutoHideOnFullScreen || g_bAutoHideOnMaximized))
					{
						if (cairo_dock_search_window_on_our_way (g_bAutoHideOnMaximized, g_bAutoHideOnFullScreen) != NULL)
						{
							cd_message (" => une fenetre est genante");
							cairo_dock_activate_temporary_auto_hide ();
						}
					}
					cairo_dock_notify (CAIRO_DOCK_DESKTOP_CHANGED, NULL);
				}
				else if (event.xproperty.atom == s_aNetNbDesktops)
				{
					cd_message ("changementdu nombre de bureaux virtuels");
					g_iNbDesktops = cairo_dock_get_nb_desktops ();
					cairo_dock_notify (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, NULL);
				}
				else if (event.xproperty.atom == s_aNetDesktopGeometry)
				{
					cd_message ("geometrie du bureau alteree");
					
					cairo_dock_get_nb_viewports (&g_iNbViewportX, &g_iNbViewportY);
					
					if (cairo_dock_update_screen_geometry ())  // modification de la resolution.
					{
						cairo_dock_set_window_position_at_balance (pDock, pDock->iCurrentWidth, pDock->iCurrentHeight);
						gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
					}
					cairo_dock_notify (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, NULL);
				}
				else if (event.xproperty.atom == s_aRootMapID)
				{
					cd_message ("changement du fond d'ecran");
					if (g_bUseFakeTransparency)
						cairo_dock_load_desktop_background_surface ();
					else
						cairo_dock_invalidate_desktop_bg_surface ();
					cairo_dock_notify (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, NULL);
				}
			}
			else
			{
				if (event.xproperty.atom == s_aNetWmState || event.xproperty.atom == XInternAtom (s_XDisplay, "_KDE_WM_WINDOW_OPACITY", False))
				{
					gboolean bIsFullScreen, bIsHidden, bIsMaximized;
					cairo_dock_window_is_fullscreen_or_hidden_or_maximized (Xid, &bIsFullScreen, &bIsHidden, &bIsMaximized);
					cd_message ("changement d'etat de %d => {%d ; %d ; %d}", Xid, bIsFullScreen, bIsHidden, bIsMaximized);
					/*if (g_bAutoHideOnFullScreen)
					{
						if (bIsFullScreen && ! cairo_dock_quick_hide_is_activated ())
						{
							cd_message (" => devient plein ecran");
							cairo_dock_activate_temporary_auto_hide ();
						}
						else if (! bIsFullScreen && cairo_dock_quick_hide_is_activated ())
						{
							if (cairo_dock_search_fullscreen_window_on_current_desktop () == NULL)
								cairo_dock_deactivate_temporary_auto_hide ();
						}
					}*/
					gboolean bChangeIntercepted = FALSE;
					if (g_bAutoHideOnMaximized || g_bAutoHideOnFullScreen)
					{
						if ( ((bIsMaximized && ! bIsHidden && g_bAutoHideOnMaximized) || (bIsFullScreen && g_bAutoHideOnFullScreen)) && ! cairo_dock_quick_hide_is_activated ())
						{
							icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
							cd_message (" => %s devient genante", icon != NULL ? icon->acName : "une fenetre");
							if (icon != NULL && cairo_dock_window_hovers_dock (&icon->windowGeometry, g_pMainDock))
								cairo_dock_activate_temporary_auto_hide ();
							//bChangeIntercepted = TRUE;
						}
						else if ((! bIsMaximized || ! g_bAutoHideOnMaximized || bIsHidden) && (! bIsFullScreen || ! g_bAutoHideOnFullScreen) && cairo_dock_quick_hide_is_activated ())
						{
							if (cairo_dock_search_window_on_our_way (g_bAutoHideOnMaximized, g_bAutoHideOnFullScreen) == NULL)
							{
								cd_message (" => plus aucune fenetre genante");
								cairo_dock_deactivate_temporary_auto_hide ();
								//bChangeIntercepted = TRUE;
							}
						}
					}
					if (! bChangeIntercepted)
					{
						icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
						if (icon != NULL && icon->fPersonnalScale <= 0)  // pour une icône en cours de supression, on ne fait rien.
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
									if (cairo_dock_search_window_on_our_way (g_bAutoHideOnMaximized, g_bAutoHideOnFullScreen) == NULL)
										cairo_dock_deactivate_temporary_auto_hide ();
								}*/
								
								if (g_bHideVisibleApplis && pParentDock != NULL)
								{
									if (bIsHidden)
									{
										cd_message (" => se cache");
										if (! g_bAppliOnCurrentDesktopOnly || cairo_dock_window_is_on_current_desktop (Xid))
											pParentDock = cairo_dock_insert_appli_in_dock (icon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
									}
									else
									{
										cd_message (" => re-apparait");
										cairo_dock_detach_icon_from_dock (icon, pParentDock, TRUE);
										cairo_dock_update_dock_size (pParentDock);
									}
									if (pParentDock != NULL)
										gtk_widget_queue_draw (pParentDock->pWidget);
								}
								else if (g_fVisibleAppliAlpha != 0)
								{
									icon->fAlpha = 1;  // on triche un peu.
									if (pParentDock != NULL)
										cairo_dock_redraw_my_icon (icon, CAIRO_CONTAINER (pParentDock));
								}
							}
						}
					}
				}
				if (event.xproperty.atom == s_aNetWmDesktop)  // cela ne gere pas les changements de viewports, qui eux se font en changeant les coordonnees. Il faudrait donc recueillir les ConfigureNotify, qui donnent les redimensionnements et les deplacements.
				{
					cd_message ("changement de bureau pour %d", Xid);
					if (g_bAppliOnCurrentDesktopOnly)
					{
						int iDesktopNumber = cairo_dock_get_current_desktop ();

						int data[2] = {iDesktopNumber, GPOINTER_TO_INT (pDock)};
						icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
						_cairo_dock_hide_show_windows_on_other_desktops (&Xid, icon, data);
					}
				}
				else
				{
					icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
					if (icon != NULL && icon->fPersonnalScale <= 0)  // pour une icone en cours de supression, on ne fait rien.
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
			if (icon != NULL)
			{
				memcpy (&icon->windowGeometry, &event.xconfigure.x, sizeof (GtkAllocation));
			}
			
			if (g_bAppliOnCurrentDesktopOnly)
			{
				if (icon != NULL && icon->fPersonnalScale <= 0)  // pour une icone en cours de supression, on ne fait rien.
				{
					if (event.xconfigure.x + event.xconfigure.width < 0 || event.xconfigure.x > g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] || event.xconfigure.y + event.xconfigure.height < 0 || event.xconfigure.y > g_iScreenHeight[CAIRO_DOCK_HORIZONTAL])  // en fait il faudrait faire ca modulo le nombre de viewports * la largeur d'un bureau, car avec une fenetre a droite, elle peut revenir sur le bureau par la gauche si elle est tres large...
					{
						CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
						if (pParentDock == NULL)
							pParentDock = pDock;
						cairo_dock_detach_icon_from_dock (icon, pParentDock, TRUE);
						cairo_dock_update_dock_size (pParentDock);
						gtk_widget_queue_draw (pParentDock->pWidget);
					}
					else  // elle est sur le bureau.
					{
						gboolean bInsideDock;
						if (g_list_find (pDock->icons, icon) == NULL)
						{
							if (! g_bGroupAppliByClass)
								bInsideDock = FALSE;
							else
							{
								Icon *pSameClassIcon = cairo_dock_get_icon_with_class (pDock->icons, icon->cClass);
								bInsideDock = (pSameClassIcon != NULL && pSameClassIcon->pSubDock != NULL && g_list_find (pSameClassIcon->pSubDock->icons, icon) != NULL);
							}
						}
						else
							bInsideDock = TRUE;
						if (! bInsideDock)
							cairo_dock_insert_appli_in_dock (icon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
					}
				}
			}
			cairo_dock_notify (CAIRO_DOCK_WINDOW_CONFIGURED, &event.xconfigure);
		}
		//else
		//	cd_message ("  type : %d; window : %d\n", event.xany.type, Xid);
	}
	if (XEventsQueued (s_XDisplay, QueuedAlready) != 0)
		XSync (s_XDisplay, True);  // True <=> discard.
	//g_print ("XEventsQueued : %d\n", XEventsQueued (s_XDisplay, QueuedAfterFlush));  // QueuedAlready, QueuedAfterReading, QueuedAfterFlush
	
	bInProgress = FALSE;
	return TRUE;
}

void cairo_dock_set_window_mask (Window Xid, long iMask)
{
	//StructureNotifyMask | /*ResizeRedirectMask*/
	//SubstructureRedirectMask |
	//SubstructureNotifyMask |  // place sur le root, donne les evenements Map, Unmap, Destroy, Create de chaque fenetre.
	//PropertyChangeMask
	XSelectInput (s_XDisplay, Xid, iMask);  // c'est le 'event_mask' d'un XSetWindowAttributes.
}


Window *cairo_dock_get_windows_list (gulong *iNbWindows)
{
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	Window *XidList = NULL;

	Window root = DefaultRootWindow (s_XDisplay);
	gulong iLeftBytes;
	XGetWindowProperty (s_XDisplay, root, s_aNetClientList, 0, G_MAXLONG, False, XA_WINDOW, &aReturnedType, &aReturnedFormat, iNbWindows, &iLeftBytes, (guchar **)&XidList);
	return XidList;
}

CairoDock *cairo_dock_insert_appli_in_dock (Icon *icon, CairoDock *pMainDock, gboolean bUpdateSize, gboolean bAnimate)
{
	//\_________________ On determine dans quel dock l'inserer.
	cd_message ("%s (%s, %d)", __func__, icon->acName, icon->Xid);
	if (g_bMixLauncherAppli && cairo_dock_prevent_inhibated_class (icon))
	{
		cd_message (" -> se fait inhiber");
		return NULL;
	}
	CairoDock *pParentDock = cairo_dock_manage_appli_class (icon, pMainDock);  // renseigne cParentDockName.
	g_return_val_if_fail (pParentDock != NULL, NULL);

	//\_________________ On l'insere dans son dock parent en animant ce dernier eventuellement.
	cairo_dock_insert_icon_in_dock (icon, pParentDock, bUpdateSize, bAnimate, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
	cd_message (" insertion de %s complete (%.2f %.2fx%.2f) dans %s", icon->acName, icon->fPersonnalScale, icon->fWidth, icon->fHeight, icon->cParentDockName);

	if (bAnimate && cairo_dock_animation_will_be_visible (pParentDock))
	{
		cairo_dock_start_animation (icon, pParentDock);
	}
	else
	{
		icon->fPersonnalScale = 0;
		icon->fScale = 1.;
	}

	return pParentDock;
}

static gboolean _cairo_dock_remove_old_applis (Window *Xid, Icon *icon, double *fTime)
{
	gboolean bToBeRemoved = FALSE;
	if (icon != NULL)
	{
		//g_print ("%s (%s, %f / %f)\n", __func__, icon->acName, icon->fLastCheckTime, *fTime);
		if (icon->fLastCheckTime > 0 && icon->fLastCheckTime < *fTime && icon->fPersonnalScale <= 0)
		{
			cd_message ("cette fenetre (%ld) est trop vieille", *Xid);
			
			if (cairo_dock_quick_hide_is_activated () && (g_bAutoHideOnFullScreen || g_bAutoHideOnMaximized))
			{
				if (cairo_dock_search_window_on_our_way (g_bAutoHideOnMaximized, g_bAutoHideOnFullScreen) == NULL)
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
					icon->fPersonnalScale = 1.0;
				//g_print ("icon->fPersonnalScale <- %.2f\n", icon->fPersonnalScale);
				
				cairo_dock_start_animation (icon, pParentDock);
			}
			else
			{
				cd_message ("  pas dans un container, on la detruit donc immediatement");
				icon->fLastCheckTime = -1;  // pour ne pas la desenregistrer de la HashTable lors du 'free'.
				cairo_dock_free_icon (icon);
				bToBeRemoved = TRUE;
			}
		}
	}
	return bToBeRemoved;
}
void cairo_dock_update_applis_list (CairoDock *pDock, double fTime)
{
	//g_print ("%s ()\n", __func__);
	gulong i, iNbWindows = 0;
	Window *pXWindowsList = cairo_dock_get_windows_list (&iNbWindows);

	Window Xid;
	Icon *icon;
	gpointer pOriginalXid;
	gboolean bAppliAlreadyRegistered;
	gboolean bUpdateMainDockSize = FALSE;
	CairoDock *pParentDock;

	for (i = 0; i < iNbWindows; i ++)
	{
		Xid = pXWindowsList[i];

		bAppliAlreadyRegistered = g_hash_table_lookup_extended (s_hXWindowTable, &Xid, &pOriginalXid, (gpointer *) &icon);
		if (! bAppliAlreadyRegistered)
		{
			cd_message (" cette fenetre (%ld) de la pile n'est pas dans la liste", Xid);
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
			if (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS)
				icon = cairo_dock_create_icon_from_xwindow (pCairoContext, Xid, pDock);
			if (icon != NULL)
			{
				icon->fLastCheckTime = fTime;
				if ((icon->bIsHidden || ! g_bHideVisibleApplis) && (! g_bAppliOnCurrentDesktopOnly || cairo_dock_window_is_on_current_desktop (Xid)))
				{
					cd_message (" insertion de %s ...", icon->acName);
					pParentDock = cairo_dock_insert_appli_in_dock (icon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
					if (pParentDock != NULL)
					{
						if (pParentDock->bIsMainDock)
							bUpdateMainDockSize = TRUE;
						else
							cairo_dock_update_dock_size (pParentDock);
					}
				}
			}
			else
				cairo_dock_blacklist_appli (Xid);
		}
		else if (icon != NULL)
		{
			icon->fLastCheckTime = fTime;
		}
	}

	g_hash_table_foreach_remove (s_hXWindowTable, (GHRFunc) _cairo_dock_remove_old_applis, &fTime);
	
	if (bUpdateMainDockSize)
		cairo_dock_update_dock_size (pDock);

	XFree (pXWindowsList);
}


void cairo_dock_start_application_manager (CairoDock *pDock)
{
	g_return_if_fail (s_iSidUpdateAppliList == 0);

	//\__________________ On recupere l'ensemble des fenetres presentes.
	Window root = DefaultRootWindow (s_XDisplay);
	cairo_dock_set_window_mask (root, PropertyChangeMask /*| StructureNotifyMask | SubstructureNotifyMask | ResizeRedirectMask | SubstructureRedirectMask*/);

	gulong i, iNbWindows = 0;
	Window *pXWindowsList = cairo_dock_get_windows_list (&iNbWindows);

	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);

	int iDesktopNumber = cairo_dock_get_current_desktop ();
	
	//\__________________ On cree les icones de toutes ces applis.
	CairoDock *pParentDock;
	gboolean bUpdateMainDockSize = FALSE;
	Window Xid;
	Icon *pIcon;
	for (i = 0; i < iNbWindows; i ++)
	{
		Xid = pXWindowsList[i];
		pIcon = cairo_dock_create_icon_from_xwindow (pCairoContext, Xid, pDock);

		if (pIcon != NULL)
		{
			if ((pIcon->bIsHidden || ! g_bHideVisibleApplis) && (! g_bAppliOnCurrentDesktopOnly || cairo_dock_window_is_on_current_desktop (Xid)))
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
			if ((g_bAutoHideOnMaximized && pIcon->bIsMaximized) || (g_bAutoHideOnFullScreen && pIcon->bIsFullScreen))
			{
				if (! cairo_dock_quick_hide_is_activated () && cairo_dock_window_is_on_this_desktop (pIcon->Xid, iDesktopNumber))
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
	if (pXWindowsList != NULL)
		XFree (pXWindowsList);
	
	if (bUpdateMainDockSize)
		cairo_dock_update_dock_size (pDock);
	
	//\__________________ On lance le gestionnaire d'evenements X.
	s_iSidUpdateAppliList = g_timeout_add (CAIRO_DOCK_TASKBAR_CHECK_INTERVAL, (GSourceFunc) cairo_dock_unstack_Xevents, (gpointer) pDock);  // un g_idle_add () consomme 90% de CPU ! :-/
}

void cairo_dock_pause_application_manager (void)
{
	if (s_iSidUpdateAppliList != 0)
	{
		Window root = DefaultRootWindow (s_XDisplay);
		cairo_dock_set_window_mask (root, 0);
		
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
