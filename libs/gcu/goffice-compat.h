/* 
 * Gnome Chemistry Utils
 * gcu/goffice-compat.h
 * definitions helping with goffice API changes
 *
 * Copyright (C) 2009 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifdef HAVE_GO_ERROR_INFO_FREE
#	define IOContext GOIOContext
#	define gnumeric_io_context_new go_io_context_new
#	define ErrorInfo GOErrorInfo
#	define error_info_new_printf go_error_info_new_printf
#	define error_info_peek_message go_error_info_peek_message
#	define error_info_print go_error_info_print
#	define gnm_io_warning go_io_warning
#	define PluginServiceSimple GOPluginServiceSimple
#	define PluginServiceSimpleClass GOPluginServiceSimpleClass
#	define plugin_service_define go_plugin_service_define
#	define plugin_service_load go_plugin_service_load
#endif