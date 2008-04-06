#include "string.h"
#include <glib/gi18n.h>

#include "powermanager-struct.h"
#include "powermanager-draw.h"

CD_APPLET_INCLUDE_MY_VARS


void iconWitness(int animationLenght)
{
	CD_APPLET_ANIMATE_MY_ICON (1, animationLenght)
}

void update_icon(void)
{
	if(myData.battery_present)
	{
		if (myData.previous_battery_time != myData.battery_time)
		{
			if(myConfig.quickInfoType == POWER_MANAGER_TIME)
			{
				CD_APPLET_SET_HOURS_MINUTES_AS_QUICK_INFO (myData.battery_time)
			}
			else if(myConfig.quickInfoType == POWER_MANAGER_CHARGE)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d%%", myData.battery_charge)
			}
			else
			{
			  CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
			}
		}
		
		make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) myData.battery_charge / 100);
		if (myData.previously_on_battery != myData.on_battery)
		{
			myData.previously_on_battery = myData.on_battery;
			myData.alerted = FALSE;  //On a changer de statut, donc on réinitialise les alertes
		}
		myData.previous_battery_charge = myData.battery_charge;
		
		if(myData.on_battery)
		{
<<<<<<< .mine
			//Alert when battery charge is under a configured value
			if((myData.battery_charge <= myConfig.lowBatteryValue) && (myData.alerted != TRUE))
			{
				cd_powermanager_alert(1);
			}
			
			/*
			if(myData.battery_charge >= 95)
				{ cd_powermanager_set_surface ("battery_44.svg"); }
			else if(myData.battery_charge >= 65)
				{ cd_powermanager_set_surface ("battery_34.svg"); }
			else if(myData.battery_charge >= 35)
				{ cd_powermanager_set_surface ("battery_24.svg"); }
			else if(myData.battery_charge >= 5)
				{ cd_powermanager_set_surface ("battery_14.svg"); }
			else
				{ cd_powermanager_set_surface ("battery_04.svg"); }
			*/
		}
		else
		{
			//Alert when battery is charged
			if((myData.battery_charge == 100) && (myData.alerted != TRUE))
			{
				cd_powermanager_alert(0);
			}
			
			/*
			if(myData.battery_charge >= 95)
				{ cd_powermanager_set_surface ("charge_44.svg"); }
			else if(myData.battery_charge >= 65)
				{ cd_powermanager_set_surface ("charge_34.svg"); }
			else if(myData.battery_charge >= 35)
				{ cd_powermanager_set_surface ("charge_24.svg"); }
			else if(myData.battery_charge >= 5)
				{ cd_powermanager_set_surface ("charge_14.svg"); }
			else
				{ cd_powermanager_set_surface ("charge_04.svg"); }
			*/
		}
	}
	else
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg")
	}
	CD_APPLET_REDRAW_MY_ICON
	//Il faut recharger l'icône car elle ne se met pas a jour a tout les coups
}

gchar *get_hours_minutes(int iTimeInSeconds)
{
	gchar *time = g_strdup (D_("None"));
	if (iTimeInSeconds == NULL || iTimeInSeconds == 0)
	{
		return time;
	}
	int h=0, m=0;
	m = iTimeInSeconds / 60;
	h = m / 60;
	m = m - (h * 60);
	if (h > 0) time = g_strdup_printf("%dh%02dm", h, m);
	else if (m > 0) time = g_strdup_printf("%dm", m);
	else time = g_strdup (D_("None"));
	
	return time;
}

void cd_powermanager_bubble(void)
{
	if(myData.battery_present)
	{
		gchar *hms = get_hours_minutes(myData.battery_time);
		if(myData.on_battery)
		{
			cairo_dock_show_temporary_dialog ("%s %d%% \n %s %s", myIcon, myContainer, 6000, D_("Laptop on Battery.\n Battery charged at:"), myData.battery_charge, D_("Estimated time with Charge:"), hms);
		}
		else
		{
			cairo_dock_show_temporary_dialog ("%s %d%% \n %s %s", myIcon, myContainer, 6000, D_("Laptop on Charge.\n Battery charged at:"), myData.battery_charge, D_("Estimated time with Charge:"), hms);
		}
		g_free (hms);
	}
	else
	{
		cairo_dock_show_temporary_dialog ("%s", myIcon, myContainer, 6000, D_("No Battery found."));
	}
}

gboolean cd_powermanager_alert(int alert)
{
	gchar *hms = get_hours_minutes(myData.battery_time);
	if ((alert == 1) && (myConfig.lowBatteryWitness))
	{
		cairo_dock_show_temporary_dialog ("%s (%d%%) \n %s %s \n %s", myIcon, myContainer, 6000, D_("PowerManager.\nBattery charge seems to be low"), myData.battery_charge, D_("Estimated time with Charge:"), hms, D_("Please put your Laptop on charge."));
	}
	else if ((alert == 0) && (myConfig.highBatteryWitness))
	{
		cairo_dock_show_temporary_dialog ("%s (%d%%) \n %s %s ", myIcon, myContainer, 6000, D_("PowerManager.\nYour battery is now Charged"), myData.battery_charge, D_("Estimated time with Charge:"), hms);
	}
	if (myConfig.batteryWitness) 
	{
		CD_APPLET_ANIMATE_MY_ICON (myConfig.batteryWitnessAnimation, 3)
	}
	g_free (hms);
	myData.alerted = TRUE;
	return FALSE;
}

void cd_powermanager_set_surface (gchar *svgFile) {	
	cairo_surface_t *pSurface;
	gchar *cImagePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, svgFile);
	pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
	g_free (cImagePath);
	
	switch (myConfig.iEffect)
	{
		case POWER_MANAGER_EFFECT_NONE:
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
			break;
		case POWER_MANAGER_EFFECT_ZOOM:
			cairo_save (myDrawContext);
			double fScale = .3 + .7 * myData.battery_charge / 100.;
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale)
			cairo_restore (myDrawContext);
			break;
		case POWER_MANAGER_EFFECT_TRANSPARENCY: 
			cairo_save (myDrawContext);
			double fAlpha = .3 + .7 * myData.battery_charge / 100.;
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha)
			cairo_restore (myDrawContext);
			break;
		case POWER_MANAGER_EFFECT_BAR:
			cairo_save (myDrawContext);
			CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR(pSurface, myData.battery_charge * .01)
			cairo_restore (myDrawContext);
			break;
	}
	cairo_surface_destroy (pSurface);
}

