// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/spectra/view.h
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

#ifndef GSV_VIEW_H
#define GSV_VIEW_H

#include <gcugtk/spectrumview.h>


class gsvDocument;
class gsvWindow;

class gsvView: public gcugtk::SpectrumView
{
public:
	gsvView (gsvDocument *Doc);
	virtual ~gsvView ();

// Properties
GCU_PROP (gsvWindow*, Window);
};

#endif	//	GSV_VIEW_H
