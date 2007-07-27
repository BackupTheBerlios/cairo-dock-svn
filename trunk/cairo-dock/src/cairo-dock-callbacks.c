/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.


******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <cairo.h>
#include <pango/pango.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-menu.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-load.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-applications.h"
#include "cairo-dock-desktop-file-factory.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-config.h"
#include "cairo-dock-callbacks.h"


extern GList* icons;

extern gint g_iScreenWidth;
extern gint g_iScreenHeight;
extern gint g_iCurrentWidth;
extern gint g_iCurrentHeight;

extern float g_fMagnitude;
extern double g_fAmplitude;
extern int g_iLabelSize;
extern gboolean g_bUseText;
extern int g_iDockRadius;
extern int g_iDockLineWidth;
extern gboolean g_bAutoHide;
extern int g_iIconGap;
extern int g_iMaxIconHeight;

extern gboolean g_bAtBottom;
extern gboolean g_bAtTop;
extern gboolean g_bInside;
extern gchar *g_cConfFile;
extern gchar *g_cCairoDockDataDir;

extern gint g_iWindowPositionX;
extern gint g_iWindowPositionY;

extern gdouble g_fGradientOffsetX;

extern int g_iMaxDockWidth;
extern int g_iMaxDockHeight;
extern int g_iMinDockWidth;
extern int g_iVisibleZoneWidth;
extern int g_iVisibleZoneHeight;
extern int g_iGapX;
extern int g_iGapY;
extern gchar *g_cLabelPolice;

extern int g_iSidMoveDown;
extern int g_iSidMoveUp;
extern int g_iSidGrowUp;
extern int g_iSidShrinkDown;
extern gboolean g_bMenuVisible;
extern gboolean g_bDirectionUp;
extern gboolean g_bHorizontalDock;

extern int g_iNbStripes;

extern double g_fMoveUpSpeed;
extern double g_fMoveDownSpeed;

extern int g_tAnimationType[CAIRO_DOCK_NB_TYPES];
extern int g_tNbAnimationRounds[CAIRO_DOCK_NB_TYPES];
extern GList *g_tIconsSubList[CAIRO_DOCK_NB_TYPES];

#ifdef HAVE_GLITZ
extern gboolean g_bUseGlitz;
extern glitz_drawable_format_t *gDrawFormat;
extern glitz_drawable_t* g_pGlitzDrawable;
extern glitz_format_t* g_pGlitzFormat;
#endif // HAVE_GLITZ


gboolean move_up2 (GtkWidget *pWidget)
{
	int deltaY_possible;
	if (g_bHorizontalDock)
		deltaY_possible = (g_bDirectionUp ? g_iWindowPositionY - (g_iScreenHeight - g_iMaxDockHeight - g_iGapY) : g_iWindowPositionY - g_iScreenHeight + g_iGapY + g_iVisibleZoneHeight);
	else
		deltaY_possible = (g_bDirectionUp ? g_iGapX - g_iWindowPositionX : - g_iScreenWidth + g_iGapX + g_iMaxDockWidth + g_iWindowPositionX);
	//g_print ("%s (%dx%d -> %d)\n", __func__, g_iWindowPositionX, g_iWindowPositionY, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 0) || (! g_bDirectionUp && deltaY_possible < 0))  // alors on peut encore monter.
	{
		if (g_bHorizontalDock)
			g_iWindowPositionY -= (int) (deltaY_possible * g_fMoveUpSpeed) + (g_bDirectionUp ? 1 : -1); // 0.5
		else
			g_iWindowPositionX += (int) (deltaY_possible * g_fMoveUpSpeed) + (g_bDirectionUp ? 1 : -1);
		//g_print ("  move to (%dx%d)\n", g_iWindowPositionX, g_iWindowPositionY);
		gtk_window_move (GTK_WINDOW (pWidget), g_iWindowPositionX, g_iWindowPositionY);
		g_bAtBottom = FALSE;
		return TRUE;
	}
	else
	{
		g_bAtTop = TRUE;
		g_iSidMoveUp = 0;
		return FALSE;
	}
}


gboolean move_down2 (GtkWidget *pWidget)
{
	if (g_fMagnitude > 0.1)  // on retarde le cachage du dock pour apercevoir les effets.
		return TRUE;
	int deltaY_possible = g_iScreenHeight - g_iGapY - (g_bDirectionUp ? g_iVisibleZoneHeight : g_iMaxDockHeight) - g_iWindowPositionY;
	//g_print ("%s (%d)\n", __func__, deltaY_possible);
	if ((g_bDirectionUp && deltaY_possible > 4) || (! g_bDirectionUp && deltaY_possible < -4))  // alors on peut encore descendre.
	{
		g_iWindowPositionY += (int) (deltaY_possible * g_fMoveDownSpeed) + (g_bDirectionUp ? 1 : -1);  // 0.33
		gtk_window_move (GTK_WINDOW (pWidget), g_iWindowPositionX, g_iWindowPositionY);
		g_bAtTop = FALSE;
		return TRUE;
	}
	else  // on se fixe en bas, et on montre la zone visible.
	{
		g_bAtBottom = TRUE;
		g_iWindowPositionX = (g_iScreenWidth - g_iVisibleZoneWidth) / 2 + g_iGapX;
		g_iWindowPositionY = g_iScreenHeight - g_iVisibleZoneHeight - g_iGapY;
		//g_print ("%s () -> %dx%d\n", __func__, g_iVisibleZoneWidth, g_iVisibleZoneHeight);
		gdk_window_move_resize (pWidget->window,
			g_iWindowPositionX,
			g_iWindowPositionY,
			g_iVisibleZoneWidth,
			g_iVisibleZoneHeight);
		g_iSidMoveDown = 0;
		
		if (g_bAutoHide)
		{
			Icon *pBouncingIcon = cairo_dock_get_bouncing_icon ();
			if (pBouncingIcon != NULL)
			{
				pBouncingIcon->iCount = 0;
			}
			Icon *pRemovingIcon = cairo_dock_get_removing_or_inserting_icon ();
			if (pRemovingIcon != NULL)
			{
				pRemovingIcon->fPersonnalScale = 0.001;
			}
		}
		
		return FALSE;
	}
}


gboolean on_expose (GtkWidget *pWidget,
			GdkEventExpose *pExpose)
{
	//g_print ("%s ((%d;%d) %dx%d)\n", __func__, pExpose->area.x, pExpose->area.y, pExpose->area.width, pExpose->area.height);
	
	if (pExpose->area.x + pExpose->area.y != 0)  // x et y sont >= 0.
	{
		if (! g_bAutoHide || ! g_bAtBottom)
			cairo_dock_render_optimized (pWidget, &pExpose->area);
		
		return FALSE;
	}
	
	if (!g_bAtBottom)
	{
		render (pWidget);
		
	}
	else
	{
		if (g_bAutoHide)
		{
			if (g_bInside)
				cairo_dock_render_blank (pWidget);
			else
				cairo_dock_render_background (pWidget);
		}
		else
			render (pWidget);
	}
	
	return FALSE;
}


gboolean on_motion_notify2 (GtkWidget* pWidget,
					GdkEventMotion* pMotion,
					gpointer data)
{
	//g_print ("%s ()\n", __func__);
	if (g_bAtBottom || ! g_bInside || g_iSidShrinkDown > 0)  // si les icones sont en train de diminuer de taille (suite a un clic) on ne redimensionne pas les icones, le temps que l'animation se finisse.
		return FALSE;
	//gint iMouseX, iMouseY;
	//gdk_window_get_pointer (pWidget->window, &iMouseX, &iMouseY, NULL);
	//g_fGradientOffsetX = iMouseX % 15;  // pour le decalage des rayures.
	
	//\_______________ On accentue le mouvement actuel.
	/*if (g_iSidMoveUp != 0)
		move_up2 (pWidget);
	else if (g_iSidMoveDown != 0)
		move_down2 (pWidget);*/
	
	//\_______________ On recalcule toutes les icones.
	cairo_dock_calculate_icons (pWidget, g_fMagnitude);  // incroyable mais vrai : si on utilise les coordonnees (x,y) presentes dans le GdkEventMotion, et qu'on ne les recupere pas par un gdk_window_get_pointer () dans la fonction de recalcul, on ne recevra plus d'autres EventMotion !
	gtk_widget_queue_draw (pWidget);
	
	return FALSE;
}


gboolean on_leave_notify2 (GtkWidget* pWidget,
					GdkEventCrossing* pEvent,
					gpointer data)
{
	if (g_bAtBottom || ! g_bInside)
		return FALSE;
	//g_print ("%s ()\n", __func__);
	
	g_bInside = FALSE;
	if (g_bMenuVisible)
	{
		g_bAtTop = FALSE;
		return FALSE;
	}
	if (g_iSidMoveUp > 0)  // si on est en train de monter, on arrete.
	{
		g_source_remove (g_iSidMoveUp);
		g_iSidMoveUp = 0;
	}
	if (g_iSidGrowUp > 0)  // si on est en train de faire grossir les icones, on arrete.
	{
		g_source_remove (g_iSidGrowUp);
		g_iSidGrowUp = 0;
	}
	
	
	if (g_bAutoHide)
	{
		if (g_iSidMoveDown == 0)  // on commence a descendre.
			g_iSidMoveDown = g_timeout_add (40, (GSourceFunc) move_down2, (gpointer) pWidget);
	}
	else
	{
		g_bAtTop = FALSE;
		g_bAtBottom = TRUE;
	}
	
	if (g_iSidShrinkDown == 0)  // on commence a faire diminuer la taille des icones.
		g_iSidShrinkDown = g_timeout_add (35, (GSourceFunc) shrink_down2, (gpointer) pWidget);
	
	return FALSE;
	
}


gboolean on_enter_notify2 (GtkWidget* pWidget,
					GdkEventCrossing* pEvent,
					gpointer data)
{
	if (g_bAtTop || g_bInside)
		return FALSE;
	//g_print ("%s (%d)\n", __func__, g_iWindowPositionY);
	
	gtk_window_present (GTK_WINDOW (pWidget));
	g_bInside = TRUE;
	
	
	if (g_bHorizontalDock)
	{
		g_iWindowPositionX = (g_iScreenWidth - g_iMaxDockWidth) / 2 + g_iGapX;
		if (! g_bAutoHide)
			g_iWindowPositionY = (g_bDirectionUp ? g_iScreenHeight - g_iMaxDockHeight - g_iGapY : g_iScreenHeight - g_iGapY);
		else
			g_iWindowPositionY = (g_bDirectionUp ? g_iWindowPositionY : g_iVisibleZoneHeight - g_iMaxDockHeight + (g_iScreenHeight - g_iGapY));
	}
	else
	{
		g_iWindowPositionY = (g_iScreenHeight - g_iMaxDockHeight) / 2 + g_iGapY;
		if (! g_bAutoHide)
			g_iWindowPositionX = (g_bDirectionUp ? g_iWindowPositionX : g_iScreenWidth - g_iGapX - g_iMaxDockWidth);
		else
			g_iWindowPositionX = (g_bDirectionUp ? g_iGapX - g_iMaxDockWidth : g_iScreenWidth - g_iGapX - g_iMaxDockWidth);
	}
	//g_print (" -> %dx%d\n", g_iWindowPositionX, g_iWindowPositionY);
		
	gdk_window_move_resize (pWidget->window,
		g_iWindowPositionX,
		g_iWindowPositionY,
		g_iMaxDockWidth,
		g_iMaxDockHeight);
	//gtk_widget_queue_draw (pWidget);
	
	if (g_iSidMoveDown > 0)  // si on est en train de descendre, on arrete.
	{
		g_source_remove (g_iSidMoveDown);
		g_iSidMoveDown = 0;
	}
	/*if (g_iSidShrinkDown > 0)  // si on est en train de faire diminuer la tailler des icones, on arrete.
	{
		g_source_remove (g_iSidShrinkDown);
		g_iSidShrinkDown = 0;
	}*/
	
	if (g_bAutoHide)
	{
		if (g_iSidMoveUp == 0)  // on commence a monter.
			g_iSidMoveUp = g_timeout_add (40, (GSourceFunc) move_up2, (gpointer) pWidget);
	}
	else
	{
		g_bAtTop = TRUE;
		g_bAtBottom = FALSE;
	}
	if (g_iSidGrowUp == 0 && g_iSidShrinkDown == 0)  // on commence a faire grossir les icones.
		g_iSidGrowUp = g_timeout_add (35, (GSourceFunc) grow_up2, (gpointer) pWidget);
	
	return FALSE;
	
	
}


void cairo_dock_update_gaps_with_window_position (GtkWidget *pWidget)
{
	//g_print ("%s ()\n", __func__);
	int iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pWidget), &iWidth, &iHeight);
	gtk_window_get_position (GTK_WINDOW (pWidget), &g_iWindowPositionX, &g_iWindowPositionY);
	
	int x, y;  // position du centre bas du dock;
	x = g_iWindowPositionX + iWidth / 2;
	y = g_iWindowPositionY + (g_bDirectionUp ? iHeight : (g_bAutoHide ? g_iVisibleZoneHeight : 0));
	
	g_iGapX = x - g_iScreenWidth / 2;
	g_iGapY = g_iScreenHeight - y;
	
	cairo_dock_update_conf_file_with_position (g_cConfFile, g_iGapX, g_iGapY);
}

static int iMoveByArrow = 0;
gboolean on_key_release (GtkWidget *pWidget,
				GdkEventKey *pKey,
				gpointer userData)
{
	//g_print ("%s ()\n", __func__);
	iMoveByArrow = 0;
	if (pKey->state & GDK_MOD1_MASK)  // On relache la touche ALT, typiquement apres avoir fait un ALT + clique gauche + deplacement.
	{
		cairo_dock_update_gaps_with_window_position (pWidget);
	}
	return FALSE;
}

gboolean on_key_press (GtkWidget *pWidget,
				GdkEventKey *pKey,
				gpointer userData)
{
	//g_print ("%s ()\n", __func__);
	iMoveByArrow ++;
	int iPossibleMove;
	
	int iWidth, iHeight;
	gtk_window_get_size (GTK_WINDOW (pWidget), &iWidth, &iHeight);
	int x, y;  // position du centre bas du dock;
	x = g_iWindowPositionX +iWidth / 2;
	y = g_iWindowPositionY + iHeight;
	if (pKey->type == GDK_KEY_PRESS)
	{
		switch (pKey->keyval)
		{
			case GDK_q :
				if (pKey != NULL && pKey->state & GDK_CONTROL_MASK)  // CTRL + q quitte l'appli.
					gtk_main_quit ();
			break;
			
			case GDK_Down :
				iPossibleMove = MAX (0, g_iScreenHeight - 1 - y);
				iMoveByArrow = MIN (iMoveByArrow, iPossibleMove);
				if (iMoveByArrow > 0)
				{
					g_iWindowPositionY += iMoveByArrow;
					g_iGapY -= iMoveByArrow;
					gtk_window_move (GTK_WINDOW (pWidget), g_iWindowPositionX, g_iWindowPositionY);
					cairo_dock_update_conf_file_with_position (g_cConfFile, g_iGapX, g_iGapY);
				}
			break;

			case GDK_Up :
				iPossibleMove = MAX (0, y - g_iMaxDockHeight);
				iMoveByArrow = MIN (iMoveByArrow, iPossibleMove);
				if (iMoveByArrow > 0)
				{
					g_iWindowPositionY -= iMoveByArrow;
					g_iGapY += iMoveByArrow;
					gtk_window_move (GTK_WINDOW (pWidget), g_iWindowPositionX, g_iWindowPositionY);
					cairo_dock_update_conf_file_with_position (g_cConfFile, g_iGapX, g_iGapY);
				}
			break;
			
			case GDK_Left :
				iPossibleMove = MAX (0, x - g_iMinDockWidth / 2);
				iMoveByArrow = MIN (iMoveByArrow, iPossibleMove);
				if (iMoveByArrow > 0)
				{
					g_iWindowPositionX -= iMoveByArrow;
					g_iGapX -= iMoveByArrow;
					gtk_window_move (GTK_WINDOW (pWidget), g_iWindowPositionX, g_iWindowPositionY);
					cairo_dock_update_conf_file_with_position (g_cConfFile, g_iGapX, g_iGapY);
				}
			break;
			
			case GDK_Right :
				iPossibleMove = MAX (0, g_iScreenWidth - (x + g_iMinDockWidth / 2 + g_iDockRadius + g_iDockLineWidth));
				iMoveByArrow = MIN (iMoveByArrow, iPossibleMove);
				if (iMoveByArrow > 0)
				{
					g_iWindowPositionX += iMoveByArrow;
					g_iGapX += iMoveByArrow;
					gtk_window_move (GTK_WINDOW (pWidget), g_iWindowPositionX, g_iWindowPositionY);
					cairo_dock_update_conf_file_with_position (g_cConfFile, g_iGapX, g_iGapY);
				}
			break;
		}
	}
	
	return FALSE;
}


gboolean on_button_press2 (GtkWidget* pWidget,
					GdkEventButton* pButton,
					GdkWindowEdge edge)
{
	//g_print ("%s (%d/%d)\n", __func__, pButton->type, pButton->button);
	if (pButton->type == GDK_BUTTON_PRESS)  // simple clique.
	{
		if (pButton->button == 1)  // clique gauche.
		{
			Icon *icon = cairo_dock_get_pointed_icon ();
			if (icon != NULL && ! CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				icon->iAnimationType = g_tAnimationType[icon->iType];
				do
				{
					switch (icon->iAnimationType)
					{
						case CAIRO_DOCK_BOUNCE:
							icon->iCount = 10;  // 5 tours pour monter et 5 pour descendre.
						break;
						case CAIRO_DOCK_ROTATE:
							icon->iCount = 20;  // 5 iterations par quart de tour.
						break;
						case CAIRO_DOCK_BLINK:
							icon->iCount = 20;  // 10 iterations par inversion d'alpha.
						break;
						case CAIRO_DOCK_RANDOM:
							icon->iCount = 0;
							icon->iAnimationType =  g_random_int_range (0, CAIRO_DOCK_NB_ANIMATIONS);
						break;
						default :
							icon->iCount = 10;
					}
				} while (icon->iCount == 0);
				icon->iCount = icon->iCount * g_tNbAnimationRounds[icon->iType] - 1;
				
				if (icon->acCommand != NULL)
				{
					g_spawn_command_line_async (icon->acCommand, NULL);
				}
				else if (icon->Xid != 0)
				{
					cairo_dock_show_appli (icon->Xid);
				}
				else if (icon->pModule != NULL && icon->pModule->actionModule != NULL)
				{
					icon->pModule->actionModule ();
				}
				
				if (g_iSidShrinkDown == 0)
					g_iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pWidget);  // fera diminuer de taille les icones, et rebondir celle qui est cliquee.
			}
		}
		else if (pButton->button == 3)  // clique droit.
		{
			g_bMenuVisible = TRUE;
			GtkWidget *menu = cairo_dock_build_menu (pWidget);
			
			gtk_widget_show_all (menu);
			
			gtk_menu_popup (GTK_MENU (menu),
				NULL,
				NULL,
				NULL,
				NULL,
				1,
				gtk_get_current_event_time ());
		}
		else if (pButton->button == 2)  // clique milieu.
		{
			gtk_window_begin_move_drag (GTK_WINDOW (pWidget),
				pButton->button,
				pButton->x_root,
				pButton->y_root,
				pButton->time);  // permet de déplacer la fenetre, marche avec Metacity, mais pas avec Beryl !
		}
	}
	
	return FALSE;
}
gboolean on_button_release (GtkWidget* pWidget,
					GdkEventButton* pButton,
					GdkWindowEdge edge)
{
	//g_print ("%s ()\n", __func__);
	if (pButton->button == 2)  // fin d'un drag and drop, sauf que ca ne marche ni avec Beryl, ni avec Metacity, car ils ne laissent pas passer le signal de relache du bouton :-/
	{
		cairo_dock_update_gaps_with_window_position (pWidget);
	}
	return FALSE;
}


gboolean on_configure (GtkWidget* pWidget,
	   			GdkEventConfigure* pEvent,
	   			gpointer userData)
{
	//g_print ("%s ()\n", __func__);
	gint iNewWidth = pEvent->width;
	gint iNewHeight = pEvent->height;

	if (iNewWidth != g_iCurrentWidth || iNewHeight != g_iCurrentHeight)
	{
		//g_print ("-> %dx%d\n", iNewWidth, iNewHeight);
		g_iCurrentWidth = iNewWidth;
		g_iCurrentHeight = iNewHeight;
		cairo_dock_calculate_icons (pWidget, g_fMagnitude);
		if (!g_bAtBottom)
		{
			render (pWidget);
		}
		else
		{
			if (g_bAutoHide)
				cairo_dock_render_background (pWidget);
			else
				render (pWidget);
		}


#ifdef HAVE_GLITZ
		if (g_pGlitzDrawable)
			glitz_drawable_update_size (g_pGlitzDrawable,
						    g_iCurrentWidth,
						    g_iCurrentHeight);
#endif
	}

	return FALSE;
}


void on_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	
	gchar *cReceivedData = (gchar *) selection_data->data;
	if (strncmp (cReceivedData, "file://", 7) == 0)
	{
		GError *erreur = NULL;
		//\_________________ On recupere l'URI du .desktop.
		gchar *cFilePath = g_strdup (cReceivedData + 7);  // on saute le "file://".
		gchar *str = strrchr (cFilePath, '\n');
		if (str != NULL)
			*(str-1) = '\0';  // on vire les retrours chariot.
		
		//\_________________ On calcule la position a laquelle on l'a lache.
		double fOrder = 0;
		GList *ic;
		Icon *icon, *next_icon, *prev_icon;
		for (ic = icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->bPointed)
			{
				//g_print ("On pointe sur %s\n", icon->acName);
				if (x > icon->fX + icon->fWidth * icon->fScale / 2)  // on est apres.
				{
					next_icon = (ic->next != NULL ? ic->next->data : NULL);
					fOrder = (next_icon != NULL ? (icon->fOrder + next_icon->fOrder) / 2 : icon->fOrder + 1);
				}
				else
				{
					prev_icon = (ic->prev != NULL ? ic->prev->data : NULL);
					fOrder = (prev_icon != NULL ? (icon->fOrder + prev_icon->fOrder) / 2 : icon->fOrder - 1);
				}
			}
		}
		
		//\_________________ On l'ajoute dans le repertoire .cairo-dock.
		gchar *cNewDesktopFilePath = cairo_dock_add_desktop_file_from_path (cFilePath, fOrder, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			return ;
		}
		
		//\_________________ On charge ce nouveau lanceur.
		gchar *cDesktopFileName = g_path_get_basename (cNewDesktopFilePath);
		g_free (cNewDesktopFilePath);
		cairo_t* pCairoContext = cairo_dock_create_context_from_window (pWidget->window);
		Icon *pNewIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFileName, pCairoContext);
		g_free (cDesktopFileName);
		cairo_destroy (pCairoContext);
		
		if (pNewIcon != NULL)
			cairo_dock_insert_icon_in_list (pNewIcon, pWidget, TRUE, TRUE);
		
		if (g_iSidShrinkDown == 0)
			g_iSidShrinkDown = g_timeout_add (50, (GSourceFunc) shrink_down2, (gpointer) pWidget);
		
		g_free (cFilePath);
	}
}


void on_drag_motion (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
{
	//g_print ("%s (%dx%d)\n", __func__, x, y);
	//\_________________ On simule les evenements souris habituels.
	on_enter_notify2 (pWidget, NULL, NULL);  // ne sera effectif que la 1ere fois.
	on_motion_notify2 (pWidget, NULL, NULL);
}

