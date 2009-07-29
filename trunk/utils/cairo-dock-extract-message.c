/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

#define write_message(m) fprintf (f, "_(\"%s\")\n\n", g_strescape (m, NULL))

int
main (int argc, char** argv)
{
	if (argc < 2)
		g_error ("il manque le chemin du fichier !\n");
	gchar *cConfFilePath = argv[1];
	
	GKeyFile *pKeyFile = g_key_file_new ();
	
	GError *erreur = NULL;
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
		g_error ("%s\n", erreur->message);
	
	int iNbBuffers = 0;
	gsize length = 0;
	gchar **pKeyList;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	
	gchar *cGroupName, *cKeyName, *cKeyComment, *cUsefulComment, *cAuthorizedValuesChain, *pTipString, **pAuthorizedValuesList;
	int i, j, k, iNbElements;
	char iElementType;
	gboolean bIsAligned;
	gboolean bValue, *bValueList;
	int iValue, iMinValue, iMaxValue, *iValueList;
	double fValue, fMinValue, fMaxValue, *fValueList;
	gchar *cValue, **cValueList;
	
	gchar *cDirPath = g_path_get_dirname (cConfFilePath);
	gchar *cMessagesFilePath = g_strconcat (cDirPath, "/messages", NULL);
	FILE *f = fopen (cMessagesFilePath, "a");
	if (!f)
		g_error ("impossible d'ouvrir %s", cMessagesFilePath);
	
	i = 0;
	cGroupName = pGroupList[0];
	if (cGroupName != NULL && strcmp (cGroupName, "ChangeLog") == 0)
	{
		pKeyList = g_key_file_get_keys (pKeyFile, cGroupName, NULL, NULL);
		j = 0;
		while (pKeyList[j] != NULL)
		{
			cKeyName = pKeyList[j];
			cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, NULL);
			write_message (cValue);
			g_free (cValue);
			j ++;
		}
		g_strfreev (pKeyList);
	}
	else
	while (pGroupList[i] != NULL)
	{
		cGroupName = pGroupList[i];
		write_message (cGroupName);
		
		pKeyList = g_key_file_get_keys (pKeyFile, cGroupName, NULL, NULL);
		
		j = 0;
		while (pKeyList[j] != NULL)
		{
			cKeyName = pKeyList[j];

			cKeyComment =  g_key_file_get_comment (pKeyFile, cGroupName, cKeyName, NULL);
			//g_print ("%s -> %s\n", cKeyName, cKeyComment);
			if (cKeyComment != NULL && strcmp (cKeyComment, "") != 0)
			{
				cUsefulComment = cKeyComment;
				while (*cUsefulComment == '#' || *cUsefulComment == ' ')  // on saute les # et les espaces.
					cUsefulComment ++;

				iElementType = *cUsefulComment;
				cUsefulComment ++;

				if (! g_ascii_isdigit (*cUsefulComment) && *cUsefulComment != '[')
				{
					cUsefulComment ++;
				}
				
				if (g_ascii_isdigit (*cUsefulComment))
				{
					iNbElements = atoi (cUsefulComment);
					g_return_val_if_fail (iNbElements > 0, 1);
					while (g_ascii_isdigit (*cUsefulComment))
						cUsefulComment ++;
				}
				else
				{
					iNbElements = 1;
				}
				//g_print ("%d element(s)\n", iNbElements);

				while (*cUsefulComment == ' ')  // on saute les espaces.
					cUsefulComment ++;

				if (*cUsefulComment == '[')
				{
					cUsefulComment ++;
					cAuthorizedValuesChain = cUsefulComment;

					while (*cUsefulComment != '\0' && *cUsefulComment != ']')
						cUsefulComment ++;
					g_return_val_if_fail (*cUsefulComment != '\0', 1);
					*cUsefulComment = '\0';
					cUsefulComment ++;
					while (*cUsefulComment == ' ')  // on saute les espaces.
						cUsefulComment ++;

					pAuthorizedValuesList = g_strsplit (cAuthorizedValuesChain, ";", 0);
				}
				else
				{
					pAuthorizedValuesList = NULL;
				}
				if (cUsefulComment[strlen (cUsefulComment) - 1] == '\n')
					cUsefulComment[strlen (cUsefulComment) - 1] = '\0';
				if (cUsefulComment[strlen (cUsefulComment) - 1] == '/')
				{
					bIsAligned = FALSE;
					cUsefulComment[strlen (cUsefulComment) - 1] = '\0';
				}
				else
				{
					bIsAligned = TRUE;
				}
				//g_print ("cUsefulComment : %s\n", cUsefulComment);

				pTipString = strchr (cUsefulComment, '{');
				if (pTipString != NULL)
				{
					if (*(pTipString-1) == '\n')
						*(pTipString-1) ='\0';
					else
						*pTipString = '\0';

					pTipString ++;

					gchar *pTipEnd = strrchr (pTipString, '}');
					if (pTipEnd != NULL)
						*pTipEnd = '\0';
				}

				if (pTipString != NULL)
				{
					//g_print ("pTipString : '%s'\n", pTipString);
					write_message (pTipString);
				}

				if (*cUsefulComment != '\0' && strcmp (cUsefulComment, "...") != 0 && iElementType != 'F' && iElementType != 'X')
				{
					write_message (cUsefulComment);
				}
				
				switch (iElementType)
				{
					case 'b' :  // boolean
						
					break;

					case 'i' :  // integer
					case 'I' :  // integer dans un HScale
					case 'j' :  // integer par paire
						
						break;

					case 'f' :  // float.
					case 'c' :  // float avec un bouton de choix de couleur.
					case 'C' :
					case 'e' :  // float dans un HScale.
						
					break;
					
					case 'n' :
					case 'h' :
					case 'H' :
					case 'x' :
					case 'a' :
					case 't' :
					case 'o' :
					case 'O' :
					case 'g' :
					case 'd' :
					case 'm' :
					case 'M' :
					case '_' :
					case '>' :
						
					break;

					case 's' :  // string
					case 'S' :  // string avec un selecteur de fichier a cote du GtkEntry.
					case 'u' :  // string avec un selecteur de fichier a cote du GtkEntry et un boutton play.
					case 'D' :  // string avec un selecteur de repertoire a cote du GtkEntry.
					case 'T' :  // string, mais sans pouvoir decochez les cases.
					case 'E' :  // string, mais avec un GtkComboBoxEntry pour le choix unique.
					case 'R' :  // string, avec un label pour la description.
					case 'P' :  // string avec un selecteur de font a cote du GtkEntry.
					case 'r' :  // string representee par son numero dans une liste de choix.
					case 'K' :  // string avec un selecteur de touche clavier (Merci Ctaf !)
						//g_print ("  + string (%s)\n", cUsefulComment);
						length = 0;
						cValueList = g_key_file_get_locale_string_list (pKeyFile, cGroupName, cKeyName, NULL, &length, NULL);
						if (iNbElements == 1)
						{
							cValue =  (0 < length ? cValueList[0] : "");
							if (pAuthorizedValuesList == NULL || pAuthorizedValuesList[0] == NULL)
							{
								
							}
							else
							{
								k = 0;
								int iSelectedItem = -1;
								if (iElementType == 'r')
								iSelectedItem = atoi (cValue);
								gchar *cResult = (iElementType == 'r' ? g_new0 (gchar , 10) : NULL);
								while (pAuthorizedValuesList[k] != NULL)
								{
									//g_print ("%d) %s\n", k, pAuthorizedValuesList[k]);
									if (iSelectedItem == -1 && strcmp (cValue, pAuthorizedValuesList[k]) == 0)
										iSelectedItem = (iElementType == 'R' || iElementType == 'M' ? k / 3 : k);

									if (cResult != NULL)
									{
										snprintf (cResult, 10, "%d", k);
									}
									write_message (pAuthorizedValuesList[k]);
									
									if (iElementType == 'R' || iElementType == 'M')
									{
										k += 3;
										if (pAuthorizedValuesList[k-2] == NULL)  // ne devrait pas arriver si le fichier de conf est bien rempli.
											break;
									}
									else
									k ++;
								}
								g_free (cResult);
								
								if (iElementType != 'E' && iSelectedItem == -1)
									iSelectedItem = 0;
							}
						}
						g_strfreev (cValueList);
					break;

					case 'F' :
					case 'X' :
						//g_print ("  + frame\n");
						if (pAuthorizedValuesList == NULL)
						{
							
						}
						else
						{
							if (pAuthorizedValuesList[0] == NULL || *pAuthorizedValuesList[0] == '\0')
								cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, NULL);
							else
								cValue = pAuthorizedValuesList[0];
							write_message (cValue);
						}
						break;

					case 'v' :  // separateur.
					break ;

					case 'k' :
					break;
					
					default :
						g_print ("Attention : this conf file seems to be incorrect ! (%c)\n", iElementType);
					break ;
				}
				g_strfreev (pAuthorizedValuesList);
				g_free (cKeyComment);
			}

			j ++;
		}
		g_strfreev (pKeyList);

		i ++;
	}
	
	g_strfreev (pGroupList);
	fclose (f);
	return 0;
}
