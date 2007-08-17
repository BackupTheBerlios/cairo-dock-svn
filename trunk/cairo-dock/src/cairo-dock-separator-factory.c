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


extern gint g_iScreenWidth;
extern gint g_iScreenHeight;

extern double g_fAmplitude;
extern int g_iLabelSize;
extern gboolean g_bUseText;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern int g_iIconGap;
extern int g_iMaxIconHeight;

extern gchar *g_cConfFile;
extern gchar *g_cCairoDockDataDir;

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern gchar *g_cLabelPolice;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;
extern double g_fLineColor[4];;
extern int g_iNbStripes;

extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];
extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern gchar *g_cSeparatorImage;

#ifdef HAVE_GLITZ
extern gboolean g_bUseGlitz;
extern glitz_drawable_format_t *gDrawFormat;
extern glitz_drawable_t* g_pGlitzDrawable;
extern glitz_format_t* g_pGlitzFormat;
#endif // HAVE_GLITZ


cairo_surface_t *cairo_dock_create_separator_surface (cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	*fWidth = 10;
	*fHeight = 48;
	
	cairo_surface_t *pNewSurface = NULL;
	if (g_cSeparatorImage != NULL)
	{
		gchar *cImagePath = cairo_dock_search_image_path (g_cSeparatorImage);
		pNewSurface = cairo_dock_create_surface_from_image (cImagePath,
			pSourceContext,
			fMaxScale,
			g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12],
			g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12],
			g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12],
			g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12],
			fWidth,
			fHeight,
			(g_bHorizontalDock ? 0 : (g_bDirectionUp ? -G_PI/2 : G_PI/2)),
			1,
			FALSE);
		g_free (cImagePath);
	}
	/*else
	{
		int iLineWidth = g_iDockLineWidth;
		
		double fIconWidthSaturationFactor, fIconHeightSaturationFactor;
		cairo_dock_calculate_contrainted_icon_size (fWidth, fHeight, g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12], g_tMinIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12], g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12], g_tMaxIconAuthorizedSize[CAIRO_DOCK_SEPARATOR12], &fIconWidthSaturationFactor, &fIconHeightSaturationFactor);
		
		pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			ceil ((*fWidth) * fMaxScale),
			ceil ((*fHeight) * fMaxScale));
		cairo_t *pCairoContext = cairo_create (pNewSurface);
		cairo_save (pCairoContext);
		cairo_translate (pCairoContext, fMaxScale * (*fWidth) / 2, fMaxScale * (*fHeight) / 2);
		cairo_scale (pCairoContext, fMaxScale * (*fWidth) / 2., fMaxScale * (*fHeight - iLineWidth) / 2.);
		cairo_arc (pCairoContext, 0., 0., 1., 0., 2 * G_PI);
		cairo_restore (pCairoContext);
		
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_set_line_width (pCairoContext, iLineWidth);
		cairo_stroke (pCairoContext);
		
		cairo_destroy (pCairoContext);
	}*/
	return pNewSurface;
}



Icon *cairo_dock_create_separator_icon (cairo_t *pSourceContext, int iSeparatorType, CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	double fWidth, fHeight;
	cairo_surface_t *pSeparatorSurface = cairo_dock_create_separator_surface (pSourceContext, 1 + g_fAmplitude, &fWidth, &fHeight);
	
	if (pSeparatorSurface == NULL)
		return NULL;
	
	Icon *icon = g_new0 (Icon, 1);
	icon->pIconBuffer = pSeparatorSurface;
	icon->fWidth = fWidth;
	icon->fHeight = fHeight;
	Icon * pLastLauncher = cairo_dock_get_last_launcher (pDock->icons);
	icon->fOrder = (pLastLauncher != NULL ? pLastLauncher->fOrder + 1 : 1);
	icon->iType = iSeparatorType;
	
	return icon;
}

