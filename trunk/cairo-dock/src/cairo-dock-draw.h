
#ifndef __CAIRO_DOCK_DRAW__
#define  __CAIRO_DOCK_DRAW__

#include <glib.h>

#include "cairo-dock-struct.h"


/**
*Applique la colormap de l'ecran a une fenetre GTK. Permet de la rendre transparente.
*@param pWidget
*/
void cairo_dock_set_colormap_for_window (GtkWidget *pWidget);


double cairo_dock_get_current_dock_width_linear (CairoDock *pDock);

/**
*Cree un contexte de dessin pour la libcairo. Si glitz est active, le contexte sera lie a une surface glitz (et donc on dessinera directement sur la carte graphique), sinon a une surface X representant la fenetre du dock.
*@param pDock le dock sur lequel on veut dessiner.
*@return le contexte sur lequel dessiner. N'est jamais nul; tester sa coherence avec cairo_status() avant de l'utiliser, et le detruire avec cairo_destroy() apres en avoir fini avec lui.
*/
cairo_t * cairo_dock_create_context_from_window (CairoDock *pDock);

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
void cairo_dock_draw_frame (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination, gboolean bHorizontal);

/**
*Dessine les decorations d'un dock a l'interieur d'un cadre prealablement trace sur le contexte.
*@param pCairoContext le contexte du dessin, est laisse intact par la fonction.
*@param pDock le dock sur lequel appliquer les decorations.
*@param fOffsetY un decalage, dans le sens de la hauteur du dock, a partir duquel appliquer les decorations.
*/
void cairo_dock_render_decorations_in_frame (cairo_t *pCairoContext, CairoDock *pDock, double fOffsetY);


/**
*Dessine entierement une icone, dont toutes les caracteristiques ont ete prealablement calculees. Gere sa position, sa transparence (modulee par la transparence du dock au repos), son reflet, son placement de profil, son etiquette, et son info-rapide.
*@param icon l'icone a dessiner.
*@param pDock le dock auquel elle appartient.
*/
void cairo_dock_manage_animations (Icon *icon, CairoDock *pDock);

/**
*Dessine entierement une icone, dont toutes les caracteristiques ont ete prealablement calculees. Gere sa position, sa transparence (modulee par la transparence du dock au repos), son reflet, son placement de profil, son etiquette, et son info-rapide.
*@param icon l'icone a dessiner.
*@param pCairoContext le contexte du dessin, est altere pendant le dessin.
*@param bHorizontalDock l'horizontalite du dock contenant l'icone.
*@param fRatio le ratio de taille des icones dans ce dock.
*@param fDockMagnitude la magnitude actuelle du dock.
*@param bUseReflect TRUE pour dessiner les reflets.
*/
void cairo_dock_render_one_icon (Icon *icon, cairo_t *pCairoContext, gboolean bHorizontalDock, double fRatio, double fDockMagnitude, gboolean bUseReflect);
void cairo_dock_render_icons_linear (cairo_t *pCairoContext, CairoDock *pDock, double fRatio);

/**
*Dessine une ficelle reliant le centre de toutes les icones, en commencant par la 1ere dessinee.
*@param pCairoContext le contexte du dessin, n'est pas altere par la fonction.
*@param pDock le dock contenant les icônes a relier.
*@param fStringLineWidth epaisseur de la ligne.
*@param bIsLoop TRUE si on veut boucler (relier la derniere icone a la 1ere).
*@param bForceConstantSeparator TRUE pour forcer les separateurs a etre consideres comme de taille constante.
*/
void cairo_dock_draw_string (cairo_t *pCairoContext, CairoDock *pDock, double fStringLineWidth, gboolean bIsLoop, gboolean bForceConstantSeparator);


void cairo_dock_render_background (CairoDock *pDock);

void cairo_dock_render_blank (CairoDock *pDock);


/**
*Efface et redessine entierement une seule icone. Appelle la fonction de trace optimise de la vue courante; si cette derniere ne fournit pas de trace optimise, retrace tout le dock (ce qui peut etre penalisant).
*@param icon l'icone a retracer.
*@param pDock le dock contenant l' icone.
*/
void cairo_dock_redraw_my_icon (Icon *icon, CairoDock *pDock);


/**
*Cache le dock pere d'un dock.
*@param pDock le dock fils.
*/
void cairo_dock_hide_parent_dock (CairoDock *pDock);
/**
*Cache recursivement tous les sous-docks fils d'un dock donne.
*@param pDock le dock parent.
*/
gboolean cairo_dock_hide_child_docks (CairoDock *pDock);


void cairo_dock_set_window_position_at_balance (CairoDock *pDock, int iNewWidth, int iNewHeight);
void cairo_dock_get_window_position_and_geometry_at_balance (CairoDock *pDock, CairoDockSizeType iSizeType, int *iNewWidth, int *iNewHeight);

double cairo_dock_calculate_extra_width_for_trapeze (double fFrameHeight, double fInclination, double fRadius, double fLineWidth);


#endif
