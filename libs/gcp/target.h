// -*- C++ -*-

/* 
 * GChemPaint library
 * target.h 
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

#ifndef GCP_TARGET_H
#define GCP_TARGET_H

#include <gcu/macros.h>
#include <gtk/gtkwindow.h>

namespace gcp {

class Application;
class Document;

class Target
{
public:
	Target (Application *App);
	virtual ~Target ();

	void SetWindow (GtkWindow *window);

	virtual bool Close () = 0;

GCU_RO_PROP (GtkWindow*, Window);
GCU_PROT_PROP (Application*, Application)
GCU_PROT_PROP (Document*, Document)
};

}	// namespace gcp

#endif	//	GCP_TARGET_H
