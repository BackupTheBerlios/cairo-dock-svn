
#ifndef __CAIRO_DOCK_APPLET_MULTI_INSTANCE__
#define  __CAIRO_DOCK_APPLET_MULTI_INSTANCE__


#define CD_APPLET_PRE_INIT_BEGIN(cName, iMajorVersion, iMinorVersion, iMicroVersion, iAppletCategory) \
	CD_APPLET_PRE_INIT_ALL_BEGIN (cName, iMajorVersion, iMinorVersion, iMicroVersion, iAppletCategory) \
	pVisitCard->bMultiInstance = TRUE;


#define CD_APPLET_INIT_BEGIN CD_APPLET_INIT_ALL_BEGIN

#define myIcon myApplet->pIcon
#define myContainer myApplet->pContainer
#define myDock myApplet->pDock
#define myDesklet myApplet->pDesklet
#define myDrawContext myApplet->pDrawContext
#define myConfig (* ((AppletConfig *)(((gpointer)myApplet)+sizeof(CairoDockModuleInstance))))
#define myData (* ((AppletData *)(((gpointer)myApplet)+sizeof(CairoDockModuleInstance)+sizeof(AppletConfig))))

#define CD_APPLET_RELOAD_BEGIN CD_APPLET_RELOAD_ALL_BEGIN

#define CD_APPLET_RESET_DATA_END CD_APPLET_RESET_DATA_ALL_END

#define CD_APPLET_RESET_CONFIG_BEGIN CD_APPLET_RESET_CONFIG_ALL_BEGIN

#define CD_APPLET_GET_CONFIG_BEGIN CD_APPLET_GET_CONFIG_ALL_BEGIN


#endif
