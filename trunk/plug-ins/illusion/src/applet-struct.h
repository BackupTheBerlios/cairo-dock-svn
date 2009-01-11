
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum _CDIllusionEffect {
	CD_ILLUSION_EVAPORATE=0,
	CD_ILLUSION_FADE_OUT,
	CD_ILLUSION_EXPLODE,
	CD_ILLUSION_NB_EFFECTS
	} CDIllusionEffect;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	CDIllusionEffect iEffect;
	
	gint iEvaporateDuration;
	gint iEvaporateNbParticles;
	gdouble pEvaporateColor1[3];
	gdouble pEvaporateColor2[3];
	gboolean bMysticalEvaporate;
	gint iNbEvaporateParticles;
	gint iEvaporateParticleSize;
	gdouble fEvaporateParticleSpeed;
	gboolean bEvaporateFromBottom;
	
	gint iFadeOutDuration;
	
	gint iExplodeDuration;
	gint iExplodeNbPiecesX, iExplodeNbPiecesY;
	gdouble fExplosionRadius;
	gboolean bExplodeCube;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iEvaporateTexture;
	} ;

typedef struct {
	gdouble fAngleOffset;
	gdouble v, vz;
	} CDIllusionExplosion;

typedef struct _CDIllusionData {
	gdouble fEvaporateSpeed;
	gdouble fEvaporatePercent;
	CairoParticleSystem *pEvaporateSystem;
	
	gdouble fFadeOutSpeed;
	gdouble fFadeOutAlpha;
	
	gdouble fExplodeDeltaT;
	gint iExplosionCount;
	gdouble fExplosionRadius;
	gdouble fExplosionRotation;
	gdouble fExplodeAlpha;
	CDIllusionExplosion *pExplosionPart;
	
	} CDIllusionData;

#endif