/*
 * GChemPaint selection plugin
 * groupdlg.cc
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

#include "config.h"
#include "groupdlg.h"
#include "selectiontool.h"
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>

using namespace std;

static void on_align_toggled (gcpGroupDlg *dlg)
{
	dlg->OnAlignToggled ();
}

static void on_space_toggled (gcpGroupDlg *dlg)
{
	dlg->OnSpaceToggled ();
}

gcpGroupDlg::gcpGroupDlg (gcp::Document *Doc, gcpGroup *group):
	gcugtk::Dialog (Doc->GetApplication (), UIDIR"/group.ui", "group", GETTEXT_PACKAGE, (group)? static_cast < DialogOwner * > (group): static_cast < DialogOwner * > (Doc))
{
	m_Group = group;
	m_Doc = Doc;
	m_Data = (gcp::WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	align_box = GTK_COMBO_BOX (GetWidget ("align-type"));
	align_btn = GTK_TOGGLE_BUTTON (GetWidget ("align_btn"));
	group_btn = GTK_TOGGLE_BUTTON (GetWidget ("group_btn"));
	space_btn = GTK_TOGGLE_BUTTON (GetWidget ("space"));
	padding_btn = GTK_SPIN_BUTTON (GetWidget ("padding"));
	dist_lbl = GetWidget ("dist_lbl");
	if (group) {
		gtk_toggle_button_set_active (group_btn, true);
		gcpAlignType type;
		bool aligned = group->GetAlignType (type);
		gtk_toggle_button_set_active (align_btn, aligned);
		if (!aligned) {
			gtk_widget_set_sensitive (GTK_WIDGET (align_box), false);
			gtk_widget_set_sensitive (GTK_WIDGET (padding_btn), false);
			gtk_toggle_button_set_active (space_btn, false);
		} else {
			SetAlignType (type);
			double padding;
			bool spaced = group->GetPadding (padding);
			gtk_toggle_button_set_active (space_btn, spaced);
			if (spaced)
				gtk_spin_button_set_value (padding_btn, padding);
			else
			gtk_widget_set_sensitive (GTK_WIDGET (padding_btn), false);
		}
	} else {
		gcp::Theme *pTheme = Doc->GetTheme ();
		gtk_combo_box_set_active (align_box, 0);
		gtk_spin_button_set_value (padding_btn, pTheme->GetObjectPadding () / pTheme->GetZoomFactor ());
	}
	g_signal_connect_swapped (align_btn, "toggled", G_CALLBACK (on_align_toggled), this);
	g_signal_connect_swapped (space_btn, "toggled", G_CALLBACK (on_space_toggled), this);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

gcpGroupDlg::~gcpGroupDlg ()
{
}

void gcpGroupDlg::SetAlignType (gcpAlignType type)
{
	gtk_combo_box_set_active (align_box, type);
}

bool gcpGroupDlg::Apply ()
{
	std::set < Object  *>::iterator i, end;
	bool align = gtk_toggle_button_get_active (align_btn);
	bool group = gtk_toggle_button_get_active (group_btn);
	bool space = gtk_toggle_button_get_active (space_btn);
	gcpAlignType align_type = (gcpAlignType) gtk_combo_box_get_active (align_box);
	double padding = gtk_spin_button_get_value (padding_btn);
	gcp::Operation *pOp = m_Doc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
	if (m_Group)
		pOp->AddObject (m_Group, 0);
	else {
		end = m_Data->SelectedObjects.end ();
		for (i = m_Data->SelectedObjects.begin (); i!= end; i++)
			pOp->AddObject (*i, 0);
	}

	if (!m_Group) {
		m_Group = new gcpGroup ();
		m_Group->SetParent (m_Doc);
		for (i = m_Data->SelectedObjects.begin (); i!= end; i++)
			(*i)->SetParent (m_Group);
		m_Data->UnselectAll ();
		m_Data->SetSelected (m_Group);
	}

	// align objects now
	if (align) {
		m_Group->SetAligned (align_type);
		if (space)
			m_Group->SetPadding (padding);
		m_Group->GetParent ()->EmitSignal (gcp::OnChangedSignal);
	}

	if (!group && m_Group) {
		bool selected = m_Data->IsSelected (m_Group);
		if (selected)
			m_Data->Unselect (m_Group);
		map< string, Object * >::iterator j;
		Object *obj = m_Group->GetFirstChild (j);
		while (obj) {
			pOp->AddObject (obj, 1);
			if (selected)
				m_Data->SetSelected (obj);
			obj = m_Group->GetNextChild (j);
		}
		obj = m_Group->GetParent();
		delete m_Group;
		obj->EmitSignal (gcp::OnChangedSignal);
		m_Group = NULL;
	}

	if (m_Group)
		pOp->AddObject (m_Group, 1);
	m_Doc->FinishOperation ();

	// Update clipboard stuff
	gcp::Application *app = m_Doc->GetApplication ();
	gcpSelectionTool *tool = (gcpSelectionTool*) app->GetTool ("Select");
	if (tool)
		tool->AddSelection (m_Data);

	return true;
}

void gcpGroupDlg::OnAlignToggled ()
{
	if (gtk_toggle_button_get_active (align_btn)) {
		gtk_widget_set_sensitive (GTK_WIDGET (align_box), true);
		gtk_widget_set_sensitive (GTK_WIDGET (space_btn), true);
		gtk_widget_set_sensitive (GTK_WIDGET (padding_btn), true);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (align_box), false);
		gtk_toggle_button_set_active (space_btn, false);
		gtk_widget_set_sensitive (GTK_WIDGET (space_btn), false);
	}
}

void gcpGroupDlg::OnSpaceToggled ()
{
	if (gtk_toggle_button_get_active (space_btn)) {
		gtk_widget_set_sensitive (GTK_WIDGET (padding_btn), true);
		gtk_widget_set_sensitive (dist_lbl, true);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (padding_btn), false);
		gtk_widget_set_sensitive (dist_lbl, false);
	}
}
