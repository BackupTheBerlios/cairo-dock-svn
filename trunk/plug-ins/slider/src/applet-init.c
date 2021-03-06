/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-slider.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("slider"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet is a basic image slider\n"
	" You just have to select a directory and a display effect and you're done\n"
	"Click to play/pause or to edit the current image.\n"
	"Middle-click to open the images folder."),
	"ChAnGFu (Rémy Robertson) & Fabounet (Fabrice Rey)")

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	CD_APPLET_GET_MY_ICON_EXTENT (&myData.iSurfaceWidth, &myData.iSurfaceHeight);
	
	myData.pMeasureImage = cairo_dock_new_task (0,
		(CairoDockGetDataAsyncFunc) cd_slider_read_image,
		(CairoDockUpdateSyncFunc) cd_slider_update_transition,
		myApplet);  // 0 <=> one shot task.
	
	myData.pMeasureDirectory = cairo_dock_new_task (0,
		(CairoDockGetDataAsyncFunc) cd_slider_get_files_from_dir,
		(CairoDockUpdateSyncFunc) cd_slider_next_slide,
		myApplet);  // 0 <=> one shot task.
	cairo_dock_launch_task_delayed (myData.pMeasureDirectory, 1500.);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_REGISTER_FOR_UPDATE_ICON_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_EVENT;
	
	if (myData.iTimerID != 0)
		g_source_remove (myData.iTimerID);
	if (myData.iScrollID != 0)
		g_source_remove (myData.iScrollID);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//Stop all process!
	if (myData.iTimerID != 0) {
		g_source_remove(myData.iTimerID);
		myData.iTimerID = 0;
	}
	if (myData.iScrollID != 0) {
		g_source_remove (myData.iScrollID);
		myData.iScrollID = 0;
	}
	CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_EVENT;
	
	cairo_surface_destroy (myData.pCairoSurface);
	myData.pCairoSurface = NULL;
	cairo_surface_destroy (myData.pPrevCairoSurface);
	myData.pPrevCairoSurface = NULL;
	
	if (myDesklet) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	CD_APPLET_GET_MY_ICON_EXTENT (&myData.iSurfaceWidth, &myData.iSurfaceHeight);  // meme si le container n'a pas change, car un desklet se redimensionne, et l'icone avec.
	
	
	if ((!myConfig.bImageName || myDock) && myIcon->cQuickInfo != NULL) {
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
	}
	
	//\_______________ Reload all changed data.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cairo_dock_stop_task (myData.pMeasureImage);
		cairo_dock_stop_task (myData.pMeasureDirectory);
		cd_slider_free_images_list (myData.pList);
		myData.pList = NULL;
		myData.pElement = NULL;
		myData.bPause = FALSE;
		cairo_dock_launch_task (myData.pMeasureDirectory);
	}
	else {
		//Nothing to do ^^
		cd_slider_next_slide (myApplet); //restart sliding
	}
	CD_APPLET_REGISTER_FOR_UPDATE_ICON_EVENT;
CD_APPLET_RELOAD_END
