// -*- C++ -*-

/*
 * GChemPaint bonds plugin
 * newman.h
 *
 * Copyright (C) 2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_NEWMAN_TOOL_H
#define GCHEMPAINT_NEWMAN_TOOL_H

#include <gcp/tool.h>

class gcpNewmanTool: public gcp::Tool
{
friend class gcpNewmanToolPrivate;
public:
	gcpNewmanTool (gcp::Application *App);
	virtual ~gcpNewmanTool ();

	bool OnClicked ();
	void OnDrag ();
	void OnRelease ();
	GtkWidget *GetPropertyPage ();
	void Activate ();

private:
	GtkSpinButton *m_LengthBtn, *m_OrderBtn, *m_ForeBondsBtn, *m_RearBondsBtn,
				  *m_ForeFirstAngleBtn, *m_RearFirstAngleBtn,
				  *m_ForeBondAngleBtn, *m_RearBondAngleBtn;
	double m_ForeFirstAngle, m_RearFirstAngle, m_ForeBondAngle, m_RearBondAngle;
	int m_ForeBonds, m_RearBonds, m_Order;
};

#endif	//	GCHEMPAINT_NEWMAN_TOOL_H
