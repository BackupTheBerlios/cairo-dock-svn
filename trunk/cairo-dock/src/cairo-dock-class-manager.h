
#ifndef __CAIRO_DOCK_CLASS_MANAGER__
#define  __CAIRO_DOCK_CLASS_MANAGER__

#include <X11/Xlib.h>

#include "cairo-dock-struct.h"


void cairo_dock_initialize_class_manager (void);

void cairo_dock_free_class_appli (CairoDockClassAppli *pClassAppli);
const GList *cairo_dock_list_existing_appli_with_class (const gchar *cClass);

gboolean cairo_dock_add_appli_to_class (Icon *pIcon);
gboolean cairo_dock_remove_appli_from_class (Icon *pIcon);
gboolean cairo_dock_set_class_use_xicon (const gchar *cClass, gboolean bUseXIcon);
gboolean cairo_dock_inhibate_class (const gchar *cClass, Icon *pInhibatorIcon);

gboolean cairo_dock_class_is_inhibated (const gchar *cClass);
gboolean cairo_dock_class_is_using_xicon (const gchar *cClass);
gboolean cairo_dock_prevent_inhibated_class (Icon *pIcon);

gboolean cairo_dock_remove_icon_from_class (Icon *pInhibatorIcon);
void cairo_dock_deinhibate_class (const gchar *cClass, Icon *pInhibatorIcon);
void cairo_dock_update_Xid_on_inhibators (Window Xid, const gchar *cClass);
void cairo_dock_remove_all_applis_from_class_table (void);
void cairo_dock_reset_class_table (void);

cairo_surface_t *cairo_dock_create_surface_from_class (gchar *cClass, cairo_t *pSourceContext, double fMaxScale, double *fWidth, double *fHeight);

void cairo_dock_update_visibility_on_inhibators (gchar *cClass, Window Xid, gboolean bIsHidden);
void cairo_dock_update_activity_on_inhibators (gchar *cClass, Window Xid);


#endif
