// -*- C++ -*-

/* 
 * GChemPaint libray
 * text-object.h 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TEXT_OBJECT_H
#define GCHEMPAINT_TEXT_OBJECT_H

#include <gcu/object.h>
#include <gcu/macros.h>
#include <pango/pango-layout.h>
#include <string>
#include <canvas/gcp-canvas-pango.h>

namespace gcp {

class TextObject: public gcu::Object
{
public:
	TextObject (gcu::TypeId Type);
	TextObject (double x, double y, gcu::TypeId Type);
	virtual ~TextObject ();
	
	void GetSize (double& x, double& y) {x = m_length; y = m_height;}
	xmlNodePtr SaveSelected ();
	void LoadSelected (xmlNodePtr node);
	virtual bool OnChanged (bool save) = 0;
	void OnSelChanged (struct GnomeCanvasPangoSelBounds *bounds);
	bool SaveNode (xmlDocPtr xml, xmlNodePtr);
	bool Load (xmlNodePtr);
	void Move (double x, double y, double z = 0);
	bool IsLocked () {return m_bLoading;}
	void GetSelectionBounds (unsigned &start, unsigned &end) {start = m_StartSel; end = m_EndSel;}
	std::string GetBuffer () {return m_buf;}

protected:
	double m_x, m_y, m_length, m_height;
	int m_ascent;
	int m_InsertOffset;
	std::string m_buf;
	bool m_bLoading;
	unsigned m_StartSel, m_EndSel;
	bool m_RealSave;

GCU_PROT_PROP (PangoLayout*, Layout);
GCU_PROT_PROP (PangoAttrList*, AttrList);
};

}	// namespace gcp

#endif	//GCHEMPAINT_TEXT_OBJECT_H
