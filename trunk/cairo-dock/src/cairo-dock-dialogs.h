
#ifndef __CAIRO_DOCK_DIALOGS__
#define  __CAIRO_DOCK_DIALOGS__

#include <glib.h>

#include "cairo-dock-struct.h"


void cairo_dock_load_dialog_buttons (CairoDock *pDock, gchar *cButtonOkImage, gchar *cButtonCancelImage);

/**
*Incremente le compteur de reference de 1 de maniere atomique, de maniere a empecher toute destruction du dialogue pendant son utilisation. Utiliser #cairo_dock_dialog_reference apres en avoir termine.
*@param pIcon l'icone contenant le dialogue.
@Returns TRUE ssi la reference sur le dialogue n'etait pas deja nulle et a pu etre incrementee, FALSE sinon.
*/
gboolean cairo_dock_dialog_reference (Icon *pIcon);
/**
*Decremente le compteur de reference de 1 de maniere atomique, et detruit le dialogue si la refernce est tombee a 0.
*@param pIcon l'icone contenant le dialogue.
*/
void cairo_dock_dialog_unreference (Icon *pIcon);

CairoDockDialog *cairo_dock_isolate_dialog (Icon *pIcon);
void cairo_dock_free_dialog (CairoDockDialog *pDialog);
/**
*Tente de detruire le dialogue d'une icone; si le dialogue est utilise par quelqu'un d'autre, il sera detruit plus tard, mais le dialogue sera au moins isole, et ne pourra donc plus etre utilise par une nouvelle personne.
*@param icon l'icone contenant le dialogue.
*/
void cairo_dock_remove_dialog_if_any (Icon *icon);


GtkWidget *cairo_dock_build_interactive_widget_for_dialog (const gchar *cInitialAnswer, double fValueForHScale);

CairoDockDialog *cairo_dock_build_dialog (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cImageFilePath, GtkWidget *pInteractiveWidget, GtkButtonsType iButtonsType, CairoDockActionOnAnswerFunc pActionFunc, gpointer data);


void cairo_dock_dialog_calculate_aimed_point (Icon *pIcon, CairoDock *pDock, int *iX, int *iY, gboolean *bRight, gboolean *bIsPerpendicular, gboolean *bDirectionUp);

void cairo_dock_dialog_find_optimal_placement  (CairoDockDialog *pDialog, Icon *pIcon, CairoDock *pDock);

void cairo_dock_place_dialog (CairoDockDialog *pDialog, Icon *pIcon, CairoDock *pDock);

void cairo_dock_replace_all_dialogs (void);


/**
*Fait apparaitre un dialogue avec un widget et 2 boutons.
*@param cText le message du dialogue.
*@param pIcon l'icone sur laquelle pointe le dialogue.
*@param pDock le dock contenant l'icone.
*@param cIconPath le chemin vers une icone a afficher dans la marge.
*@param iButtonsType type des boutons (GTK_BUTTONS_OK_CANCEL ou GTK_BUTTONS_YES_NO).
*@param pInteractiveWidget un widget d'interaction avec l'utilisateur.
*@param pActionFunc la fonction d'action appelee lorsque l'utilisateur valide son choix.
*@param data 
*@param pFreeDataFunc 
@Returns Si le widget est une entree de texte, retourne le texte si oui, et "" si non. Si le widget est une echelle, retourne la valeur sous forme de chaine si "oui", et "-1" si non, et sinon, retourne "yes" si oui, et "no" si non. Si besoin est, le widget est accessible via le dialogue, lui-meme accessible via l'icone.
*/
CairoDockDialog *cairo_dock_show_dialog_full (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength, gchar *cIconPath, GtkButtonsType iButtonsType, GtkWidget *pInteractiveWidget, CairoDockActionOnAnswerFunc pActionFunc, gpointer data, GFreeFunc pFreeDataFunc);

/**
*Fait apparaitre un dialogue a duree de vie limitee avec une icone dans la marge a cote du texte.
*@param cText le message du dialogue.
*@param pIcon l'icone sur laquelle pointe le dialogue.
*@param pDock le dock contenant l'icone.
*@param fTimeLength la duree de vie du dialogue (ou 0 pour une duree de vie illimitee).
*@param cIconPath le chemin vers une icone.
*/
void cairo_dock_show_temporary_dialog_with_icon (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength, gchar *cIconPath);
/**
*Fait apparaitre un dialogue a duree de vie limitee sans icone.
*@param cText le message du dialogue.
*@param pIcon l'icone sur laquelle pointe le dialogue.
*@param pDock le dock contenant l'icone.
*@param fTimeLength la duree de vie du dialogue (ou 0 pour une duree de vie illimitee).
*/
void cairo_dock_show_temporary_dialog (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength);
/**
*Fait apparaitre un dialogue a duree de vie limitee et avec l'icone par defaut.
*@param cText le message du dialogue.
*@param pIcon l'icone sur laquelle pointe le dialogue.
*@param pDock le dock contenant l'icone.
*@param fTimeLength la duree de vie du dialogue (ou 0 pour une duree de vie illimitee).
*/
void cairo_dock_show_temporary_dialog_with_default_icon (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength);

/**
*Fait apparaitre un dialogue a duree de vie illimitee avec une question et 2 boutons oui/non. Lorsque l'utilisateur clique sur "oui", la fonction d'action est appelee avec "yes", et avec "no" s'il a clique sur "non".
*@param cText le message du dialogue.
*@param pIcon l'icone sur laquelle pointe le dialogue.
*@param pDock le dock contenant l'icone.
*@param cIconPath le chemin vers une icone a afficher dans la marge.
*@param pActionFunc la fonction d'action, appelee lors du clique utilisateur.
*@param data pointeur qui sera passe en argument de la fonction d'action.
*@param pFreeDataFunc fonction qui liberera le pointeur.
@Returns le dialogue nouvellement cree.
*/
CairoDockDialog *cairo_dock_show_dialog_with_question (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cIconPath, CairoDockActionOnAnswerFunc pActionFunc, gpointer data, GFreeFunc pFreeDataFunc);
/**
*Fait apparaitre un dialogue a duree de vie illimitee avec une entree texte et 2 boutons ok/annuler. Lorsque l'utilisateur clique sur "ok", la fonction d'action est appelee avec le texte de l'entree, et avec "" s'il a clique sur "annuler".
*@param cText le message du dialogue.
*@param pIcon l'icone sur laquelle pointe le dialogue.
*@param pDock le dock contenant l'icone.
*@param cIconPath le chemin vers une icone a afficher dans la marge.
*@param cTextForEntry le texte a afficher initialement dans l'entree.
*@param pActionFunc la fonction d'action, appelee lors du clique utilisateur.
*@param data pointeur qui sera passe en argument de la fonction d'action.
*@param pFreeDataFunc fonction qui liberera le pointeur.
@Returns le dialogue nouvellement cree.
*/
CairoDockDialog *cairo_dock_show_dialog_with_entry (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cIconPath, const gchar  *cTextForEntry, CairoDockActionOnAnswerFunc pActionFunc, gpointer data, GFreeFunc pFreeDataFunc);
/**
*Fait apparaitre un dialogue a duree de vie illimitee avec une echelle horizontale et 2 boutons ok/annuler. Lorsque l'utilisateur clique sur "ok", la fonction d'action est appelee avec la valeur de l'echelle sous forme de texte, et avec "-1" s'il a clique sur "annuler".
*@param cText le message du dialogue.
*@param pIcon l'icone sur laquelle pointe le dialogue.
*@param pDock le dock contenant l'icone.
*@param cIconPath le chemin vers une icone a afficher dans la marge.
*@param fValue la valeur initiale de l'echelle.
*@param pActionFunc la fonction d'action, appelee lors du clique utilisateur.
*@param data pointeur qui sera passe en argument de la fonction d'action.
*@param pFreeDataFunc fonction qui liberera le pointeur.
@Returns le dialogue nouvellement cree.
*/
CairoDockDialog *cairo_dock_show_dialog_with_value (const gchar *cText, Icon *pIcon, CairoDock *pDock, gchar *cIconPath, double fValue, CairoDockActionOnAnswerFunc pActionFunc, gpointer data, GFreeFunc pFreeDataFunc);

/**
*Fait apparaitre un dialogue avec un widget et 2 boutons, et met en pause le programme jusqu'a ce que l'utilisateur ait fait son choix.
*@param cText le message du dialogue.
*@param pIcon l'icone sur laquelle pointe le dialogue.
*@param pDock le dock contenant l'icone.
*@param cIconPath le chemin vers une icone a afficher dans la marge.
*@param iButtonsType type des boutons (GTK_BUTTONS_OK_CANCEL ou GTK_BUTTONS_YES_NO).
*@param pInteractiveWidget un widget d'interaction avec l'utilisateur.
@Returns La reponse sous la fome d'une chaine de caracters nouvellement allouee. Si le widget est une entree de texte, retourne le texte si oui, et "" si non. Si le widget est une echelle, retourne la valeur sous forme de chaine si "oui", et "-1" si non, et sinon, retourne "yes" si oui, et "no" si non. Si le dialogue est detruit entre-temps, NULL est retourne. Si c'est un autre widget, il faut le referencer avant, de facon a ce qu'il ne soit pas detruit avec le dialogue.
*/
gchar *cairo_dock_show_dialog_and_wait (const gchar *cText, Icon *pIcon, CairoDock *pDock, double fTimeLength, gchar *cIconPath, GtkButtonsType iButtonsType, GtkWidget *pInteractiveWidget);
/**
*Fait apparaitre un dialogue avec une entree de texte et 2 boutons ok/annuler, et met en pause le programme jusqu'a ce que l'utilisateur ait fait son choix.
*@param cMessage le message du dialogue.
*@param pIcon l'icone qui fait la demande.
*@param pDock le dock contenant l'icone.
*@param cInitialAnswer la valeur initiale de l'entree de texte, ou NULL si aucune n'est fournie.
@Returns le texte entre par l'utilisateur, ou NULL s'il a annule ou si le dialogue s'est fait detruire avant.
*/
gchar *cairo_dock_show_demand_and_wait (const gchar *cMessage, Icon *pIcon, CairoDock *pDock, const gchar *cInitialAnswer);
/**
*Fait apparaitre un dialogue avec une echelle horizontale entre 0 et 1, et 2 boutons ok/annuler, et met en pause le programme jusqu'a ce que l'utilisateur ait fait son choix.
*@param cMessage le message du dialogue.
*@param pIcon l'icone qui fait la demande.
*@param pDock le dock contenant l'icone.
*@param fInitialValue la valeur initiale de l'echelle, entre 0 et 1.
@Returns la valeur choisie par l'utilisateur, ou -1 s'il a annule ou si le dialogue s'est fait detruire avant.
*/
double cairo_dock_show_value_and_wait (const gchar *cMessage, Icon *pIcon, CairoDock *pDock, double fInitialValue);
/**
*Fait apparaitre un dialogue de question pointant sur l'icone pointee (ou la 1ere si aucune n'est pointee) avec une question et 2 boutons oui/non, et met en pause le programme jusqu'a ce que l'utilisateur ait fait son choix.
*@param cQuestion la question a poser.
*@param pIcon l'icone qui fait la demande.
*@param pDock le dock contenant l'icone.
@Returns GTK_RESPONSE_YES ou GTK_RESPONSE_NO suivant le choix de l'utilisateur, ou GTK_RESPONSE_NONE si le dialogue s'est fait detruire avant.
*/
int cairo_dock_ask_question_and_wait (const gchar *cQuestion, Icon *pIcon, CairoDock *pDock);
/**
*Fait apparaitre un dialogue de question bloquant, et pointant sur l'icone pointee du dock principal (ou la 1ere icone si aucune n'est pointee). Cela permet a cairo-dock de poser une question d'ordre general.
*@param cQuestion la question a poser.
@Returns idem que pour #cairo_dock_ask_question_and_wait .
*/
int cairo_dock_ask_general_question_and_wait (const gchar *cQuestion);


#endif
