/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
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
*********************** VERSION 0 (2006)*********************
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
*********************** VERSION 1.x.x (2007-20008)*********************
**
** author(s) :
**     Fabrice Rey <fabounet@users.berlios.de>
**
** notes :
**     I've completely rewritten the calculation part, and the callback system.
**     Plus added a conf file that allows to dynamically modify most of the parameters.
**     Plus a visible zone that make the hiding/showing more friendly.
**     Plus a menu and the drag'n'drop ability.
**     Also I've separated functions in several files in order to make the code more readable.
**     Now it sems more like a real dock !
**
**     Edit : plus a taskbar, plus an applet system,
**            plus the container ability, plus different views, plus the top and vertical position, ...
**
**
*******************************************************************************/

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

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
#include "cairo-dock-keyfile-manager.h"
#include "cairo-dock-config.h"
#include "cairo-dock-file-manager.h"
#include "cairo-dock-renderer-manager.h"
#include "cairo-dock-keybinder.h"
#include "cairo-dock-log.h"
#include "cairo-dock-X-utilities.h"

CairoDock *g_pMainDock;  // pointeur sur le dock principal.
GHashTable *g_hDocksTable = NULL;  // table des docks existant.
int g_iWmHint = GDK_WINDOW_TYPE_HINT_DOCK;  // hint pour la fenetre du dock principal.
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
gchar *g_cEasyConfFile = NULL;  // le chemin du fichier de conf pour les noobs ;-)
gpointer *g_pDefaultIconDirectory = NULL;  // les repertoires/themes ou on va chercher les icones.
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

double g_fReflectSize;  // taille des reflets, en pixels, calcules par rapport a la hauteur max des icones.
double g_fAlbedo;  // pouvoir reflechissant du plan.
gboolean g_bDynamicReflection;  // dis s'il faut recalculer en temps reel le degrade en transparence des reflets.

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
int g_tIconAuthorizedWidth[CAIRO_DOCK_NB_TYPES];  // les tailles min et max pour chaque type d'icone.
int g_tIconAuthorizedHeight[CAIRO_DOCK_NB_TYPES];
int g_tAnimationType[CAIRO_DOCK_NB_TYPES];  // le type de l'animation pour chaque type d'icone.
int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];  // le nombre de rebonds/rotation/etc lors d'un clique gauche.
int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];  // l'ordre de chaque type dans le dock.
int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS] = {17, 20, 20, 12, 20, 20, 0};  // 2n+3, 4n, 2n, 2n, 4n, 4n.

int g_iVisibleZoneWidth = 0;  // dimensions de la zone ou on place le curseur pour faire apparaitre le dock.
int g_iVisibleZoneHeight = 0;

gboolean g_bDirectionUp;  // la direction dans laquelle les icones grossissent. Vers le haut ou vers le bas.
gboolean g_bSameHorizontality;  // dit si les sous-docks ont la meme horizontalite que les docks racines.
double g_fSubDockSizeRatio;  // ratio de la taille des icones des sous-docks par rapport a celles du dock principal.
gboolean g_bAnimateSubDock;
int g_iLeaveSubDockDelay;
int g_iShowSubDockDelay;
gboolean bShowSubDockOnClick;  // TRUE <=> ne montrer les sous-docks qu'au clique, sinon au survol.

int g_iLabelSize;  // taille de la police des etiquettes.
gchar *g_cLabelPolice = NULL;  // police de caracteres des etiquettes.
int g_iLabelWeight;  // epaisseur des traits.
int g_iLabelStyle;  // italique ou droit.
gboolean g_bLabelForPointedIconOnly;  // n'afficher les etiquettes que pour l'icone pointee.
double g_fLabelAlphaThreshold;  // seuil de visibilité de etiquettes.
gboolean g_bTextAlwaysHorizontal;  // true <=> etiquettes horizontales meme pour les docks verticaux.
double g_fLabelBackgroundColor[4];  // couleur des etiquettes, alpha=0 pour utiliser des contours epais.
gboolean g_bUseBackgroundForLabel;

double g_fAlphaAtRest;

double g_fUnfoldAcceleration = 0;
int g_iGrowUpInterval;
int g_iShrinkDownInterval;
double g_fMoveUpSpeed = 0.5;
double g_fMoveDownSpeed = 0.33;
double g_fRefreshInterval = .04;

gboolean g_bShowAppli = FALSE;  // au debut on ne montre pas les applis, il faut que cairo-dock le sache.
gboolean g_bUniquePid = FALSE;  // une seule icone par PID.
gboolean g_bGroupAppliByClass = FALSE;  // une seule icone par classe, les autres dans un container.
int g_iAppliMaxNameLength;  // longueur max de la chaine de caractere du nom des applis.
gboolean g_bMinimizeOnClick;  // minimiser l'appli lorsqu'on clique sur son icone si elle est deja active.
gboolean g_bCloseAppliOnMiddleClick;  // utiliser le clique du milieu pour fermer une appli.
gboolean g_bAutoHideOnFullScreen;  // quick-hide automatique lorsqu'une fenetre passe en plein ecran (pour pas gener).
gboolean g_bDemandsAttentionWithDialog;  // attirer l'attention avec une bulle de dialogue.
gboolean g_bDemandsAttentionWithAnimation;  // attirer l'attention avec une animation.
gboolean g_bAnimateOnActiveWindow;  // jouer une breve animation de l'icone lorsque la fenetre correspondante devient active.
double g_fVisibleAppliAlpha;  // transparence des icones des applis dont la fenetre est visible.
gboolean g_bHideVisibleApplis;  // TRUE <=> cacher les applis dont la fenetre est visible.
gboolean g_bAppliOnCurrentDesktopOnly;  // TRUE <=> cacher les applis dont la fenetre n'est pas sur le bureau courant.
int g_iNbDesktops;  // nombre de bureaux.
int g_iNbViewportX, g_iNbViewportY;  // nombre de "faces du cube".

gboolean g_bUseSeparator = TRUE;  // utiliser les separateurs ou pas.
gchar *g_cSeparatorImage = NULL;
gboolean g_bRevolveSeparator;  // faire pivoter l'image des separateurs.
gboolean g_bConstantSeparatorSize;  // garder les separateurs de taille constante.

int g_iDialogButtonWidth;
int g_iDialogButtonHeight;
double g_fDialogColor[4];
int g_iDialogIconSize;
int g_iDialogMessageSize;  // taille de la police des messagez.
gchar *g_cDialogMessagePolice = NULL;  // police de caracteres des etiquettes.
int g_iDialogMessageWeight;  // epaisseur des traits.
int g_iDialogMessageStyle;  // italique ou droit.
double g_fDialogTextColor[4];

double g_fDeskletColor[4];
double g_fDeskletColorInside[4];

gchar *g_cRaiseDockShortcut = NULL;

gboolean g_bKeepAbove = FALSE;
gboolean g_bSkipPager = TRUE;
gboolean g_bSkipTaskbar = TRUE;
gboolean g_bSticky = TRUE;

gboolean g_bUseGlitz = FALSE;
gboolean g_bVerbose = FALSE;

short g_iMajorVersion, g_iMinorVersion, g_iMicroVersion;
CairoDockDesktopEnv g_iDesktopEnv = CAIRO_DOCK_UNKNOWN_ENV;
CairoDockFMSortType g_iFileSortType;
gboolean g_bShowHiddenFiles;

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

static void _cairo_dock_set_verbosity(gchar *cVerbosity)
{
  if (!cVerbosity)
    cd_log_set_level(G_LOG_LEVEL_WARNING);
  else if (!strcmp(cVerbosity, "debug"))
    cd_log_set_level(G_LOG_LEVEL_DEBUG);
  else if (!strcmp(cVerbosity, "message"))
    cd_log_set_level(G_LOG_LEVEL_MESSAGE);
  else if (!strcmp(cVerbosity, "warning"))
    cd_log_set_level(G_LOG_LEVEL_WARNING);
  else if (!strcmp(cVerbosity, "critical"))
    cd_log_set_level(G_LOG_LEVEL_CRITICAL);
  else if (!strcmp(cVerbosity, "error"))
    cd_log_set_level(G_LOG_LEVEL_ERROR);
  else {
    cd_log_set_level(G_LOG_LEVEL_WARNING);
		cd_warning("bad verbosity option: default to warning");
	}
}

int
main (int argc, char** argv)
{
	gint i;
	for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)
		g_tIconTypeOrder[i] = i;
	cd_log_init(FALSE);
        //No log
        cd_log_set_level(0);
	gtk_init (&argc, &argv);
	GError *erreur = NULL;

	//\___________________ On recupere quelques options.
	gboolean bDialogTest = FALSE, bSafeMode = FALSE, bNoSkipPager = FALSE, bNoSkipTaskbar = FALSE, bNoSticky = FALSE, bToolBarHint = FALSE, bNormalHint = FALSE, bCappuccino = FALSE, bExpresso = FALSE, bCafeLatte = FALSE, bPrintVersion = FALSE;
	gchar *cEnvironment = NULL, *cUserDefinedDataDir = NULL, *cVerbosity = 0;
	GOptionEntry TableDesOptions[] =
	{
		{"log", 'l', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
			&cVerbosity,
			"log verbosity (debug,message,warning,critical,error) default is warning", NULL},
		{"glitz", 'g', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&g_bUseGlitz,
			"use hardware acceleration through Glitz (needs a glitz-enabled libcairo)", NULL},
		{"keep-above", 'a', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&g_bKeepAbove,
			"keep the dock above other windows whatever", NULL},
		{"no-skip-pager", 'p', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bNoSkipPager,
			"show the dock in pager", NULL},
		{"no-skip-taskbar", 'b', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bNoSkipTaskbar,
			"show the dock in taskbar", NULL},
		{"no-sticky", 's', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bNoSticky,
			"don't make the dock appear on all desktops", NULL},
		{"toolbar-hint", 't', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bToolBarHint,
			"force the window manager to consider cairo-dock as a toolbar instead of a dock", NULL},
		{"normal-hint", 'n', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bNormalHint,
			"force the window manager to consider cairo-dock as a normal appli instead of a dock", NULL},
		{"env", 'e', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
			&cEnvironment,
			"force the dock to consider this environnement - it may crush the dock if not set properly.", NULL},
		{"dir", 'd', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
			&cUserDefinedDataDir,
			"force the dock to load this directory, instead of ~/.cairo-dock.", NULL},
		{"safe-mode", 's', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bSafeMode,
			"don't load any plug-ins", NULL},
		{"dialog", 'D', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bDialogTest,
			"for test on dialogs only", NULL},
		{"capuccino", 'C', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bCappuccino,
			"Cairo-Dock makes anything, including coffee !", NULL},
		{"expresso", 'E', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bExpresso,
			"Cairo-Dock makes anything, including coffee !", NULL},
		{"cafe-latte", 'L', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bCafeLatte,
			"Cairo-Dock makes anything, including coffee !", NULL},
		{"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bPrintVersion,
			"print version and quit.", NULL},
		{NULL}
	};

	GOptionContext *context = g_option_context_new ("Cairo-Dock");
	g_option_context_add_main_entries (context, TableDesOptions, NULL);
	g_option_context_parse (context, &argc, &argv, &erreur);
	if (erreur != NULL)
	{
		g_print ("ERREUR : %s\n", erreur->message);
		exit (-1);
	}

/* FIXME: utiliser l'option --enable-verbose du configure, l'idee etant que les fonctions de log sont non definies dans les versions officielles, histoire de pas faire le test tout le temps.
#ifndef CAIRO_DOCK_VERBOSE
	if (cVerbosity != NULL)
	{
		g_print ("Cairo-Dock was not compiled with verbose, configure it with --enable-verbose for that\n");
		g_free (cVerbosity);
		cVerbosity = NULL;
	}
#endif */
	_cairo_dock_set_verbosity(cVerbosity);
	g_free (cVerbosity);

	g_bSkipPager = ! bNoSkipPager;
	g_bSkipTaskbar = ! bNoSkipTaskbar;
	g_bSticky = ! bNoSticky;

	if (bToolBarHint)
		g_iWmHint = GDK_WINDOW_TYPE_HINT_TOOLBAR;
	if (bNormalHint)
		g_iWmHint = GDK_WINDOW_TYPE_HINT_NORMAL;
	if (cEnvironment != NULL)
	{
		if (strcmp (cEnvironment, "gnome") == 0)
			g_iDesktopEnv = CAIRO_DOCK_GNOME;
		else if (strcmp (cEnvironment, "kde") == 0)
			g_iDesktopEnv = CAIRO_DOCK_KDE;
		else if (strcmp (cEnvironment, "xfce") == 0)
			g_iDesktopEnv = CAIRO_DOCK_XFCE;
		else
			cd_warning ("Attention : unknown environnment '%s'", cEnvironment);
		g_free (cEnvironment);
	}
#ifndef HAVE_GLITZ
	if (g_bUseGlitz)
	{
		cd_warning ("Attention : Cairo-Dock was not compiled with glitz");
		g_bUseGlitz = FALSE;
	}
#endif

	if (bCappuccino)
	{
		g_print ("Please insert one coin into your PC.\n");
		return 0;
	}
	if (bExpresso)
	{
		g_print ("Sorry, no more sugar; please try again later.\n");
		return 0;
	}
	if (bCafeLatte)
	{
		g_print ("Honestly, you trust someone who includes such options in his code ?\n");
		return 0;
	}
	if (bPrintVersion)
	{
		g_print ("v%s\n", CAIRO_DOCK_VERSION);
		return 0;
	}

	//\___________________ On internationalise l'appli.
	bindtextdomain (CAIRO_DOCK_GETTEXT_PACKAGE, CAIRO_DOCK_LOCALE_DIR);
	bind_textdomain_codeset (CAIRO_DOCK_GETTEXT_PACKAGE, "UTF-8");
	textdomain (CAIRO_DOCK_GETTEXT_PACKAGE);

	//\___________________ On teste l'existence du repertoire des donnees .cairo-dock.
	g_cCairoDockDataDir = (cUserDefinedDataDir != NULL ? cUserDefinedDataDir : g_strdup_printf ("%s/%s", getenv("HOME"), CAIRO_DOCK_DATA_DIR));
	if (! g_file_test (g_cCairoDockDataDir, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir (g_cCairoDockDataDir, 7*8*8+7*8+5) != 0)
			cd_warning ("Attention : couldn't create directory %s", g_cCairoDockDataDir);
	}
	gchar *cThemesDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_THEMES_DIR);
	if (! g_file_test (cThemesDir, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir (cThemesDir, 7*8*8+7*8+5) != 0)
			cd_warning ("Attention : couldn't create directory %s", cThemesDir);
	}
	g_free (cThemesDir);
	g_cCurrentThemePath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_CURRENT_THEME_NAME);
	if (! g_file_test (g_cCurrentThemePath, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir (g_cCurrentThemePath, 7*8*8+7*8+5) != 0)
			cd_warning ("Attention : couldn't create directory %s", g_cCurrentThemePath);
	}
	g_cCurrentLaunchersPath = g_strdup_printf ("%s/%s", g_cCurrentThemePath, CAIRO_DOCK_LAUNCHERS_DIR);
	if (! g_file_test (g_cCurrentLaunchersPath, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir (g_cCurrentLaunchersPath, 7*8*8+7*8+5) != 0)
			cd_warning ("Attention : couldn't create directory %s", g_cCurrentLaunchersPath);
	}

	//\___________________ On initialise les numeros de version.
	gchar **cVersions = g_strsplit (CAIRO_DOCK_VERSION, ".", -1);
	if (cVersions[0] != NULL)
		g_iMajorVersion = atoi (cVersions[0]);
	if (cVersions[1] != NULL)
		g_iMinorVersion = atoi (cVersions[1]);
	if (cVersions[2] != NULL)
		g_iMicroVersion = atoi (cVersions[2]);
	g_strfreev (cVersions);

	//\___________________ On initialise la table des docks.
	g_hDocksTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		NULL);
	
	//\___________________ On initialise le multi-threading.
	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	//\___________________ initialise the keybinder
	cd_keybinder_init();

	//\___________________ On initialise le support de X.
	cairo_dock_initialize_X_support ();
	
	//\___________________ On detecte l'environnement de bureau (apres les applis et avant les modules).
	if (g_iDesktopEnv == CAIRO_DOCK_UNKNOWN_ENV)
		g_iDesktopEnv = cairo_dock_guess_environment ();
	cd_message ("environnement de bureau : %d", g_iDesktopEnv);
	
	//\___________________ On initialise le gestionnaire de modules et on pre-charge les modules existant.
	if (g_module_supported () && ! bSafeMode)
		cairo_dock_initialize_module_manager (CAIRO_DOCK_MODULES_DIR);

	//\___________________ On enregistre nos notifications.
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) cairo_dock_notification_build_menu, CAIRO_DOCK_RUN_AFTER);
	cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA, (CairoDockNotificationFunc) cairo_dock_notification_drop_data, CAIRO_DOCK_RUN_AFTER);
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) cairo_dock_notification_click_icon, CAIRO_DOCK_RUN_AFTER);
	cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON, (CairoDockNotificationFunc) cairo_dock_notification_middle_click_icon, CAIRO_DOCK_RUN_AFTER);
	cairo_dock_register_notification (CAIRO_DOCK_REMOVE_ICON, (CairoDockNotificationFunc) cairo_dock_notification_remove_icon, CAIRO_DOCK_RUN_FIRST);
	
	
	//\___________________ On initialise le gestionnaire de vues.
	cairo_dock_initialize_renderer_manager ();


	//\___________________ On charge le dernier theme ou on demande a l'utilisateur d'en choisir un.
	g_cConfFile = g_strdup_printf ("%s/%s", g_cCurrentThemePath, CAIRO_DOCK_CONF_FILE);
	g_cEasyConfFile = g_strdup_printf ("%s/%s", g_cCurrentThemePath, CAIRO_DOCK_EASY_CONF_FILE);
	if (! g_file_test (g_cConfFile, G_FILE_TEST_EXISTS) || bSafeMode)
	{
		cairo_dock_mark_theme_as_modified (FALSE);
		int r;
		while ((r = cairo_dock_ask_initial_theme ()) == 0);
		if (r == -1)
		{
			cd_message ("mata ne !\n");
			exit (0);
		}
	}

	cairo_dock_load_theme (g_cCurrentThemePath);


	//\___________________ On affiche le changelog en cas de nouvelle version.
	gchar *cLastVersionFilePath = g_strdup_printf ("%s/.cairo-dock-last-version", g_cCairoDockDataDir);
	gboolean bWriteChangeLog;
	if (! g_file_test (cLastVersionFilePath, G_FILE_TEST_EXISTS))
	{
		bWriteChangeLog = TRUE;
	}
	else
	{
		gsize length = 0;
		gchar *cContent = NULL;
		g_file_get_contents (cLastVersionFilePath,
			&cContent,
			&length,
			NULL);
		if (length > 0 && strcmp (cContent, CAIRO_DOCK_VERSION) == 0)
			bWriteChangeLog = FALSE;
		else
			bWriteChangeLog = TRUE;
		g_free (cContent);
	}

	g_file_set_contents (cLastVersionFilePath,
		CAIRO_DOCK_VERSION,
		-1,
		NULL);
	g_free (cLastVersionFilePath);

	if (bWriteChangeLog)
	{
		gchar *cChangeLogFilePath = g_strdup_printf ("%s/ChangeLog.txt", CAIRO_DOCK_SHARE_DATA_DIR);
		GKeyFile *pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, cChangeLogFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_message ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		else
		{
			gchar *cChangeLogMessage = g_key_file_get_string (pKeyFile, "ChangeLog", CAIRO_DOCK_VERSION, &erreur);
			if (erreur != NULL)
			{
				g_error_free (erreur);
				erreur = NULL;
			}
			else
			{
				Icon *pFirstIcon = cairo_dock_get_first_icon (g_pMainDock->icons);
				cairo_dock_show_temporary_dialog_with_default_icon (gettext (cChangeLogMessage), pFirstIcon, g_pMainDock, 0);
			}
			g_free (cChangeLogMessage);
		}
	}


	if (bDialogTest)
		g_timeout_add (2000, (GSourceFunc) random_dialog, NULL);  // pour tests seulement.


	//\___________________ Message a caractere informatif (ou pas).
	gchar *cSillyMessageFilePath = g_strdup_printf ("%s/.cairo-dock-silly-question", g_cCairoDockDataDir);
	//const gchar *cSillyMessage = "Le saviez-vous ?\nUtiliser cairo-dock vous rendra beau et intelligent !";
	//const gchar *cSillyMessage = "Le saviez-vous ?\nUtiliser cairo-dock augmentera votre popularité auprès de la gente féminine !";
	//const gchar *cSillyMessage = "Le saviez-vous ?\nCairo-Dock contribue à réduire le trou de la couche d'ozone !";
	//const gchar *cSillyMessage = "Montrer Cairo-Dock à un utilisateur de Mac est le meilleur moyen de s'en faire un ennemi;\nN'oubliez pas qu'il a payé 129$ pour avoir la même chose !";  // 7500
	//const gchar *cSillyMessage = "Petite annonce :\n  Projet sérieux recherche secrétaire pour rédiger documentation.\n  Niveau d'étude exigé : 95C.";  // 7500
	//const gchar *cSillyMessage = "Cairo-Dock fait même le café ! Au choix :\n cairo-dock --capuccino , cairo-dock --expresso , cairo-dock --cafe_latte";  // 8000
	//const gchar *cSillyMessage = "Veuillez rentrer un compliment élogieux à la gloire Fab pour pouvoir utiliser cairo-dock.";
	//const gchar *cSillyMessage = "Sondage :\n Combien cairo-dock c'est trop bien :";
	//const gchar *cSillyMessage = "Cairo-Dock : just launch it !";  // 4000
	//const gchar *cSillyMessage = "Cairo-Dock lave plus blanc que blanc.";  // 4000
	//const gchar *cSillyMessage = "Sondage :\nVoulez-vous voir plus de filles nues dans Cairo-Dock ?";
	//const gchar *cSillyMessage = "C'est les soldes !\n Pour tout sous-dock acheté, un sous-dock offert !";
	//const gchar *cSillyMessage = "J-2 avant la 1.5, la tension monte !";
	const gchar *cSillyMessage = NULL;
	const gchar *cNumSilllyMessage = "14";
	gboolean bWriteSillyMessage;
	if (! g_file_test (cSillyMessageFilePath, G_FILE_TEST_EXISTS))
	{
		bWriteSillyMessage = TRUE;
	}
	else
	{
		gsize length = 0;
		gchar *cContent = NULL;
		g_file_get_contents (cSillyMessageFilePath,
			&cContent,
			&length,
			NULL);
		if (length > 0 && strcmp (cContent, cNumSilllyMessage) == 0)
			bWriteSillyMessage = FALSE;
		else
			bWriteSillyMessage = TRUE;
		g_free (cContent);
	}

	g_file_set_contents (cSillyMessageFilePath,
		cNumSilllyMessage,
		-1,
		NULL);
	g_free (cSillyMessageFilePath);

	if (bWriteSillyMessage && ! bWriteChangeLog && cSillyMessage != NULL)
	{
		///cairo_dock_show_general_message (cSillyMessage, 4000);
		/*double fAnswer = cairo_dock_show_value_and_wait (cSillyMessage, pFirstIcon, g_pMainDock, 1.);
		cd_message (" ==> %.2f\n", fAnswer);
		if (fAnswer == 0)
			cd_message ("Cela sera consigné et utilisé contre vous le moment venu ;-)\n");
		else if (fAnswer == 1)
			cd_message ("je suis aussi d'accord ! ;-)\n");*/

		/*int iAnswer = cairo_dock_ask_question_and_wait (cSillyMessage, pFirstIcon, g_pMainDock);
		if (iAnswer == GTK_RESPONSE_YES)
			cd_message ("c'est bien ce que je pensais ;-)\n");
		else
			cd_message ("allez on ne me la fais pas ! ;-)\n");*/

		/*gchar *cAnswer = cairo_dock_show_demand_and_wait ("Test :", NULL, g_pMainDock, "pouet");
		cd_message (" -> %s\n", cAnswer);*/
		/*double fAnswer = cairo_dock_show_value_and_wait ("Test :", cairo_dock_get_first_appli (g_pMainDock->icons), g_pMainDock, .7);
		cd_message (" ==> %.2f\n", fAnswer);*/
	}
	
	cd_dbus_init();
	
	gtk_main ();

	rsvg_term ();
	
	cd_message ("Bye bye !");
	g_print ("\033[0m\n");

	return 0;
}

