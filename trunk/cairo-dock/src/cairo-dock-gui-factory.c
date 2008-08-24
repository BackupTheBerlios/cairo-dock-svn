/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <unistd.h>
#define __USE_XOPEN_EXTENDED
#include <stdlib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include "cairo-dock-modules.h"
#include "cairo-dock-gui-factory.h"
#include "cairo-dock-log.h"
#include "cairo-dock-applet-facility.h"

#define CAIRO_DOCK_GUI_MARGIN 4
#define CAIRO_DOCK_ICON_MARGIN 6
#define CAIRO_DOCK_PREVIEW_WIDTH 250
#define CAIRO_DOCK_PREVIEW_HEIGHT 150
#define CAIRO_DOCK_APPLET_ICON_SIZE 32
#define CAIRO_DOCK_TAB_ICON_SIZE 32
#define CAIRO_DOCK_FRAME_ICON_SIZE 24

#ifndef mkstemp
int mkstemp(char *template);
#endif

extern gboolean g_bPopUp;

typedef enum {
	CAIRO_DOCK_MODEL_NAME = 0,
	CAIRO_DOCK_MODEL_RESULT,
	CAIRO_DOCK_MODEL_DESCRIPTION_FILE,
	CAIRO_DOCK_MODEL_ACTIVE,
	CAIRO_DOCK_MODEL_ORDER,
	CAIRO_DOCK_MODEL_IMAGE,
	CAIRO_DOCK_MODEL_ICON,
	CAIRO_DOCK_MODEL_NB_COLUMNS
	} _CairoDockModelColumns;

static GtkListStore *s_pRendererListStore = NULL;

static void _cairo_dock_activate_one_element (GtkCellRendererToggle * cell_renderer, gchar * path, GtkTreeModel * model)
{
	GtkTreeIter iter;
	gtk_tree_model_get_iter_from_string (model, &iter, path);
	gboolean bState;
	gtk_tree_model_get (model, &iter, CAIRO_DOCK_MODEL_ACTIVE, &bState, -1);

	gtk_list_store_set (GTK_LIST_STORE (model), &iter, CAIRO_DOCK_MODEL_ACTIVE, !bState, -1);
}
static void _cairo_dock_activate_one_module (GtkCellRendererToggle * cell_renderer, gchar * path, GtkTreeModel * model)
{
	GtkTreeIter iter;
	gtk_tree_model_get_iter_from_string (model, &iter, path);
	gboolean bState;
	gchar *cModuleName = NULL;
	gtk_tree_model_get (model, &iter,
		CAIRO_DOCK_MODEL_ACTIVE, &bState,
		CAIRO_DOCK_MODEL_NAME, &cModuleName, -1);
	
	if (! bState)
	{
		cairo_dock_activate_module_and_load (cModuleName);
	}
	else
	{
		cairo_dock_deactivate_module_and_unload (cModuleName);
	}
	g_free (cModuleName);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, CAIRO_DOCK_MODEL_ACTIVE, !bState, -1);
}

static gboolean _cairo_dock_increase_order (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, int *pOrder)
{
	int iMyOrder;
	gtk_tree_model_get (model, iter, CAIRO_DOCK_MODEL_ORDER, &iMyOrder, -1);

	if (iMyOrder == *pOrder)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), iter, CAIRO_DOCK_MODEL_ORDER, iMyOrder + 1, -1);
		return TRUE;
	}
	return FALSE;
}

static gboolean _cairo_dock_decrease_order (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, int *pOrder)
{
	int iMyOrder;
	gtk_tree_model_get (model, iter, CAIRO_DOCK_MODEL_ORDER, &iMyOrder, -1);

	if (iMyOrder == *pOrder)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), iter, CAIRO_DOCK_MODEL_ORDER, iMyOrder - 1, -1);
		return TRUE;
	}
	return FALSE;
}

static gboolean _cairo_dock_decrease_order_if_greater (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, int *pOrder)
{
	int iMyOrder;
	gtk_tree_model_get (model, iter, CAIRO_DOCK_MODEL_ORDER, &iMyOrder, -1);

	if (iMyOrder > *pOrder)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), iter, CAIRO_DOCK_MODEL_ORDER, iMyOrder - 1, -1);
		return TRUE;
	}
	return FALSE;
}

static void _cairo_dock_go_up (GtkButton *button, GtkTreeView *pTreeView)
{
	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);

	GtkTreeModel *pModel;
	GtkTreeIter iter;
	if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
		return ;

	int iOrder;
	gtk_tree_model_get (pModel, &iter, CAIRO_DOCK_MODEL_ORDER, &iOrder, -1);
	iOrder --;
	if (iOrder < 0)
		return;

	gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _cairo_dock_increase_order, &iOrder);

	gtk_list_store_set (GTK_LIST_STORE (pModel), &iter, CAIRO_DOCK_MODEL_ORDER, iOrder, -1);
}

static void _cairo_dock_go_down (GtkButton *button, GtkTreeView *pTreeView)
{
	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);

	GtkTreeModel *pModel;
	GtkTreeIter iter;
	if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
		return ;

	int iOrder;
	gtk_tree_model_get (pModel, &iter, CAIRO_DOCK_MODEL_ORDER, &iOrder, -1);
	iOrder ++;
	//g_print ("  ordre max : %d\n", gtk_tree_model_iter_n_children (pModel, NULL) - 1);
	if (iOrder > gtk_tree_model_iter_n_children (pModel, NULL) - 1)
		return;

	gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _cairo_dock_decrease_order, &iOrder);

	gtk_list_store_set (GTK_LIST_STORE (pModel), &iter, CAIRO_DOCK_MODEL_ORDER, iOrder, -1);
}

static void _cairo_dock_configure (GtkButton *button, gpointer *data)
{
	GtkTreeView *pTreeView = data[0];
	GtkWindow *pDialog = data[1];
	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);

	GtkTreeModel *pModel;
	GtkTreeIter iter;
	if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
		return ;

	gchar *cSelectedValue = NULL;
	gtk_tree_model_get (pModel, &iter, CAIRO_DOCK_MODEL_RESULT, &cSelectedValue, -1);

	cairo_dock_configure_module (NULL, cSelectedValue);
	g_free (cSelectedValue);
}

static void _cairo_dock_add (GtkButton *button, gpointer *data)
{
	GtkTreeView *pTreeView = data[0];
	GtkWidget *pEntry = data[1];

	GtkTreeIter iter;
	memset (&iter, 0, sizeof (GtkTreeIter));

	GtkTreeModel *pModel = gtk_tree_view_get_model (pTreeView);
	gtk_list_store_append (GTK_LIST_STORE (pModel), &iter);

	gtk_list_store_set (GTK_LIST_STORE (pModel), &iter,
		CAIRO_DOCK_MODEL_ACTIVE, TRUE,
		CAIRO_DOCK_MODEL_NAME, gtk_entry_get_text (GTK_ENTRY (pEntry)),
		CAIRO_DOCK_MODEL_ORDER, gtk_tree_model_iter_n_children (pModel, NULL) - 1, -1);
	//g_print (" -> ordre %d\n", gtk_tree_model_iter_n_children (pModel, NULL) - 1);

	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);
	gtk_tree_selection_select_iter (pSelection, &iter);
}

static void _cairo_dock_remove (GtkButton *button, gpointer *data)
{
	GtkTreeView *pTreeView = data[0];
	GtkWidget *pEntry = data[1];

	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);
	GtkTreeModel *pModel;

	GtkTreeIter iter;
	if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
	return ;

	gchar *cValue = NULL;
	int iOrder;
	gtk_tree_model_get (pModel, &iter,
		CAIRO_DOCK_MODEL_NAME, &cValue,
		CAIRO_DOCK_MODEL_ORDER, &iOrder, -1);

	gtk_list_store_remove (GTK_LIST_STORE (pModel), &iter);
	gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _cairo_dock_decrease_order_if_greater, &iOrder);

	gtk_entry_set_text (GTK_ENTRY (pEntry), cValue);
	g_free (cValue);
}

static void _cairo_dock_selection_changed (GtkTreeModel *model, GtkTreeIter iter, gpointer *data)
{
	GtkLabel *pDescriptionLabel = data[0];
	GtkImage *pPreviewImage = data[1];

	gchar *cDescriptionFilePath = NULL, *cPreviewFilePath;
	gtk_tree_model_get (model, &iter, CAIRO_DOCK_MODEL_DESCRIPTION_FILE, &cDescriptionFilePath, CAIRO_DOCK_MODEL_IMAGE, &cPreviewFilePath, -1);

	if (cDescriptionFilePath != NULL)
	{
		gboolean bDistant = FALSE;
		if (strncmp (cDescriptionFilePath, "http://", 7) == 0 || strncmp (cDescriptionFilePath, "ftp://", 6) == 0)
		{
			g_print ("fichier readme distant (%s)\n", cDescriptionFilePath);
			
			gchar *cTmpFilePath = g_strdup ("/tmp/cairo-dock-net-readme.XXXXXX");
			int fds = mkstemp (cTmpFilePath);
			if (fds == -1)
			{
				g_free (cTmpFilePath);
				return ;
			}
			
			gchar *cCommand = g_strdup_printf ("wget \"%s\" -O '%s' -t 2 -w 2", cDescriptionFilePath, cTmpFilePath);
			system (cCommand);
			g_free (cCommand);
			close(fds);
			
			g_free (cDescriptionFilePath);
			cDescriptionFilePath = cTmpFilePath;
			bDistant = TRUE;
		}
		gchar *cDescription = NULL;
		gsize length = 0;
		GError *erreur = NULL;
		g_file_get_contents  (cDescriptionFilePath,
			&cDescription,
			&length,
			&erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			cDescription = g_strdup ("");
		}
		gtk_label_set_markup (pDescriptionLabel, cDescription);
		g_free (cDescription);
		if (bDistant)
		{
			g_remove (cDescriptionFilePath);
		}
	}

	if (cPreviewFilePath != NULL)
	{
		gboolean bDistant = FALSE;
		if (strncmp (cPreviewFilePath, "http://", 7) == 0 || strncmp (cPreviewFilePath, "ftp://", 6) == 0)
		{
			g_print ("fichier preview distant (%s)\n", cPreviewFilePath);
			
			gchar *cTmpFilePath = g_strdup ("/tmp/cairo-dock-net-preview.XXXXXX");
			int fds = mkstemp (cTmpFilePath);
			if (fds == -1)
			{
				g_free (cTmpFilePath);
				return ;
			}
			
			gchar *cCommand = g_strdup_printf ("wget \"%s\" -O '%s' -t 2 -w 2", cPreviewFilePath, cTmpFilePath);
			system (cCommand);
			g_free (cCommand);
			close(fds);
			
			g_free (cPreviewFilePath);
			cPreviewFilePath = cTmpFilePath;
			bDistant = TRUE;
		}
		
		int iPreviewWidth, iPreviewHeight;
		GdkPixbuf *pPreviewPixbuf = NULL;
		if (gdk_pixbuf_get_file_info (cPreviewFilePath, &iPreviewWidth, &iPreviewHeight) != NULL)
		{
			iPreviewWidth = MIN (iPreviewWidth, CAIRO_DOCK_PREVIEW_WIDTH);
			iPreviewHeight = MIN (iPreviewHeight, CAIRO_DOCK_PREVIEW_HEIGHT);
			pPreviewPixbuf = gdk_pixbuf_new_from_file_at_size (cPreviewFilePath, iPreviewWidth, iPreviewHeight, NULL);
		}
		if (pPreviewPixbuf == NULL)
		{
			cd_warning ("pas de prevue disponible\n");
			pPreviewPixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
				TRUE,
				8,
				1,
				1);
		}
		gtk_image_set_from_pixbuf (pPreviewImage, pPreviewPixbuf);
		gdk_pixbuf_unref (pPreviewPixbuf);
		
		if (bDistant)
		{
			g_remove (cPreviewFilePath);
		}
	}

	g_free (cDescriptionFilePath);
	g_free (cPreviewFilePath);
}

static void _cairo_dock_select_one_item_in_combo (GtkComboBox *widget, gpointer *data)
{
	GtkTreeModel *model = gtk_combo_box_get_model (widget);
	g_return_if_fail (model != NULL);

	GtkTreeIter iter;
	gtk_combo_box_get_active_iter (widget, &iter);

	_cairo_dock_selection_changed (model, iter, data);
}
static gboolean _cairo_dock_select_one_item_in_tree (GtkTreeSelection * selection, GtkTreeModel * model, GtkTreePath * path, gboolean path_currently_selected, gpointer *data)
{
	GtkTreeIter iter;
	gtk_tree_model_get_iter (model, &iter, path);

	_cairo_dock_selection_changed (model, iter, data);
	return TRUE;
}


static void _cairo_dock_show_image_preview (GtkFileChooser *pFileChooser, GtkImage *pPreviewImage)
{
	gchar *cFileName = gtk_file_chooser_get_preview_filename (pFileChooser);
	if (cFileName == NULL)
		return ;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cFileName, 64, 64, NULL);
	g_free (cFileName);
	if (pixbuf != NULL)
	{
		gtk_image_set_from_pixbuf (pPreviewImage, pixbuf);
		gdk_pixbuf_unref (pixbuf);
		gtk_file_chooser_set_preview_widget_active (pFileChooser, TRUE);
	}
	else
		gtk_file_chooser_set_preview_widget_active (pFileChooser, FALSE);
}
static void _cairo_dock_pick_a_file (GtkButton *button, gpointer *data)
{
	GtkEntry *pEntry = data[0];
	gint iFileType = GPOINTER_TO_INT (data[1]);
	GtkWindow *pParentWindow = data[2];

	GtkWidget* pFileChooserDialog = gtk_file_chooser_dialog_new (
		(iFileType == 0 ? "Pick up a file" : "Pick up a directory"),
		pParentWindow,
		(iFileType == 0 ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER),
		GTK_STOCK_OK,
		GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);
	gchar *cDirectoryPath = g_path_get_basename (gtk_entry_get_text (pEntry));
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (pFileChooserDialog), cDirectoryPath);
	g_free (cDirectoryPath);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (pFileChooserDialog), FALSE);

	GtkWidget *pPreviewImage = gtk_image_new ();
	gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (pFileChooserDialog), pPreviewImage);
	g_signal_connect (GTK_FILE_CHOOSER (pFileChooserDialog), "update-preview", G_CALLBACK (_cairo_dock_show_image_preview), pPreviewImage);

	gtk_widget_show (pFileChooserDialog);
	int answer = gtk_dialog_run (GTK_DIALOG (pFileChooserDialog));
	if (answer == GTK_RESPONSE_OK)
	{
		gchar *cFilePath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (pFileChooserDialog));
		gtk_entry_set_text (pEntry, cFilePath);
	}
	gtk_widget_destroy (pFileChooserDialog);
}

//Sound Callback
static void _cairo_dock_play_a_sound (GtkButton *button, gpointer *data)
{
	GtkWidget *pEntry = data[0];
	const gchar *cSoundPath = gtk_entry_get_text (GTK_ENTRY (pEntry));
	cairo_dock_play_sound (cSoundPath);
}

static void _cairo_dock_key_grab_cb (GtkWidget *wizard_window, GdkEventKey *event, GtkEntry *pEntry)
{
	gchar *key;
	cd_message ("key press event\n");
	if (gtk_accelerator_valid (event->keyval, event->state))
	{
		/* This lets us ignore all ignorable modifier keys, including
		* NumLock and many others. :)
		*
		* The logic is: keep only the important modifiers that were pressed
		* for this event. */
		event->state &= gtk_accelerator_get_default_mod_mask();

		/* Generate the correct name for this key */
		key = gtk_accelerator_name (event->keyval, event->state);

		g_printerr ("KEY GRABBED: %s\n", key);

		/* Re-enable widgets */
		gtk_widget_set_sensitive (GTK_WIDGET(pEntry), TRUE);

		/* Disconnect the key grabber */
		g_signal_handlers_disconnect_by_func (GTK_OBJECT(wizard_window), GTK_SIGNAL_FUNC(_cairo_dock_key_grab_cb), pEntry);

		/* Copy the pressed key to the text entry */
		gtk_entry_set_text (GTK_ENTRY(pEntry), key);

		/* Free the string */
		g_free (key);
	}
}

static void _cairo_dock_key_grab_clicked (GtkButton *button, gpointer *data)
{
	GtkEntry *pEntry = data[0];
	GtkWindow *pParentWindow = data[1];

	cd_message ("clicked\n");
	//set widget insensitive
	gtk_widget_set_sensitive (GTK_WIDGET(pEntry), FALSE);
	//  gtk_widget_set_sensitive (wizard_notebook, FALSE);

	g_signal_connect (GTK_WIDGET(pParentWindow), "key-press-event", GTK_SIGNAL_FUNC(_cairo_dock_key_grab_cb), pEntry);
}

static void _cairo_dock_set_font (GtkFontButton *widget, GtkEntry *pEntry)
{
	const gchar *cFontName = gtk_font_button_get_font_name (GTK_FONT_BUTTON (widget));
	cd_message (" -> %s\n", cFontName);
	if (cFontName != NULL)
		gtk_entry_set_text (pEntry, cFontName);
}

static void _cairo_dock_set_color (GtkColorButton *pColorButton, GSList *pWidgetList)
{
	GdkColor gdkColor;
	gtk_color_button_get_color (pColorButton, &gdkColor);

	GtkSpinButton *pSpinButton;
	GSList *pList = pWidgetList;
	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gtk_spin_button_set_value (pSpinButton, 1. * gdkColor.red / 65535);
	pList = pList->next;

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gtk_spin_button_set_value (pSpinButton, 1. * gdkColor.green / 65535);
	pList = pList->next;

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gtk_spin_button_set_value (pSpinButton, 1. * gdkColor.blue / 65535);
	pList = pList->next;

	if (gtk_color_button_get_use_alpha (pColorButton))
	{
		if (pList == NULL)
		return;
		pSpinButton = pList->data;
		gtk_spin_button_set_value (pSpinButton, 1. * gtk_color_button_get_alpha (pColorButton) / 65535);
	}
}

static void _cairo_dock_get_current_color (GtkColorButton *pColorButton, GSList *pWidgetList)
{
	GdkColor gdkColor;
	GtkSpinButton *pSpinButton;

	GSList *pList = pWidgetList;
	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gdkColor.red = gtk_spin_button_get_value (pSpinButton) * 65535;
	pList = pList->next;

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gdkColor.green = gtk_spin_button_get_value (pSpinButton) * 65535;
	pList = pList->next;

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gdkColor.blue = gtk_spin_button_get_value (pSpinButton) * 65535;
	pList = pList->next;

	gtk_color_button_set_color (pColorButton, &gdkColor);

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	if (gtk_color_button_get_use_alpha (pColorButton))
		gtk_color_button_set_alpha (pColorButton, gtk_spin_button_get_value (pSpinButton) * 65535);
}

/*static gboolean _cairo_dock_free_conf_widget_data (GtkWidget *pWidget, GdkEvent *event, gpointer *user_data)
{
	g_print ("%s ()\n", __func__);
	GSList *pWidgetList = user_data[0];
	GPtrArray *pDataGarbage = user_data[1];
	GPtrArray *pModelGarbage = user_data[2];
	
	if (pWidgetList != NULL)
	{
		cairo_dock_free_generated_widget_list (pWidgetList);
	}
	if (pDataGarbage != NULL)
	{
		g_ptr_array_foreach (pDataGarbage, (GFunc) g_free, NULL);
		g_ptr_array_free (pDataGarbage, TRUE);
	}
	if (pModelGarbage != NULL)
	{
		//g_ptr_array_foreach (pModelGarbage, (GFunc) gtk_list_store_clear, NULL);
		//g_ptr_array_foreach (pModelGarbage, (GFunc) g_object_unref, NULL);
		g_ptr_array_free (pModelGarbage, TRUE);
	}
	g_free (user_data);
	g_print (" FINn", __func__);
	
	return FALSE;
}*/

void _cairo_dock_add_one_renderer_item (gchar *cName, CairoDockRenderer *pRenderer, GtkListStore *pModele)
{
	GtkTreeIter iter;
	memset (&iter, 0, sizeof (GtkTreeIter));
	gtk_list_store_append (GTK_LIST_STORE (pModele), &iter);
	gtk_list_store_set (GTK_LIST_STORE (pModele), &iter,
		CAIRO_DOCK_MODEL_NAME, cName,
		CAIRO_DOCK_MODEL_RESULT, cName,
		CAIRO_DOCK_MODEL_DESCRIPTION_FILE, (pRenderer != NULL ? pRenderer->cReadmeFilePath : "none"),
		CAIRO_DOCK_MODEL_IMAGE, (pRenderer != NULL ? pRenderer->cPreviewFilePath : "none"), -1);
}
void cairo_dock_build_renderer_list_for_gui (GHashTable *pHashTable)
{
	if (s_pRendererListStore != NULL)
		g_object_unref (s_pRendererListStore);  // gtk_list_store_clear (s_pRendererListStore) fait planter :-(
	
	s_pRendererListStore = gtk_list_store_new (CAIRO_DOCK_MODEL_NB_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_STRING, GDK_TYPE_PIXBUF);
	//g_object_ref (s_pRendererListStore);
	
	_cairo_dock_add_one_renderer_item ("", NULL, s_pRendererListStore);
	g_hash_table_foreach (pHashTable, (GHFunc) _cairo_dock_add_one_renderer_item, s_pRendererListStore);
}

static gboolean _cairo_dock_test_one_renderer_name (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer *data)
{
	gchar *cName = NULL;
	gtk_tree_model_get (model, iter, CAIRO_DOCK_MODEL_NAME, &cName, -1);
	if (strcmp (data[0], cName) == 0)
	{
		GtkTreeIter *iter_to_fill = data[1];
		memcpy (iter_to_fill, iter, sizeof (GtkTreeIter));
		gboolean *bFound = data[2];
		*bFound = TRUE;
		return TRUE;
	}
	return FALSE;
}
gboolean _cairo_dock_find_iter_from_renderer_name (gchar *cName, GtkTreeIter *iter)
{
	if (cName == NULL)
		return FALSE;
	gboolean bFound = FALSE;
	gpointer data[3] = {cName, iter, &bFound};
	gtk_tree_model_foreach (GTK_TREE_MODEL (s_pRendererListStore), (GtkTreeModelForeachFunc) _cairo_dock_test_one_renderer_name, data);
	return bFound;
}

static void _cairo_dock_configure_renderer (GtkButton *button, gpointer *data)
{
	GtkTreeView *pCombo = data[0];
	GtkWindow *pDialog = data[1];
	 
	cairo_dock_configure_module (pDialog, "rendering");
}

#define _allocate_new_buffer\
	data = g_new (gpointer, 3); \
	g_ptr_array_add (pDataGarbage, data);

#define _allocate_new_model\
	modele = gtk_list_store_new (CAIRO_DOCK_MODEL_NB_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_STRING, GDK_TYPE_PIXBUF);

GtkWidget *cairo_dock_generate_advanced_ihm_from_keyfile (GKeyFile *pKeyFile, const gchar *cTitle, GtkWindow *pParentWindow, GSList **pWidgetList, gboolean bApplyButtonPresent, gchar iIdentifier, gchar *cPresentedGroup, gboolean bSwitchButtonPresent, gchar *cButtonConvert, gchar *cGettextDomain, GPtrArray *pDataGarbage)
{
	g_return_val_if_fail (cairo_dock_is_advanced_keyfile (pKeyFile), NULL);
	
	//GPtrArray *pDataGarbage = g_ptr_array_new ();
	//GPtrArray *pModelGarbage = g_ptr_array_new ();
	
	gpointer *data;
	int iNbBuffers = 0;
	gsize length = 0;
	gchar **pKeyList;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	
	GtkWidget *pOneWidget;
	GSList * pSubWidgetList;
	GtkWidget *pLabel, *pLabelContainer;
	GtkWidget *pVBox, *pHBox, *pSmallVBox, *pEventBox, *pRightHBox;
	GtkWidget *pEntry;
	GtkWidget *pTable;
	GtkWidget *pButtonAdd, *pButtonRemove;
	GtkWidget *pButtonDown, *pButtonUp, *pButtonConfig;
	GtkWidget *pButtonFileChooser, *pButtonPlay;
	GtkWidget *pFrame, *pFrameVBox;
	GtkWidget *pScrolledWindow;
	GtkWidget *pColorButton;
	GtkWidget *pFontButton;
	GtkWidget *pDescriptionLabel;
	GtkWidget *pPreviewImage;
	GtkWidget *pButtonConfigRenderer;
	gchar *cGroupName, *cGroupComment , *cKeyName, *cKeyComment, *cUsefulComment, *cAuthorizedValuesChain, *pTipString, **pAuthorizedValuesList, *cSmallGroupIcon;
	gpointer *pGroupKeyWidget;
	int i, j, k, iNbElements;
	int iNumPage=0, iPresentedNumPage=0;
	char iElementType;
	gboolean bIsAligned;
	gboolean bValue, *bValueList;
	int iValue, iMinValue, iMaxValue, *iValueList;
	double fValue, fMinValue, fMaxValue, *fValueList;
	gchar *cValue, **cValueList, *cSmallIcon;
	GdkColor gdkColor;
	GtkListStore *modele;
	
	GtkWidget *pDialog;
	if (bApplyButtonPresent)
	{
		if (bSwitchButtonPresent)
		pDialog = gtk_dialog_new_with_buttons ((cTitle != NULL ? cTitle : ""),
			(pParentWindow != NULL ? GTK_WINDOW (pParentWindow) : NULL),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			(cButtonConvert != NULL ? cButtonConvert : GTK_STOCK_CONVERT),
			GTK_RESPONSE_HELP,
			GTK_STOCK_APPLY,
			GTK_RESPONSE_APPLY,
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
			GTK_STOCK_QUIT,
			GTK_RESPONSE_REJECT,
			NULL);
		else
		pDialog = gtk_dialog_new_with_buttons ((cTitle != NULL ? cTitle : ""),
			(pParentWindow != NULL ? GTK_WINDOW (pParentWindow) : NULL),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_APPLY,
			GTK_RESPONSE_APPLY,
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
			GTK_STOCK_QUIT,
			GTK_RESPONSE_REJECT,
			NULL);
	}
	else
	{
		pDialog = gtk_dialog_new_with_buttons ((cTitle != NULL ? cTitle : ""),
			(pParentWindow != NULL ? GTK_WINDOW (pParentWindow) : NULL),
			GTK_DIALOG_DESTROY_WITH_PARENT,  // GTK_DIALOG_MODAL | 
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
			GTK_STOCK_QUIT,
			GTK_RESPONSE_REJECT,
			NULL);
	}
	if (g_bPopUp)
	{
		gtk_window_set_keep_above (GTK_WINDOW (pDialog), TRUE);
		gtk_window_set_keep_below (GTK_WINDOW (pDialog), FALSE);
		gtk_window_present (GTK_WINDOW (pDialog));
	}
	gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), CAIRO_DOCK_GUI_MARGIN);
	
	GtkTooltips *pToolTipsGroup = gtk_tooltips_new ();
	
	GtkWidget *pNoteBook = gtk_notebook_new ();
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (pNoteBook), TRUE);
	gtk_notebook_popup_enable (GTK_NOTEBOOK (pNoteBook));
	g_object_set (G_OBJECT (pNoteBook), "tab-pos", GTK_POS_LEFT, NULL);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), pNoteBook);
	
	i = 0;
	while (pGroupList[i] != NULL)
	{
		pVBox = NULL;
		pFrame = NULL;
		pFrameVBox = NULL;
		cGroupName = pGroupList[i];
		cGroupComment  = g_key_file_get_comment (pKeyFile, cGroupName, NULL, NULL);
		cSmallGroupIcon = NULL;
		if (cGroupComment != NULL)
		{
			cGroupComment[strlen(cGroupComment)-1] = '\0';
			gchar *str = strrchr (cGroupComment, '[');
			if (str != NULL)
			{
				cSmallGroupIcon = str+1;
				str = strrchr (cSmallGroupIcon, ']');
				if (str != NULL)
					*str = '\0';
			}
		}
		

		pKeyList = g_key_file_get_keys (pKeyFile, cGroupName, NULL, NULL);

		j = 0;
		while (pKeyList[j] != NULL)
		{
			cKeyName = pKeyList[j];

			cKeyComment =  g_key_file_get_comment (pKeyFile, cGroupName, cKeyName, NULL);
			//g_print ("%s -> %s\n", cKeyName, cKeyComment);
			if (cKeyComment != NULL && strcmp (cKeyComment, "") != 0)
			{
				cUsefulComment = cKeyComment;
				while (*cUsefulComment == '#' || *cUsefulComment == ' ')  // on saute les # et les espaces.
					cUsefulComment ++;

				iElementType = *cUsefulComment;
				cUsefulComment ++;

				if (! g_ascii_isdigit (*cUsefulComment) && *cUsefulComment != '[')
				{
					if (iIdentifier != 0 && *cUsefulComment != iIdentifier)
					{
						g_free (cKeyComment);
						j ++;
						continue;
					}
					cUsefulComment ++;
				}

				if (pVBox == NULL)  // maintenant qu'on a au moins un element dans ce groupe, on cree sa page dans le notebook.
				{
					pLabel = gtk_label_new (dgettext (cGettextDomain, cGroupName));
					
					pLabelContainer = NULL;
					GtkWidget *pAlign = NULL;
					if (cSmallGroupIcon != NULL && *cSmallGroupIcon != '\0')
					{
						pLabelContainer = gtk_hbox_new (FALSE, CAIRO_DOCK_ICON_MARGIN);
						pAlign = gtk_alignment_new (0., 0.5, 0., 0.);
						gtk_container_add (GTK_CONTAINER (pAlign), pLabelContainer);
						
						GtkWidget *pImage = gtk_image_new ();
						GdkPixbuf *pixbuf;
						if (*cSmallGroupIcon != '/')
							pixbuf = gtk_widget_render_icon (pImage,
								cSmallGroupIcon ,
								GTK_ICON_SIZE_BUTTON,
								NULL);
						else
							pixbuf = gdk_pixbuf_new_from_file_at_size (cSmallGroupIcon, CAIRO_DOCK_TAB_ICON_SIZE, CAIRO_DOCK_TAB_ICON_SIZE, NULL);
						if (pixbuf != NULL)
						{
							gtk_image_set_from_pixbuf (GTK_IMAGE (pImage), pixbuf);
							gdk_pixbuf_unref (pixbuf);
							gtk_container_add (GTK_CONTAINER (pLabelContainer),
								pImage);
						}
						gtk_container_add (GTK_CONTAINER (pLabelContainer),
							pLabel);
						gtk_widget_show_all (pLabelContainer);
					}
					
					pVBox = gtk_vbox_new (FALSE, CAIRO_DOCK_GUI_MARGIN);

					pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
					gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
					gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pVBox);

					gtk_notebook_append_page (GTK_NOTEBOOK (pNoteBook), pScrolledWindow, (pAlign != NULL ? pAlign : pLabel));
					if (cPresentedGroup != NULL && strcmp (cPresentedGroup, cGroupName) == 0)
						iPresentedNumPage = iNumPage;
					iNumPage ++;
				}

				if (g_ascii_isdigit (*cUsefulComment))
				{
					iNbElements = atoi (cUsefulComment);
					g_return_val_if_fail (iNbElements > 0, NULL);
					while (g_ascii_isdigit (*cUsefulComment))
						cUsefulComment ++;
				}
				else
				{
					iNbElements = 1;
				}
				//g_print ("%d element(s)\n", iNbElements);

				while (*cUsefulComment == ' ')  // on saute les espaces.
					cUsefulComment ++;

				if (*cUsefulComment == '[')
				{
					cUsefulComment ++;
					cAuthorizedValuesChain = cUsefulComment;

					while (*cUsefulComment != '\0' && *cUsefulComment != ']')
						cUsefulComment ++;
					g_return_val_if_fail (*cUsefulComment != '\0', NULL);
					*cUsefulComment = '\0';
					cUsefulComment ++;
					while (*cUsefulComment == ' ')  // on saute les espaces.
						cUsefulComment ++;

					pAuthorizedValuesList = g_strsplit (cAuthorizedValuesChain, ";", 0);
				}
				else
				{
					pAuthorizedValuesList = NULL;
				}
				if (cUsefulComment[strlen (cUsefulComment) - 1] == '\n')
					cUsefulComment[strlen (cUsefulComment) - 1] = '\0';
				if (cUsefulComment[strlen (cUsefulComment) - 1] == '/')
				{
					bIsAligned = FALSE;
					cUsefulComment[strlen (cUsefulComment) - 1] = '\0';
				}
				else
				{
					bIsAligned = TRUE;
				}
				//g_print ("cUsefulComment : %s\n", cUsefulComment);

				pTipString = strchr (cUsefulComment, '{');
				if (pTipString != NULL)
				{
					if (*(pTipString-1) == '\n')
						*(pTipString-1) ='\0';
					else
						*pTipString = '\0';

					pTipString ++;

					gchar *pTipEnd = strrchr (pTipString, '}');
					if (pTipEnd != NULL)
						*pTipEnd = '\0';
				}

				pHBox = gtk_hbox_new (FALSE, CAIRO_DOCK_GUI_MARGIN);
				if (pTipString != NULL)
				{
					//g_print ("pTipString : '%s'\n", pTipString);
					pEventBox = gtk_event_box_new ();
					gtk_container_add (GTK_CONTAINER (pEventBox), pHBox);
					gtk_tooltips_set_tip (GTK_TOOLTIPS (pToolTipsGroup),
						pEventBox,
						dgettext (cGettextDomain, pTipString),
						"pouet");
				}
				else
					pEventBox = NULL;

				if (*cUsefulComment != '\0' && strcmp (cUsefulComment, "...") != 0 && iElementType != 'F' && iElementType != 'X')
				{
					pLabel = gtk_label_new (dgettext (cGettextDomain, cUsefulComment));
					GtkWidget *pAlign = gtk_alignment_new (0., 0.5, 0., 0.);
					gtk_container_add (GTK_CONTAINER (pAlign), pLabel);
					gtk_box_pack_start ((bIsAligned ? GTK_BOX (pHBox) : (pFrameVBox == NULL ? GTK_BOX (pVBox) : GTK_BOX (pFrameVBox))),
						pAlign,
						FALSE,
						FALSE,
						0);
				}

				gtk_box_pack_start (pFrameVBox == NULL ? GTK_BOX (pVBox) : GTK_BOX (pFrameVBox),
					(pEventBox != NULL ? pEventBox : pHBox),
					FALSE,
					FALSE,
					0);

				if (bIsAligned)
				{
					pRightHBox = gtk_hbox_new (FALSE, CAIRO_DOCK_GUI_MARGIN);
					gtk_box_pack_end (GTK_BOX (pHBox),
						pRightHBox,
						FALSE,
						FALSE,
						0);
					pHBox = pRightHBox;
				}

				pSubWidgetList = NULL;

				switch (iElementType)
				{
					case 'b' :  // boolean
						//g_print ("  + boolean\n");
						length = 0;
						bValueList = g_key_file_get_boolean_list (pKeyFile, cGroupName, cKeyName, &length, NULL);

						for (k = 0; k < iNbElements; k ++)
						{
							bValue =  (k < length ? bValueList[k] : FALSE);
							pOneWidget = gtk_check_button_new ();
							gtk_toggle_button_set_active  (GTK_TOGGLE_BUTTON (pOneWidget), bValue);

							pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
							gtk_box_pack_start (GTK_BOX (pHBox),
								pOneWidget,
								FALSE,
								FALSE,
								0);
						}
						g_free (bValueList);
					break;

					case 'i' :  // integer
					case 'I' :  // integer dans un HScale
						//g_print ("  + integer\n");
						length = 0;
						iValueList = g_key_file_get_integer_list (pKeyFile, cGroupName, cKeyName, &length, NULL);
						for (k = 0; k < iNbElements; k ++)
						{
							iValue =  (k < length ? iValueList[k] : 0);
							if (pAuthorizedValuesList != NULL && pAuthorizedValuesList[0] != NULL)
								iMinValue = g_ascii_strtod (pAuthorizedValuesList[0], NULL);
							else
								iMinValue = 0;
							if (pAuthorizedValuesList != NULL && pAuthorizedValuesList[1] != NULL)
								iMaxValue = g_ascii_strtod (pAuthorizedValuesList[1], NULL);
							else
								iMaxValue = 9999;

							GtkObject *pAdjustment = gtk_adjustment_new (iValue,
								0,
								1,
								1,
								MAX (1, (iMaxValue - iMinValue) / 20),
								0);

							if (iElementType == 'I')
							{
								pOneWidget = gtk_hscale_new (GTK_ADJUSTMENT (pAdjustment));
								gtk_scale_set_digits (GTK_SCALE (pOneWidget), 0);
								gtk_widget_set (pOneWidget, "width-request", 150, NULL);
							}
							else
							{
								pOneWidget = gtk_spin_button_new (GTK_ADJUSTMENT (pAdjustment),
									1.,
									0);
							}
							g_object_set (pAdjustment, "lower", (double) iMinValue, "upper", (double) iMaxValue, NULL); // le 'width-request' sur un GtkHScale avec 'fMinValue' non nul plante ! Donc on les met apres...
							gtk_adjustment_set_value (GTK_ADJUSTMENT (pAdjustment), iValue);

							pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
							gtk_box_pack_start(GTK_BOX (pHBox),
								pOneWidget,
								FALSE,
								FALSE,
								0);
						}
						g_free (iValueList);
					break;

					case 'f' :  // float.
					case 'c' :  // float avec un bouton de choix de couleur.
					case 'e' :  // float dans un HScale.
						//g_print ("  + float\n");
						length = 0;
						fValueList = g_key_file_get_double_list (pKeyFile, cGroupName, cKeyName, &length, NULL);
						for (k = 0; k < iNbElements; k ++)
						{
							fValue =  (k < length ? fValueList[k] : 0);
							if (pAuthorizedValuesList != NULL && pAuthorizedValuesList[0] != NULL)
								fMinValue = g_ascii_strtod (pAuthorizedValuesList[0], NULL);
							else
								fMinValue = 0;
							if (pAuthorizedValuesList != NULL && pAuthorizedValuesList[1] != NULL)
								fMaxValue = g_ascii_strtod (pAuthorizedValuesList[1], NULL);
							else
								fMaxValue = 9999;

							GtkObject *pAdjustment = gtk_adjustment_new (fValue,
								0,
								1,
								(fMaxValue - fMinValue) / 20.,
								(fMaxValue - fMinValue) / 10.,
								0);

							if (iElementType == 'e')
							{
								pOneWidget = gtk_hscale_new (GTK_ADJUSTMENT (pAdjustment));
								gtk_scale_set_digits (GTK_SCALE (pOneWidget), 3);
								gtk_widget_set (pOneWidget, "width-request", 150, NULL);
							}
							else
							{
								pOneWidget = gtk_spin_button_new (GTK_ADJUSTMENT (pAdjustment),
									1.,
									3);
							}
							g_object_set (pAdjustment, "lower", fMinValue, "upper", fMaxValue, NULL); // le 'width-request' sur un GtkHScale avec 'fMinValue' non nul plante ! Donc on les met apres...
							gtk_adjustment_set_value (GTK_ADJUSTMENT (pAdjustment), fValue);

							pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
							gtk_box_pack_start(GTK_BOX (pHBox),
								pOneWidget,
								FALSE,
								FALSE,
								0);
						}
						if (iElementType == 'c' && length > 2)
						{
							gdkColor.red = fValueList[0] * 65535;
							gdkColor.green = fValueList[1] * 65535;
							gdkColor.blue = fValueList[2] * 65535;
							pColorButton = gtk_color_button_new_with_color (&gdkColor);
							if (length > 3)
							{
								gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON (pColorButton), TRUE);
								gtk_color_button_set_alpha (GTK_COLOR_BUTTON (pColorButton), fValueList[3] * 65535);
							}
							else
								gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON (pColorButton), FALSE);

							gtk_box_pack_start (GTK_BOX (pHBox),
								pColorButton,
								FALSE,
								FALSE,
								0);
							g_signal_connect (G_OBJECT (pColorButton), "color-set", G_CALLBACK(_cairo_dock_set_color), pSubWidgetList);
							g_signal_connect (G_OBJECT (pColorButton), "clicked", G_CALLBACK(_cairo_dock_get_current_color), pSubWidgetList);
						}
						g_free (fValueList);
					break;

					case 'n' :
						cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, NULL);
						modele = s_pRendererListStore;
						pOneWidget = gtk_combo_box_new_with_model (GTK_TREE_MODEL (modele));
						GtkCellRenderer *rend = gtk_cell_renderer_text_new ();
						gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (pOneWidget), rend, FALSE);
						gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (pOneWidget), rend, "text", CAIRO_DOCK_MODEL_NAME, NULL);
						
						pDescriptionLabel = gtk_label_new (NULL);
						gtk_label_set_use_markup  (GTK_LABEL (pDescriptionLabel), TRUE);
						pPreviewImage = gtk_image_new_from_pixbuf (NULL);
						_allocate_new_buffer;
						data[0] = pDescriptionLabel;
						data[1] = pPreviewImage;
						g_signal_connect (G_OBJECT (pOneWidget), "changed", G_CALLBACK (_cairo_dock_select_one_item_in_combo), data);
						
						GtkWidget *pPreviewBox = gtk_vbox_new (FALSE, CAIRO_DOCK_GUI_MARGIN);
						gtk_box_pack_start (GTK_BOX (pHBox),
							pPreviewBox,
							FALSE,
							FALSE,
							0);
						gtk_box_pack_start (GTK_BOX (pPreviewBox),
							pDescriptionLabel,
							FALSE,
							FALSE,
							0);
						gtk_box_pack_start (GTK_BOX (pPreviewBox),
							pPreviewImage,
							FALSE,
							FALSE,
							0);
						
						GtkTreeIter iter;
						if (_cairo_dock_find_iter_from_renderer_name (cValue, &iter))
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (pOneWidget), &iter);
						
						pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
						gtk_box_pack_start (GTK_BOX (pHBox),
							pOneWidget,
							FALSE,
							FALSE,
							0);
						g_free (cValue);
						
						pButtonConfigRenderer = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
						_allocate_new_buffer;
						data[0] = pOneWidget;
						data[1] = pParentWindow;
						g_signal_connect (G_OBJECT (pButtonConfigRenderer),
							"clicked",
							G_CALLBACK (_cairo_dock_configure_renderer),
							data);
						gtk_box_pack_start (GTK_BOX (pHBox),
							pButtonConfigRenderer,
							FALSE,
							FALSE,
							0);
					break ;
					
					case 's' :  // string
					case 'S' :  // string avec un selecteur de fichier a cote du GtkEntry.
					case 'u' :  // string avec un selecteur de fichier a cote du GtkEntry et un boutton play.
					case 'D' :  // string avec un selecteur de repertoire a cote du GtkEntry.
					case 'T' :  // string, mais sans pouvoir decochez les cases.
					case 'E' :  // string, mais avec un GtkComboBoxEntry pour le choix unique.
					case 'R' :  // string, avec un label pour la description.
					case 'P' :  // string avec un selecteur de font a cote du GtkEntry.
					case 'r' :  // string representee par son numero dans une liste de choix.
					case 'M' :  // string, avec un label pour la description et un bouton configurer (specialement fait pour les modules).
					case 'k' :  // string avec un selecteur de touche clavier (Merci Ctaf !)
						//g_print ("  + string (%s)\n", cUsefulComment);
						pEntry = NULL;
						pDescriptionLabel = NULL;
						pPreviewImage = NULL;
						length = 0;
						GdkPixbuf *pixbuf;
						cValueList = g_key_file_get_locale_string_list (pKeyFile, cGroupName, cKeyName, NULL, &length, NULL);
						if (iNbElements == 1)
						{
							cValue =  (0 < length ? cValueList[0] : "");
							if (pAuthorizedValuesList == NULL || pAuthorizedValuesList[0] == NULL)
							{
								pOneWidget = gtk_entry_new ();
								pEntry = pOneWidget;
								gtk_entry_set_text (GTK_ENTRY (pOneWidget), cValue);
							}
							else
							{
								_allocate_new_model
								if (iElementType == 'E')
								{
									pOneWidget = gtk_combo_box_entry_new_with_model (GTK_TREE_MODEL (modele), CAIRO_DOCK_MODEL_NAME);
								}
								else
								{
									pOneWidget = gtk_combo_box_new_with_model (GTK_TREE_MODEL (modele));
									GtkCellRenderer *rend = gtk_cell_renderer_text_new ();
									gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (pOneWidget), rend, FALSE);
									gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (pOneWidget), rend, "text", CAIRO_DOCK_MODEL_NAME, NULL);
								}

								k = 0;
								int iSelectedItem = -1;
								if (iElementType == 'r')
									iSelectedItem = atoi (cValue);
								gchar *cResult = (iElementType == 'r' ? g_new0 (gchar , 10) : NULL);
								int ii, iNbElementsByItem = (iElementType == 'R' ? 3 : (iElementType == 'M' ? 4 : 1));
								while (pAuthorizedValuesList[k] != NULL)
								{
									for (ii=0;ii<iNbElementsByItem;ii++)
									{
										if (pAuthorizedValuesList[k+ii] == NULL)
										{
											cd_warning ("bad conf file format, you can try to delete it and restart the dock");
											break;
										}
									}
									if (ii != iNbElementsByItem)
										break;
									//g_print ("%d) %s\n", k, pAuthorizedValuesList[k]);
									GtkTreeIter iter;
									gtk_list_store_append (GTK_LIST_STORE (modele), &iter);
									if (iSelectedItem == -1 && strcmp (cValue, pAuthorizedValuesList[k]) == 0)
										iSelectedItem = k / iNbElementsByItem;

									if (cResult != NULL)
									{
										snprintf (cResult, 10, "%d", k);
									}
									if (iElementType == 'M')
										pixbuf = gdk_pixbuf_new_from_file_at_size (pAuthorizedValuesList[k+3], CAIRO_DOCK_APPLET_ICON_SIZE, CAIRO_DOCK_APPLET_ICON_SIZE, NULL);
									gtk_list_store_set (GTK_LIST_STORE (modele), &iter,
										CAIRO_DOCK_MODEL_NAME, (iElementType == 'r' ? dgettext (cGettextDomain, pAuthorizedValuesList[k]) : pAuthorizedValuesList[k]),
										CAIRO_DOCK_MODEL_RESULT, (cResult != NULL ? cResult : pAuthorizedValuesList[k]),
										CAIRO_DOCK_MODEL_DESCRIPTION_FILE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+1] : NULL),
										CAIRO_DOCK_MODEL_IMAGE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+2] : NULL),
										CAIRO_DOCK_MODEL_ICON, (iElementType == 'M' ? pixbuf : NULL), -1);

									k += iNbElementsByItem;
									if (iElementType == 'R' || iElementType == 'M')
									{
										if (pAuthorizedValuesList[k-2] == NULL)  // ne devrait pas arriver si le fichier de conf est bien rempli.
											break;
									}
								}
								g_free (cResult);
								if (k == 0)  // rien dans le gtktree => plantage.
								{
									j ++;
									continue;
								}
								if (iElementType == 'R' || iElementType == 'M')
								{
									pDescriptionLabel = gtk_label_new (NULL);
									gtk_label_set_use_markup  (GTK_LABEL (pDescriptionLabel), TRUE);
									pPreviewImage = gtk_image_new_from_pixbuf (NULL);
									_allocate_new_buffer;
									data[0] = pDescriptionLabel;
									data[1] = pPreviewImage;
									g_signal_connect (G_OBJECT (pOneWidget), "changed", G_CALLBACK (_cairo_dock_select_one_item_in_combo), data);
								}

								if (iElementType != 'E' && iSelectedItem == -1)
									iSelectedItem = 0;
								gtk_combo_box_set_active (GTK_COMBO_BOX (pOneWidget), iSelectedItem);
							}
							pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
							gtk_box_pack_start (GTK_BOX (pHBox),
								pOneWidget,
								FALSE,
								FALSE,
								0);
						}
						else
						{
							pOneWidget = gtk_tree_view_new ();
							_allocate_new_model
							gtk_tree_view_set_model (GTK_TREE_VIEW (pOneWidget), GTK_TREE_MODEL (modele));
							gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (modele), CAIRO_DOCK_MODEL_ORDER, GTK_SORT_ASCENDING);
							gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pOneWidget), FALSE);
							
							GtkCellRenderer *rend;
							if (pAuthorizedValuesList != NULL && iElementType != 'T')  // && pAuthorizedValuesList[0] != NULL
							{
								rend = gtk_cell_renderer_toggle_new ();
								gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pOneWidget), -1, NULL, rend, "active", CAIRO_DOCK_MODEL_ACTIVE, NULL);
								g_signal_connect (G_OBJECT (rend), "toggled", (GCallback) (iElementType == 'M' ? _cairo_dock_activate_one_module : _cairo_dock_activate_one_element), modele);
							}
							
							if (iElementType == 'M')
							{
								rend = gtk_cell_renderer_pixbuf_new ();
								gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pOneWidget), -1, NULL, rend, "pixbuf", CAIRO_DOCK_MODEL_ICON, NULL);
							}

							rend = gtk_cell_renderer_text_new ();
							gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pOneWidget), -1, NULL, rend, "text", CAIRO_DOCK_MODEL_NAME, NULL);
							GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pOneWidget));
							gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

							pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
							pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
							gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
							gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pOneWidget);
							if (iElementType == 'M')
								gtk_widget_set (pScrolledWindow, "height-request", (int) (150 + CAIRO_DOCK_PREVIEW_HEIGHT), NULL);
							gtk_box_pack_start (GTK_BOX (pHBox),
								pScrolledWindow,
								FALSE,
								FALSE,
								0);

							pSmallVBox = gtk_vbox_new (FALSE, 3);
							gtk_box_pack_start (GTK_BOX (pHBox),
								pSmallVBox,
								FALSE,
								FALSE,
								0);

							if (iElementType != 'M')
							{
								pButtonUp = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
								g_signal_connect (G_OBJECT (pButtonUp),
									"clicked",
									G_CALLBACK (_cairo_dock_go_up),
									pOneWidget);
								gtk_box_pack_start (GTK_BOX (pSmallVBox),
									pButtonUp,
									FALSE,
									FALSE,
									0);
	
								pButtonDown = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
								g_signal_connect (G_OBJECT (pButtonDown),
									"clicked",
									G_CALLBACK (_cairo_dock_go_down),
									pOneWidget);
								gtk_box_pack_start (GTK_BOX (pSmallVBox),
									pButtonDown,
									FALSE,
									FALSE,
									0);
							}
							else
							{
								_allocate_new_buffer;
								data[0] = pOneWidget;
								data[1] = pDialog;
								pButtonConfig = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
								g_signal_connect (G_OBJECT (pButtonConfig),
									"clicked",
									G_CALLBACK (_cairo_dock_configure),
									data);
								gtk_box_pack_start (GTK_BOX (pSmallVBox),
									pButtonConfig,
									FALSE,
									FALSE,
									0);
							}

							GtkTreeIter iter;
							int iNbElementsByItem = (iElementType == 'R' ? 3 : (iElementType == 'M' ? 4 : 1));
							if (pAuthorizedValuesList != NULL)  //  && pAuthorizedValuesList[0] != NULL
							{
								int l, iOrder = 0;
								for (l = 0; l < length; l ++)
								{
									cValue = cValueList[l];
									k = 0;
									while (pAuthorizedValuesList[k] != NULL)
									{
										if (strcmp (cValue, pAuthorizedValuesList[k]) == 0)
										{
											break;
										}
										k += iNbElementsByItem;
									}

									if (pAuthorizedValuesList[k] != NULL)  // c'etait bien une valeur autorisee.
									{
										memset (&iter, 0, sizeof (GtkTreeIter));
										gtk_list_store_append (modele, &iter);
										if (iElementType == 'M')
											pixbuf = gdk_pixbuf_new_from_file_at_size (pAuthorizedValuesList[k+3], CAIRO_DOCK_APPLET_ICON_SIZE, CAIRO_DOCK_APPLET_ICON_SIZE, NULL);
										gtk_list_store_set (modele, &iter,
											CAIRO_DOCK_MODEL_ACTIVE, TRUE,
											CAIRO_DOCK_MODEL_NAME, cValue,
											CAIRO_DOCK_MODEL_RESULT, cValue,
											CAIRO_DOCK_MODEL_DESCRIPTION_FILE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+1] : NULL),
											CAIRO_DOCK_MODEL_ORDER, iOrder ++,
											CAIRO_DOCK_MODEL_IMAGE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+2] : NULL), 
											CAIRO_DOCK_MODEL_ICON, (iElementType == 'M' ? pixbuf : NULL), -1);
									}
								}
								k = 0;
								while (pAuthorizedValuesList[k] != NULL)
								{
									cValue =  pAuthorizedValuesList[k];
									for (l = 0; l < length; l ++)
									{
										if (strcmp (cValue, cValueList[l]) == 0)
										{
											break;
										}
									}

									if (l == length)  // elle n'a pas encore ete inseree.
									{
										memset (&iter, 0, sizeof (GtkTreeIter));
										gtk_list_store_append (modele, &iter);
										if (iElementType == 'M')
											pixbuf = gdk_pixbuf_new_from_file_at_size (pAuthorizedValuesList[k+3], CAIRO_DOCK_APPLET_ICON_SIZE, CAIRO_DOCK_APPLET_ICON_SIZE, NULL);
										gtk_list_store_set (modele, &iter,
											CAIRO_DOCK_MODEL_ACTIVE, FALSE,
											CAIRO_DOCK_MODEL_NAME, cValue,
											CAIRO_DOCK_MODEL_RESULT, cValue,
											CAIRO_DOCK_MODEL_DESCRIPTION_FILE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+1] : NULL),
											CAIRO_DOCK_MODEL_ORDER, iOrder ++,
											CAIRO_DOCK_MODEL_IMAGE,
											(iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+2] : NULL), 
											CAIRO_DOCK_MODEL_ICON, (iElementType == 'M' ? pixbuf : NULL), -1);
									}
									k += iNbElementsByItem;
								}

								if (iElementType == 'R' || iElementType == 'M')
								{
									pDescriptionLabel = gtk_label_new (NULL);
									gtk_label_set_use_markup (GTK_LABEL (pDescriptionLabel), TRUE);
									pPreviewImage = gtk_image_new_from_pixbuf (NULL);
									_allocate_new_buffer;
									data[0] = pDescriptionLabel;
									data[1] = pPreviewImage;
									GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pOneWidget));
									gtk_tree_selection_set_select_function (selection, (GtkTreeSelectionFunc) _cairo_dock_select_one_item_in_tree, data, NULL);
								}
							}
							else  // pas de valeurs autorisees.
							{
								for (k = 0; k < iNbElements; k ++)
								{
									cValue =  (k < length ? cValueList[k] : NULL);
									if (cValue != NULL)
									{
										memset (&iter, 0, sizeof (GtkTreeIter));
										gtk_list_store_append (modele, &iter);
										gtk_list_store_set (modele, &iter,
											CAIRO_DOCK_MODEL_ACTIVE, TRUE,
											CAIRO_DOCK_MODEL_NAME, cValue,
											CAIRO_DOCK_MODEL_RESULT, cValue,
											CAIRO_DOCK_MODEL_ORDER, k, -1);
									}
								}
								pTable = gtk_table_new (2, 2, FALSE);
								gtk_box_pack_start (GTK_BOX (pHBox),
									pTable,
									FALSE,
									FALSE,
									0);
									
								_allocate_new_buffer;
								
								pButtonAdd = gtk_button_new_from_stock (GTK_STOCK_ADD);
								g_signal_connect (G_OBJECT (pButtonAdd),
									"clicked",
									G_CALLBACK (_cairo_dock_add),
									data);
								gtk_table_attach (GTK_TABLE (pTable),
									pButtonAdd,
									0,
									1,
									0,
									1,
									GTK_SHRINK,
									GTK_SHRINK,
									0,
									0);
								pButtonRemove = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
								g_signal_connect (G_OBJECT (pButtonRemove),
									"clicked",
									G_CALLBACK (_cairo_dock_remove),
									data);
								gtk_table_attach (GTK_TABLE (pTable),
									pButtonRemove,
									0,
									1,
									1,
									2,
									GTK_SHRINK,
									GTK_SHRINK,
									0,
									0);
								pEntry = gtk_entry_new ();
								gtk_table_attach (GTK_TABLE (pTable),
									pEntry,
									1,
									2,
									0,
									2,
									GTK_SHRINK,
									GTK_SHRINK,
									0,
									0);
								
								data[0] = pOneWidget;
								data[1] = pEntry;
							}
						}

						if (iElementType == 'S' || iElementType == 'D' || iElementType == 'u')
						{
							if (pEntry != NULL)
							{
								_allocate_new_buffer;
								data[0] = pEntry;
								data[1] = GINT_TO_POINTER (iElementType != 'u' ? (iElementType == 'S' ? 0 : 1) : 0);
								data[2] = GTK_WINDOW (pDialog);
								pButtonFileChooser = gtk_button_new_from_stock (GTK_STOCK_OPEN);
								g_signal_connect (G_OBJECT (pButtonFileChooser),
									"clicked",
									G_CALLBACK (_cairo_dock_pick_a_file),
									data);
								gtk_box_pack_start (GTK_BOX (pHBox),
									pButtonFileChooser,
									FALSE,
									FALSE,
									0);
								if (iElementType == 'u') //Sound Play Button
								{
									pButtonPlay = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY); //Outch
									g_signal_connect (G_OBJECT (pButtonPlay),
										"clicked",
										G_CALLBACK (_cairo_dock_play_a_sound),
										data);
									gtk_box_pack_start (GTK_BOX (pHBox),
										pButtonPlay,
										FALSE,
										FALSE,
										0);
								}
							}
						}
						else if (iElementType == 'R' || iElementType == 'M')
						{
							GtkWidget *pPreviewBox = gtk_vbox_new (FALSE, CAIRO_DOCK_GUI_MARGIN);
							gtk_box_pack_start (GTK_BOX (pHBox),
								pPreviewBox,
								FALSE,
								FALSE,
								0);
							if (pDescriptionLabel != NULL)
								gtk_box_pack_start (GTK_BOX (pPreviewBox),
									pDescriptionLabel,
									FALSE,
									FALSE,
									0);
							if (pPreviewImage != NULL)
								gtk_box_pack_start (GTK_BOX (pPreviewBox),
									pPreviewImage,
									FALSE,
									FALSE,
									0);
						}
						else if (iElementType == 'P' && pEntry != NULL)
						{
							pFontButton = gtk_font_button_new_with_font (gtk_entry_get_text (GTK_ENTRY (pEntry)));
							gtk_font_button_set_show_style (GTK_FONT_BUTTON (pFontButton), FALSE);
							gtk_font_button_set_show_size (GTK_FONT_BUTTON (pFontButton), FALSE);
							g_signal_connect (G_OBJECT (pFontButton),
								"font-set",
								G_CALLBACK (_cairo_dock_set_font),
								pEntry);
							gtk_box_pack_start (GTK_BOX (pHBox),
								pFontButton,
								FALSE,
								FALSE,
								0);
						}
						else if (iElementType == 'k' && pEntry != NULL)
						{
							GtkWidget *pGrabKeyButton = gtk_button_new_with_label(_("grab"));

							_allocate_new_buffer;
							data[0] = pOneWidget;
							data[1] = pDialog;
							gtk_widget_add_events(pDialog, GDK_KEY_PRESS_MASK);

							g_signal_connect (G_OBJECT (pGrabKeyButton),
								"clicked",
								G_CALLBACK (_cairo_dock_key_grab_clicked),
								data);

							gtk_box_pack_start (GTK_BOX (pHBox),
								pGrabKeyButton,
								FALSE,
								FALSE,
								0);
						}
						g_strfreev (cValueList);
					break;

					case 'F' :
					case 'X' :
						//g_print ("  + frame\n");
						if (pAuthorizedValuesList == NULL)
						{
							pFrame = NULL;
							pFrameVBox = NULL;
						}
						else
						{
							if (pAuthorizedValuesList[0] == NULL || *pAuthorizedValuesList[0] == '\0')
								cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, NULL);  // utile ?
							else
							{
								cValue = pAuthorizedValuesList[0];
								cSmallIcon = pAuthorizedValuesList[1];
							}
							gchar *cFrameTitle;

							
							cFrameTitle = g_strdup_printf ("<b>%s</b>", dgettext (cGettextDomain, cValue));
							pLabel= gtk_label_new (NULL);
							gtk_label_set_markup (GTK_LABEL (pLabel), cFrameTitle);
							
							pLabelContainer = NULL;
							if (cSmallIcon != NULL)
							{
								pLabelContainer = gtk_hbox_new (FALSE, CAIRO_DOCK_ICON_MARGIN/2);
								GtkWidget *pImage = gtk_image_new ();
								GdkPixbuf *pixbuf;
								if (*cSmallIcon != '/')
									pixbuf = gtk_widget_render_icon (pImage,
										cSmallIcon ,
										GTK_ICON_SIZE_MENU,
										NULL);
								else
									pixbuf = gdk_pixbuf_new_from_file_at_size (cSmallIcon, CAIRO_DOCK_FRAME_ICON_SIZE, CAIRO_DOCK_FRAME_ICON_SIZE, NULL);
								if (pixbuf != NULL)
								{
									gtk_image_set_from_pixbuf (GTK_IMAGE (pImage), pixbuf);
									gdk_pixbuf_unref (pixbuf);
									gtk_container_add (GTK_CONTAINER (pLabelContainer),
										pImage);
								}
								gtk_container_add (GTK_CONTAINER (pLabelContainer),
									pLabel);
							}
							
							GtkWidget *pExternFrame;
							if (iElementType == 'F')
							{
								pExternFrame = gtk_frame_new (NULL);
								gtk_container_set_border_width (GTK_CONTAINER (pExternFrame), CAIRO_DOCK_GUI_MARGIN);
								gtk_frame_set_shadow_type (GTK_FRAME (pExternFrame), GTK_SHADOW_OUT);
								gtk_frame_set_label_widget (GTK_FRAME (pExternFrame), (pLabelContainer != NULL ? pLabelContainer : pLabel));
								pFrame = pExternFrame;
							}
							else
							{
								pExternFrame = gtk_expander_new (NULL);
								gtk_expander_set_expanded (GTK_EXPANDER (pExternFrame), FALSE);
								gtk_expander_set_label_widget (GTK_EXPANDER (pExternFrame), (pLabelContainer != NULL ? pLabelContainer : pLabel));
								pFrame = gtk_frame_new (NULL);
								gtk_container_set_border_width (GTK_CONTAINER (pFrame), CAIRO_DOCK_GUI_MARGIN);
								gtk_frame_set_shadow_type (GTK_FRAME (pFrame), GTK_SHADOW_OUT);
								gtk_container_add (GTK_CONTAINER (pExternFrame),
									pFrame);
							}
							
							gtk_box_pack_start (GTK_BOX (pVBox),
								pExternFrame,
								FALSE,
								FALSE,
								0);

							pFrameVBox = gtk_vbox_new (FALSE, CAIRO_DOCK_GUI_MARGIN);
							gtk_container_add (GTK_CONTAINER (pFrame),
								pFrameVBox);
							g_free (cFrameTitle);
							if (pAuthorizedValuesList[0] == NULL || *pAuthorizedValuesList[0] == '\0')
								g_free (cValue);
						}
						break;

					case 'v' :  // separateur.
						{
							GtkWidget *pAlign = gtk_alignment_new (.5, 0., 0.5, 0.);
							pOneWidget = gtk_hseparator_new ();
							gtk_container_add (GTK_CONTAINER (pAlign), pOneWidget);
							gtk_box_pack_start(GTK_BOX (pFrameVBox != NULL ? pFrameVBox : pVBox),
								pAlign,
								FALSE,
								FALSE,
								0);
						}
					break ;

					/*case 'k' :
						cValue = g_key_file_get_string(pKeyFile, cGroupName, cKeyName, NULL);

						pOneWidget = gtk_entry_new();
						gtk_entry_set_text(GTK_ENTRY(pOneWidget), cValue);
						GtkWidget *w = gtk_button_new_with_label("grab");
						GtkWidget *b = gtk_hbox_new(FALSE, 0);
						gtk_box_pack_start(GTK_BOX(b), pOneWidget, FALSE, FALSE, 0);
						gtk_box_pack_start(GTK_BOX(b), w, FALSE, FALSE, 0);

						pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
						gtk_box_pack_start(GTK_BOX (pHBox),
							b,
							FALSE,
							FALSE,
							0);

						_allocate_new_buffer;
						data[0] = pOneWidget;
						data[1] = GTK_WINDOW (pDialog);
						gtk_widget_add_events(pDialog, GDK_KEY_PRESS_MASK);

						g_signal_connect (G_OBJECT (w),
							"clicked",
							G_CALLBACK (_cairo_dock_key_grab_clicked),
							data);*/

					break;

					default :
						cd_warning ("this conf file seems to be incorrect !");
					break ;
				}

				if (pSubWidgetList != NULL)
				{
					pGroupKeyWidget = g_new (gpointer, 3);
					pGroupKeyWidget[0] = g_strdup (cGroupName);  // car on ne pourra pas le liberer s'il est partage entre plusieurs 'data'.
					pGroupKeyWidget[1] = cKeyName;
					pGroupKeyWidget[2] = pSubWidgetList;
					*pWidgetList = g_slist_prepend (*pWidgetList, pGroupKeyWidget);
				}

				g_strfreev (pAuthorizedValuesList);
				g_free (cKeyComment);
			}

			j ++;
		}
		g_free (pKeyList);  // on libere juste la liste de chaines, pas les chaines a l'interieur.
		g_free (cGroupComment);
		
		i ++;
	}

	gtk_tooltips_enable (GTK_TOOLTIPS (pToolTipsGroup));
	gtk_widget_show_all (pDialog);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (pNoteBook), iPresentedNumPage);

	g_strfreev (pGroupList);
	
	/*gpointer *user_data = g_new (gpointer, 3);
	user_data[0] = *pWidgetList;
	user_data[1] = pDataGarbage;
	user_data[2] = pModelGarbage;
	g_print ("CONNECT\n");
	g_signal_connect (G_OBJECT (pDialog),
		"delete-event",
		G_CALLBACK (_cairo_dock_free_conf_widget_data),
		user_data);*/
		
	return pDialog;
}


gboolean cairo_dock_is_advanced_keyfile (GKeyFile *pKeyFile)
{
	gchar *cFirstComment =  g_key_file_get_comment (pKeyFile, NULL, NULL, NULL);
	if (cFirstComment == NULL || *cFirstComment != '!')
	{
		g_free (cFirstComment);
		return FALSE;
	}
	g_free (cFirstComment);
	return TRUE;
}


GtkWidget *cairo_dock_generate_basic_ihm_from_keyfile (gchar *cConfFilePath, const gchar *cTitle, GtkWindow *pParentWindow, GtkTextBuffer **pTextBuffer, gboolean bApplyButtonPresent, gboolean bSwitchButtonPresent, gchar *cButtonConvert, gchar *cGettextDomain)
	{
	gchar *cConfiguration;
	gboolean read_ok = g_file_get_contents (cConfFilePath, &cConfiguration, NULL, NULL);
	if (! read_ok)
	{
		cd_warning ("file %s does not exist or is not readble", cConfFilePath);
		return NULL;
	}

	GtkWidget *view = NULL;
	GtkTextBuffer *buffer = NULL;
	GtkWidget *pDialog;
	if (bApplyButtonPresent)
	{
		if (bSwitchButtonPresent)
		pDialog = gtk_dialog_new_with_buttons ((cTitle != NULL ? cTitle : ""),
								(pParentWindow != NULL ? GTK_WINDOW (pParentWindow) : NULL),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								(cButtonConvert != NULL ? cButtonConvert : GTK_STOCK_CONVERT),
								GTK_RESPONSE_HELP,
								GTK_STOCK_APPLY,
								GTK_RESPONSE_APPLY,
								GTK_STOCK_OK,
								GTK_RESPONSE_ACCEPT,
								GTK_STOCK_QUIT,
								GTK_RESPONSE_REJECT,
								NULL);
		else
		pDialog = gtk_dialog_new_with_buttons ((cTitle != NULL ? cTitle : ""),
								(pParentWindow != NULL ? GTK_WINDOW (pParentWindow) : NULL),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_STOCK_APPLY,
								GTK_RESPONSE_APPLY,
								GTK_STOCK_OK,
								GTK_RESPONSE_ACCEPT,
								GTK_STOCK_QUIT,
								GTK_RESPONSE_REJECT,
								NULL);
	}
	else
	{
		pDialog = gtk_dialog_new_with_buttons ((cTitle != NULL ? cTitle : ""),
								(pParentWindow != NULL ? GTK_WINDOW (pParentWindow) : NULL),
								GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_STOCK_OK,
								GTK_RESPONSE_ACCEPT,
								GTK_STOCK_QUIT,
								GTK_RESPONSE_REJECT,
								NULL);
	}
	gtk_window_resize (GTK_WINDOW (pDialog), 400, 300);

	view = gtk_text_view_new ();

	GtkWidget *scr = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scr), view);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), scr);

	PangoFontDescription *font_desc = pango_font_description_from_string ("Serif 10");
	gtk_widget_modify_font (view, font_desc);
	pango_font_description_free (font_desc);

	GdkColor color;
	gdk_color_parse ("blue", &color);
	gtk_widget_modify_text (view, GTK_STATE_NORMAL, &color);

	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 20);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (view), 10);

	gtk_text_buffer_set_text (buffer, cConfiguration, -1);
	g_free (cConfiguration);

	gtk_widget_show_all (pDialog);
	*pTextBuffer = buffer;
	return pDialog;
}



static gboolean _cairo_dock_get_active_elements (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, GSList **pStringList)
{
	//g_print ("%s (%d)\n", __func__, *pOrder);
	gboolean bActive;
	gchar *cValue = NULL;
	gtk_tree_model_get (model, iter, CAIRO_DOCK_MODEL_ACTIVE, &bActive, CAIRO_DOCK_MODEL_NAME, &cValue, -1);

	if (bActive)
	{
		*pStringList = g_slist_append (*pStringList, cValue);
	}
	else
	{
		g_free (cValue);
	}
	return FALSE;
}
static void _cairo_dock_get_each_widget_value (gpointer *data, GKeyFile *pKeyFile)
	{
	gchar *cGroupName = data[0];
	gchar *cKeyName = data[1];
	GSList *pSubWidgetList = data[2];
	GSList *pList;
	gsize i = 0, iNbElements = g_slist_length (pSubWidgetList);
	GtkWidget *pOneWidget = pSubWidgetList->data;

	if (GTK_IS_CHECK_BUTTON (pOneWidget))
	{
		gboolean *tBooleanValues = g_new0 (gboolean, iNbElements);
		for (pList = pSubWidgetList; pList != NULL; pList = pList->next)
		{
			pOneWidget = pList->data;
			tBooleanValues[i] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pOneWidget));
			i ++;
		}
		if (iNbElements > 1)
			g_key_file_set_boolean_list (pKeyFile, cGroupName, cKeyName, tBooleanValues, iNbElements);
		else
			g_key_file_set_boolean (pKeyFile, cGroupName, cKeyName, tBooleanValues[0]);
		g_free (tBooleanValues);
	}
	else if (GTK_IS_SPIN_BUTTON (pOneWidget) || GTK_IS_HSCALE (pOneWidget))
	{
		gboolean bIsSpin = GTK_IS_SPIN_BUTTON (pOneWidget);
		
		if ((bIsSpin && gtk_spin_button_get_digits (GTK_SPIN_BUTTON (pOneWidget)) == 0) || (! bIsSpin && gtk_scale_get_digits (GTK_SCALE (pOneWidget)) == 0))
		{
			int *tIntegerValues = g_new0 (int, iNbElements);
			for (pList = pSubWidgetList; pList != NULL; pList = pList->next)
			{
				pOneWidget = pList->data;
				tIntegerValues[i] = (bIsSpin ? gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (pOneWidget)) : gtk_range_get_value (GTK_RANGE (pOneWidget)));
				i ++;
			}
			if (iNbElements > 1)
				g_key_file_set_integer_list (pKeyFile, cGroupName, cKeyName, tIntegerValues, iNbElements);
			else
				g_key_file_set_integer (pKeyFile, cGroupName, cKeyName, tIntegerValues[0]);
			g_free (tIntegerValues);
		}
		else
		{
			double *tDoubleValues = g_new0 (double, iNbElements);
			for (pList = pSubWidgetList; pList != NULL; pList = pList->next)
			{
				pOneWidget = pList->data;
				tDoubleValues[i] = (bIsSpin ? gtk_spin_button_get_value (GTK_SPIN_BUTTON (pOneWidget)) : gtk_range_get_value (GTK_RANGE (pOneWidget)));
				i ++;
			}
			if (iNbElements > 1)
				g_key_file_set_double_list (pKeyFile, cGroupName, cKeyName, tDoubleValues, iNbElements);
			else
				g_key_file_set_double (pKeyFile, cGroupName, cKeyName, tDoubleValues[0]);
			g_free (tDoubleValues);
		}
	}
	else if (GTK_IS_COMBO_BOX (pOneWidget))
	{
		GtkTreeIter iter;
		gchar *cValue =  NULL;
		if (GTK_IS_COMBO_BOX_ENTRY (pOneWidget))
		{
			cValue = gtk_combo_box_get_active_text (GTK_COMBO_BOX (pOneWidget));
		}
		else if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (pOneWidget), &iter))
		{
			GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (pOneWidget));
			if (model != NULL)
				gtk_tree_model_get (model, &iter, CAIRO_DOCK_MODEL_RESULT, &cValue, -1);
		}
		g_key_file_set_string (pKeyFile, cGroupName, cKeyName, (cValue != NULL ? cValue : ""));
		g_free (cValue);
	}
	else if (GTK_IS_ENTRY (pOneWidget))
	{
		const gchar *cValue = gtk_entry_get_text (GTK_ENTRY (pOneWidget));
		const gchar* const * cPossibleLocales = g_get_language_names ();
		gchar *cKeyNameFull, *cTranslatedValue;
		while (cPossibleLocales[i] != NULL)  // g_key_file_set_locale_string ne marche pas avec une locale NULL comme le fait 'g_key_file_get_locale_string', il faut donc le faire a la main !
		{
			cKeyNameFull = g_strdup_printf ("%s[%s]", cKeyName, cPossibleLocales[i]);
			cTranslatedValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyNameFull, NULL);
			g_free (cKeyNameFull);
			if (cTranslatedValue != NULL && strcmp (cTranslatedValue, "") != 0)
				{
				g_free (cTranslatedValue);
				break;
				}
			g_free (cTranslatedValue);
			i ++;
		}
		if (cPossibleLocales[i] != NULL)
			g_key_file_set_locale_string (pKeyFile, cGroupName, cKeyName, cPossibleLocales[i], cValue);
		else
			g_key_file_set_string (pKeyFile, cGroupName, cKeyName, cValue);
	}
	else if (GTK_IS_TREE_VIEW (pOneWidget))
	{
		GtkTreeModel *pModel = gtk_tree_view_get_model (GTK_TREE_VIEW (pOneWidget));
		GSList *pActiveElementList = NULL;
		gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _cairo_dock_get_active_elements, &pActiveElementList);

		iNbElements = g_slist_length (pActiveElementList);
		gchar **tStringValues = g_new0 (gchar *, iNbElements + 1);

		i = 0;
		GSList * pListElement;
		for (pListElement = pActiveElementList; pListElement != NULL; pListElement = pListElement->next)
		{
			tStringValues[i] = pListElement->data;
			i ++;
		}
		if (iNbElements > 1)
			g_key_file_set_string_list (pKeyFile, cGroupName, cKeyName, (const gchar * const *)tStringValues, iNbElements);
		else
			g_key_file_set_string (pKeyFile, cGroupName, cKeyName, (tStringValues[0] != NULL ? tStringValues[0] : ""));
		g_slist_free (pActiveElementList);  // ses donnees sont dans 'tStringValues' et seront donc liberees avec.
		g_strfreev (tStringValues);
	}
}
void cairo_dock_update_keyfile_from_widget_list (GKeyFile *pKeyFile, GSList *pWidgetList)
{
	g_slist_foreach (pWidgetList, (GFunc) _cairo_dock_get_each_widget_value, pKeyFile);
}



static void _cairo_dock_free_widget_list (gpointer *data, gpointer user_data)
{
	//g_print ("%s - %s\n", (gchar *)data[0], (gchar *)data[1]);
	g_free (data[0]);
	g_free (data[1]);
	g_slist_free (data[2]);  // les elements de data[2] sont les widgets, et se feront liberer lors de la destruction de la fenetre.
}

void cairo_dock_free_generated_widget_list (GSList *pWidgetList)
{
	g_slist_foreach (pWidgetList, (GFunc) _cairo_dock_free_widget_list, NULL);
	g_slist_free (pWidgetList);
}
