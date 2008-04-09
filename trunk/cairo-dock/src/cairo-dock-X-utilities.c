/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <signal.h>

#include "cairo-dock-applications-manager.h"
#include "cairo-dock-application-factory.h"
#include "cairo-dock-log.h"
#include "cairo-dock-X-utilities.h"

extern int g_iNbDesktops;
extern int g_iNbViewportX,g_iNbViewportY ;
extern int g_iScreenWidth[2], g_iScreenHeight[2];

static Display *s_XDisplay = NULL;
static Atom s_aNetWmWindowType;
static Atom s_aNetWmWindowTypeNormal;
static Atom s_aNetWmWindowTypeUtility;
static Atom s_aNetCurrentDesktop;
static Atom s_aNetDesktopViewport;
static Atom s_aNetDesktopGeometry;

static int _cairo_dock_xerror_handler (Display * pDisplay, XErrorEvent *pXError)
{
	cd_debug ("Erreur (%d, %d, %d) lors d'une requete X sur %d", pXError->error_code, pXError->request_code, pXError->minor_code, pXError->resourceid);
	return 0;
}
void cairo_dock_initialize_X_support (void)
{
	s_XDisplay = XOpenDisplay (0);
	g_return_if_fail (s_XDisplay != NULL);
	
	XSetErrorHandler (_cairo_dock_xerror_handler);
	
	s_aNetWmWindowType = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE", False);
	s_aNetWmWindowTypeNormal = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	s_aNetWmWindowTypeUtility = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE_UTILITY", False);
	s_aNetCurrentDesktop = XInternAtom (s_XDisplay, "_NET_CURRENT_DESKTOP", False);
	s_aNetDesktopViewport = XInternAtom (s_XDisplay, "_NET_DESKTOP_VIEWPORT", False);
	s_aNetDesktopGeometry = XInternAtom (s_XDisplay, "_NET_DESKTOP_GEOMETRY", False);
	
	cairo_dock_update_screen_geometry ();
	
	g_iNbDesktops = cairo_dock_get_nb_desktops ();
	cairo_dock_get_nb_viewports (&g_iNbViewportX, &g_iNbViewportY);
	
	cairo_dock_initialize_application_manager (s_XDisplay);
	cairo_dock_initialize_application_factory (s_XDisplay);
	
	/*GdkWindow * root;
	GdkPixmap* pix = NULL;
	GdkPixbuf* img;
	GdkScreen* screen = gdk_screen_get_default ();
	root = gdk_screen_get_root_window( screen );
	GdkColor desktopBg1;
	desktopBg1.red = 0;
	desktopBg1.green = 0;
	desktopBg1.blue = 65535;
	
	int screenw, screenh;
	int imgw, imgh, x = 0, y = 0;
	screenw = gdk_screen_get_width( screen );
	screenh = gdk_screen_get_height( screen );
	img = gdk_pixbuf_new_from_file_at_scale (
		"/usr/share/backgrounds/space-01.jpg",
		screenw, screenh,
		TRUE, NULL);
	if ( img )
	{
		g_print ("WALLPAPER (%dx%d)\n", screenw, screenh);
		GdkGC * gc;
		pix = gdk_pixmap_new( root, screenw, screenh, -1 );
		imgw = gdk_pixbuf_get_width( img );
		imgh = gdk_pixbuf_get_height( img );
		if ( imgw == screenw )
		{
			//center vertically 
			y = ( screenh - imgh ) / 2;
		}
		else
		{
			// center horizontally 
			x = ( screenw - imgw ) / 2;
		}
		// FIXME: fill the blank area with bg color.
		gc = gdk_gc_new( pix );
		gdk_gc_set_rgb_fg_color( gc, &desktopBg1 );
		gdk_gc_set_rgb_bg_color( gc, &desktopBg1 );
		gdk_gc_set_fill( gc, GDK_SOLID );
		// fill the whole pixmap is not efficient at all!!! 
		gdk_draw_rectangle( pix, gc, TRUE,
			0, 0, screenw, screenh );
		g_object_unref( G_OBJECT( gc ) );
		gdk_draw_pixbuf( pix, NULL, img, 0, 0, x, y,
			imgw, imgh,
			GDK_RGB_DITHER_NONE, 0, 0 );
		gdk_pixbuf_unref( img );
		
		gdk_window_set_back_pixmap( root, pix, FALSE );
		if ( pix )
			g_object_unref( G_OBJECT( pix ) );
		gdk_window_clear( root );
		
		XSetWindowBackgroundPixmap (s_XDisplay, root, pixmap);
	}*/
}

const Display *cairo_dock_get_Xdisplay (void)
{
	return s_XDisplay;
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




void cairo_dock_set_strut_partial (int Xid, int left, int right, int top, int bottom, int left_start_y, int left_end_y, int right_start_y, int right_end_y, int top_start_x, int top_end_x, int bottom_start_x, int bottom_end_x)
{
	g_return_if_fail (Xid > 0);

	cd_debug ("%s (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", __func__, left, right, top, bottom, left_start_y, left_end_y, right_start_y, right_end_y, top_start_x, top_end_x, bottom_start_x, bottom_end_x);
	gulong iGeometryStrut[12] = {left, right, top, bottom, left_start_y, left_end_y, right_start_y, right_end_y, top_start_x, top_end_x, bottom_start_x, bottom_end_x};

	XChangeProperty (s_XDisplay,
		Xid,
		XInternAtom (s_XDisplay, "_NET_WM_STRUT", False),
		XA_CARDINAL, 32, PropModeReplace,
		(guchar *) iGeometryStrut, 12);
	
	Window root = DefaultRootWindow (s_XDisplay);
	cairo_dock_set_xwindow_timestamp (Xid, cairo_dock_get_xwindow_timestamp (root));
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
		cd_debug ("%s (%d) -> %d (%d,%d)", __func__, Xid, *pTypeBuffer, s_aNetWmWindowTypeNormal, s_aNetWmWindowTypeUtility);
		bIsUtility = (*pTypeBuffer == s_aNetWmWindowTypeUtility);
		XFree (pTypeBuffer);
	}
	else
		bIsUtility = FALSE;
	return bIsUtility;
}


void cairo_dock_set_one_icon_geometry_for_window_manager (int Xid, int iX, int iY, int iWidth, int iHeight)
{
	g_return_if_fail (Xid > 0);

	gulong iIconGeometry[4] = {iX, iY, iWidth, iHeight};

	XChangeProperty (s_XDisplay,
		Xid,
		XInternAtom (s_XDisplay, "_NET_WM_ICON_GEOMETRY", False),
		XA_CARDINAL, 32, PropModeReplace,
		(guchar *) iIconGeometry, 4);
}

gboolean cairo_dock_update_screen_geometry (void)
{
	Screen *XScreen = XDefaultScreenOfDisplay (s_XDisplay);
	//g_print ("screen : %dx%d ; root : %dx%d\n", WidthOfScreen (XScreen), HeightOfScreen (XScreen), width_return, height_return);
	if (WidthOfScreen (XScreen) != g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] || HeightOfScreen (XScreen) != g_iScreenHeight[CAIRO_DOCK_HORIZONTAL])
	{
		g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] = WidthOfScreen (XScreen);
		g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] = HeightOfScreen (XScreen);
		g_iScreenWidth[CAIRO_DOCK_VERTICAL] = g_iScreenHeight[CAIRO_DOCK_HORIZONTAL];
		g_iScreenHeight[CAIRO_DOCK_VERTICAL] = g_iScreenWidth[CAIRO_DOCK_HORIZONTAL];
		return TRUE;
	}
	else
		return FALSE;
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

void cairo_dock_get_nb_viewports (int *iNbViewportX, int *iNbViewportY)
{
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
		cd_debug ("pVirtualScreenSizeBuffer : %dx%d", pVirtualScreenSizeBuffer[0], pVirtualScreenSizeBuffer[1]);
		*iNbViewportX = pVirtualScreenSizeBuffer[0] / WidthOfScreen (XScreen);
		*iNbViewportY = pVirtualScreenSizeBuffer[1] / HeightOfScreen (XScreen);
		XFree (pVirtualScreenSizeBuffer);
	}
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

static void cairo_dock_move_current_viewport_to (int iDesktopViewportX, int iDesktopViewportY)
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
void cairo_dock_set_current_viewport (int iViewportNumberX, int iViewportNumberY)
{
	cairo_dock_move_current_viewport_to (iViewportNumberX * g_iScreenWidth[CAIRO_DOCK_HORIZONTAL], iViewportNumberY * g_iScreenHeight[CAIRO_DOCK_HORIZONTAL]);
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
