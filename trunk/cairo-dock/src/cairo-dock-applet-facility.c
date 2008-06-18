/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

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
#include "cairo-dock-keyfile-utilities.h"
#include "cairo-dock-applet-factory.h"
#include "cairo-dock-log.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-applet-facility.h"

extern gchar *g_cCurrentThemePath;

extern double g_fAmplitude;
extern double g_fAlbedo;

extern CairoDockLabelDescription g_iconTextDescription;
extern CairoDockLabelDescription g_quickInfoTextDescription;
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
	g_free (pMinimalConfig->cDockName);
	g_free (pMinimalConfig);
}


void cairo_dock_set_icon_surface_full (cairo_t *pIconContext, cairo_surface_t *pSurface, double fScale, double fAlpha, Icon *pIcon, CairoContainer *pContainer)
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
		cairo_save (pIconContext);
		if (fScale != 1 && pIcon != NULL)
		{
			double fMaxScale = cairo_dock_get_max_scale (pContainer);
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
		cairo_restore (pIconContext);
	}
}

void cairo_dock_add_reflection_to_icon (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer)
{
	g_return_if_fail (pIcon != NULL && pContainer!= NULL);
	if (pIcon->pReflectionBuffer != NULL)
	{
		cairo_surface_destroy (pIcon->pReflectionBuffer);
		pIcon->pReflectionBuffer = NULL;
	}
	
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK (pContainer)->fRatio : 1);
	gboolean bIsHorizontal = pContainer->bIsHorizontal;
	pIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (pIcon->pIconBuffer,
		pIconContext,
		(bIsHorizontal ? pIcon->fWidth : pIcon->fHeight) * fMaxScale,
		(bIsHorizontal ? pIcon->fHeight : pIcon->fWidth) * fMaxScale,
		bIsHorizontal,
		fMaxScale,
		pContainer->bDirectionUp);

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
		fMaxScale,
		pContainer->bDirectionUp);
}

void cairo_dock_set_icon_surface_with_reflect (cairo_t *pIconContext, cairo_surface_t *pSurface, Icon *pIcon, CairoContainer *pContainer)
{
	cairo_dock_set_icon_surface (pIconContext, pSurface);
	
	cairo_dock_add_reflection_to_icon (pIconContext, pIcon, pContainer);
}

void cairo_dock_set_image_on_icon (cairo_t *pIconContext, gchar *cImagePath, Icon *pIcon, CairoContainer *pContainer)
{
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK (pContainer)->fRatio : 1);
	cairo_surface_t *pImageSurface = cairo_dock_create_surface_for_icon (cImagePath,
		pIconContext,
		pIcon->fWidth * fMaxScale,
		pIcon->fHeight * fMaxScale);
	
	cairo_dock_set_icon_surface_with_reflect (pIconContext, pImageSurface, pIcon, pContainer);
	
	cairo_surface_destroy (pImageSurface);
}

void cairo_dock_draw_bar_on_icon (cairo_t *pIconContext, double fValue, Icon *pIcon, CairoContainer *pContainer)
{
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / CAIRO_DOCK (pContainer)->fRatio : 1);
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
	
	cairo_save (pIconContext);
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
	
	cairo_set_line_width (pIconContext, 6);
	cairo_set_line_cap (pIconContext, CAIRO_LINE_CAP_ROUND);
	
	cairo_move_to (pIconContext, 3, pIcon->fHeight * fMaxScale - 3);
	cairo_rel_line_to (pIconContext, (pIcon->fWidth * fMaxScale - 6) * fValue, 0);
	
	cairo_set_source (pIconContext, pGradationPattern);
	cairo_stroke (pIconContext);
	
	cairo_pattern_destroy (pGradationPattern);
	cairo_restore (pIconContext);
}

//Fonction proposée par Nécropotame, rédigée par ChAnGFu
void cairo_dock_draw_emblem_on_my_icon (cairo_t *pIconContext, const gchar *cIconFile, Icon *pIcon, CairoContainer *pContainer, CairoDockEmblem pEmblemType)
{
	cd_debug ("%s (%s %d)", __func__, cIconFile, pEmblemType);
	g_return_if_fail (pIcon != NULL && pContainer != NULL); 
	
	if (cIconFile == NULL) 
		return;
	
	cairo_surface_t *pCairoSurface=NULL;
	double fImgX, fImgY, fImgW, fImgH, emblemW = pIcon->fWidth / 3, emblemH = pIcon->fHeight / 3;
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / 1 : 1);
	pCairoSurface = cairo_dock_create_surface_from_image (cIconFile, pIconContext, fMaxScale, emblemW, emblemH, CAIRO_DOCK_KEEP_RATIO, &fImgW, &fImgH, NULL, NULL);

	switch (pEmblemType) {
		default:
		case CAIRO_DOCK_EMBLEM_UPPER_RIGHT :
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale;
			fImgY = 1.;
		break;

		case CAIRO_DOCK_EMBLEM_LOWER_RIGHT :
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale;
			fImgY = ((pIcon->fHeight - emblemH - pIcon->fScale) * fMaxScale) + 1.;
		break;
		
		case CAIRO_DOCK_EMBLEM_MIDDLE :
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale / 2.;
			fImgY = (pIcon->fHeight - emblemH - pIcon->fScale) * fMaxScale / 2.;
		break;
		
		case CAIRO_DOCK_EMBLEM_MIDDLE_BOTTOM:
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale / 2.;
			fImgY = ((pIcon->fHeight - emblemH - pIcon->fScale) * fMaxScale) + 1.;
		break;

		case CAIRO_DOCK_EMBLEM_BACKGROUND :
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale / 2.;
			fImgY = 0.;
			cairo_surface_t *pNewSurfaceGradated = cairo_surface_create_similar (pCairoSurface, CAIRO_CONTENT_COLOR_ALPHA, emblemW, emblemH);
			cairo_t *pCairoContext = cairo_create (pNewSurfaceGradated);
			cairo_set_source_surface (pCairoContext, pCairoSurface, 0, 0);

			cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0., 1., 0., (emblemH - 1.));  // de haut en bas.
			g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);

			cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
			cairo_pattern_add_color_stop_rgba (pGradationPattern, 1., 0., 0., 0., 0.);
			cairo_pattern_add_color_stop_rgba (pGradationPattern, 0., 0., 0., 0., emblemH);

			cairo_translate (pCairoContext, 0, 0);
			cairo_mask (pCairoContext, pGradationPattern);

			cairo_pattern_destroy (pGradationPattern);
			cairo_destroy (pCairoContext);
			pCairoSurface = pNewSurfaceGradated;
		break;
	}
	
	//cd_debug ("Emblem: X %.0f Y %.0f W %.0f H %.0f - Icon: W %.0f H %.0f", fImgX, fImgY, emblemW, emblemH, pIcon->fWidth, pIcon->fHeight);
	
	cairo_save (pIconContext);
	cairo_translate (pIconContext, fImgX , fImgY);
	cairo_set_source_surface (pIconContext, pCairoSurface, 0.0, 0.0);
	cairo_paint (pIconContext);
	cairo_restore (pIconContext);
	cairo_surface_destroy (pCairoSurface);
}

void cairo_dock_draw_emblem_classic (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer, CairoDockClassicEmblem pEmblemClassic, CairoDockEmblem pEmblemType)
{
	cd_debug ("%s (%s %d)", __func__, pIcon->acName, pEmblemType);
	g_return_if_fail (pIcon != NULL); 
	
	gchar *cClassicEmblemPath = NULL;
	switch (pEmblemClassic) {
		default :
		case CAIRO_DOCK_EMBLEM_BLANK :
			cClassicEmblemPath = g_strdup_printf ("%s/emblems/blank.svg", CAIRO_DOCK_SHARE_DATA_DIR);
		break;
		case CAIRO_DOCK_EMBLEM_CHARGE:
			cClassicEmblemPath = g_strdup_printf ("%s/emblems/charge.svg", CAIRO_DOCK_SHARE_DATA_DIR);
		break;
		case CAIRO_DOCK_EMBLEM_DROP_INDICATOR:
			cClassicEmblemPath = g_strdup_printf ("%s/emblems/drop.svg", CAIRO_DOCK_SHARE_DATA_DIR);
		break;
		case CAIRO_DOCK_EMBLEM_PLAY:
			cClassicEmblemPath = g_strdup_printf ("%s/emblems/play.svg", CAIRO_DOCK_SHARE_DATA_DIR);
		break;
		case CAIRO_DOCK_EMBLEM_PAUSE:
			cClassicEmblemPath = g_strdup_printf ("%s/emblems/pause.svg", CAIRO_DOCK_SHARE_DATA_DIR);
		break;
		case CAIRO_DOCK_EMBLEM_STOP:
			cClassicEmblemPath = g_strdup_printf ("%s/emblems/stop.svg", CAIRO_DOCK_SHARE_DATA_DIR);
		break;
		case CAIRO_DOCK_EMBLEM_BROKEN:
			cClassicEmblemPath = g_strdup_printf ("%s/emblems/broken.svg", CAIRO_DOCK_SHARE_DATA_DIR);
		break;
		//Il reste play pause stop broken a faire.
	}
	
	cairo_dock_draw_emblem_on_my_icon (pIconContext, cClassicEmblemPath, pIcon, pContainer, pEmblemType);
	g_free (cClassicEmblemPath);
}

void cairo_dock_set_icon_name (cairo_t *pSourceContext, const gchar *cIconName, Icon *pIcon, CairoContainer *pContainer)  // fonction proposee par Necropotame.
{
	g_return_if_fail (pIcon != NULL && pContainer != NULL);  // le contexte sera verifie plus loin.

	g_free (pIcon->acName);
	pIcon->acName = g_strdup (cIconName);

	cairo_dock_fill_one_text_buffer(
		pIcon,
		pSourceContext,
		&g_iconTextDescription,
		(g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pContainer->bIsHorizontal),
		pContainer->bDirectionUp);
}
void cairo_dock_set_icon_name_full (cairo_t *pSourceContext, Icon *pIcon, CairoContainer *pContainer, const gchar *cIconNameFormat, ...)
{
	va_list args;
	va_start (args, cIconNameFormat);
	gchar *cFullText = g_strdup_vprintf (cIconNameFormat, args);
	cairo_dock_set_icon_name (pSourceContext, cFullText, pIcon, pContainer);
	g_free (cFullText);
	va_end (args);
}

void cairo_dock_set_quick_info (cairo_t *pSourceContext, const gchar *cQuickInfo, Icon *pIcon, double fMaxScale)
{
	g_return_if_fail (pIcon != NULL);  // le contexte sera verifie plus loin.

	g_free (pIcon->cQuickInfo);
	pIcon->cQuickInfo = g_strdup (cQuickInfo);
	
	cairo_dock_fill_one_quick_info_buffer (pIcon,
		pSourceContext,
		&g_quickInfoTextDescription,
		fMaxScale);
	cd_debug (" cQuickInfo <- '%s' (%dx%d)", pIcon->cQuickInfo, pIcon->iQuickInfoWidth, pIcon->iQuickInfoHeight);
}

void cairo_dock_set_quick_info_full (cairo_t *pSourceContext, Icon *pIcon, CairoContainer *pContainer, const gchar *cQuickInfoFormat, ...)
{
	va_list args;
	va_start (args, cQuickInfoFormat);
	gchar *cFullText = g_strdup_vprintf (cQuickInfoFormat, args);
	cairo_dock_set_quick_info (pSourceContext, cFullText, pIcon, (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / 1 : 1));
	g_free (cFullText);
	va_end (args);
}

void cairo_dock_set_hours_minutes_as_quick_info (cairo_t *pSourceContext, Icon *pIcon, CairoContainer *pContainer, int iTimeInSeconds)
{
	int hours = iTimeInSeconds / 3600;
	int minutes = (iTimeInSeconds % 3600) / 60;
	if (hours != 0)
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%dh%02d", hours, abs (minutes));
	else
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%dmn", minutes);
}

void cairo_dock_set_minutes_secondes_as_quick_info (cairo_t *pSourceContext, Icon *pIcon, CairoContainer *pContainer, int iTimeInSeconds)
{
	int minutes = iTimeInSeconds / 60;
	int secondes = iTimeInSeconds % 60;
	if (minutes != 0)
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%d:%02d", minutes, abs (secondes));
	else
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%s0:%02d", (secondes < 0 ? "-" : ""), abs (secondes));
}

void cairo_dock_set_size_as_quick_info (cairo_t *pSourceContext, Icon *pIcon, CairoContainer *pContainer, long long int iSizeInBytes)
{
	if (iSizeInBytes < 1024)
	{
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%dB", iSizeInBytes);
	}
	else if (iSizeInBytes < (1 << 20))
	{
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%dK", (int) (iSizeInBytes>>10));
	}
	else if (iSizeInBytes < (1 << 30))
	{
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%dM", (int) (iSizeInBytes>>20));
	}
	else
	{
		cairo_dock_set_quick_info_full (pSourceContext, pIcon, pContainer, "%dG", (int) (iSizeInBytes>>30));
	}
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
	if (pMeasureTimer->acquisition != NULL)
		pMeasureTimer->acquisition ();
	
	g_mutex_lock (pMeasureTimer->pMutexData);
	pMeasureTimer->read ();
	g_mutex_unlock (pMeasureTimer->pMutexData);
	
	g_atomic_int_set (&pMeasureTimer->iThreadIsRunning, 0);
	cd_debug ("*** fin du thread");
	return NULL;
}
static gboolean _cairo_dock_check_for_redraw (CairoDockMeasure *pMeasureTimer)
{
	int iThreadIsRunning = g_atomic_int_get (&pMeasureTimer->iThreadIsRunning);
	cd_debug ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning)
	{
		//\_______________________ On recharge ce qu'il faut avec ces nouvelles donnees.
		g_mutex_lock (pMeasureTimer->pMutexData);
		gboolean bContinue = pMeasureTimer->update ();
		g_mutex_unlock (pMeasureTimer->pMutexData);
		
		//\_______________________ On lance/arrete le timer si necessaire.
		if (! bContinue)
		{
			if (pMeasureTimer->iSidTimer != 0)
			{
				g_source_remove (pMeasureTimer->iSidTimer);
				pMeasureTimer->iSidTimer = 0;
			}
		}
		else if (pMeasureTimer->iSidTimer == 0 && pMeasureTimer->iCheckInterval != 0)
		{
			pMeasureTimer->iFrequencyState = CAIRO_DOCK_FREQUENCY_NORMAL;
			pMeasureTimer->iSidTimer = g_timeout_add_seconds (pMeasureTimer->iCheckInterval, (GSourceFunc) _cairo_dock_timer, pMeasureTimer);
		}
		
		pMeasureTimer->iSidTimerRedraw = 0;
		return FALSE;
	}
	return TRUE;
}
void cairo_dock_launch_measure (CairoDockMeasure *pMeasureTimer)
{
	g_return_if_fail (pMeasureTimer != NULL);
	if (pMeasureTimer->pMutexData == NULL)
	{
		if (pMeasureTimer->acquisition != NULL)
			pMeasureTimer->acquisition ();
		if (pMeasureTimer->read != NULL)
			pMeasureTimer->read ();
		gboolean bContinue = pMeasureTimer->update ();
		
		if (! bContinue)
		{
			if (pMeasureTimer->iSidTimer != 0)
			{
				g_source_remove (pMeasureTimer->iSidTimer);
				pMeasureTimer->iSidTimer = 0;
			}
		}
		else if (pMeasureTimer->iSidTimer == 0 && pMeasureTimer->iCheckInterval != 0)
		{
			pMeasureTimer->iFrequencyState = CAIRO_DOCK_FREQUENCY_NORMAL;
			pMeasureTimer->iSidTimer = g_timeout_add_seconds (pMeasureTimer->iCheckInterval, (GSourceFunc) _cairo_dock_timer, pMeasureTimer);
		}
	}
	else if (g_atomic_int_compare_and_exchange (&pMeasureTimer->iThreadIsRunning, 0, 1))  // il etait egal a 0, on lui met 1 et on lance le thread.
	{
		cd_debug (" ==> lancement du thread de calcul");
		
		if (pMeasureTimer->iSidTimerRedraw == 0) 
			pMeasureTimer->iSidTimerRedraw = g_timeout_add (MAX (150, MIN (0.15 * pMeasureTimer->iCheckInterval, 333)), (GSourceFunc) _cairo_dock_check_for_redraw, pMeasureTimer);
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) _cairo_dock_threaded_calculation, pMeasureTimer, FALSE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
	else if (pMeasureTimer->iSidTimer == 0 && pMeasureTimer->iCheckInterval != 0)
	{
		pMeasureTimer->iFrequencyState = CAIRO_DOCK_FREQUENCY_NORMAL;
		pMeasureTimer->iSidTimer = g_timeout_add_seconds (pMeasureTimer->iCheckInterval, (GSourceFunc) _cairo_dock_timer, pMeasureTimer);
	}
}

static gboolean _cairo_dock_one_shot_timer (CairoDockMeasure *pMeasureTimer)
{
	pMeasureTimer->iSidTimerRedraw = 0;
	cairo_dock_launch_measure (pMeasureTimer);
	return FALSE;
}
void cairo_dock_launch_measure_delayed (CairoDockMeasure *pMeasureTimer, double fDelay)
{
	pMeasureTimer->iSidTimerRedraw = g_timeout_add (fDelay, (GSourceFunc) _cairo_dock_one_shot_timer, pMeasureTimer);
}

CairoDockMeasure *cairo_dock_new_measure_timer (int iCheckInterval, GVoidFunc acquisition, GVoidFunc read, CairoDockUpdateTimerFunc update)
{
	CairoDockMeasure *pMeasureTimer = g_new0 (CairoDockMeasure, 1);
	if (read != NULL || acquisition != NULL)
		pMeasureTimer->pMutexData = g_mutex_new ();
	pMeasureTimer->iCheckInterval = iCheckInterval;
	pMeasureTimer->acquisition = acquisition;
	pMeasureTimer->read = read;
	pMeasureTimer->update = update;
	return pMeasureTimer;
}

static void _cairo_dock_pause_measure_timer (CairoDockMeasure *pMeasureTimer)
{
	if (pMeasureTimer == NULL)
		return ;
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

void cairo_dock_stop_measure_timer (CairoDockMeasure *pMeasureTimer)
{
	if (pMeasureTimer == NULL)
		return ;
	
	_cairo_dock_pause_measure_timer (pMeasureTimer);
	
	cd_message ("on attend que le thread termine...");
	while (g_atomic_int_get (&pMeasureTimer->iThreadIsRunning));
	cd_message ("temine.");
}

void cairo_dock_free_measure_timer (CairoDockMeasure *pMeasureTimer)
{
	if (pMeasureTimer == NULL)
		return ;
	cairo_dock_stop_measure_timer (pMeasureTimer);
	
	if (pMeasureTimer->pMutexData != NULL)
		g_mutex_free (pMeasureTimer->pMutexData);
	g_free (pMeasureTimer);
}

gboolean cairo_dock_measure_is_active (CairoDockMeasure *pMeasureTimer)
{
	return (pMeasureTimer != NULL && pMeasureTimer->iSidTimer != 0);
}

static void _cairo_dock_restart_timer_with_frequency (CairoDockMeasure *pMeasureTimer, int iNewCheckInterval)
{
	gboolean bNeedsRestart = (pMeasureTimer->iSidTimer != 0);
	_cairo_dock_pause_measure_timer (pMeasureTimer);
	
	if (bNeedsRestart && iNewCheckInterval != 0)
		pMeasureTimer->iSidTimer = g_timeout_add_seconds (iNewCheckInterval, (GSourceFunc) _cairo_dock_timer, pMeasureTimer);
}

void cairo_dock_change_measure_frequency (CairoDockMeasure *pMeasureTimer, int iNewCheckInterval)
{
	g_return_if_fail (pMeasureTimer != NULL);
	pMeasureTimer->iCheckInterval = iNewCheckInterval;
	
	_cairo_dock_restart_timer_with_frequency (pMeasureTimer, iNewCheckInterval);
}

void cairo_dock_relaunch_measure_immediately (CairoDockMeasure *pMeasureTimer, int iNewCheckInterval)
{
	cairo_dock_stop_measure_timer (pMeasureTimer);  // on stoppe avant car on ne veut pas attendre la prochaine iteration.
	cairo_dock_change_measure_frequency (pMeasureTimer, iNewCheckInterval); // nouvelle frequence eventuelement.
	cairo_dock_launch_measure (pMeasureTimer);  // mesure immediate.
}

void cairo_dock_downgrade_frequency_state (CairoDockMeasure *pMeasureTimer)
{
	if (pMeasureTimer->iFrequencyState < CAIRO_DOCK_FREQUENCY_SLEEP)
	{
		pMeasureTimer->iFrequencyState ++;
		int iNewCheckInterval;
		switch (pMeasureTimer->iFrequencyState)
		{
			case CAIRO_DOCK_FREQUENCY_LOW :
				iNewCheckInterval = 2 * pMeasureTimer->iCheckInterval;
			break ;
			case CAIRO_DOCK_FREQUENCY_VERY_LOW :
				iNewCheckInterval = 4 * pMeasureTimer->iCheckInterval;
			break ;
			case CAIRO_DOCK_FREQUENCY_SLEEP :
				iNewCheckInterval = 10 * pMeasureTimer->iCheckInterval;
			break ;
			default :  // ne doit pas arriver.
				iNewCheckInterval = pMeasureTimer->iCheckInterval;
			break ;
		}
		
		cd_message ("degradation de la mesure (etat <- %d/%d)", pMeasureTimer->iFrequencyState, CAIRO_DOCK_NB_FREQUENCIES-1);
		_cairo_dock_restart_timer_with_frequency (pMeasureTimer, iNewCheckInterval);
	}
}

void cairo_dock_set_normal_frequency_state (CairoDockMeasure *pMeasureTimer)
{
	if (pMeasureTimer->iFrequencyState != CAIRO_DOCK_FREQUENCY_NORMAL)
	{
		pMeasureTimer->iFrequencyState = CAIRO_DOCK_FREQUENCY_NORMAL;
		_cairo_dock_restart_timer_with_frequency (pMeasureTimer, pMeasureTimer->iCheckInterval);
	}
}

//Utile pour jouer des fichiers son depuis le dock.
//A utiliser avec l'Objet UI 'u' dans les .conf
void cairo_dock_play_sound (const gchar *cSoundPath)
{
	cd_debug ("%s (%s)", __func__, cSoundPath);
	if (cSoundPath == NULL)
	{
		cd_warning ("No sound to play, halt.");
		return;
	}
	
	GError *erreur = NULL;
	gchar *cSoundCommand = NULL;
	if (g_file_test ("/usr/bin/play", G_FILE_TEST_EXISTS))
		cSoundCommand = g_strdup_printf("play \"%s\"", cSoundPath);
		
	else if (g_file_test ("/usr/bin/aplay", G_FILE_TEST_EXISTS))
		cSoundCommand = g_strdup_printf("aplay \"%s\"", cSoundPath);
	
	else if (g_file_test ("/usr/bin/paplay", G_FILE_TEST_EXISTS))
		cSoundCommand = g_strdup_printf("paplay \"%s\"", cSoundPath);
	
	cairo_dock_launch_command (cSoundCommand);
	
	g_free (cSoundCommand);
}
