/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by ChAnGFu (for any bug report, please mail me to changfu@cairo-dock.org)

*********************************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>

#include "cairo-dock-draw.h"
#include "cairo-dock-config.h"
#include "cairo-dock-surface-factory.h"
#include "cairo-dock-log.h"
#include "cairo-dock-dock-factory.h"
#include "cairo-dock-emblem.h"

CairoDockFullEmblem pFullEmblems[CAIRO_DOCK_EMBLEM_CLASSIC_NB];
gchar *cEmblemConfPath[CAIRO_DOCK_EMBLEM_CLASSIC_NB];
extern double g_fAmplitude;
extern gboolean g_bDisplayDropEmblem;

//Fonctions proposée par Nécropotame, rédigée par ChAnGFu
void cairo_dock_draw_emblem_on_my_icon (cairo_t *pIconContext, const gchar *cIconFile, Icon *pIcon, CairoContainer *pContainer, CairoDockEmblem pEmblemType, gboolean bPersistent)
{
	cd_debug ("%s (%s %d)", __func__, cIconFile, pEmblemType);
	g_return_if_fail (pIcon != NULL && pContainer != NULL); 
	
	if (cIconFile == NULL) 
		return;
	
	cairo_surface_t *pCairoSurface=NULL;
	double fImgX, fImgY, fImgW, fImgH, emblemW = pIcon->fWidth / 3, emblemH = pIcon->fHeight / 3;
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / 1 : 1);
	pCairoSurface = cairo_dock_create_surface_from_image (cIconFile, pIconContext, fMaxScale, emblemW, emblemH, CAIRO_DOCK_KEEP_RATIO, &fImgW, &fImgH, NULL, NULL);
	cairo_dock_draw_emblem_from_surface (pIconContext, pCairoSurface, pIcon, pContainer, pEmblemType, bPersistent);

	cairo_surface_destroy (pCairoSurface);
}

void cairo_dock_draw_emblem_from_surface (cairo_t *pIconContext, cairo_surface_t *pSurface, Icon *pIcon, CairoContainer *pContainer, CairoDockEmblem pEmblemType, gboolean bPersistent)
{
	cd_debug ("%s (%d %d)", __func__, pEmblemType, bPersistent);
	g_return_if_fail (pIcon != NULL && pContainer != NULL); 
	
	if (pSurface == NULL) 
		return;
	
	double fImgX, fImgY, emblemW = pIcon->fWidth / 3, emblemH = pIcon->fHeight / 3;
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / 1 : 1);
	
	switch (pEmblemType) {
		default:
		case CAIRO_DOCK_EMBLEM_UPPER_RIGHT :
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale;
			fImgY = 1.;
		break;

		case CAIRO_DOCK_EMBLEM_LOWER_RIGHT :
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale;
			fImgY = ((pIcon->fHeight - emblemH - pIcon->fScale) * fMaxScale) + 1.;
		break;
		
		case CAIRO_DOCK_EMBLEM_UPPER_LEFT :
			fImgX = 1.;
			fImgY = 1.;
		break;
		
		case CAIRO_DOCK_EMBLEM_LOWER_LEFT :
			fImgX = 1.;
			fImgY = ((pIcon->fHeight - emblemH - pIcon->fScale) * fMaxScale) + 1.;
		break;
		
		case CAIRO_DOCK_EMBLEM_MIDDLE :
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale / 2.;
			fImgY = (pIcon->fHeight - emblemH - pIcon->fScale) * fMaxScale / 2.;
		break;
		
		case CAIRO_DOCK_EMBLEM_MIDDLE_BOTTOM:
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale / 2.;
			fImgY = ((pIcon->fHeight - emblemH - pIcon->fScale) * fMaxScale) + 1.;
		break;

		case CAIRO_DOCK_EMBLEM_BACKGROUND :
			fImgX = (pIcon->fWidth - emblemW - pIcon->fScale) * fMaxScale / 2.;
			fImgY = 0.;
			cairo_surface_t *pNewSurfaceGradated = cairo_surface_create_similar (pSurface, CAIRO_CONTENT_COLOR_ALPHA, emblemW, emblemH);
			cairo_t *pCairoContext = cairo_create (pNewSurfaceGradated);
			cairo_set_source_surface (pCairoContext, pSurface, 0, 0);

			cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0., 1., 0., (emblemH - 1.));  // de haut en bas.
			g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);

			cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
			cairo_pattern_add_color_stop_rgba (pGradationPattern, 1., 0., 0., 0., 0.);
			cairo_pattern_add_color_stop_rgba (pGradationPattern, 0., 0., 0., 0., emblemH);

			cairo_translate (pCairoContext, 0, 0);
			cairo_mask (pCairoContext, pGradationPattern);

			cairo_pattern_destroy (pGradationPattern);
			cairo_destroy (pCairoContext);
			pSurface = pNewSurfaceGradated;
		break;
	}
	
	//cd_debug ("Emblem: X %.0f Y %.0f W %.0f H %.0f - Icon: W %.0f H %.0f", fImgX, fImgY, emblemW, emblemH, pIcon->fWidth, pIcon->fHeight);
	
	if (!bPersistent)
		cairo_save (pIconContext);
		
	cairo_set_source_surface (pIconContext, pSurface, fImgX, fImgY);
	cairo_paint (pIconContext);
	
	if (!bPersistent)
		cairo_restore (pIconContext);
}

void cairo_dock_draw_emblem_classic (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer, CairoDockClassicEmblem pEmblemClassic, CairoDockEmblem pEmblemType, gboolean bPersistent)
{
	cd_debug ("%s (%s %d %d)", __func__, pIcon->acName, pEmblemClassic, pEmblemType);
	g_return_if_fail (pIcon != NULL); 
	
	gchar *cClassicEmblemPath = NULL;
	if (cEmblemConfPath[pEmblemClassic] == NULL) {
		switch (pEmblemClassic) {
			default :
			case CAIRO_DOCK_EMBLEM_BLANK :
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/blank.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_CHARGE:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/charge.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_DROP_INDICATOR:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/drop.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_PLAY:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/play.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_PAUSE:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/pause.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_STOP:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/stop.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_BROKEN:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/broken.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_ERROR:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/error.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_WARNING:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/warning.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
			case CAIRO_DOCK_EMBLEM_LOCKED:
				cClassicEmblemPath = g_strdup_printf ("%s/emblems/locked.svg", CAIRO_DOCK_SHARE_DATA_DIR);
			break;
		}
	}
	else
		cClassicEmblemPath = g_strdup(cEmblemConfPath[pEmblemClassic]);
		
	//On évite de recharger les surfaces
	double fImgX, fImgY, fImgW, fImgH, emblemW = pIcon->fWidth / 3, emblemH = pIcon->fHeight / 3;
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / 1 : 1);
	if (pFullEmblems[pEmblemClassic].pSurface == NULL || (pFullEmblems[pEmblemClassic].fEmblemW != emblemW || pFullEmblems[pEmblemClassic].fEmblemH != emblemH) || strcmp (pFullEmblems[pEmblemClassic].cImagePath, cClassicEmblemPath) != 0)
	{
		if (pFullEmblems[pEmblemClassic].pSurface != NULL)
			cairo_surface_destroy (pFullEmblems[pEmblemClassic].pSurface); //On sauvegarde un maximum de mémoire
			
		pFullEmblems[pEmblemClassic].pSurface = cairo_dock_create_surface_from_image (cClassicEmblemPath, pIconContext, fMaxScale, emblemW, emblemH, CAIRO_DOCK_KEEP_RATIO, &fImgW, &fImgH, NULL, NULL);
		pFullEmblems[pEmblemClassic].fEmblemW = emblemW;
		pFullEmblems[pEmblemClassic].fEmblemH = emblemH;
		
		if (pFullEmblems[pEmblemClassic].cImagePath != NULL)
			g_free (pFullEmblems[pEmblemClassic].cImagePath);
		pFullEmblems[pEmblemClassic].cImagePath = g_strdup (cClassicEmblemPath);
	} //On (re)charge uniquement si la surface n'existe pas, si le fichier image est différent ou si les emblemes on changés de tailles (en particulier pour les desklets)
	
	cairo_dock_draw_emblem_from_surface (pIconContext, pFullEmblems[pEmblemClassic].pSurface, pIcon, pContainer, pEmblemType, bPersistent);
	g_free (cClassicEmblemPath);
}

gboolean _cairo_dock_erase_temporary_emblem (CairoDockTempEmblem *pEmblem)
{
	if (pEmblem != NULL) {
		pEmblem->iSidTimer = 0;
		cairo_dock_draw_emblem_classic (pEmblem->pIconContext, pEmblem->pIcon, pEmblem->pContainer, CAIRO_DOCK_EMBLEM_BLANK, CAIRO_DOCK_EMBLEM_MIDDLE, FALSE);
		cairo_dock_redraw_my_icon (pEmblem->pIcon, pEmblem->pContainer);
	}
	g_free (pEmblem);
	return FALSE;
}

void cairo_dock_draw_temporary_emblem_on_my_icon (cairo_t *pIconContext, Icon *pIcon, CairoContainer *pContainer, const gchar *cIconFile, CairoDockClassicEmblem pEmblemClassic, CairoDockEmblem pEmblemType, double fTimeLength)
{
	cd_debug ("%s (%s %d %d %.0f)", __func__, cIconFile, pEmblemClassic, pEmblemType, fTimeLength);
	if (cIconFile == NULL && (pEmblemClassic < 0 || pEmblemClassic > CAIRO_DOCK_EMBLEM_CLASSIC_NB))
		return;
	
	if (pEmblemType < 0 || pEmblemType > CAIRO_DOCK_EMBLEM_TOTAL_NB)
		return;
	
	if (cIconFile != NULL)
		cairo_dock_draw_emblem_on_my_icon (pIconContext, cIconFile, pIcon, pContainer, pEmblemType, FALSE);
	else
		cairo_dock_draw_emblem_classic (pIconContext, pIcon, pContainer, pEmblemClassic, pEmblemType, FALSE);
	
	CairoDockTempEmblem *pEmblem = g_new0 (CairoDockTempEmblem, 1);
	pEmblem->pIcon = pIcon;
	pEmblem->pContainer = pContainer;
	pEmblem->pIconContext = pIconContext;
	pEmblem->iSidTimer = 0;
	
	if (fTimeLength > 0)
		pEmblem->iSidTimer = g_timeout_add (fTimeLength, (GSourceFunc) _cairo_dock_erase_temporary_emblem, (gpointer) pEmblem);
}

//A lancer a l'init du thèmes
void cairo_dock_get_emblem_path (gchar *cConfFilePath)
{
	cd_debug ("%s (%s)", __func__, cConfFilePath);
	
	if (cConfFilePath == NULL)
		return;
	
	gboolean bFlushConfFileNeeded = FALSE;
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		return ;
	}
	
	g_bDisplayDropEmblem = cairo_dock_get_boolean_key_value (pKeyFile, "Emblems", "drop indicator", &bFlushConfFileNeeded, NULL, NULL, NULL);
	
	gint i;
	gchar *cKeyName = NULL;
	for (i = 1; i <= CAIRO_DOCK_EMBLEM_CLASSIC_NB; i++) {
		cKeyName = g_strdup_printf ("emblem_%d", i);
		cEmblemConfPath[i] = cairo_dock_get_string_key_value (pKeyFile, "Emblems", cKeyName, &bFlushConfFileNeeded, NULL, NULL, NULL);
	}
	g_free (cKeyName);
	g_key_file_free (pKeyFile);
}

//A lancer a la sortie du dock
void cairo_dock_free_emblem (void)
{
	gint i;
	
	for (i = 1; i <= CAIRO_DOCK_EMBLEM_CLASSIC_NB; i++) {
		g_free (cEmblemConfPath[i]);
		cEmblemConfPath[i] = NULL;
	}
	for (i = 0; i <= CAIRO_DOCK_EMBLEM_CLASSIC_NB; i++) {
		if (pFullEmblems[i].pSurface != NULL) {
			cairo_surface_destroy (pFullEmblems[i].pSurface);
			pFullEmblems[i].pSurface = NULL;
		}
		if (pFullEmblems[i].cImagePath != NULL) {
			g_free (pFullEmblems[i].cImagePath);
			pFullEmblems[i].cImagePath = NULL;
		}
	}
}

//A lancer quand la configuration est mise a jour
void cairo_dock_updated_emblem_conf_file (gchar *cConfFilePath)
{
	cairo_dock_free_emblem ();
	cairo_dock_get_emblem_path (cConfFilePath);
}
