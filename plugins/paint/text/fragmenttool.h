// -*- C++ -*-

/* 
 * GChemPaint text plugin
 * fragmenttool.h 
 *
 * Copyright (C) 2003-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_FRAGMENT_TOOL_H
#define GCHEMPAINT_FRAGMENT_TOOL_H

#include "texttool.h"

class gcpFragmentTool: public gcpTextTool
{
public:
	gcpFragmentTool (gcp::Application *App);
	virtual ~gcpFragmentTool ();

	virtual bool OnClicked ();
	virtual bool Deactivate ();
	virtual void Activate ();
	virtual bool OnKeyPress (GdkEventKey *event);
	virtual bool CopySelection (GtkClipboard *clipboard);
	virtual bool CutSelection (GtkClipboard *clipboard);
	virtual bool OnReceive (GtkClipboard *clipboard, GtkSelectionData *data, int type);
	void OnGetData (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info);
	GtkWidget *GetPropertyPage () {return NULL;}
	char const *GetHelpTag () {return "fragment";}

protected:
	virtual bool Unselect();
};

#endif	//GCHEMPAINT_FRAGMENT_TOOL_H
