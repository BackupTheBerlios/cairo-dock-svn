/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "cairo-dock-keyfile-manager.h"

extern gchar *g_cLanguage;


void cairo_dock_write_keys_to_file (GKeyFile *key_file, gchar *conf_file)
{
	GError *erreur = NULL;
	
	gchar *cDirectory = g_path_get_dirname (conf_file);
	if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE))
	{
		g_mkdir_with_parents (cDirectory, 7*8*8+7*8+5);
	}
	g_free (cDirectory);
	
	
	gsize length;
	gchar *new_conf_file = g_key_file_to_data (key_file, &length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Error while fetching data : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	g_file_set_contents (conf_file, new_conf_file, length, &erreur);
	if (erreur != NULL)
	{
		g_print ("Error while writing data : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
}

void cairo_dock_flush_conf_file (GKeyFile *pKeyFile, gchar *cConfFilePath, gchar *cShareDataDirPath)
{
	gchar *cConfFileName = g_path_get_basename (cConfFilePath);
	gchar *cBaseName = g_strdup (cConfFileName);
	gchar *cExtension = NULL;
	gchar *str = strrchr (cBaseName, '.');
	if (str != NULL)
	{
		cExtension = g_strdup (str);
		*str = '\0';
	}
	
	gchar *cTranslatedConfFilePath = g_strdup_printf ("%s/%s-%s%s", cShareDataDirPath, cBaseName, g_cLanguage, (cExtension != NULL ? cExtension : ""));
	if (! g_file_test (cTranslatedConfFilePath, G_FILE_TEST_EXISTS))
	{
		g_free (cTranslatedConfFilePath);
		cTranslatedConfFilePath = NULL;
		if (strcmp (g_cLanguage, "en") != 0)
		{
			cTranslatedConfFilePath = g_strdup_printf ("%s/%s-en%s", cShareDataDirPath, cBaseName, (cExtension != NULL ? cExtension : ""));
			if (! g_file_test (cTranslatedConfFilePath, G_FILE_TEST_EXISTS))
			{
				g_free (cTranslatedConfFilePath);
				cTranslatedConfFilePath = NULL;
			}
		}
		
		if (cTranslatedConfFilePath == NULL)
		{
			cTranslatedConfFilePath = g_strdup_printf ("%s/%s%s", cShareDataDirPath, cBaseName, (cExtension != NULL ? cExtension : ""));
			if (! g_file_test (cTranslatedConfFilePath, G_FILE_TEST_EXISTS))
			{
				g_free (cTranslatedConfFilePath);
				cTranslatedConfFilePath = NULL;
			}
		}
	}
	
	if (cTranslatedConfFilePath == NULL)
	{
		g_print ("Attention : couldn't find any installed conf file\n");
	}
	else
	{
		gchar *cCommand = g_strdup_printf ("/bin/cp %s %s", cTranslatedConfFilePath, cConfFilePath);
		system (cCommand);
		g_free (cCommand);
		g_free (cTranslatedConfFilePath);
		
		cairo_dock_replace_values_in_conf_file (cConfFilePath, pKeyFile, TRUE, 0);
	}
	
	g_free (cConfFileName);
	g_free (cBaseName);
	g_free (cExtension);
}

void cairo_dock_replace_comments (GKeyFile *pOriginalKeyFile, GKeyFile *pReplacementKeyFile)
{
	GError *erreur = NULL;
	gsize length = 0;
	gchar **pKeyList;
	gchar **pGroupList = g_key_file_get_groups (pReplacementKeyFile, &length);
	gchar *cGroupName, *cKeyName, *cKeyComment;
	int i, j;
	
	cKeyComment =  g_key_file_get_comment (pReplacementKeyFile, NULL, NULL, NULL);
	if (cKeyComment != NULL && *cKeyComment != '\0')
	{
		g_key_file_set_comment (pOriginalKeyFile, NULL, NULL, cKeyComment, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
	}
	g_free (cKeyComment);
	
	i = 0;
	while (pGroupList[i] != NULL)
	{
		cGroupName = pGroupList[i];
		
		length = 0;
		pKeyList = g_key_file_get_keys (pReplacementKeyFile, cGroupName, NULL, NULL);
		
		j = 0;
		while (pKeyList[j] != NULL)
		{
			cKeyName = pKeyList[j];
			
			cKeyComment =  g_key_file_get_comment (pReplacementKeyFile, cGroupName, cKeyName, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			else if (cKeyComment != NULL && *cKeyComment != '\0')
			{
				if (cKeyComment[strlen(cKeyComment) - 1] == '\n')
					cKeyComment[strlen(cKeyComment) - 1] = '\0';
				g_key_file_set_comment (pOriginalKeyFile, cGroupName, cKeyName, cKeyComment, &erreur);
				if (erreur != NULL)
				{
					g_print ("Attention : %s\n", erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
			}
			g_free (cKeyComment);
			j ++;
		}
		i ++;
	}
}


void cairo_dock_replace_key_values (GKeyFile *pOriginalKeyFile, GKeyFile *pReplacementKeyFile, gboolean bUseOriginalKeys, gchar iIdentifier)
{
	g_print ("%s (%d)\n", __func__, iIdentifier);
	GError *erreur = NULL;
	gsize length = 0;
	gchar **pKeyList;
	gchar **pGroupList = g_key_file_get_groups ((bUseOriginalKeys ? pOriginalKeyFile : pReplacementKeyFile), &length);
	gchar *cGroupName, *cKeyName, *cKeyValue, *cComment;
	int i, j;
	
	i = 0;
	while (pGroupList[i] != NULL)
	{
		cGroupName = pGroupList[i];
		
		length = 0;
		pKeyList = g_key_file_get_keys ((bUseOriginalKeys ? pOriginalKeyFile : pReplacementKeyFile), cGroupName, NULL, NULL);
		
		j = 0;
		while (pKeyList[j] != NULL)
		{
			cKeyName = pKeyList[j];
			
			if (iIdentifier != 0)
			{
				cComment = g_key_file_get_comment (pReplacementKeyFile, cGroupName, cKeyName, NULL);
				
				if (cComment == NULL || strlen (cComment) < 2 || cComment[1] != iIdentifier)
				{
					//g_print ("  on saute %s;%s (%s)\n", cGroupName, cKeyName, cComment);
					g_free (cComment);
					j ++;
					continue ;
				}
				g_free (cComment);
			}
			
			cKeyValue =  g_key_file_get_string (pReplacementKeyFile, cGroupName, cKeyName, &erreur);
			if (erreur != NULL)
			{
				g_print ("Attention : %s\n", erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			else
			{
				if (cKeyValue[strlen(cKeyValue) - 1] == '\n')
					cKeyValue[strlen(cKeyValue) - 1] = '\0';
				g_key_file_set_string (pOriginalKeyFile, cGroupName, cKeyName, (cKeyValue != NULL ? cKeyValue : ""));
			}
			g_free (cKeyValue);
			j ++;
		}
		
		g_strfreev (pKeyList);
		i ++;
	}
	g_strfreev (pGroupList);
}



void cairo_dock_write_one_name (gchar *cName, gpointer value, GString *pString)
{
	g_string_append_printf (pString, "%s;", cName);
}
void cairo_dock_write_one_name_description (gchar *cName, gchar *cDescriptionFilePath, GString *pString)
{
	g_string_append_printf (pString, "%s;%s;", cName, cDescriptionFilePath);
}
void cairo_dock_write_one_module_name (gchar *cName, CairoDockModule *pModule, GString *pString)
{
	g_string_append_printf (pString, "%s;%s;", cName, (pModule->cReadmeFilePath != NULL ? pModule->cReadmeFilePath : "none"));
}
void cairo_dock_write_one_theme_name (gchar *cName, gchar *cThemePath, GString *pString)
{
	g_string_append_printf (pString, "%s;%s/readme;", cName, cThemePath);
}
void cairo_dock_write_one_renderer_name (gchar *cName, CairoDockRenderer *pRenderer, GString *pString)
{
	g_string_append_printf (pString, "%s;%s;", cName, (pRenderer->cReadmeFilePath != NULL ? pRenderer->cReadmeFilePath : "none"));
}

void cairo_dock_update_conf_file_with_hash_table (gchar *cConfFile, GHashTable *pModuleTable, gchar *cGroupName, gchar *cKeyName, gchar *cNewUsefullComment, GHFunc pWritingFunc)
{
	//g_print ("%s (%s)\n", __func__, cConfFile);
	GError *erreur = NULL;
	
	//\___________________ On ouvre le fichier de conf.
	GKeyFile *pKeyFile = g_key_file_new ();
	
	g_key_file_load_from_file (pKeyFile, cConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	gchar *cUsefullComment;
	gchar *cOldComment = g_key_file_get_comment (pKeyFile, cGroupName, cKeyName, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_return_if_fail (cOldComment != NULL);
	cOldComment[strlen (cOldComment) - 1] = '\0';
	
	gchar *cPrefix= cOldComment;
	while (*cPrefix == ' ')
		cPrefix ++;
	gchar *str = strchr (cPrefix, '[');
	if (str != NULL)
	{
		cPrefix = g_strndup (cPrefix, str - cPrefix + 1);
	}
	else
	{
		cPrefix = g_strdup ("s99[");  // par defaut.
	}
	
	//\___________________ On recupere le commentaire explicatif.
	if (cNewUsefullComment == NULL)
	{
		cUsefullComment = strchr (cOldComment, ']');
		if (cUsefullComment == NULL)
		{
			cUsefullComment = cOldComment;
			while (*cUsefullComment == ' ')
				cUsefullComment ++;
		}
		if (*cUsefullComment != '\0')
			cUsefullComment ++;  // on saute le caractere de type ou le crochet.
		else
			cUsefullComment = NULL;
		if (cUsefullComment != NULL)
			cUsefullComment = g_strdup (cUsefullComment);
	}
	else
	{
		cUsefullComment = g_strdup (cNewUsefullComment);
	}
	g_free (cOldComment);
	
	//\___________________ On ecrit la liste des possibilites.
	GString *sComment = g_string_new (cPrefix);
	g_free (cPrefix);
	g_hash_table_foreach (pModuleTable, (pWritingFunc != NULL ? pWritingFunc : (GHFunc) cairo_dock_write_one_name), sComment);
	if (sComment->str[sComment->len-1] == ';')  // peut etre faux si aucune valeur n'a ete ecrite.
		sComment->len --;
	g_string_append_printf (sComment, "] %s", (cUsefullComment != NULL ? cUsefullComment : ""));
	
	g_key_file_set_comment (pKeyFile, cGroupName, cKeyName, sComment->str, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_string_free (sComment, TRUE);
	
	cairo_dock_write_keys_to_file (pKeyFile, cConfFile);
	g_key_file_free (pKeyFile);
	g_free (cUsefullComment);
}

void cairo_dock_apply_translation_on_conf_file (gchar *cConfFilePath, gchar *cTranslatedConfFilePath)
{
	GError *erreur = NULL;
	
	GKeyFile *pConfKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pConfKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	GKeyFile *pTranslatedKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pTranslatedKeyFile, cTranslatedConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		g_key_file_free (pConfKeyFile);
		return ;
	}
	
	cairo_dock_replace_comments (pConfKeyFile, pTranslatedKeyFile);
	
	cairo_dock_write_keys_to_file (pConfKeyFile, cConfFilePath);
	
	g_key_file_free (pConfKeyFile);
	g_key_file_free (pTranslatedKeyFile);
}

void cairo_dock_replace_values_in_conf_file (gchar *cConfFilePath, GKeyFile *pValidKeyFile, gboolean bUseFileKeys, gchar iIdentifier)
{
	GKeyFile *pConfKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pConfKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	cairo_dock_replace_key_values (pConfKeyFile, pValidKeyFile, bUseFileKeys, iIdentifier);
	
	cairo_dock_write_keys_to_file (pConfKeyFile, cConfFilePath);
	
	g_key_file_free (pConfKeyFile);
}

void cairo_dock_replace_keys_by_identifier (gchar *cConfFilePath, gchar *cReplacementConfFilePath, gchar iIdentifier)
{
	//g_print ("%s (%s <- %s, '%c')\n", __func__, cConfFilePath, cReplacementConfFilePath, iIdentifier);
	GError *erreur = NULL;
	GKeyFile *pReplacementKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pReplacementKeyFile, cReplacementConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	cairo_dock_replace_values_in_conf_file (cConfFilePath, pReplacementKeyFile, TRUE, iIdentifier);
	
	g_key_file_free (pReplacementKeyFile);
}



GHashTable *cairo_dock_list_available_translations (gchar *cTranslationsDir, gchar *cFilePrefix, GError **erreur)
{
	g_return_val_if_fail (cFilePrefix != NULL, NULL);
	GError *tmp_erreur = NULL;
	GDir *dir = g_dir_open (cTranslationsDir, 0, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	GHashTable *pTranslationTable = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
	
	int iPrefixLength = strlen (cFilePrefix);
	const gchar* cFileName;
	gchar *cLanguage;
	gchar *cFilePath;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		if (g_str_has_suffix (cFileName, ".conf") && strncmp (cFileName, cFilePrefix, iPrefixLength) == 0)
		{
			cFilePath = g_strdup_printf ("%s/%s", cTranslationsDir, cFileName);
			
			cLanguage = g_strdup (cFileName + iPrefixLength);
			cLanguage[strlen (cLanguage) - 5] = '\0';
			
			g_hash_table_insert (pTranslationTable, cLanguage, cFilePath);
		}
	}
	while (1);
	g_dir_close (dir);
	
	return pTranslationTable;
}

void cairo_dock_get_conf_file_language_and_version (GKeyFile *pKeyFile, gchar **cConfFileLanguage, gchar **cConfFileVersion)
{
	*cConfFileLanguage = NULL;
	*cConfFileVersion = NULL;
	
	gchar *cFirstComment =  g_key_file_get_comment (pKeyFile, NULL, NULL, NULL);
	if (cFirstComment != NULL && *cFirstComment == '!')
	{
		gchar *str = strchr (cFirstComment, ';');
		if (str != NULL)
		{
			*str = '\0';
			if (cConfFileVersion != NULL)
			{
				gchar *str2 = strchr (str+1, '\n');
				if (str2 == NULL)
					strchr (str+1, ';');
				if (str2 != NULL)
					*str2 = '\0';
				*cConfFileVersion = g_strdup (str+1);
			}
		}
		else
		{
			int iStringLenght = strlen (cFirstComment);
			if (cFirstComment[iStringLenght-1] == '\n')
				cFirstComment[iStringLenght-1] = '\0';
		}
		if (cConfFileLanguage != NULL)
			*cConfFileLanguage = g_strdup (cFirstComment+1);
	}
	g_free (cFirstComment);
}
