// -*- C++ -*-

/* 
 * GChemPaint libray
 * settings.h 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_SETTINGS_H
#define GCHEMPAINT_SETTINGS_H

#include <glib.h>
#include <pango/pango.h>

#define ROOTDIR "/apps/gchempaint/settings/"

namespace gcp {

extern gchar *Color, *DeleteColor, *AddColor, *SelectColor;
extern unsigned MaxStackSize;
extern bool MergeAtoms;
extern int CompressionLevel;
extern bool TearableMendeleiev;

}	//	namespace gcp

#endif // GCHEMPAINT_SETTINGS_H
