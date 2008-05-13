/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif
#include <gtk/gtkgl.h>

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-application-factory.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-applet-factory.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-log.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-class-manager.h"
#include "cairo-dock-load.h"

extern CairoDock *g_pMainDock;
extern double g_fSubDockSizeRatio;
extern gboolean g_bSameHorizontality;

extern int g_iSinusoidWidth;
extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern gint g_iFrameMargin;
extern double g_fAmplitude;
extern int g_iIconGap;
extern double g_fAlbedo;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern gboolean g_bReverseVisibleImage;

extern int g_iLabelWeight;
extern int g_iLabelStyle;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;
extern double g_fLabelBackgroundColor[4];
extern gboolean g_bUseBackgroundForLabel;

extern gchar *g_cCurrentThemePath;

extern int g_iDockRadius;
extern int g_iDockLineWidth;

extern gchar *g_cBackgroundImageFile;
extern double g_fBackgroundImageAlpha;
extern cairo_surface_t *g_pBackgroundSurface[2];
extern cairo_surface_t *g_pBackgroundSurfaceFull[2];
extern double g_fBackgroundImageWidth, g_fBackgroundImageHeight;
extern gboolean g_bBackgroundImageRepeat;
extern int g_iNbStripes;
extern double g_fStripesAngle;
extern double g_fStripesWidth;
extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];

extern unsigned int g_iAppliMaxNameLength;

extern int g_tIconAuthorizedWidth[CAIRO_DOCK_NB_TYPES];
extern int g_tIconAuthorizedHeight[CAIRO_DOCK_NB_TYPES];
extern gboolean g_bOverWriteXIcons;

extern cairo_surface_t *g_pDropIndicatorSurface;
extern double g_fDropIndicatorWidth, g_fDropIndicatorHeight;

extern gboolean g_bUseGlitz;
extern gboolean g_bUseOpenGL;


gchar *cairo_dock_generate_file_path (gchar *cImageFile)
{
	g_return_val_if_fail (cImageFile != NULL, NULL);
	gchar *cImagePath;
	if (*cImageFile == '~')
	{
		cImagePath = g_strdup_printf ("%s%s", getenv("HOME"), cImageFile + 1);
	}
	else if (*cImageFile == '/')
	{
		cImagePath = g_strdup (cImageFile);
	}
	else
	{
		cImagePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, cImageFile);
	}
	return cImagePath;
}

cairo_surface_t *cairo_dock_load_image (cairo_t *pSourceContext, gchar *cImageFile, double *fImageWidth, double *fImageHeight, double fRotationAngle, double fAlpha, gboolean bReapeatAsPattern)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	cairo_surface_t *pNewSurface = NULL;

	if (cImageFile != NULL)
	{
		gchar *cImagePath = cairo_dock_generate_file_path (cImageFile);

		int iDesiredWidth = (int) (*fImageWidth), iDesiredHeight = (int) (*fImageHeight);
		pNewSurface = cairo_dock_create_surface_from_image (cImagePath,
			pSourceContext,
			1.,
			bReapeatAsPattern ? 0 : iDesiredWidth,  // pas de contrainte sur
			bReapeatAsPattern ? 0 : iDesiredHeight,  // la taille du motif initialement.
			fImageWidth,
			fImageHeight,
			FALSE);
		
		if (bReapeatAsPattern)
		{
			cairo_surface_t *pNewSurfaceFilled = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				iDesiredWidth,
				iDesiredHeight);
			cairo_t *pCairoContext = cairo_create (pNewSurfaceFilled);
	
			cairo_pattern_t* pPattern = cairo_pattern_create_for_surface (pNewSurface);
			cairo_pattern_set_extend (pPattern, CAIRO_EXTEND_REPEAT);
	
			cairo_set_source (pCairoContext, pPattern);
			cairo_paint (pCairoContext);
			cairo_destroy (pCairoContext);
	
			cairo_surface_destroy (pNewSurface);
			pNewSurface = pNewSurfaceFilled;
			*fImageWidth = iDesiredWidth;
			*fImageHeight = iDesiredHeight;
		}
		
		if (fAlpha < 1)
		{
			cairo_surface_t *pNewSurfaceAlpha = cairo_surface_create_similar (cairo_get_target (pSourceContext),
				CAIRO_CONTENT_COLOR_ALPHA,
				*fImageWidth,
				*fImageHeight);
			cairo_t *pCairoContext = cairo_create (pNewSurfaceAlpha);
	
			cairo_set_source_surface (pCairoContext, pNewSurface, 0, 0);
			cairo_paint_with_alpha (pCairoContext, fAlpha);
			cairo_destroy (pCairoContext);
	
			cairo_surface_destroy (pNewSurface);
			pNewSurface = pNewSurfaceAlpha;
		}
		
		if (fRotationAngle != 0)
		{
			cairo_surface_t *pNewSurfaceRotated = cairo_dock_rotate_surface (pNewSurface,
				pSourceContext,
				*fImageWidth,
				*fImageHeight,
				fRotationAngle);
			cairo_surface_destroy (pNewSurface);
			pNewSurface = pNewSurfaceRotated;
		}
		
		g_free (cImagePath);
	}
	
	return pNewSurface;
}

cairo_surface_t *cairo_dock_load_image_for_icon (cairo_t *pSourceContext, gchar *cImageFile, double fImageWidth, double fImageHeight)
{
	double fImageWidth_ = fImageWidth, fImageHeight_ = fImageHeight;
	return cairo_dock_load_image (pSourceContext, cImageFile, &fImageWidth_, &fImageHeight_, 0., 1., FALSE);
}


void cairo_dock_load_reflect_on_icon (Icon *icon, cairo_t *pSourceContext, gdouble fMaxScale, gboolean bHorizontalDock, gboolean bDirectionUp)
{
	if (g_fAlbedo > 0 && icon->pIconBuffer != NULL && ! (CAIRO_DOCK_IS_APPLET (icon) && icon->acFileName == NULL))
	{
		icon->pReflectionBuffer = cairo_dock_create_reflection_surface (icon->pIconBuffer,
			pSourceContext,
			(bHorizontalDock ? icon->fWidth : icon->fHeight) * fMaxScale,
			(bHorizontalDock ? icon->fHeight : icon->fWidth) * fMaxScale,
			bHorizontalDock,
			fMaxScale,
			bDirectionUp);

		icon->pFullIconBuffer = cairo_dock_create_icon_surface_with_reflection (icon->pIconBuffer,
			icon->pReflectionBuffer,
			pSourceContext,
			(bHorizontalDock ? icon->fWidth : icon->fHeight) * fMaxScale,
			(bHorizontalDock ? icon->fHeight : icon->fWidth) * fMaxScale,
			bHorizontalDock,
			fMaxScale,
			bDirectionUp);
	}
}

void cairo_dock_fill_one_icon_buffer (Icon *icon, cairo_t* pSourceContext, gdouble fMaxScale, gboolean bHorizontalDock, gboolean bApplySizeRestriction, gboolean bDirectionUp)
{
	//g_print ("%s (%d, %.2f, %s)\n", __func__, icon->iType, fMaxScale, icon->acFileName);
	cairo_surface_destroy (icon->pIconBuffer);
	icon->pIconBuffer = NULL;
	cairo_surface_destroy (icon->pReflectionBuffer);
	icon->pReflectionBuffer = NULL;
	cairo_surface_destroy (icon->pFullIconBuffer);
	icon->pFullIconBuffer = NULL;
	
	if (icon->fWidth < 0 || icon->fHeight < 0)  // on ne veut pas de surface.
		return;
	
	if (CAIRO_DOCK_IS_LAUNCHER (icon) || (CAIRO_DOCK_IS_USER_SEPARATOR (icon) && icon->acFileName != NULL))
	{
		//\_______________________ On recherche une icone.
		gchar *cIconPath = cairo_dock_search_icon_s_path (icon->acFileName);
		//g_print (" -> %s\n", cIconPath);

		//\_______________________ On cree la surface cairo a afficher.
		if (cIconPath != NULL && *cIconPath != '\0')
		{
			cairo_surface_t *pSurface =cairo_dock_create_surface_from_image (cIconPath,
			//icon->pIconBuffer = cairo_dock_create_surface_from_image (cIconPath,
				pSourceContext,
				fMaxScale,
				(bApplySizeRestriction ? g_tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] : icon->fWidth),
				(bApplySizeRestriction ? g_tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] : icon->fHeight),
				(bHorizontalDock ? &icon->fWidth : &icon->fHeight),
				(bHorizontalDock ? &icon->fHeight : &icon->fWidth),
				FALSE);
			//g_print (" => %.2fx%.2f\n", icon->fWidth, icon->fHeight);
			int w = (int) icon->fWidth * (1 + g_fAmplitude), h = (int) icon->fHeight * (1 + g_fAmplitude);
			g_free (icon->pSurfaceData);
			icon->pSurfaceData = g_new0 (guchar, w * h * 4);
			icon->pIconBuffer = cairo_image_surface_create_for_data (icon->pSurfaceData,
				CAIRO_FORMAT_ARGB32,
				w,
				h,
				w * 4);
			cd_message (" + (%dx%d)\n", w, h);
			
			if (icon->pCairoContext!= NULL)
				cairo_destroy (icon->pCairoContext);
			icon->pCairoContext = cairo_create (icon->pIconBuffer);
			cairo_save (icon->pCairoContext);
			cairo_set_source_surface (icon->pCairoContext, pSurface, 0.0, 0.0);
			cairo_paint (icon->pCairoContext);
			cairo_restore (icon->pCairoContext);
			cairo_surface_destroy (pSurface);
			
			if (g_bUseOpenGL)
			{
				GdkGLContext* pGlContext = gtk_widget_get_gl_context (g_pMainDock->pWidget);
				GdkGLDrawable* pGlDrawable = gtk_widget_get_gl_drawable (g_pMainDock->pWidget);
				if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
					return ;
				
				glGenTextures (1, &icon->iColorBuffer);
				glBindTexture (GL_TEXTURE_RECTANGLE_ARB, icon->iColorBuffer);  // GL_TEXTURE_2D
				glTexImage2D (GL_TEXTURE_RECTANGLE_ARB,  // GL_TEXTURE_2D
					0,
					GL_RGBA,  // GL_ALPHA
					w,
					h,
					0,
					GL_BGRA,  // GL_ALPHA
					GL_UNSIGNED_BYTE,
					icon->pSurfaceData);
				
				gdk_gl_drawable_gl_end (pGlDrawable);
			}
		}
		
		g_free (cIconPath);
	}
	else if (CAIRO_DOCK_IS_APPLET (icon))  // c'est l'icône d'une applet.
	{
		//g_print ("  icon->acFileName : %s\n", icon->acFileName);
		icon->pIconBuffer = cairo_dock_create_applet_surface (icon->acFileName, pSourceContext, fMaxScale, &icon->fWidth, &icon->fHeight, bApplySizeRestriction);
	}
	else if (CAIRO_DOCK_IS_APPLI (icon))  // c'est l'icône d'une appli valide. Dans cet ordre on n'a pas besoin de verifier que c'est NORMAL_APPLI.
	{
		if (g_bOverWriteXIcons && ! cairo_dock_class_is_using_xicon (icon->cClass))
			icon->pIconBuffer = cairo_dock_create_surface_from_class (icon->cClass, pSourceContext, fMaxScale, &icon->fWidth, &icon->fHeight);
		if (icon->pIconBuffer == NULL)
			icon->pIconBuffer = cairo_dock_create_surface_from_xwindow (icon->Xid, pSourceContext, fMaxScale, &icon->fWidth, &icon->fHeight);
	}
	else  // c'est une icone de separation.
	{
		icon->pIconBuffer = cairo_dock_create_separator_surface (pSourceContext, fMaxScale, bHorizontalDock, bDirectionUp, &icon->fWidth, &icon->fHeight);
	}

	if (icon->pIconBuffer == NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/%s", CAIRO_DOCK_SHARE_DATA_DIR, CAIRO_DOCK_DEFAULT_ICON_NAME);
		icon->pIconBuffer = cairo_dock_create_surface_from_image (cIconPath,
			pSourceContext,
			fMaxScale,
			(bApplySizeRestriction ? g_tIconAuthorizedWidth[icon->iType] : icon->fWidth),
			(bApplySizeRestriction ? g_tIconAuthorizedHeight[icon->iType] : icon->fHeight),
			(bHorizontalDock ? &icon->fWidth : &icon->fHeight),
			(bHorizontalDock ? &icon->fHeight : &icon->fWidth),
			FALSE);
		g_free (cIconPath);
	}
	cd_debug ("%s () -> %.2fx%.2f", __func__, icon->fWidth, icon->fHeight);

	if (g_fAlbedo > 0 && icon->pIconBuffer != NULL && ! (CAIRO_DOCK_IS_APPLET (icon) && icon->acFileName == NULL))
	{
		icon->pReflectionBuffer = cairo_dock_create_reflection_surface (icon->pIconBuffer,
			pSourceContext,
			(bHorizontalDock ? icon->fWidth : icon->fHeight) * fMaxScale,
			(bHorizontalDock ? icon->fHeight : icon->fWidth) * fMaxScale,
			bHorizontalDock,
			fMaxScale,
			bDirectionUp);

		icon->pFullIconBuffer = cairo_dock_create_icon_surface_with_reflection (icon->pIconBuffer,
			icon->pReflectionBuffer,
			pSourceContext,
			(bHorizontalDock ? icon->fWidth : icon->fHeight) * fMaxScale,
			(bHorizontalDock ? icon->fHeight : icon->fWidth) * fMaxScale,
			bHorizontalDock,
			fMaxScale,
			bDirectionUp);
	}
}

void cairo_dock_fill_one_text_buffer (Icon *icon, cairo_t* pSourceContext, int iLabelSize, gchar *cLabelPolice, gboolean bHorizontalDock, gboolean bDirectionUp)
{
	//g_print ("%s (%s, %d)\n", __func__, cLabelPolice, iLabelSize);
	cairo_surface_destroy (icon->pTextBuffer);
	icon->pTextBuffer = NULL;
	if (icon->acName == NULL || (iLabelSize == 0))
		return ;

	gchar *cTruncatedName = NULL;
	if (CAIRO_DOCK_IS_APPLI (icon) && g_iAppliMaxNameLength > 0)
	{
		//g_print ("troncature de %s\n", icon->acName);
		gsize bytes_read, bytes_written;
		GError *erreur = NULL;
		gchar *cUtf8Name = g_locale_to_utf8 (icon->acName,
			-1,
			&bytes_read,
			&bytes_written,
			&erreur);  // inutile sur Ubuntu, qui est nativement UTF8, mais sur les autres on ne sait pas.
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		if (cUtf8Name == NULL)  // une erreur s'est produite, on tente avec la chaine brute.
			cUtf8Name = g_strdup (icon->acName);

		const gchar *cEndValidChain = NULL;
		if (g_utf8_validate (cUtf8Name, -1, &cEndValidChain))
		{
			if (g_utf8_strlen (cUtf8Name, -1) > g_iAppliMaxNameLength)
			{
				cTruncatedName = g_new0 (gchar, 8 * (g_iAppliMaxNameLength + 4));  // 8 octets par caractere.
				g_utf8_strncpy (cTruncatedName, cUtf8Name, g_iAppliMaxNameLength);

				gchar *cTruncature = g_utf8_offset_to_pointer (cTruncatedName, g_iAppliMaxNameLength);
				*cTruncature = '.';
				*(cTruncature+1) = '.';
				*(cTruncature+2) = '.';
			}
		}
		else
		{
			if (strlen (icon->acName) > g_iAppliMaxNameLength)
			{
				cTruncatedName = g_new0 (gchar, g_iAppliMaxNameLength + 4);
				strncpy (cTruncatedName, icon->acName, g_iAppliMaxNameLength);

				cTruncatedName[g_iAppliMaxNameLength] = '.';
				cTruncatedName[g_iAppliMaxNameLength+1] = '.';
				cTruncatedName[g_iAppliMaxNameLength+2] = '.';
			}
		}
		g_free (cUtf8Name);
		//g_print (" -> etiquette : %s\n", cTruncatedName);
	}

	cairo_surface_t* pNewSurface = cairo_dock_create_surface_from_text ((cTruncatedName != NULL ? cTruncatedName : icon->acName), pSourceContext, iLabelSize, cLabelPolice, g_iLabelWeight, (g_bUseBackgroundForLabel ? g_fLabelBackgroundColor : NULL), 1., &icon->iTextWidth, &icon->iTextHeight, &icon->fTextXOffset, &icon->fTextYOffset);
	g_free (cTruncatedName);
	//g_print (" -> %s : (%.2f;%.2f) %dx%d\n", icon->acName, icon->fTextXOffset, icon->fTextYOffset, icon->iTextWidth, icon->iTextHeight);

	double fRotationAngle = (bHorizontalDock ? 0 : (bDirectionUp ? -G_PI/2 : G_PI/2));
	cairo_surface_t *pNewSurfaceRotated = cairo_dock_rotate_surface (pNewSurface, pSourceContext, icon->iTextWidth, icon->iTextHeight, fRotationAngle);
	if (pNewSurfaceRotated != NULL)
	{
		cairo_surface_destroy (pNewSurface);
		pNewSurface = pNewSurfaceRotated;
	}

	icon->pTextBuffer = pNewSurface;
}

void cairo_dock_fill_one_quick_info_buffer (Icon *icon, cairo_t* pSourceContext, int iLabelSize, gchar *cLabelPolice, int iLabelWeight, double fMaxScale)
{
	cairo_surface_destroy (icon->pQuickInfoBuffer);
	icon->pQuickInfoBuffer = NULL;
	if (icon->cQuickInfo == NULL)
		return ;

	icon->pQuickInfoBuffer = cairo_dock_create_surface_from_text (icon->cQuickInfo, pSourceContext, iLabelSize, cLabelPolice, iLabelWeight, g_fLabelBackgroundColor, fMaxScale, &icon->iQuickInfoWidth, &icon->iQuickInfoHeight, &icon->fQuickInfoXOffset, &icon->fQuickInfoYOffset);
}



void cairo_dock_fill_icon_buffers (Icon *icon, cairo_t *pSourceContext, double fMaxScale, gboolean bHorizontalDock, gboolean bApplySizeRestriction, gboolean bDirectionUp)
{
	cairo_dock_fill_one_icon_buffer (icon, pSourceContext, fMaxScale, bHorizontalDock, bApplySizeRestriction, bDirectionUp);

	cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : bHorizontalDock), bDirectionUp);

	cairo_dock_fill_one_quick_info_buffer (icon, pSourceContext, 12, g_cLabelPolice, PANGO_WEIGHT_HEAVY, fMaxScale);
}

void cairo_dock_load_one_icon_from_scratch (Icon *pIcon, CairoContainer *pContainer)
{
	g_return_if_fail (pIcon != NULL);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pContainer));
	if (CAIRO_DOCK_IS_DOCK (pContainer))
	{
		CairoDock *pDock = CAIRO_DOCK (pContainer);
		cairo_dock_fill_icon_buffers_for_dock (pIcon, pCairoContext, pDock);
	}
	else
	{
		cairo_dock_fill_icon_buffers_for_desklet (pIcon, pCairoContext);
	}
	cairo_destroy (pCairoContext);
}

void cairo_dock_reload_buffers_in_dock (gchar *cDockName, CairoDock *pDock, gpointer data)
{
	gboolean bReloadAppletsToo = GPOINTER_TO_INT (data);
	cd_message ("%s (%s, %d)", __func__, cDockName, bReloadAppletsToo);
	if (pDock->iRefCount > 0)
		pDock->bHorizontalDock = (g_bSameHorizontality ? g_pMainDock->bHorizontalDock : ! g_pMainDock->bHorizontalDock);
	//else
	//	pDock->bHorizontalDock = g_pMainDock->bHorizontalDock;

	double fFlatDockWidth = - g_iIconGap;
	pDock->iMaxIconHeight = 0;

	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	///double fMaxScale = 1 + g_fAmplitude;

	Icon* icon;
	GList* ic;
	//double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		if (CAIRO_DOCK_IS_APPLET (icon))
		{
			if (bReloadAppletsToo)
				cairo_dock_reload_module (icon->pModule, FALSE);
		}
		else
		{
			icon->fWidth /= pDock->fRatio;
			icon->fHeight /= pDock->fRatio;
			cairo_dock_fill_icon_buffers_for_dock (icon, pCairoContext, pDock);
			icon->fWidth *= pDock->fRatio;
			icon->fHeight *= pDock->fRatio;
		}
		
		//g_print (" =size <- %.2fx%.2f\n", icon->fWidth, icon->fHeight);
		fFlatDockWidth += g_iIconGap + icon->fWidth;
		pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, icon->fHeight);
	}
	pDock->fFlatDockWidth = (int) fFlatDockWidth;  /// (int) n'est plus tellement necessaire ...
	cairo_destroy (pCairoContext);
}


void cairo_dock_load_visible_zone (CairoDock *pDock, gchar *cVisibleZoneImageFile, int iVisibleZoneWidth, int iVisibleZoneHeight, double fVisibleZoneAlpha)
{
	double fVisibleZoneWidth = iVisibleZoneWidth, fVisibleZoneHeight = iVisibleZoneHeight;
	cairo_surface_destroy (g_pVisibleZoneSurface);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_pVisibleZoneSurface = cairo_dock_load_image (pCairoContext,
		cVisibleZoneImageFile,
		&fVisibleZoneWidth,
		&fVisibleZoneHeight,
		(pDock->bHorizontalDock ? (! pDock->bDirectionUp && g_bReverseVisibleImage ? G_PI : 0) : (pDock->bDirectionUp ? -G_PI/2 : G_PI/2)),
		fVisibleZoneAlpha,
		FALSE);
	cairo_destroy (pCairoContext);
}

cairo_surface_t *cairo_dock_load_stripes (cairo_t* pSourceContext, int iStripesWidth, int iStripesHeight, double fRotationAngle)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	cairo_pattern_t *pStripesPattern;
	double fWidth = (g_iNbStripes > 0 ? 200. : iStripesWidth);
	if (fabs (g_fStripesAngle) != 90)
		pStripesPattern = cairo_pattern_create_linear (0.0f,
			0.0f,
			fWidth,
			fWidth * tan (g_fStripesAngle * G_PI/180.));
	else
		pStripesPattern = cairo_pattern_create_linear (0.0f,
			0.0f,
			0.,
			(g_fStripesAngle == 90) ? iStripesHeight : - iStripesHeight);
	g_return_val_if_fail (cairo_pattern_status (pStripesPattern) == CAIRO_STATUS_SUCCESS, NULL);

	cairo_pattern_set_extend (pStripesPattern, CAIRO_EXTEND_REPEAT);

	if (g_iNbStripes > 0)
	{
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
	}
	else
	{
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			0.,
			g_fStripesColorDark[0],
			g_fStripesColorDark[1],
			g_fStripesColorDark[2],
			g_fStripesColorDark[3]);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			1.,
			g_fStripesColorBright[0],
			g_fStripesColorBright[1],
			g_fStripesColorBright[2],
			g_fStripesColorBright[3]);
	}

	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		iStripesWidth,
		iStripesHeight);
	cairo_t *pImageContext = cairo_create (pNewSurface);
	cairo_set_source (pImageContext, pStripesPattern);
	cairo_paint (pImageContext);

	cairo_pattern_destroy (pStripesPattern);
	cairo_destroy (pImageContext);

	if (fRotationAngle != 0)
	{
		cairo_surface_t *pNewSurfaceRotated = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			iStripesHeight,
			iStripesWidth);
		pImageContext = cairo_create (pNewSurfaceRotated);

		if (fRotationAngle < 0)
		{
			cairo_move_to (pImageContext, iStripesWidth, 0);
			cairo_rotate (pImageContext, fRotationAngle);
			cairo_translate (pImageContext, - iStripesWidth, 0);
		}
		else
		{
			cairo_move_to (pImageContext, 0, 0);
			cairo_rotate (pImageContext, fRotationAngle);
			cairo_translate (pImageContext, 0, - iStripesHeight);
		}
		cairo_set_source_surface (pImageContext, pNewSurface, 0, 0);

		cairo_paint (pImageContext);
		cairo_surface_destroy (pNewSurface);
		cairo_destroy (pImageContext);
		pNewSurface = pNewSurfaceRotated;
	}

	return pNewSurface;
}



void cairo_dock_update_background_decorations_if_necessary (CairoDock *pDock, int iNewDecorationsWidth, int iNewDecorationsHeight)
{
	//g_print ("%s (%dx%d) [%.2fx%.2f]\n", __func__, iNewDecorationsWidth, iNewDecorationsHeight, g_fBackgroundImageWidth, g_fBackgroundImageHeight);
	if (2 * iNewDecorationsWidth > g_fBackgroundImageWidth || iNewDecorationsHeight > g_fBackgroundImageHeight)
	{
		cairo_surface_destroy (g_pBackgroundSurface[0]);
		g_pBackgroundSurface[0] = NULL;
		cairo_surface_destroy (g_pBackgroundSurface[1]);
		g_pBackgroundSurface[1] = NULL;
		cairo_surface_destroy (g_pBackgroundSurfaceFull[0]);
		g_pBackgroundSurfaceFull[0] = NULL;
		cairo_surface_destroy (g_pBackgroundSurfaceFull[1]);
		g_pBackgroundSurfaceFull[1] = NULL;
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
		
		if (g_cBackgroundImageFile != NULL)
		{
			if (g_bBackgroundImageRepeat)
			{
				g_fBackgroundImageWidth = MAX (g_fBackgroundImageWidth, 2 * iNewDecorationsWidth);
				g_fBackgroundImageHeight = MAX (g_fBackgroundImageHeight, iNewDecorationsHeight);
				g_pBackgroundSurfaceFull[CAIRO_DOCK_HORIZONTAL] = cairo_dock_load_image (pCairoContext,
					g_cBackgroundImageFile,
					&g_fBackgroundImageWidth,
					&g_fBackgroundImageHeight,
					0,  // (pDock->bHorizontalDock ? 0 : (pDock->bDirectionUp ? -G_PI/2 : G_PI/2)),
					g_fBackgroundImageAlpha,
					g_bBackgroundImageRepeat);
					
				g_pBackgroundSurfaceFull[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (g_pBackgroundSurfaceFull[CAIRO_DOCK_HORIZONTAL], pCairoContext, g_fBackgroundImageWidth, g_fBackgroundImageHeight, (pDock->bDirectionUp ? -G_PI/2 : G_PI/2));
			}
			else/** if (g_fBackgroundImageWidth == 0 || g_fBackgroundImageHeight == 0)*/
			{
				g_fBackgroundImageWidth = MAX (g_fBackgroundImageWidth, iNewDecorationsWidth);  /// 0
				g_fBackgroundImageHeight = MAX (g_fBackgroundImageHeight, iNewDecorationsHeight);
				g_pBackgroundSurface[CAIRO_DOCK_HORIZONTAL] = cairo_dock_load_image (pCairoContext,
					g_cBackgroundImageFile,
					&g_fBackgroundImageWidth,
					&g_fBackgroundImageHeight,
					0,  // (pDock->bHorizontalDock ? 0 : (pDock->bDirectionUp ? -G_PI/2 : G_PI/2)),
					g_fBackgroundImageAlpha,
					g_bBackgroundImageRepeat);
					
				g_pBackgroundSurface[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (g_pBackgroundSurface[CAIRO_DOCK_HORIZONTAL], pCairoContext, g_fBackgroundImageWidth, g_fBackgroundImageHeight, (pDock->bDirectionUp ? -G_PI/2 : G_PI/2));
			}
		}
		else
		{
			g_fBackgroundImageWidth = MAX (g_fBackgroundImageWidth, 2 * iNewDecorationsWidth);
			g_fBackgroundImageHeight = MAX (g_fBackgroundImageHeight, iNewDecorationsHeight);
			g_pBackgroundSurfaceFull[CAIRO_DOCK_HORIZONTAL] = cairo_dock_load_stripes (pCairoContext, g_fBackgroundImageWidth, g_fBackgroundImageHeight, 0);  // (pDock->bHorizontalDock ? 0 : (pDock->bDirectionUp ? -G_PI/2 : G_PI/2))
			
			g_pBackgroundSurfaceFull[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (g_pBackgroundSurfaceFull[CAIRO_DOCK_HORIZONTAL], pCairoContext, g_fBackgroundImageWidth, g_fBackgroundImageHeight, (pDock->bDirectionUp ? -G_PI/2 : G_PI/2));
		}
		
		cairo_destroy (pCairoContext);
		cd_debug ("  MaJ des decorations du fond -> %.2fx%.2f", g_fBackgroundImageWidth, g_fBackgroundImageHeight);
	}
}


void cairo_dock_load_background_decorations (CairoDock *pDock)
{
	int iWidth, iHeight;
	cairo_dock_search_max_decorations_size (&iWidth, &iHeight);
	
	g_fBackgroundImageWidth = 0;
	g_fBackgroundImageHeight = 0;
	cairo_dock_update_background_decorations_if_necessary (pDock, iWidth, iHeight);
}


void cairo_dock_load_drop_indicator (gchar *cImagePath, cairo_t* pSourceContext, double fMaxScale)
{
	if (g_pDropIndicatorSurface != NULL)
		cairo_surface_destroy (g_pDropIndicatorSurface);
	g_pDropIndicatorSurface = cairo_dock_create_surface_from_image (cImagePath,
		pSourceContext,
		1.,
		g_tIconAuthorizedWidth[CAIRO_DOCK_LAUNCHER] * fMaxScale,
		g_tIconAuthorizedHeight[CAIRO_DOCK_LAUNCHER] * fMaxScale / 2,
		&g_fDropIndicatorWidth, &g_fDropIndicatorHeight,
		TRUE);
}