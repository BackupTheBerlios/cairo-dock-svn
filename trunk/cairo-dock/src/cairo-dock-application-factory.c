
/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

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
#include "cairo-dock-internal-system.h"
#include "cairo-dock-internal-taskbar.h"
#include "cairo-dock-internal-labels.h"
#include "cairo-dock-internal-icons.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-container.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dock-facility.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-application-factory.h"

extern CairoDock *g_pMainDock;

static GHashTable *s_hAppliTable = NULL;  // table des PID connus de cairo-dock (affichees ou non dans le dock).
static Display *s_XDisplay = NULL;
static Atom s_aNetWmIcon;
static Atom s_aNetWmState;
static Atom s_aNetWmSkipPager;
static Atom s_aNetWmSkipTaskbar;
///static Atom s_aNetWmPid;
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
static Atom s_aNetWmDemandsAttention;


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

	///s_aNetWmPid = XInternAtom (s_XDisplay, "_NET_WM_PID", False);

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
	s_aNetWmDemandsAttention = XInternAtom (s_XDisplay, "_NET_WM_STATE_DEMANDS_ATTENTION", False);
}

void cairo_dock_unregister_pid (Icon *icon)
{
	/**if (myTaskBar.bUniquePid && CAIRO_DOCK_IS_APPLI (icon) && icon->iPid != 0)
	{
		g_hash_table_remove (s_hAppliTable, &icon->iPid);
	}*/
}

cairo_surface_t *cairo_dock_create_surface_from_xpixmap (Pixmap Xid, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS && Xid > 0, NULL);
	GdkPixbuf *pPixbuf = cairo_dock_get_pixbuf_from_pixmap (Xid, TRUE);
	if (pPixbuf == NULL)
	{
		cd_warning ("Can't have thumbnail for a window that is minimized when the dock is launched.");
		return NULL;
	}
	cd_debug ("window pixmap : %dx%d", gdk_pixbuf_get_width (pPixbuf), gdk_pixbuf_get_height (pPixbuf));
	cairo_surface_t *pSurface = cairo_dock_create_surface_from_pixbuf (pPixbuf,
		pSourceContext,
		fMaxScale,
		myIcons.tIconAuthorizedWidth[CAIRO_DOCK_APPLI],
		myIcons.tIconAuthorizedHeight[CAIRO_DOCK_APPLI],
		CAIRO_DOCK_KEEP_RATIO | CAIRO_DOCK_FILL_SPACE,  // on conserve le ratio de la fenetre, tout en gardant la taille habituelle des icones d'appli.
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
				myIcons.tIconAuthorizedWidth[CAIRO_DOCK_APPLI],
				myIcons.tIconAuthorizedHeight[CAIRO_DOCK_APPLI],
				CAIRO_DOCK_KEEP_RATIO | CAIRO_DOCK_FILL_SPACE,
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
	if (CAIRO_DOCK_IS_APPLI (icon) && myTaskBar.bGroupAppliByClass && icon->cClass != NULL && ! cairo_dock_class_is_expanded (icon->cClass))
	{
		Icon *pSameClassIcon = cairo_dock_get_classmate (icon);  // un inhibiteur dans un dock OU une appli de meme classe dans le main dock.
		if (pSameClassIcon == NULL)  // aucun classmate => elle va dans le main dock.
		{
			cd_message ("  classe %s encore vide", icon->cClass);
			pParentDock = cairo_dock_search_dock_from_name (icon->cClass);
			if (pParentDock == NULL)
			{
				pParentDock = pMainDock;
				icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);
			}
			else
			{
				icon->cParentDockName = g_strdup (icon->cClass);
			}
		}
		else  // on la met dans le sous-dock de sa classe.
		{
			icon->cParentDockName = g_strdup (icon->cClass);

			//\____________ On cree ce sous-dock si necessaire.
			pParentDock = cairo_dock_search_dock_from_name (icon->cClass);
			if (pParentDock == NULL)  // alors il faut creer le sous-dock, qu'on associera soit a pSameClassIcon soit a un fake.
			{
				cd_message ("  creation du dock pour la classe %s", icon->cClass);
				pParentDock = cairo_dock_create_subdock_from_scratch (NULL, icon->cClass, pMainDock);
			}
			else
				cd_message ("  sous-dock de la classe %s existant", icon->cClass);
			
			if (CAIRO_DOCK_IS_LAUNCHER (pSameClassIcon) || CAIRO_DOCK_IS_APPLET (pSameClassIcon))  // c'est un inhibiteur.
			{
				if (pSameClassIcon->Xid != 0)  // actuellement l'inhibiteur inhibe 1 seule appli.
				{
					cd_debug ("actuellement l'inhibiteur inhibe 1 seule appli");
					Icon *pInhibatedIcon = cairo_dock_get_icon_with_Xid (pSameClassIcon->Xid);
					pSameClassIcon->Xid = 0;  // on lui laisse par contre l'indicateur.
					if (pSameClassIcon->pSubDock == NULL)
					{
						if (pSameClassIcon->cInitialName != NULL)
						{
							CairoDock *pSameClassDock = cairo_dock_search_dock_from_name (pSameClassIcon->cParentDockName);
							if (pSameClassDock != NULL)
							{
								cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pSameClassDock));
								cairo_dock_set_icon_name (pCairoContext, pSameClassIcon->cInitialName, pSameClassIcon, CAIRO_CONTAINER (pSameClassDock));  // on lui remet son nom de lanceur.
								cairo_destroy (pCairoContext);
							}
						}
						pSameClassIcon->pSubDock = pParentDock;
						CairoDock *pRootDock = cairo_dock_search_dock_from_name (pSameClassIcon->cParentDockName);
						if (pRootDock != NULL)
							cairo_dock_redraw_icon (pSameClassIcon, CAIRO_CONTAINER (pRootDock));  // on la redessine car elle prend l'indicateur de classe.
					}
					else if (pSameClassIcon->pSubDock != pParentDock)
						cd_warning ("this launcher (%s) already has a subdock, but it's not the class's subdock !", pSameClassIcon->acName);
					if (pInhibatedIcon != NULL)
					{
						cd_debug (" on insere %s dans le dock de la classe", pInhibatedIcon->acName);
						g_free (pInhibatedIcon->cParentDockName);
						pInhibatedIcon->cParentDockName = g_strdup (icon->cClass);
						cairo_dock_insert_icon_in_dock_full (pInhibatedIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, ! CAIRO_DOCK_INSERT_SEPARATOR, NULL);
					}
				}
				else if (pSameClassIcon->pSubDock != pParentDock)
					cd_warning ("this inhibator doesn't hold the class dock !");
			}
			else  // c'est donc une appli du main dock.
			{
				//\______________ On cree une icone de paille.
				cd_debug (" on cree un fake...");
				CairoDock *pClassMateParentDock = cairo_dock_search_dock_from_name (pSameClassIcon->cParentDockName);  // c'est en fait le main dock.
				Icon *pFakeClassIcon = g_new0 (Icon, 1);
				pFakeClassIcon->acName = g_strdup (pSameClassIcon->cClass);
				pFakeClassIcon->cClass = g_strdup (pSameClassIcon->cClass);
				pFakeClassIcon->iType = pSameClassIcon->iType;
				pFakeClassIcon->fOrder = pSameClassIcon->fOrder;
				pFakeClassIcon->cParentDockName = g_strdup (pSameClassIcon->cParentDockName);
				pFakeClassIcon->fWidth = pSameClassIcon->fWidth / pClassMateParentDock->fRatio;
				pFakeClassIcon->fHeight = pSameClassIcon->fHeight / pClassMateParentDock->fRatio;
				pFakeClassIcon->fXMax = pSameClassIcon->fXMax;
				pFakeClassIcon->fXMin = pSameClassIcon->fXMin;
				pFakeClassIcon->fXAtRest = pSameClassIcon->fXAtRest;
				pFakeClassIcon->pSubDock = pParentDock;  // grace a cela ce sera un lanceur.
				
				//\______________ On la charge.
				cairo_dock_load_one_icon_from_scratch (pFakeClassIcon, CAIRO_CONTAINER (pClassMateParentDock));
				
				//\______________ On detache le classmate, on le place dans le sous-dock, et on lui substitue le faux.
				cd_debug (" on detache %s pour la passer dans le sous-dock de sa classe", pSameClassIcon->acName);
				cairo_dock_detach_icon_from_dock (pSameClassIcon, pClassMateParentDock, FALSE);
				g_free (pSameClassIcon->cParentDockName);
				pSameClassIcon->cParentDockName = g_strdup (pSameClassIcon->cClass);
				cairo_dock_insert_icon_in_dock_full (pSameClassIcon, pParentDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, ! CAIRO_DOCK_INSERT_SEPARATOR, NULL);
				
				cd_debug (" on lui substitue le fake");
				cairo_dock_insert_icon_in_dock_full (pFakeClassIcon, pClassMateParentDock,  CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, ! CAIRO_DOCK_INSERT_SEPARATOR, NULL);
				cairo_dock_calculate_dock_icons (pClassMateParentDock);
				cairo_dock_redraw_icon (pFakeClassIcon, CAIRO_CONTAINER (pClassMateParentDock));
			}
		}
	}
	else
		icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);

	return pParentDock;
}

static Window _cairo_dock_get_parent_window (Window Xid)
{
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	Window *pXBuffer = NULL;
	XGetWindowProperty (s_XDisplay, Xid, XInternAtom (s_XDisplay, "WM_TRANSIENT_FOR", False), 0, G_MAXULONG, False, XA_WINDOW, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXBuffer);

	Window xParentWindow = (iBufferNbElements > 0 && pXBuffer != NULL ? pXBuffer[0] : 0);
	if (pXBuffer != NULL)
		XFree (pXBuffer);
	return xParentWindow;
}
Icon * cairo_dock_create_icon_from_xwindow (cairo_t *pSourceContext, Window Xid, CairoDock *pDock)
{
	//g_print ("%s (%d)\n", __func__, Xid);
	guchar *pNameBuffer = NULL;
	///gulong *pPidBuffer = NULL;
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements;
	cairo_surface_t *pNewSurface = NULL;
	double fWidth, fHeight;

	//\__________________ On regarde si on doit l'afficher ou la sauter.
	gboolean bSkip = FALSE, bIsHidden = FALSE, bIsFullScreen = FALSE, bIsMaximized = FALSE, bDemandsAttention = FALSE;
	gulong *pXStateBuffer = NULL;
	iBufferNbElements = 0;
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmState, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	if (iBufferNbElements > 0)
	{
		guint i, iNbMaximizedDimensions = 0;
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
			else if (pXStateBuffer[i] == s_aNetWmDemandsAttention)
				bDemandsAttention = TRUE;
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
		cd_debug ("  cette fenetre est timide");
		return NULL;
	}

	//\__________________ On recupere son PID si on est en mode "PID unique".
	/**if (myTaskBar.bUniquePid)
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
	}*/

	//\__________________ On regarde son type.
	gulong *pTypeBuffer = NULL;
	cd_debug (" + nouvelle icone d'appli (%d)", Xid);
	XGetWindowProperty (s_XDisplay, Xid, s_aNetWmWindowType, 0, G_MAXULONG, False, XA_ATOM, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pTypeBuffer);
	if (iBufferNbElements != 0)
	{
		if (*pTypeBuffer != s_aNetWmWindowTypeNormal)
		{
			if (*pTypeBuffer == s_aNetWmWindowTypeDialog)
			{
				/*Window iPropWindow;
				XGetTransientForHint (s_XDisplay, Xid, &iPropWindow);
				g_print ("%s\n", gdk_x11_get_xatom_name (iPropWindow));*/
				Window XMainAppliWindow = _cairo_dock_get_parent_window (Xid);
				if (XMainAppliWindow != 0)
				{
					cd_debug ("  dialogue 'transient for' => on ignore");
					if (bDemandsAttention && (myTaskBar.bDemandsAttentionWithDialog || myTaskBar.cAnimationOnDemandsAttention))
					{
						Icon *pParentIcon = cairo_dock_get_icon_with_Xid (XMainAppliWindow);
						if (pParentIcon != NULL)
						{
							cd_debug ("%s requiert votre attention indirectement !", pParentIcon->acName);
							cairo_dock_appli_demands_attention (pParentIcon);
						}
						else
							cd_debug ("ce dialogue est bien bruyant ! (%d)", XMainAppliWindow);
					}
					XFree (pTypeBuffer);
					return NULL;  // inutile de rajouter le PID ici, c'est le meme que la fenetre principale.
				}
				//g_print ("dialogue autorise\n");
			}
			else
			{
				cd_debug ("type indesirable");
				XFree (pTypeBuffer);
				/**if (myTaskBar.bUniquePid)
					g_hash_table_insert (s_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.*/
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
			cd_debug ("  fenetre modale => on saute.");
			if (bDemandsAttention && (myTaskBar.bDemandsAttentionWithDialog || myTaskBar.cAnimationOnDemandsAttention))
			{
				Icon *pParentIcon = cairo_dock_get_icon_with_Xid (XMainAppliWindow);
				if (pParentIcon != NULL)
				{
					cd_debug ("%s requiert votre attention indirectement !", pParentIcon->acName);
					cairo_dock_appli_demands_attention (pParentIcon);
				}
				else
					cd_debug ("ce dialogue est bien bruyant ! (%d)", XMainAppliWindow);
			}
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
		cd_debug ("pas de nom, on en mettra un par defaut.");
		return NULL;
	}
	cd_debug ("recuperation de '%s' (bIsHidden : %d)", pNameBuffer, bIsHidden);
	
	
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
	
	/*const XklEngine *eng = xkl_engine_get_instance (cairo_dock_get_Xdisplay ());
	XklState state;
	xkl_engine_get_state (eng, Xid, &state);
	const gchar **names = xkl_engine_get_group_names (eng);
	int n = xkl_engine_get_num_groups (eng);
	for (i=0; i<n; i++)
		g_print ("name %d : %s\n", i, names[i]);*/
	
	//\__________________ On cree, on remplit l'icone, et on l'enregistre, par contre elle sera inseree plus tard.
	Icon *icon = g_new0 (Icon, 1);
	icon->acName = (pNameBuffer ? g_strdup ((gchar *)pNameBuffer) : g_strdup (cClass));
	/**if (myTaskBar.bUniquePid)
		icon->iPid = *pPidBuffer;*/
	icon->Xid = Xid;
	icon->cClass = cClass;
	Icon * pLastAppli = cairo_dock_get_last_appli (pDock->icons);
	icon->fOrder = (pLastAppli != NULL ? pLastAppli->fOrder + 1 : 1);
	icon->iType = CAIRO_DOCK_APPLI;
	icon->bIsHidden = bIsHidden;
	icon->bIsMaximized = bIsMaximized;
	icon->bIsFullScreen = bIsFullScreen;
	icon->bIsDemandingAttention = bDemandsAttention;
	icon->bHasIndicator = myTaskBar.bDrawIndicatorOnAppli;
	
	cairo_dock_get_xwindow_geometry (Xid,
		&icon->windowGeometry.x,
		&icon->windowGeometry.y,
		&icon->windowGeometry.width,
		&icon->windowGeometry.height);
	icon->iNumDesktop = cairo_dock_get_xwindow_desktop (Xid);
	#ifdef HAVE_XEXTEND
	if (myTaskBar.bShowThumbnail)
	{
		icon->iBackingPixmap = XCompositeNameWindowPixmap (s_XDisplay, Xid);
		/*icon->iDamageHandle = XDamageCreate (s_XDisplay, Xid, XDamageReportNonEmpty);  // XDamageReportRawRectangles
		g_print ("backing pixmap : %d ; iDamageHandle : %d\n", icon->iBackingPixmap, icon->iDamageHandle);*/
	}
	#endif
	
	cairo_dock_fill_icon_buffers_for_dock (icon, pSourceContext, pDock);
	
	/**if (myTaskBar.bUniquePid)
		g_hash_table_insert (s_hAppliTable, pPidBuffer, icon);*/
	cairo_dock_register_appli (icon);
	if (pNameBuffer)
		XFree (pNameBuffer);
	
	cairo_dock_set_xwindow_mask (Xid, PropertyChangeMask | StructureNotifyMask);

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
				cairo_dock_fill_one_text_buffer (icon, pCairoContext, &myLabels.iconTextDescription);
				cairo_destroy (pCairoContext);
				
				cairo_dock_update_name_on_inhibators (icon->cClass, icon->Xid, icon->acName);
			}
		}
	}
	else if (iState == PropertyNewValue && aProperty == s_aNetWmIcon)
	{
		cd_debug ("%s change son icone (%d)", icon->acName, cairo_dock_class_is_using_xicon (icon->cClass) || ! myTaskBar.bOverWriteXIcons);
		if (cairo_dock_class_is_using_xicon (icon->cClass) || ! myTaskBar.bOverWriteXIcons)
		{
			cairo_dock_reload_one_icon_buffer_in_dock (icon, pDock);
			cairo_dock_redraw_icon (icon, CAIRO_CONTAINER (pDock));
		}
	}
	else if (aProperty == s_aWmHints)
	{
		XWMHints *pWMHints = XGetWMHints (s_XDisplay, icon->Xid);
		if (pWMHints != NULL)
		{
			if ((pWMHints->flags & XUrgencyHint) && (myTaskBar.bDemandsAttentionWithDialog || myTaskBar.cAnimationOnDemandsAttention))
			{
				if (iState == PropertyNewValue)
				{
					cd_debug ("%s vous interpelle !", icon->acName);
					cairo_dock_appli_demands_attention (icon);
				}
				else if (iState == PropertyDelete)
				{
					cd_debug ("%s arrette de vous interpeler.", icon->acName);
					cairo_dock_appli_stops_demanding_attention (icon);
				}
				else
					cd_warning ("  etat du changement d'urgence inconnu sur %s !", icon->acName);
			}
			if (iState == PropertyNewValue && (pWMHints->flags & (IconPixmapHint | IconMaskHint | IconWindowHint)))
			{
				//g_print ("%s change son icone\n", icon->acName);
				if (cairo_dock_class_is_using_xicon (icon->cClass) || ! myTaskBar.bOverWriteXIcons)
				{
					cairo_dock_reload_one_icon_buffer_in_dock (icon, pDock);
					cairo_dock_redraw_icon (icon, CAIRO_CONTAINER (pDock));
				}
			}
		}
	}
}


static void _cairo_dock_appli_demands_attention (Icon *icon, CairoDock *pDock, gboolean bForceDemand, Icon *pHiddenIcon)
{
	cd_debug ("%s (%s, force:%d)\n", __func__, icon->acName, bForceDemand);
	icon->bIsDemandingAttention = TRUE;
	if (myTaskBar.bDemandsAttentionWithDialog)
	{
		CairoDialog *pDialog;
		if (pHiddenIcon == NULL)
		{
			pDialog = cairo_dock_show_temporary_dialog_with_icon (icon->acName, icon, CAIRO_CONTAINER (pDock), 1000*myTaskBar.iDialogDuration, "same icon");
		}
		else
		{
			pDialog = cairo_dock_show_temporary_dialog (pHiddenIcon->acName, icon, CAIRO_CONTAINER (pDock), 1000*myTaskBar.iDialogDuration);
			g_return_if_fail (pDialog != NULL);
			cairo_dock_set_new_dialog_icon_surface (pDialog, pHiddenIcon->pIconBuffer, pDialog->iIconSize);
		}
		if (pDialog && bForceDemand)
		{
			g_print ("force dialog on top\n");
			gtk_window_set_keep_above (GTK_WINDOW (pDialog->pWidget), TRUE);
			Window Xid = GDK_WINDOW_XID (pDialog->pWidget->window);
			cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_DOCK");  // pour passer devant les fenetres plein ecran; depend du WM.
		}
	}
	if (myTaskBar.cAnimationOnDemandsAttention && ! pHiddenIcon)  // on ne l'anime pas si elle n'est pas dans un dock.
	{
		if (pDock->iRefCount == 0)
		{
			cairo_dock_pop_up (pDock);
			if (pDock->iSidPopDown != 0)
			{
				g_source_remove(pDock->iSidPopDown);
				pDock->iSidPopDown = 0;
			}
			if (pDock->bAutoHide && bForceDemand)
			{
				g_print ("force dock to raise\n");
				cairo_dock_emit_enter_signal (pDock);
			}
		}
		else if (bForceDemand)
		{
			g_print ("force sub-dock to raise\n");
			CairoDock *pParentDock = NULL;
			Icon *pPointedIcon = cairo_dock_search_icon_pointing_on_dock (pDock, &pParentDock);
			if (pParentDock)
				cairo_dock_show_subdock (pPointedIcon, pParentDock, FALSE);
		}
		cairo_dock_request_icon_animation (icon, pDock, myTaskBar.cAnimationOnDemandsAttention, 10000);
		if (bForceDemand)
			cairo_dock_launch_animation (CAIRO_CONTAINER (pDock));  // precaution au cas ou le dock ne serait pas encore visible.
	}
}
void cairo_dock_appli_demands_attention (Icon *icon)
{
	//g_print ("%s (%s)\n", __func__, icon->acName);
	
	if (icon->bIsDemandingAttention &&
		cairo_dock_icon_has_dialog (icon) &&
		((! icon->cLastAttentionDemand && ! icon->acName) ||
		(icon->cLastAttentionDemand && icon->acName && strcmp (icon->cLastAttentionDemand, icon->acName) == 0)))  // la demande n'a pas change entre les 2 demandes.
	{
		return ;
	}
	g_free (icon->cLastAttentionDemand);
	icon->cLastAttentionDemand = g_strdup (icon->acName);
	
	gboolean bForceDemand = (myTaskBar.cForceDemandsAttention && icon->cClass && g_strstr_len (myTaskBar.cForceDemandsAttention, -1, icon->cClass));
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
	if (pParentDock == NULL)  // appli inhibee ou non affichee.
	{
		icon->bIsDemandingAttention = TRUE;  // on met a TRUE meme si ce n'est pas reellement elle qui va prendre la demande.
		Icon *pInhibitorIcon = cairo_dock_get_inhibator (icon, TRUE);  // on cherche son inhibiteur dans un dock.
		if (pInhibitorIcon != NULL)  // appli inhibee.
		{
			pParentDock = cairo_dock_search_dock_from_name (pInhibitorIcon->cParentDockName);
			if (pParentDock != NULL)
				_cairo_dock_appli_demands_attention (pInhibitorIcon, pParentDock, bForceDemand, NULL);
		}
		else if (bForceDemand)  // appli pas affichee, mais on veut tout de m�me etre notifie.
		{
			Icon *pOneIcon = cairo_dock_get_dialogless_icon ();
			if (pOneIcon != NULL)
				_cairo_dock_appli_demands_attention (pOneIcon, g_pMainDock, bForceDemand, icon);
		}
	}
	else  // appli dans un dock.
		_cairo_dock_appli_demands_attention (icon, pParentDock, bForceDemand, NULL);
}

static void _cairo_dock_appli_stops_demanding_attention (Icon *icon, CairoDock *pDock)
{
	icon->bIsDemandingAttention = FALSE;
	if (myTaskBar.bDemandsAttentionWithDialog)
		cairo_dock_remove_dialog_if_any (icon);
	cairo_dock_notify (CAIRO_DOCK_STOP_ICON, icon);  // arrete son animation quelqu'elle soit.
	if (! pDock->bInside)
	{
		g_print ("pop down the dock\n");
		cairo_dock_pop_down (pDock);
		
		if (pDock->bAutoHide)
		{
			g_print ("force dock to auto-hide\n");
			cairo_dock_emit_leave_signal (pDock);
		}
	}
}
void cairo_dock_appli_stops_demanding_attention (Icon *icon)
{
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
	if (pParentDock == NULL)
	{
		icon->bIsDemandingAttention = FALSE;  // idem que plus haut.
		Icon *pInhibitorIcon = cairo_dock_get_inhibator (icon, TRUE);
		if (pInhibitorIcon != NULL)
		{
			pParentDock = cairo_dock_search_dock_from_name (pInhibitorIcon->cParentDockName);
			if (pParentDock != NULL)
				_cairo_dock_appli_stops_demanding_attention (pInhibitorIcon, pParentDock);
		}
	}
	else
		_cairo_dock_appli_stops_demanding_attention (icon, pParentDock);
}

