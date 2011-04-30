// -*- C++ -*-

/* 
 * GChemPaint
 * standaloneapp.cc
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "standaloneapp.h"
#include "standalonewin.h"
#include <gcp/view.h>
#include <glib/gi18n-lib.h>

gcpStandaloneApp::gcpStandaloneApp(): gcp::Application()
{
}

gcpStandaloneApp::~gcpStandaloneApp()
{
	while (!m_Docs.empty())
		delete *m_Docs.begin();
}

void gcpStandaloneApp::OnFileNew(char const *Theme)
{
	gchar tmp[32];
	if (m_pActiveDoc && !m_pActiveDoc->GetView ()->PrepareUnselect ())
		return;
	g_snprintf (tmp, sizeof (tmp), _("Untitled %d"), m_NumWindow++);
	gcp::Window *win = new gcpStandaloneWindow (this, Theme);
	win->GetDocument ()->SetLabel (tmp);
	win->GetDocument ()->SetUseAtomColors (GetUseAtomColors ());
}

GtkWindow* gcpStandaloneApp::GetWindow()
{
	return (m_pActiveTarget)? m_pActiveTarget->GetWindow (): NULL;
}

void gcpStandaloneApp::NoMoreDocsEvent ()
{
	gtk_main_quit ();
}

/* code copied from AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
 */
extern void signalWrapper (int);
void gcpStandaloneApp::CatchSignals (G_GNUC_UNUSED int sig_num)
{
	static int s_signal_count = 0;

	// Reset the signal handler 
	// (not that it matters - this is mostly for race conditions)
	signal ( SIGSEGV, signalWrapper);

	s_signal_count = s_signal_count + 1;
	if (s_signal_count > 1) // new crash during emergency file save
   		abort();

	std::set <gcu::Document*>::iterator it, end = m_Docs.end ();
	static unsigned docnum;
	for (it = m_Docs.begin (); it != end; it++) {
		gcp::Document *doc = static_cast <gcp::Document *> (*it);
		if (!doc->GetDirty ())
			continue;
		char *uri = (doc->GetFileName ())? g_strdup_printf ("%s.saved", doc->GetFileName ()): g_strdup_printf ("%s/unnamed%u.gchempaint.saved", GetCurDir (), docnum++);
		doc->SetFileName (uri, "application/x-gchempaint"); // always save in gchempaint format
		doc->Save ();
	}

	// Abort and dump core
	abort();
}