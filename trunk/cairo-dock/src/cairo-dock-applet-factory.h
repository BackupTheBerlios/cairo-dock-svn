
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
*@return la surface nouvellement generee.
*/
cairo_surface_t *cairo_dock_create_applet_surface (gchar *cIconFileName, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);


/**
*Cree une icone destinee a une applet.
*@param pDock le dock ou sera inseree ulterieurement cette icone.
*@param iWidth la largeur desiree de l'icone.
*@param iHeight la hauteur desiree de l'icone.
*@param cName le nom de l'icone, tel qu'il apparaitra en etiquette de l'icone.
*@param cIconFileName le nom d'un fichier image a afficher dans l'icone, ou NULL si l'on souhaitera dessiner soi-meme dans l'icone.
*@return l'icone nouvellement cree. Elle n'est _pas_ inseree dans le dock, c'est le gestionnaire de module qui se charge d'inserer les icones renvoyees par les modules.
*/
Icon *cairo_dock_create_icon_for_applet (CairoDock *pDock, int iWidth, int iHeight, gchar *cName, gchar *cIconFileName);



/**
*Ouvre et lit certaines cles  pre-definis d'un fichier de conf. Les cles sont : "width", "height", et "name", toutes dans le groupes "ICON".
*@param cConfFilePath le chemin du fichier de conf.
*@param iWidth la valeur lue dans la cle "width".
*@param iHeight la valeur lue dans la cle "height".
*@param cName la valeur lue dans la cle "name".
*@param bFlushConfFileNeeded est positionne a TRUE si une des cles est manquante et a ete rajoutee par defaut.
*@return le fichier de cles cree a partir du fichier de conf. A liberer avec g_keyfile_free() apres utilisation.
*/
GKeyFile *cairo_dock_read_header_applet_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gboolean *bFlushConfFileNeeded);


/**
*Liste les themes disponibles. Un theme est un repertoire, et tous doivent etre places dans un meme repertoire.
*@param cThemesDir le repertoire contenant les themes.
*@param hProvidedTable une table de hashage (string , string) qui sera remplie, ou NULL pour que la fonction vous la cree.
*@param erreur : erreur positionnee au cas ou le repertoire serait illisible.
*@return la table de hashage contenant les doublets (nom_du_theme , chemin_du_theme). Si une table avait ete fournie en entree, c'est elle qui est retournee, sinon c'est une nouvelle table, a detruire avec 'g_hash_table_destroy' apres utilisation (tous les elements seront liberes).
*/
GHashTable *cairo_dock_list_themes (gchar *cThemesDir, GHashTable *hProvidedTable, GError **erreur);


/**
*Verifie que le fichier de conf d'un plug-in est bien present dans le repertoire utilisateur du plug-in, sinon le copie a partir du fichier de conf fournit lors de l'installation. Cree au besoin le repertoire utilisateur du plug-in.
*@param cUserDataDirName le nom du repertoire utilisateur du plug-in.
*@param cShareDataDir le chemin du repertoire d'installation du plug-in.
*@param cConfFileName : le nom du fichier de conf fournit a l'installation.
*@return Le chemin du fichier de conf en espace utilisateur, ou NULL si le fichier n'a pu etre ni trouve, ni cree.
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
*Applique une image sur le contexte d'une icone, en effacant tout au prealable et en creant les reflets correspondant.
*@param pIconContext le contexte de dessin lie a la surface de l'icone; est modifie par la fonction.
*@param cImagePath chemin de l'image a appliquer a l'icone.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_set_image_on_icon (cairo_t *pIconContext, gchar *cImagePath, Icon *pIcon, CairoDock *pDock);

/**
*Modifie l'etiquette d'une icone.
*@param pSourceContext un contexte de dessin; n'est pas altere par la fonction.
*@param cIconName la nouvelle etiquette de l'icone.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*/
void cairo_dock_set_icon_name (cairo_t *pSourceContext, const gchar *cIconName, Icon *pIcon, CairoDock *pDock);

/**
*Ecris une info-rapide sur l'icone. C'est un petit texte (quelques caracteres) qui vient se superposer sur l'icone, avec un fond fonce.
*@param pSourceContext un contexte de dessin; n'est pas altere par la fonction.
*@param cQuickInfo le texte de l'info-rapide.
*@param pIcon l'icone.
*/
void cairo_dock_set_quick_info (cairo_t *pSourceContext, const gchar *cQuickInfo, Icon *pIcon);
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


//\___________________________ INIT
#define CD_APPLET_H \
CairoDockVisitCard *pre_init (void);\
Icon *init (CairoDock *pDock, gchar **cAppletConfFilePath, GError **erreur);\
void stop (void);

//\___________________ pre_init.
#define CD_APPLET_PRE_INIT_BEGIN(cName, iMajorVersion, iMinorVersion, iMicroVersion) \
Icon *myIcon = NULL;\
CairoDock *myDock = NULL;\
cairo_t *myDrawContext = NULL;\
CairoDockVisitCard *pre_init (void)\
{\
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);\
	pVisitCard->cModuleName = g_strdup (cName);\
	pVisitCard->cReadmeFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_README_FILE);\
	pVisitCard->iMajorVersionNeeded = iMajorVersion;\
	pVisitCard->iMinorVersionNeeded = iMinorVersion;\
	pVisitCard->iMicroVersionNeeded = iMicroVersion;\
	pVisitCard->cPreviewFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_PREVIEW_FILE);\
	pVisitCard->cGettextDomain = g_strdup (MY_APPLET_GETTEXT_DOMAIN);

#define CD_APPLET_PRE_INIT_END \
	return pVisitCard;\
}

#define CD_APPLET_DEFINITION(cName, iMajorVersion, iMinorVersion, iMicroVersion) \
CD_APPLET_PRE_INIT_BEGIN (cName, iMajorVersion, iMinorVersion, iMicroVersion) \
CD_APPLET_PRE_INIT_END

//\___________________ init.
#define CD_APPLET_INIT_BEGIN(erreur) \
Icon *init (CairoDock *pDock, gchar **cAppletConfFilePath, GError **erreur) \
{ \
	myDock = pDock; \
	gchar *cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, MY_APPLET_CONF_FILE); \
	int iDesiredWidth = 48, iDesiredHeight = 48; \
	gchar *cAppletName = NULL, *cIconName = NULL; \
	read_conf_file (cConfFilePath, &iDesiredWidth, &iDesiredHeight, &cAppletName, &cIconName); \
	myIcon = cairo_dock_create_icon_for_applet (pDock, iDesiredWidth, iDesiredHeight, cAppletName, cIconName); \
	g_return_val_if_fail (myIcon != NULL, NULL); \
	myDrawContext = cairo_create (myIcon->pIconBuffer); \
	g_return_val_if_fail (cairo_status (myDrawContext) == CAIRO_STATUS_SUCCESS, NULL);

#define CD_APPLET_INIT_END \
	*cAppletConfFilePath = cConfFilePath; \
	g_free (cAppletName); \
	g_free (cIconName); \
	return myIcon; \
}

//\___________________ stop.
#define CD_APPLET_STOP_BEGIN \
void stop (void) \
{

#define CD_APPLET_STOP_END \
	myDock = NULL; \
	myIcon = NULL; \
	cairo_destroy (myDrawContext); \
	myDrawContext = NULL; \
}

#define CD_APPLET_MY_CONF_FILE cConfFilePath


//\___________________________ CONFIG
#define CD_APPLET_CONFIG_BEGIN(cDefaultAppletName, cDefaultIconName) \
void read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cAppletName, gchar **cIconName) \
{ \
	GError *erreur = NULL; \
	gboolean bFlushConfFileNeeded = FALSE; \
	GKeyFile *pKeyFile = g_key_file_new (); \
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur); \
	if (erreur != NULL) \
	{ \
		g_print ("Attention : %s\n", erreur->message); \
		g_error_free (erreur); \
		return ; \
	} \
	*iWidth = cairo_dock_get_integer_key_value (pKeyFile, "ICON", "width", &bFlushConfFileNeeded, 48, NULL, NULL); \
	*iHeight = cairo_dock_get_integer_key_value (pKeyFile, "ICON", "height", &bFlushConfFileNeeded, 48, NULL, NULL); \
	if (cDefaultAppletName != NULL) \
		*cAppletName = cairo_dock_get_string_key_value (pKeyFile, "ICON", "name", &bFlushConfFileNeeded, cDefaultAppletName, NULL, NULL); \
	if (cDefaultIconName != NULL) \
		*cIconName = cairo_dock_get_string_key_value (pKeyFile, "ICON", "icon", &bFlushConfFileNeeded, cDefaultIconName, NULL, NULL); \

#define CD_APPLET_CONFIG_END \
	if (! bFlushConfFileNeeded) \
		bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile, MY_APPLET_VERSION); \
	if (bFlushConfFileNeeded) \
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, MY_APPLET_SHARE_DATA_DIR);\
	g_key_file_free (pKeyFile); \
}

#define CD_APPLET_CONFIG_H \
void read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cAppletName, gchar **cIconName);


#define CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT(cGroupName, cKeyName, bDefaultValue) cairo_dock_get_boolean_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, bDefaultValue, NULL, NULL)
#define CD_CONFIG_GET_BOOLEAN(cGroupName, cKeyName) CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT (cGroupName, cKeyName, TRUE)

#define CD_CONFIG_GET_INTEGER_WITH_DEFAULT(cGroupName, cKeyName, iDefaultValue) cairo_dock_get_integer_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, iDefaultValue, NULL, NULL)
#define CD_CONFIG_GET_INTEGER(cGroupName, cKeyName)CD_CONFIG_GET_INTEGER_WITH_DEFAULT (cGroupName, cKeyName, 0)

#define CD_CONFIG_GET_DOUBLE_WITH_DEFAULT(cGroupName, cKeyName, fDefaultValue) cairo_dock_get_double_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, 0., NULL, NULL)
#define CD_CONFIG_GET_DOUBLE(cGroupName, cKeyName) CD_CONFIG_GET_DOUBLE_WITH_DEFAULT (cGroupName, cKeyName, 0.)

#define CD_CONFIG_GET_STRING_WITH_DEFAULT(cGroupName, cKeyName, cDefaultValue) cairo_dock_get_string_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, cDefaultValue, NULL, NULL)
#define CD_CONFIG_GET_STRING(cGroupName, cKeyName) CD_CONFIG_GET_STRING_WITH_DEFAULT (cGroupName, cKeyName, NULL)

#define CD_CONFIG_GET_STRING_LIST_WITH_DEFAULT(cGroupName, cKeyName, length, cDefaultValues) cairo_dock_get_string_list_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, length, cDefaultValues, NULL, NULL)
#define CD_CONFIG_GET_STRING_LIST(cGroupName, cKeyName, length) CD_CONFIG_GET_STRING_LIST_WITH_DEFAULT(cGroupName, cKeyName, length, NULL)

#define CD_CONFIG_GET_ANIMATION_WITH_DEFAULT(cGroupName, cKeyName, iDefaultAnimation) cairo_dock_get_animation_type_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, iDefaultAnimation, NULL, NULL);
#define CD_CONFIG_GET_ANIMATION(cGroupName, cKeyName) CD_CONFIG_GET_ANIMATION_WITH_DEFAULT(cGroupName, cKeyName, CAIRO_DOCK_BOUNCE)

#define CD_CONFIG_GET_COLOR_WITH_DEFAULT(cGroupName, cKeyName, pColorBuffer, pDefaultColor) cairo_dock_get_double_list_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, pColorBuffer, 4, pDefaultColor, NULL, NULL);
#define CD_CONFIG_GET_COLOR(cGroupName, cKeyName, pColorBuffer) CD_CONFIG_GET_COLOR_WITH_DEFAULT(cGroupName, cKeyName, pColorBuffer, NULL)

//\___________________________ NOTIFICATIONS
//\___________________ fonction about.
#define CD_APPLET_ABOUT(cMessage) \
void about (GtkMenuItem *menu_item, gpointer *data) \
{ \
	cairo_dock_show_temporary_dialog (cMessage, myIcon, myDock, 0); \
}

#define CD_APPLET_ABOUT_H \
void about (GtkMenuItem *menu_item, gpointer *data);

//\___________________ notification clique gauche.
#define CD_APPLET_ON_CLICK action_on_click
#define CD_APPLET_REGISTER_FOR_CLICK_EVENT cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK, CAIRO_DOCK_RUN_FIRST);
#define CD_APPLET_UNREGISTER_FOR_CLICK_EVENT cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK);

#define CD_APPLET_ON_CLICK_BEGIN \
gboolean CD_APPLET_ON_CLICK (gpointer *data) \
{ \
	if (data[0] == myIcon) \
	{

#define CD_APPLET_ON_CLICK_END \
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION; \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION; \
}

#define CD_APPLET_ON_CLICK_H \
gboolean CD_APPLET_ON_CLICK (gpointer *data);

//\___________________ notification construction menu.
#define CD_APPLET_ON_BUILD_MENU applet_on_build_menu
#define CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU, CAIRO_DOCK_RUN_FIRST);
#define CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU);

#define CD_APPLET_ON_BUILD_MENU_BEGIN \
gboolean CD_APPLET_ON_BUILD_MENU (gpointer *data) \
{ \
	if (data[0] == myIcon) \
	{ \
		GtkWidget *pAppletMenu = data[2]; \
		GtkWidget *pMenuItem;

#define CD_APPLET_ON_BUILD_MENU_END \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION; \
}

#define CD_APPLET_MY_MENU pAppletMenu

#define CD_APPLET_ADD_SUB_MENU(cLabel, pSubMenu, pMenu) \
	GtkWidget *pSubMenu = gtk_menu_new (); \
	pMenuItem = gtk_menu_item_new_with_label (cLabel); \
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem); \
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);

#define CD_APPLET_ADD_IN_MENU_WITH_DATA(cLabel, pFunction, pMenu, pData) \
	pMenuItem = gtk_menu_item_new_with_label (cLabel); \
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem); \
	g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (pFunction), pData);

#define CD_APPLET_ADD_IN_MENU(cLabel, pFunction, pMenu) CD_APPLET_ADD_IN_MENU_WITH_DATA(cLabel, pFunction, pMenu, NULL)

#define CD_APPLET_ADD_ABOUT_IN_MENU(pMenu) CD_APPLET_ADD_IN_MENU (_("About"), about, pMenu)

#define CD_APPLET_LAST_ITEM_IN_MENU pMenuItem

#define CD_APPLET_ON_BUILD_MENU_H \
gboolean CD_APPLET_ON_BUILD_MENU (gpointer *data);

//\___________________ notification clique milieu.
#define CD_APPLET_ON_MIDDLE_CLICK action_on_middle_click
#define CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK, CAIRO_DOCK_RUN_FIRST);
#define CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT cairo_dock_remove_notification_func (CAIRO_DOCK_MIDDLE_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK);

#define CD_APPLET_ON_MIDDLE_CLICK_BEGIN \
gboolean CD_APPLET_ON_MIDDLE_CLICK (gpointer *data) \
{ \
	if (data[0] == myIcon) \
	{

#define CD_APPLET_ON_MIDDLE_CLICK_END \
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION; \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION; \
}

#define CD_APPLET_ON_MIDDLE_CLICK_H \
gboolean CD_APPLET_ON_MIDDLE_CLICK (gpointer *data);

//\___________________________ DESSIN
#define CD_APPLET_SET_SURFACE_ON_MY_ICON(pSurface) \
	cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, myDock); \
	cairo_dock_redraw_my_icon (myIcon, myDock);

#define CD_APPLET_SET_IMAGE_ON_MY_ICON(cImagePath) \
	cairo_dock_set_image_on_icon (myDrawContext, cImagePath, myIcon, myDock);

#define CD_APPLET_SET_NAME_FOR_MY_ICON(cIconName) \
	cairo_dock_set_icon_name (myDrawContext, cIconName, myIcon, myDock);

#define CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(cQuickInfo) \
	cairo_dock_set_quick_info (myDrawContext, cQuickInfo, myIcon);\
	cairo_dock_redraw_my_icon (myIcon, myDock);

#define CD_APPLET_ANIMATE_MY_ICON(iAnimationType, iAnimationLength) \
	cairo_dock_animate_icon (myIcon, myDock, iAnimationType, iAnimationLength);

#define CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET(cImagePath) cairo_dock_create_surface_for_icon (cImagePath, myDrawContext, myIcon->fWidth * (1 + g_fAmplitude), myIcon->fHeight* (1 + g_fAmplitude));

//\___________________________ INCLUDE
#define CD_APPLET_INCLUDE_MY_VARS \
extern Icon *myIcon; \
extern cairo_t *myDrawContext; \
extern CairoDock *myDock;

//\___________________________ INTERNATIONNALISATION
/// Macro pour gettext, similaire à _() et _N(), mais avec le nom de domaine en parametre.
#define _D(message) dgettext (MY_APPLET_GETTEXT_DOMAIN, message)

#endif
