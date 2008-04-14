// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-operator.h 
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTION_OPERATOR_H
#define GCHEMPAINT_REACTION_OPERATOR_H

#include <gcu/object.h>

namespace gcp {

class ReactionOperator: public gcu::Object
{
public:
	ReactionOperator ();
	virtual ~ReactionOperator ();
	
	virtual void Add (GtkWidget* w) const;
	virtual void Update (GtkWidget* w) const;
	virtual void Move (double x, double y, double z = 0);
	virtual void SetSelected (GtkWidget* w, int state);
	void SetCoords (double x, double y);
	bool GetCoords (double* x, double* y) const;
	virtual double GetYAlign ();
	
private:
	double m_x, m_y;
	PangoLayout *m_Layout;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_OPERATOR_H
