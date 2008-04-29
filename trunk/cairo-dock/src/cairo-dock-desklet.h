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

#ifndef __CAIRO_DESKLET_H__
#define  __CAIRO_DESKLET_H__

#include <cairo-dock-struct.h>

#define CD_NB_ITER_FOR_GRADUATION 10

/**
*Teste si le container est un desklet.
*@param pContainer le container.
*@return TRUE ssi le container a ete declare comme un desklet.
*/
#define CAIRO_DOCK_IS_DESKLET(pContainer) (pContainer != NULL && (pContainer)->iType == CAIRO_DOCK_TYPE_DESKLET)
/**
*Caste un container en desklet.
*@param pContainer le container.
*@return le desklet.
*/
#define CAIRO_DESKLET(pContainer) ((CairoDesklet *)pContainer)

CairoDesklet *cairo_dock_create_desklet (Icon *pIcon, GtkWidget *pInteractiveWidget, gboolean bOnWidgetLayer);

Icon *cairo_dock_find_clicked_icon_in_desklet (CairoDesklet *pDesklet);

void cairo_dock_place_desklet (CairoDesklet *pDesklet, CairoDockMinimalAppletConfig *pMinimalConfig);

void cairo_dock_steal_interactive_widget_from_desklet (CairoDesklet *pDesklet);
void cairo_dock_free_desklet (CairoDesklet *pDesklet);


void cairo_dock_hide_desklet (CairoDesklet *pDesklet);
void cairo_dock_show_desklet (CairoDesklet *pDesklet);

void cairo_dock_add_interactive_widget_to_desklet (GtkWidget *pInteractiveWidget, CairoDesklet *pDesklet);


void cairo_dock_set_all_desklets_visible (gboolean bOnWidgetLayerToo);
void cairo_dock_set_desklets_visibility_to_default (void);

CairoDesklet *cairo_dock_get_desklet_by_Xid (Window Xid);


#endif
