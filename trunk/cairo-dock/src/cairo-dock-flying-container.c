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
#include "cairo-dock-log.h"
#include "cairo-dock-desklet.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-flying-container.h"

#define HAND_WIDTH 76
#define HAND_HEIGHT 50

extern double g_fAmplitude;
extern gchar *g_cCurrentLaunchersPath;

static gboolean on_expose_flying_icon (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoFlyingContainer *pFlyingContainer)
{
	//g_print ("%s ()\n", __func__);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pFlyingContainer));
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	if (pFlyingContainer->pIcon != NULL)
	{
		//pFlyingContainer->pIcon->fScale = (1 + g_fAmplitude);
		cairo_save (pCairoContext);
		cairo_translate (pCairoContext, 
			(pFlyingContainer->iWidth - pFlyingContainer->pIcon->fWidth * pFlyingContainer->pIcon->fScale) / 2,
			pFlyingContainer->iHeight - pFlyingContainer->pIcon->fHeight * pFlyingContainer->pIcon->fScale);
		/*cairo_scale (pCairoContext, 1./(1+g_fAmplitude), 1./(1+g_fAmplitude));
		cairo_set_source_surface (pCairoContext, pFlyingContainer->pIcon->pIconBuffer, 0., 0.);
		cairo_paint (pCairoContext);*/
		cairo_dock_render_one_icon (pFlyingContainer->pIcon, pCairoContext, TRUE, 1., 1., FALSE, FALSE, pFlyingContainer->iWidth, TRUE);
		cairo_restore (pCairoContext);
		
		double fImageWidth, fImageHeight;
		gchar *cImagePath = g_strdup_printf ("%s/%s", CAIRO_DOCK_SHARE_DATA_DIR, "hand.svg");
		cairo_surface_t *pHandSurface = cairo_dock_create_surface_from_image (cImagePath, pCairoContext, 1., pFlyingContainer->iWidth, 0, CAIRO_DOCK_KEEP_RATIO, &fImageWidth, &fImageHeight, NULL, NULL);
		cairo_set_source_surface (pCairoContext, pHandSurface, 0., 0.);
		cairo_paint (pCairoContext);
		
		g_free (cImagePath);
		cairo_surface_destroy (pHandSurface);
	}
	else
	{
		g_print ("compte a rebours : %d\n", pFlyingContainer->iAnimationCount);
		if (pFlyingContainer->iAnimationCount > 0)
		{
			gchar *cImagePath = g_strdup_printf ("%s/%s/%d.png", CAIRO_DOCK_SHARE_DATA_DIR, "explosion", 10+1 - pFlyingContainer->iAnimationCount);
			cairo_surface_t *pExplosionSurface = cairo_dock_create_surface_for_icon (cImagePath, pCairoContext, pFlyingContainer->iWidth, pFlyingContainer->iWidth);
			
			cairo_translate (pCairoContext, 
				0.,
				(pFlyingContainer->iHeight - pFlyingContainer->iWidth) / 2);
			cairo_set_source_surface (pCairoContext, pExplosionSurface, 0., 0.);
			cairo_paint (pCairoContext);
			
			g_free (cImagePath);
			cairo_surface_destroy (pExplosionSurface);
		}
	}
	cairo_destroy (pCairoContext);
	return FALSE;
}

static gboolean _cairo_dock_animate_flying_icon (CairoFlyingContainer *pFlyingContainer)
{
	if (pFlyingContainer->pIcon == NULL)
	{
		pFlyingContainer->iAnimationCount --;
		if (pFlyingContainer->iAnimationCount < 0)
		{
			cairo_dock_free_flying_container (pFlyingContainer);
			return FALSE;
		}
	}
	else
	{
		pFlyingContainer->iAnimationCount ++;
		
		cairo_dock_manage_animations (pFlyingContainer->pIcon, CAIRO_DOCK (pFlyingContainer));  // ici c'est un peu hacky, ca passe car en fait on n'utilise pas les champs du dock.
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
	
	g_print ("pFlyingContainer->pIcon->fScale : %.2f\n", pFlyingContainer->pIcon->fScale);
	pFlyingContainer->pIcon->fScale = 1.;
	pFlyingContainer->iWidth = pFlyingIcon->fWidth * pFlyingContainer->pIcon->fScale * 3.7;
	pFlyingContainer->iHeight = pFlyingIcon->fHeight * pFlyingContainer->pIcon->fScale + 1.*pFlyingContainer->iWidth / HAND_WIDTH * HAND_HEIGHT * .6;
	if (pOriginDock->bHorizontalDock)
	{
		pFlyingContainer->iPositionX = pOriginDock->iWindowPositionX + pOriginDock->iMouseX - pFlyingContainer->iWidth/2;
		pFlyingContainer->iPositionY = pOriginDock->iWindowPositionY + pOriginDock->iMouseY - pFlyingContainer->iHeight/2;
	}
	else
	{
		pFlyingContainer->iPositionY = pOriginDock->iWindowPositionX + pOriginDock->iMouseX - pFlyingContainer->iWidth/2;
		pFlyingContainer->iPositionX = pOriginDock->iWindowPositionY + pOriginDock->iMouseY - pFlyingContainer->iHeight/2;
	}
	/*g_print ("%s (%d;%d %dx%d)\n", __func__ pFlyingContainer->iPositionX,
		pFlyingContainer->iPositionY,
		pFlyingContainer->iWidth,
		pFlyingContainer->iHeight);*/
	gdk_window_move_resize (pWindow->window,
		pFlyingContainer->iPositionX,
		pFlyingContainer->iPositionY,
		pFlyingContainer->iWidth,
		pFlyingContainer->iHeight);
	/*gtk_window_resize (GTK_WINDOW (pWindow),
		pFlyingContainer->iWidth,
		pFlyingContainer->iHeight);
	gtk_window_move (GTK_WINDOW (pWindow),
		pFlyingContainer->iPositionX,
		pFlyingContainer->iPositionY);*/
	gtk_window_present (GTK_WINDOW (pWindow));
	
	pFlyingContainer->pIcon->iAnimationType = CAIRO_DOCK_PULSE;
	pFlyingContainer->pIcon->iCount = 1e6;  // attention : cette animation s'arretera au bout de 11.5 ans :o)
	pFlyingContainer->pIcon->fDrawX = 0;
	pFlyingContainer->pIcon->fDrawY = 0;
	pFlyingContainer->iSidAnimationTimer = g_timeout_add (60, (GSourceFunc) _cairo_dock_animate_flying_icon, (gpointer) pFlyingContainer);
	return pFlyingContainer;
}

void cairo_dock_drag_flying_container (CairoFlyingContainer *pFlyingContainer, CairoDock *pOriginDock)
{
	if (pOriginDock->bHorizontalDock)
	{
		pFlyingContainer->iPositionX = pOriginDock->iWindowPositionX + pOriginDock->iMouseX - pFlyingContainer->iWidth/2;
		pFlyingContainer->iPositionY = pOriginDock->iWindowPositionY + pOriginDock->iMouseY - pFlyingContainer->iHeight/2;
	}
	else
	{
		pFlyingContainer->iPositionY = pOriginDock->iWindowPositionX + pOriginDock->iMouseX - pFlyingContainer->iWidth/2;
		pFlyingContainer->iPositionX = pOriginDock->iWindowPositionY + pOriginDock->iMouseY - pFlyingContainer->iHeight/2;
	}
	//g_print ("  on tire l'icone volante en (%d;%d)\n", pFlyingContainer->iPositionX, pFlyingContainer->iPositionY);
	gtk_window_move (GTK_WINDOW (pFlyingContainer->pWidget),
		pFlyingContainer->iPositionX,
		pFlyingContainer->iPositionY);
}

void cairo_dock_free_flying_container (CairoFlyingContainer *pFlyingContainer)
{
	gtk_widget_destroy (pFlyingContainer->pWidget);  // enleve les signaux.
	if (pFlyingContainer->iSidAnimationTimer != 0)
		g_source_remove (pFlyingContainer->iSidAnimationTimer);
	g_free (pFlyingContainer);
}

void cairo_dock_terminate_flying_container (CairoFlyingContainer *pFlyingContainer)
{
	Icon *pIcon = pFlyingContainer->pIcon;
	pFlyingContainer->pIcon = NULL;
	pFlyingContainer->iAnimationCount = 10+1;
	
	if (pIcon->acDesktopFileName != NULL)  // c'est un lanceur, ou un separateur manuel, ou un sous-dock.
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
		
		GError *erreur = NULL;
		GKeyFile *pKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pKeyFile, pIcon->pModuleInstance->cConfFilePath, 0, &erreur);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
		}
		else
		{
			//\______________ On centre le desklet sur l'icone volante.
			int iDeskletWidth = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "width", NULL, 92, NULL, NULL);
			int iDeskletHeight = cairo_dock_get_integer_key_value (pKeyFile, "Desklet", "height", NULL, 92, NULL, NULL);
			g_key_file_free (pKeyFile);
			
			int iDeskletPositionX = pFlyingContainer->iPositionX + (pFlyingContainer->iWidth - iDeskletWidth)/2;
			int iDeskletPositionY = pFlyingContainer->iPositionY + (pFlyingContainer->iHeight - iDeskletHeight)/2;
			cairo_dock_update_conf_file (pIcon->pModuleInstance->cConfFilePath,
				G_TYPE_BOOLEAN, "Desklet", "initially detached", TRUE,
				G_TYPE_INT, "Desklet", "x position", iDeskletPositionX,
				G_TYPE_INT, "Desklet", "y position", iDeskletPositionY,
				G_TYPE_INVALID);
			
			//\______________ On detache le module.
			cairo_dock_reload_module_instance (pIcon->pModuleInstance, TRUE);
			
			//\______________ On fait apparaitre le desklet avec un effet de zoom.
			if (pIcon->pModuleInstance->pDesklet)  // normalement toujours vrai.
			{
				while (pIcon->pModuleInstance->pDesklet->iDesiredWidth != 0 && pIcon->pModuleInstance->pDesklet->iDesiredHeight != 0 && (pIcon->pModuleInstance->pDesklet->iKnownWidth != pIcon->pModuleInstance->pDesklet->iDesiredWidth || pIcon->pModuleInstance->pDesklet->iKnownHeight != pIcon->pModuleInstance->pDesklet->iDesiredHeight))
				{
					gtk_main_iteration ();
					//if (! pIcon->pModuleInstance->pDesklet)  // ne devrait pas arriver.
					//	break ;
				}
				cairo_dock_zoom_out_desklet (pIcon->pModuleInstance->pDesklet);
			}
		}
	}
	else  // ne devrait pas arriver.
	{
		cairo_dock_free_icon (pIcon);
	}
}
