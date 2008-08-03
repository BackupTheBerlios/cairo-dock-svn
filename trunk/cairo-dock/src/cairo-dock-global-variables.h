#ifndef __CAIRO_DOCK_GLOBAL_VARIABLES_H__
#define __CAIRO_DOCK_GLOBAL_VARIABLES_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "cairo-dock-struct.h"
G_BEGIN_DECLS

/// Pointeur sur le dock principal.
extern CairoDock *g_pMainDock;
/// Largeur de l'ecran en mode horizontal/vertical.
extern gint g_iScreenWidth[2];
/// Hauteur de l'ecran en mode horizontal/vertical.
extern gint g_iScreenHeight[2];
/// Nombre de bureaux virtuels.
extern int g_iNbDesktops;
extern int g_iNbViewportX;
extern int g_iNbViewportY;
/// Nombre de "faces" du cube en largeur et en hauteur.
//extern int g_iNbViewportX, g_iNbViewportY ;
/// Chemin du fichier de conf de l'appli.
extern gchar *g_cConfFile;
/// doublets de repertoires d'icones (pair) et de themes d'icones GTK (impair).
extern gpointer *g_pDefaultIconDirectory;

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
/// Description des etiquettes des icones.
extern CairoDockLabelDescription g_iconTextDescription;
/// Description des infos-rapides.
extern CairoDockLabelDescription g_quickInfoTextDescription;
/// Description des titres des dialogues.
extern CairoDockLabelDescription g_dialogTextDescription;

/// facteur de visibilite des etiquettes adjacentes a l'icone pointee.
extern double g_fLabelAlphaThreshold;
/// le texte est-il toujours horizontal ?
extern gboolean g_bTextAlwaysHorizontal;
/// seule l'etiquette de l'icone pointee doit-elle etre dessinee ?
extern gboolean g_bLabelForPointedIconOnly;

/// TRUE ssi les coins du bas sont arrondis.
extern gboolean g_bRoundedBottomCorner;

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
/// Ordre des groupe d'icones.
extern int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];


/// Ratio de la taille des icones des sous-docks par rapport a celles du dock principal.
extern double g_fSubDockSizeRatio;

/// Epaisseur de la ficelle.
extern int g_iStringLineWidth;

/// Taille max autorisee pour les docks.
extern int g_iMaxAuthorizedWidth;

/// Environnement de bureau detecte.
extern CairoDockDesktopEnv g_iDesktopEnv;

/// Type de tri des fichiers (par nom, taille, date, ...).
extern CairoDockFMSortType g_iFileSortType;
/// Montrer les fichiers caches ou non.
extern gboolean g_bShowHiddenFiles;

/// Couleur du fond des desklets.
extern double g_fDeskletColor[4];
/// Couleur du fond des desklets lorsque la souris est dedans..
extern double g_fDeskletColorInside[4];

extern int g_iDialogMessageSize;
extern gchar *g_cDialogMessagePolice;
extern int g_iDialogMessageWeight;
extern int g_iDialogMessageStyle;

G_END_DECLS
#endif
