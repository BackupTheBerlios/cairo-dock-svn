
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
#ifdef HAVE_XEXTEND
#include <X11/extensions/Xcomposite.h>
//#include <X11/extensions/Xdamage.h>
#endif

#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-log.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-class-manager.h"
#include "cairo-dock-X-utilities.h"
#include "cairo-dock-application-factory.h"

extern double g_fAmplitude;
extern CairoDockLabelDescription g_iconTextDescription;
extern gboolean g_bTextAlwaysHorizontal;

extern int g_tIconAuthorizedWidth[CAIRO_DOCK_NB_TYPES];
extern int g_tIconAuthorizedHeight[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUniquePid;
extern gboolean g_bGroupAppliByClass;
extern gboolean g_bDemandsAttentionWithDialog;
extern gboolean g_bDemandsAttentionWithAnimation;
extern gboolean g_bOverWriteXIcons;
extern gboolean g_bShowThumbnail;

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
static Atom s_aNetWmHidden;
static Atom s_aNetWmFullScreen;
static Atom s_aNetWmMaximizedHoriz;
static Atom s_aNetWmMaximizedVert;


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
	s_aNetWmHidden = XInternAtom (s_XDisplay, "_NET_WM_STATE_HIDDEN", False);

	s_aNetWmPid = XInternAtom (s_XDisplay, "_NET_WM_PID", False);

	s_aNetWmWindowType = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE", False);
	s_aNetWmWindowTypeNormal = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	s_aNetWmWindowTypeDialog = XInternAtom (s_XDisplay, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	
	s_aNetWmName = XInternAtom (s_XDisplay, "_NET_WM_NAME", False);
	s_aUtf8String = XInternAtom (s_XDisplay, "UTF8_STRING", False);
	s_aWmName = XInternAtom (s_XDisplay, "WM_NAME", False);
	s_aString = XInternAtom (s_XDisplay, "STRING", False);

	s_aWmHints = XInternAtom (s_XDisplay, "WM_HINTS", False);
	
	s_aNetWmFullScreen = XInternAtom (s_XDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	s_aNetWmMaximizedHoriz = XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	s_aNetWmMaximizedVert = XInternAtom (s_XDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);
}

void cairo_dock_unregister_pid (Icon *icon)
{
	if (g_bUniquePid && CAIRO_DOCK_IS_APPLI (icon) && icon->iPid != 0)
	{
		g_hash_table_remove (s_hAppliTable, &icon->iPid);
	}
}

cairo_surface_t *cairo_dock_create_surface_from_xpixmap (Pixmap Xid, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS && Xid > 0, NULL);
	GdkPixbuf *pPixbuf = cairo_dock_get_pixbuf_from_pixmap (Xid, TRUE);
	if (pPixbuf == NULL)
	{
		cd_warning ("This pixmap is undefined. It can happen for exemple for a window that is in a minimized state when the dock is launching.");
		return NULL;
	}
	g_print ("window pixmap : %dx%d\n", gdk_pixbuf_get_width (pPixbuf), gdk_pixbuf_get_height (pPixbuf));
	cairo_surface_t *pSurface = cairo_dock_create_surface_from_pixbuf (pPixbuf,
		pSourceContext,
		fMaxScale,
		g_tIconAuthorizedWidth[CAIRO_DOCK_APPLI],
		g_tIconAuthorizedHeight[CAIRO_DOCK_APPLI],
		FALSE,
		fWidth,
		fHeight,
		NULL, NULL);
	g_object_unref (pPixbuf);
	return pSurface;
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
			cd_debug ("  aucun WMHints");
			return NULL;
		}
		//\__________________ On recupere les donnees dans un  pixbuf.
		GdkPixbuf *pIconPixbuf = NULL;
		if (pWMHints->flags & IconWindowHint)
		{
			Window XIconID = pWMHints->icon_window;
			cd_debug ("  pas de _NET_WM_ICON, mais une fenetre (ID:%d)", XIconID);
			Pixmap iPixmap = cairo_dock_get_window_background_pixmap (XIconID);
			pIconPixbuf = cairo_dock_get_pixbuf_from_pixmap (iPixmap, TRUE);  /// A valider ...
		}
		else if (pWMHints->flags & IconPixmapHint)
		{
			cd_debug ("  pas de _NET_WM_ICON, mais un pixmap");
			Pixmap XPixmapID = pWMHints->icon_pixmap;
			pIconPixbuf = cairo_dock_get_pixbuf_from_pixmap (XPixmapID, TRUE);

			//\____________________ On lui applique le masque de transparence s'il existe.
			if (pWMHints->flags & IconMaskHint)
			{
				Pixmap XPixmapMaskID = pWMHints->icon_mask;
				GdkPixbuf *pMaskPixbuf = cairo_dock_get_pixbuf_from_pixmap (XPixmapMaskID, FALSE);

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
				g_tIconAuthorizedWidth[CAIRO_DOCK_APPLI],
				g_tIconAuthorizedHeight[CAIRO_DOCK_APPLI],
				FALSE,
				fWidth,
				fHeight,
				NULL, NULL);

			g_object_unref (pIconPixbuf);
			return pNewSurface;
		}
		return NULL;
	}
}


CairoDock *cairo_dock_manage_appli_class (Icon *icon, CairoDock *pMainDock)
{
	cd_message ("%s (%s)", __func__, icon->acName);
	CairoDock *pParentDock = pMainDock;
	g_free (icon->cParentDockName);
	if (CAIRO_DOCK_IS_APPLI (icon) && g_bGroupAppliByClass && icon->cClass != NULL)
	{
		Icon *pSameClassIcon = cairo_dock_get_classmate (icon);
		//if (pSameClassIcon != NULL)
		//	g_print ("class-mate : %s (%s)\n", pSameClassIcon->acName, pSameClassIcon->cParentDockName);
		//pSameClassIcon = cairo_dock_get_icon_with_class (pMainDock->icons, icon->cClass);
		if (pSameClassIcon == NULL || pSameClassIcon == icon || pSameClassIcon->cParentDockName == NULL)
		{
			cd_message ("  classe %s encore vide", icon->cClass);
			icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);
			CairoDock *pClassDock = cairo_dock_search_dock_from_name (icon->cClass);
			if (pClassDock != NULL)
			{
				if (icon->pSubDock == NULL)
				{
					///icon->pSubDock = pClassDock;
					///cd_warning ("on lie de force le sous-dock de la classe %s a l'icone %s", icon->cClass, icon->acName);
				}
				else
					cd_warning ("le sous-dock de la classe %s est orphelin  (%s a deja un sous-dock) !", icon->cClass, icon->acName);
			}
		}
		else
		{
			icon->cParentDockName = g_strdup (icon->cClass);

			pParentDock = cairo_dock_search_dock_from_name (icon->cClass);
			if (pParentDock == NULL)  // alors il faut creer le sous-dock, et on decide de l'associer a pSameClassIcon.
			{
				cd_message ("  creation du dock pour la classe %s", icon->cClass);
				pParentDock = cairo_dock_create_subdock_for_class_appli (icon->cClass, pMainDock);
			}
			else
			{
				cd_message ("  sous-dock de la classe %s existant", icon->cClass);
			}
			if (pSameClassIcon->pSubDock != NULL && pSameClassIcon->pSubDock != pParentDock)
			{
				cd_warning ("Attention : this appli (%s) already has a subdock, but it is not the class's subdock => we'll add its classmate in the main dock");
				
			}
			else if (pSameClassIcon->pSubDock == NULL)
				pSameClassIcon->pSubDock = pParentDock;
		}
	}
	else
		icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);

	return pParentDock;
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
	gboolean bSkip = FALSE, bIsHidden = FALSE, bIsFullScreen = FALSE, bIsMaximized = FALSE;
	gulong *pXStateBuffer = NULL;
	iBufferNbElements = 0;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	if (iBufferNbElements > 0)
	{
		int i, iNbMaximizedDimensions = 0;
		for (i = 0; i < iBufferNbElements && ! bSkip; i ++)
		{
			if (pXStateBuffer[i] == s_aNetWmSkipTaskbar)
				bSkip = TRUE;
			else if (pXStateBuffer[i] == s_aNetWmHidden)
				bIsHidden = TRUE;
			else if (pXStateBuffer[i] == s_aNetWmMaximizedVert)
				iNbMaximizedDimensions ++;
			else if (pXStateBuffer[i] == s_aNetWmMaximizedHoriz)
				iNbMaximizedDimensions ++;
			else if (pXStateBuffer[i] == s_aNetWmFullScreen)
				bIsFullScreen = TRUE;
			//else if (pXStateBuffer[i] == s_aNetWmSkipPager)  // contestable ...
			//	bSkip = TRUE;
		}
		bIsMaximized = (iNbMaximizedDimensions == 2);
		//g_print (" -------- bSkip : %d\n",  bSkip);
		XFree (pXStateBuffer);
	}
	//else
	//	cd_message ("pas d'etat defini, donc on continue\n");
	if (bSkip)
	{
		//g_print ("  cette fenetre est timide\n");
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
				Window XMainAppliWindow = 0;
				XGetTransientForHint (s_XDisplay, Xid, &XMainAppliWindow);
				if (XMainAppliWindow != 0)
				{
					//g_print ("dialogue 'transient for' => on ignore\n");
					XFree (pTypeBuffer);
					return NULL;  // inutile de rajouter le PID ici, c'est le meme que la fenetre principale.
				}
				//g_print ("dialogue autorise\n");
			}
			else if (*pTypeBuffer != s_aNetWmWindowTypeNormal)
			{
				//g_print ("type indesirable\n");
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
		Window XMainAppliWindow = 0;
		XGetTransientForHint (s_XDisplay, Xid, &XMainAppliWindow);
		if (XMainAppliWindow != 0)
		{
			//g_print ("fenetre modale => on saute.\n");
			return NULL;  // meme remarque.
		}
		//else
		//	cd_message (" pas de type defini -> on suppose que son type est 'normal'\n");
	}
	
	
	//\__________________ On recupere son nom.
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmName, 0, G_MAXULONG, False, s_aUtf8String, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);
	if (iBufferNbElements == 0)
	{
		XGetWindowProperty (s_XDisplay, Xid, s_aWmName, 0, G_MAXULONG, False, s_aString, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);
	}
	if (iBufferNbElements == 0)
	{
		//g_print ("pas de nom, elle degage\n");
		if (g_bUniquePid)
			g_hash_table_insert (s_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.
		return NULL;
	}
	cd_message ("recuperation de '%s' (bIsHidden : %d)", pNameBuffer, bIsHidden);
	
	
	//\__________________ On recupere la classe.
	XClassHint *pClassHint = XAllocClassHint ();
	gchar *cClass = NULL;
	if (XGetClassHint (s_XDisplay, Xid, pClassHint) != 0)
	{
		cd_debug ("  res_name : %s(%x); res_class : %s(%x)", pClassHint->res_name, pClassHint->res_name, pClassHint->res_class, pClassHint->res_class);
		cClass = g_ascii_strdown (pClassHint->res_class, -1);  // on la passe en minuscule, car certaines applis ont la bonne idee de donner des classes avec une majuscule ou non suivant les fenetres. Il reste le cas des applis telles que Glade2 ('Glade' et 'Glade-2' ...)
		XFree (pClassHint->res_name);
		XFree (pClassHint->res_class);
		//g_print (".\n");
	}
	XFree (pClassHint);
	
	
	//\__________________ On cree, on remplit l'icone, et on l'enregistre, par contre elle sera inseree plus tard.
	Icon *icon = g_new0 (Icon, 1);
	icon->acName = g_strdup ((gchar *)pNameBuffer);
	if (g_bUniquePid)
		icon->iPid = *pPidBuffer;
	icon->Xid = Xid;
	icon->cClass = cClass;
	Icon * pLastAppli = cairo_dock_get_last_appli (pDock->icons);
	icon->fOrder = (pLastAppli != NULL ? pLastAppli->fOrder + 1 : 1);
	icon->iType = CAIRO_DOCK_APPLI;
	icon->bIsHidden = bIsHidden;
	icon->bIsMaximized = bIsMaximized;
	icon->bIsFullScreen = bIsFullScreen;
	
	cairo_dock_get_window_geometry (Xid,
		&icon->windowGeometry.x,
		&icon->windowGeometry.y,
		&icon->windowGeometry.width,
		&icon->windowGeometry.height);
	#ifdef HAVE_XEXTEND
	if (g_bShowThumbnail)
	{
		icon->iBackingPixmap = XCompositeNameWindowPixmap (s_XDisplay, Xid);
		/*icon->iDamageHandle = XDamageCreate (s_XDisplay, Xid, XDamageReportNonEmpty);  // XDamageReportRawRectangles
		g_print ("backing pixmap : %d ; iDamageHandle : %d\n", icon->iBackingPixmap, icon->iDamageHandle);*/
	}
	#endif
	
	cairo_dock_fill_icon_buffers_for_dock (icon, pSourceContext, pDock);
	
	if (g_bUniquePid)
		g_hash_table_insert (s_hAppliTable, pPidBuffer, icon);
	cairo_dock_register_appli (icon);
	XFree (pNameBuffer);
	
	cairo_dock_set_window_mask (Xid, PropertyChangeMask | StructureNotifyMask);

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

				pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
				cairo_dock_fill_one_text_buffer (icon, pCairoContext, &g_iconTextDescription, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock), pDock->bDirectionUp);
				cairo_destroy (pCairoContext);
			}
		}
	}
	else if (iState == PropertyNewValue && aProperty == s_aNetWmIcon)
	{
		//g_print ("%s change son icone\n", icon->acName);
		if (cairo_dock_class_is_using_xicon (icon->cClass) || ! g_bOverWriteXIcons)
		{
			pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
			icon->fWidth /= pDock->fRatio;
			icon->fHeight /= pDock->fRatio;
			cairo_dock_fill_one_icon_buffer (icon, pCairoContext, 1 + g_fAmplitude, pDock->bHorizontalDock, TRUE, pDock->bDirectionUp);
			icon->fWidth *= pDock->fRatio;
			icon->fHeight *= pDock->fRatio;
			cairo_destroy (pCairoContext);
			cairo_dock_redraw_my_icon (icon, CAIRO_CONTAINER (pDock));
		}
	}
	else if (aProperty == s_aWmHints)
	{
		XWMHints *pWMHints = XGetWMHints (s_XDisplay, icon->Xid);
		if (pWMHints != NULL)
		{
			if ((pWMHints->flags & XUrgencyHint) && (g_bDemandsAttentionWithDialog || g_bDemandsAttentionWithAnimation))
			{
				if (iState == PropertyNewValue)
				{
					cd_message ("%s vous interpelle !", icon->acName);
					if (g_bDemandsAttentionWithDialog)
						cairo_dock_show_temporary_dialog (icon->acName, icon, CAIRO_CONTAINER (pDock), 2000);
					if (g_bDemandsAttentionWithAnimation)
					{
						cairo_dock_arm_animation (icon, -1, 1e6);  // animation sans fin.
						cairo_dock_start_animation (icon, pDock);
					}
				}
				else if (iState == PropertyDelete)
				{
					cd_message ("%s arrette de vous interpeler.", icon->acName);
					if (g_bDemandsAttentionWithDialog)
						cairo_dock_remove_dialog_if_any (icon);
					if (g_bDemandsAttentionWithAnimation)
						cairo_dock_arm_animation (icon, 0, 0);  // arrete son animation quelqu'elle soit.
				}
				else
					cd_warning ("  etat du changement inconnu sur %s !", icon->acName);
			}
			if (iState == PropertyNewValue && (pWMHints->flags & (IconPixmapHint | IconMaskHint | IconWindowHint)))
			{
				//g_print ("%s change son icone\n", icon->acName);
				if (cairo_dock_class_is_using_xicon (icon->cClass) || ! g_bOverWriteXIcons)
				{
					pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
					icon->fWidth /= pDock->fRatio;
					icon->fHeight /= pDock->fRatio;
					cairo_dock_fill_one_icon_buffer (icon, pCairoContext, 1 + g_fAmplitude, pDock->bHorizontalDock, TRUE, pDock->bDirectionUp);
					icon->fWidth *= pDock->fRatio;
					icon->fHeight *= pDock->fRatio;
					cairo_destroy (pCairoContext);
					cairo_dock_redraw_my_icon (icon, CAIRO_CONTAINER (pDock));
				}
			}
		}
	}
}
