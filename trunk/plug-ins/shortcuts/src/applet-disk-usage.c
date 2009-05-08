/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <mntent.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-disk-usage.h"


void cd_shortcuts_get_disk_usage (CairoDockModuleInstance *myApplet)
{
	static struct statfs sts;
	
	const gchar *cMountPath;
	GList *pElement = myData.pDiskUsageList;
	CDDiskUsage *pDiskUsage;
	long long iAvail, iFree, iTotal, iUsed, iType;
	Icon *pIcon;
	GList *ic;
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->iType != 6)
			break;
		if (pIcon->acCommand != NULL)
		{
			cMountPath = (strncmp (pIcon->acCommand, "file://", 7) == 0 ? pIcon->acCommand + 7 : pIcon->acCommand);
			cd_debug ("checking device on '%s'...", cMountPath);
			
			if (pElement != NULL)
			{
				pDiskUsage = pElement->data;
				pElement = pElement->next;
			}
			else
			{
				pDiskUsage = g_new0 (CDDiskUsage, 1);
				myData.pDiskUsageList = g_list_append (myData.pDiskUsageList, pDiskUsage);
			}
			
			if (statfs (cMountPath, &sts) == 0)
			{
				if (pDiskUsage->iType == 0)
					pDiskUsage->iType = sts.f_type;
				pDiskUsage->iPrevAvail = pDiskUsage->iAvail;
				pDiskUsage->iAvail = (long long)sts.f_bavail * sts.f_bsize;  // Blocs libres pour utilisateurs
				pDiskUsage->iFree  = (long long)sts.f_bfree  * sts.f_bsize;  // Blocs libres
				pDiskUsage->iTotal = (long long)sts.f_blocks * sts.f_bsize;  // Nombre total de blocs
				pDiskUsage->iUsed  = pDiskUsage->iTotal - pDiskUsage->iAvail;
				cd_debug ("%d / %d", (int)pDiskUsage->iAvail, (int)pDiskUsage->iTotal);
			}
		}
	}
}

gboolean cd_shortcuts_update_disk_usage (CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myData.pDiskUsageList != NULL, TRUE);
	
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	GList *pElement = myData.pDiskUsageList;
	CDDiskUsage *pDiskUsage;
	Icon *pIcon;
	double fValue;
	GList *ic;
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->iType != 6)
			break;
		if (pIcon->acCommand != NULL && pElement != NULL)
		{
			pDiskUsage = pElement->data;
			if (pDiskUsage->iPrevAvail != pDiskUsage->iAvail)
			{
				switch (myConfig.iDisplayType)
				{
					case CD_SHOW_FREE_SPACE :
						fValue = (double) pDiskUsage->iAvail / pDiskUsage->iTotal;
						cairo_dock_set_size_as_quick_info (myDrawContext, pIcon, pContainer, pDiskUsage->iAvail);
					break ;
					case CD_SHOW_USED_SPACE :
						fValue = (double) pDiskUsage->iUsed / pDiskUsage->iTotal;
						cairo_dock_set_size_as_quick_info (myDrawContext, pIcon, pContainer, pDiskUsage->iUsed);
					break ;
					case CD_SHOW_FREE_SPACE_PERCENT :
						fValue = (double) pDiskUsage->iAvail / pDiskUsage->iTotal;
						cairo_dock_set_quick_info_full (myDrawContext, pIcon, pContainer, "%.1f%%", 100.*fValue);
					break ;
					case CD_SHOW_USED_SPACE_PERCENT :
						g_print ("+%ld / %ld\n", pDiskUsage->iUsed, pDiskUsage->iTotal);
						fValue = (double) pDiskUsage->iUsed / pDiskUsage->iTotal;
						cairo_dock_set_quick_info_full (myDrawContext, pIcon, pContainer, "%.1f%%", 100.*fValue);
					break ;
				}
				
				if (myConfig.bDrawBar)
				{
					int iWidth, iHeight;
					cairo_dock_get_icon_extent (pIcon, pContainer, &iWidth, &iHeight);
					cairo_surface_t *pSurface = cairo_dock_create_surface_for_icon (pIcon->acFileName, myDrawContext, iWidth, iHeight);;
					cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
					
					cairo_dock_set_icon_surface_with_bar (pIconContext, pSurface, fValue, pIcon, pContainer);
					
					cairo_destroy (pIconContext);
					cairo_surface_destroy (pSurface);
				}
				
				if (pDiskUsage->iPrevAvail != 0)
					cairo_dock_redraw_icon (pIcon, pContainer);
			}
			pElement = pElement->next;
		}
	}
	
	return TRUE;
}


void cd_shortcuts_stop_disk_measure (CairoDockModuleInstance *myApplet)
{
	cairo_dock_stop_measure_timer (myData.pDiskMeasureTimer);
	g_list_foreach (myData.pDiskUsageList, (GFunc) g_free, NULL);
	g_list_free (myData.pDiskUsageList);
	myData.pDiskUsageList = NULL;
}

void cd_shortcuts_launch_disk_measure (CairoDockModuleInstance *myApplet)
{
	if (myConfig.iDisplayType != CD_SHOW_NOTHING)
	{
		if (myData.pDiskMeasureTimer == NULL)
		{
			myData.pDiskMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
				NULL,
				(CairoDockReadTimerFunc) cd_shortcuts_get_disk_usage, 
				(CairoDockUpdateTimerFunc) cd_shortcuts_update_disk_usage, 
				myApplet);
		}
		cairo_dock_launch_measure (myData.pDiskMeasureTimer);
	}
}



gchar *cd_shortcuts_get_fs_type (const gchar *cMountPath)
{
	struct mntent *me;
	FILE *mtab = setmntent ("/etc/mtab", "r");
	char *search_path;
	int match;
	char *slash;

	if (mtab == NULL)
	{
		cd_warning ("couldn't open /etc/mtab");
		return NULL;
	}
	
	gchar *cFsType = NULL;
	while ((me = getmntent (mtab)) != NULL)
	{
		if (me->mnt_dir && strcmp (me->mnt_dir, cMountPath) == 0)
		{
			cFsType = g_strdup (me->mnt_dir);
			break ;
		}
	}
	
	endmntent (mtab);
	
	return cFsType;
}