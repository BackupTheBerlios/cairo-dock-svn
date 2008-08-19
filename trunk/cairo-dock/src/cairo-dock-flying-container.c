/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <stdlib.h>
#include <cairo.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-draw.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-file-manager.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-config.h"
#include "cairo-dock-flying-container.h"

#define CAIRO_DOCK_FLYING_WIDTH 72
#define CAIRO_DOCK_FLYING_HEIGHT 122

extern double g_fAmplitude;
extern gchar *g_cCurrentLaunchersPath;

static gboolean on_expose_flying_icon (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoFlyingContainer *pFlyingContainer)
{
	g_print ("%s ()\n", __func__);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pFlyingContainer));
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	if (pFlyingContainer->pIcon != NULL)
	{
		/// dessiner une main et agiter l'icone ...
		cairo_translate (pCairoContext, 
			(pFlyingContainer->iWidth - pFlyingContainer->pIcon->fWidth) / 2,
			(pFlyingContainer->iHeight - pFlyingContainer->pIcon->fHeight) / 2);
		/*cairo_scale (pCairoContext, 1./(1+g_fAmplitude), 1./(1+g_fAmplitude));
		cairo_set_source_surface (pCairoContext, pFlyingContainer->pIcon->pIconBuffer, 0., 0.);
		cairo_paint (pCairoContext);*/
		cairo_dock_render_one_icon (pFlyingContainer->pIcon, pCairoContext, TRUE, 1., 1., FALSE, FALSE, pFlyingContainer->iWidth, TRUE);
	}
	else
	{
		
	}
	cairo_destroy (pCairoContext);
	return FALSE;
}

static gboolean _cairo_dock_animate_flying_icon (CairoFlyingContainer *pFlyingContainer)
{
	if (pFlyingContainer->iFinalCountDown > 0)
	{
		pFlyingContainer->iFinalCountDown --;
		if (pFlyingContainer->iFinalCountDown == 0)
		{
			cairo_dock_free_flying_container (pFlyingContainer);
			return FALSE;
		}
	}
	else
	{
		pFlyingContainer->iAnimationCount ++;
		
		cairo_dock_manage_animations (pFlyingContainer->pIcon, pFlyingContainer);
	}
	gtk_widget_queue_draw (pFlyingContainer->pWidget);
	return TRUE;
}
CairoFlyingContainer *cairo_dock_create_flying_container (Icon *pFlyingIcon, CairoDock *pOriginDock)
{
	CairoFlyingContainer * pFlyingContainer = g_new0 (CairoFlyingContainer, 1);
	pFlyingContainer->iType = CAIRO_DOCK_TYPE_FLYING_CONTAINER;
	GtkWidget* pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	pFlyingContainer->pWidget = pWindow;
	pFlyingContainer->pIcon = pFlyingIcon;
	pFlyingContainer->bIsHorizontal = TRUE;
	pFlyingContainer->bDirectionUp = TRUE;
	
	gtk_window_set_skip_pager_hint(GTK_WINDOW(pWindow), TRUE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pWindow), TRUE);
	cairo_dock_set_colormap_for_window(pWindow);
	gtk_widget_set_app_paintable(pWindow, TRUE);
	gtk_window_set_decorated(GTK_WINDOW(pWindow), FALSE);
	gtk_window_set_resizable(GTK_WINDOW(pWindow), TRUE);
	//gtk_widget_add_events(pWindow, GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
	
	g_signal_connect (G_OBJECT (pWindow),
		"expose-event",
		G_CALLBACK (on_expose_flying_icon),
		pFlyingContainer);
	
	pFlyingContainer->iWidth = CAIRO_DOCK_FLYING_WIDTH;
	pFlyingContainer->iHeight = CAIRO_DOCK_FLYING_HEIGHT;
	pFlyingContainer->iPositionX = pOriginDock->iWindowPositionX + pOriginDock->iMouseX - pFlyingContainer->iWidth/2;
	pFlyingContainer->iPositionY = pOriginDock->iWindowPositionY + pOriginDock->iMouseY - pFlyingContainer->iHeight/2;
	g_print ("(%d;%d) %dx%d\n", pFlyingContainer->iPositionX,
		pFlyingContainer->iPositionY,
		pFlyingContainer->iWidth,
		pFlyingContainer->iHeight);
	/*gdk_window_move_resize (pWindow->window,
		pFlyingContainer->iPositionX,
		pFlyingContainer->iPositionY,
		pFlyingContainer->iWidth,
		pFlyingContainer->iHeight);*/
	gtk_window_resize (GTK_WINDOW (pWindow), 
		pFlyingContainer->iWidth,
		pFlyingContainer->iHeight);
	gtk_window_move (GTK_WINDOW (pWindow), 
		pFlyingContainer->iPositionX,
		pFlyingContainer->iPositionY);
	gtk_window_present (GTK_WINDOW (pWindow));
	
	pFlyingContainer->pIcon->iAnimationType = CAIRO_DOCK_PULSE;
	pFlyingContainer->pIcon->iCount = 1e6;
	pFlyingContainer->pIcon->fDrawX = 0;
	pFlyingContainer->pIcon->fDrawY = 0;
	pFlyingContainer->pIcon->fScale = 1.;
	pFlyingContainer->iSidAnimationTimer = g_timeout_add (50, (GSourceFunc) _cairo_dock_animate_flying_icon, (gpointer) pFlyingContainer);
	return pFlyingContainer;
}

void cairo_dock_drag_flying_container (CairoFlyingContainer *pFlyingContainer, CairoDock *pOriginDock)
{
	pFlyingContainer->iPositionX = pOriginDock->iWindowPositionX + pOriginDock->iMouseX - pFlyingContainer->iWidth/2;
	pFlyingContainer->iPositionY = pOriginDock->iWindowPositionY + pOriginDock->iMouseY - pFlyingContainer->iHeight/2;
	g_print ("  on tire l'icone volante en (%d;%d)\n", pFlyingContainer->iPositionX, pFlyingContainer->iPositionY);
	gtk_window_move (GTK_WINDOW (pFlyingContainer->pWidget),
		pFlyingContainer->iPositionX,
		pFlyingContainer->iPositionY);
}

void cairo_dock_free_flying_container (CairoFlyingContainer *pFlyingContainer)
{
	gtk_widget_destroy (pFlyingContainer->pWidget);  // enleve les signaux.
	g_free (pFlyingContainer);
}

void cairo_dock_terminate_flying_container (CairoFlyingContainer *pFlyingContainer)
{
	Icon *pIcon = pFlyingContainer->pIcon;
	if (pIcon->acDesktopFileName != NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, pIcon->acDesktopFileName);
		g_remove (cIconPath);
		g_free (cIconPath);

		if (CAIRO_DOCK_IS_URI_LAUNCHER (pIcon))
		{
			cairo_dock_fm_remove_monitor (pIcon);
		}
	}
	else if (CAIRO_DOCK_IS_APPLET(pIcon))
	{
		g_print ("le module %s devient un desklet\n", pIcon->pModuleInstance->cConfFilePath);
		cairo_dock_update_conf_file (pIcon->pModuleInstance->cConfFilePath,
			G_TYPE_BOOLEAN, "Desklet", "initially detached", TRUE,
			G_TYPE_INT, "Desklet", "x position", pFlyingContainer->iPositionX,
			G_TYPE_INT, "Desklet", "y position", pFlyingContainer->iPositionY,
			G_TYPE_INVALID);
		
		cairo_dock_reload_module_instance (pIcon->pModuleInstance, TRUE);
	}
	else
	{
		cairo_dock_free_icon (pFlyingContainer->pIcon);
	}
	
	pFlyingContainer->pIcon = NULL;
	pFlyingContainer->iFinalCountDown = 10;
}
