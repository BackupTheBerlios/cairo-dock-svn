/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-draw.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-config.h"
#include "cairo-dock-keyfile-utilities.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-separator-factory.h"
#include "cairo-dock-file-manager.h"
#include "cairo-dock-log.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-class-manager.h"
#include "cairo-dock-icons.h"


extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern gboolean g_bReserveSpace;
extern int g_iLeaveSubDockDelay;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern gint g_iFrameMargin;
extern int g_iIconGap;
extern double g_fSubDockSizeRatio;
extern int g_iMaxAuthorizedWidth;

extern gchar *g_cCurrentLaunchersPath;

extern double g_fAmplitude;
extern int g_iSinusoidWidth;

extern double g_fUnfoldAcceleration;

extern gboolean g_bUniquePid;

extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];
extern gchar *g_cConfFile;


void cairo_dock_free_icon (Icon *icon)
{
	if (icon == NULL)
		return ;
	cd_message ("%s (%s , %s)", __func__, icon->acName, icon->cClass);

	cairo_dock_remove_dialog_if_any (icon);
	if (CAIRO_DOCK_IS_NORMAL_APPLI (icon))
		cairo_dock_unregister_appli (icon);
	else if (icon->cClass != NULL)  // c'est un inhibiteur.
		cairo_dock_deinhibate_class (icon->cClass, icon);
	cairo_dock_deactivate_module (icon->pModule);
	
	cairo_dock_free_icon_buffers (icon);
	
	g_free (icon);
}

void cairo_dock_free_icon_buffers (Icon *icon)
{
	if (icon == NULL)
		return ;
	
	g_free (icon->acDesktopFileName);
	g_free (icon->acFileName);
	g_free (icon->acName);
	g_free (icon->acCommand);
	g_free (icon->cWorkingDirectory);
	g_free (icon->cBaseURI);
	g_free (icon->cParentDockName);  // on ne liberera pas le sous-dock ici sous peine de se mordre la queue, donc il faut l'avoir fait avant.
	g_free (icon->cClass);
	g_free (icon->cQuickInfo);

	cairo_surface_destroy (icon->pIconBuffer);
	cairo_surface_destroy (icon->pReflectionBuffer);
	cairo_surface_destroy (icon->pFullIconBuffer);
	cairo_surface_destroy (icon->pTextBuffer);
	cairo_surface_destroy (icon->pQuickInfoBuffer);
	
	GLuint pTextures[5] = {icon->iIconTexture, icon->iReflectionTexture, icon->iFullIconTexture, icon->iLabelTexture, icon->iQuickInfoTexture};
	glDeleteTextures (5, pTextures);
}

CairoDockIconType cairo_dock_get_icon_type (Icon *icon)
{
	if (CAIRO_DOCK_IS_APPLI (icon))
		return CAIRO_DOCK_APPLI;
	else if (CAIRO_DOCK_IS_APPLET (icon))
		return CAIRO_DOCK_APPLET;
	else if (CAIRO_DOCK_IS_SEPARATOR (icon))
		return CAIRO_DOCK_SEPARATOR12;
	else
		return CAIRO_DOCK_LAUNCHER;
}


int cairo_dock_compare_icons_order (Icon *icon1, Icon *icon2)
{
	int iOrder1 = cairo_dock_get_group_order (icon1);
	int iOrder2 = cairo_dock_get_group_order (icon2);
	if (iOrder1 < iOrder2)
		return -1;
	else if (iOrder1 > iOrder2)
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
static int cairo_dock_compare_icons_name (Icon *icon1, Icon *icon2)
{
	if (icon1->acName == NULL)
		return -1;
	if (icon2->acName == NULL)
		return 1;
	gchar *cURI_1 = g_ascii_strdown (icon1->acName, -1);
	gchar *cURI_2 = g_ascii_strdown (icon2->acName, -1);
	int iOrder = strcmp (cURI_1, cURI_2);
	g_free (cURI_1);
	g_free (cURI_2);
	return iOrder;
}

GList *cairo_dock_sort_icons_by_order (GList *pIconList)
{
	return g_list_sort (pIconList, (GCompareFunc) cairo_dock_compare_icons_order);
}

GList *cairo_dock_sort_icons_by_name (GList *pIconList)
{
	GList *pSortedIconList = g_list_sort (pIconList, (GCompareFunc) cairo_dock_compare_icons_name);
	int iOrder = 0;
	Icon *icon;
	GList *ic;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fOrder = iOrder ++;
	}
	return pSortedIconList;
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
		if (icon->acCommand != NULL && strncmp (icon->acCommand, cCommand, MIN (strlen (icon->acCommand), strlen (cCommand))) == 0)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_icon_with_base_uri (GList *pIconList, const gchar *cBaseURI)
{
	g_return_val_if_fail (cBaseURI != NULL, NULL);
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		//cd_message ("  icon->cBaseURI : %s\n", icon->cBaseURI);
		if (icon->cBaseURI != NULL && strcmp (icon->cBaseURI, cBaseURI) == 0)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_icon_with_name (GList *pIconList, const gchar *cName)
{
	g_return_val_if_fail (cName != NULL, NULL);
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		//cd_message ("  icon->acName : %s\n", icon->acName);
		if (icon->acName != NULL && strcmp (icon->acName, cName) == 0)
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

Icon *cairo_dock_get_icon_with_module (GList *pIconList, CairoDockModule *pModule)
{
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->pModule == pModule)
			return icon;
	}
	return NULL;
}

Icon *cairo_dock_get_icon_with_class (GList *pIconList, gchar *cClass)
{
	g_return_val_if_fail (cClass != NULL, NULL);
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (CAIRO_DOCK_IS_APPLI (icon) && icon->cClass != NULL && strcmp (icon->cClass, cClass) == 0)
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
	if (CAIRO_DOCK_IS_LAUNCHER (icon1))  // ce sont des lanceurs.
	{
		GError *erreur = NULL;
		gchar *cDesktopFilePath;
		GKeyFile* pKeyFile;

		if (icon1->acDesktopFileName != NULL)
		{
			cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon1->acDesktopFileName);
			pKeyFile = g_key_file_new();
			g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
			if (erreur != NULL)
			{
				cd_warning ("Attention : %s", erreur->message);
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
			cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon2->acDesktopFileName);
			pKeyFile = g_key_file_new();
			g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
			if (erreur != NULL)
			{
				cd_warning ("Attention : %s", erreur->message);
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
	cairo_dock_update_dock_size (pDock);

	//\_________________ On met a jour l'ordre des applets dans le fichier de conf.
	if (CAIRO_DOCK_IS_APPLET (icon1))
		cairo_dock_update_module_order (icon1->pModule, icon1->fOrder);
		///cairo_dock_update_conf_file_with_active_modules (NULL, g_cConfFile, pDock->icons);
	if (CAIRO_DOCK_IS_APPLET (icon2))
		cairo_dock_update_module_order (icon2->pModule, icon2->fOrder);
}

void cairo_dock_move_icon_after_icon (CairoDock *pDock, Icon *icon1, Icon *icon2)
{
	//g_print ("%s (%s, %.2f)\n", __func__, icon1->acName, icon1->fOrder);
	///if ((icon2 != NULL) && (! ( (CAIRO_DOCK_IS_APPLI (icon1) && CAIRO_DOCK_IS_APPLI (icon2)) || (CAIRO_DOCK_IS_LAUNCHER (icon1) && CAIRO_DOCK_IS_LAUNCHER (icon2)) || (CAIRO_DOCK_IS_APPLET (icon1) && CAIRO_DOCK_IS_APPLET (icon2)) ) ))
	if ((icon2 != NULL) && fabs (icon1->iType - icon2->iType) > 1)
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
	if ((CAIRO_DOCK_IS_LAUNCHER (icon1) || CAIRO_DOCK_IS_SEPARATOR (icon1)) && icon1->acDesktopFileName != NULL)
	{
		GError *erreur = NULL;
		gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon1->acDesktopFileName);
		GKeyFile* pKeyFile = g_key_file_new();
		g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		else
		{
			g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", icon1->fOrder);
			cairo_dock_write_keys_to_file (pKeyFile, cDesktopFilePath);
			g_key_file_free (pKeyFile);
			g_free (cDesktopFilePath);
		}
	}

	//\_________________ On change sa place dans la liste.
	pDock->pFirstDrawnElement = NULL;
	pDock->icons = g_list_remove (pDock->icons, icon1);
	pDock->icons = g_list_insert_sorted (pDock->icons,
		icon1,
		(GCompareFunc) cairo_dock_compare_icons_order);

	//\_________________ On recalcule la largeur max, qui peut avoir ete influencee par le changement d'ordre.
	cairo_dock_update_dock_size (pDock);

	//\_________________ On prend en compte le changement de position pour les applets.
	if (CAIRO_DOCK_IS_APPLET (icon1))
		cairo_dock_update_module_order (icon1->pModule, icon1->fOrder);
		///cairo_dock_update_conf_file_with_active_modules (NULL, g_cConfFile, pDock->icons);
}



gboolean cairo_dock_detach_icon_from_dock (Icon *icon, CairoDock *pDock, gboolean bCheckUnusedSeparator)
{
	if (g_list_find (pDock->icons, icon) == NULL)  // elle est deja detachee.
		return FALSE;

	cd_message ("%s (%s)", __func__, icon->acName);
	g_free (icon->cParentDockName);
	icon->cParentDockName = NULL;

	icon->iCount = 0;

	//\___________________ On gere le cas des classes d'applis.
	if (CAIRO_DOCK_IS_NORMAL_APPLI (icon))
	{
		CairoDock *pClassSubDock = icon->pSubDock;
		if (pClassSubDock != NULL)  // cette icone pointe sur le sous-dock de sa classe, il faut enlever la 1ere icone de ce sous-dock, la deplacer au dock parent, et lui affecter le sous-dock si il est non vide, ou sinon le detruire.
		{
			Icon *pSameClassIcon = cairo_dock_get_first_icon (pClassSubDock->icons);
			if (pSameClassIcon != NULL)  // a priori toujours vrai.
			{
				icon->pSubDock = NULL;  // on detache le sous-dock de l'icone, il sera detruit ou rattache.

				cairo_dock_detach_icon_from_dock (pSameClassIcon, pClassSubDock, FALSE);  // inutile de verifier si un separateur est present.

				pSameClassIcon->fOrder = icon->fOrder;
				g_free (pSameClassIcon->cParentDockName);
				pSameClassIcon->cParentDockName = g_strdup (cairo_dock_search_dock_name (pDock));
				cairo_dock_insert_icon_in_dock (pSameClassIcon, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, ! CAIRO_DOCK_INSERT_SEPARATOR);

				if (pClassSubDock->icons != NULL)
				{
					cd_message ("  on re-attribue le sous-dock de la classe a l'icone deplacee");
					pSameClassIcon->pSubDock = pClassSubDock;
				}
				else
				{
					cd_message ("  plus d'icone de cette classe");
					const gchar *cClassSubDockName = cairo_dock_search_dock_name (pClassSubDock);  // on aurait pu utiliser l'ancien 'cParentDockName' de pSameClassIcon mais bon ...
					cairo_dock_destroy_dock (pClassSubDock, cClassSubDockName, NULL, NULL);
					icon->pSubDock = NULL;
					pClassSubDock = NULL;
				}
			}
		}

		if (pDock == cairo_dock_search_dock_from_name (icon->cClass) && g_list_length (pDock->icons) == 1)  // il n'y aura plus aucune icone de cette classe.
		{
			cd_message ("  le sous-dock de la classe %s n'aura plus d'element", icon->cClass);
			/**Icon *pPointingIcon = cairo_dock_search_icon_pointing_on_dock (pDock, NULL);
			if (pPointingIcon != NULL)
				pPointingIcon->pSubDock = NULL;*/
		}
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
	pDock->fFlatDockWidth -= icon->fWidth + g_iIconGap;

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

	//\___________________ On la remet a la taille normale en vue d'une reinsertion quelque part.
	///if (pDock->iRefCount > 0)
	{
		icon->fWidth /= pDock->fRatio;  /// g_fSubDockSizeRatio
		icon->fHeight /= pDock->fRatio;
	}
	//g_print (" -size <- %.2fx%.2f\n", icon->fWidth, icon->fHeight);

	//\___________________ On enleve le separateur si c'est la derniere icone de son type.
	if (bCheckUnusedSeparator)
	{
		Icon * pSeparatorIcon = NULL;
		if (! CAIRO_DOCK_IS_AUTOMATIC_SEPARATOR (icon))
		{
			Icon *pSameTypeIcon = cairo_dock_get_first_icon_of_type (pDock->icons, icon->iType);
			if (pSameTypeIcon == NULL)
			{
				int iOrder = cairo_dock_get_group_order (icon);
				if (iOrder > 1)  // attention : iType - 1 > 0 si iType = 0, car c'est un unsigned int !
					pSeparatorIcon = cairo_dock_get_first_icon_of_type (pDock->icons, iOrder - 1);
				else if (iOrder + 1 < CAIRO_DOCK_NB_TYPES)
					pSeparatorIcon = cairo_dock_get_first_icon_of_type (pDock->icons, iOrder + 1);

				if (pSeparatorIcon != NULL)
				{
					//g_print ("  on enleve un separateur\n");
					cairo_dock_detach_icon_from_dock (pSeparatorIcon, pDock, FALSE);
					cairo_dock_free_icon (pSeparatorIcon);
				}
			}
		}
	}
	return TRUE;
}
static void _cairo_dock_remove_one_icon_from_dock (CairoDock *pDock, Icon *icon, gboolean bCheckUnusedSeparator)
{
	g_return_if_fail (icon != NULL);
	//\___________________ On effectue les taches de fermeture de l'icone suivant son type.
	if (icon->acDesktopFileName != NULL)
	{
		gchar *icon_path = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon->acDesktopFileName);
		g_remove (icon_path);
		g_free (icon_path);

		if (CAIRO_DOCK_IS_URI_LAUNCHER (icon))
		{
			cairo_dock_fm_remove_monitor (icon);
		}
	}
	if (CAIRO_DOCK_IS_APPLET (icon))
	{
		cairo_dock_deactivate_module (icon->pModule);  // desactive le module mais ne le ferme pas.
		icon->pModule = NULL;  // pour ne pas le liberer lors du free_icon.
	}  // rien a faire pour les separateurs automatiques.

	//\___________________ On detache l'icone du dock.
	cairo_dock_detach_icon_from_dock (icon, pDock, bCheckUnusedSeparator);
	
	if (CAIRO_DOCK_IS_NORMAL_APPLI (icon))
	{
		cairo_dock_unregister_appli (icon);
	}
	
	if (pDock->bIsMainDock && g_bReserveSpace)
		cairo_dock_reserve_space_for_dock (pDock, TRUE);  // l'espace est reserve sur la taille min, qui a deja ete mise a jour.
}

void cairo_dock_remove_one_icon_from_dock (CairoDock *pDock, Icon *icon)
{
	_cairo_dock_remove_one_icon_from_dock (pDock, icon, FALSE);
}
void cairo_dock_remove_icon_from_dock (CairoDock *pDock, Icon *icon)
{
	_cairo_dock_remove_one_icon_from_dock (pDock, icon, TRUE);
}

static void _cairo_dock_remove_one_icon (Icon *icon, gpointer data)
{
	CairoDock *pDock = data;
	if (CAIRO_DOCK_IS_APPLI (icon) && icon->pSubDock != NULL && cairo_dock_search_dock_from_name (icon->cClass) == icon->pSubDock)
		cairo_dock_destroy_dock (icon->pSubDock, icon->cClass, NULL, NULL);
	cairo_dock_remove_one_icon_from_dock (pDock, icon);
	cairo_dock_free_icon (icon);
}
void cairo_dock_remove_icons_of_type (CairoDock *pDock, CairoDockIconType iType)
{
	//g_print ("%s (%d)\n", __func__, iType);
	/*Icon *icon;
	GList *ic;
	if (pDock->icons == NULL)
		return ;

	gboolean bOneIconFound = FALSE;
	Icon *pSeparatorIcon = NULL;
	ic = pDock->icons;
	do
	{
		if (ic->next == NULL)
			break;

		icon = ic->next->data;  // on ne peut pas enlever l'element courant, sinon on perd 'ic'.
		if (icon->iType == iType)
		{
			bOneIconFound = TRUE;
			if (CAIRO_DOCK_IS_APPLI (icon) && icon->pSubDock != NULL && cairo_dock_search_dock_from_name (icon->cClass) == icon->pSubDock)
				cairo_dock_destroy_dock (icon->pSubDock, icon->cClass, NULL, NULL);
			cairo_dock_remove_one_icon_from_dock (pDock, icon);
			cairo_dock_free_icon (icon);
		}
		else
		{
			if (CAIRO_DOCK_IS_AUTOMATIC_SEPARATOR (icon))
			{
				if ( (bOneIconFound && pSeparatorIcon == NULL) || (! bOneIconFound) )
					pSeparatorIcon = icon;
			}
			ic = ic->next;
		}
	} while (TRUE);  // on parcourt tout, inutile de complexifier la chose pour gagner 3ns.

	icon = cairo_dock_get_first_icon_of_type (pDock->icons, iType);
	if (icon != NULL && icon->iType == iType)
	{
		bOneIconFound = TRUE;
		if (CAIRO_DOCK_IS_APPLI (icon) && icon->pSubDock != NULL && cairo_dock_search_dock_from_name (icon->cClass) == icon->pSubDock)
			cairo_dock_destroy_dock (icon->pSubDock, icon->cClass, NULL, NULL);
		cairo_dock_remove_one_icon_from_dock (pDock, icon);
		cairo_dock_free_icon (icon);
	}

	if (bOneIconFound && pSeparatorIcon != NULL)
	{
		//g_print ("  on enleve un separateur\n");
		cairo_dock_remove_one_icon_from_dock (pDock, pSeparatorIcon);
		cairo_dock_free_icon (pSeparatorIcon);
	}*/
	Icon *pSeparatorIcon = cairo_dock_foreach_icons_of_type (pDock->icons, iType, _cairo_dock_remove_one_icon, pDock);
	if (pSeparatorIcon != NULL)
	{
		//g_print ("  on enleve un separateur\n");
		cairo_dock_remove_one_icon_from_dock (pDock, pSeparatorIcon);
		cairo_dock_free_icon (pSeparatorIcon);
	}
}

Icon *cairo_dock_foreach_icons_of_type (GList *pIconList, CairoDockIconType iType, CairoDockForeachIconFunc pFuntion, gpointer data)
{
	//g_print ("%s (%d)\n", __func__, iType);
	if (pIconList == NULL)
		return NULL;

	Icon *icon;
	GList *ic;
	gboolean bOneIconFound = FALSE;
	Icon *pSeparatorIcon = NULL;
	ic = pIconList;
	do
	{
		if (ic->next == NULL)
			break;

		icon = ic->next->data;  // on ne peut pas enlever l'element courant, sinon on perd 'ic'.
		if (icon->iType == iType)
		{
			bOneIconFound = TRUE;
			pFuntion (icon, data);
		}
		else
		{
			if (CAIRO_DOCK_IS_AUTOMATIC_SEPARATOR (icon))
			{
				if ( (bOneIconFound && pSeparatorIcon == NULL) || (! bOneIconFound) )
					pSeparatorIcon = icon;
			}
			ic = ic->next;
		}
	} while (TRUE);  // on parcourt tout, inutile de complexifier la chose pour gagner 3ns.

	icon = cairo_dock_get_first_icon_of_type (pIconList, iType);
	if (icon != NULL && icon->iType == iType)
	{
		bOneIconFound = TRUE;
		pFuntion (icon, data);
	}

	if (bOneIconFound)
		return pSeparatorIcon;
	else
		return NULL;
}




void cairo_dock_remove_all_separators (CairoDock *pDock)
{
	Icon *icon;
	GList *ic;
	if (pDock->icons == NULL)
		return ;

	ic = pDock->icons;
	do
	{
		if (ic->next == NULL)
			break;

		icon = ic->next->data;  // on ne peut pas enlever l'element courant, sinon on perd 'ic'.
		if (CAIRO_DOCK_IS_AUTOMATIC_SEPARATOR (icon))
		{
			cairo_dock_remove_one_icon_from_dock (pDock, icon);
			cairo_dock_free_icon (icon);
		}
		else
		{
			ic = ic->next;
		}
	} while (TRUE);
}


void cairo_dock_insert_separators_in_dock (CairoDock *pDock)
{
	Icon *icon, *next_icon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (! CAIRO_DOCK_IS_AUTOMATIC_SEPARATOR (icon))
		{
			if (ic->next != NULL)
			{
				next_icon = ic->next->data;
				if (! CAIRO_DOCK_IS_AUTOMATIC_SEPARATOR (icon) && icon->iType != next_icon->iType)
				{
					int iSeparatorType = g_tIconTypeOrder[next_icon->iType] - 1;
					cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
					Icon *pSeparator = cairo_dock_create_separator_icon (pCairoContext, iSeparatorType, pDock, ! CAIRO_DOCK_APPLY_RATIO);
					cairo_destroy (pCairoContext);

					cairo_dock_insert_icon_in_dock (pSeparator, pDock, !CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, ! CAIRO_DOCK_INSERT_SEPARATOR);
				}
			}
		}
	}
}


void cairo_dock_insert_separator_between_launchers_and_applis (CairoDock *pDock)
{
	Icon *pFirstLauncher = cairo_dock_get_first_icon_of_type (pDock->icons, CAIRO_DOCK_LAUNCHER);
	Icon *pFirstAppli = cairo_dock_get_first_icon_of_type (pDock->icons, CAIRO_DOCK_APPLI);

	if (pFirstLauncher != NULL && pFirstAppli != NULL)
	{
		int iSeparatorType = MIN (g_tIconTypeOrder[CAIRO_DOCK_LAUNCHER], g_tIconTypeOrder[CAIRO_DOCK_APPLI]) + 1;
		if (cairo_dock_get_first_icon_of_type (pDock->icons, iSeparatorType) == NULL)
		{
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
			Icon *pSeparator = cairo_dock_create_separator_icon (pCairoContext, iSeparatorType, pDock, ! CAIRO_DOCK_APPLY_RATIO);
			cairo_destroy (pCairoContext);

			cairo_dock_insert_icon_in_dock (pSeparator, pDock, !CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, ! CAIRO_DOCK_INSERT_SEPARATOR);
		}
	}
}


GList *cairo_dock_calculate_icons_positions_at_rest_linear (GList *pIconList, double fFlatDockWidth, int iXOffset)
{
	//g_print ("%s (%d, +%d)\n", __func__, fFlatDockWidth, iXOffset);
	double x_cumulated = iXOffset;
	double fXMin = 99999;
	GList* ic, *pFirstDrawnElement = NULL;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;

		if (x_cumulated + icon->fWidth / 2 < 0)
			icon->fXAtRest = x_cumulated + fFlatDockWidth;
		else if (x_cumulated + icon->fWidth / 2 > fFlatDockWidth)
			icon->fXAtRest = x_cumulated - fFlatDockWidth;
		else
			icon->fXAtRest = x_cumulated;

		if (icon->fXAtRest < fXMin)
		{
			fXMin = icon->fXAtRest;
			pFirstDrawnElement = ic;
		}
		//g_print ("%s : fXAtRest = %.2f\n", icon->acName, icon->fXAtRest);

		x_cumulated += icon->fWidth + g_iIconGap;
	}

	return pFirstDrawnElement;
}

void cairo_dock_update_removing_inserting_icon (Icon *icon)
{
	if (icon->fPersonnalScale > 0)
	{
		icon->fPersonnalScale *= .85;
		if (icon->fPersonnalScale < 0.05)
			icon->fPersonnalScale = 0.05;
	}
	else if (icon->fPersonnalScale < 0)
	{
		icon->fPersonnalScale *= .85;
		if (icon->fPersonnalScale > -0.05)
			icon->fPersonnalScale = -0.05;
	}
}

Icon * cairo_dock_calculate_wave_with_position_linear (GList *pIconList, GList *pFirstDrawnElementGiven, int x_abs, gdouble fMagnitude, double fFlatDockWidth, int iWidth, int iHeight, double fAlign, double fFoldingFactor, gboolean bDirectionUp)
{
	//g_print (">>>>>%s (%d/%.2f, %dx%d, %.2f, %.2f)\n", __func__, x_abs, fFlatDockWidth, iWidth, iHeight, fAlign, fFoldingFactor);
	if (x_abs < 0 && iWidth > 0)  // ces cas limite sont la pour empecher les icones de retrecir trop rapidement quand on sort par les cotes.
		x_abs = -1;
	else if (x_abs > fFlatDockWidth && iWidth > 0)
		x_abs = fFlatDockWidth+1;
	if (pIconList == NULL)
		return NULL;

	float x_cumulated = 0, fXMiddle, fDeltaExtremum;
	GList* ic, *pointed_ic;
	Icon *icon, *prev_icon;

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
		{
			icon->fPhase = 0;
		}
		else if (icon->fPhase > G_PI)
		{
			icon->fPhase = G_PI;
		}
		
		//\_______________ On en deduit l'amplitude de la sinusoide au niveau de cette icone, et donc son echelle.
		icon->fScale = 1 + fMagnitude * g_fAmplitude * sin (icon->fPhase);
		if (iWidth > 0 && icon->fPersonnalScale != 0)
		{
			if (icon->fPersonnalScale > 0)
				icon->fScale *= icon->fPersonnalScale;
			else
				icon->fScale *= (1 + icon->fPersonnalScale);
		}
// 		if (icon->fPersonnalScale > 0 && iWidth > 0)
// 		{
// 			icon->fPersonnalScale *= .85;
// 			icon->fScale *= icon->fPersonnalScale;
// 			if (icon->fPersonnalScale < 0.05)
// 				icon->fPersonnalScale = 0.05;
// 		}
// 		else if (icon->fPersonnalScale < 0 && iWidth > 0)
// 		{
// 			icon->fPersonnalScale *= .85;
// 			icon->fScale *= (1 + icon->fPersonnalScale);
// 			if (icon->fPersonnalScale > -0.05)
// 				icon->fPersonnalScale = -0.05;
// 		}
		icon->fY = (bDirectionUp ? iHeight - g_iDockLineWidth - g_iFrameMargin - icon->fScale * icon->fHeight : g_iDockLineWidth + g_iFrameMargin);
		
		//\_______________ Si on avait deja defini l'icone pointee, on peut placer l'icone courante par rapport a la precedente.
		if (pointed_ic != NULL)
		{
			if (ic == pFirstDrawnElement)  // peut arriver si on est en dehors a gauche du dock.
			{
				icon->fX = x_cumulated - 1. * (fFlatDockWidth - iWidth) / 2;
				//g_print ("  en dehors a gauche : icon->fX = %.2f (%.2f)\n", icon->fX, x_cumulated);
			}
			else
			{
				prev_icon = (ic->prev != NULL ? ic->prev->data : cairo_dock_get_last_icon (pIconList));
				icon->fX = prev_icon->fX + (prev_icon->fWidth + g_iIconGap) * prev_icon->fScale;

				if (icon->fX + icon->fWidth * icon->fScale > icon->fXMax - g_fAmplitude * fMagnitude * (icon->fWidth + 1.5*g_iIconGap) / 8 && iWidth != 0)
				{
					//g_print ("  on contraint %s (fXMax=%.2f , fX=%.2f\n", prev_icon->acName, prev_icon->fXMax, prev_icon->fX);
					fDeltaExtremum = icon->fX + icon->fWidth * icon->fScale - (icon->fXMax - g_fAmplitude * fMagnitude * (icon->fWidth + 1.5*g_iIconGap) / 16);
					icon->fX -= fDeltaExtremum * (1 - (icon->fScale - 1) / g_fAmplitude) * fMagnitude;
				}
			}
			icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1. - fFoldingFactor);
			//g_print ("  a droite : icon->fX = %.2f (%.2f)\n", icon->fX, x_cumulated);
		}
		
		//\_______________ On regarde si on pointe sur cette icone.
		if (x_cumulated + icon->fWidth + .5*g_iIconGap >= x_abs && x_cumulated - .5*g_iIconGap <= x_abs && pointed_ic == NULL)  // on a trouve l'icone sur laquelle on pointe.
		{
			pointed_ic = ic;
			icon->bPointed = TRUE;
			icon->fX = x_cumulated - (fFlatDockWidth - iWidth) / 2 + (1 - icon->fScale) * (x_abs - x_cumulated + .5*g_iIconGap);
			icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1. - fFoldingFactor);
			//g_print ("  icone pointee : fX = %.2f (%.2f)\n", icon->fX, x_cumulated);
		}
		else
			icon->bPointed = FALSE;
			
		ic = cairo_dock_get_next_element (ic, pIconList);
	} while (ic != pFirstDrawnElement);
	
	//\_______________ On place les icones precedant l'icone pointee par rapport a celle-ci.
	if (pointed_ic == NULL)  // on est a droite des icones.
	{
		pointed_ic = (pFirstDrawnElement->prev == NULL ? g_list_last (pIconList) : pFirstDrawnElement->prev);
		icon = pointed_ic->data;
		icon->fX = x_cumulated - (fFlatDockWidth - iWidth) / 2 + (1 - icon->fScale) * (icon->fWidth + .5*g_iIconGap);
		icon->fX = fAlign * iWidth + (icon->fX - fAlign * iWidth) * (1 - fFoldingFactor);
		//g_print ("  en dehors a droite : icon->fX = %.2f (%.2f)\n", icon->fX, x_cumulated);
	}
	
	ic = pointed_ic;
	while (ic != pFirstDrawnElement)
	{
		icon = ic->data;
		
		ic = ic->prev;
		if (ic == NULL)
			ic = g_list_last (pIconList);
			
		prev_icon = ic->data;
		
		prev_icon->fX = icon->fX - (prev_icon->fWidth + g_iIconGap) * prev_icon->fScale;
		//g_print ("fX <- %.2f; fXMin : %.2f\n", prev_icon->fX, prev_icon->fXMin);
		if (prev_icon->fX < prev_icon->fXMin + g_fAmplitude * fMagnitude * (prev_icon->fWidth + 1.5*g_iIconGap) / 8 && iWidth != 0 && x_abs < iWidth && fMagnitude > 0)  /// && prev_icon->fPhase == 0   // on rajoute 'fMagnitude > 0' sinon il y'a un leger "saut" du aux contraintes a gauche de l'icone pointee.
		{
			//g_print ("  on contraint %s (fXMin=%.2f , fX=%.2f\n", prev_icon->acName, prev_icon->fXMin, prev_icon->fX);
			fDeltaExtremum = prev_icon->fX - (prev_icon->fXMin + g_fAmplitude * fMagnitude * (prev_icon->fWidth + 1.5*g_iIconGap) / 16);
			prev_icon->fX -= fDeltaExtremum * (1 - (prev_icon->fScale - 1) / g_fAmplitude) * fMagnitude;
		}
		prev_icon->fX = fAlign * iWidth + (prev_icon->fX - fAlign * iWidth) * (1. - fFoldingFactor);
		//g_print ("  prev_icon->fX : %.2f\n", prev_icon->fX);
	}

	return pointed_ic->data;
}

Icon *cairo_dock_apply_wave_effect (CairoDock *pDock)
{
	//\_______________ On calcule la position du curseur dans le referentiel du dock a plat.
	int dx = pDock->iMouseX - pDock->iCurrentWidth / 2;  // ecart par rapport au milieu du dock a plat.
	int x_abs = dx + pDock->fFlatDockWidth / 2;  // ecart par rapport a la gauche du dock minimal  plat.

	//\_______________ On calcule l'ensemble des parametres des icones.
	double fMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex) * pDock->fMagnitudeMax;
	Icon *pPointedIcon = cairo_dock_calculate_wave_with_position_linear (pDock->icons, pDock->pFirstDrawnElement, x_abs, fMagnitude, pDock->fFlatDockWidth, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->fAlign, pDock->fFoldingFactor, pDock->bDirectionUp);  // iMaxDockWidth
	return pPointedIcon;
}

CairoDockMousePositionType cairo_dock_check_if_mouse_inside_linear (CairoDock *pDock)
{
	CairoDockMousePositionType iMousePositionType;
	int iWidth = pDock->iCurrentWidth;
	int iHeight = pDock->iCurrentHeight;
	int iMouseX = pDock->iMouseX;
	int iMouseY = pDock->iMouseY;
	//g_print ("%s (%dx%d, %dx%d, %f)\n", __func__, iMouseX, iMouseY, iWidth, iHeight, pDock->fFoldingFactor);

	//\_______________ On regarde si le curseur est dans le dock ou pas, et on joue sur la taille des icones en consequence.
	int x_abs = pDock->iMouseX + (pDock->fFlatDockWidth - iWidth) / 2;  // abscisse par rapport a la gauche du dock minimal plat.
	gboolean bMouseInsideDock = (/**pDock->bInside && */x_abs >= 0 && x_abs <= pDock->fFlatDockWidth && iMouseX > 0 && iMouseX < iWidth);
	//g_print ("bMouseInsideDock : %d (%d;%d/%.2f)\n", bMouseInsideDock, pDock->bInside, x_abs, pDock->fFlatDockWidth);

	if (! bMouseInsideDock)
	{
		double fSideMargin = fabs (pDock->fAlign - .5) * (iWidth - pDock->fFlatDockWidth);
		if (x_abs < - fSideMargin || x_abs > pDock->fFlatDockWidth + fSideMargin)
			iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
		else
			iMousePositionType = CAIRO_DOCK_MOUSE_ON_THE_EDGE;
	}
	else if (iMouseY >= 0 && iMouseY <= iHeight)  // et en plus on est dedans en y.  //  && pPointedIcon != NULL
	{
		//g_print ("on est dedans en x et en y et la taille des icones est non maximale bien qu'aucune icone ne soit animee (iMouseX=%d => x_abs=%d)\n", iMouseX, x_abs);
		//pDock->bInside = TRUE;
		iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
	}
	else
		iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;

	return iMousePositionType;
}

void cairo_dock_manage_mouse_position (CairoDock *pDock, CairoDockMousePositionType iMousePositionType)
{
	static gboolean bReturn = FALSE;
	switch (iMousePositionType)
	{
		case CAIRO_DOCK_MOUSE_INSIDE :
			//g_print ("INSIDE\n");
			if (cairo_dock_entrance_is_allowed (pDock) && pDock->iMagnitudeIndex < CAIRO_DOCK_NB_MAX_ITERATIONS && pDock->iSidGrowUp == 0 && cairo_dock_none_animated (pDock->icons))  // on est dedans et la taille des icones est non maximale bien qu'aucune icone ne soit animee.  ///  && pDock->iSidMoveDown == 0
			{
				cd_debug ("on est dedans en x et en y et la taille des icones est non maximale bien qu'aucune icone  ne soit animee");
				//pDock->bInside = TRUE;
				if ((pDock->bAtBottom && pDock->iRefCount == 0 && ! pDock->bAutoHide) || (pDock->iCurrentWidth != pDock->iMaxDockWidth || pDock->iCurrentHeight != pDock->iMaxDockHeight))  // on le fait pas avec l'auto-hide, car un signal d'entree est deja emis a cause des mouvements/redimensionnements de la fenetre, et en rajouter un ici fout le boxon.
				{
					cd_debug ("  on emule une re-rentree (pDock->iMagnitudeIndex:%f)", pDock->iMagnitudeIndex);
					///cairo_dock_render_blank (pDock);  // utile ?
					g_signal_emit_by_name (pDock->pWidget, "enter-notify-event", NULL, &bReturn);
				}
				else  // on se contente de faire grossir les icones.
				{
					cd_debug ("  on se contente de faire grossir les icones");
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
					if (pDock->bAutoHide && pDock->iRefCount == 0 && pDock->iSidMoveUp == 0)
						pDock->iSidMoveUp = g_timeout_add (40, (GSourceFunc) cairo_dock_move_up, pDock);
				}
			}
		break ;

		case CAIRO_DOCK_MOUSE_ON_THE_EDGE :
			///pDock->fDecorationsOffsetX = - pDock->iCurrentWidth / 2;  // on fixe les decorations.
			if (pDock->iSidGrowUp == 0 && pDock->iSidShrinkDown == 0 && pDock->iMagnitudeIndex > 0)  // pDock->iMagnitudeIndex == CAIRO_DOCK_NB_MAX_ITERATIONS
			{
				//g_print ("on dezoome le dock\n");
				pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, pDock);
			}
		break ;

		case CAIRO_DOCK_MOUSE_OUTSIDE :
			//g_print ("en dehors du dock (%d;%d;%d;%d)\n", pDock->iSidGrowUp, pDock->iSidShrinkDown, pDock->iMagnitudeIndex, pDock->iSidLeaveDemand);
			///pDock->fDecorationsOffsetX = - pDock->iCurrentWidth / 2;  // on fixe les decorations.
			if (pDock->iSidGrowUp == 0 && pDock->iSidShrinkDown == 0 && pDock->iSidLeaveDemand == 0 && ! pDock->bAtBottom)  // bAtBottom ajoute pour la 1.5.4
			{
				//g_print ("on force a quitter (iRefCount:%d)\n", pDock->iRefCount);
				if (pDock->iRefCount > 0 && g_iLeaveSubDockDelay > 0)
					pDock->iSidLeaveDemand = g_timeout_add (g_iLeaveSubDockDelay, (GSourceFunc) cairo_dock_emit_leave_signal, (gpointer) pDock);
				else
					cairo_dock_emit_leave_signal (pDock);
			}
		break ;
	}
}



double cairo_dock_calculate_max_dock_width (CairoDock *pDock, GList *pFirstDrawnElementGiven, double fFlatDockWidth, double fWidthConstraintFactor, double fExtraWidth)
{
	double fMaxDockWidth = 0.;
	//g_print ("%s (%d)\n", __func__, fFlatDockWidth);
	GList *pIconList = pDock->icons;
	if (pIconList == NULL)
		return 2 * g_iDockRadius + g_iDockLineWidth + 2 * g_iFrameMargin;

	//\_______________ On remet a zero les positions extremales des icones.
	GList* ic;
	Icon *icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fXMax = -1e4;
		icon->fXMin = 1e4;
	}

	//\_______________ On simule le passage du curseur sur toute la largeur du dock, et on chope la largeur maximale qui s'en degage, ainsi que les positions d'equilibre de chaque icone.
	GList *pFirstDrawnElement = (pFirstDrawnElementGiven != NULL ? pFirstDrawnElementGiven : pIconList);
	//for (int iVirtualMouseX = 0; iVirtualMouseX < fFlatDockWidth; iVirtualMouseX ++)
	GList *ic2;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;

		cairo_dock_calculate_wave_with_position_linear (pIconList, pFirstDrawnElement, icon->fXAtRest, pDock->fMagnitudeMax, fFlatDockWidth, 0, 0, 0.5, 0, pDock->bDirectionUp);
		ic2 = pFirstDrawnElement;
		do
		{
			icon = ic2->data;

			if (icon->fX + icon->fWidth * icon->fScale > icon->fXMax)
				icon->fXMax = icon->fX + icon->fWidth * icon->fScale;
			if (icon->fX < icon->fXMin)
				icon->fXMin = icon->fX;

			ic2 = cairo_dock_get_next_element (ic2, pDock->icons);
		} while (ic2 != pFirstDrawnElement);
	}
	cairo_dock_calculate_wave_with_position_linear (pIconList, pFirstDrawnElement, fFlatDockWidth - 1, pDock->fMagnitudeMax, fFlatDockWidth, 0, 0, pDock->fAlign, 0, pDock->bDirectionUp);  // pDock->fFoldingFactor
	ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;

		if (icon->fX + icon->fWidth * icon->fScale > icon->fXMax)
			icon->fXMax = icon->fX + icon->fWidth * icon->fScale;
		if (icon->fX < icon->fXMin)
			icon->fXMin = icon->fX;

		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);

	fMaxDockWidth = (icon->fXMax - ((Icon *) pFirstDrawnElement->data)->fXMin) * fWidthConstraintFactor + fExtraWidth;
	fMaxDockWidth = ceil (fMaxDockWidth) + 1;

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





void cairo_dock_mark_icons_as_avoiding_mouse (CairoDock *pDock, CairoDockIconType iType, double fMargin)
{
	if (pDock->icons == NULL)
		return;
	
	gboolean bCanDrop = FALSE;
	Icon *icon;
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		if (icon->bPointed && icon->iAnimationType != CAIRO_DOCK_FOLLOW_MOUSE)
		{
			icon->iAnimationType = 0;

			if (pDock->iMouseX < icon->fDrawXAtRest + icon->fWidth * icon->fScale * fMargin)  // on est a gauche.
			{
				Icon *prev_icon = cairo_dock_get_previous_element (ic, pDock->icons) -> data;
				if ((icon->iType == iType || prev_icon->iType == iType) && prev_icon->iAnimationType != CAIRO_DOCK_FOLLOW_MOUSE)
				{
					icon->iAnimationType = CAIRO_DOCK_AVOID_MOUSE;
					prev_icon->iAnimationType = CAIRO_DOCK_AVOID_MOUSE;
					//g_print ("%s> <%s\n", prev_icon->acName, icon->acName);
					bCanDrop = TRUE;
				}
			}
			else if (pDock->iMouseX > icon->fDrawXAtRest + icon->fWidth * icon->fScale * (1 - fMargin))  // on est a droite.
			{
				Icon *next_icon = cairo_dock_get_next_element (ic, pDock->icons) -> data;
				if ((icon->iType == iType || next_icon->iType == iType) && next_icon->iAnimationType != CAIRO_DOCK_FOLLOW_MOUSE)
				{
					icon->iAnimationType = CAIRO_DOCK_AVOID_MOUSE;
					next_icon->iAnimationType = CAIRO_DOCK_AVOID_MOUSE;
					//g_print ("%s> <%s\n", icon->acName, next_icon->acName);
					bCanDrop = TRUE;
				}
				ic = cairo_dock_get_next_element (ic, pDock->icons);  // on la saute.
				if (ic == pFirstDrawnElement)
					break ;
			}
			else  // on est dessus.
			{
				//g_print ("on est sur %s\n", icon->acName);
			}
		}
		else if (icon->iAnimationType == CAIRO_DOCK_AVOID_MOUSE)
			icon->iAnimationType = 0;

		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	
	if (bCanDrop)
	{
		if (pDock->bIsDragging && pDock->iSidDropIndicator == 0)
			pDock->iSidDropIndicator = g_timeout_add (40, (GSourceFunc) cairo_dock_display_drop_indicator, pDock);
	}
	else
	{
		if (pDock->iSidDropIndicator != 0)
		{
			g_source_remove (pDock->iSidDropIndicator);
			pDock->iSidDropIndicator = 0;
			pDock->iDropIndicatorOffset = 0;
		}
	}
}
void cairo_dock_mark_avoiding_mouse_icons_linear (CairoDock *pDock)
{
	if (pDock->bInside && pDock->iAvoidingMouseIconType != -1 && pDock->fAvoidingMouseMargin > 0)
	{
		cairo_dock_mark_icons_as_avoiding_mouse (pDock, pDock->iAvoidingMouseIconType, pDock->fAvoidingMouseMargin);
	}
}

void cairo_dock_stop_marking_icons (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return;
	//g_print ("%s (%d)\n", __func__, iType);

	Icon *icon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->iAnimationType > CAIRO_DOCK_NB_ANIMATIONS)
		{
			icon->iAnimationType = 0;
		}
	}
}



void cairo_dock_update_icon_s_container_name (Icon *icon, const gchar *cNewParentDockName)
{
	g_free (icon->cParentDockName);
	icon->cParentDockName = g_strdup (cNewParentDockName);

	if (CAIRO_DOCK_IS_NORMAL_LAUNCHER (icon))  // icon->acDesktopFileName != NULL
	{
		gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, icon->acDesktopFileName);

		GError *erreur = NULL;
		GKeyFile *pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			g_free (cDesktopFilePath);
			return ;
		}

		g_key_file_set_string (pKeyFile, "Desktop Entry", "Container", cNewParentDockName);
		cairo_dock_write_keys_to_file (pKeyFile, cDesktopFilePath);

		g_free (cDesktopFilePath);
		g_key_file_free (pKeyFile);
	}
	else if (CAIRO_DOCK_IS_APPLET (icon))
	{
		GError *erreur = NULL;
		GKeyFile *pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, icon->pModule->cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			return ;
		}
		g_key_file_set_string (pKeyFile, "Icon", "dock name", cNewParentDockName);
		cairo_dock_write_keys_to_file (pKeyFile, icon->pModule->cConfFilePath);
		g_key_file_free (pKeyFile);
	}
}
