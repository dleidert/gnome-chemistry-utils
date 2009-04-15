// -*- C++ -*-

/* 
 * GChemPaint templates plugin
 * templatetool.h 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TEMPLATETOOL_H
#define GCHEMPAINT_TEMPLATETOOL_H

#include <gcp/tool.h>

class gcpTemplate;

namespace gcp {
	class Document;
}

class gcpTemplateTool: public gcp::Tool
{
public:
	gcpTemplateTool (gcp::Application* App);
	~gcpTemplateTool ();

	virtual bool OnClicked ();
	virtual void OnDrag ();
	virtual void OnRelease ();
	gcpTemplate* GetTemplate () {return m_Template;}
	GtkWidget *GetPropertyPage ();
	void OnChanged (GtkComboBox *combo);
	void OnPreviewSize (GtkAllocation *allocation);
	void OnAddTemplate ();
	void OnDeleteTemplate ();
	char const *GetHelpTag () {return "templates";}

private:
	gcpTemplate* m_Template;
	GtkWidget *m_DeleteBtn;
	GtkNotebook *m_Book;
	int m_Width, m_Height;
};

#endif //GCHEMPAINT_TEMPLATETOOL_H
