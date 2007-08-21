/***********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

************************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <glib/gstdio.h>


#include "cairo-dock-applet-factory.h"
#include "cairo-dock-config.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-themes-manager.h"

#define CAIRO_DOCK_REMEMBER_THEME_FILE ".cairo-dock.last"

extern gchar *g_cCairoDockDataDir;
extern gchar *g_cConfFile;
extern gchar *g_cCurrentThemePath;

extern CairoDock *g_pMainDock;
extern int g_iWmHint;
extern GHashTable *g_hModuleTable;

static void _cairo_dock_write_one_theme_name (gchar *cThemeName, gchar *cThemePath, GString *sThemeNames)
{
	g_string_append_printf (sThemeNames, "%s;", cThemeName);
}
gchar *cairo_dock_edit_themes (gchar *cLanguage, GHashTable **hThemeTable)
{
	//\___________________ On recupere la liste des themes existant (pre-installes et utilisateur).
	GError *erreur = NULL;
	gchar *cThemesDir = CAIRO_DOCK_SHARE_THEMES_DIR;
	*hThemeTable = cairo_dock_list_themes (cThemesDir, NULL, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	cThemesDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR);
	*hThemeTable = cairo_dock_list_themes (cThemesDir, *hThemeTable, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	GHashTable *hUserThemeTable = cairo_dock_list_themes (cThemesDir, NULL, NULL);
	g_free (cThemesDir);
	gchar *cCurrentThemeName = (g_cCurrentThemePath != NULL ? g_path_get_basename (g_cCurrentThemePath) : NULL);
	if (cCurrentThemeName != NULL)
		g_hash_table_remove (hUserThemeTable, cCurrentThemeName);
	
	GString *sThemeNames = g_string_new ("");
	g_hash_table_foreach (hUserThemeTable, (GHFunc) _cairo_dock_write_one_theme_name, sThemeNames);
	sThemeNames->str[sThemeNames->len-1] = '\0';
	
	
	//\___________________ On met a jour le fichier de conf.
	const gchar *cTmpDir = g_get_tmp_dir ();
	gchar *cTmpConfFile = g_strdup_printf ("%s/cairo-dock-init", cTmpDir);
	gchar *cCommand = g_strdup_printf ("cp %s/themes-%s.conf %s", CAIRO_DOCK_SHARE_DATA_DIR, cLanguage, cTmpConfFile);
	system (cCommand);
	g_free (cCommand);
	
	
	cairo_dock_update_conf_file_with_hash_table (cTmpConfFile, *hThemeTable, "Themes", "chosen theme", 1, NULL);
	cairo_dock_update_conf_file_with_hash_table (cTmpConfFile, hUserThemeTable, "Delete", "wanted themes", 999, NULL);
	g_hash_table_destroy (hUserThemeTable);
	
	
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cTmpConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	g_key_file_set_string (pKeyFile, "Themes", "chosen theme", (cCurrentThemeName != NULL ? cCurrentThemeName : "default"));
	g_key_file_set_string (pKeyFile, "Rename", "theme name", (cCurrentThemeName != NULL ? cCurrentThemeName : ""));
	g_free (cCurrentThemeName);
	g_key_file_set_string (pKeyFile, "Delete", "wanted themes", sThemeNames->str);
	g_string_free (sThemeNames, TRUE);
	
	
	cairo_dock_write_keys_to_file (pKeyFile, cTmpConfFile);
	g_key_file_free (pKeyFile);
	
	//\___________________ On laisse l'utilisateur l'editer.
	gboolean bChoiceOK = cairo_dock_edit_conf_file (NULL, cTmpConfFile, "Manage themes", 400, 400, TRUE, NULL, NULL, NULL);
	if (! bChoiceOK)
	{
		g_free (cTmpConfFile);
		cTmpConfFile = NULL;
	}
	return cTmpConfFile;
}

gchar *cairo_dock_get_chosen_theme (gchar *cConfFile)
{
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	
	g_key_file_load_from_file (pKeyFile, cConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	gchar *cThemeName = g_key_file_get_string (pKeyFile, "Themes", "chosen theme", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	if (cThemeName != NULL && *cThemeName == '\0')
	{
		g_free (cThemeName);
		cThemeName = NULL;
	}
	
	g_key_file_free (pKeyFile);
	return cThemeName;
}


gchar *cairo_dock_get_theme_path (gchar *cThemeName, GHashTable *hThemeTable)
{
	if (cThemeName == NULL)
		return NULL;
	gchar *cUserThemePath;
	const gchar *cThemePath = NULL;
	if (hThemeTable != NULL)
	{
		cThemePath = g_hash_table_lookup (hThemeTable, cThemeName);
		g_return_val_if_fail (cThemePath != NULL && g_file_test (cThemePath, G_FILE_TEST_IS_DIR), NULL);
	}
	
	cUserThemePath = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR, cThemeName);
	if (! g_file_test (cUserThemePath, G_FILE_TEST_EXISTS) && cThemePath != NULL)
	{
		gchar *cCommand = g_strdup_printf ("cp -r %s %s", cThemePath, cUserThemePath);
		system (cCommand);
		g_free (cCommand);
	}
	
	gchar *cRememberFileName = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_REMEMBER_THEME_FILE);
	g_file_set_contents (cRememberFileName,
		cThemeName,
		-1,
		NULL);
	g_free (cRememberFileName);
	
	return cUserThemePath;
}

gchar *cairo_dock_load_theme (gchar *cThemePath)
{
	g_return_val_if_fail (cThemePath != NULL && g_file_test (cThemePath, G_FILE_TEST_IS_DIR), NULL);
	
	gchar *cConfFile = g_strdup_printf ("%s/%s", cThemePath, CAIRO_DOCK_CONF_FILE);
	
	//\___________________ On libere toute la memoire allouee pour les docks (stoppe aussi tous les threads).
	cairo_dock_free_all_docks (g_pMainDock);
	
	//\___________________ On cree le dock principal.
	g_pMainDock = cairo_dock_create_new_dock (g_iWmHint, CAIRO_DOCK_MAIN_DOCK_NAME);
	g_pMainDock->bIsMainDock = TRUE;
	g_pMainDock->iRefCount --;
	
	//\___________________ On lit son fichier de conf et on charge tout.
	cairo_dock_update_conf_file_with_modules (cConfFile, g_hModuleTable);
	cairo_dock_update_conf_file_with_translations (cConfFile, CAIRO_DOCK_SHARE_DATA_DIR);
	
	cairo_dock_read_conf_file (cConfFile, g_pMainDock);  // chargera des valeurs par defaut si le fichier de conf fourni est incorrect.
	return cConfFile;
}


gchar *cairo_dock_get_last_theme_name (gchar *cCairoDockDataDir)
{
	gchar *cRememberFileName = g_strdup_printf ("%s/%s", cCairoDockDataDir, CAIRO_DOCK_REMEMBER_THEME_FILE);
	gchar *cThemeName = NULL;
	GError *erreur = NULL;
	g_file_get_contents (cRememberFileName,
		&cThemeName,
		NULL,
		&erreur);
	g_free (cRememberFileName);
	
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	return cThemeName;
}


gchar *cairo_dock_ask_initial_theme (void)
{
	gchar *cCurrentThemePath = NULL;
	
	GHashTable *hThemeTable = NULL;
	gchar *cInitConfFile = cairo_dock_edit_themes ("en", &hThemeTable);
	
	if (cInitConfFile != NULL)
	{
		gchar *cThemeName = cairo_dock_get_chosen_theme (cInitConfFile);
		
		cCurrentThemePath = cairo_dock_get_theme_path (cThemeName, hThemeTable);
		
		g_free (cThemeName);
	}
	
	g_free (cInitConfFile);
	g_hash_table_destroy (hThemeTable);
	return cCurrentThemePath;
}

static void _cairo_dock_delete_one_theme (gchar *cThemeName, gchar *cThemePath, gpointer *data)
{
	gchar **cThemesList = data[0];
	GtkWidget *pWidget = data[1];
	gchar *cWantedThemeName;
	int i = 0;
	while (cThemesList[i] != NULL)
	{
		cWantedThemeName = cThemesList[i];
		if (strcmp (cWantedThemeName, cThemeName) == 0)
			break;
		i ++;
	}
	
	if (cThemesList[i] == NULL)  // le theme ne se trouve pas dans la liste des themes desires.
	{
		gchar *question = g_strdup_printf ("Are you sure you want to delete theme %s ?", cThemeName);
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			question);
		g_free (question);
		int answer = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (answer == GTK_RESPONSE_YES)
		{
			gchar *cCommand = g_strdup_printf ("rm -rf %s", cThemePath);
			system (cCommand);  // g_rmdir n'efface qu'un repertoire vide.
			g_free (cCommand);
		}
	}
}
void cairo_dock_manage_themes (GtkWidget *pWidget)
{
	GHashTable *hThemeTable = NULL;
	gchar *cInitConfFile = cairo_dock_edit_themes ("en", &hThemeTable);
	
	if (cInitConfFile != NULL)
	{
		GError *erreur = NULL;
		
		//\___________________ On recupere les donnees de l'IHM apres modification par l'utilisateur.
		GKeyFile *pKeyFile = g_key_file_new ();
		
		g_key_file_load_from_file (pKeyFile, cInitConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return ;
		}
		
		//\___________________ On charge le nouveau theme choisi.
		gchar *cNewThemeName = g_key_file_get_string (pKeyFile, "Themes", "chosen theme", &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur == NULL;
		}
		if (cNewThemeName != NULL && *cNewThemeName == '\0')
		{
			g_free (cNewThemeName);
			cNewThemeName = NULL;
		}
		
		gchar *cCurrentThemeName = g_path_get_basename (g_cCurrentThemePath);
		if (cNewThemeName != NULL && strcmp (cNewThemeName, cCurrentThemeName) != 0)
		{
			g_free (g_cCurrentThemePath);
			g_cCurrentThemePath = cairo_dock_get_theme_path (cNewThemeName, hThemeTable);
			g_cConfFile = cairo_dock_load_theme (g_cCurrentThemePath);
			
			g_free (cCurrentThemeName);
			g_free (cNewThemeName);
			g_free (cInitConfFile);
			g_hash_table_destroy (hThemeTable);
			return ;
		}
		g_free (cCurrentThemeName);
		g_free (cNewThemeName);
		
		//\___________________ On renomme le theme actuel 
		cNewThemeName = g_key_file_get_string (pKeyFile, "Rename", "theme name", &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur == NULL;
		}
		if (cNewThemeName != NULL && *cNewThemeName == '\0')
		{
			g_free (cNewThemeName);
			cNewThemeName = NULL;
		}
		
		cCurrentThemeName = g_path_get_basename (g_cCurrentThemePath);
		if (cNewThemeName != NULL && strcmp (cNewThemeName, cCurrentThemeName) != 0)
		{
			if (g_hash_table_lookup (hThemeTable, cNewThemeName) == NULL)
			{
				gchar *cNewThemePath = cairo_dock_get_theme_path (cNewThemeName, NULL);
				
				g_rename (g_cCurrentThemePath, cNewThemePath);
				
				g_free (g_cCurrentThemePath);
				g_cCurrentThemePath = cNewThemePath;
				g_free (g_cConfFile);
				g_cConfFile = g_strdup_printf ("%s/%s", cNewThemePath, CAIRO_DOCK_CONF_FILE);
			}
			else
			{
				g_print ("Attention : theme %s already exists, nothing's done.\n", cNewThemeName);
			}
		}
		g_free (cNewThemeName);
		g_free (cCurrentThemeName);
		
		//\___________________ On efface les themes qui ne sont plus desires, sauf le theme actuel.
		gsize length = 0;
		gchar ** cThemesList = g_key_file_get_string_list (pKeyFile, "Delete", "wanted themes", &length, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur == NULL;
		}
		if (cThemesList != NULL)
		{
			gchar *cThemesDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR);
			GHashTable *hUserThemeTable = cairo_dock_list_themes (cThemesDir, NULL, &erreur);
			g_free (cThemesDir);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			else
			{
				gchar *cCurrentThemeName = g_path_get_basename (g_cCurrentThemePath);
				g_hash_table_remove (hUserThemeTable, cCurrentThemeName);  // on n'efface pas le theme courant.
				g_free (cCurrentThemeName);
				
				gpointer data[2] = {cThemesList, pWidget};
				g_hash_table_foreach (hUserThemeTable, (GHFunc) _cairo_dock_delete_one_theme, data);
			}
			g_hash_table_destroy (hUserThemeTable);
		}
		g_strfreev (cThemesList);
		
		g_key_file_free (pKeyFile);
	}
	
	g_free (cInitConfFile);
	g_hash_table_destroy (hThemeTable);
}
