#ifndef __CAIRO_DOCK_GLOBAL_VARIABLES_H__
#define __CAIRO_DOCK_GLOBAL_VARIABLES_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <cairo.h>

/// Pointeur sur le dock principal.
extern CairoDock *g_pMainDock;
/// Table des docks existant.
extern GHashTable *g_hDocksTable;
extern gchar *g_cLanguage;
/// Largeur de l'ecran en mode horizontal/vertical.
extern gint g_iScreenWidth[2];
/// Hauteur/largeur de l'ecran en mode horizontal/vertical.
extern gint g_iScreenHeight[2];

/// Epaisseur du cadre (en pixels).
extern gint g_iDockLineWidth;
/// rayon des coins du cadre.
extern gint g_iDockRadius;
/// Marge entre le cadre et les icones.
gint g_iFrameMargin;
/// La couleur du cadre.
extern double g_fLineColor[4];
/// Ecart en pixels entre les icones.
extern int g_iIconGap;
/// Taille de la police des etiquettes.
extern int g_iLabelSize;
/// Police de caracteres des etiquettes.
extern gchar *g_cLabelPolice;
/// Epaisseur des traits.
extern int g_iLabelWeight;
/// Italique ou droit.
extern int g_iLabelStyle;

/// TRUE ssi les coins du bas sont arrondis.
extern gboolean g_bRoundedBottomCorner;
/// TRUE si l'auto-hide est active.
extern gboolean g_bAutoHide;

/// Taille des reflets, en pixels, calcules par rapport a la hauteur max des icones.
extern double g_fReflectSize;
/// pouvoir reflechissant du plan.
extern double g_fAlbedo;

/// Couleur claire du fond ou des rayures.
extern double g_fStripesColorBright[4];
/// couleur foncee du fond ou des rayures.
extern double g_fStripesColorDark[4];

/// Largeur de la zone de rappel.
extern int g_iVisibleZoneWidth;
/// hauteur de la zone de rappel.
extern int g_iVisibleZoneHeight;

/// Le theme d'icone choisi.
extern GtkIconTheme *g_pIconTheme;
/// Le chemin vers le repertoire racine.
extern gchar *g_cCairoDockDataDir;
/// Le chemin vers le repertoire du theme courant.
extern gchar *g_cCurrentThemePath;
/// Le chemin vers le repertoire des lanceurs/icones du theme courant.
extern gchar *g_cCurrentLaunchersPath;


/// Surface de la zone de rappel.
extern cairo_surface_t *g_pVisibleZoneSurface;
extern double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
/// Transparence de la zone de rappel.
extern double g_fVisibleZoneAlpha;
extern int g_iNbStripes;
/// Amplitude de la siunsoide.
extern double g_fAmplitude;
/// Largeur de la sinusoide en pixels. On va de 0 a pi en la parcourant, en etant a pi/2 au niveau du curseur; en dehors de cet intervalle, la sinusoide est plate.
extern int g_iSinusoidWidth;

/// Utiliser les separateurs ou pas.
extern gboolean g_bUseSeparator;


/// La direction dans laquelle les icones grossissent. Vers le haut ou vers le bas.
extern gboolean g_bDirectionUp;

#endif
