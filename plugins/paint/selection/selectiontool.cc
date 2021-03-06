// -*- C++ -*-

/*
 * GChemPaint selection plugin
 * selectiontool.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcugtk/message.h>
#include <gcugtk/ui-manager.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/line.h>
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
	tool->OnFlip (strcmp (gtk_widget_get_name (btn), "VertFlip"));
}

static void on_rotate (GtkToggleToolButton *btn, gcp::Application* App)
{
	gcpSelectionTool *tool = (gcpSelectionTool*) App->GetTool ("Select");
	tool->Rotate (gtk_toggle_tool_button_get_active (btn));
}

static void on_merge (G_GNUC_UNUSED GtkWidget *btn, gcp::Application* App)
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
		delete m_UIManager;
}

bool gcpSelectionTool::OnClicked ()
{
	gcp::Window *win = static_cast < gcp::Window * > (m_pView->GetDoc ()->GetWindow ());
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
		std::set < Object * >::iterator i, end = m_pData->SelectedObjects.end ();
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
			std::set < Object * >::iterator i, end = m_pData->SelectedObjects.end ();
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
	std::map <gcp::WidgetData *, guint>::iterator i;
	while (!SelectedWidgets.empty ())
	{
		i = SelectedWidgets.begin ();
		(*i).first->UnselectAll ();
		g_signal_handler_disconnect ((*i).first->Canvas, (*i).second);
		SelectedWidgets.erase (i);
	}
	return true;
}

void gcpSelectionTool::AddSelection (gcp::WidgetData* data)
{
	gcp::WidgetData *d = m_pData;
	m_pData = data;
	m_pView = data->m_View;
	gcp::Window *win = static_cast < gcp::Window * > (m_pView->GetDoc ()->GetWindow ());
	if (m_pData->HasSelection()) {
		GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		m_pView->OnCopySelection (m_pData->Canvas, clipboard);
		if (win) {
			win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", true);
			win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", true);
			win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", true);
		}
		std::map <gcp::WidgetData *, guint>::iterator i;
		if (SelectedWidgets.find (m_pData) == SelectedWidgets.end ())
			SelectedWidgets[m_pData] = g_signal_connect (m_pData->Canvas, "destroy", G_CALLBACK (OnWidgetDestroyed), this);
		if (d) {
			m_pView = d->m_View;
			m_pData = d;
		}
		// If the selection is made of two molecules, activate the merge tool
		if (m_MergeBtn) {
			std::set < Object * >::iterator i = m_pData->SelectedObjects.begin ();
			gtk_widget_set_sensitive (m_MergeBtn, ((m_pData->SelectedObjects.size () == 2) &&
				((*i++)->GetType () == MoleculeType) &&
				((*i)->GetType () == MoleculeType)));
		}
	} else {
		if (m_UIManager)
			gtk_widget_set_sensitive (m_MergeBtn, false);
		win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", false);
	}
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
	std::set < Object * >::iterator i, end = m_pData->SelectedObjects.end ();
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
	std::set < gcu::Object * >::iterator i = m_pData->SelectedObjects.begin ();
	pMol0 = (gcp::Molecule*) (*i++);
	pMol1 = static_cast < gcp::Molecule * > (*i);
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
	gcu::Dialog *dlg = pDoc->GetDialog ("group");
	if (dlg)
		dlg->Present ();
	else
		new gcpGroupDlg (pDoc, NULL);
}

void gcpSelectionTool::CreateGroup ()
{
	gcp::Document *pDoc = m_pView->GetDoc ();
	Object *pObj = Object::CreateObject (Object::GetTypeName (m_Type), pDoc);
	try {
		m_pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
		std::set < Object * >::iterator i, end = m_pData->SelectedObjects.end();
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
		m_pView->EnsureSize ();
		m_pData->UnselectAll ();
		m_pData->SetSelected (pObj);
		AddSelection (m_pData);
		m_pOp->AddObject (pObj, 1);
		pDoc->FinishOperation ();
	}
	catch (invalid_argument& e) {
			pDoc->AbortOperation ();
			delete pObj;
			gcugtk::Message *box = new gcugtk::Message (static_cast < gcugtk::Application * > (pDoc->GetApp ()),
			                                            e.what (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, pDoc->GetGtkWindow ());
			box->Show ();
	}
}

bool gcpSelectionTool::OnRightButtonClicked (gcu::UIManager *UIManager)
{
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	if (m_pData->SelectedObjects.size () > 1) {
		GtkActionGroup *group = gtk_action_group_new ("selection");
		GtkAction *action = gtk_action_new ("group", _("Group and/or align objects"), NULL, NULL);
		gtk_action_group_add_action (group, action);
		m_uiIds.push_front (gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menuitem action='group'/></popup></ui>", -1, NULL));
		g_signal_connect_swapped (action, "activate", G_CALLBACK (on_group), this);
		set<TypeId> possible_types, types, wrong_types, children_types;
		set < Object * >::iterator  i = m_pData->SelectedObjects.begin (),
												end = m_pData->SelectedObjects.end ();
		children_types.insert ((*i)->GetType());
		(*i)->GetPossibleAncestorTypes (possible_types);
		set<TypeId>::iterator requested, end_requested, type, end_type;
		for (i++; i != end; i++) {
			children_types.insert ((*i)->GetType());
			(*i)->GetPossibleAncestorTypes (types);
			for (type = possible_types.begin(); type != possible_types.end (); type++)
				if (types.find (*type) == types.end ())
					wrong_types.insert (*type);
			for (type = wrong_types.begin(); type != wrong_types.end (); type++)
				possible_types.erase (*type);
			wrong_types.clear ();
			types.clear ();
		}
		/* verify that all requested children are there */
		end_type = possible_types.end ();
		for  (type = possible_types.begin (); type != end_type; type++) {
			TypeDesc const *desc = m_pApp->GetTypeDescription (*type), *rdesc;
			if (desc->RequiredParents.size () > 0) {
				wrong_types.insert (*type);
				continue;
			}
			end_requested = desc->RequiredChildren.end ();
			for (requested = desc->RequiredChildren.begin (); requested != end_requested; requested++) {
				rdesc = m_pApp->GetTypeDescription (*requested);
				if (rdesc->RequiredChildren.size () > 0 || rdesc->RequiredParents.size () > 0)
					continue;
				if (children_types.find (*requested) == children_types.end()) {
					wrong_types.insert (*type);
					break;
				 }
			}
		}
		for (type = wrong_types.begin(); type != wrong_types.end (); type++)
			possible_types.erase (*type);
		if (possible_types.size () == 1) {
			// Add a new action.
			m_Type = *possible_types.begin ();
			const string &label = Object::GetCreationLabel (m_Type);
			if (label.size ()) {
				action = gtk_action_new ("create_group", label.c_str (), NULL, NULL);
				gtk_action_group_add_action (group, action);
				char buf[] = "<ui><popup><menuitem action='create_group'/></popup></ui>";
				m_uiIds.push_front (gtk_ui_manager_add_ui_from_string (uim, buf, -1, NULL));
				g_signal_connect_swapped (action, "activate", G_CALLBACK (on_create_group), this);
			}
		}
		gtk_ui_manager_insert_action_group (uim, group, 0);
		return true;
	}
	return false;
}

GtkWidget *gcpSelectionTool::GetPropertyPage ()
{
	GtkWidget *grid, *w;
	GtkToolbar *tb;
	GtkToolItem *ti;

	grid = gtk_grid_new ();
	g_object_set (G_OBJECT (grid), "orientation", GTK_ORIENTATION_VERTICAL, "border-width", 6, NULL);
	w = gtk_toolbar_new ();
	tb = GTK_TOOLBAR (w);
	gtk_toolbar_set_style (tb, GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_show_arrow (tb, false);
	gtk_container_add (GTK_CONTAINER (grid), w);
	ti = gtk_tool_button_new ( gtk_image_new_from_icon_name ("object-flip-horizontal", GTK_ICON_SIZE_LARGE_TOOLBAR), NULL);
	gtk_tool_item_set_tooltip_text (ti, _("Flip the selection horizontally"));
	gtk_widget_set_name (GTK_WIDGET (ti), "HorizFlip");
	gtk_toolbar_insert (tb, ti, -1);
	g_signal_connect (G_OBJECT (ti), "clicked", G_CALLBACK (on_flip), m_pApp);
	ti = gtk_tool_button_new ( gtk_image_new_from_icon_name ("object-flip-vertical", GTK_ICON_SIZE_LARGE_TOOLBAR), NULL);
	gtk_tool_item_set_tooltip_text (ti, _("Flip the selection vertically"));
	gtk_widget_set_name (GTK_WIDGET (ti), "VertFlip");
	gtk_toolbar_insert (tb, ti, -1);
	g_signal_connect (G_OBJECT (ti), "clicked", G_CALLBACK (on_flip), m_pApp);
	ti = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (ti), gtk_image_new_from_icon_name ("object-rotate-right", GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_tool_item_set_tooltip_text (ti, _("Rotate the selection"));
	gtk_toolbar_insert (tb, ti, -1);
	g_signal_connect (G_OBJECT (ti), "toggled", G_CALLBACK (on_rotate), m_pApp);
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	gccv::Rectangle *rect = new gccv::Rectangle (canvas, 1., 1., 8., 7.);
	rect->SetAutoColor (true);
	rect->SetFillColor (0);
	rect->SetLineWidth (2.);
	rect = new gccv::Rectangle (canvas, 15., 1., 8., 7.);
	rect->SetAutoColor (true);
	rect->SetFillColor (0);
	rect->SetLineWidth (2.);
	rect = new gccv::Rectangle (canvas, 4., 16., 16., 7.);
	rect->SetAutoColor (true);
	rect->SetFillColor (0);
	rect->SetLineWidth (2.);
	gccv::Line *line = new gccv::Line (canvas, 12., 16., 12., 23.);
	line->SetAutoColor (true);
	line->SetLineWidth (2.);
	double const dash[] = {1.};
	line->SetDashes (dash, 1, 0.);
	line = new gccv::Line (canvas, 5., 8., 8., 16.);
	line->SetAutoColor (true);
	line->SetLineWidth (2.);
	line->SetDashes (dash, 1, 0.);
	line = new gccv::Line (canvas, 19., 8., 16., 16.);
	line->SetAutoColor (true);
	line->SetLineWidth (2.);
	line->SetDashes (dash, 1, 0.);
	gtk_widget_set_size_request (canvas->GetWidget (), 24, 24);
	ti = gtk_tool_button_new (canvas->GetWidget (), NULL);
	gtk_tool_item_set_tooltip_text (ti, _("Merge two molecules"));
	gtk_toolbar_insert (tb, ti, -1);
	g_signal_connect (G_OBJECT (ti), "clicked", G_CALLBACK (on_merge), m_pApp);
	m_MergeBtn = GTK_WIDGET (ti);
	gtk_widget_show_all (grid);
	gtk_widget_set_sensitive (m_MergeBtn, false);
	return grid;
}

char const *gcpSelectionTool::GetHelpTag ()
{
	if (m_bRotate)
		return "rotate";
	return "selection";
}

void gcpSelectionTool::OnWidgetDestroyed (GtkWidget *widget, gcpSelectionTool *tool)
{
	tool->SelectedWidgets.erase (static_cast <gcp::WidgetData *> (g_object_get_data (G_OBJECT (widget), "data")));
}
