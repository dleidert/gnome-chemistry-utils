/* 
 * GChemPaint selection plugin
 * groupdlg.h
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_GROUP_DLG_H
#define GCHEMPAINT_GROUP_DLG_H

#include <gcp/document.h>
#include <gcp/widgetdata.h>
#include "group.h"
#include <gcugtk/dialog.h>

using namespace gcu;

class gcpGroupDlg: public gcugtk::Dialog
{
public:
	gcpGroupDlg (gcp::Document *Doc, gcpGroup *group);
	virtual ~gcpGroupDlg ();

	virtual bool Apply ();
	void SetAlignType (gcpAlignType type);

	void OnAlignToggled ();
	void OnSpaceToggled ();

private:
	GtkComboBox *align_box;
	GtkToggleButton *align_btn, *group_btn, *space_btn;
	GtkSpinButton *padding_btn;
	GtkWidget *dist_lbl;
	gcp::Document *m_Doc;
	gcp::WidgetData *m_Data;
	gcpGroup *m_Group;
};

#endif	// GCHEMPAINT_GROUP_DLG_H
