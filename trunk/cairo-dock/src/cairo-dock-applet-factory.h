/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */

#ifndef __CAIRO_DOCK_APPLET_FACTORY__
#define  __CAIRO_DOCK_APPLET_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"
G_BEGIN_DECLS


/* Cree une surface cairo qui pourra servir de zone de dessin pour une applet.
*@param cIconFileName le nom d'un fichier image a appliquer sur la surface, ou NULL pour creer une surface vide.
*@param pSourceContext un contexte de dessin; n'est pas altere.
*@param fMaxScale le zoom max auquel sera soumis la surface.
*@param fWidth largeur de la surface obtenue.
*@param fHeight hauteur de la surface obtenue.
*@return la surface nouvellement generee.
*/
cairo_surface_t *cairo_dock_create_applet_surface (const gchar *cIconFileName, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);


/* Cree une icone destinee a une applet.
*@param pContainer le container ou sera inseree ulterieurement cette icone.
*@param iWidth la largeur desiree de l'icone.
*@param iHeight la hauteur desiree de l'icone.
*@param cName le nom de l'icone, tel qu'il apparaitra en etiquette de l'icone.
*@param cIconFileName le nom d'un fichier image a afficher dans l'icone, ou NULL si l'on souhaitera dessiner soi-meme dans l'icone.
*@param pModuleInstance l'instance du module (necessaire pour que l'icone soit consideree comme une applet lors de son remplissage).
*@return l'icone nouvellement cree. Elle n'est _pas_ inseree dans le dock, c'est le gestionnaire de module qui se charge d'inserer les icones renvoyees par les modules.
*/
Icon *cairo_dock_create_icon_for_applet (CairoContainer *pContainer, int iWidth, int iHeight, const gchar *cName, const gchar *cIconFileName, CairoDockModuleInstance *pModuleInstance);


G_END_DECLS
#endif
