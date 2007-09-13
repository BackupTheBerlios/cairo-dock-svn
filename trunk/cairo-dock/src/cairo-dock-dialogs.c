/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "cairo-dock-struct.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dialogs.h"

extern gboolean g_bSticky;
extern gboolean g_bKeepAbove;
extern gboolean g_bAutoHide;

extern int g_iLabelSize;
extern gchar *g_cLabelPolice;
extern int g_iLabelStyle;
extern int g_iLabelWeight;

extern int g_iDockLineWidth;
extern int g_iDockRadius;
extern double g_fLineColor[4];

#define CAIRO_DOCK_DIALOG_GAP 20


static gboolean on_button_press_dialog (GtkWidget* pWidget,
	GdkEventButton* pButton,
	CairoDockDialog *pDialog)
{
	g_print ("%s (%d/%d)\n", __func__, pButton->type, pButton->button);
	if (pButton->button == 1)  // clique gauche.
	{
		if (pButton->type == GDK_BUTTON_PRESS)
		{
			gtk_widget_destroy (pWidget);
		}
	}
	return FALSE;
}

static gboolean on_expose_dialog (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	CairoDockDialog *pDialog)
{
	g_print ("%s ()\n", __func__);
	
	double fLineWidth = g_iDockLineWidth;
	double fRadius = (pDialog->iTextHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDialog->iTextHeight + fLineWidth) / 2 - 1);
	
	cairo_t *pCairoContext = gdk_cairo_create (pWidget->window);
	cairo_set_source_rgba (pCairoContext, 0., 0., 0., 0.);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	cairo_save (pCairoContext);
	double fOffsetX = fRadius + fLineWidth / 2;
	int sens = 1;
	cairo_move_to (pCairoContext, fOffsetX, fLineWidth / 2);
	int iWidth = pDialog->iWidth;
	
	cairo_rel_line_to (pCairoContext, iWidth - (2 * fRadius + fLineWidth), 0);
	// Top Right.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens * (pDialog->iTextHeight + fLineWidth - fRadius * 2));
	// Bottom Right.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, sens * fRadius,
		-fRadius, sens * fRadius);
	
	cairo_rel_line_to (pCairoContext, -iWidth + fLineWidth + 2 * fRadius, 0);
	// Bottom Left
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-fRadius, 0,
		-fRadius, -sens * fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens * (- pDialog->iTextHeight - fLineWidth + fRadius * 2));
	// Top Left.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
	
	cairo_save (pCairoContext);
	cairo_set_source_rgba (pCairoContext, 1., 1., 1., .5);
	cairo_fill_preserve (pCairoContext);
	cairo_restore (pCairoContext);
	
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	cairo_stroke (pCairoContext);
	cairo_restore (pCairoContext);
	
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface (pCairoContext, pDialog->pTextBuffer, fOffsetX, fLineWidth);
	cairo_paint (pCairoContext);
	
	cairo_destroy (pCairoContext);
	
	return FALSE;
}

static gboolean on_configure_dialog (GtkWidget* pWidget,
			GdkEventConfigure* pEvent,
			CairoDockDialog *pDialog)
{
	g_print ("%s ()\n", __func__);
	
	pDialog->iWidth = pEvent->width;
	pDialog->iHeight = pEvent->height;
	
	
	return FALSE;
}


GtkWidget *cairo_dock_build_dialog (gchar *cText, Icon *pIcon, CairoDock *pDock)
{
	g_return_val_if_fail (cText != NULL, NULL);
	GtkWidget* pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	if (g_bSticky)
		gtk_window_stick (GTK_WINDOW (pWindow));
	gtk_window_set_keep_above (GTK_WINDOW (pWindow), g_bKeepAbove);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (pWindow), TRUE);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (pWindow), TRUE);
	gtk_window_set_gravity (GTK_WINDOW (pWindow), GDK_GRAVITY_STATIC);
	
	gtk_window_set_type_hint (GTK_WINDOW (pWindow), GDK_WINDOW_TYPE_HINT_MENU);
	
	
	cairo_dock_set_colormap_for_window (pWindow);
	
	gtk_widget_set_app_paintable (pWindow, TRUE);
	gtk_window_set_decorated (GTK_WINDOW (pWindow), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (pWindow), TRUE);
	gtk_window_set_title (GTK_WINDOW (pWindow), "cairo-dock-dialog");
	
	
	gtk_widget_add_events (pWindow, GDK_BUTTON_PRESS_MASK);
	
	CairoDockDialog *pDialog = g_new0 (CairoDockDialog, 1);
	pDialog->pWidget = pWindow;
	
	
	gtk_widget_show_all (pWindow);
	
	int iLabelSize = (g_iLabelSize > 0 ? g_iLabelSize : 14);
	cairo_t *pSourceContext = gdk_cairo_create (pWindow->window);
	cairo_set_source_rgba (pSourceContext, 0., 0., 0., 0.);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	
	PangoLayout *pLayout = pango_cairo_create_layout (pSourceContext);
	
	PangoFontDescription *pDesc = pango_font_description_new ();
	pango_font_description_set_absolute_size (pDesc, iLabelSize * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, g_cLabelPolice);
	pango_font_description_set_weight (pDesc, g_iLabelWeight);
	pango_font_description_set_style (pDesc, g_iLabelStyle);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_font_description_free (pDesc);
	
	pango_layout_set_text (pLayout, cText, -1);
	
	PangoRectangle ink, log;
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	pDialog->iTextWidth = ink.width;
	pDialog->iTextHeight = ink.height;
	
	pDialog->pTextBuffer = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		ink.width, ink.height);
	cairo_t* pSurfaceContext = cairo_create (pDialog->pTextBuffer);
	cairo_translate (pSurfaceContext, -ink.x, -ink.y);
	
	cairo_set_source_rgb (pSurfaceContext, 0., 0., 0.);
	pango_cairo_show_layout (pSurfaceContext, pLayout);
	
	cairo_destroy (pSurfaceContext);
	
	
	double fLineWidth = g_iDockLineWidth;
	double fRadius = (pDialog->iTextHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDialog->iTextHeight + fLineWidth) / 2 - 1);
	int iDialogPositionX, iDialogPositionY, iWidth, iHeight;
	iWidth = ink.width + 2 * fRadius + fLineWidth;
	iHeight = ink.height + CAIRO_DOCK_DIALOG_GAP;
	if ((pDock->iRefCount > 0 && ! pDock->bInside) || (pDock->iRefCount == 0 && (pDock->bInside || ! g_bAutoHide)))
	{
		iDialogPositionX = pDock->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale / 2 - iWidth / 2;
		iDialogPositionY = pDock->iWindowPositionY - iHeight;
	}
	gdk_window_move_resize (pWindow->window,
		iDialogPositionX,
		iDialogPositionY,
		iWidth,
		iHeight);
	g_print ("%s () : (%d;%d) %dx%d\n", __func__, iDialogPositionX, iDialogPositionY, iWidth, iHeight);
	
	g_signal_connect (G_OBJECT (pWindow),
		"expose-event",
		G_CALLBACK (on_expose_dialog),
		pDialog);
	g_signal_connect (G_OBJECT (pWindow),
		"configure-event",
		G_CALLBACK (on_configure_dialog),
		pDialog);
	g_signal_connect (G_OBJECT (pWindow),
		"button-press-event",
		G_CALLBACK (on_button_press_dialog),
		pDialog);
	
	return pDialog;
}
