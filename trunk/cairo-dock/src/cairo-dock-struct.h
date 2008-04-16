/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#ifndef __CAIRO_DOCK_STRUCT__
#define  __CAIRO_DOCK_STRUCT__

#include <glib.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include <glib/gi18n.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

typedef struct _CairoDockRenderer CairoDockRenderer;
typedef struct _CairoDockDeskletRenderer CairoDockDeskletRenderer;
typedef struct _CairoDockDialogRenderer CairoDockDialogRenderer;

typedef struct _Icon Icon;
typedef struct _CairoDockContainer CairoDockContainer;
typedef struct _CairoDock CairoDock;
typedef struct _CairoDockDesklet CairoDockDesklet;
typedef struct _CairoDockDialog CairoDockDialog;

typedef struct _CairoDockModule CairoDockModule;
typedef struct _CairoDockVisitCard CairoDockVisitCard;
typedef struct _CairoDockMinimalAppletConfig CairoDockMinimalAppletConfig;
typedef struct _CairoDockVFSBackend CairoDockVFSBackend;

typedef struct _CairoDockClassAppli CairoDockClassAppli;

typedef void (*CairoDockCalculateMaxDockSizeFunc) (CairoDock *pDock);
typedef Icon * (*CairoDockCalculateIconsFunc) (CairoDock *pDock);
typedef void (*CairoDockRenderFunc) (CairoDock *pDock);
typedef void (*CairoDockRenderOptimizedFunc) (CairoDock *pDock, GdkRectangle *pArea);
typedef void (*CairoDockSetSubDockPositionFunc) (Icon *pPointedIcon, CairoDock *pParentDock);

struct _CairoDockRenderer {
	/// chemin d'un fichier readme destine a presenter de maniere succinte la vue.
	gchar *cReadmeFilePath;
	/// fonction calculant la taille max d'un dock.
	CairoDockCalculateMaxDockSizeFunc calculate_max_dock_size;
	/// fonction calculant l'ensemble des parametres des icones.
	CairoDockCalculateIconsFunc calculate_icons;
	/// fonction de rendu.
	CairoDockRenderFunc render;
	/// fonction de rendu optimise, ne dessinant qu'une seule icone.
	CairoDockRenderOptimizedFunc render_optimized;
	/// fonction calculant la position d'un sous-dock.
	CairoDockSetSubDockPositionFunc set_subdock_position;
	/// TRUE ssi cette vue utilise les reflets.
	gboolean bUseReflect;
	/// chemin d'une image de previsualisation.
	gchar *cPreviewFilePath;
	};



typedef enum {
	CAIRO_DOCK_VERTICAL = 0,
	CAIRO_DOCK_HORIZONTAL
	} CairoDockTypeHorizontality;

typedef enum {
	CAIRO_DOCK_TYPE_DOCK = 0,
	CAIRO_DOCK_TYPE_DESKLET,
	CAIRO_DOCK_TYPE_DIALOG
	} CairoDockTypeContainer;

struct _CairoDockContainer {
	/// type de container.
	CairoDockTypeContainer iType;
	/// La fenetre du widget.
	GtkWidget *pWidget;
	/// Taille de la fenetre. La surface allouee a l'applet s'en deduit.
	gint iWidth, iHeight;
	/// Position de la fenetre.
	int iWindowPositionX, iWindowPositionY;
	/// Vrai ssi le pointeur est dans le desklet (widgets fils inclus).
	gboolean bInside;
	/// TRUE ssi le container est horizontal.
	CairoDockTypeHorizontality bIsHorizontal;
#ifdef HAVE_GLITZ
	glitz_drawable_format_t *pDrawFormat;
	glitz_drawable_t* pGlitzDrawable;
	glitz_format_t* pGlitzFormat;
#else
	gpointer padding[3];
#endif // HAVE_GLITZ
};

#define CAIRO_DOCK_CONTAINER(p) ((CairoDockContainer *) (p))


struct _CairoDock {
	/// type "dock".
	CairoDockTypeContainer iType;
	/// sa fenetre de dessin.
	GtkWidget *pWidget;
	/// largeur de la fenetre, _apres_ le redimensionnement par GTK.
	gint iCurrentWidth;
	/// hauteur de la fenetre, _apres_ le redimensionnement par GTK.
	gint iCurrentHeight;
	/// position courante en X du coin haut gauche de la fenetre sur l'ecran.
	gint iWindowPositionX;
	/// position courante en Y du coin haut gauche de la fenetre sur l'ecran.
	gint iWindowPositionY;
	/// lorsque la souris est dans le dock.
	gboolean bInside;
	/// dit si le dock est horizontal ou vertical.
	CairoDockTypeHorizontality bHorizontalDock;
#ifdef HAVE_GLITZ
	glitz_drawable_format_t *pDrawFormat;
	glitz_drawable_t* pGlitzDrawable;
	glitz_format_t* pGlitzFormat;
#else
	gpointer padding[3];
#endif // HAVE_GLITZ

	/// la liste de ses icones.
	GList* icons;
	/// si le dock est le dock racine.
	gboolean bIsMainDock;
	/// le nombre d'icones pointant sur lui.
	gint iRefCount;

	//\_______________ Les parametres de position et de geometrie de la fenetre du dock.
	/// ecart de la fenetre par rapport au bord de l'ecran.
	gint iGapX;
	/// decalage de la fenetre par rapport au point d'alignement sur le bord de l'ecran.
	gint iGapY;
	/// alignement, entre 0 et 1, du dock sur le bord de l'ecran.
	gdouble fAlign;
	
	/// magnitude maximale de la vague.
	gdouble fMagnitudeMax;
	/// max des hauteurs des icones.
	gdouble iMaxIconHeight;
	/// largeur du dock a plat, avec juste les icones.
	gdouble fFlatDockWidth;
	/// largeur du dock au repos.
	gint iMinDockWidth;
	/// hauteur du dock au repos.
	gint iMinDockHeight;
	/// largeur du dock actif.
	gint iMaxDockWidth;
	/// hauteur du dock actif.
	gint iMaxDockHeight;
	/// largeur des decorations.
	gint iDecorationsWidth;
	/// hauteur des decorations.
	gint iDecorationsHeight;
	/// ratio applique aux icones.
	gdouble fRatio;

	gint iMaxLabelWidth;
	gint iRightMargin;
	gint iTopMargin;

	//\_______________ Les parametres lies a une animation du dock.
	/// pour faire defiler les icones avec la molette.
	gint iScrollOffset;
	/// indice de calcul du coef multiplicateur de l'amplitude de la sinusoide (entre 0 et CAIRO_DOCK_NB_MAX_ITERATIONS).
	gint iMagnitudeIndex;
	/// un facteur d'acceleration lateral des icones lors du grossissement initial.
	gdouble fFoldingFactor;
	/// derniere position en X du curseur dans le referentiel du dock.
	gint iMouseX;
	/// derniere position en Y du curseur dans le referentiel du dock.
	gint iMouseY;
	/// type d'icone devant eviter la souris, -1 si aucun.
	gint iAvoidingMouseIconType;
	/// marge d'evitement de la souris, en fraction de la largeur d'une icone (entre 0 et 0.5)
	gdouble fAvoidingMouseMargin;

	/// pointeur sur le 1er element de la liste des icones a etre dessine, en partant de la gauche.
	GList *pFirstDrawnElement;
	/// decalage des decorations pour les faire suivre la souris.
	gdouble fDecorationsOffsetX;

	/// le dock est en bas au repos.
	gboolean bAtBottom;
	/// le dock est en haut pret a etre utilise.
	gboolean bAtTop;
	/// lorsque le menu du clique droit est visible.
	gboolean bMenuVisible;
	/// Est-on en train de survoler le dock avec quelque chose dans la souris ?
	gboolean bIsDragging;

	//\_______________ Les ID des threads en cours sur le dock.
	/// serial ID du thread de descente de la fenetre.
	gint iSidMoveDown;
	/// serial ID du thread de montee de la fenetre.
	gint iSidMoveUp;
	/// serial ID du thread de grossisement des icones.
	gint iSidGrowUp;
	/// serial ID du thread de diminution des icones.
	gint iSidShrinkDown;
	/// serial ID du thread qui enverra le signal de sortie retarde.
	gint iSidLeaveDemand;

	//\_______________ Les fonctions de dessin du dock.
	/// nom de la vue, utile pour charger les fonctions de rendu posterieurement a la creation du dock.
	gchar *cRendererName;
	/// recalculer la taille maximale du dock.
	CairoDockCalculateMaxDockSizeFunc calculate_max_dock_size;
	/// calculer tous les parametres des icones.
	CairoDockCalculateIconsFunc calculate_icons;
	/// dessiner le tout.
	CairoDockRenderFunc render;
	/// dessiner une portion du dock de maniere optimisee.
	CairoDockRenderOptimizedFunc render_optimized;
	/// calculer la position d'un sous-dock.
	CairoDockSetSubDockPositionFunc set_subdock_position;
	/// dit si la vue courante utilise les reflets ou pas (utile pour les plug-ins).
	gboolean bUseReflect;
};


typedef enum {
	CAIRO_DOCK_CATEGORY_ACCESSORY=0,
	CAIRO_DOCK_CATEGORY_DESKTOP,
	CAIRO_DOCK_CATEGORY_CONTROLER,
	CAIRO_DOCK_NB_CATEGORY
	} CairoDockPluginCategory;

struct _CairoDockVisitCard {
	/// nom du module qui servira a l'identifier.
	gchar *cModuleName;
	/// chemin d'un fichier readme destine a presenter de maniere succinte le module.
	gchar *cReadmeFilePath;
	/// numero de version majeure de cairo-dock necessaire au bon fonctionnement du module.
	short iMajorVersionNeeded;
	/// numero de version mineure de cairo-dock necessaire au bon fonctionnement du module.
	short iMinorVersionNeeded;
	/// numero de version micro de cairo-dock necessaire au bon fonctionnement du module.
	short iMicroVersionNeeded;
	/// chemin d'une image de previsualisation.
	gchar *cPreviewFilePath;
	/// Nom du domaine pour la traduction du module par 'gettext'.
	gchar *cGettextDomain;
	/// Version du dock pour laquelle a ete compilee le module.
	gchar *cDockVersionOnCompilation;
	/// version courante du module.
	gchar *cModuleVersion;
	/// repertoire du plug-in cote utilisateur.
	gchar *cUserDataDir;
	/// repertoire d'installation du plug-in.
	gchar *cShareDataDir;
	/// nom de son fichier de conf.
	gchar *cConfFileName;
	/// categorie de l'applet.
	CairoDockPluginCategory iCategory;
	/// chemin d'une image pour l'icone du module dans le panneau de conf du dock.
	gchar *cIconFilePath;
	/// octets reserves pour preserver la compatibilite binaire lors de futurs ajouts sur l'interface entre plug-ins et dock.
	char reserved[40];
};

/// Construit et renvoie la carte de visite du module.
typedef CairoDockVisitCard * (* CairoDockModulePreInit) (void);

/// Initialise le module, et renvoie son icone si il en a.
typedef void (*CairoDockModuleInit) (GKeyFile *pKeyFile, Icon *pIcon, CairoDockContainer *pContainer, gchar *cConfFilePath, GError **erreur);

/// Stoppe le module et libere toutes les ressources allouees par lui.
typedef void (*CairoDockModuleStop) (void);

/// Recharge le module (optionnel).
typedef gboolean (*CairoDockModuleReload) (GKeyFile *pKeyFile, gchar *cConfFileToReload, CairoDockContainer *pContainer);

struct _CairoDockModule {
	/// chemin du .so
	gchar *cSoFilePath;
	/// structure du module, contenant les pointeurs vers les fonctions du module.
	GModule *pModule;
	/// fonction d'initialisation du module.
	CairoDockModuleInit initModule;
	/// fonction d'arret du module.
	CairoDockModuleStop stopModule;
	/// fonction de configuration du module. Appelee a chaque reconfiguration du module.
	CairoDockModuleReload reloadModule;
	/// carte de visite du module.
	CairoDockVisitCard *pVisitCard;
	/// chemin du fichier de conf du module.
	gchar *cConfFilePath;
	/// TRUE si le module est actif (c'est-a-dire utilise).
	gboolean bActive;
	/// VRAI ssi l'appet est prevue pour pouvoir se detacher.
	gboolean bCanDetach;
	/// le container dans lequel va se charger le module, ou NULL.
	CairoDockContainer *pContainer;
	/// Heure de derniere (re)activation du module.
	double fLastLoadingTime;
};

struct _CairoDockMinimalAppletConfig {
	gint iDesiredIconWidth;
	gint iDesiredIconHeight;
	gchar *cLabel;
	gchar *cIconFileName;
	gdouble fOrder;
	gboolean bDeskletUseSize;
	gint iDeskletWidth;
	gint iDeskletHeight;
	gint iDeskletPositionX;
	gint iDeskletPositionY;
	gboolean bIsDetached;
	gboolean bKeepBelow;
	gboolean bKeepAbove;
	gboolean bOnWidgetLayer;
};


typedef void (* CairoDockActionOnAnswerFunc) (int iAnswer, GtkWidget *pWidget, gpointer data);
typedef void (* CairoDockDialogRenderFunc) (cairo_t *pCairoContext, CairoDockDialog *pDialog, gboolean bRenderOptimized);
typedef gpointer (* CairoDockDialogLoadRendererFunc) (CairoDockDialog *pDialog, cairo_t *pSourceContext, gpointer *pConfig);
typedef void (* CairoDockDialogFreeRendererDataFunc) (CairoDockDialog *pDesklet);
struct _CairoDockDialogRenderer {
	CairoDockDialogRenderFunc render;
	CairoDockDialogLoadRendererFunc load_data;
	CairoDockDialogFreeRendererDataFunc free_data;
};

struct _CairoDockDialog {
	/// type de container.
	CairoDockTypeContainer iType;
	/// la fenetre GTK du dialogue.
	GtkWidget *pWidget;
	/// largeur de la fenetre GTK du dialogue (pointe comprise).
	int iWidth;
	/// hauteur de la fenetre GTK du dialogue (pointe comprise).
	int iHeight;
	/// position en X du coin haut gauche de la fenetre GTK du dialogue.
	int iPositionX;
	/// position en Y du coin haut gauche de la fenetre GTK du dialogue.
	int iPositionY;
	/// vrai ssi la souris est dans le dialogue, auquel cas on le garde immobile.
	gboolean bInside;
	/// FALSE ssi le dialogue est perpendiculaire au dock.
	CairoDockTypeHorizontality bIsHorizontal;
#ifdef HAVE_GLITZ
	glitz_drawable_format_t *pDrawFormat;
	glitz_drawable_t* pGlitzDrawable;
	glitz_format_t* pGlitzFormat;
#else
	gpointer padding[3];
#endif // HAVE_GLITZ
	/// le moteur de rendu utilise pour dessiner le dialogue.
	CairoDockDialogRenderer *pRenderer;
	/// donnees pouvant etre utilisees par le moteur de rendu.
	gpointer pRendererData;
	/// position en X visee par la pointe dans le référentiel de l'écran.
	int iAimedX;
	/// position en Y visee par la pointe dans le référentiel de l'écran.
	int iAimedY;
	/// TRUE ssi le dialogue est a droite de l'écran; dialogue a droite <=> pointe a gauche.
	gboolean bRight;
	/// TRUE ssi la pointe est orientée vers le bas.
	gboolean bDirectionUp;
	/// rayon des coins.
	double fRadius;
	/// hauteur de la pointe, sans la partie "aiguisee".
	double fTipHeight;
	/// surface representant le message + l'icone dans la marge a gauche du texte.
	cairo_surface_t* pTextBuffer;
	/// dimensions de la bulle (message + widget utilisateur + boutons).
	int iBubbleWidth, iBubbleHeight;
	/// dimensions du message en comptant la marge du texte + vgap en bas si necessaire.
	int iMessageWidth, iMessageHeight;
	/// dimensions des boutons + vgap en haut.
	int iButtonsWidth, iButtonsHeight;
	/// dimensions du widget interactif.
	int iInteractiveWidth, iInteractiveHeight;
	/// distance de la bulle au dock, donc hauteur totale de la pointe.
	int iDistanceToDock;
	/// Marge due au rayon.
	int iMargin;
	/// decalage pour l'effet de clique sur le bouton Ok.
	int iButtonOkOffset;
	/// decalage pour l'effet de clique sur le bouton Annuler.
	int iButtonCancelOffset;
	/// le widget de remplissage ou l'on dessine le message.
	GtkWidget *pMessageWidget;
	/// le widget de remplissage ou l'on dessine les boutons.
	GtkWidget *pButtonsWidget;
	/// le widget de remplissage ou l'on dessine la pointe.
	GtkWidget *pTipWidget;
	/// le timer pour la destruction automatique du dialogue.
	int iSidTimer;
	/// reference atomique.
	int iRefCount;
	/// le widget d'interaction utilisateur (GtkEntry, GtkHScale, etc).
	GtkWidget *pInteractiveWidget;
	/// le type des boutons (GTK_BUTTONS_NONE, GTK_BUTTONS_OK_CANCEL ou GTK_BUTTONS_YES_NO).
	int iButtonsType;
	/// fonction appelee au clique sur l'un des 2 boutons.
	CairoDockActionOnAnswerFunc action_on_answer;
	/// donnees transmises a la fonction.
	gpointer pUserData;
	/// fonction appelee pour liberer les donnees.
	GFreeFunc pFreeUserDataFunc;
	/// icone sur laquelle pointe le dialogue.
	Icon *pIcon;
};


typedef enum {
	CAIRO_DOCK_LAUNCHER = 0,
	CAIRO_DOCK_SEPARATOR12,
	CAIRO_DOCK_APPLI,
	CAIRO_DOCK_SEPARATOR23,
	CAIRO_DOCK_APPLET,
	CAIRO_DOCK_NB_TYPES
	} CairoDockIconType;

typedef enum {
	CAIRO_DOCK_BOUNCE = 0,
	CAIRO_DOCK_ROTATE,
	CAIRO_DOCK_BLINK,
	CAIRO_DOCK_PULSE,
	CAIRO_DOCK_UPSIDE_DOWN,
	CAIRO_DOCK_WOBBLY,
	CAIRO_DOCK_RANDOM,
	CAIRO_DOCK_NB_ANIMATIONS,
	CAIRO_DOCK_FOLLOW_MOUSE,
	CAIRO_DOCK_AVOID_MOUSE
	} CairoDockAnimationType;

struct _Icon {
	//\____________ renseignes lors de la creation de l'icone.
	/// Nom (et non pas chemin) du fichier .desktop definissant l'icone, ou NULL si l'icone n'est pas definie pas un fichier.
	gchar *acDesktopFileName;
	/// URI.
	gchar *cBaseURI;
	/// ID d'un volume.
	gint iVolumeID;
	/// Nom (et non pas chemin) du fichier de l'image, ou NULL si son image n'est pas definie pas un fichier.
	gchar *acFileName;
	/// Nom de l'icone tel qu'il apparaitra dans son etiquette. Donne le nom au sous-dock.
	gchar *acName;
	/// Commande a executer lors d'un clique gauche clique, ou NULL si aucune.
	gchar *acCommand;
	/// Repertoire ou s'executera la commande.
	gchar *cWorkingDirectory;
	/// Type de l'icone.
	CairoDockIconType iType;
	/// Ordre de l'icone dans son dock, parmi les icones de meme type.
	gdouble fOrder;
	/// Sous-dock sur lequel pointe l'icone, ou NULL si aucun.
	CairoDock *pSubDock;
	/// Nom du dock contenant l'icone (y compris lorsque l'icone est dans un desklet).
	gchar *cParentDockName;
	//\____________ calcules lors du chargement de l'icone.
	/// Largeur de l'image de l'icone.
	gdouble fWidth;
	/// Hauteur de l'image de l'icone.
	gdouble fHeight;
	/// Surface cairo de l'image.
	cairo_surface_t* pIconBuffer;
	/// Surface cairo de l'etiquette.
	cairo_surface_t* pTextBuffer;
	/// Surface cairo du reflet.
	cairo_surface_t* pReflectionBuffer;
	/// Surface cairo de l'image et de son reflet.
	cairo_surface_t* pFullIconBuffer;
	/// Largeur de l'etiquette.
	int iTextWidth;
	/// Hauteur de l'etiquette.
	int iTextHeight;
	/// Decalage en X de l'etiquette.
	gdouble fTextXOffset;
	/// Decalage en Y de l'etiquette.
	gdouble fTextYOffset;
	/// Abscisse maximale (droite) que l'icone atteindra (variable avec la vague).
	gdouble fXMax;
	/// Abscisse minimale (gauche) que l'icone atteindra (variable avec la vague).
	gdouble fXMin;
	//\____________ calcules a chaque scroll et insertion/suppression d'une icone.
	/// Abscisse de l'icone au repos.
	gdouble fXAtRest;
	//\____________ calcules a chaque fois.
	/// Phase de l'icone (entre -pi et pi).
	gdouble fPhase;
	/// Abscisse temporaire du bord gauche de l'image de l'icone.
	gdouble fX;
	/// Ordonnee temporaire du bord haut de l'image de l'icone.
	gdouble fY;
	/// Echelle courante de l'icone (facteur de zoom, >= 1).
	gdouble fScale;
	/// Abscisse du bord gauche de l'image de l'icone.
	gdouble fDrawX;
	/// Ordonnee du bord haut de l'image de l'icone.
	gdouble fDrawY;
	/// Facteur de zoom sur la largeur de l'icone.
	gdouble fWidthFactor;
	/// Facteur de zoom sur la hauteur de l'icone.
	gdouble fHeightFactor;
	/// Transparence (<= 1).
	gdouble fAlpha;
	/// TRUE ssi l'icone est couramment pointee.
	gboolean bPointed;
	/// Compteur de l'animation de l'icone (> 0 si une animation est en cours, 0 sinon).
	gint iCount;
	/// Type de l'animation.
	CairoDockAnimationType iAnimationType;
	/// Facteur de zoom personnel, utilise pour l'apparition et la suppression des icones.
	gdouble fPersonnalScale;
	/// Decalage en ordonnees de reflet (pour le rebond, >= 0).
	gdouble fDeltaYReflection;
	/// Orientation de l'icone (angle par rapport a la verticale).
	gdouble fOrientation;
	//\____________ Pour les applis.
	/// PID de l'application correspondante.
	gint iPid;
	/// ID de la fenetre X de l'application correspondante.
	Window Xid;
	/// Classe de l'application correspondante (ou NULL si aucune).
	gchar *cClass;
	/// Heure de derniere verification de la presence de l'application dans la barre des taches.
	double fLastCheckTime;
	/// TRUE ssi la fenetre de l'application correspondante est minimisee.
	gboolean bIsHidden;
	/// Position et taille de la fenetre.
	GtkAllocation windowGeometry;
	//\____________ Pour les modules.
	/// Module que represente l'icone.
	CairoDockModule *pModule;
	/// Texte de l'info rapide.
	gchar *cQuickInfo;
	/// Surface cairo de l'info rapide.
	cairo_surface_t* pQuickInfoBuffer;
	/// Largeur de l'info rapide.
	int iQuickInfoWidth;
	/// Heuteur de l'info rapide.
	int iQuickInfoHeight;
	/// Decalage en X de la surface de l'info rapide.
	double fQuickInfoXOffset;
	/// Decalage en Y de la surface de l'info rapide.
	double fQuickInfoYOffset;
	/// TRUE ssi l'icone a un indicateur (elle controle une appli).
	gboolean bHasIndicator;
};


typedef enum {
	CAIRO_DOCK_FILE_MODIFIED=0,
	CAIRO_DOCK_FILE_DELETED,
	CAIRO_DOCK_FILE_CREATED,
	CAIRO_DOCK_NB_EVENT_ON_FILES
	} CairoDockFMEventType;

typedef enum {
	CAIRO_DOCK_FM_SORT_BY_NAME=0,
	CAIRO_DOCK_FM_SORT_BY_DATE,
	CAIRO_DOCK_FM_SORT_BY_SIZE,
	CAIRO_DOCK_FM_SORT_BY_TYPE,
	CAIRO_DOCK_NB_SORT_ON_FILE
	} CairoDockFMSortType;

typedef void (*CairoDockFMGetFileInfoFunc) (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType);
typedef void (*CairoDockFMFilePropertiesFunc) (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask);
typedef GList * (*CairoDockFMListDirectoryFunc) (const gchar *cURI, CairoDockFMSortType g_fm_iSortType, int iNewIconsType, gboolean bListHiddenFiles, gchar **cFullURI);
typedef void (*CairoDockFMLaunchUriFunc) (const gchar *cURI);

typedef gchar * (*CairoDockFMIsMountedFunc) (const gchar *cURI, gboolean *bIsMounted);
typedef void (*CairoDockFMMountCallback) (gboolean bMounting, gboolean bSuccess, const gchar *cName, Icon *icon, CairoDock *pDock);
typedef void (*CairoDockFMMountFunc) (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDock);
typedef void (*CairoDockFMUnmountFunc) (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoDock *pDockCairoDock);

typedef void (*CairoDockFMMonitorCallback) (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data);
typedef void (*CairoDockFMAddMonitorFunc) (const gchar *cURI, gboolean bDirectory, CairoDockFMMonitorCallback pCallback, gpointer data);
typedef void (*CairoDockFMRemoveMonitorFunc) (const gchar *cURI);

typedef gboolean (*CairoDockFMDeleteFileFunc) (const gchar *cURI);
typedef gboolean (*CairoDockFMRenameFileFunc) (const gchar *cOldURI, const gchar *cNewName);
typedef gboolean (*CairoDockFMMoveFileFunc) (const gchar *cURI, const gchar *cDirectoryURI);

typedef gchar * (*CairoDockFMGetTrashFunc) (const gchar *cNearURI, gboolean bCreateIfNecessary);
typedef gchar * (*CairoDockFMGetDesktopFunc) (void);
typedef void (*CairoDockFMUserActionFunc) (void);

struct _CairoDockVFSBackend {
	CairoDockFMGetFileInfoFunc 		get_file_info;
	CairoDockFMFilePropertiesFunc 	get_file_properties;
	CairoDockFMListDirectoryFunc 	list_directory;
	CairoDockFMLaunchUriFunc 		launch_uri;
	CairoDockFMIsMountedFunc 		is_mounted;
	CairoDockFMMountFunc 			mount;
	CairoDockFMUnmountFunc 		unmount;
	CairoDockFMAddMonitorFunc 		add_monitor;
	CairoDockFMRemoveMonitorFunc 	remove_monitor;
	CairoDockFMDeleteFileFunc 		delete;
	CairoDockFMRenameFileFunc 		rename;
	CairoDockFMMoveFileFunc 		move;
	CairoDockFMGetTrashFunc 		get_trash_path;
	CairoDockFMGetDesktopFunc 	get_desktop_path;
	CairoDockFMUserActionFunc		logout;
	CairoDockFMUserActionFunc		setup_time;
};


typedef void (* CairoDockDeskletRenderFunc) (cairo_t *pCairoContext, CairoDockDesklet *pDesklet, gboolean bRenderOptimized);
typedef gpointer (* CairoDockDeskletLoadRendererFunc) (CairoDockDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);
typedef void (* CairoDockDeskletFreeRendererDataFunc) (CairoDockDesklet *pDesklet);
typedef void (* CairoDockDeskletLoadIconsFunc) (CairoDockDesklet *pDesklet, cairo_t *pSourceContext);
struct _CairoDockDeskletRenderer {
	CairoDockDeskletRenderFunc render;
	CairoDockDeskletLoadRendererFunc load_data;
	CairoDockDeskletFreeRendererDataFunc free_data;
	CairoDockDeskletLoadIconsFunc load_icons;
};

struct _CairoDockDesklet {
	/// type "desklet".
	CairoDockTypeContainer iType;
	/// La fenetre du widget.
	GtkWidget *pWidget;
	/// Taille de la fenetre. La surface allouee a l'applet s'en deduit.
	gint iWidth, iHeight;
	/// Position de la fenetre.
	int iWindowPositionX, iWindowPositionY;
	/// Vrai ssi le pointeur est dans le desklet (widgets fils inclus).
	gboolean bInside;
	/// Toujours vrai pour un desklet.
	CairoDockTypeHorizontality bIsHorizontal;
#ifdef HAVE_GLITZ
	glitz_drawable_format_t *pDrawFormat;
	glitz_drawable_t* pGlitzDrawable;
	glitz_format_t* pGlitzFormat;
#else
	gpointer padding[3];
#endif // HAVE_GLITZ
	/// le moteur de rendu utilise pour dessiner le desklet.
	CairoDockDeskletRenderer *pRenderer;
	/// donnees pouvant etre utilisees par le moteur de rendu.
	gpointer pRendererData;
	/// Liste eventuelle d'icones placees sur le desklet, et susceptibles de recevoir des clics.
	GList *icons;
	/// pour le deplacement manuel de la fenetre.
	gint diff_x, diff_y;
	gboolean moving;
	/// L'icone de l'applet.
	Icon *pIcon;
	/// un timer pour retarder l'ecriture dans le fichier lors des deplacements.
	gint iSidWritePosition;
	/// un timer pour retarder l'ecriture dans le fichier lors des redimensionnements.
	gint iSidWriteSize;
	/// compteur pour le fondu lors de l'entree dans le desklet.
	gint iGradationCount;
	/// timer associe.
	gint iSidGradationOnEnter;
};


#define CAIRO_DOCK_FM_VFS_ROOT "_vfsroot_"
#define CAIRO_DOCK_FM_NETWORK "_network_"
#define CAIRO_DOCK_FM_VFS_ROOT_NETWORK "_vfsroot+network_"


typedef gboolean (* CairoDockForeachDeskletFunc) (CairoDockDesklet *pDesklet, CairoDockModule *pModule, gpointer data);

typedef void (* CairoDockForeachIconFunc) (Icon *icon, gpointer data);

typedef void (* CairoDockConfigFunc) (gchar *cConfFile, gpointer data);


struct _CairoDockClassAppli {
	/// TRUE ssi l'appli doit utiliser l'icone fournie par X au lieu de celle du theme.
	gboolean bUseXIcon;
	/// Liste des inhibiteurs de la classe.
	GList *pIconsOfClass;
	/// Liste des icones d'appli de cette classe.
	GList *pAppliOfClass;
};


/// Nom du repertoire de travail de cairo-dock.
#define CAIRO_DOCK_DATA_DIR ".cairo-dock"
/// Nom du repertoire des themes.
#define CAIRO_DOCK_THEMES_DIR "themes"
/// Nom du repertoire du theme courant.
#define CAIRO_DOCK_CURRENT_THEME_NAME "current_theme"
/// Nom du repertoire des lanceurs et leurs icones.
#define CAIRO_DOCK_LAUNCHERS_DIR "launchers"
/// Nom du dock principal (le 1er cree).
#define CAIRO_DOCK_MAIN_DOCK_NAME "_MainDock_"
/// Nom de la vue par defaut.
#define CAIRO_DOCK_DEFAULT_RENDERER_NAME "default"


#define CAIRO_DOCK_LAST_ORDER -1e9
#define CAIRO_DOCK_NB_MAX_ITERATIONS 1000

#define CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET TRUE

#define CAIRO_DOCK_UPDATE_DOCK_SIZE TRUE
#define CAIRO_DOCK_ANIMATE_ICON TRUE
#define CAIRO_DOCK_APPLY_RATIO TRUE
#define CAIRO_DOCK_INSERT_SEPARATOR TRUE

#ifdef CAIRO_DOCK_VERBOSE
#define CAIRO_DOCK_MESSAGE if (g_bVerbose) g_message
#else
#define CAIRO_DOCK_MESSAGE(s, ...)
#endif


typedef enum {
	CAIRO_DOCK_MAX_SIZE,
	CAIRO_DOCK_NORMAL_SIZE,
	CAIRO_DOCK_MIN_SIZE
	} CairoDockSizeType;

typedef enum {
	CAIRO_DOCK_MOUSE_INSIDE,
	CAIRO_DOCK_MOUSE_ON_THE_EDGE,
	CAIRO_DOCK_MOUSE_OUTSIDE
	} CairoDockMousePositionType;

typedef enum {
	CAIRO_DOCK_UNKNOWN_ENV=0,
	CAIRO_DOCK_GNOME,
	CAIRO_DOCK_KDE,
	CAIRO_DOCK_XFCE
	} CairoDockDesktopEnv;

typedef enum
{
	CAIRO_DOCK_BOTTOM = 0,
	CAIRO_DOCK_TOP,
	CAIRO_DOCK_LEFT,
	CAIRO_DOCK_RIGHT,
	CAIRO_DOCK_NB_POSITIONS
	} CairoDockPositionType;

typedef enum
{
	CAIRO_DOCK_LAUNCHER_FROM_DESKTOP_FILE = 0,
	CAIRO_DOCK_LAUNCHER_FOR_CONTAINER,
	CAIRO_DOCK_LAUNCHER_FOR_SEPARATOR,
	CAIRO_DOCK_NB_NEW_LAUNCHER_TYPE
	} CairoDockNewLauncherType;

#endif
