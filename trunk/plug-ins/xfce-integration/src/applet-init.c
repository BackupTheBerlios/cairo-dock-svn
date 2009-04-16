/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Christophe Chapuis (for any bug report, please mail me to chris.chapuis@gmail.com)

******************************************************************************/
#include "stdlib.h"

#include "applet-thunar-vfs.h"
#include "applet-utils.h"
#include "applet-init.h"


CD_APPLET_PRE_INIT_BEGIN ("xfce integration",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_PLUG_IN,
	N_("This applet provides functions for a better integration into a XFCE environnement.\n"
	"It is auto-activated, so you don't need to activate it."),
	"Tofe (Christophe Chapuis")
	if (g_iDesktopEnv == CAIRO_DOCK_XFCE)
	{
		if (init_vfs_backend ())
		{
			CairoDockDesktopEnvBackend *pVFSBackend = g_new0 (CairoDockDesktopEnvBackend, 1);
			
			pVFSBackend->get_file_info = vfs_backend_get_file_info;
			pVFSBackend->get_file_properties = vfs_backend_get_file_properties;
			pVFSBackend->list_directory = vfs_backend_list_directory;
			pVFSBackend->launch_uri = vfs_backend_launch_uri;
			pVFSBackend->is_mounted = vfs_backend_is_mounted;
			pVFSBackend->mount = vfs_backend_mount;
			pVFSBackend->unmount = vfs_backend_unmount;
			pVFSBackend->add_monitor = vfs_backend_add_monitor;
			pVFSBackend->remove_monitor = vfs_backend_remove_monitor;
			pVFSBackend->delete_file = vfs_backend_delete_file;
			pVFSBackend->rename = vfs_backend_rename_file;
			pVFSBackend->move = vfs_backend_move_file;
			pVFSBackend->get_trash_path = vfs_backend_get_trash_path;
			pVFSBackend->get_desktop_path = vfs_backend_get_desktop_path;
			pVFSBackend->logout = env_backend_logout;
			pVFSBackend->shutdown = env_backend_logout;  /// a confirmer ...
			pVFSBackend->setup_time = env_backend_setup_time;
			pVFSBackend->show_system_monitor = env_backend_show_system_monitor;
			cairo_dock_fm_register_vfs_backend (pVFSBackend);
		}
	}
	else
		return FALSE;
CD_APPLET_PRE_INIT_END
