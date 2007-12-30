// -*- C++ -*-

/* 
 * GChemPaint library
 * text.h 
 *
 * Copyright (C) 2002-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TEXT_H
#define GCHEMPAINT_TEXT_H

#include <gcu/object.h>
#include <libxml/tree.h>
#include <gtk/gtk.h>
#include "text-object.h"

namespace gcp {

class Text: public TextObject
{
public:
	Text ();
	Text (double x, double y);
	virtual ~Text ();
	
	void GetCoords (double *x, double *y);
	void SetCoords (double x, double y);
	xmlNodePtr Save (xmlDocPtr xml);
	xmlNodePtr SaveSelection (xmlDocPtr xml);
	bool Load (xmlNodePtr);
	bool LoadSelection (xmlNodePtr node, unsigned pos);
	bool LoadNode (xmlNodePtr, unsigned &pos, int level = 0);
	void Add (GtkWidget* w);
	void Update (GtkWidget* w);
	void SetSelected (GtkWidget* w, int state);
	bool OnChanged (bool save);
	void Transform2D (gcu::Matrix2D& m, double x, double y);
	bool OnEvent (GdkEvent *event);
	void GetSize (double& x, double& y) {x = m_length; y = m_height;}
	double GetYAlign ();
	void SetText (char const *text) {m_buf = text;}
	bool SetProperty (unsigned property, char const *value);
};

}	//	namespace gcp

#endif	//GCHEMPAINT_TEXT_H
