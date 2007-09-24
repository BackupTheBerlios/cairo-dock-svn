/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include "stdlib.h"

#include "cd-clock-struct.h"
#include "cd-clock-draw.h"
#include "cd-clock-config.h"
#include "cd-clock-menu-functions.h"
#include "cd-clock-init.h"


CairoDock *my_pDock = NULL;
gboolean my_bShowDate;
gboolean my_bShowSeconds;
gboolean my_b24Mode;
gboolean my_bOldStyle;
int my_iTheme = 0;

int my_iSidUpdateClock = 0;
Icon *my_pIcon = NULL;
cairo_t *my_pCairoContext = NULL;
GHashTable *my_pThemeTable = NULL;

cairo_surface_t* my_pBackgroundSurface = NULL;
cairo_surface_t* my_pForegroundSurface = NULL;
RsvgDimensionData my_DimensionData;
RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];

GPtrArray *my_pAlarms = NULL;

char my_cFileNames[CLOCK_ELEMENTS][30] =
{
	"clock-drop-shadow.svg",
	"clock-face.svg",
	"clock-marks.svg",
	"clock-hour-hand-shadow.svg",
	"clock-minute-hand-shadow.svg",
	"clock-second-hand-shadow.svg",
	"clock-hour-hand.svg",
	"clock-minute-hand.svg",
	"clock-second-hand.svg",
	"clock-face-shadow.svg",
	"clock-glass.svg",
	"clock-frame.svg"
};


gchar *cd_clock_pre_init (void)
{
	//g_print ("%s ()\n", __func__);
	return g_strdup_printf ("%s/%s", CD_CLOCK_SHARE_DATA_DIR, CD_CLOCK_README_FILE);
}


Icon *cd_clock_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	//g_print ("%s ()\n", __func__);
	//\_______________ On verifie que nos fichiers existent.
	*cConfFilePath = cairo_dock_check_conf_file_exists (CD_CLOCK_USER_DATA_DIR, CD_CLOCK_SHARE_DATA_DIR, CD_CLOCK_CONF_FILE);
	
	
	//\_______________ On charge la liste des themes disponibles.
	GError *tmp_erreur = NULL;
	gchar *cThemesDir = g_strdup_printf ("%s/themes", CD_CLOCK_SHARE_DATA_DIR);
	my_pThemeTable = cairo_dock_list_themes (cThemesDir, NULL, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	cairo_dock_update_conf_file_with_hash_table (*cConfFilePath, my_pThemeTable, "MODULE", "theme", NULL, (GHFunc) cairo_dock_write_one_theme_name);
	
	int i;
	for (i = 0; i < CLOCK_ELEMENTS; i ++)
		my_pSvgHandles[i] = NULL;
	
	
	//\_______________ On lit le fichier de conf.
	int iOriginalWidth = 1, iOriginalHeight = 1;
	gchar *cName = NULL;
	cd_clock_read_conf_file (*cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cName);
	
	
	//\_______________ On cree notre icone.
	my_pIcon = cairo_dock_create_icon_for_applet (pDock, iOriginalWidth, iOriginalHeight, cName, NULL);
	my_pDock = pDock;
	my_pCairoContext = cairo_create (my_pIcon->pIconBuffer);
	g_return_val_if_fail (cairo_status (my_pCairoContext) == CAIRO_STATUS_SUCCESS, NULL);
	
	
	//\_______________ On charge les surfaces d'arriere-plan et d'avant-plan.
	cairo_t* pSourceContext = cairo_create (my_pIcon->pIconBuffer);
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	
	my_pBackgroundSurface = update_surface (NULL,
		pSourceContext,
		my_pIcon->fWidth * (1 + g_fAmplitude),
		my_pIcon->fHeight * (1 + g_fAmplitude),
		KIND_BACKGROUND);
	my_pForegroundSurface = update_surface (NULL,
		pSourceContext,
		my_pIcon->fWidth * (1 + g_fAmplitude),
		my_pIcon->fHeight * (1 + g_fAmplitude),
		KIND_FOREGROUND);
	cairo_destroy (pSourceContext);
	
	//\_______________ On enregistre nos notifications.
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) cd_clock_notification_click_icon, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) cd_clock_notification_build_menu, CAIRO_DOCK_RUN_FIRST);
	
	//\_______________ On lance le timer.
	cd_clock_update_with_time (my_pIcon);
	my_iSidUpdateClock = g_timeout_add ((my_bShowSeconds ? 1000 : 60000), (GSourceFunc) cd_clock_update_with_time, (gpointer) my_pIcon);
	
	g_free (cName);
	return my_pIcon;
}

void cd_clock_stop (void)
{
	//g_print ("%s ()\n", __func__);
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) cd_clock_notification_click_icon);
	cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) cd_clock_notification_build_menu);
	
	g_source_remove (my_iSidUpdateClock);
	my_iSidUpdateClock = 0;
	my_pIcon = NULL;
	
	cairo_destroy (my_pCairoContext);
	my_pCairoContext = NULL;
	int i;
	for (i = 0; i < CLOCK_ELEMENTS; i ++)
	{
		rsvg_handle_free (my_pSvgHandles[i]);
		my_pSvgHandles[i] = NULL;
	}
	
	cairo_surface_destroy (my_pForegroundSurface);
	my_pForegroundSurface = NULL;
	cairo_surface_destroy (my_pBackgroundSurface);
	my_pBackgroundSurface = NULL;
	
	g_hash_table_destroy (my_pThemeTable);
	my_pThemeTable = NULL;
	
	CDClockAlarm *pAlarm;
	for (i = 0; i < my_pAlarms->len; i ++)
	{
		pAlarm = g_ptr_array_index (my_pAlarms, i);
		cd_clock_free_alarm (pAlarm);
	}
	g_ptr_array_free (my_pAlarms, TRUE);
	my_pAlarms = NULL;
}
