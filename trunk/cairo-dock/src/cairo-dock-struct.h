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


typedef gpointer (*CairoDockModuleInit) (cairo_t *pSourceContext, GError **erreur);  // renvoie son icone si il en a.

typedef void (*CairoDockModuleStop) ();

typedef gboolean (*CairoDockModuleConfig) (void);

typedef gboolean (*CairoDockModuleAction) (void);

typedef struct _CairoDockModule {
	gchar *cModuleName;  // le nom du module : libtruc.so => cModuleName = 'truc'.
	gchar *cSoFilePath;  // le chemin du .so.
	GModule *pModule;
	CairoDockModuleInit initModule;
	CairoDockModuleStop stopModule;
	CairoDockModuleConfig configModule;
	CairoDockModuleAction actionModule;
	gboolean bActive;
} CairoDockModule;

/*typedef struct _CairoDockMenuEntry {
	gchar *cLabel;
	gpointer pCallback;
	} CairoDockMenuEntry;*/

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
	gchar* acFileName;
	gchar* acName;
	gchar* acCommand;
	CairoDockIconType iType;
	gdouble fOrder;
	//\____________ calcules lors du chargement de l'icone.
	gdouble fWidth;
	gdouble fHeight;
	cairo_surface_t* pIconBuffer;
	cairo_surface_t* pTextBuffer;
	gdouble fTextXOffset;
	gdouble fTextYOffset;
	gdouble fPhase;
	gdouble fXMax;
	gdouble fXMin;
	//\____________ calcules a chaque fois.
	gdouble fX;
	gdouble fY;
	gdouble fScale;
	gboolean bPointed;
	gint iCount;
	gint iAnimationType;
	gdouble fPersonnalScale;
	//\____________ Pour les fenetres.
	gint iPid;
	Window Xid;
	//\____________ Pour les modules.
	CairoDockModule *pModule;
	//GList *pMenuEntryList;  // une liste de CairoDockMenuEntry*.
	GtkWidget *pMenu;
} Icon;


#define CAIRO_DOCK_LAST_ORDER -1e9


typedef enum {
	CAIRO_DOCK_BOUNCE = 0,
	CAIRO_DOCK_ROTATE,
	CAIRO_DOCK_BLINK,
	CAIRO_DOCK_RANDOM,
	CAIRO_DOCK_NB_ANIMATIONS
	} CairoDockAnimationType;


#endif

