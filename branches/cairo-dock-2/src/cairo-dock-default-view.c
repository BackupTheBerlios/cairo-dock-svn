/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-animations.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-draw-opengl.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-log.h"
#include "cairo-dock-default-view.h"

#define RADIAN (G_PI / 180.0)  // Conversion Radian/Degres
#define DELTA_ROUND_DEGREE 1

extern double g_fSubDockSizeRatio;

extern gint g_iScreenWidth[2];
extern gint g_iScreenHeight[2];
extern gint g_iMaxAuthorizedWidth;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern gint g_iFrameMargin;
extern gint g_iStringLineWidth;
extern double g_fStringColor[4];

extern double g_fAmplitude;
extern CairoDockLabelDescription g_iconTextDescription;
extern cairo_surface_t *g_pDesktopBgSurface;

extern int g_iBackgroundTexture;


void cairo_dock_set_subdock_position_linear (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	//pSubDock->bDirectionUp = pDock->bDirectionUp;
	//g_print ("%s (%s, %d/%d ; %d/%d)\n", __func__, pPointedIcon->acName, pDock->bDirectionUp, pDock->bHorizontalDock, pSubDock->bDirectionUp, pSubDock->bHorizontalDock);
	///int iX = iMouseX + (-iMouseX + (pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2)) / 2;
	int iX = pPointedIcon->fXAtRest - (pDock->fFlatDockWidth - pDock->iMaxDockWidth) / 2 + pPointedIcon->fWidth / 2;
	//int iX = iMouseX + (iMouseX < pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2 ? (pDock->bDirectionUp ? 1 : 0) : (pDock->bDirectionUp ? 0 : -1)) * pPointedIcon->fWidth * pPointedIcon->fScale / 2;
	if (pSubDock->bHorizontalDock == pDock->bHorizontalDock)
	{
		pSubDock->fAlign = 0.5;
		pSubDock->iGapX = iX + pDock->iWindowPositionX - g_iScreenWidth[pDock->bHorizontalDock] / 2;  // les sous-dock ont un alignement egal a 0.5.  // pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2
		pSubDock->iGapY = pDock->iGapY + pDock->iMaxDockHeight;
	}
	else
	{
		pSubDock->fAlign = (pDock->bDirectionUp ? 1 : 0);
		pSubDock->iGapX = (pDock->iGapY + pDock->iMaxDockHeight) * (pDock->bDirectionUp ? -1 : 1);
		if (pDock->bDirectionUp)
			pSubDock->iGapY = g_iScreenWidth[pDock->bHorizontalDock] - (iX + pDock->iWindowPositionX) - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 1.
		else
			pSubDock->iGapY = iX + pDock->iWindowPositionX - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 0.
	}
}



void cairo_dock_calculate_max_dock_size_linear (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);

	pDock->iDecorationsHeight = pDock->iMaxIconHeight + 2 * g_iFrameMargin;

	double fRadius = MIN (g_iDockRadius, (pDock->iDecorationsHeight + g_iDockLineWidth) / 2 - 1);
	double fExtraWidth = g_iDockLineWidth + 2 * (fRadius + g_iFrameMargin);
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., fExtraWidth));
	///pDock->iMaxDockWidth = MIN (pDock->iMaxDockWidth, g_iMaxAuthorizedWidth);

	pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * pDock->iMaxIconHeight) + g_iconTextDescription.iSize + g_iDockLineWidth + g_iFrameMargin;

	pDock->iDecorationsWidth = pDock->iMaxDockWidth;

	pDock->iMinDockWidth = pDock->fFlatDockWidth + fExtraWidth;
	pDock->iMinDockHeight = pDock->iMaxIconHeight + 2 * g_iFrameMargin + 2 * g_iDockLineWidth;
}



void cairo_dock_calculate_construction_parameters_generic (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth)
{
	icon->fDrawX = icon->fX;
	icon->fDrawY = icon->fY;
	icon->fWidthFactor = 1.;
	icon->fHeightFactor = 1.;
	icon->fDeltaYReflection = 0.;
	icon->fOrientation = 0.;
	if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= iCurrentWidth)
	{
		icon->fAlpha = 1;
	}
	else
	{
		icon->fAlpha = .25;
	}
}

void cairo_dock_render_linear (cairo_t *pCairoContext, CairoDock *pDock)
{
	//\____________________ On trace le cadre.
	double fChangeAxes = 0.5 * (pDock->iCurrentWidth - pDock->iMaxDockWidth);
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	double fRadius = (pDock->iDecorationsHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDock->iDecorationsHeight + fLineWidth) / 2 - 1);
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);

	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + 0 - fMargin : fRadius + fLineWidth / 2);  // fChangeAxes
	if (fDockOffsetX - (fRadius + fLineWidth / 2) < 0)
		fDockOffsetX = fRadius + fLineWidth / 2;
	if (fDockOffsetX + fDockWidth + (fRadius + fLineWidth / 2) > pDock->iCurrentWidth)
		fDockWidth = pDock->iCurrentWidth - fDockOffsetX - (fRadius + fLineWidth / 2);
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - pDock->iDecorationsHeight - 1.5 * fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iDecorationsHeight + 1.5 * fLineWidth;
	}

	cairo_save (pCairoContext);
	cairo_dock_draw_frame (pCairoContext, fRadius, fLineWidth, fDockWidth, pDock->iDecorationsHeight, fDockOffsetX, fDockOffsetY, sens, 0., pDock->bHorizontalDock);

	//\____________________ On dessine les decorations dedans.
	fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY);

	//\____________________ On dessine le cadre.
	if (fLineWidth > 0)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
		cairo_new_path (pCairoContext);
	cairo_restore (pCairoContext);

	//\____________________ On dessine la ficelle qui les joint.
	if (g_iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, g_iStringLineWidth, FALSE, FALSE);

	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	fRatio = pDock->fRatio;
	cairo_dock_render_icons_linear (pCairoContext, pDock, fRatio);
}



void cairo_dock_render_optimized_linear (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ((%d;%d) x (%d;%d) / (%dx%d))\n", __func__, pArea->x, pArea->y, pArea->width, pArea->height, pDock->iCurrentWidth, pDock->iCurrentHeight);
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	int iWidth = pDock->iCurrentWidth;
	int iHeight = pDock->iCurrentHeight;

	//\____________________ On dessine les decorations du fond sur la portion de fenetre.
	cairo_save (pCairoContext);

	double fDockOffsetX, fDockOffsetY;
	if (pDock->bHorizontalDock)
	{
		fDockOffsetX = pArea->x;
		fDockOffsetY = (pDock->bDirectionUp ? iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	}
	else
	{
		fDockOffsetX = (pDock->bDirectionUp ? iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
		fDockOffsetY = pArea->y;
	}

	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	if (pDock->bHorizontalDock)
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, pDock->iDecorationsHeight);
	else
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pDock->iDecorationsHeight, pArea->height);

	fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY);


	//\____________________ On dessine la partie du cadre qui va bien.
	cairo_new_path (pCairoContext);

	if (pDock->bHorizontalDock)
	{
		cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY - fLineWidth / 2);
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);

		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, fDockOffsetX, (pDock->bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth));
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	}
	else
	{
		cairo_move_to (pCairoContext, fDockOffsetX - fLineWidth / 2, fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);

		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, (pDock->bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth), fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	}
	cairo_stroke (pCairoContext);

	cairo_restore (pCairoContext);

	//\____________________ On dessine les icones impactees.
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement != NULL)
	{
		double fXMin = (pDock->bHorizontalDock ? pArea->x : pArea->y), fXMax = (pDock->bHorizontalDock ? pArea->x + pArea->width : pArea->y + pArea->height);
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
		fRatio = pDock->fRatio;
		double fXLeft, fXRight;
		
		//g_print ("redraw [%d -> %d]\n", (int) fXMin, (int) fXMax);
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		do
		{
			icon = ic->data;

			fXLeft = icon->fDrawX + icon->fScale + 1;
			fXRight = icon->fDrawX + (icon->fWidth - 1) * icon->fScale * icon->fWidthFactor - 1;

			if (fXLeft < fXMax && fXRight > fXMin)
			{
				cairo_save (pCairoContext);
				//g_print ("dessin optimise de %s [%.2f -> %.2f]\n", icon->acName, fXLeft, fXRight);
				
				if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->iCurrentWidth)
				{
					icon->fAlpha = 1;
				}
				else
				{
					icon->fAlpha = .25;
				}
				
				if (icon->iAnimationType == CAIRO_DOCK_AVOID_MOUSE)
				{
					icon->fAlpha = 0.4;
				}
				cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE, pDock->iCurrentWidth, pDock->bDirectionUp);
				cairo_restore (pCairoContext);
			}

			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
}




void cairo_dock_render_opengl_linear (CairoDock *pDock)
{
	GLsizei w = pDock->iCurrentWidth;
	GLsizei h = pDock->iCurrentHeight;
	
	//\_____________ On definit notre rectangle.
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	double fRadius = (pDock->iDecorationsHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDock->iDecorationsHeight + fLineWidth) / 2 - 1);
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
	double fFrameHeight = pDock->iDecorationsHeight + fLineWidth - 2 * fRadius;
	
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL)
		return ;
	Icon *pFirstIcon = pFirstDrawnElement->data;
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + 0 - fMargin : fRadius + fLineWidth / 2);
	if (fDockOffsetX - (fRadius + fLineWidth / 2) < 0)
		fDockOffsetX = fRadius + fLineWidth / 2;
	if (fDockOffsetX + fDockWidth + (fRadius + fLineWidth / 2) > pDock->iCurrentWidth)
		fDockWidth = pDock->iCurrentWidth - fDockOffsetX - (fRadius + fLineWidth / 2);
	if (! pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - (fLineWidth/2 + fRadius);
	}
	else
	{
		sens = -1;
		fDockOffsetY = fLineWidth/2 + fRadius + fFrameHeight;
	}
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	double fRatio = pDock->fRatio;
	
	//\_____________ On genere les coordonnees du contour.
	GLfloat pVertexTab[((90/DELTA_ROUND_DEGREE+1)*4+1)*3];
	int iNbVertex = (90/DELTA_ROUND_DEGREE+1)*4;
	memset (pVertexTab, 0, (90/DELTA_ROUND_DEGREE+1)*4*3*sizeof (GLfloat));
	int i=0, t;
	int iPrecision = DELTA_ROUND_DEGREE;
	double fInclinaisonCadre = 0.;
	for (t = 0;t <= 90;t += iPrecision, i++) // Le cote haut droit 
	{ 
		pVertexTab[3*i] = fDockWidth/2 + fRadius * cos (t*RADIAN);
		pVertexTab[3*i+1] = fFrameHeight/2 + fRadius * sin (t*RADIAN);
	} 
	for (t = 90;t <= 180;t += iPrecision, i++) // Bas droit 
	{ 
		pVertexTab[3*i] = -fDockWidth/2 + fRadius * cos (t*RADIAN);
		pVertexTab[3*i+1] = fFrameHeight/2 + fRadius * sin (t*RADIAN);
	} 
	for (t = 180;t <= 270;t += iPrecision, i++) // Bas gauche 
	{ 
		pVertexTab[3*i] = -fDockWidth/2 + fRadius * cos (t*RADIAN);
		pVertexTab[3*i+1] = -fFrameHeight/2 + fRadius * sin (t*RADIAN);
	} 
	for (t = 270;t <= 360;t += iPrecision, i++) // Haut gauche 
	{ 
		pVertexTab[3*i] = fDockWidth/2 + fRadius * cos (t*RADIAN);
		pVertexTab[3*i+1] = -fFrameHeight/2 + fRadius * sin (t*RADIAN);
	}
	pVertexTab[3*i] = fDockWidth/2 + fRadius;  // on boucle.
	pVertexTab[3*i+1] = fFrameHeight/2;
	
	//\_____________ On definit l'etat courant.
	glDisable(GL_DEPTH_TEST);// On desactive le tampon de profondeur 
	
	glEnable(GL_TEXTURE_2D); // Je veux de la texture
	
	glBindTexture(GL_TEXTURE_2D, g_iBackgroundTexture); // allez on bind la texture
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR ); // ok la on selectionne le type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glEnable(GL_TEXTURE_GEN_S); // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T); // Et en T aussi
	
	glLoadIdentity();
	
	glTranslatef (fDockOffsetX + fDockWidth/2, fDockOffsetY - fFrameHeight/2, -pDock->iMaxIconHeight * (1 + g_fAmplitude) + 1);
	glRotatef (fInclinaisonCadre, 1.0f, 0.0f, 0.0); // Rotation ou pas selon trapeze ou autre 
	
	//\_____________ On trace en texturant par des triangles.
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Couleur a fond
	
	glBlendFunc (GL_SRC_ALPHA, 1.); // Transparence avec le canal alpha
	glEnable(GL_BLEND); // On active le blend
	//glEnable(GL_ALPHA_TEST); // On active l'alpha test
	//glAlphaFunc(GL_GREATER, 0.0f); // on affiche tout les pixels dont l'alpha est superieur a 0 
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (1., 1.);
	
	glBegin (GL_TRIANGLE_FAN);
	glVertex3f(0., 0., 0.0f);
	for (i = 0; i <= iNbVertex; i++) // La on affiche un polygone plein texture
	{
		glVertex3fv (&pVertexTab[3*i]);
	}
	glEnd();
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D); // Plus de texture merci 
	glDisable(GL_TEXTURE);
	
	//\_____________ On trace le contour.
	glPolygonMode(GL_FRONT, GL_LINE);
	glLineWidth(fLineWidth); // Ici on choisi l'epaisseur du contour du polygone 
	glColor4f(g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]); // Et sa couleur 
	glBegin(GL_LINE_LOOP);
	for (i = 0; i < iNbVertex; i++) // Et on affiche le contour 
	{
		glVertex3fv (&pVertexTab[3*i]);
	}
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	
	//\_____________ On dessine la ficelle.
	glLoadIdentity();
	int n = g_list_length (pDock->icons);
	GLfloat *buffer = g_new (GLfloat, 3*n);
	GLfloat **pStringCtrlPts = g_new (GLfloat *, n);
	if (pFirstDrawnElement != NULL)
	{
		i=0;
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		do
		{
			icon = ic->data;
			
			pStringCtrlPts[i] = &buffer[3*i];
			pStringCtrlPts[i][0] = icon->fDrawX + icon->fWidth * icon->fScale/2;
			pStringCtrlPts[i][1] = pDock->iCurrentHeight - icon->fDrawY - icon->fHeight * icon->fScale/2;
			pStringCtrlPts[i][2] = 0.;
			
			i ++;
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
	glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &pStringCtrlPts[0][0]);
	glEvalMesh1(GL_LINE, 0, 5*n);
	g_free (pStringCtrlPts);
	g_free (buffer);
	
	
	//\_____________ On dessine les icones.
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc (GL_SRC_ALPHA, 1.); // Transparence avec le canal alpha 
	glLoadIdentity();
	glTranslatef (0, 0, -pDock->iMaxIconHeight * (1 + g_fAmplitude) + 1);
	
	glEnable (GL_LIGHTING);  // pour indiquer a OpenGL qu'il devra prendre en compte l'eclairage.
	glLightModelf (GL_LIGHT_MODEL_TWO_SIDE, 1.0f);
	//glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);  // OpenGL doit considerer pour ses calculs d'eclairage que l'oeil est dans la scene (plus realiste).
	GLfloat fGlobalAmbientColor[4] = {0., 0., 0., 0.};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fGlobalAmbientColor);  // on definit la couleur de la lampe d'ambiance.
	glEnable (GL_LIGHT0);  // on allume la lampe 0.
	GLfloat fDiffuseColor[4] = {0.9, 0.9, 0.9, 1.};
	glLightfv (GL_LIGHT0, GL_AMBIENT, fDiffuseColor);  // GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR
	//glLightfv (GL_LIGHT0, GL_DIFFUSE, fDiffuseColor);
	GLfloat fSpecularColor[4] = {0.5, 0.5, 0.1, 1.};
	glLightfv (GL_LIGHT0, GL_SPECULAR, fSpecularColor);
	GLfloat fDirection[4] = {20, 20, -100., 0.};  // le dernier 0 <=> direction.
	glLightfv(GL_LIGHT0, GL_POSITION, fDirection);
	
	if (pFirstDrawnElement != NULL)
	{
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		do
		{
			icon = ic->data;
			
			glPushMatrix ();
			
			/**glBindTexture (GL_TEXTURE_2D, icon->iIconTexture);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Couleur a fond 
			glTranslatef (icon->fDrawX + icon->fScale * icon->fWidth/2, pDock->iCurrentHeight - icon->fDrawY - icon->fScale * icon->fHeight/2, 0.);
			glRotatef (icon->iRotationY, 0, 1, 0);
			//glScalef (icon->fScale * icon->fWidth/2/sqrt(2.), icon->fScale * icon->fHeight/2/sqrt(2.), icon->fScale * icon->fHeight/2/sqrt(2.));  // pour le cube.
			glScalef (icon->fScale * icon->fWidth, icon->fScale * icon->fHeight, icon->fScale * icon->fHeight);  // pour la pastille.*/
			
			
			/*glBegin(GL_QUADS);
			// Front Face (note that the texture's corners have to match the quad's corners)
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1,  1.0f);  // Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  1.0f);  // Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  1.0f);  // Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  -1,  1.0f);  // Top Left Of The Texture and Quad
			
			// Back Face
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1, -1.0f);  // Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f,  -1, -1.0f);  // Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( -1.0f, -1, -1.0f);  // Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( -1.0f, 1, -1.0f);  // Bottom Left Of The Texture and Quad
			
			// Top Face
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1, 1.0f);  // Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1,  -1.0f);  // Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1,  -1.0f);  // Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1, 1.0f);  // Top Right Of The Texture and Quad
			
			// Bottom Face
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1, -1.0f);  // Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( -1.0f, -1, -1.0f);  // Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( -1.0f, -1,  1.0f);  // Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1,  1.0f);  // Bottom Right Of The Texture and Quad
			
			// Right face
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1, -1.0f);  // Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  -1.0f, -1.0f);  // Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  -1.0f,  1.0f);  // Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, 1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
			
			// Left Face
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  -1.0f,  1.0f);  // Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
			glEnd();*/
			
			///glCallList(CAIRO_DOCK_CAPSULE_MESH); // Et hop on affiche la capsule
			
			cairo_dock_render_one_icon_opengl (icon, pDock, fRatio, fDockMagnitude, TRUE);
			
			glPopMatrix ();
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
	glDisable (GL_LIGHTING);
}


Icon *cairo_dock_calculate_icons_linear (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);

	CairoDockMousePositionType iMousePositionType = cairo_dock_check_if_mouse_inside_linear (pDock);

	cairo_dock_manage_mouse_position (pDock, iMousePositionType);

	//\____________________ On calcule les position/etirements/alpha des icones.
	cairo_dock_mark_avoiding_mouse_icons_linear (pDock);

	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cairo_dock_calculate_construction_parameters_generic (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth);
		cairo_dock_manage_animations (icon, pDock);
	}

	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
}

void cairo_dock_register_default_renderer (void)
{
	CairoDockRenderer *pDefaultRenderer = g_new0 (CairoDockRenderer, 1);
	pDefaultRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-default-view", CAIRO_DOCK_SHARE_DATA_DIR);
	pDefaultRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-default.png", CAIRO_DOCK_SHARE_DATA_DIR);
	pDefaultRenderer->calculate_max_dock_size = cairo_dock_calculate_max_dock_size_linear;
	pDefaultRenderer->calculate_icons = cairo_dock_calculate_icons_linear;
	pDefaultRenderer->render = cairo_dock_render_linear;
	pDefaultRenderer->render_optimized = cairo_dock_render_optimized_linear;
	pDefaultRenderer->render_opengl = cairo_dock_render_opengl_linear;
	pDefaultRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pDefaultRenderer->bUseReflect = FALSE;

	cairo_dock_register_renderer (CAIRO_DOCK_DEFAULT_RENDERER_NAME, pDefaultRenderer);
}
