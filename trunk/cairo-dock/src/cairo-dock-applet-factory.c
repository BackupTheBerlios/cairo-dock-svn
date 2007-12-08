/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-load.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-config.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-applet-factory.h"

extern gchar *g_cCurrentThemePath;

extern double g_fAmplitude;
extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern gboolean g_bTextAlwaysHorizontal;

extern double g_fAlbedo;

extern int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
extern int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];

extern gboolean g_bUseGlitz;


/**
*Cree une surface cairo qui pourra servir de zone de dessin pour une applet.
*@param cIconFileName le nom d'un fichier image a appliquer sur la surface, ou NULL pour creer une surface vide.
*@param pSourceContext un contexte de dessin; n'est pas altere.
*@param fMaxScale le zoom max auquel sera soumis la surface.
*@param fWidth largeur de la surface obtenue.
*@param fHeight hauteur de la surface obtenue.
*@Returns la surface nouvellement generee.
*/
cairo_surface_t *cairo_dock_create_applet_surface (gchar *cIconFileName, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight)
{
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
	double fIconWidthSaturationFactor, fIconHeightSaturationFactor;
	cairo_dock_calculate_contrainted_icon_size (fWidth,
		fHeight,
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET],
		g_tMinIconAuthorizedSize[CAIRO_DOCK_APPLET],
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET],
		g_tMaxIconAuthorizedSize[CAIRO_DOCK_APPLET],
		&fIconWidthSaturationFactor, &fIconHeightSaturationFactor);
	
	cairo_surface_t *pNewSurface;
	if (cIconFileName == NULL)
		pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
			CAIRO_CONTENT_COLOR_ALPHA,
			ceil (*fWidth * fMaxScale),
			ceil (*fHeight * fMaxScale));
	else
	{
		gchar *cIconPath = cairo_dock_search_icon_s_path (cIconFileName);
		pNewSurface = cairo_dock_create_surface_from_image (cIconPath,
			pSourceContext,
			fMaxScale,
			*fWidth,
			*fHeight,
			*fWidth,
			*fHeight,
			fWidth,
			fHeight,
			0,
			1,
			FALSE);
		g_free (cIconPath);
	}
	return pNewSurface;
}


/**
*Cree une icone destinee a une applet.
*@param pDock le dock ou sera inseree ulterieurement cette icone.
*@param iWidth la largeur desiree de l'icone.
*@param iHeight la hauteur desiree de l'icone.
*@param cName le nom de l'icone, tel qu'il apparaitra en etiquette de l'icone.
*@param cIconFileName le nom d'un fichier image a afficher dans l'icone, ou NULL si l'on souhaitera dessiner soi-meme dans l'icone.
*@Returns l'icone nouvellement cree. Elle n'est _pas_ inseree dans le dock, c'est le gestionnaire de module qui se charge d'inserer les icones renvoyees par les modules.
*/
Icon *cairo_dock_create_icon_for_applet (CairoDock *pDock, int iWidth, int iHeight, gchar *cName, gchar *cIconFileName)
{
	Icon *icon = g_new0 (Icon, 1);
	icon->iType = CAIRO_DOCK_APPLET;
	
	icon->acName = g_strdup (cName);
	icon->acFileName = g_strdup (cIconFileName);  // NULL si cIconFileName = NULL.
	
	icon->fWidth =iWidth;
	icon->fHeight =iHeight;
	icon->fWidthFactor = 1.;
	cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock);
	g_return_val_if_fail (cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, icon);
	
	cairo_dock_fill_one_icon_buffer (icon, pSourceContext, 1 + g_fAmplitude, pDock->bHorizontalDock);
	
	cairo_dock_fill_one_text_buffer (icon, pSourceContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
	
	cairo_destroy (pSourceContext);
	return icon;
}


/**
*Ouvre et lit certaines cles  pre-definis d'un fichier de conf. Les cles sont : "width", "height", et "name", toutes dans le groupes "ICON".
*@param cConfFilePath le chemin du fichier de conf.
*@param iWidth la valeur lue dans la cle "width".
*@param iHeight la valeur lue dans la cle "height".
*@param cName la valeur lue dans la cle "name".
*@param bFlushConfFileNeeded est positionne a TRUE si une des cles est manquante et a ete rajoutee par defaut.
*@Returns le fichier de cles cree a partir du fichier de conf. A liberer avec #g_keyfile_free apres utilisation.
*/
GKeyFile *cairo_dock_read_header_applet_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gboolean *bFlushConfFileNeeded)
{
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	*iWidth = cairo_dock_get_integer_key_value (pKeyFile, "ICON", "width", bFlushConfFileNeeded, 48);
	
	*iHeight = cairo_dock_get_integer_key_value (pKeyFile, "ICON", "height", bFlushConfFileNeeded, 48);
	
	*cName = cairo_dock_get_string_key_value (pKeyFile, "ICON", "name", bFlushConfFileNeeded, NULL);
	
	return pKeyFile;
}


/**
*Liste les themes disponibles. Un theme est un repertoire, et tous doivent etre places dans un meme repertoire.
*@param cThemesDir le repertoire contenant les themes.
*@param hProvidedTable une table de hashage (string , string) qui sera remplie, ou NULL pour que la fonction vous la cree.
*@param erreur : erreur positionnee au cas ou le repertoire serait illisible.
*@Returns la table de hashage contenant les doublets (nom_du_theme , chemin_du_theme). Si une table avait ete fournie en entree, c'est elle qui est retournee, sinon c'est une nouvelle table, a detruire avec 'g_hash_table_destroy' apres utilisation (tous les elements seront liberes).
*/
GHashTable *cairo_dock_list_themes (gchar *cThemesDir, GHashTable *hProvidedTable, GError **erreur)
{
	GError *tmp_erreur = NULL;
	GDir *dir = g_dir_open (cThemesDir, 0, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		return NULL;
	}
	
	GHashTable *pThemeTable = (hProvidedTable != NULL ? hProvidedTable : g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free));
	
	const gchar* cThemeName;
	gchar *cThemePath;
	do
	{
		cThemeName = g_dir_read_name (dir);
		if (cThemeName == NULL)
			break ;
		
		cThemePath = g_strdup_printf ("%s/%s", cThemesDir, cThemeName);
		
		if (g_file_test (cThemePath, G_FILE_TEST_IS_DIR))
			g_hash_table_insert (pThemeTable, g_strdup (cThemeName), cThemePath);
	}
	while (1);
	g_dir_close (dir);
	
	return pThemeTable;
}



/**
*Verifie que le fichier de conf d'un plug-in est bien present dans le repertoire utilisateur du plug-in, sinon le copie a partir du fichier de conf fournit lors de l'installation. Cree au besoin le repertoire utilisateur du plug-in.
*@param cUserDataDirName le nom du repertoire utilisateur du plug-in.
*@param cShareDataDir le chemin du repertoire d'installation du plug-in.
*@param cConfFileName : le nom du fichier de conf fournit a l'installation.
*@Returns Le chemin du fichier de conf en espace utilisateur, ou NULL si le fichier n'a pu etre ni trouve, ni cree.
*/
gchar *cairo_dock_check_conf_file_exists (gchar *cUserDataDirName, gchar *cShareDataDir, gchar *cConfFileName)
{
	gchar *cUserDataDirPath = g_strdup_printf ("%s/plug-ins/%s", g_cCurrentThemePath, cUserDataDirName);
	if (! g_file_test (cUserDataDirPath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		g_print ("directory %s doesn't exist, it will be added.\n", cUserDataDirPath);
		
		gchar *command = g_strdup_printf ("mkdir -p %s", cUserDataDirPath);
		system (command);
		g_free (command);
	}
	
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", cUserDataDirPath, cConfFileName);
	if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))
	{
		gchar *command = g_strdup_printf ("cp %s/%s %s", cShareDataDir, cConfFileName, cConfFilePath);
		system (command);
		g_free (command);
	}
	
	if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))  // la copie ne s'est pas bien passee.
	{
		g_print ("Attention : couldn't copy %s/%s in %s; check permissions and file's existence\n", cShareDataDir, cConfFileName, cUserDataDirPath);
		g_free (cUserDataDirPath);
		g_free (cConfFilePath);
		return NULL;
	}
	
	g_free (cUserDataDirPath);
	return cConfFilePath;
}


/**
*Applique une surface sur un contexte, en effacant tout au prealable.
*@param pIconContext le contexte du dessin; est modifie par la fonction.
*@param pSurface la surface a appliquer
*/
void cairo_dock_set_icon_surface (cairo_t *pIconContext, cairo_surface_t *pSurface)  // fonction proposee par Necropotame.
{
	g_return_if_fail (cairo_status (pIconContext) == CAIRO_STATUS_SUCCESS);
	
	//\________________ On efface l'ancienne image.
	cairo_set_source_rgba (pIconContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pIconContext);
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
	
	//\________________ On applique la nouvelle image.
	if (pSurface != NULL)
	{
		cairo_set_source_surface (
			pIconContext,
			pSurface,
			0.,
			0.);
		cairo_paint (pIconContext);
	}
}

/**
*Cree les surfaces de reflection d'une icone.
*@param pIconContext le contexte de dessin lie a la surface de l'icone; est modifie par la fonction.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_add_reflection_to_icon (cairo_t *pIconContext, Icon *pIcon, CairoDock *pDock)
{
	if (pIcon->pReflectionBuffer != NULL)
	{
		cairo_surface_destroy (pIcon->pReflectionBuffer);
		pIcon->pReflectionBuffer = NULL;
	}
	pIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (pIcon->pIconBuffer,
		pIconContext,
		(pDock->bHorizontalDock ? pIcon->fWidth : pIcon->fHeight) * (1 + g_fAmplitude),
		(pDock->bHorizontalDock ? pIcon->fHeight : pIcon->fWidth) * (1 + g_fAmplitude),
		pDock->bHorizontalDock);
	
	if (pIcon->pFullIconBuffer != NULL)
	{
		cairo_surface_destroy (pIcon->pFullIconBuffer);
		pIcon->pFullIconBuffer = NULL;
	}
	pIcon->pFullIconBuffer = cairo_dock_create_icon_surface_with_reflection (pIcon->pIconBuffer,
		pIcon->pReflectionBuffer,
		pIconContext,
		(pDock->bHorizontalDock ? pIcon->fWidth : pIcon->fHeight) * (1 + g_fAmplitude),
		(pDock->bHorizontalDock ? pIcon->fHeight : pIcon->fWidth) * (1 + g_fAmplitude),
		pDock->bHorizontalDock);
}

/**
*Applique une surface sur le contexte d'une icone, en effacant tout au prealable et en creant les reflets correspondant.
*@param pIconContext le contexte de dessin lie a la surface de l'icone; est modifie par la fonction.
*@param pSurface la surface a appliquer a l'icone.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_set_icon_surface_with_reflect (cairo_t *pIconContext, cairo_surface_t *pSurface, Icon *pIcon, CairoDock *pDock)
{
	cairo_dock_set_icon_surface (pIconContext, pSurface);
	
	cairo_dock_add_reflection_to_icon (pIconContext, pIcon, pDock);
}

/**
*Modifie l'etiquette d'une icone.
*@param pIconContext un contexte de dessin; n'est pas altere par la fonction.
*@param cIconName la nouvelle etiquette de l'icone.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_set_icon_name (cairo_t *pSourceContext, const gchar *cIconName, Icon *pIcon, CairoDock *pDock)  // fonction proposee par Necropotame.
{
	g_return_if_fail (pIcon != NULL);  // le contexte sera verifie plus loin.
	
	g_free (pIcon->acName);
	pIcon->acName = g_strdup (cIconName);
	
	cairo_dock_fill_one_text_buffer(
		pIcon,
		pSourceContext,
		g_iLabelSize,
		g_cLabelPolice,
		(g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : pDock->bHorizontalDock));
}

/**
*Ecris une info-rapide sur l'icone. C'est un petit texte (quelques caracteres) qui vient se superposer sur l'icone, avec un fond fonce.
*@param pIconContext un contexte de dessin; n'est pas altere par la fonction.
*@param cQuickInfo le texte de l'info-rapide.
*@param pIcon l'icone.
*/
void cairo_dock_set_quick_info (cairo_t *pSourceContext, const gchar *cQuickInfo, Icon *pIcon)
{
	g_return_if_fail (pIcon != NULL);  // le contexte sera verifie plus loin.
	
	g_free (pIcon->cQuickInfo);
	pIcon->cQuickInfo = g_strdup (cQuickInfo);
	
	cairo_dock_fill_one_extra_info_buffer (pIcon,
		pSourceContext,
		12,
		g_cLabelPolice,
		PANGO_WEIGHT_HEAVY,
		.4);
}

/**
*Prepare l'animation d'une icone, et la lance immediatement.
*@param pIcon  l'icone a animer.
*@param pDock le dock contenant l'icone.
*@param iAnimationType le type d'animation voulu, ou -1 pour utiliser l'animtion correspondante au type de l'icone.
*@param iNbRounds le nombre de fois ou l'animation sera jouee, ou -1 pour utiliser la valeur correspondante au type de l'icone.
*/
void cairo_dock_animate_icon (Icon *pIcon, CairoDock *pDock, CairoDockAnimationType iAnimationType, int iNbRounds)
{
	cairo_dock_arm_animation (pIcon, iAnimationType, iNbRounds);
	cairo_dock_start_animation (pIcon, pDock);
}
