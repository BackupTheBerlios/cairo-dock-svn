/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>
#include <pango/pango.h>
#include <gtk/gtk.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-icons.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-draw.h"

extern CairoDock *g_pMainDock;
extern GHashTable *g_hDocksTable;
extern double g_fSubDockSizeRatio;
extern gboolean g_bAutoHide;
extern gboolean g_bTextAlwaysHorizontal;

extern gint g_iScreenWidth[2];
extern gint g_iScreenHeight[2];
extern gint g_iMaxAuthorizedWidth;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern gint g_iFrameMargin;
extern gint g_iStringLineWidth;
extern double g_fStringColor[4];
extern double g_fReflectSize;

extern gboolean g_bRoundedBottomCorner;
extern gboolean g_bDirectionUp;
extern double g_fStripesSpeedFactor;
extern double g_fBackgroundImageWidth, g_fBackgroundImageHeight;
extern cairo_surface_t *g_pBackgroundSurface[2];
extern cairo_surface_t *g_pBackgroundSurfaceFull[2];

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneAlpha;
extern double g_fAmplitude;

extern int g_iLabelSize;
extern gboolean g_bLabelForPointedIconOnly;
extern double g_fLabelAlphaThreshold;

extern double g_fAlphaAtRest;
extern double g_fVisibleAppliAlpha;

extern int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS];
extern gboolean g_bDynamicReflection;
extern double g_fAlbedo;
extern gboolean g_bConstantSeparatorSize;


void cairo_dock_set_colormap_for_window (GtkWidget *pWidget)
{
	GdkScreen* pScreen = gtk_widget_get_screen (pWidget);
	GdkColormap* pColormap = gdk_screen_get_rgba_colormap (pScreen);
	if (!pColormap)
		pColormap = gdk_screen_get_rgb_colormap (pScreen);
		
	gtk_widget_set_colormap (pWidget, pColormap);
}


double cairo_dock_get_current_dock_width_linear (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		//return 2 * g_iDockRadius + g_iDockLineWidth + 2 * g_iFrameMargin;
		return 1 + 2 * g_iFrameMargin;
	
	Icon *pLastIcon = cairo_dock_get_last_drawn_icon (pDock);
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	double fWidth = pLastIcon->fX - pFirstIcon->fX + pLastIcon->fWidth * pLastIcon->fScale + 2 * g_iFrameMargin;  //  + 2 * g_iDockRadius + g_iDockLineWidth + 2 * g_iFrameMargin
	
	return fWidth;
}
/*void cairo_dock_get_current_dock_width_height (CairoDock *pDock, double *fWidth, double *fHeight)
{
	if (pDock->icons == NULL)
	{
		*fWidth = 2 * g_iDockRadius + g_iDockLineWidth + 2 * g_iFrameMargin;
		*fHeight = 2 * g_iDockRadius + g_iDockLineWidth + 2 * g_iFrameMargin;
	}
	
	Icon *pLastIcon = cairo_dock_get_last_drawn_icon (pDock);
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	*fWidth = pLastIcon->fX - pFirstIcon->fX + 2 * g_iDockRadius + g_iDockLineWidth + 2 * g_iFrameMargin + pLastIcon->fWidth * pLastIcon->fScale;
	if (pLastIcon->fY > pFirstIcon->fY)
		*fHeight = MAX (pLastIcon->fY + pLastIcon->fHeight * pLastIcon->fScale - pFirstIcon->fY, pFirstIcon->fHeight * pFirstIcon->fScale);
	else
		*fHeight = MAX (pFirstIcon->fY + pFirstIcon->fHeight * pFirstIcon->fScale - pLastIcon->fY, pLastIcon->fHeight * pLastIcon->fScale);
}*/

/**
*Cree un contexte de dessin pour la libcairo. Si glitz est active, le contexte sera lie a une surface glitz (et donc on dessinera directement sur la carte graphique via OpenGL), sinon a une surface X representant la fenetre du dock.
*@param pDock le dock sur lequel on veut dessiner.
*@Returns le contexte sur lequel dessiner. N'est jamais nul; tester sa coherence avec #cairo_status avant de l'utiliser, et le detruire avec #cairo_destroy apres en avoir fini avec lui.
*/
cairo_t * cairo_dock_create_context_from_window (CairoDock *pDock)
{
#ifdef HAVE_GLITZ
	if (pDock->pGlitzDrawable)
	{
		//g_print ("creation d'un contexte lie a une surface glitz\n");
		glitz_surface_t* pGlitzSurface;
		cairo_surface_t* pCairoSurface;
		cairo_t* pCairoContext;
		
		pGlitzSurface = glitz_surface_create (pDock->pGlitzDrawable,
			pDock->pGlitzFormat,
			pDock->iCurrentWidth,
			pDock->iCurrentHeight,
			0,
			NULL);
		
		if (pDock->pDrawFormat->doublebuffer)
			glitz_surface_attach (pGlitzSurface,
				pDock->pGlitzDrawable,
				GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
		else
			glitz_surface_attach (pGlitzSurface,
				pDock->pGlitzDrawable,
				GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
		
		pCairoSurface = cairo_glitz_surface_create (pGlitzSurface);
		pCairoContext = cairo_create (pCairoSurface);
		
		cairo_surface_destroy (pCairoSurface);
		glitz_surface_destroy (pGlitzSurface);
		
		return pCairoContext;
	}
#endif // HAVE_GLITZ
	return gdk_cairo_create (pDock->pWidget->window);
}

static void cairo_dock_draw_frame_horizontal (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination)  // la largeur est donnee par rapport "au fond".
{
	if (2*fRadius > fFrameHeight + fLineWidth)
		fRadius = (fFrameHeight + fLineWidth) / 2 - 1;
	double fDeltaXForLoop = (fFrameHeight + fLineWidth - 2 * fRadius) * fInclination;
	double cosa = 1. / sqrt (1 + fInclination * fInclination);
	double sina = cosa * fInclination;
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	
	cairo_rel_line_to (pCairoContext, fFrameWidth, 0);
	//\_________________ Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius * (1. / cosa - fInclination), 0,
		fRadius * cosa, sens * fRadius * (1 - sina));
	cairo_rel_line_to (pCairoContext, fDeltaXForLoop, sens * (fFrameHeight + fLineWidth - fRadius * (g_bRoundedBottomCorner ? 2 : 1 - sina)));
	//\_________________ Coin bas droit.
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			fRadius * (1 + sina) * fInclination, sens * fRadius * (1 + sina),
			-fRadius * cosa, sens * fRadius * (1 + sina));
	
	cairo_rel_line_to (pCairoContext, - fFrameWidth -  2 * fDeltaXForLoop - (g_bRoundedBottomCorner ? 0 : 2 * fRadius * cosa), 0);
	//\_________________ Coin bas gauche.
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			-fRadius * (fInclination + 1. / cosa), 0,
			-fRadius * cosa, -sens * fRadius * (1 + sina));
	cairo_rel_line_to (pCairoContext, fDeltaXForLoop, sens * (- fFrameHeight - fLineWidth + fRadius * (g_bRoundedBottomCorner ? 2 : 1 - sina)));
	//\_________________ Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius * (1 - sina) * fInclination, -sens * fRadius * (1 - sina),
		fRadius * cosa, -sens * fRadius * (1 - sina));
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
}
static void cairo_dock_draw_frame_vertical (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination)
{
	if (2*fRadius > fFrameHeight + fLineWidth)
		fRadius = (fFrameHeight + fLineWidth) / 2 - 1;
	double fDeltaXForLoop = (fFrameHeight + fLineWidth - 2 * fRadius) * fInclination;
	double cosa = 1. / sqrt (1 + fInclination * fInclination);
	double sina = cosa * fInclination;
	
	cairo_move_to (pCairoContext, fDockOffsetY, fDockOffsetX);
	
	cairo_rel_line_to (pCairoContext, 0, fFrameWidth);
	//\_________________ Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, fRadius * (1. / cosa - fInclination),
		sens * fRadius * (1 - sina), fRadius * cosa);
	cairo_rel_line_to (pCairoContext, sens * (fFrameHeight + fLineWidth - fRadius * (g_bRoundedBottomCorner ? 2 : 1)), fDeltaXForLoop);
	//\_________________ Coin bas droit.
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			sens * fRadius * (1 + sina), fRadius * (1 + sina) * fInclination,
			sens * fRadius * (1 + sina), -fRadius * cosa);
	
	cairo_rel_line_to (pCairoContext, 0, - fFrameWidth -  2 * fDeltaXForLoop);
	//\_________________ Coin bas gauche.
	if (g_bRoundedBottomCorner)
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			0, -fRadius * (fInclination + 1. / cosa),
			-sens * fRadius * (1 + sina), -fRadius * cosa);
	cairo_rel_line_to (pCairoContext, sens * (- fFrameHeight - fLineWidth + fRadius * (g_bRoundedBottomCorner ? 2 : 1)), fDeltaXForLoop);
	//\_________________ Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-sens * fRadius * (1 - sina), fRadius * (1 - sina) * fInclination,
		-sens * fRadius * (1 - sina), fRadius * cosa);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
}
/**
*Trace sur le contexte un contour trapezoidale aux coins arrondis. Le contour n'est pas dessine, mais peut l'etre a posteriori, et peut servir de cadre pour y dessiner des choses dedans.
*@param pCairoContext le contexte du dessin, contenant le cadre a la fin de la fonction.
*@param fRadius le rayon en pixels des coins.
*@param fLineWidth l'epaisseur en pixels du contour.
*@param fFrameWidth la largeur de la plus petite base du trapeze.
*@param fFrameHeight la hauteur du trapeze.
*@param fDockOffsetX un decalage, dans le sens de la largeur du dock, a partir duquel commencer a tracer la plus petite base du trapeze.
*@param fDockOffsetY un decalage, dans le sens de la hauteur du dock, a partir duquel commencer a tracer la plus petite base du trapeze.
*@param sens 1 pour un tracer dans le sens des aiguilles d'une montre (indirect), -1 sinon.
*@param fInclination tangente de l'angle d'inclinaison des cotes du trapeze par rapport a la vertical. 0 pour tracer un rectangle.
*@param bHorizontal CAIRO_DOCK_HORIZONTAL ou CAIRO_DOCK_VERTICAL suivant l'horizontalité du dock.
*/
void cairo_dock_draw_frame (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination, gboolean bHorizontal)
{
	if (bHorizontal)
		cairo_dock_draw_frame_horizontal (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclination);
	else
		cairo_dock_draw_frame_vertical (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclination);
}

/**
*Dessine les decorations d'un dock a l'interieur d'un cadre prealablement trace sur le contexte.
*@param pCairoContext le contexte du dessin, est laisse intact par la fonction.
*@param pDock le dock sur lequel appliquer les decorations.
*@param fOffsetY un decalage, dans le sens de la hauteur du dock, a partir duquel appliquer les decorations.
*/
void cairo_dock_render_decorations_in_frame (cairo_t *pCairoContext, CairoDock *pDock, double fOffsetY)
{
	//g_print ("%.2f\n", pDock->fDecorationsOffsetX);
	if (g_pBackgroundSurfaceFull[pDock->bHorizontalDock] != NULL)
	{
		cairo_save (pCairoContext);
		
		if (pDock->bHorizontalDock)
			cairo_translate (pCairoContext, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor - pDock->iCurrentWidth * 0.5, fOffsetY);
		else
			cairo_translate (pCairoContext, fOffsetY, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor - pDock->iCurrentWidth * 0.5);
		
		cairo_set_source_surface (pCairoContext, g_pBackgroundSurfaceFull[pDock->bHorizontalDock], 0., 0.);
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
	else if (g_pBackgroundSurface[pDock->bHorizontalDock] != NULL)
	{
		cairo_save (pCairoContext);
		
		if (pDock->bHorizontalDock)
		{
			cairo_translate (pCairoContext, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor, fOffsetY);
			cairo_scale (pCairoContext, 1. * pDock->iCurrentWidth / g_fBackgroundImageWidth, 1. * pDock->iDecorationsHeight / g_fBackgroundImageHeight);
		}
		else
		{
			cairo_translate (pCairoContext, fOffsetY, pDock->fDecorationsOffsetX * g_fStripesSpeedFactor);
			cairo_scale (pCairoContext, 1. * pDock->iDecorationsHeight / g_fBackgroundImageHeight, 1. * pDock->iCurrentWidth / g_fBackgroundImageWidth);
		}
		
		//g_print ("(%dx%d) / (%dx%d)\n", pDock->iCurrentWidth, pDock->iMaxIconHeight, (int) g_fBackgroundImageWidth, (int) g_fBackgroundImageHeight);
		cairo_set_source_surface (pCairoContext, g_pBackgroundSurface[pDock->bHorizontalDock], 0., 0.);
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
	}
}

/**
*Dessine entierement une icone, dont toutes les caracteristiques ont ete prealablement calculees. Gere sa position, sa transparence (modulee par la transparence du dock au repos), son reflet, son placement de profil, son etiquette, et son info-rapide.
*@param 
*/
void cairo_dock_manage_animations (Icon *icon, CairoDock *pDock)
{
	
	//\_____________________ On gere l'animation de rebond.
	if (icon->iAnimationType == CAIRO_DOCK_BOUNCE && icon->iCount > 0)
	{
		int n = g_tNbIterInOneRound[CAIRO_DOCK_BOUNCE];  // nbre d'iteration pour 1 aplatissement+montree+descente.
		int k = n - (icon->iCount % n) - 3;  // 3 iterations pour s'aplatir.
		n -= 3;   // nbre d'iteration pour 1 montree+descente.
		
		if (k > 0)
		{
			double fPossibleDeltaY = MIN (100, (g_bDirectionUp ? icon->fDrawY : pDock->iCurrentHeight - (icon->fDrawY + icon->fHeight * icon->fScale)));  // on borne a 100 pixels pour les rendus qui ont des fenetres grandes..
			
			icon->fDeltaYReflection = (g_bDirectionUp ? -1. : 1.) * k / (n/2) * fPossibleDeltaY * (2 - 1.*k/(n/2));
			icon->fDrawY += icon->fDeltaYReflection;
			icon->fDeltaYReflection *= 1.25;  // le reflet "rebondira" de 25% de la hauteur au sol.
			//g_print ("%d) + %.2f (%d)\n", icon->iCount, (g_bDirectionUp ? -1. : 1.) * k / (n/2) * fPossibleDeltaY * (2 - 1.*k/(n/2)), k);
		}
		else  // on commence par s'aplatir.
		{
			icon->fHeightFactor *= 1.*(2 - 1.5*k) / 10;
			icon->fDeltaYReflection = (g_bDirectionUp ? 1 : -1) * (1 - icon->fHeightFactor) / 2 * icon->fHeight * icon->fScale;
			icon->fDrawY += icon->fDeltaYReflection;
			//g_print ("%d) * %.2f (%d)\n", icon->iCount, icon->fHeightFactor, k);
		}
		icon->iCount --;  // c'est une loi de type acceleration dans le champ de pesanteur. 'g' et 'v0' n'interviennent pas directement, car s'expriment en fonction de 'fPossibleDeltaY' et 'n'.
	}
	
	//\_____________________ On gere l'animation de rotation sur elle-meme.
	if (icon->iAnimationType == CAIRO_DOCK_ROTATE && icon->iCount > 0)
	{
		int c = icon->iCount;
		int n = g_tNbIterInOneRound[CAIRO_DOCK_ROTATE] / 4;  // nbre d'iteration pour 1/2 tour.
		if ((c/n) & 1)
		{
			icon->fWidthFactor *= ((c/(2*n)) & 1 ? 1. : -1.) * (c%n) / n;
		}
		else
		{
			icon->fWidthFactor *= ((c/(2*n)) & 1 ? 1. : -1.) * ((c%n) - n) / n;
		}
		icon->iCount --;
	}
	
	//\_____________________ On gere l'animation de rotation horizontale.
	if (icon->iAnimationType == CAIRO_DOCK_UPSIDE_DOWN && icon->iCount > 0)
	{
		int c = icon->iCount;
		int n = g_tNbIterInOneRound[CAIRO_DOCK_UPSIDE_DOWN] / 4;  // nbre d'iteration pour 1/2 tour.
		if ((c/n) & 1)
		{
			icon->fHeightFactor *= ((c/(2*n)) & 1 ? 1. : -1.) * (c%n) / n;
		}
		else
		{
			icon->fHeightFactor *= ((c/(2*n)) & 1 ? 1. : -1.) * ((c%n) - n) / n;
		}
		icon->iCount --;
	}
	
	//\_____________________ On gere l'animation wobbly.
	if (icon->iAnimationType == CAIRO_DOCK_WOBBLY && icon->iCount > 0)  // merci a Tshirtman cette animation !
	{
		int c = icon->iCount;
		int n = g_tNbIterInOneRound[CAIRO_DOCK_WOBBLY] / 4;  // nbre d'iteration pour 1 etirement/retrecissement.
		int k = c%n;
		
		double fMinSize = .3, fMaxSize = MIN (1.75, pDock->iCurrentHeight / icon->fWidth);  // au plus 1.75, soit 3/8 de l'icone qui deborde de part et d'autre de son emplacement. c'est suffisamment faible pour ne pas trop empieter sur ses voisines.
		
		double fSizeFactor = ((c/n) & 1 ? 1. / (n - k) : 1. / (1 + k));
		//double fSizeFactor = ((c/n) & 1 ? 1.*(k+1)/n : 1.*(n-k)/n);
		fSizeFactor = (fMinSize - fMaxSize) * fSizeFactor + fMaxSize;
		if ((c/(2*n)) & 1)
		{
			icon->fWidthFactor *= fSizeFactor;
			icon->fHeightFactor *= fMinSize;
			//g_print ("%d) width <- %.2f ; height <- %.2f (%d)\n", c, icon->fWidthFactor, icon->fHeightFactor, k);
		}
		else
		{
			icon->fHeightFactor *= fSizeFactor;
			icon->fWidthFactor *= fMinSize;
			//g_print ("%d) height <- %.2f ; width <- %.2f (%d)\n", c, icon->fHeightFactor, icon->fWidthFactor, k);
		}
		icon->iCount --;
	}
	
	//\_____________________ On gere l'animation de l'icone qui suit ou evite le curseur.
	if (icon->iAnimationType == CAIRO_DOCK_FOLLOW_MOUSE)
	{
		icon->fScale = 1 + g_fAmplitude;
		icon->fDrawX = pDock->iMouseX  - icon->fWidth * icon->fScale / 2;
		icon->fDrawY = pDock->iMouseY - icon->fHeight * icon->fScale / 2 ;
		icon->fAlpha = 0.4;
	}
	else if (icon->iAnimationType == CAIRO_DOCK_AVOID_MOUSE)
	{
		icon->fAlpha = 0.4;
		icon->fDrawX += icon->fWidth / 2 * (icon->fScale - 1) / g_fAmplitude * (icon->fPhase < G_PI/2 ? -1 : 1);
	}
	
	//\_____________________ On gere l'animation d'ondelette.
	if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_PULSE)
	{
		icon->fAlpha = 1. * (icon->iCount % g_tNbIterInOneRound[CAIRO_DOCK_PULSE]) / g_tNbIterInOneRound[CAIRO_DOCK_PULSE];
		icon->iCount --;
	}
	
	//\_____________________ On gere l'animation de clignotement.
	if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_BLINK)
	{
		int c = icon->iCount;
		int n = g_tNbIterInOneRound[CAIRO_DOCK_BLINK] / 2;  // nbre d'iteration pour une inversion d'alpha.
		if ( (c/n) & 1)
			icon->fAlpha *= 1. * (c%n) / n;
		else
			icon->fAlpha *= 1. * (n - 1 - (c%n)) / n;
		icon->fAlpha *= icon->fAlpha;  // pour accentuer.
		icon->iCount --;
	}
	
	
	if (icon->fWidthFactor >= 0 && icon->fWidthFactor < 0.05)
		icon->fWidthFactor = 0.05;
	else if (icon->fWidthFactor < 0 && icon->fWidthFactor > -0.05)
		icon->fWidthFactor = -0.05;
	
	icon->fDrawX += (1 - icon->fWidthFactor) / 2 * icon->fWidth * icon->fScale;
	
	if (icon->fHeightFactor >= 0 && icon->fHeightFactor < 0.05)
		icon->fHeightFactor = 0.05;
	else if (icon->fHeightFactor < 0 && icon->fHeightFactor > -0.05)
		icon->fHeightFactor = -0.05;
	
	icon->fDrawY += (1 - icon->fHeightFactor) / 2 * icon->fHeight * icon->fScale;
}


/**
*Dessine entierement une icone, dont toutes les caracteristiques ont ete prealablement calculees. Gere sa position, sa transparence (modulee par la transparence du dock au repos), son reflet, son placement de profil, son etiquette, et son info-rapide.
*@param icon l'icone a dessiner.
*@param pCairoContext le contexte du dessin, est altere pendant le dessin.
*@param bHorizontalDock l'horizontalite du dock contenant l'icone.
*@param fRatio le ratio de taille des icones dans ce dock.
*@param fDockMagnitude la magnitude actuelle du dock.
*@param bUseReflect TRUE pour dessiner les reflets.
*/
void cairo_dock_render_one_icon (Icon *icon, cairo_t *pCairoContext, gboolean bHorizontalDock, double fRatio, double fDockMagnitude, gboolean bUseReflect, gboolean bUseText)
{
	//\_____________________ On separe 2 cas : dessin avec le tampon complet, et dessin avec le ou les petits tampons.
	if (CAIRO_DOCK_IS_APPLI (icon) && ! icon->bIsHidden)
		icon->fAlpha *= g_fVisibleAppliAlpha;
	gboolean bDrawFullBuffer;
	bDrawFullBuffer  = (bUseReflect && icon->pFullIconBuffer != NULL && (! g_bDynamicReflection || icon->fScale == 1) && (icon->iCount == 0 || icon->iAnimationType == CAIRO_DOCK_ROTATE || icon->iAnimationType == CAIRO_DOCK_BLINK));
	int iCurrentWidth= 1;
	//\_____________________ On dessine l'icone en fonction de son placement, son angle, et sa transparence.
	//cairo_push_group (pCairoContext);
	//g_print ("%s (%.2f;%.2f)\n", __func__, icon->fDrawX, icon->fDrawY);
	if (bHorizontalDock)
	{
		cairo_translate (pCairoContext, icon->fDrawX, icon->fDrawY);
		cairo_save (pCairoContext);
		if (bDrawFullBuffer && ! g_bDirectionUp)
			cairo_translate (pCairoContext, 0, - g_fReflectSize * icon->fScale);
		if (g_bConstantSeparatorSize && CAIRO_DOCK_IS_SEPARATOR (icon))
		{
			cairo_translate (pCairoContext, fRatio * icon->fWidthFactor * icon->fWidth * (icon->fScale - 1) / 2, (g_bDirectionUp ? fRatio * icon->fHeightFactor * icon->fHeight * (icon->fScale - 1) : 0));
			cairo_scale (pCairoContext, fRatio * icon->fWidthFactor / (1 + g_fAmplitude), fRatio * icon->fHeightFactor / (1 + g_fAmplitude));
		}
		else
			cairo_scale (pCairoContext, fRatio * icon->fWidthFactor * icon->fScale / (1 + g_fAmplitude), fRatio * icon->fHeightFactor * icon->fScale / (1 + g_fAmplitude));
		if (icon->fOrientation != 0)
		{
			cairo_rotate (pCairoContext, icon->fOrientation);
		}
	}
	else
	{
		cairo_translate (pCairoContext, icon->fDrawY, icon->fDrawX);
		cairo_save (pCairoContext);
		if (bDrawFullBuffer && ! g_bDirectionUp)
			cairo_translate (pCairoContext, - g_fReflectSize * icon->fScale, 0);
		if (g_bConstantSeparatorSize && CAIRO_DOCK_IS_SEPARATOR (icon))
		{
			cairo_translate (pCairoContext, fRatio * icon->fHeightFactor * icon->fHeight * (icon->fScale - 1) / 2, (g_bDirectionUp ? fRatio * icon->fWidthFactor * icon->fWidth * (icon->fScale - 1) : 0));
			cairo_scale (pCairoContext, fRatio * icon->fHeightFactor / (1 + g_fAmplitude), fRatio * icon->fWidthFactor / (1 + g_fAmplitude));
		}
		else
			cairo_scale (pCairoContext, fRatio * icon->fHeightFactor * icon->fScale / (1 + g_fAmplitude), fRatio * icon->fWidthFactor * icon->fScale / (1 + g_fAmplitude));
		if (icon->fOrientation != 0)
			cairo_rotate (pCairoContext, icon->fOrientation);
	}
	
	double fPreviousAlpha = icon->fAlpha;
	if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_PULSE)
	{
		if (icon->fAlpha > 0)
		{
			cairo_save (pCairoContext);
			double fScaleFactor = 1 + (1 - icon->fAlpha);
			if (bHorizontalDock)
				cairo_translate (pCairoContext, icon->fWidth / fRatio * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fHeight / fRatio * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
			else
				cairo_translate (pCairoContext, icon->fHeight / fRatio * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fWidth / fRatio * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
			cairo_scale (pCairoContext, fScaleFactor, fScaleFactor);
			if (icon->pIconBuffer != NULL)
				cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
			cairo_paint_with_alpha (pCairoContext, icon->fAlpha);
			cairo_restore (pCairoContext);
		}
		icon->fAlpha = .8;
	}
	
	double fAlpha = icon->fAlpha * (fDockMagnitude + g_fAlphaAtRest * (1 - fDockMagnitude));
	
	if (bUseReflect && icon->pReflectionBuffer != NULL)  // on dessine les reflets.
	{
		if (bDrawFullBuffer)  // on les dessine d'un bloc.
		{
			cairo_set_source_surface (pCairoContext, icon->pFullIconBuffer, 0.0, 0.0);
			if (fAlpha == 1)
				cairo_paint (pCairoContext);
			else
				cairo_paint_with_alpha (pCairoContext, fAlpha);
		}
		else  // on les dessine separement, en gerant eventuellement dynamiquement la transparence du reflet.
		{
			if (icon->pIconBuffer != NULL)
				cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
			if (fAlpha == 1)
				cairo_paint (pCairoContext);
			else
				cairo_paint_with_alpha (pCairoContext, fAlpha);
			
			cairo_restore (pCairoContext);  // retour juste apres la translation (fDrawX, fDrawY).
			
			cairo_save (pCairoContext);
			if (bHorizontalDock)
			{
				cairo_translate (pCairoContext, 0, - icon->fDeltaYReflection + (g_bDirectionUp ? icon->fHeight * icon->fScale : - g_fReflectSize * icon->fScale));
				cairo_scale (pCairoContext, fRatio * icon->fWidthFactor * icon->fScale / (1 + g_fAmplitude), fRatio * icon->fHeightFactor * icon->fScale / (1 + g_fAmplitude));
			}
			else
			{
				cairo_translate (pCairoContext, - icon->fDeltaYReflection + (g_bDirectionUp ? icon->fHeight * icon->fScale : - g_fReflectSize * icon->fScale), 0);
				cairo_scale (pCairoContext, fRatio * icon->fHeightFactor * icon->fScale / (1 + g_fAmplitude), fRatio * icon->fWidthFactor * icon->fScale / (1 + g_fAmplitude));
			}
			
			if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_PULSE)
			{
				if (fPreviousAlpha > 0)
				{
					cairo_save (pCairoContext);
					double fScaleFactor = 1 + (1 - fPreviousAlpha);
					if (bHorizontalDock)
						cairo_translate (pCairoContext, icon->fWidth * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fHeight * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
					else
						cairo_translate (pCairoContext, icon->fHeight * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2, icon->fWidth * (1 - fScaleFactor) * (1 + g_fAmplitude) / 2);
					cairo_scale (pCairoContext, fScaleFactor, fScaleFactor);
					if (icon->pIconBuffer != NULL)
						cairo_set_source_surface (pCairoContext, icon->pReflectionBuffer, 0.0, 0.0);
					cairo_paint_with_alpha (pCairoContext, fPreviousAlpha);
					cairo_restore (pCairoContext);
				}
			}
			
			cairo_set_source_surface (pCairoContext, icon->pReflectionBuffer, 0.0, 0.0);
			
			if (g_bDynamicReflection && icon->fScale > 1)
			{
				cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
					0.,
					0.,
					g_fReflectSize * (1 + g_fAmplitude) / icon->fScale);  // de haut en bas.
				g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
				
				cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.,
					0.,
					0.,
					0.,
					(g_bDirectionUp ? 1. : 1 - (icon->fScale - 1) / g_fAmplitude));  // astuce pour ne pas avoir a re-creer la surface de la reflection.
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					1.,
					0.,
					0.,
					0.,
					(g_bDirectionUp ? 1 - (icon->fScale - 1) / g_fAmplitude : 1.));
				
				cairo_save (pCairoContext);
				cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
				cairo_translate (pCairoContext, 0, 0);
				cairo_mask (pCairoContext, pGradationPattern);
				cairo_restore (pCairoContext);
				
				cairo_pattern_destroy (pGradationPattern);
			}
			else
			{
				if (fAlpha == 1)
					cairo_paint (pCairoContext);
				else
					cairo_paint_with_alpha (pCairoContext, fAlpha);
			}
		}
	}
	else  // on dessine l'icone tout simplement.
	{
		if (icon->pIconBuffer != NULL)
			cairo_set_source_surface (pCairoContext, icon->pIconBuffer, 0.0, 0.0);
		if (fAlpha == 1)
			cairo_paint (pCairoContext);
		else
			cairo_paint_with_alpha (pCairoContext, fAlpha);
	}
	//cairo_pop_group (pCairoContext);
	
	cairo_restore (pCairoContext);  // retour juste apres la translation (fDrawX, fDrawY).
	
	cairo_save (pCairoContext);
	
	//\_____________________ On dessine les etiquettes, avec un alpha proportionnel au facteur d'echelle de leur icone.
	if (bUseText && icon->pTextBuffer != NULL && icon->fScale > 1.01 && (! g_bLabelForPointedIconOnly || icon->bPointed))  // 1.01 car sin(pi) = 1+epsilon :-/
	{
		if (! bHorizontalDock && g_bTextAlwaysHorizontal)
		{
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				0,
				0);
		}
		else if (bHorizontalDock)
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				-icon->fTextXOffset + icon->fWidthFactor * icon->fWidth * icon->fScale * 0.5,
				g_bDirectionUp ? -g_iLabelSize : icon->fHeight * icon->fScale + icon->fTextYOffset);
		else
			cairo_set_source_surface (pCairoContext,
				icon->pTextBuffer,
				g_bDirectionUp ? -g_iLabelSize : icon->fHeight * icon->fScale + icon->fTextYOffset,
				-icon->fTextXOffset + icon->fWidthFactor * icon->fWidth * icon->fScale * 0.5);
		
		double fMagnitude;
		if (g_bLabelForPointedIconOnly)
		{
			fMagnitude = fDockMagnitude;  // (icon->fScale - 1) / g_fAmplitude / sin (icon->fPhase);  // sin (phi ) != 0 puisque fScale > 1.
		}
		else
		{
			fMagnitude = (icon->fScale - 1) / g_fAmplitude;
			fMagnitude = (fMagnitude > 1. - 1. / g_fLabelAlphaThreshold ? 1.0 : fRatio / (1. - fMagnitude) / g_fLabelAlphaThreshold);
		}
		cairo_paint_with_alpha (pCairoContext, fMagnitude);
	}
	
	//\_____________________ On dessine les infos additionnelles.
	cairo_restore (pCairoContext);  // retour juste apres la translation (fDrawX, fDrawY).
	if (icon->pQuickInfoBuffer != NULL)
	{
		cairo_translate (pCairoContext,
			//-icon->fQuickInfoXOffset + icon->fWidth / 2,
			//icon->fHeight - icon->fQuickInfoYOffset);
			(- icon->iQuickInfoWidth + icon->fWidth) / 2 * icon->fScale,
			(icon->fHeight - icon->iQuickInfoHeight) * icon->fScale);
		
		cairo_scale (pCairoContext,
			fRatio * icon->fScale / (1 + g_fAmplitude),
			fRatio * icon->fScale / (1 + g_fAmplitude));
		
		cairo_set_source_surface (pCairoContext,
			icon->pQuickInfoBuffer,
			0,
			0);
		cairo_paint (pCairoContext);
	}
}


/**
*Dessine une ficelle reliant le centre de toutes les icones, en commencant par la 1ere dessinee.
*@param pCairoContext le contexte du dessin, n'est pas altere par la fonction.
*@param pDock le dock contenant les icônes a relier.
*@param fStringLineWidth epaisseur de la ligne.
*@param bIsLoop TRUE si on veut boucler (relier la derniere icone a la 1ere).
*/
void cairo_dock_draw_string (cairo_t *pCairoContext, CairoDock *pDock, double fStringLineWidth, gboolean bIsLoop, gboolean bForceConstantSeparator)
{
	bForceConstantSeparator = bForceConstantSeparator || g_bConstantSeparatorSize;
	GList *ic, *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL || fStringLineWidth <= 0)
		return ;
	
	cairo_save (pCairoContext);
	Icon *prev_icon = NULL, *next_icon, *icon;
	double x, y, fCurvature = 0.3;
	if (bIsLoop)
	{
		ic = cairo_dock_get_previous_element (pFirstDrawnElement, pDock->icons);
		prev_icon = ic->data;
	}
	ic = pFirstDrawnElement;
	icon = ic->data;
	GList *next_ic;
	double x1, x2, x3;
	double y1, y2, y3;
	double dx, dy;
	x = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor / 2;
	y = icon->fDrawY + icon->fHeight * icon->fScale / 2 + (bForceConstantSeparator && CAIRO_DOCK_IS_SEPARATOR (icon) ? icon->fHeight * (icon->fScale - 1) / 2 : 0);
	if (pDock->bHorizontalDock)
		cairo_move_to (pCairoContext, x, y);
	else
		cairo_move_to (pCairoContext, y, x);
	do
	{
		if (prev_icon != NULL)
		{
			x1 = prev_icon->fDrawX + prev_icon->fWidth * prev_icon->fScale * prev_icon->fWidthFactor / 2;
			y1 = prev_icon->fDrawY + prev_icon->fHeight * prev_icon->fScale / 2 + (bForceConstantSeparator && CAIRO_DOCK_IS_SEPARATOR (prev_icon) ? prev_icon->fHeight * (prev_icon->fScale - 1) / 2 : 0);
		}
		else
		{
			x1 = x;
			y1 = y;
		}
		prev_icon = icon;
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
		if (ic == pFirstDrawnElement && ! bIsLoop)
			break;
		icon = ic->data;
		x2 = icon->fDrawX + icon->fWidth * icon->fScale * icon->fWidthFactor / 2;
		y2 = icon->fDrawY + icon->fHeight * icon->fScale / 2 + (bForceConstantSeparator && CAIRO_DOCK_IS_SEPARATOR (icon) ? icon->fHeight * (icon->fScale - 1) / 2 : 0);
		
		dx = x2 - x;
		dy = y2 - y;
		
		next_ic = cairo_dock_get_next_element (ic, pDock->icons);
		next_icon = (next_ic == pFirstDrawnElement && ! bIsLoop ? NULL : next_ic->data);
		if (next_icon != NULL)
		{
			x3 = next_icon->fDrawX + next_icon->fWidth * next_icon->fScale * next_icon->fWidthFactor / 2;
			y3 = next_icon->fDrawY + next_icon->fHeight * next_icon->fScale / 2 + (bForceConstantSeparator && CAIRO_DOCK_IS_SEPARATOR (next_icon) ? next_icon->fHeight * (next_icon->fScale - 1) / 2 : 0);
		}
		else
		{
			x3 = x2;
			y3 = y2;
		}
		
		if (pDock->bHorizontalDock)
			cairo_rel_curve_to (pCairoContext,
				(fabs ((x - x1) / (y - y1)) > .35 ? dx * fCurvature : 0),
				(fabs ((x - x1) / (y - y1)) > .35 ? dx * fCurvature * (y - y1) / (x - x1) : 0),
				(fabs ((x3 - x2) / (y3 - y2)) > .35 ? dx * (1 - fCurvature) : dx),
				(fabs ((x3 - x2) / (y3 - y2)) > .35 ? MAX (0, MIN (dy, dy - dx * fCurvature * (y3 - y2) / (x3 - x2))) : dy),
				dx,
				dy);
		else
			cairo_rel_curve_to (pCairoContext,
				(fabs ((x - x1) / (y - y1)) > .35 ? dx * fCurvature * (y - y1) / (x - x1) : 0),
				(fabs ((x - x1) / (y - y1)) > .35 ? dx * fCurvature : 0),
				(fabs ((x3 - x2) / (y3 - y2)) > .35 ? MAX (0, MIN (dy, dy - dx * fCurvature * (y3 - y2) / (x3 - x2))) : dy),
				(fabs ((x3 - x2) / (y3 - y2)) > .35 ? dx * (1 - fCurvature) : dx),
				dy,
				dx);
		x = x2;
		y = y2;
	}
	while (ic != pFirstDrawnElement);
	
	cairo_set_line_width (pCairoContext, g_iStringLineWidth);
	cairo_set_source_rgba (pCairoContext, g_fStringColor[0], g_fStringColor[1], g_fStringColor[2], g_fStringColor[3]);
	cairo_stroke (pCairoContext);
	cairo_restore (pCairoContext);
}

void cairo_dock_render_icons_linear (cairo_t *pCairoContext, CairoDock *pDock, double fRatio)
{
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex) * pDock->fMagnitudeMax;
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE);
		cairo_restore (pCairoContext);
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
}



void cairo_dock_render_background (CairoDock *pDock)
{
	//g_print ("%s (%.2f)\n", __func__, g_fVisibleZoneAlpha);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	if (! cairo_dock_quick_hide_is_activated ())
	{
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
		cairo_move_to (pCairoContext, 0, 0);
		if (g_pVisibleZoneSurface != NULL)
		{
			cairo_set_source_surface (pCairoContext, g_pVisibleZoneSurface, 0.0, 0.0);
			cairo_paint_with_alpha (pCairoContext, g_fVisibleZoneAlpha);
		}
	}
	cairo_destroy (pCairoContext);
	
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}

void cairo_dock_render_blank (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}


/**
*Efface et redessine entierement une seule icone. Appelle la fonction de trace optimise de la vue courante; si cette derniere ne fournit pas de trace optimise, retrace tout le dock (ce qui peut etre penalisant).
*@param icon l'icone a retracer.
*@param pDock le dock contenant l' icone.
*/
void cairo_dock_redraw_my_icon (Icon *icon, CairoDock *pDock)
{
	g_return_if_fail (icon != NULL && pDock != NULL);
	if (pDock->bAtBottom && (pDock->iRefCount > 0 || g_bAutoHide))  // inutile de redessiner.
		return ;
	GdkRectangle rect = {(int) round (icon->fDrawX + MIN (0, icon->fWidth * icon->fScale * icon->fWidthFactor)),
		(int) icon->fDrawY - (pDock->bUseReflect && ! g_bDirectionUp ? g_fReflectSize : 0),
		(int) round (icon->fWidth * icon->fScale * fabs (icon->fWidthFactor)) - 1,
		(int) icon->fHeight * icon->fScale + (pDock->bUseReflect ? g_fReflectSize : 0)};
	if (! pDock->bHorizontalDock)
	{
		rect.x = (int) icon->fDrawY - (pDock->bUseReflect && ! g_bDirectionUp ? g_fReflectSize : 0),
		rect.y = (int) round (icon->fDrawX + MIN (0, icon->fWidth * icon->fScale * icon->fWidthFactor));
		rect.width = (int) icon->fHeight * icon->fScale + (pDock->bUseReflect ? g_fReflectSize : 0);
		rect.height = (int) round (icon->fWidth * icon->fScale * fabs (icon->fWidthFactor)) - 1;
	}
	//g_print ("rect (%d;%d) (%dx%d)\n", rect.x, rect.y, rect.width, rect.height);
	if (rect.width > 0 && rect.height > 0)
	{
#ifdef HAVE_GLITZ
		if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
			gtk_widget_queue_draw (pDock->pWidget);
		else
#endif
		gdk_window_invalidate_rect (pDock->pWidget->window, &rect, FALSE);
	}
}




static gboolean _cairo_dock_hide_dock (gchar *cDockName, CairoDock *pDock, CairoDock *pChildDock)
{
	if (pDock->bInside)
		return FALSE;
	
	Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	if (pPointedIcon == NULL || pPointedIcon->pSubDock != pChildDock)
		pPointedIcon = cairo_dock_get_icon_with_subdock (pDock->icons, pChildDock);
	
	if (pPointedIcon != NULL)
	{
		g_print (" il faut cacher ce dock parent\n");
		if (pDock->iRefCount == 0)  // pDock->bIsMainDock
		{
			cairo_dock_leave_from_main_dock (pDock);
		}
		else
		{
			if (pDock->iScrollOffset != 0)  // on remet systematiquement a 0 l'offset pour les containers.
			{
				pDock->iScrollOffset = 0;
				pDock->iMouseX = pDock->iCurrentWidth / 2;  // utile ?
				pDock->iMouseY = 0;
				pDock->calculate_icons (pDock);
				pDock->render (pDock);  // peut-etre qu'il faudrait faire un redraw.
			}
			
			g_print ("on cache %s par parente\n", cDockName);
			gtk_widget_hide (pDock->pWidget);
			cairo_dock_hide_parent_dock (pDock);
		}
		return TRUE;
	}
	return FALSE;
}
void cairo_dock_hide_parent_dock (CairoDock *pDock)
{
	 g_hash_table_find (g_hDocksTable, (GHRFunc)_cairo_dock_hide_dock, pDock);
}

gboolean cairo_dock_hide_child_docks (CairoDock *pDock)
{
	GList* ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->pSubDock != NULL && GTK_WIDGET_VISIBLE (icon->pSubDock->pWidget))
		{
			if (icon->pSubDock->bInside)
			{
				g_print ("on est dans le sous-dock, donc on ne le cache pas\n");
				pDock->bInside = FALSE;
				pDock->bAtTop = FALSE;
				return FALSE;
			}
			else  // si on sort du dock sans passer par le sous-dock, par exemple en sortant par le bas.
			{
				g_print ("on cache %s par filiation\n", icon->acName);
				icon->pSubDock->iScrollOffset = 0;
				icon->pSubDock->fFoldingFactor = 0;
				gtk_widget_hide (icon->pSubDock->pWidget);
			}
		}
	}
	return TRUE;
}



void cairo_dock_set_window_position_at_balance (CairoDock *pDock, int iNewWidth, int iNewHeight)
{
	pDock->iWindowPositionX = (g_iScreenWidth[pDock->bHorizontalDock] - iNewWidth) * pDock->fAlign + pDock->iGapX;
	pDock->iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - iNewHeight - pDock->iGapY : pDock->iGapY);
	//g_print ("pDock->iGapX : %d => iWindowPositionX <- %d\n", pDock->iGapX, pDock->iWindowPositionX);
	//g_print ("iNewHeight : %d -> pDock->iWindowPositionY <- %d\n", iNewHeight, pDock->iWindowPositionY);
	
	if (pDock->iWindowPositionX < 0)
		pDock->iWindowPositionX = 0;
	else if (pDock->iWindowPositionX > g_iScreenWidth[pDock->bHorizontalDock])
		pDock->iWindowPositionX = g_iScreenWidth[pDock->bHorizontalDock];
	
	if (pDock->iWindowPositionY < 0)
		pDock->iWindowPositionY = 0;
	else if (pDock->iWindowPositionY > g_iScreenHeight[pDock->bHorizontalDock])
		pDock->iWindowPositionY = g_iScreenHeight[pDock->bHorizontalDock];
}

void cairo_dock_get_window_position_and_geometry_at_balance (CairoDock *pDock, CairoDockSizeType iSizeType, int *iNewWidth, int *iNewHeight)
{
	if (iSizeType == CAIRO_DOCK_MAX_SIZE)
	{
		*iNewWidth = MIN (g_iMaxAuthorizedWidth, pDock->iMaxDockWidth);
		*iNewHeight = pDock->iMaxDockHeight;
	}
	else if (iSizeType == CAIRO_DOCK_NORMAL_SIZE)
	{
		*iNewWidth = MIN (g_iMaxAuthorizedWidth, pDock->iMinDockWidth);
		*iNewHeight = pDock->iMinDockHeight;
	}
	else
	{
		*iNewWidth = g_iVisibleZoneWidth;
		*iNewHeight = g_iVisibleZoneHeight;
	}
	
	cairo_dock_set_window_position_at_balance (pDock, *iNewWidth, *iNewHeight);
}

double cairo_dock_calculate_extra_width_for_trapeze (double fFrameHeight, double fInclination, double fRadius, double fLineWidth)
{
	if (2 * fRadius > fFrameHeight + fLineWidth)
		fRadius = (fFrameHeight + fLineWidth) / 2 - 1;
	double fDeltaXForLoop = fInclination * (fFrameHeight + fLineWidth - 2 * fRadius);
	double cosa = 1. / sqrt (1 + fInclination * fInclination);
	double sina = fInclination * cosa;
	double fDeltaCornerForLoop = fRadius * cosa + fRadius * (1 + sina) * fInclination;
	return (2 * (fLineWidth/2 + fDeltaXForLoop + fDeltaCornerForLoop + g_iFrameMargin));
}
