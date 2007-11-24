
#ifndef __CAIRO_DOCK_DIALOGS__
#define  __CAIRO_DOCK_DIALOGS__

#include <glib.h>

#include "cairo-dock-struct.h"


gboolean cairo_dock_dialog_reference (Icon *pIcon);
void cairo_dock_dialog_unreference (Icon *pIcon);

CairoDockDialog *cairo_dock_isolate_dialog (Icon *pIcon);
void cairo_dock_free_dialog (CairoDockDialog *pDialog);
void cairo_dock_remove_dialog_if_any (Icon *icon);


GtkWidget *cairo_dock_build_interactive_widget_for_dialog (gchar *cInitialAnswer);

CairoDockDialog *cairo_dock_build_dialog (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cImageFilePath, GtkWidget *pInteractiveWidget, GtkButtonsType iButtonsType, CairoDockActionOnAnswerFunc pActionFunc, gpointer data);


void cairo_dock_dialog_calculate_aimed_point (Icon *pIcon, CairoDock *pDock, int *iX, int *iY, gboolean *bRight, gboolean *bIsPerpendicular, gboolean *bDirectionUp);

void cairo_dock_dialog_find_optimal_placement  (CairoDockDialog *pDialog, Icon *pIcon, CairoDock *pDock);

void cairo_dock_place_dialog (CairoDockDialog *pDialog, Icon *pIcon, CairoDock *pDock);

void cairo_dock_replace_all_dialogs (void);


CairoDockDialog *cairo_dock_show_dialog_full (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength, gchar *cIconPath, GtkButtonsType iButtonsType, gchar *cTextForEntry, CairoDockActionOnAnswerFunc pActionFunc, gpointer data);

#define cairo_dock_show_temporary_dialog_with_icon(cText, pIcon, pDock, fTimeLength, cIconPath) cairo_dock_show_dialog_full (cText, pIcon, pDock, fTimeLength, cIconPath, GTK_BUTTONS_NONE, NULL, NULL, NULL)
#define cairo_dock_show_temporary_dialog(cText, pIcon, pDock, fTimeLength) cairo_dock_show_temporary_dialog_with_icon (cText, pIcon, pDock, fTimeLength, NULL)
void cairo_dock_show_temporary_dialog_with_default_icon (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength);

#define cairo_dock_show_dialog_with_question(cText, pIcon, pDock, cIconPath, pActionFunc, data) cairo_dock_show_dialog_full (cText, pIcon, pDock, 0, cIconPath, GTK_BUTTONS_YES_NO, NULL, pActionFunc, data)
#define cairo_dock_show_dialog_with_entry(cText, pIcon, pDock, cIconPath, cTextForEntry, pActionFunc, data) cairo_dock_show_dialog_full (cText, pIcon, pDock, 0, cIconPath, GTK_BUTTONS_OK_CANCEL, cTextForEntry, pActionFunc, data)

gchar *cairo_dock_show_dialog_and_wait (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength, gchar *cIconPath, GtkButtonsType iButtonsType, const gchar *cTextForEntry);
gchar *cairo_dock_show_demand_and_wait (const gchar *cMessage, Icon *pIcon, CairoDock *pDock, const gchar *cInitialAnswer);
int cairo_dock_ask_question_and_wait (const gchar *cQuestion, Icon *pIcon, CairoDock *pDock);
int cairo_dock_ask_general_question_and_wait (const gchar *cQuestion);


#endif
