/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"

CD_APPLET_INCLUDE_MY_VARS

#define COMPIZ_TMP_FILE "/tmp/compiz"

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;
static GStaticMutex mutexData = G_STATIC_MUTEX_INIT;

void _compiz_cmd(gchar *cmd) {
  cd_message("Compiz: Launching %s", cmd);
	system (cmd);
}

gboolean cd_compiz_start_wm(void) {
  gchar *cmd = NULL;
  switch (myConfig.iWM) {
    case COMPIZ_FUSION: default: //Compiz
      cmd = "compiz.real --replace --ignore-desktop-hints ccp";
      if (myConfig.lBinding) {
        cmd = g_strdup_printf("%s --loose-binding", cmd);
      }
      if (myConfig.iRendering) {
        cmd = g_strdup_printf("%s --indirect rendering", cmd);
      }
      if (myConfig.selfDecorator) {
        gchar *decorator = NULL;
        if (g_iDesktopEnv == CAIRO_DOCK_GNOME || g_iDesktopEnv == CAIRO_DOCK_XFCE) {
         decorator = "gtk-window-decorator --replace";
        }
        else if (g_iDesktopEnv == CAIRO_DOCK_KDE) {
         decorator = "kde-window-decorator --replace"; //A remplacer par le Decorateur de KDE
        }
        cmd = g_strdup_printf("%s --sm-disable & %s", cmd, decorator);
      }
      cmd = g_strdup_printf("%s &", cmd);
    break;
    case METACITY: case XFCE: //Gnome & XFCE
      cmd = "metacity --replace &";
    break;
    case KWIN: //KDE
      cmd = "kwin --replace &";
    break;
  }
  if (cmd != NULL) {
    _compiz_cmd(cmd);
    cd_compiz_launch_measure();
  }
  else {
    cd_message("Compiz: No Window Manager to launch, aborting.\n");
  }
  return FALSE;
}

void cd_compiz_check_my_wm(void) {
  if (myConfig.protectDecorator) {
	  if ((myData.iCompizIcon == 0) && (myConfig.iWM != 0)) { //on a compiz alors qu'on en veut pas
	    cd_compiz_start_wm(); //On Tue le WM et on recharge
	  }
  	else if ((myData.iCompizIcon == 2) && (myConfig.iWM == 0)) { //on veut compiz mais on ne l'a pas, dangereux si la personne a un bug de CG
	    cd_compiz_start_wm(); 
	  }
	}
}

gboolean cd_compiz_timer(void) {
  cd_compiz_launch_measure();
  return TRUE;
}

void cd_compiz_launch_measure(void) {
	cd_message (" ");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1)) {  //il etait egal a 0, on lui met 1 et on lance le thread.
		cd_message (" ==> lancement du thread de calcul");
		
		if (s_iSidTimerRedraw == 0) {
			s_iSidTimerRedraw = g_timeout_add (333, (GSourceFunc) _cd_compiz_check_for_redraw, (gpointer) NULL);
		}
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_compiz_threaded_calculation, NULL, FALSE, &erreur);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
}

gpointer cd_compiz_threaded_calculation (gpointer data) {
	cd_compiz_get_data();
	
	g_static_mutex_lock (&mutexData);
	myData.bAcquisitionOK = cd_compiz_isRunning();
	g_static_mutex_unlock (&mutexData);
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread compiz");
	return NULL;
}

void cd_compiz_get_data(void) {
	gchar *cCommand = g_strdup_printf("bash %s/compiz", MY_APPLET_SHARE_DATA_DIR);
	system (cCommand);
	g_free (cCommand);
}

gboolean cd_compiz_isRunning(void) {
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(COMPIZ_TMP_FILE, &cContent, &length, &erreur);
	if (erreur != NULL)	{
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		return FALSE;
	}
	else {
		_compiz_get_values_from_file (cContent);
		g_free (cContent);
		return TRUE;
	}
}

static void _compiz_get_values_from_file (gchar *cContent) {
	gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
	gchar *cOneInfopipe = NULL;
	int i;
	for (i = 0; cInfopipesList[i] != NULL; i ++) {
		cOneInfopipe = cInfopipesList[i];
		if (*cOneInfopipe == '\0')
			continue;
		if ((i == 0) && (strcmp(cOneInfopipe,"Compiz") == 0)) {
		  cd_message("Compiz: Running");
			myData.isCompiz = TRUE;
			if (myData.iCompizIcon != 0) {
			  myData.iCompizIcon = 0;
			  myData.bNeedRedraw = TRUE;
			}
			break;
		}
		else {
		  cd_message("Compiz: Not running");
		  myData.isCompiz = FALSE;
		  if (myData.iCompizIcon != 2) {
		    myData.bNeedRedraw = TRUE;
		    myData.iCompizIcon = 2;
		  }
		  break;
		}
	}
	g_strfreev (cInfopipesList);
}
