
#ifndef __CAIRO_DOCK_CONFIG__
#define  __CAIRO_DOCK_CONFIG__

#include <glib.h>

#include "cairo-dock-struct.h"

/**
*Recupere une cle booleene d'un fichier de cles.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param bDefaultValue valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@param cDefaultGroupName nom de groupe alternatif, ou NULL si aucun autre.
*@param cDefaultKeyName nom de cle alternative, ou NULL si aucune autre.
*@return la valeur booleene de la cle.
*/
gboolean cairo_dock_get_boolean_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gboolean bDefaultValue, gchar *cDefaultGroupName, gchar *cDefaultKeyName);
/**
*Recupere une cle entiere d'un fichier de cles.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param iDefaultValue valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@param cDefaultGroupName nom de groupe alternatif, ou NULL si aucun autre.
*@param cDefaultKeyName nom de cle alternative, ou NULL si aucune autre.
@return la valeur entiere de la cle.
*/
int cairo_dock_get_integer_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int iDefaultValue, gchar *cDefaultGroupName, gchar *cDefaultKeyName);
/**
*Recupere une cle flottante d'un fichier de cles.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param fDefaultValue valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@param cDefaultGroupName nom de groupe alternatif, ou NULL si aucun autre.
*@param cDefaultKeyName nom de cle alternative, ou NULL si aucune autre.
*@return la valeur flottante de la cle.
*/
double cairo_dock_get_double_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double fDefaultValue, gchar *cDefaultGroupName, gchar *cDefaultKeyName);
/**
*Recupere une cle d'un fichier de cles sous la forme d'une chaine de caractere.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param cDefaultValue valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@param cDefaultGroupName nom de groupe alternatif, ou NULL si aucun autre.
*@param cDefaultKeyName nom de cle alternative, ou NULL si aucune autre.
*@return la chaine de caractere nouvellement allouee correspondante a la cle.
*/
gchar *cairo_dock_get_string_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, const gchar *cDefaultValue, gchar *cDefaultGroupName, gchar *cDefaultKeyName);
/**
*Recupere une cle d'un fichier de cles sous la forme d'un tableau d'entiers.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param iValueBuffer tableau qui sera rempli.
*@param iNbElements nombre d'elements a recuperer; c'est le nombre d'elements du tableau passe en entree.
*@param iDefaultValues valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@param cDefaultGroupName nom de groupe alternatif, ou NULL si aucun autre.
*@param cDefaultKeyName nom de cle alternative, ou NULL si aucune autre.
*/
void cairo_dock_get_integer_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, int *iValueBuffer, int iNbElements, int *iDefaultValues, gchar *cDefaultGroupName, gchar *cDefaultKeyName);
/**
*Recupere une cle d'un fichier de cles sous la forme d'un tableau de doubles.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param fValueBuffer tableau qui sera rempli.
*@param iNbElements nombre d'elements a recuperer; c'est le nombre d'elements du tableau passe en entree.
*@param fDefaultValues valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@param cDefaultGroupName nom de groupe alternatif, ou NULL si aucun autre.
*@param cDefaultKeyName nom de cle alternative, ou NULL si aucune autre.
*/
void cairo_dock_get_double_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, double *fValueBuffer, int iNbElements, double *fDefaultValues, gchar *cDefaultGroupName, gchar *cDefaultKeyName);
/**
*Recupere une cle d'un fichier de cles sous la forme d'un tableau de chaines de caracteres.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param length nombre de chaines de caracteres recuperees.
*@param cDefaultValues valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@param cDefaultGroupName nom de groupe alternatif, ou NULL si aucun autre.
*@param cDefaultKeyName nom de cle alternative, ou NULL si aucune autre.
*@return un tableau de chaines de caracteres; a liberer avec g_strfreev().
*/
gchar **cairo_dock_get_string_list_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gsize *length, gchar *cDefaultValues, gchar *cDefaultGroupName, gchar *cDefaultKeyName);
/**
*Recupere une cle d'un fichier de cles sous la forme d'un tableau de chaines de caracteres.
*@param pKeyFile le fichier de cles.
*@param cGroupName le com du groupe.
*@param cKeyName le nom de la cle.
*@param bFlushConfFileNeeded est mis a TRUE si la cle est manquante.
*@param iDefaultAnimation valeur par defaut a utiliser et a inserer dans le fichier de cles au cas ou la cle est manquante.
*@param cDefaultGroupName nom de groupe alternatif, ou NULL si aucun autre.
*@param cDefaultKeyName nom de cle alternative, ou NULL si aucune autre.
*@return le type de l'animation correspondante a la cle.
*/
CairoDockAnimationType cairo_dock_get_animation_type_key_value (GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, CairoDockAnimationType iDefaultAnimation, gchar *cDefaultGroupName, gchar *cDefaultKeyName);

/**
*Lis le fichier de conf et recharge l'appli en consequence.
*@param cConfFilePath chemin du fichier de conf.
*@param pDock le dock principal, cree prealablement si necessaire.
*/
void cairo_dock_read_conf_file (gchar *cConfFilePath, CairoDock *pDock);

/**
*Dis si l'appli est en cours de chargement.
*@return TRUE ssi le dock est en cours de rechargement.
*/
gboolean cairo_dock_is_loading (void);

/**
*Lis un fichier de conf, construit l'IHM appropriee, et la presente a l'utilisateur.
*@param pWindow fenetre pour laquelle le panneau de conf sera modal.
*@param cConfFilePath chemin du fichier de conf.
*@param cTitle titre du panneau de conf.
*@param iWindowWidth largeur du panneau de conf.
*@param iWindowHeight hauteur du panneau de conf.
*@param iIdentifier identifiant des cles, ou 0 pour aucun.
*@param cPresentedGroup onglet devant etre selectionne initialement.
*@param pConfigFunc fonction appelee a l'appui sur le bouton "valider"
*@param data donnees passees a la fonction.
*@param pFreeUserDataFunc fonction liberant les donnees, ou NULL.
*@param pConfigFunc2 2eme fonction dans le cas d'une config s'affichant sous 2 formes.
*@param cConfFilePath2 2eme fichier de conf dans le cas d'une config s'affichant sous 2 formes.
*@param cButtonConvert chaine a afficher dans le bouton permettant de passer d'une forme a l'autre.
*@param cButtonRevert chaine a afficher dans le bouton permettant de passer d'une forme a l'autre dans l'autre sens.
*@param cGettextDomain nom du domainde de traduction a utiliser, ou NULL pour utiliser celui du dock.
*@return TRUE si l'utilisateur a ferme le panneau de conf en appuyant sur OK, FALSE sinon.
*/
gboolean cairo_dock_edit_conf_file_full (GtkWindow *pWindow, gchar *cConfFilePath, gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CairoDockConfigFunc pConfigFunc, gpointer data, GFunc pFreeUserDataFunc, CairoDockConfigFunc pConfigFunc2, gchar *cConfFilePath2, gchar *cButtonConvert, gchar *cButtonRevert, gchar *cGettextDomain);
#define cairo_dock_edit_conf_file(pWindow, cConfFilePath, cTitle, iWindowWidth, iWindowHeight, iIdentifier, cPresentedGroup, pConfigFunc, data, pFreeUserDataFunc, cGettextDomain) cairo_dock_edit_conf_file_full (pWindow, cConfFilePath, cTitle, iWindowWidth, iWindowHeight, iIdentifier, cPresentedGroup, pConfigFunc, data, pFreeUserDataFunc, NULL, NULL, NULL, NULL, cGettextDomain)

/**
*Met a jour un fichier de conf avec une liste de valeurs de la forme : type, nom du groupe, nom de la cle, valeur. Finir par G_TYPE_INVALID.
*@param cConfFilePath chemin du fichier de conf.
*@param iFirstDataType type de la 1ere donnee.
*/
void cairo_dock_update_conf_file (gchar *cConfFilePath, GType iFirstDataType, ...);

void cairo_dock_update_conf_file_with_position (gchar *cConfFilePath, int x, int y);

/**
*Essaye de determiner l'environnement de bureau dela session courante.
*@return l'environnement de bureau (couramment Gnome et KDE son detectes).
*/
CairoDockDesktopEnv cairo_dock_guess_environment (void);


void cairo_dock_copy_easy_conf_file (gchar *cEasyConfFilePath, GKeyFile *pMainKeyFile);
void cairo_dock_copy_to_easy_conf_file (GKeyFile *pMainKeyFile, gchar *cEasyConfFilePath);
void cairo_dock_build_easy_conf_file (gchar *cMainConfFilePath, gchar *cEasyConfFilePath);
void cairo_dock_read_easy_conf_file (gchar *cEasyConfFilePath, gpointer data);

gboolean cairo_dock_use_full_conf_file (void);
void cairo_dock_mark_prefered_conf_file (gchar *cConfFilePath);

/**
*Recupere les 3 numeros de version d'une chaine.
*@param cVersionString la version representee par une chaine.
*@param iMajorVersion numero de version majeure renvoyee.
*@param iMinorVersion numero de version mineure renvoyee.
*@param iMicroVersionnumero de version micro renvoyee.
*/
void cairo_dock_get_version_from_string (gchar *cVersionString, int *iMajorVersion, int *iMinorVersion, int *iMicroVersion);

#endif
