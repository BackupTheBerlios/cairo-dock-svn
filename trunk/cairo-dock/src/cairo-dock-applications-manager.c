/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

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
#include "cairo-dock-applications-manager.h"

#define CAIRO_DOCK_TASKBAR_CHECK_INTERVAL 250

extern gboolean g_bAutoHide;
extern double g_fAmplitude;

extern int g_iScreenWidth[2], g_iScreenHeight[2];

extern gboolean g_bUniquePid;

static GHashTable *s_hXWindowTable = NULL;  // table des fenetres X affichees dans le dock.
static Atom s_aNetClientList;
static Display *s_XDisplay = NULL;
static int s_iSidUpdateAppliList = 0;

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
	
	cairo_dock_initialize_application_factory (s_XDisplay);
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

void cairo_dock_unregister_appli (Icon *icon)
{
	if (CAIRO_DOCK_IS_VALID_APPLI (icon))
	{
		g_hash_table_remove (s_hXWindowTable, &icon->Xid);
		icon->Xid = 0;
		
		cairo_dock_unregister_pid (icon);
		
		g_free (icon->cClass);
		icon->cClass = NULL;
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
	XChangeProperty (s_XDisplay, Xid,
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
			g_print ("kill (%d)\n", pPidBuffer[0]);
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
	Atom aNetWmDesktop = XInternAtom (s_XDisplay, "_NET_WM_DESKTOP", False);
	gulong iLeftBytes, iBufferNbElements = 0;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	gulong *pBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, aNetWmDesktop, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pBuffer);
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
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_CURRENT_DESKTOP", False);
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

void cairo_dock_maximize_xwindow (Window Xid, gboolean bMaximize)
{
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_WM_STATE", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = bMaximize;
	xClientMessage.xclient.data.l[1] = XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	xClientMessage.xclient.data.l[2] = XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
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

void cairo_dock_set_xwindow_fullscreen (Window Xid, gboolean bFullScreen)
{
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_WM_STATE", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = bFullScreen;
	xClientMessage.xclient.data.l[1] = XInternAtom (s_XDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xClientMessage.xclient.data.l[2] = 0;
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
	XGetGeometry(s_XDisplay, Xid, &root_return, &x_return, &y_return, &width_return, 
		&height_return, &border_width_return, &depth_return);
	while (x_return < 0)
		x_return += g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
	while (x_return >= g_iScreenWidth[CAIRO_DOCK_HORIZONTAL])
		x_return -= g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
	while (y_return < 0)
		y_return += g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
	while (y_return >= g_iScreenHeight[CAIRO_DOCK_HORIZONTAL])
		y_return -= g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
	//g_print ("position relative : (%d;%d)\n", x_return, y_return);
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = s_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (s_XDisplay, "_NET_MOVERESIZE_WINDOW", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = StaticGravity | (1 << 8) | (1 << 9) | (0 << 10) | (0 << 11);
	xClientMessage.xclient.data.l[1] = iDesktopViewportX + x_return;
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


gboolean cairo_dock_window_is_maximized (Window Xid)
{
	g_return_val_if_fail (Xid > 0, FALSE);
	Atom aNetWmMState = XInternAtom (s_XDisplay, "_NET_WM_STATE", False);
	
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, aNetWmMState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
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

gboolean cairo_dock_window_is_fullscreen (Window Xid)
{
	g_return_val_if_fail (Xid > 0, FALSE);
	Atom aNetWmMState = XInternAtom (s_XDisplay, "_NET_WM_STATE", False);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, aNetWmMState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	
	gboolean bIsFullScreen = FALSE;
	if (iBufferNbElements > 0)
	{
		int i;
		Atom aNetWmFullScreen = XInternAtom (s_XDisplay, "_NET_WM_STATE_FULLSCREEN", False);
		for (i = 0; i < iBufferNbElements && ! bIsFullScreen; i ++)
		{
			if (pXStateBuffer[i] == aNetWmFullScreen)
				bIsFullScreen = TRUE;
		}
	}
	
	XFree (pXStateBuffer);
	return bIsFullScreen;
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

void cairo_dock_get_current_desktop (int *iDesktopNumber, int *iDesktopViewportX, int *iDesktopViewportY)
{
	Atom aNetCurrentDesktop = XInternAtom (s_XDisplay, "_NET_CURRENT_DESKTOP", False);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXDesktopNumberBuffer = NULL;
	Window root = DefaultRootWindow (s_XDisplay);
	XGetWindowProperty (s_XDisplay, root, aNetCurrentDesktop, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXDesktopNumberBuffer);
	
	if (iBufferNbElements > 0)
		*iDesktopNumber = *pXDesktopNumberBuffer;
	else
		*iDesktopNumber = 0;
	XFree (pXDesktopNumberBuffer);
	//g_print ("bureau actuel : %d\n", *iDesktopNumber);
	
	Atom aNetDesktopViewport = XInternAtom (s_XDisplay, "_NET_DESKTOP_VIEWPORT", False);
	iBufferNbElements = 0;
	gulong *pXDesktopViewport = NULL;
	XGetWindowProperty (s_XDisplay, root, aNetDesktopViewport, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXDesktopViewport);
	if (iBufferNbElements > 1)
	{
		if (iBufferNbElements >= *iDesktopNumber)
		{
			*iDesktopViewportX = pXDesktopViewport[2*(*iDesktopNumber)];
			*iDesktopViewportY = pXDesktopViewport[2*(*iDesktopNumber)+1];
		}
		else
		{
			*iDesktopViewportX = pXDesktopViewport[0];
			*iDesktopViewportY = pXDesktopViewport[1];
		}
	}
	else
	{
		
		*iDesktopViewportX = 0;
		*iDesktopViewportY = 0;
	}
	XFree (pXDesktopViewport);
	//g_print ("viewport actuel : (%d;%d)\n", *iDesktopViewportX, *iDesktopViewportY);
	
	/*Atom aNetWorkArea = XInternAtom (s_XDisplay, "_NET_WORKAREA", False);
	iBufferNbElements = 0;
	gulong *pXWorkArea = NULL;
	XGetWindowProperty (s_XDisplay, root, aNetWorkArea, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXWorkArea);
	int i;
	for (i = 0; i < iBufferNbElements/4; i ++)
	{
		g_print ("work area : (%d;%d) %dx%d\n", pXWorkArea[4*i], pXWorkArea[4*i+1], pXWorkArea[4*i+2], pXWorkArea[4*i+3]);
	}
	XFree (pXWorkArea);*/
}




gboolean cairo_dock_update_applis_list (CairoDock *pDock)
{
	static XEvent event;
	static gboolean bInProgress = FALSE;
	if (bInProgress)
		return TRUE;
	bInProgress = TRUE;
	
	g_return_val_if_fail (pDock != NULL, FALSE);
	/*Bool bEventPresent;
	gboolean bInterestedEvent = FALSE;
	Window root = DefaultRootWindow (s_XDisplay);
	//Window xUnwantedWindow = 0;
	
	//\_____________________ On regarde si une fenetre apparait.
	bEventPresent = TRUE;
	while (bEventPresent)
	{
		bEventPresent = XCheckTypedEvent(s_XDisplay, MapNotify, &event);
		if (bEventPresent)  // && event.xmap.window != xUnwantedWindow
		{
			bInterestedEvent = TRUE;
			g_print ("Map (%d)\n", event.xmap.window);
			Icon *icon = g_hash_table_lookup (s_hXWindowTable, &event.xmap.window);
			if (icon != NULL)
			{
				//g_print ("c'est %s qui re-apparait\n", icon->acName);
				icon->bIsMapped = TRUE;
				if (icon->fPersonnalScale > 0)  // elle est en train de disparaitre, on inverse le processus.
				{
					//if (pDock->iSidShrinkDown > 0)
					//	g_source_remove (pDock->iSidShrinkDown);
					//pDock->iSidShrinkDown = 0;
					if (! pDock->bInside && g_bAutoHide && pDock->bAtBottom)
						icon->fPersonnalScale = - 0.05;
					else
						icon->fPersonnalScale = -0.95;
					//pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
				}
			}
			else
			{
				g_print ("c'est une nouvelle fenetre qui apparait\n");
				cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
				if (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS)
					icon = cairo_dock_create_icon_from_xwindow (pCairoContext, event.xmap.window, pDock);
				if (icon != NULL)
				{
					g_print ("insertion de %s dans %s\n", icon->acName, icon->cParentDockName);
					CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
					g_return_val_if_fail (pParentDock != NULL, TRUE);
					cairo_dock_insert_icon_in_dock (icon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
					if (! pParentDock->bInside && g_bAutoHide && pParentDock->bAtBottom)
						icon->fPersonnalScale = - 0.05;
					cairo_dock_start_animation (icon, pParentDock);
				}
			}
		}
	}
	
	//\_____________________ On regarde si une fenetre s'est faite effacee.
	bEventPresent = TRUE;
	///while (bEventPresent)
	{
		bEventPresent = XCheckTypedEvent(s_XDisplay, DestroyNotify, &event);
		if (bEventPresent)
		{
			bInterestedEvent = TRUE;
			g_print ("Destroy (%d)\n", event.xdestroywindow.window);
			Icon *icon = g_hash_table_lookup (s_hXWindowTable, &event.xdestroywindow.window);
			if (icon != NULL && icon->fPersonnalScale <= 0)
			{
				//g_print ("c'est %s qui se fait exploser\n", icon->acName);
				icon->bIsMapped = FALSE;
				
				CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
				if (pParentDock == NULL)
					pParentDock = pDock;
				
				if (! pParentDock->bInside && (g_bAutoHide || pParentDock->iRefCount != 0) && pParentDock->bAtBottom)
					icon->fPersonnalScale = 0.05;
				else
					icon->fPersonnalScale = 1.0;
				
				cairo_dock_start_animation (icon, pParentDock);
			}
		}
	}
	//\_____________________ On regarde si une fenetre est creee.
	bEventPresent = TRUE;
	//while (bEventPresent)
	{
		bEventPresent = XCheckTypedEvent(s_XDisplay, CreateNotify, &event);
		if (bEventPresent)
		{
			bInterestedEvent = TRUE;
			g_print ("Create (%d)\n", event.xcreatewindow.window);
			Icon *icon = g_hash_table_lookup (s_hXWindowTable, &event.xcreatewindow.window);
			if (icon != NULL)  // a priori impossible.
			{
				g_print ("c'est %s qui ressucite d'entre les morts\n", icon->acName);
				icon->bIsMapped = TRUE;
			}
			else
			{
				g_print ("c'est une nouvelle fenetre qui est cree\n");
				cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
				icon = cairo_dock_create_icon_from_xwindow (pCairoContext, event.xcreatewindow.window, pDock);
				if (icon != NULL)
				{
					g_print (" -> %s\n", icon->acName);
					cairo_dock_insert_icon_in_dock (icon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
					if (! pDock->bInside && g_bAutoHide && pDock->bAtBottom)
						icon->fPersonnalScale = - 0.05;
					if (pDock->iSidShrinkDown == 0)
						pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
				}
				else
					xUnwantedWindow = event.xcreatewindow.window;
			}
		}
	}
	//\_____________________ On regarde si une fenetre se cache.
	bEventPresent = TRUE;
	///while (bEventPresent)
	{
		bEventPresent = XCheckTypedEvent(s_XDisplay, UnmapNotify, &event);
		if (bEventPresent)
		{
			bInterestedEvent = TRUE;
			g_print ("Unmap (%d)\n", event.xunmap.window);
			Icon *icon = g_hash_table_lookup (s_hXWindowTable, &event.xunmap.window);
			if (icon != NULL)
			{
				if (icon->bIsMapped)
				{
					//g_print ("est %s qui se cache\n", icon->acName);
					icon->bIsMapped = FALSE;
				}
				else if (icon->fPersonnalScale <= 0)
				{
					// Ce qu'il faudrait faire : reduire son icone de moitie et la deplacer a droite des applis. Cependant, la zone de notification de gnome reduit la fenetre des qu'on veut la remapper nous-memes ! Du coup pas le choix, on l'enleve de la barre.
					//g_print ("c'est %s qui se fait degager (%d)\n", icon->acName, event.xunmap.from_configure);
					CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
					if (pParentDock == NULL)
						pParentDock = pDock;
					
					if (! pParentDock->bInside && (g_bAutoHide || pParentDock->iRefCount != 0) && pParentDock->bAtBottom)
						icon->fPersonnalScale = 0.05;
					else
						icon->fPersonnalScale = 1.0;
					
					cairo_dock_start_animation (icon, pParentDock);
				}
			}
		}
	}
	
	
	//g_print ("%d events\n", XEventsQueued (s_XDisplay, QueuedAlready));
	//\_____________________ On vide la queue des messages qui ne nous interessent pas.
	bEventPresent = FALSE;
	//if (!bInterestedEvent)
	{
		long event_mask = 0xFFFFFFFF;
		//while (XCheckWindowEvent (s_XDisplay, root, event_mask, &event))
		while (XCheckMaskEvent (s_XDisplay, event_mask, &event))
		{
			//g_print ("on vide un evenement\n");
			if (event.type == DestroyNotify || event.type == UnmapNotify || event.type == MapNotify)  // event.type == CreateNotify || 
			{
				XPutBackEvent (s_XDisplay, &event);
				g_print ("  on le remet dans la queue (%d/%d,%d,%d)\n", event.type, MapNotify, UnmapNotify, DestroyNotify);
				bEventPresent = TRUE;
				break ;
			}
		}
	}
	if (! bEventPresent)
	{
		XSync (s_XDisplay, True);
	}*/
	
	long event_mask = 0xFFFFFFFF;
	Icon *icon;
	while (XCheckMaskEvent (s_XDisplay, event_mask, &event))
	{
		icon = NULL;
		switch (event.type)
		{
			//case CreateNotify :
			case MapNotify :
				g_print ("Map/Create (%d)\n", event.xmap.window);
				icon = g_hash_table_lookup (s_hXWindowTable, &event.xmap.window);
				if (icon != NULL)
				{
					if (event.type == MapNotify)
						g_print ("c'est %s qui re-apparait\n", icon->acName);
					else
						g_print ("c'est %s qui ressucite d'entre les morts\n", icon->acName);
					icon->bIsMapped = TRUE;
					if (icon->fPersonnalScale > 0)  // elle etait en train de disparaitre, on inverse le processus.
						icon->fPersonnalScale = - icon->fPersonnalScale;
				}
				else
				{
					//g_print ("c'est une nouvelle fenetre qui apparait\n");
					cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
					if (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS)
						icon = cairo_dock_create_icon_from_xwindow (pCairoContext, event.xmap.window, pDock);
					if (icon != NULL)
					{
						g_print ("  insertion de %s dans %s\n", icon->acName, icon->cParentDockName);
						CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
						g_return_val_if_fail (pParentDock != NULL, TRUE);
						cairo_dock_insert_icon_in_dock (icon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
						if (! pParentDock->bInside && g_bAutoHide && pParentDock->bAtBottom)
							icon->fPersonnalScale = - 0.05;
						g_print ("  insertion complete (%.2f)\n", icon->fPersonnalScale);
						cairo_dock_start_animation (icon, pParentDock);
					}
				}
			break;
			
			case UnmapNotify :
				g_print ("Unmap (%d)\n", event.xunmap.window);
				icon = g_hash_table_lookup (s_hXWindowTable, &event.xunmap.window);
				if (icon != NULL)
				{
					if (icon->bIsMapped)
					{
						//g_print ("est %s qui se cache\n", icon->acName);
						icon->bIsMapped = FALSE;
					}
					else if (icon->fPersonnalScale <= 0)  // pas deja en cours de suppression.
					{
						// Ce qu'il faudrait faire : reduire son icone de moitie et la deplacer a droite des applis. Cependant, la zone de notification de gnome reduit la fenetre des qu'on veut la remapper nous-memes ! Du coup pas le choix, on l'enleve de la barre.
						//g_print ("c'est %s qui se fait degager (%d)\n", icon->acName, event.xunmap.from_configure);
						CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
						if (pParentDock == NULL)
							pParentDock = pDock;
						
						if (! pParentDock->bInside && (g_bAutoHide || pParentDock->iRefCount != 0) && pParentDock->bAtBottom)
							icon->fPersonnalScale = 0.05;
						else
							icon->fPersonnalScale = 1.0;
						
						cairo_dock_start_animation (icon, pParentDock);
					}
				}
			break;
			
			case DestroyNotify :
				g_print ("Destroy (%d)\n", event.xdestroywindow.window);
				icon = g_hash_table_lookup (s_hXWindowTable, &event.xdestroywindow.window);
				if (icon != NULL && icon->fPersonnalScale <= 0)  // pas deja en cours de suppression.
				{
					//g_print ("c'est %s qui se fait exploser\n", icon->acName);
					icon->bIsMapped = FALSE;
					
					CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
					if (pParentDock == NULL)
						pParentDock = pDock;
					
					if (! pParentDock->bInside && (g_bAutoHide || pParentDock->iRefCount != 0) && pParentDock->bAtBottom)
						icon->fPersonnalScale = 0.05;
					else
						icon->fPersonnalScale = 1.0;
					
					cairo_dock_start_animation (icon, pParentDock);
				}
			break;
			
			case PropertyNotify :
				//g_print ("PropertyNotify (%d)\n", event.xproperty.window);
				//g_print ("  type : %d; atom : %s\n", event.xproperty.type, gdk_x11_get_xatom_name (event.xproperty.atom));
				icon = g_hash_table_lookup (s_hXWindowTable, &event.xproperty.window);
				if (icon != NULL)
				{
					CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
					if (pParentDock == NULL)
						pParentDock = pDock;
					cairo_dock_Xproperty_changed (icon, event.xproperty.atom, pParentDock);
				}
			break;
		}
	}
	XSync (s_XDisplay, True);
	
	bInProgress = FALSE;
	return TRUE;
}

void cairo_dock_set_root_window_mask (void)
{
	static gboolean bAlreadySet = FALSE;
	if (bAlreadySet)
		return ;
	bAlreadySet = TRUE;
	
	//XWindowAttributes wattr;
	XSetWindowAttributes attr;
	memset (&attr.event_mask, 0, sizeof (attr.event_mask));
	
	attr.event_mask = 
		//StructureNotifyMask | /* ResizeRedirectMask | */
		//SubstructureRedirectMask |
		//PropertyChangeMask |
		SubstructureNotifyMask;
	
	Window root = DefaultRootWindow (s_XDisplay);
	//XGetWindowAttributes(s_XDisplay, root, &wattr);
	//if (wattr.all_event_masks & ButtonPressMask)
	//	attr.event_mask &= ~ButtonPressMask;  // l'utilite reste a demontrer.
	XSelectInput(s_XDisplay, root, attr.event_mask);
}

void cairo_dock_set_normal_window_mask (Window Xid)
{
	XSetWindowAttributes attr;
	//XWindowAttributes wattr;
	memset (&attr.event_mask, 0, sizeof (attr.event_mask));
	
	attr.event_mask = 
		//StructureNotifyMask | /* ResizeRedirectMask | */
		//SubstructureRedirectMask |
		PropertyChangeMask |
		SubstructureNotifyMask;
	
	//XGetWindowAttributes (s_XDisplay, Xid, &wattr);
	//if (wattr.all_event_masks & ButtonPressMask)
	//	attr.event_mask &= ~ButtonPressMask;  // l'utilite reste a demontrer.
	//attr.event_mask &= ~SubstructureRedirectMask;  // inutile.
	XSelectInput (s_XDisplay, Xid, attr.event_mask);
}



Window *cairo_dock_get_windows_list (gulong *iNbWindows)
{
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	Window *XidList = 0;
	
	Window root = DefaultRootWindow (s_XDisplay);
	gulong iLeftBytes;
	XGetWindowProperty (s_XDisplay, root, s_aNetClientList, 0, G_MAXLONG, False, XA_WINDOW, &aReturnedType, &aReturnedFormat, iNbWindows, &iLeftBytes, (guchar **)&XidList);
	return XidList;
}

void cairo_dock_start_application_manager (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	g_return_if_fail (s_iSidUpdateAppliList == 0);
	cairo_dock_set_root_window_mask ();
	gulong i, iNbWindows = 0;
	Window *pXWindowsList = cairo_dock_get_windows_list (&iNbWindows);
	Window Xid;
	Icon *pIcon;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	//\__________________ On cree les icones de toutes les applis existantes.
	for (i = 0; i < iNbWindows; i ++)
	{
		Xid = pXWindowsList[i];
		pIcon = cairo_dock_create_icon_from_xwindow (pCairoContext, Xid, pDock);
		
		if (pIcon != NULL)
		{
			CairoDock *pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
			g_return_if_fail (pParentDock != NULL);
			cairo_dock_insert_icon_in_dock (pIcon, pParentDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
			//g_print (">>>>>>>>>>>> Xid : %d\n", Xid);
		}
	}
	
	cairo_dock_update_dock_size (pDock);
	s_iSidUpdateAppliList = g_timeout_add (CAIRO_DOCK_TASKBAR_CHECK_INTERVAL, (GSourceFunc) cairo_dock_update_applis_list, (gpointer) pDock);  // un g_idle_add () consomme 90% de CPU ! :-/
}

void cairo_dock_pause_application_manager (void)
{
	if (s_iSidUpdateAppliList != 0)
	{
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
	
	gulong iwindowType = XInternAtom(s_XDisplay, cWindowTypeName, False);
	
	XChangeProperty (s_XDisplay,
		Xid,
		XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE", False),
		XA_ATOM, 32, PropModeReplace,
		(guchar *) &iwindowType, 1);
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
		iX = pDock->iWindowPositionX + icon->fXAtRest + (pDock->iCurrentWidth - pDock->iFlatDockWidth) / 2;
		iY = pDock->iWindowPositionY + icon->fDrawY - icon->fHeight * g_fAmplitude;  // il faudrait un fYAtRest ...  /// il faudrait calculer fDrawY avant ...
		iWidth = icon->fWidth;
		iHeight = icon->fHeight * (1. + 2. * g_fAmplitude);  // on elargit en haut et en bas, pour gerer les cas ou l'icone grossirait vers le haut ou vers le bas.
		
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
