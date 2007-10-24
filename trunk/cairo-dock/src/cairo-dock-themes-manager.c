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
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-themes-manager.h"

//#define CAIRO_DOCK_REMEMBER_THEME_FILE ".cairo-dock.last"
#define CAIRO_DOCK_MODIFIED_THEME_FILE ".cairo-dock-need-save"

extern gchar *g_cCairoDockDataDir;
extern gchar *g_cConfFile;
extern gchar *g_cCurrentThemePath;
extern gchar *g_cCurrentLaunchersPath;
extern gchar *g_cLanguage;

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
	g_hash_table_insert (*hThemeTable, g_strdup (""), g_strdup (""));
	
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
	
	GString *sThemeNames = g_string_new ("");
	g_hash_table_foreach (hUserThemeTable, (GHFunc) _cairo_dock_write_one_theme_name, sThemeNames);
	sThemeNames->str[sThemeNames->len-1] = '\0';
	
	
	//\___________________ On met a jour le fichier de conf.
	const gchar *cTmpDir = g_get_tmp_dir ();
	gchar *cTmpConfFile = g_strdup_printf ("%s/cairo-dock-init", cTmpDir);
	gchar *cCommand = g_strdup_printf ("cp %s/themes-%s.conf %s", CAIRO_DOCK_SHARE_DATA_DIR, cLanguage, cTmpConfFile);
	system (cCommand);
	g_free (cCommand);
	
	
	cairo_dock_update_conf_file_with_hash_table (cTmpConfFile, *hThemeTable, "Themes", "chosen theme", NULL, (GHFunc) cairo_dock_write_one_theme_name);
	cairo_dock_update_conf_file_with_hash_table (cTmpConfFile, hUserThemeTable, "Delete", "wanted themes", NULL, (GHFunc) cairo_dock_write_one_name);
	//g_hash_table_insert (hUserThemeTable, g_strdup (""), g_strdup (""));
	cairo_dock_update_conf_file_with_hash_table (cTmpConfFile, hUserThemeTable, "Save", "theme name", NULL, (GHFunc) cairo_dock_write_one_name);
	g_hash_table_destroy (hUserThemeTable);
	
	
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cTmpConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	g_key_file_set_string (pKeyFile, "Delete", "wanted themes", sThemeNames->str);
	g_string_free (sThemeNames, TRUE);
	
	
	cairo_dock_write_keys_to_file (pKeyFile, cTmpConfFile);
	g_key_file_free (pKeyFile);
	
	//\___________________ On laisse l'utilisateur l'editer.
	gchar *cPresentedGroup = (cairo_dock_theme_need_save () ? "Save" : NULL);
	gboolean bChoiceOK = cairo_dock_edit_conf_file (NULL, cTmpConfFile, "Manage themes", 600, 400, 0, cPresentedGroup, NULL, NULL, NULL);
	if (! bChoiceOK)
	{
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
	
	*bUseThemeBehaviour = g_key_file_get_boolean (pKeyFile, "Themes", "use theme behaviour", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		g_free (cThemeName);
		return NULL;
	}
	
	*bUseThemeLaunchers = g_key_file_get_boolean (pKeyFile, "Themes", "use theme launchers", &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
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
	
	//\___________________ On libere toute la memoire allouee pour les docks (stoppe aussi tous les threads).
	cairo_dock_free_all_docks (g_pMainDock);
	
	//\___________________ On cree le dock principal.
	g_pMainDock = cairo_dock_create_new_dock (g_iWmHint, CAIRO_DOCK_MAIN_DOCK_NAME);
	g_pMainDock->bIsMainDock = TRUE;
	
	//\___________________ On lit son fichier de conf et on charge tout.
	cairo_dock_update_conf_file_with_modules (g_cConfFile, g_hModuleTable);
	cairo_dock_update_conf_file_with_translations (g_cConfFile, CAIRO_DOCK_SHARE_DATA_DIR);
	
	gboolean bNeedSave = cairo_dock_theme_need_save ();
	cairo_dock_read_conf_file (g_cConfFile, g_pMainDock);  // chargera des valeurs par defaut si le fichier de conf fourni est incorrect.
	if (! bNeedSave)  // le chargement du fichier de conf le marque a 'TRUE'.
		cairo_dock_mark_theme_as_modified (bNeedSave);
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
	GError *erreur = NULL;
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
	gchar *cInitConfFile = cairo_dock_edit_themes ("en", &hThemeTable);
	
	if (cInitConfFile != NULL)
	{
		gboolean bUseThemeBehaviour, bUseThemeLaunchers;
		gchar *cThemeName = cairo_dock_get_chosen_theme (cInitConfFile, &bUseThemeBehaviour, &bUseThemeLaunchers);
		
		gchar *cThemePath = (cThemeName != NULL ? g_hash_table_lookup (hThemeTable, cThemeName) : NULL);
		g_return_val_if_fail (cThemePath != NULL, 0);
		
		gchar *cCommand = g_strdup_printf ("rm -rf %s/*", g_cCurrentThemePath);
		g_print ("%s\n", cCommand);
		system (cCommand);
		g_free (cCommand);
		
		cCommand = g_strdup_printf ("/bin/cp -r %s/* %s", cThemePath, g_cCurrentThemePath);
		g_print ("%s\n", cCommand);
		system (cCommand);
		g_free (cCommand);
		
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
			gchar *cCommand = g_strdup_printf ("rm -rf '%s'", cThemePath);
			system (cCommand);  // g_rmdir n'efface qu'un repertoire vide.
			g_free (cCommand);
		}
	}
}
gboolean cairo_dock_manage_themes (GtkWidget *pWidget)
{
	gchar *cCommand;
	GHashTable *hThemeTable = NULL;
	gchar *cInitConfFile = cairo_dock_edit_themes (g_cLanguage, &hThemeTable);
	
	if (cInitConfFile != NULL)
	{
		GError *erreur = NULL;
		gboolean bNeedSave = cairo_dock_theme_need_save ();
		
		//\___________________ On recupere les donnees de l'IHM apres modification par l'utilisateur.
		GKeyFile *pKeyFile = g_key_file_new ();
		
		g_key_file_load_from_file (pKeyFile, cInitConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return FALSE;
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
		
		if (cNewThemeName != NULL)
		{
			if (bNeedSave)
			{
				/*GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					"Discard changes in current theme and load this one ?");*/
				GtkWidget *dialog = gtk_dialog_new_with_buttons ("Confirm discarding changes",
					GTK_WINDOW (pWidget),
					GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					GTK_STOCK_SAVE,
					GTK_RESPONSE_NO,
					GTK_STOCK_APPLY,  // GTK_STOCK_DISCARD pour GTK 2.12
					GTK_RESPONSE_YES,
					NULL);
				
				GtkWidget *pHBox = gtk_hbox_new (FALSE, 3);
				gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), pHBox);
				GtkWidget *pImage = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
				GtkWidget *pQuestionLabel = gtk_label_new ("\nYou made some modifications in this currenttheme.\nSave it before or apply new theme ?");
				gtk_container_add (GTK_CONTAINER (pHBox), pImage);
				gtk_container_add (GTK_CONTAINER (pHBox), pQuestionLabel);
				gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);
				gtk_window_move (GTK_WINDOW (dialog), 500, 500);
				int answer = gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);
				if (answer != GTK_RESPONSE_YES)
				{
					g_free (cInitConfFile);
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
				cCommand = g_strdup_printf ("/bin/cp '%s'/%s '%s'", cNewThemePath, CAIRO_DOCK_CONF_FILE, g_cCurrentThemePath);
				g_print ("%s\n", cCommand);
				system (cCommand);
				g_free (cCommand);
			}
			else
			{
				gchar *cNewConfFilePath = g_strdup_printf ("%s/%s", cNewThemePath, CAIRO_DOCK_CONF_FILE);
				cairo_dock_replace_keys_by_identifier (g_cConfFile, cNewConfFilePath, '+');
				g_free (cNewConfFilePath);
			}
			//\___________________ On charge les lanceurs.
			if (g_key_file_get_boolean (pKeyFile, "Themes", "use theme launchers", NULL))
			{
				cCommand = g_strdup_printf ("rm -f '%s'/*", g_cCurrentLaunchersPath);
				g_print ("%s\n", cCommand);
				system (cCommand);
				g_free (cCommand);
				
				cCommand = g_strdup_printf ("cp '%s/%s'/* '%s'", cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR, g_cCurrentLaunchersPath);
				g_print ("%s\n", cCommand);
				system (cCommand);
				g_free (cCommand);
			}
			//\___________________ On remplace tous les autres fichiers par les nouveaux.
			cCommand = g_strdup_printf ("find '%s' -mindepth 1 -maxdepth 1  ! -name '*.conf' ! -name %s -exec rm -rf '{}' \\;", g_cCurrentThemePath, CAIRO_DOCK_LAUNCHERS_DIR);  // efface aussi les conf des plug-ins.
			g_print ("%s\n", cCommand);
			system (cCommand);
			g_free (cCommand);
			
			cCommand = g_strdup_printf ("find '%s'/* -prune ! -name '*.conf' ! -name %s -exec /bin/cp -r '{}' '%s' \\;", cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR, g_cCurrentThemePath);
			g_print ("%s\n", cCommand);
			system (cCommand);
			g_free (cCommand);
			
			//\___________________ On charge le theme courant.
			cairo_dock_load_theme (g_cCurrentThemePath);
			
			g_free (cNewThemeName);
			g_free (cInitConfFile);
			g_hash_table_destroy (hThemeTable);
			return FALSE;
		}
		g_free (cNewThemeName);
		
		//\___________________ On sauvegarde le theme actuel 
		cNewThemeName = g_key_file_get_string (pKeyFile, "Save", "theme name", &erreur);
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
		g_print ("cNewThemeName : %s\n", cNewThemeName);
		
		if (cNewThemeName != NULL)
		{
			g_print ("on sauvegarde dans %s\n", cNewThemeName);
			gboolean bThemeSaved = FALSE;
			gchar *cNewThemePath = g_hash_table_lookup (hThemeTable, cNewThemeName);
			if (cNewThemePath != NULL)  // on ecrase un theme existant.
			{
				g_print ("  theme existant\n");
				if (strncmp (cNewThemePath, CAIRO_DOCK_SHARE_THEMES_DIR, strlen (CAIRO_DOCK_SHARE_THEMES_DIR)) == 0)  // c'est un theme pre-installe.
				{
					g_print ("Can't overwrite pre-installed theme\n");
				}
				else
				{
					gchar *question = g_strdup_printf ("Are you sure you want to overwrite theme %s ?", cNewThemeName);
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
							cCommand = g_strdup_printf ("/bin/cp '%s' '%s'", g_cConfFile, cNewConfFilePath);
							g_print ("%s\n", cCommand);
							system (cCommand);
							g_free (cCommand);
						}
						else
						{
							cairo_dock_replace_keys_by_identifier (cNewConfFilePath, g_cConfFile, '+');
						}
						g_free (cNewConfFilePath);
						
						if (g_key_file_get_boolean (pKeyFile, "Save", "save current launchers", NULL))
						{
							cCommand = g_strdup_printf ("rm -f '%s/%s'/*", cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR);
							g_print ("%s\n", cCommand);
							system (cCommand);
							g_free (cCommand);
							
							cCommand = g_strdup_printf ("cp '%s'/* '%s/%s'", g_cCurrentLaunchersPath, cNewThemePath, CAIRO_DOCK_LAUNCHERS_DIR);
							g_print ("%s\n", cCommand);
							system (cCommand);
							g_free (cCommand);
						}
						
						cCommand = g_strdup_printf ("find '%s' -mindepth 1 -maxdepth 1  ! -name '*.conf' ! -name '%s' -exec /bin/cp -r '{}' '%s' \\;", g_cCurrentThemePath, CAIRO_DOCK_LAUNCHERS_DIR, cNewThemePath);
						g_print ("%s\n", cCommand);
						system (cCommand);
						g_free (cCommand);
						
						bThemeSaved = TRUE;
					}
				}
			}
			else
			{
				cNewThemePath = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR, cNewThemeName);
				g_print ("  nouveau theme (%s)\n", cNewThemePath);
				
				if (g_mkdir (cNewThemePath, 7*8*8+7*8+5) != 0)
					bThemeSaved = FALSE;
				else
				{
					cCommand = g_strdup_printf ("cp -r '%s'/* '%s'", g_cCurrentThemePath, cNewThemePath);
					g_print ("%s\n", cCommand);
					system (cCommand);
					g_free (cCommand);
					
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
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur == NULL;
		}
		else if (cThemesList != NULL)
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
	
	g_free (cInitConfFile);
	g_hash_table_destroy (hThemeTable);
	return FALSE;
}
