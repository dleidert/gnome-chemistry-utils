// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * chargetool.h 
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

#ifndef GCHEMPAINT_CHARGE_TOOL_H
#define GCHEMPAINT_CHARGE_TOOL_H

#include <gcp/tool.h>

namespace gccv {
class TextTag;
}

class gcpChargeTool: public gcp::Tool
{
public:
	gcpChargeTool(gcp::Application *App, std::string Id);
	virtual ~gcpChargeTool();

	virtual bool OnClicked();
	virtual void OnDrag();
	virtual void OnRelease();
	char const *GetHelpTag () {return "charge";}

private:
	char const *m_glyph;
	double m_dDist, m_dDistMax, m_dAngle;
	int m_Charge;
	unsigned char m_Pos, m_DefaultPos;
	bool m_bDragged;
	double m_ChargeWidth, m_ChargeTWidth, m_ChargeHeight;
	gccv::TextTag *m_Tag;
};

#endif	//GCHEMPAINT_CHARGE_TOOL_H
