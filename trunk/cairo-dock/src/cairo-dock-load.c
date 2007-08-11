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
#include "cairo-dock-modules.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-load.h"

extern GHashTable *g_hDocksTable;
extern CairoDock *g_pMainDock;

GHashTable *g_hModuleTable;

extern int g_iSinusoidWidth;
extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern int g_iIconGap;

extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gchar **g_cDefaultIconDirectory;
extern gchar *g_cCairoDockDataDir;
extern gchar *g_cConfFile;

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;

extern gchar *g_cCairoDockBackgroundFileName;

extern int g_iNbAnimationRounds;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bRoundedBottomCorner;
extern double g_fStripesColor1[4];
extern double g_fStripesColor2[4];
extern double g_fLineColor[4];

extern cairo_surface_t *g_pVisibleZoneSurface;
extern cairo_surface_t *g_pVisibleZoneSurfaceAlpha;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern int g_iNbStripes;
extern double g_fStripesAngle;
extern double g_fStripesWidth;
extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];
extern cairo_surface_t *g_pStripesBuffer;

extern gboolean g_bHorizontalDock;
extern gboolean g_bDirectionUp;

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
	
	if (CAIRO_DOCK_IS_APPLI (icon) && g_iAppliMaxNameLength > 0 && strlen (icon->acName) > g_iAppliMaxNameLength)  // marchera pas avec les caracteres non latins, mais avec la glib c'est la galere...
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
	//icon->fTextYOffset = log.height     - ink.y;
	icon->fTextYOffset = iLabelSize;
	
	double fAdjustmentFactor = 1. * iLabelSize / (ink.height + 2);
	if (fAdjustmentFactor < .999)  // le texte obtenu est trop grand (pas de beaucoup, mais cela arrive avec sertaines polices).
	{
		icon->pTextBuffer = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		(ink.width + 2) * fAdjustmentFactor, (ink.height + 2) * fAdjustmentFactor);
		pCairoContext = cairo_create (icon->pTextBuffer);
		
		cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
		cairo_paint (pCairoContext);
		
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
		cairo_move_to (pCairoContext, 0, 0);
		cairo_scale (pCairoContext, fAdjustmentFactor, fAdjustmentFactor);
		cairo_set_source_surface (pCairoContext, pNewSurface, 0., 0.);
		cairo_paint (pCairoContext);
		
		cairo_surface_destroy (pNewSurface);
		cairo_destroy (pCairoContext);
	}
	else
		icon->pTextBuffer = pNewSurface;
	
	g_object_unref (pLayout);
}


static void _cairo_dock_reload_buffers_in_dock (gchar *cDockName, CairoDock *pDock, gpointer *data)
{
	pDock->iMinDockWidth = - g_iIconGap;
	pDock->iMaxIconHeight = 0;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock->pWidget->window);
	double fMaxScale = *((double *) data[0]);
	int iLabelSize = *((int *) data[1]);
	gboolean bUseText = *((gboolean *) data[2]);
	gchar *cLabelPolice = (gchar *) data[3];
	
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		cairo_dock_fill_one_icon_buffer (icon, pCairoContext, fMaxScale);
		pDock->iMinDockWidth += g_iIconGap + icon->fWidth;
		pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, icon->fHeight);
		
		cairo_dock_fill_one_text_buffer (icon, pCairoContext, bUseText, iLabelSize, cLabelPolice);
		
		//if (icon->pSubDock != NULL)
		//	cairo_dock_reload_buffers_in_dock (icon->pSubDock, fMaxScale, iLabelSize, bUseText);
	}
	
	cairo_destroy (pCairoContext);
	pDock->iMaxDockHeight = (int) (fMaxScale * pDock->iMaxIconHeight) + iLabelSize + g_iDockLineWidth;
}
void cairo_dock_reload_buffers_in_all_dock (GHashTable *hDocksTable, double fMaxScale, int iLabelSize, gboolean bUseText, gchar *cLabelPolice)
{
	gpointer data[4] = {&fMaxScale, &iLabelSize, &bUseText, cLabelPolice};
	g_hash_table_foreach (hDocksTable, (GHFunc) _cairo_dock_reload_buffers_in_dock, data);
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
		if (! g_bHorizontalDock)
		{
			/*double tmp = g_fVisibleZoneImageWidth;
			g_fVisibleZoneImageWidth = g_fVisibleZoneImageHeight;
			g_fVisibleZoneImageHeight = tmp;*/
			/*cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pCairoContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				g_fVisibleZoneImageHeight,
				g_fVisibleZoneImageWidth);
			cairo_destroy (pCairoContext);
			pCairoContext = cairo_create (pNewSurface);
			
			cairo_move_to (pCairoContext, g_fVisibleZoneImageHeight, 0);
			cairo_rotate (pCairoContext, -G_PI / 2);
			cairo_translate (pCairoContext, - g_fVisibleZoneImageWidth, 0);
			cairo_set_source_surface (pCairoContext, g_pVisibleZoneSurface, 0, 0);
			
			cairo_paint (pCairoContext);
			cairo_surface_destroy (g_pVisibleZoneSurface);
			g_pVisibleZoneSurface = pNewSurface;*/
		}
		cairo_destroy (pCairoContext);
		
		pCairoContext = cairo_dock_create_context_from_window (GTK_WIDGET (pWindow)->window);
		g_pVisibleZoneSurfaceAlpha = cairo_surface_create_similar (cairo_get_target (pCairoContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			g_fVisibleZoneImageWidth,
			g_fVisibleZoneImageHeight);
		cairo_destroy (pCairoContext);
		pCairoContext = cairo_create (g_pVisibleZoneSurfaceAlpha);
		cairo_set_source_surface (pCairoContext, g_pVisibleZoneSurface, 0, 0);
		cairo_paint_with_alpha (pCairoContext, 0.5);
		
		cairo_destroy (pCairoContext);
		g_free (cImagePath);
	}
}

void cairo_dock_update_stripes_if_necessary (GtkWidget *pWidget, int iNewMaxDockWidth, int iNewMaxIconHeight, gboolean bForce)
{
	static int iStripesWidth = 0, iStripesHeight = 0;
	if (g_iNbStripes > 0)
	{
		if (iNewMaxDockWidth > iStripesWidth || iNewMaxIconHeight > iStripesHeight || bForce)
		{
			if (bForce)
			{
				iStripesWidth = iNewMaxDockWidth;
				iStripesHeight = iNewMaxIconHeight;
			}
			else
			{
				iStripesWidth = MAX (iStripesWidth, iNewMaxDockWidth);
				iStripesHeight = MAX (iStripesHeight, iNewMaxIconHeight);
			}
			
			cairo_pattern_t *pStripesPattern = cairo_pattern_create_linear (0.0f,
			0.0f,
			200,
			(gdouble) 200. * tan (g_fStripesAngle * G_PI/180.));
			g_return_if_fail (cairo_pattern_status (pStripesPattern) == CAIRO_STATUS_SUCCESS);
			
			
			cairo_pattern_set_extend (pStripesPattern, CAIRO_EXTEND_REPEAT);
			gdouble fStep;
			double fStripesGap = 1. / (g_iNbStripes);  // ecart entre 2 rayures foncees.
			for (fStep = 0.0f; fStep < 1.0f; fStep += fStripesGap)
			{
				cairo_pattern_add_color_stop_rgba (pStripesPattern,
					fStep - g_fStripesWidth / 2,
					g_fStripesColorBright[0],
					g_fStripesColorBright[1],
					g_fStripesColorBright[2],
					g_fStripesColorBright[3]);
				cairo_pattern_add_color_stop_rgba (pStripesPattern,
					fStep,
					g_fStripesColorDark[0],
					g_fStripesColorDark[1],
					g_fStripesColorDark[2],
					g_fStripesColorDark[3]);
				cairo_pattern_add_color_stop_rgba (pStripesPattern,
					fStep + g_fStripesWidth / 2,
					g_fStripesColorBright[0],
					g_fStripesColorBright[1],
					g_fStripesColorBright[2],
					g_fStripesColorBright[3]);
			}
			
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
			
			cairo_surface_destroy (g_pStripesBuffer);
			g_pStripesBuffer = cairo_surface_create_similar (cairo_get_target (pCairoContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				ceil (2 * iStripesWidth),
				ceil (iStripesHeight));
			cairo_t *pImageContext = cairo_create (g_pStripesBuffer);
			cairo_set_source (pImageContext, pStripesPattern);
			cairo_paint (pImageContext);
			
			cairo_pattern_destroy (pStripesPattern);
			cairo_destroy (pCairoContext);
			cairo_destroy (pImageContext);
		}
	}
}


static void _cairo_dock_search_max_docks_size (gchar *cDockName, CairoDock *pDock, int *data)
{
	if (pDock->iMaxDockWidth > data[0])
		data[0] = pDock->iMaxDockWidth;
	if (pDock->iMaxIconHeight > data[1])
		data[1] = pDock->iMaxIconHeight;
}
void cairo_dock_load_stripes_background (CairoDock *pMainDock)
{
	cairo_surface_destroy (g_pStripesBuffer);
	g_pStripesBuffer = NULL;
	if (g_iNbStripes > 0)
	{
		int iMaxDocksWidth = 0, iMaxIconsHeight = 0;
		int data[2] = {0, 0};  // iMaxDocksWidth, iMaxIconsHeight.
		g_hash_table_foreach (g_hDocksTable, (GHFunc) _cairo_dock_search_max_docks_size, &data);
		
		cairo_dock_update_stripes_if_necessary (pMainDock->pWidget, data[0], data[1], TRUE);
	}
}


gpointer cairo_dock_init (gpointer data)
{
	//\___________________ On teste l'existence du repertoire des donnees .cairo-dock.
	GError *erreur = NULL;
	if (! g_file_test (g_cCairoDockDataDir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		GHashTable *pThemeTable = cairo_dock_list_themes (CAIRO_DOCK_SHARE_THEMES_DIR, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			exit (1);
		}
		
		gchar *cTemporaryFilePath = g_strdup ("/tmp/cairo-dock-init");
		g_file_set_contents (cTemporaryFilePath, "#!\n[INIT]\ntheme = default", -1, &erreur);
		if (erreur != NULL)
		{
			g_print ("Error while writing data : %s\n", erreur->message);
			g_error_free (erreur);
			exit (1);
		}
		cairo_dock_update_conf_file_with_hash_table (cTemporaryFilePath, pThemeTable, "INIT", "theme", 1, "Choose a theme to start using cairo-dock\n (you can choose another theme later\n by deleting your ~/.cairo-dock direcory,\n and re-launching cairo-dock)");
		
		gboolean bChoiceOK = cairo_dock_edit_conf_file (NULL, cTemporaryFilePath, "Choose a theme to start with", 400, 300);
		if (! bChoiceOK)
		{
			g_print ("Mata ne.\n");
			exit (0);
		}
		
		GKeyFile *pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, cTemporaryFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			exit (1);
		}
		gchar *cChosenTheme = g_key_file_get_string (pKeyFile, "INIT", "theme", &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			exit (1);
		}
		g_key_file_free (pKeyFile);
		g_free (cTemporaryFilePath);
		
		
		gchar *cCommand = g_strdup_printf ("mkdir -p %s", g_cCairoDockDataDir);
		system (cCommand);
		g_free (cCommand);
		
		cCommand = g_strdup_printf ("cp -r %s/%s/* %s", CAIRO_DOCK_SHARE_THEMES_DIR, cChosenTheme, g_cCairoDockDataDir);
		system (cCommand);
		g_free (cCommand);
		
		g_free (cChosenTheme);
		g_hash_table_destroy (pThemeTable);
		
		if (! g_file_test (g_cCairoDockDataDir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
		{
			g_print("Attention : directory %s unreadable.\n", g_cCairoDockDataDir);
			exit (1) ;
		}
	}
	else if (! g_file_test (g_cConfFile, G_FILE_TEST_EXISTS))
	{
		gchar *cCommand = g_strdup_printf ("cp %s/%s %s", CAIRO_DOCK_SHARE_DATA_DIR, CAIRO_DOCK_CONF_FILE, g_cCairoDockDataDir);
		system (cCommand);
		g_free (cCommand);
	}
	
	
	//\___________________ On lit le fichier de conf et on charge tout.
	cairo_dock_update_conf_file_with_modules (g_cConfFile, g_hModuleTable);
	cairo_dock_update_conf_file_with_translations (g_cConfFile, CAIRO_DOCK_SHARE_DATA_DIR);
	
	cairo_dock_read_conf_file (g_cConfFile, g_pMainDock);
	
	
	return NULL;
}
