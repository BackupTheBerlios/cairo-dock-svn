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
#include <stdlib.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-icons.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-callbacks.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-menu.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-renderer-manager.h"

CairoDock *g_pMainDock;  // pointeur sur le dock principal.
GHashTable *g_hDocksTable = NULL;  // table des docks existant.
int g_iWmHint;  // hint pour la fenetre du dock principal.
gchar *g_cLanguage = NULL;  // langue courante.
gboolean g_bReserveSpace;
gchar *g_cMainDockDefaultRendererName = NULL;
gchar *g_cSubDockDefaultRendererName = NULL;

gboolean g_bReverseVisibleImage;  // retrouner l'image de la zone de rappel quand le dock est en haut.
gint g_iScreenWidth[2];  // dimensions de l'ecran.
gint g_iScreenHeight[2];
int g_iMaxAuthorizedWidth;  // largeur maximale autorisee pour la fenetre (0 pour la taille de l'ecran).
int g_iScrollAmount;  // de combien de pixels fait defiler un coup de molette.
gboolean g_bResetScrollOnLeave;  // revenir a un defilement nul lorsqu'on quitte la fenetre.
double g_fScrollAcceleration;  // acceleration du defilement quand il revient a la normale.

gchar *g_cCurrentThemePath = NULL;  // le chemin vers le repertoire du theme courant.
gchar *g_cCurrentLaunchersPath = NULL;  // le chemin vers le repertoire des lanceurs/icones du theme courant.
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
gint g_iFrameMargin;  // marge entre le cadre et les icones.
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
gboolean g_bDecorationsFollowMouse;  // dis si les decorations sont asservies au curseur, ou si le delta de deplacement ne depend que de la direction de celui-ci.

int g_iIconGap;  // ecart en pixels entre les icones.
int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];  // les tailles min et max pour chaque type d'icone.
int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
int g_tAnimationType[CAIRO_DOCK_NB_TYPES];  // le type de l'animation pour chaque type d'icone.
int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];  // le nombre de rebonds/rotation/etc lors d'un clique gauche.
int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];  // l'ordre de chaque type dans le dock.
int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS] = {14, 20, 20, 12, 0};  // multiples de 2,4,2.

int g_iVisibleZoneWidth = 0;  // dimensions de la zone ou on place le curseur pour faire apparaitre le dock.
int g_iVisibleZoneHeight = 0;

gboolean g_bDirectionUp;  // la direction dans laquelle les icones grossissent. Vers le haut ou vers le bas.
gboolean g_bSameHorizontality;  // dit si les sous-docks ont la meme horizontalite que les docks racines.
double g_fSubDockSizeRatio;  // ratio de la taille des icones des sous-docks par rapport a celles du dock principal.
gboolean g_bAnimateSubDock;
int g_iLeaveSubDockDelay;
int g_iShowSubDockDelay;

int g_iLabelSize;  // taille de la police des etiquettes.
gchar *g_cLabelPolice = NULL;  // police de caracteres des etiquettes.
int g_iLabelWeight;  // epaisseur des traits.
int g_iLabelStyle;  // italique ou droit.
gboolean g_bLabelForPointedIconOnly;  // n'afficher les etiquettes que pour l'icone pointee.
double g_fLabelAlphaThreshold;  // seuil de visibilité de etiquettes.
gboolean g_bTextAlwaysHorizontal;  // true <=> etiquettes horizontales meme pour les docks verticaux.

double g_fUnfoldAcceleration = 0;
int g_iGrowUpInterval;
int g_iShrinkDownInterval;
double g_fMoveUpSpeed = 0.5;
double g_fMoveDownSpeed = 0.33;
double g_fRefreshInterval = .04;

gboolean g_bShowAppli = FALSE;  // au debut on ne montre pas les applis, il faut que cairo-dock le sache.
gboolean g_bUniquePid;
gboolean g_bGroupAppliByClass = TRUE;
int unsigned g_iAppliMaxNameLength;
gchar *g_cSeparatorImage = NULL;
gboolean g_bRevolveSeparator;

gboolean g_bKeepAbove = TRUE;
gboolean g_bSkipPager = TRUE;
gboolean g_bSkipTaskbar = TRUE;
gboolean g_bSticky = TRUE;

gboolean g_bUseGlitz = FALSE;
gboolean g_bVerbose = FALSE;


static gboolean random_dialog (gpointer user_data)
{
	g_return_val_if_fail (g_pMainDock != NULL && g_pMainDock->icons != NULL, TRUE);
	
	int num_icone = g_random_int_range (0, g_list_length (g_pMainDock->icons));
	
	Icon *icon = g_list_nth_data (g_pMainDock->icons, num_icone);
	if (CAIRO_DOCK_IS_SEPARATOR (icon))
		return random_dialog (user_data);
	cairo_dock_show_temporary_dialog (icon->acName, icon, g_pMainDock, 7000);
	return TRUE;
}

int
main (int argc, char** argv)
{
	gint i;
	for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
		g_tIconTypeOrder[i] = i;
	
	
	gtk_init (&argc, &argv);
	
	
	//\___________________ On recupere quelques options.
	g_iWmHint = GDK_WINDOW_TYPE_HINT_DOCK;
	gboolean bDialogTest = FALSE;
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
		else if (strcmp (argv[i], "--normal-hint") == 0)
			g_iWmHint = GDK_WINDOW_TYPE_HINT_NORMAL;
		else if (strcmp (argv[i], "--dock-hint") == 0)  // le dock restera devant quoiqu'il arrive, mais ne recuperera plus les touches clavier.
			g_print ("Attention : the '--dock-hint' option is deprecated since v1.3.7\n  It is now the default behaviour.");
		else if (strcmp (argv[i], "--dialog") == 0)
			bDialogTest = TRUE;
		else if (strcmp (argv[i], "--version") == 0)
		{
			g_print ("v%s\n", CAIRO_DOCK_VERSION);
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
				 "Usage:\n%s\n  [--glitz] (use hardware acceleration (needs a glitz-enabled libcairo))\n  [--no-keep-above] (don't keep the dock above other windows)\n  [--no-skip-pager] (show the dock in the pager)\n  [--no-skip-taskbar] (show the dock in taskbar)\n  [--no-sticky] (don't show the dock on all desktops)\n  [--normal-hint] (force the window manager to consider cairo-dock as a normal appli instead of a dock)\n  [--toolbar-hint] (force the window manager to consider cairo-dock as a toolbar instead of a dock)\n  [--help] (print this help and quit)\n",
				 argv[0]);
			return help ? 0 : 1;
		}
	}
	
	//\___________________ On initialise la table des docks..
	g_hDocksTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		NULL);
	
	//\___________________ On initialise le gestionnaire des applications ouvertes.
	cairo_dock_initialize_application_manager ();
	
	//\___________________ On initialise le gestionnaire de modules et on pre-charge les modules existant.
	cairo_dock_initialize_module_manager (CAIRO_DOCK_MODULES_DIR);
	
	//\___________________ On initialise le gestionnaire de vues.
	cairo_dock_initialize_renderer_manager ();
	
	
	//\___________________ On teste l'existence du repertoire des donnees .cairo-dock.
	g_cCairoDockDataDir = g_strdup_printf ("%s/%s", getenv("HOME"), CAIRO_DOCK_DATA_DIR);
	if (! g_file_test (g_cCairoDockDataDir, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir (g_cCairoDockDataDir, 7*8*8+7*8+5) != 0)
			g_print ("Attention : couldn't create directory %s\n", g_cCairoDockDataDir);
	}
	gchar *cThemesDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR);
	if (! g_file_test (cThemesDir, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir (cThemesDir, 7*8*8+7*8+5) != 0)
			g_print ("Attention : couldn't create directory %s\n", cThemesDir);
	}
	g_free (cThemesDir);
	g_cCurrentThemePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_CURRENT_THEME_NAME);
	if (! g_file_test (g_cCurrentThemePath, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir (g_cCurrentThemePath, 7*8*8+7*8+5) != 0)
			g_print ("Attention : couldn't create directory %s\n", g_cCurrentThemePath);
	}
	g_cCurrentLaunchersPath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, CAIRO_DOCK_LAUNCHERS_DIR);
	if (! g_file_test (g_cCurrentLaunchersPath, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir (g_cCurrentLaunchersPath, 7*8*8+7*8+5) != 0)
			g_print ("Attention : couldn't create directory %s\n", g_cCurrentLaunchersPath);
	}
	
	
	//\___________________ On enregistre nos notifications.
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) cairo_dock_notification_build_menu, CAIRO_DOCK_RUN_AFTER);
	cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA, (CairoDockNotificationFunc) cairo_dock_notification_drop_data, CAIRO_DOCK_RUN_AFTER);
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) cairo_dock_notification_click_icon, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_DOUBLE_CLICK_ICON, (CairoDockNotificationFunc) cairo_dock_notification_double_click_icon, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_REMOVE_ICON, (CairoDockNotificationFunc) cairo_dock_notification_remove_icon, CAIRO_DOCK_RUN_FIRST);
	
	//\___________________ On charge le dernier theme ou on demande a l'utilisateur d'en choisir un.
	g_cConfFile = g_strdup_printf ("%s/%s", g_cCurrentThemePath, CAIRO_DOCK_CONF_FILE);
	if (! g_file_test (g_cConfFile, G_FILE_TEST_EXISTS))
	{
		int r;
		while ((r = cairo_dock_ask_initial_theme ()) == 0);
		if (r == -1)
		{
			g_print ("mata ne !\n");
			exit (0);
		}
	}
	
	cairo_dock_load_theme (g_cCurrentThemePath);
	
	if (bDialogTest)
		g_timeout_add (2000, (GSourceFunc) random_dialog, NULL);  // pour tests seulement.
	
	gtk_main ();
	
	rsvg_term ();
	
	return 0;
}

