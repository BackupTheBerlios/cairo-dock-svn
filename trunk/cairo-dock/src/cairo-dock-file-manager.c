/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>

#include "cairo-dock-dock-factory.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-load.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-file-manager.h"

extern CairoDockDesktopEnv g_iDesktopEnv;
extern CairoDockFMSortType g_iFileSortType;
extern GHashTable *g_hDocksTable;

static CairoDockVFSBackend *s_pVFSBackend = NULL;


void cairo_dock_fm_register_vfs_backend (CairoDockVFSBackend *pVFSBackend)
{
	g_free (s_pVFSBackend);
	s_pVFSBackend = pVFSBackend;
}



GList * cairo_dock_fm_list_directory (const gchar *cURI, CairoDockFMSortType g_fm_iSortType, gchar **cFullURI)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->list_directory != NULL)
	{
		return s_pVFSBackend->list_directory (cURI, g_fm_iSortType, cFullURI);
	}
	else
	{
		cFullURI = NULL;
		return NULL;
	}
}

gboolean cairo_dock_fm_get_file_info (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->get_file_info != NULL)
	{
		s_pVFSBackend->get_file_info (cBaseURI, cName, cURI, cIconName, bIsDirectory, iVolumeID, fOrder, iSortType);
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_get_file_properties (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->get_file_properties != NULL)
	{
		s_pVFSBackend->get_file_properties (cURI, iSize, iLastModificationTime, cMimeType, iUID, iGID, iPermissionsMask);
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_launch_uri (const gchar *cURI)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->launch_uri != NULL)
	{
		s_pVFSBackend->launch_uri (cURI);
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_add_monitor_full (const gchar *cURI, gboolean bDirectory, const gchar *cMountedURI, CairoDockFMMonitorCallback pCallback, gpointer data)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->add_monitor != NULL)
	{
		if (cMountedURI != NULL && strcmp (cMountedURI, cURI) != 0)
		{
			s_pVFSBackend->add_monitor (cURI, FALSE, pCallback, data);
			if (bDirectory)
				s_pVFSBackend->add_monitor (cMountedURI, TRUE, pCallback, data);
		}
		else
		{
			s_pVFSBackend->add_monitor (cURI, bDirectory, pCallback, data);
		}
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_remove_monitor_full (const gchar *cURI, gboolean bDirectory, const gchar *cMountedURI)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->remove_monitor != NULL)
	{
		s_pVFSBackend->remove_monitor (cURI);
		if (cMountedURI != NULL && strcmp (cMountedURI, cURI) != 0 && bDirectory)
		{
			s_pVFSBackend->remove_monitor (cMountedURI);
		}
		return TRUE;
	}
	else
		return FALSE;
}



gboolean cairo_dock_fm_mount_full (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->mount != NULL && iVolumeID > 0)
	{
		s_pVFSBackend->mount (cURI, iVolumeID, pCallback, icon, pDock);
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_unmount_full (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->unmount != NULL && iVolumeID > 0)
	{
		s_pVFSBackend->unmount (cURI, iVolumeID, pCallback, icon, pDock);
		return TRUE;
	}
	else
		return FALSE;
}

gchar *cairo_dock_fm_is_mounted (const gchar *cURI, gboolean *bIsMounted)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->is_mounted != NULL)
		return s_pVFSBackend->is_mounted (cURI, bIsMounted);
	else
		return NULL;
}

gboolean cairo_dock_fm_delete_file (const gchar *cURI)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->delete != NULL)
	{
		return s_pVFSBackend->delete (cURI);
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_rename_file (const gchar *cOldURI, const gchar *cNewName)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->rename != NULL)
	{
		return s_pVFSBackend->rename (cOldURI, cNewName);
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_move_file (const gchar *cURI, const gchar *cDirectoryURI)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->move != NULL)
	{
		return s_pVFSBackend->move (cURI, cDirectoryURI);
	}
	else
		return FALSE;
}


gchar *cairo_dock_fm_get_trash_path (const gchar *cNearURI, gboolean bCreateIfNecessary)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->get_trash_path != NULL)
	{
		return s_pVFSBackend->get_trash_path (cNearURI, bCreateIfNecessary);
	}
	else
		return NULL;
}

gchar *cairo_dock_fm_get_desktop_path (void)
{
	if (s_pVFSBackend != NULL && s_pVFSBackend->get_desktop_path != NULL)
	{
		return s_pVFSBackend->get_desktop_path ();
	}
	else
		return NULL;
}


Icon *cairo_dock_fm_create_icon_from_URI (const gchar *cURI, CairoDock *pDock)
{
	if (s_pVFSBackend == NULL)
		return NULL;
	Icon *pNewIcon = g_new0 (Icon, 1);
	pNewIcon->iType = CAIRO_DOCK_LAUNCHER;
	pNewIcon->cBaseURI = g_strdup (cURI);
	gboolean bIsDirectory;
	s_pVFSBackend->get_file_info (cURI, &pNewIcon->acName, &pNewIcon->acCommand, &pNewIcon->acFileName, &bIsDirectory, &pNewIcon->iVolumeID, &pNewIcon->fOrder, g_iFileSortType);
	if (pNewIcon->acName == NULL)
	{
		cairo_dock_free_icon (pNewIcon);
		return NULL;
	}
	
	if (bIsDirectory)
	{
		g_print ("  c'est un sous-repertoire\n");
	}
	
	if (g_iFileSortType == CAIRO_DOCK_FM_SORT_BY_NAME)
	{
		GList *ic;
		Icon *icon;
		for (ic = pDock->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (strcmp (pNewIcon->acName, icon->acName) < 0)
			{
				if (ic->prev != NULL)
				{
					Icon *prev_icon = ic->prev->data;
					pNewIcon->fOrder = (icon->fOrder + prev_icon->fOrder) / 2;
				}
				else
					pNewIcon->fOrder = icon->fOrder - 1;
				break ;
			}
			else if (ic->next == NULL)
			{
				pNewIcon->fOrder = icon->fOrder + 1;
			}
		}
	}
	cairo_dock_load_one_icon_from_scratch (pNewIcon, pDock);
	
	return pNewIcon;
}

void cairo_dock_fm_create_dock_from_directory (Icon *pIcon)
{
	if (s_pVFSBackend == NULL)
		return;
	g_print ("%s ()\n", __func__);
	g_free (pIcon->acCommand);
	GList *pIconList = cairo_dock_fm_list_directory (pIcon->cBaseURI, g_iFileSortType, &pIcon->acCommand);
	pIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, pIcon->acName);
	
	cairo_dock_update_dock_size (pIcon->pSubDock);  // le 'load_buffer' ne le fait pas.
	
	cairo_dock_fm_add_monitor (pIcon);
}



static Icon *cairo_dock_fm_alter_icon_if_necessary (Icon *pIcon, CairoDock *pDock)
{
	if (s_pVFSBackend == NULL)
		return NULL;
	Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (pIcon->cBaseURI, pDock);
	g_return_val_if_fail (pNewIcon != NULL && pNewIcon->acName != NULL, NULL);
	
	if (strcmp (pIcon->acName, pNewIcon->acName) != 0 || strcmp (pIcon->acFileName, pNewIcon->acFileName) != 0 || pIcon->fOrder != pNewIcon->fOrder)
	{
		g_print ("  on remplace %s\n", pIcon->acName);
		cairo_dock_remove_one_icon_from_dock (pDock, pIcon);
		if (pIcon->acDesktopFileName != NULL)
			cairo_dock_fm_remove_monitor (pIcon);
		
		pNewIcon->acDesktopFileName = g_strdup (pIcon->acDesktopFileName);
		pNewIcon->cParentDockName = g_strdup (pIcon->cParentDockName);
		if (pIcon->pSubDock != NULL)
		{
			pNewIcon->pSubDock == pIcon->pSubDock;
			pIcon->pSubDock = NULL;
			
			if (pNewIcon->acName != NULL && strcmp (pIcon->acName, pNewIcon->acName) != 0)
			{
				g_hash_table_steal (g_hDocksTable, pIcon->acName);
				g_hash_table_insert (g_hDocksTable, pNewIcon->acName, pNewIcon->pSubDock);
			}  // else : detruire le sous-dock.
		}
		pNewIcon->fX = pIcon->fX;
		pNewIcon->fXAtRest = pIcon->fXAtRest;
		pNewIcon->fDrawX = pIcon->fDrawX;
		
		cairo_dock_insert_icon_in_dock (pNewIcon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);  // on met a jour la taille du dock pour le fXMin/fXMax, et eventuellement la taille de l'icone peut aussi avoir change.
		
		cairo_dock_redraw_my_icon (pNewIcon, pDock);
		
		if (pNewIcon->acDesktopFileName != NULL)
			cairo_dock_fm_add_monitor (pNewIcon);
		
		cairo_dock_free_icon (pIcon);
		return pNewIcon;
	}
	else
	{
		cairo_dock_free_icon (pNewIcon);
		return pIcon;
	}
}
void cairo_dock_fm_action_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	g_return_if_fail (cURI != NULL && pIcon != NULL);
	g_print ("%s (%d sur %s)\n", __func__, iEventType, cURI);
	
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :
		{
			Icon *pConcernedIcon;
			CairoDock *pParentDock;
			if (strcmp (cURI, pIcon->cBaseURI) == 0)
			{
				pConcernedIcon = pIcon;
				pParentDock = cairo_dock_search_container_from_icon (pIcon);
			}
			else if (pIcon->pSubDock != NULL)
			{
				pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIcon->pSubDock->icons, cURI);
				g_return_if_fail (pConcernedIcon != NULL);
				pParentDock = pIcon->pSubDock;
			}
			else
			{
				g_print ("  on n'aurait pas du recevoir cet evenement !\n");
				return ;
			}
			g_print ("  %s sera supprimee\n", pConcernedIcon->acName);
			
			cairo_dock_remove_one_icon_from_dock (pParentDock, pConcernedIcon);
			if (pConcernedIcon->acDesktopFileName != NULL)  // alors elle a un moniteur.
				cairo_dock_fm_remove_monitor (pConcernedIcon);
			cairo_dock_update_dock_size (pParentDock);
			cairo_dock_free_icon (pConcernedIcon);
		}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :
		{
			if (strcmp (cURI, pIcon->cBaseURI) != 0 && pIcon->pSubDock != NULL)  // dans des cas foirreux, il se peut que le fichier soit cree alors qu'il existait deja dans le dock.
			{
				Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, pIcon->pSubDock);
				g_return_if_fail (pNewIcon != NULL);
				
				cairo_dock_insert_icon_in_dock (pNewIcon, pIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
				g_print ("  %s a ete insere(e)\n", (pNewIcon != NULL ? pNewIcon->acName : "aucune icone"));
			}
		}
		break ;
		
		case CAIRO_DOCK_FILE_MODIFIED :
		{
			Icon *pConcernedIcon;
			CairoDock *pParentDock;
			if (strcmp (pIcon->cBaseURI, cURI) == 0)  // c'est l'icone elle-meme.
			{
				pConcernedIcon = pIcon;
				pParentDock = cairo_dock_search_container_from_icon (pIcon);
				g_return_if_fail (pParentDock != NULL);
			}
			else if (pIcon->pSubDock != NULL)  // c'est a l'interieur du repertoire qu'elle represente.
			{
				pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIcon->pSubDock->icons, cURI);
				g_return_if_fail (pConcernedIcon != NULL);
				pParentDock = pIcon->pSubDock;
			}
			g_print ("  %s est modifiee (iRefCount:%d)\n", pConcernedIcon->acName, pParentDock->iRefCount);
			
			Icon *pNewIcon = cairo_dock_fm_alter_icon_if_necessary (pConcernedIcon, pParentDock);
			
			if (pNewIcon != NULL && pNewIcon != pConcernedIcon && pNewIcon->iVolumeID > 0)
			{
				gboolean bIsMounted = FALSE;
				if (s_pVFSBackend->is_mounted != NULL)
				{
					gchar *cActivationURI = s_pVFSBackend->is_mounted (pNewIcon->acCommand, &bIsMounted);
					g_free (cActivationURI);
				}
				gchar *cMessage = g_strdup_printf (_("%s is now %s"), pNewIcon->acName, (bIsMounted ? _("mounted") : _("unmounted")));
				
				cairo_dock_show_temporary_dialog (cMessage, pNewIcon, pParentDock, 4000);
				g_free (cMessage);
			}
		}
		break ;
	}
}

void cairo_dock_fm_action_after_mounting (gboolean bMounting, gboolean bSuccess, const gchar *cName, Icon *icon, CairoDock *pDock)  // FileManagerMountCallback.
{
	g_print ("%s (%s) : %d\n", __func__, (bMounting ? "mount" : "unmount"), bSuccess);  // en cas de demontage effectif, l'icone n'est plus valide !
	
	gchar *cMessage;
	if (! bSuccess && pDock != NULL)  // dans l'autre cas (succes), l'icone peut ne plus etre valide ! mais on s'en fout, puisqu'en cas de succes, il y'aura rechargement de l'icone, et donc on pourra balancer le message a ce moment-la.
	{
		cMessage = g_strdup_printf (_("failed to %s %s"), (bMounting ? _("mount") : _("unmount")), cName);
		if (icon != NULL)
			cairo_dock_show_temporary_dialog (cMessage, icon, pDock, 4000);
		else
		{
			cairo_dock_show_general_message (cMessage, 4000);
		}
		
		g_free (cMessage);
	}
	else if (icon == NULL)
	{
		cMessage = g_strdup_printf (_("failed to %s %s"), (bMounting ? _("mount") : _("unmount")), cName);
		cairo_dock_show_general_message (cMessage, 4000);
		g_free (cMessage);
	}
}



gboolean cairo_dock_fm_move_into_directory (const gchar *cURI, Icon *icon, CairoDock *pDock)
{
	g_return_val_if_fail (cURI != NULL && icon != NULL, FALSE);
	g_print (" -> copie de %s dans %s\n", cURI, icon->cBaseURI);
	gboolean bSuccess = cairo_dock_fm_move_file (cURI, icon->cBaseURI);
	if (! bSuccess)
	{
		g_print ("Attention : couldn't copy this file.\nCheck that you have writing rights, and that the new does not already exist.\n");
		gchar *cMessage = g_strdup_printf ("Attention : couldn't copy %s into %s.\nCheck that you have writing rights, and that the name does not already exist.", cURI, icon->cBaseURI);
		cairo_dock_show_temporary_dialog (cMessage, icon, pDock, 4000);
		g_free (cMessage);
	}
	return bSuccess;
}
