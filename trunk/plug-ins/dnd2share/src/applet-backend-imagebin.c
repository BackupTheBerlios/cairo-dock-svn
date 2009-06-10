#define _BSD_SOURCE
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-backend-imagebin.h"

#define API_KEY "rtrtXD3HF2MIAj7k6lrmRScDkTg3U5In"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {"DirectLink"};


static gboolean upload (const gchar *cFilePath, CDFileType iFileType)
{
	g_print ("%s (%s, %d)\n", __func__, cFilePath, iFileType);
	// On lance la commande d'upload.
	gchar *cLogFile = g_strdup ("/tmp/dnd2share-log.XXXXXX");
	int fds = mkstemp (cLogFile);
	if (fds == -1)
	{
		g_free (cLogFile);
		return ;
	}
	close(fds);
	
	gchar *cCommand = NULL;
	if (iFileType == CD_TYPE_TEXT)
	{
		// on remplace les espaces du texte par des %20.
		gchar **cTextParts = g_strsplit (cFilePath, " ", -1);
		GString *sContent = g_string_new_sized (strlen (cFilePath) + 300);  // 100 espaces d'avance.
		int i;
		for (i = 0; cTextParts[i] != NULL; i ++)
		{
			g_string_append (sContent, "%20");
			g_string_append (sContent, cTextParts[i]);
		}
		g_strfreev (cTextParts);
		cCommand = g_strdup_printf ("curl --connect-timeout 5 --retry 2 http://pastebin.ca/quiet-paste.php -F api=%s -F content=%s -F type=1 -F expiry=1%%20month -o %s", API_KEY, sContent->str, cLogFile);
		g_string_free (sContent, TRUE);
	}
	else
	{
		cCommand = g_strdup_printf ("curl --connect-timeout 5 --retry 2 http://imagebin.ca/upload.php -F f=@%s -F t=file -o %s", cFilePath, cLogFile);
	}
	g_print ("%s\n", cCommand);
	int r = system (cCommand);
	g_free (cCommand);
	
	
	// On récupère l'URL dans le log :
	gchar *cURL = NULL;
	if (iFileType == CD_TYPE_TEXT)
		cCommand = g_strdup_printf ("sed '/SUCCESS:/!d;s/^.*:\\([0-9][0-9]*\\).*$/http:\\/\\/pastebin.ca\\/\\1/' '%s'", cLogFile);
	else
		cCommand = g_strdup_printf ("sed 's/<p>You can find this at <a href='([^<]+)'>([^<]+)</a></p>/http:\\/\\/imagebin.ca\\/\\1/' '%s'", cLogFile);
	g_spawn_command_line_sync (cCommand, &cURL,  NULL, NULL, NULL);
	g_free (cCommand);
	
	g_remove (cLogFile);
	g_free (cLogFile);
	
	if (cURL == NULL || *cURL == '\0')
	{
		return ;
	}
	
	// Enfin on remplit la memoire partagee avec nos URLs.
	myData.cResultUrls = g_new0 (gchar *, NB_URLS+1);
	myData.cResultUrls[0] = cURL;
}


void cd_dnd2share_register_imagebin_backend (void)
{
	myData.backends[CD_IMAGEBIN].cSiteName = "imagebin.ca";
	myData.backends[CD_IMAGEBIN].iNbUrls = NB_URLS;
	myData.backends[CD_IMAGEBIN].cUrlLabels = s_UrlLabels;
	myData.backends[CD_IMAGEBIN].iPreferedUrlType = 0;
	myData.backends[CD_IMAGEBIN].upload = upload;
}
