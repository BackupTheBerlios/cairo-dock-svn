
#ifndef __CAIRO_DOCK_CALLBACKS__
#define  __CAIRO_DOCK_CALLBACKS__

#include <gtk/gtk.h>


gboolean move_up2 (GtkWidget *pWidget);

gboolean move_down2 (GtkWidget *pWidget);


gboolean on_expose (GtkWidget *pWidget,
			GdkEventExpose *pExpose);

gboolean on_motion_notify2 (GtkWidget* pWidget,
					GdkEventMotion* pMotion,
					gpointer data);


gboolean on_leave_notify2 (GtkWidget* pWidget,
					GdkEventCrossing* pEvent,
					gpointer data);

gboolean on_enter_notify2 (GtkWidget* pWidget,
					GdkEventCrossing* pEvent,
					gpointer data);


void cairo_dock_update_gaps_with_window_position (GtkWidget *pWidget);

gboolean on_key_release (GtkWidget *pWidget,
				GdkEventKey *pKey,
				gpointer userData);
gboolean on_key_press (GtkWidget *pWidget,
				GdkEventKey *pKey,
				gpointer userData);


gboolean on_button_press2 (GtkWidget* pWidget,
					GdkEventButton* pButton,
					GdkWindowEdge edge);
gboolean on_button_release (GtkWidget* pWidget,
					GdkEventButton* pButton,
					GdkWindowEdge edge);


gboolean on_configure (GtkWidget* pWidget,
	   			GdkEventConfigure* pEvent,
	   			gpointer userData);

void on_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data);
void on_drag_motion (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data);


#endif

