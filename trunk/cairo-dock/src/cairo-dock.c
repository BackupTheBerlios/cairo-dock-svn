/*******************************************************************************
**
** Program:
**    cairo-dock
**
** License :
**    This program is released under the terms of the GNU General Public License.
**    If you don't know what that means take a look at:
**       http://www.gnu.org/licenses/licenses.html#GPL
**
** Original idea :
**    Mirco Mueller, June 2006.
**
*********************** VERSION 1 *********************
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
*********************** VERSION 2 *********************
**
** author(s) :
**     Fabrice Rey <fabounet_03@yahoo.fr>, 
**
** notes :
**     I've completely rewritten the calculation part, and the callback system.
**     Plus added a conf file that allows to dynamically modify most of the parameters.
**     Plus a visible zone that make the hiding/showing more friendly.
**     Plus added a menu and the drag'n'drop ability.
**     Also I've separated functions in several files in order to make the code more readable.
**     Now it sems more like a real dock !
**     Feel free to upgrade it according to your wishes !
**
**
*******************************************************************************/

#include <math.h>
#include <string.h>

#include <pango/pango.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <dirent.h>
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
#include "cairo-dock-config.h"


#define CAIRO_DOCK_DATA_DIR ".cairo-dock"
//#define CAIRO_DOCK_CONF_FILE "cairo-dock.conf"


GList* icons = NULL;
GtkWidget *g_pWidget = NULL;

gint g_iScreenWidth = 0;  // dimensions de l'ecran.
gint g_iScreenHeight = 0;
gint g_iCurrentWidth = 0;  // inutile sans glitz.
gint g_iCurrentHeight = 0;

gboolean g_bAtBottom = TRUE;
gboolean g_bAtTop = FALSE;
gboolean g_bInside = FALSE;
gboolean g_bMenuVisible = FALSE;

float g_fMagnitude = 0.0; // coef multiplicateur de l'amplitude de la sinusoide (entre 0 et 1)
gdouble g_fGradientOffsetX = 4.0f;  // decalage des rayures pour les faire suivre la souris.

int g_iMaxIconHeight = 0;  // max des hauteurs des icones.
int g_iMinDockWidth = 0;  // taille minimale du dock.
int g_iMaxDockWidth = 0;  // taille maximale du dock.
int g_iMaxDockHeight = 0;

gint g_iWindowPositionX = 0;  // dock-windows current y-position
gint g_iWindowPositionY = 0;  // dock-windows current y-position
int g_iSidMoveDown = 0;  // serial ID du thread de descente de la fenetre.
int g_iSidMoveUp = 0;  // serial ID du thread de montee de la fenetre.
int g_iSidGrowUp = 0;  // serial ID du thread de grossisement des icones.
int g_iSidShrinkDown = 0;  // serial ID du thread de diminution des icones.

gchar *g_cCairoDockBackgroundFileName = NULL;  // le chemin de l'image de fond de la partie tout le temps visible.
gchar *g_cConfFile = NULL;  // le chemin du fichier de conf.
gchar **g_cDefaultIconDirectory = NULL;  // les repertoires par defaut ou on va chercher les icones.
gchar *g_cCairoDockDataDir = NULL;  // le repertoire ou on va chercher les .desktop.
gchar *g_cDefaultFileBrowser = NULL;


gboolean g_bAutoHide;
double g_fAmplitude;  // amplitude de la siunsoide.
int g_iSinusoidWidth;  // largeur de la sinusoide en pixels. On va de 0 a pi en la parcourant, en etant a pi/2 au niveau du curseur; en dehors de cet intervalle, la sinusoide est plate.
int g_iNbAnimationRounds;
gint g_iDockLineWidth;  // thickness of dock-bg outline.
gint g_iDockRadius;  // radius of dock-bg corners.
gboolean g_bRoundedBottomCorner;  // vrai ssi les coins du bas sont arrondis.
double g_fLineColor[4];  // la couleur du cadre.

cairo_surface_t *g_pVisibleZoneSurface = NULL;
double g_fVisibleZoneImageWidth, g_fVisibleZoneImageHeight;
double g_fVisibleZoneAlpha;
int g_iNbStripes = 0;  // le nombre de rayures a dessiner en fond dans chaque motif elementaire.
double g_fStripesWidth;  // leur epaisseur relative.
double g_fStripesColorBright[4];  // la couleur claire des rayures.
double g_fStripesColorDark[4];  // la couleur foncee des rayures.

int g_iIconGap = 0;  // ecart en pixels entre les icones (pour l'instant une valeur > 0 cree des decalages intempestifs).
int g_tMinIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];
int g_tMaxIconAuthorizedSize[CAIRO_DOCK_NB_TYPES];  // les tailles min et max pour chaque type d'icone.
int g_tAnimationType[CAIRO_DOCK_NB_TYPES];  // le type de l'animation pour chaque type d'icone.
int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];  // le nombre de rebonds/rotation/etc lors d'un clique gauche.
int g_tIconTypeOrder[CAIRO_DOCK_NB_TYPES];  // l'ordre de chaque type dans le dock.
GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];

int g_iVisibleZoneWidth = 0;  // dimensions de la zone ou on place le curseur pour faire apparaitre le dock.
int g_iVisibleZoneHeight = 0;
int g_iGapX = 0;  // decalage de la zone par rapport au milieu bas de l'ecran.
int g_iGapY = 0;
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

gboolean g_bDirectionUp;  // la direction dans laquelle les icones grossissent. Vers le haut ou vers le bas.
gboolean g_bHorizontalDock = TRUE;  // dit si le dock est horizontal ou vertical.
gboolean g_bUseText;  // vrai ssi on doit afficher les etiquettes au-dessus des icones.
int g_iLabelSize;  // taille de la police des etiquettes.
gchar *g_cLabelPolice;  // police de caracteres des etiquettes.
int g_iLabelWeight;
int g_iLabelStyle;
gboolean g_bLabelForPointedIconOnly;

double g_fGrowUpFactor = 1.4;
double g_fShrinkDownFactor = 0.6;
double g_fMoveUpSpeed = 0.5;
double g_fMoveDownSpeed = 0.33;

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

GHashTable *g_hModuleTable = NULL;  // table des modules charges dans l'appli.

Icon *g_pLastFixedIconLeft = NULL;
Icon *g_pLastFixedIconRight = NULL;


static gboolean g_bKeepAbove = TRUE;
static gboolean g_bSkipPager = TRUE;
static gboolean g_bSkipTaskbar = TRUE;
static gboolean g_bSticky = TRUE;


#ifdef HAVE_GLITZ
gboolean g_bUseGlitz = TRUE;
glitz_drawable_format_t *gDrawFormat = NULL;
glitz_drawable_t* g_pGlitzDrawable = NULL;
glitz_format_t* g_pGlitzFormat = NULL;
#endif // HAVE_GLITZ


static void
on_alpha_screen_changed (GtkWidget* pWidget,
			GdkScreen* pOldScreen,
			GtkWidget* pLabel)
{
	GdkScreen* pScreen = gtk_widget_get_screen (pWidget);
	GdkColormap* pColormap = gdk_screen_get_rgba_colormap (pScreen);
	
	if (!pColormap)
		pColormap = gdk_screen_get_rgb_colormap (pScreen);
		
	gtk_widget_set_colormap (pWidget, pColormap);
}


int
main (int argc, char** argv)
{
	memset (g_tIconsSubList, 0, CAIRO_DOCK_NB_TYPES * sizeof (GList *));
	gint i;
	for (i = 0; i < CAIRO_DOCK_NB_TYPES; i ++)  // pour l'instant en dur.
		g_tIconTypeOrder[i] = i;
	
	
 	gtk_init (&argc, &argv);
 	
 		
	//\___________________ On recupere quelques options (peu utile).
	int iWmHint = GDK_WINDOW_TYPE_HINT_NORMAL;
	for (i = 0; i < argc; i++)
	{
		if (strcmp (argv[i], "--glitz") == 0)
		{
#ifdef HAVE_GLITZ
			g_bUseGlitz = TRUE;
#else
			fprintf (stderr, "Not compiled with glitz\n");
			return 1;
#endif
		}
		else if (strcmp (argv[i], "--no-glitz") == 0)
		{
#ifdef HAVE_GLITZ
			g_bUseGlitz = FALSE;
#endif
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
			iWmHint = GDK_WINDOW_TYPE_HINT_TOOLBAR;
		else if (strcmp (argv[i], "--dock-hint") == 0)
			iWmHint = GDK_WINDOW_TYPE_HINT_DOCK;
		else if (strcmp (argv[i], "--version") == 0)  // le dock restera devant quoiqu'il arrive, mais ne recupere plus les touches clavier.
		{
			system ("pkg-config --modversion cairo-dock");
			return 0;
		}
		else if (argv[i][0] == '-')
		{
			gboolean help = (strcmp (argv[i], "--help") == 0);
			fprintf (help ? stdout : stderr,
				 "Usage: %s [--no-glitz] [--no-keep-above] [--no-skip-pager] [--no-skip-taskbar] [--no-sticky] [--help]\n",
				 argv[0]);
			return help ? 0 : 1;
		}
	}
	

	//\___________________ On cree la fenetre.
	GtkWidget* pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_pWidget = pWindow;
	
	if (g_bSticky)
		gtk_window_stick (GTK_WINDOW (pWindow));
	gtk_window_set_keep_above (GTK_WINDOW (pWindow), g_bKeepAbove);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (pWindow), g_bSkipPager);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (pWindow), g_bSkipTaskbar);
	gtk_window_set_gravity (GTK_WINDOW (pWindow), GDK_GRAVITY_STATIC);
	gtk_window_set_type_hint (GTK_WINDOW (pWindow), iWmHint);
	on_alpha_screen_changed (pWindow, NULL, NULL);

	gtk_widget_set_app_paintable (pWindow, TRUE);
	gtk_window_set_decorated (GTK_WINDOW (pWindow), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (pWindow), TRUE);
	gtk_window_set_title (GTK_WINDOW (pWindow), "cairo-dock");
	
	GdkScreen *gdkscreen = gtk_window_get_screen (GTK_WINDOW (pWindow));
	g_iScreenWidth = gdk_screen_get_width (gdkscreen);
	g_iScreenHeight = gdk_screen_get_height (gdkscreen);
	
	gtk_widget_show_all (pWindow);
	
	//\___________________ On definit des variables necessaires aux applis.
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
	
	//\___________________ On teste l'existence du repertoire des donnees .cairo-dock.
	g_cCairoDockDataDir = g_strdup_printf ("%s/%s", getenv("HOME"), CAIRO_DOCK_DATA_DIR);
	g_cConfFile = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CAIRO_DOCK_CONF_FILE);
	if (! g_file_test (g_cCairoDockDataDir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		g_print("Attention : directory %s doesn't exist or is not readable. Trying to fix it ...\n", g_cCairoDockDataDir);
		
		gchar *cCommand = g_strdup_printf ("mkdir -p %s", g_cCairoDockDataDir);
		system (cCommand);
		g_free (cCommand);
		
		cCommand = g_strdup_printf ("cp %s/%s %s", CAIRO_DOCK_SHARE_DIR, CAIRO_DOCK_CONF_FILE, g_cCairoDockDataDir);
		system (cCommand);
		g_free (cCommand);
		
		cCommand = g_strdup_printf ("cp %s/* %s", CAIRO_DOCK_SHARE_DATA_DIR, g_cCairoDockDataDir);
		system (cCommand);
		g_free (cCommand);
		
		if (! g_file_test (g_cCairoDockDataDir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
		{
			g_print("Attention : directory %s unreadable.\n", g_cCairoDockDataDir);
			exit (1) ;
		}
	}
	else if (! g_file_test (g_cCairoDockDataDir, G_FILE_TEST_EXISTS))
	{
		gchar *cCommand = g_strdup_printf ("cp %s/%s %s", CAIRO_DOCK_SHARE_DIR, CAIRO_DOCK_CONF_FILE, g_cCairoDockDataDir);
		system (cCommand);
		g_free (cCommand);
	}
	
	//\___________________ On pre-charge les modules existant.
	GError *erreur = NULL;
	cairo_dock_preload_module_from_directory (CAIRO_DOCK_MODULES_DIR, g_hModuleTable, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	cairo_dock_update_conf_file_with_modules (g_cConfFile, g_hModuleTable);
	
	//\___________________ On lit le fichier de conf et on charge tout.
	
	cairo_dock_read_conf_file (pWindow, g_cConfFile);
	
	
#ifdef HAVE_GLITZ
	g_iCurrentWidth = g_iVisibleZoneWidth;
	g_iCurrentHeight = g_iVisibleZoneHeight;
	if (g_bUseGlitz)
	{
	    glitz_drawgeable_format_t templ, *format;
	    unsigned long	    mask = 0;
	    XVisualInfo		    *vinfo = NULL;
	    int			    screen = 0;
	    GdkVisual		    *visual;
	    GdkColormap		    *colormap;
	    GdkDisplay		    *gdkdisplay;
	    Display		    *g_XDisplay;

	    templ.doublebuffer = 1;
	    mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;

	    gdkdisplay = gtk_widget_get_display (pWindow);
	    g_XDisplay   = gdk_x11_display_get_g_XDisplay (gdkdisplay);

	    i = 0;
	    do {
		format = glitz_glx_find_window_format (g_XDisplay, screen,
						       mask, &templ, i++);
		if (format)
		{
		    vinfo = glitz_glx_get_visual_info_from_format (g_XDisplay,
								   screen,
								   format);
		    if (vinfo->depth == 32)
		    {
			gDrawFormat = format;
			break;
		    }
		    else
		    {
			if (!gDrawFormat)
			    gDrawFormat = format;
		    }
		}
	    } while (format);

		if (!gDrawFormat)
		{
			fprintf (stderr, "no double buffered GLX visual\n");
			return 1;
		}

		vinfo = glitz_glx_get_visual_info_from_format (g_XDisplay,
							       screen,
							       gDrawFormat);

		visual = gdkx_visual_get (vinfo->visualid);
		colormap = gdk_colormap_new (visual, TRUE);

		gtk_widget_set_colormap (pWindow, colormap);
		gtk_widget_set_double_buffered (pWindow, FALSE);
	}
	else
#endif
	

	
	//\___________________ On connecte les evenements.
	gtk_widget_add_events (pWindow,
			       GDK_BUTTON_PRESS_MASK |
			       GDK_POINTER_MOTION_MASK |
			       GDK_POINTER_MOTION_HINT_MASK);
	
	g_signal_connect (G_OBJECT (pWindow),
			  "destroy",
			  G_CALLBACK (gtk_main_quit),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "expose-event",
			  G_CALLBACK (on_expose),
			  NULL);
//#ifdef HAVE_GLITZ	
	g_signal_connect (G_OBJECT (pWindow),
			  "configure-event",
			  G_CALLBACK (on_configure),
			  NULL);
//#endif
	g_signal_connect (G_OBJECT (pWindow),
			  "key-press-event",
			  G_CALLBACK (on_key_press),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "key-release-event",
			  G_CALLBACK (on_key_release),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "button-press-event",
			  G_CALLBACK (on_button_press2),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "button-release-event",
			  G_CALLBACK (on_button_release),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "motion-notify-event",
			  G_CALLBACK (on_motion_notify2),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "enter-notify-event",
			  G_CALLBACK (on_enter_notify2),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "leave-notify-event",
			  G_CALLBACK (on_leave_notify2),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "drag_data_received",
			  G_CALLBACK (on_drag_data_received),
			  NULL);
	g_signal_connect (G_OBJECT (pWindow),
			  "drag_motion",
			  G_CALLBACK (on_drag_motion),
			  NULL);
	GtkTargetEntry *pTargetEntry = g_new0 (GtkTargetEntry, 1);
	pTargetEntry[0].target = g_strdup ("text/uri-list");
	pTargetEntry[0].flags = (GtkTargetFlags) 0;
	pTargetEntry[0].info = 0;
	gtk_drag_dest_set (pWindow,
		GTK_DEST_DEFAULT_DROP | GTK_DEST_DEFAULT_MOTION,  // GTK_DEST_DEFAULT_HIGHLIGHT ne rend pas joli je trouve.
		pTargetEntry,
		1,
		GDK_ACTION_COPY);
	
	
#ifdef HAVE_GLITZ
	if (g_bUseGlitz)
	{
		glitz_format_t templ;
		GdkDisplay	   *gdkdisplay;
		Display	   *g_XDisplay;
		Window	   xid;

		gdkdisplay = gdk_display_get_default ();
		g_XDisplay   = gdk_x11_display_get_g_XDisplay (gdkdisplay);

		xid = gdk_x11_drawable_get_xid (GDK_DRAWABLE (pWindow->window));

		g_pGlitzDrawable = glitz_glx_create_drawable_for_window (g_XDisplay,
									 0,
									 gDrawFormat,
									 xid,
									 g_iCurrentWidth,
									 g_iCurrentHeight);
		if (!g_pGlitzDrawable)
		{
			fprintf (stderr, "failed to create drawable\n");
			return 1;
		}

		templ.color        = gDrawFormat->color;g_iMaxDockWidth
		templ.color.fourcc = GLITZ_FOURCC_RGB;

		g_pGlitzFormat = glitz_find_format (g_pGlitzDrawable,
						    GLITZ_FORMAT_RED_SIZE_MASK   |
						    GLITZ_FORMAT_GREEN_SIZE_MASK |
						    GLITZ_FORMAT_BLUE_SIZE_MASK  |
						    GLITZ_FORMAT_ALPHA_SIZE_MASK |
						    GLITZ_FORMAT_FOURCC_MASK,
						    &templ,
						    0);
		if (!g_pGlitzFormat)
		{
			fprintf (stderr, "couldn't find surface format\n");
			return 1;
		}
	}
#endif
      
	
	gtk_main ();
	/*Window root = DefaultRootWindow (g_XDisplay);
	Window w = GDK_WINDOW_XID (GTK_WIDGET (pWindow)->window);
	XEvent event;
	while (1)
	{
		XNextEvent (g_XDisplay, &event);
		if (event.window == w)
		{
			g_print ("w (%d)\n", event.type);
			switch (event.type)
			{
				case MotionNotify :
				
				break;
				
				case Expose : 
				
				break;
				
				case ConfigureNotify :
				
				break;
				
				default :
				break;
			}
		}
		else if (event.window == root)
		{
			g_print ("root (%d)\n", event.type);
			switch (event.type)
			{
				
				default :
				break;
			}
		}
	}*/
	
	
	Icon *icon;
	GList *ic;
	for (ic = icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cairo_dock_free_icon (icon);
	}
	g_list_free (icons);

	rsvg_term ();

	return 0;
}

