/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <cairo.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <gdk/gdkx.h>

#ifdef HAVE_GLITZ
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-application-factory.h"


extern double g_fAmplitude;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUniquePid;
extern gboolean g_bGroupAppliByClass;
extern Display *g_XDisplay;
extern GHashTable *g_hAppliTable;
extern GHashTable *g_hXWindowTable;

extern gboolean g_bUseGlitz;


static GdkPixbuf *_cairo_dock_get_pixbuf_from_pixmap (int XPixmapID, gboolean bAddAlpha)  // cette fonction est inspiree par celle de libwnck.
{
	Window root;  // inutile.
	int x, y;  // inutile.
	guint border_width;  // inutile.
	guint iWidth, iHeight, iDepth;
	XGetGeometry (g_XDisplay,
		XPixmapID, &root, &x, &y,
		&iWidth, &iHeight, &border_width, &iDepth);
	g_print ("%s (%d) : %dx%dx%d pixels (%d;%d)\n", __func__, XPixmapID, iWidth, iHeight, iDepth, x, y);
	
	//\__________________ On recupere le drawable associe.
	GdkDrawable *pGdkDrawable = gdk_xid_table_lookup (XPixmapID);
	if (pGdkDrawable)
		g_object_ref (G_OBJECT (pGdkDrawable));
	else
		pGdkDrawable = gdk_pixmap_foreign_new (XPixmapID);
	
	//\__________________ On recupere la colormap.
	GdkColormap* pColormap = gdk_drawable_get_colormap (pGdkDrawable);
	if (pColormap == NULL && gdk_drawable_get_depth (pGdkDrawable) > 1)  // pour les bitmaps, on laisse la colormap a NULL, ils n'en ont pas besoin.
	{
		GdkScreen* pScreen = gdk_drawable_get_screen (GDK_DRAWABLE (pGdkDrawable));
		pColormap = gdk_screen_get_system_colormap (pScreen);  // au pire on a un colormap nul.
	}
	
	//\__________________ On recupere le buffer dans un GdkPixbuf.
	GdkPixbuf *pIconPixbuf = gdk_pixbuf_get_from_drawable (NULL,
		pGdkDrawable,
		pColormap,
		0,
		0,
		0,
		0,
		iWidth,
		iHeight);
	g_object_unref (G_OBJECT (pGdkDrawable));
	g_return_val_if_fail (pIconPixbuf != NULL, NULL);
	
	//\__________________ On lui ajoute un canal alpha si necessaire.
	if (! gdk_pixbuf_get_has_alpha (pIconPixbuf) && bAddAlpha)
	{
		GdkPixbuf *tmp_pixbuf = gdk_pixbuf_add_alpha (pIconPixbuf, TRUE, 255, 255, 255);
		g_object_unref (pIconPixbuf);
		pIconPixbuf = tmp_pixbuf;
	}
	return pIconPixbuf;
}

cairo_surface_t *cairo_dock_create_surface_from_xwindow (Window Xid, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	
	Atom aNetWmIcon = XInternAtom (g_XDisplay, "_NET_WM_ICON", False);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXIconBuffer = NULL;
	XGetWindowProperty (g_XDisplay, Xid, aNetWmIcon, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXIconBuffer);
	
	if (iBufferNbElements > 2)
	{
		cairo_surface_t *pNewSurface = cairo_dock_create_surface_from_xicon_buffer (pXIconBuffer, iBufferNbElements, pSourceContext, fMaxScale, fWidth, fHeight);
		XFree (pXIconBuffer);
		return pNewSurface;
	}
	else  // sinon on tente avec l'icone eventuellement presente dans les WMHints.
	{
		XWMHints *pWMHints = XGetWMHints (g_XDisplay, Xid);
		if (pWMHints == NULL)
			return NULL;
		
		//\__________________ On recupere les donnees dans un  pixbuf.
		GdkPixbuf *pIconPixbuf = NULL;
		if (pWMHints->flags & IconWindowHint)
		{
			g_print ("  pas de _NET_WM_ICON, mais une fenetre\n");
			Window XIconID = pWMHints->icon_window;
			pIconPixbuf = _cairo_dock_get_pixbuf_from_pixmap (XIconID, TRUE);  // pas teste.
		}
		else if (pWMHints->flags & IconPixmapHint)
		{
			g_print ("  pas de _NET_WM_ICON, mais un pixmap\n");
			Pixmap XPixmapID = pWMHints->icon_pixmap;
			pIconPixbuf = _cairo_dock_get_pixbuf_from_pixmap (XPixmapID, TRUE);
			
			//\____________________ On lui applique le masque de transparence s'il existe.
			if (pWMHints->flags & IconMaskHint)
			{
				Pixmap XPixmapMaskID = pWMHints->icon_mask;
				GdkPixbuf *pMaskPixbuf = _cairo_dock_get_pixbuf_from_pixmap (XPixmapMaskID, FALSE);
				
				int iNbChannels = gdk_pixbuf_get_n_channels (pIconPixbuf);
				int iRowstride = gdk_pixbuf_get_rowstride (pIconPixbuf);
				guchar *p, *pixels = gdk_pixbuf_get_pixels (pIconPixbuf);
				
				int iNbChannelsMask = gdk_pixbuf_get_n_channels (pMaskPixbuf);
				int iRowstrideMask = gdk_pixbuf_get_rowstride (pMaskPixbuf);
				guchar *q, *pixelsMask = gdk_pixbuf_get_pixels (pMaskPixbuf);
				
				int w = MIN (gdk_pixbuf_get_width (pIconPixbuf), gdk_pixbuf_get_width (pMaskPixbuf));
				int h = MIN (gdk_pixbuf_get_height (pIconPixbuf), gdk_pixbuf_get_height (pMaskPixbuf));
				int x, y;
				for (y = 0; y < h; y ++)
				{
					for (x = 0; x < w; x ++)
					{
						p = pixels + y * iRowstride + x * iNbChannels;
						q = pixelsMask + y * iRowstrideMask + x * iNbChannelsMask;
						if (q[0] == 0)
							p[3] = 0;
						else
							p[3] = 255;
					}
				}
				
				g_object_unref (pMaskPixbuf);
			}
		}
		XFree (pWMHints);
		
		//\____________________ On cree la surface.
		if (pIconPixbuf != NULL)
		{
			cairo_surface_t *pNewSurface = cairo_dock_create_surface_from_pixbuf (pIconPixbuf,
				pSourceContext,
				fMaxScale,
				TRUE,
				g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI],
				g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI],
				g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI],
				g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI],
				fWidth,
				fHeight);
			
			g_object_unref (pIconPixbuf);
			return pNewSurface;
		}
		return NULL;
	}
}



Icon * cairo_dock_create_icon_from_xwindow (cairo_t *pSourceContext, Window Xid, CairoDock *pDock)
{
	g_print ("%s (%d)\n", __func__, Xid);
	guchar *pNameBuffer;
	gulong *pPidBuffer = NULL;
	double fWidth, fHeight;
	cairo_surface_t *pNewSurface = NULL;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	
	//\__________________ On regarde si on doit l'afficher ou la sauter.
	Atom aNetWmMState = XInternAtom (g_XDisplay, "_NET_WM_STATE", False);
	gulong *pXStateBuffer = NULL;
	iBufferNbElements = 0;
	XGetWindowProperty (g_XDisplay, Xid, aNetWmMState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	gboolean bSkip = FALSE;
	if (iBufferNbElements > 0)
	{
		int i;
		Atom aNetWmSkipPager = XInternAtom (g_XDisplay, "_NET_WM_STATE_SKIP_PAGER", False);
		Atom aNetWmSkipTaskbar = XInternAtom (g_XDisplay, "_NET_WM_STATE_SKIP_TASKBAR", False);
		for (i = 0; i < iBufferNbElements && ! bSkip; i ++)
		{
			if (pXStateBuffer[i] == aNetWmSkipPager)
				bSkip = TRUE;
			else if (pXStateBuffer[i] == aNetWmSkipTaskbar)
				bSkip = TRUE;
		}
		//g_print (" -------- bSkip : %d\n",  bSkip);
		XFree (pXStateBuffer);
	}
	//else
	//	g_print ("pas d'etat defini, donc on continue\n");
	if (bSkip)
		return NULL;
	
	//\__________________ On recupere son PID si on est en mode "PID unique".
	if (g_bUniquePid)
	{
		Atom aNetWmPid = XInternAtom (g_XDisplay, "_NET_WM_PID", False);
		iBufferNbElements = 0;
		XGetWindowProperty (g_XDisplay, Xid, aNetWmPid, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pPidBuffer);
		if (iBufferNbElements > 0)
		{
			//g_print (" +++ PID %d\n", *pPidBuffer);
			
			Icon *pIcon = g_hash_table_lookup (g_hAppliTable, pPidBuffer);
			if (pIcon != NULL)  // si c'est une fenetre d'une appli deja referencee, on ne rajoute pas d'icones.
			{
				XFree (pPidBuffer);
				return NULL;
			}
		}
		else
		{
			//g_print ("pas de PID defini -> elle degage\n");
			return NULL;
		}
	}
	
	//\__________________ On regarde son type.
	gulong *pTypeBuffer;
	Atom aNetWmWindowType = XInternAtom (g_XDisplay, "_NET_WM_WINDOW_TYPE", False);
	XGetWindowProperty (g_XDisplay, Xid, aNetWmWindowType, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pTypeBuffer);
	if (iBufferNbElements != 0)
	{
		if (*pTypeBuffer != XInternAtom (g_XDisplay, "_NET_WM_WINDOW_TYPE_NORMAL", False))
		{
			XFree (pTypeBuffer);
			if (g_bUniquePid)
				g_hash_table_insert (g_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.
			return NULL;
		}
		XFree (pTypeBuffer);
	}
	//else
	//	g_print (" pas de type defini -> on suppose que son type est 'normal'\n");
	
	//\__________________ On recupere son nom.
	Atom aNetWmName = XInternAtom (g_XDisplay, "_NET_WM_NAME", False);
	Atom aUtf8String = XInternAtom (g_XDisplay, "UTF8_STRING", False);
	XGetWindowProperty (g_XDisplay, Xid, aNetWmName, 0, G_MAXULONG, False, aUtf8String, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);
	if (iBufferNbElements == 0)
	{
		Atom aWmName = XInternAtom (g_XDisplay, "WM_NAME", False);
		Atom aString = XInternAtom (g_XDisplay, "STRING", False);
		XGetWindowProperty (g_XDisplay, Xid, aWmName, 0, G_MAXULONG, False, aString, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);
	}
	if (iBufferNbElements == 0)
	{
		if (g_bUniquePid)
			g_hash_table_insert (g_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.
		return NULL;
	}
	//g_print ("recuperation de %s\n", pNameBuffer);
	
	//\__________________ On recupere son icone.
	pNewSurface = cairo_dock_create_surface_from_xwindow (Xid, pSourceContext, 1 + g_fAmplitude, &fWidth, &fHeight);
	if (pNewSurface == NULL)
	{
		//g_print ("pas d'icone\n");
		XFree (pNameBuffer);
		if (g_bUniquePid)
			g_hash_table_insert (g_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.
		return NULL;
	}
	
	//\__________________ On cree et on remplit l'icone, et on l'insere apres les autres applis.
	Icon *icon = g_new0 (Icon, 1);
	icon->acName = g_strdup ((gchar *)pNameBuffer);
	if (g_bUniquePid)
		icon->iPid = *pPidBuffer;
	icon->Xid = Xid;
	Icon * pLastAppli = cairo_dock_get_last_appli (pDock->icons);
	icon->fOrder = (pLastAppli != NULL ? pLastAppli->fOrder + 1 : 1);
	icon->iType = CAIRO_DOCK_APPLI;
	
	icon->fWidth = fWidth;
	icon->fHeight = fHeight;
	icon->pIconBuffer = pNewSurface;
	icon->bIsMapped = TRUE;  // si elle n'est en fait pas visible, le 2eme UnmapNotify sera juste ignore.
	cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
	
	if (g_bUniquePid)
		g_hash_table_insert (g_hAppliTable, pPidBuffer, icon);
	Window *pXid = g_new (Window, 1);
	*pXid = Xid;
	g_hash_table_insert (g_hXWindowTable, pXid, icon);
	XFree (pNameBuffer);
	
	//\__________________ On regarde si il faut la grouper avec une autre.
	XClassHint class_hint;
	if (XGetClassHint (g_XDisplay, Xid, &class_hint) != 0)
	{
		g_print ("  res_name : %s; res_class : %s\n", class_hint.res_name, class_hint.res_class);
		icon->cClass = g_ascii_strdown (class_hint.res_class, -1);  // on la passe en minuscule, car certaines applis ont la bonne idee de donner des classes avec une majuscule ou non suivant les fenetres. Il reste le cas des aplis telles que Glade2 ('Glade' et 'Glade-2' ...)
		XFree (class_hint.res_name);
		XFree (class_hint.res_class);
	}
	if (g_bGroupAppliByClass && icon->cClass != NULL)
	{
		Icon *pSameClassIcon = cairo_dock_get_icon_with_class (pDock->icons, icon->cClass);
		if (pSameClassIcon == NULL)
		{
			icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);
		}
		else
		{
			icon->cParentDockName = g_strdup (icon->cClass);
			
			if (cairo_dock_search_dock_from_name (icon->cClass) == NULL)  // alors il faut creer le sous-dock, et on decide de l'associer a pSameClassIcon.
			{
				g_print ("  creation du dock pour la classe %s\n", icon->cClass);
				pSameClassIcon->pSubDock = cairo_dock_create_subdock_for_class_appli (icon->cClass);
			}
		}
	}
	else
		icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);
	
	return icon;
}
