#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"
#include "cairo-dock.h"

#define NETSPEED_DATA_PIPE "/proc/net/dev"


// Prend un debit en octet par seconde et le transforme en une chaine de la forme : xxx yB/s
static void cd_netspeed_formatRate (CairoDockModuleInstance *myApplet, unsigned long long rate, gchar* debit) {
	int smallRate;
	
	if (rate <= 0)
	{
		if (myDesklet)
			g_sprintf(debit, "0 %s/s", D_("B"));
		else
			g_sprintf(debit, "0");
	}
	else if (rate < 1024)
	{
		smallRate = rate;
		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("B"));
		else
			g_sprintf(debit, "%iB", smallRate);
	}
	else if (rate < (1<<20))
	{
		smallRate = rate >> 10;
		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("KB"));
		else
			g_sprintf(debit, "%iK", smallRate);
	}
	else if (rate < (1<<30))
	{
		smallRate = rate >> 20;
		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("MB"));
		else
			g_sprintf(debit, "%iM", smallRate);
	}
	else if (rate < ((unsigned long long)1<<40))
	{
		smallRate = rate >> 30;
		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("GB"));
		else
			g_sprintf(debit, "%iG", smallRate);
	}
	else  // c'est vraiment pour dire qu'on est exhaustif :-)
	{
		smallRate = rate >> 40;
		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("TB"));
		else
			g_sprintf(debit, "%iT", smallRate);
	}
}


void cd_netspeed_get_data (CairoDockModuleInstance *myApplet)
{
	g_timer_stop (myData.pClock);
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_timer_start (myData.pClock);
	g_return_if_fail (fTimeElapsed > 0.1);
	
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents (NETSPEED_DATA_PIPE, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning("NetSpeed : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else
	{
		int iNumLine = 1;
		gchar *tmp = cContent;
		long long int iReceivedBytes, iTransmittedBytes;
		while (TRUE)
		{
			if (iNumLine > 3)  // les 2 premieres lignes sont les noms des champs, la 3eme est la loopback.
			{
				while (*tmp == ' ')  // on saute les espaces.
					tmp ++;
				
				if (strncmp (tmp, myConfig.cInterface, myConfig.iStringLen) == 0 && *(tmp+myConfig.iStringLen) == ':')  // c'est l'interface qu'on veut.
				{
					tmp += myConfig.iStringLen+1;  // on saute le ':' avec.
					iReceivedBytes = atoll (tmp);
					
					int i = 0;
					for (i = 0; i < 8; i ++)  // on saute les 8 valeurs suivantes.
					{
						while (*tmp != ' ')  // saute le chiffre courant.
							tmp ++;
						while (*tmp == ' ')  // saute les espaces.
							tmp ++;
					}
					iTransmittedBytes = atoll (tmp);
					
					if (myData.bInitialized)  // la 1ere iteration on ne peut pas calculer le debit.
					{
						myData.iDownloadSpeed = (iReceivedBytes - myData.iReceivedBytes) / fTimeElapsed;
						myData.iUploadSpeed = (iTransmittedBytes - myData.iTransmittedBytes) / fTimeElapsed;
					}
					
					myData.iReceivedBytes = iReceivedBytes;
					myData.iTransmittedBytes = iTransmittedBytes;
					break ;
				}
			}
			tmp = strchr (tmp+1, '\n');
			if (tmp == NULL)
				break;
			tmp ++;
			iNumLine ++;
		}
		myData.bAcquisitionOK = (tmp != NULL);
		g_free (cContent);
		if (! myData.bInitialized)
			myData.bInitialized = TRUE;
	}
}

gboolean cd_netspeed_update_from_data (CairoDockModuleInstance *myApplet)
{
	static double s_fValues[CD_NETSPEED_NB_MAX_VALUES];
	static gchar s_upRateFormatted[11];
	static gchar s_downRateFormatted[11];
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		
		memset (s_fValues, 0, sizeof (s_fValues));
		CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		
		cairo_dock_downgrade_task_frequency (myData.pPeriodicTask);
	}
	else
	{
		cairo_dock_set_normal_task_frequency (myData.pPeriodicTask);
		
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (myDock ? "..." : D_("Loading"));
			memset (s_fValues, 0, sizeof (s_fValues));
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		}
		else
		{
			if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_NONE)
			{
				cd_netspeed_formatRate (myApplet, myData.iUploadSpeed, s_upRateFormatted);
				cd_netspeed_formatRate (myApplet, myData.iDownloadSpeed, s_downRateFormatted);
				if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				{
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("↓%s\n↑%s", s_downRateFormatted, s_upRateFormatted);
				}
				else
				{
					CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("↓%s\n↑%s", s_downRateFormatted, s_upRateFormatted);
				}
			}
			
			if(myData.iUploadSpeed > myData.iMaxUpRate) {
				myData.iMaxUpRate = myData.iUploadSpeed;
			}
			if(myData.iDownloadSpeed > myData.iMaxDownRate) {
				myData.iMaxDownRate = myData.iDownloadSpeed;
			}
			
			double fUpValue, fDownValue;
			if (myData.iMaxUpRate != 0)
				fUpValue = (double) myData.iUploadSpeed / myData.iMaxUpRate;
			else
				fUpValue = 0.;
			if (myData.iMaxDownRate != 0)
				fDownValue = (double) myData.iDownloadSpeed / myData.iMaxDownRate;
			else
				fDownValue = 0.;
			
			s_fValues[0] = fDownValue;
			s_fValues[1] = fUpValue;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		}
	}
	return TRUE;
}
