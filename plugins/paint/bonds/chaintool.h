// -*- C++ -*-

/*
 * GChemPaint bonds plugin
 * chaintool.h
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCHEMPAINT_CHAIN_TOOL_H
#define GCHEMPAINT_CHAIN_TOOL_H

#include <gcp/tool.h>
#include <gccv/structs.h>
#include <gtk/gtk.h>
#include <vector>

namespace gcp {
	class Atom;
	class Operation;
}

class gcpChainTool: public gcp::Tool
{
public:
	gcpChainTool (gcp::Application *App);
	virtual ~gcpChainTool ();

	bool OnClicked();
	void OnDrag();
	void OnRelease();
	GtkWidget *GetPropertyPage ();
	void Activate ();
	bool OnKeyPress (GdkEvent* event);

	void SetAutoNumber (bool state) {gtk_widget_set_sensitive (GTK_WIDGET (m_NumberBtn), state);}
	void SetChainLength (unsigned length) {m_Length = length;}
	void SetAngle (double angle);
	void SetLength (double length);
	char const *GetHelpTag () {return "chain";}
	void Draw ();

private:
	void FindAtoms ();
	bool CheckIfAllowed ();

private:
	unsigned m_Length, m_CurPoints;
	bool m_Positive, m_AutoNb, m_Allowed;
	double m_dAngle, m_dMeanLength, m_BondLength;
	std::vector <gcp::Atom *> m_Atoms;
	gccv::Point *m_Points;
	GtkSpinButton *m_LengthBtn, *m_AngleBtn, *m_NumberBtn;
	GtkToggleButton *m_MergeBtn, *m_AutoBtn;
	bool m_AutoDir;
	double m_RefAngle;
};

#endif	//	GCHEMPAINT_CHAIN_TOOL_H
