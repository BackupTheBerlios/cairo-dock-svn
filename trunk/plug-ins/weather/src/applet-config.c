/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN ("Meteo", NULL)
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.cLocationCode = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "location code", "FRXX0076");
	myConfig.bISUnits = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "IS units", TRUE);
	myConfig.bCurrentConditions = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display cc", TRUE);
	myConfig.bDisplayNights = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display nights", FALSE);
	myConfig.iNbDays = MIN (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb days", WEATHER_NB_DAYS_MAX), WEATHER_NB_DAYS_MAX);
	myConfig.bDisplayTemperature = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display temperature", FALSE);
	myConfig.cDialogDuration = 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "dialog duration", 5);
	myConfig.iCheckInterval = 60000 * MAX (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 15), 1);
	
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "basic");
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	cairo_dock_update_conf_file_with_renderers (CD_APPLET_MY_CONF_FILE, "Configuration", "renderer");
CD_APPLET_CONFIG_END


static void _reset_units (Unit *pUnits)
{
	g_free (pUnits->cTemp);
	g_free (pUnits->cDistance);
	g_free (pUnits->cSpeed);
	g_free (pUnits->cPressure);
}

static void _reset_current_conditions (CurrentContitions *pCurrentContitions)
{
	g_free (pCurrentContitions->cSunRise);
	g_free (pCurrentContitions->cSunSet);
	g_free (pCurrentContitions->cDataAcquisitionDate);
	g_free (pCurrentContitions->cObservatory);
	g_free (pCurrentContitions->cTemp);
	g_free (pCurrentContitions->cFeeledTemp);
	g_free (pCurrentContitions->cWeatherDescription);
	g_free (pCurrentContitions->cIconNumber);
	g_free (pCurrentContitions->cWindSpeed);
	g_free (pCurrentContitions->cWindDirection);
	g_free (pCurrentContitions->cPressure);
	g_free (pCurrentContitions->cHumidity);
	g_free (pCurrentContitions->cMoonIconNumber);
}

static void _reset_current_one_day (Day *pDay)
{
	g_free (pDay->cName);
	g_free (pDay->cDate);
	g_free (pDay->cTempMax);
	g_free (pDay->cTempMin);
	g_free (pDay->cSunRise);
	g_free (pDay->cSunSet);
	int j;
	for (j = 0; j < 2; j ++)
	{
		g_free (pDay->part[j].cIconNumber);
		g_free (pDay->part[j].cWeatherDescription);
		g_free (pDay->part[j].cWindSpeed);
		g_free (pDay->part[j].cWindDirection);
		g_free (pDay->part[j].cHumidity);
	}
}

void reset_config (void)
{
	g_free (myConfig.cLocationCode);
	g_free (myConfig.cRenderer);
	g_free (myConfig.cThemePath);
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	g_free (myData.cLon);
	g_free (myData.cLat);
	_reset_units (&myData.units);
	_reset_current_conditions (&myData.currentConditions);
	int i;
	for (i = 0; i < myConfig.iNbDays; i ++)
	{
		_reset_current_one_day (&myData.days[i]);
	}
	
	cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
	myIcon->pSubDock = NULL;  // normalement inutile.
	
	memset (&myData, 0, sizeof (AppletData));
}