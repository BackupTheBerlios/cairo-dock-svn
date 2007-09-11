/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>
#include <pango/pango.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-draw.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-config.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-icons.h"


extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern gboolean g_bReserveSpace;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern int g_iIconGap;

extern gchar *g_cCurrentThemePath;

extern double g_fAmplitude;
extern int g_iSinusoidWidth;
extern double g_fUnfoldAcceleration;
extern gboolean g_bAutoHide;

extern gboolean g_bDirectionUp;
extern GHashTable *g_hAppliTable;
extern gboolean g_bUniquePid;
extern GHashTable *g_hXWindowTable;
extern int g_iSidUpdateAppliList;

extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];
extern gchar *g_cConfFile;
extern GHashTable *g_hModuleTable;


void cairo_dock_free_icon (Icon *icon)
{
	if (icon == NULL)
		return ;
	g_free (icon->acDesktopFileName);
	g_free (icon->acFileName);
	g_free (icon->acName);
	g_free (icon->acCommand);
	g_free (icon->cBaseURI);
	
	cairo_surface_destroy (icon->pIconBuffer);
	cairo_surface_destroy (icon->pTextBuffer);
	
	if (CAIRO_DOCK_IS_APPLI (icon) && g_bUniquePid)
		g_hash_table_remove (g_hAppliTable, &icon->iPid);
	if (CAIRO_DOCK_IS_APPLET (icon) && icon->pModule != NULL)
		cairo_dock_free_module (icon->pModule);
	
	g_free (icon);
}

int cairo_dock_compare_icons_order (Icon *icon1, Icon *icon2)
{
	if (g_tIconTypeOrder[icon1->iType] < g_tIconTypeOrder[icon2->iType])
		return -1;
	else if (g_tIconTypeOrder[icon1->iType] > g_tIconTypeOrder[icon2->iType])
		return 1;
	else
	{
		if (icon1->fOrder < icon2->fOrder)
			return -1;
		else if (icon1->fOrder > icon2->fOrder)
			return 1;
		else
			return 0;
	}
}


Icon* cairo_dock_get_first_icon (GList *pIconList)
{
	GList *pListHead = g_list_first(pIconList);
	return (pListHead != NULL ? pListHead->data : NULL);
}

Icon* cairo_dock_get_last_icon (GList *pIconList)
{
	GList *pListTail = g_list_last(pIconList);
	return (pListTail != NULL ? pListTail->data : NULL);
}

Icon *cairo_dock_get_first_drawn_icon (CairoDock *pDock)
{
	if (pDock->pFirstDrawnElement != NULL)
		return pDock->pFirstDrawnElement->data;
	else
		return cairo_dock_get_first_icon (pDock->icons);
}
Icon *cairo_dock_get_last_drawn_icon (CairoDock *pDock)
{
	if (pDock->pFirstDrawnElement != NULL)
	{
		if (pDock->pFirstDrawnElement->prev != NULL)
			return pDock->pFirstDrawnElement->prev->data;
		else
			return cairo_dock_get_last_icon (pDock->icons);
	}
	else
		return cairo_dock_get_last_icon (pDock->icons);;
}

Icon* cairo_dock_get_first_icon_of_type (GList *pIconList, CairoDockIconType iType)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->iType == iType)
			return icon;
	}
	return NULL;
}


Icon* cairo_dock_get_last_icon_of_type (GList *pIconList, CairoDockIconType iType)
{
	GList* ic;
	Icon *icon;
	for (ic = g_list_last (pIconList); ic != NULL; ic = ic->prev)
	{
		icon = ic->data;
		if (icon->iType == iType)
			return icon;
	}
	return NULL;
}

Icon* cairo_dock_get_pointed_icon (GList *pIconList)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->bPointed)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_bouncing_icon (GList *pIconList)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->iCount > 0)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_removing_or_inserting_icon (GList *pIconList)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->fPersonnalScale != 0)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_animated_icon (GList *pIconList)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->fPersonnalScale != 0 || icon->iCount > 0)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_next_icon (GList *pIconList, Icon *pIcon)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon == pIcon)
		{
			if (ic->next != NULL)
				return ic->next->data;
			else
				return NULL;
		}
	}
	return NULL;
}

Icon *cairo_dock_get_previous_icon (GList *pIconList, Icon *pIcon)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon == pIcon)
		{
			if (ic->prev != NULL)
				return ic->prev->data;
			else
				return NULL;
		}
	}
	return NULL;
}

Icon *cairo_dock_get_icon_with_command (GList *pIconList, gchar *cCommand)
{
	g_return_val_if_fail (cCommand != NULL, NULL);
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (strcmp (icon->acCommand, cCommand) == 0)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_icon_with_base_uri (GList *pIconList, gchar *cBaseURI)
{
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->cBaseURI != NULL && strcmp (icon->cBaseURI, cBaseURI) == 0)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_icon_with_subdock (GList *pIconList, CairoDock *pSubDock)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->pSubDock == pSubDock)
			return icon;
	}
	return NULL;
}



void cairo_dock_swap_icons (CairoDock *pDock, Icon *icon1, Icon *icon2)
{
	//g_print ("%s (%s, %s) : %.2f <-> %.2f\n", __func__, icon1->acName, icon2->acName, icon1->fOrder, icon2->fOrder);
	if (! ( (CAIRO_DOCK_IS_APPLI (icon1) && CAIRO_DOCK_IS_APPLI (icon2)) || (CAIRO_DOCK_IS_LAUNCHER (icon1) && CAIRO_DOCK_IS_LAUNCHER (icon2)) || (CAIRO_DOCK_IS_APPLET (icon1) && CAIRO_DOCK_IS_APPLET (icon2)) ) )
		return ;
	
	//\_________________ On intervertit les ordres des 2 lanceurs.
	double fSwap = icon1->fOrder;
	icon1->fOrder = icon2->fOrder;
	icon2->fOrder = fSwap;
	
	//\_________________ On change l'ordre dans les fichiers des 2 lanceurs.
	if (CAIRO_DOCK_IS_LAUNCHER (icon1))
	{
		GError *erreur = NULL;
		gchar *cDesktopFilePath;
		GKeyFile* pKeyFile;
		
		if (icon1->acDesktopFileName != NULL)
		{
			cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, icon1->acDesktopFileName);
			pKeyFile = g_key_file_new();
			g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				return ;
			}
			
			g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", icon1->fOrder);
			cairo_dock_write_keys_to_file (pKeyFile, cDesktopFilePath);
			g_key_file_free (pKeyFile);
			g_free (cDesktopFilePath);
		}
		
		if (icon2->acDesktopFileName != NULL)
		{
			cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, icon2->acDesktopFileName);
			pKeyFile = g_key_file_new();
			g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				return ;
			}
			
			g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", icon2->fOrder);
			cairo_dock_write_keys_to_file (pKeyFile, cDesktopFilePath);
			g_key_file_free (pKeyFile);
			g_free (cDesktopFilePath);
		}
	}
	
	//\_________________ On les intervertit dans la liste.
	if (pDock->pFirstDrawnElement != NULL && (pDock->pFirstDrawnElement->data == icon1 || pDock->pFirstDrawnElement->data == icon2))
		pDock->pFirstDrawnElement = NULL;
	pDock->icons = g_list_remove (pDock->icons, icon1);
	pDock->icons = g_list_remove (pDock->icons, icon2);
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon1,
		(GCompareFunc) cairo_dock_compare_icons_order);
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon2,
		(GCompareFunc) cairo_dock_compare_icons_order);
	
	//\_________________ On recalcule la largeur max, qui peut avoir ete influencee par le changement d'ordre.
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, pDock->iMinDockWidth, pDock->iScrollOffset);
	pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->iMinDockWidth)) + 1;
	
	//\_________________ On met a jour l'ordre des applets dans le fichier de conf.
	if (CAIRO_DOCK_IS_APPLET (icon1))
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, pDock->icons, g_hModuleTable);
}

void cairo_dock_move_icon_after_icon (CairoDock *pDock, Icon *icon1, Icon *icon2)
{
	//g_print ("%s (%s, %s) : %.2f <-> %.2f\n", __func__, icon1->acName, icon2->acName, icon1->fOrder, icon2->fOrder);
	if ((icon2 != NULL) && (! ( (CAIRO_DOCK_IS_APPLI (icon1) && CAIRO_DOCK_IS_APPLI (icon2)) || (CAIRO_DOCK_IS_LAUNCHER (icon1) && CAIRO_DOCK_IS_LAUNCHER (icon2)) || (CAIRO_DOCK_IS_APPLET (icon1) && CAIRO_DOCK_IS_APPLET (icon2)) ) ))
		return ;
	
	//\_________________ On change l'ordre de l'icone.
	if (icon2 != NULL)
	{
		Icon *pNextIcon = cairo_dock_get_next_icon (pDock->icons, icon2);
		if (pNextIcon == NULL || pNextIcon->iType != icon2->iType)
			icon1->fOrder = icon2->fOrder + 1;
		else
			icon1->fOrder = (pNextIcon->fOrder - icon2->fOrder > 1 ? icon2->fOrder + 1 : (pNextIcon->fOrder + icon2->fOrder) / 2);
	}
	else
	{
		Icon *pFirstIcon = cairo_dock_get_first_icon_of_type (pDock->icons, icon1->iType);
		if (pFirstIcon != NULL)
			icon1->fOrder = pFirstIcon->fOrder - 1;
		else
			icon1->fOrder = 1;
	}
	
	//\_________________ On change l'ordre dans le fichier du lanceur 1.
	if (CAIRO_DOCK_IS_LAUNCHER (icon1) && icon1->acDesktopFileName != NULL)
	{
		GError *erreur = NULL;
		gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, icon1->acDesktopFileName);
		GKeyFile* pKeyFile = g_key_file_new();
		g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return ;
		}
		
		g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", icon1->fOrder);
		cairo_dock_write_keys_to_file (pKeyFile, cDesktopFilePath);
		g_key_file_free (pKeyFile);
		g_free (cDesktopFilePath);
	}
	
	//\_________________ On change sa place dans la liste.
	pDock->pFirstDrawnElement = NULL;
	pDock->icons = g_list_remove (pDock->icons, icon1);
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon1,
		(GCompareFunc) cairo_dock_compare_icons_order);
	
	//\_________________ On recalcule la largeur max, qui peut avoir ete influencee par le changement d'ordre.
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest (pDock->icons, pDock->iMinDockWidth, pDock->iScrollOffset);
	pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->iMinDockWidth)) + 1;
	
	if (CAIRO_DOCK_IS_APPLET (icon1))
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, pDock->icons, g_hModuleTable);
}


void cairo_dock_remove_one_icon_from_dock (CairoDock *pDock, Icon *icon)
{
	//\___________________ On effectue les taches de fermeture de l'icone suivant son type.
	if (icon->iPid != 0 && g_bUniquePid)
		g_hash_table_remove (g_hAppliTable, &icon->iPid);
	if (icon->Xid != 0)
		g_hash_table_remove (g_hXWindowTable, &icon->Xid);
	if (icon->pModule != NULL)
	{
		cairo_dock_deactivate_module (icon->pModule);  // desactive le module mais ne le decharge pas.
		icon->pModule = NULL;  // pour ne pas le liberer lors du free_icon.
	}
	
	//\___________________ On l'enleve de la liste.
	if (pDock->pFirstDrawnElement != NULL && pDock->pFirstDrawnElement->data == icon)
	{
		if (pDock->pFirstDrawnElement->next != NULL)
			pDock->pFirstDrawnElement = pDock->pFirstDrawnElement->next;
		else
		{
			if (pDock->icons != NULL && pDock->icons->next != NULL)  // la liste n'a pas qu'un seul element.
				pDock->pFirstDrawnElement = pDock->icons;
			else
				pDock->pFirstDrawnElement = NULL;
		}
	}
	pDock->icons = g_list_remove (pDock->icons, icon);
	pDock->iMinDockWidth -= g_iIconGap + icon->fWidth;
	
	//\___________________ Cette icone realisait peut-etre le max des hauteurs, comme on l'enleve on recalcule ce max. 
	Icon *pOtherIcon;
	GList *ic;
	if (icon->fHeight == pDock->iMaxIconHeight)
	{
		pDock->iMaxIconHeight = 0;
		for (ic = pDock->icons; ic != NULL; ic = ic->next)
		{
			pOtherIcon = ic->data;
			pDock->iMaxIconHeight = MAX (pDock->iMaxIconHeight, pOtherIcon->fHeight);
		}
	}
	
	if (pDock->bIsMainDock && g_bReserveSpace)
		cairo_dock_reserve_space_for_dock (pDock, TRUE);
}

void cairo_dock_remove_icon_from_dock (CairoDock *pDock, Icon *icon)
{
	//g_print ("%s (%s)\n", __func__, icon->acName);
	cairo_dock_remove_one_icon_from_dock (pDock, icon);
	
	//\___________________ On enleve le separateur si c'est la derniere icone de son type.
	Icon * pSeparatorIcon = NULL;
	if (! CAIRO_DOCK_IS_SEPARATOR (icon))
	{
		Icon *pSameTypeIcon = cairo_dock_get_first_icon_of_type (pDock->icons, icon->iType);
		if (pSameTypeIcon == NULL)
		{
			if (g_tIconTypeOrder[icon->iType] > 1)  // attention : iType - 1 > 0 si iType = 0, car c'est un unsigned int !
				pSeparatorIcon = cairo_dock_get_first_icon_of_type (pDock->icons, g_tIconTypeOrder[icon->iType] - 1);
			else if (g_tIconTypeOrder[icon->iType] + 1 < CAIRO_DOCK_NB_TYPES)
				pSeparatorIcon = cairo_dock_get_first_icon_of_type (pDock->icons, g_tIconTypeOrder[icon->iType] + 1);
			
			if (pSeparatorIcon != NULL)
			{
				//g_print ("  on enleve un separateur\n");
				cairo_dock_remove_one_icon_from_dock (pDock, pSeparatorIcon);
				cairo_dock_free_icon (pSeparatorIcon);
			}
		}
	}
}

void cairo_dock_remove_icons_of_type (CairoDock *pDock, CairoDockIconType iType)
{
	//g_print ("%s (%d)\n", __func__, iType);
	Icon *icon;
	GList *ic;
	if (pDock->icons == NULL)
		return ;
	
	gboolean bOneIconFound = FALSE;
	Icon *pSeparatorIcon = NULL;
	ic = pDock->icons;
	for (ic = pDock->icons->next; ic != NULL; ic = ic->next)
	{
		icon = ic->prev->data;  // on ne peut pas enlever l'element courant, sinon on perd 'ic'.
		if (icon->iType == iType)
		{
			bOneIconFound = TRUE;
			cairo_dock_remove_one_icon_from_dock (pDock, icon);
			cairo_dock_free_icon (icon);
		}
		else if (CAIRO_DOCK_IS_SEPARATOR (icon))
		{
			if ( (bOneIconFound && pSeparatorIcon == NULL) || (! bOneIconFound) )
				pSeparatorIcon = icon;
		}
	}
	
	icon = cairo_dock_get_last_icon_of_type (pDock->icons, iType);
	if (icon != NULL && icon->iType == iType)
	{
		bOneIconFound = TRUE;
		cairo_dock_remove_one_icon_from_dock (pDock, icon);
		cairo_dock_free_icon (icon);
	}
	
	if (bOneIconFound && pSeparatorIcon != NULL)
	{
		//g_print ("  on enleve un separateur\n");
		cairo_dock_remove_one_icon_from_dock (pDock, pSeparatorIcon);
		cairo_dock_free_icon (pSeparatorIcon);
	}
}

void cairo_dock_remove_separator (CairoDock *pDock, CairoDockIconType iType)
{
	GList* ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->iType == iType)
			pDock->icons = g_list_remove (pDock->icons, icon);
	}
}


void cairo_dock_remove_all_separators (CairoDock *pDock)
{
	GList* ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (CAIRO_DOCK_IS_SEPARATOR (icon))
			cairo_dock_remove_icon_from_dock (pDock, icon);
	}
}

void cairo_dock_remove_all_applis (CairoDock *pDock)
{
	if (g_iSidUpdateAppliList != 0)
		g_source_remove (g_iSidUpdateAppliList);
	g_iSidUpdateAppliList = 0;
	
	cairo_dock_remove_icons_of_type (pDock, CAIRO_DOCK_APPLI);
}

void cairo_dock_remove_all_applets (CairoDock *pDock)
{
	cairo_dock_remove_icons_of_type (pDock, CAIRO_DOCK_APPLET);
}



GList *cairo_dock_calculate_icons_positions_at_rest (GList *pIconList, int iMinDockWidth, int iXOffset)
{
	double x_cumulated = iXOffset;
	double fXMin = 99999;
	GList* ic, *pFirstDrawnElement = NULL;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		if (x_cumulated + icon->fWidth / 2 < 0)
			icon->fXAtRest = x_cumulated + iMinDockWidth;
		else if (x_cumulated + icon->fWidth / 2 > iMinDockWidth)
			icon->fXAtRest = x_cumulated - iMinDockWidth;
		else
			icon->fXAtRest = x_cumulated;
		
		if (icon->fXAtRest < fXMin)
		{
			fXMin = icon->fXAtRest;
			pFirstDrawnElement = ic;
		}
		
		x_cumulated += icon->fWidth + g_iIconGap;
	}
	
	return pFirstDrawnElement;
}

Icon * cairo_dock_calculate_icons_with_position (GList *pIconList, GList *pFirstDrawnElementGiven, int x_abs, gdouble fMagnitude, int iMinDockWidth, int iWidth, int iHeight, int iMouseY, double fAlign, double fLateralFactor)
{
	//g_print (">>>>>%s (%d, %dx%d)\n", __func__, x_abs, iWidth, iHeight);
	if (pIconList == NULL)
		return NULL;
	float x_cumulated = 0, fXMiddle, fDeltaExtremum;
	int iXMinSinusoid = x_abs - g_iSinusoidWidth / 2;
	int iXMaxSinusoid = x_abs + g_iSinusoidWidth / 2;
	//g_print ("%d <-> %d\n", iXMinSinusoid, iXMaxSinusoid);
	int c;
	GList* ic, *pointed_ic;
	Icon *icon, *prev_icon, *next_icon;
	
	GList *pFirstDrawnElement = (pFirstDrawnElementGiven != NULL ? pFirstDrawnElementGiven : pIconList);
	ic = pFirstDrawnElement;
	pointed_ic = (x_abs < 0 ? ic : NULL);
	do
	{
		icon = ic->data;
		x_cumulated = icon->fXAtRest;
		fXMiddle = icon->fXAtRest + icon->fWidth / 2;
		
		//\_______________ On calcule sa phase (pi/2 au niveau du curseur).
		icon->fPhase = (fXMiddle - x_abs) / g_iSinusoidWidth * G_PI + G_PI / 2;
		if (icon->fPhase < 0)
			icon->fPhase = 0;
		else if (icon->fPhase > G_PI)
			icon->fPhase = G_PI;
		
		//\_______________ On en deduit l'amplitude de la sinusoide au niveau de cette icone, et donc son echelle.
		icon->fScale = 1 + fMagnitude * g_fAmplitude * sin (icon->fPhase);
		//if (CAIRO_DOCK_IS_SEPARATOR (icon))
		//	icon->fScale = 1;
		if (icon->fPersonnalScale > 0 && iWidth > 0)
		{
			icon->fPersonnalScale *= .85;
			icon->fScale *= icon->fPersonnalScale;
			if (icon->fPersonnalScale < 0.05)
				icon->fPersonnalScale = 0.05;
		}
		else if (icon->fPersonnalScale < 0 && iWidth > 0)
		{
			icon->fPersonnalScale *= .85;
			icon->fScale *= (1 + icon->fPersonnalScale);
			if (icon->fPersonnalScale > -0.05)
				icon->fPersonnalScale = -0.05;
		}
		icon->fY = (g_bDirectionUp ? iHeight - g_iDockLineWidth - icon->fScale * icon->fHeight : g_iDockLineWidth);
		
		//\_______________ Si on avait deja defini l'icone pointee, on peut placer l'icone courante par rapport a la precedente.
		if (pointed_ic != NULL)
		{
			if (ic == pFirstDrawnElement)  // peut arriver si on est en dehors a gauche du dock.
			{
				icon->fX = x_cumulated - 1. * (iMinDockWidth - iWidth) / 2;
			}
			else
			{
				prev_icon = (ic->prev != NULL ? ic->prev->data : cairo_dock_get_last_icon (pIconList));
				icon->fX = prev_icon->fX + prev_icon->fWidth * prev_icon->fScale + g_iIconGap;
				
				if (icon->fX + icon->fWidth * icon->fScale > icon->fXMax - g_fAmplitude * icon->fWidth / 6 && iWidth != 0)  /// && icon->fPhase == G_PI
				{
					//g_print ("  on contraint %s (fXMax=%.2f , fX=%.2f\n", prev_icon->acName, prev_icon->fXMax, prev_icon->fX);
					fDeltaExtremum = icon->fX + icon->fWidth * icon->fScale - (icon->fXMax - g_fAmplitude * icon->fWidth / 12);
					icon->fX -= fDeltaExtremum * (1 - (prev_icon->fScale - 1) / g_fAmplitude);
					///icon->fX = icon->fXMax - icon->fWidth * icon->fScale - g_fAmplitude * icon->fWidth / 16;
				}
			}
			icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1. - fLateralFactor);
		}
		
		//\_______________ On regarde si on pointe sur cette icone.
		if (x_cumulated + icon->fWidth + g_iIconGap >= x_abs && x_cumulated <= x_abs && pointed_ic == NULL)  // on a trouve l'icone sur laquelle on pointe.
		{
			pointed_ic = ic;
			icon->bPointed = TRUE;
			icon->fX = x_cumulated - (iMinDockWidth - iWidth) / 2 + (1 - icon->fScale) * (x_abs - x_cumulated);
			icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1. - fLateralFactor);
			//g_print ("icone pointee : fX=%.2f\n", icon->fX);
		}
		else
			icon->bPointed = FALSE;
		
		if (icon->iAnimationType == CAIRO_DOCK_FOLLOW_MOUSE)
		{
			icon->fScale = 1 + g_fAmplitude;
			icon->fDrawX = x_abs - (iMinDockWidth - iWidth) / 2;
			icon->fDrawY = iMouseY;
		}
		
		ic = ic->next;
		if (ic == NULL)
			ic = pIconList;
	} while (ic != pFirstDrawnElement);
	
	//\_______________ On place les icones precedant l'icone pointee par rapport a celle-ci.
	if (pointed_ic == NULL)  // on est a droite des icones.
	{
		pointed_ic = (pFirstDrawnElement->prev == NULL ? g_list_last (pIconList) : pFirstDrawnElement->prev);
		icon = pointed_ic->data;
		icon->fX = x_cumulated - (iMinDockWidth - iWidth) / 2 + (1 - icon->fScale) * icon->fWidth;
		icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1 - fLateralFactor);
	}
	
	ic = pointed_ic;
	while (ic != pFirstDrawnElement)
	{
		icon = ic->data;
		
		ic = ic->prev;
		if (ic == NULL)
			ic = g_list_last (pIconList);
		
		prev_icon = ic->data;
		
		prev_icon->fX = icon->fX - g_iIconGap - prev_icon->fWidth * prev_icon->fScale;
		//g_print ("fX <- %.2f; fXMin : %.2f\n", prev_icon->fX, prev_icon->fXMin);
		if (prev_icon->fX < prev_icon->fXMin + g_fAmplitude * prev_icon->fWidth / 6 && iWidth != 0 && x_abs < iWidth)  /// && prev_icon->fPhase == 0 
		{
			//g_print ("  on contraint %s (fXMin=%.2f , fX=%.2f\n", prev_icon->acName, prev_icon->fXMin, prev_icon->fX);
			fDeltaExtremum = prev_icon->fX - (prev_icon->fXMin + g_fAmplitude * prev_icon->fWidth / 12);
			prev_icon->fX -= fDeltaExtremum * (1 - (prev_icon->fScale - 1) / g_fAmplitude);
			///prev_icon->fX = prev_icon->fXMin + g_fAmplitude * prev_icon->fWidth / 16;
		}
		prev_icon->fX = fAlign * iWidth + (prev_icon->fX - fAlign * iWidth) * (1. - fLateralFactor);
	}
	
	return pointed_ic->data;
}


Icon *cairo_dock_calculate_icons (CairoDock *pDock, int iMouseX, int iMouseY)
{
	//\_______________ On calcule la position du curseur dans le referentiel du dock a plat.
	static gboolean bReturn = FALSE;
	int iWidth, iHeight;
	iWidth = pDock->iCurrentWidth;
	iHeight = pDock->iCurrentHeight;
	//g_print ("%s (%dx%d, %dx%d)\n", __func__, iMouseX, iMouseY, iWidth, iHeight);
	
	int dx = iMouseX - iWidth / 2;  // ecart par rapport au milieu du dock a plat.
	int x_abs = dx + pDock->iMinDockWidth / 2;  // ecart par rapport a la gauche du dock minimal.
	
	///pDock->fDecorationsOffsetX = iMouseX - pDock->iCurrentWidth / 2;
	
	//\_______________ On calcule l'ensemble des parametres des icones.
	Icon *pPointedIcon = cairo_dock_calculate_icons_with_position (pDock->icons, pDock->pFirstDrawnElement, x_abs, pDock->fMagnitude, pDock->iMinDockWidth, pDock->iMaxDockWidth, iHeight, iMouseY, pDock->fAlign, pDock->fLateralFactor);
	
	//\_______________ On regarde si le curseur est dans le dock ou pas, et on joue sur la taille des icones en consequence.
	
	gboolean bMouseInsideDock = (x_abs >= 0 && x_abs <= pDock->iMinDockWidth && iMouseX > 0 && iMouseX < iWidth);
	if (! bMouseInsideDock)
		pDock->fDecorationsOffsetX = - pDock->iMinDockWidth / 2;  // on fixe les decorations.
	
	if (! bMouseInsideDock && pDock->iSidGrowUp == 0 && pDock->iSidShrinkDown == 0 && pDock->fMagnitude > 0)
	{
		double fSideMargin = (pDock->fAlign - .5) * (iWidth - pDock->iMinDockWidth);
		if (x_abs < fSideMargin || x_abs > pDock->iMinDockWidth + fSideMargin)
			g_signal_emit_by_name (pDock->pWidget, "leave-notify-event", NULL, &bReturn);
		else
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, pDock);
	}
	
	if (bMouseInsideDock && pDock->fMagnitude < 1 && pDock->iSidGrowUp == 0 && cairo_dock_none_animated (pDock->icons) && pDock->iSidMoveDown == 0)  // on est dedans en x et la taille des icones est non maximale bien qu'aucune icone  ne soit animee.  // && pDock->iSidShrinkDown == 0 && ! pDock->bAtBottom
	{
		if ( (g_bDirectionUp && pPointedIcon != NULL && iMouseY > 0 && iMouseY < iHeight) || (! g_bDirectionUp && pPointedIcon != NULL && iMouseY < iHeight && iMouseY > 0) )  // et en plus on est dedans en y.
		{
			//g_print ("on est dedans en x et en y et la taille des icones est non maximale bien qu'aucune icone  ne soit animee (iMouseX=%d => x_abs=%d)\n", iMouseX, x_abs);
			//pDock->bInside = TRUE;
			if (pDock->bAtBottom)  // on emule une re-rentree.
			{
				g_signal_emit_by_name (pDock->pWidget, "enter-notify-event", NULL, &bReturn);
			}
			else  // on se contente de faire grossir les icones.
			{
				pDock->bAtBottom = FALSE;
				if (pDock->iSidShrinkDown != 0)
				{
					g_source_remove (pDock->iSidShrinkDown);
					pDock->iSidShrinkDown = 0;
				}
				if (pDock->iSidMoveDown != 0)
				{
					g_source_remove (pDock->iSidMoveDown);
					pDock->iSidMoveDown = 0;
				}
				if (pDock->iSidGrowUp == 0)
					pDock->iSidGrowUp = g_timeout_add (40, (GSourceFunc) cairo_dock_grow_up, pDock);
				if (g_bAutoHide && pDock->iRefCount == 0 && pDock->iSidMoveUp == 0)
					pDock->iSidMoveUp = g_timeout_add (40, (GSourceFunc) cairo_dock_move_up, pDock);
			}
		}
	}
	
	return (bMouseInsideDock ? pPointedIcon : NULL);
}


double cairo_dock_calculate_max_dock_width (CairoDock *pDock, GList *pFirstDrawnElementGiven, int iFlatDockWidth)
{
	double fMaxDockWidth = 0.;
	//g_print ("%s (%d)\n", __func__, iFlatDockWidth);
	GList *pIconList = pDock->icons;
	if (pIconList == NULL)
		return 2 * g_iDockRadius + g_iDockLineWidth;
	
	//\_______________ On remet a zero les positions d'equilibre des icones.
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fXMax = -1e4;
		icon->fXMin = 1e4;
	}
	
	//\_______________ On simule le passage du curseur sur toute la largeur du dock, et on chope la largeur maximale qui s'en degage, ainsi que les positions d'equilibre de chaque icone.
	int iVirtualMouseX;
	double fMaxBorderX = 0;
	GList *pFirstDrawnElement = (pFirstDrawnElementGiven != NULL ? pFirstDrawnElementGiven : pIconList);
	//for (iVirtualMouseX = 0; iVirtualMouseX < iFlatDockWidth; iVirtualMouseX ++)
	GList *ic2;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		cairo_dock_calculate_icons_with_position (pIconList, pFirstDrawnElement, icon->fXAtRest, 1, iFlatDockWidth, 0, 0, 0, 0.5, 0);
		ic2 = pFirstDrawnElement;
		do
		{
			icon = ic2->data;
			
			if (icon->fX + icon->fWidth * icon->fScale > icon->fXMax)
				icon->fXMax = icon->fX + icon->fWidth * icon->fScale;
			if (icon->fX < icon->fXMin)
				icon->fXMin = icon->fX;
			
			ic2 = ic2->next;
			if (ic2 == NULL)
				ic2 = pIconList;
		} while (ic2 != pFirstDrawnElement);
		fMaxBorderX = MAX (fMaxBorderX, icon->fX + icon->fWidth * icon->fScale);
	}
	cairo_dock_calculate_icons_with_position (pIconList, pFirstDrawnElement, iFlatDockWidth - 1, 1, iFlatDockWidth, 0, 0, 0, pDock->fAlign, pDock->fLateralFactor);
	ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		if (icon->fX + icon->fWidth * icon->fScale > icon->fXMax)
			icon->fXMax = icon->fX + icon->fWidth * icon->fScale;
		if (icon->fX < icon->fXMin)
			icon->fXMin = icon->fX;
		
		ic = ic->next;
		if (ic == NULL)
			ic = pIconList;
	} while (ic != pFirstDrawnElement);
	fMaxBorderX = MAX (fMaxBorderX, icon->fX + icon->fWidth * icon->fScale);
	
	fMaxDockWidth = icon->fXMax - ((Icon *) pFirstDrawnElement->data)->fXMin + 2 * g_iDockRadius + g_iDockLineWidth;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fXMin += fMaxDockWidth / 2;
		icon->fXMax += fMaxDockWidth / 2;
		//g_print ("%s : [%d;%d]\n", icon->acName, (int) icon->fXMin, (int) icon->fXMax);
		icon->fX = icon->fXAtRest;
		icon->fScale = 1;
	}
	
	return fMaxDockWidth;
}

void cairo_dock_mark_icons_as_avoiding_mouse (CairoDock *pDock, int iMouseX)
{
	if (pDock->icons == NULL)
		return;
	int x_abs = iMouseX - (pDock->iCurrentWidth - pDock->iMinDockWidth) / 2;  // ecart par rapport a la gauche du dock minimal.
	
	Icon *icon;
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		if (x_abs >= icon->fXAtRest && x_abs <= icon->fXAtRest + icon->fWidth)  // on n'utilise pas icon->bPointed, pour pouvoir remettre a zero.
		{
			icon->iAnimationType = 0;
			
			if (x_abs < icon->fXAtRest + icon->fWidth / 4)  // on est a gauche.
			{
				Icon *prev_icon;
				if (ic->prev != NULL)
					prev_icon = ic->prev->data;
				else
					prev_icon = g_list_last (pDock->icons)->data;
				if (CAIRO_DOCK_IS_LAUNCHER (icon) || CAIRO_DOCK_IS_LAUNCHER (prev_icon))
				{
					icon->iAnimationType = CAIRO_DOCK_AVOID_MOUSE;
					prev_icon->iAnimationType = CAIRO_DOCK_AVOID_MOUSE;
				}
			}
			else if (x_abs > icon->fXAtRest + 3. * icon->fWidth / 4)  // on est a droite.
			{
				Icon *next_icon;
				if (ic->next != NULL)
					next_icon = ic->next->data;
				else
					next_icon = pDock->icons->data;
				if (CAIRO_DOCK_IS_LAUNCHER (icon) || CAIRO_DOCK_IS_LAUNCHER (next_icon))
				{
					icon->iAnimationType = CAIRO_DOCK_AVOID_MOUSE;
					next_icon->iAnimationType = CAIRO_DOCK_AVOID_MOUSE;
				}
				ic = ic->next;  // on la saute.
				if (ic == NULL)
					ic = pDock->icons;
				if (ic == pFirstDrawnElement)
					break ;
			}
		}
		else
			icon->iAnimationType = 0;
		
		ic = ic->next;
		if (ic == NULL)
			ic = pDock->icons;
	} while (ic != pFirstDrawnElement);
}


void cairo_dock_update_icon_s_container_name (Icon *icon, gchar *cNewParentDockName)
{
	g_free (icon->cParentDockName);
	icon->cParentDockName = g_strdup (cNewParentDockName);
	
	if (icon->acDesktopFileName != NULL)
	{
		gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, icon->acDesktopFileName);
		
		GError *erreur = NULL;
		GKeyFile *pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			g_free (cDesktopFilePath);
			return ;
		}
		
		g_key_file_set_string (pKeyFile, "Desktop Entry", "Container", cNewParentDockName);
		cairo_dock_write_keys_to_file (pKeyFile, cDesktopFilePath);
		
		g_free (cDesktopFilePath);
		g_key_file_free (pKeyFile);
	}
}
