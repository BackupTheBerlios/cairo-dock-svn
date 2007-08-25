/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <cairo.h>
#include <pango/pango.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-icons.h"
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-desktop-file-factory.h"


extern gchar *g_cCurrentThemePath;

extern gchar *g_cDefaultFileBrowser;


gchar *cairo_dock_add_desktop_file_from_path (gchar *cFilePath, gchar *cDockName, double fOrder, CairoDock *pDock, GError **erreur)
{
	//g_print ("%s (%s)\n", __func__, cFilePath);
	double fEffectiveOrder;
	if (fOrder == CAIRO_DOCK_LAST_ORDER)
	{
		Icon *pLastIcon = cairo_dock_get_last_launcher (pDock->icons);
		if (pLastIcon != NULL)
			fEffectiveOrder = pLastIcon->fOrder + 1;
		else
			fEffectiveOrder = 1;
	}
	else
		fEffectiveOrder = fOrder;
	
	GError *tmp_erreur = NULL;
	gchar *cNewDesktopFileName;
	gchar *cNewDesktopContent;
	gsize lenght;
	
	//\_________________ On regarde si c'est un repertoire ou un fichier ou sinon un fichier cree a partir de zero.
	if (cFilePath == NULL)
	{
		//\___________________ On cree le texte qu'on va y mettre par defaut.
		cNewDesktopContent = g_strdup_printf ("#!\n[Desktop Entry]\n\
#s Launcher's name :\n\
Name = New Launcher\n\
Name[fr] = Nouveau Lanceur\n\
#s Exec command :\n\
Exec = echo 'edit me !'\n\
#S Image's name or path :\n\
Icon = \n\
#f Order you want for this launcher among the other launchers :\n\
Order = %d\n\
#s Name of the container it belongs to:\n\
Container = %s\n\
#b Is this icon a container ?\n\
Is container = false",
		(int) fEffectiveOrder, cDockName);
		lenght = -1;
		
		//\___________________ On lui choisit un nom de fichier tel qu'il n'y ait pas de collision.
		cNewDesktopFileName = cairo_dock_generate_desktop_filename (g_cCurrentThemePath);
	}
	else if (g_file_test (cFilePath, G_FILE_TEST_IS_DIR))
	{
		//\___________________ On cree le texte qu'on va y mettre par defaut.
		cNewDesktopContent = g_strdup_printf ("#!\n[Desktop Entry]\n\
#s Launcher's name :\n\
Name = %s\n\
Name[fr] = %s\n\
#s Exec command :\n\
Exec = %s %s\n\
#S Image's name or path :\n\
Icon = %s\n\
#f Order you want for this launcher among the other launchers :\n\
Order = %f\n\
#s Name of the container it belongs to:\n\
Container = %s\n\
#b Is this icon a container ?\n\
Is container = false",
		cFilePath, cFilePath, g_cDefaultFileBrowser, cFilePath, g_cDefaultFileBrowser, fEffectiveOrder, cDockName);
		lenght = -1;
		
		//\___________________ On lui choisit un nom de fichier tel qu'il n'y ait pas de collision.
		cNewDesktopFileName = cairo_dock_generate_desktop_filename (g_cCurrentThemePath);
	}
	else
	{
		//\_________________ On ouvre le fichier pour controler certain champ.
		GKeyFile *pKeyFile = g_key_file_new();
		g_key_file_load_from_file (pKeyFile, cFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
		
		//\_________________ On enleve les %F, %U et autres du champ "Exec".
		gchar *cExecField = g_key_file_get_string (pKeyFile, "Desktop Entry", "Exec", &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
		
		gchar *cUndesiredArgument = strchr (cExecField, '%');
		if (cUndesiredArgument != NULL)
			*cUndesiredArgument = '\0';
		g_key_file_set_string (pKeyFile, "Desktop Entry", "Exec", cExecField);
		
		//\_________________ On lui rajoute les champ propres a Cairo-Dock.
		g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", fEffectiveOrder);
		g_key_file_set_string (pKeyFile, "Desktop Entry", "Container", cDockName);
		g_key_file_set_boolean (pKeyFile, "Desktop Entry", "Is container", FALSE);
		
		//\_________________ Et surtout on vire l'infame usine a gaz de traduction automatique d'Ubuntu !
		g_key_file_remove_key (pKeyFile, "Desktop Entry", "X-Ubuntu-Gettext-Domain", &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_error_free (tmp_erreur);
			tmp_erreur = NULL;
		}
		//else  // il y en a 2 !
		//	g_key_file_set_string (pKeyFile, "Desktop Entry", "X-Ubuntu-Gettext-Domain", "");
		
		//\_________________ On en fait un fichier de conf evolue.
		if (! cairo_dock_is_advanced_keyfile (pKeyFile))
		{
			g_key_file_set_comment (pKeyFile, NULL, NULL, "!", NULL);
			g_key_file_set_comment (pKeyFile, "Desktop Entry", "Name", "s Launcher's name :", NULL);
			g_key_file_set_comment (pKeyFile, "Desktop Entry", "Exec", "s Exec command :", NULL);
			g_key_file_set_comment (pKeyFile, "Desktop Entry", "Icon", "S Image's name or path :", NULL);
			g_key_file_set_comment (pKeyFile, "Desktop Entry", "Order", "f Order you want for this launcher among the other launchers :", NULL);
			g_key_file_set_comment (pKeyFile, "Desktop Entry", "Container", "s Name of the container it belongs to:", NULL);
			g_key_file_set_comment (pKeyFile, "Desktop Entry", "Is container", "b Is this icon a container ?", NULL);
		}
		
		//\_________________ On ecrit tout dans un nouveau fichier portant le meme nom dans le repertoire .cairo-dock.
		cNewDesktopContent = g_key_file_to_data (pKeyFile, &lenght, &tmp_erreur);
		g_key_file_free (pKeyFile);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return NULL;
		}
		
		cNewDesktopFileName = g_path_get_basename (cFilePath);
	}
	
	gchar *cNewFilePath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, cNewDesktopFileName);
	g_free (cNewDesktopFileName);
	
	g_file_set_contents (cNewFilePath, cNewDesktopContent, lenght, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		g_free (cNewDesktopContent);
		g_free (cNewFilePath);
		return NULL;
	}
	
	g_free (cNewDesktopContent);
	return cNewFilePath;
}


gchar *cairo_dock_generate_desktop_filename (gchar *cCairoDockDataDir)
{
	int iPrefixNumber, i = -1;
	gchar *cFileName;
	struct dirent *dirpointer;
	DIR* dir = opendir (cCairoDockDataDir);
	g_return_val_if_fail (dir != NULL, NULL);
	do
	{
		i ++;
		rewinddir (dir);

		while ((dirpointer = readdir (dir)) != NULL)
		{
			cFileName = dirpointer->d_name;
			if (g_str_has_suffix (cFileName, ".desktop"))
			{
				iPrefixNumber = atoi (cFileName);
				if (i == iPrefixNumber)
					break;
			}
		}
	}
	while (dirpointer != NULL);  // si on a pas parcouru tout le repertoire, c'est qu'on a trouve une collision.
	
	return g_strdup_printf ("%02d%s", i, "launcher.desktop");
}

