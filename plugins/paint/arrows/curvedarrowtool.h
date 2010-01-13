// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * curvedarrowtool.h
 *
 * Copyright (C) 2004-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_CURVED_ARROW_TOOL_H
#define GCHEMPAINT_CURVED_ARROW_TOOL_H

#include <gcp/tool.h>

namespace gcp {
	class Atom;
	class Bond;
};

class gcpCurvedArrowTool: public gcp::Tool
{
public:
	gcpCurvedArrowTool (gcp::Application *App, std::string Id);
	virtual ~gcpCurvedArrowTool ();

	bool OnClicked ();
	void OnDrag ();
	void OnMotion ();
	void OnRelease ();
	void OnLeaveNotify ();

private:
	bool AllowAsSource (gcp::Atom *atom);
	bool AllowAsTarget (gcp::Atom *atom);
	bool AllowAsSource (gcp::Bond *bond);
	bool AllowAsTarget (gcp::Bond *bond);

	void AtomToAdjBond ();
	void AtomToAtom ();
	void BondToAdjAtom ();
	void BondToAdjBond ();
	void BondToAtom ();
	void ElectronToAdjBond ();
	void ElectronToAtom ();

private:
	bool m_Full; // if false use half heads
	gcu::Object *m_Target, *m_SourceAux;
	double m_CPx0, m_CPy0, m_CPx1, m_CPy1, m_CPx2, m_CPy2; // x and y deltas from ends to central control points
};

#endif	//GCHEMPAINT_CURVED_ARROW_TOOL_H
