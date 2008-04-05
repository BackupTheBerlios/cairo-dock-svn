/***********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include "cairo-dock-applet-factory.h"
#include "cairo-dock-config.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-log.h"

#define CAIRO_DOCK_MODIFIED_THEME_FILE ".cairo-dock-need-save"
#define CAIRO_DOCK_THEME_PANEL_WIDTH 750
#define CAIRO_DOCK_THEME_PANEL_HEIGHT 400

extern gchar *g_cCairoDockDataDir;
extern gchar *g_cConfFile;
extern gchar *g_cCurrentThemePath;
extern gchar *g_cCurrentLaunchersPath;
extern gchar *g_cMainDockDefaultRendererName;

extern CairoDock *g_pMainDock;
extern int g_iWmHint;


GHashTable *cairo_dock_list_themes (gchar *cThemesDir, GHashTable *hProvidedTable, GError **erreur)
{
	GError *tmp_erreur = NULL;
	GDir *dir = g_dir_open (cThemesDir, 0, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}

	GHashTable *pThemeTable = (hProvidedTable != NULL ? hProvidedTable : g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free));

	const gchar* cThemeName;
	gchar *cThemePath;
	do
	{
		cThemeName = g_dir_read_name (dir);
		if (cThemeName == NULL)
			break ;

		cThemePath = g_strdup_printf ("%s/%s", cThemesDir, cThemeName);

		if (g_file_test (cThemePath, G_FILE_TEST_IS_DIR))
			g_hash_table_insert (pThemeTable, g_strdup (cThemeName), cThemePath);
	}
	while (1);
	g_dir_close (dir);

	return pThemeTable;
}


gchar *cairo_dock_edit_themes (GHashTable **hThemeTable)
{
	//\___________________ On recupere la liste des themes existant (pre-installes et utilisateur).
	GError *erreur = NULL;
	gchar *cThemesDir = CAIRO_DOCK_SHARE_THEMES_DIR;
	*hThemeTable = cairo_dock_list_themes (cThemesDir, NULL, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_hash_table_insert (*hThemeTable, g_strdup (""), g_strdup (""));

	cThemesDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR);
	*hThemeTable = cairo_dock_list_themes (cThemesDir, *hThemeTable, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}

	GHashTable *hUserThemeTable = cairo_dock_list_themes (cThemesDir, NULL, NULL);
	g_free (cThemesDir);

	gchar *cUserThemeNames = cairo_dock_write_table_content (hUserThemeTable, (GHFunc) cairo_dock_write_one_name, TRUE, FALSE);

	//\___________________ On cree un fichier de conf temporaire.
	const gchar *cTmpDir = g_get_tmp_dir ();
	gchar *cTmpConfFile = g_strdup_printf ("%s/cairo-dock-init", cTmpDir);

	gchar *cCommand = g_strdup_printf ("cp %s/%s %s", CAIRO_DOCK_SHARE_DATA_DIR, CAIRO_DOCK_THEME_CONF_FILE, cTmpConfFile);
	system (cCommand);
	g_free (cCommand);

	//\___________________ On met a jour ce fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cTmpConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	cairo_dock_update_conf_file_with_themes (pKeyFile, cTmpConfFile, *hThemeTable, "Themes", "chosen theme");
	cairo_dock_update_conf_file_with_themes (pKeyFile, cTmpConfFile, hUserThemeTable, "Delete", "wanted themes");
	cairo_dock_update_conf_file_with_themes (pKeyFile, cTmpConfFile, hUserThemeTable, "Save", "theme name");
	g_hash_table_destroy (hUserThemeTable);
	
	g_key_file_set_string (pKeyFile, "Delete", "wanted themes", cUserThemeNames);  // sThemeNames
	g_free (cUserThemeNames);

	cairo_dock_write_keys_to_file (pKeyFile, cTmpConfFile);
	g_key_file_free (pKeyFile);

	//\___________________ On laisse l'utilisateur l'editer.
	gchar *cPresentedGroup = (cairo_dock_theme_need_save () ? "Save" : NULL);
	gboolean bChoiceOK = cairo_dock_edit_conf_file (NULL, cTmpConfFile, "Manage themes", CAIRO_DOCK_THEME_PANEL_WIDTH, CAIRO_DOCK_THEME_PANEL_HEIGHT, 0, cPresentedGroup, NULL, NULL, NULL, NULL);
	if (! bChoiceOK)
	{
		g_remove (cTmpConfFile);
		g_free (cTmpConfFile);
		cTmpConfFile = NULL;
	}
	return cTmpConfFile;
}

gchar *cairo_dock_get_chosen_theme (gchar *cConfFile, gboolean *bUseThemeBehaviour, gboolean *bUseThemeLaunchers)
{
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();

	g_key_file_load_from_file (pKeyFile, cConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}

	gchar *cThemeName = g_key_file_get_string (pKeyFile, "Themes", "chosen theme", &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	if (cThemeName != NULL && *cThemeName == '\0')
	{
		g_free (cThemeName);
		cThemeName = NULL;
	}

	*bUseThemeBehaviour = g_key_file_get_boolean (pKeyFile, "Themes", "use theme behaviour", &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		g_free (cThemeName);
		return NULL;
	}

	*bUseThemeLaunchers = g_key_file_get_boolean (pKeyFile, "Themes", "use theme launchers", &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		g_free (cThemeName);
		return NULL;
	}

	g_key_file_free (pKeyFile);
	return cThemeName;
}


void cairo_dock_load_theme (gchar *cThemePath)
{
	//g_print ("%s (%s)\n", __func__, cThemePath);
	g_return_if_fail (cThemePath != NULL && g_file_test (cThemePath, G_FILE_TEST_IS_DIR));

	//\___________________ On libere toute la memoire allouee pour les docks (stoppe aussi tous les timeout).
	cairo_dock_free_all_docks (g_pMainDock);

	//\___________________ On cree le dock principal.
	g_pMainDock = cairo_dock_create_new_dock (g_iWmHint, CAIRO_DOCK_MAIN_DOCK_NAME, NULL);  // on ne lui assigne pas de vues, puisque la vue par defaut des docks principaux sera definie plus tard.
	g_pMainDock->bIsMainDock = TRUE;

	//\___________________ On lit son fichier de conf et on charge tout.
	cairo_dock_update_conf_file_with_available_modules2 (NULL, g_cConfFile);

	cairo_dock_read_conf_file (g_cConfFile, g_pMainDock);  // chargera des valeurs par defaut si le fichier de conf fourni est incorrect.
	cairo_dock_mark_theme_as_modified (FALSE);  // le chargement du fichier de conf le marque a 'TRUE'.
}


void cairo_dock_mark_theme_as_modified (gboolean bModified)
{
	gchar *cModifiedFile = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_MODIFIED_THEME_FILE);

	g_file_set_contents (cModifiedFile,
		(bModified ? "1" : "0"),
		-1,
		NULL);

	g_free (cModifiedFile);
}
gboolean cairo_dock_theme_need_save (void)
{
	gchar *cModifiedFile = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_MODIFIED_THEME_FILE);
	gsize length = 0;
	gchar *cContent = NULL;
	g_file_get_contents (cModifiedFile,
		&cContent,
		&length,
		NULL);
	g_free (cModifiedFile);
	gboolean bNeedSave;
	if (length > 0)
		bNeedSave = (*cContent == '1');
	else
		bNeedSave = FALSE;
	g_free (cContent);
	return bNeedSave;
}

int cairo_dock_ask_initial_theme (void)
{
	int iInitialChoiceOK = -1;
	GHashTable *hThemeTable = NULL;
	gchar *cInitConfFile = cairo_dock_edit_themes (&hThemeTable);

	if (cInitConfFile != NULL)
	{
		gboolean bUseThemeBehaviour, bUseThemeLaunchers;
		gchar *cThemeName = cairo_dock_get_chosen_theme (cInitConfFile, &bUseThemeBehaviour, &bUseThemeLaunchers);

		gchar *cThemePath = (cThemeName != NULL ? g_hash_table_lookup (hThemeTable, cThemeName) : NULL);
		g_return_val_if_fail (cThemePath != NULL, 0);

		gchar *cCommand = g_strdup_printf ("rm -rf %s/*", g_cCurrentThemePath);
		cd_message ("%s\n", cCommand);
		system (cCommand);
		g_free (cCommand);

		cCommand = g_strdup_printf ("/bin/cp -r %s/* %s", cThemePath, g_cCurrentThemePath);
		cd_message ("%s\n", cCommand);
		system (cCommand);
		g_free (cCommand);

		g_remove (cInitConfFile);

		g_free (cThemeName);
		g_free (cInitConfFile);
		iInitialChoiceOK = 1;
	}

	g_hash_table_destroy (hThemeTable);
	return iInitialChoiceOK;
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
		gchar *question = g_strdup_printf (_("Are you sure you want to delete theme %s ?"), cThemeName);
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
			gchar *cCommand = g_strdup_printf ("rm -rf '%s'", cThemePath);
			system (cCommand);  // g_rmdir n'efface qu'un repertoire vide.
			g_free (cCommand);
		}
	}
}
gboolean cairo_dock_manage_themes (GtkWidget *pWidget)
{
	GString *sCommand = g_string_new ("");
	GHashTable *hThemeTable = NULL;
	gchar *cInitConfFile = cairo_dock_edit_themes (&hThemeTable);

	if (cInitConfFile != NULL)
	{
		GError *erreur = NULL;
		gboolean bNeedSave = cairo_dock_theme_need_save ();

		//\___________________ On recupere les donnees de l'IHM apres modification par l'utilisateur.
		GKeyFile *pKeyFile = g_key_file_new ();

		g_key_file_load_from_file (pKeyFile, cInitConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		g_remove (cInitConfFile);
		g_free (cInitConfFile);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			return FALSE;
		}

		//\___________________ On charge le nouveau theme choisi.
		gchar *cNewThemeName = g_key_file_get_string (pKeyFile, "Themes", "chosen theme", &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		if (cNewThemeName != NULL && *cNewThemeName == '\0')
		{
			g_free (cNewThemeName);
			cNewThemeName = NULL;
		}

		if (cNewThemeName != NULL)
		{
			if (bNeedSave)
			{
				int iAnswer = cairo_dock_ask_general_question_and_wait (_("You made some modifications in the current theme.\nYou will loose them if you don't save before choosing a new theme. Continue anyway ?"));
				if (iAnswer != GTK_RESPONSE_YES)
				{
					g_hash_table_destroy (hThemeTable);
					return TRUE;
				}
				else
					cairo_dock_mark_theme_as_modified (FALSE);
			}

			gchar *cNewThemePath = g_hash_table_lookup (hThemeTable, cNewThemeName);
			//\___________________ On charge les parametres de comportement.
			if (g_key_file_get_boolean (pKeyFile, "Themes", "use theme behaviour", NULL))
			{
				g_string_printf (sCommand, "/bin/cp '%s'/%s '%s'", cNewThemePath, CAIRO_DOCK_CONF_FILE, g_cCurrentThemePath);
				cd_message ("%s", sCommand->str);
				system (sCommand->str);
			}
			else
			{
				gchar *cNewConfFilePath = g_strdup_printf ("%s/%s", cNewThemePath, CAIRO_DOCK_CONF_FILE);
				cairo_dock_replace_keys_by_identifier (g_cConfFile, cNewConfFilePath, '+');
				g_free (cNewConfFilePath);
			}
			//\___________________ On charge les icones.
			g_string_printf (sCommand, "find '%s' -mindepth 1 ! -name '*.desktop' ! -name 'container-*' -delete", g_cCurrentLaunchersPath);
			cd_message ("%s", sCommand->str);
			system (sCommand->str);
			g_string_printf (sCommand, "find '%s/%s' -mindepth 1 ! -name '*.desktop' -exec /bin/cp -p '{}' '%s' \\;", cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR, g_cCurrentLaunchersPath);
			cd_message ("%s", sCommand->str);
			system (sCommand->str);
			
			//\___________________ On charge les lanceurs si necessaire.
			if (g_key_file_get_boolean (pKeyFile, "Themes", "use theme launchers", NULL))
			{
				g_string_printf (sCommand, "rm -f '%s'/*.desktop", g_cCurrentLaunchersPath);
				cd_message ("%s", sCommand->str);
				system (sCommand->str);

				g_string_printf (sCommand, "cp '%s/%s'/*.desktop '%s'", cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR, g_cCurrentLaunchersPath);
				cd_message ("%s", sCommand->str);
				system (sCommand->str);
			}
			
			//\___________________ On remplace tous les autres fichiers par les nouveaux.
			g_string_printf (sCommand, "find '%s' -mindepth 1 -maxdepth 1  ! -name '*.conf' ! -name %s -exec rm -rf '{}' \\;", g_cCurrentThemePath, CAIRO_DOCK_LAUNCHERS_DIR);  // efface aussi les conf des plug-ins.
			cd_message ("%s", sCommand->str);
			system (sCommand->str);

			g_string_printf (sCommand, "find '%s'/* -prune ! -name '*.conf' ! -name %s -exec /bin/cp -r '{}' '%s' \\;", cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR, g_cCurrentThemePath);
			cd_message ("%s", sCommand->str);
			system (sCommand->str);

			//\___________________ On charge le theme courant.
			cairo_dock_load_theme (g_cCurrentThemePath);

			g_free (cNewThemeName);
			g_hash_table_destroy (hThemeTable);
			return FALSE;
		}
		g_free (cNewThemeName);

		//\___________________ On sauvegarde le theme actuel
		cNewThemeName = g_key_file_get_string (pKeyFile, "Save", "theme name", &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		if (cNewThemeName != NULL && *cNewThemeName == '\0')
		{
			g_free (cNewThemeName);
			cNewThemeName = NULL;
		}
		cd_message ("cNewThemeName : %s", cNewThemeName);

		if (cNewThemeName != NULL)
		{
			cd_message ("on sauvegarde dans %s", cNewThemeName);
			gboolean bThemeSaved = FALSE;
			gchar *cNewThemePath = g_hash_table_lookup (hThemeTable, cNewThemeName);
			if (cNewThemePath != NULL)  // on ecrase un theme existant.
			{
				cd_message ("  theme existant");
				if (strncmp (cNewThemePath, CAIRO_DOCK_SHARE_THEMES_DIR, strlen (CAIRO_DOCK_SHARE_THEMES_DIR)) == 0)  // c'est un theme pre-installe.
				{
					cd_warning ("You can't overwrite a pre-installed theme");
				}
				else
				{
					gchar *question = g_strdup_printf (_("Are you sure you want to overwrite theme %s ?"), cNewThemeName);
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
						gchar *cNewConfFilePath = g_strdup_printf ("%s/%s", cNewThemePath, CAIRO_DOCK_CONF_FILE);
						if (g_key_file_get_boolean (pKeyFile, "Save", "save current behaviour", NULL))
						{
							g_string_printf (sCommand, "/bin/cp '%s' '%s'", g_cConfFile, cNewConfFilePath);
							cd_message ("%s", sCommand->str);
							system (sCommand->str);
						}
						else
						{
							cairo_dock_replace_keys_by_identifier (cNewConfFilePath, g_cConfFile, '+');
						}
						g_free (cNewConfFilePath);

						if (g_key_file_get_boolean (pKeyFile, "Save", "save current launchers", NULL))
						{
							g_string_printf (sCommand, "rm -f '%s/%s'/*", cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR);
							cd_message ("%s", sCommand->str);
							system (sCommand->str);
							
							g_string_printf (sCommand, "cp '%s'/* '%s/%s'", g_cCurrentLaunchersPath, cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR);
							cd_message ("%s", sCommand->str);
							system (sCommand->str);
						}

						g_string_printf (sCommand, "find '%s' -mindepth 1 -maxdepth 1  ! -name '*.conf' ! -name '%s' -exec /bin/cp -r '{}' '%s' \\;", g_cCurrentThemePath, CAIRO_DOCK_LAUNCHERS_DIR, cNewThemePath);
						cd_message ("%s", sCommand->str);
						system (sCommand->str);

						bThemeSaved = TRUE;
					}
				}
			}
			else
			{
				cNewThemePath = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR, cNewThemeName);
				cd_message ("  nouveau theme (%s)", cNewThemePath);

				if (g_mkdir (cNewThemePath, 7*8*8+7*8+5) != 0)
					bThemeSaved = FALSE;
				else
				{
					g_string_printf (sCommand, "cp -r '%s'/* '%s'", g_cCurrentThemePath, cNewThemePath);
					cd_message ("%s", sCommand->str);
					system (sCommand->str);

					g_free (cNewThemePath);
					bThemeSaved = TRUE;
				}
			}
			if (bThemeSaved)
				cairo_dock_mark_theme_as_modified (FALSE);
		}

		//\___________________ On efface les themes qui ne sont plus desires.
		gsize length = 0;
		gchar ** cThemesList = g_key_file_get_string_list (pKeyFile, "Delete", "wanted themes", &length, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		else if (cThemesList != NULL)
		{
			gchar *cThemesDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR);
			GHashTable *hUserThemeTable = cairo_dock_list_themes (cThemesDir, NULL, &erreur);
			g_free (cThemesDir);
			if (erreur != NULL)
			{
				cd_warning ("Attention : %s", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			else
			{
				if (cNewThemeName != NULL)
					g_hash_table_remove (hUserThemeTable, cNewThemeName);  // pour ne pas effacer le theme qu'on vient d'enregistrer.
				gpointer data[2] = {cThemesList, pWidget};
				g_hash_table_foreach (hUserThemeTable, (GHFunc) _cairo_dock_delete_one_theme, data);
			}
			g_hash_table_destroy (hUserThemeTable);
		}
		g_strfreev (cThemesList);

		g_free (cNewThemeName);
		g_key_file_free (pKeyFile);
	}
	
	g_string_free (sCommand, TRUE);
	g_hash_table_destroy (hThemeTable);
	return FALSE;
}
