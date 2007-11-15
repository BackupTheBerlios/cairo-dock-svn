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

#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-application-factory.h"

extern double g_fAmplitude;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUniquePid;
extern gboolean g_bGroupAppliByClass;
extern gboolean g_bDemandsAttentionWithDialog;
extern gboolean g_bDemandsAttentionWithAnimation;

static GHashTable *s_hAppliTable = NULL;  // table des PID connus de cairo-dock (affichees ou non dans le dock).
static Display *s_XDisplay = NULL;
static Atom s_aNetWmIcon;
static Atom s_aNetWmState;
static Atom s_aNetWmSkipPager;
static Atom s_aNetWmSkipTaskbar;
static Atom s_aNetWmPid;
static Atom s_aNetWmWindowType;
static Atom s_aNetWmWindowTypeNormal;
static Atom s_aNetWmWindowTypeDialog;
static Atom s_aNetWmName;
static Atom s_aUtf8String;
static Atom s_aWmName;
static Atom s_aString;
static Atom s_aWmHints;


void cairo_dock_initialize_application_factory (Display *pXDisplay)
{
	s_XDisplay = pXDisplay;
	g_return_if_fail (s_XDisplay != NULL);
	
	s_hAppliTable = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	
	s_aNetWmIcon = XInternAtom (s_XDisplay, "_NET_WM_ICON", False);
	
	s_aNetWmState = XInternAtom (s_XDisplay, "_NET_WM_STATE", False);
	s_aNetWmSkipPager = XInternAtom (s_XDisplay, "_NET_WM_STATE_SKIP_PAGER", False);
	s_aNetWmSkipTaskbar = XInternAtom (s_XDisplay, "_NET_WM_STATE_SKIP_TASKBAR", False);
	
	s_aNetWmPid = XInternAtom (s_XDisplay, "_NET_WM_PID", False);
	
	s_aNetWmWindowType = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE", False);
	s_aNetWmWindowTypeNormal = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	s_aNetWmWindowTypeDialog = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	
	s_aNetWmName = XInternAtom (s_XDisplay, "_NET_WM_NAME", False);
	s_aUtf8String = XInternAtom (s_XDisplay, "UTF8_STRING", False);
	s_aWmName = XInternAtom (s_XDisplay, "WM_NAME", False);
	s_aString = XInternAtom (s_XDisplay, "STRING", False);
	
	s_aWmHints = XInternAtom (s_XDisplay, "WM_HINTS", False);
}

void cairo_dock_unregister_pid (Icon *icon)
{
	if (g_bUniquePid && CAIRO_DOCK_IS_APPLI (icon) && icon->iPid != 0)
	{
		g_hash_table_remove (s_hAppliTable, &icon->iPid);
	}
}


static GdkPixbuf *_cairo_dock_get_pixbuf_from_pixmap (int XPixmapID, gboolean bAddAlpha)  // cette fonction est inspiree par celle de libwnck.
{
	Window root;  // inutile.
	int x, y;  // inutile.
	guint border_width;  // inutile.
	guint iWidth, iHeight, iDepth;
	XGetGeometry (s_XDisplay,
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
	
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXIconBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmIcon, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXIconBuffer);
	
	if (iBufferNbElements > 2)
	{
		cairo_surface_t *pNewSurface = cairo_dock_create_surface_from_xicon_buffer (pXIconBuffer, iBufferNbElements, pSourceContext, fMaxScale, fWidth, fHeight);
		XFree (pXIconBuffer);
		return pNewSurface;
	}
	else  // sinon on tente avec l'icone eventuellement presente dans les WMHints.
	{
		XWMHints *pWMHints = XGetWMHints (s_XDisplay, Xid);
		if (pWMHints == NULL)
		{
			g_print ("  aucun WMHints\n");
			return NULL;
		}
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
	//g_print ("%s (%d)\n", __func__, Xid);
	guchar *pNameBuffer = NULL;
	gulong *pPidBuffer = NULL;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements;
	cairo_surface_t *pNewSurface = NULL;
	double fWidth, fHeight;
	
	//\__________________ On regarde si on doit l'afficher ou la sauter.
	gboolean bSkip = FALSE;
	gulong *pXStateBuffer = NULL;
	iBufferNbElements = 0;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	if (iBufferNbElements > 0)
	{
		int i;
		for (i = 0; i < iBufferNbElements && ! bSkip; i ++)
		{
			if (pXStateBuffer[i] == s_aNetWmSkipTaskbar)
				bSkip = TRUE;
			//else if (pXStateBuffer[i] == s_aNetWmSkipPager)  // contestable ...
			//	bSkip = TRUE;
		}
		//g_print (" -------- bSkip : %d\n",  bSkip);
		XFree (pXStateBuffer);
	}
	//else
	//	g_print ("pas d'etat defini, donc on continue\n");
	if (bSkip)
	{
		g_print ("  cette fenetre est timide\n");
		return NULL;
	}
	
	//\__________________ On recupere son PID si on est en mode "PID unique".
	if (g_bUniquePid)
	{
		iBufferNbElements = 0;
		XGetWindowProperty (s_XDisplay, Xid, s_aNetWmPid, 0, G_MAXULONG, False, XA_CARDINAL, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pPidBuffer);
		if (iBufferNbElements > 0)
		{
			//g_print (" +++ PID %d\n", *pPidBuffer);
			
			Icon *pIcon = g_hash_table_lookup (s_hAppliTable, pPidBuffer);
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
	gulong *pTypeBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmWindowType, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pTypeBuffer);
	if (iBufferNbElements != 0)
	{
		if (*pTypeBuffer != s_aNetWmWindowTypeNormal)
		{
			if (*pTypeBuffer == s_aNetWmWindowTypeDialog)
			{
				Window XMainAppliWindow;
				if (XGetTransientForHint (s_XDisplay, Xid, &XMainAppliWindow) != 0)
					XMainAppliWindow = 0;
				
				if (XMainAppliWindow != 0)
				{
					g_print ("dialogue modal => on ignore\n");
					XFree (pTypeBuffer);
					return NULL;  // inutile de rajouter le PID ici, c'est le meme que la fenetre principale.
				}
				g_print ("dialogue autorise\n");
			}
			else if (*pTypeBuffer != s_aNetWmWindowTypeNormal)
			{
				g_print ("type indesirable\n");
				XFree (pTypeBuffer);
				if (g_bUniquePid)
					g_hash_table_insert (s_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.
				return NULL;
			}
		}
		XFree (pTypeBuffer);
	}
	else
	{
		Window XMainAppliWindow;
		if (XGetTransientForHint (s_XDisplay, Xid, &XMainAppliWindow) != 0)
			XMainAppliWindow = 0;
		if (XMainAppliWindow != 0)
		{
			g_print ("fenetre modale => on saute.\n");
			return NULL;  // meme remarque.
		}
		//else
		//	g_print (" pas de type defini -> on suppose que son type est 'normal'\n");
	}
	
	//\__________________ On recupere son nom.
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmName, 0, G_MAXULONG, False, s_aUtf8String, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);
	if (iBufferNbElements == 0)
	{
		XGetWindowProperty (s_XDisplay, Xid, s_aWmName, 0, G_MAXULONG, False, s_aString, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);
	}
	if (iBufferNbElements == 0)
	{
		g_print ("pas de nom\n");
		if (g_bUniquePid)
			g_hash_table_insert (s_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.
		return NULL;
	}
	g_print ("recuperation de '%s'\n", pNameBuffer);
	
	//\__________________ On recupere son icone.
	/**pNewSurface = cairo_dock_create_surface_from_xwindow (Xid, pSourceContext, 1 + g_fAmplitude, &fWidth, &fHeight);
	if (pNewSurface == NULL)
	{
		g_print ("pas d'icone\n");
		XFree (pNameBuffer);
		if (g_bUniquePid)
			g_hash_table_insert (s_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.
		return NULL;
	}*/
	
	//\__________________ On cree et on remplit l'icone, et on l'insere apres les autres applis.
	Icon *icon = g_new0 (Icon, 1);
	icon->acName = g_strdup ((gchar *)pNameBuffer);
	if (g_bUniquePid)
		icon->iPid = *pPidBuffer;
	icon->Xid = Xid;
	Icon * pLastAppli = cairo_dock_get_last_appli (pDock->icons);
	icon->fOrder = (pLastAppli != NULL ? pLastAppli->fOrder + 1 : 1);
	icon->iType = CAIRO_DOCK_APPLI;
	
	cairo_dock_fill_one_icon_buffer (icon, pSourceContext, 1 + g_fAmplitude, pDock->bHorizontalDock);
	/**icon->fWidth = fWidth;
	icon->fHeight = fHeight;
	icon->pIconBuffer = pNewSurface;*/
	cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
	
	if (g_bUniquePid)
		g_hash_table_insert (s_hAppliTable, pPidBuffer, icon);
	cairo_dock_register_appli (icon);
	XFree (pNameBuffer);
	
	//\__________________ On regarde si il faut la grouper avec une autre.
	XClassHint *pClassHint = XAllocClassHint ();
	if (XGetClassHint (s_XDisplay, Xid, pClassHint) != 0)
	{
		g_print ("  res_name : %s(%x); res_class : %s(%x)", pClassHint->res_name, pClassHint->res_name, pClassHint->res_class, pClassHint->res_class);
		icon->cClass = g_ascii_strdown (pClassHint->res_class, -1);  // on la passe en minuscule, car certaines applis ont la bonne idee de donner des classes avec une majuscule ou non suivant les fenetres. Il reste le cas des aplis telles que Glade2 ('Glade' et 'Glade-2' ...)
		XFree (pClassHint->res_name);
		XFree (pClassHint->res_class);
		g_print (".\n");
	}
	XFree (pClassHint);
	
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
	
	cairo_dock_set_window_mask (Xid, PropertyChangeMask);
	
	return icon;
}


void cairo_dock_Xproperty_changed (Icon *icon, Atom aProperty, int iState, CairoDock *pDock)
{
	//g_print ("%s (%s, %s)\n", __func__, icon->acName, gdk_x11_get_xatom_name (aProperty));
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements=0;
	
	cairo_t* pCairoContext;
	
	if (iState == PropertyNewValue && (aProperty == s_aNetWmName || aProperty == s_aWmName))
	{
		//g_print ("chgt de nom (%d)\n", aProperty);
		guchar *pNameBuffer = NULL;
		XGetWindowProperty (s_XDisplay, icon->Xid, s_aNetWmName, 0, G_MAXULONG, False, s_aUtf8String, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);  // on cherche en priorite le nom en UTF8, car on est notifie des 2, mais il vaut mieux eviter le WM_NAME qui, ne l'etant pas, contient des caracteres bizarres qu'on ne peut pas convertir avec g_locale_to_utf8, puisque notre locale _est_ UTF8.
		if (iBufferNbElements == 0 && aProperty == s_aWmName)
			XGetWindowProperty (s_XDisplay, icon->Xid, aProperty, 0, G_MAXULONG, False, s_aString, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);
		if (iBufferNbElements > 0)
		{
			if (icon->acName == NULL || strcmp (icon->acName, (gchar *)pNameBuffer) != 0)
			{
				g_free (icon->acName);
				icon->acName = g_strdup ((gchar *)pNameBuffer);
				XFree (pNameBuffer);
				
				pCairoContext = cairo_dock_create_context_from_window (pDock);
				cairo_dock_fill_one_text_buffer (icon, pCairoContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
				cairo_destroy (pCairoContext);
			}
		}
	}
	else if (iState == PropertyNewValue && aProperty == s_aNetWmIcon)  // on prefere garder l'ancienne icone que de l'effacer.
	{
		//g_print ("%s change son icone\n", icon->acName);
		pCairoContext = cairo_dock_create_context_from_window (pDock);
		cairo_dock_fill_one_icon_buffer (icon, pCairoContext, 1 + g_fAmplitude, pDock->bHorizontalDock);
		cairo_destroy (pCairoContext);
		cairo_dock_redraw_my_icon (icon, pDock);
	}
	else if (aProperty == s_aWmHints)
	{
		XWMHints *pWMHints = XGetWMHints (s_XDisplay, icon->Xid);
		if (pWMHints != NULL)
		{
			if (pWMHints->flags & XUrgencyHint && (g_bDemandsAttentionWithDialog || g_bDemandsAttentionWithAnimation))
			{
				if (iState == PropertyNewValue)
				{
					g_print ("%s vous interpelle !\n", icon->acName);
					if (g_bDemandsAttentionWithDialog)
						cairo_dock_show_temporary_dialog (icon->acName, icon, pDock, 2000);
					if (g_bDemandsAttentionWithAnimation)
					{
						cairo_dock_arm_animation (icon, -1, 1e6);  // animation sans fin.
						cairo_dock_start_animation (icon, pDock);
					}
				}
				else if (iState == PropertyDelete)
				{
					g_print ("%s arrette de vous interpeler.\n", icon->acName);
					if (g_bDemandsAttentionWithDialog)
						cairo_dock_remove_dialog_if_any (icon);
					if (g_bDemandsAttentionWithAnimation)
						cairo_dock_arm_animation (icon, -1, 0);  // arrete son animation quelqu'elle soit.
				}
				else
					g_print ("  etat du changement inconnu !\n");
			}
			if (iState == PropertyNewValue && (pWMHints->flags & (IconPixmapHint | IconMaskHint | IconWindowHint)))
			{
				//g_print ("%s change son icone\n", icon->acName);
				pCairoContext = cairo_dock_create_context_from_window (pDock);
				cairo_dock_fill_one_icon_buffer (icon, pCairoContext, 1 + g_fAmplitude, pDock->bHorizontalDock);
				cairo_destroy (pCairoContext);
				cairo_dock_redraw_my_icon (icon, pDock);
			}
		}
	}
}
