// -*- C++ -*-

/*
 * GChemPaint cycles plugin
 * cycletool.h
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_CYCLE_TOOL_H
#define GCHEMPAINT_CYCLE_TOOL_H

#include <gcp/tool.h>
#include <gccv/structs.h>

namespace gcu {
	class Atom;
	class Chain;
}

namespace gcp {
	class Atom;
}

class gcpCycleTool: public gcp::Tool
{
public:
	gcpCycleTool (gcp::Application *App, unsigned char size);
	virtual ~gcpCycleTool ();

	virtual bool OnClicked ();
	virtual void OnDrag ();
	virtual void OnRelease ();
	virtual void OnChangeState ();
	GtkWidget *GetPropertyPage ();
	void Activate ();
	void SetLength (double length);
	char const *GetHelpTag () {return "cycle";}

protected:
	void Init ();
	bool CheckIfAllowed ();

private:
	void Draw ();

protected:
	unsigned char m_size;
	gccv::Point *m_Points;
	GtkSpinButton *m_LengthBtn;
	GtkToggleButton *m_MergeBtn;

private:
	double m_dAngle, m_dDev;	//m_dAngle gives the direction of the first bond, m_dDev is deviation between two successive bonds
	double m_dDefAngle;	//default angle when appropriate
	double m_dLength;	//Lenght of newly created bonds
	gcp::Atom *m_pAtom, *m_Start, *m_End;
	gcu::Chain *m_Chain;
	bool m_Direct;
};

class gcpNCycleTool: public gcpCycleTool
{
public:
	gcpNCycleTool (gcp::Application *App, unsigned char size);
	virtual ~gcpNCycleTool ();

	void SetSize (unsigned char size);
	unsigned char GetSize () {return m_size;}
	GtkWidget *GetPropertyPage ();

private:
	GtkSpinButton *m_SizeBtn;
};

#endif	//GCHEMPAINT_CYCLE_TOOL_H
