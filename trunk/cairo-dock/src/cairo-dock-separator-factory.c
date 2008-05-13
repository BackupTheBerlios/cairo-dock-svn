/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <math.h>
#include <string.h>
#include <cairo.h>
#include <stdlib.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-icons.h"
#include "cairo-dock-load.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-log.h"

extern double g_fAmplitude;
extern int g_iDockLineWidth;
extern double g_fSubDockSizeRatio;

extern gboolean g_bHorizontalDock;

extern int g_tIconAuthorizedWidth[CAIRO_DOCK_NB_TYPES];
extern int g_tIconAuthorizedHeight[CAIRO_DOCK_NB_TYPES];
extern gboolean g_bUseSeparator;
extern gchar *g_cSeparatorImage;
extern gboolean g_bRevolveSeparator;

extern gboolean g_bUseGlitz;


cairo_surface_t *cairo_dock_create_separator_surface (cairo_t *pSourceContext, double fMaxScale, gboolean bHorizontalDock, gboolean bDirectionUp, double *fWidth, double *fHeight)
{
	*fWidth = 10;
	*fHeight = 48;

	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	cairo_surface_t *pNewSurface = NULL;
	if (g_cSeparatorImage != NULL)
	{
		gchar *cImagePath = cairo_dock_generate_file_path (g_cSeparatorImage);
		double fRotationAngle;
		if (! g_bRevolveSeparator)
			fRotationAngle = 0;
		else if (bHorizontalDock)
			if (bDirectionUp)
				fRotationAngle = 0;
			else
				fRotationAngle = G_PI;
		else
			if (bDirectionUp)
				fRotationAngle = -G_PI/2;
			else
				fRotationAngle = G_PI/2;
		
		pNewSurface = cairo_dock_create_surface_from_image (cImagePath,
			pSourceContext,
			fMaxScale,
			g_tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR12],
			g_tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR12],
			fWidth,
			fHeight,
			CAIRO_DOCK_FILL_SPACE);
		if (fRotationAngle != 0)
		{
			cairo_surface_t *pNewSurfaceRotated = cairo_dock_rotate_surface (pNewSurface, pSourceContext, *fWidth * fMaxScale, *fHeight * fMaxScale, fRotationAngle);
			cairo_surface_destroy (pNewSurface);
			pNewSurface = pNewSurfaceRotated;
		}
		g_free (cImagePath);
	}
	else
	{
		double fIconWidthSaturationFactor, fIconHeightSaturationFactor;
		cairo_dock_calculate_size_fill (fWidth,
			fHeight,
			g_tIconAuthorizedWidth[CAIRO_DOCK_SEPARATOR12],
			g_tIconAuthorizedHeight[CAIRO_DOCK_SEPARATOR12],
			FALSE,
			&fIconWidthSaturationFactor,
			&fIconHeightSaturationFactor);

		pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			ceil (*fWidth * fMaxScale),
			ceil (*fHeight * fMaxScale));
	}

	return pNewSurface;
}



Icon *cairo_dock_create_separator_icon (cairo_t *pSourceContext, int iSeparatorType, CairoDock *pDock, gboolean bApplyRatio)
{
	//g_print ("%s ()\n", __func__);
	if ((iSeparatorType & 1) && ! g_bUseSeparator)
		return NULL;

	Icon *icon = g_new0 (Icon, 1);
	icon->iType = iSeparatorType;
	cairo_dock_fill_one_icon_buffer (icon, pSourceContext, 1 + g_fAmplitude, pDock->bHorizontalDock, TRUE, pDock->bDirectionUp);

	if (bApplyRatio)  ///  && pDock->iRefCount > 0
	{
		icon->fWidth *= pDock->fRatio;  /// g_fSubDockSizeRatio
		icon->fHeight *= pDock->fRatio;
	}
	//g_print ("1 separateur : %.2fx%.2f\n", icon->fWidth, icon->fHeight);

	return icon;
}
