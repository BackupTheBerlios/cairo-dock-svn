/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-session.h"
#include "applet-draw.h"

#define STATIC_ANGLE 20.

#define _alpha_prompt(k,n) cos (G_PI/2*fabs ((double) ((k % (2*n)) - n) / n));

const int s_iNbPromptAnimationSteps = 40;

static inline int _cd_do_get_matching_icons_width (int *iNbIcons)
{
	int i = 0, iIconsWidth = 0;
	CairoDock *pParentDock;
	Icon *pIcon;
	int iWidth, iHeight;
	double fZoom;
	GList *ic;
	for (ic = myData.pMatchingIcons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->pIconBuffer == NULL && pIcon->iIconTexture == 0)  // icone pas encore chargee.
			continue;
		pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
		cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pParentDock), &iWidth, &iHeight);
		if (iHeight != 0)
		{
			fZoom = (double) g_pMainDock->iCurrentHeight/2 / iHeight;
			iIconsWidth += iWidth * fZoom;
		}
		i ++;
	}
	*iNbIcons = i;
	return iIconsWidth;
}

static inline int _cd_do_get_icon_x (GList *pElement)
{
	int iOffset = 0;
	CairoDock *pParentDock;
	Icon *pIcon;
	int iWidth, iHeight;
	double fZoom;
	GList *ic;
	for (ic = myData.pMatchingIcons; ic != NULL && ic != pElement; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->pIconBuffer == NULL && pIcon->iIconTexture == 0)  // icone pas encore chargee.
			continue;
		pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
		cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pParentDock), &iWidth, &iHeight);
		if (iHeight != 0)
		{
			fZoom = (double) g_pMainDock->iCurrentHeight/2 / iHeight;
			iOffset += iWidth * fZoom;
		}
	}
	return iOffset;
}

void cd_do_render_cairo (CairoDock *pMainDock, cairo_t *pCairoContext)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pMainDock->iMagnitudeIndex);
	double fScale = (1. + fDockMagnitude * g_fAmplitude) / (1 + g_fAmplitude);
	
	if (myData.pCharList == NULL && myData.pListingHistory == NULL)  // aucune lettre de tapee => on montre le prompt.
	{
		if (! myData.bNavigationMode && myData.pPromptSurface != NULL)
		{
			double fFrameWidth = myData.iPromptWidth * fScale;
			double fFrameHeight = myData.iPromptHeight * fScale;
			
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
			fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth) / 2;
			fDockOffsetY = (pMainDock->iCurrentHeight - fFrameHeight) / 2;  // centre verticalement.
			
			fAlpha *= _alpha_prompt (myData.iPromptAnimationCount, s_iNbPromptAnimationSteps);
			
			if (fAlpha != 0)
			{
				cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);
				if (fScale != 1)
					cairo_scale (pCairoContext, fScale, fScale);
				cairo_set_source_surface (pCairoContext, myData.pPromptSurface, 0., 0.);
				cairo_paint_with_alpha (pCairoContext, fAlpha);
			}
		}
		else if (myData.bNavigationMode && myData.pArrowSurface != NULL)
		{
			double fFrameWidth = myData.iArrowWidth * fScale;
			double fFrameHeight = myData.iArrowHeight * fScale;
			
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
			fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth) / 2;
			fDockOffsetY = (pMainDock->iCurrentHeight - fFrameHeight) / 2;
			
			fAlpha *= _alpha_prompt (myData.iPromptAnimationCount, s_iNbPromptAnimationSteps);
			
			if (fAlpha != 0)
			{
				cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);
				if (fScale != 1)
					cairo_scale (pCairoContext, fScale, fScale);
				cairo_set_source_surface (pCairoContext, myData.pArrowSurface, 0., 0.);
				cairo_paint_with_alpha (pCairoContext, fAlpha);
			}
		}
	}
	else  // si du texte a ete entre, on le dessine, ainsi que eventuellement la liste des icones correspondantes.
	{
		// dessin des icones correspondantes.
		int iIconsWidth = 0, iNbIcons = 0;
		if (myData.pMatchingIcons != NULL)
		{
			// on determine au prealable la largeur des icones pour pouvoir les centrer.
			iIconsWidth = _cd_do_get_matching_icons_width (&iNbIcons);
			
			// dessin du fond des icones.
			double fFrameWidth = iIconsWidth * fScale;
			double fFrameHeight = pMainDock->iCurrentHeight/2 * fScale;
			double fRadius = fFrameHeight / 10;
			double fLineWidth = 0.;
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
			fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth) / 2;
			fDockOffsetY = (!myConfig.bTextOnTop ? 0. : pMainDock->iCurrentHeight - fFrameHeight);
			
			cairo_save (pCairoContext);
			cairo_translate (pCairoContext, fDockOffsetX -fRadius, fDockOffsetY);
			cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight);
			cairo_set_line_width (pCairoContext, fLineWidth);
			cairo_set_source_rgba (pCairoContext, myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha);
			cairo_fill (pCairoContext);
			cairo_restore (pCairoContext);
			
			// on les dessine.
			//int iOffsetX = _cd_do_get_icon_x (myData.pCurrentMatchingElement) + myData.iCurrentMatchingOffset * myData.iCurrentMatchingDirection - iIconsWidth/2;  // ecart au centre du debut de l'icone courante.
			//iOffsetX = - iOffsetX;
			int iOffsetX = myData.iCurrentMatchingOffset + iIconsWidth/2;
			while (iOffsetX > iIconsWidth)
				iOffsetX -= iIconsWidth;
			while (iOffsetX < 0)
				iOffsetX += iIconsWidth;
			
			double fIconScale = (iIconsWidth * fScale > pMainDock->iCurrentWidth ? (double) pMainDock->iCurrentWidth / (iIconsWidth * fScale) : 1.);
			double x0 = (pMainDock->iCurrentWidth - iIconsWidth * fScale * fIconScale) / 2;
			double x = x0 + iOffsetX * fIconScale * fScale;  // abscisse de l'icone courante.
			CairoDock *pParentDock;
			Icon *pIcon;
			int iWidth, iHeight;
			double fZoom;
			gboolean bRound = FALSE;
			GList *ic;
			for (ic = myData.pMatchingIcons; ic != NULL; ic = ic->next)
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer == NULL)  // icone pas encore chargee.
					continue;
				pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
				cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pParentDock), &iWidth, &iHeight);
				fZoom = fIconScale * pMainDock->iCurrentHeight/2 / iHeight;
				cairo_save (pCairoContext);
				
				//if (!bRound)
				{
					if (x < x0)
					{
						bRound = TRUE;
						x += iIconsWidth * fIconScale * fScale;
					}
					else if (x > x0 + iIconsWidth * fIconScale * fScale)
					{
						x -= iIconsWidth * fIconScale * fScale;
						bRound = TRUE;
					}
				}
				cairo_translate (pCairoContext,
					x - (iNbIcons & 1 ? iWidth * fZoom * fScale * fIconScale / 2 : 0.),
					(myConfig.bTextOnTop ?
						pMainDock->iCurrentHeight/2 :
						0.));
				cairo_scale (pCairoContext,
					fZoom * fScale,
					fZoom * fScale);
				cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0., 0.);
				cairo_paint_with_alpha (pCairoContext, (myData.pCurrentMatchingElement == ic ? 1. : .7));
				
				if (myData.pCurrentMatchingElement == ic)
				{
					fLineWidth = 4.;
					cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, iWidth-2*fRadius, iHeight-fLineWidth);
					cairo_set_line_width (pCairoContext, fLineWidth);
					cairo_set_source_rgba (pCairoContext, myConfig.pFrameColor[0]+.1, myConfig.pFrameColor[1]+.1, myConfig.pFrameColor[2]+.1, 1.);
					cairo_stroke (pCairoContext);
				}
				
				cairo_restore (pCairoContext);
				x += iWidth * fZoom * fScale;
			}
		}
		
		// dessin du fond du texte.
		double fFrameWidth = myData.iTextWidth * fScale;
		double fTextScale = (fFrameWidth > pMainDock->iCurrentWidth ? (double) pMainDock->iCurrentWidth / fFrameWidth : 1.);
		double fFrameHeight = myData.iTextHeight * fScale;
		double fRadius = fFrameHeight / 5 * myConfig.fFontSizeRatio;
		double fLineWidth = 0.;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth * fTextScale) / 2;
		fDockOffsetY = (myConfig.bTextOnTop ? 0. : pMainDock->iCurrentHeight - fFrameHeight);
		
		cairo_save (pCairoContext);
		cairo_translate (pCairoContext, fDockOffsetX -fRadius, fDockOffsetY);
		cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha);
		cairo_fill (pCairoContext);
		cairo_restore (pCairoContext);
		
		// dessin des lettres.
		CDChar *pChar;
		GList *c;
		for (c = myData.pCharList; c != NULL; c = c->next)
		{
			pChar = c->data;
			cairo_save (pCairoContext);
			
			cairo_translate (pCairoContext,
				(pChar->iCurrentX * fScale * fTextScale + pMainDock->iCurrentWidth/2),
				pChar->iCurrentY + (myConfig.bTextOnTop ?
					(myData.iTextHeight - pChar->iHeight) * fScale :
					pMainDock->iCurrentHeight - pChar->iHeight * fScale));  // aligne en bas.
			
			if (fScale * fTextScale != 1)
				cairo_scale (pCairoContext, fScale*fTextScale, fScale*fTextScale);
			cairo_set_source_surface (pCairoContext, pChar->pSurface, 0., 0.);
			cairo_paint_with_alpha (pCairoContext, fAlpha);
			
			cairo_restore (pCairoContext);
		}
	}
}


void cd_do_render_opengl (CairoDock *pMainDock)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pMainDock->iMagnitudeIndex);
	double fScale = (1. + fDockMagnitude * g_fAmplitude) / (1 + g_fAmplitude);
	
	if (myData.pCharList == NULL && myData.pListingHistory == NULL)  // aucune lettre de tapee => on montre le prompt.
	{
		if (! myData.bNavigationMode && myData.iPromptTexture != 0)
		{
			double fFrameWidth = myData.iPromptWidth * fScale;
			double fFrameHeight = myData.iPromptHeight * fScale;
			
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
			fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth) / 2;
			fDockOffsetY = (pMainDock->iCurrentHeight - fFrameHeight) / 2;
			
			fAlpha *= _alpha_prompt (myData.iPromptAnimationCount, s_iNbPromptAnimationSteps);
			
			if (fAlpha != 0)
			{
				glPushMatrix ();
				glTranslatef (pMainDock->iCurrentWidth/2, pMainDock->iCurrentHeight/2, 0.);
				
				_cairo_dock_enable_texture ();
				_cairo_dock_set_blend_alpha ();
				
				_cairo_dock_apply_texture_at_size_with_alpha (myData.iPromptTexture, fFrameWidth, fFrameHeight, fAlpha);
				
				_cairo_dock_disable_texture ();
				
				glPopMatrix();
			}
		}
		else if (myData.bNavigationMode && myData.iArrowTexture != 0)
		{
			double fFrameWidth = myData.iArrowWidth * fScale;
			double fFrameHeight = myData.iArrowHeight * fScale;
						
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
			fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth) / 2;
			fDockOffsetY = (pMainDock->iCurrentHeight - fFrameHeight) / 2;
			
			fAlpha *= _alpha_prompt (myData.iPromptAnimationCount, s_iNbPromptAnimationSteps);
			
			if (fAlpha != 0)
			{
				glPushMatrix ();
				glTranslatef (pMainDock->iCurrentWidth/2, pMainDock->iCurrentHeight/2, 0.);
				
				_cairo_dock_enable_texture ();
				_cairo_dock_set_blend_alpha ();
				
				_cairo_dock_apply_texture_at_size_with_alpha (myData.iArrowTexture, fFrameWidth, fFrameHeight, fAlpha);
				
				_cairo_dock_disable_texture ();
				
				glPopMatrix();
			}
		}
	}
	else  // si du texte a ete entre, on le dessine, ainsi que eventuellement la liste des icones correspondantes.
	{
		// dessin des icones correspondantes.
		int iIconsWidth = 0, iNbIcons = 0;
		if (myData.pMatchingIcons != NULL)
		{
			// on determine au prealable la largeur des icones pour povouir les centrer.
			iIconsWidth = _cd_do_get_matching_icons_width (&iNbIcons);
			
			// dessin du fond des icones.
			double fFrameWidth = iIconsWidth * fScale;
			double fFrameHeight = pMainDock->iCurrentHeight/2 * fScale;
			double fRadius = fFrameHeight / 10;
			double fLineWidth = 0.;
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
			fDockOffsetX = pMainDock->iCurrentWidth/2 - fFrameWidth / 2 - fRadius;
			fDockOffsetY = (!myConfig.bTextOnTop ? pMainDock->iCurrentHeight : fFrameHeight);
			double fFrameColor[4] = {myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha};
			glPushMatrix ();
			cairo_dock_draw_rounded_rectangle_opengl (fRadius, fLineWidth, fFrameWidth, fFrameHeight, fDockOffsetX, fDockOffsetY, fFrameColor);
			glPopMatrix ();
			
			// on les dessine.
			//int iOffsetX = _cd_do_get_icon_x (myData.pCurrentMatchingElement) + myData.iCurrentMatchingOffset * myData.iCurrentMatchingDirection - iIconsWidth/2;  // ecart au centre du debut de l'icone courante.
			//iOffsetX = - iOffsetX;
			//g_print ("myData.iCurrentMatchingOffset : %d\n", myData.iCurrentMatchingOffset);
			int iOffsetX = myData.iCurrentMatchingOffset + iIconsWidth/2;
			while (iOffsetX > iIconsWidth)
				iOffsetX -= iIconsWidth;
			while (iOffsetX < 0)
				iOffsetX += iIconsWidth;
			
			double fIconScale = (iIconsWidth * fScale > pMainDock->iCurrentWidth ? (double) pMainDock->iCurrentWidth / (iIconsWidth * fScale) : 1.);
			double x0 = (pMainDock->iCurrentWidth - iIconsWidth * fScale * fIconScale) / 2;
			double x = x0 - iOffsetX * fIconScale * fScale;  // abscisse de l'icone courante.
			CairoDock *pParentDock;
			Icon *pIcon;
			int iWidth, iHeight;
			double fZoom;
			gboolean bRound = FALSE;
			GList *ic;
			
			_cairo_dock_enable_texture ();
			_cairo_dock_set_blend_alpha ();
			_cairo_dock_set_alpha (1.);
			for (ic = myData.pMatchingIcons; ic != NULL; ic = ic->next)
			{
				pIcon = ic->data;
				if (pIcon->iIconTexture == 0)  // icone pas encore chargee.
					continue;
				pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
				cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pParentDock), &iWidth, &iHeight);
				fZoom = (double) pMainDock->iCurrentHeight/2 / iHeight;
				glPushMatrix ();
				
				//if (!bRound)
				{
					if (x < x0)
					{
						bRound = TRUE;
						x += iIconsWidth * fIconScale * fScale;
					}
					else if (x > x0 + iIconsWidth * fIconScale * fScale)
					{
						x -= iIconsWidth * fIconScale * fScale;
						bRound = TRUE;
					}
				}
				glTranslatef (x + (iNbIcons & 1 ? 0. : iWidth * fZoom/2 * fScale * fIconScale),
					(myConfig.bTextOnTop ?
						pMainDock->iCurrentHeight/4 :
						pMainDock->iCurrentHeight - iHeight * fZoom/2 * fScale),
					0.);
				_cairo_dock_apply_texture_at_size_with_alpha (pIcon->iIconTexture,
					iWidth * fZoom * fScale * fIconScale,
					pMainDock->iCurrentHeight/2 * fScale * fIconScale,
					(myData.pCurrentMatchingElement == ic ? 1. : .7));
				
				if (myData.pCurrentMatchingElement == ic)
				{
					_cairo_dock_disable_texture ();
					fLineWidth = 4.;
					double fFrameColor[4] = {myConfig.pFrameColor[0]+.1, myConfig.pFrameColor[1]+.1, myConfig.pFrameColor[2]+.1, 1.};
					cairo_dock_draw_rounded_rectangle_opengl (fRadius, fLineWidth, iWidth * fZoom * fScale * fIconScale - fLineWidth, iHeight * fZoom * fScale * fIconScale - fLineWidth, -iWidth/2 * fZoom * fScale * fIconScale + fLineWidth/2, iHeight * fZoom/2 * fScale * fIconScale - fLineWidth/2, fFrameColor);
					_cairo_dock_enable_texture ();
					_cairo_dock_set_blend_over ();
					_cairo_dock_set_alpha (1.);
				}
				
				glPopMatrix ();
				x += iWidth * fZoom * fScale * fIconScale;
			}
			_cairo_dock_disable_texture ();
		}
		
		// dessin du fond du texte.
		double fFrameWidth = myData.iTextWidth * fScale;
		double fTextScale = (fFrameWidth > pMainDock->iCurrentWidth ? (double) pMainDock->iCurrentWidth / fFrameWidth : 1.);
		double fFrameHeight = myData.iTextHeight * fScale;
		double fRadius = myBackground.iDockRadius * myConfig.fFontSizeRatio;
		double fLineWidth = 0.;
		
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = pMainDock->iCurrentWidth/2 - fFrameWidth * fTextScale / 2 - fRadius;
		fDockOffsetY = (myConfig.bTextOnTop ? pMainDock->iCurrentHeight : fFrameHeight);
		
		glPushMatrix ();
		double fFrameColor[4] = {myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha};
		cairo_dock_draw_rounded_rectangle_opengl (fRadius, 0, fFrameWidth, fFrameHeight, fDockOffsetX, fDockOffsetY, fFrameColor);
		glPopMatrix();
		
		// dessin des lettres.
		_cairo_dock_enable_texture ();
		_cairo_dock_set_blend_alpha ();
		_cairo_dock_set_alpha (fAlpha);
		
		cairo_dock_set_perspective_view (pMainDock->iCurrentWidth, pMainDock->iCurrentHeight);
		glTranslatef (-pMainDock->iCurrentWidth/2, -pMainDock->iCurrentHeight/2, 0.);
		glEnable (GL_DEPTH_TEST);
		
		CDChar *pChar;
		GList *c;
		for (c = myData.pCharList; c != NULL; c = c->next)
		{
			pChar = c->data;
			glPushMatrix();
			
			glTranslatef (pChar->iCurrentX * fScale * fTextScale + pMainDock->iCurrentWidth/2 + pChar->iWidth/2,
				(myConfig.bTextOnTop ?
					pMainDock->iCurrentHeight - (myData.iTextHeight - pChar->iHeight/2) * fScale :
					pChar->iHeight/2 * fScale),
				0.);  // aligne en bas.
			
			double fRotationAngle = pChar->fRotationAngle;
			if (myConfig.iAppearanceDuration != 0)
			{
				glBindTexture (GL_TEXTURE_2D, pChar->iTexture);
				
				glScalef (pChar->iWidth * fScale * fTextScale, pChar->iHeight * fScale * fTextScale, 1.);
				
				glRotatef (fRotationAngle, 1., 0., 0.);
				glRotatef (MIN (fRotationAngle/3, STATIC_ANGLE), 0., 0., 1.);
				glRotatef (MIN (fRotationAngle/3, STATIC_ANGLE), 0., 1., 0.);
				
				glPolygonMode (GL_FRONT, GL_FILL);
				double a = .5 / sqrt (2);
				glBegin(GL_QUADS);
				// Front Face (note that the texture's corners have to match the quad's corners)
				glNormal3f(0,0,1);
				glTexCoord2f (0., 0.); glVertex3f(-a,  a,  a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (1., 0.); glVertex3f( a,  a,  a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.); glVertex3f( a, -a,  a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.); glVertex3f(-a, -a,  a);  // Top Left Of The Texture and Quad
				// Back Face
				glNormal3f(0,0,-1);
				glTexCoord2f (1., 0.); glVertex3f( -a, a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.); glVertex3f( -a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.); glVertex3f(a, -a, -a);  // Top Left Of The Texture and Quad
				glTexCoord2f (0., 0.); glVertex3f(a, a, -a);  // Bottom Left Of The Texture and Quad
				// Top Face
				glNormal3f(0,1,0);
				glTexCoord2f (0., 1.); glVertex3f(-a,  a,  a);  // Top Left Of The Texture and Quad
				glTexCoord2f (0., 0.); glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (1., 0.); glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.); glVertex3f( a,  a,  a);  // Top Right Of The Texture and Quad
				// Bottom Face
				glNormal3f(0,-1,0);
				glTexCoord2f (1., 1.); glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.); glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
				glTexCoord2f (0., 0.); glVertex3f(-a, -a,  a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (1., 0.); glVertex3f( a, -a,  a);  // Bottom Right Of The Texture and Quad
				// Right face
				glNormal3f(1,0,0);
				glTexCoord2f (1., 0.);  glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.);  glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.);  glVertex3f( a, -a,  a);  // Top Left Of The Texture and Quad
				glTexCoord2f (0., 0.);  glVertex3f( a,  a,  a);  // Bottom Left Of The Texture and Quad
				// Left Face
				glNormal3f(-1,0,0);
				glTexCoord2f (0., 0.);  glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (1., 0.);  glVertex3f(-a,  a,  a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.);  glVertex3f(-a, -a,  a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.);  glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
				glEnd();
			}
			else
			{
				_cairo_dock_apply_texture_at_size (pChar->iTexture, pChar->iWidth, pChar->iHeight);
			}
			
			glPopMatrix();
		}
		cairo_dock_set_ortho_view (pMainDock->iCurrentWidth, pMainDock->iCurrentHeight);
		glTranslatef (-pMainDock->iCurrentWidth/2, -pMainDock->iCurrentHeight/2, 0.);
		glDisable (GL_DEPTH_TEST);
	}
}
