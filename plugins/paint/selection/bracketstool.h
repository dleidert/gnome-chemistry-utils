// -*- C++ -*-

/* 
 * GChemPaint selection plugin
 * bracketstool.h 
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_BRACKETS_TOOL_H
#define GCHEMPAINT_BRACKETS_TOOL_H

#include <gcp/tool.h>
#include <gcp/fontsel.h>

namespace gcp {
	class Application;
}

typedef enum {
	GCP_BRACKET_NORMAL,
	GCP_BRACKET_SQUARE,
	GCP_BRACKET_CURLY
} gcpBracketType;

typedef enum {
	GCP_BRACKETS_BOTH,
	GCP_BRACKETS_OPENING,
	GCP_BRACKETS_CLOSING
} gcpBracketsUsed;

class gcpBracketsTool: public gcp::Tool
{
public:
	gcpBracketsTool (gcp::Application* App);
	virtual ~gcpBracketsTool ();

	virtual bool OnClicked ();
	virtual void OnDrag ();
	virtual void OnRelease ();
	char const *GetHelpTag () {return "brackets";}
	GtkWidget *GetPropertyPage ();
	void Activate ();

	static void OnTypeChanged (GtkComboBox *box, gcpBracketsTool *tool);
	static void OnUsedChanged (GtkComboBox *box, gcpBracketsTool *tool);

private:
	gcpBracketType m_Type;
	gcpBracketsUsed m_Used;
	GcpFontSel *m_FontSel;
};

#endif // GCHEMPAINT_BRACKETS_TOOL_H
