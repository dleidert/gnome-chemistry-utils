// -*- C++ -*-

/* 
 * GChemPaint library
 * target.cc 
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
#include "target.h"
#include "application.h"
#include "document.h"
#include "view.h"

namespace gcp {

static bool on_focus_in (GtkWidget *widget, GdkEventFocus *event, Target *target)
{
	target->GetDocument ()->GetView ()->ShowCursor (true);
	gcp::Application *App = target->GetApplication ();
	App->NotifyFocus (true, target);
	gtk_clipboard_request_contents (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), gdk_atom_intern ("TARGETS", FALSE),  (GtkClipboardReceivedFunc) gcp::on_receive_targets, App);
	return true;
}

static bool on_focus_out (GtkWidget *widget, GdkEventFocus *event, Target *target)
{
	target->GetDocument ()->GetView ()->ShowCursor (false);
	target->GetApplication ()->NotifyFocus (false);
	return true;
}

static bool on_state (GtkWidget *widget, GdkEventWindowState *event, Target *target)
{
	if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
		target->GetApplication ()->NotifyIconification (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED);
	return true;
}

Target::Target (Application *App):
	m_Application (App)
{
	if (m_Application)
		m_Application->AddTarget (this);
}

Target::~Target ()
{
	if (m_Application)
		m_Application->DeleteTarget (this);
}

void Target::SetWindow (GtkWindow *window)
{
	m_Window = window;
	g_signal_connect (G_OBJECT (m_Window), "focus_in_event", G_CALLBACK (on_focus_in), this);
	g_signal_connect (G_OBJECT (m_Window), "focus_out_event", G_CALLBACK (on_focus_out), this);
	g_signal_connect (G_OBJECT (m_Window), "window-state-event", G_CALLBACK (on_state), this);
}

}	//	namespace gcp
