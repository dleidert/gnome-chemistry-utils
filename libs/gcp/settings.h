// -*- C++ -*-

/* 
 * GChemPaint libray
 * settings.h 
 *
 * Copyright (C) 2001-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <goffice/goffice.h>

#define ROOTDIR "/apps/gchemutils/paint/settings/"
#define GCP_CONF_DIR_SETTINGS "paint/settings"

/*!\file*/
namespace gcp {

/*!
The color used for normal drawing.
*/
extern GOColor Color;
/*!
The color used for objects that are to be deleted.
*/
extern GOColor DeleteColor;
/*!
The color used for new objects.
*/
extern GOColor AddColor;
/*!
The color used for selected objects.
*/
extern GOColor SelectColor;
/*!
The stack size for undo/redo operations. Default is no limit.
*/
extern unsigned MaxStackSize;
/*!
Whether to use existing atoms or create new one at the same place when
adding bonds.
*/
extern bool MergeAtoms;
/*!
XML file compression level.
*/
extern int CompressionLevel;
/*!
Whether the mendeleiv table widget might be detached from the tool box or not.
*/
extern bool TearableMendeleiev;

/*!
Clipboard formats identifiers
*/
enum {
/*!
GChemPaint native xml data
*/
	GCP_CLIPBOARD_NATIVE,
/*!
SVG image format.
*/
	GCP_CLIPBOARD_SVG,
/*!
SVG+XML image format (actually equivalent to GCP_CLIPBOARD_SVG).
*/
	GCP_CLIPBOARD_SVG_XML,
/*!
Encapsulated Postscript (not really used).
*/
	GCP_CLIPBOARD_EPS,
/*!
PNG image format.
*/
	GCP_CLIPBOARD_PNG,
/*!
JPEG image format.
*/
	GCP_CLIPBOARD_JPEG,
/*!
BMP image format.
*/
	GCP_CLIPBOARD_BMP,
/*!
Number of supported formats, excluding string formats which are used
only for debugging purposes.
*/
	GCP_CLIPBOARD_NO_TEXT,
/*!
UTF8 string.
*/
	GCP_CLIPBOARD_UTF8_STRING = GCP_CLIPBOARD_NO_TEXT,
/*!
ASCII string.
*/
	GCP_CLIPBOARD_STRING,
/*!
Number of supported formats, including string formats
*/
	GCP_CLIPBOARD_ALL,
};

/*!
Number of really used clipboard formats when copying
should be either GCP_CLIPBOARD_NO_TEXT or GCP_CLIPBOARD_ALL
*/
extern int ClipboardFormats;

}	//	namespace gcp

#endif // GCHEMPAINT_SETTINGS_H
