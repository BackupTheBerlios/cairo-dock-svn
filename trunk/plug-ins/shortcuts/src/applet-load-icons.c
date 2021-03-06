/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-bookmarks.h"
#include "applet-disk-usage.h"
#include "applet-load-icons.h"


static void cd_shortcuts_on_change_drives (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	cd_shortcuts_stop_disk_periodic_task (myApplet);
	
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 6);
	
	//\________________ On met a jour les signets qui pointeraient sur un repertoire du point de montage nouvellement (de)monte.
	GList *ic;
	Icon *icon;
	gboolean bIsMounted;
	gchar *cTargetURI = cairo_dock_fm_is_mounted (cURI, &bIsMounted);
	if (cTargetURI == NULL)  // version bourrinne.
	{
		cd_shortcuts_on_change_bookmarks (CAIRO_DOCK_FILE_MODIFIED, NULL, myApplet);  // NULL <=> on recharge tout.
	}
	else  // version optimisee.
	{
		for (ic = (myDock ? myIcon->pSubDock->icons : myDesklet->icons); ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->iType == 10)
			{
				if (strncmp (cTargetURI, icon->cBaseURI, strlen (cTargetURI)) == 0)
				{
					cd_message ("le signet %s est situe sur un point de montage ayant change (%s)", icon->cBaseURI, cTargetURI);
					gchar *cName = NULL, *cRealURI = NULL, *cIconName = NULL, *cUserName = NULL;
					int iVolumeID = 0;
					gboolean bIsDirectory = FALSE;
					double fOrder;
					if (cairo_dock_fm_get_file_info (icon->cBaseURI, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, mySystem.iFileSortType))
					{
						g_print (" -> %s (%d)\n", cIconName, bIsMounted);
						g_free (icon->acName);
						if (bIsMounted || cIconName == NULL)
							icon->acName = cName;
						else
						{
							icon->acName = g_strdup_printf ("%s\n[%s]", cName, D_("Unmounted"));
							g_free (cName);
						}
						g_free (icon->acCommand);
						icon->acCommand = cRealURI;
						g_free (icon->acFileName);
						icon->acFileName = cIconName;
						icon->iVolumeID = iVolumeID;
						cairo_dock_load_one_icon_from_scratch (icon, (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer));
					}
				}
			}
		}
		g_free (cTargetURI);
	}
	
	cd_shortcuts_launch_disk_periodic_task (myApplet);
}
static void cd_shortcuts_on_change_network (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 8);
}


static GList * _load_icons (CairoDockModuleInstance *myApplet)
{
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;
	
	if (myConfig.bListDrives)
	{
		pIconList = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_VFS_ROOT, CAIRO_DOCK_FM_SORT_BY_NAME, 6, FALSE, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		if (pIconList == NULL)
		{
			cd_warning ("couldn't detect any drives");  // on decide de poursuivre malgre tout, pour les signets.
		}
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_drives, myApplet))
			cd_warning ("Shortcuts : can't monitor drives");
		myData.cDisksURI = cFullURI;
	}
	
	if (myConfig.bListNetwork)
	{
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, 8, FALSE, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		
		if (myConfig.bUseSeparator && pIconList2 != NULL && pIconList != NULL)
		{
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 7;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_network, myApplet))
			cd_warning ("Shortcuts : can't monitor network");
		myData.cNetworkURI = cFullURI;
	}
		
	if (myConfig.bListBookmarks)
	{
		gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
		if (! g_file_test (cBookmarkFilePath, G_FILE_TEST_EXISTS))  // on le cree pour pouvoir ajouter des signets.
		{
			FILE *f = fopen (cBookmarkFilePath, "a");
			fclose (f);
		}
		
		GList *pIconList2 = cd_shortcuts_list_bookmarks (cBookmarkFilePath);
		
		if (myConfig.bUseSeparator && pIconList2 != NULL && pIconList != NULL)
		{
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 9;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cBookmarkFilePath, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_bookmarks, myApplet))
			cd_warning ("Shortcuts : can't monitor bookmarks");
		
		myData.cBookmarksURI = cBookmarkFilePath;
	}
	
	return pIconList;
}


void cd_shortcuts_get_shortcuts_data (CairoDockModuleInstance *myApplet)
{
	myData.pIconList = _load_icons (myApplet);
}


gboolean cd_shortcuts_build_shortcuts_from_data (CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myIcon != NULL, FALSE);  // paranoia
	/*if (myIcon == NULL)
	{
		g_print ("annulation du chargement des raccourcis\n");
		g_list_foreach (myData.pIconList, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myData.pIconList);
		myData.pIconList = NULL;
		return FALSE;
	}*/
	
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On charge la nouvelle liste.
	const gchar *cDeskletRendererName = NULL;
	switch (myConfig.iDeskletRendererType)
	{
		case CD_DESKLET_SLIDE :
		default :
			cDeskletRendererName = "Slide";
		break ;
		
		case CD_DESKLET_TREE :
			cDeskletRendererName = "Tree";
		break ;
	}
	CD_APPLET_LOAD_MY_ICONS_LIST (myData.pIconList, myConfig.cRenderer, cDeskletRendererName, NULL);
	myData.pIconList = NULL;
	
	//\_______________________ On lance la tache de mesure des disques.
	cd_shortcuts_launch_disk_periodic_task (myApplet);
	
	return TRUE;
}

