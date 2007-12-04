
#ifndef __CAIRO_DOCK_APPLET_FACTORY__
#define  __CAIRO_DOCK_APPLET_FACTORY__

#include <glib.h>

#include "cairo-dock-struct.h"


cairo_surface_t *cairo_dock_create_applet_surface (gchar *cImageFilePath, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);


Icon *cairo_dock_create_icon_for_applet (CairoDock *pDock, int iWidth, int iHeight, gchar *cName, gchar *cIconName);


GKeyFile *cairo_dock_read_header_applet_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName, gboolean *bFlushConfFileNeeded);


GHashTable *cairo_dock_list_themes (gchar *cThemesDir, GHashTable *hProvidedTable, GError **erreur);


gchar *cairo_dock_check_conf_file_exists (gchar *cUserDataDirName, gchar *cShareDataDir, gchar *cConfFileName);


void cairo_dock_set_icon_surface (cairo_t *pIconContext, cairo_surface_t *pSurface);
void cairo_dock_set_icon_surface_with_reflect (cairo_t *pIconContext, cairo_surface_t *pSurface, Icon *pIcon, CairoDock *pDock);
void cairo_dock_set_icon_name (cairo_t *pIconContext, const gchar *cIconName, Icon *pIcon, CairoDock *pDock);
void cairo_dock_set_quick_info (cairo_t *pIconContext, const gchar *cExtraInfo, Icon *pIcon);
#define cairo_dock_remove_quick_info(pIcon) cairo_dock_set_quick_info (NULL, NULL, pIcon)

void cairo_dock_animate_icon (Icon *pIcon, CairoDock *pDock, CairoDockAnimationType iAnimationType, int iNbRounds);

void cairo_dock_add_reflection_to_icon (Icon *pIcon, CairoDock *pDock, cairo_t *pCairoContext);


#define CD_CONFIG_APPLET \
void read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar *cName)

#define CD_CONFIG_BEGIN \
	GError *erreur = NULL; \
	gboolean bFlushConfFileNeeded = FALSE; \
	GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, &cName, &bFlushConfFileNeeded); \
	g_return_if_fail (pKeyFile != NULL);
#define CD_CONFIG_END \
	if (bFlushConfFileNeeded) \
		cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath); \
	g_key_file_free (pKeyFile);

#define CD_CONFIG_GET_BOOLEAN(cGroupName, cKeyName) cairo_dock_get_boolean_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, TRUE)
#define CD_CONFIG_GET_INTEGER(cGroupName, cKeyName) cairo_dock_get_integer_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, 0)
#define CD_CONFIG_GET_DOUBLE(cGroupName, cKeyName) cairo_dock_get_double_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, 0.)
#define CD_CONFIG_GET_STRING(cGroupName, cKeyName) cairo_dock_get_string_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, NULL)
#define CD_CONFIG_GET_ANIMATION(cGroupName, cKeyName) cairo_dock_get_animation_type_key_value (pKeyFile, cGroupName, cKeyName, &bFlushConfFileNeeded, NULL);


#define CD_APPLET_DEFINITION(cName, iMajorVersion, iMinorVersion, iMicroVersion) \
CairoDockVisitCard *pre_init (void)\
{\
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);\
	pVisitCard->cModuleName = g_strdup (cName);\
	pVisitCard->cReadmeFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_README_FILE);\
	pVisitCard->iMajorVersionNeeded = iMajorVersion;\
	pVisitCard->iMinorVersionNeeded = iMinorVersion;\
	pVisitCard->iMicroVersionNeeded = iMicroVersion;\
	return pVisitCard;\
}

#define CD_INIT_APPLET Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
#define CD_STOP_APPLET void stop (void)
#define CD_PRE_INIT_APPLET gchar *pre_init (void)

#define CD_APPLET_INIT_BEGIN \
	myDock = pDock; \
	*cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, APPLET_CONF_FILE); \
	int iOriginalWidth = 48, iOriginalHeight = 48; \
	gchar *cAppletName = NULL; \
	read_conf_file (*cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cAppletName); \
	myIcon = cairo_dock_create_icon_for_applet (pDock, iOriginalWidth, iOriginalHeight, conf_defaultTitle, NULL); \
	myDrawContext = cairo_create (myIcon->pIconBuffer);

#define CD_APPLET_INIT_END \
	g_free (cAppletName); \
	return myIcon;


#define CD_CLICK_ON_APPLET \
gboolean action_on_click (gpointer *data)

#define CD_CLICK_ON_APPLET_BEGIN \
	if (data[0] == myIcon) \
	{
#define CD_CLICK_ON_APPLET_END \
	} \
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;


#endif

