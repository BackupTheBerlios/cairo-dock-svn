/*
** cairo-dock-log.h
** Login : <ctaf42@gmail.Com>
** Started on  Sat Feb  9 16:11:48 2008 Cedric GESTES
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

#ifndef   	CAIRO_DOCK_LOG_H_
# define   	CAIRO_DOCK_LOG_H_

# ifndef _INSIDE_CAIRO_DOCK_LOG_C_
  extern GLogLevelFlags gLogLevel;
# endif

/*
 * internal function
 */
void cd_log_location(const char *file, const char *func, const int line);

/**
 * initialise the log system
 */
void cd_log_init();

/**
 * set the verbosity level
 */
void cd_log_set_level(GLogLevelFlags loglevel);

#define cd_error(str, ...) cd_log_location(__FILE__, __PRETTY_FUNCTION__, __LINE__); g_error(str, __VA_ARGS__)
#define cd_critical(str, ...) cd_log_location(__FILE__, __PRETTY_FUNCTION__, __LINE__); g_critical(str, __VA_ARGS__)
#define cd_warning(str, ...) cd_log_location(__FILE__, __PRETTY_FUNCTION__, __LINE__); g_warning(str, __VA_ARGS__)
#define cd_message(str, ...) cd_log_location(__FILE__, __PRETTY_FUNCTION__, __LINE__); g_message(str, __VA_ARGS__)
#define cd_info(str, ...) cd_log_location(__FILE__, __PRETTY_FUNCTION__, __LINE__); g_info(str, __VA_ARGS__)
#define cd_debug(str, ...) cd_log_location(__FILE__, __PRETTY_FUNCTION__, __LINE__); g_debug(str, __VA_ARGS__)

#endif 	    /* !CAIRO_DOCK_LOG_H_ */
