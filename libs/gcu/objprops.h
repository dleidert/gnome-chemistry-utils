// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/loader.h
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

#ifndef GCU_OBJ_PROPS_H
#define GCU_OBJ_PROPS_H

enum {
	// Common properties
	GCU_PROP_ID,
	GCU_PROP_POS2D,
	// Document properties
	GCU_PROP_DOC_TITLE,
	GCU_PROP_DOC_COMMENT,
	GCU_PROP_DOC_CREATOR,
	GCU_PROP_DOC_CREATION_TIME,
	GCU_PROP_DOC_MODIFICATION_TIME,
	// Theme related properties (might be doc properties in some formats)
	GCU_PROP_THEME_BOND_LENGTH,
	// Atom properties
	GCU_PROP_ATOM_SYMBOL,
	GCU_PROP_ATOM_Z,
	// Bond properties
	GCU_PROP_BOND_BEGIN,
	GCU_PROP_BOND_END,
	GCU_PROP_BOND_ORDER,
	GCU_PROP_BOND_TYPE, //normal, hash, wedge,...
	// Text properties
	GCU_PROP_TEXT_TEXT,
};

#endif	//	GCU_OBJ_PROPS_H
