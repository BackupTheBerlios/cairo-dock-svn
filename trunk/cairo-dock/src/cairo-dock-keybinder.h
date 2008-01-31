/*
** cairo-dock-keybinder.h
** Login : <ctaf42@localhost.localdomain>
** Started on  Thu Jan 31 03:57:17 2008 Cedric GESTES
** $Id$
**
** Copyright (C) 2008 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* imported from tomboy_key_binder.h
*/


#ifndef __CD_KEY_BINDER_H__
#define __CD_KEY_BINDER_H__

#include <glib/gtypes.h>

G_BEGIN_DECLS

typedef void (* CDBindkeyHandler) (const char *keystring, gpointer user_data);

void cd_keybinder_init (void);

gboolean cd_keybinder_bind (const char           *keystring,
                            CDBindkeyHandler  handler,
                            gpointer              user_data);

void cd_keybinder_unbind   (const char           *keystring,
                            CDBindkeyHandler  handler);

gboolean cd_keybinder_is_modifier (guint keycode);

guint32 cd_keybinder_get_current_event_time (void);

G_END_DECLS

#endif /* __CD_KEY_BINDER_H__ */

