
#ifndef __CAIRO_DOCK_APPLET_FACTORY__
#define  __CAIRO_DOCK_APPLET_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"


/**
*Cree une surface cairo qui pourra servir de zone de dessin pour une applet.
*@param cIconFileName le nom d'un fichier image a appliquer sur la surface, ou NULL pour creer une surface vide.
*@param pSourceContext un contexte de dessin; n'est pas altere.
*@param fMaxScale le zoom max auquel sera soumis la surface.
*@param fWidth largeur de la surface obtenue.
*@param fHeight hauteur de la surface obtenue.
*@Returns la surface nouvellement generee.
*/
cairo_surface_t *cairo_dock_create_applet_surface (gchar *cImageFilePath, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);


/**
*Cree une icone destinee a une applet.
*@param pDock le dock ou sera inseree ulterieurement cette icone.
*@param iWidth la largeur desiree de l'icone.
*@param iHeight la hauteur desiree de l'icone.
*@param cName le nom de l'icone, tel qu'il apparaitra en etiquette de l'icone.
*@param cIconFileName le nom d'un fichier image a afficher dans l'icone, ou NULL si l'on souhaitera dessiner soi-meme dans l'icone.
*@Returns l'icone nouvellement cree. Elle n'est _pas_ inseree dans le dock, c'est le gestionnaire de module qui se charge d'inserer les icones renvoyees par les modules.
*/
Icon *cairo_dock_create_icon_for_applet (CairoDock *pDock, int iWidth, int iHeight, gchar *cName, gchar *cIconName);



/**
*Ouvre et lit certaines cles  pre-definis d'un fichier de conf. Les cles sont : "width", "height", et "name", toutes dans le groupes "ICON".
*@param cConfFilePath le chemin du fichier de conf.
*@param iWidth la valeur lue dans la cle "width".
*@param iHeight la valeur lue dans la cle "height".
*@param cName la valeur lue dans la cle "name".
*@param bFlushConfFileNeeded est positionne a TRUE si une des cles est manquante et a ete rajoutee par defaut.
*@Returns le fichier de cles cree a partir du fichier de conf. A liberer avec #g_keyfile_free apres utilisation.
*/
GKeyFile *cairo_dock_read_header_applet_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gboolean *bFlushConfFileNeeded);


/**
*Liste les themes disponibles. Un theme est un repertoire, et tous doivent etre places dans un meme repertoire.
*@param cThemesDir le repertoire contenant les themes.
*@param hProvidedTable une table de hashage (string , string) qui sera remplie, ou NULL pour que la fonction vous la cree.
*@param erreur : erreur positionnee au cas ou le repertoire serait illisible.
*@Returns la table de hashage contenant les doublets (nom_du_theme , chemin_du_theme). Si une table avait ete fournie en entree, c'est elle qui est retournee, sinon c'est une nouvelle table, a detruire avec 'g_hash_table_destroy' apres utilisation (tous les elements seront liberes).
*/
GHashTable *cairo_dock_list_themes (gchar *cThemesDir, GHashTable *hProvidedTable, GError **erreur);


/**
*Verifie que le fichier de conf d'un plug-in est bien present dans le repertoire utilisateur du plug-in, sinon le copie a partir du fichier de conf fournit lors de l'installation. Cree au besoin le repertoire utilisateur du plug-in.
*@param cUserDataDirName le nom du repertoire utilisateur du plug-in.
*@param cShareDataDir le chemin du repertoire d'installation du plug-in.
*@param cConfFileName : le nom du fichier de conf fournit a l'installation.
*@Returns Le chemin du fichier de conf en espace utilisateur, ou NULL si le fichier n'a pu etre ni trouve, ni cree.
*/
gchar *cairo_dock_check_conf_file_exists (gchar *cUserDataDirName, gchar *cShareDataDir, gchar *cConfFileName);


/**
*Applique une surface sur un contexte, en effacant tout au prealable.
*@param pIconContext le contexte du dessin; est modifie par la fonction.
*@param pSurface la surface a appliquer
*/
void cairo_dock_set_icon_surface (cairo_t *pIconContext, cairo_surface_t *pSurface);
/**
*Cree les surfaces de reflection d'une icone.
*@param pIconContext le contexte de dessin lie a la surface de l'icone; est modifie par la fonction.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_add_reflection_to_icon (cairo_t *pIconContext, Icon *pIcon, CairoDock *pDock);
/**
*Applique une surface sur le contexte d'une icone, en effacant tout au prealable et en creant les reflets correspondant.
*@param pIconContext le contexte de dessin lie a la surface de l'icone; est modifie par la fonction.
*@param pSurface la surface a appliquer a l'icone.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_set_icon_surface_with_reflect (cairo_t *pIconContext, cairo_surface_t *pSurface, Icon *pIcon, CairoDock *pDock);

/**
*Modifie l'etiquette d'une icone.
*@param pIconContext un contexte de dessin; n'est pas altere par la fonction.
*@param cIconName la nouvelle etiquette de l'icone.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_set_icon_name (cairo_t *pSourceContext, const gchar *cIconName, Icon *pIcon, CairoDock *pDock);

/**
*Ecris une info-rapide sur l'icone. C'est un petit texte (quelques caracteres) qui vient se superposer sur l'icone, avec un fond fonce.
*@param pIconContext un contexte de dessin; n'est pas altere par la fonction.
*@param cQuickInfo le texte de l'info-rapide.
*@param pIcon l'icone.
*/
void cairo_dock_set_quick_info (cairo_t *pSourceContext, const gchar *cExtraInfo, Icon *pIcon);
/**
*Efface l'info-rapide d'une icone.
*@param pIcon l'icone.
*/
#define cairo_dock_remove_quick_info(pIcon) cairo_dock_set_quick_info (NULL, NULL, pIcon)


/**
*Prepare l'animation d'une icone, et la lance immediatement.
*@param pIcon  l'icone a animer.
*@param pDock le dock contenant l'icone.
*@param iAnimationType le type d'animation voulu, ou -1 pour utiliser l'animtion correspondante au type de l'icone.
*@param iNbRounds le nombre de fois ou l'animation sera jouee, ou -1 pour utiliser la valeur correspondante au type de l'icone.
*/
void cairo_dock_animate_icon (Icon *pIcon, CairoDock *pDock, CairoDockAnimationType iAnimationType, int iNbRounds);


#define CD_CONFIG_APPLET \
void read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar *cName)

#define CD_CONFIG_BEGIN \
	GError *erreur = NULL; \
	gboolean bFlushConfFileNeeded = FALSE; \
	GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, &cName, &bFlushConfFileNeeded); \
	g_return_if_fail (pKeyFile != NULL);
#define CD_CONFIG_END \
	if (bFlushConfFileNeeded) \
		cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath); \
	g_key_file_free (pKeyFile);

#define CD_CONFIG_GET_BOOLEAN(cGroupName, cKeyName) cairo_dock_get_boolean_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, TRUE)
#define CD_CONFIG_GET_INTEGER(cGroupName, cKeyName) cairo_dock_get_integer_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, 0)
#define CD_CONFIG_GET_DOUBLE(cGroupName, cKeyName) cairo_dock_get_double_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, 0.)
#define CD_CONFIG_GET_STRING(cGroupName, cKeyName) cairo_dock_get_string_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, NULL)
#define CD_CONFIG_GET_ANIMATION(cGroupName, cKeyName) cairo_dock_get_animation_type_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, NULL);


#define CD_APPLET_DEFINITION(cName, iMajorVersion, iMinorVersion, iMicroVersion) \
CairoDockVisitCard *pre_init (void)\
{\
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);\
	pVisitCard->cModuleName = g_strdup (cName);\
	pVisitCard->cReadmeFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_README_FILE);\
	pVisitCard->iMajorVersionNeeded = iMajorVersion;\
	pVisitCard->iMinorVersionNeeded = iMinorVersion;\
	pVisitCard->iMicroVersionNeeded = iMicroVersion;\
	return pVisitCard;\
}

#define CD_INIT_APPLET Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
#define CD_STOP_APPLET void stop (void)
#define CD_PRE_INIT_APPLET gchar *pre_init (void)

#define CD_APPLET_INIT_BEGIN \
	myDock = pDock; \
	*cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, APPLET_CONF_FILE); \
	int iOriginalWidth = 48, iOriginalHeight = 48; \
	gchar *cAppletName = NULL; \
	read_conf_file (*cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cAppletName); \
	myIcon = cairo_dock_create_icon_for_applet (pDock, iOriginalWidth, iOriginalHeight, conf_defaultTitle, NULL); \
	myDrawContext = cairo_create (myIcon->pIconBuffer);

#define CD_APPLET_INIT_END \
	g_free (cAppletName); \
	return myIcon;


#define CD_CLICK_ON_APPLET \
gboolean action_on_click (gpointer *data)

#define CD_CLICK_ON_APPLET_BEGIN \
	if (data[0] == myIcon) \
	{

#define CD_CLICK_ON_APPLET_END \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;


#endif
