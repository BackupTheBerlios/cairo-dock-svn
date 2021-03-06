
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define WEATHER_NB_DAYS_MAX 5

#define WEATHER_RATIO_ICON_DESKLET .5

#define WEATHER_DEFAULT_NAME "weather"

#define _display(cValue) (cValue == NULL || *cValue == 'N' ? "?" : cValue)

/**typedef enum {
  MY_DESKLET_CAROUSSEL = 0,
  MY_DESKLET_MAIN_ICON
  } MyDeskletRender ;*/

struct _AppletConfig {
	gchar *cLocationCode;
	gboolean bISUnits;
	gboolean bCurrentConditions;
	gboolean bDisplayNights;
	gboolean bDisplayTemperature;
	gint iNbDays;
	gchar *cRenderer;
	gint cDialogDuration;
	gint iCheckInterval;
	gchar *cThemePath;
	gboolean bDesklet3D;
	gboolean bSetName;
	///MyDeskletRender iDeskletRenderer;
	} ;

typedef struct {
	gchar *cTemp;
	gchar *cDistance;
	gchar *cSpeed;
	gchar *cPressure;
	} Unit;

typedef struct {
	gchar *cSunRise;
	gchar *cSunSet;
	gchar *cDataAcquisitionDate;
	gchar *cObservatory;
	gchar *cTemp;
	gchar *cFeeledTemp;
	gchar *cWeatherDescription;
	gchar *cIconNumber;
	gchar *cWindSpeed;
	gchar *cWindDirection;
	gchar *cPressure;
	gchar *cHumidity;
	gchar *cMoonIconNumber;
	} CurrentContitions;

typedef struct {
	gchar *cIconNumber;
	gchar *cWeatherDescription;
	gchar *cWindSpeed;
	gchar *cWindDirection;
	gchar *cHumidity;
	gchar *cPrecipitationProba;
	} DayPart;

typedef struct {
	gchar *cName;
	gchar *cDate;
	gchar *cTempMax;
	gchar *cTempMin;
	gchar *cSunRise;
	gchar *cSunSet;
	DayPart part[2];
	} Day;

struct _AppletData {
	// memoire partagee.
	Unit units;
	gchar *cLocation;
	gchar *cLon;
	gchar *cLat;
	CurrentContitions currentConditions;
	Day days[WEATHER_NB_DAYS_MAX];
	gboolean bErrorInThread;
	// fin memoire partagee.
	CairoDockTask *pTask;
	gboolean bErrorRetrievingData;
	GList *pLocationsList;
	} ;


#endif
