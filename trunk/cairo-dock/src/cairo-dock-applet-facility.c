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
		g_print ("directory %s doesn't exist, it will be added.\n", cUserDataDirPath);
		
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
		g_print ("Attention : couldn't copy %s/%s in %s; check permissions and file's existence\n", cShareDataDir, cConfFileName, cUserDataDirPath);
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


void cairo_dock_set_icon_surface (cairo_t *pIconContext, cairo_surface_t *pSurface)  // fonction proposee par Necropotame.
{
	g_return_if_fail (cairo_status (pIconContext) == CAIRO_STATUS_SUCCESS);
	
	//\________________ On efface l'ancienne image.
	cairo_set_source_rgba (pIconContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pIconContext);
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
	
	//\________________ On applique la nouvelle image.
	if (pSurface != NULL)
	{
		cairo_set_source_surface (
			pIconContext,
			pSurface,
			0.,
			0.);
		cairo_paint (pIconContext);
	}
}

void cairo_dock_add_reflection_to_icon (cairo_t *pIconContext, Icon *pIcon, CairoDock *pDock)
{
	g_return_if_fail (pIcon != NULL && pDock != NULL);
	if (pIcon->pReflectionBuffer != NULL)
	{
		cairo_surface_destroy (pIcon->pReflectionBuffer);
		pIcon->pReflectionBuffer = NULL;
	}
	pIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (pIcon->pIconBuffer,
		pIconContext,
		(pDock->bHorizontalDock ? pIcon->fWidth : pIcon->fHeight) * (1 + g_fAmplitude),
		(pDock->bHorizontalDock ? pIcon->fHeight : pIcon->fWidth) * (1 + g_fAmplitude),
		pDock->bHorizontalDock);
	
	if (pIcon->pFullIconBuffer != NULL)
	{
		cairo_surface_destroy (pIcon->pFullIconBuffer);
		pIcon->pFullIconBuffer = NULL;
	}
	pIcon->pFullIconBuffer = cairo_dock_create_icon_surface_with_reflection (pIcon->pIconBuffer,
		pIcon->pReflectionBuffer,
		pIconContext,
		(pDock->bHorizontalDock ? pIcon->fWidth : pIcon->fHeight) * (1 + g_fAmplitude),
		(pDock->bHorizontalDock ? pIcon->fHeight : pIcon->fWidth) * (1 + g_fAmplitude),
		pDock->bHorizontalDock);
}

void cairo_dock_set_icon_surface_with_reflect (cairo_t *pIconContext, cairo_surface_t *pSurface, Icon *pIcon, CairoDock *pDock)
{
	cairo_dock_set_icon_surface (pIconContext, pSurface);
	
	cairo_dock_add_reflection_to_icon (pIconContext, pIcon, pDock);
}

void cairo_dock_set_image_on_icon (cairo_t *pIconContext, gchar *cImagePath, Icon *pIcon, CairoDock *pDock)
{
	cairo_surface_t *pImageSurface = cairo_dock_create_surface_for_icon (cImagePath,
		pIconContext,
		pIcon->fWidth * (1 + g_fAmplitude),
		pIcon->fHeight * (1 + g_fAmplitude));
	
	cairo_dock_set_icon_surface_with_reflect (pIconContext, pImageSurface, pIcon, pDock);
	
	cairo_surface_destroy (pImageSurface);
}


void cairo_dock_set_icon_name (cairo_t *pSourceContext, const gchar *cIconName, Icon *pIcon, CairoDock *pDock)  // fonction proposee par Necropotame.
{
	g_return_if_fail (pIcon != NULL && pDock != NULL);  // le contexte sera verifie plus loin.
	
	g_free (pIcon->acName);
	pIcon->acName = g_strdup (cIconName);
	
	cairo_dock_fill_one_text_buffer(
		pIcon,
		pSourceContext,
		g_iLabelSize,
		g_cLabelPolice,
		(g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
}

void cairo_dock_set_quick_info (cairo_t *pSourceContext, const gchar *cQuickInfo, Icon *pIcon, double fMaxScale)
{
	g_return_if_fail (pIcon != NULL);  // le contexte sera verifie plus loin.
	
	g_free (pIcon->cQuickInfo);
	pIcon->cQuickInfo = g_strdup (cQuickInfo);
	g_print ("cQuickInfo <- %s\n", pIcon->cQuickInfo);
	
	cairo_dock_fill_one_quick_info_buffer (pIcon,
		pSourceContext,
		12,
		g_cLabelPolice,
		PANGO_WEIGHT_HEAVY,
		fMaxScale);
}

void cairo_dock_set_quick_info_full (cairo_t *pSourceContext, Icon *pIcon, CairoDock *pDock, const gchar *cQuickInfoFormat, ...)
{
	va_list args;
	va_start (args, cQuickInfoFormat);
	gchar *cFullText = g_strdup_vprintf (cQuickInfoFormat, args);
	cairo_dock_set_quick_info (pSourceContext, cFullText, pIcon, (pDock->iType == CAIRO_DOCK_DOCK ? 1 + g_fAmplitude : 1));
	g_free (cFullText);
	va_end (args);
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
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_free (cThemesDirPath);
	
	gchar *cThemePath = NULL;
	if (pThemeTable != NULL)
	{
		cairo_dock_update_conf_file_with_hash_table (cAppletConfFilePath, pThemeTable, cGroupName, cKeyName, NULL, (GHFunc) cairo_dock_write_one_theme_name, TRUE, FALSE);
		
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

