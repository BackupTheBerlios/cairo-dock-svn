#include <cairo-dock.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "cairo-dock-struct.h"

void cd_xml_open_file(gchar *filePath,gchar *mainNodeName,xmlDocPtr *myXmlDoc,xmlNodePtr *myXmlNode)
{
	xmlDocPtr doc = xmlParseFile (filePath);
	
	if (doc == NULL)
	{
		cd_warning("Impossible de lire le fichier XML.");
		*myXmlDoc = NULL;
		return ;
	}
	
	xmlNodePtr node = xmlDocGetRootElement (doc);
	
	if (node == NULL || xmlStrcmp (node->name, (const xmlChar *) mainNodeName) != 0)
	{
		cd_warning("Le format du fichier XML n'est pas valide.");
		*myXmlDoc = NULL;
		return ;
	}
	
	*myXmlDoc = doc;
	*myXmlNode = node;
}

Gauge *init_cd_Gauge(cairo_t *pSourceContext, gchar *themePath, int iWidth, int iHeight)
{
	cd_debug("");
	cd_debug("gauge : On cherche le theme : %s",themePath);
	Gauge *pGauge = g_new0 (Gauge, 1);
	pGauge->themeName = NULL;
	
	if (themePath != NULL)
	{
		gchar *imagePath = NULL;
		GaugeImage *pGaugeImage;
		
		pGauge->indicatorList = NULL;
		
		pGauge->imageBackground = NULL;
		pGauge->imageForeground = NULL;
		
		pGauge->sizeX = iWidth;
		pGauge->sizeY = iHeight;
			
		xmlInitParser ();
		xmlDocPtr pGaugeTheme;
		xmlNodePtr pGaugeMainNode;
		
		cd_xml_open_file(g_strdup_printf("%s/theme.xml",themePath),"gauge",&pGaugeTheme,&pGaugeMainNode);
		
		if(pGaugeTheme != NULL)
		{
			xmlNodePtr pGaugeNode;
			xmlAttrPtr ap;
			
			for (pGaugeNode = pGaugeMainNode->xmlChildrenNode; pGaugeNode != NULL; pGaugeNode = pGaugeNode->next)
			{
				if (xmlStrcmp (pGaugeNode->name, (const xmlChar *) "name") == 0)
				{
					pGauge->themeName = xmlNodeGetContent(pGaugeNode);
					cd_debug("gauge : Nom du theme(%s)",pGauge->themeName);
				}
				if (xmlStrcmp (pGaugeNode->name, (const xmlChar *) "file") == 0)
				{
					ap = xmlHasProp(pGaugeNode, "key");
					if(strcmp(xmlNodeGetContent(ap->xmlChildrenNode),"background") == 0)
					{
						imagePath = g_strdup_printf("%s/%s", themePath,xmlNodeGetContent(pGaugeNode));
						pGauge->imageBackground = init_cd_GaugeImage(imagePath);
						draw_cd_GaugeImage(pSourceContext, pGauge->imageBackground, iWidth, iHeight);
					}
					else if(strcmp(xmlNodeGetContent(ap->xmlChildrenNode),"foreground") == 0)
					{
						imagePath = g_strdup_printf("%s/%s", themePath,xmlNodeGetContent(pGaugeNode));
						pGauge->imageForeground = init_cd_GaugeImage(imagePath);
						draw_cd_GaugeImage(pSourceContext, pGauge->imageForeground, iWidth, iHeight);
					}
				}
				if(xmlStrcmp (pGaugeNode->name, (const xmlChar *) "indicator") == 0)
				{
					GaugeIndicator *pGaugeIndicator = g_new0 (GaugeIndicator, 1);
					pGaugeIndicator->direction = 1;
					
					cd_debug("gauge : On charge un indicateur");
					xmlNodePtr pGaugeSubNode;
					for (pGaugeSubNode = pGaugeNode->xmlChildrenNode; pGaugeSubNode != NULL; pGaugeSubNode = pGaugeSubNode->next)
					{
						if(xmlStrcmp (pGaugeSubNode->name, (const xmlChar *) "posX") == 0)
						{
							pGaugeIndicator->posX = strtod(xmlNodeGetContent(pGaugeSubNode),NULL);
							cd_debug("gauge : posX(%s,%d)",xmlNodeGetContent(pGaugeSubNode),pGaugeIndicator->posX);
						}
						else if(xmlStrcmp (pGaugeSubNode->name, (const xmlChar *) "direction") == 0) pGaugeIndicator->direction = strtod(xmlNodeGetContent(pGaugeSubNode),NULL);
						else if(xmlStrcmp (pGaugeSubNode->name, (const xmlChar *) "posY") == 0) pGaugeIndicator->posY = strtod(xmlNodeGetContent(pGaugeSubNode),NULL);
						else if(xmlStrcmp (pGaugeSubNode->name, (const xmlChar *) "posStart") == 0) pGaugeIndicator->posStart = strtod(xmlNodeGetContent(pGaugeSubNode),NULL);
						else if(xmlStrcmp (pGaugeSubNode->name, (const xmlChar *) "posStop") == 0) pGaugeIndicator->posStop = strtod(xmlNodeGetContent(pGaugeSubNode),NULL);
						else if(xmlStrcmp (pGaugeSubNode->name, (const xmlChar *) "file") == 0)
						{
							cd_debug("gauge : On charge un fichier(%s)",xmlNodeGetContent(pGaugeSubNode));
							ap = xmlHasProp(pGaugeSubNode, "key");
							if(strcmp(xmlNodeGetContent(ap->xmlChildrenNode),"needle") == 0)
							{
								imagePath = g_strdup_printf("%s/%s", themePath,xmlNodeGetContent(pGaugeSubNode));
								pGaugeImage = init_cd_GaugeImage(imagePath);
								
								pGaugeIndicator->imageNeedle = g_list_append (pGaugeIndicator->imageNeedle, pGaugeImage);
							}
							if(strcmp(xmlNodeGetContent(ap->xmlChildrenNode),"image") == 0)
							{
								imagePath = g_strdup_printf("%s/%s", themePath,xmlNodeGetContent(pGaugeSubNode));
								pGaugeImage = init_cd_GaugeImage(imagePath);
								draw_cd_GaugeImage(pSourceContext, pGaugeImage, iWidth, iHeight);
								
								pGaugeIndicator->nbImage++;
								
								pGaugeIndicator->imageList = g_list_append (pGaugeIndicator->imageList, pGaugeImage);
							}
						}
					}
					pGauge->indicatorList = g_list_append(pGauge->indicatorList,pGaugeIndicator);
				}
			}
			xmlFreeDoc (pGaugeTheme);
		}
		else pGauge = NULL;
		
		xmlCleanupParser ();
		
		g_free(imagePath);
	}
	return pGauge;
}

GaugeImage *init_cd_GaugeImage(gchar *sImagePath)
{
	cd_debug("gauge : Image(%s)",sImagePath);
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

void make_cd_Gauge(cairo_t *pSourceContext, CairoContainer *pContainer, Icon *pIcon, Gauge *pGauge, double fValue)
{
	GList *pList = NULL;
	pList = g_list_prepend (pList, &fValue);
	
	make_cd_Gauge_multiValue(pSourceContext,pContainer,pIcon,pGauge,pList);
	
	g_list_free (pList);
}

void make_cd_Gauge_multiValue(cairo_t *pSourceContext, CairoContainer *pContainer, Icon *pIcon, Gauge *pGauge, GList *valueList)
{
	g_return_if_fail (pGauge != NULL && pContainer != NULL && pSourceContext != NULL);
	cd_debug("gauge : %s ()",__func__);
	
	double fMaxScale = 1 + g_fAmplitude;
	//fMaxScale = cairo_dock_get_max_scale (pContainer);
	GaugeImage *pGaugeImage;
	
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	
	//On affiche le fond
	if(pGauge->imageBackground != NULL)
	{
		pGaugeImage = pGauge->imageBackground;
		cairo_set_source_surface (pSourceContext, pGaugeImage->cairoSurface, 0.0f, 0.0f);
		cairo_paint (pSourceContext);
	}
	
	GList *pIndicatorElement;
	GList *pValueList;
	double *pValue;
	
	pIndicatorElement = pGauge->indicatorList;
	for(pValueList = valueList; pValueList != NULL; pValueList = pValueList->next)
	{
		pValue = pValueList->data;
		if(*pValue > 1) *pValue = 1;
		else if(*pValue < 0) *pValue = 0;
		draw_cd_Gauge_image(pSourceContext, pGauge, pIndicatorElement->data, *pValue);
		
		if(pIndicatorElement->next != NULL) pIndicatorElement = pIndicatorElement->next;
	}
	
	cairo_paint (pSourceContext);
	
	pIndicatorElement = pGauge->indicatorList;
	for(pValueList = valueList; pValueList != NULL; pValueList = pValueList->next)
	{
		pValue = pValueList->data;
		if(*pValue > 1) *pValue = 1;
		else if(*pValue < 0) *pValue = 0;
		
		draw_cd_Gauge_needle(pSourceContext, pGauge, pIndicatorElement->data, *pValue);
		
		if(pIndicatorElement->next != NULL) pIndicatorElement = pIndicatorElement->next;
	}
	
	//On affiche le fond
	if(pGauge->imageForeground != NULL)
	{
		pGaugeImage = pGauge->imageForeground;
		cairo_set_source_surface (pSourceContext, pGaugeImage->cairoSurface, 0.0f, 0.0f);
		cairo_paint (pSourceContext);
	}
	
	if (CAIRO_DOCK_IS_DOCK (pContainer) && CAIRO_DOCK (pContainer)->bUseReflect)
	{
		cairo_surface_t *pReflet = pIcon->pReflectionBuffer;
		pIcon->pReflectionBuffer = NULL;
		cairo_surface_destroy (pReflet);
		
		pIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (pIcon->pIconBuffer,
			pSourceContext,
			(pContainer->bIsHorizontal ? pIcon->fWidth : pIcon->fHeight) * fMaxScale,
			(pContainer->bIsHorizontal ? pIcon->fHeight : pIcon->fWidth) * fMaxScale,
			pContainer->bIsHorizontal,
			fMaxScale,
			pContainer->bDirectionUp);
	}
	
	cairo_dock_redraw_my_icon (pIcon, pContainer);
}

void draw_cd_Gauge_needle(cairo_t *pSourceContext, Gauge *pGauge, GaugeIndicator *pGaugeIndicator, double dValue)
{
	cd_debug("gauge : %s\n",__func__);
	
	if(pGaugeIndicator->imageNeedle != NULL)
	{
		GaugeImage *pGaugeImage;
		double fHalfX;
		double fHalfY;
		double physicValue = ((dValue * (pGaugeIndicator->posStop - pGaugeIndicator->posStart)) + pGaugeIndicator->posStart) / 360;

		int direction = pGaugeIndicator->direction >= 0 ? 1 : -1;
		cd_debug("gauge : Direction(%i)",direction);
		//On affiche l'aiguille
		GList *pElement;
		pElement = pGaugeIndicator->imageNeedle;
		pGaugeImage = pElement->data;
		
		fHalfX = pGauge->imageBackground->sizeX / 2.0f * (1 + pGaugeIndicator->posX);
		fHalfY = pGauge->imageBackground->sizeY / 2.0f * (1 - pGaugeIndicator->posY);
		
		cairo_save (pSourceContext);
		
		cairo_scale (pSourceContext,
			(double) pGauge->sizeX / (double) pGaugeImage->sizeX,
			(double) pGauge->sizeY / (double) pGaugeImage->sizeY);
		
		cairo_translate (pSourceContext, fHalfX, fHalfY);
		cairo_rotate (pSourceContext, -G_PI/2.0f + G_PI*physicValue*direction*2.0f);
		
		rsvg_handle_render_cairo (pGaugeImage->svgNeedle, pSourceContext);
		
		cairo_restore (pSourceContext);
	}
}

void draw_cd_Gauge_image(cairo_t *pSourceContext, Gauge *pGauge, GaugeIndicator *pGaugeIndicator, double dValue)
{
	cd_debug("gauge : %s\n",__func__);
	
	if(pGaugeIndicator->imageList != NULL)
	{
		GaugeImage *pGaugeImage;
		int trueImage;
		double imageWidthZone;
		
		//Equation donnant la bonne image.
		imageWidthZone = 1 / ((double) pGaugeIndicator->nbImage - 1);
		trueImage = imageWidthZone * (pGaugeIndicator->nbImage - 1) * (dValue * (pGaugeIndicator->nbImage - 1) + 0.5);
		cd_debug("gauge : La bonne image est : %d / %d (%d)",trueImage,pGaugeIndicator->nbImage,imageWidthZone);
		
		//On charge l'image correspondante à la valeur
		int i = 0;
		GList *pElement;
		for(pElement = pGaugeIndicator->imageList; pElement != NULL; pElement = pElement->next)
		{
			if(i > trueImage) break;
			else if(i == trueImage)
			{
				cd_debug("gauge : On a trouver l'image %d",i);
				pGaugeImage = pElement->data;
			}
			else pGaugeImage = NULL;
			i++;
		}
		
		if(pGaugeImage != NULL)
		{
			cairo_set_source_surface (pSourceContext, pGaugeImage->cairoSurface, 0.0f, 0.0f);
		}
	}
}

void free_cd_GaugeImage(GaugeImage *pGaugeImage)
{
	cd_debug("gauge : %s\n",__func__);
	
	if(pGaugeImage->svgNeedle != NULL)
	{
		rsvg_handle_free (pGaugeImage->svgNeedle);
	}
	if(pGaugeImage->cairoSurface != NULL)
	{
		cairo_surface_destroy (pGaugeImage->cairoSurface);
	}
	
	g_free (pGaugeImage);
}

void free_cd_GaugeIndicator(GaugeIndicator *pGaugeIndicator)
{
	cd_debug("gauge : %s\n",__func__);
	if (pGaugeIndicator == NULL)
		return ;
	
	GList *pElement;
	for (pElement = pGaugeIndicator->imageList; pElement != NULL; pElement = pElement->next)
	{
		free_cd_GaugeImage(pElement->data);
	}
	g_list_free (pGaugeIndicator->imageList);
	
	for (pElement = pGaugeIndicator->imageNeedle; pElement != NULL; pElement = pElement->next)
	{
		free_cd_GaugeImage(pElement->data);
	}
	g_list_free (pGaugeIndicator->imageNeedle);
	
	g_free (pGaugeIndicator);
}

void free_cd_Gauge(Gauge *pGauge)
{
	cd_debug("gauge : %s\n",__func__);
	
	if(pGauge != NULL)
	{
		if(pGauge->themeName != NULL) g_free(pGauge->themeName);
		
		if(pGauge->imageBackground != NULL) free_cd_GaugeImage(pGauge->imageBackground);
		if(pGauge->imageForeground != NULL) free_cd_GaugeImage(pGauge->imageForeground);
		
		GList *pElement;
		for (pElement = pGauge->indicatorList; pElement != NULL; pElement = pElement->next)
		{
			free_cd_GaugeIndicator(pElement->data);
		}
		g_list_free (pGauge->indicatorList);
		
		g_free (pGauge);
	}
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
