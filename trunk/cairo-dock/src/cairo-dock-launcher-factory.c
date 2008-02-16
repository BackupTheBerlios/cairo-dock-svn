/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-file-manager.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-log.h"

extern CairoDock *g_pMainDock;
extern double g_fSubDockSizeRatio;

extern double g_fAmplitude;
extern int g_iIconGap;

extern int g_iLabelSize;
extern gboolean g_bTextAlwaysHorizontal;
extern gchar *g_cLabelPolice;

extern gchar *g_cConfFile;
extern gchar *g_cCurrentLaunchersPath;
extern gpointer *g_pDefaultIconDirectory;

extern gboolean g_bDirectionUp;
extern gboolean g_bSameHorizontality;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUseGlitz;
extern CairoDockFMSortType g_iFileSortType;


gchar *cairo_dock_search_icon_s_path (gchar *cFileName)
{
	g_return_val_if_fail (cFileName != NULL, NULL);
	GString *sIconPath = g_string_new ("");
	gchar *cSuffixTab[4] = {".svg", ".png", ".xpm", NULL};
	gboolean bAddSuffix, bFileFound;
	GtkIconInfo* pIconInfo = NULL;
	int i, j;

	//\_______________________ On construit le chemin de l'icone a afficher.
	if (*cFileName == '~')
	{
		g_string_printf (sIconPath, "%s%s", g_getenv ("HOME"), cFileName+1);
	}
	else if (*cFileName == '/')
	{
		g_string_printf (sIconPath, "%s", cFileName);
	}
	else
	{
		//\_______________________ On determine si le suffixe est present ou non.
		bAddSuffix = FALSE;
		j = 0;
		while (cSuffixTab[j] != NULL && ! g_str_has_suffix (cFileName, cSuffixTab[j]))
			j ++;

		if (cSuffixTab[j] == NULL)
			bAddSuffix = TRUE;

		//\_______________________ On parcourt les repertoires disponibles, en testant tous les suffixes connus.
		i = 0;
		bFileFound = FALSE;
		if (g_pDefaultIconDirectory != NULL)
		{
			while ((g_pDefaultIconDirectory[2*i] != NULL || g_pDefaultIconDirectory[2*i+1] != NULL) && ! bFileFound)
			{
				if (g_pDefaultIconDirectory[2*i] != NULL)
				{
					//g_print ("on recherche %s dans le repertoire %s\n", sIconPath->str, g_pDefaultIconDirectory[2*i]);
					j = 0;
					while (! bFileFound && (cSuffixTab[j] != NULL || ! bAddSuffix))
					{
						g_string_printf (sIconPath, "%s/%s", g_pDefaultIconDirectory[2*i], cFileName);
						if (bAddSuffix)
							g_string_append_printf (sIconPath, "%s", cSuffixTab[j]);
						//g_print ("  -> %s\n", sIconPath->str);
						if ( g_file_test (sIconPath->str, G_FILE_TEST_EXISTS) )
							bFileFound = TRUE;

						j ++;
						if (! bAddSuffix)
							break;
					}
				}
				else if (g_pDefaultIconDirectory[2*i+1] != NULL)
				{
					g_string_printf (sIconPath, "%s", cFileName);
					if (! bAddSuffix)
					{
						gchar *str = strrchr (sIconPath->str, '.');
						if (str != NULL)
							*str = '\0';
					}
					//g_print ("on recherche %s dans le theme d'icones\n", sIconPath->str);
					pIconInfo = gtk_icon_theme_lookup_icon  (GTK_ICON_THEME (g_pDefaultIconDirectory[2*i+1]),
						sIconPath->str,
						64,
						GTK_ICON_LOOKUP_FORCE_SVG);
					if (pIconInfo != NULL)
					{
						g_string_printf (sIconPath, "%s", gtk_icon_info_get_filename (pIconInfo));
						bFileFound = TRUE;
						gtk_icon_info_free (pIconInfo);
					}
				}
				i ++;
			}
		}

		if (! bFileFound)
		{
			g_string_printf (sIconPath, "%s", cFileName);
			if (! bAddSuffix)
			{
				gchar *str = strrchr (sIconPath->str, '.');
				if (str != NULL)
					*str = '\0';
			}
			//g_print ("on recherche %s dans le theme par defaut.\n", sIconPath->str);
			GtkIconTheme *pDefaultIconTheme = gtk_icon_theme_get_default ();
			pIconInfo = gtk_icon_theme_lookup_icon  (pDefaultIconTheme,
				sIconPath->str,
				64,
				GTK_ICON_LOOKUP_FORCE_SVG);

			if (pIconInfo != NULL)
			{
				g_string_printf (sIconPath, "%s", gtk_icon_info_get_filename (pIconInfo));
				gtk_icon_info_free (pIconInfo);
			}
			else
				g_string_printf (sIconPath, cFileName);
		}
	}

	gchar *cIconPath = sIconPath->str;
	g_string_free (sIconPath, FALSE);
	return cIconPath;
}


void cairo_dock_load_icon_info_from_desktop_file (const gchar *cDesktopFileName, Icon *icon)
{
	gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, cDesktopFileName);
	//g_print ("%s (%s)\n", __func__, cDesktopFilePath);

	GError *erreur = NULL;
	GKeyFile* keyfile = g_key_file_new();
	g_key_file_load_from_file (keyfile, cDesktopFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		return ;
	}

	icon->iType = CAIRO_DOCK_LAUNCHER;
	g_free (icon->acDesktopFileName);
	icon->acDesktopFileName = g_strdup (cDesktopFileName);

	g_free (icon->acFileName);
	icon->acFileName = g_key_file_get_string (keyfile, "Desktop Entry", "Icon", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	if (icon->acFileName != NULL && *icon->acFileName == '\0')
	{
		g_free (icon->acFileName);
		icon->acFileName = NULL;
	}


	g_free (icon->acName);
	icon->acName = g_key_file_get_locale_string (keyfile, "Desktop Entry", "Name", NULL, &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	if (icon->acName != NULL && *icon->acName == '\0')
	{
		g_free (icon->acName);
		icon->acName = NULL;
	}

	g_free (icon->acCommand);
	icon->acCommand = g_key_file_get_string (keyfile, "Desktop Entry", "Exec", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	if (icon->acCommand != NULL && *icon->acCommand == '\0')
	{
		g_free (icon->acCommand);
		icon->acCommand = NULL;
	}

	icon->fOrder = g_key_file_get_double (keyfile, "Desktop Entry", "Order", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}

	icon->cBaseURI = g_key_file_get_string (keyfile, "Desktop Entry", "Base URI", &erreur);
	if (erreur != NULL)
	{
		icon->cBaseURI = NULL;
		g_error_free (erreur);
		erreur = NULL;
	}
	if (icon->cBaseURI != NULL && *icon->cBaseURI == '\0')
	{
		g_free (icon->cBaseURI);
		icon->cBaseURI = NULL;
	}

	icon->iVolumeID = g_key_file_get_boolean (keyfile, "Desktop Entry", "Is mounting point", &erreur);
	if (erreur != NULL)
	{
		icon->iVolumeID = FALSE;
		g_error_free (erreur);
		erreur = NULL;
	}
	if (icon->iVolumeID)  // les infos dans le .desktop ne sont pas a jour.
	{
		g_free (icon->acName);
		icon->acName = NULL;
		g_free (icon->acCommand);
		icon->acCommand = NULL;
		g_free (icon->acFileName);
		icon->acFileName = NULL;

		gboolean bIsDirectory;  // on n'ecrase pas le fait que ce soit un container ou pas, car c'est l'utilisateur qui l'a decide.
		cairo_dock_fm_get_file_info (icon->cBaseURI, &icon->acName, &icon->acCommand, &icon->acFileName, &bIsDirectory, &icon->iVolumeID, &icon->fOrder, g_iFileSortType);
	}

	gboolean bIsContainer = g_key_file_get_boolean (keyfile, "Desktop Entry", "Is container", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		bIsContainer = FALSE;
	}
	if (bIsContainer && icon->acName != NULL)
	{
		gchar *cRendererName = g_key_file_get_string (keyfile, "Desktop Entry", "Renderer", NULL);
		CairoDock *pChildDock = cairo_dock_search_dock_from_name (icon->acName);
		if (pChildDock == NULL)
		{
			cd_message ("le dock fils (%s) n'existe pas, on le cree avec la vue %s\n", icon->acName, cRendererName);
			if (icon->cBaseURI == NULL)
				icon->pSubDock = cairo_dock_create_subdock_from_scratch (NULL, icon->acName);
			else
				cairo_dock_fm_create_dock_from_directory (icon);
		}
		else
		{
			cairo_dock_reference_dock (pChildDock);
			icon->pSubDock = pChildDock;
		}
		if (cRendererName != NULL && icon->pSubDock != NULL)
			cairo_dock_set_renderer (icon->pSubDock, cRendererName);

		g_free (cRendererName);
	}

	g_free (icon->cParentDockName);
	icon->cParentDockName = g_key_file_get_string (keyfile, "Desktop Entry", "Container", &erreur);
	if (erreur != NULL)
	{
		cd_message ("Attention : while trying to load %s : %s\n", cDesktopFileName, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		icon->cParentDockName = NULL;
	}
	if (icon->cParentDockName == NULL || *icon->cParentDockName == '\0')
	{
		g_free (icon->cParentDockName);
		icon->cParentDockName = g_strdup (CAIRO_DOCK_MAIN_DOCK_NAME);
	}

	g_free (cDesktopFilePath);
	g_key_file_free (keyfile);
}



Icon * cairo_dock_create_icon_from_desktop_file (const gchar *cDesktopFileName, cairo_t *pSourceContext)
{
	//g_print ("%s (%s)\n", __func__, cDesktopFileName);

	Icon *icon = g_new0 (Icon, 1);
	cairo_dock_load_icon_info_from_desktop_file (cDesktopFileName, icon);
	g_return_val_if_fail (icon->acDesktopFileName != NULL, NULL);

	CairoDock *pParentDock = cairo_dock_search_dock_from_name (icon->cParentDockName);
	if (pParentDock == NULL)
	{
		cd_message ("le dock parent (%s) n'existe pas, on le cree\n", icon->cParentDockName);
		pParentDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_DOCK, icon->cParentDockName, NULL);
	}

	cairo_dock_fill_one_icon_buffer (icon, pSourceContext, 1 + g_fAmplitude, pParentDock->bHorizontalDock, TRUE);

	cairo_dock_fill_one_text_buffer (icon,
		pSourceContext,
		g_iLabelSize,
		g_cLabelPolice,
		(g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pParentDock->bHorizontalDock));

	return icon;
}
