/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <cairo.h>
#include <pango/pango.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include </usr/include/X11/Xlib.h>
#include </usr/include/X11/Xatom.h>
#include </usr/include/X11/Xutil.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-application-factory.h"


extern double g_fAmplitude;
extern int g_iLabelSize;
extern gboolean g_bUseText;
extern gchar *g_cLabelPolice;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUniquePid;
extern Display *g_XDisplay;
extern GHashTable *g_hAppliTable;
extern GHashTable *g_hXWindowTable;

extern gboolean g_bUseGlitz;


cairo_surface_t *cairo_dock_create_surface_from_xicon_buffer (gulong *pXIconBuffer, int iBufferNbElements, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	//g_print ("%s (%d)\n", __func__, iBufferNbElements);
	int iNbChannels = 4;
	
	//\____________________ On recupere la plus grosse des icones presentes dans le tampon (meilleur rendu).
	int iIndex = 0, iBestIndex = 0;
	while (iIndex + 2 < iBufferNbElements)
	{
		if (pXIconBuffer[iIndex] > pXIconBuffer[iBestIndex])
			iBestIndex = iIndex;
		iIndex += 2 + pXIconBuffer[iIndex] * pXIconBuffer[iIndex+1];
	}
	
	//\____________________ On pre-multiplie chaque composante par le alpha (necessaire pour libcairo).
	*fWidth = (double) pXIconBuffer[iBestIndex];
	*fHeight = (double) pXIconBuffer[iBestIndex+1];
	
	int i;
	int alpha, red, green, blue;
	float fAlphaFactor;
	for (i = 0; i < (int) (*fHeight) * (*fWidth); i ++)
	{
		alpha = (pXIconBuffer[iBestIndex+2+i] & 0xFF000000) >> 24;
		red = (pXIconBuffer[iBestIndex+2+i] & 0x00FF0000) >> 16;
		green = (pXIconBuffer[iBestIndex+2+i] & 0x0000FF00) >> 8;
		blue = pXIconBuffer[iBestIndex+2+i] & 0x000000FF;
		fAlphaFactor = (float) alpha / 255;
		red *= fAlphaFactor;
		green *= fAlphaFactor;
		blue *= fAlphaFactor;
		pXIconBuffer[iBestIndex+2+i] = (pXIconBuffer[iBestIndex+2+i] & 0xFF000000) + (red << 16) + (green << 8) + blue;
	}
	
	//\____________________ On cree la surface a partir du tampon.
	cairo_surface_t *surface_ini = cairo_image_surface_create_for_data ((guchar *)&pXIconBuffer[iBestIndex+2],
		CAIRO_FORMAT_ARGB32,
		(int) pXIconBuffer[iBestIndex],
		(int) pXIconBuffer[iBestIndex+1],
		(int) pXIconBuffer[iBestIndex] * iNbChannels);
	
	double fIconWidthSaturationFactor, fIconHeightSaturationFactor;
	cairo_dock_calculate_contrainted_icon_size (fWidth, 
		fHeight,
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI],
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLI],
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI],
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLI],
		&fIconWidthSaturationFactor,
		&fIconHeightSaturationFactor);
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		ceil (*fWidth * fMaxScale),
		ceil (*fHeight * fMaxScale));
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	/*cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);*/
	
	cairo_scale (pCairoContext, fMaxScale * fIconWidthSaturationFactor, fMaxScale * fIconHeightSaturationFactor);
	cairo_set_source_surface (pCairoContext, surface_ini, 0, 0);
	cairo_paint (pCairoContext);
	
	cairo_surface_destroy (surface_ini);
	cairo_destroy (pCairoContext);
	
	return pNewSurface;
}



cairo_surface_t *cairo_dock_create_surface_from_xwindow (Window Xid, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
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
	else
	{
		return NULL;
	}
}



Icon * cairo_dock_create_icon_from_xwindow (cairo_t *pSourceContext, Window Xid, CairoDock *pDock)
{
	//g_print ("%s (%d)\n", __func__, Xid);
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
			if (pXStateBuffer[i] == aNetWmSkipTaskbar)
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
	else
	{
		//g_print (" pas de type defini -> elle degage\n");
		return NULL;
	}
	
	//\__________________ On recupere son nom.
	Atom aNetWmName = XInternAtom (g_XDisplay, "_NET_WM_NAME", False);  // _NET_WM_CLASS marche pas :-(
	Atom aUtf8String = XInternAtom (g_XDisplay, "UTF8_STRING", False);
	XGetWindowProperty (g_XDisplay, Xid, aNetWmName, 0, G_MAXULONG, False, aUtf8String, &aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, &pNameBuffer);
	if (iBufferNbElements == 0)
	{
		if (g_bUniquePid)
			g_hash_table_insert (g_hAppliTable, pPidBuffer, NULL);  // On rajoute son PID meme si c'est une appli qu'on n'affichera pas.
		return NULL;
	}
	//g_print ("ajout de %s\n", pNameBuffer);
	
	//\__________________ On recupere son icone.
	pNewSurface = cairo_dock_create_surface_from_xwindow (Xid, pSourceContext, 1 + g_fAmplitude, &fWidth, &fHeight);
	if (pNewSurface == NULL)
	{
		//g_print ("pas d'icones\n");
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
	///icon->bIsMapped = TRUE;  // si elle n'est pas visible, le 2eme UnmapNotify sera juste ignore.
	cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_bUseText, g_iLabelSize, g_cLabelPolice);
	
	if (g_bUniquePid)
		g_hash_table_insert (g_hAppliTable, pPidBuffer, icon);
	Window *pXid = g_new (Window, 1);
	*pXid = Xid;
	g_hash_table_insert (g_hXWindowTable, pXid, icon);
	XFree (pNameBuffer);
	
	return icon;
}

