#include <cairo-dock.h>

#include "cairo-dock-struct.h"

Gauge *init_cd_Gauge(cairo_t *pSourceContext, gchar *themePath, int iWidth, int iHeight)
{
	cd_debug("");
	Gauge *pGauge = g_new0 (Gauge, 1);
	if (themePath != NULL)
	{
		GString *sElementPath = g_string_new ("");
		g_string_printf (sElementPath, "%s/theme.conf",themePath);
	
		cd_debug("gauge : Fichier de config du theme(%s)",sElementPath->str);
	
		//On cherche les informations sur le theme
		GError *erreur = NULL;
		gboolean bFlushConfFileNeeded = FALSE;
		GKeyFile *pThemeKeyFile = g_key_file_new ();
		g_key_file_load_from_file (pThemeKeyFile,sElementPath->str, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		else
		{
			pGauge->sizeX = iWidth;
			pGauge->sizeY = iHeight;
			pGauge->themeName = cairo_dock_get_string_key_value (pThemeKeyFile, "Informations", "name", &bFlushConfFileNeeded, "", NULL, NULL);
			pGauge->themeType = cairo_dock_get_string_key_value (pThemeKeyFile, "Informations", "format", &bFlushConfFileNeeded, "", NULL, NULL);
			
			cd_debug("gauge : Load theme (%s,%s)",pGauge->themeName,pGauge->themeType);
			
			gchar *imagePath;
			//On charge l'image de fond
			
			imagePath = cairo_dock_get_string_key_value (pThemeKeyFile, "Files", "background", &bFlushConfFileNeeded, "background.svg", NULL, NULL);
			g_string_printf (sElementPath, "%s/%s", themePath,imagePath);
			pGauge->imageBackground = init_cd_GaugeImage(sElementPath->str);
			draw_cd_GaugeImage(pSourceContext, pGauge->imageBackground, iWidth, iHeight);
			
			//On charge l'image du dessus
			imagePath = cairo_dock_get_string_key_value (pThemeKeyFile, "Files", "foreground", &bFlushConfFileNeeded, "foreground.svg", NULL, NULL);
			g_string_printf (sElementPath, "%s/%s", themePath,imagePath);
			pGauge->imageForeground = init_cd_GaugeImage(sElementPath->str);
			draw_cd_GaugeImage(pSourceContext, pGauge->imageForeground, iWidth, iHeight);
			
			if(strcmp(pGauge->themeType,"rotate") == 0)
			{
				pGauge->centerX = cairo_dock_get_double_key_value (pThemeKeyFile, "Rotate", "centerX", &bFlushConfFileNeeded, 0, NULL, NULL);
				pGauge->centerY = cairo_dock_get_double_key_value (pThemeKeyFile, "Rotate", "centerY", &bFlushConfFileNeeded, 0, NULL, NULL);
				pGauge->posStart = cairo_dock_get_double_key_value (pThemeKeyFile, "Rotate", "degreeStart", &bFlushConfFileNeeded, 0, NULL, NULL);
				pGauge->posStop = cairo_dock_get_double_key_value (pThemeKeyFile, "Rotate", "degreeStop", &bFlushConfFileNeeded, 360, NULL, NULL);

				GaugeImage *pGaugeImage;
				imagePath = cairo_dock_get_string_key_value (pThemeKeyFile, "Files", "needle", &bFlushConfFileNeeded, "needle.svg", NULL, NULL);
				g_string_printf (sElementPath, "%s/%s", themePath,imagePath);
				pGaugeImage = init_cd_GaugeImage(sElementPath->str);
				pGauge->imageList = g_list_append (pGauge->imageList, pGaugeImage);
				
				pGauge->nbImage = 1;
			}
			else if(strcmp(pGauge->themeType,"image") == 0)
			{
				int nbFile = cairo_dock_get_double_key_value (pThemeKeyFile, "Image", "nb_image", &bFlushConfFileNeeded, 0, NULL, NULL);
				int i,j = 0;
				for(i = 0; i <= nbFile; i++)
				{
					GaugeImage *pGaugeImage;
					imagePath = cairo_dock_get_string_key_value (pThemeKeyFile, "Files", g_strdup_printf ("image%d", i), &bFlushConfFileNeeded, "", NULL, NULL);
					if(imagePath != NULL)
					{
						g_string_printf (sElementPath, "%s/%s", themePath,imagePath);
						pGaugeImage = init_cd_GaugeImage(sElementPath->str);
						draw_cd_GaugeImage(pSourceContext, pGaugeImage, iWidth, iHeight);
						pGauge->imageList = g_list_append (pGauge->imageList, pGaugeImage);
						
						j++;
					}
				}
				
				pGauge->nbImage = j;
			}
			
			g_free(imagePath);
			g_key_file_free (pThemeKeyFile);	
		}
		
		g_string_free (sElementPath, TRUE);
	}
	return pGauge;
}

GaugeImage *init_cd_GaugeImage(gchar *sImagePath)
{
	GaugeImage *pGaugeImage = g_new0 (GaugeImage, 1);
	
	pGaugeImage->svgNeedle = rsvg_handle_new_from_file (sImagePath, NULL);
	
	//On récupère la taille de l'image
	RsvgDimensionData SizeInfo;
	rsvg_handle_get_dimensions (pGaugeImage->svgNeedle, &SizeInfo);
	pGaugeImage->sizeX = SizeInfo.width;
	pGaugeImage->sizeY = SizeInfo.height;
	
	return pGaugeImage;
}

void draw_cd_GaugeImage(cairo_t *pSourceContext, GaugeImage *pGaugeImage, int iWidth, int iHeight)
{
	cd_debug("gauge : %s\n",__func__);
	
	cairo_surface_t* pNewSurface = NULL;
	cairo_t* pDrawingContext = NULL;
	
	if (pGaugeImage->cairoSurface != NULL)
		cairo_surface_destroy (pGaugeImage->cairoSurface);
	
	pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		iWidth,
		iHeight);
	
	pDrawingContext = cairo_create (pNewSurface);
	
	cairo_scale (pDrawingContext,
		(double) iWidth / (double) pGaugeImage->sizeX,
		(double) iHeight / (double) pGaugeImage->sizeY);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_OVER);
	cairo_paint (pDrawingContext);
	
	if (pGaugeImage->svgNeedle != NULL)
		rsvg_handle_render_cairo (pGaugeImage->svgNeedle, pDrawingContext);
	
	cairo_destroy (pDrawingContext);
	pGaugeImage->cairoSurface = pNewSurface;
}

void make_cd_Gauge(cairo_t *pSourceContext, CairoDock *pDock, Icon *pIcon, Gauge *pGauge, double dValue)
{
	cd_debug("gauge : %s\n",__func__);
	
	if(dValue > 1) dValue = 1;
	else if(dValue < 0) dValue = 0;
	
	double fMaxScale = (pDock != NULL ? 1 + g_fAmplitude : 1);
	
	if(strcmp(pGauge->themeType,"rotate") == 0)
	{
		draw_cd_Gauge_rotate(pSourceContext, pGauge, dValue);
	}
	else if(strcmp(pGauge->themeType,"image") == 0)
	{
		draw_cd_Gauge_image(pSourceContext, pGauge, dValue);
	}
	
	if (pDock != NULL && pDock->bUseReflect)
	{
		cairo_surface_t *pReflet = pIcon->pReflectionBuffer;
		pIcon->pReflectionBuffer = NULL;
		cairo_surface_destroy (pReflet);
		
		pIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (pIcon->pIconBuffer,
			pSourceContext,
			(pDock->bHorizontalDock ? pIcon->fWidth : pIcon->fHeight) * fMaxScale,
			(pDock->bHorizontalDock ? pIcon->fHeight : pIcon->fWidth) * fMaxScale,
			pDock->bHorizontalDock,
			1 + g_fAmplitude);
	}
	
	cairo_dock_redraw_my_icon(pIcon,CAIRO_DOCK_CONTAINER(pDock));
}

void draw_cd_Gauge_rotate(cairo_t *pSourceContext, Gauge *pGauge, double dValue)
{
	cd_debug("gauge : %s\n",__func__);
	
	double fHalfX;
	double fHalfY;
	
	double physicValue = ((dValue * (pGauge->posStop - pGauge->posStart)) + pGauge->posStart) / 360;
	
	GaugeImage *pGaugeImage;
	pGaugeImage = pGauge->imageBackground;
	
	fHalfX = pGaugeImage->sizeX / 2.0f * (1 + pGauge->centerX);
	fHalfY = pGaugeImage->sizeY / 2.0f * (1 - pGauge->centerY);
	
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	
	//On affiche le fond
	cairo_set_source_surface (pSourceContext, pGaugeImage->cairoSurface, 0.0f, 0.0f);
	
	//On tranforme la surface pour l'aiguille
	cairo_save (pSourceContext);
	cairo_scale (pSourceContext,
		(double) pGauge->sizeX / (double) pGaugeImage->sizeX,
		(double) pGauge->sizeY / (double) pGaugeImage->sizeY);
	cairo_paint (pSourceContext);
	
	cairo_translate (pSourceContext, fHalfX, fHalfY);
	cairo_rotate (pSourceContext, -G_PI/2.0f);
	cairo_rotate (pSourceContext, G_PI*physicValue*2.0f);
	
	//On charge l'image de l'aiguille
	GList *pElement;
	pElement = pGauge->imageList;
	pGaugeImage = pElement->data;
		
	rsvg_handle_render_cairo (pGaugeImage->svgNeedle, pSourceContext);
	cairo_restore (pSourceContext);
	
	//On affiche l'image du dessus
	pGaugeImage = pGauge->imageForeground;
	cairo_set_source_surface (pSourceContext, pGaugeImage->cairoSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
}

void draw_cd_Gauge_image(cairo_t *pSourceContext, Gauge *pGauge, double dValue)
{
	cd_debug("gauge : %s\n",__func__);
	
	double fHalfX;
	double fHalfY;
	
	//On cherche l'image à afficher en fonction de la valeur
	int trueImage;
	if(pGauge->nbImage == 1) trueImage = 0;
	else if(pGauge->nbImage == 2)
	{
		if(dValue <= 0.5) trueImage = 0;
		else trueImage = 1;
	} 
	else if(pGauge->nbImage == 3)
	{
		if(dValue <= 0.5) trueImage = 0;
		else trueImage = 1;
	} 
	else trueImage = ((dValue*100)-5)/(90/(pGauge->nbImage - 2))+1;
	
	GaugeImage *pGaugeImage;
	
	//On charge l'image correspondante à la valeur
	int i = 0;
	GList *pElement;
	for(pElement = pGauge->imageList; pElement != NULL; pElement = pElement->next)
	{
		if(i > trueImage) break;
		else if(i == trueImage)
		{
			pGaugeImage = pElement->data;
		}
		i++;
	}
	
	
	fHalfX = pGaugeImage->sizeX / 2.0f * (1 + pGauge->centerX);
	fHalfY = pGaugeImage->sizeY / 2.0f * (1 - pGauge->centerY);
	
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	
	cairo_set_source_surface (pSourceContext, pGaugeImage->cairoSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
}

void free_cd_Gauge(Gauge *pGauge)
{
	cd_debug("");
	if (pGauge == NULL)
		return ;
	
	if(pGauge->themeName != NULL) g_free(pGauge->themeName);
	if(pGauge->themeType != NULL) g_free(pGauge->themeType);
	
	if(pGauge->imageBackground != NULL) free_cd_GaugeImage(pGauge->imageBackground);
	if(pGauge->imageForeground != NULL) free_cd_GaugeImage(pGauge->imageForeground);
	GList *pElement;
	for (pElement = pGauge->imageList; pElement != NULL; pElement = pElement->next)
	{
		free_cd_GaugeImage(pElement->data);
	}
	g_list_free (pGauge->imageList);
	
	g_free (pGauge);
}

void free_cd_GaugeImage(GaugeImage *pGaugeImage)
{
	cd_debug("");
	
	if(pGaugeImage->svgNeedle != NULL) rsvg_handle_free (pGaugeImage->svgNeedle);
	if(pGaugeImage->cairoSurface != NULL) cairo_surface_destroy (pGaugeImage->cairoSurface);
	
	g_free (pGaugeImage);
}

gchar *cairo_dock_get_gauge_key_value(gchar *cAppletConfFilePath, GKeyFile *pKeyFile, gchar *cGroupName, gchar *cKeyName, gboolean *bFlushConfFileNeeded, gchar *cDefaultThemeName)
{
	gchar *themePath;
	themePath = cairo_dock_manage_themes_for_applet (CAIRO_DOCK_SHARE_DATA_DIR, "gauges", cAppletConfFilePath, pKeyFile, cGroupName, cKeyName, bFlushConfFileNeeded, cDefaultThemeName);
	cd_debug("Clés du theme : [%s] %s",cGroupName,cKeyName);
	cd_debug("Theme de la jauge : %s",themePath);
	cd_debug("Dossier des jauges : %s/gauges",CAIRO_DOCK_SHARE_DATA_DIR);
	return themePath;
	g_free(themePath);
}
