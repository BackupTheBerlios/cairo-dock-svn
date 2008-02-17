/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>
#include <gtk/gtk.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-icons.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-log.h"

extern double g_fScrollAcceleration;
extern gboolean g_bResetScrollOnLeave;

extern int g_iScreenHeight[2];

extern gboolean g_bAutoHide;
extern gboolean g_bDirectionUp;

extern int g_iVisibleZoneHeight;

extern double g_fUnfoldAcceleration;
extern int g_iGrowUpInterval;
extern int g_iShrinkDownInterval;
extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS];


gboolean cairo_dock_move_up (CairoDock *pDock)
{
	int deltaY_possible;
	deltaY_possible = pDock->iWindowPositionY - (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - pDock->iMaxDockHeight - pDock->iGapY : pDock->iGapY);
	//g_print ("%s (%dx%d -> %d)\n", __func__, pDock->iWindowPositionX, pDock->iWindowPositionY, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 0) || (! g_bDirectionUp && deltaY_possible < 0))  // alors on peut encore monter.
	{
		pDock->iWindowPositionY -= (int) (deltaY_possible * g_fMoveUpSpeed) + (g_bDirectionUp ? 1 : -1);
		//g_print ("  move to (%dx%d)\n", g_iWindowPositionX, g_iWindowPositionY);
		if (pDock->bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionY, pDock->iWindowPositionX);
		pDock->bAtBottom = FALSE;
		return TRUE;
	}
	else
	{
		pDock->bAtTop = TRUE;
		pDock->iSidMoveUp = 0;
		return FALSE;
	}
}

gboolean cairo_dock_move_down (CairoDock *pDock)
{
	//g_print ("%s ()\n", __func__);
	if (pDock->iMagnitudeIndex > 0 || (g_bResetScrollOnLeave && pDock->iScrollOffset != 0))  // on retarde le cachage du dock pour apercevoir les effets.
		return TRUE;
	int deltaY_possible = (g_bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - pDock->iGapY - 0 : pDock->iGapY + 0 - pDock->iMaxDockHeight) - pDock->iWindowPositionY;  // 0 <-> g_iVisibleZoneHeight
	//g_print ("%s (%d)\n", __func__, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 8) || (! g_bDirectionUp && deltaY_possible < -8))  // alors on peut encore descendre.
	{
		pDock->iWindowPositionY += (int) (deltaY_possible * g_fMoveDownSpeed) + (g_bDirectionUp ? 1 : -1);  // 0.33
		//g_print ("pDock->iWindowPositionY <- %d\n", pDock->iWindowPositionY);
		if (pDock->bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionY, pDock->iWindowPositionX);
		pDock->bAtTop = FALSE;
		return TRUE;
	}
	else  // on se fixe en bas, et on montre la zone visible.
	{
		g_print ("  on se fixe en bas\n");
		pDock->bAtBottom = TRUE;
		pDock->iSidMoveDown = 0;
		int iNewWidth, iNewHeight;
		cairo_dock_get_window_position_and_geometry_at_balance (pDock, CAIRO_DOCK_MIN_SIZE, &iNewWidth, &iNewHeight);
		if (pDock->bHorizontalDock)
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionX,
				pDock->iWindowPositionY,
				iNewWidth,
				iNewHeight);
		else
			gdk_window_move_resize (pDock->pWidget->window,
				pDock->iWindowPositionY,
				pDock->iWindowPositionX,
				iNewHeight,
				iNewWidth);

		if (g_bAutoHide && pDock->iRefCount == 0)
		{
			//g_print ("on arrete les animations\n");
			Icon *pBouncingIcon = cairo_dock_get_bouncing_icon (pDock->icons);
			if (pBouncingIcon != NULL)  // s'il y'a une icone en cours d'animation, on l'arrete.
			{
				pBouncingIcon->iCount = 0;
			}
			Icon *pRemovingIcon = cairo_dock_get_removing_or_inserting_icon (pDock->icons);
			if (pRemovingIcon != NULL)  // idem.
			{
				pRemovingIcon->fPersonnalScale = 0.001;
			}
			pDock->iScrollOffset = 0;

			pDock->calculate_max_dock_size (pDock);
			pDock->fFoldingFactor = g_fUnfoldAcceleration;

			cairo_dock_allow_entrance ();
		}

		gtk_widget_queue_draw (pDock->pWidget);

		return FALSE;
	}
}


gfloat cairo_dock_calculate_magnitude (gint iMagnitudeIndex)  // merci a Robrob pour le patch !
{
	gfloat tmp= ((gfloat)iMagnitudeIndex)/CAIRO_DOCK_NB_MAX_ITERATIONS;

	if (tmp>0.5f)
		tmp=1.0f-(1.0f-tmp)*(1.0f-tmp)*(1.0f-tmp)*4.0f;
	else
		tmp=tmp*tmp*tmp*4.0f;

	if (tmp<0.0f)
		tmp=0.0f;

	if (tmp>1.0f)
		tmp=1.0f;

	return  tmp;
}

gboolean cairo_dock_grow_up (CairoDock *pDock)
{
	//g_print ("%s (%d ; %f ; %d)\n", __func__, pDock->iMagnitudeIndex, pDock->fFoldingFactor, pDock->bInside);
	if (pDock->iSidShrinkDown != 0)
		return TRUE;  // on se met en attente de fin d'animation.

	pDock->iMagnitudeIndex += g_iGrowUpInterval;
	if (pDock->iMagnitudeIndex > CAIRO_DOCK_NB_MAX_ITERATIONS)
		pDock->iMagnitudeIndex = CAIRO_DOCK_NB_MAX_ITERATIONS;

	pDock->fFoldingFactor *= sqrt (pDock->fFoldingFactor);
	if (pDock->fFoldingFactor < 0.03)
		pDock->fFoldingFactor = 0;

	if (pDock->bHorizontalDock)
		gdk_window_get_pointer (pDock->pWidget->window, &pDock->iMouseX, &pDock->iMouseY, NULL);
	else
		gdk_window_get_pointer (pDock->pWidget->window, &pDock->iMouseY, &pDock->iMouseX, NULL);

	pDock->calculate_icons (pDock);
	gtk_widget_queue_draw (pDock->pWidget);

	if (pDock->iMagnitudeIndex == CAIRO_DOCK_NB_MAX_ITERATIONS && pDock->fFoldingFactor == 0)
	{
		pDock->iMagnitudeIndex = CAIRO_DOCK_NB_MAX_ITERATIONS;
		pDock->iSidGrowUp = 0;
		if (pDock->iRefCount == 0 && g_bAutoHide)  // on arrive en fin de l'animation qui montre le dock, les icones sont bien placees a partir de maintenant.
		{
			cairo_dock_set_icons_geometry_for_window_manager (pDock);
			cairo_dock_replace_all_dialogs ();
		}
		return FALSE;
	}
	else
		return TRUE;
}

gboolean cairo_dock_shrink_down (CairoDock *pDock)
{
	//g_print ("%s (%d)\n", __func__, pDock->iMagnitudeIndex);
	pDock->iMagnitudeIndex -= g_iShrinkDownInterval;
	if (pDock->iMagnitudeIndex < 0)
		pDock->iMagnitudeIndex = 0;
	//g_print ("pDock->fFoldingFactor : %f\n", pDock->fFoldingFactor);
	if (pDock->fFoldingFactor != 0 && (! g_bResetScrollOnLeave || pDock->iScrollOffset == 0))
	{
		pDock->fFoldingFactor = pow (pDock->fFoldingFactor, 2./3);
		if (pDock->fFoldingFactor > g_fUnfoldAcceleration)
			pDock->fFoldingFactor = g_fUnfoldAcceleration;
	}
	pDock->fDecorationsOffsetX *= .8;
	//g_print ("fDecorationsOffsetX <- %.2f\n", pDock->fDecorationsOffsetX);

	if (pDock->bHorizontalDock)  // ce n'est pas le motion_notify qui va nous donner des coordonnees en dehors du dock, et donc le fait d'etre dedans va nous faire interrompre le shrink_down et re-grossir, du coup il faut le faire ici. L'inconvenient, c'est que quand on sort par les cotes, il n'y a soudain plus d'icone pointee, et donc le dock devient tout plat subitement au lieu de le faire doucement. Heureusement j'ai trouve une astuce. ^_^
		gdk_window_get_pointer (pDock->pWidget->window, &pDock->iMouseX, &pDock->iMouseY, NULL);
	else
		gdk_window_get_pointer (pDock->pWidget->window, &pDock->iMouseY, &pDock->iMouseX, NULL);

	if (pDock->iScrollOffset != 0 && g_bResetScrollOnLeave)
	{
		//g_print ("iScrollOffset : %d\n", pDock->iScrollOffset);
		if (pDock->iScrollOffset < pDock->fFlatDockWidth / 2)
		{
			//pDock->iScrollOffset = pDock->iScrollOffset * g_fScrollAcceleration;
			pDock->iScrollOffset -= MAX (2, ceil (pDock->iScrollOffset * (1 - g_fScrollAcceleration)));
			if (pDock->iScrollOffset < 0)
				pDock->iScrollOffset = 0;
		}
		else
		{
			pDock->iScrollOffset += MAX (2, ceil ((pDock->fFlatDockWidth - pDock->iScrollOffset) * (1 - g_fScrollAcceleration)));
			if (pDock->iScrollOffset > pDock->fFlatDockWidth)
				pDock->iScrollOffset = 0;
		}
		pDock->calculate_max_dock_size (pDock);
	}

	pDock->calculate_icons (pDock);
	gtk_widget_queue_draw (pDock->pWidget);

	if (! pDock->bInside)
		cairo_dock_replace_all_dialogs ();

	if (pDock->iMagnitudeIndex == 0)
	{
		Icon *pBouncingIcon = cairo_dock_get_bouncing_icon (pDock->icons);
		Icon *pRemovingIcon = cairo_dock_get_removing_or_inserting_icon (pDock->icons);

		if (pBouncingIcon == NULL && pRemovingIcon == NULL && (! g_bResetScrollOnLeave || pDock->iScrollOffset == 0))  // plus aucune animation en cours.
		{
			if (! (g_bAutoHide && pDock->iRefCount == 0) && ! pDock->bInside)
			{
				int iNewWidth, iNewHeight;
				cairo_dock_get_window_position_and_geometry_at_balance (pDock, CAIRO_DOCK_NORMAL_SIZE, &iNewWidth, &iNewHeight);
				if (pDock->bHorizontalDock)
					gdk_window_move_resize (pDock->pWidget->window,
						pDock->iWindowPositionX,
						pDock->iWindowPositionY,
						iNewWidth,
						iNewHeight);
				else
					gdk_window_move_resize (pDock->pWidget->window,
						pDock->iWindowPositionY,
						pDock->iWindowPositionX,
						iNewHeight,
						iNewWidth);
			}

			pDock->calculate_icons (pDock);  // relance le grossissement si on est dedans.
			if (! pDock->bInside && pDock->iRefCount > 0)
			{
				//g_print ("on cache ce sous-dock en sortant par lui\n");
				gtk_widget_hide (pDock->pWidget);
				cairo_dock_hide_parent_dock (pDock);
			}

			pDock->iSidShrinkDown = 0;
			return FALSE;
		}

		//\______________ Au moins une icone est en cours d'animation suite a un clique, on continue le 'shrink_down'.
		if (pRemovingIcon != NULL)
		{
			cd_debug ("au moins 1 icone en cours d'insertion/suppression (%f)", pRemovingIcon->fPersonnalScale);
			if (pRemovingIcon->fPersonnalScale == 0.05)
			{
				cd_debug ("  fin");
				cairo_dock_remove_icon_from_dock (pDock, pRemovingIcon);
				
				if (CAIRO_DOCK_IS_APPLI (pRemovingIcon) && pRemovingIcon->cClass != NULL && pDock == cairo_dock_search_dock_from_name (pRemovingIcon->cClass) && pDock->icons == NULL)  // il n'y a plus aucune icone de cette classe.
				{
					cd_message ("le sous-dock de la classe %s n'a plus d'element et sera detruit", pRemovingIcon->cClass);
					cairo_dock_destroy_dock (pDock, pRemovingIcon->cClass, NULL, NULL);
				}
				else
				{
					cairo_dock_update_dock_size (pDock);
					cd_message ("destruction de %s", pRemovingIcon->acName);
					cairo_dock_free_icon (pRemovingIcon);
				}
			}
			else if (pRemovingIcon->fPersonnalScale == -0.05)
			{
				//g_print ("  fin\n");
				pRemovingIcon->fPersonnalScale = 0;
			}
		}

		return TRUE;
	}
	else
		return TRUE;
}


/**
*Arme l'animation d'une icone
*@param icon l'icone dont on veut preparer l'animation.
*@param iAnimationType le type d'animation voulu, ou -1 pour utiliser l'animtion correspondante au type de l'icone.
*@param iNbRounds le nombre de fois ou l'animation sera jouee, ou -1 pour utiliser la valeur correspondante au type de l'icone.
*/
void cairo_dock_arm_animation (Icon *icon, CairoDockAnimationType iAnimationType, int iNbRounds)
{
	CairoDockIconType iType = cairo_dock_get_icon_type (icon);
	if (iAnimationType == -1)
		icon->iAnimationType = g_tAnimationType[iType];
	else
		icon->iAnimationType = iAnimationType;

	if (icon->iAnimationType == CAIRO_DOCK_RANDOM)
		icon->iAnimationType = g_random_int_range (0, CAIRO_DOCK_NB_ANIMATIONS-1);  // [a;b[


	if (iNbRounds == -1)
		iNbRounds = g_tNbAnimationRounds[iType];
	icon->iCount = MAX (0, g_tNbIterInOneRound[icon->iAnimationType] * iNbRounds - 1);
}

/**
*Lance l'animation de l'icone. Ne fait rien si l'icone ne sera pas animee.
*@param icon l'icone a animer.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_start_animation (Icon *icon, CairoDock *pDock)
{
	cd_message ("%s (%s, %d)", __func__, icon->acName, icon->iAnimationType);
	if ((icon->iCount > 0 && icon->iAnimationType < CAIRO_DOCK_RANDOM) || icon->fPersonnalScale != 0)
	{
		if (pDock->iSidGrowUp != 0)
		{
			pDock->fFoldingFactor = 0;
			g_source_remove (pDock->iSidGrowUp);
			pDock->iSidGrowUp = 0;
		}
		if (pDock->iSidShrinkDown == 0)
			pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);  // fera diminuer de taille les icones, et rebondir/tourner/clignoter celle sui est animee.
	}
}
