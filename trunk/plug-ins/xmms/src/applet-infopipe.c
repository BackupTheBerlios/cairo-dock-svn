/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#define _BSD_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-infopipe.h"
#include "applet-draw.h"


static char  *s_cTmpFile = NULL;

enum {
	INFO_STATUS = 0,
	INFO_TRACK_IN_PLAYLIST,
	INFO_TIME_ELAPSED_IN_SEC,
	INFO_TIME_ELAPSED,
	INFO_TOTAL_TIME_IN_SEC,
	INFO_TOTAL_TIME,
	INFO_NOW_TITLE,
	NB_INFO
} AppletInfoEnum;

static int s_pLineNumber[MY_NB_PLAYERS][NB_INFO] = {
	{2,4,5,6,7,8,12} ,
	{0,1,2,3,4,5,6} ,
	{0,1,2,3,4,5,6} ,
	{0,1,2,3,4,5,6} ,
};


//Fonction de lecture du tuyau.
void cd_xmms_read_pipe (CairoDockModuleInstance *myApplet)
{
	//\________________________ On determine le pipe.
	gchar *cPipeCommand = NULL;
	switch (myConfig.iPlayer) {  // On emule un pipe pour les lecteurs qui n'en ont pas.
		case MY_XMMS :
		break ;
		case MY_AUDACIOUS :  //Il faut émuler le pipe d'audacious par AUDTOOL
			cPipeCommand = g_strdup_printf ("bash %s/infoaudacious.sh", MY_APPLET_SHARE_DATA_DIR);
		break ;
		case MY_BANSHEE :  //Le pipe est trop lent et cause des freezes... // Il faut émuler le pipe de banshee par le script
			cPipeCommand = g_strdup_printf ("bash %s/infobanshee.sh", MY_APPLET_SHARE_DATA_DIR);
		break ;
		case MY_EXAILE :  //Le pipe est trop lent, récupération des infos une fois sur deux avec un pique du cpu lors de l'éxécution du script // Il faut émuler le pipe d'audacious par Exaile -q
			cPipeCommand = g_strdup_printf ("bash %s/infoexaile.sh", MY_APPLET_SHARE_DATA_DIR);
		break ;
		default :
		break ;
	}
	
	//\________________________ On lit le pipe.
	gchar *cResult = NULL;
	if (cPipeCommand != NULL)  // c'est un lecteur avec un pipe emule.
	{
		cResult = cairo_dock_launch_command_sync (cPipeCommand);
		g_free (cPipeCommand);
	}
	else if (myConfig.iPlayer == MY_XMMS)  // c'est XMMS, il a deja son propre pipe.
	{
		gchar *cPipe = g_strdup_printf ("/tmp/xmms-info_%s.0",g_getenv ("USER"));
		gsize length=0;
		GError *erreur = NULL;
		g_file_get_contents (cPipe, &cResult, &length, &erreur);
		
		if (erreur != NULL) {
			cd_warning("xmms : %s", erreur->message);
			g_error_free(erreur);
		}
		
		g_free (cPipe);
	}
	
	if (cResult == NULL)  // erreur lors de la leture du pipe, on sort.
	{
		myData.playingStatus = PLAYER_NONE;
		cd_xmms_player_none (myApplet);
		return;
	}
	
	//\________________________ On recupere les donnees.
	gchar **cInfopipesList = g_strsplit(cResult, "\n", -1);
	g_free(cResult);
	
	gchar *cQuickInfo = NULL;
	gchar *cOneInfopipe;
	myData.iTrackNumber = -1;
	myData.iCurrentTime = -1;
	myData.iSongLength = -1;
	int *pLineNumber = s_pLineNumber[myConfig.iPlayer];
	int i;
	for (i = 0; cInfopipesList[i] != NULL; i ++) {
		cOneInfopipe = cInfopipesList[i];
		if (i == pLineNumber[INFO_STATUS]) {
			gchar *str = strchr (cOneInfopipe, ' ');
			if (str != NULL) {
				str ++;
				while (*str == ' ')
					str ++;
				if ((strcmp (str, "Playing") == 0) || (strcmp (str, "playing") == 0))
					myData.playingStatus = PLAYER_PLAYING;
				else if ((strcmp (str, "Paused") == 0) || (strcmp (str, "paused") == 0))
					myData.playingStatus = PLAYER_PAUSED;
				else if ((strcmp (str, "Stopped") == 0) || (strcmp (str, "stopped") == 0))
					myData.playingStatus = PLAYER_STOPPED;
				else
					myData.playingStatus = PLAYER_BROKEN;
			}
			else
				myData.playingStatus = PLAYER_BROKEN;
		}
		else if (i == pLineNumber[INFO_TRACK_IN_PLAYLIST]) {
			if (myConfig.quickInfoType == MY_APPLET_TRACK) {
				gchar *str = strchr (cOneInfopipe, ':');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					myData.iTrackNumber = atoi (str);
				}
			}
		}
		else if (i == pLineNumber[INFO_TIME_ELAPSED_IN_SEC]) {
			if (myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.quickInfoType == MY_APPLET_TIME_LEFT) {
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					if (*str != 'N')
						myData.iCurrentTime = atoi(str) * 1e-3;
				}
			}
		}
		else if (i == pLineNumber[INFO_TIME_ELAPSED]) {
			if ((myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.quickInfoType == MY_APPLET_TIME_LEFT) && myData.iCurrentTime == -1) {
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					gchar *str2 = strchr (str, ':');
					if (str2 == NULL) { // pas de minutes.
						myData.iCurrentTime = atoi(str);
					}
					else {
						*str2 = '\0';
						myData.iCurrentTime = atoi(str2+1) + 60*atoi (str);  // prions pour qu'ils n'ecrivent jamais les heures ... xD
					}
				}
			}
		}
		else if (i == pLineNumber[INFO_TOTAL_TIME_IN_SEC]) {
			if (myConfig.quickInfoType == MY_APPLET_TIME_LEFT) {
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					if (*str != 'N')
						myData.iSongLength = atoi(str) * 1e-3;
				}
			}
		}
		else if (i == pLineNumber[INFO_TOTAL_TIME]) {
			if (myConfig.quickInfoType == MY_APPLET_TIME_LEFT && myData.iSongLength == -1) {
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					gchar *str2 = strchr (str, ':');
					if (str2 == NULL) { // pas de minutes.
						myData.iSongLength = atoi(str);
					}
					else {
						*str2 = '\0';
						myData.iSongLength = atoi(str2+1) + 60*atoi (str);  // prions pour qu'ils n'ecrivent jamais les heures ...
					}
				}
			}
		}
		else if (i == pLineNumber[INFO_NOW_TITLE]) {
			gchar *str = strchr (cOneInfopipe, ':');
			if (str != NULL) {
				str ++;
				while (*str == ' ')
					str ++;
				if ((strcmp(str," (null)") != 0) && (myData.playingTitle == NULL || strcmp(str, myData.playingTitle) != 0)) {
					g_free (myData.playingTitle);
					myData.playingTitle = g_strdup (str);
					cd_message("On a changé de son! (%s)", myData.playingTitle);
					cd_xmms_change_desklet_data(myApplet);
				}
			}
		}
	}  // fin de parcours des lignes.
	g_strfreev (cInfopipesList);
	
	if (myConfig.iPlayer != MY_XMMS) {
		g_remove (s_cTmpFile);
	}
	g_free (s_cTmpFile);
	s_cTmpFile = NULL;
}

