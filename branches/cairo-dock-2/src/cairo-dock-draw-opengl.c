 /*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <cairo.h>
#include <pango/pango.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#ifdef HAVE_GLITZ
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include <gtk/gtkgl.h>
#include <X11/extensions/Xrender.h> 
#include <GL/gl.h> 
#include <GL/glu.h> 
#include <GL/glx.h> 
#include <gdk/x11/gdkglx.h>

#include "cairo-dock-icons.h"
#include "cairo-dock-draw-opengl.h"

#define RADIAN (G_PI / 180.0)  // Conversion Radian/Degres
#define DELTA_ROUND_DEGREE 1

extern double g_fAmplitude;
extern CairoDockLabelDescription g_iconTextDescription;
extern cairo_surface_t *g_pDesktopBgSurface;

extern int g_iBackgroundTexture;

extern double g_fVisibleAppliAlpha;
extern gboolean g_bConstantSeparatorSize;
extern double g_fAlphaAtRest;
extern double g_fReflectSize;
extern gboolean g_bIndicatorAbove;
extern gboolean g_bLabelForPointedIconOnly;
extern gboolean g_bTextAlwaysHorizontal;
extern double g_fLabelAlphaThreshold;
extern GLuint g_iIndicatorTexture;

extern gboolean g_bLinkIndicatorWithIcon;
extern double g_fIndicatorWidth, g_fIndicatorHeight;
extern int g_iIndicatorDeltaY;
extern GLuint g_iIndicatorTexture;

static void _cairo_dock_draw_appli_indicator_opengl (Icon *icon, gboolean bHorizontalDock, double fRatio, gboolean bDirectionUp)
{
	glPushMatrix ();
	//cairo_save (pCairoContext);
	if (icon->fOrientation != 0)
		//cairo_rotate (pCairoContext, icon->fOrientation);
		glRotatef (icon->fOrientation, 0., 0., 1.);
	if (g_bLinkIndicatorWithIcon)
	{
		if (bHorizontalDock)
		{
			glTranslatef (0, - icon->fHeight * icon->fHeightFactor * icon->fScale/2 - (g_fIndicatorHeight - g_iIndicatorDeltaY*(1 + g_fAmplitude)) * fRatio * icon->fScale/2, 0);
			glScalef (g_fIndicatorWidth * fRatio * icon->fScale / 2, g_fIndicatorHeight * fRatio * icon->fScale / 2, 1.);
			/*cairo_translate (pCairoContext,
				(icon->fWidth - g_fIndicatorWidth * fRatio) * icon->fWidthFactor * icon->fScale / 2,
				(bDirectionUp ? 
					(icon->fHeight - (g_fIndicatorHeight - g_iIndicatorDeltaY / (1 + g_fAmplitude)) * fRatio) * icon->fScale :
					(g_fIndicatorHeight - g_iIndicatorDeltaY / (1 + g_fAmplitude)) * icon->fScale * fRatio));
			cairo_scale (pCairoContext,
				fRatio * icon->fWidthFactor * icon->fScale / (1 + g_fAmplitude),
				fRatio * icon->fHeightFactor * icon->fScale / (1 + g_fAmplitude) * (bDirectionUp ? 1 : -1));*/
		}
		else
		{
			
			/*cairo_translate (pCairoContext,
				(bDirectionUp ? 
					(icon->fHeight - (g_fIndicatorHeight - g_iIndicatorDeltaY / (1 + g_fAmplitude)) * fRatio) * icon->fScale : 
					(g_fIndicatorHeight - g_iIndicatorDeltaY / (1 + g_fAmplitude)) * icon->fScale * fRatio),
					(icon->fWidth - g_fIndicatorWidth * fRatio) * icon->fWidthFactor * icon->fScale / 2);
			cairo_scale (pCairoContext,
				fRatio * icon->fHeightFactor * icon->fScale / (1 + g_fAmplitude) * (bDirectionUp ? 1 : -1),
				fRatio * icon->fWidthFactor * icon->fScale / (1 + g_fAmplitude));*/
		}
		
	}
	else
	{
		if (bHorizontalDock)
		{
			glTranslatef (0, - icon->fHeight * icon->fHeightFactor * icon->fScale/2 - (g_fIndicatorHeight - g_iIndicatorDeltaY*(1 + g_fAmplitude)) * fRatio * icon->fScale/2, 0);
			glScalef (g_fIndicatorWidth * fRatio * icon->fScale / 2, g_fIndicatorHeight * fRatio * icon->fScale / 2, 1.);
			/*cairo_translate (pCairoContext,
				icon->fDrawXAtRest - icon->fDrawX + (icon->fWidth * icon->fScale - g_fIndicatorWidth * fRatio) / 2,
				icon->fDrawYAtRest - icon->fDrawY + (bDirectionUp ? 
					(icon->fHeight * icon->fScale - (g_fIndicatorHeight - g_iIndicatorDeltaY / (1 + g_fAmplitude)) * fRatio) :
					(g_fIndicatorHeight * icon->fScale - g_iIndicatorDeltaY) * fRatio));*/
		}
		else
		{
			/*
			cairo_translate (pCairoContext,
				icon->fDrawYAtRest - icon->fDrawY + (bDirectionUp ? 
					(icon->fHeight - (g_fIndicatorHeight - g_iIndicatorDeltaY / (1 + g_fAmplitude)) * fRatio) * icon->fScale : 
					(g_fIndicatorHeight - g_iIndicatorDeltaY / (1 + g_fAmplitude)) * icon->fScale * fRatio),
				icon->fDrawXAtRest - icon->fDrawX + (icon->fWidth * icon->fScale - g_fIndicatorWidth * fRatio) / 2);*/
		}
	}
	/*cairo_set_source_surface (pCairoContext, g_pIndicatorSurface[bHorizontalDock], 0.0, 0.0);
	cairo_paint (pCairoContext);
	cairo_restore (pCairoContext);*/
	glBindTexture (GL_TEXTURE_2D, g_iIndicatorTexture);
	glColor4f(1.0f, 1.0f, 1.0f, 1.);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1,  0.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  0.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  0.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  -1,  0.0f);  // Top Left Of The Texture and Quad
	glEnd();
	glPopMatrix ();
}
void cairo_dock_render_one_icon_opengl (Icon *icon, CairoDock *pDock, double fRatio, double fDockMagnitude, gboolean bUseText)
{
	icon->iRotationY += 3;
	//glDisable(GL_DEPTH_TEST);// On desactive le tampon de profondeur
	glEnable(GL_DEPTH_TEST);// On desactive le tampon de profondeur  
	if (CAIRO_DOCK_IS_APPLI (icon) && g_fVisibleAppliAlpha != 0 && ! CAIRO_DOCK_IS_APPLET (icon))
	{
		double fAlpha = (icon->bIsHidden ? MIN (1 - g_fVisibleAppliAlpha, 1) : MIN (g_fVisibleAppliAlpha + 1, 1));
		if (fAlpha != 1)
			icon->fAlpha = fAlpha;  // astuce bidon pour pas multiplier 2 fois.
	}
	
	//\_____________________ On se place au centre de l'icone.
	double fY = pDock->iCurrentHeight - icon->fDrawY - icon->fHeight * icon->fScale * icon->fHeightFactor;  // ordonnee du bas de l'icone.
	glLoadIdentity ();
	if (pDock->bHorizontalDock)
		glTranslatef (icon->fDrawX + icon->fWidth * icon->fWidthFactor * icon->fScale/2, fY + icon->fHeight * icon->fHeightFactor * icon->fScale/2, 0);
	else
		glTranslatef (fY + icon->fHeight * icon->fHeightFactor * icon->fScale/2, icon->fDrawX + icon->fWidth * icon->fScale/2, 0.);
	glPushMatrix ();
	
	//\_____________________ On dessine l'indicateur derriere.
	if (icon->bHasIndicator && ! g_bIndicatorAbove && g_iIndicatorTexture != 0)
	{
		_cairo_dock_draw_appli_indicator_opengl (icon, pDock->bHorizontalDock, fRatio, pDock->bDirectionUp);
	}
	
	
	glBindTexture (GL_TEXTURE_2D, icon->iIconTexture);
	
	//\_____________________ Cas de l'animation Pulse.
	double fPreviousAlpha = icon->fAlpha;
	if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_PULSE)
	{
		if (icon->fAlpha > 0)
		{
			glPushMatrix ();
			double fScaleFactor = 1 + (1 - icon->fAlpha);
			glScalef (icon->fWidth * icon->fScale / 2 * fScaleFactor, icon->fHeight * icon->fScale / 2 * fScaleFactor, 1.);
			glColor4f(1.0f, 1.0f, 1.0f, icon->fAlpha);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1,  0.0f);  // Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  0.0f);  // Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  0.0f);  // Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  -1,  0.0f);  // Top Left Of The Texture and Quad
			glEnd();
			glPopMatrix ();
			/*cairo_save (pCairoContext);
			double fScaleFactor = 1 + (1 - icon->fAlpha);
			if (bHorizontalDock)
				cairo_translate (pCairoContext, icon->fWidth / fRatio * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fHeight / fRatio * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
			else
				cairo_translate (pCairoContext, icon->fHeight / fRatio * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fWidth / fRatio * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
			cairo_scale (pCairoContext, fScaleFactor, fScaleFactor);
			if (icon->pIconBuffer != NULL)
				cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
			cairo_paint_with_alpha (pCairoContext, icon->fAlpha);
			cairo_restore (pCairoContext);*/
			
		}
		icon->fAlpha = .8;
	}
	
	//\_____________________ On positionne l'icone.
	if (pDock->bHorizontalDock)
	{
		if (g_bConstantSeparatorSize && CAIRO_DOCK_IS_SEPARATOR (icon))
		{
			glTranslatef (0., (pDock->bDirectionUp ? icon->fHeight * (- icon->fScale + 1)/2 : icon->fHeight * (icon->fScale - 1)/2), 0.);
			glScalef (icon->fWidth / 2, icon->fHeight / 2, 1.);
		}
		else
		{
			glScalef (icon->fWidth * icon->fWidthFactor * icon->fScale / 2, icon->fHeight * icon->fHeightFactor * icon->fScale / 2, 1.);
		}
		if (icon->fOrientation != 0)
		{
			glRotatef (icon->fOrientation, 0., 0., 1.);
		}
	}
	else
	{
		
	}
	
	//\_____________________ On dessine l'icone.
	double fAlpha = icon->fAlpha * (fDockMagnitude + g_fAlphaAtRest * (1 - fDockMagnitude));
	glColor4f(1.0f, 1.0f, 1.0f, fAlpha);
	/*glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1,  0.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  0.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  0.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  -1,  0.0f);  // Top Left Of The Texture and Quad
	glEnd();*/
	glRotatef (icon->iRotationY, 0, 1, 0);
	glCallList(CAIRO_DOCK_CAPSULE_LIST); // Et hop on affiche la capsule
	glPopMatrix ();  // retour juste apres la translation au milieu de l'icone.
	return ;
	//\_____________________ On dessine les reflets.
	if (pDock->bUseReflect && icon->iReflectionTexture != 0)  // on dessine les reflets.
	{
		glBindTexture (GL_TEXTURE_2D, icon->iReflectionTexture);
		glPushMatrix ();
		
		//\_____________________ Cas de l'animation Pulse.
		if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_PULSE)
		{
			if (fPreviousAlpha > 0)
			{
				glPushMatrix ();
				double fScaleFactor = 1 + (1 - fPreviousAlpha);
				glScalef (icon->fWidth * icon->fScale / 2 * fScaleFactor, icon->fHeight * icon->fScale / 2 * fScaleFactor, 1.);
				glColor4f(1.0f, 1.0f, 1.0f, fPreviousAlpha);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1,  0.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  0.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  0.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  -1,  0.0f);  // Top Left Of The Texture and Quad
				glEnd ();
				glPopMatrix ();
				/*cairo_save (pCairoContext);
				double fScaleFactor = 1 + (1 - fPreviousAlpha);
				if (bHorizontalDock)
					cairo_translate (pCairoContext, icon->fWidth * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fHeight * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
				else
					cairo_translate (pCairoContext, icon->fHeight * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fWidth * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
				cairo_scale (pCairoContext, fScaleFactor, fScaleFactor);
				if (icon->pIconBuffer != NULL)
					cairo_set_source_surface (pCairoContext, icon->pReflectionBuffer, 0.0, 0.0);
				cairo_paint_with_alpha (pCairoContext, fPreviousAlpha);
				cairo_restore (pCairoContext);*/
			}
		}
		
		//\_____________________ On positionne les reflets.
		if (pDock->bHorizontalDock)
		{
			glTranslatef (0., pDock->bDirectionUp ? - icon->fHeight * icon->fScale/2 - g_fReflectSize * icon->fScale/2 : icon->fHeight * icon->fScale/2 + g_fReflectSize * icon->fScale/2, 0.);
		}
		else
		{
			
		}
		
		//\_____________________ On dessine les reflets.
		glColor4f(1.0f, 1.0f, 1.0f, fAlpha);
		glScalef (icon->fWidth * icon->fScale / 2, g_fReflectSize * icon->fScale / 2, 1.);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1,  0.0f);  // Bottom Left Of The Texture and Quad
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  0.0f);  // Bottom Right Of The Texture and Quad
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  0.0f);  // Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  -1,  0.0f);  // Top Left Of The Texture and Quad
		glEnd();
		glPopMatrix ();  // retour juste apres la translation (fDrawX, fDrawY).
		
		//\_____________________ Cas des reflets dynamiques.
		/*if (g_bDynamicReflection && icon->fScale > 1)
		{
			cairo_pattern_t *pGradationPattern;
			if (bHorizontalDock)
			{
				pGradationPattern = cairo_pattern_create_linear (0.,
					(bDirectionUp ? 0. : g_fReflectSize / fRatio * (1 + g_fAmplitude)),
					0.,
					(bDirectionUp ? g_fReflectSize / fRatio * (1 + g_fAmplitude) / icon->fScale : g_fReflectSize / fRatio * (1 + g_fAmplitude) * (1. - 1./ icon->fScale)));  // de haut en bas.
				g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
				
				cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.,
					0.,
					0.,
					0.,
					1.);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					1.,
					0.,
					0.,
					0.,
					1 - (icon->fScale - 1) / g_fAmplitude);  // astuce pour ne pas avoir a re-creer la surface de la reflection.
			}
			else
			{
				pGradationPattern = cairo_pattern_create_linear ((bDirectionUp ? 0. : g_fReflectSize / fRatio * (1 + g_fAmplitude)),
					0.,
					(bDirectionUp ? g_fReflectSize / fRatio * (1 + g_fAmplitude) / icon->fScale : g_fReflectSize / fRatio * (1 + g_fAmplitude) * (1. - 1./ icon->fScale)),
					0.);
				g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
				
				cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.,
					0.,
					0.,
					0.,
					1.);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					1.,
					0.,
					0.,
					0.,
					1. - (icon->fScale - 1) / g_fAmplitude);  // astuce pour ne pas avoir a re-creer la surface de la reflection.
			}
			cairo_save (pCairoContext);
			cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
			cairo_translate (pCairoContext, 0, 0);
			cairo_mask (pCairoContext, pGradationPattern);
			cairo_restore (pCairoContext);

			cairo_pattern_destroy (pGradationPattern);
		}*/
	}
	
	//\_____________________ On dessine l'indicateur devant.
	if (icon->bHasIndicator && g_bIndicatorAbove && g_iIndicatorTexture != 0)
	{
		_cairo_dock_draw_appli_indicator_opengl (icon, pDock->bHorizontalDock, fRatio, pDock->bDirectionUp);
	}
	
	
	//\_____________________ On dessine les etiquettes, avec un alpha proportionnel au facteur d'echelle de leur icone.
	if (bUseText && icon->iLabelTexture != 0 && icon->fScale > 1.01 && (! g_bLabelForPointedIconOnly || icon->bPointed) && icon->iCount == 0)  // 1.01 car sin(pi) = 1+epsilon :-/
	{
		glPushMatrix ();
		
		double fOffsetX = 0.;
		if (icon->fDrawX + icon->fWidth * icon->fScale/2 - icon->iTextWidth/2 < 0)
			fOffsetX = icon->iTextWidth/2 - (icon->fDrawX + icon->fWidth * icon->fScale/2);
		else if (icon->fDrawX + icon->fWidth * icon->fScale/2 + icon->iTextWidth/2 > pDock->iCurrentWidth)
			fOffsetX = pDock->iCurrentWidth - (icon->fDrawX + icon->fWidth * icon->fScale/2 + icon->iTextWidth/2);
		if (icon->fOrientation != 0 && ! g_bTextAlwaysHorizontal)
		{
			//cairo_rotate (pCairoContext, icon->fOrientation);
			glRotatef (icon->fOrientation, 0., 0., 1.);
		}
		
		if (! pDock->bHorizontalDock && g_bTextAlwaysHorizontal)
		{
			/*cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				0,
				0);*/
			glTranslatef (0., (pDock->bDirectionUp ? 1:-1)* (icon->fHeight * icon->fScale/2 + icon->iTextHeight / 2), 1.);
			
		}
		else if (pDock->bHorizontalDock)
		{
			glTranslatef (fOffsetX, (pDock->bDirectionUp ? icon->fHeight * icon->fScale/2 + g_iconTextDescription.iSize - icon->iTextHeight / 2 : - icon->fHeight * icon->fScale/2 - icon->iTextHeight / 2 + icon->fTextYOffset), 0.);
			/*cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				fOffsetX,
				bDirectionUp ? -g_iconTextDescription.iSize : icon->fHeight * icon->fScale - icon->fTextYOffset);*/
		}
		else
		{
			glTranslatef ((pDock->bDirectionUp ? 1:-1)* (icon->fHeight * icon->fScale/2 + icon->iTextHeight / 2), fOffsetX, 1.);
			/*cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				bDirectionUp ? -g_iconTextDescription.iSize : icon->fHeight * icon->fScale - icon->fTextYOffset,
				fOffsetX);*/
		}
		
		double fMagnitude;
		if (g_bLabelForPointedIconOnly)
		{
			fMagnitude = fDockMagnitude;  // (icon->fScale - 1) / g_fAmplitude / sin (icon->fPhase);  // sin (phi ) != 0 puisque fScale > 1.
		}
		else
		{
			fMagnitude = (icon->fScale - 1) / g_fAmplitude;  /// il faudrait diviser par pDock->fMagnitudeMax ...
			fMagnitude *= (fMagnitude * g_fLabelAlphaThreshold + 1) / (g_fLabelAlphaThreshold + 1);
		}
		glColor4f(1.0f, 1.0f, 1.0f, fMagnitude);
		glBindTexture (GL_TEXTURE_2D, icon->iLabelTexture);
		glScalef (.5*icon->iTextWidth, .5*icon->iTextHeight, 1.);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1,  0.0f);  // Bottom Left Of The Texture and Quad
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  0.0f);  // Bottom Right Of The Texture and Quad
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  0.0f);  // Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  -1,  0.0f);  // Top Left Of The Texture and Quad
		glEnd ();
		glPopMatrix ();
	}
	
	//\_____________________ On dessine les infos additionnelles.
	if (icon->iQuickInfoTexture != 0)
	{
		glPushMatrix ();
		glTranslatef (0., (- icon->fHeight + icon->iQuickInfoHeight * fRatio) * icon->fScale/2, 0.);
		glColor4f(1.0f, 1.0f, 1.0f, fAlpha);
		glBindTexture (GL_TEXTURE_2D, icon->iQuickInfoTexture);
		glScalef (icon->iQuickInfoWidth * fRatio * icon->fScale/2, icon->iQuickInfoHeight * fRatio * icon->fScale/2, 1.);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1,  0.0f);  // Bottom Left Of The Texture and Quad
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  0.0f);  // Bottom Right Of The Texture and Quad
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  0.0f);  // Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  -1,  0.0f);  // Top Left Of The Texture and Quad
		glEnd ();
		glPopMatrix ();
		/*cairo_translate (pCairoContext,
			//-icon->fQuickInfoXOffset + icon->fWidth / 2,
			//icon->fHeight - icon->fQuickInfoYOffset);
			(- icon->iQuickInfoWidth * fRatio + icon->fWidthFactor * icon->fWidth) / 2 * icon->fScale,
			(icon->fHeight - icon->iQuickInfoHeight * fRatio) * icon->fScale);
		
		cairo_scale (pCairoContext,
			fRatio * icon->fScale / (1 + g_fAmplitude) * 1,
			fRatio * icon->fScale / (1 + g_fAmplitude) * 1);
		
		cairo_set_source_surface (pCairoContext,
			icon->pQuickInfoBuffer,
			0,
			0);
		if (fAlpha == 1)
			cairo_paint (pCairoContext);
		else
			cairo_paint_with_alpha (pCairoContext, fAlpha);*/
	}
}


GLuint cairo_dock_create_texture_from_surface (cairo_surface_t *pImageSurface)
{
	GLuint iTexture = 0;
	int w = cairo_image_surface_get_width (pImageSurface);
	int h = cairo_image_surface_get_height (pImageSurface);
	glGenTextures (1, &iTexture);
	g_print ("texture %d generee (%x)\n", iTexture, cairo_image_surface_get_data (pImageSurface));
	glBindTexture (GL_TEXTURE_2D, iTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
	glTexImage2D (GL_TEXTURE_2D,
		0,
		4,  // GL_ALPHA / GL_BGRA
		w,
		h,
		0,
		GL_BGRA,  // GL_ALPHA / GL_BGRA
		GL_UNSIGNED_BYTE,
		cairo_image_surface_get_data (pImageSurface));
	return iTexture;
}



static float fCapsuleObjectPlaneS[4] = { 0.5f, 0.0f, 0.0f, 0.0f }; // pour un plaquages propre des textures
static float fCapsuleObjectPlaneT[4] = { 0.0f, 0.5f, 0.0f, 0.0f };

void cairo_dock_init_capsule_display (GLuint iChromeTexture)
{
	static gboolean bDone = FALSE;
	if (bDone)
		return ;
	bDone = TRUE;
	int        deg, deg2, iter;
	float        amp, rayon;
	
	rayon        = 1.0f;
	amp        = 90.0 / 5;
	deg2        = 0;
	
	glNewList(CAIRO_DOCK_CAPSULE_LIST, GL_COMPILE); // Go pour la compilation de la display list
	
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR ); // ok la on selectionne le type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glTexGenfv(GL_S, GL_OBJECT_PLANE, fCapsuleObjectPlaneS); // On place la texture correctement en X
	glTexGenfv(GL_T, GL_OBJECT_PLANE, fCapsuleObjectPlaneT); // Et en Y
	glEnable(GL_TEXTURE_GEN_S);                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);                // Et en T aussi
	
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glLoadIdentity(); // On la reset
	glTranslatef(0.5f, 0.5f, 0.0f); // Et on decale la texture pour un affiche propre
	glRotatef (180, 1, 0, 0);  // sinono les icones sont a l'envers.
	glMatrixMode(GL_MODELVIEW); // On revient sur la matrice d'affichage
	
	// bon la je commente pas on fait juste une demi sphere applatie
	double a = .3;  // applatissement;
	for (iter = 0;iter < 5;iter ++)
	{
		for (deg = 0;deg < 360;deg += 10)
		{
			glBegin(GL_QUADS);
			glVertex3f(rayon * sin(deg*RADIAN),
				rayon * cos(deg*RADIAN),
				a * sin(deg2*RADIAN) + 0.1f);
			glVertex3f((rayon-0.2f) * sin(deg*RADIAN),
				(rayon-0.2f) * cos(deg*RADIAN),
				a * sin((deg2+amp)*RADIAN) +0.1f);
			glVertex3f((rayon-0.2f) * sin((deg+10)*RADIAN),    
				(rayon-0.2f) * cos((deg+10)*RADIAN),
				a * sin((deg2+amp)*RADIAN) + 0.1f);
			glVertex3f(rayon * sin((deg+10)*RADIAN),
				rayon * cos((deg+10)*RADIAN),
				a * sin(deg2*RADIAN) + 0.1f);
		
			glVertex3f(rayon * sin(deg*RADIAN),                
				rayon * cos(deg*RADIAN),            
				a * sin((deg2+180.0f)*RADIAN) - 0.1f);
			glVertex3f((rayon-0.2f) * sin(deg*RADIAN),        
				(rayon-0.2f) * cos(deg*RADIAN),        
				a * sin((deg2+amp+180.0f)*RADIAN) -0.1f);
			glVertex3f((rayon-0.2f) * sin((deg+10)*RADIAN),    
				(rayon-0.2f) * cos((deg+10)*RADIAN),
				a * sin((deg2+amp+180.0f)*RADIAN) - 0.1f);
			glVertex3f(rayon * sin((deg+10)*RADIAN),        
				rayon * cos((deg+10)*RADIAN),        
				a * sin((deg2+180.0f)*RADIAN) - 0.1f);
			glEnd();
		}
		rayon    -= 0.2f;
		deg2    += amp;
	}

	// Ici c'est pour faire le cylindre qui relie les demi spheres
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Couleur a fond 
	g_print ("iChromeTexture : %d\n", iChromeTexture);
	glBindTexture(GL_TEXTURE_2D, iChromeTexture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT); // Ici c'est pour le type de combinaison de texturing en cas de multi
	//glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE); // pas de multi je remplace donc l'ancienne texture par celle ci
	
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // ok la on selectionne le type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // Ce sera du sphere mapping pour un petit effet chrome
	glEnable(GL_TEXTURE_GEN_S); // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T); // Et en T aussi
	
	rayon = 1.0f;
	
	//for (iter = 0;iter < 5;iter ++)
	{
		for (deg = 0;deg < 360;deg += 10)
		{
			glBegin(GL_QUADS);
			glVertex3f(rayon * sin(deg*RADIAN), rayon * cos(deg*RADIAN), 0.1f);
			glVertex3f(rayon * sin((deg+10)*RADIAN), rayon * cos((deg+10)*RADIAN), 0.1f);
			glVertex3f(rayon * sin((deg+10)*RADIAN), rayon * cos((deg+10)*RADIAN), -0.1f);
			glVertex3f(rayon * sin(deg*RADIAN), rayon * cos(deg*RADIAN), -0.1f);
			glEnd();
		}
	
		rayon -= 0.2f;
		deg2 += amp;
	}
	
	glEndList(); // Fini la display list
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D); // Plus de texture merci 
	glDisable(GL_TEXTURE);
}

