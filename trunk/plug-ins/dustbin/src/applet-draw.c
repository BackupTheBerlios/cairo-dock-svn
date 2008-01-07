/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include "cairo-dock.h"

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS

extern gchar **my_cTrashDirectoryList;
extern int *my_pTrashState;
extern cairo_surface_t *my_pEmptyBinSurface;
extern cairo_surface_t *my_pFullBinSurface;
extern int my_iState;


gboolean cd_dustbin_check_trashes (Icon *icon)
{
	//g_print ("%s ()\n", __func__);
	
	GDir *dir;
	int i = 0, iNewState = 0;
	const gchar *cFirstFileInBin;
	GError *erreur = NULL;
	while (my_cTrashDirectoryList[i] != NULL)
	{
		dir = g_dir_open (my_cTrashDirectoryList[i], 0, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
		}
		else
		{
			cFirstFileInBin = g_dir_read_name (dir);
			if (cFirstFileInBin != NULL)
			{
				//g_print ("%s est rempli\n", my_cTrashDirectoryList[i]);
				if (my_pTrashState[i] != CD_DUSTBIN_FULL)
				{
					my_pTrashState[i] = CD_DUSTBIN_FULL;
				}
			}
			else
			{
				//g_print ("%s est vide\n", my_cTrashDirectoryList[i]);
				if (my_pTrashState[i] != CD_DUSTBIN_EMPTY)
				{
					my_pTrashState[i] = CD_DUSTBIN_EMPTY;
				}
			}
			iNewState += my_pTrashState[i];
			
			g_dir_close (dir);
		}
		i ++;
	}
	
	if (my_iState != iNewState)
	{
		my_iState = iNewState;
		double fMaxScale = 1 + g_fAmplitude;
		cairo_save (myDrawContext);
		
		if (iNewState == CD_DUSTBIN_EMPTY)
		{
			//g_print (" -> on a vide la corbeille\n");
			g_return_val_if_fail (my_pEmptyBinSurface != NULL, TRUE);
			cairo_dock_set_icon_surface_with_reflect (myDrawContext, my_pEmptyBinSurface, icon, myDock);
		}
		else
		{
			//g_print (" -> on a rempli la corbeille\n");
			g_return_val_if_fail (my_pFullBinSurface != NULL, TRUE);
			cairo_dock_set_icon_surface_with_reflect (myDrawContext, my_pFullBinSurface, icon, myDock);
		}
		cairo_restore (myDrawContext);  // retour a l'etat initial.
		
		cairo_dock_redraw_my_icon (icon, myDock);
	}
	
	return TRUE;
}