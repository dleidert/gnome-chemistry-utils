// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/ui-builder.h 
 *
 * Copyright (C) 2008-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_UI_BUILDER_H
#define GCU_UI_BUILDER_H

#include <gtk/gtk.h>
#include <gcu/macros.h>
#include <stdexcept>

/*!\file*/
namespace gcu
{

class UIBuilder
{
public:
/*!
@param filename: the name of the ui file which contains the description of
the widgets.
@param domain: the translation domain.
*/
	UIBuilder (char const *filename, char const *domain) throw (std::runtime_error);
	virtual ~UIBuilder ();

	GtkWidget *GetWidget (char const *wname);
	GtkWidget *GetRefdWidget (char const *wname);
	GtkComboBox *GetComboBox (char const *cbname);

GCU_RO_PROP (GtkBuilder *, Builder)
};

}   //  namespace gcu

#endif  //  GCU_UI_BUILDER_H