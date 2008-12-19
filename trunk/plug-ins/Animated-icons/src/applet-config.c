/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	int i;
	for (i = 0; i < CD_ANIMATIONS_NB_EFFECTS; i ++)
	{
		myConfig.iEffectsOnMouseOver[i] = -1;
	}
	for (i = 0; i < CD_ANIMATIONS_NB_EFFECTS; i ++)
	{
		myConfig.iEffectsOnClick[i] = -1;
	}
	
	CD_CONFIG_GET_INTEGER_LIST ("Global", "hover effects", CD_ANIMATIONS_NB_EFFECTS, myConfig.iEffectsOnMouseOver);
	CD_CONFIG_GET_INTEGER_LIST ("Global", "click effects", CD_ANIMATIONS_NB_EFFECTS, myConfig.iEffectsOnClick);
	
	for (i = 0; i < CD_ANIMATIONS_NB_EFFECTS; i ++)
	{
		switch (myConfig.iEffectsOnMouseOver[i])
		{
			case CD_ANIMATIONS_BOUNCE :
				
			break;
			
			case CD_ANIMATIONS_ROTATE :
				myConfig.iRotationDuration = CD_CONFIG_GET_INTEGER ("Rotation", "duration");
				myConfig.bContinueRotation = CD_CONFIG_GET_BOOLEAN ("Rotation", "continue");
				myConfig.iMeshType = CD_CONFIG_GET_INTEGER ("Rotation", "mesh");
				gdouble pMeshColor[4];
				CD_CONFIG_GET_COLOR ("Rotation", "color", pMeshColor);
				for (i=0; i<4; i++)
					myConfig.pMeshColor[i] = pMeshColor[i];
			break;
			
			case CD_ANIMATIONS_PULSE :
				
			break;
			
			case CD_ANIMATIONS_WOBBLY :
				myConfig.iInitialStrecth = CD_CONFIG_GET_INTEGER ("Wobbly", "stretch");
				myConfig.fSpringConstant = CD_CONFIG_GET_DOUBLE ("Wobbly", "spring cst");
				myConfig.fFriction = CD_CONFIG_GET_DOUBLE ("Wobbly", "friction");
				myConfig.iNbGridNodes = CD_CONFIG_GET_INTEGER ("Wobbly", "grid nodes");
			break;
			
			case CD_ANIMATIONS_SPOT :
				myConfig.iSpotDuration = CD_CONFIG_GET_INTEGER ("Spot", "duration");
				myConfig.bContinueSpot = CD_CONFIG_GET_BOOLEAN ("Spot", "continue");
				gdouble pColor[4];
				CD_CONFIG_GET_COLOR_RVB ("Spot", "spot color", pColor);
				for (i=0; i<3; i++)
					myConfig.pSpotColor[i] = pColor[i];
				CD_CONFIG_GET_COLOR ("Spot", "halo color", pColor);
				for (i=0; i<4; i++)
					myConfig.pHaloColor[i] = pColor[i];
				
				
				CD_CONFIG_GET_COLOR_RVB ("Spot", "color1", myConfig.pRaysColor1);
				//for (i=0; i<3; i++)
				//	myConfig.pRaysColor1[i] = pColor[i];
				CD_CONFIG_GET_COLOR_RVB ("Spot", "color2", myConfig.pRaysColor2);
				//for (i=0; i<3; i++)
				//	myConfig.pRaysColor2[i] = pColor[i];
				myConfig.bMysticalRays = CD_CONFIG_GET_BOOLEAN ("Spot", "mystical");
				myConfig.iNbRaysParticles = CD_CONFIG_GET_INTEGER ("Spot", "nb part");
				myConfig.iRaysParticleSize = CD_CONFIG_GET_INTEGER ("Spot", "part size");
				myConfig.fRaysParticleSpeed = CD_CONFIG_GET_DOUBLE ("Spot", "part speed");
			break;
			
			case CD_ANIMATIONS_WAVE :
				myConfig.iWaveDuration = CD_CONFIG_GET_INTEGER ("Wave", "duration");
				myConfig.bContinueWave = CD_CONFIG_GET_BOOLEAN ("Wave", "continue");
				myConfig.fWaveWidth = CD_CONFIG_GET_DOUBLE ("Wave", "width");
				myConfig.fWaveAmplitude = CD_CONFIG_GET_DOUBLE ("Wave", "amplitude");
			break;
			
			default :
			break;
		}
	}
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.iChromeTexture != 0)
		glDeleteTextures (1, &myData.iChromeTexture);
	if (myData.iSpotTexture == 0)
		glDeleteTextures (1, &myData.iSpotTexture);
	if (myData.iHaloTexture == 0)
		glDeleteTextures (1, &myData.iHaloTexture);
	if (myData.iSpotFrontTexture == 0)
		glDeleteTextures (1, &myData.iSpotFrontTexture);
	if (myData.iRaysTexture == 0)
		glDeleteTextures (1, &myData.iRaysTexture);
	
	if (myData.iCallList[CD_SQUARE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_SQUARE_MESH], 1);
	if (myData.iCallList[CD_CUBE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_CUBE_MESH], 1);
	if (myData.iCallList[CD_CAPSULE_MESH] != 0)
		glDeleteLists (myData.iCallList[CD_CAPSULE_MESH], 1);
CD_APPLET_RESET_DATA_END
