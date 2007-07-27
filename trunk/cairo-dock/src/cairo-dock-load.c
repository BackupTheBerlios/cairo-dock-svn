/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


******************************************************************************/
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>
#include <pango/pango.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-config.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-applications.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-application-factory.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-applet-factory.h"
#include "cairo-dock-load.h"


extern GList* icons;

extern int g_iSinusoidWidth;
extern double g_fAmplitude;

extern gint g_iWindowPositionX;
extern gint g_iWindowPositionY;
extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern int g_iIconGap;

extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gchar **g_cDefaultIconDirectory;
extern gchar *g_cCairoDockDataDir;

extern int g_iMaxDockWidth;
extern int g_iMaxDockHeight;
extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_iGapX;
extern int g_iGapY;
extern gchar *g_cCairoDockBackgroundFileName;
extern int g_iMaxIconHeight;
extern int g_iMinDockWidth;
extern int g_iNbAnimationRounds;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bRoundedBottomCorner;
extern double g_fStripesColor1[4];
extern double g_fStripesColor2[4];
extern double g_fLineColor[4];

extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern int g_iNbStripes;
extern gboolean g_bHorizontalDock;
extern gboolean g_bDirectionUp;

extern gboolean g_bAtBottom;
extern int g_iScreenWidth;
extern int g_iScreenHeight;
extern gboolean g_bShowAppli;
extern unsigned int g_iAppliMaxNameLength;
extern gchar *g_cDefaultFileBrowser;

extern int g_iLabelWeight;
extern int g_iLabelStyle;

extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];

#ifdef HAVE_GLITZ
extern gboolean g_bUseGlitz;
#endif // HAVE_GLITZ



void cairo_dock_calculate_contrainted_icon_size (double *fImageWidth, double *fImageHeight, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fIconWidthSaturationFactor, double *fIconHeightSaturationFactor)
{
	*fIconWidthSaturationFactor = 1;
	*fIconHeightSaturationFactor = 1;
	
	if (iMaxIconAuthorizedWidth > 0 && (*fImageWidth) > iMaxIconAuthorizedWidth)
	{
		*fIconWidthSaturationFactor = 1. * iMaxIconAuthorizedWidth / (*fImageWidth);
		*fImageWidth = (double) iMaxIconAuthorizedWidth;
	}
	if (iMaxIconAuthorizedHeight > 0 && (*fImageHeight) > iMaxIconAuthorizedHeight)
	{
		*fIconHeightSaturationFactor = 1. * iMaxIconAuthorizedHeight / (*fImageHeight);
		*fImageHeight = (double) iMaxIconAuthorizedHeight;
	}
	if (iMinIconAuthorizedWidth > 0 && (*fImageWidth) < iMinIconAuthorizedWidth)
	{
		*fIconWidthSaturationFactor = 1. * iMinIconAuthorizedWidth / (*fImageWidth);
		*fImageWidth = (double) iMinIconAuthorizedWidth;
	}
	if (iMinIconAuthorizedHeight > 0 && (*fImageHeight) < iMinIconAuthorizedHeight)
	{
		*fIconHeightSaturationFactor = 1. * iMinIconAuthorizedHeight / (*fImageHeight);
		*fImageHeight = (double) iMinIconAuthorizedHeight;
	}
}



void cairo_dock_insert_icon_in_list (Icon *icon, GtkWidget *pWidget, gboolean bUpdateSize, gboolean bAnimated)
{
	g_return_if_fail (icon != NULL);
	
	//\______________ On regarde si on doit inserer un separateur.
	if (! CAIRO_DOCK_IS_SEPARATOR (icon))
	{
		Icon *pSameTypeIcon = cairo_dock_get_first_icon_of_type (icon->iType);
		if (pSameTypeIcon == NULL && icons != NULL)
		{
			cairo_t *pSourceContext = cairo_dock_create_context_from_window (pWidget->window);
			int iSeparatorType = -1;
			if (g_tIconTypeOrder[icon->iType] > 1)  // on l'insere avant nous de preference.
				iSeparatorType = icon->iType - 1;
			else if (g_tIconTypeOrder[icon->iType] + 1 < CAIRO_DOCK_NB_TYPES)
				iSeparatorType = icon->iType + 1;
			//g_print ("iSeparatorType : %d\n", iSeparatorType);
			if (iSeparatorType != -1)
			{
				Icon *pSeparatorIcon = cairo_dock_create_separator_icon (pSourceContext, iSeparatorType);
				icons = g_list_insert_sorted (icons,
					pSeparatorIcon,
					(GCompareFunc) cairo_dock_compare_icons_order);
				g_iMinDockWidth += g_iIconGap + pSeparatorIcon->fWidth;
				g_iMaxIconHeight = MAX (g_iMaxIconHeight, pSeparatorIcon->fHeight);
			}
		}
	}
	
	//\______________ On insere l'icone a sa place dans la liste.
	icons = g_list_insert_sorted (icons,
		icon,
		(GCompareFunc) cairo_dock_compare_icons_order);
	
	g_iMinDockWidth += g_iIconGap + icon->fWidth;
	g_iMaxIconHeight = MAX (g_iMaxIconHeight, icon->fHeight);
	
	//\______________ On effectue les actions demandees.
	if (bAnimated)
		icon->fPersonnalScale = - 0.95;
	
	if (bUpdateSize)
		cairo_dock_update_dock_size (pWidget, g_iMaxIconHeight, g_iMinDockWidth);
	
	
}



void cairo_dock_fill_one_icon_buffer (Icon *icon, cairo_t* pSourceContext, gdouble fMaxScale)
{
	//g_print ("%s (%.2f)\n", __func__, fMaxScale);
	icon->fWidth = 48.;  // valeur par defaut au cas ou l'icone est inexistante ou ne se chargerait pas comme il faut.
	icon->fHeight = 48.;
	cairo_surface_destroy (icon->pIconBuffer);
	icon->pIconBuffer = NULL;
	
	if (CAIRO_DOCK_IS_LAUNCHER (icon))  // c'est l'icone d'un .desktop.
	{
		//\_______________________ On recherche une icone.
		gchar *cIconPath = cairo_dock_search_image_path (icon->acFileName);
		
		//\_______________________ On cree la surface cairo a afficher.
		if (strlen (cIconPath) > 0)
		{
			icon->pIconBuffer = cairo_dock_create_surface_from_image (cIconPath, pSourceContext, fMaxScale, g_tMinIconAuthorizedSize[CAIRO_DOCK_LAUNCHER], g_tMinIconAuthorizedSize[CAIRO_DOCK_LAUNCHER], g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER], g_tMaxIconAuthorizedSize[CAIRO_DOCK_LAUNCHER], &icon->fWidth, &icon->fHeight, 0);
		}
		
		g_free (cIconPath);
	}
	else if (CAIRO_DOCK_IS_VALID_APPLI (icon))  // c'est l'icône d'une appli valide.
	{
		icon->pIconBuffer = cairo_dock_create_surface_from_xwindow (icon->Xid, pSourceContext, fMaxScale, &icon->fWidth, &icon->fHeight);
	}
	else if (CAIRO_DOCK_IS_APPLET (icon))  // c'est l'icône d'une applet.
	{
		icon->pIconBuffer = cairo_dock_create_applet_surface (pSourceContext, fMaxScale, &icon->fWidth, &icon->fHeight);
	}
	else  // c'est une icone de separation.
	{
		icon->pIconBuffer = cairo_dock_create_separator_surface (pSourceContext, fMaxScale, &icon->fWidth, &icon->fHeight);
	}
	
	g_iMinDockWidth += g_iIconGap + icon->fWidth;
	g_iMaxIconHeight = MAX (g_iMaxIconHeight, icon->fHeight);
}


void cairo_dock_fill_one_text_buffer (Icon *icon, cairo_t* pSourceContext, gboolean bUseText, int iLabelSize, gchar *cLabelPolice)
{
	//g_print ("%s (%s, %d)\n", __func__, cLabelPolice, iLabelSize);
	cairo_surface_destroy (icon->pTextBuffer);
	icon->pTextBuffer = NULL;
	if (icon->acName == NULL || ! bUseText)
		return ;
	
	PangoFontDescription *pDesc;
	PangoLayout *pLayout;
	
	pLayout = pango_cairo_create_layout (pSourceContext);
	
	pDesc = pango_font_description_new ();
	pango_font_description_set_absolute_size (pDesc, iLabelSize * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, cLabelPolice);
	pango_font_description_set_weight (pDesc, g_iLabelWeight);
	pango_font_description_set_style (pDesc, g_iLabelStyle);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_font_description_free (pDesc);
	
	
	cairo_surface_t* pNewSurface = NULL;
	cairo_t* pCairoContext;
	gint i;
	PangoRectangle ink, log;
	
	if (CAIRO_DOCK_IS_APPLI (icon) && g_iAppliMaxNameLength > 0 && strlen (icon->acName) > g_iAppliMaxNameLength)  // marchera pas avec les caracteres non latins, mais avec la glib c'est la galere.
	{
		gchar *cTruncatedName = g_new0 (gchar, g_iAppliMaxNameLength + 4);
		strncpy (cTruncatedName, icon->acName, g_iAppliMaxNameLength);
		cTruncatedName[g_iAppliMaxNameLength] = '.';
		cTruncatedName[g_iAppliMaxNameLength+1] = '.';
		cTruncatedName[g_iAppliMaxNameLength+2] = '.';
		pango_layout_set_text (pLayout, cTruncatedName, -1);
		g_free (cTruncatedName);
	}
	else
		pango_layout_set_text (pLayout, icon->acName, -1);
	
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	
	pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
						    CAIRO_CONTENT_COLOR_ALPHA,
						    ink.width + 2, ink.height + 2);
	pCairoContext = cairo_create (pNewSurface);
	cairo_translate (pCairoContext, -ink.x, -ink.y);
	
	cairo_push_group (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0.2, 0.2, 0.2);
	for (i = 0; i < 4; i++)
	{
		cairo_move_to (pCairoContext, i&2, 2*(i&1));
		pango_cairo_show_layout (pCairoContext, pLayout);
	}
	cairo_pop_group_to_source (pCairoContext);
	if (! g_bHorizontalDock)
		cairo_rotate (pCairoContext, (g_bDirectionUp ? - G_PI / 2 : G_PI / 2));
	cairo_paint_with_alpha (pCairoContext, .7);
	
	cairo_set_source_rgb (pCairoContext, 1., 1., 1.);
	cairo_move_to (pCairoContext, 1., 1.);
	pango_cairo_show_layout (pCairoContext, pLayout);

	cairo_destroy (pCairoContext);

	/* set_device_offset is buggy, doesn't work for positive
	 * offsets.  so we use explicit offsets... so unfortunate.
	cairo_surface_set_device_offset (pNewSurface, 
					 log.width / 2. - ink.x,
					 log.height     - ink.y);*/
	icon->fTextXOffset = log.width / 2. - ink.x;
	icon->fTextYOffset = log.height     - ink.y;
	icon->pTextBuffer = pNewSurface;
	
	g_object_unref (pLayout);
}


void cairo_dock_reload_buffers (GtkWidget *pWidget, double fMaxScale, int iLabelSize, gboolean bUseText)
{
	g_iMinDockWidth = - g_iIconGap;
	g_iMaxIconHeight = 0;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
	
	Icon* icon;
	GList* ic;
	for (ic = icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		cairo_dock_fill_one_icon_buffer (icon, pCairoContext, fMaxScale);
		
		cairo_dock_fill_one_text_buffer (icon, pCairoContext, bUseText, g_iLabelSize, g_cLabelPolice);
	}
	
	cairo_destroy (pCairoContext);
	g_iMaxDockHeight = (int) (fMaxScale * g_iMaxIconHeight) + iLabelSize;
}






void cairo_dock_load_background_image (GtkWindow *pWindow, gchar *image_filename, int image_width, int image_height)
{
	//g_print ("%s (%dx%d)\n", __func__, image_width, image_height);
	cairo_surface_destroy (g_pVisibleZoneSurface);
	g_pVisibleZoneSurface = NULL;
	
	if (image_filename != NULL)
	{
		gchar *cImagePath;
		if (*image_filename == '~')
		{
			cImagePath = g_strdup_printf ("%s%s", getenv("HOME"), image_filename + 1);
		}
		else if (*image_filename == '/')
		{
			cImagePath = g_strdup (image_filename);
		}
		else
		{
			cImagePath = g_strdup_printf ("%s/.cairo-dock/%s", getenv("HOME"), image_filename);
		}
		
		cairo_t * pCairoContext = cairo_dock_create_context_from_window (GTK_WIDGET (pWindow)->window);
		
		g_pVisibleZoneSurface = cairo_dock_create_surface_from_image (cImagePath,
			pCairoContext,
			1.,
			image_width,
			image_height,
			image_width,
			image_height,
			&g_fVisibleZoneImageWidth,
			&g_fVisibleZoneImageHeight,
			(g_bHorizontalDock ? 0 : (g_bDirectionUp ? -G_PI/2 : G_PI/2)));
		/*if (! g_bHorizontalDock)
		{
			double tmp = g_fVisibleZoneImageWidth;
			g_fVisibleZoneImageWidth = g_fVisibleZoneImageHeight;
			g_fVisibleZoneImageHeight = tmp;
			cairo_rotate (pCairoContext, .2,);
		}*/
		g_free (cImagePath);
	}
}




void cairo_dock_init_list_with_desktop_files (GtkWidget *pWidget, gchar *cDirectory)
{
	icons = NULL;
	
	DIR* dir = opendir (cDirectory);
	g_return_if_fail (dir != NULL);
	
	Icon* icon;
	gchar *cFileName;
	struct dirent *dirpointer;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
	while ((dirpointer = readdir (dir)) != NULL)
	{
		cFileName = dirpointer->d_name;
		if (g_str_has_suffix (cFileName, ".desktop"))
		{
			icon = cairo_dock_create_icon_from_desktop_file (cFileName, pCairoContext);
			cairo_dock_insert_icon_in_list (icon, pWidget, FALSE, FALSE);  // inutile de passer le widget ici.
		}
	}
}

