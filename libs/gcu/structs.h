// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/structs.h
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

#ifndef GCU_STRUCTS_H
#define GCU_STRUCTS_H

/*!\file*/
namespace gcu
{

/*!\enum ContentType
The list of supported data types.
*/
typedef enum {
/*!
Unknown content.
*/
	ContentTypeUnknown,
/*!
3D molecular structure.
*/
	ContentType3D,
/*!
2D molecular structure.
*/
	ContentType2D,
/*!
Crystal structure.
*/
	ContentTypeCrystal,
/*!
Spectral data.
*/
	ContentTypeSpectrum,
/*!
Miscalleneous contents. Might include anything.
*/
	ContentTypeMisc
} ContentType;

}   //  namespace gcu

#endif  //  GCU_STRUCTS_H
