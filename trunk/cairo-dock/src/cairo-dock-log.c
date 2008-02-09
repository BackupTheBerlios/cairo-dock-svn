/*
** cairo-dock-log.c
** Login : <ctaf42@gmail.com>
** Started on  Sat Feb  9 15:54:57 2008 Cedric GESTES
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
*/


#include <stdio.h>
#include <stdarg.h>
#include <glib.h>

#define _INSIDE_CAIRO_DOCK_LOG_C_

#include "cairo-dock-log.h"

static GLogLevelFlags gLogLevel = 0;

void cd_log_location(const char *file, const char *func, const int line)
{
  printf("%s:%s:%d", file, func, line);
}

const char*_cd_log_level_to_string(const GLogLevelFlags loglevel)
{
  switch(loglevel)
  {
  case G_LOG_LEVEL_CRITICAL:
    return "CRITICAL: ";
  case G_LOG_LEVEL_ERROR:
    return "ERROR   : ";
  case G_LOG_LEVEL_WARNING:
    return "warning : ";
  case G_LOG_LEVEL_MESSAGE:
    return "message : ";
  case G_LOG_LEVEL_INFO:
    return "info    : ";
  case G_LOG_LEVEL_DEBUG:
    return "debug   : ";
  }
  return "";
}

static void cairo_dock_log_handler(const gchar *log_domain,
                                   GLogLevelFlags log_level,
                                   const gchar *message,
                                   gpointer user_data)
{
  if (log_level < gLogLevel)
    printf("%s%s", _cd_log_level_to_string(log_level), message);
}

void cd_log_init()
{
  g_log_set_default_handler(cairo_dock_log_handler, NULL);
}

void cd_log_set_level(GLogLevelFlags loglevel)
{
  gLogLevel = loglevel;
}
