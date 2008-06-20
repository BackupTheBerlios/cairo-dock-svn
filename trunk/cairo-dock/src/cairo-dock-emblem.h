#ifndef __CAIRO_DOCK_EMBLEM__
#define  __CAIRO_DOCK_EMBLEM__

#include "cairo-dock-struct.h"

//Needed for emblem type
typedef enum {
	CAIRO_DOCK_EMBLEM_UPPER_RIGHT = 0,
	CAIRO_DOCK_EMBLEM_LOWER_RIGHT,
	CAIRO_DOCK_EMBLEM_MIDDLE,
	CAIRO_DOCK_EMBLEM_MIDDLE_BOTTOM,
	CAIRO_DOCK_EMBLEM_BACKGROUND,
	CAIRO_DOCK_EMBLEM_TOTAL_NB,
} CairoDockEmblem;

#define cairo_dock_make_emblem_from_file(cIconFile, pEmblemType, bPersistent) cairo_dock_draw_emblem_on_my_icon(myDrawContext, cIconFile, myIcon, myContainer, pEmblemType, bPersistent);
#define CD_APPLET_DRAW_EMBLEM_FROM_FILE(cIconFile, pEmblemType) cairo_dock_draw_emblem_on_my_icon(myDrawContext, cIconFile, myIcon, myContainer, pEmblemType, TRUE);

/**
*Dessine un embleme sur une icone.
*@param pIconContext le contexte du dessin; n'est pas altere par la fonction.
*@param cIconFile le fichier image a afficher comme embleme.
*@param pIcon l'icone.
*@param pContainer le container de l'icone.
*@param pEmblemType énumération du type d'embleme.
*/
void cairo_dock_draw_emblem_on_my_icon(cairo_t *pIconContext, const gchar *cIconFile, Icon *pIcon, CairoContainer *pContainer, CairoDockEmblem pEmblemType, gboolean bPersistent);

#define cairo_dock_make_emblem_from_surface(pSurface, pEmblemType, bPersistent) cairo_dock_draw_emblem_on_my_icon(myDrawContext, pSurface, myIcon, myContainer, pEmblemType, bPersistent);
#define CD_APPLET_DRAW_EMBLEM_FROM_SURFACE(pSurface, pEmblemType) cairo_dock_draw_emblem_on_my_icon(myDrawContext, pSurface, myIcon, myContainer, pEmblemType, TRUE);

/**
*Dessine un embleme sur une icone a partir d'une surface.
*@param pIconContext le contexte du dessin; n'est pas altere par la fonction.
*@param pSurface la surface a utiliser comme embleme.
*@param pIcon l'icone.
*@param pContainer le container de l'icone.
*@param pEmblemType énumération du type d'embleme.
*/
void cairo_dock_draw_emblem_from_surface (cairo_t *pIconContext, cairo_surface_t *pSurface, Icon *pIcon, CairoContainer *pContainer, CairoDockEmblem pEmblemType, gboolean bPersistent);

//Needed for emblem classic type
typedef enum {
	CAIRO_DOCK_EMBLEM_BLANK = 0,
	CAIRO_DOCK_EMBLEM_CHARGE,
	CAIRO_DOCK_EMBLEM_DROP_INDICATOR,
	CAIRO_DOCK_EMBLEM_PLAY,
	CAIRO_DOCK_EMBLEM_PAUSE,
	CAIRO_DOCK_EMBLEM_STOP,
	CAIRO_DOCK_EMBLEM_BROKEN,
	CAIRO_DOCK_EMBLEM_CLASSIC_NB,
} CairoDockClassicEmblem;

#define cairo_dock_make_emblem(pEmblemClassic, pEmblemType, bPersistent) cairo_dock_draw_emblem_classic(myDrawContext, myIcon, myContainer, pEmblemClassic, pEmblemType, bPersistent);
#define CD_APPLET_DRAW_EMBLEM(pEmblemClassic, pEmblemType) cairo_dock_draw_emblem_classic(myDrawContext, myIcon, myContainer, pEmblemClassic, pEmblemType, TRUE);

/**
*Dessine un embleme sur une icone avec une banque local.
*@param pIconContext le contexte du dessin; n'est pas altere par la fonction.
*@param pIcon l'icone.
*@param pContainer le container de l'icone.
*@param pEmblemClassic énumération du type d'embleme classique.
*@param pEmblemType énumération du type d'embleme.
*/
void cairo_dock_draw_emblem_classic (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer, CairoDockClassicEmblem pEmblemClassic, CairoDockEmblem pEmblemType, gboolean bPersistent);

/**
*Récupère l'emplacement des emblèmes utilisateur dans le fichier de configuration.
*/
void cairo_dock_get_emblem_path (void);

/**
*Libère les surfaces et les emplacements dans la mémoire.
*/
void cairo_dock_free_emblem (void);

/**
*Met a jour l'emplacement des emblèmes utilisateurs.
*/
void cairo_dock_updated_emblem_conf_file (void);

#endif
