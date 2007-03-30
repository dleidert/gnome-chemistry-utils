// -*- C++ -*-

/* 
 * GChemPaint
 * standaloneapp.cc
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
}

GtkWindow* gcpStandaloneApp::GetWindow()
{
	return m_pActiveWin->GetWindow ();
}

void gcpStandaloneApp::NoMoreDocsEvent ()
{
	gtk_main_quit ();
}
