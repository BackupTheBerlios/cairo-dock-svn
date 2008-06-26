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
#include <gtk/gtkgl.h>
#include <GL/glu.h>

#include "cairo-dock-animations.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-default-view.h"
#include "cairo-dock-log.h"

#define        RADIAN    (G_PI / 180.0)    // Conversion Radian/Degres 
typedef struct 
{ 
    GLfloat x, y, z; 
} sVertex;

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
extern int g_iLabelSize;
extern gboolean g_bUseOpenGL;
extern int g_iBackgroundTexture;

void cairo_dock_set_subdock_position_linear (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
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

	pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * pDock->iMaxIconHeight) + g_iLabelSize + g_iDockLineWidth + g_iFrameMargin;

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







// du polygone de base avec les cotes arrondis

void    GenerateVertex(sVertex *Temp, double ellipse, double fLongueurCadre, double fHauteurCadre, double fRayonY) 
{ 
    int i=0, t; 

    //Temp = TableauVertex; 
    int iPrecision=10;
    double fRayonX=g_iDockRadius;

    for (t = 0;t <= 90;t += iPrecision, i++) // Le cote haut droit 
    { 
        Temp[i].x = (fRayonX+ellipse) * sin(t*RADIAN) + (fLongueurCadre-ellipse)/2+(1.0f-fRayonX); 
        Temp[i].y = (fRayonY+ellipse/2) * cos(t*RADIAN) + (fHauteurCadre-ellipse/2)/2+(1.0f-fRayonY); 
        Temp[i].z = 0.0f; 

    } 
    for (t = 90;t <= 180;t += iPrecision, i++) // Bas droit 
    { 
        Temp[i].x = fRayonX * sin(t*RADIAN) + fLongueurCadre/2+(1.0f-fRayonX); 
        Temp[i].y = fRayonY * cos(t*RADIAN) - fHauteurCadre/2-(1.0f-fRayonY); 
        Temp[i].z = 0.0f; 
    } 
    for (t = 180;t <= 270;t += iPrecision, i++) // Bas gauche 
    { 
        Temp[i].x = fRayonX * sin(t*RADIAN) - fLongueurCadre/2-(1.0f-fRayonX); 
        Temp[i].y = fRayonY * cos(t*RADIAN) - fHauteurCadre/2-(1.0f-fRayonY); 
        Temp[i].z = 0.0f; 
    } 
    for (t = 270;t <= 360;t += iPrecision, i++) // Haut gauche 
    { 
        Temp[i].x = (fRayonX+ellipse) * sin(t*RADIAN) - (fLongueurCadre-ellipse)/2-(1.0f-fRayonX); 
        Temp[i].y = (fRayonY+ellipse/2) * cos(t*RADIAN) + (fHauteurCadre-ellipse/2)/2+(1.0f-fRayonY); 
        Temp[i].z = 0.0f; 
    } 
    Temp[i].x = (fRayonX+ellipse) * sin(0*RADIAN) + (fLongueurCadre-ellipse)/2+(1.0f-fRayonX ); // On rajoute ca pour boucler le polygone 
    Temp[i].y = (fRayonY+ellipse/2) * cos(0*RADIAN) + (fHauteurCadre-ellipse/2)/2+(1.0f-fRayonY); 
    Temp[i].z = 0.0f; 
}


// La fonction pour changer les formes

gboolean ChangeShape(sVertex *Temp, char forme, double ellipse, double fLongueurCadre, double fHauteurCadre, double *_fInclinaisonCadre, double fRayonY)
{ 
    float                    fStepMorph        = 1000.0f; // Ca c'est pour la vitesse de transformation des shapes 
    float    StepEllipse = fLongueurCadre / 2 / fStepMorph; 
    float    StepInclinaison = 75.0f / fStepMorph; 
    float    StepRayon = g_iDockRadius / fStepMorph; 
float fInclinaisonCadre = *_fInclinaisonCadre;
    if(forme == 0) // Passage en mode Curve 
    { 

        ellipse += StepEllipse; 
        fInclinaisonCadre += StepInclinaison; 
        if (fInclinaisonCadre >= 0.0f) 
            fInclinaisonCadre = 0.0f; 
        if (ellipse >= fLongueurCadre / 2) 
        { 
            ellipse = fLongueurCadre / 2; 
            GenerateVertex(Temp, ellipse, fLongueurCadre, fHauteurCadre, fRayonY);
           // if (fInclinaisonCadre == 0.0f) 
            //    return FALSE; 
           // else 
            //    return TRUE; 
        } 
        else 
        { 
            GenerateVertex(Temp, ellipse, fLongueurCadre, fHauteurCadre, fRayonY); 
            //return    TRUE; 
        } 
    } 
    else if (forme == 1) // Passage au mode barre de face 
    { 
        ellipse -= StepEllipse; 
        fInclinaisonCadre += StepInclinaison; 
        if (fInclinaisonCadre >= 0.0f) 
            fInclinaisonCadre = 0.0f; 
        if (ellipse <= 0.0f) 
        { 
            ellipse = 0.0f; 
            GenerateVertex(Temp, ellipse, fLongueurCadre, fHauteurCadre, fRayonY); 
            //if (fInclinaisonCadre == 0.0f) 
               // return FALSE; 
           // else 
               // return TRUE; 
        } 
        else 
        { 
            GenerateVertex(Temp, ellipse, fLongueurCadre, fHauteurCadre, fRayonY); 
            //return TRUE; 
        } 
    } 
    else if (forme == 2) // Passage en mode trapeze 
    { 
        ellipse -= StepEllipse; 
        if (ellipse <= 0.0) 
            ellipse = 0.0f; 
        fInclinaisonCadre -= StepInclinaison; 
        if (fInclinaisonCadre <= -75.0f) 
        { 
            ellipse = 0.0f; 
            fInclinaisonCadre = -75.0f; 
            GenerateVertex(Temp, ellipse, fLongueurCadre, fHauteurCadre, fRayonY); 
           // return FALSE; 
        } 
        else 
        { 
            GenerateVertex(Temp, ellipse, fLongueurCadre, fHauteurCadre, fRayonY); 
            //return TRUE; 
        } 
    } 
    *_fInclinaisonCadre = fInclinaisonCadre;
} 

void cairo_dock_render_linear (CairoDock *pDock)
{
	cairo_t *pCairoContext;
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	double fRadius = (pDock->iDecorationsHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDock->iDecorationsHeight + fLineWidth) / 2 - 1);
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	Icon *pFirstIcon = pFirstDrawnElement->data;
	
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pGlitzDrawable)
	{
		glitz_context_t *ctx = glitz_context_create (pDock->pGlitzDrawable, pDock->pDrawFormat);
		g_print ("ctx:%d\n", ctx);
		
		glitz_context_make_current (ctx, pDock->pGlitzDrawable);
		
		GLsizei w = pDock->iCurrentWidth;
		GLsizei h = pDock->iCurrentHeight;
		glViewport(0, 0, w, h);
		
// 		glMatrixMode(GL_PROJECTION);
// 		glLoadIdentity();
// 		glOrtho(0, w, 0, h, 0.0, 500.0);
		
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		gluLookAt (w/2, h/2, 3,
			w/2, h/2, 0.,
			0.0f, 1.0f, 0.0f);
		glTranslatef (0.0f, 0.0f, -10);
		
		
		static float alpha = 0;
		glBlendFunc(GL_ZERO, GL_ZERO);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepth(1.0f);
		glFlush ();
		
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glColor4f(0.0f, 0.0f, 0.0f, .5);
		
		Icon *icon = pDock->icons->data;
		glBindTexture (GL_TEXTURE_2D, icon->iColorBuffer);
		glBegin(GL_QUADS);
			// Front Face (note that the texture's corners have to match the quad's corners)
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
		glEnd ();
		glFlush ();
		
		/*glPushMatrix ();
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef (0, 0, -10);
		glColor3f(1.0f, 1.0f, 1.0f);
				
				glBegin(GL_QUADS);
				// Front Face (note that the texture's corners have to match the quad's corners)
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
				
				// Back Face 
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
				
				// Top Face
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
				
				// Bottom Face
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				
				// Right face
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				
				// Left Face
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glEnd();
		
		glPopMatrix ();
		
		
		glFlush ();
		*/
		glitz_context_destroy (ctx);
		if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
			glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
		return ;
	}
#endif
	if (g_bUseOpenGL)
	{
		GdkGLContext *pGlContext = gtk_widget_get_gl_context (pDock->pWidget);
		GdkGLDrawable *pGlDrawable = gtk_widget_get_gl_drawable (pDock->pWidget);
		if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
			return;
		
		static float alpha = 0;
		GLsizei w = pDock->iCurrentWidth;
		GLsizei h = pDock->iCurrentHeight;
		
		glBlendFunc(GL_ZERO, GL_ZERO);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFlush ();
		
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		
		/*float            fRayonX = fRadius; // Ton rayon X de tes rebords 
		float            fRayonY = fRayonX; // Et le Y tu verras pourquoi j ai fait ca au lieu d'u seul 
		float                    fLongueurCadre    = fDockWidth; // La longueur de ton rectangle 
		float                    fHauteurCadre    = pDock->iMaxIconHeight; // La hauteur 
		static float                    fInclinaisonCadre = 45.0f; // Petite variable pour l'incliner et donner l'effet trapeze 
		float                    ellipse            = 0.0f; // Une petite variable pour la forme elliptique 
		int                iPrecision        = 10; // Precision c'est le nombre de points pour les rebords  ----> le pas en degres.
		gboolean                bGenerate = TRUE; // ca c'est pour savoir si on va generer ou pas une nouvelle figure 
		char                cShape=0; // pour les differentes formes 0=ellipse 1=trapeze 2=rectangle 
		
		sVertex TableauVertex [360/iPrecision * sizeof(sVertex) + 50];
		
		int t;
		
		glDisable(GL_DEPTH_TEST);// On desactive le tampon de profondeur 
		
		///glLoadIdentity(); // Matrice d'identite please 
		///glTranslatef(0.0f, -3.0f, -10.0f); // Petite translation pour tout voir 
		glRotatef(fInclinaisonCadre, 1.0f, 0.0f, 0.0); // Rotation ou pas selon trapeze ou autre 
		glEnable(GL_TEXTURE_2D); // Je veux de la texture 
		
		glBindTexture(GL_TEXTURE_2D, g_iBackgroundTexture); // allez on bind la texture 
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR ); // ok la on selectionne le type de generation des coordonnees de la texture 
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR ); 
		glEnable(GL_TEXTURE_GEN_S); // oui je veux une generation en S 
		glEnable(GL_TEXTURE_GEN_T); // Et en T aussi 
		
		glColor3f(1.0f, 1.0f, 1.0f); // Couleur a fond 
		
		if (bGenerate) // Est ce que je dois regenerer mon polygone 
		{ 
			// Vertex *Temp, char forme, double ellipse, double fLongueurCadre, double fHauteurCadre, double fInclinaisonCadre, double fRayonY
			bGenerate = ChangeShape(TableauVertex, cShape, ellipse, fLongueurCadre, fHauteurCadre, &fInclinaisonCadre, fRayonY); // Si oui bein go 
		} 
		
		sVertex *Temp = TableauVertex; // Hop je pointe sur mon tableau de vertex 
		
		for (t = 0;t <= 360/iPrecision+3;t++) // La on affiche un polygone plein texture 
		{ 
			Temp = &TableauVertex[t];
			glBegin(GL_POLYGON); 
			glVertex3fv(Temp); 
			
			Temp = &TableauVertex[t+1];
			glVertex3f(fLongueurCadre/2, fHauteurCadre/2, 0.0f); 
			glVertex3fv(Temp); 
			glEnd(); 
		} 
		glDisable(GL_TEXTURE_2D); // Plus de texture merci 
		glDisable(GL_TEXTURE); 
		
		
		glLineWidth(fLineWidth); // Ici on choisi l'epaisseur du contour du polygone 
		glColor3f(g_fLineColor[0], g_fLineColor[1], g_fLineColor[2]); // Et sa couleur 
		Temp = TableauVertex; // Je pointe sur mon tableau de vertex 
		for (t = 0;t <= 360/iPrecision+3;t ++) // Et on affiche le contour 
		{
			Temp = &TableauVertex[t];
			glBegin(GL_LINE_LOOP);
			glVertex3fv(Temp);
			
			Temp = &TableauVertex[t+1];
			glVertex3fv(Temp);
			glEnd();
		}*/
		
		
		//\_____________ On definit notre rectangle.
		double fLineWidth = g_iDockLineWidth;
		double fMargin = g_iFrameMargin;
		double fRadius = (pDock->iDecorationsHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDock->iDecorationsHeight + fLineWidth) / 2 - 1);
		double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
		
		int sens;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		//Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
		fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + 0 - fMargin : fRadius + fLineWidth / 2);
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
		
		//\_____________ On genere les coordonnees du contour.
		GLfloat pVertexTab[((90/10+1)*4+1)*3];
		int iNbVertex = (90/10+1)*4;
		memset (pVertexTab, 0, (90/10+1)*4*3*sizeof (GLfloat));
		int i=0, t;
		int iPrecision = 10;
		double fInclinaisonCadre = 0.;
		for (t = 0;t <= 90;t += iPrecision, i++) // Le cote haut droit 
		{ 
			pVertexTab[3*i] = fDockWidth/2 + fRadius * cos (t*RADIAN);
			pVertexTab[3*i+1] = pDock->iDecorationsHeight/2 + fRadius * sin (t*RADIAN);
		} 
		for (t = 90;t <= 180;t += iPrecision, i++) // Bas droit 
		{ 
			pVertexTab[3*i] = -fDockWidth/2 + fRadius * cos (t*RADIAN);
			pVertexTab[3*i+1] = pDock->iDecorationsHeight/2 + fRadius * sin (t*RADIAN);
		} 
		for (t = 180;t <= 270;t += iPrecision, i++) // Bas gauche 
		{ 
			pVertexTab[3*i] = -fDockWidth/2 + fRadius * cos (t*RADIAN);
			pVertexTab[3*i+1] = -pDock->iDecorationsHeight/2 + fRadius * sin (t*RADIAN);
		} 
		for (t = 270;t <= 360;t += iPrecision, i++) // Haut gauche 
		{ 
			pVertexTab[3*i] = fDockWidth/2 + fRadius * cos (t*RADIAN);
			pVertexTab[3*i+1] = -pDock->iDecorationsHeight/2 + fRadius * sin (t*RADIAN);
		}
		pVertexTab[3*i] = fDockWidth/2 + fRadius;  // on boucle.
		pVertexTab[3*i+1] = pDock->iDecorationsHeight/2;
		
		//\_____________ On definit l'etat courant.
		glDisable(GL_DEPTH_TEST);// On desactive le tampon de profondeur 
		
		glRotatef (fInclinaisonCadre, 1.0f, 0.0f, 0.0); // Rotation ou pas selon trapeze ou autre 
		glEnable(GL_TEXTURE_2D); // Je veux de la texture 
		
		glBindTexture(GL_TEXTURE_2D, g_iBackgroundTexture); // allez on bind la texture 
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR ); // ok la on selectionne le type de generation des coordonnees de la texture 
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR ); 
		glEnable(GL_TEXTURE_GEN_S); // oui je veux une generation en S 
		glEnable(GL_TEXTURE_GEN_T); // Et en T aussi 
		
		glColor3f(1.0f, 1.0f, 1.0f); // Couleur a fond 
		
		glLoadIdentity();
		glTranslatef (pDock->iCurrentWidth/2, pDock->iMaxIconHeight/2, -pDock->iMaxIconHeight * (1 + g_fAmplitude) + 1);
		
		//\_____________ On trace en texturant par des triangles.
		for (i = 0; i <= iNbVertex; i++) // La on affiche un polygone plein texture
		{
			glBegin (GL_POLYGON);
			glVertex3fv (&pVertexTab[3*i]);
			
			glVertex3f(0., 0., 0.0f);
			
			glVertex3fv(&pVertexTab[3*(i+1)]);
			glEnd();
		}
		glDisable(GL_TEXTURE_2D); // Plus de texture merci 
		glDisable(GL_TEXTURE); 
		
		
		glLineWidth(fLineWidth); // Ici on choisi l'epaisseur du contour du polygone 
		glColor3f(g_fLineColor[0], g_fLineColor[1], g_fLineColor[2]); // Et sa couleur 
		for (i = 0; i <= iNbVertex; i++) // Et on affiche le contour 
		{
			glBegin(GL_LINE_LOOP);
			glVertex3fv (&pVertexTab[3*i]);
			
			glVertex3fv (&pVertexTab[3*(i+1)]);
			glEnd();
		}
		
		
		
		
		
		
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		
		/*glLineWidth (2.0);
		w -= 4;
		h -= 4;
		glTranslatef (2, 2, 0);
		glDisable (GL_TEXTURE_2D);
		GLfloat pFrameCtrlPts[4][4][3] = {
			{ {10, 0, 0}, {10, 0, 0}, {w-10, 0, 0}, {w-10, 0, 0} },
			{ {0, 10, 0}, {0, 10, 0}, {w, 10, 0}, {w, 10, 0} },
			{ {0, h-10, 0}, {0, h-10, 0}, {w, h-10, 0}, {w, h-10, 0} },
			{ {10, h, 0}, {10, h, 0}, {w-10, h, 0}, {w-10, h, 0} } };
		
		glMap2f (GL_MAP2_VERTEX_3,
			0, 1.,
			3,
			4,
			0, 1.,
			12,
			4,
			&pFrameCtrlPts[0][0][0]);
		glEnable(GL_MAP2_VERTEX_3);
		glEnable(GL_AUTO_NORMAL);
		glEnable(GL_NORMALIZE);
		glMapGrid2f (2,
			0.0, 1.,
			2,
			0.0, 1.);
		
		glPushMatrix ();
		glEvalMesh2(GL_LINE, 0, 2, 0, 2);
		double t[4] = {0., 10./h, 1-10./h, 1.};
		int Nx=4, Ny=4;
		int i, j;
		for( j = 0 ; j <= Ny ; j++ )
		{
			glBegin(GL_LINE_STRIP);
			for( i = 0 ; i <= Nx ; i++ )
				glEvalCoord2f((float)i/Nx, t[j]);
			glEnd();
			glBegin(GL_LINE_STRIP);
			for(i = 0; i <= Nx; i++)
				glEvalCoord2f((float)t[j], (float)i/Nx);
			glEnd();
		}	
		
		
		glPopMatrix ();
		glFlush ();
		glEnable (GL_TEXTURE_2D);*/
		
		
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);
		glLoadIdentity();
		glTranslatef (0, 0, -pDock->iMaxIconHeight * (1 + g_fAmplitude) + 1);
		if (pFirstDrawnElement != NULL)
		{
			Icon *icon;
			GList *ic = pFirstDrawnElement;
			do
			{
				icon = ic->data;
				glBindTexture (GL_TEXTURE_2D, icon->iColorBuffer);
				
				glPushMatrix ();
				
				glTranslatef (icon->fDrawX, pDock->iCurrentHeight - icon->fDrawY - icon->fScale * icon->fHeight/2, 0.);
				glRotatef (alpha, 1, 0, 0);
				glScalef (icon->fScale * icon->fWidth/sqrt(2.), icon->fScale * icon->fHeight/2/sqrt(2.), icon->fScale * icon->fHeight/2/sqrt(2.));
				//glColor3f(0.5f, 0.5f, 1.0f);
				glColor3f(1.0f, 1.0f, 1.0f);
				
				glBegin(GL_QUADS);
				// Front Face (note that the texture's corners have to match the quad's corners)
				glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 1,  1.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1,  1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1,  1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f,  -1,  1.0f);  // Top Left Of The Texture and Quad
				
				// Back Face 
				glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1, -1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f,  1, -1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f, 1, -1.0f);  // Top Left Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f, -1, -1.0f);  // Bottom Left Of The Texture and Quad
				
				// Top Face
				glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f,  1, 1.0f);  // Top Left Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f,  1,  -1.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1,  -1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1, 1.0f);  // Top Right Of The Texture and Quad
				
				// Bottom Face
				glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1, -1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f, -1, -1.0f);  // Top Left Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f, -1,  1.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1,  1.0f);  // Bottom Right Of The Texture and Quad
				
				// Right face
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1, -1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
				glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				
				// Left Face
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.0f, 1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glEnd();
				
				glPopMatrix ();
				
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
		}
		
		glFlush ();
		
		alpha = alpha + 2;
		
		gdk_gl_drawable_swap_buffers (pGlDrawable);
		gdk_gl_drawable_gl_end (pGlDrawable);
		return ;
	}
	
	
	//\____________________ On cree le contexte du dessin.
	pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);

	cairo_set_tolerance (pCairoContext, 0.5);  // avec moins que 0.5 on ne voit pas la difference.
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	//\____________________ On trace le cadre.

	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	//Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + 0 - fMargin : fRadius + fLineWidth / 2);
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
	cairo_restore (pCairoContext);

	//\____________________ On dessine la ficelle qui les joint.
	if (g_iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, g_iStringLineWidth, FALSE, FALSE);

	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	fRatio = pDock->fRatio;
	cairo_dock_render_icons_linear (pCairoContext, pDock, fRatio);

	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}



void cairo_dock_render_optimized_linear (CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ((%d;%d) x (%d;%d) / (%dx%d))\n", __func__, pArea->x, pArea->y, pArea->width, pArea->height, pDock->iCurrentWidth, pDock->iCurrentHeight);
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	int iWidth = pDock->iCurrentWidth;
	int iHeight = pDock->iCurrentHeight;

	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_rectangle (pCairoContext,
		pArea->x,
		pArea->y,
		pArea->width,
		pArea->height);
	cairo_clip (pCairoContext);
	cairo_set_tolerance (pCairoContext, 0.5);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);

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
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);


	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement != NULL)
	{
		double fXMin = (pDock->bHorizontalDock ? pArea->x : pArea->y), fXMax = (pDock->bHorizontalDock ? pArea->x + pArea->width : pArea->y + pArea->height);
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
		fRatio = pDock->fRatio;
		double fXLeft, fXRight;

		Icon *icon;
		GList *ic = pFirstDrawnElement;
		do
		{
			icon = ic->data;

			fXLeft = icon->fDrawX;
			fXRight = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor;

			if (fXLeft <= fXMax && floor (fXRight) > fXMin)
			{
				cairo_save (pCairoContext);
				//g_print ("dessin optimise de %s\n", icon->acName);
				
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

	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
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
	pDefaultRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pDefaultRenderer->bUseReflect = FALSE;

	cairo_dock_register_renderer (CAIRO_DOCK_DEFAULT_RENDERER_NAME, pDefaultRenderer);
}
