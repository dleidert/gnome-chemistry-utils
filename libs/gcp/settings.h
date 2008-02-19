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

#define ROOTDIR "/apps/gchemutils/paint/settings/"
#define GCP_CONF_DIR_SETTINGS "paint/settings"

namespace gcp {

extern const gchar *Color, *DeleteColor, *AddColor, *SelectColor;
extern unsigned MaxStackSize;
extern bool MergeAtoms;
extern int CompressionLevel;
extern bool TearableMendeleiev;

// Clipboard formats identifiers
enum {
	GCP_CLIPBOARD_NATIVE,
	GCP_CLIPBOARD_SVG,
	GCP_CLIPBOARD_SVG_XML,
	GCP_CLIPBOARD_PNG,
	GCP_CLIPBOARD_JPEG,
	GCP_CLIPBOARD_BMP,
	GCP_CLIPBOARD_NO_TEXT,
	GCP_CLIPBOARD_UTF8_STRING = GCP_CLIPBOARD_NO_TEXT,
	GCP_CLIPBOARD_STRING,
	GCP_CLIPBOARD_ALL,
};

extern int ClipboardFormats; /* number of really used formats when copying
should be either GCP_CLIPBOARD_NO_TEXT or GCP_CLIPBOARD_ALL */

}	//	namespace gcp

#endif // GCHEMPAINT_SETTINGS_H
