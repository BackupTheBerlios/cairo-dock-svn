#ifndef __CAIRO_DOCK_GLOBAL_VARIABLES_H__
#define __CAIRO_DOCK_GLOBAL_VARIABLES_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <cairo.h>

/// pointeur sur le dock principal.
extern CairoDock *g_pMainDock;
/// table des docks existant.
extern GHashTable *g_hDocksTable;
extern gchar *g_cLanguage;
/// largeur de l'ecran en mode horizontal/vertical.
extern gint g_iScreenWidth[2];
/// hauteur/largeur de l'ecran en mode horizontal/vertical.
extern gint g_iScreenHeight[2];

/// epaisseur du cadre (en pixels).
extern gint g_iDockLineWidth;
/// rayon des coins du cadre.
extern gint g_iDockRadius;
/// marge entre le cadre et les icones.
gint g_iFrameMargin;
/// la couleur du cadre.
extern double g_fLineColor[4];
/// ecart en pixels entre les icones.
extern int g_iIconGap;
/// taille de la police des etiquettes.
extern int g_iLabelSize;
/// police de caracteres des etiquettes.
extern gchar *g_cLabelPolice;
/// epaisseur des traits.
extern int g_iLabelWeight;
/// italique ou droit.
extern int g_iLabelStyle;

/// vrai ssi les coins du bas sont arrondis.
extern gboolean g_bRoundedBottomCorner;
/// TRUE si l'auto-hide est active.
extern gboolean g_bAutoHide;

/// taille des reflets, en pixels, calcules par rapport a la hauteur max des icones.
extern double g_fReflectSize;
/// pouvoir reflechissant du plan.
extern double g_fAlbedo;

/// couleur claire du fond ou des rayures.
extern double g_fStripesColorBright[4];
/// couleur foncee du fond ou des rayures.
extern double g_fStripesColorDark[4];

/// largeur de la zone de rappel.
extern int g_iVisibleZoneWidth;
/// hauteur de la zone de rappel.
extern int g_iVisibleZoneHeight;

/// le theme d'icone choisi.
extern GtkIconTheme *g_pIconTheme;
/// le chemin vers le repertoire racine.
extern gchar *g_cCairoDockDataDir;
/// le chemin vers le repertoire du theme courant.
extern gchar *g_cCurrentThemePath;
/// le chemin vers le repertoire des lanceurs/icones du theme courant.
extern gchar *g_cCurrentLaunchersPath;


/// surface de la zone de rappel.
extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
/// transparence de la zone de rappel.
extern double g_fVisibleZoneAlpha;
extern int g_iNbStripes;
/// amplitude de la siunsoide.
extern double g_fAmplitude;
/// largeur de la sinusoide en pixels. On va de 0 a pi en la parcourant, en etant a pi/2 au niveau du curseur; en dehors de cet intervalle, la sinusoide est plate.
extern int g_iSinusoidWidth;

/// la direction dans laquelle les icones grossissent. Vers le haut ou vers le bas.
extern gboolean g_bDirectionUp;

#endif
