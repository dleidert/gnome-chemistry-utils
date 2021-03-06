// -*- C++ -*-

/*
 * GChemPaint selection plugin
 * bracketstool.h
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_BRACKETS_TOOL_H
#define GCHEMPAINT_BRACKETS_TOOL_H

#include <gcp/tool.h>
#include <gcp/fontsel.h>
#include <gccv/brackets.h>
#include <gccv/structs.h>

namespace gcp {
	class Application;
}

class gcpBracketsTool: public gcp::Tool
{
public:
	gcpBracketsTool (gcp::Application* App);
	virtual ~gcpBracketsTool ();

	bool OnClicked ();
	void OnDrag ();
	void OnRelease ();
	char const *GetHelpTag () {return "brackets";}
	GtkWidget *GetPropertyPage ();
	void Activate ();
	bool Evaluate ();

	static void OnTypeChanged (GtkComboBox *box, gcpBracketsTool *tool);
	static void OnUsedChanged (GtkComboBox *box, gcpBracketsTool *tool);
	static void OnFontChanged (GcpFontSel *fontsel, gcpBracketsTool *tool);

private:
	gccv::BracketsTypes m_Type;
	gccv::BracketsUses m_Used;
	GcpFontSel *m_FontSel;
	gccv::Rect m_ActualBounds;
	gccv::Item *m_Rect, *m_Bracket;
	std::string m_FontFamily;
	int m_FontSize;
	PangoFontDescription *m_FontDesc;
	std::string m_FontName;
	gcu::Object *m_Target;
};

#endif // GCHEMPAINT_BRACKETS_TOOL_H
