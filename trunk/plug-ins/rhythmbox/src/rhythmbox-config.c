#include <string.h>
#include <stdlib.h>
#include <cairo-dock.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-config.h"

gchar *conf_defaultTitle = NULL;
CairoDockAnimationType conf_changeAnimation;
gboolean conf_enableDialogs;
double conf_timeDialogs;


void rhythmbox_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName)
{
	GError *erreur = NULL;
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, cName, &bFlushConfFileNeeded);
	g_return_if_fail (pKeyFile != NULL);
	
	conf_enableDialogs = cairo_dock_get_boolean_key_value (pKeyFile, "Configuration", "enable_dialogs", &bFlushConfFileNeeded, TRUE);
	
	conf_timeDialogs = cairo_dock_get_double_key_value (pKeyFile, "Configuration", "time_dialogs", &bFlushConfFileNeeded, 3000);
	
	conf_changeAnimation = cairo_dock_get_animation_type_key_value (pKeyFile, "Configuration", "change_animation", &bFlushConfFileNeeded, "rotate");
	
	if (bFlushConfFileNeeded)
	{
		cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
	}
	
	g_key_file_free (pKeyFile);
}
