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
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-log.h"

#define CAIRO_DOCK_TASKBAR_CHECK_INTERVAL 250

extern CairoDock *g_pMainDock;
extern gboolean g_bAutoHide;
extern double g_fAmplitude;
extern gboolean g_bUseSeparator;

extern int g_iScreenWidth[2], g_iScreenHeight[2];

extern gboolean g_bUniquePid;
extern gboolean g_bAnimateOnActiveWindow;
extern gboolean g_bAutoHideOnFullScreen;
extern gboolean g_bHideVisibleApplis;
extern double g_fVisibleAppliAlpha;
extern gboolean g_bAppliOnCurrentDesktopOnly;
extern gboolean g_bGroupAppliByClass;
extern int g_iNbDesktops;
extern int g_iNbFacesForViewportX,g_iNbFacesForViewportY ;

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
static Atom s_aNetWmWindowType;
static Atom s_aNetWmWindowTypeNormal;
static Atom s_aNetWmWindowTypeUtility;

int cairo_dock_xerror_handler (Display * pDisplay, XErrorEvent *pXError)
{
	//g_print ("Erreur (%d, %d, %d) lors d'une requete sur %d\n", pXError->error_code, pXError->request_code, pXError->minor_code, pXError->resourceid);
	return 0;
}
void cairo_dock_initialize_application_manager (void)
{
	s_XDisplay = XOpenDisplay (0);
	g_return_if_fail (s_XDisplay != NULL);

	s_hXWindowTable = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);

	XSetErrorHandler (cairo_dock_xerror_handler);

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
	s_aNetWmWindowType = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE", False);
	s_aNetWmWindowTypeNormal = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	s_aNetWmWindowTypeUtility = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE_UTILITY", False);
	
	g_iNbDesktops = cairo_dock_get_nb_desktops ();
	cairo_dock_get_nb_face_in_viewport (&g_iNbFacesForViewportX, &g_iNbFacesForViewportY);
	
	cairo_dock_initialize_application_factory (s_XDisplay);
}

const Display *cairo_dock_get_Xdisplay (void)
{
	return s_XDisplay;
}

void cairo_dock_register_appli (Icon *icon)
{
	if (CAIRO_DOCK_IS_VALID_APPLI (icon))
	{
		Window *pXid = g_new (Window, 1);
			*pXid = icon->Xid;
		g_hash_table_insert (s_hXWindowTable, pXid, icon);
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
	if (CAIRO_DOCK_IS_VALID_APPLI (icon))
	{
		g_hash_table_remove (s_hXWindowTable, &icon->Xid);

		cairo_dock_unregister_pid (icon);
		///g_free (icon->cClass);
		///icon->cClass = NULL;
	}
}



gulong cairo_dock_get_xwindow_timestamp (Window Xid)
{
	g_return_val_if_fail (Xid > 0, 0);
	Atom aNetWmUserTime = XInternAtom (s_XDisplay, "_NET_WM_USER_TIME", False);
	gulong iLeftBytes, iBufferNbElements = 0;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	gulong *pTimeBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, aNetWmUserTime, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pTimeBuffer);
	gulong iTimeStamp = 0;
	if (iBufferNbElements > 0)
		iTimeStamp = *pTimeBuffer;
	XFree (pTimeBuffer);
	return iTimeStamp;
}

void cairo_dock_set_xwindow_timestamp (Window Xid, gulong iTimeStamp)
{
	g_return_if_fail (Xid > 0);
	Atom aNetWmUserTime = XInternAtom (s_XDisplay, "_NET_WM_USER_TIME", False);
	XChangeProperty (s_XDisplay,
		Xid,
		aNetWmUserTime,
		XA_CARDINAL, 32, PropModeReplace,
		(guchar *)&iTimeStamp, 1);
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
			cd_message ("kill (%ld)\n", pPidBuffer[0]);
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


void cairo_dock_show_appli (Window Xid)
{
	//g_print ("%s (%d)\n", __func__, Xid);
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;
	Window root = DefaultRootWindow (s_XDisplay);

	//\______________ On recupere le numero du bureau de la fenetre a afficher.
	gulong iLeftBytes, iBufferNbElements = 0;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	gulong *pBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmDesktop, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pBuffer);
	gulong iDesktopNumber = 0;
	if (iBufferNbElements > 0)
		iDesktopNumber = *pBuffer;
	XFree (pBuffer);
	//g_print ("iDesktopNumber : %d\n", iDesktopNumber);

	//\______________ On se deplace dessus (autrement Metacity deplacera la fenetre sur le bureau actuel).
	int iTimeStamp = cairo_dock_get_xwindow_timestamp (root);
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = root;
	xClientMessage.xclient.message_type = s_aNetCurrentDesktop;
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = iDesktopNumber;
	xClientMessage.xclient.data.l[1] = iTimeStamp;
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;

	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);

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
	_cairo_dock_change_window_state (Xid, bMaximize, XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False), XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False));
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

	Window root_return;
	int x_return=1, y_return=1;
	unsigned int width_return, height_return, border_width_return, depth_return;
	XGetGeometry (s_XDisplay, Xid,
		&root_return,
		&x_return, &y_return,
		&width_return, &height_return,
		&border_width_return, &depth_return);  // renvoie les coordonnees du coin haut gauche dans le referentiel du viewport actuel. // sous KDE, x et y sont toujours nuls ! (meme avec XGetWindowAttributes).
	
	while (x_return < 0)  // on passe au referentiel du viewport de la fenetre; inutile de connaitre sa position, puisqu'ils ont tous la meme taille.
		x_return += g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
	while (x_return >= g_iScreenWidth[CAIRO_DOCK_HORIZONTAL])
		x_return -= g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
	while (y_return < 0)
		y_return += g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
	while (y_return >= g_iScreenHeight[CAIRO_DOCK_HORIZONTAL])
		y_return -= g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
	g_print ("position relative : (%d;%d) taille : %dx%d\n", x_return, y_return, width_return, height_return);

	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_MOVERESIZE_WINDOW", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = StaticGravity | (1 << 8) | (1 << 9) | (0 << 10) | (0 << 11);
	xClientMessage.xclient.data.l[1] = iDesktopViewportX + x_return;  // coordonnees dans le referentiel du viewport desire.
	xClientMessage.xclient.data.l[2] = iDesktopViewportY + y_return;
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;
	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);

	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}

void cairo_dock_show_hide_desktop (gboolean bShow)
{
	XEvent xClientMessage;
	Window root = DefaultRootWindow (s_XDisplay);

	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = root;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_SHOWING_DESKTOP", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = bShow;
	xClientMessage.xclient.data.l[1] = 0;
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 2;
	xClientMessage.xclient.data.l[4] = 0;

	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
}

void cairo_dock_set_current_viewport (int iDesktopViewportX, int iDesktopViewportY)
{
	XEvent xClientMessage;
	Window root = DefaultRootWindow (s_XDisplay);
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = root;
	xClientMessage.xclient.message_type = s_aNetDesktopViewport;
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = iDesktopViewportX;
	xClientMessage.xclient.data.l[1] = iDesktopViewportY;
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;
	
	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
}
void cairo_dock_set_current_desktop (int iDesktopNumber)
{
	Window root = DefaultRootWindow (s_XDisplay);
	int iTimeStamp = cairo_dock_get_xwindow_timestamp (root);
	XEvent xClientMessage;
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = root;
	xClientMessage.xclient.message_type = s_aNetCurrentDesktop;
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = iDesktopNumber;
	xClientMessage.xclient.data.l[1] = iTimeStamp;
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;

	XSendEvent (s_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
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
		Atom aNetWmMaximizedVertically = XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		Atom aNetWmMaximizedHorizontally = XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		for (i = 0; i < iBufferNbElements && iIsMaximized < 2; i ++)
		{
			if (pXStateBuffer[i] == aNetWmMaximizedVertically)
				iIsMaximized ++;
			if (pXStateBuffer[i] == aNetWmMaximizedHorizontally)
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

void cairo_dock_window_is_fullscreen_or_hidden (Window Xid, gboolean *bIsFullScreen, gboolean *bIsHidden)
{
	g_return_if_fail (Xid > 0);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);

	*bIsFullScreen = FALSE;
	*bIsHidden = FALSE;
	if (iBufferNbElements > 0)
	{
		int i;
		for (i = 0; i < iBufferNbElements; i ++)
		{
			if (pXStateBuffer[i] == s_aNetWmFullScreen)
			{
				cd_message (  "s_aNetWmFullScreen\n");
				*bIsFullScreen = TRUE;
				break ;
			}
			else if (pXStateBuffer[i] == s_aNetWmHidden)
			{
				cd_message (  "s_aNetWmHidden\n");
				*bIsHidden = TRUE;
				break ;
			}
		}
	}

	XFree (pXStateBuffer);
}

gboolean cairo_dock_desktop_is_visible (void)
{
	Atom aNetShowingDesktop = XInternAtom (s_XDisplay, "_NET_SHOWING_DESKTOP", False);
	gulong iLeftBytes, iBufferNbElements = 0;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	gulong *pXBuffer = NULL;
	Window root = DefaultRootWindow (s_XDisplay);
	XGetWindowProperty (s_XDisplay, root, aNetShowingDesktop, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXBuffer);

	gboolean bDesktopIsShown = (iBufferNbElements > 0 && pXBuffer != NULL ? *pXBuffer : FALSE);
	XFree (pXBuffer);
	return bDesktopIsShown;
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

void cairo_dock_get_window_desktop_and_position (int Xid, int *iDesktopNumber, int *iGlobalPositionX, int *iGlobalPositionY, int *iWidthExtent, int *iHeightExtent)
{
	gulong iLeftBytes, iBufferNbElements = 0;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	gulong *pBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmDesktop, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pBuffer);
	if (iBufferNbElements > 0)
		*iDesktopNumber = *pBuffer;
	else
		*iDesktopNumber = 0;
	XFree (pBuffer);

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
	g_print ("%s () -> %d;%d %dx%d / %d,%d\n", __func__, x_return, y_return, *iWidthExtent, *iHeightExtent, border_width_return, depth_return);
}

void cairo_dock_get_current_viewport (int *iCurrentViewPortX, int *iCurrentViewPortY)
{
	Window root = DefaultRootWindow (s_XDisplay);
	
	Window root_return;
	int x_return=1, y_return=1;
	unsigned int width_return, height_return, border_width_return, depth_return;
	XGetGeometry (s_XDisplay, root,
		&root_return,
		&x_return, &y_return,
		&width_return, &height_return,
		&border_width_return, &depth_return);
	*iCurrentViewPortX = x_return;
	*iCurrentViewPortY = y_return;
	
	
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pViewportsXY = NULL;
	XGetWindowProperty (s_XDisplay, root, s_aNetDesktopViewport, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pViewportsXY);
	if (iBufferNbElements > 0)
	{
		*iCurrentViewPortX = pViewportsXY[0];
		*iCurrentViewPortY = pViewportsXY[1];
		XFree (pViewportsXY);
	}
	
}

int cairo_dock_get_current_desktop (void)
{
	Window root = DefaultRootWindow (s_XDisplay);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXDesktopNumberBuffer = NULL;
	XGetWindowProperty (s_XDisplay, root, s_aNetCurrentDesktop, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXDesktopNumberBuffer);

	int iDesktopNumber;
	if (iBufferNbElements > 0)
		iDesktopNumber = *pXDesktopNumberBuffer;
	else
		iDesktopNumber = 0;
	
	XFree (pXDesktopNumberBuffer);
	return iDesktopNumber;
}

int cairo_dock_get_nb_desktops (void)
{
	Window root = DefaultRootWindow (s_XDisplay);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXDesktopNumberBuffer = NULL;
	XGetWindowProperty (s_XDisplay, root, XInternAtom (s_XDisplay, "_NET_NUMBER_OF_DESKTOPS", False), 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXDesktopNumberBuffer);
	
	int iNumberOfDesktops;
	if (iBufferNbElements > 0)
		iNumberOfDesktops = *pXDesktopNumberBuffer;
	else
		iNumberOfDesktops = 0;
	
	return iNumberOfDesktops;
}

int cairo_dock_get_viewports_coordinates (gulong **pViewportsXY)
{
	Window root = DefaultRootWindow (s_XDisplay);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	*pViewportsXY = NULL;
	XGetWindowProperty (s_XDisplay, root, s_aNetDesktopViewport, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)pViewportsXY);
	g_print ("%d viewports\n", iBufferNbElements/2);
	return iBufferNbElements;
	/*Atom aNetWorkArea = XInternAtom (s_XDisplay, "_NET_WORKAREA", False);
	iBufferNbElements = 0;
	gulong *pXWorkArea = NULL;
	XGetWindowProperty (s_XDisplay, root, aNetWorkArea, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXWorkArea);
	int i;
	for (i = 0; i < iBufferNbElements/4; i ++)
	{
		cd_message ("work area : (%d;%d) %dx%d\n", pXWorkArea[4*i], pXWorkArea[4*i+1], pXWorkArea[4*i+2], pXWorkArea[4*i+3]);
	}
	XFree (pXWorkArea);*/
}

void cairo_dock_get_nb_face_in_viewport (int *iNbFacesForViewportX, int *iNbFacesForViewportY)
{
	/*Window root = DefaultRootWindow (s_XDisplay);
	Window root_return;
	int x_return=1, y_return=1;
	unsigned int width_return, height_return, border_width_return, depth_return;
	XGetGeometry (s_XDisplay, root,
		&root_return,
		&x_return, &y_return,
		&width_return, &height_return,
		&border_width_return, &depth_return);*/
	
	
	Screen *XScreen = XDefaultScreenOfDisplay (s_XDisplay);
	//g_print ("screen : %dx%d ; root : %dx%d\n", WidthOfScreen (XScreen), HeightOfScreen (XScreen), width_return, height_return);
	
	Window root = DefaultRootWindow (s_XDisplay);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pVirtualScreenSizeBuffer = NULL;
	XGetWindowProperty (s_XDisplay, root, s_aNetDesktopGeometry, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pVirtualScreenSizeBuffer);
	if (iBufferNbElements > 0)
	{
		g_print ("pVirtualScreenSizeBuffer : %dx%d\n", pVirtualScreenSizeBuffer[0], pVirtualScreenSizeBuffer[1]);
		*iNbFacesForViewportX = pVirtualScreenSizeBuffer[0] / WidthOfScreen (XScreen);
		*iNbFacesForViewportY = pVirtualScreenSizeBuffer[1] / HeightOfScreen (XScreen);
		XFree (pVirtualScreenSizeBuffer);
	}
}



gboolean cairo_dock_window_is_on_this_desktop (int Xid, int iDesktopNumber)
{
	cd_message ("", __func__);
	int iWindowDesktopNumber, iGlobalPositionX, iGlobalPositionY, iWidthExtent, iHeightExtent;  // coordonnees du coin haut gauche dans le referentiel du viewport actuel.
	cairo_dock_get_window_desktop_and_position (Xid, &iWindowDesktopNumber, &iGlobalPositionX, &iGlobalPositionY, &iWidthExtent, &iHeightExtent);

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
			g_return_if_fail (pParentDock != NULL);
			cairo_dock_detach_icon_from_dock (icon, pParentDock, TRUE);
			cairo_dock_update_dock_size (pParentDock);
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
	if (bInProgress)
		return TRUE;
	bInProgress = TRUE;

	g_return_val_if_fail (pDock != NULL, FALSE);

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
				if (event.xproperty.atom == s_aNetClientList)
				{
					GTimeVal time_val;
					g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
					cairo_dock_update_applis_list (pDock, time_val.tv_sec + time_val.tv_usec * 1e-6);
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
							cd_message ("%s devient active\n", icon->acName);
							if (icon->iCount == 0 && icon->fPersonnalScale == 0)  // sinon on laisse l'animation actuelle.
							{
								CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
								if (pParentDock == NULL)
									pParentDock = pDock;
								if (cairo_dock_animation_will_be_visible (pParentDock) && ! pParentDock->bInside && g_bAnimateOnActiveWindow)
								{
									cairo_dock_arm_animation (icon, CAIRO_DOCK_WOBBLY, 1);  // un clignotement. il faut choisir une animation qui ne necessite pas que la fenetre du dock soit de taille maximale.
									cairo_dock_start_animation (icon, pParentDock);
								}
							}
						}
					}
				}
				else if (event.xproperty.atom == s_aNetCurrentDesktop || event.xproperty.atom == s_aNetDesktopViewport)
				{
					cd_message ("on change de bureau\n");
					if (g_bAppliOnCurrentDesktopOnly)
					{
						int iDesktopNumber = cairo_dock_get_current_desktop ();
						int data[2] = {iDesktopNumber, GPOINTER_TO_INT (pDock)};
						g_hash_table_foreach (s_hXWindowTable, (GHFunc) _cairo_dock_hide_show_windows_on_other_desktops, data);
					}
					cairo_dock_notify (CAIRO_DOCK_DESKTOP_CHANGED, NULL);
				}
				else if (event.xproperty.atom == s_aNetDesktopGeometry)
				{
					cd_message ("geometrie du bureau alteree\n");
					if (cairo_dock_update_screen_geometry (pDock))  // modification largeur et/ou hauteur.
					{
						cairo_dock_set_window_position_at_balance (pDock, pDock->iCurrentWidth, pDock->iCurrentHeight);
						gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
						cairo_dock_notify (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, NULL);
					}
					g_iNbDesktops = cairo_dock_get_nb_desktops ();
					cairo_dock_get_nb_face_in_viewport (&g_iNbFacesForViewportX, &g_iNbFacesForViewportY);
				}
			}
			else
			{
				if (event.xproperty.atom == s_aNetWmState || event.xproperty.atom == XInternAtom (s_XDisplay, "_KDE_WM_WINDOW_OPACITY", False))
				{
					gboolean bIsFullScreen, bIsHidden;
					cairo_dock_window_is_fullscreen_or_hidden (Xid, &bIsFullScreen, &bIsHidden);
					cd_message ("changement d'etat de %d => {%d ; %d}\n", Xid, bIsFullScreen, bIsHidden);
					if (g_bAutoHideOnFullScreen && bIsFullScreen && ! cairo_dock_quick_hide_is_activated ())
					{
						cd_message (" => devient plein ecran\n");
						cairo_dock_activate_temporary_auto_hide (g_pMainDock);
					}
					else
					{
						icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
						if (icon != NULL && icon->fPersonnalScale <= 0)  // pour une icône en cours de supression, on ne fait rien.
						{
							CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
							if (pParentDock == NULL)
								pParentDock = pDock;
							
							if (bIsHidden != icon->bIsHidden)
							{
								cd_message ("  changement de visibilite -> %d\n", bIsHidden);
								icon->bIsHidden = bIsHidden;
								
								if (g_bHideVisibleApplis)
								{
									if (bIsHidden)
									{
										cd_message (" => se cache\n");
										if (! g_bAppliOnCurrentDesktopOnly || cairo_dock_window_is_on_current_desktop (Xid))
											pParentDock = cairo_dock_insert_appli_in_dock (icon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
									}
									else
									{
										cd_message (" => re-apparait\n");
										cairo_dock_detach_icon_from_dock (icon, pParentDock, TRUE);
										cairo_dock_update_dock_size (pParentDock);
									}
									if (pParentDock != NULL)
										gtk_widget_queue_draw (pParentDock->pWidget);
								}
								else if (g_fVisibleAppliAlpha != 0)
								{
									icon->fAlpha = 1;  // on triche un peu.
									cairo_dock_redraw_my_icon (icon, CAIRO_DOCK_CONTAINER (pParentDock));
								}
							}
						}
					}
				}
				if (event.xproperty.atom == s_aNetWmDesktop)  // cela ne gere pas les changements de viewports, qui eux se font en changeant les coordonnees. Il faudrait donc recueillir les ConfigureNotify, qui donnent les redimensionnements et les deplacements.
				{
					cd_message ("changement de bureau pour %d\n", Xid);
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

			if (g_bAppliOnCurrentDesktopOnly)
			{
				icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
				if (icon != NULL && icon->fPersonnalScale <= 0)  // pour une icone en cours de supression, on ne fait rien.
				{
					if (event.xconfigure.x + event.xconfigure.width < 0 || event.xconfigure.x > g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] || event.xconfigure.y + event.xconfigure.height < 0 || event.xconfigure.y > g_iScreenHeight[CAIRO_DOCK_HORIZONTAL])  // en fait il faudrait faire ca modulo le nombre de viewports * la largeur d'un bureau, car avec une fenetre a droite, elle peut revenir sur le bureau par la gauche si elle est tres large.
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
	///XSync (s_XDisplay, True);
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
	CairoDock *pParentDock = cairo_dock_manage_appli_class (icon, pMainDock);
	g_return_val_if_fail (pParentDock != NULL, NULL);

	//\_________________ On l'insere dans son dock parent en animant ce dernier eventuellement.
	cairo_dock_insert_icon_in_dock (icon, pParentDock, bUpdateSize, bAnimate, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
	cd_message (" insertion de %s complete (%.2f)\n", icon->acName, icon->fPersonnalScale);

	if (bAnimate && cairo_dock_animation_will_be_visible (pParentDock))
	{
		icon->fPersonnalScale = - 0.95;
		cairo_dock_start_animation (icon, pParentDock);
		return NULL;
	}

	return pParentDock;
}

static void _cairo_dock_remove_old_applis (Window *Xid, Icon *icon, double *fTime)
{
	if (icon != NULL)
	{
		//g_print ("%s (%s, %f / %f)\n", __func__, icon->acName, icon->fLastCheckTime, *fTime);
		if (icon->fLastCheckTime > 0 && icon->fLastCheckTime < *fTime && icon->fPersonnalScale <= 0)
		{
			cd_message ("cette fenetre (%ld) est trop vieille\n", *Xid);
			CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
			if (pParentDock != NULL)
			{
				if (! pParentDock->bInside && (g_bAutoHide || pParentDock->iRefCount != 0) && pParentDock->bAtBottom)
					icon->fPersonnalScale = 0.05;
				else
					icon->fPersonnalScale = 1.0;

				cairo_dock_start_animation (icon, pParentDock);
			}
			else
			{
				cairo_dock_free_icon (icon);
			}
		}
	}
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

	for (i = 0; i < iNbWindows; i ++)
	{
		Xid = pXWindowsList[i];

		bAppliAlreadyRegistered = g_hash_table_lookup_extended (s_hXWindowTable, &Xid, &pOriginalXid, (gpointer *) &icon);
		//icon = g_hash_table_lookup (s_hXWindowTable, &Xid);
		if (! bAppliAlreadyRegistered)
		{
			cd_message (" cette fenetre (%ld) de la pile n'est pas dans la liste\n", Xid);
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDock));
			if (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS)
				icon = cairo_dock_create_icon_from_xwindow (pCairoContext, Xid, pDock);
			if (icon != NULL)
			{
				icon->fLastCheckTime = fTime;
				if ((icon->bIsHidden || ! g_bHideVisibleApplis) && (! g_bAppliOnCurrentDesktopOnly || cairo_dock_window_is_on_current_desktop (Xid)))
				{
					cd_message (" insertion de %s dans %s\n", icon->acName, icon->cParentDockName);
					cairo_dock_insert_appli_in_dock (icon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
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

	g_hash_table_foreach (s_hXWindowTable, (GHFunc) _cairo_dock_remove_old_applis, &fTime);

	XFree (pXWindowsList);
}


GdkFilterReturn filter (GdkXEvent *gdkxevent, GdkEvent *event, gpointer data)
{
	XEvent *xevent = (XEvent *) gdkxevent;

	cd_message ("**************** %s () : type : %d ; window : %d ; message_type : %d(%d) ; format : %d\n", __func__, xevent->type, xevent->xclient.window, xevent->xclient.message_type, XInternAtom (s_XDisplay, "_NET_SYSTEM_TRAY_OPCODE", False), xevent->xclient.format);
	cd_message ("  data : %ld; %ld; %ld; %ld; %ld\n", xevent->xclient.data.l[0], xevent->xclient.data.l[1], xevent->xclient.data.l[2], xevent->xclient.data.l[3], xevent->xclient.data.l[4]);
	return GDK_FILTER_CONTINUE;  // GDK_FILTER_REMOVE
}
GdkFilterReturn filter2 (GdkXEvent *gdkxevent, GdkEvent *event, gpointer data)
{
	XEvent *xevent = (XEvent *) gdkxevent;

	cd_message ("**************** %s () : type : %d ; window : %d\n", __func__, xevent->type, xevent->xclient.window);
	return GDK_FILTER_CONTINUE;  // GDK_FILTER_REMOVE
}
void cairo_dock_start_application_manager (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	g_return_if_fail (s_iSidUpdateAppliList == 0);

	//\__________________ On recupere l'ensemble des fenetres presentes.
	Window root = DefaultRootWindow (s_XDisplay);
	cairo_dock_set_window_mask (root, PropertyChangeMask /*| StructureNotifyMask | SubstructureNotifyMask | ResizeRedirectMask | SubstructureRedirectMask*/);

	gulong i, iNbWindows = 0;
	Window *pXWindowsList = cairo_dock_get_windows_list (&iNbWindows);

	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);

	//\__________________ On cree les icones de toutes ces applis.
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
				cairo_dock_insert_appli_in_dock (pIcon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
				//g_print (">>>>>>>>>>>> Xid : %d\n", Xid);
			}
		}
		else
			cairo_dock_blacklist_appli (Xid);
	}
	XFree (pXWindowsList);

	cairo_dock_update_dock_size (pDock);

	//\__________________ On lance le gestionnaire d'evenements X.
	s_iSidUpdateAppliList = g_timeout_add (CAIRO_DOCK_TASKBAR_CHECK_INTERVAL, (GSourceFunc) cairo_dock_unstack_Xevents, (gpointer) pDock);  // un g_idle_add () consomme 90% de CPU ! :-/

	/*GdkAtom selection = gdk_atom_intern ("_NET_SYSTEM_TRAY_S0", FALSE);
	gboolean bSelectionOk = gtk_selection_owner_set (g_pMainDock->pWidget, selection, GDK_CURRENT_TIME);
	cd_message ("bSelectionOk : %d\n", bSelectionOk);

	GdkAtom message_type = gdk_atom_intern ("_NET_SYSTEM_TRAY_OPCODE", False);
	gdk_add_client_message_filter (message_type, filter, pDock);

	message_type = gdk_atom_intern ("_NET_SYSTEM_TRAY_MESSAGE_DATA", False );
	gdk_add_client_message_filter (message_type, filter2, pDock);*/
}

void cairo_dock_pause_application_manager (void)
{
	if (s_iSidUpdateAppliList != 0)
	{
		Window root = DefaultRootWindow (s_XDisplay);
		cairo_dock_set_window_mask (root, 0);

		g_source_remove (s_iSidUpdateAppliList);
		s_iSidUpdateAppliList = 0;
	}
}

void cairo_dock_stop_application_manager (CairoDock *pDock)
{
	if (s_iSidUpdateAppliList != 0)
	{
		cairo_dock_pause_application_manager ();

		cairo_dock_remove_all_applis (pDock);
	}
}

gboolean cairo_dock_application_manager_is_running (void)
{
	return (s_iSidUpdateAppliList != 0);
}


void cairo_dock_set_strut_partial (int Xid, int left, int right, int top, int bottom, int left_start_y, int left_end_y, int right_start_y, int right_end_y, int top_start_x, int top_end_x, int bottom_start_x, int bottom_end_x)
{
	g_return_if_fail (Xid > 0);

	gulong iGeometryStrut[12] = {left, right, top, bottom, left_start_y, left_end_y, right_start_y, right_end_y, top_start_x, top_end_x, bottom_start_x, bottom_end_x};

	XChangeProperty (s_XDisplay,
		Xid,
		XInternAtom (s_XDisplay, "_NET_WM_STRUT", False),
		XA_CARDINAL, 32, PropModeReplace,
		(guchar *) iGeometryStrut, 12);

	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}

void cairo_dock_set_xwindow_type_hint (int Xid, gchar *cWindowTypeName)
{
	g_return_if_fail (Xid > 0);
	
	gulong iWindowType = XInternAtom (s_XDisplay, cWindowTypeName, False);
	cd_debug ("%s (%d, %s=%d)", __func__, Xid, cWindowTypeName, iWindowType);
	
	XChangeProperty (s_XDisplay,
		Xid,
		s_aNetWmWindowType,
		XA_ATOM, 32, PropModeReplace,
		(guchar *) &iWindowType, 1);
}

gboolean cairo_dock_window_is_utility (int Xid)
{
	g_return_val_if_fail (Xid > 0, FALSE);
	
	gboolean bIsUtility;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements;
	gulong *pTypeBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmWindowType, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pTypeBuffer);
	if (iBufferNbElements != 0)
	{
		cd_debug ("%s (%d) -> %d (%d,%d)\n", __func__, Xid, *pTypeBuffer, s_aNetWmWindowTypeNormal, s_aNetWmWindowTypeUtility);
		bIsUtility = (*pTypeBuffer == s_aNetWmWindowTypeUtility);
		XFree (pTypeBuffer);
	}
	else
		bIsUtility = FALSE;
	return bIsUtility;
}


static void _cairo_dock_set_one_icon_geometry_for_appli (int Xid, int iX, int iY, int iWidth, int iHeight)
{
	g_return_if_fail (Xid > 0);

	gulong iIconGeometry[4] = {iX, iY, iWidth, iHeight};

	XChangeProperty (s_XDisplay,
		Xid,
		XInternAtom (s_XDisplay, "_NET_WM_ICON_GEOMETRY", False),
		XA_CARDINAL, 32, PropModeReplace,
		(guchar *) iIconGeometry, 4);
}
void cairo_dock_set_one_icon_geometry_for_window_manager (Icon *icon, CairoDock *pDock)
{
	if (CAIRO_DOCK_IS_VALID_APPLI (icon))
	{
		int iX, iY, iWidth, iHeight;
		iX = pDock->iWindowPositionX + icon->fXAtRest + (pDock->iCurrentWidth - pDock->fFlatDockWidth) / 2;
		iY = pDock->iWindowPositionY + icon->fDrawY - icon->fHeight * g_fAmplitude;  // il faudrait un fYAtRest ...
		iWidth = icon->fWidth;
		iHeight = icon->fHeight * (1. + 2. * g_fAmplitude);  // on elargit en haut et en bas, pour gerer les cas ou l'icone grossirait vers le haut ou vers le bas.
		//g_print (" -> (%d;%d)\n", iX, iY);

		if (pDock->bHorizontalDock)
			_cairo_dock_set_one_icon_geometry_for_appli (icon->Xid, iX, iY, iWidth, iHeight);
		else
			_cairo_dock_set_one_icon_geometry_for_appli (icon->Xid, iY, iX, iHeight, iWidth);
	}
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
		if (CAIRO_DOCK_IS_VALID_APPLI (icon))
			cairo_dock_set_one_icon_geometry_for_window_manager (icon, pDock);
	}
}


gboolean cairo_dock_update_screen_geometry (CairoDock *pDock)
{
	GdkScreen *gdkscreen = gtk_window_get_screen (GTK_WINDOW (pDock->pWidget));
	if (gdk_screen_get_width (gdkscreen) != g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] || gdk_screen_get_height (gdkscreen) != g_iScreenHeight[CAIRO_DOCK_HORIZONTAL])
	{
		g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] = gdk_screen_get_width (gdkscreen);
		g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] = gdk_screen_get_height (gdkscreen);
		g_iScreenWidth[CAIRO_DOCK_VERTICAL] = g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
		g_iScreenHeight[CAIRO_DOCK_VERTICAL] = g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
		return TRUE;
	}
	else
		return FALSE;
}


gboolean cairo_dock_property_is_present_on_root (gchar *cPropertyName)
{
	g_return_val_if_fail (s_XDisplay != NULL, FALSE);
	Atom atom = XInternAtom (s_XDisplay, cPropertyName, False);
	Window root = DefaultRootWindow (s_XDisplay);
	int iNbProperties;
	Atom *pAtomList = XListProperties (s_XDisplay, root, &iNbProperties);
	int i;
	for (i = 0; i < iNbProperties; i ++)
	{
		if (pAtomList[i] == atom)
			break;
	}
	XFree (pAtomList);
	return (i != iNbProperties);
}
