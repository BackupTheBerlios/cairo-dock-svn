/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


******************************************************************************/
#ifndef __CAIRO_DOCK_STRUCT__
#define  __CAIRO_DOCK_STRUCT__

#include <glib.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif


typedef enum {
	CAIRO_DOCK_VERTICAL = 0,
	CAIRO_DOCK_HORIZONTAL
	} CairoDockTypeHorizontality;

typedef struct _CairoDock {
	GList* icons;  // la liste de ses icones.
	GtkWidget *pWidget;  // sa fenetre de dessin.
	gboolean bIsMainDock;  // si le dock est le dock racine.
	gint iRefCount;  // le nombre d'icones pointant sur lui.
	gint iGapX;  // decalage de la zone par rapport au milieu bas de l'ecran.
	gint iGapY;
	gdouble fAlign;  // alignement, entre 0 et 1, du dock sur le bord de l'ecran.
	
	gint iCurrentWidth;  // taille de la fenetre, _apres_ le redimensionnement par GTK.
	gint iCurrentHeight;
	gint iScrollOffset;  // pour faire defiler les icones avec la molette.
	GList *pFirstDrawnElement;  // pointeur sur le 1er element de la liste des icones a etre dessine, en partant de la gauche.
	
	gboolean bAtBottom;  // le dock est en bas au repos.
	gboolean bAtTop;  // le dock est en haut pret a etre utilise.
	gboolean bInside;  // lorsque la souris est dans le dock.
	gboolean bMenuVisible;  // lorsque le menu du clique droit est visible.
	
	gfloat fMagnitude; // coef multiplicateur de l'amplitude de la sinusoide (entre 0 et 1)
	gdouble fDecorationsOffsetX;  // decalage des decorations pour les faire suivre la souris.
	gdouble fLateralFactor;  // un facteur d'acceleration lateral des icones lors du grossissement initial.
	
	CairoDockTypeHorizontality bHorizontalDock;  // dit si le dock est horizontal ou vertical.
	gint iMaxIconHeight;  // max des hauteurs des icones.
	gint iMinDockWidth;  // taille minimale du dock.
	gint iMaxDockWidth;  // taille maximale du dock.
	gint iMaxDockHeight;
	
	gint iWindowPositionX;  // dock-windows current y-position
	gint iWindowPositionY;  // dock-windows current y-position
	gint iSidMoveDown;  // serial ID du thread de descente de la fenetre.
	gint iSidMoveUp;  // serial ID du thread de montee de la fenetre.
	gint iSidGrowUp;  // serial ID du thread de grossisement des icones.
	gint iSidShrinkDown;  // serial ID du thread de diminution des icones.
#ifdef HAVE_GLITZ
	glitz_drawable_format_t *pDrawFormat;
	glitz_drawable_t* pGlitzDrawable;
	glitz_format_t* pGlitzFormat;
#endif // HAVE_GLITZ
	} CairoDock;



typedef gpointer (*CairoDockModuleInit) (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);  // renvoie son icone si il en a.

typedef void (*CairoDockModuleStop) ();

typedef gboolean (*CairoDockModuleConfig) (void);

typedef gboolean (*CairoDockModuleAction) (void);

typedef struct _CairoDockModule {
	gchar *cModuleName;  // le nom du module : libtruc.so => cModuleName = 'truc'.
	gchar *cSoFilePath;  // le chemin du .so.
	GModule *pModule;
	CairoDockModuleInit initModule;
	CairoDockModuleStop stopModule;
	CairoDockModuleAction actionModule;
	gchar *cConfFilePath;
	gboolean bActive;
} CairoDockModule;



typedef enum {
	CAIRO_DOCK_LAUNCHER = 0,
	CAIRO_DOCK_SEPARATOR12,
	CAIRO_DOCK_APPLI,
	CAIRO_DOCK_SEPARATOR23,
	CAIRO_DOCK_APPLET,
	CAIRO_DOCK_NB_TYPES
	} CairoDockIconType;

typedef struct _Icon {
	//\____________ renseignes lors de la creation de l'icone.
	gchar *acDesktopFileName;
	gboolean bIsURI;
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
	gdouble fAlpha;
	gboolean bPointed;
	gint iCount;
	gint iAnimationType;
	gdouble fPersonnalScale;
	//\____________ Pour les fenetres.
	gint iPid;
	Window Xid;
	gboolean bIsMapped;
	//\____________ Pour les modules.
	CairoDockModule *pModule;
	GtkWidget *pMenu;
} Icon;


#define CAIRO_DOCK_LAST_ORDER -1e9

#define CAIRO_DOCK_MAIN_DOCK_NAME "_MainDock_"

#define CAIRO_DOCK_THEMES_DIR "themes"

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
	CAIRO_DOCK_BOUNCE = 0,
	CAIRO_DOCK_ROTATE,
	CAIRO_DOCK_BLINK,
	CAIRO_DOCK_RANDOM,
	CAIRO_DOCK_NB_ANIMATIONS,
	CAIRO_DOCK_FOLLOW_MOUSE,
	CAIRO_DOCK_AVOID_MOUSE
	} CairoDockAnimationType;

typedef void (* CairoDockConfigFunc) (gchar *cConfFile, gpointer data);

typedef void (*CairoDockClickFunc) (Icon *icon);

typedef gchar * (*CairoDockFileManagerFunc) (gchar *cURI, gchar *cDockName, double fOrder, CairoDock *pDock, GError **erreur);



typedef void (*CairoDockCalculateShapeFunc) (CairoDock *pDock, CairoDockSizeType iSizeType, int *iNewWidth, int *iNewHeight);
typedef void (*CairoDockCalculateParametersFunc) (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, gboolean bInside);
typedef void (*CairoDockRender) (CairoDock *pDock);


#endif

