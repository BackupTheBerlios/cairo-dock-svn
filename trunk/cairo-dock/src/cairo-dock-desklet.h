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

#ifndef __CAIRO_DOCK_DESKLET_H__
#define  __CAIRO_DOCK_DESKLET_H__

#include <cairo-dock-struct.h>

/**
*Teste si le container est un desklet.
*@param pContainer le container.
*@return TRUE ssi le container a ete declare comme un desklet.
*/
#define CAIRO_DOCK_IS_DESKLET(pContainer) (pContainer != NULL && pContainer->iType == CAIRO_DOCK_TYPE_DESKLET)
/**
*Caste un container en desklet.
*@param pContainer le container.
*@return le desklet.
*/
#define CAIRO_DOCK_DESKLET(pContainer) ((CairoDockDesklet *)pContainer)

CairoDockDesklet *cairo_dock_create_desklet (Icon *pIcon, GtkWidget *pInteractiveWidget);

void cairo_dock_place_desklet (CairoDockDesklet *pDesklet, int iWidth, int iHeight, int iPositionX, int iPositionY, gboolean bKeepBelow, gboolean bKeepAbove, gboolean bOnWidgetLayer);


void cairo_dock_free_desklet (CairoDockDesklet *pDesklet);


void cairo_dock_hide_desklet (CairoDockDesklet *pDesklet);
void cairo_dock_show_desklet (CairoDockDesklet *pDesklet);

void cairo_dock_add_interactive_widget_to_desklet (GtkWidget *pInteractiveWidget, CairoDockDesklet *pDesklet);

#endif
