// -*- C++ -*-

/* 
 * GChemPaint library
 * settings.cc 
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

#include "config.h"
#include "settings.h"

namespace gcp {
	
GOColor Color = GO_RGBA_BLACK;
GOColor DeleteColor = GO_RGBA_RED;
GOColor AddColor = GO_RGBA_GREEN;
GOColor SelectColor = GO_RGBA_CYAN;
unsigned MaxStackSize = 0;//infinite size authorized for undo:redo stacks
bool MergeAtoms = true;
int CompressionLevel = 0;
bool TearableMendeleiev = false;
int ClipboardFormats = GCP_CLIPBOARD_NO_TEXT;

}
