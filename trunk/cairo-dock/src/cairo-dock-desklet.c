/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
** Login : <ctaf42@gmail.com>
** Started on  Sun Jan 27 18:35:38 2008 Cedric GESTES
** $Id$
**
** Author(s)
**  - Cedric GESTES <ctaf42@gmail.com>
**  - Fabrice REY
**
** Copyright (C) 2008 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtkgl.h>
#include <GL/gl.h> 
#include <GL/glu.h> 
#include <GL/glx.h> 
#include <gdk/x11/gdkglx.h>

#include <gdk/gdkx.h>
#include "cairo-dock-draw.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-config.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-log.h"
#include "cairo-dock-menu.h"
#include "cairo-dock-container.h"
#include "cairo-dock-X-utilities.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-draw-opengl.h"
#include "cairo-dock-load.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-internal-desklets.h"
#include "cairo-dock-internal-system.h"
#include "cairo-dock-internal-background.h"
#include "cairo-dock-gui-manager.h"
#include "cairo-dock-desklet.h"

extern CairoDock *g_pMainDock;
extern int g_iXScreenWidth[2], g_iXScreenHeight[2];
extern gchar *g_cConfFile;
extern gboolean g_bSticky;
extern gboolean g_bUseOpenGL;
extern gboolean g_bIndirectRendering;
extern GdkGLConfig* g_pGlConfig;

static cairo_surface_t *s_pRotateButtonSurface = NULL;
static cairo_surface_t *s_pRetachButtonSurface = NULL;
static cairo_surface_t *s_pDepthRotateButtonSurface = NULL;
static GLuint s_iRotateButtonTexture = 0;
static GLuint s_iRetachButtonTexture = 0;
static GLuint s_iDepthRotateButtonTexture = 0;

#define CD_NB_ITER_FOR_GRADUATION 10

void cairo_dock_load_desklet_buttons (cairo_t *pSourceContext)
{
	if (s_pRotateButtonSurface != NULL)
	{
		cairo_surface_destroy (s_pRotateButtonSurface);
		s_pRotateButtonSurface = NULL;
	}
	if (myDesklets.cRotateButtonImage != NULL)
	{
		s_pRotateButtonSurface = cairo_dock_create_surface_from_image_simple (myDesklets.cRotateButtonImage,
			pSourceContext,
			myDesklets.iDeskletButtonSize,
			myDesklets.iDeskletButtonSize);
	}
	if (s_pRotateButtonSurface == NULL)
	{
		s_pRotateButtonSurface = cairo_dock_create_surface_from_image_simple (CAIRO_DOCK_SHARE_DATA_DIR"/rotate-desklet.svg",
			pSourceContext,
			myDesklets.iDeskletButtonSize,
			myDesklets.iDeskletButtonSize);
	}
	
	if (s_pRetachButtonSurface != NULL)
	{
		cairo_surface_destroy (s_pRetachButtonSurface);
		s_pRetachButtonSurface = NULL;
	}
	if (myDesklets.cRetachButtonImage != NULL)
	{
		s_pRetachButtonSurface = cairo_dock_create_surface_from_image_simple (myDesklets.cRetachButtonImage,
			pSourceContext,
			myDesklets.iDeskletButtonSize,
			myDesklets.iDeskletButtonSize);
	}
	if (s_pRetachButtonSurface == NULL)
	{
		s_pRetachButtonSurface = cairo_dock_create_surface_from_image_simple (CAIRO_DOCK_SHARE_DATA_DIR"/retach-desklet.svg",
			pSourceContext,
			myDesklets.iDeskletButtonSize,
			myDesklets.iDeskletButtonSize);
	}

	if (s_pDepthRotateButtonSurface != NULL)
	{
		cairo_surface_destroy (s_pDepthRotateButtonSurface);
		s_pDepthRotateButtonSurface = NULL;
	}
	if (myDesklets.cDepthRotateButtonImage != NULL)
	{
		s_pDepthRotateButtonSurface = cairo_dock_create_surface_from_image_simple (myDesklets.cDepthRotateButtonImage,
			pSourceContext,
			myDesklets.iDeskletButtonSize,
			myDesklets.iDeskletButtonSize);
	}
	if (s_pDepthRotateButtonSurface == NULL)
	{
		s_pDepthRotateButtonSurface = cairo_dock_create_surface_from_image_simple (CAIRO_DOCK_SHARE_DATA_DIR"/depth-rotate-desklet.svg",
			pSourceContext,
			myDesklets.iDeskletButtonSize,
			myDesklets.iDeskletButtonSize);
	}
	
	cairo_dock_load_desklet_buttons_texture ();
}

void cairo_dock_load_desklet_buttons_texture (void)
{
	cd_message ("%s (%x;%x)", __func__, s_pRotateButtonSurface, s_pRetachButtonSurface);
	if (! g_bUseOpenGL)
		return ;
	
	cairo_dock_unload_desklet_buttons_texture ();

	if (s_pRotateButtonSurface != NULL)
	{
		s_iRotateButtonTexture = cairo_dock_create_texture_from_surface (s_pRotateButtonSurface);
	}
	if (s_pRetachButtonSurface != NULL)
	{
		s_iRetachButtonTexture = cairo_dock_create_texture_from_surface (s_pRetachButtonSurface);
	}
	if (s_pDepthRotateButtonSurface != NULL)
	{
		s_iDepthRotateButtonTexture = cairo_dock_create_texture_from_surface (s_pDepthRotateButtonSurface);
	}
}

void cairo_dock_unload_desklet_buttons_texture (void)
{
	if (s_iRotateButtonTexture != 0)
	{
		_cairo_dock_delete_texture (s_iRotateButtonTexture);
		s_iRotateButtonTexture = 0;
	}
	if (s_iRetachButtonTexture != 0)
	{
		_cairo_dock_delete_texture (s_iRetachButtonTexture);
		s_iRetachButtonTexture = 0;
	}
	if (s_iDepthRotateButtonTexture != 0)
	{
		_cairo_dock_delete_texture (s_iDepthRotateButtonTexture);
		s_iDepthRotateButtonTexture = 0;
	}
}

static inline double _compute_zoom_for_rotation (CairoDesklet *pDesklet)
{
	double w = pDesklet->iWidth/2, h = pDesklet->iHeight/2;
	double alpha = atan2 (h, w);
	double theta = fabs (pDesklet->fRotation);
	if (theta > G_PI/2)
		theta -= G_PI/2;
	
	double d = sqrt (w * w + h * h);
	double xmax = d * MAX (fabs (cos (alpha + theta)), fabs (cos (alpha - theta)));
	double ymax = d * MAX (fabs (sin (alpha + theta)), fabs (sin (alpha - theta)));
	double fZoom = MIN (w / xmax, h / ymax);
	return fZoom;
}

static void _cairo_dock_render_desklet (CairoDesklet *pDesklet, GdkRectangle *area)
{
	gint w = 0, h = 0;
	cairo_t *pCairoContext;
	double fColor[4] = {1., 1., 1., 0.};
	if (! gtk_window_is_active (GTK_WINDOW (pDesklet->pWidget)))
		fColor[3] = 0.;
	else
		fColor[3] = 1.*pDesklet->iGradationCount / CD_NB_ITER_FOR_GRADUATION;
	
	gboolean bRenderOptimized = (area->x > 0 || area->y > 0);
	/*if (bRenderOptimized)
	{
		//g_print ("Using optimized render\n");
		pCairoContext = cairo_dock_create_drawing_context_on_area (CAIRO_CONTAINER (pDesklet), area, fColor);
	}
	else*/
	{
		//cd_debug ("Using normal render");
		pCairoContext = cairo_dock_create_drawing_context (CAIRO_CONTAINER (pDesklet));
	}
	
	if (pDesklet->fZoom != 1)
	{
		//g_print (" desklet zoom : %.2f (%dx%d)\n", pDesklet->fZoom, pDesklet->iWidth, pDesklet->iHeight);
		cairo_translate (pCairoContext,
			pDesklet->iWidth * (1 - pDesklet->fZoom)/2,
			pDesklet->iHeight * (1 - pDesklet->fZoom)/2);
		cairo_scale (pCairoContext, pDesklet->fZoom, pDesklet->fZoom);
	}
	
	if (fColor[3] != 0)
	{
		cairo_save (pCairoContext);
		cairo_pattern_t *pPattern = cairo_pattern_create_radial (.5*pDesklet->iWidth,
			.5*pDesklet->iHeight,
			0.,
			.5*pDesklet->iWidth,
			.5*pDesklet->iHeight,
			.5*MIN (pDesklet->iWidth, pDesklet->iHeight));
		cairo_pattern_set_extend (pPattern, CAIRO_EXTEND_NONE);
		
		cairo_pattern_add_color_stop_rgba   (pPattern,
			0.,
			fColor[0], fColor[1], fColor[2], fColor[3]);
		cairo_pattern_add_color_stop_rgba   (pPattern,
			1.,
			fColor[0], fColor[1], fColor[2], 0.);
		cairo_set_source (pCairoContext, pPattern);
		cairo_paint (pCairoContext);
		cairo_pattern_destroy (pPattern);
		cairo_restore (pCairoContext);
	}
	
	cairo_save (pCairoContext);
	
	if (pDesklet->fRotation != 0)
	{
		double fZoom = _compute_zoom_for_rotation (pDesklet);
		
		cairo_translate (pCairoContext,
			.5*pDesklet->iWidth,
			.5*pDesklet->iHeight);
		
		cairo_rotate (pCairoContext, pDesklet->fRotation);
		
		cairo_scale (pCairoContext, fZoom, fZoom);
		
		cairo_translate (pCairoContext,
			-.5*pDesklet->iWidth,
			-.5*pDesklet->iHeight);
	}
	
	cairo_save (pCairoContext);
	if (pDesklet->pBackGroundSurface != NULL && pDesklet->fBackGroundAlpha != 0)
	{
		cairo_set_source_surface (pCairoContext,
			pDesklet->pBackGroundSurface,
			0.,
			0.);
		if (pDesklet->fBackGroundAlpha == 1)
			cairo_paint (pCairoContext);
		else
			cairo_paint_with_alpha (pCairoContext, pDesklet->fBackGroundAlpha);
	}
	
	if (pDesklet->iLeftSurfaceOffset != 0 || pDesklet->iTopSurfaceOffset != 0 || pDesklet->iRightSurfaceOffset != 0 || pDesklet->iBottomSurfaceOffset != 0)
	{
		cairo_translate (pCairoContext, pDesklet->iLeftSurfaceOffset, pDesklet->iTopSurfaceOffset);
		cairo_scale (pCairoContext,
			1. - 1.*(pDesklet->iLeftSurfaceOffset + pDesklet->iRightSurfaceOffset) / pDesklet->iWidth,
			1. - 1.*(pDesklet->iTopSurfaceOffset + pDesklet->iBottomSurfaceOffset) / pDesklet->iHeight);
	}
	
	if (pDesklet->pRenderer != NULL && pDesklet->pRenderer->render != NULL)  // un moteur de rendu specifique a ete fourni.
	{
		pDesklet->pRenderer->render (pCairoContext, pDesklet, bRenderOptimized);
	}
	
	cairo_restore (pCairoContext);
	if (pDesklet->pForeGroundSurface != NULL && pDesklet->fForeGroundAlpha != 0)
	{
		cairo_set_source_surface (pCairoContext,
			pDesklet->pForeGroundSurface,
			0.,
			0.);
		if (pDesklet->fForeGroundAlpha == 1)
			cairo_paint (pCairoContext);
		else
			cairo_paint_with_alpha (pCairoContext, pDesklet->fForeGroundAlpha);
	}
	
	if ((pDesklet->bInside || pDesklet->rotating) && ! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude)
	{
		if (! pDesklet->rotating)
			cairo_restore (pCairoContext);
		if (s_pRotateButtonSurface != NULL)
		{
			cairo_set_source_surface (pCairoContext, s_pRotateButtonSurface, 0., 0.);
			cairo_paint (pCairoContext);
		}
		if (s_pRetachButtonSurface != NULL)
		{
			cairo_set_source_surface (pCairoContext, s_pRetachButtonSurface, pDesklet->iWidth - myDesklets.iDeskletButtonSize, 0.);
			cairo_paint (pCairoContext);
		}
	}

	cairo_destroy (pCairoContext);
}
static inline void _cairo_dock_set_desklet_matrix (CairoDesklet *pDesklet)
{
	glTranslatef (0., 0., -pDesklet->iHeight * sqrt(3)/2 - 
		.45 * MAX (pDesklet->iWidth * fabs (sin (pDesklet->fDepthRotationY)),
			pDesklet->iHeight * fabs (sin (pDesklet->fDepthRotationX)))
		);  // avec 60 deg de perspective
	
	if (pDesklet->fZoom != 1)
	{
		glScalef (pDesklet->fZoom, pDesklet->fZoom, 1.);
	}
	
	if (pDesklet->fRotation != 0)
	{
		double fZoom = _compute_zoom_for_rotation (pDesklet);
		glScalef (fZoom, fZoom, 1.);
		glRotatef (- pDesklet->fRotation / G_PI * 180., 0., 0., 1.);
	}
	
	if (pDesklet->fDepthRotationY != 0)
	{
		glRotatef (- pDesklet->fDepthRotationY / G_PI * 180., 0., 1., 0.);
	}
	
	if (pDesklet->fDepthRotationX != 0)
	{
		glRotatef (- pDesklet->fDepthRotationX / G_PI * 180., 1., 0., 0.);
	}
}

gboolean cairo_dock_render_desklet_notification (gpointer pUserData, CairoDesklet *pDesklet)
{
	glPushMatrix ();
	///glTranslatef (0*pDesklet->iWidth/2, 0*pDesklet->iHeight/2, 0.);  // avec une perspective ortho.
	///glTranslatef (0*pDesklet->iWidth/2, 0*pDesklet->iHeight/2, -pDesklet->iWidth*(1.87 +.35*fabs (sin(pDesklet->fDepthRotationY))));  // avec 30 deg de perspective
	_cairo_dock_set_desklet_matrix (pDesklet);
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_pbuffer ();
	
	if (pDesklet->iBackGroundTexture != 0 && pDesklet->fBackGroundAlpha != 0)
	{
		_cairo_dock_apply_texture_at_size_with_alpha (pDesklet->iBackGroundTexture, pDesklet->iWidth, pDesklet->iHeight, pDesklet->fBackGroundAlpha);
	}
	
	glPushMatrix ();
	if (pDesklet->iLeftSurfaceOffset != 0 || pDesklet->iTopSurfaceOffset != 0 || pDesklet->iRightSurfaceOffset != 0 || pDesklet->iBottomSurfaceOffset != 0)
	{
		glTranslatef ((pDesklet->iLeftSurfaceOffset - pDesklet->iRightSurfaceOffset)/2, (pDesklet->iBottomSurfaceOffset - pDesklet->iTopSurfaceOffset)/2, 0.);
		glScalef (1. - 1.*(pDesklet->iLeftSurfaceOffset + pDesklet->iRightSurfaceOffset) / pDesklet->iWidth,
			1. - 1.*(pDesklet->iTopSurfaceOffset + pDesklet->iBottomSurfaceOffset) / pDesklet->iHeight,
			1.);
	}
	
	if (pDesklet->pRenderer != NULL && pDesklet->pRenderer->render_opengl != NULL)  // un moteur de rendu specifique a ete fourni.
	{
		_cairo_dock_set_alpha (1.);
		pDesklet->pRenderer->render_opengl (pDesklet);
	}
	glPopMatrix ();
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_pbuffer ();
	if (pDesklet->iForeGroundTexture != 0 && pDesklet->fForeGroundAlpha != 0)
	{
		_cairo_dock_apply_texture_at_size_with_alpha (pDesklet->iForeGroundTexture, pDesklet->iWidth, pDesklet->iHeight, pDesklet->fForeGroundAlpha);
	}
	
	if (pDesklet->bInside && ! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude)
	{
		if (! pDesklet->rotating && ! pDesklet->rotatingY && ! pDesklet->rotatingX)
		{
			glPopMatrix ();
			glPushMatrix ();
			glTranslatef (0., 0., -pDesklet->iHeight*(sqrt(3)/2));
		}
	}
	
	if ((pDesklet->bInside || pDesklet->rotating || pDesklet->rotatingY || pDesklet->rotatingX) && ! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude)
	{
		_cairo_dock_set_alpha (1.);
		if (s_iRotateButtonTexture != 0)
		{
			glPushMatrix ();
			glTranslatef (-pDesklet->iWidth/2 + myDesklets.iDeskletButtonSize/2,
				pDesklet->iHeight/2 - myDesklets.iDeskletButtonSize/2,
				0.);
			_cairo_dock_apply_texture_at_size (s_iRotateButtonTexture, myDesklets.iDeskletButtonSize, myDesklets.iDeskletButtonSize);
			glPopMatrix ();
		}
		if (s_iRetachButtonTexture != 0)
		{
			glPushMatrix ();
			glTranslatef (pDesklet->iWidth/2 - myDesklets.iDeskletButtonSize/2,
				pDesklet->iHeight/2 - myDesklets.iDeskletButtonSize/2,
				0.);
			_cairo_dock_apply_texture_at_size (s_iRetachButtonTexture, myDesklets.iDeskletButtonSize, myDesklets.iDeskletButtonSize);
			glPopMatrix ();
		}
		if (s_iDepthRotateButtonTexture != 0)
		{
			glPushMatrix ();
			glTranslatef (0.,
				pDesklet->iHeight/2 - myDesklets.iDeskletButtonSize/2,
				0.);
			_cairo_dock_apply_texture_at_size (s_iDepthRotateButtonTexture, myDesklets.iDeskletButtonSize, myDesklets.iDeskletButtonSize);
			glPopMatrix ();
			
			glPushMatrix ();
			glRotatef (90., 0., 0., 1.);
			glTranslatef (0.,
				pDesklet->iWidth/2 - myDesklets.iDeskletButtonSize/2,
				0.);
			_cairo_dock_apply_texture_at_size (s_iDepthRotateButtonTexture, myDesklets.iDeskletButtonSize, myDesklets.iDeskletButtonSize);
			glPopMatrix ();
		}
	}
	
	_cairo_dock_disable_texture ();
	
	glPopMatrix ();
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
static void _cairo_dock_render_desklet_opengl (CairoDesklet *pDesklet)
{
	GdkGLContext *pGlContext = gtk_widget_get_gl_context (pDesklet->pWidget);
	GdkGLDrawable *pGlDrawable = gtk_widget_get_gl_drawable (pDesklet->pWidget);
	if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
		return ;
	
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity ();
	
	cairo_dock_apply_desktop_background (CAIRO_CONTAINER (pDesklet));
	
	cairo_dock_notify_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_RENDER_DESKLET, pDesklet);
	
	if (gdk_gl_drawable_is_double_buffered (pGlDrawable))
		gdk_gl_drawable_swap_buffers (pGlDrawable);
	else
		glFlush ();
	gdk_gl_drawable_gl_end (pGlDrawable);
}
static gboolean on_expose_desklet(GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoDesklet *pDesklet)
{
	if (pDesklet->iDesiredWidth != 0 && pDesklet->iDesiredHeight != 0 && (pDesklet->iKnownWidth != pDesklet->iDesiredWidth || pDesklet->iKnownHeight != pDesklet->iDesiredHeight))
	{
		//g_print ("on saute le dessin\n");
		if (g_bUseOpenGL)
		{
			// dessiner la fausse transparence.
		}
		else
		{
			cairo_t *pCairoContext = cairo_dock_create_drawing_context (CAIRO_CONTAINER (pDesklet));
			cairo_destroy (pCairoContext);
		}
		return FALSE;
	}
	
	pDesklet->iDesiredWidth = 0;  /// ???
	pDesklet->iDesiredHeight = 0;
		
	if (g_bUseOpenGL && pDesklet->pRenderer && pDesklet->pRenderer->render_opengl)
	{
		_cairo_dock_render_desklet_opengl (pDesklet);
	}
	else
	{
		_cairo_dock_render_desklet (pDesklet, &pExpose->area);
	}
	
	return FALSE;
}


static gboolean _cairo_dock_write_desklet_size (CairoDesklet *pDesklet)
{
	if ((pDesklet->iDesiredWidth == 0 && pDesklet->iDesiredHeight == 0) && pDesklet->pIcon != NULL && pDesklet->pIcon->pModuleInstance != NULL)
	{
		gchar *cSize = g_strdup_printf ("%d;%d", pDesklet->iWidth, pDesklet->iHeight);
		cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
			G_TYPE_STRING, "Desklet", "size", cSize,
			G_TYPE_INVALID);
		g_free (cSize);
		cairo_dock_update_desklet_size_in_gui (pDesklet->pIcon->pModuleInstance->pModule->pVisitCard->cModuleName,
			pDesklet->iWidth,
			pDesklet->iHeight);
	}
	pDesklet->iSidWriteSize = 0;
	pDesklet->iKnownWidth = pDesklet->iWidth;
	pDesklet->iKnownHeight = pDesklet->iHeight;
	if (((pDesklet->iDesiredWidth != 0 || pDesklet->iDesiredHeight != 0) && pDesklet->iDesiredWidth == pDesklet->iWidth && pDesklet->iDesiredHeight == pDesklet->iHeight) || (pDesklet->iDesiredWidth == 0 && pDesklet->iDesiredHeight == 0))
	{
		pDesklet->iDesiredWidth = 0;
		pDesklet->iDesiredHeight = 0;
		
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDesklet));
		cairo_dock_load_desklet_decorations (pDesklet, pCairoContext);
		cairo_destroy (pCairoContext);
		
		if (pDesklet->pIcon != NULL && pDesklet->pIcon->pModuleInstance != NULL)
		{
			//g_print ("RELOAD\n");
			cairo_dock_reload_module_instance (pDesklet->pIcon->pModuleInstance, FALSE);
			gtk_widget_queue_draw (pDesklet->pWidget);  // sinon on ne redessine que l'interieur.
		}
		
		Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
		if (/*cairo_dock_window_is_dock (Xid) || */pDesklet->bSpaceReserved)
		{
			cairo_dock_reserve_space_for_desklet (pDesklet, TRUE);
		}
	}
	
	//g_print ("iWidth <- %d;iHeight <- %d ; (%dx%d) (%x)\n", pDesklet->iWidth, pDesklet->iHeight, pDesklet->iKnownWidth, pDesklet->iKnownHeight, pDesklet->pIcon);
	return FALSE;
}
static gboolean _cairo_dock_write_desklet_position (CairoDesklet *pDesklet)
{
	if (pDesklet->pIcon != NULL && pDesklet->pIcon->pModuleInstance != NULL)
	{
		int iRelativePositionX = (pDesklet->iWindowPositionX + pDesklet->iWidth/2 <= g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]/2 ? pDesklet->iWindowPositionX : pDesklet->iWindowPositionX - g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]);
		int iRelativePositionY = (pDesklet->iWindowPositionY + pDesklet->iHeight/2 <= g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]/2 ? pDesklet->iWindowPositionY : pDesklet->iWindowPositionY - g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]);
		cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
			G_TYPE_INT, "Desklet", "x position", iRelativePositionX,
			G_TYPE_INT, "Desklet", "y position", iRelativePositionY,
			G_TYPE_INVALID);
		cairo_dock_update_desklet_position_in_gui (pDesklet->pIcon->pModuleInstance->pModule->pVisitCard->cModuleName,
			iRelativePositionX,
			iRelativePositionY);
	}
	
	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	if (/*cairo_dock_window_is_dock (Xid) || */pDesklet->bSpaceReserved)
	{
		cairo_dock_reserve_space_for_desklet (pDesklet, TRUE);
	}
	pDesklet->iSidWritePosition = 0;
	return FALSE;
}
static gboolean on_configure_desklet (GtkWidget* pWidget,
	GdkEventConfigure* pEvent,
	CairoDesklet *pDesklet)
{
	//g_print (" >>>>>>>>> %s (%dx%d)", __func__, pEvent->width, pEvent->height);
	if (pDesklet->iWidth != pEvent->width || pDesklet->iHeight != pEvent->height)
	{
		if ((pEvent->width < pDesklet->iWidth || pEvent->height < pDesklet->iHeight) && (pDesklet->iDesiredWidth != 0 && pDesklet->iDesiredHeight != 0))
		{
			gdk_window_resize (pDesklet->pWidget->window,
				pDesklet->iDesiredWidth,
				pDesklet->iDesiredHeight);
		}
		
		pDesklet->iWidth = pEvent->width;
		pDesklet->iHeight = pEvent->height;
		
		if (g_bUseOpenGL)
		{
			GdkGLContext* pGlContext = gtk_widget_get_gl_context (pWidget);
			GdkGLDrawable* pGlDrawable = gtk_widget_get_gl_drawable (pWidget);
			GLsizei w = pEvent->width;
			GLsizei h = pEvent->height;
			if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
				return FALSE;
			
			glViewport(0, 0, w, h);
			
			cairo_dock_set_perspective_view (w, h);
			
			gdk_gl_drawable_gl_end (pGlDrawable);
		}
		
		if (pDesklet->iSidWriteSize != 0)
		{
			g_source_remove (pDesklet->iSidWriteSize);
		}
		pDesklet->iSidWriteSize = g_timeout_add (500, (GSourceFunc) _cairo_dock_write_desklet_size, (gpointer) pDesklet);
	}

	if (pDesklet->iWindowPositionX != pEvent->x || pDesklet->iWindowPositionY != pEvent->y)
	{
		pDesklet->iWindowPositionX = pEvent->x;
		pDesklet->iWindowPositionY = pEvent->y;

		if (pDesklet->iSidWritePosition != 0)
		{
			g_source_remove (pDesklet->iSidWritePosition);
		}
		pDesklet->iSidWritePosition = g_timeout_add (500, (GSourceFunc) _cairo_dock_write_desklet_position, (gpointer) pDesklet);
	}
	pDesklet->moving = FALSE;

	return FALSE;
}

gboolean on_scroll_desklet (GtkWidget* pWidget,
	GdkEventScroll* pScroll,
	CairoDesklet *pDesklet)
{
	g_print ("scroll\n");
	if (! (pScroll->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)))
	{
		Icon *icon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
		if (icon != NULL)
		{
			cairo_dock_notify_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_SCROLL_ICON, icon, pDesklet, pScroll->direction);
		}
	}
	return FALSE;
}

gboolean on_unmap_desklet (GtkWidget* pWidget,
	GdkEvent *pEvent,
	CairoDesklet *pDesklet)
{
	//g_print ("unmap (%d)\n", pDesklet->bAllowMinimize);
	if (! pDesklet->bAllowMinimize)
		gtk_window_present (GTK_WINDOW (pWidget));
	else
		pDesklet->bAllowMinimize = FALSE;
	return TRUE;  // stops other handlers from being invoked for the event.
}

Icon *cairo_dock_pick_icon_on_opengl_desklet (CairoDesklet *pDesklet)
{
	GLuint selectBuf[4];
    GLint hits=0;
    GLint viewport[4];
	
	GdkGLContext* pGlContext = gtk_widget_get_gl_context (pDesklet->pWidget);
	GdkGLDrawable* pGlDrawable = gtk_widget_get_gl_drawable (pDesklet->pWidget);
	if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
		return NULL;
	
	glGetIntegerv (GL_VIEWPORT, viewport);
	glSelectBuffer (4, selectBuf);
	
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);
	
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
    gluPickMatrix ((GLdouble) pDesklet->iMouseX, (GLdouble) (viewport[3] - pDesklet->iMouseY), 2.0, 2.0, viewport);
	gluPerspective (60.0, 1.0*(GLfloat)pDesklet->iWidth/(GLfloat)pDesklet->iHeight, 1., 4*pDesklet->iHeight);
	
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();
	
	_cairo_dock_set_desklet_matrix (pDesklet);
	
	if (pDesklet->iLeftSurfaceOffset != 0 || pDesklet->iTopSurfaceOffset != 0 || pDesklet->iRightSurfaceOffset != 0 || pDesklet->iBottomSurfaceOffset != 0)
	{
		glTranslatef ((pDesklet->iLeftSurfaceOffset - pDesklet->iRightSurfaceOffset)/2, (pDesklet->iBottomSurfaceOffset - pDesklet->iTopSurfaceOffset)/2, 0.);
		glScalef (1. - 1.*(pDesklet->iLeftSurfaceOffset + pDesklet->iRightSurfaceOffset) / pDesklet->iWidth,
			1. - 1.*(pDesklet->iTopSurfaceOffset + pDesklet->iBottomSurfaceOffset) / pDesklet->iHeight,
			1.);
	}
	
	glPolygonMode (GL_FRONT, GL_FILL);
	glColor4f (1., 1., 1., 1.);
	
	pDesklet->iPickedObject = 0;
	if (pDesklet->render_bounding_box != NULL)  // surclasse la fonction du moteur de rendu.
	{
		pDesklet->render_bounding_box (pDesklet);
	}
	else if (pDesklet->pRenderer && pDesklet->pRenderer->render_bounding_box != NULL)
	{
		pDesklet->pRenderer->render_bounding_box (pDesklet);
	}
	else  // on le fait nous-memes a partir des coordonnees des icones.
	{
		glTranslatef (-pDesklet->iWidth/2, -pDesklet->iHeight/2, 0.);
		
		double x, y, w, h;
		Icon *pIcon;
		
		pIcon = pDesklet->pIcon;
		if (pIcon != NULL && pIcon->iIconTexture != 0)
		{
			w = pIcon->fWidth/2;
			h = pIcon->fHeight/2;
			x = pIcon->fDrawX + w;
			y = pDesklet->iHeight - pIcon->fDrawY - h;
			
			glLoadName(pIcon->iIconTexture);
			
			glBegin(GL_QUADS);
			glVertex3f(x-w, y+h, 0.);
			glVertex3f(x+w, y+h, 0.);
			glVertex3f(x+w, y-h, 0.);
			glVertex3f(x-w, y-h, 0.);
			glEnd();
		}
		
		GList *ic;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->iIconTexture == 0)
				continue;
			
			w = pIcon->fWidth/2;
			h = pIcon->fHeight/2;
			x = pIcon->fDrawX + w;
			y = pDesklet->iHeight - pIcon->fDrawY - h;
			
			glLoadName(pIcon->iIconTexture);
			
			glBegin(GL_QUADS);
			glVertex3f(x-w, y+h, 0.);
			glVertex3f(x+w, y+h, 0.);
			glVertex3f(x+w, y-h, 0.);
			glVertex3f(x-w, y-h, 0.);
			glEnd();
		}
	}
	
	glPopName();
	
	hits = glRenderMode (GL_RENDER);

	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix ();
	
	Icon *pFoundIcon = NULL;
	if (hits != 0)
	{
		GLuint id = selectBuf[3];
		Icon *pIcon;
		
		if (pDesklet->render_bounding_box != NULL)
		{
			pDesklet->iPickedObject = id;
			//g_print ("iPickedObject <- %d\n", id);
			pFoundIcon = pDesklet->pIcon;  // il faut mettre qqch, sinon la notification est filtree par la macro CD_APPLET_ON_CLICK_BEGIN.
		}
		else
		{
			pIcon = pDesklet->pIcon;
			if (pIcon != NULL && pIcon->iIconTexture != 0)
			{
				if (pIcon->iIconTexture == id)
				{
					pFoundIcon = pIcon;
				}
			}
			
			if (pFoundIcon == NULL)
			{
				GList *ic;
				for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
				{
					pIcon = ic->data;
					if (pIcon->iIconTexture == id)
					{
						pFoundIcon = pIcon;
						break ;
					}
				}
			}
		}
	}
	
	gdk_gl_drawable_gl_end (pGlDrawable);
	return pFoundIcon;
}


Icon *cairo_dock_find_clicked_icon_in_desklet (CairoDesklet *pDesklet)
{
	if (g_bUseOpenGL && pDesklet->pRenderer && pDesklet->pRenderer->render_opengl)
	{
		return cairo_dock_pick_icon_on_opengl_desklet (pDesklet);
	}
	
	int iMouseX = pDesklet->iMouseX, iMouseY = pDesklet->iMouseY;
	if (pDesklet->fRotation != 0)
	{
		g_print (" clic en (%d;%d) rotations : %.2frad\n", iMouseX, iMouseY, pDesklet->fRotation);
		double x, y;  // par rapport au centre du desklet.
		x = iMouseX - pDesklet->iWidth/2;
		y = pDesklet->iHeight/2 - iMouseY;
		
		double r, t;  // coordonnees polaires.
		r = sqrt (x*x + y*y);
		t = atan2 (y, x);
		
		double z = _compute_zoom_for_rotation (pDesklet);
		r /= z;
		
		x = r * cos (t + pDesklet->fRotation);  // la rotation de cairo est dans le sene horaire.
		y = r * sin (t + pDesklet->fRotation);
		
		iMouseX = x + pDesklet->iWidth/2;
		iMouseY = pDesklet->iHeight/2 - y;
		g_print (" => (%d;%d)\n", iMouseX, iMouseY);
	}
	pDesklet->iMouseX2d = iMouseX;
	pDesklet->iMouseY2d = iMouseY;
	
	Icon *icon = pDesklet->pIcon;
	g_return_val_if_fail (icon != NULL, NULL);  // peut arriver au tout debut, car on associe l'icone au desklet _apres_ l'avoir cree, et on fait tourner la gtk_main entre-temps (pour le redessiner invisible).
	if (icon->fDrawX < iMouseX && icon->fDrawX + icon->fWidth * icon->fScale > iMouseX && icon->fDrawY < iMouseY && icon->fDrawY + icon->fHeight * icon->fScale > iMouseY)
	{
		return icon;
	}
	
	if (pDesklet->icons != NULL)
	{
		GList* ic;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->fDrawX < iMouseX && icon->fDrawX + icon->fWidth * icon->fScale > iMouseX && icon->fDrawY < iMouseY && icon->fDrawY + icon->fHeight * icon->fScale > iMouseY)
			{
				return icon;
			}
		}
	}
	return NULL;
}

static gboolean on_button_press_desklet(GtkWidget *pWidget,
	GdkEventButton *pButton,
	CairoDesklet *pDesklet)
{
	if (pButton->button == 1)  // clic gauche.
	{
		if (pButton->type == GDK_BUTTON_PRESS)
		{
			pDesklet->iMouseX = pButton->x;  // pour le deplacement manuel.
			pDesklet->iMouseY = pButton->y;
			
			if (pButton->x < myDesklets.iDeskletButtonSize && pButton->y < myDesklets.iDeskletButtonSize)
				pDesklet->rotating = TRUE;
			else if (pButton->x > pDesklet->iWidth - myDesklets.iDeskletButtonSize && pButton->y < myDesklets.iDeskletButtonSize)
				pDesklet->retaching = TRUE;
			else if (pButton->x > (pDesklet->iWidth - myDesklets.iDeskletButtonSize)/2 && pButton->x < (pDesklet->iWidth + myDesklets.iDeskletButtonSize)/2 && pButton->y < myDesklets.iDeskletButtonSize)
				pDesklet->rotatingY = TRUE;
			else if (pButton->y > (pDesklet->iHeight - myDesklets.iDeskletButtonSize)/2 && pButton->y < (pDesklet->iHeight + myDesklets.iDeskletButtonSize)/2 && pButton->x < myDesklets.iDeskletButtonSize)
				pDesklet->rotatingX = TRUE;
			else
				pDesklet->time = pButton->time;
		}
		else if (pButton->type == GDK_BUTTON_RELEASE)
		{
			cd_debug ("GDK_BUTTON_RELEASE");
			if (pDesklet->moving)
			{
				pDesklet->moving = FALSE;
			}
			else if (pDesklet->rotating)
			{
				pDesklet->rotating = FALSE;
				cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
					G_TYPE_INT, "Desklet", "rotation", (int) (pDesklet->fRotation / G_PI * 180.),
					G_TYPE_INVALID);
				gtk_widget_queue_draw (pDesklet->pWidget);
			}
			else if (pDesklet->retaching)
			{
				pDesklet->retaching = FALSE;
				if (! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude && pButton->x > pDesklet->iWidth - myDesklets.iDeskletButtonSize && pButton->y < myDesklets.iDeskletButtonSize)
				{
					Icon *icon = pDesklet->pIcon;
					g_return_val_if_fail (CAIRO_DOCK_IS_APPLET (icon), FALSE);
					cairo_dock_update_conf_file (icon->pModuleInstance->cConfFilePath,
						G_TYPE_BOOLEAN, "Desklet", "initially detached", FALSE,
						G_TYPE_INVALID);
					cairo_dock_reload_module_instance (icon->pModuleInstance, TRUE);
					return TRUE;  // interception du signal.
				}
			}
			else if (pDesklet->rotatingY)
			{
				pDesklet->rotatingY = FALSE;
				cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
					G_TYPE_INT, "Desklet", "depth rotation y", (int) (pDesklet->fDepthRotationY / G_PI * 180.),
					G_TYPE_INVALID);
				gtk_widget_queue_draw (pDesklet->pWidget);
			}
			else if (pDesklet->rotatingX)
			{
				pDesklet->rotatingX = FALSE;
				cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
					G_TYPE_INT, "Desklet", "depth rotation x", (int) (pDesklet->fDepthRotationX / G_PI * 180.),
					G_TYPE_INVALID);
				gtk_widget_queue_draw (pDesklet->pWidget);
			}
			else
			{
				Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
				cairo_dock_notify (CAIRO_DOCK_CLICK_ICON, pClickedIcon, pDesklet, pButton->state);
			}
		}
		else if (pButton->type == GDK_2BUTTON_PRESS)
		{
			if (! (pButton->x < myDesklets.iDeskletButtonSize && pButton->y < myDesklets.iDeskletButtonSize) && ! (pButton->x > (pDesklet->iWidth - myDesklets.iDeskletButtonSize)/2 && pButton->x < (pDesklet->iWidth + myDesklets.iDeskletButtonSize)/2 && pButton->y < myDesklets.iDeskletButtonSize))
			{
				Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
				cairo_dock_notify (CAIRO_DOCK_DOUBLE_CLICK_ICON, pClickedIcon, pDesklet);
			}
		}
	}
	else if (pButton->button == 3 && pButton->type == GDK_BUTTON_PRESS)  // clique droit.
	{
		Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
		GtkWidget *menu = cairo_dock_build_menu (pClickedIcon, CAIRO_CONTAINER (pDesklet));  // genere un CAIRO_DOCK_BUILD_MENU.
		gtk_widget_show_all (menu);
		gtk_menu_popup (GTK_MENU (menu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
		pDesklet->bInside = FALSE;
		pDesklet->iGradationCount = 0;  // on force le fond a redevenir transparent.
		gtk_widget_queue_draw (pDesklet->pWidget);
	}
	else if (pButton->button == 2 && pButton->type == GDK_BUTTON_PRESS)  // clique milieu.
	{
		if (pButton->x < myDesklets.iDeskletButtonSize && pButton->y < myDesklets.iDeskletButtonSize)
		{
			pDesklet->fRotation = 0.;
			gtk_widget_queue_draw (pDesklet->pWidget);
			cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
				G_TYPE_INT, "Desklet", "rotation", 0,
				G_TYPE_INVALID);
		}
		else if (pButton->x > (pDesklet->iWidth - myDesklets.iDeskletButtonSize)/2 && pButton->x < (pDesklet->iWidth + myDesklets.iDeskletButtonSize)/2 && pButton->y < myDesklets.iDeskletButtonSize)
		{
			pDesklet->fDepthRotationY = 0.;
			gtk_widget_queue_draw (pDesklet->pWidget);
			cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
				G_TYPE_INT, "Desklet", "depth rotation y", 0,
				G_TYPE_INVALID);
		}
		else if (pButton->y > (pDesklet->iHeight - myDesklets.iDeskletButtonSize)/2 && pButton->y < (pDesklet->iHeight + myDesklets.iDeskletButtonSize)/2 && pButton->x < myDesklets.iDeskletButtonSize)
		{
			pDesklet->fDepthRotationX = 0.;
			gtk_widget_queue_draw (pDesklet->pWidget);
			cairo_dock_update_conf_file (pDesklet->pIcon->pModuleInstance->cConfFilePath,
				G_TYPE_INT, "Desklet", "depth rotation x", 0,
				G_TYPE_INVALID);
		}
		else
		{
			cairo_dock_notify (CAIRO_DOCK_MIDDLE_CLICK_ICON, pDesklet->pIcon, pDesklet);
		}
	}
	return FALSE;
}

void on_drag_data_received_desklet (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, CairoDesklet *pDesklet)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	
	//\_________________ On recupere l'URI.
	gchar *cReceivedData = (gchar *) selection_data->data;
	g_return_if_fail (cReceivedData != NULL);
	
	pDesklet->iMouseX = x;
	pDesklet->iMouseY = y;
	Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
	cairo_dock_notify_drop_data (cReceivedData, pClickedIcon, 0, CAIRO_CONTAINER (pDesklet));
}

static gboolean on_motion_notify_desklet(GtkWidget *pWidget,
	GdkEventMotion* pMotion,
	CairoDesklet *pDesklet)
{
	if (pMotion->state & GDK_BUTTON1_MASK && ! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude)
	{
		cd_debug ("root : %d;%d", (int) pMotion->x_root, (int) pMotion->y_root);
		/*pDesklet->moving = TRUE;
		gtk_window_move (GTK_WINDOW (pWidget),
			pMotion->x_root + pDesklet->diff_x,
			pMotion->y_root + pDesklet->diff_y);*/
	}
	else  // le 'press-button' est local au sous-widget clique, alors que le 'motion-notify' est global a la fenetre; c'est donc par lui qu'on peut avoir a coup sur les coordonnees du curseur (juste avant le clic).
	{
		pDesklet->iMouseX = pMotion->x;
		pDesklet->iMouseY = pMotion->y;
		gboolean bStartAnimation = FALSE;
		cairo_dock_notify_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_MOUSE_MOVED, pDesklet, &bStartAnimation);
		if (bStartAnimation)
			cairo_dock_launch_animation (CAIRO_CONTAINER (pDesklet));
	}
	
	if (pDesklet->rotating && ! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude)
	{
		double alpha = atan2 (pDesklet->iHeight, - pDesklet->iWidth);
		pDesklet->fRotation = alpha - atan2 (.5*pDesklet->iHeight - pMotion->y, pMotion->x - .5*pDesklet->iWidth);
		while (pDesklet->fRotation > G_PI)
			pDesklet->fRotation -= 2 * G_PI;
		while (pDesklet->fRotation <= - G_PI)
			pDesklet->fRotation += 2 * G_PI;
		gtk_widget_queue_draw(pDesklet->pWidget);
	}
	else if (pDesklet->rotatingY && ! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude)
	{
		pDesklet->fDepthRotationY = G_PI * (pMotion->x - .5*pDesklet->iWidth) / pDesklet->iWidth;
		gtk_widget_queue_draw(pDesklet->pWidget);
	}
	else if (pDesklet->rotatingX && ! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude)
	{
		pDesklet->fDepthRotationX = G_PI * (pMotion->y - .5*pDesklet->iHeight) / pDesklet->iHeight;
		gtk_widget_queue_draw(pDesklet->pWidget);
	}
	else if (pMotion->state & GDK_BUTTON1_MASK && ! pDesklet->bPositionLocked && ! pDesklet->bFixedAttitude && ! pDesklet->moving)
	{
		gtk_window_begin_move_drag (GTK_WINDOW (gtk_widget_get_toplevel (pWidget)),
			1/*pButton->button*/,
			pMotion->x_root/*pButton->x_root*/,
			pMotion->y_root/*pButton->y_root*/,
			pDesklet->time/*pButton->time*/);
		pDesklet->moving = TRUE;
	}
	else
	{
		gboolean bStartAnimation = FALSE;
		Icon *pIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
		if (pIcon != NULL)
		{
			if (! pIcon->bPointed)
			{
				Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDesklet->icons);
				if (pPointedIcon != NULL)
					pPointedIcon->bPointed = FALSE;
				pIcon->bPointed = TRUE;
				
				g_print ("on survole %s\n", pIcon->acName);
				cairo_dock_notify_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_ENTER_ICON, pIcon, pDesklet, &bStartAnimation);
			}
		}
		else
		{
			Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDesklet->icons);
			if (pPointedIcon != NULL)
			{
				pPointedIcon->bPointed = FALSE;
				
				g_print ("kedal\n");
				cairo_dock_notify_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_ENTER_ICON, pPointedIcon, pDesklet, &bStartAnimation);

			}
		}
		if (bStartAnimation)
		{
			cairo_dock_launch_animation (CAIRO_CONTAINER (pDesklet));
		}
	}
	
	gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);  // pour recevoir d'autres MotionNotify.
	return FALSE;
}


static gboolean cd_desklet_on_focus_in_out(GtkWidget *widget,
	GdkEventFocus *event,
	CairoDesklet *pDesklet)
{
	if (pDesklet)
		gtk_widget_queue_draw(pDesklet->pWidget);
	return FALSE;
}

static gboolean _cairo_dock_desklet_gradation (CairoDesklet *pDesklet)
{
	pDesklet->iGradationCount += (pDesklet->bInside ? 1 : -1);
	gtk_widget_queue_draw (pDesklet->pWidget);
	
	if (pDesklet->iGradationCount <= 0 || pDesklet->iGradationCount >= CD_NB_ITER_FOR_GRADUATION)
	{
		if (pDesklet->iGradationCount < 0)
			pDesklet->iGradationCount = 0;
		else if (pDesklet->iGradationCount > CD_NB_ITER_FOR_GRADUATION)
			pDesklet->iGradationCount = CD_NB_ITER_FOR_GRADUATION;
		pDesklet->iSidGradationOnEnter = 0;
		return FALSE;
	}
	return TRUE;
}
static gboolean on_enter_desklet (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDesklet *pDesklet)
{
	//g_print ("%s (%d)\n", __func__, pDesklet->bInside);
	if (! pDesklet->bInside)  // avant on etait dehors, on redessine donc.
	{
		pDesklet->bInside = TRUE;
		if (pDesklet->iSidGradationOnEnter == 0)
		{
			pDesklet->iSidGradationOnEnter = g_timeout_add (50, (GSourceFunc) _cairo_dock_desklet_gradation, (gpointer) pDesklet);
		}
		
		if (g_bUseOpenGL/* && pDesklet->pRenderer && pDesklet->pRenderer->render_opengl != NULL*/)
		{
			gboolean bStartAnimation = FALSE;
			cairo_dock_notify_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_ENTER_DESKLET, pDesklet, &bStartAnimation);
			if (bStartAnimation)
				cairo_dock_launch_animation (CAIRO_CONTAINER (pDesklet));
		}
	}
	return FALSE;
}
static gboolean on_leave_desklet (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDesklet *pDesklet)
{
	//g_print ("%s (%d)\n", __func__, pDesklet->bInside);
	int iMouseX, iMouseY;
	gdk_window_get_pointer (pWidget->window, &iMouseX, &iMouseY, NULL);
	if (gtk_bin_get_child (GTK_BIN (pDesklet->pWidget)) != NULL && iMouseX > 0 && iMouseX < pDesklet->iWidth && iMouseY > 0 && iMouseY < pDesklet->iHeight)  // en fait on est dans un widget fils, donc on ne fait rien.
	{
		return FALSE;
	}

	pDesklet->bInside = FALSE;
	if (pDesklet->iSidGradationOnEnter == 0)
	{
		pDesklet->iSidGradationOnEnter = g_timeout_add (50, (GSourceFunc) _cairo_dock_desklet_gradation, (gpointer) pDesklet);
	}
	return FALSE;
}


CairoDesklet *cairo_dock_create_desklet (Icon *pIcon, GtkWidget *pInteractiveWidget, CairoDeskletAccessibility iAccessibility)
{
	cd_message ("%s ()", __func__);
	CairoDesklet *pDesklet = g_new0(CairoDesklet, 1);
	pDesklet->iType = CAIRO_DOCK_TYPE_DESKLET;
	pDesklet->bIsHorizontal = TRUE;
	pDesklet->bDirectionUp = TRUE;
	pDesklet->fZoom = 1;
	
	GtkWidget* pWindow = cairo_dock_create_container_window ();
	if (iAccessibility == CAIRO_DESKLET_ON_WIDGET_LAYER)
		gtk_window_set_type_hint (GTK_WINDOW (pWindow), GDK_WINDOW_TYPE_HINT_UTILITY);
	else if (iAccessibility == CAIRO_DESKLET_RESERVE_SPACE)
		gtk_window_set_type_hint (GTK_WINDOW (pWindow), GDK_WINDOW_TYPE_HINT_DOCK);
		
	pDesklet->pWidget = pWindow;
	pDesklet->pIcon = pIcon;
	
	gtk_window_set_title (GTK_WINDOW(pWindow), "cairo-dock-desklet");
	gtk_widget_add_events( pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_FOCUS_CHANGE_MASK);
	gtk_container_set_border_width(GTK_CONTAINER(pWindow), 3);  // comme ca.
	gtk_window_set_default_size(GTK_WINDOW(pWindow), 10, 10);  // idem.

	g_signal_connect (G_OBJECT (pWindow),
		"expose-event",
		G_CALLBACK (on_expose_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"configure-event",
		G_CALLBACK (on_configure_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"motion-notify-event",
		G_CALLBACK (on_motion_notify_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"button-press-event",
		G_CALLBACK (on_button_press_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"button-release-event",
		G_CALLBACK (on_button_press_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"focus-in-event",
		G_CALLBACK (cd_desklet_on_focus_in_out),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"focus-out-event",
		G_CALLBACK (cd_desklet_on_focus_in_out),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"enter-notify-event",
		G_CALLBACK (on_enter_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"leave-notify-event",
		G_CALLBACK (on_leave_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"unmap-event",
		G_CALLBACK (on_unmap_desklet),
		pDesklet);
	g_signal_connect (G_OBJECT (pWindow),
		"scroll-event",
		G_CALLBACK (on_scroll_desklet),
		pDesklet);
	cairo_dock_allow_widget_to_receive_data (pWindow, G_CALLBACK (on_drag_data_received_desklet), pDesklet);

	//user widget
	if (pInteractiveWidget != NULL)
	{
		cd_debug ("ref = %d", pInteractiveWidget->object.parent_instance.ref_count);
		cairo_dock_add_interactive_widget_to_desklet (pInteractiveWidget, pDesklet);
		cd_debug ("pack -> ref = %d", pInteractiveWidget->object.parent_instance.ref_count);
	}

	gtk_widget_show_all(pWindow);

	return pDesklet;
}

void cairo_dock_configure_desklet (CairoDesklet *pDesklet, CairoDeskletAttribute *pAttribute)
{
	cd_debug ("%s (%dx%d ; (%d,%d) ; %d)", __func__, pAttribute->iDeskletWidth, pAttribute->iDeskletHeight, pAttribute->iDeskletPositionX, pAttribute->iDeskletPositionY, pAttribute->iAccessibility);
	if (pAttribute->bDeskletUseSize && (pAttribute->iDeskletWidth != pDesklet->iWidth || pAttribute->iDeskletHeight != pDesklet->iHeight))
	{
		pDesklet->iDesiredWidth = pAttribute->iDeskletWidth;
		pDesklet->iDesiredHeight = pAttribute->iDeskletHeight;
		gdk_window_resize (pDesklet->pWidget->window,
			pAttribute->iDeskletWidth,
			pAttribute->iDeskletHeight);
	}
	if (! pAttribute->bDeskletUseSize)
		gtk_container_set_border_width (GTK_CONTAINER (pDesklet->pWidget), 0);
	
	int iAbsolutePositionX = (pAttribute->iDeskletPositionX < 0 ? g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] + pAttribute->iDeskletPositionX : pAttribute->iDeskletPositionX);
	iAbsolutePositionX = MAX (0, MIN (g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] - pAttribute->iDeskletWidth, iAbsolutePositionX));
	int iAbsolutePositionY = (pAttribute->iDeskletPositionY < 0 ? g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] + pAttribute->iDeskletPositionY : pAttribute->iDeskletPositionY);
	iAbsolutePositionY = MAX (0, MIN (g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] - pAttribute->iDeskletHeight, iAbsolutePositionY));
	
	gdk_window_move(pDesklet->pWidget->window,
		iAbsolutePositionX,
		iAbsolutePositionY);
	cd_debug (" -> (%d;%d)", iAbsolutePositionX, iAbsolutePositionY);

	gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), pAttribute->iAccessibility == CAIRO_DESKLET_KEEP_BELOW);
	gtk_window_set_keep_above (GTK_WINDOW (pDesklet->pWidget), pAttribute->iAccessibility == CAIRO_DESKLET_KEEP_ABOVE);

	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	pDesklet->bSpaceReserved = FALSE;
	if (pAttribute->iAccessibility == CAIRO_DESKLET_ON_WIDGET_LAYER)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_UTILITY");  // le hide-show le fait deconner completement, il perd son skip_task_bar ! au moins sous KDE3.
	else if (pAttribute->iAccessibility == CAIRO_DESKLET_RESERVE_SPACE)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
	else
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
	cairo_dock_reserve_space_for_desklet (pDesklet, pAttribute->iAccessibility == CAIRO_DESKLET_RESERVE_SPACE);
	pDesklet->bSpaceReserved = (pAttribute->iAccessibility == CAIRO_DESKLET_RESERVE_SPACE);
	
	if (pAttribute->bOnAllDesktops)
		gtk_window_stick (GTK_WINDOW (pDesklet->pWidget));
	else
		gtk_window_unstick (GTK_WINDOW (pDesklet->pWidget));
	
	pDesklet->bPositionLocked = pAttribute->bPositionLocked;
	pDesklet->fRotation = pAttribute->iRotation / 180. * G_PI ;
	pDesklet->fDepthRotationY = pAttribute->iDepthRotationY / 180. * G_PI ;
	pDesklet->fDepthRotationX = pAttribute->iDepthRotationX / 180. * G_PI ;
	
	g_free (pDesklet->cDecorationTheme);
	pDesklet->cDecorationTheme = pAttribute->cDecorationTheme;
	pAttribute->cDecorationTheme = NULL;
	cairo_dock_free_desklet_decoration (pDesklet->pUserDecoration);
	pDesklet->pUserDecoration = pAttribute->pUserDecoration;
	pAttribute->pUserDecoration = NULL;
	
	cd_debug ("%s (%dx%d ; %d)", __func__, pDesklet->iDesiredWidth, pDesklet->iDesiredHeight, pDesklet->iSidWriteSize);
	if (pDesklet->iDesiredWidth == 0 && pDesklet->iDesiredHeight == 0 && pDesklet->iSidWriteSize == 0)
	{
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDesklet));
		cairo_dock_load_desklet_decorations (pDesklet, pCairoContext);
		cairo_destroy (pCairoContext);
	}
}

void cairo_dock_free_desklet (CairoDesklet *pDesklet)
{
	if (pDesklet == NULL)
		return;

	if (pDesklet->iSidWriteSize != 0)
		g_source_remove (pDesklet->iSidWriteSize);
	if (pDesklet->iSidWritePosition != 0)
		g_source_remove (pDesklet->iSidWritePosition);
	if (pDesklet->iSidGrowUp != 0)
		g_source_remove (pDesklet->iSidGrowUp);
	if (pDesklet->iSidGLAnimation != 0)
		g_source_remove (pDesklet->iSidGLAnimation);
	if (pDesklet->iSidGradationOnEnter != 0)
		g_source_remove (pDesklet->iSidGradationOnEnter);
	cairo_dock_notify_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_STOP_DESKLET, pDesklet);
	
	cairo_dock_steal_interactive_widget_from_desklet (pDesklet);

	gtk_widget_destroy (pDesklet->pWidget);
	pDesklet->pWidget = NULL;
	
	if (pDesklet->pRenderer != NULL)
	{
		if (pDesklet->pRenderer->free_data != NULL)
		{
			pDesklet->pRenderer->free_data (pDesklet);
			pDesklet->pRendererData = NULL;
		}
	}
	
	if (pDesklet->icons != NULL)
	{
		g_list_foreach (pDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (pDesklet->icons);
		pDesklet->icons = NULL;
	}
	
	g_free (pDesklet->cDecorationTheme);
	cairo_dock_free_desklet_decoration (pDesklet->pUserDecoration);
	
	if (pDesklet->pBackGroundSurface != NULL)
		cairo_surface_destroy (pDesklet->pBackGroundSurface);
	if (pDesklet->pForeGroundSurface != NULL)
		cairo_surface_destroy (pDesklet->pForeGroundSurface);
	if (pDesklet->iBackGroundTexture != 0)
		_cairo_dock_delete_texture (pDesklet->iBackGroundTexture);
	if (pDesklet->iForeGroundTexture != 0)
		_cairo_dock_delete_texture (pDesklet->iForeGroundTexture);
	
	g_free(pDesklet);
}



void cairo_dock_add_interactive_widget_to_desklet_full (GtkWidget *pInteractiveWidget, CairoDesklet *pDesklet, int iRightMargin)
{
	g_return_if_fail (pDesklet != NULL && pInteractiveWidget != NULL);
	if (pDesklet->pInteractiveWidget != NULL || gtk_bin_get_child (GTK_BIN (pDesklet->pWidget)) != NULL)
	{
		cd_warning ("This desklet already has an interactive widget !");
		return;
	}
	
	//gtk_container_add (GTK_CONTAINER (pDesklet->pWidget), pInteractiveWidget);
	GtkWidget *pHBox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (pDesklet->pWidget), pHBox);
	
	gtk_box_pack_start (GTK_BOX (pHBox), pInteractiveWidget, TRUE, TRUE, 0);
	pDesklet->pInteractiveWidget = pInteractiveWidget;
	
	if (iRightMargin != 0)
	{
		GtkWidget *pMarginBox = gtk_vbox_new (FALSE, 0);
		gtk_widget_set (pMarginBox, "width-request", iRightMargin, NULL);
		gtk_box_pack_start (GTK_BOX (pHBox), pMarginBox, FALSE, FALSE, 0);  // a tester ...
	}
	
	gtk_widget_show_all (pHBox);
}

void cairo_dock_set_desklet_margin (CairoDesklet *pDesklet, int iRightMargin)
{
	g_return_if_fail (pDesklet != NULL && pDesklet->pInteractiveWidget != NULL);
	
	GtkWidget *pHBox = gtk_bin_get_child (GTK_BIN (pDesklet->pWidget));
	if (pHBox != pDesklet->pInteractiveWidget)  // precaution.
	{
		GList *pChildList = gtk_container_get_children (GTK_CONTAINER (pHBox));
		if (pChildList != NULL && pChildList->next != NULL)
		{
			GtkWidget *pMarginBox = GTK_WIDGET (pChildList->next->data);
			gtk_widget_set (pMarginBox, "width-request", iRightMargin, NULL);
		}
		else
		{
			GtkWidget *pMarginBox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set (pMarginBox, "width-request", iRightMargin, NULL);
			gtk_box_pack_start (GTK_BOX (pHBox), pMarginBox, FALSE, FALSE, 0);
		}
	}
}

void cairo_dock_steal_interactive_widget_from_desklet (CairoDesklet *pDesklet)
{
	if (pDesklet == NULL)
		return;

	GtkWidget *pInteractiveWidget = pDesklet->pInteractiveWidget;
	if (pInteractiveWidget != NULL)
	{
		cairo_dock_steal_widget_from_its_container (pInteractiveWidget);
		pDesklet->pInteractiveWidget = NULL;
		GtkWidget *pBox = gtk_bin_get_child (GTK_BIN (pDesklet->pWidget));
		if (pBox != NULL)
			gtk_widget_destroy (pBox);
	}
}



void cairo_dock_hide_desklet (CairoDesklet *pDesklet)
{
	if (pDesklet)
		gtk_widget_hide (pDesklet->pWidget);
}

void cairo_dock_show_desklet (CairoDesklet *pDesklet)
{
	if (pDesklet)
		gtk_window_present(GTK_WINDOW(pDesklet->pWidget));
}


static gboolean _cairo_dock_set_one_desklet_visible (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, gpointer data)
{
	gboolean bOnWidgetLayerToo = GPOINTER_TO_INT (data);
	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	gboolean bIsOnWidgetLayer = cairo_dock_window_is_utility (Xid);
	if (bOnWidgetLayerToo || ! bIsOnWidgetLayer)
	{
		cd_debug ("%s (%d)", __func__, Xid);
		
		if (bIsOnWidgetLayer)  // on le passe sur la couche visible.
			cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
		
		gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), FALSE);
		
		cairo_dock_show_desklet (pDesklet);
	}
	return FALSE;
}
void cairo_dock_set_all_desklets_visible (gboolean bOnWidgetLayerToo)
{
	cd_debug ("%s (%d)", __func__, bOnWidgetLayerToo);
	cairo_dock_foreach_desklet (_cairo_dock_set_one_desklet_visible, GINT_TO_POINTER (bOnWidgetLayerToo));
}

static gboolean _cairo_dock_set_one_desklet_visibility_to_default (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, CairoDockMinimalAppletConfig *pMinimalConfig)
{
	GKeyFile *pKeyFile = cairo_dock_pre_read_module_instance_config (pInstance, pMinimalConfig);
	g_key_file_free (pKeyFile);
	
	gtk_window_set_keep_below (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->deskletAttribute.iAccessibility == CAIRO_DESKLET_KEEP_BELOW);
	gtk_window_set_keep_above (GTK_WINDOW (pDesklet->pWidget), pMinimalConfig->deskletAttribute.iAccessibility == CAIRO_DESKLET_KEEP_ABOVE);
	
	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	if (pMinimalConfig->deskletAttribute.iAccessibility == CAIRO_DESKLET_ON_WIDGET_LAYER)
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_UTILITY");
	else if (pMinimalConfig->deskletAttribute.iAccessibility == CAIRO_DESKLET_RESERVE_SPACE)
	{
		pDesklet->bSpaceReserved = TRUE;
		//cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_DOCK");
		cairo_dock_reserve_space_for_desklet (pDesklet, TRUE);
	}
	else
		cairo_dock_set_xwindow_type_hint (Xid, "_NET_WM_WINDOW_TYPE_NORMAL");
	pDesklet->bAllowMinimize = FALSE;
	
	return FALSE;
}
void cairo_dock_set_desklets_visibility_to_default (void)
{
	CairoDockMinimalAppletConfig minimalConfig;
	cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cairo_dock_set_one_desklet_visibility_to_default, &minimalConfig);
}

static gboolean _cairo_dock_test_one_desklet_Xid (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, Window *pXid)
{
	return (GDK_WINDOW_XID (pDesklet->pWidget->window) == *pXid);
}
CairoDesklet *cairo_dock_get_desklet_by_Xid (Window Xid)
{
	CairoDockModuleInstance *pInstance = cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cairo_dock_test_one_desklet_Xid, &Xid);
	return (pInstance != NULL ? pInstance->pDesklet : NULL);
}


static gboolean _cairo_dock_grow_up_desklet (CairoDesklet *pDesklet)
{
	pDesklet->fZoom += .1;
	gtk_widget_queue_draw (pDesklet->pWidget);
	
	if (pDesklet->fZoom >= 1.11)  // la derniere est a x1.1
	{
		pDesklet->fZoom = 1;
		pDesklet->iSidGrowUp = 0;
		return FALSE;
	}
	return TRUE;
}
void cairo_dock_zoom_out_desklet (CairoDesklet *pDesklet)
{
	g_return_if_fail (pDesklet != NULL);
	pDesklet->fZoom = 0;
	pDesklet->iSidGrowUp = g_timeout_add (50, (GSourceFunc) _cairo_dock_grow_up_desklet, (gpointer) pDesklet);
}



void cairo_dock_load_desklet_decorations (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	if (pDesklet->pBackGroundSurface != NULL)
	{
		cairo_surface_destroy (pDesklet->pBackGroundSurface);
		pDesklet->pBackGroundSurface = NULL;
	}
	if (pDesklet->pForeGroundSurface != NULL)
	{
		cairo_surface_destroy (pDesklet->pForeGroundSurface);
		pDesklet->pForeGroundSurface = NULL;
	}
	if (pDesklet->iBackGroundTexture != 0)
	{
		_cairo_dock_delete_texture (pDesklet->iBackGroundTexture);
		pDesklet->iBackGroundTexture = 0;
	}
	if (pDesklet->iForeGroundTexture != 0)
	{
		_cairo_dock_delete_texture (pDesklet->iForeGroundTexture);
		pDesklet->iForeGroundTexture = 0;
	}
	
	CairoDeskletDecoration *pDeskletDecorations;
	cd_debug ("%s (%s)", __func__, pDesklet->cDecorationTheme);
	if (pDesklet->cDecorationTheme == NULL || strcmp (pDesklet->cDecorationTheme, "personnal") == 0)
		pDeskletDecorations = pDesklet->pUserDecoration;
	else if (strcmp (pDesklet->cDecorationTheme, "default") == 0)
		pDeskletDecorations = cairo_dock_get_desklet_decoration (myDesklets.cDeskletDecorationsName);
	else
		pDeskletDecorations = cairo_dock_get_desklet_decoration (pDesklet->cDecorationTheme);
	if (pDeskletDecorations == NULL)  // peut arriver si rendering n'a pas encore charge ses decorations.
		return ;
	cd_debug ("pDeskletDecorations : %s (%x)", pDesklet->cDecorationTheme, pDeskletDecorations);
	
	double fZoomX = 0., fZoomY = 0.;
	if  (pDeskletDecorations->cBackGroundImagePath != NULL && pDeskletDecorations->fBackGroundAlpha > 0)
	{
		cd_debug ("bg : %s", pDeskletDecorations->cBackGroundImagePath);
		pDesklet->pBackGroundSurface = cairo_dock_create_surface_from_image (pDeskletDecorations->cBackGroundImagePath,
			pSourceContext,
			1.,  // cairo_dock_get_max_scale (pDesklet)
			pDesklet->iWidth, pDesklet->iHeight,
			pDeskletDecorations->iLoadingModifier,
			&pDesklet->fImageWidth, &pDesklet->fImageHeight,
			&fZoomX, &fZoomY);
	}
	if (pDeskletDecorations->cForeGroundImagePath != NULL && pDeskletDecorations->fForeGroundAlpha > 0)
	{
		cd_debug ("fg : %s", pDeskletDecorations->cForeGroundImagePath);
		pDesklet->pForeGroundSurface = cairo_dock_create_surface_from_image (pDeskletDecorations->cForeGroundImagePath,
			pSourceContext,
			1.,  // cairo_dock_get_max_scale (pDesklet)
			pDesklet->iWidth, pDesklet->iHeight,
			pDeskletDecorations->iLoadingModifier,
			&pDesklet->fImageWidth, &pDesklet->fImageHeight,
			&fZoomX, &fZoomY);
	}
	cd_debug ("image : %.2fx%.2f ; zoom : %.2fx%.2f", pDesklet->fImageWidth, pDesklet->fImageHeight, fZoomX, fZoomY);
	pDesklet->iLeftSurfaceOffset = pDeskletDecorations->iLeftMargin * fZoomX;
	pDesklet->iTopSurfaceOffset = pDeskletDecorations->iTopMargin * fZoomY;
	pDesklet->iRightSurfaceOffset = pDeskletDecorations->iRightMargin * fZoomX;
	pDesklet->iBottomSurfaceOffset = pDeskletDecorations->iBottomMargin * fZoomY;
	pDesklet->fBackGroundAlpha = pDeskletDecorations->fBackGroundAlpha;
	pDesklet->fForeGroundAlpha = pDeskletDecorations->fForeGroundAlpha;
	cd_debug ("%d;%d;%d;%d ; %.2f;%.2f", pDesklet->iLeftSurfaceOffset, pDesklet->iTopSurfaceOffset, pDesklet->iRightSurfaceOffset, pDesklet->iBottomSurfaceOffset, pDesklet->fBackGroundAlpha, pDesklet->fForeGroundAlpha);
	if (g_bUseOpenGL)
	{
		if (pDesklet->pBackGroundSurface != NULL)
			pDesklet->iBackGroundTexture = cairo_dock_create_texture_from_surface (pDesklet->pBackGroundSurface);
		if (pDesklet->pForeGroundSurface != NULL)
			pDesklet->iForeGroundTexture = cairo_dock_create_texture_from_surface (pDesklet->pForeGroundSurface);
	}
}

static gboolean _cairo_dock_reload_one_desklet_decorations (CairoDesklet *pDesklet, CairoDockModuleInstance *pInstance, gpointer *data)
{
	gboolean bDefaultThemeOnly = GPOINTER_TO_INT (data[0]);
	cairo_t *pSourceContext = data[1];
	
	if (bDefaultThemeOnly)
	{
		if (pDesklet->cDecorationTheme == NULL || strcmp (pDesklet->cDecorationTheme, "default") == 0)
		{
			cd_debug ("on recharge les decorations de ce desklet");
			cairo_dock_load_desklet_decorations (pDesklet, pSourceContext);
		}
	}
	else  // tous ceux qui ne sont pas encore charges et qui ont leur taille definitive.
	{
		cd_debug ("pouet %dx%d ; %d ; %x;%x", pDesklet->iDesiredWidth, pDesklet->iDesiredHeight, pDesklet->iSidWriteSize, pDesklet->pBackGroundSurface, pDesklet->pForeGroundSurface);
		if (pDesklet->iDesiredWidth == 0 && pDesklet->iDesiredHeight == 0 && pDesklet->iSidWriteSize == 0 && pDesklet->pBackGroundSurface == NULL && pDesklet->pForeGroundSurface == NULL)
		{
			cd_debug ("ce desklet a saute le chargement de ses deco => on l'aide.");
			cairo_dock_load_desklet_decorations (pDesklet, pSourceContext);
		}
	}
	return FALSE;
}
void cairo_dock_reload_desklets_decorations (gboolean bDefaultThemeOnly, cairo_t *pSourceContext)  // tous ou bien seulement ceux avec "default".
{
	cd_message ("%s (%d)", __func__, bDefaultThemeOnly);
	gpointer data[2] = {GINT_TO_POINTER (bDefaultThemeOnly), pSourceContext};
	cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc)_cairo_dock_reload_one_desklet_decorations, data);
}


void cairo_dock_free_desklet_decoration (CairoDeskletDecoration *pDecoration)
{
	if (pDecoration == NULL)
		return ;
	g_free (pDecoration->cBackGroundImagePath);
	g_free (pDecoration->cForeGroundImagePath);
	g_free (pDecoration);
}


void cairo_dock_reserve_space_for_desklet (CairoDesklet *pDesklet, gboolean bReserve)
{
	g_print ("%s (%d)\n", __func__, bReserve);
	Window Xid = GDK_WINDOW_XID (pDesklet->pWidget->window);
	int left=0, right=0, top=0, bottom=0;
	int left_start_y=0, left_end_y=0, right_start_y=0, right_end_y=0, top_start_x=0, top_end_x=0, bottom_start_x=0, bottom_end_x=0;
	int iHeight = pDesklet->iHeight, iWidth = pDesklet->iWidth;
	int iX = pDesklet->iWindowPositionX, iY = pDesklet->iWindowPositionY;
	if (bReserve)
	{
		int iTopOffset, iBottomOffset, iRightOffset, iLeftOffset;
		iTopOffset = iY;
		iBottomOffset = g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] - 1 - (iY + iHeight);
		iLeftOffset = iX;
		iRightOffset = g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] - 1 - (iX + iWidth);
		
		if (iBottomOffset < MIN (iLeftOffset, iRightOffset))  // en bas.
		{
			bottom = iHeight + iBottomOffset;
			bottom_start_x = iX;
			bottom_end_x = iX + iWidth;
		}
		else if (iTopOffset < MIN (iLeftOffset, iRightOffset))  // en haut.
		{
			top = iHeight + iTopOffset;
			top_start_x = iX;
			top_end_x = iX + iWidth;
		}
		else if (iLeftOffset < iRightOffset)  // a gauche.
		{
			left = iWidth + iLeftOffset;
			left_start_y = iY;
			left_end_y = iY + iHeight;
		}
		else  // a droite.
		{
			right = iWidth + iRightOffset;
			right_start_y = iY;
			right_end_y = iY + iHeight;
		}
	}

	cairo_dock_set_strut_partial (Xid, left, right, top, bottom, left_start_y, left_end_y, right_start_y, right_end_y, top_start_x, top_end_x, bottom_start_x, bottom_end_x);
}
