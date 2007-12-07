
#ifndef __CAIRO_DOCK_DIALOGS__
#define  __CAIRO_DOCK_DIALOGS__

#include <glib.h>

#include "cairo-dock-struct.h"


void cairo_dock_load_dialog_buttons (CairoDock *pDock, gchar *cButtonOkImage, gchar *cButtonCancelImage);

gboolean cairo_dock_dialog_reference (Icon *pIcon);
void cairo_dock_dialog_unreference (Icon *pIcon);

CairoDockDialog *cairo_dock_isolate_dialog (Icon *pIcon);
void cairo_dock_free_dialog (CairoDockDialog *pDialog);
void cairo_dock_remove_dialog_if_any (Icon *icon);


GtkWidget *cairo_dock_build_interactive_widget_for_dialog (const gchar *cInitialAnswer, double fValueForHScale);

CairoDockDialog *cairo_dock_build_dialog (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cImageFilePath, GtkWidget *pInteractiveWidget, GtkButtonsType iButtonsType, CairoDockActionOnAnswerFunc pActionFunc, gpointer data);


void cairo_dock_dialog_calculate_aimed_point (Icon *pIcon, CairoDock *pDock, int *iX, int *iY, gboolean *bRight, gboolean *bIsPerpendicular, gboolean *bDirectionUp);

void cairo_dock_dialog_find_optimal_placement  (CairoDockDialog *pDialog, Icon *pIcon, CairoDock *pDock);

void cairo_dock_place_dialog (CairoDockDialog *pDialog, Icon *pIcon, CairoDock *pDock);

void cairo_dock_replace_all_dialogs (void);


CairoDockDialog *cairo_dock_show_dialog_full (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength, gchar *cIconPath, GtkButtonsType iButtonsType, GtkWidget *pInteractiveWidget, CairoDockActionOnAnswerFunc pActionFunc, gpointer data, GFreeFunc pFreeDataFunc);

void cairo_dock_show_temporary_dialog_with_icon (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength, gchar *cIconPath);
void cairo_dock_show_temporary_dialog (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength);
void cairo_dock_show_temporary_dialog_with_default_icon (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength);

CairoDockDialog *cairo_dock_show_dialog_with_question (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cIconPath, CairoDockActionOnAnswerFunc pActionFunc, gpointer data, GFreeFunc pFreeDataFunc);
CairoDockDialog *cairo_dock_show_dialog_with_entry (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cIconPath, const gchar  *cTextForEntry, CairoDockActionOnAnswerFunc pActionFunc, gpointer data, GFreeFunc pFreeDataFunc);
CairoDockDialog *cairo_dock_show_dialog_with_value (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cIconPath, double fValue, CairoDockActionOnAnswerFunc pActionFunc, gpointer data, GFreeFunc pFreeDataFunc);

gchar *cairo_dock_show_dialog_and_wait (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength, gchar *cIconPath, GtkButtonsType iButtonsType, GtkWidget *pInteractiveWidget);
gchar *cairo_dock_show_demand_and_wait (const gchar *cMessage, Icon *pIcon, CairoDock *pDock, const gchar *cInitialAnswer);
double cairo_dock_show_value_and_wait (const gchar *cMessage, Icon *pIcon, CairoDock *pDock, double fInitialValue);
int cairo_dock_ask_question_and_wait (const gchar *cQuestion, Icon *pIcon, CairoDock *pDock);
int cairo_dock_ask_general_question_and_wait (const gchar *cQuestion);


#endif
