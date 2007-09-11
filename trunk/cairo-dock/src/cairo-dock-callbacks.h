
#ifndef __CAIRO_DOCK_CALLBACKS__
#define  __CAIRO_DOCK_CALLBACKS__

#include <gtk/gtk.h>


gboolean on_expose (GtkWidget *pWidget,
			GdkEventExpose *pExpose,
			CairoDock *pDock);

gboolean on_motion_notify2 (GtkWidget* pWidget,
					GdkEventMotion* pMotion,
					CairoDock *pDock);


void cairo_dock_leave_from_main_dock (CairoDock *pDock);
gboolean on_leave_notify2 (GtkWidget* pWidget,
					GdkEventCrossing* pEvent,
					CairoDock *pDock);

gboolean on_enter_notify2 (GtkWidget* pWidget,
					GdkEventCrossing* pEvent,
					CairoDock *pDock);


void cairo_dock_update_gaps_with_window_position (CairoDock *pDock);

gboolean on_key_release (GtkWidget *pWidget,
				GdkEventKey *pKey,
				CairoDock *pDock);
gboolean on_key_press (GtkWidget *pWidget,
				GdkEventKey *pKey,
				CairoDock *pDock);


gboolean on_button_press2 (GtkWidget* pWidget,
					GdkEventButton* pButton,
					CairoDock *pDock);

gboolean on_scroll (GtkWidget* pWidget,
				GdkEventScroll* pScroll,
				CairoDock *pDock);


gboolean on_configure (GtkWidget* pWidget,
	   			GdkEventConfigure* pEvent,
	   			CairoDock *pDock);


void on_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, CairoDock *pDock);
gboolean cairo_dock_notification_drop_data (gpointer *data);

void on_drag_motion (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, guint t, CairoDock *pDock);


gboolean on_delete (GtkWidget *pWidget, GdkEvent *event, CairoDock *pDock);

#endif

