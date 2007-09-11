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

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-icons.h"
#include "cairo-dock-load.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-separator-factory.h"

extern double g_fAmplitude;
extern int g_iDockLineWidth;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern gchar *g_cSeparatorImage;
extern gboolean g_bRevolveSeparator;

extern gboolean g_bUseGlitz;


cairo_surface_t *cairo_dock_create_separator_surface (cairo_t *pSourceContext, double fMaxScale, gboolean bHorizontalDock, double *fWidth, double *fHeight)
{
	*fWidth = 10;
	*fHeight = 48;
	
	cairo_surface_t *pNewSurface = NULL;
	if (g_cSeparatorImage != NULL)
	{
		//gchar *cImagePath = cairo_dock_search_image_path (g_cSeparatorImage);
		gchar *cImagePath = cairo_dock_generate_file_path (g_cSeparatorImage);
		double fRotationAngle;
		if (! g_bRevolveSeparator)
			fRotationAngle = 0;
		else if (bHorizontalDock)
			if (g_bDirectionUp)
				fRotationAngle = 0;
			else
				fRotationAngle = G_PI;
		else
			if (g_bDirectionUp)
				fRotationAngle = -G_PI/2;
			else
				fRotationAngle = G_PI/2;
		pNewSurface = cairo_dock_create_surface_from_image (cImagePath,
			pSourceContext,
			fMaxScale,
			g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12],
			g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12],
			g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12],
			g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12],
			fWidth,
			fHeight,
			fRotationAngle,
			1,
			FALSE);
		g_free (cImagePath);
	}
	return pNewSurface;
}



Icon *cairo_dock_create_separator_icon (cairo_t *pSourceContext, int iSeparatorType, CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	double fWidth, fHeight;
	cairo_surface_t *pSeparatorSurface = cairo_dock_create_separator_surface (pSourceContext, 1 + g_fAmplitude, pDock->bHorizontalDock, &fWidth, &fHeight);
	
	if (pSeparatorSurface == NULL)
		return NULL;
	
	Icon *icon = g_new0 (Icon, 1);
	icon->pIconBuffer = pSeparatorSurface;
	icon->fWidth = fWidth;
	icon->fHeight = fHeight;
	//Icon * pLastLauncher = cairo_dock_get_last_launcher (pDock->icons);
	//icon->fOrder = (pLastLauncher != NULL ? pLastLauncher->fOrder + 1 : 1);
	icon->iType = iSeparatorType;
	
	return icon;
}

