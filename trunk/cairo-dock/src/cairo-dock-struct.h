/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#ifndef __CAIRO_DOCK_STRUCT__
#define  __CAIRO_DOCK_STRUCT__

#include <glib.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

typedef struct _CairoDock CairoDock;
typedef struct _CairoDockModule CairoDockModule;
typedef struct _CairoDockDialog CairoDockDialog;
typedef struct _Icon Icon;


typedef enum {
	CAIRO_DOCK_VERTICAL = 0,
	CAIRO_DOCK_HORIZONTAL
	} CairoDockTypeHorizontality;


typedef void (*CairoDockCalculateMaxDockSizeFunc) (CairoDock *pDock);
typedef Icon * (*CairoDockCalculateIconsFunc) (CairoDock *pDock);
typedef void (*CairoDockRenderFunc) (CairoDock *pDock);
typedef void (*CairoDockRenderOptimizedFunc) (CairoDock *pDock, GdkRectangle *pArea);
typedef void (*CairoDockSetSubDockPositionFunc) (Icon *pPointedIcon, CairoDock *pParentDock);

typedef struct _CairoDockRenderer {
	gchar *cReadmeFilePath;
	CairoDockCalculateMaxDockSizeFunc calculate_max_dock_size;
	CairoDockCalculateIconsFunc calculate_icons;
	CairoDockRenderFunc render;
	CairoDockRenderOptimizedFunc render_optimized;
	CairoDockSetSubDockPositionFunc set_subdock_position;
	} CairoDockRenderer;


struct _CairoDock {
	GList* icons;  // la liste de ses icones.
	GtkWidget *pWidget;  // sa fenetre de dessin.
	gboolean bIsMainDock;  // si le dock est le dock racine.
	gint iRefCount;  // le nombre d'icones pointant sur lui.
	
	//\_______________ Les parametres de position et de geometrie de la fenetre du dock.
	gint iGapX;  // decalage de la zone par rapport au milieu bas de l'ecran.
	gint iGapY;
	gdouble fAlign;  // alignement, entre 0 et 1, du dock sur le bord de l'ecran.
	CairoDockTypeHorizontality bHorizontalDock;  // dit si le dock est horizontal ou vertical.
	
	gint iMaxIconHeight;  // max des hauteurs des icones.
	gint iFlatDockWidth;  // largeur du dock a plat, avec juste les icones.
	gint iMinDockWidth;  // taille du dock au repos.
	gint iMinDockHeight;
	gint iMaxDockWidth;  // taille du dock actif.
	gint iMaxDockHeight;
	gint iDecorationsWidth;  // la taille des decorations.
	gint iDecorationsHeight;
	gint iWindowPositionX;  // dock-windows current y-position
	gint iWindowPositionY;  // dock-windows current y-position
	gint iCurrentWidth;  // taille de la fenetre, _apres_ le redimensionnement par GTK.
	gint iCurrentHeight;
	
	//\_______________ Les parametres lies a une animation du dock.
	gint iScrollOffset;  // pour faire defiler les icones avec la molette.
	gint iMagnitudeIndex; // indice de calcul du coef multiplicateur de l'amplitude de la sinusoide (entre 0 et 1000).
	gdouble fFoldingFactor;  // un facteur d'acceleration lateral des icones lors du grossissement initial.
	gint iMouseX;  // derniere position du curseur (pour l'instant, dans le cas de decorations non asservies).
	gint iMouseY;
	gint iAvoidingMouseIconType;  // type d'icone devant eviter la souris, -1 si aucun.
	gdouble fAvoidingMouseMargin;  // marge d'evitement de la souris, en fraction de la largeur d'une icone (entre 0 et 0.5) 
	
	GList *pFirstDrawnElement;  // pointeur sur le 1er element de la liste des icones a etre dessine, en partant de la gauche.
	gdouble fDecorationsOffsetX;  // decalage des decorations pour les faire suivre la souris.
	
	gboolean bAtBottom;  // le dock est en bas au repos.
	gboolean bAtTop;  // le dock est en haut pret a etre utilise.
	gboolean bInside;  // lorsque la souris est dans le dock.
	gboolean bMenuVisible;  // lorsque le menu du clique droit est visible.
	
	//\_______________ Les ID des threads en cours sur le dock.
	gint iSidMoveDown;  // serial ID du thread de descente de la fenetre.
	gint iSidMoveUp;  // serial ID du thread de montee de la fenetre.
	gint iSidGrowUp;  // serial ID du thread de grossisement des icones.
	gint iSidShrinkDown;  // serial ID du thread de diminution des icones.
	gint iSidLeaveDemand;  // serial ID du thread qui enverra le signal de sortie retarde.
	
	//\_______________ Les fonctions de dessin du dock.
	CairoDockCalculateMaxDockSizeFunc calculate_max_dock_size;
	CairoDockCalculateIconsFunc calculate_icons;
	CairoDockRenderFunc render;
	CairoDockRenderOptimizedFunc render_optimized;
	CairoDockSetSubDockPositionFunc set_subdock_position;
	
#ifdef HAVE_GLITZ
	glitz_drawable_format_t *pDrawFormat;
	glitz_drawable_t* pGlitzDrawable;
	glitz_format_t* pGlitzFormat;
#endif // HAVE_GLITZ
	};


typedef gchar * (* CairoDockModulePreInit) (void);

typedef gpointer (*CairoDockModuleInit) (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);  // renvoie son icone si il en a.

typedef void (*CairoDockModuleStop) ();

struct _CairoDockModule {
	gchar *cModuleName;  // le nom du module : libtruc.so => cModuleName = 'truc'.
	gchar *cSoFilePath;  // le chemin du .so.
	GModule *pModule;
	CairoDockModuleInit initModule;
	CairoDockModuleStop stopModule;
	gchar *cConfFilePath;
	gchar *cReadmeFilePath;
	gboolean bActive;
};

struct _CairoDockDialog
{
	int iWidth;
	int iHeight;
	int iPositionX;
	int iPositionY;
	int iAimedX;
	int iAimedY;
	gboolean bRight;
	gboolean bIsPerpendicular;
	gboolean bDirectionUp;
	double fRadius;
	double fTipHeight;
	cairo_surface_t* pTextBuffer;
	int iTextWidth;
	int iTextHeight;
	GtkWidget *pWidget;
	int iSidTimer;
	int iRefCount;
	gboolean bBuildComplete;
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
	gchar *acDesktopFileName;
	gchar *cBaseURI;
	gint iVolumeID;
	gchar* acFileName;
	gchar* acName;
	gchar* acCommand;
	CairoDockIconType iType;
	gdouble fOrder;
	CairoDock *pSubDock;
	gchar *cParentDockName;
	//\____________ calcules lors du chargement de l'icone.
	gdouble fWidth;
	gdouble fHeight;
	cairo_surface_t* pIconBuffer;
	cairo_surface_t* pTextBuffer;
	int iTextWidth;
	int iTextHeight;
	gdouble fTextXOffset;
	gdouble fTextYOffset;
	gdouble fXMax;
	gdouble fXMin;
	//\____________ calcules a chaque scroll et insertion/suppression d'une icone.
	gdouble fXAtRest;
	//\____________ calcules a chaque fois.
	gdouble fPhase;
	gdouble fX;
	gdouble fY;
	gdouble fScale;
	gdouble fDrawX;
	gdouble fDrawY;
	gdouble fWidthFactor;
	gdouble fHeightFactor;
	gdouble fAlpha;
	gboolean bPointed;
	gint iCount;
	CairoDockAnimationType iAnimationType;
	gdouble fPersonnalScale;
	//\____________ Pour les applis.
	gint iPid;
	Window Xid;
	gchar *cClass;
	double fLastCheckTime;
	//\____________ Pour les modules.
	CairoDockModule *pModule;
	//\____________ Pour les bulles de dialogues.
	CairoDockDialog *pDialog;
};



#define CAIRO_DOCK_DATA_DIR ".cairo-dock"
#define CAIRO_DOCK_THEMES_DIR "themes"
#define CAIRO_DOCK_CURRENT_THEME_NAME "current_theme"
#define CAIRO_DOCK_LAUNCHERS_DIR "launchers"

#define CAIRO_DOCK_MAIN_DOCK_NAME "_MainDock_"

#define CAIRO_DOCK_DEFAULT_RENDERER_NAME "default"


#define CAIRO_DOCK_LAST_ORDER -1e9
#define CAIRO_DOCK_NB_MAX_ITERATIONS 1000

#define CAIRO_DOCK_UPDATE_DOCK_SIZE TRUE
#define CAIRO_DOCK_ANIMATE_ICON TRUE
#define CAIRO_DOCK_APPLY_RATIO TRUE

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

typedef void (* CairoDockConfigFunc) (gchar *cConfFile, gpointer data);

typedef enum {
	CAIRO_DOCK_UNKNOWN_ENV=0,
	CAIRO_DOCK_GNOME,
	CAIRO_DOCK_KDE
	} CairoDockDesktopEnv;

#endif
