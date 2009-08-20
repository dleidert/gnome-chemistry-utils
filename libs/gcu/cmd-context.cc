/* 
 * Gnome Chemistry Utils
 * cmd-context.cc
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "cmd-context.h"
#ifndef GOFFICE_HAS_GLOBAL_HEADER
#   include <goffice/app/error-info.h>
#endif
#include <gcu/goffice-compat.h>
#include <gsf/gsf-impl-utils.h>
#include <stdio.h>

#ifndef GO_TYPE_CMD_CONTEXT
#   define GO_TYPE_CMD_CONTEXT GO_CMD_CONTEXT_TYPE
#endif

typedef	GObject GcuCmdContext;
typedef GObjectClass GcuCmdContextClass;

static void
gcu_cc_error_error (GOCmdContext *cc, GError *error)
{
	fprintf (stderr, "Error: %s\n", error->message);
}

static void
gcu_cc_error_info (GOCmdContext *cc, ErrorInfo *error)
{
	error_info_print (error);
}

static char *
gcu_cc_get_password (G_GNUC_UNUSED GOCmdContext *cc,
		  G_GNUC_UNUSED char const* filename)
{
	return NULL;
}

static void
gcu_cc_set_sensitive (G_GNUC_UNUSED GOCmdContext *cc,
		   G_GNUC_UNUSED gboolean sensitive)
{
}

static void
gcu_cc_progress_set (GOCmdContext *cc, gfloat val)
{
}

static void
gcu_cc_progress_message_set (GOCmdContext *cc, gchar const *msg)
{
}

static void
gcu_cc_cmd_context_init (GOCmdContextClass *iface)
{
	iface->get_password			= gcu_cc_get_password;
	iface->set_sensitive		= gcu_cc_set_sensitive;
	iface->error.error			= gcu_cc_error_error;
	iface->error.error_info		= gcu_cc_error_info;
	iface->progress_set			= gcu_cc_progress_set;
	iface->progress_message_set	= gcu_cc_progress_message_set;
}

GSF_CLASS_FULL (GcuCmdContext, gcu_cmd_context,
		NULL, NULL, NULL, NULL,
		NULL, G_TYPE_OBJECT, 0,
		GSF_INTERFACE (gcu_cc_cmd_context_init, GO_TYPE_CMD_CONTEXT))

static GOCmdContext *cc = NULL;

GOCmdContext *
gcu_get_cmd_context (void)
{
	if (!cc)
		cc = GO_CMD_CONTEXT (g_object_new (GCU_CMD_CONTEXT_TYPE, NULL));
	return cc;
}
