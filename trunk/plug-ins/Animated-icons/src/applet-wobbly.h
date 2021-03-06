
#ifndef __APPLET_WOBBLY__
#define  __APPLET_WOBBLY__


#include <cairo-dock.h>


void cd_animations_init_wobbly (CDAnimationData *pData, gboolean  bUseOpenGL);


gboolean cd_animations_update_wobbly (CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bWillContinue);

gboolean cd_animations_update_wobbly_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, gboolean bWillContinue);


void cd_animations_draw_wobbly_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData);

void cd_animations_draw_wobbly_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext);


#endif
