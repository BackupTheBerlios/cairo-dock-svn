#ifndef __CAIRO_DOCK_H__
#define __CAIRO_DOCK_H__

/** /mainpage ceci est la mainpage
* /subsection ceci est une sous-section 1
* /subsection ceci est une sous-section 2
*/

#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <cairo-dock/cairo-dock-struct.h>
#include <cairo-dock/cairo-dock-global-variables.h>
// structures de base.
#include <cairo-dock/cairo-dock-icons.h>
#include <cairo-dock/cairo-dock-modules.h>
#include <cairo-dock/cairo-dock-flying-container.h>
#include <cairo-dock/cairo-dock-dialogs.h>
#include <cairo-dock/cairo-dock-desklet.h>
#include <cairo-dock/cairo-dock-dock-factory.h>
#include <cairo-dock/cairo-dock-container.h>
// actions et comportement des containers.
#include <cairo-dock/cairo-dock-draw-opengl.h>
#include <cairo-dock/cairo-dock-draw.h>
#include <cairo-dock/cairo-dock-dock-facility.h>
#include <cairo-dock/cairo-dock-animations.h>
#include <cairo-dock/cairo-dock-callbacks.h>
#include <cairo-dock/cairo-dock-menu.h>
// chargeurs d'icone.
#include <cairo-dock/cairo-dock-application-factory.h>
#include <cairo-dock/cairo-dock-applet-factory.h>
#include <cairo-dock/cairo-dock-launcher-factory.h>
#include <cairo-dock/cairo-dock-separator-factory.h>
#include <cairo-dock/cairo-dock-desktop-file-factory.h>
#include <cairo-dock/cairo-dock-load.h>
#include <cairo-dock/cairo-dock-config.h>
// managers.
#include <cairo-dock/cairo-dock-applications-manager.h>
#include <cairo-dock/cairo-dock-class-manager.h>
#include <cairo-dock/cairo-dock-dock-manager.h>
#include <cairo-dock/cairo-dock-renderer-manager.h>
#include <cairo-dock/cairo-dock-file-manager.h>
#include <cairo-dock/cairo-dock-emblem.h>
// GUI.
#include <cairo-dock/cairo-dock-themes-manager.h>
#include <cairo-dock/cairo-dock-gui-manager.h>
#include <cairo-dock/cairo-dock-gui-factory.h>
#include <cairo-dock/cairo-dock-gui-filter.h>
#include <cairo-dock/cairo-dock-gui-callbacks.h>
// donnees et modules internes.
#include <cairo-dock/cairo-dock-internal-position.h>
#include <cairo-dock/cairo-dock-internal-accessibility.h>
#include <cairo-dock/cairo-dock-internal-system.h>
#include <cairo-dock/cairo-dock-internal-taskbar.h>
#include <cairo-dock/cairo-dock-internal-dialogs.h>
#include <cairo-dock/cairo-dock-internal-indicators.h>
#include <cairo-dock/cairo-dock-internal-views.h>
#include <cairo-dock/cairo-dock-internal-desklets.h>
#include <cairo-dock/cairo-dock-internal-labels.h>
#include <cairo-dock/cairo-dock-internal-background.h>
#include <cairo-dock/cairo-dock-internal-icons.h>
// architecture pour les applets.
#include <cairo-dock/cairo-dock-applet-facility.h>
#include <cairo-dock/cairo-dock-applet-canvas.h>
// classes presque independantes de CD.
#include <cairo-dock/cairo-dock-data-renderer.h>
#include <cairo-dock/cairo-dock-graph.h>
#include <cairo-dock/cairo-dock-gauge.h>
#include <cairo-dock/cairo-dock-notifications.h>
#include <cairo-dock/cairo-dock-surface-factory.h>
#include <cairo-dock/cairo-dock-X-utilities.h>
// classes independantes de CD.
#include <cairo-dock/cairo-dock-log.h>
#include <cairo-dock/cairo-dock-dbus.h>
#include <cairo-dock/cairo-dock-keyfile-utilities.h>
#include <cairo-dock/cairo-dock-keybinder.h>
#include <cairo-dock/cairo-dock-task.h>
#include <cairo-dock/cairo-dock-particle-system.h>

#endif
