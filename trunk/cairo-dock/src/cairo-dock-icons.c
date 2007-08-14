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
#include "cairo-dock-icons.h"


extern gint g_iScreenWidth;
extern gint g_iScreenHeight;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern int g_iIconGap;
extern int g_iLabelSize;
extern gboolean g_bRoundedBottomCorner;

extern double g_fStripesColorBright[4];
extern double g_fStripesColorDark[4];

extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern gchar *g_cCairoDockDataDir;

extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
extern double g_fVisibleZoneAlpha;
extern int g_iNbStripes;
extern double g_fAmplitude;
extern int g_iSinusoidWidth;

extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;
extern gboolean g_bUseText;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern GHashTable *g_hAppliTable;
extern gboolean g_bUniquePid;
extern GHashTable *g_hXWindowTable;
extern int g_iSidUpdateAppliList;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];
extern gchar *g_cConfFile;
extern GHashTable *g_hModuleTable;


extern gboolean g_bKeepAbove;
extern gboolean g_bSkipPager;
extern gboolean g_bSkipTaskbar;
extern gboolean g_bSticky;
extern CairoDock *g_pMainDock;


void cairo_dock_free_icon (Icon *icon)
{
	if (icon == NULL)
		return ;
	g_free (icon->acDesktopFileName);
	g_free (icon->acFileName);
	g_free (icon->acName);
	g_free (icon->acCommand);
	
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
		gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, icon1->acDesktopFileName);
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
		
		cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, icon2->acDesktopFileName);
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
	
	//\_________________ On les intervertit dans la liste.
	pDock->icons = g_list_remove (pDock->icons, icon1);
	pDock->icons = g_list_remove (pDock->icons, icon2);
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon1,
		(GCompareFunc) cairo_dock_compare_icons_order);
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon2,
		(GCompareFunc) cairo_dock_compare_icons_order);
	
	if (CAIRO_DOCK_IS_APPLET (icon1))
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, pDock->icons, g_hModuleTable);
	
	//\_________________ On recalcule la largeur max, qui peut avoir ete influencee par le changement d'ordre.
	pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock->icons, pDock->iMinDockWidth)) + 1;
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
			icon1->fOrder = (pNextIcon->fOrder - icon2->fOrder > 1 ? icon2->fOrder + 1 : (pNextIcon->fOrder - icon2->fOrder) / 2);
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
	if (CAIRO_DOCK_IS_LAUNCHER (icon1))
	{
		GError *erreur = NULL;
		gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, icon1->acDesktopFileName);
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
	pDock->icons = g_list_remove (pDock->icons, icon1);
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon1,
		(GCompareFunc) cairo_dock_compare_icons_order);
	
	if (CAIRO_DOCK_IS_APPLET (icon1))
		cairo_dock_update_conf_file_with_active_modules (g_cConfFile, pDock->icons, g_hModuleTable);
	
	//\_________________ On recalcule la largeur max, qui peut avoir ete influencee par le changement d'ordre.
	pDock->iMaxDockWidth = (int) ceil (cairo_dock_calculate_max_dock_width (pDock->icons, pDock->iMinDockWidth)) + 1;
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
		cairo_dock_deactivate_module (icon->pModule);
		//cairo_dock_close_module (icon->pModule, NULL);
		icon->pModule = NULL;  // pour ne pas le liberer lors du free_icon.
	}
	
	//\___________________ On l'enleve de la liste.
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
}

void cairo_dock_remove_icon_from_dock (CairoDock *pDock, Icon *icon)
{
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
				pDock->icons = g_list_remove (pDock->icons, pSeparatorIcon);
				cairo_dock_free_icon (pSeparatorIcon);
			}
		}
	}
}

void cairo_dock_remove_icons_of_type (CairoDock *pDock, CairoDockIconType iType)
{
	Icon *icon;
	GList *ic;
	if (pDock->icons == NULL)
		return ;
	
	gboolean bInside = FALSE;
	Icon *pSeparatorIcon = NULL;
	ic = pDock->icons;
	if (ic->next != NULL)
	{
		for (ic = pDock->icons->next; ic != NULL; ic = ic->next)
		{
			icon = ic->prev->data;  // on ne peut pas enlever l'element courant, sinon on perd 'ic'.
			if (icon->iType == iType)
			{
				bInside = TRUE;
				cairo_dock_remove_one_icon_from_dock (pDock, icon);
				cairo_dock_free_icon (icon);
			}
			else if ((! bInside || pSeparatorIcon == NULL) && CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				pSeparatorIcon = icon;
			}
		}
	}
	
	icon = cairo_dock_get_last_icon_of_type (pDock->icons, iType);
	if (icon != NULL)
	{
		cairo_dock_remove_one_icon_from_dock (pDock, icon);
		cairo_dock_free_icon (icon);
	}
	
	if (pSeparatorIcon != NULL)
	{
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


Icon * cairo_dock_calculate_icons_with_position (GList *pIconList, int x_abs, gdouble fMagnitude, int iMinDockWidth, int iWidth, int iHeight)
{
	//g_print ("%s (%d)\n", __func__, x_abs);
	float x_cumulated = 0;
	int c;
	GList* ic, *pointed_ic = (x_abs < 0 ? pIconList : NULL);
	Icon *icon, *prev_icon, *next_icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		//\_______________ On calcule sa phase (pi/2 au niveau du curseur).
		icon->fPhase = ((x_cumulated + icon->fWidth / 2) - x_abs) / g_iSinusoidWidth * G_PI + G_PI / 2;
		if (icon->fPhase < 0)
			icon->fPhase = 0;
		else if (icon->fPhase > G_PI)
			icon->fPhase = G_PI;
		//if (CAIRO_DOCK_IS_SEPARATOR (icon))
		//	icon->fPhase = 0;
		
		//\_______________ On en deduit l'amplitude de la sinusoide au niveau de cette icone, et donc son echelle.
		icon->fScale = 1 + fMagnitude * g_fAmplitude * sin (icon->fPhase);
		if (icon->fPersonnalScale > 0 && iWidth > 0)
		{
			icon->fPersonnalScale *= .9;
			icon->fScale *= icon->fPersonnalScale;
			if (icon->fPersonnalScale < 0.05)
				icon->fPersonnalScale = 0.05;
		}
		else if (icon->fPersonnalScale < 0 && iWidth > 0)
		{
			icon->fPersonnalScale *= .9;
			icon->fScale *= (1 + icon->fPersonnalScale);
			if (icon->fPersonnalScale > -0.05)
				icon->fPersonnalScale = -0.05;
		}
		if (icon->iCount > 0 && icon->iAnimationType == CAIRO_DOCK_BOUNCE && iWidth > 0)
		{
			c = icon->iCount;
			if ( (c/5) & 1)  // c/5 est impair, on monte.
			{
				icon->fY -= (g_bDirectionUp ? (icon->fY - g_iLabelSize) : - (iHeight - g_iLabelSize - icon->fY - icon->fWidth * icon->fScale)) * .6;
			}
			else
			{
				icon->fY += (g_bDirectionUp ? (iHeight - (icon->fY + icon->fScale * icon->fHeight)) : - icon->fY) * .6;
			}
			icon->iCount --;
		}
		else
		{
			icon->fY = (g_bDirectionUp ? iHeight - g_iDockLineWidth - icon->fScale * icon->fHeight : g_iDockLineWidth);
		}
		//\_______________ Si on avait deja defini l'icone pointee, on peut placer l'icone courante par rapport a la precedente.
		if (pointed_ic != NULL)
		{
			if (ic->prev != NULL)  // peut etre faux si on est en dehors a gauche du dock.
			{
				prev_icon = ic->prev->data;
				icon->fX = prev_icon->fX + prev_icon->fWidth * prev_icon->fScale + g_iIconGap;
				if (icon->fX > icon->fXMax - icon->fWidth / 10 && iWidth != 0 && icon->fPhase == G_PI)
					icon->fX = icon->fXMax - icon->fWidth / 20;
			}
			else
			{
				icon->fX = x_cumulated - 1. * (iMinDockWidth - iWidth) / 2;
			}
		}
		
		//\_______________ On regarde si on pointe sur cette icone.
		if (x_cumulated + icon->fWidth + g_iIconGap >= x_abs && x_cumulated <= x_abs && pointed_ic == NULL)  // on a trouve l'icone sur laquelle on pointe.
		{
			pointed_ic = ic;
			icon->bPointed = TRUE;
			icon->fX = x_cumulated - iMinDockWidth / 2 + iWidth / 2 + (1 - icon->fScale) * (x_abs - x_cumulated);
		}
		else
			icon->bPointed = FALSE;
		
		x_cumulated += icon->fWidth + g_iIconGap;
	}
	
	//\_______________ On place les icones precedant l'icone pointee par rapport a celle-ci.
	if (pointed_ic == NULL)
	{
		pointed_ic = g_list_last (pIconList);
		if (pointed_ic == NULL)  // peut arriver si le dock est vide.
			return NULL;
		icon = pointed_ic->data;
		icon->fX = x_cumulated - iMinDockWidth / 2 + iWidth / 2 - icon->fWidth * icon->fScale - g_iIconGap;
	}
	for (ic = pointed_ic->prev; ic != NULL; ic = ic->prev)
	{
		icon = ic->data;
		next_icon = ic->next->data;
		
		icon->fX = next_icon->fX - g_iIconGap - icon->fWidth * icon->fScale;
		if (icon->fX < icon->fXMin + icon->fWidth / 10 && iWidth != 0 && icon->fPhase == 0 && x_abs < iWidth)
		{
			icon->fX = icon->fXMin + icon->fWidth / 20;
		}
	}
	
	return pointed_ic->data;
}


Icon *cairo_dock_calculate_icons (CairoDock *pDock, int iMouseX, int iMouseY)
{
	//\_______________ On calcule la position du curseur dans le referentiel du dock a plat.
	int iWidth, iHeight;
	iWidth = pDock->iCurrentWidth;
	iHeight = pDock->iCurrentHeight;
	if (g_bHorizontalDock)
		gtk_window_get_size (GTK_WINDOW (pDock->pWidget), &iWidth, &iHeight);
	else
		gtk_window_get_size (GTK_WINDOW (pDock->pWidget), &iHeight, &iWidth);
	//g_print ("%s (%dx%d)\n", __func__, iMouseX, iMouseY);
	
	int dx = iMouseX - iWidth / 2;  // ecart par rapport au milieu du dock a plat.
	int x_abs = dx + pDock->iMinDockWidth / 2 - pDock->iScrollOffset;  // ecart par rapport a la gauche du dock minimal.
	
	pDock->fGradientOffsetX = - iMouseX;  // indice de decalage des rayures.
	
	//\_______________ On calcule l'ensemble des parametres des icones.
	Icon *pPointedIcon = cairo_dock_calculate_icons_with_position (pDock->icons, x_abs, pDock->fMagnitude, pDock->iMinDockWidth, pDock->iMaxDockWidth, iHeight);  /// a la base ca marchait nickel avec iWidth.
	
	
	//\_______________ On regarde si le curseur est dans le dock ou pas, et on joue sur la taille des icones en consequence.
	gboolean bMouseInsideDock = (x_abs >= 0 && x_abs <= pDock->iMinDockWidth);
	
	if (! bMouseInsideDock && pDock->iSidShrinkDown == 0 && pDock->fMagnitude > 0 && pDock->iSidGrowUp == 0)
	{
		/*if (pDock->iSidGrowUp > 0)
		{
			g_source_remove (pDock->iSidGrowUp);
			pDock->iSidGrowUp = 0;
		}*/
		pDock->iSidShrinkDown = g_timeout_add (75, (GSourceFunc) cairo_dock_shrink_down, pDock);
	}
	
	if (bMouseInsideDock && pDock->fMagnitude < 1 && pDock->iSidGrowUp == 0 && cairo_dock_none_animated (pDock->icons))  // on est dedans en x et la taille des icones est non maximale bien qu'aucune icone  ne soit animee.  // && pDock->iSidShrinkDown == 0
	{
		if ( (g_bDirectionUp && pPointedIcon != NULL && iMouseY > iHeight - pPointedIcon->fHeight * pPointedIcon->fScale && iMouseY < iHeight) || (! g_bDirectionUp && pPointedIcon != NULL && iMouseY < pPointedIcon->fHeight * pPointedIcon->fScale && iMouseY > 0) )  // et en plus on est dedans en y.
		{
			//g_print ("on est dedans en x et en y et la taille des icones est non maximale bien qu'aucune icone  ne soit animee\n");
			if (pDock->iSidShrinkDown != 0)
			{
				g_source_remove (pDock->iSidShrinkDown);
				pDock->iSidShrinkDown = 0;
			}
			pDock->iSidGrowUp = g_timeout_add (75, (GSourceFunc) cairo_dock_grow_up, pDock);
		}
	}
	
	return (bMouseInsideDock ? pPointedIcon : NULL);
}


double cairo_dock_calculate_max_dock_width (GList *pIconList, int iFlatDockWidth)
{
	double fMaxDockWidth = 0.;
	//g_print ("%s (%d)\n", __func__, iFlatDockWidth);
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
	for (iVirtualMouseX = 0; iVirtualMouseX < iFlatDockWidth; iVirtualMouseX ++)
	{
		cairo_dock_calculate_icons_with_position (pIconList, iVirtualMouseX, 1, iFlatDockWidth, 0, 0);
		for (ic = pIconList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			
			if (icon->fX > icon->fXMax)
				icon->fXMax = icon->fX;
			if (icon->fX < icon->fXMin)
				icon->fXMin = icon->fX;
		}
		//fMaxDockWidth = MAX (fMaxDockWidth, get_current_dock_width (pIconList));
		fMaxBorderX = MAX (fMaxBorderX, icon->fX + icon->fWidth * icon->fScale);
	}
	fMaxDockWidth = fMaxBorderX - cairo_dock_get_first_icon (pIconList)->fXMin + 2 * g_iDockRadius + g_iDockLineWidth;
	
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fXMin += fMaxDockWidth / 2;
		icon->fXMax += fMaxDockWidth / 2;
	}
	
	//\_______________ On recalcule les icones avec une position situee en dehors du dock pour remettre les icones a plat.
	cairo_dock_calculate_icons_with_position (pIconList, -1, 1, iFlatDockWidth, 0, 0);
	
	return fMaxDockWidth;
}
