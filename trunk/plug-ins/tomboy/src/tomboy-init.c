#include <stdlib.h>

#include "tomboy-draw.h"
#include "tomboy-config.h"
#include "tomboy-dbus.h"
#include "tomboy-notifications.h"
#include "tomboy-struct.h"
#include "tomboy-init.h"


CD_APPLET_DEFINITION ("TomBoy", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)

CD_APPLET_INIT_BEGIN
	if (myIcon->acName == NULL && myDock)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (TOMBOY_DEFAULT_NAME);
	}
	
	load_all_surfaces();
	
	myData.hNoteTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		NULL,  // l'URI est partage avec l'icone.
		(GDestroyNotify) cairo_dock_free_icon);
	
	myData.dbus_enable = dbus_connect_to_bus ();
	if (myData.dbus_enable)
	{
		dbus_detect_tomboy();
		myData.pMeasureTimer = cairo_dock_new_measure_timer (0,
				NULL,
				(CairoDockReadTimerFunc) getAllNotes,
				(CairoDockUpdateTimerFunc) cd_tomboy_load_notes,
				myApplet);
		cairo_dock_launch_measure (myData.pMeasureTimer);
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconClose, "broken.svg");
	}
	
	if (myConfig.bNoDeletedSignal)
		myData.iSidCheckNotes = g_timeout_add_seconds (2, (GSourceFunc) cd_tomboy_check_deleted_notes, (gpointer) NULL);
	
	//Enregistrement des notifications
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	
	if (myData.iSidCheckNotes != 0)
	{
		g_source_remove (myData.iSidCheckNotes);
		myData.iSidCheckNotes = 0;
	}
	dbus_disconnect_from_bus ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les surfaces.
	load_all_surfaces();
	
	//\_______________ On recharge les parametres qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myIcon->acName == NULL && myDock)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (TOMBOY_DEFAULT_NAME);
		}
		if (myData.dbus_enable)
		{
			//\___________ On arrete le timer.
			if (myData.iSidCheckNotes != 0)
			{
				g_source_remove (myData.iSidCheckNotes);
				myData.iSidCheckNotes = 0;
			}
			/**if (myConfig.bNoDeletedSignal && myData.iSidCheckNotes == 0)
				myData.iSidCheckNotes = g_timeout_add_seconds (2, (GSourceFunc) cd_tomboy_check_deleted_notes, (gpointer) NULL);
			else if (! myConfig.bNoDeletedSignal && myData.iSidCheckNotes != 0)
			{
				g_source_remove (myData.iSidCheckNotes);
				myData.iSidCheckNotes = 0;
			}*/
			
			//\___________ On reconstruit le sous-dock (si l'icone de la note a change).
			cairo_dock_launch_measure (myData.pMeasureTimer);
		}
	}
	
	//\___________ On redessine l'icone principale.
	if (myData.dbus_enable)
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceDefault);
	}
	else  // on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconClose, "broken.svg");
	}
CD_APPLET_RELOAD_END
