/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-xgamma.h"
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	if (myDock)
	{
		double fGamma = xgamma_get_gamma (&myData.Xgamma);
		if (fGamma > 0)
		{
			fGamma = cairo_dock_show_value_and_wait (D_("Set up gamma :"), myIcon, myContainer, fGamma, GAMMA_MAX);
			if (fGamma > 0)
			{
				fGamma = MAX (fGamma, GAMMA_MIN);
				
				myData.Xgamma.red = fGamma;
				myData.Xgamma.blue = fGamma;
				myData.Xgamma.green = fGamma;
				xgamma_set_gamma (&myData.Xgamma);
			}
		}
	}
CD_APPLET_ON_CLICK_END

static void _cd_xgamma_remember_current_gamma (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	g_return_if_fail (fGamma > 0);
	
	cairo_dock_update_conf_file (myApplet->cConfFilePath,
		G_TYPE_DOUBLE, "Configuration", "initial gamma", fGamma,
		G_TYPE_INVALID);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU (D_("Apply current luminosity on startup"), _cd_xgamma_remember_current_gamma, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myDock)
	{
		double fGamma = xgamma_get_gamma (&myData.Xgamma);
		if (fGamma > 0)
		{
			//\___________________ On construit notre widget si c'est la 1ere fois.
			if (myData.pWidget == NULL)
			{
				xgamma_build_and_show_widget ();
			}
			else
			{
				//\___________________ On lui met les valeurs a jour, sans appeler les callbacks.
				g_signal_handler_block (myData.pGlobalScale, myData.iGloalScaleSignalID);
				g_signal_handler_block (myData.pRedScale, myData.iRedScaleSignalID);
				g_signal_handler_block (myData.pGreenScale, myData.iGreenScaleSignalID);
				g_signal_handler_block (myData.pBlueScale, myData.iBlueScaleSignalID);
				
				gtk_range_set_value (GTK_RANGE (myData.pGlobalScale), fGamma);
				gtk_range_set_value (GTK_RANGE (myData.pRedScale), myData.Xgamma.red);
				gtk_range_set_value (GTK_RANGE (myData.pGreenScale), myData.Xgamma.green);
				gtk_range_set_value (GTK_RANGE (myData.pBlueScale), myData.Xgamma.blue);
				myData.XoldGamma = myData.Xgamma;
				
				g_signal_handler_unblock (myData.pGlobalScale, myData.iGloalScaleSignalID);
				g_signal_handler_unblock (myData.pRedScale, myData.iRedScaleSignalID);
				g_signal_handler_unblock (myData.pGreenScale, myData.iGreenScaleSignalID);
				g_signal_handler_unblock (myData.pBlueScale, myData.iBlueScaleSignalID);
				
				if (myData.pDialog != NULL)
				{
					cairo_dock_unhide_dialog (myData.pDialog);
				}
			}
		}
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_SCROLL_BEGIN
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	g_print ("%.2f;%.2f;%.2f\n", myData.Xgamma.red, myData.Xgamma.green, myData.Xgamma.blue);
	double f;
	if (CD_APPLET_SCROLL_UP)
	{
		f = (1. - myConfig.iScrollVariation/100.);
		myData.Xgamma.red *= f;
		myData.Xgamma.green *= f;
		myData.Xgamma.blue *= f;
	}
	else
	{
		f = (1. + myConfig.iScrollVariation/100.);
		myData.Xgamma.red *= f;
		myData.Xgamma.green *= f;
		myData.Xgamma.blue *= f;
	}
	
	xgamma_set_gamma (&myData.Xgamma);
CD_APPLET_ON_SCROLL_END
