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

cairo_surface_t *pEmblemSurface[CAIRO_DOCK_EMBLEM_CLASSIC_NB];
double fEmblemW[CAIRO_DOCK_EMBLEM_CLASSIC_NB];
double fEmblemH[CAIRO_DOCK_EMBLEM_CLASSIC_NB];
gchar *cEmblemConfPath[CAIRO_DOCK_EMBLEM_CLASSIC_NB];
extern gchar *g_cConfFile;
extern double g_fAmplitude;

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
	cd_debug ("%s (%s %d)", __func__, pIcon->acName, pEmblemType);
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
			//Il reste les svg de play pause stop broken a faire.
		}
	}
	else
		cClassicEmblemPath = g_strdup(cEmblemConfPath[pEmblemClassic]);
		
	//On évite de recharger les surfaces
	double fImgX, fImgY, fImgW, fImgH, emblemW = pIcon->fWidth / 3, emblemH = pIcon->fHeight / 3;
	double fMaxScale = (CAIRO_DOCK_IS_DOCK (pContainer) ? (1 + g_fAmplitude) / 1 : 1);
	if (pEmblemSurface[pEmblemClassic] == NULL || (fEmblemW[pEmblemClassic] != emblemW || fEmblemH[pEmblemClassic] != emblemH))
	{
		if (pEmblemSurface[pEmblemClassic] != NULL)
			cairo_surface_destroy (pEmblemSurface[pEmblemClassic]); //On sauvegarde au maximum de mémoire
			
		pEmblemSurface[pEmblemClassic] = cairo_dock_create_surface_from_image (cClassicEmblemPath, pIconContext, fMaxScale, emblemW, emblemH, CAIRO_DOCK_KEEP_RATIO, &fImgW, &fImgH, NULL, NULL);
		fEmblemW[pEmblemClassic] = emblemW;
		fEmblemH[pEmblemClassic] = emblemH;
	} //On (re)charge uniquement si la surface n'existe pas ou si les emblemes on changés de tailles (en particulier les desklets)
	
	cairo_dock_draw_emblem_from_surface (pIconContext, pEmblemSurface[pEmblemClassic], pIcon, pContainer, pEmblemType, bPersistent);
	g_free (cClassicEmblemPath);
}

//A lancer a l'init du thèmes
void cairo_dock_get_emblem_path (void)
{
	gboolean bFlushConfFileNeeded = FALSE;
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, g_cConfFile, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		return ;
	}
	
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
	
	for (i = 1; i <= CAIRO_DOCK_EMBLEM_CLASSIC_NB; i++)
		g_free(cEmblemConfPath[i]);
		
	for (i = 0; i <= CAIRO_DOCK_EMBLEM_CLASSIC_NB; i++) {
		if (pEmblemSurface[i] != NULL)
			cairo_surface_destroy(pEmblemSurface[i]);
	}
}

//A lancer quand la configuration est mise a jour
void cairo_dock_updated_emblem_conf_file (void)
{
	cairo_dock_free_emblem ();
	cairo_dock_get_emblem_path ();
}
