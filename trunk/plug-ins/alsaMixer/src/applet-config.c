
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-mixer.h"
#include "applet-notifications.h"
#include "applet-config.h"



CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.card_id = CD_CONFIG_GET_STRING ("Configuration", "card id");
	if (myConfig.card_id == NULL)
		myConfig.card_id = g_strdup ("default");
	
	gchar *cMixerElementName = CD_CONFIG_GET_STRING ("Configuration", "mixer element");
	gchar *cMixerElementName2 = CD_CONFIG_GET_STRING ("Configuration", "mixer element 2");
	if (cMixerElementName2 != NULL && strcmp (cMixerElementName, cMixerElementName2) == 0)
	{
		myConfig.cMixerElementName = g_strconcat (cMixerElementName, ",0", NULL);
		myConfig.cMixerElementName2 = g_strconcat (cMixerElementName, ",1", NULL);
		g_free (cMixerElementName);
	}
	else
	{
		myConfig.cMixerElementName = cMixerElementName;
	}
	g_free (cMixerElementName2);
	
	myConfig.cShowAdvancedMixerCommand = CD_CONFIG_GET_STRING ("Configuration", "show mixer");
	
	myConfig.cShortcut = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	
	myConfig.iScrollVariation = CD_CONFIG_GET_INTEGER ("Configuration", "scroll variation");
	
	myConfig.bHideScaleOnLeave = CD_CONFIG_GET_BOOLEAN ("Configuration", "hide on leave");
	
	
	myConfig.iVolumeDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "display volume");
	
	myConfig.iVolumeEffect = CD_CONFIG_GET_INTEGER ("Configuration", "effect");
	
	myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cBrokenIcon = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
	myConfig.cMuteIcon = CD_CONFIG_GET_STRING ("Configuration", "mute icon");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.card_id);
	g_free (myConfig.cMixerElementName);
	g_free (myConfig.cMixerElementName2);
	g_free (myConfig.cShowAdvancedMixerCommand);
	if (myConfig.cShortcut)
		cd_keybinder_unbind(myConfig.cShortcut, (CDBindkeyHandler) mixer_on_keybinding_pull);
	g_free (myConfig.cShortcut);
	g_free (myConfig.cDefaultIcon);
	g_free (myConfig.cBrokenIcon);
	g_free (myConfig.cMuteIcon);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myData.pScale != NULL)
	{
		gtk_widget_destroy (myData.pScale);
		myData.pScale = NULL;
	}
	mixer_stop ();
	cairo_surface_destroy (myData.pSurface);
	cairo_surface_destroy (myData.pMuteSurface);
	cairo_dock_dialog_unreference (myData.pDialog);
	g_free (myData.cErrorMessage);
	g_free (myData.mixer_card_name);
	g_free (myData.mixer_device_name);
CD_APPLET_RESET_DATA_END


/*void mixer_write_elements_list (gchar *cConfFilePath, GKeyFile *pKeyFile)
{
	gchar *cElements = mixer_get_elements_list ();
	
	cairo_dock_update_conf_file_with_list (pKeyFile, cConfFilePath, cElements, "Configuration", "mixer element", NULL);
	gchar *cElements2 = g_strconcat (";", cElements, NULL);
	cairo_dock_update_conf_file_with_list (pKeyFile, cConfFilePath, cElements2, "Configuration", "mixer element 2", NULL);
	
	g_free (cElements);
	g_free (cElements2);
}*/

void cd_mixer_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile)
{
	g_print ("%s (%s)\n", __func__, myIcon->acName);
	//\____________ On construit la liste des canaux a controler.
	GList *pList = mixer_get_elements_list ();
	
	//\____________ On recupere la combo.
	GtkWidget *pCombo = cairo_dock_get_widget_from_name ("Configuration", "mixer element");
	g_return_if_fail (pCombo != NULL);
	cairo_dock_fill_combo_with_list (pCombo, pList, myConfig.cMixerElementName);
	
	//\____________ Idem pour la 2eme, avec une entree vide au debut.
	pCombo = cairo_dock_get_widget_from_name ("Configuration", "mixer element 2");
	g_return_if_fail (pCombo != NULL);
	pList = g_list_prepend (pList, "");
	cairo_dock_fill_combo_with_list (pCombo, pList, myConfig.cMixerElementName2);
	
	g_list_free (pList);  // les elements appartiennent au mixer_handle.
}
