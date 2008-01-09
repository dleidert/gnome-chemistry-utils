// -*- C++ -*-

/* 
 * GChemPaint library
 * arrow.h 
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

#ifndef GCHEMPAINT_ARROW_H
#define GCHEMPAINT_ARROW_H

#include <gcu/object.h>

namespace gcp {

enum
{
	SimpleArrow,
	ReversibleArrow,
	FullReversibleArrow,
};

class Arrow: public gcu::Object
{
public:
	Arrow(gcu::TypeId Type);
	virtual ~Arrow();
	
	bool Load (xmlNodePtr);
	void SetSelected (GtkWidget* w, int state);
	void SetCoords (double xstart, double ystart, double xend, double yend);
	bool GetCoords (double* xstart, double* ystart, double* xend, double* yend);
	void Move (double x, double y, double z = 0);
	void Transform2D (gcu::Matrix2D& m, double x, double y);
	double GetYAlign ();
	bool SetProperty (unsigned property, char const *value);

protected:
	bool Save (xmlDocPtr xml, xmlNodePtr node);
	
protected:
	double m_x, m_y, m_width, m_height;
};

}	//	namespace gcp

#endif	//GCHEMPAINT_ARROW_H
