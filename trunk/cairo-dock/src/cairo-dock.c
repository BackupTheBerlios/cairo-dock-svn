/*****************************************************************************************************
**
** Program:
**    cairo-dock
**
** License :
**    This program is released under the terms of the GNU General Public License, version 3 or above.
**    If you don't know what that means take a look at:
**       http://www.gnu.org/licenses/licenses.html#GPL
**
** Original idea :
**    Mirco Mueller, June 2006.
**
*********************** VERSION 1 (2006)*********************
** author(s):
**    Mirco "MacSlow" Mueller <macslow@bangang.de>
**    Behdad Esfahbod <behdad@behdad.org>
**    David Reveman <davidr@novell.com>
**    Karl Lattimer <karl@qdh.org.uk> 
**
** notes :
**    Originally conceived as a stress-test for cairo, librsvg, and glitz. 
**
** notes from original author:
**
**    I just know that some folks will bug me regarding this, so... yes there's
**    nearly everything hard-coded, it is not nice, it is not very usable for
**    easily (without any hard work) making a full dock-like application out of
**    this, please don't email me asking to me to do so... for everybody else
**    feel free to make use of it beyond this being a small but heavy stress
**    test. I've written this on an Ubuntu-6.06 box running Xgl/compiz. The
**    icons used are from the tango-project...
**
**        http://tango-project.org/
**
**    Over the last couple of days Behdad and David helped me (MacSlow) out a
**    great deal by sending me additional tweaked and optimized versions. I've
**    now merged all that with my recent additions.
**
*********************** VERSION 2 (2007)*********************
**
** author(s) :
**     Fabrice Rey <fabounet_03@yahoo.fr>, 
**
** notes :
**     I've completely rewritten the calculation part, and the callback system.
**     Plus added a conf file that allows to dynamically modify most of the parameters.
**     Plus a visible zone that make the hiding/showing more friendly.
**     Plus a menu and the drag'n'drop ability.
**     Also I've separated functions in several files in order to make the code more readable.
**     Now it sems more like a real dock !
**     Feel free to upgrade it according to your wishes !
**
**     Edit : plus a taskbar, plus an applet system,
**            plus the container ability, plus the carousel view, plus the top and vertical view.
**
**
*******************************************************************************/

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <pango/pango.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-struct.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-applications.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-load.h"
#include "cairo-dock-config.h"
#include "cairo-dock-themes-manager.h"

#define CAIRO_DOCK_DATA_DIR ".cairo-dock"
//#define CAIRO_DOCK_CONF_FILE "cairo-dock.conf"

CairoDock *g_pMainDock;  // pointeur sur le dock principal.
GHashTable *g_hDocksTable = NULL;  // table des docks existant.
int g_iWmHint;  // hint pour la fenetre du dock principal.
gchar *g_cLanguage = NULL;  // langue courante.
gboolean g_bReserveSpace;

gboolean g_bReverseVisibleImage;  // retrouner l'image de la zone de rappel quand le dock est en haut.
gint g_iScreenWidth[2];  // dimensions de l'ecran.
gint g_iScreenHeight[2];
int g_iMaxAuthorizedWidth;  // largeur maximale autorisee pour la fenetre (0 pour la taille de l'ecran).
int g_iScrollAmount;  // de combien de pixels fait defiler un coup de molette.
gboolean g_bResetScrollOnLeave;  // revenir a un defilement nul lorsqu'on quitte la fenetre.
double g_fScrollAcceleration;  // acceleration du defilement quand il revient a la normale.
gboolean g_bForceLoop;

gchar *g_cCurrentThemePath = NULL;  // le chemin vers le repertoire du theme courant.
gchar *g_cConfFile = NULL;  // le chemin du fichier de conf.
gchar **g_cDefaultIconDirectory = NULL;  // les repertoires ou on va chercher les icones avant d'aller chercher dans le theme d'icones.
GtkIconTheme *g_pIconTheme = NULL;  // le theme d'icone choisi.
gchar *g_cCairoDockDataDir = NULL;  // le repertoire ou on va chercher les .desktop.

gboolean g_bAutoHide;
double g_fAmplitude;  // amplitude de la siunsoide.
int g_iSinusoidWidth;  // largeur de la sinusoide en pixels. On va de 0 a pi en la parcourant, en etant a pi/2 au niveau du curseur; en dehors de cet intervalle, la sinusoide est plate.
int g_iNbAnimationRounds;
gint g_iDockLineWidth;  // thickness of dock-bg outline.
gint g_iDockRadius;  // radius of dock-bg corners.
gboolean g_bRoundedBottomCorner;  // vrai ssi les coins du bas sont arrondis.
double g_fLineColor[4];  // la couleur du cadre.
gint g_iStringLineWidth;  // epaisseur de la ficelle.
double g_fStringColor[4];  // la couleur de la ficelle.

cairo_surface_t *g_pVisibleZoneSurface = NULL;  // surface de la zone de rappel.
double g_fVisibleZoneAlpha;  // transparence de la zone de rappel.
int g_iNbStripes;  // le nombre de rayures a dessiner en fond dans chaque motif elementaire.
double g_fStripesSpeedFactor;  // =1 les rayures suivent le curseur, <1 les rayures vont d'autant moins vite.
double g_fStripesWidth;  // leur epaisseur relative.
double g_fStripesColorBright[4];  // couleur claire du fond ou des rayures.
double g_fStripesColorDark[4];  // couleur foncee du fond ou des rayures.
double g_fStripesAngle;  // angle par rapport a la verticale des decorations.
gchar *g_cBackgroundImageFile = NULL;  // nom du fichier image a mettre en fond.
double g_fBackgroundImageWidth = 0, g_fBackgroundImageHeight = 0;  // sa taille reelle.
gboolean g_bBackgroundImageRepeat; // repeter l'image du fond comme un motif.
double g_fBackgroundImageAlpha;  // transparence de l'image de fond.
cairo_surface_t *g_pBackgroundSurface[2] = {NULL, NULL};  // surface associee a l'image du fond, de la taille de l'image du fond.
cairo_surface_t *g_pBackgroundSurfaceFull[2] = {NULL, NULL};  // surface associee aux decorations, de 2 fois la taille de la fenetre.

int g_iIconGap = 0;  // ecart en pixels entre les icones (pour l'instant une valeur > 0 cree des decalages intempestifs).
int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];  // les tailles min et max pour chaque type d'icone.
int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
int g_tAnimationType[CAIRO_DOCK_NB_TYPES];  // le type de l'animation pour chaque type d'icone.
int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];  // le nombre de rebonds/rotation/etc lors d'un clique gauche.
int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];  // l'ordre de chaque type dans le dock.
int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS] = {14, 20, 20, 12, 0};  // multiples de 2,4,2.

int g_iVisibleZoneWidth = 0;  // dimensions de la zone ou on place le curseur pour faire apparaitre le dock.
int g_iVisibleZoneHeight = 0;

//		|                                |
//		|                                |
//		|                                |
//		|                                |
//		|                                |
//		|              ______            |
//		|             |______|  <-- zone |
//		|        barre des taches        | <-- gapY
//		 ---------------.----------------
//		                 ^-- gapX

gboolean g_bDirectionUp = TRUE;  // la direction dans laquelle les icones grossissent. Vers le haut ou vers le bas.
gboolean g_bSameHorizontality = TRUE;  // dit si les sous-docks ont la meme horizontalite que les docks racines.
double g_fSubDockSizeRatio;  // ratio de la taille des icones des sous-docks par rapport a celles du dock principal.
gboolean bShowSubDockOnMouseOver;

int g_iLabelSize;  // taille de la police des etiquettes.
gchar *g_cLabelPolice;  // police de caracteres des etiquettes.
int g_iLabelWeight;  // epaisseur des traits.
int g_iLabelStyle;  // italique ou droit.
gboolean g_bLabelForPointedIconOnly;  // n'afficher les etiquettes que pour l'icone pointee.
double g_fLabelAlphaThreshold;  // seuil de visibilité de etiquettes.
gboolean g_bTextAlwaysHorizontal;  // true <=> tetiquettes horizontales meme pour les docks verticaux.

double g_fUnfoldAcceleration = 0;
double g_fGrowUpFactor = 1.4;
double g_fShrinkDownFactor = 0.6;
double g_fMoveUpSpeed = 0.5;
double g_fMoveDownSpeed = 0.33;
double g_fRefreshInterval = .04;

gboolean g_bShowAppli = FALSE;  // au debut on ne montre pas les applis, il faut que cairo-dock le sache.
gboolean g_bUniquePid;
int unsigned g_iAppliMaxNameLength;
Display *g_XDisplay = NULL;
Screen *g_XScreen = NULL;
Atom g_aNetClientList = 0;
GHashTable *g_hAppliTable = NULL;  // table des PID connus de cairo-dock (affichees ou non dans le dock).
GHashTable *g_hXWindowTable = NULL;  // table des fenetres X affichees dans le dock.
int g_iSidUpdateAppliList = 0;
gchar *g_cSeparatorImage = NULL;
gboolean g_bRevolveSeparator;

GHashTable *g_hModuleTable = NULL;  // table des modules charges dans l'appli.

gboolean g_bKeepAbove = TRUE;
gboolean g_bSkipPager = TRUE;
gboolean g_bSkipTaskbar = TRUE;
gboolean g_bSticky = TRUE;

CairoDockClickFunc cairo_dock_launch_uri_func = NULL;
CairoDockFileManagerFunc cairo_dock_add_uri_func = NULL;
CairoDockLoadDirectoryFunc cairo_dock_load_directory_func = NULL;

gboolean g_bUseGlitz = FALSE;
gboolean g_bVerbose = FALSE;


int
main (int argc, char** argv)
{
	gint i;
	for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
		g_tIconTypeOrder[i] = i;
	
	
	gtk_init (&argc, &argv);
	
	
	//\___________________ On recupere quelques options.
	g_iWmHint = GDK_WINDOW_TYPE_HINT_NORMAL;
	for (i = 0; i < argc; i++)
	{
		if (strcmp (argv[i], "--glitz") == 0)
		{
#ifdef HAVE_GLITZ
			g_bUseGlitz = TRUE;
#else
			g_print ("Attention : Cairo-Dock was not compiled with glitz\n");
			g_bUseGlitz = FALSE;
#endif
		}
		else if (strcmp (argv[i], "--no-glitz") == 0)
		{
			g_bUseGlitz = FALSE;
		}
		else if (strcmp (argv[i], "--no-keep-above") == 0)
			g_bKeepAbove = FALSE;
		else if (strcmp (argv[i], "--no-skip-pager") == 0)
			g_bSkipPager = FALSE;
		else if (strcmp (argv[i], "--no-skip-taskbar") == 0)
			g_bSkipTaskbar = FALSE;
		else if (strcmp (argv[i], "--no-sticky") == 0)
			g_bSticky = FALSE;
		else if (strcmp (argv[i], "--toolbar-hint") == 0)
			g_iWmHint = GDK_WINDOW_TYPE_HINT_TOOLBAR;
		else if (strcmp (argv[i], "--dock-hint") == 0)
			g_iWmHint = GDK_WINDOW_TYPE_HINT_DOCK;
		else if (strcmp (argv[i], "--version") == 0)  // le dock restera devant quoiqu'il arrive, mais ne recupere plus les touches clavier.
		{
			system ("pkg-config --modversion cairo-dock");
			return 0;
		}
		else if (strcmp (argv[i], "--verbose") == 0)
		{
#ifdef CAIRO_DOCK_VERBOSE
			g_bVerbose = TRUE;
#else
			g_print ("Attention : Cairo-Dock was not compiled with verbose\n");
			g_bVerbose = FALSE;
#endif
		}
		else if (argv[i][0] == '-')
		{
			gboolean help = (strcmp (argv[i], "--help") == 0);
			fprintf (help ? stdout : stderr,
				 "Usage: %s [--glitz] [--no-keep-above] [--no-skip-pager] [--no-skip-taskbar] [--no-sticky] [--dock-hint] [--toolbar-hint] [--help]\n",
				 argv[0]);
			return help ? 0 : 1;
		}
	}
	
	//\___________________ On definit quelques structures et quelques variables necessaires aux applis.
	g_hAppliTable = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	g_hXWindowTable = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	g_XDisplay = XOpenDisplay (0);
	g_XScreen = DefaultScreenOfDisplay (g_XDisplay);
	g_aNetClientList = XInternAtom (g_XDisplay, "_NET_CLIENT_LIST", False);
	XSetErrorHandler (cairo_dock_xerror_handler);
	
	g_hModuleTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		NULL,  // la cle est le nom du module, et pointe directement sur le champ 'cModuleName' du module.
		(GDestroyNotify) cairo_dock_free_module);
	
	g_hDocksTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		NULL);
	
	//\___________________ On pre-charge les modules existant.
	GError *erreur = NULL;
	cairo_dock_preload_module_from_directory (CAIRO_DOCK_MODULES_DIR, g_hModuleTable, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	
	//\___________________ On cree le dock principal.
	g_pMainDock = cairo_dock_create_new_dock (g_iWmHint, CAIRO_DOCK_MAIN_DOCK_NAME);
	g_pMainDock->bIsMainDock = TRUE;
	
	//\___________________ On teste l'existence du repertoire des donnees .cairo-dock.
	g_cCairoDockDataDir = g_strdup_printf ("%s/%s", getenv("HOME"), CAIRO_DOCK_DATA_DIR);
	if (! g_file_test (g_cCairoDockDataDir, G_FILE_TEST_IS_DIR))
	{
		g_mkdir (g_cCairoDockDataDir, 7*8*8+7*8+5);
	}
	gchar *cThemeDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR);
	if (! g_file_test (cThemeDir, G_FILE_TEST_IS_DIR))
	{
		g_mkdir (cThemeDir, 7*8*8+7*8+5);
	}
	g_free (cThemeDir);
	
	//\___________________ On charge le dernier theme ou on demande a l'utilisateur d'en choisir un.
	gchar *cLastThemeName = cairo_dock_get_last_theme_name (g_cCairoDockDataDir);
	g_cCurrentThemePath = cairo_dock_get_theme_path (cLastThemeName, NULL);
	g_free (cLastThemeName);
	
	if (g_cCurrentThemePath != NULL)
		g_cConfFile = cairo_dock_load_theme (g_cCurrentThemePath);
	
	if (g_cConfFile == NULL || ! g_file_test (g_cConfFile, G_FILE_TEST_EXISTS))
	{
		g_cCurrentThemePath = cairo_dock_ask_initial_theme ();
		
		if (g_cCurrentThemePath == NULL)
		{
			g_print ("Mata ne.\n");
			exit (0);
		}
		else
		{
			g_cConfFile = cairo_dock_load_theme (g_cCurrentThemePath);
			if (g_cConfFile == NULL || ! g_file_test (g_cConfFile, G_FILE_TEST_EXISTS))
			{
				g_error ("failed to open theme\n");
			}
		}
	}
	
	
	gtk_main ();
	/*Window root = DefaultRootWindow (g_XDisplay);
	Window w = GDK_WINDOW_XID (GTK_WIDGET (pWindow)->window);
	XSetWindowAttributes attr;
	attr.event_mask = 
		KeyPressMask | KeyReleaseMask | ButtonPressMask
		| ButtonReleaseMask | EnterWindowMask
		| LeaveWindowMask | PointerMotionMask
		| Button1MotionMask
		//| Button2MotionMask | Button3MotionMask
		//| Button4MotionMask | Button5MotionMask
		| ButtonMotionMask | KeymapStateMask
		| ExposureMask //VisibilityChangeMask
		| StructureNotifyMask // | ResizeRedirectMask
		| SubstructureNotifyMask | SubstructureRedirectMask
		//| FocusChangeMask | PropertyChangeMask
		//| ColormapChangeMask | OwnerGrabButtonMask
		;
	XSelectInput(g_XDisplay, w, attr.event_mask);
	XEvent event;
	while (1)
	{
		XNextEvent (g_XDisplay, &event);
		if (event.xany.window != root)
		{
			g_print ("w (%d)\n", event.type);
			switch (event.type)
			{
				case MotionNotify :
					on_xmotion_notify (&event.xmotion);
				break;
				
				case Expose : 
					on_xexpose (&event.xexpose);
				break;
				
				case ConfigureNotify :
					on_xconfigure (&event.xconfigure);
				break;
				
				case EnterNotify :
					on_xenter_notify (&event.xcrossing);
				break;
				
				case LeaveNotify :
					on_xleave_notify (&event.xcrossing);
				break;
				
				default :
				break;
			}
		}
		else //if (event.xany.window == root)
		{
			g_print ("root (%d)\n", event.type);
			switch (event.type)
			{
				
				default :
				break;
			}
		}
	}*/
	
	rsvg_term ();
	
	return 0;
}

