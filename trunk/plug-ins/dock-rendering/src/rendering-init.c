/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "rendering-config.h"
#include "rendering-caroussel.h"
#include "rendering-parabole.h"
#include "rendering-3D-plane.h"
#include "rendering-rainbow.h"
//#include "rendering-diapo.h"
#include "rendering-diapo-simple.h"
#include "rendering-curve.h"
#include "rendering-commons.h"
#include "rendering-init.h"

#define CD_RENDERING_CAROUSSEL_VIEW_NAME N_("Caroussel")
#define CD_RENDERING_3D_PLANE_VIEW_NAME N_("3D plane")
#define CD_RENDERING_PARABOLIC_VIEW_NAME N_("Parabolic")
#define CD_RENDERING_RAINBOW_VIEW_NAME N_("Rainbow")
//#define CD_RENDERING_DIAPO_VIEW_NAME "Slide"
#define CD_RENDERING_DIAPO_SIMPLE_VIEW_NAME N_("Slide")
#define CD_RENDERING_CURVE_VIEW_NAME N_("Curve")


int iVanishingPointY;  // distance du point de fuite au plan (au niveau du point de contact du plan et des icones).
CDSpeparatorType my_iDrawSeparator3D;

double my_fInclinationOnHorizon;  // inclinaison de la ligne de fuite vers l'horizon.
cairo_surface_t *my_pFlatSeparatorSurface[2] = {NULL, NULL};
double my_fSeparatorColor[4];
GLuint my_iFlatSeparatorTexture;

double my_fForegroundRatio;  // fraction des icones presentes en avant-plan (represente donc l'etirement en profondeur de l'ellipse).
double my_iGapOnEllipse;  // regle la profondeur du caroussel.
gboolean my_bRotateIconsOnEllipse;  // tourner les icones de profil ou pas.
double my_fScrollAcceleration;
double my_fScrollSpeed;

double my_fParaboleCurvature;  // puissance de x.
double my_fParaboleRatio;  // hauteur/largeur.
double my_fParaboleMagnitude;
int my_iParaboleTextGap;
gboolean my_bDrawTextWhileUnfolding;

int my_iSpaceBetweenRows;
int my_iSpaceBetweenIcons;
double my_fRainbowMagnitude;
int my_iRainbowNbIconsMin;
double my_fRainbowConeOffset;
double my_fRainbowColor[4];
double my_fRainbowLineColor[4];

/*gint     my_diapo_iconGapX;
gint     my_diapo_iconGapY;
gdouble  my_diapo_fScaleMax;
gint     my_diapo_sinW;
gboolean my_diapo_lineaire;
gboolean  my_diapo_wide_grid;
gboolean  my_diapo_text_only_on_pointed;

gdouble  my_diapo_color_frame_start[4];
gdouble  my_diapo_color_frame_stop[4];
gboolean my_diapo_fade2bottom;
gboolean my_diapo_fade2right;
guint    my_diapo_arrowWidth;
guint    my_diapo_arrowHeight;
gdouble  my_diapo_arrowShift;
guint    my_diapo_lineWidth;
guint    my_diapo_radius;
gdouble  my_diapo_color_border_line[4];
gboolean my_diapo_draw_background;
gboolean my_diapo_display_all_icons;*/

gint     my_diapo_simple_iconGapX;
gint     my_diapo_simple_iconGapY;
gdouble  my_diapo_simple_fScaleMax;
gint     my_diapo_simple_sinW;
gboolean my_diapo_simple_lineaire;
gboolean  my_diapo_simple_wide_grid;
gboolean  my_diapo_simple_text_only_on_pointed;
gboolean my_diapo_simple_display_all_icons;

gdouble  my_diapo_simple_color_frame_start[4];
gdouble  my_diapo_simple_color_frame_stop[4];
gboolean my_diapo_simple_fade2bottom;
gboolean my_diapo_simple_fade2right;
guint    my_diapo_simple_arrowWidth;
guint    my_diapo_simple_arrowHeight;
gdouble  my_diapo_simple_arrowShift;
guint    my_diapo_simple_lineWidth;
guint    my_diapo_simple_radius;
gdouble  my_diapo_simple_color_border_line[4];
gboolean my_diapo_simple_draw_background;


gdouble my_fCurveCurvature;
gint my_iCurveAmplitude;
CDSpeparatorType my_curve_iDrawSeparator3D;


CD_APPLET_PRE_INIT_BEGIN ("dock rendering",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_THEME,
	N_("This module adds different views to your dock.\n"
	"Any dock or sub-dock can be displayed with the view of your choice.\n"
	"Currently, 3D-plane, Caroussel, Parabolic, Rainbow, Slide, and Curve views are provided."),
	"Fabounet (Fabrice Rey) & parAdOxxx_ZeRo")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE;
	CD_APPLET_ATTACH_TO_INTERNAL_MODULE ("Views");
CD_APPLET_PRE_INIT_END


CD_APPLET_INIT_BEGIN
	//\_______________ On enregistre les vues.
	cd_rendering_register_caroussel_renderer 		(CD_RENDERING_CAROUSSEL_VIEW_NAME);
	
	cd_rendering_register_3D_plane_renderer 		(CD_RENDERING_3D_PLANE_VIEW_NAME);
	
	cd_rendering_register_parabole_renderer 		(CD_RENDERING_PARABOLIC_VIEW_NAME);
	
	cd_rendering_register_rainbow_renderer 		(CD_RENDERING_RAINBOW_VIEW_NAME);
	
	//cd_rendering_register_diapo_renderer 			(CD_RENDERING_DIAPO_VIEW_NAME);  // By Paradoxxx_Zero

	cd_rendering_register_diapo_simple_renderer 	(CD_RENDERING_DIAPO_SIMPLE_VIEW_NAME);  // By Paradoxxx_Zero
	
	cd_rendering_register_curve_renderer 			(CD_RENDERING_CURVE_VIEW_NAME);  // By Paradoxxx_Zero and Fabounet
	
	if (! cairo_dock_is_loading ())  // plug-in active a la main (en-dehors du chargement du theme).
	{
		cairo_dock_set_all_views_to_default (0);
		cairo_dock_update_renderer_list_for_gui ();
	}
	else
		gtk_widget_queue_draw (g_pMainDock->pWidget);
	
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_rendering_caroussel_update_dock, CAIRO_DOCK_RUN_AFTER, NULL);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_rendering_caroussel_update_dock, NULL);
	
	cairo_dock_remove_renderer (CD_RENDERING_CAROUSSEL_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_3D_PLANE_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_PARABOLIC_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_RAINBOW_VIEW_NAME);
	//cairo_dock_remove_renderer (CD_RENDERING_DIAPO_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_DIAPO_SIMPLE_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_CURVE_VIEW_NAME);
	
	cairo_dock_reset_all_views ();
	cairo_dock_update_renderer_list_for_gui ();
	gtk_widget_queue_draw (g_pMainDock->pWidget);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (my_pFlatSeparatorSurface[0] != NULL)
		{
			cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
			cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL]);
			my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = NULL;
			my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = NULL;
		}
		if (my_iFlatSeparatorTexture != 0)
		{
			_cairo_dock_delete_texture (my_iFlatSeparatorTexture);
			my_iFlatSeparatorTexture = 0;
		}
		
		cairo_dock_set_all_views_to_default (0);
	}
CD_APPLET_RELOAD_END
