/*
 * GChemUtils GOffice component
 * gchemutils.h
 *
 * Copyright (C) 2005-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GOFFICE_GCHEMPAINT_H
#define GOFFICE_GCHEMPAINT_H

#include <goffice/component/goffice-component.h>
#include <goffice/component/go-component.h>

extern "C"
{

typedef struct _GOGChemUtilsComponent GOGChemUtilsComponent;

typedef GOComponentClass GOGChemUtilsComponentClass;

#define GO_TYPE_GCHEMUTILS_COMPONENT	(go_gchemutils_component_get_type ())
#define GO_GCHEMUTILS_COMPONENT(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GO_TYPE_GCHEMUTILS_COMPONENT, GOGChemUtilsComponent))
#define GO_IS_GCHEMUTILS_COMPONENT(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), GO_TYPE_GCHEMUTILS_COMPONENT))

GType go_gchemutils_component_get_type (void);

}
#endif	// GOFFICE_GCHEMPAINT_H
