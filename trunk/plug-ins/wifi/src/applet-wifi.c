#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-wifi.h"

CD_APPLET_INCLUDE_MY_VARS


#define WIFI_TMP_FILE "/tmp/wifi"

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;
static GStaticMutex mutexData = G_STATIC_MUTEX_INIT;


gboolean cd_wifi_timer (gpointer data) {
	cd_wifi_launch_measure();
	return TRUE;
}

gpointer cd_wifi_threaded_calculation (gpointer data) {
	cd_wifi_get_data();
	
	g_static_mutex_lock (&mutexData);
	myData.bAcquisitionOK = cd_wifi_getStrength ();
	g_static_mutex_unlock (&mutexData);
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread wifi");
	return NULL;
}

void cd_wifi_get_data (void) {
	gchar *cCommand = g_strdup_printf("bash %s/wifi", MY_APPLET_SHARE_DATA_DIR);
	system (cCommand);
	g_free (cCommand);
}

static gboolean _cd_wifi_check_for_redraw (gpointer data) {
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning) {
		s_iSidTimerRedraw = 0;
		if (myIcon == NULL) {
			g_print ("annulation du chargement du wifi\n");
			return FALSE;
		}
		
		g_static_mutex_lock (&mutexData);
		if (myData.bAcquisitionOK)
			cd_wifi_draw_icon ();
		else
			cd_wifi_draw_no_wireless_extension ();
		g_static_mutex_unlock (&mutexData);
		
		if (myConfig.iCheckInterval != myConfig.dCheckInterval) {
			myConfig.iCheckInterval = myConfig.dCheckInterval;
			if (myData.iSidTimer != 0) {
				g_source_remove (myData.iSidTimer);
				myData.iSidTimer = 0;
			}
			myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
		}
		
		//\_______________________ On lance le timer si necessaire.
		if (myData.iSidTimer == 0) {
			myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
		}
		return FALSE;
	}
	return TRUE;
}
void cd_wifi_launch_measure (void) {
	cd_message (" ");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1)) {  //il etait egal a 0, on lui met 1 et on lance le thread.
		cd_message (" ==> lancement du thread de calcul");
		
		myData.iPreviousQuality = -1;
		myData.prev_prcnt = -1;
		
		if (s_iSidTimerRedraw == 0) {
			s_iSidTimerRedraw = g_timeout_add (333, (GSourceFunc) _cd_wifi_check_for_redraw, (gpointer) NULL);
		}
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_wifi_threaded_calculation, NULL, FALSE, &erreur);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
}

static float pourcent(float x, float y) {
  float p=0;
  if (x > y) {
    x = y;
  }
  else if (x < 0) {
   x = 0;
  }
  p = (x / y) *100;
  return p;
}
static gboolean _wifi_get_values_from_file (gchar *cContent, int *iFlink, int *iMlink, int *iPercentage, CDWifiQuality *iQuality) {
	gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
	gchar *cOneInfopipe;
	const gchar *ESSID = NULL;
	int flink=0, mlink=0, i=0,prcnt=0;
	for (i = 0; cInfopipesList[i] != NULL; i ++) {
		cOneInfopipe = cInfopipesList[i];
		if ((i == 0) && (strcmp(cOneInfopipe,"Wifi") == 0)) {
			g_strfreev (cInfopipesList);
			return FALSE;
		}
		else {
			int c = 0, iNbSpace = 0;
			gchar *cUtilInfo = NULL;
			while (cOneInfopipe[c] != '\0') {
				if (cOneInfopipe[c] == ' ') {
					iNbSpace ++;
					if ((iNbSpace == 8) && (ESSID == NULL)) { /// plutot que de compter les espaces, il est plus prudent de compter les mots. En effet les espaces on n'est pas sur qu'ils vont pas changer.
					  ESSID = &cOneInfopipe[c+1];
					}
					if (iNbSpace == 11) {
						cUtilInfo = &cOneInfopipe[c+1];
						break;
					}
				}
				c ++;
			}
			if (cUtilInfo != NULL) {
				gchar *str = strchr (cUtilInfo, '=');
				if (str == NULL) {
					str = strchr (cUtilInfo, ':');
				}
				if (str != NULL) {
					cUtilInfo = str + 1;
					str = strchr (cUtilInfo, '/');
					if (str != NULL) {
						*str = '\0';
						flink = atoi (cUtilInfo);
						mlink = atoi (str+1);
						
						cd_debug("Signal Quality: %d/%d", flink, mlink);
						prcnt = pourcent(flink,mlink);
					}
				}
			}
			
		}  /// que fait-on des autres lignes ?? seule la derniere semble etre importante.
	}
	
	const gchar *ESSID_ok = NULL;
	if (ESSID != NULL) {
	  gchar *str = strchr (ESSID, '"');
	  if (str != NULL) {
	    str ++;
	    gchar *str2 = strchr (str, '"');
	    if (str2 != NULL) {
	      *str2 = '\0';
	      //gchar **str2 = g_strsplit(ESSID, "\"", -1);
	      //ESSID = str2[0];
	      ESSID_ok = str;
	      cd_message("ESSID: %s", ESSID_ok);
	    }
	  }
	}
	if (ESSID_ok == NULL) {
		ESSID_ok = D_("Unknown");
	}
	
	*iFlink = flink;
	*iMlink = mlink;
	if (prcnt <= 0) {
		*iQuality = WIFI_QUALITY_NO_SIGNAL;
	}
	else if (prcnt < 20) {
		*iQuality = WIFI_QUALITY_VERY_LOW;
	}
	else if (prcnt < 40) {
		*iQuality = WIFI_QUALITY_LOW;
	}
	else if (prcnt < 60) {
		*iQuality = WIFI_QUALITY_MIDDLE;
	}
	else if (prcnt < 80) {
		*iQuality = WIFI_QUALITY_GOOD;
	}
	else {
		*iQuality = WIFI_QUALITY_EXCELLENT;
	}
	*iPercentage = prcnt;
	
	g_free (myData.cESSID);
	myData.cESSID = g_strdup (ESSID_ok);
	
	g_strfreev (cInfopipesList);
	return TRUE;
}

gboolean cd_wifi_getStrength(void) {
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(WIFI_TMP_FILE, &cContent, &length, &erreur);
	if (erreur != NULL)	{
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		return FALSE;
	}
	else {
		gboolean bAcquisitionOK = _wifi_get_values_from_file (cContent, &myData.flink, &myData.mlink, &myData.prcnt, &myData.iQuality);
		g_free (cContent);
		
		if (! bAcquisitionOK || myData.prcnt < 0) {
			myData.iQuality = WIFI_QUALITY_NO_SIGNAL;
			myData.prcnt = 0;
			return FALSE;
		}
		
		return TRUE;
	}
}
