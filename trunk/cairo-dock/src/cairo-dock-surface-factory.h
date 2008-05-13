
#ifndef __CAIRO_DOCK_SURFACE_FACTORY__
#define  __CAIRO_DOCK_SURFACE_FACTORY__

#include <glib.h>
#include <gdk/gdk.h>
#include <cairo.h>


/**
* Calcule la taille d'une image selon une contrainte en largeur et hauteur de manière à remplir l'espace donné.
*@param fImageWidth la largeur de l'image. Contient initialement la largeur de l'image, et sera écrasée avec la largeur obtenue.
*@param fImageHeight la hauteur de l'image. Contient initialement la hauteur de l'image, et sera écrasée avec la hauteur obtenue.
*@param iWidthConstraint contrainte en largeur (0 <=> pas de contrainte).
*@param iHeightConstraint contrainte en hauteur (0 <=> pas de contrainte).
*@param bNoZoomUp TRUE ssi on ne doit pas agrandir l'image (seulement la rétrécir).
*@param fZoomWidth sera renseigné avec le facteur de zoom en largeur qui a été appliqué.
*@param fZoomHeight sera renseigné avec le facteur de zoom en hauteur qui a été appliqué.
*/
void cairo_dock_calculate_size_fill (double *fImageWidth, double *fImageHeight, int iWidthConstraint, int iHeightConstraint, gboolean bNoZoomUp, double *fZoomWidth, double *fZoomHeight);
/**
* Calcule la taille d'une image selon une contrainte en largeur et hauteur en gardant le ratio hauteur/largeur constant.
*@param fImageWidth la largeur de l'image. Contient initialement la largeur de l'image, et sera écrasée avec la largeur obtenue.
*@param fImageHeight la hauteur de l'image. Contient initialement la hauteur de l'image, et sera écrasée avec la hauteur obtenue.
*@param iWidthConstraint contrainte en largeur (0 <=> pas de contrainte).
*@param iHeightConstraint contrainte en hauteur (0 <=> pas de contrainte).
*@param bNoZoomUp TRUE ssi on ne doit pas agrandir l'image (seulement la rétrécir).
*@param fZoom sera renseigné avec le facteur de zoom qui a été appliqué.
*/
void cairo_dock_calculate_size_constant_ratio (double *fImageWidth, double *fImageHeight, int iWidthConstraint, int iHeightConstraint, gboolean bNoZoomUp, double *fZoom);
/**
* Calcule la taille d'une image selon une contrainte en largeur et hauteur.
*@param fImageWidth la largeur de l'image. Contient initialement la largeur de l'image, et sera écrasée avec la largeur obtenue.
*@param fImageHeight la hauteur de l'image. Contient initialement la hauteur de l'image, et sera écrasée avec la hauteur obtenue.
*@param iWidthConstraint contrainte en largeur (0 <=> pas de contrainte).
*@param iHeightConstraint contrainte en hauteur (0 <=> pas de contrainte).
*@param iLoadingModifier composition de modificateurs de chargement.
*@param fZoomWidth sera renseigné avec le facteur de zoom en largeur qui a été appliqué.
*@param fZoomHeight sera renseigné avec le facteur de zoom en hauteur qui a été appliqué.
*/
void cairo_dock_calculate_constrainted_size (double *fImageWidth, double *fImageHeight, int iWidthConstraint, int iHeightConstraint, CairoDockLoadImageModifier iLoadingModifier, double *fZoomWidth, double *fZoomHeight);

/**
* Cree une surface à partir des données brutes d'une icone X. L'icone de plus grande taille contenue dans le buffer est prise.
*@param pXIconBuffer le tableau de données.
*@param iBufferNbElements le nombre d'éléments du tableau.
*@param pSourceContext un contexte (non modifié par la fonction).
*@param fMaxScale le zoom maximal de l'icone.
*@param fWidth sera renseigné avec la largeur de l'icone.
*@param fHeight sera renseigné avec la hauteur de l'icone.
*@returns la surface nouvellement créée.
*/
cairo_surface_t *cairo_dock_create_surface_from_xicon_buffer (gulong *pXIconBuffer, int iBufferNbElements, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);

/**
* Cree une surface à partir d'un GdkPixbuf.
*@param pixbuf le pixbuf.
*@param pSourceContext un contexte (non modifié par la fonction).
*@param fMaxScale le zoom maximal de l'icone.
*@param iWidthConstraint contrainte sur la largeur, ou 0 pour ne pas la contraindre.
*@param iHeightConstraint contrainte sur la hauteur, ou 0 pour ne pas la contraindre.
*@param bKeepRatio TRUE ssi on veut conserver le ratio largeur/hauteur du pixbuf.
*@param fImageWidth sera renseigné avec la largeur de l'icone (hors zoom).
*@param fImageHeight sera renseigné avec la hauteur de l'icone (hors zoom).
*@returns la surface nouvellement créée.
*/
cairo_surface_t *cairo_dock_create_surface_from_pixbuf (GdkPixbuf *pixbuf, cairo_t *pSourceContext, double fMaxScale, int iWidthConstraint, int iHeightConstraint, CairoDockLoadImageModifier iLoadingModifier, double *fImageWidth, double *fImageHeight);

/**
* Cree une surface à partir d'une image au format quelconque.
*@param cImagePath le chemin complet de l'image.
*@param pSourceContext un contexte (non modifié par la fonction).
*@param fMaxScale le zoom maximal de l'icone.
*@param iWidthConstraint contrainte sur la largeur, ou 0 pour ne pas la contraindre.
*@param iHeightConstraint contrainte sur la hauteur, ou 0 pour ne pas la contraindre.
*@param fImageWidth sera renseigné avec la largeur de l'icone (hors zoom).
*@param fImageHeight sera renseigné avec la hauteur de l'icone (hors zoom).
*@param bKeepRatio TRUE ssi on veut conserver le ratio largeur/hauteur du pixbuf.
*@returns la surface nouvellement créée.
*/
cairo_surface_t *cairo_dock_create_surface_from_image (gchar *cImagePath, cairo_t* pSourceContext, double fMaxScale, int iWidthConstraint, int iHeightConstraint, double *fImageWidth, double *fImageHeight, CairoDockLoadImageModifier iLoadingModifier);
/**
* Cree une surface à partir d'une image au format quelconque, a la taille donnée, sans zoom.
*@param cImagePath le chemin complet de l'image.
*@param pSourceContext un contexte (non modifié par la fonction).
*@param fImageWidth la largeur de l'icone.
*@param fImageHeight la hauteur de l'icone.
*@returns la surface nouvellement créée.
*/
cairo_surface_t *cairo_dock_create_surface_for_icon (gchar *cImagePath, cairo_t* pSourceContext, double fImageWidth, double fImageHeight);
/**
* Cree une surface à partir d'une image au format quelconque, a la taille donnée carrée, sans zoom.
*@param cImagePath le chemin complet de l'image.
*@param pSourceContext un contexte (non modifié par la fonction).
*@param fImageSize la taille de l'icone.
*@returns la surface nouvellement créée.
*/
#define cairo_dock_create_surface_for_square_icon(cImagePath, pSourceContext, fImageSize) cairo_dock_create_surface_for_icon (cImagePath, pSourceContext, fImageSize, fImageSize)



cairo_surface_t * cairo_dock_rotate_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, double fRotationAngle);

cairo_surface_t * cairo_dock_create_reflection_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, gboolean bHorizontalDock, double fMaxScale, gboolean bDirectionUp);

cairo_surface_t * cairo_dock_create_icon_surface_with_reflection (cairo_surface_t *pIconSurface, cairo_surface_t *pReflectionSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, gboolean bHorizontalDock,double fMaxScale, gboolean bDirectionUp);

cairo_surface_t *cairo_dock_create_surface_from_text (gchar *cText, cairo_t *pSourceContext, int iLabelSize, gchar *cLabelPolice, int iLabelWeight, double *fBackgroundColor, double fMaxScale, int *iTextWidth, int *iTextHeight, double *fTextXOffset, double *fTextYOffset);

cairo_surface_t * cairo_dock_duplicate_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fWidth, double fHeight, double fDesiredWidth, double fDesiredHeight);


#endif
