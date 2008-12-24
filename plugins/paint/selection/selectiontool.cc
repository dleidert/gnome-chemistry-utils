// -*- C++ -*-

/* 
 * GChemPaint selection plugin
 * selectiontool.cc
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

#include "config.h"
#include "selectiontool.h"
#include "group.h"
#include "groupdlg.h"
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/molecule.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/window.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/rectangle.h>
#include <gccv/structs.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <stdexcept>

using namespace gccv;
using namespace gcu;
using namespace std;

static void on_flip (GtkWidget *btn, gcp::Application* App)
{
	gcpSelectionTool *tool = (gcpSelectionTool*) App->GetTool ("Select");
	if (GTK_IS_WIDGET (btn))
		tool->OnFlip (strcmp (gtk_widget_get_name (btn), "VertFlip"));
	else
		tool->OnFlip (strcmp (gtk_action_get_name (GTK_ACTION (btn)), "VertFlip"));
}

static void on_rotate (GtkWidget *btn, gcp::Application* App)
{
	gcpSelectionTool *tool = (gcpSelectionTool*) App->GetTool ("Select");
	if (GTK_IS_WIDGET (btn))
		tool->Rotate (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (btn)));
	else
		tool->Rotate (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (btn)));
}

static void on_merge (GtkWidget *btn, gcp::Application* App)
{
	gcpSelectionTool *tool = (gcpSelectionTool*) App->GetTool ("Select");
	tool->Merge ();
}

gcpSelectionTool::gcpSelectionTool (gcp::Application *App): gcp::Tool (App, "Select")
{
	m_bRotate = false;
	m_UIManager = NULL;
}

gcpSelectionTool::~gcpSelectionTool ()
{
	if (m_UIManager)
		g_object_unref (m_UIManager);
}

bool gcpSelectionTool::OnClicked ()
{
	gcp::Window *win = m_pView->GetDoc ()->GetWindow ();
	if (m_pObject) {
		Object* pObj = m_pObject->GetGroup ();
		if (pObj)
			m_pObject = pObj;
		if (!m_pData->IsSelected (m_pObject)) {
			m_pData->UnselectAll ();
			m_pData->SetSelected (m_pObject);
			if (win) {
				win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", true);
				win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", true);
				win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", true);
			}
		}
	} else {
		m_pData->UnselectAll ();
		if (win) {
			win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", false);
			win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", false);
			win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", false);
		}
	}
	if (m_bRotate) {
		// Calculate center of selection
		gccv::Rect rect;
		m_pData->GetSelectionBounds (rect);
		m_cx = (rect.x0 + rect.x1) / 2.;
		m_cy = (rect.y0 + rect.y1) / 2.;
		m_dAngle = 0.;
		m_x0 -= m_cx;
		m_y0 -= m_cy;
		if (m_x0 == 0)
			m_dAngleInit = (m_y0 <= 0) ? 90 : 270;
		else
			m_dAngleInit = atan (-m_y0 / m_x0) * 180 / M_PI;
		if (m_x0 < 0) m_dAngleInit += 180.;
		std::list<Object*>::iterator i, end = m_pData->SelectedObjects.end ();
		gcp::Document* pDoc = m_pView->GetDoc ();
		m_pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
		for (i = m_pData->SelectedObjects.begin (); i != end; i++)
			m_pOp->AddObject (*i,0);
	}
	return true;
}

void gcpSelectionTool::OnDrag ()
{
	double dx = m_x - m_x1, dy = m_y - m_y1;
	m_x1 = m_x;
	m_y1 = m_y;
	if (m_pObject) {
		if (m_bRotate) {
			double dAngle;
			m_x-= m_cx;
			m_y -= m_cy;
			if (m_x == 0) {
				if (m_y == 0)
					return;
				dAngle = (m_y < 0) ? 90 : 270;
			} else {
				dAngle = atan (-m_y / m_x) * 180. / M_PI;
				if (m_x < 0)
					dAngle += 180.;
				dAngle -= m_dAngleInit;
				if (!(m_nState & GDK_CONTROL_MASK))
					dAngle = rint(dAngle / 5) * 5;
			}
			if (dAngle < -180.)
				dAngle += 360.;
			if (dAngle > 180.)
				dAngle -= 360.;
			if (dAngle != m_dAngle) {
				m_pData->RotateSelection (m_cx, m_cy, dAngle - m_dAngle);
				m_dAngle = dAngle;
			}
			char tmp[32];
			snprintf (tmp, sizeof(tmp) - 1, _("Orientation: %g"), dAngle);
			m_pApp->SetStatusText (tmp);
		} else
			m_pData->MoveSelectedItems (dx, dy);
	} else {
		if (m_Item) {
			reinterpret_cast <Rectangle *> (m_Item)->SetPosition (m_x0, m_y0, m_x - m_x0, m_y - m_y0);
		} else {
			m_Item = new Rectangle (m_pView->GetCanvas (), m_x0, m_y0, m_x - m_x0, m_y - m_y0);
			gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
			static_cast <LineItem *> (m_Item)->SetLineColor (gcp::SelectColor);
			static_cast <LineItem *> (m_Item)->SetLineWidth (pTheme->GetBondWidth ());
			static_cast <FillItem *> (m_Item)->SetFillColor (0);
		}
	}
}

void gcpSelectionTool::OnRelease ()
{
	m_pApp->ClearStatus ();
	if (m_pObject) {
		if (m_bRotate) {
			std::list<Object*>::iterator i, end = m_pData->SelectedObjects.end ();
			gcp::Document* pDoc = m_pView->GetDoc ();
			for (i = m_pData->SelectedObjects.begin (); i != end; i++)
				m_pOp->AddObject (*i,1);
			pDoc->FinishOperation ();
		} else {
			double dx = m_x1 - m_x0, dy = m_y1 - m_y0;
			if (dx != 0.0 && dy != 0.0) {
				m_pData->MoveSelectedItems (-dx, -dy);
				m_pData->MoveSelection (dx, dy);
			}
		}
	} else {
		if (m_x < m_x0) {
			m_x1 = m_x0;
			m_x0 = m_x;
		} else
			m_x1 = m_x;
		if (m_y < m_y0) {
			m_y1 = m_y0;
			m_y0 = m_y;
		}
		else
			m_y1 = m_y;
		double x0, y0, x1, y1;
		if (m_Item) {
			delete m_Item;
			m_Item = NULL;
		}
		gccv::Group *group = m_pView->GetCanvas ()->GetRoot ();
		// all client top items are implemented as a child of the root
		list<Item *>::iterator it;
		Item *item = group->GetFirstChild (it);
		Object *object;
		while (item) {
			item->GetBounds (x0, y0, x1, y1);
			if ((x0 < m_x1) && (y0 < m_y1) && (x1 > m_x0) && (y1 > m_y0)) {
				object = dynamic_cast <Object *> (item->GetClient ());
				m_pObject = object->GetGroup ();
				if (m_pObject) {
					if (!m_pData->IsSelected (m_pObject))
						m_pData->SetSelected (m_pObject);
				}
				else
					m_pData->SetSelected (object);
			}
			item = group->GetNextChild (it);
		}
	}
	AddSelection (m_pData);
}

void gcpSelectionTool::Activate ()
{
	if (m_UIManager)
		gtk_widget_set_sensitive (m_MergeBtn, false);
	gcp::Document *pDoc = m_pApp->GetActiveDocument ();
	if (pDoc) {
		m_pView = m_pApp->GetActiveDocument ()->GetView ();
		GtkWidget *w = m_pView->GetWidget ();
		m_pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	}
}

bool gcpSelectionTool::Deactivate ()
{
	while (!SelectedWidgets.empty ())
	{
		SelectedWidgets.front ()->UnselectAll ();
		SelectedWidgets.pop_front ();
	}
	return true;
}

void gcpSelectionTool::AddSelection (gcp::WidgetData* data)
{
	gcp::WidgetData *d = m_pData;
	m_pData = data;
	m_pView = data->m_View;
	gcp::Window *win = m_pView->GetDoc ()->GetWindow ();
	if (m_pData->HasSelection()) {
		GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		m_pView->OnCopySelection (m_pData->Canvas, clipboard);
		if (win) {
			win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", true);
			win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", true);
			win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", true);
		}
	}
	SelectedWidgets.remove (m_pData);
	SelectedWidgets.push_front (m_pData);
	if (d) {
		m_pView = d->m_View;
		m_pData = d;
	}
	// If the selection is made of two molecules, activate the merge tool
	if (m_UIManager)
		gtk_widget_set_sensitive (m_MergeBtn, ((m_pData->SelectedObjects.size () == 2) &&
			(m_pData->SelectedObjects.front ()->GetType () == MoleculeType) &&
			(m_pData->SelectedObjects.back ()->GetType () == MoleculeType)));
}

void gcpSelectionTool::OnFlip (bool horizontal)
{
	if (!m_pData) {
		m_pView = m_pApp->GetActiveDocument ()->GetView ();
		GtkWidget *w = m_pView->GetWidget ();
		m_pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	}
	if (!m_pData->SelectedObjects.size ())
		return;
	gccv::Rect rect;
	m_pData->GetSelectionBounds (rect);
	m_cx = (rect.x0 + rect.x1) / 2.;
	m_cy = (rect.y0 + rect.y1) / 2.;
	m_x = (horizontal)? -1.: 1.;
	Matrix2D m(m_x, 0., 0., -m_x);
	std::list<Object*>::iterator i, end = m_pData->SelectedObjects.end ();
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	m_pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
	for (i = m_pData->SelectedObjects.begin (); i != end; i++) {
		m_pOp->AddObject (*i,0);
		(*i)->Transform2D (m, m_cx / pTheme->GetZoomFactor (), m_cy / pTheme->GetZoomFactor ());
		m_pView->Update (*i);
		m_pOp->AddObject (*i,1);
	}
	pDoc->FinishOperation ();
}

void gcpSelectionTool::Rotate (bool rotate)
{
	m_bRotate = rotate;
}

void gcpSelectionTool::Merge ()
{
	gcp::Molecule *pMol0, *pMol1;
	gcp::Document* pDoc = m_pApp->GetActiveDocument ();
	if (!m_pData) {
		m_pView = pDoc->GetView ();
		GtkWidget *w = m_pView->GetWidget ();
		m_pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	}
	pMol0 = (gcp::Molecule*) m_pData->SelectedObjects.front ();
	pMol1 = (gcp::Molecule*) m_pData->SelectedObjects.back ();
	m_pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
	m_pOp->AddObject (pMol0, 0);
	m_pOp->AddObject (pMol1, 0);
	m_pData->UnselectAll ();
	if (pMol0->Merge (pMol1, true)) {
		m_pOp->AddObject (pMol0, 1);
		m_pData->SetSelected (pMol0);
		m_pView->Update (pMol0);
		pDoc->FinishOperation ();
	} else {
		pDoc->AbortOperation ();
	}
	AddSelection (m_pData);
}

static void on_create_group (gcpSelectionTool* tool)
{
	tool->CreateGroup ();
}

static void on_group (gcpSelectionTool* tool)
{
	tool->Group ();
}

void gcpSelectionTool::Group ()
{
	gcp::Document *pDoc = m_pView->GetDoc ();
	new gcpGroupDlg (pDoc, NULL);
}

void gcpSelectionTool::CreateGroup ()
{
	gcp::Document *pDoc = m_pView->GetDoc ();
	Object *pObj = Object::CreateObject (Object::GetTypeName (m_Type), pDoc);
	try {
		m_pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
		std::list<Object*>::iterator i, end = m_pData->SelectedObjects.end();
		for (i = m_pData->SelectedObjects.begin (); i != end; i++)
			m_pOp->AddObject (*i,0);
		if (!pObj->Build (m_pData->SelectedObjects)) {
			pDoc->AbortOperation ();
			delete pObj;
			GtkWidget* message = gtk_message_dialog_new (NULL, (GtkDialogFlags) 0,
								GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Creation failed!"));
			gtk_window_set_icon_name (GTK_WINDOW (message), "gchempaint");
			g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
			gtk_widget_show (message);
			return;
		}
		m_pView->Update (pObj);
		m_pData->UnselectAll ();
		m_pData->SetSelected (pObj);
		AddSelection (m_pData);
		m_pOp->AddObject (pObj, 1);
		pDoc->FinishOperation ();
	}
	catch (invalid_argument& e) {
			pDoc->AbortOperation ();
			delete pObj;
			GtkWidget* message = gtk_message_dialog_new (NULL, (GtkDialogFlags) 0,
								GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, e.what ());
			gtk_window_set_icon_name (GTK_WINDOW (message), "gchempaint");
			g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
			gtk_widget_show (message);
	}
}

bool gcpSelectionTool::OnRightButtonClicked (GtkUIManager *UIManager)
{
	// first destroy the GtkUIManager
	if (m_pData->SelectedObjects.size () > 1) {
		GtkActionGroup *group = gtk_action_group_new ("selection");
		GtkAction *action = gtk_action_new ("group", _("Group and/or align objects"), NULL, NULL);
		gtk_action_group_add_action (group, action);
		m_uiIds.push_front (gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menuitem action='group'/></popup></ui>", -1, NULL));
		g_signal_connect_swapped (action, "activate", G_CALLBACK (on_group), this);
		set<TypeId> possible_types, types, wrong_types;
		list<Object*>::iterator  i = m_pData->SelectedObjects.begin (),
												end = m_pData->SelectedObjects.end ();
		(*i)->GetPossibleAncestorTypes (possible_types);
		set<TypeId>::iterator type;
		for (i++; i != end; i++) {
			(*i)->GetPossibleAncestorTypes (types);
			for (type = possible_types.begin(); type != possible_types.end (); type++)
				if (types.find (*type) == types.end ())
					wrong_types.insert (*type);
			for (type = wrong_types.begin(); type != wrong_types.end (); type++)
				possible_types.erase (*type);
			wrong_types.clear ();
			types.clear ();
		}
		if (possible_types.size () == 1) {
			// Add a new action.
			m_Type = *possible_types.begin ();
			const string &label = Object::GetCreationLabel (m_Type);
			if (label.size ()) {
				action = gtk_action_new ("create_group", label.c_str (), NULL, NULL);
				gtk_action_group_add_action (group, action);
				char buf[] = "<ui><popup><menuitem action='create_group'/></popup></ui>";
				m_uiIds.push_front (gtk_ui_manager_add_ui_from_string (UIManager, buf, -1, NULL));
				g_signal_connect_swapped (action, "activate", G_CALLBACK (on_create_group), this);
			}
		}
		gtk_ui_manager_insert_action_group (UIManager, group, 0);
		return true;
	}
	return false;
}

static GtkActionEntry entries[] = {
	{ "HorizFlip", "gcp_Horiz", N_("Horizontal flip"), NULL,
		N_("Flip the selection horizontally"), G_CALLBACK (on_flip) },
	{ "VertFlip", "gcp_Vert", N_("Vertical flip"), NULL,
		N_("Flip the selection vertically"), G_CALLBACK (on_flip) },
	{ "Merge", "gcp_Merge", N_("Merge"), NULL,
		N_("Merge two molecules"), G_CALLBACK (on_merge) }
};

static GtkToggleActionEntry toggles[] = {
	  { "Rotate", "gcp_Rotate", N_("_Rotate"), NULL,
		  N_("Rotate the selection"), G_CALLBACK (on_rotate), false }
};

static const char *ui_description =
"<ui>"
"  <toolbar name='Selection'>"
"    <toolitem action='HorizFlip'/>"
"    <toolitem action='VertFlip'/>"
"    <toolitem action='Rotate'/>"
"    <toolitem action='Merge'/>"
"  </toolbar>"
"</ui>";

GtkWidget *gcpSelectionTool::GetPropertyPage ()
{
	GtkWidget *box, *w;
	GtkActionGroup *action_group;
	GError *error;

	box = gtk_vbox_new (FALSE, 0);
	action_group = gtk_action_group_new ("SelectionToolActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), m_pApp);
	gtk_action_group_add_toggle_actions (action_group, toggles, G_N_ELEMENTS (toggles), m_pApp);

	m_UIManager = gtk_ui_manager_new ();
	if (!gtk_ui_manager_add_ui_from_string (m_UIManager, ui_description, -1, &error))
	  {
		g_message ("building property page failed: %s", error->message);
		g_error_free (error);
		gtk_widget_destroy (box);
		g_object_unref (m_UIManager);
		m_UIManager = NULL;
		return NULL;;
	  }
	gtk_ui_manager_insert_action_group (m_UIManager, action_group, 0);
	w = gtk_ui_manager_get_widget (m_UIManager, "/Selection");
	gtk_toolbar_set_style (GTK_TOOLBAR (w), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (w), false);
	gtk_box_pack_start (GTK_BOX (box), w, false, false, 0);
	gtk_widget_show_all (box);
	m_MergeBtn = gtk_ui_manager_get_widget (m_UIManager, "/Selection/Merge");
	gtk_widget_set_sensitive (m_MergeBtn, false);
	return box;
}

char const *gcpSelectionTool::GetHelpTag ()
{
	if (m_bRotate)
		return "rotate";
	return "selection";
}
