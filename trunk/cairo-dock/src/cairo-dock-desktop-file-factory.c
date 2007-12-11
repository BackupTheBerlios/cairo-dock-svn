/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include <gtk/gtk.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-icons.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-config.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-desktop-file-factory.h"

extern GHashTable *g_hDocksTable;
extern gchar *g_cCurrentThemePath;
extern gchar *g_cCurrentLaunchersPath;
extern gchar *g_cLanguage;


gchar *cairo_dock_add_desktop_file_from_uri (gchar *cURI, const gchar *cDockName, double fOrder, CairoDock *pDock, GError **erreur)
{
	g_print ("%s (%s)\n", __func__, cURI);
	double fEffectiveOrder;
	if (fOrder == CAIRO_DOCK_LAST_ORDER)
	{
		Icon *pLastIcon = cairo_dock_get_last_launcher (pDock->icons);
		if (pLastIcon != NULL)
			fEffectiveOrder = pLastIcon->fOrder + 1;
		else
			fEffectiveOrder = 1;
	}
	else
		fEffectiveOrder = fOrder;
	
	GError *tmp_erreur = NULL;
	gchar *cNewDesktopFileName = NULL;
	
	//\_________________ On regarde si c'est un repertoire ou un fichier ou sinon un fichier cree a partir de zero.
	if (cURI == NULL)
	{
		//\___________________ On ouvre le patron.
		gchar *cDesktopFileTemplate = cairo_dock_get_translated_conf_file_path (CAIRO_DOCK_LAUNCHER_CONF_FILE, CAIRO_DOCK_SHARE_DATA_DIR);
		
		GKeyFile *pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, cDesktopFileTemplate, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &tmp_erreur);
		g_free (cDesktopFileTemplate);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
		
		//\___________________ On renseigne ce qu'on peut.
		g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", fEffectiveOrder);
		g_key_file_set_string (pKeyFile, "Desktop Entry", "Container", cDockName);
		g_key_file_set_string (pKeyFile, "Desktop Entry", "Exec", "echo 'edit me !'");
		
		//\___________________ On lui choisit un nom de fichier tel qu'il n'y ait pas de collision.
		cNewDesktopFileName = cairo_dock_generate_desktop_filename ("launcher.desktop", g_cCurrentLaunchersPath);
		
		//\___________________ On ecrit tout.
		gchar *cNewDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, cNewDesktopFileName);
		cairo_dock_write_keys_to_file (pKeyFile, cNewDesktopFilePath);
		g_free (cNewDesktopFilePath);
		g_key_file_free (pKeyFile);
	}
	else if (g_str_has_suffix (cURI, ".desktop") && (strncmp (cURI, "file://", 7) == 0 || *cURI == '/'))
	{
		gchar *cFilePath = (*cURI == '/' ? cURI : cURI + 7);  // on saute le "file://".
		gchar *cBaseName = g_path_get_basename (cFilePath);
		cNewDesktopFileName = cairo_dock_generate_desktop_filename (cBaseName, g_cCurrentLaunchersPath);
		g_free (cBaseName);
		
		//\___________________ On ouvre le fichier .desktop pour lui rajouter nos champs.
		GKeyFile *pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, cFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
		g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", fEffectiveOrder);
		g_key_file_set_string (pKeyFile, "Desktop Entry", "Container", cDockName);
		g_key_file_set_boolean (pKeyFile, "Desktop Entry", "Is container", FALSE);
		g_key_file_set_boolean (pKeyFile, "Desktop Entry", "Is URI", FALSE);
		
		//\___________________ On elimine les indesirables.
		g_key_file_remove_key (pKeyFile, "Desktop Entry", "X-Ubuntu-Gettext-Domain", NULL);
		GError *erreur = NULL;
		gchar *cCommand = g_key_file_get_string (pKeyFile, "Desktop Entry", "Exec", &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : invalid desktop file (%s)\n", erreur->message);
			g_error_free (erreur);
			g_key_file_free (pKeyFile);
			g_free (cNewDesktopFileName);
			return NULL;
		}
		gchar *str = strchr (cCommand, '%');
		if (str != NULL)
		{
			*str = '\0';
			g_key_file_set_string (pKeyFile, "Desktop Entry", "Exec", cCommand);
		}
		g_free (cCommand);
		
		gchar *cIconName = g_key_file_get_string (pKeyFile, "Desktop Entry", "Icon", &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : invalid desktop file (%s)\n", erreur->message);
			g_error_free (erreur);
			g_key_file_free (pKeyFile);
			g_free (cNewDesktopFileName);
			return NULL;
		}
		if (g_str_has_suffix (cIconName, ".png") || g_str_has_suffix (cIconName, ".xpm"))  // on prefere les svg si possible.
		{
			cIconName[strlen(cIconName) - 4] = '\0';
			g_key_file_set_string (pKeyFile, "Desktop Entry", "Icon", cIconName);
		}
		g_free (cIconName);
		
		//\___________________ On ecrit tout ca dans un fichier.
		gchar *cNewDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, cNewDesktopFileName);
		cairo_dock_write_keys_to_file (pKeyFile, cNewDesktopFilePath);
		
		gchar *cDesktopFileTemplate = cairo_dock_get_translated_conf_file_path (CAIRO_DOCK_LAUNCHER_CONF_FILE, CAIRO_DOCK_SHARE_DATA_DIR);
		cairo_dock_apply_translation_on_conf_file (cNewDesktopFilePath, cDesktopFileTemplate);  // ecrit tous les commentaires utiles.
		g_free (cDesktopFileTemplate);
		g_free (cNewDesktopFilePath);
		g_key_file_free (pKeyFile);
	}
	
	return cNewDesktopFileName;
}


gchar *cairo_dock_generate_desktop_filename (gchar *cBaseName, gchar *cCairoDockDataDir)
{
	int iPrefixNumber = 0;
	GString *sFileName = g_string_new ("");
	
	do
	{
		iPrefixNumber ++;
		g_string_printf (sFileName, "%s/%02d%s", cCairoDockDataDir, iPrefixNumber, cBaseName);
	} while (iPrefixNumber < 99 && g_file_test (sFileName->str, G_FILE_TEST_EXISTS));
	
	g_string_free (sFileName, TRUE);
	if (iPrefixNumber == 99)
		return NULL;
	else
		return g_strdup_printf ("%02d%s", iPrefixNumber, cBaseName);
}


void cairo_dock_update_launcher_desktop_file (gchar *cDesktopFilePath, gchar *cLanguage)
{
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	if (cairo_dock_conf_file_needs_update (pKeyFile))
		cairo_dock_flush_conf_file_full (pKeyFile, cDesktopFilePath, CAIRO_DOCK_SHARE_DATA_DIR, FALSE, CAIRO_DOCK_LAUNCHER_CONF_FILE);
	/**gchar *cCommand = g_strdup_printf ("/bin/cp %s/launcher-%s.conf %s\n", CAIRO_DOCK_SHARE_DATA_DIR, cLanguage, cDesktopFilePath);
	system (cCommand);
	g_free (cCommand);
	
	cairo_dock_replace_values_in_conf_file (cDesktopFilePath, pKeyFile, FALSE, 0);
	g_key_file_free (pKeyFile);*/
	
	cairo_dock_update_conf_file_with_hash_table (cDesktopFilePath, g_hDocksTable, "Desktop Entry", "Container", NULL, (GHFunc)cairo_dock_write_one_name);
	cairo_dock_update_launcher_conf_file_with_renderers (cDesktopFilePath);
}


gchar *cairo_dock_get_launcher_template_conf_file (void)
{
	return cairo_dock_get_translated_conf_file_path (CAIRO_DOCK_LAUNCHER_CONF_FILE, CAIRO_DOCK_SHARE_DATA_DIR);
}
