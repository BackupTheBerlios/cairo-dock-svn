
#ifndef __CAIRO_DOCK_CALLBACKS__
#define  __CAIRO_DOCK_CALLBACKS__

#include <gtk/gtk.h>

void on_realize (GtkWidget* pWidget,
	 CairoDock *pDock);

gboolean on_expose (GtkWidget *pWidget,
			GdkEventExpose *pExpose,
			CairoDock *pDock);

void cairo_dock_show_subdock (Icon *pPointedIcon, gboolean bUpdate, CairoDock *pDock);
void cairo_dock_on_change_icon (Icon *pLastPointedIcon, Icon *pPointedIcon, CairoDock *pDock);
gboolean on_motion_notify2 (GtkWidget* pWidget,
					GdkEventMotion* pMotion,
					CairoDock *pDock);


gboolean cairo_dock_emit_signal_on_dock (CairoDock *pDock, const gchar *cSignal);
gboolean cairo_dock_emit_leave_signal (CairoDock *pDock);
gboolean cairo_dock_emit_enter_signal (CairoDock *pDock);

gboolean cairo_dock_poll_screen_edge (CairoDock *pDock);

void cairo_dock_leave_from_main_dock (CairoDock *pDock);
gboolean on_leave_notify2 (GtkWidget* pWidget,
					GdkEventCrossing* pEvent,
					CairoDock *pDock);

gboolean on_enter_notify2 (GtkWidget* pWidget,
					GdkEventCrossing* pEvent,
					CairoDock *pDock);


gboolean on_key_release (GtkWidget *pWidget,
				GdkEventKey *pKey,
				CairoDock *pDock);
gboolean on_key_press (GtkWidget *pWidget,
				GdkEventKey *pKey,
				CairoDock *pDock);

gboolean cairo_dock_launch_command_full (const gchar *cCommand, gchar *cWorkingDirectory, ...);
#define cairo_dock_launch_command(cCommand,...) cairo_dock_launch_command_full (cCommand, NULL, ##__VA_ARGS__)

gboolean cairo_dock_notification_click_icon (gpointer *data);
gboolean cairo_dock_notification_middle_click_icon (gpointer *data);
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
void on_drag_leave (GtkWidget *pWidget, GdkDragContext *dc, guint time, CairoDock *pDock);


gboolean on_delete (GtkWidget *pWidget, GdkEvent *event, CairoDock *pDock);


/*void on_selection_get (GtkWidget *pWidget, GtkSelectionData *data, guint info, guint time, gpointer user_data);

void on_selection_received (GtkWidget *pWidget, GtkSelectionData *data, guint time, gpointer user_data);

gboolean on_selection_clear_event (GtkWidget *pWidget, GdkEventSelection *event, gpointer user_data);

gboolean on_selection_request_event (GtkWidget *pWidget, GdkEventSelection *event, gpointer user_data);

gboolean on_selection_notify_event (GtkWidget *pWidget, GdkEventSelection *event, gpointer user_data);*/


void cairo_dock_show_dock_at_mouse (CairoDock *pDock);
void cairo_dock_raise_from_keyboard (const char *cKeyShortcut, gpointer data);

gboolean cairo_dock_hide_dock_like_a_menu (void);
void cairo_dock_has_been_hidden_like_a_menu (void);


#endif
