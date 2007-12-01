
#ifndef __CAIRO_DOCK_SURFACE_FACTORY__
#define  __CAIRO_DOCK_SURFACE_FACTORY__

#include <glib.h>
#include <gdk/gdk.h>
#include <cairo.h>


void cairo_dock_calculate_contrainted_icon_size (double *fImageWidth, double *fImageHeight, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fIconWidthSaturationFactor, double *fIconHeightSaturationFactor);

cairo_surface_t *cairo_dock_create_surface_from_xicon_buffer (gulong *pXIconBuffer, int iBufferNbElements, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);

cairo_surface_t *cairo_dock_create_surface_from_pixbuf (GdkPixbuf *pixbuf, cairo_t *pSourceContext, double fMaxScale, gboolean bConstraintSize, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fImageWidth, double *fImageHeight);

cairo_surface_t *cairo_dock_create_surface_from_image (gchar *cImagePath, cairo_t* pSourceContext, double fMaxScale, int iMinIconAuthorizedWidth, int iMinIconAuthorizedHeight, int iMaxIconAuthorizedWidth, int iMaxIconAuthorizedHeight, double *fImageWidth, double *fImageHeight, double fRotationAngle, double fAlpha, gboolean bReapeatAsPattern);
cairo_surface_t *cairo_dock_create_surface_for_icon (gchar *cImagePath, cairo_t* pSourceContext, double fImageWidth, double fImageHeight);
#define cairo_dock_create_surface_for_square_icon(cImagePath, pSourceContext, fImageSize) cairo_dock_create_surface_for_icon (cImagePath, pSourceContext, fImageSize, fImageSize)



cairo_surface_t * cairo_dock_rotate_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, double fRotationAngle);

cairo_surface_t * cairo_dock_create_reflection_surface (cairo_surface_t *pSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, gboolean b_HorizontalDock);

cairo_surface_t * cairo_dock_create_icon_surface_with_reflection (cairo_surface_t *pIconSurface, cairo_surface_t *pReflectionSurface, cairo_t *pSourceContext, double fImageWidth, double fImageHeight, gboolean bHorizontalDock);

cairo_surface_t *cairo_dock_create_surface_from_text (gchar *cText, cairo_t* pSourceContext, int iLabelSize, gchar *cLabelPolice, int iLabelWeight, double fBackgroundAlpha, double fMaxScale, int *iTextWidth, int *iTextHeight, double *fTextXOffset, double *fTextYOffset);


#endif
