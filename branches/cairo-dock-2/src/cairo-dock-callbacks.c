/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <gdk/gdkx.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif
#include <gtk/gtkgl.h>
#include <GL/glu.h>

#include "cairo-dock-menu.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-desktop-file-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-config.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-notifications.h"
#include "cairo-dock-themes-manager.h"
#include "cairo-dock-dialogs.h"
#include "cairo-dock-file-manager.h"
#include "cairo-dock-log.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-keybinder.h"
#include "cairo-dock-desklet.h"
#include "cairo-dock-draw-opengl.h"
#include "cairo-dock-emblem.h" //Drop Indicator
#include "cairo-dock-callbacks.h"

static Icon *s_pIconClicked = NULL;  // pour savoir quand on deplace une icone a la souris. Dangereux si l'icone se fait effacer en cours ...
static CairoDock *s_pLastPointedDock = NULL;  // pour savoir quand on passe d'un dock a un autre.
static int s_iSidNonStopScrolling = 0;
static int s_iSidShowSubDockDemand = 0;
static int s_iSidShowAppliForDrop = 0;
static CairoDock *s_pDockShowingSubDock = NULL;  // on n'accede pas a son contenu, seulement l'adresse.

extern CairoDock *g_pMainDock;
extern double g_fSubDockSizeRatio;
extern gboolean g_bAnimateSubDock;
extern gboolean g_bAnimateOnAutoHide;
extern double g_fUnfoldAcceleration;
extern int g_iLeaveSubDockDelay;
extern int g_iShowSubDockDelay;
extern gboolean bShowSubDockOnClick;
extern gboolean g_bUseSeparator;
extern gboolean g_bKeepAbove;
extern gboolean g_bPopUp;

extern gint g_iScreenWidth[2];
extern gint g_iScreenHeight[2];
extern int g_iScrollAmount;
extern gboolean g_bResetScrollOnLeave;
extern gboolean g_bDecorationsFollowMouse;
extern cairo_surface_t *g_pBackgroundSurfaceFull[2];

extern gboolean g_bSameHorizontality;
extern gboolean g_bTextAlwaysHorizontal;
extern CairoDockLabelDescription g_iconTextDescription;

extern int g_iDockRadius;
extern int g_iDockLineWidth;

extern gchar *g_cConfFile;

extern int g_iVisibleZoneHeight, g_iVisibleZoneWidth;

extern double g_fRefreshInterval;

extern gboolean g_bMinimizeOnClick;
extern gboolean g_bCloseAppliOnMiddleClick;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern int g_tNbIterInOneRound[CAIRO_DOCK_NB_ANIMATIONS];

extern gboolean g_bUseGlitz;
extern CairoDockFMSortType g_iFileSortType;
extern gchar *g_cRaiseDockShortcut;

extern cairo_surface_t *g_pDesktopBgSurface;
extern gboolean g_bUseFakeTransparency;
extern gboolean g_bUseOpenGL;

static gboolean s_bTemporaryAutoHide = FALSE;
static gboolean s_bEntranceAllowed = TRUE;
static gboolean s_bAutoHideInitialValue;
static gboolean s_bHideAfterShortcut = FALSE;

#define CAIRO_DOCK_IN_MOVMENT(pDock) (pDock->iSidMouseOver || pDock->iSidDropIndicator || pDock->iSidShrinkDown || pDock->iSidGrowUp)

void on_realize (GtkWidget* pWidget,
	 CairoDock *pDock)
{
	static GLuint s_iChromeTexture = 0;
	if (! g_bUseOpenGL)
		return ;
	GdkGLContext* pGlContext = gtk_widget_get_gl_context (pWidget);
	GdkGLDrawable* pGlDrawable = gtk_widget_get_gl_drawable (pWidget);
	if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
		return ;
	
	if (pDock->bIsMainDock)
	{
		g_print ("OpenGL version: %s\n", glGetString (GL_VERSION));
		g_print ("OpenGL vendor: %s\n", glGetString (GL_VENDOR));
		g_print ("OpenGL renderer: %s\n", glGetString (GL_RENDERER));
		
		if (s_iChromeTexture == 0)
		{
			cairo_surface_t *pChromeSurface = cairo_dock_load_chrome_surface ();
			
			if (pChromeSurface != NULL)
				s_iChromeTexture = cairo_dock_create_texture_from_surface (pChromeSurface);
			
			cairo_dock_init_capsule_display (s_iChromeTexture);
		}
		cairo_dock_init_square_display ();
		cairo_dock_init_cube_display ();
	}
	
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth (1.0f);
	glEnable (GL_BLEND);
	glShadeModel (GL_SMOOTH);  // par defaut.
	glEnable (GL_TEXTURE_2D);
	glPolygonMode (GL_FRONT, GL_FILL);
	glDisable (GL_CULL_FACE);
	
	glEnable (GL_LIGHTING);  // pour indiquer a OpenGL qu'il devra prendre en compte l'eclairage.
	//glLightModelf (GL_LIGHT_MODEL_TWO_SIDE, 0.0f);
	//glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);  // OpenGL doit considerer pour ses calculs d'eclairage que l'oeil est dans la scene (plus realiste).
	GLfloat fGlobalAmbientColor[4] = {0., 0., 0., 0.};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fGlobalAmbientColor);  // on definit la couleur de la lampe d'ambiance.
	glEnable (GL_LIGHT0);  // on allume la lampe 0.
	GLfloat fDiffuseColor[4] = {0., 1., 1., 1.};
	glLightfv (GL_LIGHT0, GL_DIFFUSE, fDiffuseColor);  // GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR
	GLfloat fDirection[4] = {0., 1., 1., 0.};  // le dernier 0 <=> direction.
	glLightfv(GL_LIGHT0, GL_POSITION, fDirection);

	///glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);  // GL_MODULATE / GL_DECAL /  GL_BLEND
	glTexParameteri (GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri (GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);

	gdk_gl_drawable_gl_end (pGlDrawable);
}
double a = .85;
gboolean on_expose (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoDock *pDock)
{
	//g_print ("%s ((%d;%d) %dx%d) (%d)\n", __func__, pExpose->area.x, pExpose->area.y, pExpose->area.width, pExpose->area.height, pDock->bAtBottom);
	
	if (g_bUseOpenGL && pDock->render_opengl != NULL)
	{
		GdkGLContext *pGlContext = gtk_widget_get_gl_context (pDock->pWidget);
		GdkGLDrawable *pGlDrawable = gtk_widget_get_gl_drawable (pDock->pWidget);
		if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
			return FALSE;
		
		/*int iMouseX = 
		for (k = n; k >= 0; k --)
		{
			
		}*/
		
		if (pDock->iSidBlurFading != 0 || CAIRO_DOCK_IN_MOVMENT (pDock))
			glAccum(GL_MULT, a);
		
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		pDock->render_opengl (pDock);
		
		if (pDock->iSidBlurFading != 0 || CAIRO_DOCK_IN_MOVMENT (pDock))
		{
			glAccum (GL_ACCUM, 1-a);
			glAccum (GL_RETURN, 1.0);
		}
		else
		{
			glClearAccum (0., 0., 0., 0.);
			glClear (GL_ACCUM_BUFFER_BIT);
			glAccum (GL_ACCUM, 1.);
		}
		
		
		if (gdk_gl_drawable_is_double_buffered (pGlDrawable))
			gdk_gl_drawable_swap_buffers (pGlDrawable);
		else
			glFlush ();
		gdk_gl_drawable_gl_end (pGlDrawable);
		
		if (pDock->iSidDropIndicator != 0)
		{
			cairo_t *pCairoContext = cairo_dock_create_drawing_context_on_area (CAIRO_CONTAINER (pDock), &pExpose->area, NULL);
			cairo_dock_draw_drop_indicator (pDock, pCairoContext);
			cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
			if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
				glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
		}
		
		return FALSE ;
	}
	
	if (pExpose->area.x + pExpose->area.y != 0)  // x et/ou y sont > 0.
	{
		if (! (pDock->bAutoHide && pDock->iRefCount == 0) || ! pDock->bAtBottom)
		{
			cairo_t *pCairoContext = cairo_dock_create_drawing_context_on_area (CAIRO_CONTAINER (pDock), &pExpose->area, NULL);
			if (pDock->iSidDropIndicator != 0)
				cairo_save (pCairoContext);
			
			if (pDock->render_optimized != NULL)
				pDock->render_optimized (pCairoContext, pDock, &pExpose->area);
			else
				pDock->render (pCairoContext, pDock);
			
			if (pDock->iSidDropIndicator != 0)
			{
				cairo_restore (pCairoContext);
				cairo_dock_draw_drop_indicator (pDock, pCairoContext);
			}
			cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
			if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
				glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
		}
		return FALSE;
	}
	
	
	cairo_t *pCairoContext = cairo_dock_create_drawing_context (CAIRO_CONTAINER (pDock));
	
	if (cairo_dock_is_loading ())
	{
		//cairo_dock_render_blank (pDock);
	}
	else if (!pDock->bAtBottom)
	{
		pDock->render (pCairoContext, pDock);
	}
	else
	{
		if (pDock->bAutoHide && pDock->iRefCount == 0)
		{
			if (! pDock->bInside)
				cairo_dock_render_background (pCairoContext, pDock);
			//else  // ne devrait pas arriver.
			//	cairo_dock_render_blank (pDock);
		}
		else
			pDock->render (pCairoContext, pDock);
	}
	
	//Indicateur de drop, ca prend un petit peu de ressources, je me demande si ce serai pas mieux de mettre en cache les emblèmes classiques
	//Pour gagner en rapidité, parce que du coup on charge la surface à chaques mouvements dans le dock.
	/**Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	if (pDock->bIsDragging && pPointedIcon != NULL)
	{
		cairo_translate (pCairoContext, pPointedIcon->fDrawX, pPointedIcon->fDrawY);
		cairo_dock_draw_emblem_classic (pCairoContext, pPointedIcon, (CairoContainer *) pDock, CAIRO_DOCK_EMBLEM_DROP_INDICATOR, CAIRO_DOCK_EMBLEM_UPPER_RIGHT, FALSE);
	}*/
	/// je mets en commentaire pour la 1.6.0 car ca manque de tests.
	
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
	return FALSE;
}



void cairo_dock_show_subdock (Icon *pPointedIcon, gboolean bUpdate, CairoDock *pDock)
{
	cd_debug ("on montre le dock fils");
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	g_return_if_fail (pSubDock != NULL);
	
	if (GTK_WIDGET_VISIBLE (pSubDock->pWidget))  // il est deja visible.
	{
		if (pSubDock->iSidShrinkDown != 0)  // il est en cours de diminution, on renverse le processus.
		{
			g_source_remove (pSubDock->iSidShrinkDown);
			pSubDock->iSidShrinkDown = 0;
			if (pSubDock->iSidGrowUp == 0)  // on commence a faire grossir les icones.  //  && pDock->iSidShrinkDown == 0
				pSubDock->iSidGrowUp = g_timeout_add (40, (GSourceFunc) cairo_dock_grow_up, (gpointer) pSubDock);
		}
		return ;
	}
	
	if (pSubDock->iSidShrinkDown != 0)  // precaution sans doute superflue.
	{
		g_source_remove (pSubDock->iSidShrinkDown);
		pSubDock->iSidShrinkDown = 0;
		pSubDock->iMagnitudeIndex = 0;
		cairo_dock_shrink_down (pSubDock);
	}

	if (bUpdate)
	{
		pDock->calculate_icons (pDock);  // c'est un peu un hack pourri, l'idee c'est de recalculer la position exacte de l'icone pointee pour pouvoir placer le sous-dock precisement, car sa derniere position connue est decalee d'un coup de molette par rapport a la nouvelle, ce qui fait beaucoup. L'ideal etant de le faire que pour l'icone concernee ...
	}

	pSubDock->set_subdock_position (pPointedIcon, pDock);

	pSubDock->fFoldingFactor = (g_bAnimateSubDock ? g_fUnfoldAcceleration : 0);
	pSubDock->bAtBottom = FALSE;
	int iNewWidth, iNewHeight;
	if (pSubDock->fFoldingFactor == 0)
	{
		cd_debug ("  on montre le sous-dock sans animation");
		cairo_dock_get_window_position_and_geometry_at_balance (pSubDock, CAIRO_DOCK_MAX_SIZE, &iNewWidth, &iNewHeight);  // CAIRO_DOCK_NORMAL_SIZE -> CAIRO_DOCK_MAX_SIZE pour la 1.5.4
		pSubDock->bAtBottom = TRUE;  // bAtBottom ajoute pour la 1.5.4

		gtk_window_present (GTK_WINDOW (pSubDock->pWidget));

		if (pSubDock->bHorizontalDock)
			gdk_window_move_resize (pSubDock->pWidget->window,
				pSubDock->iWindowPositionX,
				pSubDock->iWindowPositionY,
				iNewWidth,
				iNewHeight);
		else
			gdk_window_move_resize (pSubDock->pWidget->window,
				pSubDock->iWindowPositionY,
				pSubDock->iWindowPositionX,
				iNewHeight,
				iNewWidth);

		/*if (pSubDock->bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pSubDock->pWidget), pSubDock->iWindowPositionX, pSubDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pSubDock->pWidget), pSubDock->iWindowPositionY, pSubDock->iWindowPositionX);

		gtk_window_present (GTK_WINDOW (pSubDock->pWidget));*/
		///gtk_widget_show (GTK_WIDGET (pSubDock->pWidget));
	}
	else
	{
		cd_debug ("  on montre le sous-dock avec animation");
		cairo_dock_get_window_position_and_geometry_at_balance (pSubDock, CAIRO_DOCK_MAX_SIZE, &iNewWidth, &iNewHeight);

		gtk_window_present (GTK_WINDOW (pSubDock->pWidget));
		///gtk_widget_show (GTK_WIDGET (pSubDock->pWidget));
		if (pSubDock->bHorizontalDock)
			gdk_window_move_resize (pSubDock->pWidget->window,
				pSubDock->iWindowPositionX,
				pSubDock->iWindowPositionY,
				iNewWidth,
				iNewHeight);
		else
			gdk_window_move_resize (pSubDock->pWidget->window,
				pSubDock->iWindowPositionY,
				pSubDock->iWindowPositionX,
				iNewHeight,
				iNewWidth);

		if (pSubDock->iSidGrowUp == 0)  // on commence a faire grossir les icones.  //  && pDock->iSidShrinkDown == 0
			pSubDock->iSidGrowUp = g_timeout_add (40, (GSourceFunc) cairo_dock_grow_up, (gpointer) pSubDock);
	}
	//g_print ("  -> Gap %d;%d -> W(%d;%d) (%d)\n", pSubDock->iGapX, pSubDock->iGapY, pSubDock->iWindowPositionX, pSubDock->iWindowPositionY, pSubDock->bHorizontalDock);
	
	gtk_window_set_keep_above (GTK_WINDOW (pSubDock->pWidget), g_bPopUp);
}
static gboolean _cairo_dock_show_sub_dock_delayed (CairoDock *pDock)
{
	cd_debug ("");
	s_iSidShowSubDockDemand = 0;
	s_pDockShowingSubDock = NULL;
	Icon *icon = cairo_dock_get_pointed_icon (pDock->icons);
	if (icon != NULL && icon->pSubDock != NULL)
		cairo_dock_show_subdock (icon, FALSE, pDock);
	
	return FALSE;
}

#define ROTATION_COUNT 40
static gboolean _cairo_dock_animate_on_mouse_over (CairoDock *pDock)
{
	gboolean bContinue = FALSE;
	Icon *icon;
	GList *ic;
	int n = g_tNbIterInOneRound[CAIRO_DOCK_ROTATE] / 2;  // nbre d'iteration pour 1/2 tour.
	n = ROTATION_COUNT/2;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->iMouseOverAnimationCount > 0)
		{
			icon->iMouseOverAnimationCount --;
			icon->iRotationY = icon->iMouseOverAnimationCount * 180 / n;
			bContinue |= (icon->iMouseOverAnimationCount != 0);
		}
	}
	gtk_widget_queue_draw (pDock->pWidget);
	if (! bContinue)
	{
		pDock->iSidMouseOver = 0;
	}
	return bContinue;
}
static gboolean _cairo_dock_show_xwindow_for_drop (Icon *pIcon)
{
	cairo_dock_show_xwindow (pIcon->Xid);
	return FALSE;
}
void cairo_dock_on_change_icon (Icon *pLastPointedIcon, Icon *pPointedIcon, CairoDock *pDock)
{
	//cd_debug ("on change d'icone dans %x (-> %s)", pDock, (pPointedIcon != NULL ? pPointedIcon->acName : "rien"));
	if (s_iSidShowSubDockDemand != 0 && pDock == s_pDockShowingSubDock)
	{
		//cd_debug ("on annule la demande de montrage de sous-dock");
		g_source_remove (s_iSidShowSubDockDemand);
		s_iSidShowSubDockDemand = 0;
		s_pDockShowingSubDock = NULL;
	}
	
	if (pDock->bIsDragging && s_iSidShowAppliForDrop != 0)
	{
		//cd_debug ("on annule la demande de montrage d'appli");
		g_source_remove (s_iSidShowAppliForDrop);
		s_iSidShowAppliForDrop = 0;
	}
	cairo_dock_replace_all_dialogs ();
	if (pDock->bIsDragging && CAIRO_DOCK_IS_APPLI (pPointedIcon))
	{
		s_iSidShowAppliForDrop = g_timeout_add (500, (GSourceFunc) _cairo_dock_show_xwindow_for_drop, (gpointer) pPointedIcon);
	}
	if ((pDock == s_pLastPointedDock || s_pLastPointedDock == NULL) && pLastPointedIcon != NULL && pLastPointedIcon->pSubDock != NULL)
	{
		CairoDock *pSubDock = pLastPointedIcon->pSubDock;
		if (GTK_WIDGET_VISIBLE (pSubDock->pWidget))
		{
			//g_print ("on cache %s en changeant d'icône\n", pLastPointedIcon->acName);
			if (pLastPointedIcon->pSubDock->iSidLeaveDemand == 0)
			{
				//g_print ("  on retarde le cachage du dock de %dms\n", MAX (g_iLeaveSubDockDelay, 330));
				pLastPointedIcon->pSubDock->iSidLeaveDemand = g_timeout_add (MAX (g_iLeaveSubDockDelay, 330), (GSourceFunc) cairo_dock_emit_leave_signal, (gpointer) pLastPointedIcon->pSubDock);
			}
		}
		//else
		//	cd_debug ("pas encore visible !\n");
	}
	if (pPointedIcon != NULL && pPointedIcon->pSubDock != NULL && pPointedIcon->pSubDock != s_pLastPointedDock && (! bShowSubDockOnClick || CAIRO_DOCK_IS_APPLI (pPointedIcon)))
	{
		//cd_debug ("il faut montrer un sous-dock");
		if (pPointedIcon->pSubDock->iSidLeaveDemand != 0)
		{
			g_source_remove (pPointedIcon->pSubDock->iSidLeaveDemand);
			pPointedIcon->pSubDock->iSidLeaveDemand = 0;
		}
		if (g_iShowSubDockDelay > 0)
		{
			//pDock->iMouseX = iX;
			s_iSidShowSubDockDemand = g_timeout_add (g_iShowSubDockDelay, (GSourceFunc) _cairo_dock_show_sub_dock_delayed, pDock);
			s_pDockShowingSubDock = pDock;
			//cd_debug ("s_iSidShowSubDockDemand <- %d\n", s_iSidShowSubDockDemand);
		}
		else
			cairo_dock_show_subdock (pPointedIcon, FALSE, pDock);
		s_pLastPointedDock = pDock;
	}
	pLastPointedIcon = pPointedIcon;
	if (s_pLastPointedDock == NULL)
	{
		//g_print ("pLastPointedDock n'est plus null\n");
		s_pLastPointedDock = pDock;
	}
	if (pPointedIcon != NULL && pDock->render_opengl != NULL)
	{
		if (pDock->iSidMouseOver == 0)
		{
			pDock->iSidMouseOver = g_timeout_add (30., (GSourceFunc) _cairo_dock_animate_on_mouse_over, pDock);
		}
		pPointedIcon->iMouseOverAnimationCount = g_tNbIterInOneRound[CAIRO_DOCK_ROTATE];
		pPointedIcon->iMouseOverAnimationCount = ROTATION_COUNT;
	}
}
static gboolean _cairo_dock_blur_fading (CairoDock *pDock)
{
	g_print ("%s ()\n", __func__);
	
	if (CAIRO_DOCK_IN_MOVMENT (pDock))
		return TRUE;
	
	gtk_widget_queue_draw (pDock->pWidget);
	pDock->iBlurCount --;
	g_print ("blur <- %d\n", pDock->iBlurCount);
	
	if (pDock->iBlurCount <= 0)
	{
		pDock->iBlurCount = 0;
		pDock->iSidBlurFading = 0;
		return FALSE;
	}
	return TRUE;
}
gboolean on_motion_notify2 (GtkWidget* pWidget,
	GdkEventMotion* pMotion,
	CairoDock *pDock)
{
	static double fLastTime = 0;
	pDock->iBlurCount = 20;
	if (pDock->iSidBlurFading != 0)
	{
		//g_source_remove (pDock->iSidBlurFading);
		//pDock->iSidBlurFading = 0;
	}
	else
	{
		//pDock->iBlurCount = 5;
		pDock->iSidBlurFading = g_timeout_add (50, _cairo_dock_blur_fading, pDock);
	}
	Icon *pPointedIcon, *pLastPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	int iLastMouseX = pDock->iMouseX;
	//g_print ("pDock->fAvoidingMouseMargin : %.2f\n", pDock->fAvoidingMouseMargin);
	
	//\_______________ On elague le flux des MotionNotify, sinon X en envoie autant que le permet le CPU !
	if (pMotion != NULL)
	{
		//g_print ("%s (%d,%d) (%d, %.2fms, bAtBottom:%d; iSidShrinkDown:%d)\n", __func__, (int) pMotion->x, (int) pMotion->y, pMotion->is_hint, pMotion->time - fLastTime, pDock->bAtBottom, pDock->iSidShrinkDown);
		if ((pMotion->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) && (pMotion->state & GDK_BUTTON1_MASK))
		{
			//g_print ("mouse : (%d;%d); pointeur : (%d;%d)\n", pDock->iMouseX, pDock->iMouseY, (int) pMotion->x_root, (int) pMotion->y_root);
			if (pDock->bHorizontalDock)
			{
				//gtk_window_get_position (GTK_WINDOW (pDock->pWidget), &pDock->iWindowPositionX, &pDock->iWindowPositionY);
				pDock->iWindowPositionX = pMotion->x_root - pDock->iMouseX;
				pDock->iWindowPositionY = pMotion->y_root - pDock->iMouseY;
				gtk_window_move (GTK_WINDOW (pWidget),
					pDock->iWindowPositionX,
					pDock->iWindowPositionY);
			}
			else
			{
				pDock->iWindowPositionX = pMotion->y_root - pDock->iMouseX;
				pDock->iWindowPositionY = pMotion->x_root - pDock->iMouseY;
				gtk_window_move (GTK_WINDOW (pWidget),
					pDock->iWindowPositionY,
					pDock->iWindowPositionX);
			}
			gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);
			return FALSE;
		}

		if (pDock->bHorizontalDock)
		{
			pDock->iMouseX = (int) pMotion->x;
			pDock->iMouseY = (int) pMotion->y;
		}
		else
		{
			pDock->iMouseX = (int) pMotion->y;
			pDock->iMouseY = (int) pMotion->x;
		}

		if (pDock->iSidShrinkDown > 0 || pMotion->time - fLastTime < g_fRefreshInterval)  // si les icones sont en train de diminuer de taille (suite a un clic) on on laisse l'animation se finir, sinon elle va trop vite.  // || ! pDock->bInside || pDock->bAtBottom
		{
			gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);
			return FALSE;
		}

		//\_______________ On recalcule toutes les icones.
		pPointedIcon = pDock->calculate_icons (pDock);
		gtk_widget_queue_draw (pWidget);
		fLastTime = pMotion->time;

		if (s_pIconClicked != NULL && pDock->iAvoidingMouseIconType == -1)
		{
			s_pIconClicked->iAnimationType = CAIRO_DOCK_FOLLOW_MOUSE;
			pDock->iAvoidingMouseIconType = s_pIconClicked->iType;  // on pourrait le faire lors du clic aussi.
			pDock->fAvoidingMouseMargin = .5;
		}

		//gdk_event_request_motions (pMotion);  // ce sera pour GDK 2.12.
		gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);  // pour recevoir d'autres MotionNotify.
	}
	else  // cas d'un drag and drop.
	{
		//g_print ("motion on drag\n");
		if (pDock->bHorizontalDock)
 			gdk_window_get_pointer (pWidget->window, &pDock->iMouseX, &pDock->iMouseY, NULL);
		else
			gdk_window_get_pointer (pWidget->window, &pDock->iMouseY, &pDock->iMouseX, NULL);

		if (pDock->iSidShrinkDown > 0)  // si les icones sont en train de diminuer de taille (suite a un clic) on on laisse l'animation se finir, sinon elle va trop vite.  // || ! pDock->bInside || pDock->bAtBottom
		{
			//gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);
			return FALSE;
		}

		pPointedIcon = pDock->calculate_icons (pDock);
		pDock->iAvoidingMouseIconType = CAIRO_DOCK_LAUNCHER;
		pDock->fAvoidingMouseMargin = .25;
		
		gtk_widget_queue_draw (pWidget);
	}

	if (g_bDecorationsFollowMouse)
	{
		pDock->fDecorationsOffsetX = pDock->iMouseX - pDock->iCurrentWidth / 2;
		//g_print ("fDecorationsOffsetX <- %.2f\n", pDock->fDecorationsOffsetX);
	}
	else
	{
		if (pDock->iMouseX > iLastMouseX)
		{
			pDock->fDecorationsOffsetX += 10;
			if (pDock->fDecorationsOffsetX > pDock->iCurrentWidth / 2)
			{
				if (g_pBackgroundSurfaceFull[0] != NULL)
					pDock->fDecorationsOffsetX -= pDock->iCurrentWidth;
				else
					pDock->fDecorationsOffsetX = pDock->iCurrentWidth / 2;
			}
		}
		else
		{
			pDock->fDecorationsOffsetX -= 10;
			if (pDock->fDecorationsOffsetX < - pDock->iCurrentWidth / 2)
			{
				if (g_pBackgroundSurfaceFull[0] != NULL)
					pDock->fDecorationsOffsetX += pDock->iCurrentWidth;
				else
					pDock->fDecorationsOffsetX = - pDock->iCurrentWidth / 2;
			}
		}
	}
	
	//g_print ("%x -> %x\n", pLastPointedIcon, pPointedIcon);
	if (pPointedIcon != pLastPointedIcon || s_pLastPointedDock == NULL)
	{
		cairo_dock_on_change_icon (pLastPointedIcon, pPointedIcon, pDock);
	}
	
	/*if (pDock->iSidBlurFading == 0)
	{
		pDock->iBlurCount = 5;
		pDock->iSidBlurFading = g_timeout_add (35, _cairo_dock_blur_fading, pDock);
	}*/
	
	return FALSE;
}

gboolean cairo_dock_emit_signal_on_dock (CairoDock *pDock, const gchar *cSignal)
{
	static gboolean bReturn;
	//g_print ("demande de quitter\n");
	g_signal_emit_by_name (pDock->pWidget, cSignal, NULL, &bReturn);
	return FALSE;
}
gboolean cairo_dock_emit_leave_signal (CairoDock *pDock)
{
	cairo_dock_emit_signal_on_dock (pDock, "leave-notify-event");
}
gboolean cairo_dock_emit_enter_signal (CairoDock *pDock)
{
	cairo_dock_emit_signal_on_dock (pDock, "enter-notify-event");
}

void cairo_dock_leave_from_main_dock (CairoDock *pDock)
{
	//g_print ("%s (iSidShrinkDown : %d, %d)\n", __func__, pDock->iSidShrinkDown, pDock->bMenuVisible);
	pDock->iAvoidingMouseIconType = -1;
	pDock->fAvoidingMouseMargin = 0;
	pDock->bInside = FALSE;
	pDock->bAtTop = FALSE;
	
	if (pDock->bMenuVisible)
	{
		return ;
	}
	/**if (g_bPopUp && pDock->bIsMainDock)
	{
		//the mouse has exited the dock window, cancel any pop up event, and trigger a pop down event.
		*if (pDock->iSidPopUp != 0)
		{
			g_source_remove(pDock->iSidPopUp);
			pDock->iSidPopUp = 0;
		}
		if (pDock->iSidPopDown == 0)
			pDock->iSidPopDown = g_timeout_add (500, (GSourceFunc) cairo_dock_pop_down, (gpointer) pDock);
	}*/
	if (pDock->iSidMoveUp != 0)  // si on est en train de monter, on arrete.
	{
		g_source_remove (pDock->iSidMoveUp);
		pDock->iSidMoveUp = 0;
	}
	if (pDock->iSidGrowUp != 0)  // si on est en train de faire grossir les icones, on arrete.
	{
		pDock->fFoldingFactor = 0;
		g_source_remove (pDock->iSidGrowUp);
		pDock->iSidGrowUp = 0;
	}

	if (pDock->iRefCount == 0)
	{
		if (pDock->bAutoHide)
		{
			pDock->fFoldingFactor = (g_bAnimateOnAutoHide && g_fUnfoldAcceleration != 0. ? 0.03 : 0.);
			if (pDock->iSidMoveDown == 0)  // on commence a descendre.
				pDock->iSidMoveDown = g_timeout_add (40, (GSourceFunc) cairo_dock_move_down, (gpointer) pDock);
		}
		else
			pDock->bAtBottom = TRUE;
	}
	else
	{
		pDock->fFoldingFactor = 0.03;
		pDock->bAtBottom = TRUE;  // mis en commentaire le 12/11/07 pour permettre le quick-hide.
		cd_debug ("on force bAtBottom");
	}

	///pDock->fDecorationsOffsetX = 0;
	if (pDock->iSidShrinkDown == 0)  // on commence a faire diminuer la taille des icones.
		pDock->iSidShrinkDown = g_timeout_add (40, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);

	//s_pLastPointedDock = NULL;
	//g_print ("s_pLastPointedDock <- NULL\n");
}
gboolean on_leave_notify2 (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDock *pDock)
{
	//g_print ("%s (bInside:%d; bAtBottom:%d; iRefCount:%d)\n", __func__, pDock->bInside, pDock->bAtBottom, pDock->iRefCount);
	/**if (pDock->bAtBottom)  // || ! pDock->bInside  // mis en commentaire pour la 1.5.4
	{
		pDock->iSidLeaveDemand = 0;
		return FALSE;
	}*/
	if (pEvent != NULL && (pEvent->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) && (pEvent->state & GDK_BUTTON1_MASK))
	{
		return FALSE;
	}
	//g_print ("%s (main dock : %d)\n", __func__, pDock->bIsMainDock);

	if (pDock->iRefCount == 0)
	{
		Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
		if (pPointedIcon != NULL && pPointedIcon->pSubDock != NULL)
		{
			if (pDock->iSidLeaveDemand == 0)
			{
				//g_print ("  on retarde la sortie du dock de %dms\n", MAX (g_iLeaveSubDockDelay, 330));
				pDock->iSidLeaveDemand = g_timeout_add (MAX (g_iLeaveSubDockDelay, 330), (GSourceFunc) cairo_dock_emit_leave_signal, (gpointer) pDock);
				return TRUE;
			}
		}
	}
	else  // pEvent != NULL
	{
		if (pDock->iSidLeaveDemand == 0 && g_iLeaveSubDockDelay != 0)
		{
			//g_print ("  on retarde la sortie du sous-dock de %dms\n", g_iLeaveSubDockDelay);
			pDock->iSidLeaveDemand = g_timeout_add (g_iLeaveSubDockDelay, (GSourceFunc) cairo_dock_emit_leave_signal, (gpointer) pDock);
			return TRUE;
		}
	}
	pDock->iSidLeaveDemand = 0;

	if (s_iSidNonStopScrolling > 0)
	{
		g_source_remove (s_iSidNonStopScrolling);
		s_iSidNonStopScrolling = 0;
	}

	pDock->bInside = FALSE;
	//cd_debug (" on attend...");
	while (gtk_events_pending ())  // on laisse le temps au signal d'entree dans le sous-dock d'etre traite.
		gtk_main_iteration ();
	//cd_debug (" ==> pDock->bInside : %d", pDock->bInside);

	if (pDock->bInside)  // on est re-rentre dedans entre-temps.
		return TRUE;

	if (! cairo_dock_hide_child_docks (pDock))  // on quitte si on entre dans un sous-dock, pour rester en position "haute".
		return TRUE;

	cairo_dock_leave_from_main_dock (pDock);

	return TRUE;
}

/// This function checks for the mouse cursor's position. If the mouse
/// cursor touches the edge of the screen upon which the dock is resting,
/// then the dock will pop up over other windows...
gboolean cairo_dock_poll_screen_edge (CairoDock *pDock)  // thanks to Smidgey for the pop-up patch !
{
	static int iPrevPointerX = -1, iPrevPointerY = -1;
	gint iMousePosX, iMousePosY;
	///static gint iSidPopUp = 0;
	
	if (pDock->iSidPopUp == 0 && !pDock->bPopped)
	{
		gdk_display_get_pointer(gdk_display_get_default(), NULL, &iMousePosX, &iMousePosY, NULL);
		if (iPrevPointerX == iMousePosX && iPrevPointerY == iMousePosY)
			return g_bPopUp;
		
		iPrevPointerX = iMousePosX;
		iPrevPointerY = iMousePosY;
		
		CairoDockPositionType iScreenBorder = 0;
		if (iMousePosY == 0)
			iScreenBorder = CAIRO_DOCK_TOP;
		else if (iMousePosY + 1 == g_iScreenHeight[CAIRO_DOCK_HORIZONTAL])
			iScreenBorder = CAIRO_DOCK_BOTTOM;
		else if (iMousePosX == 0)
			iScreenBorder = CAIRO_DOCK_LEFT;
		else if (iMousePosX + 1 == g_iScreenWidth[CAIRO_DOCK_HORIZONTAL])
			iScreenBorder = CAIRO_DOCK_RIGHT;
		else
			return g_bPopUp;
		cairo_dock_pop_up_root_docks_on_screen_edge (iScreenBorder);
	}
	
	return g_bPopUp;
}

gboolean on_enter_notify2 (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	CairoDock *pDock)
{
	//g_print ("%s (bIsMainDock : %d; bAtTop:%d; bInside:%d; iSidMoveDown:%d; iMagnitudeIndex:%d)\n", __func__, pDock->bIsMainDock, pDock->bAtTop, pDock->bInside, pDock->iSidMoveDown, pDock->iMagnitudeIndex);
	s_pLastPointedDock = NULL;  // ajoute le 04/10/07 pour permettre aux sous-docks d'apparaitre si on entre en pointant tout de suite sur l'icone.
	if (! cairo_dock_entrance_is_allowed (pDock))
	{
		cd_message ("* entree non autorisee");
		return FALSE;
	}

	if (pDock->iSidLeaveDemand != 0)
	{
		g_source_remove (pDock->iSidLeaveDemand);
		pDock->iSidLeaveDemand = 0;
	}

	if (pDock->bAtTop || pDock->bInside || (pDock->iSidMoveDown != 0))  // le 'iSidMoveDown != 0' est la pour empecher le dock de "vibrer" si l'utilisateur sort par en bas avec l'auto-hide active.
	{
		//g_print ("  %d;%d;%d\n", pDock->bAtTop,  pDock->bInside, pDock->iSidMoveDown);
		return FALSE;
	}
	//g_print ("%s (main dock : %d ; %d)\n", __func__, pDock->bIsMainDock, pDock->bHorizontalDock);

	pDock->fDecorationsOffsetX = 0;
	if (! pDock->bIsMainDock)
	{
		gtk_window_present (GTK_WINDOW (pWidget));
	}
	pDock->bInside = TRUE;
	//cairo_dock_deactivate_temporary_auto_hide ();  // se desactive tout seul.

	if (s_pIconClicked != NULL)  // on pourrait le faire a chaque motion aussi.
	{
		pDock->iAvoidingMouseIconType = s_pIconClicked->iType;
		pDock->fAvoidingMouseMargin = .5;
	}

	int iNewWidth, iNewHeight;
	cairo_dock_get_window_position_and_geometry_at_balance (pDock, CAIRO_DOCK_MAX_SIZE, &iNewWidth, &iNewHeight);
	if ((pDock->bAutoHide && pDock->iRefCount == 0) && pDock->bAtBottom)
		pDock->iWindowPositionY = (pDock->bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - g_iVisibleZoneHeight - pDock->iGapY : g_iVisibleZoneHeight + pDock->iGapY - pDock->iMaxDockHeight);

	if (pDock->bHorizontalDock)
		gdk_window_move_resize (pWidget->window,
			pDock->iWindowPositionX,
			pDock->iWindowPositionY,
			iNewWidth,
			iNewHeight);
	else
		gdk_window_move_resize (pWidget->window,
			pDock->iWindowPositionY,
			pDock->iWindowPositionX,
			iNewHeight,
			iNewWidth);
	
	if (pDock->iSidMoveDown > 0)  // si on est en train de descendre, on arrete.
	{
		//g_print ("  on est en train de descendre, on arrete\n");
		g_source_remove (pDock->iSidMoveDown);
		pDock->iSidMoveDown = 0;
	}
	/*if (g_iSidShrinkDown > 0)  // si on est en train de faire diminuer la tailler des icones, on arrete.
	{
		g_source_remove (g_iSidShrinkDown);
		g_iSidShrinkDown = 0;
	}*/
	
	if (g_bPopUp && pDock->iRefCount == 0)
	{
		//This code will trigger a pop up...
		/**if (pDock->iSidPopUp == 0)
			pDock->iSidPopUp = g_timeout_add (500, (GSourceFunc) cairo_dock_pop_up, (gpointer) pDock);*/
		cairo_dock_pop_up (pDock);
		//If the dock window is entered, and there is a pending
		//drop below event then it should be cancelled
		if (pDock->iSidPopDown != 0)
		{
			g_source_remove(pDock->iSidPopDown);
			pDock->iSidPopDown = 0;
		}
	}
	
	if (pDock->bAutoHide && pDock->iRefCount == 0)
	{
		//g_print ("  on commence a monter\n");
		if (pDock->iSidMoveUp == 0)  // on commence a monter.
			pDock->iSidMoveUp = g_timeout_add (40, (GSourceFunc) cairo_dock_move_up, (gpointer) pDock);
	}
	else
	{
		if (pDock->iRefCount > 0)
			pDock->bAtTop = TRUE;
		pDock->bAtBottom = FALSE;
	}
	if (pDock->iSidGrowUp == 0 && pDock->iSidShrinkDown == 0)  // on commence a faire grossir les icones, sinon on laisse l'animation se finir.
	{
		pDock->iSidGrowUp = g_timeout_add (40, (GSourceFunc) cairo_dock_grow_up, (gpointer) pDock);
	}

	return FALSE;
}


static int iMoveByArrow = 0;
gboolean on_key_release (GtkWidget *pWidget,
	GdkEventKey *pKey,
	CairoDock *pDock)
{
	cd_message ("");
	iMoveByArrow = 0;
	if (pKey->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK))  // On relache la touche ALT, typiquement apres avoir fait un ALT + clique gauche + deplacement.
	{
		if (pDock->iRefCount == 0)
			cairo_dock_write_root_dock_gaps (pDock);
	}
	return FALSE;
}


static int _move_up_by_arrow (int iMoveByArrow, CairoDock *pDock)
{
	int iPossibleMove = MAX (0, pDock->iWindowPositionY);
	int iEffectiveMove = MIN (iMoveByArrow, iPossibleMove);
	//g_print ("%s () : iPossibleMove=%d->%d\n", __func__, iPossibleMove, iEffectiveMove);
	if (iEffectiveMove > 0)
	{
		pDock->iWindowPositionY -= iEffectiveMove;
		pDock->iGapY += (pDock->bDirectionUp ? iEffectiveMove : -iEffectiveMove);
	}
	return iEffectiveMove;
}
static int _move_down_by_arrow (int iMoveByArrow, CairoDock *pDock)
{
	int iPossibleMove = MAX (0, g_iScreenHeight[pDock->bHorizontalDock] - (pDock->iWindowPositionY + pDock->iCurrentHeight));
	int iEffectiveMove = MIN (iMoveByArrow, iPossibleMove);
	//g_print ("%s () : iPossibleMove=%d->%d\n", __func__, iPossibleMove, iEffectiveMove);
	if (iEffectiveMove > 0)
	{
		pDock->iWindowPositionY += iEffectiveMove;
		pDock->iGapY += (pDock->bDirectionUp ? -iEffectiveMove : iEffectiveMove);
	}
	return iEffectiveMove;
}
static int _move_left_by_arrow (int iMoveByArrow, CairoDock *pDock)
{
	int iPossibleMove = MAX (0, pDock->iWindowPositionX);
	int iEffectiveMove = MIN (iMoveByArrow, iPossibleMove);
	if (iEffectiveMove > 0)
	{
		pDock->iWindowPositionX -= iEffectiveMove;
		pDock->iGapX -= iEffectiveMove;
	}
	return iEffectiveMove;
}
static int _move_right_by_arrow (int iMoveByArrow, CairoDock *pDock)
{
	int iPossibleMove = MAX (0, g_iScreenWidth[pDock->bHorizontalDock] - (pDock->iWindowPositionX + pDock->iCurrentWidth));
	int iEffectiveMove = MIN (iMoveByArrow, iPossibleMove);
	if (iEffectiveMove > 0)
	{
		pDock->iWindowPositionX += iEffectiveMove;
		pDock->iGapX += iEffectiveMove;
	}
	return iEffectiveMove;
}
gboolean on_key_press (GtkWidget *pWidget,
	GdkEventKey *pKey,
	CairoDock *pDock)
{
	cd_message ("");
	if (pKey->type == GDK_KEY_PRESS)
	{
		GdkEventScroll dummyScroll;
		int iX, iY;
		switch (pKey->keyval)
		{
			case GDK_Down :
				if (pKey->state & GDK_CONTROL_MASK)
					iMoveByArrow = (pDock->bHorizontalDock ? _move_down_by_arrow (++iMoveByArrow, pDock) : _move_right_by_arrow (++iMoveByArrow, pDock));
			break;

			case GDK_Up :
				if (pKey->state & GDK_CONTROL_MASK)
					iMoveByArrow = (pDock->bHorizontalDock ? _move_up_by_arrow (++iMoveByArrow, pDock) : _move_left_by_arrow (++iMoveByArrow, pDock));
			break;

			case GDK_Left :
				if (pKey->state & GDK_CONTROL_MASK)
					iMoveByArrow = (pDock->bHorizontalDock ? _move_left_by_arrow (++iMoveByArrow, pDock) : _move_up_by_arrow (++iMoveByArrow, pDock));
			break;

			case GDK_Right :
				if (pKey->state & GDK_CONTROL_MASK)
					iMoveByArrow = (pDock->bHorizontalDock ? _move_right_by_arrow (++iMoveByArrow, pDock) : _move_down_by_arrow (++iMoveByArrow, pDock));
			break;

			case GDK_Page_Up :
				dummyScroll.direction = GDK_SCROLL_UP;
				gdk_window_get_pointer (pWidget->window, &iX, &iY, NULL);
				dummyScroll.x = iX;
				dummyScroll.y = iY;
				dummyScroll.time = pKey->time;
				dummyScroll.state = pKey->state;
				on_scroll (pWidget,
					&dummyScroll,
					pDock);
			break;

			case GDK_Page_Down:
				dummyScroll.direction = GDK_SCROLL_DOWN;
				gdk_window_get_pointer (pWidget->window, &iX, &iY, NULL);
				dummyScroll.x = iX;
				dummyScroll.y = iY;
				dummyScroll.time = pKey->time;
				dummyScroll.state = pKey->state;
				on_scroll (pWidget,
					&dummyScroll,
					pDock);
			break;
		}
	}

	if (iMoveByArrow > 0)
	{
		if (pDock->bHorizontalDock)
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionX, pDock->iWindowPositionY);
		else
			gtk_window_move (GTK_WINDOW (pDock->pWidget), pDock->iWindowPositionY, pDock->iWindowPositionX);
		if (pDock->bIsMainDock)
			cairo_dock_update_conf_file_with_position (g_cConfFile, pDock->iGapX, pDock->iGapY);
	}

	return FALSE;
}


gboolean cairo_dock_launch_command_full (const gchar *cCommandFormat, gchar *cWorkingDirectory, ...)
{
	g_return_val_if_fail (cCommandFormat != NULL, FALSE);
	
	va_list args;
	va_start (args, cWorkingDirectory);
	gchar *cCommand = g_strdup_vprintf (cCommandFormat, args);
	cd_debug ("%s (%s , %s)", __func__, cCommand, cWorkingDirectory);
	
	GError *erreur = NULL;
	int argc;
	gchar **argv = NULL;
	g_shell_parse_argv (cCommand,
		&argc,
		&argv,
		&erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		g_free (cCommand);
		va_end (args);
		return FALSE;
	}
	
	GPid iChildPID;
	g_spawn_async (cWorkingDirectory,
		argv,
		NULL,  // env
		G_SPAWN_SEARCH_PATH,
		NULL,
		NULL,
		&iChildPID,
		&erreur);
	g_strfreev (argv);
	if (erreur != NULL)
	{
		cd_warning ("Attention : when trying to execute '%s' : %s", cCommand, erreur->message);
		g_error_free (erreur);
		g_free (cCommand);
		va_end (args);
		return FALSE;
	}
	
	g_free (cCommand);
	va_end (args);
	return TRUE;
}

gboolean cairo_dock_notification_click_icon (gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	guint iButtonState = GPOINTER_TO_INT (data[2]);
	
	if (CAIRO_DOCK_IS_URI_LAUNCHER (icon))
	{
		gboolean bIsMounted = FALSE;
		if (icon->iVolumeID > 0)
		{
			gchar *cActivationURI = cairo_dock_fm_is_mounted (icon->acCommand, &bIsMounted);
			g_free (cActivationURI);
		}
		if (icon->iVolumeID > 0 && ! bIsMounted)
		{
			int answer = cairo_dock_ask_question_and_wait (_("Do you want to mount this point ?"), icon, CAIRO_CONTAINER (pDock));
			if (answer != GTK_RESPONSE_YES)
			{
				icon->iCount = 0;
				return CAIRO_DOCK_LET_PASS_NOTIFICATION;
			}
			cairo_dock_fm_mount (icon, CAIRO_CONTAINER (pDock));
		}
		else
			cairo_dock_fm_launch_uri (icon->acCommand);
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	else if (CAIRO_DOCK_IS_APPLI (icon) && ! ((iButtonState & GDK_SHIFT_MASK) && CAIRO_DOCK_IS_LAUNCHER (icon)))
	{
		{
			if (cairo_dock_get_active_window () == icon->Xid && g_bMinimizeOnClick)  // ne marche que si le dock est une fenêtre de type 'dock', sinon il prend le focus.
				cairo_dock_minimize_xwindow (icon->Xid);
			else
				cairo_dock_show_xwindow (icon->Xid);
		}
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	else if (CAIRO_DOCK_IS_LAUNCHER (icon))
	{
		if (icon->acCommand != NULL)
		{
			gboolean bSuccess = FALSE;
			if (*icon->acCommand == '<')
			{
				bSuccess = cairo_dock_simulate_key_sequence (icon->acCommand);
				if (!bSuccess)
					bSuccess = cairo_dock_launch_command_full (icon->acCommand, icon->cWorkingDirectory);
			}
			else
			{
				bSuccess = cairo_dock_launch_command_full (icon->acCommand, icon->cWorkingDirectory);
				if (! bSuccess)
					bSuccess = cairo_dock_simulate_key_sequence (icon->acCommand);
			}
			if (bSuccess)
			{
				if (CAIRO_DOCK_IS_APPLI (icon))  // on remet l'animation du lanceur.
					cairo_dock_arm_animation_by_type (icon, CAIRO_DOCK_LAUNCHER);
			}
			else
				cairo_dock_arm_animation (icon, CAIRO_DOCK_BLINK, 1);  // 1 clignotement si echec.
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		}
		else
		{
			icon->iCount = 0;
		}
	}
	else if (icon != NULL)
	{
		cd_message ("No known action");
		icon->iCount = 0;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cairo_dock_notification_middle_click_icon (gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];

	if (CAIRO_DOCK_IS_APPLI (icon) && g_bCloseAppliOnMiddleClick)
	{
		cairo_dock_close_xwindow (icon->Xid);
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	if (CAIRO_DOCK_IS_URI_LAUNCHER (icon) && icon->pSubDock != NULL)  // icone de repertoire.
	{
		cairo_dock_fm_launch_uri (icon->acCommand);
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean on_button_press2 (GtkWidget* pWidget,
	GdkEventButton* pButton,
	CairoDock *pDock)
{
	//g_print ("%s (%d/%d)\n", __func__, pButton->type, pButton->button);
	if (pDock->bHorizontalDock)  // utile ?
	{
		pDock->iMouseX = (int) pButton->x;
		pDock->iMouseY = (int) pButton->y;
	}
	else
	{
		pDock->iMouseX = (int) pButton->y;
		pDock->iMouseY = (int) pButton->x;
	}

	Icon *icon = cairo_dock_get_pointed_icon (pDock->icons);
	if (pButton->button == 1)  // clic gauche.
	{
		switch (pButton->type)
		{
			case GDK_BUTTON_RELEASE :
				if ( ! (pButton->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)))
				{
					if (s_pIconClicked != NULL)
					{
						cd_message ("release de %s (inside:%d)", s_pIconClicked->acName, pDock->bInside);
						s_pIconClicked->iAnimationType = 0;  // stoppe les animations de suivi du curseur.
						s_pIconClicked->iCount = 0;  // precaution.
						cairo_dock_stop_marking_icons (pDock);
						pDock->iAvoidingMouseIconType = -1;
					}
					if (icon != NULL && ! CAIRO_DOCK_IS_SEPARATOR (icon) && icon == s_pIconClicked)
					{
						cairo_dock_arm_animation (icon, -1, -1);

						if (icon->pSubDock != NULL && bShowSubDockOnClick && ! CAIRO_DOCK_IS_APPLI (icon) && ! (pButton->state & GDK_SHIFT_MASK))  // icone de sous-dock.
						{
							cairo_dock_show_subdock (icon, FALSE, pDock);
							cairo_dock_arm_animation (icon, 0, 0);
						}
						else
						{
							gpointer data[3] = {icon, pDock, GINT_TO_POINTER (pButton->state)};
							cairo_dock_notify (CAIRO_DOCK_CLICK_ICON, data);
							if (g_cRaiseDockShortcut != NULL)
								s_bHideAfterShortcut = TRUE;
							
							cairo_dock_start_animation (icon, pDock);
						}
					}
					else if (s_pIconClicked != NULL && icon != NULL && icon != s_pIconClicked)  //  && icon->iType == s_pIconClicked->iType
					{
						cd_message ("deplacement de %s", s_pIconClicked->acName);
						CairoDock *pOriginDock = CAIRO_DOCK (cairo_dock_search_container_from_icon (s_pIconClicked));
						if (pOriginDock != NULL && pDock != pOriginDock)
						{
							cairo_dock_detach_icon_from_dock (s_pIconClicked, pOriginDock, TRUE);  // plutot que 'cairo_dock_remove_icon_from_dock', afin de ne pas la fermer.
							///cairo_dock_remove_icon_from_dock (pOriginDock, s_pIconClicked);
							cairo_dock_update_dock_size (pOriginDock);

							///s_pIconClicked->fWidth /= (pOriginDock->iRefCount == 0 ? 1. : g_fSubDockSizeRatio);
							///s_pIconClicked->fHeight /= (pOriginDock->iRefCount == 0 ? 1. : g_fSubDockSizeRatio);
							cairo_dock_update_icon_s_container_name (s_pIconClicked, icon->cParentDockName);
							if (pOriginDock->iRefCount > 0 && ! g_bSameHorizontality)
							{
								cairo_t* pSourceContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
								cairo_dock_fill_one_text_buffer (s_pIconClicked, pSourceContext, &g_iconTextDescription, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : g_pMainDock->bHorizontalDock), g_pMainDock->bDirectionUp);
								cairo_destroy (pSourceContext);
							}

							cairo_dock_insert_icon_in_dock (s_pIconClicked, pDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
						}

						Icon *prev_icon, *next_icon;
						if (pDock->iMouseX > icon->fX + icon->fWidth * icon->fScale / 2)
						{
							prev_icon = icon;
							next_icon = cairo_dock_get_next_icon (pDock->icons, icon);
						}
						else
						{
							prev_icon = cairo_dock_get_previous_icon (pDock->icons, icon);
							next_icon = icon;
						}
						if ((prev_icon == NULL || prev_icon->iType != s_pIconClicked->iType) && (next_icon == NULL || next_icon->iType != s_pIconClicked->iType))
						{
							s_pIconClicked = NULL;
							return FALSE;
						}
						//g_print ("deplacement de %s\n", s_pIconClicked->acName);
						if (prev_icon != NULL && prev_icon->iType != s_pIconClicked->iType)
							prev_icon = NULL;
						cairo_dock_move_icon_after_icon (pDock, s_pIconClicked, prev_icon);

						pDock->calculate_icons (pDock);
						gtk_widget_queue_draw (pWidget);

						if (! CAIRO_DOCK_IS_SEPARATOR (s_pIconClicked))
						{
							cairo_dock_arm_animation (s_pIconClicked, CAIRO_DOCK_BOUNCE, 2);  // 2 rebonds.
							cairo_dock_start_animation (s_pIconClicked, pDock);
						}
					}
				}
				else
				{
					if (pDock->iRefCount == 0)
						cairo_dock_write_root_dock_gaps (pDock);
				}
				s_pIconClicked = NULL;
			break ;

			case GDK_BUTTON_PRESS :
				if ( ! (pButton->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)))
				{
					s_pIconClicked = icon;  // on ne definit pas l'animation CAIRO_DOCK_FOLLOW_MOUSE ici , on le fera apres le 1er mouvement, pour eviter que l'icone soit dessinee comme tel quand on clique dessus alors que le dock ets en train de jouer une animation (ca provoque un flash desagreable).
				}
			break ;

			case GDK_2BUTTON_PRESS :
				{
					gpointer data[2] = {icon, pDock};
					cairo_dock_notify (CAIRO_DOCK_DOUBLE_CLICK_ICON, data);
				}
			break ;

			default :
			break ;
		}
	}
	else if (pButton->button == 3 && pButton->type == GDK_BUTTON_PRESS)  // clique droit.
	{
		pDock->bMenuVisible = TRUE;
		GtkWidget *menu = cairo_dock_build_menu (icon, CAIRO_CONTAINER (pDock));  // genere un CAIRO_DOCK_BUILD_MENU.

		gtk_widget_show_all (menu);

		gtk_menu_popup (GTK_MENU (menu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
	}
	else if (pButton->button == 2 && pButton->type == GDK_BUTTON_PRESS)  // clique milieu.
	{
		gpointer data[2] = {icon, pDock};
		cairo_dock_notify (CAIRO_DOCK_MIDDLE_CLICK_ICON, data);
	}

	return FALSE;
}


static gboolean _cairo_dock_autoscroll (gpointer *data)
{
	GdkEventScroll* pScroll = data[0];
	CairoDock *pDock = data[1];
	gboolean bAutoScroll = GPOINTER_TO_INT (data[2]);

	//g_print ("%s (%d, %.2f)\n", __func__, pDock->iSidShrinkDown, pDock->fMagnitude);
	if (pDock->iSidShrinkDown != 0 || pDock->iMagnitudeIndex == 0)
	{
		cairo_dock_set_icons_geometry_for_window_manager (pDock);
		return FALSE;
	}
	
	Icon *pLastPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	Icon *pNeighborIcon;
	if (pScroll->direction == GDK_SCROLL_UP)
	{
		pNeighborIcon = cairo_dock_get_previous_icon (pDock->icons, pLastPointedIcon);
		if (pNeighborIcon == NULL)
			pNeighborIcon = cairo_dock_get_last_icon (pDock->icons);
		pDock->iScrollOffset += (bAutoScroll ? 10 : ((pScroll->state & GDK_CONTROL_MASK) || g_iScrollAmount == 0 ? (pNeighborIcon->fWidth + (pLastPointedIcon != NULL ? pLastPointedIcon->fWidth : 0)) / 2 : g_iScrollAmount));
	}
	else if (pScroll->direction == GDK_SCROLL_DOWN)
	{
		pNeighborIcon = cairo_dock_get_next_icon (pDock->icons, pLastPointedIcon);
		if (pNeighborIcon == NULL)
			pNeighborIcon = cairo_dock_get_first_icon (pDock->icons);
		pDock->iScrollOffset -= (bAutoScroll ? 10 : ((pScroll->state & GDK_CONTROL_MASK) || g_iScrollAmount == 0 ? (pNeighborIcon->fWidth + (pLastPointedIcon != NULL ? pLastPointedIcon->fWidth : 0)) / 2 : g_iScrollAmount));
	}
	else
	{
		//g_print ("stop\n");
		cairo_dock_set_icons_geometry_for_window_manager (pDock);
		return FALSE;
	}

	if (pDock->iScrollOffset >= pDock->fFlatDockWidth)
		pDock->iScrollOffset -= pDock->fFlatDockWidth;
	if (pDock->iScrollOffset < 0)
		pDock->iScrollOffset += pDock->fFlatDockWidth;
	//g_print ("iScrollOffset <- %d, (%d;%d) (%x)\n", pDock->iScrollOffset, (int) pScroll->x, (int) pScroll->y, pDock->icons);

	///cairo_dock_update_dock_size (pDock);  // gourmand en ressources a cause de X.
	pDock->calculate_max_dock_size (pDock);  // recalcule le pFirstDrawnElement.

	//\_______________ On recalcule toutes les icones.
	Icon *pPointedIcon;
	int iX, iY;
	if (bAutoScroll)
	{
		if (pDock->bHorizontalDock)
			gdk_window_get_pointer (pDock->pWidget->window, &iX, &iY, NULL);
		else
			gdk_window_get_pointer (pDock->pWidget->window, &iY, &iX, NULL);
	}
	else
	{
		if (pDock->bHorizontalDock)
		{
			iX = pScroll->x;
			iY = pScroll->y;
		}
		else
		{
			iX = pScroll->y;
			iY = pScroll->x;
		}
	}
	pDock->iMouseX = iX;
	pDock->iMouseY = iY;
	pPointedIcon = pDock->calculate_icons (pDock);
	gtk_widget_queue_draw (pDock->pWidget);

	//\_______________ On montre les sous-docks.
	if (pPointedIcon != pLastPointedIcon || s_pLastPointedDock == NULL)
	{
		//cd_message ("on change d'icone");
		if (pDock == s_pLastPointedDock && pLastPointedIcon != NULL && pLastPointedIcon->pSubDock != NULL)
		{
			if (GTK_WIDGET_VISIBLE (pLastPointedIcon->pSubDock->pWidget))
			{
				///gdk_window_hide (pLastPointedIcon->pSubDock->pWidget->window);
				if (pLastPointedIcon->pSubDock->iSidLeaveDemand == 0)
				{
					//cd_debug ("  on retarde le cachage du dock de %dms", MAX (g_iLeaveSubDockDelay, 330));
					pLastPointedIcon->pSubDock->iSidLeaveDemand = g_timeout_add (MAX (g_iLeaveSubDockDelay, 330), (GSourceFunc) cairo_dock_emit_leave_signal, (gpointer) pLastPointedIcon->pSubDock);
				}
			}
		}
		if (pPointedIcon != NULL && pPointedIcon->pSubDock != NULL && (! bShowSubDockOnClick || CAIRO_DOCK_IS_APPLI (pPointedIcon)))
		{
			if (pPointedIcon->pSubDock->iSidLeaveDemand != 0)
			{
				g_source_remove (pPointedIcon->pSubDock->iSidLeaveDemand);
				pPointedIcon->pSubDock->iSidLeaveDemand = 0;
			}
			if (g_iShowSubDockDelay > 0)
			{
				//pDock->iMouseX = iX;
				s_iSidShowSubDockDemand = g_timeout_add (g_iShowSubDockDelay, (GSourceFunc) _cairo_dock_show_sub_dock_delayed, pDock);
				s_pDockShowingSubDock = pDock;
			}
			else
				cairo_dock_show_subdock (pPointedIcon, TRUE, pDock);
			s_pLastPointedDock = pDock;
		}
		pLastPointedIcon = pPointedIcon;
	}

	return TRUE;
}
gboolean on_scroll (GtkWidget* pWidget,
	GdkEventScroll* pScroll,
	CairoDock *pDock)
{
	static double fLastTime = 0;
	static int iNbSimultaneousScroll = 0;
	static GdkEventScroll scrollBuffer;
	static gpointer data[3] = {&scrollBuffer, NULL, NULL};
	if (pDock->icons == NULL)
		return FALSE;
	
	a += .01 * (pScroll->direction == 1 ? 1 : -1);
	g_print ("a <- %.2f\n", a);
	return FALSE;
	if (pScroll->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK))
	{
		Icon *icon = cairo_dock_get_pointed_icon (pDock->icons);
		if (icon != NULL)
		{
			gpointer data[3] = {icon, pDock, GINT_TO_POINTER (pScroll->direction)};
			cairo_dock_notify (CAIRO_DOCK_SCROLL_ICON, data);
		}
		return FALSE;
	}
	
	//g_print ("%s (%d)\n", __func__, pScroll->direction);
	if (pScroll->time - fLastTime < g_fRefreshInterval && s_iSidNonStopScrolling == 0)
		iNbSimultaneousScroll ++;
	else
		iNbSimultaneousScroll = 0;
	if (iNbSimultaneousScroll == 2 && s_iSidNonStopScrolling == 0)
	{
		cd_message ("on a scrolle comme un bourrinos");
		iNbSimultaneousScroll = -999;
		data[1] = pDock;
		data[2] = GINT_TO_POINTER (1);
		memcpy (&scrollBuffer, pScroll, sizeof (GdkEventScroll));
		s_iSidNonStopScrolling = g_timeout_add (g_fRefreshInterval, (GSourceFunc)_cairo_dock_autoscroll, data);
		return FALSE;
	}

	//g_print ("%d / %d\n", pScroll->direction, scrollBuffer.direction);
	if (s_iSidNonStopScrolling != 0 && pScroll->direction != scrollBuffer.direction)
	{
		//g_print ("on arrete\n");
		g_source_remove (s_iSidNonStopScrolling);
		s_iSidNonStopScrolling = 0;
		iNbSimultaneousScroll = 0;
		return FALSE;
	}

	if (pDock->bAtBottom || ! pDock->bInside || pDock->iSidShrinkDown > 0 || pScroll->time - fLastTime < g_fRefreshInterval)  // si les icones sont en train de diminuer de taille (suite a un clic) on ne redimensionne pas les icones, le temps que l'animation se finisse.
	{
		return FALSE;
	}

	fLastTime = pScroll->time;
	gpointer user_data[3] = {pScroll, pDock, GINT_TO_POINTER (0)};
	_cairo_dock_autoscroll (user_data);

	return FALSE;
}


gboolean on_configure (GtkWidget* pWidget,
	GdkEventConfigure* pEvent,
	CairoDock *pDock)
{
	//g_print ("%s (main dock : %d) : (%d;%d) (%dx%d)\n", __func__, pDock->bIsMainDock, pEvent->x, pEvent->y, pEvent->width, pEvent->height);
	gint iNewWidth, iNewHeight;
	if (pDock->bHorizontalDock)
	{
		iNewWidth = pEvent->width;
		iNewHeight = pEvent->height;
	}
	else
	{
		iNewWidth = pEvent->height;
		iNewHeight = pEvent->width;
	}

	if (iNewWidth != pDock->iCurrentWidth || iNewHeight != pDock->iCurrentHeight)
	{
		//cd_debug ("-> %dx%d", iNewWidth, iNewHeight);
		pDock->iCurrentWidth = iNewWidth;
		pDock->iCurrentHeight = iNewHeight;

		if (pDock->bHorizontalDock)
			gdk_window_get_pointer (pWidget->window, &pDock->iMouseX, &pDock->iMouseY, NULL);
		else
			gdk_window_get_pointer (pWidget->window, &pDock->iMouseY, &pDock->iMouseX, NULL);
		if (pDock->iMouseX < 0 || pDock->iMouseX > pDock->iCurrentWidth)  // utile ?
			pDock->iMouseX = 0;
		
		if (g_bUseOpenGL)
		{
			GdkGLContext* pGlContext = gtk_widget_get_gl_context (pWidget);
			GdkGLDrawable* pGlDrawable = gtk_widget_get_gl_drawable (pWidget);
			GLsizei w = pEvent->width;
			GLsizei h = pEvent->height;
			if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
				return FALSE;
			
			glViewport(0, 0, w, h);
			
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, w, 0, h, 0.0, 500.0);
			//gluPerspective(30.0, 1.0*(GLfloat)w/(GLfloat)h, 1.0, 500.0);
			
			glMatrixMode (GL_MODELVIEW);
			glLoadIdentity ();
			gluLookAt (w/2, h/2, 3.,
				w/2, h/2, 0.,
				0.0f, 1.0f, 0.0f);
			glTranslatef (0.0f, 0.0f, -3.);
			
			glClearAccum (0., 0., 0., 0.);
			glClear (GL_ACCUM_BUFFER_BIT);
			
			gdk_gl_drawable_gl_end (pGlDrawable);
		}
		
#ifdef HAVE_GLITZ
		if (pDock->pGlitzDrawable)
		{
			glitz_drawable_update_size (pDock->pGlitzDrawable,
				pEvent->width,
				pEvent->height);
		}
#endif
		
		pDock->calculate_icons (pDock);
		gtk_widget_queue_draw (pWidget);  // il semble qu'il soit necessaire d'en rajouter un la pour eviter un "clignotement" a l'entree dans le dock.
		//if (pDock->iRefCount > 0 || pDock->bAutoHide)
			while (gtk_events_pending ())  // on force un redessin immediat sinon on a quand meme un "flash".
				gtk_main_iteration ();
	}

	if (pDock->iSidMoveDown == 0 && pDock->iSidMoveUp == 0)  // ce n'est pas du a une animation. Donc en cas d'apparition due a l'auto-hide, ceci ne sera pas fait ici, mais a la fin de l'animation.
	{
		cairo_dock_set_icons_geometry_for_window_manager (pDock);

		cairo_dock_replace_all_dialogs ();
	}
	
	return FALSE;
}


void on_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, CairoDock *pDock)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	//\_________________ On recupere l'URI.
	gchar *cReceivedData = (gchar *) selection_data->data;
	g_return_if_fail (cReceivedData != NULL);
	int length = strlen (cReceivedData);
	if (cReceivedData[length-1] == '\n')
		cReceivedData[--length] = '\0';  // on vire le retour chariot final.
	if (cReceivedData[length-1] == '\r')
		cReceivedData[--length] = '\0';
	
	/*if (pDock->iAvoidingMouseIconType == -1)
	{
		g_print ("drag info : <%s>\n", cReceivedData);
		pDock->iAvoidingMouseIconType = CAIRO_DOCK_LAUNCHER;
		pDock->fAvoidingMouseMargin = .25;
		return ;
	}*/
	
	//\_________________ On arrete l'animation.
	cairo_dock_stop_marking_icons (pDock);
	pDock->iAvoidingMouseIconType = -1;
	pDock->fAvoidingMouseMargin = 0;
	
	//\_________________ On calcule la position a laquelle on l'a lache.
	cd_message (">>> cReceivedData : %s", cReceivedData);
	double fOrder = CAIRO_DOCK_LAST_ORDER;
	Icon *pPointedIcon = NULL, *pNeighboorIcon = NULL;
	GList *ic;
	Icon *icon;
	int iDropX = (pDock->bHorizontalDock ? x : y);
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->bPointed)
		{
			//g_print ("On pointe sur %s\n", icon->acName);
			pPointedIcon = icon;
			double fMargin;
			if (g_str_has_suffix (cReceivedData, ".desktop"))  // si c'est un .desktop, on l'ajoute.
				fMargin = 0.5;  // on ne sera jamais dessus.
			else  // sinon on le lance si on est sur l'icone, et on l'ajoute autrement.
				fMargin = 0.25;

			if (iDropX > icon->fX + icon->fWidth * icon->fScale * (1 - fMargin))  // on est apres.
			{
				pNeighboorIcon = (ic->next != NULL ? ic->next->data : NULL);
				fOrder = (pNeighboorIcon != NULL ? (icon->fOrder + pNeighboorIcon->fOrder) / 2 : icon->fOrder + 1);
			}
			else if (iDropX < icon->fX + icon->fWidth * icon->fScale * fMargin)  // on est avant.
			{
				pNeighboorIcon = (ic->prev != NULL ? ic->prev->data : NULL);
				fOrder = (pNeighboorIcon != NULL ? (icon->fOrder + pNeighboorIcon->fOrder) / 2 : icon->fOrder - 1);
			}
			else  // on est dessus.
			{
				fOrder = CAIRO_DOCK_LAST_ORDER;
			}
		}
	}
	
	cairo_dock_notify_drop_data (cReceivedData, pPointedIcon, fOrder, CAIRO_CONTAINER (pDock));
}

gboolean cairo_dock_notification_drop_data (gpointer *data)
{
	const gchar *cReceivedData = data[0];
	Icon *icon = data[1];
	double fOrder = *((double *) data[2]);
	CairoContainer *pContainer = data[3];
	
	if (! CAIRO_DOCK_IS_DOCK (pContainer))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CairoDock *pDock = CAIRO_DOCK (pContainer);
	if (icon == NULL || CAIRO_DOCK_IS_LAUNCHER (icon) || CAIRO_DOCK_IS_SEPARATOR (icon))
	{
		CairoDock *pReceivingDock = pDock;
		if (g_str_has_suffix (cReceivedData, ".desktop"))  // c'est un fichier .desktop, on choisit de l'ajouter quoiqu'il arrive.
		{
			if (fOrder == CAIRO_DOCK_LAST_ORDER)  // on a lache dessus.
			{
				if (icon->pSubDock != NULL)  // on l'ajoutera au sous-dock.
				{
					pReceivingDock = icon->pSubDock;
				}
			}
		}
		else  // c'est un fichier.
		{
			if (fOrder == CAIRO_DOCK_LAST_ORDER)  // on a lache dessus.
			{
				if (CAIRO_DOCK_IS_LAUNCHER (icon))
				{
					if (CAIRO_DOCK_IS_URI_LAUNCHER (icon))
					{
						if (icon->pSubDock != NULL || icon->iVolumeID != 0)  // on le lache sur un repertoire ou un point de montage.
						{
							cairo_dock_fm_move_into_directory (cReceivedData, icon, pContainer);
							return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
						}
						else  // on le lache sur un fichier.
						{
							return CAIRO_DOCK_LET_PASS_NOTIFICATION;
						}
					}
					else if (icon->pSubDock != NULL)  // on le lache sur un sous-dock de lanceurs.
					{
						pReceivingDock = icon->pSubDock;
					}
					else  // on le lache sur un lanceur.
					{
						gchar *cCommand = g_strdup_printf ("%s '%s'", icon->acCommand, cReceivedData);
						g_spawn_command_line_async (cCommand, NULL);
						g_free (cCommand);
						cairo_dock_arm_animation (icon, CAIRO_DOCK_BLINK, 2);  // 2 clignotements.
						cairo_dock_start_animation (icon, pDock);
						return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
					}
				}
				else  // on le lache sur autre chose qu'un lanceur.
				{
					return CAIRO_DOCK_LET_PASS_NOTIFICATION;
				}
			}
			else  // on a lache a cote.
			{
				Icon *pPointingIcon = cairo_dock_search_icon_pointing_on_dock (pDock, NULL);
				if (CAIRO_DOCK_IS_URI_LAUNCHER (pPointingIcon))  // on a lache dans un dock qui est un repertoire, on copie donc le fichier dedans.
				{
					cairo_dock_fm_move_into_directory (cReceivedData, icon, pContainer);
					return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
				}
			}
		}


		//\_________________ On l'ajoute dans le repertoire des lanceurs du theme courant.
		GError *erreur = NULL;
		const gchar *cDockName = cairo_dock_search_dock_name (pReceivingDock);
		gchar *cNewDesktopFileName = cairo_dock_add_desktop_file_from_uri (cReceivedData, cDockName, fOrder, pDock, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		}

		//\_________________ On charge ce nouveau lanceur.
		if (cNewDesktopFileName != NULL)
		{
			cairo_dock_mark_theme_as_modified (TRUE);

			cairo_t* pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pReceivingDock));
			Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cNewDesktopFileName, pCairoContext);
			g_free (cNewDesktopFileName);
			cairo_destroy (pCairoContext);

			if (pNewIcon != NULL)
			{
				cairo_dock_insert_icon_in_dock (pNewIcon, pReceivingDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);

				if (CAIRO_DOCK_IS_URI_LAUNCHER (pNewIcon))
				{
					cairo_dock_fm_add_monitor (pNewIcon);  // n'est-ce pas trop lourd de rajouter un moniteur sur les fichiers simples ?
				}

				if (pDock->iSidShrinkDown == 0)  // on lance l'animation.
					pDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pDock);
			}
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


void on_drag_motion (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, guint time, CairoDock *pDock)
{
	//g_print ("%s (%dx%d, %d)\n", __func__, x, y, time);
	//\_________________ On simule les evenements souris habituels.
	if (! pDock->bIsDragging)
	{
		cd_message ("start dragging");
		
		/*GdkAtom gdkAtom = gdk_drag_get_selection (dc);
		Atom xAtom = gdk_x11_atom_to_xatom (gdkAtom);
		
		Window Xid = GDK_WINDOW_XID (dc->source_window);
		g_print (" <%s>\n", cairo_dock_get_property_name_on_xwindow (Xid, xAtom));*/
		
		pDock->bIsDragging = TRUE;
		/*pDock->iAvoidingMouseIconType = -1;
		
		GdkAtom target = gtk_drag_dest_find_target (pWidget, dc, NULL);
		if (target == GDK_NONE)
			gdk_drag_status (dc, 0, time);
		else
			gtk_drag_get_data (pWidget, dc, target, time);
		gtk_drag_get_data (pWidget, dc, target, time);
		g_print ("get-data envoye\n");*/
		on_enter_notify2 (pWidget, NULL, pDock);  // ne sera effectif que la 1ere fois a chaque entree dans un dock.
	}
	else
		on_motion_notify2 (pWidget, NULL, pDock);
}

void on_drag_leave (GtkWidget *pWidget, GdkDragContext *dc, guint time, CairoDock *pDock)
{
	cd_message ("stop dragging");
	pDock->bIsDragging = FALSE;
	cairo_dock_stop_marking_icons (pDock);
	pDock->iAvoidingMouseIconType = -1;
	if (pDock->iSidDropIndicator != 0)
	{
		g_source_remove (pDock->iSidDropIndicator);
		pDock->iSidDropIndicator = 0;
	}
	cairo_dock_emit_leave_signal (pDock);
}


gboolean on_delete (GtkWidget *pWidget, GdkEvent *event, CairoDock *pDock)
{
	Icon *pIcon = NULL;
	if (CAIRO_DOCK_IS_DOCK (pDock))
	{
		pIcon = cairo_dock_get_pointed_icon (pDock->icons);
		if (pIcon == NULL)
			pIcon = cairo_dock_get_dialogless_icon ();
	}
	else
	{
		pIcon = CAIRO_DESKLET (pDock)->pIcon;
	}
	int answer = cairo_dock_ask_question_and_wait (_("Quit Cairo-Dock ?"), pIcon, CAIRO_CONTAINER (pDock));
	if (answer == GTK_RESPONSE_YES)
		gtk_main_quit ();
	return FALSE;
}


// Tests sur les selections.
/*void on_selection_get (GtkWidget *pWidget, GtkSelectionData *data, guint info, guint time, gpointer user_data)
{
	cd_message ("***%s ()", __func__);
}

void on_selection_received (GtkWidget *pWidget, GtkSelectionData *data, guint time, gpointer user_data)
{
	cd_message ("***%s ()", __func__);
}

gboolean on_selection_clear_event (GtkWidget *pWidget, GdkEventSelection *event, gpointer user_data)
{
	cd_message ("***%s ()", __func__);
	return FALSE;
}

gboolean on_selection_request_event (GtkWidget *pWidget, GdkEventSelection *event, gpointer user_data)
{
	cd_message ("***%s ()", __func__);
	return FALSE;
}
*/
gboolean on_selection_notify_event (GtkWidget *pWidget, GdkEventSelection *event, gpointer user_data)
{
	g_print ("***%s ()\n", __func__);
	return FALSE;
}


void cairo_dock_show_dock_at_mouse (CairoDock *pDock)
{
	g_return_if_fail (pDock != NULL);
	int iMouseX, iMouseY;
	if (pDock->bHorizontalDock)
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseX, &iMouseY, NULL);
	else
		gdk_window_get_pointer (pDock->pWidget->window, &iMouseY, &iMouseX, NULL);
	//g_print (" %d;%d\n", iMouseX, iMouseY);
	
	pDock->iGapX = pDock->iWindowPositionX + iMouseX - g_iScreenWidth[pDock->bHorizontalDock] * pDock->fAlign;
	pDock->iGapY = (pDock->bDirectionUp ? g_iScreenHeight[pDock->bHorizontalDock] - (pDock->iWindowPositionY + iMouseY) : pDock->iWindowPositionY + iMouseY);
	//g_print (" => %d;%d\n", g_pMainDock->iGapX, g_pMainDock->iGapY);
	
	cairo_dock_set_window_position_at_balance (pDock, pDock->iCurrentWidth, pDock->iCurrentHeight);
	//g_print ("   => (%d;%d)\n", g_pMainDock->iWindowPositionX, g_pMainDock->iWindowPositionY);
	
	gtk_window_move (GTK_WINDOW (pDock->pWidget),
		(pDock->bHorizontalDock ? pDock->iWindowPositionX : pDock->iWindowPositionY),
		(pDock->bHorizontalDock ? pDock->iWindowPositionY : pDock->iWindowPositionX));
	gtk_widget_show (pDock->pWidget);
}

void cairo_dock_raise_from_keyboard (const char *cKeyShortcut, gpointer data)
{
	if (GTK_WIDGET_VISIBLE (g_pMainDock->pWidget))
	{
		gtk_widget_hide (g_pMainDock->pWidget);
	}
	else
	{
		cairo_dock_show_dock_at_mouse (g_pMainDock);
	}
	s_bHideAfterShortcut = FALSE;
}

gboolean cairo_dock_hide_dock_like_a_menu (void)
{
	return s_bHideAfterShortcut;
}

void cairo_dock_has_been_hidden_like_a_menu (void)
{
	s_bHideAfterShortcut = FALSE;
}
