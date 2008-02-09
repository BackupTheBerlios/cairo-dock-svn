/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>

#include "cairo-dock-load.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-config.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-applet-factory.h"

extern double g_fAmplitude;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

extern int g_tIconAuthorizedWidth[CAIRO_DOCK_NB_TYPES];
extern int g_tIconAuthorizedHeight[CAIRO_DOCK_NB_TYPES];


cairo_surface_t *cairo_dock_create_applet_surface (gchar *cIconFileName, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight, gboolean bApplySizeRestriction)
{
	g_print ("%s (%.2fx%.2f x %.2f / %d)\n", __func__, *fWidth, *fHeight,fMaxScale, bApplySizeRestriction);
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	double fIconWidthSaturationFactor, fIconHeightSaturationFactor;
	cairo_dock_calculate_contrainted_icon_size (fWidth,
		fHeight,
		(bApplySizeRestriction ? g_tIconAuthorizedWidth[CAIRO_DOCK_APPLET] : 0),
		(bApplySizeRestriction ? g_tIconAuthorizedHeight[CAIRO_DOCK_APPLET] : 0),
		(bApplySizeRestriction ? g_tIconAuthorizedWidth[CAIRO_DOCK_APPLET] : 0),
		(bApplySizeRestriction ? g_tIconAuthorizedHeight[CAIRO_DOCK_APPLET] : 0),
		&fIconWidthSaturationFactor, &fIconHeightSaturationFactor);
	g_print (" -> %.2fx%.2f x %.2f\n", *fWidth, *fHeight,fMaxScale);
	cairo_surface_t *pNewSurface;
	if (cIconFileName == NULL)
		pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			ceil (*fWidth * fMaxScale),
			ceil (*fHeight * fMaxScale));
	else
	{
		gchar *cIconPath = cairo_dock_search_icon_s_path (cIconFileName);
		pNewSurface = cairo_dock_create_surface_from_image (cIconPath,
			pSourceContext,
			fMaxScale,
			*fWidth,
			*fHeight,
			*fWidth,
			*fHeight,
			fWidth,
			fHeight,
			0,
			1,
			FALSE);
		g_free (cIconPath);
	}
	return pNewSurface;
}


Icon *cairo_dock_create_icon_for_applet (CairoDock *pDock, CairoDockDesklet *pDesklet, int iWidth, int iHeight, gchar *cName, gchar *cIconFileName, CairoDockModule *pModule)
{
	Icon *icon = g_new0 (Icon, 1);
	icon->iType = CAIRO_DOCK_APPLET;
	icon->pModule = pModule;
	
	icon->acName = g_strdup (cName);
	icon->acFileName = g_strdup (cIconFileName);  // NULL si cIconFileName = NULL.
	
	icon->fWidth =iWidth;
	icon->fHeight =iHeight;
	icon->fWidthFactor = 1.;
	cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock != NULL ? pDock : pDesklet);
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, icon);
	
	if (pDock != NULL)
	{
		cairo_dock_fill_one_icon_buffer (icon, pSourceContext, 1 + g_fAmplitude, pDock->bHorizontalDock, TRUE);
		cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
	}
	else
	{
		cairo_dock_fill_one_icon_buffer (icon, pSourceContext, 1., CAIRO_DOCK_HORIZONTAL, FALSE);
	}
	
	cairo_destroy (pSourceContext);
	return icon;
}
