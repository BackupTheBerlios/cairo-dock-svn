/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Cchumi & Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-draw.h"

gboolean my_bRotateIconsOnEllipse = TRUE;


static void _cd_switcher_draw_windows_on_viewport (Icon *pIcon, gint *data)
{
	if (pIcon == NULL || pIcon->fPersonnalScale > 0)
		return ;
	if (pIcon->bIsHidden && ! myConfig.bDisplayHiddenWindows)
		return ;
	int iNumDesktop = data[0];
	int iNumViewportX = data[1];
	int iNumViewportY = data[2];
	int iOneViewportWidth = data[3];
	int iOneViewportHeight = data[4];
	cairo_t *pCairoContext = data[5];
	
	// On calcule les coordonnees en repere absolu.
	int x = pIcon->windowGeometry.x;  // par rapport au viewport courant.
	x += myData.switcher.iCurrentViewportX * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];  // repere absolu
	if (x < 0)
		x += g_iNbViewportX * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
	int y = pIcon->windowGeometry.y;
	y += myData.switcher.iCurrentViewportY * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
	if (y < 0)
		y += g_iNbViewportY * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
	int w = pIcon->windowGeometry.width, h = pIcon->windowGeometry.height;
	
	// test d'intersection avec le viewport donne.
	//g_print (" %s : (%d;%d) %dx%d\n", pIcon->acName, x, y, w, h);
	if ((pIcon->iNumDesktop != -1 && pIcon->iNumDesktop != iNumDesktop) ||
		x + w <= iNumViewportX * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] ||
		x >= (iNumViewportX + 1) * g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] ||
		y + h <= iNumViewportY * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] ||
		y >= (iNumViewportY + 1) * g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL])
		return ;
	//g_print (" > on la dessine (%x)\n", pIcon->pIconBuffer);
	
	// on dessine ses traits.
	cairo_save (pCairoContext);
	
	cairo_set_source_rgba (pCairoContext, myConfig.RGBWLineColors[0], myConfig.RGBWLineColors[1], myConfig.RGBWLineColors[2], myConfig.RGBWLineColors[3]);
	cairo_rectangle (pCairoContext,
		(1.*x/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] - iNumViewportX)*iOneViewportWidth,
		(1.*y/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] - iNumViewportY)*iOneViewportHeight,
		1.*w/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]*iOneViewportWidth,
		1.*h/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]*iOneViewportHeight);
	if (pIcon->Xid == cairo_dock_get_current_active_window ())
	{
		//g_print (" %s est la fenetre active\n", pIcon->acName);
		cairo_fill (pCairoContext);
	}
	else
	{
		cairo_stroke (pCairoContext);
	}
	
	if (pIcon->pIconBuffer != NULL)
	{
		CairoDock *pParentDock = NULL;
		pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
		if (pParentDock == NULL)
			pParentDock = g_pMainDock;
		double fMaxScale = cairo_dock_get_max_scale (pParentDock);
		double fRatio = pParentDock->fRatio;
		
		cairo_translate (pCairoContext,
			(1.*x/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL] - iNumViewportX)*iOneViewportWidth,
			(1.*y/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL] - iNumViewportY)*iOneViewportHeight);
		cairo_scale (pCairoContext,
			1.*w/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL]*iOneViewportWidth / (pIcon->fWidth/fRatio*fMaxScale),
			1.*h/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL]*iOneViewportHeight / (pIcon->fHeight/fRatio*fMaxScale));
		cairo_set_source_surface (pCairoContext,
			pIcon->pIconBuffer,
			0., 0.);
		cairo_paint (pCairoContext);
	}
	
	cairo_restore (pCairoContext);
}


static int _compare_icons_stack_order (Icon *icon1, Icon *icon2)
{
	if (icon1 == NULL)  // les icones nulles vont a la fin.
		return 1;
	if (icon2 == NULL)
		return -1;
	if (icon1->iStackOrder < icon2->iStackOrder)  // ordre petit => dessus => dessinee en dernier.
		return -1;
	else
		return 1;
}
void cd_switcher_draw_main_icon_compact_mode (void)
{
	cd_debug ("%s (%d;%d)", __func__, myData.switcher.iCurrentLine, myData.switcher.iCurrentColumn);
	// On efface l'icone.
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	double fRatio = (myDock ? myDock->fRatio : 1.);
	
	// definition des parametres de dessin.
	double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	myData.switcher.fOneViewportHeight = (myIcon->fHeight/fRatio * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (myIcon->fWidth/fRatio * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	
	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.bMapWallpaper)
	{
		pSurface = cairo_dock_get_desktop_bg_surface ();
		fZoomX = (double) myData.switcher.fOneViewportWidth / g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
		fZoomY= (double) myData.switcher.fOneViewportHeight / g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
	}
	if (pSurface == NULL)
	{
		pSurface = myData.pDefaultMapSurface;
		fZoomX = (double) myData.switcher.fOneViewportWidth / (myIcon->fWidth/fRatio * fMaxScale);
		fZoomY = (double) myData.switcher.fOneViewportHeight / (myIcon->fHeight/fRatio * fMaxScale);
	}
	
	// cadre exterieur.
	cairo_set_line_width (myDrawContext,myConfig.iLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBLineColors[0],myConfig.RGBLineColors[1],myConfig.RGBLineColors[2],myConfig.RGBLineColors[3]);
	cairo_rectangle(myDrawContext,
		.5*myConfig.iLineSize,
		.5*myConfig.iLineSize,
		myIcon->fWidth/fRatio * fMaxScale - myConfig.iLineSize,
		myIcon->fHeight/fRatio * fMaxScale - myConfig.iLineSize);

	cairo_stroke (myDrawContext);
	
	// lignes interieures.
	cairo_set_line_width (myDrawContext,myConfig.iInLineSize);
	cairo_set_source_rgba(myDrawContext,myConfig.RGBInLineColors[0],myConfig.RGBInLineColors[1],myConfig.RGBInLineColors[2],myConfig.RGBInLineColors[3]);
	double xi, yj;
	int i, j;
	for (i = 1; i <myData.switcher.iNbColumns; i ++)  // lignes verticales.
	{
		xi = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		cairo_move_to (myDrawContext, xi, myConfig.iLineSize);
		cairo_rel_line_to (myDrawContext, 0, myIcon->fHeight/fRatio * fMaxScale - 2*myConfig.iLineSize);
		cairo_stroke (myDrawContext);
	}
	for (j = 1; j < myData.switcher.iNbLines; j ++)  // lignes horizontales.
	{
		yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize) - .5*myConfig.iInLineSize;
		cairo_move_to (myDrawContext, myConfig.iLineSize, yj);
		cairo_rel_line_to (myDrawContext, myIcon->fWidth/fRatio * fMaxScale - 2*myConfig.iLineSize, 0);
		cairo_stroke (myDrawContext);
	}
	
	GList *pWindowList = NULL;
	if (myConfig.bDrawWindows)
	{
		pWindowList = cairo_dock_get_current_applis_list ();
		pWindowList = g_list_sort (pWindowList, (GCompareFunc) _compare_icons_stack_order);
	}
	
	// chaque bureau/viewport.
	int iNumDesktop=0, iNumViewportX=0, iNumViewportY=0;
	for (j = 0; j < myData.switcher.iNbLines; j ++)
	{
		for (i = 0; i < myData.switcher.iNbColumns; i ++)
		{
			cairo_save (myDrawContext);

			xi = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize);
			yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize);

			cairo_translate (myDrawContext,
				xi,
				yj);

			cairo_scale (myDrawContext,
				fZoomX,
				fZoomY);
			cairo_set_source_surface (myDrawContext,
				pSurface,
				0.,
				0.);
			cairo_paint(myDrawContext);
			
			cairo_restore (myDrawContext);
			
			if (myConfig.iDrawCurrentDesktopMode == SWICTHER_FILL_INVERTED && (i != myData.switcher.iCurrentColumn || j != myData.switcher.iCurrentLine))
			{
				cairo_save (myDrawContext);
				
				cairo_set_source_rgba (myDrawContext, myConfig.RGBIndColors[0], myConfig.RGBIndColors[1], myConfig.RGBIndColors[2], myConfig.RGBIndColors[3]);
				cairo_rectangle(myDrawContext,
					xi - .5*myConfig.iLineSize,
					yj - .5*myConfig.iLineSize,
					myData.switcher.fOneViewportWidth + myConfig.iLineSize,
					myData.switcher.fOneViewportHeight + myConfig.iLineSize);
				cairo_fill (myDrawContext);
				
				cairo_restore (myDrawContext);
			}
			
			if (myConfig.bDrawWindows)
			{
				cairo_save (myDrawContext);
				
				cairo_translate (myDrawContext,
					xi,
					yj);
				cairo_set_line_width (myDrawContext, 1.);
				cairo_rectangle (myDrawContext,
					0.,
					0.,
					myData.switcher.fOneViewportWidth,
					myData.switcher.fOneViewportHeight);
				cairo_clip (myDrawContext);
				
				//g_print (" dessin des fenetres du bureau (%d;%d;%d) ...\n", iNumDesktop, iNumViewportX, iNumViewportY);
				gint data[6] = {iNumDesktop, iNumViewportX, iNumViewportY, (int) myData.switcher.fOneViewportWidth, (int) myData.switcher.fOneViewportHeight, myDrawContext};
				g_list_foreach (pWindowList, _cd_switcher_draw_windows_on_viewport, data);
				
				cairo_restore (myDrawContext);
			}
			
			iNumViewportX ++;
			if (iNumViewportX == g_iNbViewportX)
			{
				iNumViewportY ++;
				if (iNumViewportY == g_iNbViewportY)
					iNumDesktop ++;
			}
		}
	}
	
	// dessin de l'indicateur sur le bureau courant (on le fait maintenant car dans le cas ou la ligne interieure est plus petite que la ligne de l'indicateur, les surfaces suivantes recouvreraient en partie la ligne.
	if (myConfig.iDrawCurrentDesktopMode != SWICTHER_FILL_INVERTED)
	{
		i = myData.switcher.iCurrentColumn;
		j = myData.switcher.iCurrentLine;
		xi = myConfig.iLineSize + i * (myData.switcher.fOneViewportWidth + myConfig.iInLineSize);
		yj = myConfig.iLineSize + j * (myData.switcher.fOneViewportHeight + myConfig.iInLineSize);
		
		cairo_set_line_width (myDrawContext,myConfig.iLineSize);
		cairo_set_source_rgba (myDrawContext,myConfig.RGBIndColors[0],myConfig.RGBIndColors[1],myConfig.RGBIndColors[2],myConfig.RGBIndColors[3]);
		cairo_rectangle(myDrawContext,
			xi - .5*myConfig.iLineSize,
			yj - .5*myConfig.iLineSize,
			myData.switcher.fOneViewportWidth + myConfig.iLineSize,
			myData.switcher.fOneViewportHeight + myConfig.iLineSize);
		
		if (myConfig.iDrawCurrentDesktopMode == SWICTHER_FILL)
			cairo_fill (myDrawContext);
		else
			cairo_stroke(myDrawContext);
	}
	
	g_list_free (pWindowList);  // le contenu appartient a la hash table, mais pas la liste.
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		cairo_dock_update_icon_texture (myIcon);
}


void cd_switcher_draw_main_icon_expanded_mode (void)
{
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba(myDrawContext, 0., 0., 0., 0.);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	// definition des parametres de dessin.
	double fRatio = (myDock ? myDock->fRatio : 1.);
	double fMaxScale = cairo_dock_get_max_scale (myContainer); //coefficient Max icone Width
	myData.switcher.fOneViewportHeight = (myIcon->fHeight/fRatio * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbLines - 1) * myConfig.iInLineSize) / myData.switcher.iNbLines; //hauteur d'un bureau/viewport sans compter les lignes exterieures et interieures.
	myData.switcher.fOneViewportWidth = (myIcon->fWidth/fRatio * fMaxScale - 2 * myConfig.iLineSize - (myData.switcher.iNbColumns - 1) * myConfig.iInLineSize) / myData.switcher.iNbColumns; //largeur d'un bureau/viewport sans compter les lignes exterieures et interieures.

	cairo_surface_t *pSurface = NULL;
	double fZoomX, fZoomY;
	if (myConfig.bMapWallpaper)
	{
		pSurface = cairo_dock_get_desktop_bg_surface ();
		fZoomX = (double) myIcon->fHeight/fRatio * fMaxScale/g_iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
		fZoomY= (double) myIcon->fHeight/fRatio * fMaxScale/g_iXScreenHeight[CAIRO_DOCK_HORIZONTAL];	
		cairo_translate (myDrawContext,
			0.,
			0.);

		cairo_save (myDrawContext);
		cairo_scale (myDrawContext,
			fZoomX ,
			fZoomY );
		cairo_set_source_surface (myDrawContext,
			pSurface,
			0.,
			0.);
		cairo_paint(myDrawContext);
		cairo_restore (myDrawContext);
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
			cairo_dock_update_icon_texture (myIcon);
	}
	else if (myIcon->acFileName == NULL)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
	}

	/*if (pSurface == NULL)
	{
		pSurface = myData.pDefaultMapSurface;
		//fZoomX = (double) myData.switcher.fOneViewportWidth / (myIcon->fWidth * fMaxScale);
		//fZoomY = (double) myData.switcher.fOneViewportHeight / (myIcon->fHeight * fMaxScale);
	}*/
		
	if (myConfig.bDrawWindows)
	{
		/*double XWgeo= (myIcon->fWidth/fRatio* fMaxScale/myData.switcher.fOneViewportWidth)* fMaxScale;
		double YWgeo = (myIcon->fHeight/fRatio* fMaxScale/myData.switcher.fOneViewportHeight)* fMaxScale;
		cd_debug ("XWgeo : %f",XWgeo);
		cd_debug ("YWgeo : %f",YWgeo);
		fZoomX = myIcon->fWidth/fRatio * fMaxScale;
		fZoomY = myIcon->fHeight/fRatio * fMaxScale;
		cairo_save(myDrawContext);
		cd_switcher_draw_windows_on_each_viewports(XWgeo,YWgeo,fZoomX,fZoomY);
		cairo_restore (myDrawContext);*/
		GList *pWindowList = cairo_dock_get_current_applis_list ();
		pWindowList = g_list_sort (pWindowList, (GCompareFunc) _compare_icons_stack_order);
		
		fMaxScale = (myIcon->pSubDock != NULL ? cairo_dock_get_max_scale (myIcon->pSubDock) : 1);
		fRatio = (myIcon->pSubDock != NULL ? myIcon->pSubDock->fRatio : 1);
		gint data[6];
		int iNumDesktop=0, iNumViewportX=0, iNumViewportY=0;
		cairo_t *pCairoContext;
		Icon *pIcon;
		GList *pIconsList = (myDock ? myDock->icons : myDesklet->icons);
		GList *ic;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			
			data[0] = iNumDesktop;
			data[1] = iNumViewportX;
			data[2] = iNumViewportY;
			data[3] = (int) pIcon->fWidth/fRatio*fMaxScale;
			data[4] = (int) pIcon->fHeight/fRatio*fMaxScale;
			pCairoContext = cairo_create (pIcon->pIconBuffer);
			data[5] = GPOINTER_TO_INT (pCairoContext);
			cairo_set_line_width (pCairoContext, 1.);
			cairo_set_source_rgba (pCairoContext, myConfig.RGBWLineColors[0], myConfig.RGBWLineColors[1], myConfig.RGBWLineColors[2], myConfig.RGBWLineColors[3]);
		
			g_list_foreach (pWindowList, _cd_switcher_draw_windows_on_viewport, data);
			
			iNumViewportX ++;
			if (iNumViewportX == g_iNbViewportX)
			{
				iNumViewportY ++;
				if (iNumViewportY == g_iNbViewportY)
					iNumDesktop ++;
			}
			cairo_destroy (pCairoContext);
		}
		g_list_free (pWindowList);  // le contenu appartient a la hash table, mais pas la liste.
	}
}

/*Fonction de base pour toutes les autres*/
void cd_switcher_draw_main_icon (void)
{
	cd_message ("%s (%d)", __func__, myConfig.bCompactView);
	if (myConfig.bCompactView)
	{
		cd_switcher_draw_main_icon_compact_mode ();
	}
	else
	{
		cd_switcher_draw_main_icon_expanded_mode ();
	}
	
	if ((myDesklet && ! myConfig.bCompactView) || (myDock && myDock->bUseReflect))
		cairo_dock_add_reflection_to_icon (myDrawContext, myIcon, myContainer);
	CD_APPLET_REDRAW_MY_ICON;
}
