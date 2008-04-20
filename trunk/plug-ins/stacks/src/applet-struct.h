
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

//\___________ structure containing the applet's configuration parameters.
typedef struct {
  gchar *cMonitoredDirectory;
  gboolean bHiddenFiles;
} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
	int nothingHere;
} AppletData;


#endif