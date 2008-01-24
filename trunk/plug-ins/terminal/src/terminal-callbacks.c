
#include <string.h>
#include <vte/vte.h>

#include "terminal-init.h"
#include "terminal-callbacks.h"

extern t_terminal term;

static void _terminal_write_command_with_data (GtkWidget *pWidget, gchar *cCommand, gchar *cData)
{
	gchar *cCommandLine = g_strconcat (cCommand, cData, NULL);
	vte_terminal_feed_child (VTE_TERMINAL (pWidget), cCommandLine, strlen (cCommandLine));
	g_free (cCommandLine);
	gtk_widget_grab_focus (pWidget);
}
static void _terminal_copy (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pWidget = data[0];
	gchar *cReceivedData = data[1];
	g_print ("%s (%s)\n", __func__, cReceivedData);
	_terminal_write_command_with_data (pWidget, "", cReceivedData);
}
static void _terminal_cd (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pWidget = data[0];
	gchar *cReceivedData = data[1];
	g_print ("%s (%s)\n", __func__, cReceivedData);
	_terminal_write_command_with_data (pWidget, "cd ", cReceivedData);
}
static void _terminal_cp (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pWidget = data[0];
	gchar *cReceivedData = data[1];
	g_print ("%s (%s)\n", __func__, cReceivedData);
	_terminal_write_command_with_data (pWidget, "cp -r ", cReceivedData);
}
static void _terminal_mv (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pWidget = data[0];
	gchar *cReceivedData = data[1];
	g_print ("%s (%s)\n", __func__, cReceivedData);
	_terminal_write_command_with_data (pWidget, "mv ", cReceivedData);
}
static void _terminal_rm (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pWidget = data[0];
	gchar *cReceivedData = data[1];
	g_print ("%s (%s)\n", __func__, cReceivedData);
	_terminal_write_command_with_data (pWidget, "rm -r ", cReceivedData);
}



static GtkWidget *_terminal_build_menu (GtkWidget *pWidget, gchar *cReceivedData)
{
	static gpointer *my_data = NULL;
	if (my_data == NULL)
		my_data = g_new0 (gpointer, 2);
	my_data[0] = pWidget;
	my_data[1] = cReceivedData;
	GtkWidget *menu = gtk_menu_new ();

	GtkWidget *menu_item, *image;
	menu_item = gtk_image_menu_item_new_with_label (_("Copy"));
	image = gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_copy), my_data);

	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_label ("cd");
	image = gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_cd), my_data);

	menu_item = gtk_image_menu_item_new_with_label ("cp");
	image = gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_cp), my_data);

	menu_item = gtk_image_menu_item_new_with_label ("mv");
	image = gtk_image_new_from_stock (GTK_STOCK_GOTO_LAST, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_mv), my_data);

	menu_item = gtk_image_menu_item_new_with_label ("rm");
	image = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK(_terminal_rm), my_data);

	return menu;
}


void on_terminal_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data)
{
	static gchar *cReceivedData = NULL;  // on en peut recevoir qu'un drop a la fois, donc pas de collision possible.
	g_print ("%s ()\n", __func__);

	g_free (cReceivedData);
	cReceivedData = g_strdup ((gchar *) selection_data->data);
	gchar *str = strrchr (cReceivedData, '\n');
	if (str != NULL)
		*(str-1) = '\0';  // on vire les retrours chariot.
	g_print (">>> cReceivedData : %s\n", cReceivedData);

	GtkWidget *menu = _terminal_build_menu (pWidget, cReceivedData);

	gtk_widget_show_all (menu);

	gtk_menu_popup (GTK_MENU (menu),
		NULL,
		NULL,
		NULL,
		NULL,
		1,
		gtk_get_current_event_time ());
}


gboolean on_terminal_button_press_dialog (GtkWidget* pWidget, GdkEventButton* pButton, gpointer data)  // a connecter au dialog->pWidget.
{
	cairo_dock_hide_dialog (term.dialog);
	return FALSE;
}