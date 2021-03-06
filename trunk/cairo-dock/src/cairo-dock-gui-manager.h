
#ifndef __CAIRO_DOCK_GUI_MANAGER__
#define  __CAIRO_DOCK_GUI_MANAGER__

#include <gtk/gtk.h>
#include <cairo-dock-struct.h>
G_BEGIN_DECLS

/** @file cairo-dock-gui-manager.h This file manages the GUIs of Cairo-Dock.
* 
* GUIs are built from a .conf file; .conf files are normal group/key files, but with some special indications in the comments. Each key will be represented by a pre-defined widget, that is defined by the first caracter of its comment. The comment also contains a description of the key, and an optionnal tooltip. See cairo-dock-gui-factory.h for the list of pre-defined widgets and a short explanation on how to use them inside a conf file. The file 'cairo-dock.conf' can be an useful example.
* 
* GUIs can be stand-alone (like the Theme-Manager), or embedded inside the main GUI in the case of modules.
* 
*/

struct _CairoDockCategoryWidgetTable {
	GtkWidget *pFrame;
	GtkWidget *pTable;
	gint iNbRows;
	gint iNbItemsInCurrentRow;
	} ;

struct _CairoDockGroupDescription {
	gchar *cGroupName;
	gint iCategory;
	gchar *cDescription;
	gchar *cPreviewFilePath;
	GtkWidget *pActivateButton;
	GtkWidget *pLabel;
	gchar *cOriginalConfFilePath;
	gchar *cIcon;
	gchar *cConfFilePath;
	const gchar *cGettextDomain;
	void (* load_custom_widget) (CairoDockModuleInstance *pInstance, GKeyFile *pKeyFile);
	const gchar **cDependencies;
	gboolean bIgnoreDependencies;
	GList *pExternalModules;
	const gchar *cInternalModule;
	gboolean bMatchFilter;
	} ;

typedef struct _CairoDockGroupDescription CairoDockGroupDescription;
typedef struct _CairoDockCategoryWidgetTable CairoDockCategoryWidgetTable;

/// Definition of the callback called when the user apply the config panelS.
typedef gboolean (* CairoDockApplyConfigFunc) (gpointer data);


int cairo_dock_get_nb_config_panels (void);
void cairo_dock_config_panel_destroyed (void);
void cairo_dock_config_panel_created (void);

GtkWidget *cairo_dock_build_main_ihm (const gchar *cConfFilePath, gboolean bMaintenanceMode);


GtkWidget *cairo_dock_get_preview_image (int *iPreviewWidth);
GtkWidget *cairo_dock_get_main_window (void);
CairoDockGroupDescription *cairo_dock_get_current_group (void);
GSList *cairo_dock_get_current_widget_list (void);
gpointer cairo_dock_get_current_widget (void);


void cairo_dock_hide_all_categories (void);
void cairo_dock_show_all_categories (void);
void cairo_dock_show_one_category (int iCategory);
void cairo_dock_insert_extern_widget_in_gui (GtkWidget *pWidget);
GtkWidget *cairo_dock_present_group_widget (const gchar *cConfFilePath, CairoDockGroupDescription *pGroupDescription, gboolean bSingleGroup, CairoDockModuleInstance *pInstance);
CairoDockGroupDescription *cairo_dock_find_module_description (const gchar *cModuleName);
void cairo_dock_present_module_gui (CairoDockModule *pModule);
void cairo_dock_present_module_instance_gui (CairoDockModuleInstance *pModuleInstance);
void cairo_dock_show_group (CairoDockGroupDescription *pGroupDescription);

void cairo_dock_free_categories (void);

void cairo_dock_write_current_group_conf_file (gchar *cConfFilePath, CairoDockModuleInstance *pInstance);
void cairo_dock_write_extra_group_conf_file (gchar *cConfFilePath, CairoDockModuleInstance *pInstance, int iNumExtraModule);


gboolean cairo_dock_build_normal_gui (gchar *cConfFilePath, const gchar *cGettextDomain, const gchar *cTitle, int iWidth, int iHeight, CairoDockApplyConfigFunc pAction, gpointer pUserData, GFreeFunc pFreeUserData, GtkWidget **pWindow);


gpointer cairo_dock_get_previous_widget (void);


/** Completely reload the current module's GUI by reading again its conf file and re-building it from scratch (so if the module has a custom widget builder, it will be called too).
*@param pInstance the instance of the external module (myApplet) or NULL for an internal module.
* @param iShowPage number of the page of the notebook to show, or -1 to stay on the current page.
*/
void cairo_dock_reload_current_group_widget_full (CairoDockModuleInstance *pInstance, int iShowPage);

/** Completely reload the current module's GUI by reading again its conf file and re-building it from scratch (so if the module has a custom widget builder, it will be called too).
*@param pInstance the instance of the external module (myApplet) or NULL for an internal module.
*/
#define cairo_dock_reload_current_group_widget(pInstance) cairo_dock_reload_current_group_widget_full (pInstance, -1)

/** Retrieve the widget associated with a given (group;key) pair of the conf file it was built from.
*@param cGroupName name of the group in the conf file.
*@param cKeyName name of the key in the conf file.
*/
GtkWidget *cairo_dock_get_widget_from_name (const gchar *cGroupName, const gchar *cKeyName);


void cairo_dock_apply_current_filter (gchar **pKeyWords, gboolean bAllWords, gboolean bSearchInToolTip, gboolean bHighLightText, gboolean bHideOther);
/** Trigger the filter, according to the current text and filter's options.
*/
void cairo_dock_trigger_current_filter (void);


void cairo_dock_deactivate_module_in_gui (const gchar *cModuleName);
void cairo_dock_update_desklet_size_in_gui (const gchar *cModuleName, int iWidth, int iHeight);
void cairo_dock_update_desklet_position_in_gui (const gchar *cModuleName, int x, int y);

void cairo_dock_set_status_message (GtkWidget *pWindow, const gchar *cMessage);
void cairo_dock_set_status_message_printf (GtkWidget *pWindow, const gchar *cFormat, ...);


G_END_DECLS
#endif
