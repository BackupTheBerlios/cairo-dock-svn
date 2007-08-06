/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <cairo.h>
#include <gtk/gtk.h>

#include </usr/include/X11/Xlib.h>
#include </usr/include/X11/Xatom.h>
#include </usr/include/X11/Xutil.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif


#include "cairo-dock-icons.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-load.h"
#include "cairo-dock-application-factory.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-applications.h"


extern gint g_iScreenWidth;
extern gint g_iScreenHeight;

extern double g_fAmplitude;
extern int g_iLabelSize;
extern gboolean g_bUseText;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern int g_iIconGap;

extern gchar *g_cConfFile;
extern gchar *g_cCairoDockDataDir;

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;

extern int g_iNbAnimationRounds;
extern gchar *g_cLabelPolice;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern int g_iNbStripes;

extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;

extern int g_iMinIconAuthorizedHeight;
extern int g_iMaxIconAuthorizedHeight;
extern double g_fLineColor[4];

extern Display *g_XDisplay;
extern Screen *g_XScreen;
extern Atom g_aNetClientList;
extern GHashTable *g_hXWindowTable;
extern unsigned int g_iAppliMaxNameLength;
extern int g_iSidUpdateAppliList;

extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];


Window *cairo_dock_get_windows_list (gulong *iNbWindows)
{
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	Window *XidList = 0;
	
	Window root = DefaultRootWindow (g_XDisplay);
	gulong iLeftBytes;
	XGetWindowProperty (g_XDisplay, root, g_aNetClientList, 0, G_MAXLONG, False, XA_WINDOW, &aReturnedType, &aReturnedFormat, iNbWindows, &iLeftBytes, (guchar **)&XidList);
	return XidList;
}


gulong cairo_dock_get_xwindow_timestamp (Window Xid)
{
	g_return_val_if_fail (Xid > 0, 0);
	Atom aNetWmUserTime = XInternAtom (g_XDisplay, "_NET_WM_USER_TIME", False);
	gulong iLeftBytes, iBufferNbElements = 0;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	gulong *pTimeBuffer = NULL;
	XGetWindowProperty (g_XDisplay, Xid, aNetWmUserTime, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pTimeBuffer);
	gulong iTimeStamp = 0;
	if (iBufferNbElements > 0)
		iTimeStamp = *pTimeBuffer;
	XFree (pTimeBuffer);
	return iTimeStamp;
}

void cairo_dock_set_xwindow_timestamp (Window Xid, gulong iTimeStamp)
{
	g_return_if_fail (Xid > 0);
	Atom aNetWmUserTime = XInternAtom (g_XDisplay, "_NET_WM_USER_TIME", False);
	XChangeProperty (g_XDisplay, Xid,
		aNetWmUserTime,
		XA_CARDINAL, 32, PropModeReplace,
		(guchar *)&iTimeStamp, 1);
}


void cairo_dock_close_xwindow (Window Xid)
{
	//g_print ("%s (%d)\n", __func__, Xid);
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = g_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (g_XDisplay, "_NET_CLOSE_WINDOW", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = cairo_dock_get_xwindow_timestamp (Xid);  // timestamp
	xClientMessage.xclient.data.l[1] = 2;  // 2 <=> pagers and other Clients that represent direct user actions.
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;
	
	Window root = DefaultRootWindow (g_XDisplay);
	XSendEvent (g_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
	
	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}


void cairo_dock_show_appli (Window Xid)
{
	//g_print ("%s (%d)\n", __func__, Xid);
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = g_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (g_XDisplay, "_NET_ACTIVE_WINDOW", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = 2;  // source indication
	xClientMessage.xclient.data.l[1] = 0;  // timestamp
	xClientMessage.xclient.data.l[2] = 0;  // requestor's currently active window, 0 if none
	xClientMessage.xclient.data.l[3] = 0;
	xClientMessage.xclient.data.l[4] = 0;
	
	Window root = DefaultRootWindow (g_XDisplay);
	XSendEvent (g_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
	
	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}


void cairo_dock_minimize_xwindow (Window Xid)
{
	g_return_if_fail (Xid > 0);
	XIconifyWindow (g_XDisplay, Xid, DefaultScreen (g_XDisplay));
	//Window root = DefaultRootWindow (g_XDisplay);
	//cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}

void cairo_dock_maximize_xwindow (Window Xid, gboolean bMaximize)
{
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = g_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (g_XDisplay, "_NET_WM_STATE", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = bMaximize;
	xClientMessage.xclient.data.l[1] = XInternAtom (g_XDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	xClientMessage.xclient.data.l[2] = XInternAtom (g_XDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	xClientMessage.xclient.data.l[3] = 2;
	xClientMessage.xclient.data.l[4] = 0;

	Window root = DefaultRootWindow (g_XDisplay);
	XSendEvent (g_XDisplay,
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
	xClientMessage.xclient.display = g_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (g_XDisplay, "_NET_WM_STATE", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = bFullScreen;
	xClientMessage.xclient.data.l[1] = XInternAtom (g_XDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 2;
	xClientMessage.xclient.data.l[4] = 0;

	Window root = DefaultRootWindow (g_XDisplay);
	XSendEvent (g_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
	
	cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}

void cairo_dock_move_xwindow_to_nth_desktop (Window Xid, int iDesktopNumber)
{
	g_return_if_fail (Xid > 0);
	XEvent xClientMessage;
	
	xClientMessage.xclient.type = ClientMessage;
	xClientMessage.xclient.serial = 0;
	xClientMessage.xclient.send_event = True;
	xClientMessage.xclient.display = g_XDisplay;
	xClientMessage.xclient.window = Xid;
	xClientMessage.xclient.message_type = XInternAtom (g_XDisplay, "_NET_WM_DESKTOP", False);
	xClientMessage.xclient.format = 32;
	xClientMessage.xclient.data.l[0] = iDesktopNumber;
	xClientMessage.xclient.data.l[1] = 2;
	xClientMessage.xclient.data.l[2] = 0;
	xClientMessage.xclient.data.l[3] = 2;
	xClientMessage.xclient.data.l[4] = 0;

	Window root = DefaultRootWindow (g_XDisplay);
	XSendEvent (g_XDisplay,
		root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xClientMessage);
	
	cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
}


gboolean cairo_dock_window_is_maximized (Window Xid)
{
	g_return_val_if_fail (Xid > 0, FALSE);
	Atom aNetWmMState = XInternAtom (g_XDisplay, "_NET_WM_STATE", False);
	
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (g_XDisplay, Xid, aNetWmMState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	int iIsMaximized = 0;
	if (iBufferNbElements > 0)
	{
		int i;
		Atom aNetWmMaximizedVertically = XInternAtom (g_XDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		Atom aNetWmMaximizedHorizontally = XInternAtom (g_XDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
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
	Atom aNetWmMState = XInternAtom (g_XDisplay, "_NET_WM_STATE", False);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	XGetWindowProperty (g_XDisplay, Xid, aNetWmMState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	
	gboolean bIsFullScreen = FALSE;
	if (iBufferNbElements > 0)
	{
		int i;
		Atom aNetWmFullScreen = XInternAtom (g_XDisplay, "_NET_WM_STATE_FULLSCREEN", False);
		for (i = 0; i < iBufferNbElements && ! bIsFullScreen; i ++)
		{
			if (pXStateBuffer[i] == aNetWmFullScreen)
				bIsFullScreen = TRUE;
		}
	}
	
	XFree (pXStateBuffer);
	return bIsFullScreen;
}

int cairo_dock_get_current_desktop (void)
{
	Atom aNetCurrentDesktop = XInternAtom (g_XDisplay, "_NET_NUMBER_OF_DESKTOPS", False);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXDesktopNumberBuffer = NULL;
	Window root = DefaultRootWindow (g_XDisplay);
	XGetWindowProperty (g_XDisplay, root, aNetCurrentDesktop, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXDesktopNumberBuffer);
	
	int iCurrentDesktop = 0;
	if (iBufferNbElements > 0)
		iCurrentDesktop = *pXDesktopNumberBuffer;
	XFree (pXDesktopNumberBuffer);
	return iCurrentDesktop;
}




void cairo_dock_set_root_window_mask (void)
{
	static gboolean bAlreadySet = FALSE;
	if (bAlreadySet)
		return ;
	bAlreadySet = TRUE;
	
	XSetWindowAttributes attr;
	XWindowAttributes wattr;
	
	attr.event_mask = 
		//StructureNotifyMask | /* ResizeRedirectMask | */
		SubstructureNotifyMask 
		//SubstructureRedirectMask 
		//PropertyChangeMask
		;
	
	Window root = DefaultRootWindow (g_XDisplay);
	XGetWindowAttributes(g_XDisplay, root, &wattr);
	//if (wattr.all_event_masks & ButtonPressMask)
	//	attr.event_mask &= ~ButtonPressMask;
	attr.event_mask &= ~SubstructureRedirectMask;
	XSelectInput(g_XDisplay, root, attr.event_mask);
}


static XEvent event;
gboolean cairo_dock_update_applis_list (CairoDock *pDock)
{
	Bool bEventPresent;
	gboolean bInterestedEvent = FALSE;
	Window root = DefaultRootWindow (g_XDisplay);
	//bEventPresent = XCheckTypedWindowEvent(g_XDisplay, root, UnmapNotify, &event);
	
	//\_____________________ On regarde si une fenetre s'est faite effacee.
	bEventPresent = TRUE;
	while (bEventPresent)
	{
		bEventPresent = XCheckTypedEvent(g_XDisplay, DestroyNotify, &event);
		if (bEventPresent)
		{
			bInterestedEvent = TRUE;
			//g_print ("Destroy (%d)\n", event.xdestroywindow.window);
			Icon *icon = g_hash_table_lookup (g_hXWindowTable, &event.xdestroywindow.window);
			if (icon != NULL)
			{
				//g_print ("c'est %s qui se fait exploser\n", icon->acName);
				icon->fPersonnalScale = 1.0;
				if (pDock->iSidShrinkDown == 0)
					pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pDock);
			}
		}
	}
	//\_____________________ On regarde si une fenetre est apparue.
	bEventPresent = TRUE;
	//while (bEventPresent)
	{
		bEventPresent = XCheckTypedEvent(g_XDisplay, CreateNotify, &event);
		if (bEventPresent)
		{
			bInterestedEvent = TRUE;
			//g_print ("Create (%d)\n", event.xunmap.window);
			Icon *icon = g_hash_table_lookup (g_hXWindowTable, &event.xunmap.window);
			if (icon != NULL)
			{
				//g_print ("c'est %s qui ressucite\n", icon->acName);
				///icon->bIsMapped = TRUE;
			}
			else
			{
				//g_print ("c'est une nouvelle fenetre\n");
				cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
				icon = cairo_dock_create_icon_from_xwindow (pCairoContext, event.xunmap.window, pDock);
				if (icon != NULL)
				{
					cairo_dock_insert_icon_in_dock (icon, pDock, TRUE, TRUE);
					if (pDock->iSidShrinkDown == 0)
						pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pDock);
				}
			}
		}
	}
	
	//g_print ("%d events\n", XEventsQueued (g_XDisplay, QueuedAlready));
	if (!bInterestedEvent)
	{
		long event_mask = 0xFFFFFFFF;
		//while (XCheckWindowEvent (g_XDisplay, root, event_mask, &event))
		while (XCheckMaskEvent (g_XDisplay, event_mask, &event))
		{
			if (event.type == CreateNotify || event.type == DestroyNotify)
			{
				XPutBackEvent (g_XDisplay, &event);
				//g_print ("On le remet dans la queue\n");
				break ;
			}
		}
	}
	return TRUE;
}


int cairo_dock_xerror_handler (Display * pDisplay, XErrorEvent *pXError)
{
	//g_print ("Erreur (%d, %d, %d) lors d'une requete sur %d\n", pXError->error_code, pXError->request_code, pXError->minor_code, pXError->resourceid);
	return 0;
}


void cairo_dock_show_all_applis (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	cairo_dock_set_root_window_mask ();
	gulong i, iNbWindows = 0;
	Window *pXWindowsList = cairo_dock_get_windows_list (&iNbWindows);
	Window Xid;
	Icon *pIcon;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
	
	//\__________________ On cree les icones de toutes les applis existantes.
	for (i = 0; i < iNbWindows; i ++)
	{
		Xid = pXWindowsList[i];
		pIcon = cairo_dock_create_icon_from_xwindow (pCairoContext, Xid, pDock);
		
		if (pIcon != NULL)
			cairo_dock_insert_icon_in_dock (pIcon, pDock, FALSE, FALSE);
		//if (pIcon != NULL)
		//	g_print (">>>>>>>>>>>> Xid : %d\n", Xid);
	}
	
	cairo_dock_update_dock_size (pDock, pDock->iMaxIconHeight, pDock->iMinDockWidth);
	g_iSidUpdateAppliList = g_timeout_add (200, (GSourceFunc) cairo_dock_update_applis_list, (gpointer) pDock);  // un g_idle_add () consomme 90% de CPU ! :-/
}


