#include <stdlib.h>

#include "tomboy-draw.h"
#include "tomboy-config.h"
#include "tomboy-dbus.h"
#include "tomboy-notifications.h"
#include "tomboy-struct.h"
#include "tomboy-init.h"


CD_APPLET_DEFINITION ("Note-Taking",
	1, 5, 4,
	CAIRO_DOCK_CATEGORY_CONTROLER,
	N_("Control your Gnote or TomBoy's notes directly in the dock !\n"
	"Click on a note to open it, Escape to close it.\n"
	"Middle-click to instantly create a new note.\n"
	"You can search inside notes and display their content on the icons."),
	"Necropotame (Adrien Pilleboue)")

CD_APPLET_INIT_BEGIN
	load_all_surfaces();
	
	myData.hNoteTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		NULL,  // l'URI est partage avec l'icone.
		(GDestroyNotify) NULL);  // on detruit les icones nous-memes.
	
	myData.dbus_enable = dbus_connect_to_bus ();
	if (myData.dbus_enable)
	{
		dbus_detect_tomboy();
		myData.pTask = cairo_dock_new_task (0,
				(CairoDockGetDataAsyncFunc) getAllNotes,
				(CairoDockUpdateSyncFunc) cd_tomboy_load_notes,
				myApplet);
		cairo_dock_launch_task (myData.pTask);
	}
	else if (myDock)  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconClose, "broken.svg");
	}
	
	if (myConfig.bNoDeletedSignal)
		myData.iSidCheckNotes = g_timeout_add_seconds (2, (GSourceFunc) cd_tomboy_check_deleted_notes, (gpointer) NULL);
	
	//Enregistrement des notifications
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK_FUNC, CAIRO_DOCK_RUN_FIRST, myApplet);  // ici on s'enregistre explicitement avant le dock, pour pas qu'il essaye de lancer nos notes.
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	cairo_dock_remove_notification_func_on_container (CD_APPLET_MY_ICONS_LIST_CONTAINER, CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) cd_tomboy_on_change_icon, myApplet);
	
	if (myData.iSidCheckNotes != 0)
		g_source_remove (myData.iSidCheckNotes);
	if (myData.iSidResetQuickInfo != 0)
		g_source_remove (myData.iSidResetQuickInfo);
	if (myData.iSidPopupDialog != 0)
		g_source_remove (myData.iSidPopupDialog);
	
	dbus_disconnect_from_bus ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les surfaces.
	load_all_surfaces();
	
	//\_______________ On recharge les parametres qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.dbus_enable)
		{
			cairo_dock_stop_task (myData.pTask);
			free_all_notes ();  // detruit aussi la liste des icones.
			
			//\___________ On arrete le timer.
			if (myData.iSidCheckNotes != 0)
			{
				g_source_remove (myData.iSidCheckNotes);
				myData.iSidCheckNotes = 0;
			}
			if (myData.iSidResetQuickInfo != 0)
			{
				g_source_remove (myData.iSidResetQuickInfo);
				myData.iSidResetQuickInfo = 0;
			}
			if (myData.iSidPopupDialog != 0)
			{
				g_source_remove (myData.iSidPopupDialog);
				myData.iSidPopupDialog = 0;
			}
			
			
			//\___________ On reconstruit les icones (si l'icone de la note a change).
			cairo_dock_launch_task (myData.pTask);
		}
	}
	else if (myDesklet)
	{
		cd_tomboy_reload_desklet_renderer ();  // on recharge juste les surfaces/textures des icones.
	}
	
	//\___________ On redessine l'icone principale.
	if (myDock)
	{
		if (myData.dbus_enable)
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceDefault);
		}
		else  // on signale par l'icone appropriee que le bus n'est pas accessible.
		{
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconClose, "broken.svg");
		}
	}
CD_APPLET_RELOAD_END
