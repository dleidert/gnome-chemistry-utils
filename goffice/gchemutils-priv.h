/*
 * GChemUtils GOffice component
 * gchempaint.h
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GOFFICE_GCHEMUTILS_PRIV_H
#define GOFFICE_GCHEMUTILS_PRIV_H

#include "gchemutils.h"
#include <gcu/chem3ddoc.h>
#include <gcu/structs.h>
#include <gogcuapp.h>
#include <gcu/document.h>
#include <gcugtk/window.h>

struct _GOGChemUtilsComponent
{
	GOComponent parent;

	GOGcuApplication *application;
	gcu::Document *document;
	gcugtk::Window *window; // TODO use a gcu::Window.
	gcu::ContentType type;
	gcu::Display3DMode mode;
	double psi, theta, phi; // only used by the chem3d code
	char *data; // only used by the chem3d code
	size_t length; // only used by the chem3d code
};


#endif	// GOFFICE_GCHEMPAINT_PRIV_H
