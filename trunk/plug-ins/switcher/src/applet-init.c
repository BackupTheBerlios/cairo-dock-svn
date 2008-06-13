
#include "stdlib.h"
#include "string.h"
#include <cairo-dock.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("switcher", 1, 5, 6, CAIRO_DOCK_CATEGORY_DESKTOP)


static gboolean on_change_desktop (gpointer *data)
{
	int iPreviousIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	cd_switcher_get_current_desktop ();
	int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	if (myDock)
	{
		if (myConfig.bDisplayNumDesk)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1)
		}
		
		if (myConfig.bCompactView)
		{
			cd_switcher_draw_main_icon_compact_mode ();
			CD_APPLET_REDRAW_MY_ICON
		}
		else
		{
			if (myConfig.bDisplayNumDesk)
				CD_APPLET_REDRAW_MY_ICON
			
			// On redessine les 2 icones du sous-dock impactees.
			g_return_val_if_fail (myIcon->pSubDock != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
			Icon *icon;
			GList *ic;
			for (ic = myIcon->pSubDock->icons; ic != NULL; ic = ic->next)
			{
				icon = ic->data;
				if (icon->fOrder == iPreviousIndex)  // l'ancienne icone du bureau courant.
				{
					cairo_dock_set_icon_name_full (myDrawContext, icon, CAIRO_CONTAINER (myIcon->pSubDock), "%s %d", D_("Desktop"), iIndex+1);
					icon->bHasIndicator = FALSE;
					cairo_dock_redraw_my_icon (icon, CAIRO_CONTAINER (myIcon->pSubDock));
				}
				if (icon->fOrder == iIndex)  // c'est l'icone du bureau courant.
				{
					cairo_dock_set_icon_name_full (myDrawContext, icon, CAIRO_CONTAINER (myIcon->pSubDock), "%s %d", D_("Current"), iIndex+1);
					icon->bHasIndicator = TRUE;
					cairo_dock_redraw_my_icon (icon, CAIRO_CONTAINER (myIcon->pSubDock));
				}
			}
		}
	}
	else
	{
		if (myConfig.bDisplayNumDesk)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1)
		}
		
		if (myConfig.bCompactView)
		{
			cd_switcher_draw_main_icon_compact_mode ();
			CD_APPLET_REDRAW_MY_ICON
		}
		else
		{
			/// idem ...
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
static gboolean on_change_screen_geometry (gpointer *data)
{
	g_print ("%s ()\n", __func__);
	cd_switcher_compute_nb_lines_and_columns ();
	cd_switcher_get_current_desktop ();
	cd_switcher_load_icons ();
	cd_switcher_draw_main_icon ();
	
	if (myDesklet != NULL)
	{
		if (myConfig.bCompactView)
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple")
		}
		else
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Caroussel")
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

CD_APPLET_INIT_BEGIN (erreur)
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	cairo_dock_register_notification (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, (CairoDockNotificationFunc) on_change_screen_geometry, CAIRO_DOCK_RUN_AFTER);/*Notifier de la gémotrie de bureau changé*/
	cairo_dock_register_notification (CAIRO_DOCK_DESKTOP_CHANGED, (CairoDockNotificationFunc) on_change_desktop, CAIRO_DOCK_RUN_AFTER);/*Notifier d'un changement de bureau*/
	
	//\___________________ On calcule la geometrie de l'icone en mode compact.
	cd_switcher_compute_nb_lines_and_columns ();
	
	//\___________________ On recupere le bureau courant et sa position sur la grille.
	cd_switcher_get_current_desktop ();
	
	//\___________________ On charge le bon nombre d'icones dans le sous-dock ou le desklet.
	cd_switcher_load_icons ();
	
	//\___________________ On dessine l'icone principale.
	cd_switcher_draw_main_icon ();
	
	//\___________________ On affiche le numero du bureau courant.
	if (myConfig.bDisplayNumDesk)
	{
		int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1)
	}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	cairo_dock_remove_notification_func (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, (CairoDockNotificationFunc) on_change_screen_geometry);
	cairo_dock_remove_notification_func (CAIRO_DOCK_DESKTOP_CHANGED, (CairoDockNotificationFunc) on_change_desktop);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (SWITCHER_DEFAULT_NAME);
	
	if (myDesklet != NULL)
	{
		if (myConfig.bCompactView)
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple")
		}
		else
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Caroussel")
		}
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myConfig.bDisplayNumDesk)
		{
			int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1)
		}
		else
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
			
		cd_switcher_load_icons ();
	}
	else
	{
		cd_switcher_load_default_map_surface ();
	}
	
	cd_switcher_draw_main_icon ();
CD_APPLET_RELOAD_END
