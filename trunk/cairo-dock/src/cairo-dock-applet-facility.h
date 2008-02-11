
#ifndef __CAIRO_DOCK_APPLET_FACILITY__
#define  __CAIRO_DOCK_APPLET_FACILITY__

#include "cairo-dock-struct.h"

/**
*@file cairo-dock-applet-facility.h Les macros forment un canevas dedie aux applets. Elles permettent un developpement rapide et normalise d'une applet pour Cairo-Dock.
*
* Pour un exemple tres simple, consultez les sources de l'applet 'logout'.
*/

/**
*Verifie que le fichier de conf d'un plug-in est bien present dans le repertoire utilisateur du plug-in, sinon le copie a partir du fichier de conf fournit lors de l'installation. Cree au besoin le repertoire utilisateur du plug-in.
*@param cUserDataDirName le nom du repertoire utilisateur du plug-in.
*@param cShareDataDir le chemin du repertoire d'installation du plug-in.
*@param cConfFileName : le nom du fichier de conf fournit a l'installation.
*@return Le chemin du fichier de conf en espace utilisateur, ou NULL si le fichier n'a pu etre ni trouve, ni cree.
*/
gchar *cairo_dock_check_conf_file_exists (gchar *cUserDataDirName, gchar *cShareDataDir, gchar *cConfFileName);

void cairo_dock_free_minimal_config (CairoDockMinimalAppletConfig *pMinimalConfig);

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
*@param fMaxScale le facteur de zoom max.
*/
void cairo_dock_set_quick_info (cairo_t *pSourceContext, const gchar *cQuickInfo, Icon *pIcon, double fMaxScale);
/**
*Ecris une info-rapide sur l'icone, en prenant une chaine au format 'printf'.
*@param pSourceContext un contexte de dessin; n'est pas altere par la fonction.
*@param pIcon l'icone.
*@param pDock le dock contenant l'icone.
*@param cQuickInfoFormat le texte de l'info-rapide, au format 'printf' (%s, %d, etc)
*@param ... les donnees a inserer dans la chaine de caracteres.
*/
void cairo_dock_set_quick_info_full (cairo_t *pSourceContext, Icon *pIcon, CairoDock *pDock, const gchar *cQuickInfoFormat, ...);
/**
*Efface l'info-rapide d'une icone.
*@param pIcon l'icone.
*/
#define cairo_dock_remove_quick_info(pIcon) cairo_dock_set_quick_info (NULL, NULL, pIcon, 1)


/**
*Prepare l'animation d'une icone, et la lance immediatement.
*@param pIcon  l'icone a animer.
*@param pDock le dock contenant l'icone.
*@param iAnimationType le type d'animation voulu, ou -1 pour utiliser l'animtion correspondante au type de l'icone.
*@param iNbRounds le nombre de fois ou l'animation sera jouee, ou -1 pour utiliser la valeur correspondante au type de l'icone.
*/
void cairo_dock_animate_icon (Icon *pIcon, CairoDock *pDock, CairoDockAnimationType iAnimationType, int iNbRounds);

/**
*Liste les themes contenu dans un repertoire, met a jour le fichier de conf avec, et renvoie le chemin correspondant au theme choisi.
*@param cAppletShareDataDir chemin du repertoire contenant les donnees de l'applet.
*@param cThemeDirName nom du sous-repertoire regroupant tous les themes.
*@param cAppletConfFilePath chemin du fichier de conf.
*@param pKeyFile le fichier de conf ouvert.
*@param cGroupName nom du groupe (dans le fichier de conf) du parametre correspondant au theme.
*@param cKeyName nom de la cle (dans le fichier de conf) du parametre correspondant au theme.
*@param bFlushConfFileNeeded pointeur sur un booleen mis a TRUE si la cle n'existe pas.
*@param cDefaultThemeName nom du theme par defaut au cas ou le precedent n'existerait pas.
*@return Le chemin du repertoire du theme choisi, dans une chaine nouvellement allouee.
*/
gchar* cairo_dock_manage_themes_for_applet (gchar *cAppletShareDataDir, gchar *cThemeDirName, gchar *cAppletConfFilePath, GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gchar *cDefaultThemeName);

/**
*Cree un sous-menu, et l'ajoute a un menu deja existant.
*@param cLabel nom du sous-menu, tel qu'il apparaitra dans le menu.
*@param pMenu menu auquel on rajoutera le sous-menu.
*@return le sous-menu cree et ajoute au menu.
*/
GtkWidget *cairo_dock_create_sub_menu (gchar *cLabel, GtkWidget *pMenu);


//\_________________________________ INIT
/**
*Definition des fonctions d'initialisation de l'applet; a inclure dans le .h du fichier d'init de l'applet.
*/
#define CD_APPLET_H \
CairoDockVisitCard *pre_init (void);\
Icon *init (CairoDock *pDock, CairoDockModule *pModule, GError **erreur);\
void stop (void);\
gboolean reload (gchar *cConfFilePath);

//\______________________ pre_init.
/**
*Debut de la fonction de pre-initialisation de l'applet (celle qui est appele a l'enregistrement de tous les plug-ins).
*Defini egalement les variables globales suivantes : myIcon, myDock, myDesklet, et myDrawContext.
*@param cName nom de sous lequel l'applet sera enregistree par Cairo-Dock.
*@param iMajorVersion version majeure du dock necessaire au bon fonctionnement de l'applet.
*@param iMinorVersion version mineure du dock necessaire au bon fonctionnement de l'applet.
*@param iMicroVersion version micro du dock necessaire au bon fonctionnement de l'applet.
*/
#define CD_APPLET_PRE_INIT_BEGIN(cName, iMajorVersion, iMinorVersion, iMicroVersion) \
Icon *myIcon = NULL;\
CairoDock *myDock = NULL;\
CairoDockDesklet *myDesklet = NULL;\
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
	pVisitCard->cGettextDomain = g_strdup (MY_APPLET_GETTEXT_DOMAIN);\
	pVisitCard->cDockVersionOnCompilation = g_strdup (MY_APPLET_DOCK_VERSION);\
	pVisitCard->cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, MY_APPLET_CONF_FILE);\
	pVisitCard->cUserDataDir = MY_APPLET_USER_DATA_DIR;\
	pVisitCard->cShareDataDir = MY_APPLET_SHARE_DATA_DIR;\
	pVisitCard->cConfFileName = MY_APPLET_CONF_FILE;
/**
*Fin de la fonction de pre-initialisation de l'applet.
*/
#define CD_APPLET_PRE_INIT_END \
	return pVisitCard;\
}
/**
*Fonction de pre-initialisation generique. Ne fais que definir l'applet (en appelant les 2 macros precedentes), la plupart du temps cela est suffisant.
*/
#define CD_APPLET_DEFINITION(cName, iMajorVersion, iMinorVersion, iMicroVersion) \
CD_APPLET_PRE_INIT_BEGIN (cName, iMajorVersion, iMinorVersion, iMicroVersion) \
CD_APPLET_PRE_INIT_END



//\______________________ init.
/**
*Debut de la fonction d'initialisation de l'applet (celle qui est appelee a chaque chargement de l'applet).
*Lis le fichier de conf de l'applet, et cree son icone ainsi que son contexte de dessin.
*@param erreur une GError, utilisable pour reporter une erreur ayant lieu durant l'initialisation.
*/
#define CD_APPLET_INIT_BEGIN(erreur) \
Icon *init (CairoDock *pDock, CairoDockModule *pModule, GError **erreur) \
{ \
	gchar *cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, MY_APPLET_CONF_FILE); \
	CairoDockMinimalAppletConfig *pMinimalConfig = read_conf_file (cConfFilePath); \
	pModule->bCanDetach = pMinimalConfig->bCanDetach; \
	g_free (cConfFilePath); \
	if (pMinimalConfig->bIsDetached) \
	{\
		myDesklet = cairo_dock_create_desklet (NULL, NULL); \
		cairo_dock_place_desklet (myDesklet, pMinimalConfig->iDeskletWidth, pMinimalConfig->iDeskletHeight, pMinimalConfig->iDeskletPositionX, pMinimalConfig->iDeskletPositionY, pMinimalConfig->bKeepBelow, pMinimalConfig->bKeepAbove, pMinimalConfig->bOnWidgetLayer);\
	}\
	else \
	{\
		myDock = pDock; \
	}\
	myIcon = cairo_dock_create_icon_for_applet (myDock, myDesklet, (myDock != NULL ? pMinimalConfig->iDesiredIconWidth : MAX (1, pMinimalConfig->iDeskletWidth - 2 * g_iDockRadius)), (myDock != NULL ? pMinimalConfig->iDesiredIconHeight : MAX (1, pMinimalConfig->iDeskletHeight - 2 * g_iDockRadius)), pMinimalConfig->cLabel, pMinimalConfig->cIconFileName, pModule); \
	myIcon->fScale = 1;\
	myIcon->fDrawX = g_iDockRadius;\
	myIcon->fDrawY = g_iDockRadius;\
	if (myDesklet != NULL) \
	{\
		myDesklet->pIcon = myIcon; \
		gtk_widget_queue_draw (myDesklet->pWidget);\
	}\
	g_return_val_if_fail (myIcon != NULL, NULL); \
	myDrawContext = cairo_create (myIcon->pIconBuffer); \
	g_return_val_if_fail (cairo_status (myDrawContext) == CAIRO_STATUS_SUCCESS, NULL);
/**
*Fin de la fonction d'initialisation de l'applet.
*/
#define CD_APPLET_INIT_END \
	cairo_dock_free_minimal_config (pMinimalConfig); \
	return (myDock != NULL ? myIcon : NULL); \
}

//\______________________ stop.
/**
*Debut de la fonction d'arret de l'applet.
*/
#define CD_APPLET_STOP_BEGIN \
void stop (void) \
{
/**
*Fin de la fonction d'arret de l'applet.
*/
#define CD_APPLET_STOP_END \
	myDock = NULL; \
	myDesklet = NULL; \
	myIcon = NULL; \
	cairo_destroy (myDrawContext); \
	myDrawContext = NULL; \
}

//\______________________ reload.
/**
*Debut de la fonction de rechargement de l'applet.
*/
#define CD_APPLET_RELOAD_BEGIN \
gboolean reload (gchar *cConfFilePath) \
{\
	cd_message ("%s (%s, %d)\n", __func__, cConfFilePath, (cConfFilePath != NULL)); \
	CairoDockMinimalAppletConfig *pMinimalConfig  = NULL;\
	gboolean bToBeInserted = FALSE;\
	if (cConfFilePath != NULL)\
	{\
		cd_message ("On recharge notre config\n");\
		gchar *cAppletName = NULL, *cIconName = NULL;\
		pMinimalConfig = read_conf_file (cConfFilePath);\
		g_free (myIcon->acName);\
		myIcon->acName = pMinimalConfig->cLabel;\
		g_free (myIcon->acFileName);\
		myIcon->acFileName = pMinimalConfig->cIconFileName;\
		if (pMinimalConfig->bIsDetached)\
		{\
			if (myDesklet == NULL)\
			{\
				cairo_dock_detach_icon_from_dock (myIcon, myDock, g_bUseSeparator);\
				myDock = NULL;\
				myDesklet = cairo_dock_create_desklet (NULL, NULL);\
				cairo_dock_place_desklet (myDesklet, pMinimalConfig->iDeskletWidth, pMinimalConfig->iDeskletHeight, pMinimalConfig->iDeskletPositionX, pMinimalConfig->iDeskletPositionY, pMinimalConfig->bKeepBelow, pMinimalConfig->bKeepAbove, pMinimalConfig->bOnWidgetLayer);\
			}\
			myIcon->fWidth = MAX (1, myDesklet->iWidth - 2 * g_iDockRadius);\
			myIcon->fHeight= MAX (1, myDesklet->iHeight - 2 * g_iDockRadius);\
		}\
		else\
		{\
			if (myDock == NULL)\
			{\
				myDock = g_pMainDock;\
				cairo_dock_free_desklet (myDesklet);\
				myDesklet = NULL;\
				bToBeInserted = TRUE;\
				myIcon->fWidth = pMinimalConfig->iDesiredIconWidth;\
				myIcon->fHeight = pMinimalConfig->iDesiredIconHeight;\
			}\
		}\
	}\
	else if (myDesklet != NULL)\
	{\
		myIcon->fWidth = myDesklet->iWidth;\
		myIcon->fHeight = myDesklet->iHeight;\
	}\
	cairo_dock_load_one_icon_from_scratch (myIcon, myDock, myDesklet);\
	if (bToBeInserted)\
		cairo_dock_insert_icon_in_dock (myIcon, myDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);\
	if (myDesklet != NULL)\
	{\
		myIcon->fScale = 1;\
		myIcon->fDrawX = g_iDockRadius;\
		myIcon->fDrawY = g_iDockRadius;\
		myDesklet->pIcon = myIcon;\
		gtk_widget_queue_draw (myDesklet->pWidget);\
	}\
	cairo_destroy (myDrawContext);\
	myDrawContext = cairo_create (myIcon->pIconBuffer);\
	g_return_val_if_fail (cairo_status (myDrawContext) == CAIRO_STATUS_SUCCESS, FALSE);

/**
*Fin de la fonction de rechargement de l'applet.
*/
#define CD_APPLET_RELOAD_END \
	g_free (pMinimalConfig);\
	return TRUE; \
}

/**
*Chemin du fichier de conf de l'applet, appelable durant les fonctions d'init, de config, et de reload.
*/
#define CD_APPLET_MY_CONFIG_CHANGED (cConfFilePath != NULL)

/**
*Chemin du fichier de conf de l'applet, appelable durant les fonctions d'init, de config, et de reload.
*/
#define CD_APPLET_MY_CONF_FILE cConfFilePath


//\_________________________________ CONFIG
//\______________________ read_conf_file.
/**
*Debut de la fonction de configuration de l'applet (celle qui est appelee au debt de l'init).
*Ouvre le fichier de conf de l'applet, et charge les parametres generiques suivant (groupe "Icon") : largeur ("width"), hauteur ("height), nom ("name", optionnel), et icone ("icon", optionnel).
*@param cDefaultAppletName nom par defaut de l'applet dans le dock (son etiquette), ou NULL si aucun nom ne doit etre lu dans la conf (habituellement l'etiquette reprend simplement le nom de l'applet).
*@param cDefaultIconName nom de l'icone a appliquer par defaut a l'applet, ou NULL si aucune icone ne doit etre lue dans la conf (auquel cas la surface de l'icone sera juste transparente).
*/
#define CD_APPLET_CONFIG_BEGIN(cDefaultAppletName, cDefaultIconName) \
CairoDockMinimalAppletConfig *read_conf_file (gchar *cConfFilePath) \
{ \
	cd_message ("%s (%s)\n", __func__, cConfFilePath); \
	GError *erreur = NULL; \
	gboolean bFlushConfFileNeeded = FALSE, bNewKeysPresent = FALSE; \
	GKeyFile *pKeyFile = g_key_file_new (); \
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur); \
	if (erreur != NULL) \
	{ \
		cd_message ("Attention : %s\n", erreur->message); \
		g_error_free (erreur); \
		return NULL; \
	} \
	CairoDockMinimalAppletConfig *pMinimalConfig = g_new0 (CairoDockMinimalAppletConfig, 1); \
	pMinimalConfig->iDesiredIconWidth = cairo_dock_get_integer_key_value (pKeyFile, "Icon", "width", &bFlushConfFileNeeded, 48, NULL, NULL); \
	pMinimalConfig->iDesiredIconHeight = cairo_dock_get_integer_key_value (pKeyFile, "Icon", "height", &bFlushConfFileNeeded, 48, NULL, NULL); \
	if (cDefaultAppletName != NULL) \
		pMinimalConfig->cLabel = cairo_dock_get_string_key_value (pKeyFile, "Icon", "name", &bFlushConfFileNeeded, cDefaultAppletName, NULL, NULL); \
	if (cDefaultIconName != NULL) \
		pMinimalConfig->cIconFileName = cairo_dock_get_string_key_value (pKeyFile, "Icon", "icon", &bFlushConfFileNeeded, cDefaultIconName, NULL, NULL); \
	pMinimalConfig->bCanDetach = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "can detach", NULL, FALSE, NULL, NULL); \
	if (pMinimalConfig->bCanDetach)\
	{\
		pMinimalConfig->iDeskletWidth = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "width", &bNewKeysPresent, 92, NULL, NULL); \
		pMinimalConfig->iDeskletHeight = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "height", &bNewKeysPresent, 92, NULL, NULL); \
		pMinimalConfig->iDeskletPositionX = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "x position", &bNewKeysPresent, 0, NULL, NULL); \
		pMinimalConfig->iDeskletPositionY = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "y position", &bNewKeysPresent, 0, NULL, NULL); \
		pMinimalConfig->bIsDetached = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "initially detached", &bNewKeysPresent, FALSE, NULL, NULL); \
		pMinimalConfig->bKeepBelow = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "keep below", &bNewKeysPresent, FALSE, NULL, NULL); \
		pMinimalConfig->bKeepAbove = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "keep above", &bNewKeysPresent, FALSE, NULL, NULL); \
		pMinimalConfig->bOnWidgetLayer = cairo_dock_get_boolean_key_value (pKeyFile, "Desklet", "on widget layer", &bNewKeysPresent, FALSE, NULL, NULL);\
	}
/**
*Fin de la fonction de configuration de l'applet.
*/
#define CD_APPLET_CONFIG_END \
	if (bNewKeysPresent) \
		cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath); \
	if (! bFlushConfFileNeeded) \
		bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile, MY_APPLET_VERSION); \
	if (bFlushConfFileNeeded) \
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, MY_APPLET_SHARE_DATA_DIR);\
	g_key_file_free (pKeyFile); \
	return pMinimalConfig; \
}

/**
*Definition de la fonction de configuration, a inclure dans le .h correspondant.
*/
#define CD_APPLET_CONFIG_H \
CairoDockMinimalAppletConfig *read_conf_file (gchar *cConfFilePath);

/**
*Recupere la valeur d'un parametre 'booleen' du fichier de conf.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param bDefaultValue valeur par defaut si la cle et/ou le groupe n'est pas trouve (typiquement si cette cle est nouvelle).
*@return un gboolean.
*/
#define CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT(cGroupName, cKeyName, bDefaultValue) cairo_dock_get_boolean_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, bDefaultValue, NULL, NULL)

/**
*Recupere la valeur d'un parametre 'booleen' du fichier de conf, avec TRUE comme valeur par defaut.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@return un gboolean.
*/
#define CD_CONFIG_GET_BOOLEAN(cGroupName, cKeyName) \
CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT (cGroupName, cKeyName, TRUE)

/**
*Recupere la valeur d'un parametre 'entier' du fichier de conf.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param iDefaultValue valeur par defaut si la cle et/ou le groupe n'est pas trouve (typiquement si cette cle est nouvelle).
*@return un entier.
*/
#define CD_CONFIG_GET_INTEGER_WITH_DEFAULT(cGroupName, cKeyName, iDefaultValue) cairo_dock_get_integer_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, iDefaultValue, NULL, NULL)
/**
*Recupere la valeur d'un parametre 'entier' du fichier de conf, avec 0 comme valeur par defaut.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@return un entier.
*/
#define CD_CONFIG_GET_INTEGER(cGroupName, cKeyName)CD_CONFIG_GET_INTEGER_WITH_DEFAULT (cGroupName, cKeyName, 0)

/**
*Recupere la valeur d'un parametre 'double' du fichier de conf.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param fDefaultValue valeur par defaut si la cle et/ou le groupe n'est pas trouve (typiquement si cette cle est nouvelle).
*@return un double.
*/
#define CD_CONFIG_GET_DOUBLE_WITH_DEFAULT(cGroupName, cKeyName, fDefaultValue) cairo_dock_get_double_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, 0., NULL, NULL)
/**
*Recupere la valeur d'un parametre 'double' du fichier de conf, avec 0. comme valeur par defaut.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@return un double.
*/
#define CD_CONFIG_GET_DOUBLE(cGroupName, cKeyName) CD_CONFIG_GET_DOUBLE_WITH_DEFAULT (cGroupName, cKeyName, 0.)

/**
*Recupere la valeur d'un parametre 'chaine de caracteres' du fichier de conf.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param cDefaultValue valeur par defaut si la cle et/ou le groupe n'est pas trouve (typiquement si cette cle est nouvelle). NULL accepte.
*@return une chaine de caracteres nouvellement allouee.
*/
#define CD_CONFIG_GET_STRING_WITH_DEFAULT(cGroupName, cKeyName, cDefaultValue) cairo_dock_get_string_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, cDefaultValue, NULL, NULL)
/**
*Recupere la valeur d'un parametre 'chaine de caracteres' du fichier de conf, avec NULL comme valeur par defaut.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@return une chaine de caracteres nouvellement allouee.
*/
#define CD_CONFIG_GET_STRING(cGroupName, cKeyName) CD_CONFIG_GET_STRING_WITH_DEFAULT (cGroupName, cKeyName, NULL)

/**
*Recupere la valeur d'un parametre 'liste de chaines de caracteres' du fichier de conf.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param length pointeur sur un entier, rempli avec le nombre de chaines recuperees.
*@param cDefaultValues valeur par defaut si la cle et/ou le groupe n'est pas trouve (typiquement si cette cle est nouvelle). C'est une chaine de caractere contenant les mots separes par des ';', ou NULL.
*@return un tableau de chaine de caracteres, a liberer avec 'g_strfreev'.
*/
#define CD_CONFIG_GET_STRING_LIST_WITH_DEFAULT(cGroupName, cKeyName, length, cDefaultValues) cairo_dock_get_string_list_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, length, cDefaultValues, NULL, NULL)
/**
*Recupere la valeur d'un parametre 'liste de chaines de caracteres' du fichier de conf, avec NULL comme valeur par defaut.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param length pointeur sur un entier, rempli avec le nombre de chaines recuperees.
*@return un tableau de chaine de caracteres, a liberer avec 'g_strfreev'.
*/
#define CD_CONFIG_GET_STRING_LIST(cGroupName, cKeyName, length) CD_CONFIG_GET_STRING_LIST_WITH_DEFAULT(cGroupName, cKeyName, length, NULL)

/**
*Recupere la valeur d'un parametre 'type d'animation' du fichier de conf.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param iDefaultAnimation valeur par defaut si la cle et/ou le groupe n'est pas trouve (typiquement si cette cle est nouvelle).
*@return le type de l'animation, un #CairoDockAnimationType.
*/
#define CD_CONFIG_GET_ANIMATION_WITH_DEFAULT(cGroupName, cKeyName, iDefaultAnimation) cairo_dock_get_animation_type_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, iDefaultAnimation, NULL, NULL);
/**
*Recupere la valeur d'un parametre 'type d'animation' du fichier de conf, avec #CAIRO_DOCK_BOUNCE comme valeur par defaut.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@return le type de l'animation, un #CairoDockAnimationType.
*/
#define CD_CONFIG_GET_ANIMATION(cGroupName, cKeyName) CD_CONFIG_GET_ANIMATION_WITH_DEFAULT(cGroupName, cKeyName, CAIRO_DOCK_BOUNCE)

/**
*Recupere la valeur d'un parametre 'couleur' au format RVBA. du fichier de conf.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param pColorBuffer tableau de 4 double deja alloue, et qui sera rempli avec les 4 composantes de la couleur.
*@param pDefaultColor valeur par defaut si la cle et/ou le groupe n'est pas trouve (typiquement si cette cle est nouvelle). C'est un tableau de 4 double, ou NULL.
*/
#define CD_CONFIG_GET_COLOR_WITH_DEFAULT(cGroupName, cKeyName, pColorBuffer, pDefaultColor) cairo_dock_get_double_list_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, pColorBuffer, 4, pDefaultColor, NULL, NULL);
/**
*Recupere la valeur d'un parametre 'couleur' au format RVBA. du fichier de conf, avec NULL comme valeur par defaut.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param pColorBuffer tableau de 4 double deja alloue, et qui sera rempli avec les 4 composantes de la couleur.
*/
#define CD_CONFIG_GET_COLOR(cGroupName, cKeyName, pColorBuffer) CD_CONFIG_GET_COLOR_WITH_DEFAULT(cGroupName, cKeyName, pColorBuffer, NULL)
/**
*Recupere la valeur d'un parametre 'couleur' au format RVB. du fichier de conf.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param pColorBuffer tableau de 3 double deja alloue, et qui sera rempli avec les 3 composantes de la couleur.
*@param pDefaultColor valeur par defaut si la cle et/ou le groupe n'est pas trouve (typiquement si cette cle est nouvelle). C'est un tableau de 3 double, ou NULL.
*/
#define CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT(cGroupName, cKeyName, pColorBuffer, pDefaultColor) cairo_dock_get_double_list_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, pColorBuffer, 3, pDefaultColor, NULL, NULL);
/**
*Recupere la valeur d'un parametre 'couleur' au format RVB. du fichier de conf, avec NULL comme valeur par defaut.
*@param cGroupName nom du groupe dans le fichier de conf.
*@param cKeyName nom de la cle dans le fichier de conf.
*@param pColorBuffer tableau de 3 double deja alloue, et qui sera rempli avec les 3 composantes de la couleur.
*/
#define CD_CONFIG_GET_COLOR_RVB(cGroupName, cKeyName, pColorBuffer) CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT(cGroupName, cKeyName, pColorBuffer, NULL)

/**
*Liste les themes contenu dans un repertoire, met a jour le fichier de conf avec, et renvoie le chemin correspondant au theme choisi.
*@param cGroupName nom du groupe (dans le fichier de conf) du parametre correspondant au theme.
*@param cKeyName nom de la cle (dans le fichier de conf) du parametre correspondant au theme.
*@param cThemesDirName nom du sous-repertoire regroupant tous les themes.
*@param cDefaultThemeName valeur par defaut si la cle et/ou le groupe et/ou le theme n'existe(nt) pas.
*@return Le chemin vers le repertoire du theme, dans une chaine nouvellement allouee.
*/
#define CD_CONFIG_GET_THEME_PATH(cGroupName, cKeyName, cThemesDirName, cDefaultThemeName) \
cairo_dock_manage_themes_for_applet (MY_APPLET_SHARE_DATA_DIR, cThemesDirName, CD_APPLET_MY_CONF_FILE, pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, cDefaultThemeName)


//\_________________________________ NOTIFICATIONS
//\______________________ fonction about.
/**
*Fonction 'A propos' toute faite, qui affiche un message dans une info-bulle. A inclure dans le .c.
*@param cMessage message a afficher dans l'info-bulle.
*/
#define CD_APPLET_ABOUT(cMessage) \
void about (GtkMenuItem *menu_item, gpointer *data) \
{ \
	cairo_dock_show_temporary_dialog (cMessage, myIcon, myDock, 0); \
}
/**
*Definition de la fonction precedente; a inclure dans le .h correspondant.
*/
#define CD_APPLET_ABOUT_H \
void about (GtkMenuItem *menu_item, gpointer *data);

//\______________________ notification clique gauche.
#define CD_APPLET_ON_CLICK action_on_click
/**
*Abonne l'applet aux notifications du clic gauche. A effectuer lors de l'init de l'applet.
*/
#define CD_APPLET_REGISTER_FOR_CLICK_EVENT cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK, CAIRO_DOCK_RUN_FIRST);
/**
*Desabonne l'applet aux notifications du clic gauche. A effectuer lors de l'arret de l'applet.
*/
#define CD_APPLET_UNREGISTER_FOR_CLICK_EVENT cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK);

/**
*Debut de la fonction de notification au clic gauche.
*/
#define CD_APPLET_ON_CLICK_BEGIN \
gboolean CD_APPLET_ON_CLICK (gpointer *data) \
{ \
	Icon *pClickedIcon = data[0]; \
	CairoDock *pClickedDock = data[1]; \
	if (pClickedIcon == myIcon || (myIcon != NULL && pClickedDock == myIcon->pSubDock)) \
	{
/**
*Fin de la fonction de notification au clic gauche. Par defaut elle intercepte la notification si elle l'a recue.
*/
#define CD_APPLET_ON_CLICK_END \
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION; \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION; \
}
/**
*Definition de la fonction precedente; a inclure dans le .h correspondant.
*/
#define CD_APPLET_ON_CLICK_H \
gboolean CD_APPLET_ON_CLICK (gpointer *data);

//\______________________ notification construction menu.
#define CD_APPLET_ON_BUILD_MENU applet_on_build_menu
/**
*Abonne l'applet aux notifications de construction du menu. A effectuer lors de l'init de l'applet.
*/
#define CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU, CAIRO_DOCK_RUN_FIRST);
/**
*Desabonne l'applet aux notifications de construction du menu. A effectuer lors de l'arret de l'applet.
*/
#define CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU);

/**
*Debut de la fonction de notification de construction du menu.
*/
#define CD_APPLET_ON_BUILD_MENU_BEGIN \
gboolean CD_APPLET_ON_BUILD_MENU (gpointer *data) \
{ \
	Icon *pClickedIcon = data[0]; \
	CairoDock *pClickedDock = data[1]; \
	if (pClickedIcon == myIcon || (myIcon != NULL && pClickedDock == myIcon->pSubDock)) \
	{ \
		GtkWidget *pAppletMenu = data[2]; \
		GtkWidget *pMenuItem, image;
/**
*Fin de la fonction de notification de construction du menu. Par defaut elle intercepte la notification si elle l'a recue.
*/
#define CD_APPLET_ON_BUILD_MENU_END \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION; \
}
/**
*Definition de la fonction precedente; a inclure dans le .h correspondant.
*/
#define CD_APPLET_ON_BUILD_MENU_H \
gboolean CD_APPLET_ON_BUILD_MENU (gpointer *data);

/**
*Menu principal de l'applet.
*/
#define CD_APPLET_MY_MENU pAppletMenu
/**
*Icone cliquee droit.
*/
#define CD_APPLET_CLICKED_ICON pClickedIcon

/**
*Cree et ajoute un sous-menu a un menu.
*@param cLabel nom du sous-menu, tel qu'il apparaitra dans le menu.
*@param pSubMenu GtkWidget du sous-menu; il doit juste avoir ete declare, il sera cree par la macro.
*@param pMenu GtkWidget du menu auquel on rajoutera le sous-menu.
*/
#define CD_APPLET_ADD_SUB_MENU(cLabel, pSubMenu, pMenu) \
	GtkWidget *pSubMenu = gtk_menu_new (); \
	pMenuItem = gtk_menu_item_new_with_label (cLabel); \
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem); \
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);

/**
*Cree et ajoute un sous-menu a un menu deja existant.
*@param cLabel nom du sous-menu, tel qu'il apparaitra dans le menu.
*@param pMenu GtkWidget du menu auquel on rajoutera le sous-menu.
*@return le GtkWidget du sous-menu.
*/
#define CD_APPLET_CREATE_AND_ADD_SUB_MENU(cLabel, pMenu) \
cairo_dock_create_sub_menu (cLabel, pMenu);

/**
*Cree et ajoute un sous-menu a un menu.
*@param cLabel nom du sous-menu, tel qu'il apparaitra dans le menu.
*@param pFunction fonction appelee lors de la selection de cette entree.
*@param pMenu GtkWidget du menu auquel on rajoutera le sous-menu.
*@param pData donnees passees en parametre de la fonction.
*/
#define CD_APPLET_ADD_IN_MENU_WITH_DATA(cLabel, pFunction, pMenu, pData) \
	pMenuItem = gtk_menu_item_new_with_label (cLabel); \
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem); \
	g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (pFunction), pData);

/**
*Ajoute une entree a un menu deja existant.
*@param cLabel nom de l'entree, tel qu'il apparaitra dans le menu.
*@param pFunction fonction appelee lors de la selection de cette entree.
*@param pMenu GtkWidget du menu auquel on rajoutera l'entree.
*/
#define CD_APPLET_ADD_IN_MENU(cLabel, pFunction, pMenu) CD_APPLET_ADD_IN_MENU_WITH_DATA(cLabel, pFunction, pMenu, NULL)

#define CD_APPLET_ADD_IN_MENU_WITH_STOCK(cLabel, gtkStock, pFunction, pMenu, pData) \
	menu_item = gtk_image_menu_item_new_with_label (_(cLabel));\
	image = gtk_image_new_from_stock (gtkStock, GTK_ICON_SIZE_MENU);\
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);\
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), menu_item);\
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(pFunction), pData);


/**
 * Ajoute un separateur dans un menu deja existant
 */
#define CD_APPLET_ADD_SEPARATOR()                               \
  pMenuItem = gtk_separator_menu_item_new ();                   \
  gtk_menu_shell_append(GTK_MENU_SHELL (pSubMenu), pMenuItem);

/**
*Ajoute une entree pour la fonction 'A propos'.
*@param pMenu GtkWidget du menu auquel sera ajoutee l'entree.
*/
#define CD_APPLET_ADD_ABOUT_IN_MENU(pMenu) CD_APPLET_ADD_IN_MENU (_("About"), about, pMenu)

/**
*Recupere la derniere entree ajoutee dans la fonction.
*@return le GtkWidget de la derniere entree.
*/
#define CD_APPLET_LAST_ITEM_IN_MENU pMenuItem

//\______________________ notification clique milieu.
#define CD_APPLET_ON_MIDDLE_CLICK action_on_middle_click
/**
*Abonne l'applet aux notifications du clic du milieu. A effectuer lors de l'init de l'applet.
*/
#define CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK, CAIRO_DOCK_RUN_FIRST);
/**
*Desabonne l'applet aux notifications du clic du milieu. A effectuer lors de l'arret de l'applet.
*/
#define CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT cairo_dock_remove_notification_func (CAIRO_DOCK_MIDDLE_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK);

/**
*Debut de la fonction de notification du clic du milieu.
*/
#define CD_APPLET_ON_MIDDLE_CLICK_BEGIN \
gboolean CD_APPLET_ON_MIDDLE_CLICK (gpointer *data) \
{ \
	Icon *pClickedIcon = data[0]; \
	CairoDock *pClickedDock = data[1]; \
	if (pClickedIcon == myIcon || (myIcon != NULL && pClickedDock == myIcon->pSubDock)) \
	{
/**
*Fin de la fonction de notification du clic du milieu. Par defaut elle intercepte la notification si elle l'a recue.
*/
#define CD_APPLET_ON_MIDDLE_CLICK_END \
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION; \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION; \
}
/**
*Definition de la fonction precedente; a inclure dans le .h correspondant.
*/
#define CD_APPLET_ON_MIDDLE_CLICK_H \
gboolean CD_APPLET_ON_MIDDLE_CLICK (gpointer *data);

//\______________________ notification drag'n'drop.
#define CD_APPLET_ON_DROP_DATA action_on_drop_data
/**
*Abonne l'applet aux notifications du glisse-depose. A effectuer lors de l'init de l'applet.
*/
#define CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA, (CairoDockNotificationFunc) CD_APPLET_ON_DROP_DATA, CAIRO_DOCK_RUN_FIRST);
/**
*Desabonne l'applet aux notifications du glisse-depose. A effectuer lors de l'arret de l'applet.
*/
#define CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT cairo_dock_remove_notification_func (CAIRO_DOCK_DROP_DATA, (CairoDockNotificationFunc) CD_APPLET_ON_DROP_DATA);

/**
*Debut de la fonction de notification du glisse-depose.
*/
#define CD_APPLET_ON_DROP_DATA_BEGIN \
gboolean CD_APPLET_ON_DROP_DATA (gpointer *data) \
{ \
	if (data[1] == myIcon) \
	{ \
		const gchar *cReceivedData = data[0]; \
		g_return_val_if_fail (cReceivedData != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);

/**
*Donnees recues (chaine de caracteres).
*/
#define CD_APPLET_RECEIVED_DATA cReceivedData

/**
*Fin de la fonction de notification du glisse-depose. Par defaut elle intercepte la notification si elle l'a recue.
*/
#define CD_APPLET_ON_DROP_DATA_END \
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION; \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION; \
}
/**
*Definition de la fonction precedente; a inclure dans le .h correspondant.
*/
#define CD_APPLET_ON_DROP_DATA_H \
gboolean CD_APPLET_ON_DROP_DATA (gpointer *data);


//\_________________________________ DESSIN

/**
*Redessine immediatement l'icone de l'applet.
*/
#define CD_APPLET_REDRAW_MY_ICON \
	cairo_dock_redraw_my_icon (myIcon, (myDock != NULL ? myDock : myDesklet));

/**
*Applique une surface existante sur le contexte de dessin de l'applet, et la redessine. La surface est redimensionnee aux dimensions de l'icone.
*@param pSurface
*/
#define CD_APPLET_SET_SURFACE_ON_MY_ICON(pSurface) \
	cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, (myDock != NULL ? myDock : myDesklet)); \
	cairo_dock_redraw_my_icon (myIcon, (myDock != NULL ? myDock : myDesklet));

/**
*Applique une image definie par son chemin sur le contexte de dessin de l'applet, et la redessine. L'image est redimensionnee aux dimensions de l'icone.
*@param cImagePath chemin du fichier de l'image.
*/
#define CD_APPLET_SET_IMAGE_ON_MY_ICON(cImagePath) \
	cairo_dock_set_image_on_icon (myDrawContext, cImagePath, myIcon, (myDock != NULL ? myDock : myDesklet));

/**
*Remplace l'etiquette de l'icone de l'applet par une nouvelle.
*@param cIconName la nouvelle etiquette.
*/
#define CD_APPLET_SET_NAME_FOR_MY_ICON(cIconName) \
	cairo_dock_set_icon_name (myDrawContext, cIconName, myIcon, (myDock != NULL ? myDock : myDesklet));

/**
*Ecris une info-rapide sur l'icone de l'applet.
*@param cQuickInfoFormat l'info-rapide, au format 'printf'. Ce doit etre une chaine de caracteres particulièrement petite, representant une info concise, puisque ecrite directement sur l'icone.
*/
#define CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(cQuickInfoFormat, ...) \
	cairo_dock_set_quick_info_full (myDrawContext, myIcon, (myDock != NULL ? myDock : myDesklet), cQuickInfoFormat, ##__VA_ARGS__);
/**
*Ecris une info-rapide sur l'icone de l'applet et la redessine.
*@param cQuickInfoFormat l'info-rapide, au format 'printf'. Ce doit etre une chaine de caracteres particulièrement petite, representant une info concise, puisque ecrite directement sur l'icone.
*/
#define CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_AND_REDRAW(cQuickInfoFormat, ...) \
	cairo_dock_set_quick_info_full (myDrawContext, myIcon, (myDock != NULL ? myDock : myDesklet), cQuickInfoFormat, ##__VA_ARGS__); \
	cairo_dock_redraw_my_icon (myIcon, (myDock != NULL ? myDock : myDesklet));

/**
*Ecris une info-rapide sur l'icone de l'applet et la redessine.
*@param cQuickInfoFormat l'info-rapide, au format 'printf'. Ce doit etre une chaine de caracteres particulièrement petite, representant une info concise, puisque ecrite directement sur l'icone.
*/


/**
*Lance l'animation de l'icone de l'applet.
*@param iAnimationType type de l'animation (un #CairoDockAnimationType).
*@param iAnimationLength duree de l'animation, en nombre de tours.
*/
#define CD_APPLET_ANIMATE_MY_ICON(iAnimationType, iAnimationLength) \
	cairo_dock_animate_icon (myIcon, (myDock != NULL ? myDock : myDesklet), iAnimationType, iAnimationLength);

/**
*Charge une image dans une surface, aux dimensions de l'icone de l'applet.
*@param cImagePath chemin du fichier de l'image.
*@return la surface nouvellement creee.
*/
#define CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET(cImagePath) cairo_dock_create_surface_for_icon (cImagePath, myDrawContext, myIcon->fWidth * (myDock != NULL ? 1 + g_fAmplitude : 1), myIcon->fHeight* (myDock != NULL ? 1 + g_fAmplitude : 1));

//\_________________________________ INCLUDE
/**
*Exportation des variables globales de l'applet. A inclure dans chaque .c ou elles sont utilisees (directement ou via les macros).
*/
#define CD_APPLET_INCLUDE_MY_VARS \
extern Icon *myIcon; \
extern cairo_t *myDrawContext; \
extern CairoDock *myDock; \
extern CairoDockDesklet *myDesklet;

//\_________________________________ INTERNATIONNALISATION
/**
*Macro pour gettext, similaire a _() et _N(), mais avec le nom de domaine en parametre. Encadrez toutes vos chaines de caracteres statiques avec ca, de maniere a ce que l'utilitaire 'xgettext' puisse les trouver et les inclure autimatiquement dans les fichiers de traduction.
*/
#define _D(message) dgettext (MY_APPLET_GETTEXT_DOMAIN, message)

#endif
