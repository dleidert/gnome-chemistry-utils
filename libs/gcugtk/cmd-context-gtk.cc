/* 
 * Gnome Chemistry Utils
 * cmd-context-gtk.cc
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "cmd-context-gtk.h"
#include "message.h"
#include <gsf/gsf-impl-utils.h>
#include <stdio.h>

typedef	struct {
	GObject base;
	gcugtk::CmdContextGtk *ccg;
} GcuCmdContextGtk;

typedef GObjectClass GcuCmdContextGtkClass;

#define GCU_TYPE_CMD_CONTEXT_GTK		(gcu_cmd_context_gtk_get_type ())
#define GCU_CMD_CONTEXT_GTK(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCU_TYPE_CMD_CONTEXT_GTK, GcuCmdContextGtk))
GType		gcu_cmd_context_gtk_get_type   (void);

namespace gcugtk {

CmdContextGtk::CmdContextGtk (Application *App): gcu::CmdContext (App)
{
	m_GOCmdContext = GO_CMD_CONTEXT (g_object_new (GCU_TYPE_CMD_CONTEXT_GTK, NULL));
	reinterpret_cast <GcuCmdContextGtk *> (m_GOCmdContext)->ccg = this;
}

CmdContextGtk::~CmdContextGtk ()
{
}

gcu::CmdContext::Response CmdContextGtk::GetResponse (char const *message, int responses)
{
	int buttons = 0;
	if (responses & ResponseOK)
		buttons |= GTK_BUTTONS_OK;
	if (responses & ResponseCancel)
		buttons |= GTK_BUTTONS_CANCEL;
	if ((responses & (ResponseYes | ResponseNo)) == (ResponseYes | ResponseNo))
		buttons |= GTK_BUTTONS_YES_NO;
	if (responses & ResponseClose)
		buttons |= GTK_BUTTONS_CLOSE;
	gcugtk::Message *Box = new gcugtk::Message (static_cast < Application * > (m_App), message, GTK_MESSAGE_QUESTION, static_cast < GtkButtonsType > (buttons), static_cast < Application * > (m_App)->GetWindow (), true);
	buttons = Box->Run ();
	switch (buttons) {
	case GTK_RESPONSE_OK:
		return ResponseOK;
	case GTK_RESPONSE_CANCEL:
		return ResponseCancel;
	case GTK_RESPONSE_YES:
		return ResponseYes;
	case GTK_RESPONSE_NO:
		return ResponseNo;
	case GTK_RESPONSE_CLOSE:
		return ResponseClose;
	default:
		return ResponseDefault;
	}
}

void CmdContextGtk::Message (char const *message, Severity severity, bool modal)
{
	GtkMessageType type;
	switch (severity) {
	case SeverityMessage:
		type = GTK_MESSAGE_INFO;
		break;
	case SeverityWarning:
		type = GTK_MESSAGE_WARNING;
		break;
	case SeverityError:
		type = GTK_MESSAGE_ERROR;
		break;
	default:
		type = GTK_MESSAGE_OTHER;
		break;
	}
	gcugtk::Message *Box = new gcugtk::Message (static_cast < Application * > (m_App), message, type, GTK_BUTTONS_CLOSE, static_cast < Application * > (m_App)->GetWindow ());
	if (modal)
		Box->Run ();
	else
		Box->Show ();
}

}	//	namespace gcu

static void
gcu_cc_gtk_error_error (GOCmdContext *cc, GError *error)
{
	GcuCmdContextGtk *ccg = GCU_CMD_CONTEXT_GTK (cc);
	char *mess = g_strdup_printf ("Error: %s\n", error->message);
	ccg->ccg->Message (mess, gcu::CmdContext::SeverityError, false);
	g_free (mess);
}

static void
gcu_cc_gtk_error_info (G_GNUC_UNUSED GOCmdContext *cc, GOErrorInfo *error)
{
	go_error_info_print (error);
}

static char *
gcu_cc_gtk_get_password (G_GNUC_UNUSED GOCmdContext *cc,
		  G_GNUC_UNUSED char const* filename)
{
	return NULL;
}

static void
gcu_cc_gtk_set_sensitive (G_GNUC_UNUSED GOCmdContext *cc,
		   G_GNUC_UNUSED gboolean sensitive)
{
}

static void
gcu_cc_gtk_progress_set (G_GNUC_UNUSED GOCmdContext *cc, G_GNUC_UNUSED double val)
{
}

static void
gcu_cc_gtk_progress_message_set (G_GNUC_UNUSED GOCmdContext *cc, G_GNUC_UNUSED gchar const *msg)
{
}

static void
gcu_cc_gtk_cmd_context_init (G_GNUC_UNUSED GOCmdContextClass *iface)
{
	iface->get_password			= gcu_cc_gtk_get_password;
	iface->set_sensitive		= gcu_cc_gtk_set_sensitive;
	iface->error.error			= gcu_cc_gtk_error_error;
	iface->error.error_info		= gcu_cc_gtk_error_info;
	iface->progress_set			= gcu_cc_gtk_progress_set;
	iface->progress_message_set	= gcu_cc_gtk_progress_message_set;
}

GSF_CLASS_FULL (GcuCmdContextGtk, gcu_cmd_context_gtk,
		NULL, NULL, NULL, NULL,
		NULL, G_TYPE_OBJECT, 0,
		GSF_INTERFACE (gcu_cc_gtk_cmd_context_init, GO_TYPE_CMD_CONTEXT))
