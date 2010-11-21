// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/spectra/document.cc
 *
 * Copyright (C) 2007-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "application.h"
#include "view.h"
#include "window.h"
#include <glib/gutils.h>

using namespace gcu;
using namespace std;

gsvDocument::gsvDocument (gsvApplication *App): SpectrumDocument (App, new gsvView (this))
{
}

gsvDocument::~gsvDocument ()
{
}

void gsvDocument::SetTitle (char const *title)
{
	gcu::SpectrumDocument::SetTitle (title);
	gsvView *view = dynamic_cast <gsvView *> (m_View);
	if (view && view->GetWindow ())
		view->GetWindow ()->SetTitle (title);
}
