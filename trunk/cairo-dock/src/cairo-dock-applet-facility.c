/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>

#include "cairo-dock-load.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-config.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-applet-factory.h"
#include "cairo-dock-log.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-applet-facility.h"

extern gchar *g_cCurrentThemePath;

extern double g_fAmplitude;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

gchar *cairo_dock_check_conf_file_exists (gchar *cUserDataDirName, gchar *cShareDataDir, gchar *cConfFileName)
{
	if (cConfFileName == NULL)
		return NULL;
	
	gchar *cUserDataDirPath = g_strdup_printf ("%s/plug-ins/%s", g_cCurrentThemePath, cUserDataDirName);
	if (! g_file_test (cUserDataDirPath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		cd_message ("directory %s doesn't exist, it will be added.", cUserDataDirPath);
		
		gchar *command = g_strdup_printf ("mkdir -p %s", cUserDataDirPath);
		system (command);
		g_free (command);
	}
	
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", cUserDataDirPath, cConfFileName);
	if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))
	{
		gchar *command = g_strdup_printf ("cp %s/%s %s", cShareDataDir, cConfFileName, cConfFilePath);
		system (command);
		g_free (command);
	}
	
	if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))  // la copie ne s'est pas bien passee.
	{
		cd_warning ("Attention : couldn't copy %s/%s in %s; check permissions and file's existence", cShareDataDir, cConfFileName, cUserDataDirPath);
		g_free (cUserDataDirPath);
		g_free (cConfFilePath);
		return NULL;
	}
	
	g_free (cUserDataDirPath);
	return cConfFilePath;
}

void cairo_dock_free_minimal_config (CairoDockMinimalAppletConfig *pMinimalConfig)
{
	if (pMinimalConfig == NULL)
		return;
	g_free (pMinimalConfig->cLabel);
	g_free (pMinimalConfig->cIconFileName);
	g_free (pMinimalConfig);
}


void cairo_dock_set_icon_surface_full (cairo_t *pIconContext, cairo_surface_t *pSurface, double fScale, double fAlpha, Icon *pIcon, CairoDockContainer *pContainer)
{
	g_return_if_fail (cairo_status (pIconContext) == CAIRO_STATUS_SUCCESS);

	//\________________ On efface l'ancienne image.
	cairo_set_source_rgba (pIconContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pIconContext);
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
	
	//\________________ On applique la nouvelle image.
	if (pSurface != NULL && fScale > 0)
	{
		if (fScale != 1 && pIcon != NULL)
		{
			double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK_DOCK (pContainer)->fRatio : 1);
			cairo_translate (pIconContext, pIcon->fWidth * fMaxScale / 2 * (1 - fScale) , pIcon->fHeight * fMaxScale / 2 * (1 - fScale));
			cairo_scale (pIconContext, fScale, fScale);
		}
		
		cairo_set_source_surface (
			pIconContext,
			pSurface,
			0.,
			0.);
		
		if (fAlpha != 1)
			cairo_paint_with_alpha (pIconContext, fAlpha);
		else
			cairo_paint (pIconContext);
	}
}

void cairo_dock_add_reflection_to_icon (cairo_t *pIconContext, Icon *pIcon, CairoDockContainer *pContainer)
{
	g_return_if_fail (pIcon != NULL && pContainer!= NULL);
	if (pIcon->pReflectionBuffer != NULL)
	{
		cairo_surface_destroy (pIcon->pReflectionBuffer);
		pIcon->pReflectionBuffer = NULL;
	}
	
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK_DOCK (pContainer)->fRatio : 1);
	gboolean bIsHorizontal = pContainer->bIsHorizontal;
	pIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (pIcon->pIconBuffer,
		pIconContext,
		(bIsHorizontal ? pIcon->fWidth : pIcon->fHeight) * fMaxScale,
		(bIsHorizontal ? pIcon->fHeight : pIcon->fWidth) * fMaxScale,
		bIsHorizontal,
		fMaxScale);

	if (pIcon->pFullIconBuffer != NULL)
	{
		cairo_surface_destroy (pIcon->pFullIconBuffer);
		pIcon->pFullIconBuffer = NULL;
	}
	pIcon->pFullIconBuffer = cairo_dock_create_icon_surface_with_reflection (pIcon->pIconBuffer,
		pIcon->pReflectionBuffer,
		pIconContext,
		(bIsHorizontal ? pIcon->fWidth : pIcon->fHeight) * fMaxScale,
		(bIsHorizontal ? pIcon->fHeight : pIcon->fWidth) * fMaxScale,
		bIsHorizontal,
		fMaxScale);
}

void cairo_dock_set_icon_surface_with_reflect (cairo_t *pIconContext, cairo_surface_t *pSurface, Icon *pIcon, CairoDockContainer *pContainer)
{
	cairo_dock_set_icon_surface (pIconContext, pSurface);

	cairo_dock_add_reflection_to_icon (pIconContext, pIcon, pContainer);
}

void cairo_dock_set_image_on_icon (cairo_t *pIconContext, gchar *cImagePath, Icon *pIcon, CairoDockContainer *pContainer)
{
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK_DOCK (pContainer)->fRatio : 1);
	cairo_surface_t *pImageSurface = cairo_dock_create_surface_for_icon (cImagePath,
		pIconContext,
		pIcon->fWidth * fMaxScale,
		pIcon->fHeight * fMaxScale);
	
	cairo_dock_set_icon_surface_with_reflect (pIconContext, pImageSurface, pIcon, pContainer);
	
	cairo_surface_destroy (pImageSurface);
}

void cairo_dock_draw_bar_on_icon (cairo_t *pIconContext, double fValue, Icon *pIcon, CairoDockContainer *pContainer)
{
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK_DOCK (pContainer)->fRatio : 1);
	cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
		0.,
		pIcon->fWidth * fMaxScale,
		0.);  // de gauche a droite.
	g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
	
	cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		0.,
		1.,
		0.,
		0.,
		1.);
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		1.,
		0.,
		1.,
		0.,
		1.);
	
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
	
	cairo_set_line_width (pIconContext, 6);
	cairo_set_line_cap (pIconContext, CAIRO_LINE_CAP_ROUND);
	
	cairo_move_to (pIconContext, 3, pIcon->fHeight * fMaxScale - 3);
	cairo_rel_line_to (pIconContext, (pIcon->fWidth * fMaxScale - 6) * fValue, 0);
	
	cairo_set_source (pIconContext, pGradationPattern);
	cairo_stroke (pIconContext);
	
	cairo_pattern_destroy (pGradationPattern);
}


void cairo_dock_set_icon_name (cairo_t *pSourceContext, const gchar *cIconName, Icon *pIcon, CairoDockContainer *pContainer)  // fonction proposee par Necropotame.
{
	g_return_if_fail (pIcon != NULL && pContainer != NULL);  // le contexte sera verifie plus loin.

	g_free (pIcon->acName);
	pIcon->acName = g_strdup (cIconName);

	cairo_dock_fill_one_text_buffer(
		pIcon,
		pSourceContext,
		g_iLabelSize,
		g_cLabelPolice,
		(g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pContainer->bIsHorizontal));
}

void cairo_dock_set_quick_info (cairo_t *pSourceContext, const gchar *cQuickInfo, Icon *pIcon, double fMaxScale)
{
	g_return_if_fail (pIcon != NULL);  // le contexte sera verifie plus loin.

	g_free (pIcon->cQuickInfo);
	pIcon->cQuickInfo = g_strdup (cQuickInfo);
	cd_message ("cQuickInfo <- '%s'", pIcon->cQuickInfo);
	
	cairo_dock_fill_one_quick_info_buffer (pIcon,
		pSourceContext,
		12,
		g_cLabelPolice,
		PANGO_WEIGHT_HEAVY,
		fMaxScale);
	cd_message (" -> %dx%d", pIcon->iQuickInfoWidth, pIcon->iQuickInfoHeight);
}

void cairo_dock_set_quick_info_full (cairo_t *pSourceContext, Icon *pIcon, CairoDockContainer *pContainer, const gchar *cQuickInfoFormat, ...)
{
	va_list args;
	va_start (args, cQuickInfoFormat);
	gchar *cFullText = g_strdup_vprintf (cQuickInfoFormat, args);
	cairo_dock_set_quick_info (pSourceContext, cFullText, pIcon, (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK_DOCK (pContainer)->fRatio : 1));
	g_free (cFullText);
	va_end (args);
}

void cairo_dock_set_hours_minutes_as_quick_info (cairo_t *pSourceContext, Icon *pIcon, CairoDockContainer *pContainer, int iTimeInSeconds)
{
	int hours = iTimeInSeconds / 3600;
	int minutes = (iTimeInSeconds % 3600) / 60;
	if (hours != 0)
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%dh%02d", hours, abs (minutes));
	else
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%d", minutes);
}

void cairo_dock_set_minutes_secondes_as_quick_info (cairo_t *pSourceContext, Icon *pIcon, CairoDockContainer *pContainer, int iTimeInSeconds)
{
	int minutes = iTimeInSeconds / 60;
	int secondes = iTimeInSeconds % 60;
	if (minutes != 0)
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%d:%02d", minutes, abs (secondes));
	else
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%d", secondes);
}



void cairo_dock_animate_icon (Icon *pIcon, CairoDock *pDock, CairoDockAnimationType iAnimationType, int iNbRounds)
{
	cairo_dock_arm_animation (pIcon, iAnimationType, iNbRounds);
	cairo_dock_start_animation (pIcon, pDock);
}


gchar* cairo_dock_manage_themes_for_applet (gchar *cAppletShareDataDir, gchar *cThemeDirName, gchar *cAppletConfFilePath, GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gchar *cDefaultThemeName)
{
	GError *erreur = NULL;
	gchar *cThemesDirPath = g_strdup_printf ("%s/%s", cAppletShareDataDir, cThemeDirName);
	GHashTable *pThemeTable = cairo_dock_list_themes (cThemesDirPath, NULL, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_free (cThemesDirPath);

	gchar *cThemePath = NULL;
	if (pThemeTable != NULL)
	{
		cairo_dock_update_conf_file_with_themes (pKeyFile, cAppletConfFilePath, pThemeTable, cGroupName, cKeyName);
		
		gchar *cChosenThemeName = cairo_dock_get_string_key_value (pKeyFile, cGroupName, cKeyName, bFlushConfFileNeeded, cDefaultThemeName, NULL, NULL);
		if (cChosenThemeName != NULL)
			cThemePath = g_strdup (g_hash_table_lookup (pThemeTable, cChosenThemeName));
		g_free (cChosenThemeName);
		
		if (cThemePath == NULL && cDefaultThemeName != NULL)
			cThemePath = g_strdup (g_hash_table_lookup (pThemeTable, cDefaultThemeName));

		g_hash_table_destroy (pThemeTable);
	}
	return cThemePath;
}

GtkWidget *cairo_dock_create_sub_menu (gchar *cLabel, GtkWidget *pMenu)
{
	GtkWidget *pSubMenu = gtk_menu_new ();
	GtkWidget *pMenuItem = gtk_menu_item_new_with_label (cLabel);
	gtk_menu_shell_append (GTK_MENU_SHELL (pMenu), pMenuItem);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
	return pSubMenu;
}



static gboolean _cairo_dock_timer (CairoDockMeasure *pMeasureTimer)
{
	cairo_dock_launch_measure (pMeasureTimer);
	return TRUE;
}

static gpointer _cairo_dock_threaded_calculation (CairoDockMeasure *pMeasureTimer)
{
	pMeasureTimer->acquisition ();
	
	g_mutex_lock (pMeasureTimer->pMutexData);
	pMeasureTimer->read ();
	g_mutex_unlock (pMeasureTimer->pMutexData);
	
	g_atomic_int_set (&pMeasureTimer->iThreadIsRunning, 0);
	cd_message ("*** fin du thread");
	return NULL;
}

static gboolean _cairo_dock_check_for_redraw (CairoDockMeasure *pMeasureTimer)
{
	int iThreadIsRunning = g_atomic_int_get (&pMeasureTimer->iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning)
	{
		//\_______________________ On recharge les icones avec ces nouvelles donnees.
		g_mutex_lock (pMeasureTimer->pMutexData);
		pMeasureTimer->update ();
		g_mutex_unlock (pMeasureTimer->pMutexData);
		
		//\_______________________ On lance le timer si necessaire.
		if (pMeasureTimer->iSidTimer == 0)
			pMeasureTimer->iSidTimer = g_timeout_add (pMeasureTimer->iCheckInterval, (GSourceFunc) _cairo_dock_timer, pMeasureTimer);
		
		pMeasureTimer->iSidTimerRedraw = 0;
		return FALSE;
	}
	return TRUE;
}

void cairo_dock_launch_measure (CairoDockMeasure *pMeasureTimer)
{
	cd_message ("");
	if (g_atomic_int_compare_and_exchange (&pMeasureTimer->iThreadIsRunning, 0, 1))  // il etait egal a 0, on lui met 1 et on lance le thread.
	{
		cd_message (" ==> lancement du thread de calcul");
		
		if (pMeasureTimer->iSidTimerRedraw == 0)
			pMeasureTimer->iSidTimerRedraw = g_timeout_add (333, (GSourceFunc) _cairo_dock_check_for_redraw, pMeasureTimer);
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) _cairo_dock_threaded_calculation, pMeasureTimer, FALSE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
	else if (pMeasureTimer->iSidTimer == 0)
	{
		pMeasureTimer->iSidTimer = g_timeout_add (pMeasureTimer->iCheckInterval, (GSourceFunc) _cairo_dock_timer, pMeasureTimer);
	}
}

CairoDockMeasure *cairo_dock_new_measure_timer (int iCheckInterval, GVoidFunc acquisition, GVoidFunc read, GVoidFunc update)
{
	CairoDockMeasure *pMeasureTimer = g_new0 (CairoDockMeasure, 1);
	pMeasureTimer->pMutexData = g_mutex_new ();
	pMeasureTimer->iCheckInterval = iCheckInterval;
	pMeasureTimer->acquisition = acquisition;
	pMeasureTimer->read = read;
	pMeasureTimer->update = update;
	return pMeasureTimer;
}

void cairo_dock_stop_measure_timer (CairoDockMeasure *pMeasureTimer)
{
	if (pMeasureTimer->iSidTimerRedraw != 0)
	{
		g_source_remove (pMeasureTimer->iSidTimerRedraw);
		pMeasureTimer->iSidTimerRedraw = 0;
	}
	if (pMeasureTimer->iSidTimer!= 0)
	{
		g_source_remove (pMeasureTimer->iSidTimer);
		pMeasureTimer->iSidTimer= 0;
	}
}

void cairo_dock_free_measure_timer (CairoDockMeasure *pMeasureTimer)
{
	cairo_dock_stop_measure_timer (pMeasureTimer);
	
	cd_message ("on attend que le thread termine...\n");
	while (g_atomic_int_get (&pMeasureTimer->iThreadIsRunning));
	cd_message ("temine\n");
	
	g_mutex_free (pMeasureTimer->pMutexData);
	g_free (pMeasureTimer);
}

gboolean cairo_dock_measure_is_active (CairoDockMeasure *pMeasureTimer)
{
	return (pMeasureTimer->iSidTimer != 0);
}

void cairo_dock_change_measure_frequency (CairoDockMeasure *pMeasureTimer, int iNewCheckInterval)
{
	pMeasureTimer->iCheckInterval = iNewCheckInterval;
	
	gboolean bNeedsRestart = (pMeasureTimer->iSidTimer != 0);
	cairo_dock_stop_measure_timer (pMeasureTimer);
	
	if (bNeedsRestart)
		pMeasureTimer->iSidTimer = g_timeout_add (pMeasureTimer->iCheckInterval, (GSourceFunc) _cairo_dock_timer, pMeasureTimer);
}
