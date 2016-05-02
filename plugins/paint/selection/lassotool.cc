// -*- C++ -*-

/*
 * GChemPaint selection plugin
 * lassotool.cc
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "lassotool.h"
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/item-client.h>
#include <gccv/polygon.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/fragment.h>
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/molecule.h>
#include <gcp/settings.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcp/window.h>
#include <gcu/matrix.h>
#include <glib/gi18n-lib.h>

gcpLassoTool::gcpLassoTool (gcp::Application *App): gcp::Tool (App, "Lasso")
{
	m_Rotate = false;
	m_UIManager = NULL;
}

gcpLassoTool::~gcpLassoTool ()
{
	if (m_UIManager)
		g_object_unref (m_UIManager);
}

bool gcpLassoTool::OnClicked ()
{
	if (m_pObject && m_pData->IsSelected (m_pObject)) {
		// save the current coordinates
		std::set < gcu::Object * >::iterator i, end = m_pData->SelectedObjects.end ();
		gcp::Document *pDoc = m_pView->GetDoc ();
		m_pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
		std::set <gcu::Object *> groups;
		gcu::Object *group;
		for (i = m_pData->SelectedObjects.begin (); i != end; i++) {
			group = (*i)->GetGroup ();
			groups.insert ((group)? group: *i);
		}
		std::set <gcu::Object *>::iterator j, jend = groups.end ();
		for (j = groups.begin (); j != jend; j++)
			m_pOp->AddObject (*j, 0);
		if (m_Rotate) {
			// Try to use the object coordinates
			if (m_pObject && m_pObject->GetCoords (&m_cx, &m_cy)) {
				m_cx *= m_dZoomFactor;
				m_cy *= m_dZoomFactor;
			} else {
				// Calculate center of selection
				gccv::Rect rect;
				m_pData->GetSelectionBounds (rect);
				m_cx = (rect.x0 + rect.x1) / 2.;
				m_cy = (rect.y0 + rect.y1) / 2.;
			}
			m_dAngle = 0.;
			m_x0 -= m_cx;
			m_y0 -= m_cy;
			if (m_x0 == 0)
				m_dAngleInit = (m_y0 <= 0) ? 90 : 270;
			else
				m_dAngleInit = atan (-m_y0 / m_x0) * 180 / M_PI;
			if (m_x0 < 0) m_dAngleInit += 180.;
		}
		return true;
	}
	std::list <gccv::Point> l;
	gccv::Point p;
	gccv::Polygon *poly;
	p.x = m_x0;
	p.y = m_y0;
	l.push_front (p);
	m_Item = poly = new gccv::Polygon (m_pView->GetCanvas (), l);
	poly->SetLineColor (gcp::SelectColor);
	return true;
}

void gcpLassoTool::OnDrag ()
{
	if (m_Item) {
		static_cast <gccv::Polygon *> (m_Item)->AddPoint (m_x, m_y);
		// Unselect everything before evaluating current selection
		m_pData->UnselectAll ();
		cairo_t *cr;
		cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1 , 1);
		cr = cairo_create (surface);
		m_Item->BuildPath (cr);
		std::list <gccv::Item *>::iterator it;
		gccv::Group *group = m_pView->GetCanvas ()->GetRoot ();
		gccv::Item *item = group->GetFirstChild (it);
		double x0, x1, y0, y1;
		gcu::Object *object;
		m_Item->GetBounds (m_x0, m_y0, m_x, m_y);
		std::set <gcu::Object *> linked_objects;
		std::set <gcu::Object *>::iterator i, iend;
		while (item) {
			if (item != m_Item) {
				item->GetBounds (x0, y0, x1, y1);
				if ((x0 < m_x) && (y0 < m_y) && (x1 > m_x0) && (y1 > m_y0)) {
					object = dynamic_cast <gcu::Object *> (item->GetClient ());
					if (object && object->GetCoords (&x0, &y0) && !m_pData->IsSelected (object)) {
						x0 *= m_dZoomFactor;
						y0 *= m_dZoomFactor;
						if (cairo_in_fill (cr, x0, y0)) {
							m_pData->SetSelected (object);
							gcp::Atom *atom = static_cast <gcp::Atom *> (object);
							switch (object->GetType ()) {
							case gcu::FragmentType:
								atom = static_cast <gcp::Fragment *> (object)->GetAtom ();
							case gcu::AtomType: {
								// go through the bonds and select them if both ends are selected
								std::map<gcu::Atom*, gcu::Bond*>::iterator i;
								gcu::Bond *bond = atom->GetFirstBond (i);
								while (bond) {
									if (m_pData->IsSelected (bond->GetAtom (atom)))
										m_pData->SetSelected (bond);
									bond = atom->GetNextBond (i);
								}
							}
							default: {
								// go through the links and store them for later treatment
								gcu::Object *linked_obj;
								linked_obj = object->GetFirstLink (i);
								while (linked_obj) {
									linked_objects.insert (linked_obj);
									linked_obj = object->GetNextLink (i);
								}
								break;
							}
							}
						}
					}
				}
			}
			item = group->GetNextChild (it);
		}
		cairo_destroy (cr);
		cairo_surface_destroy (surface);
		// now check if linked objects have all their links selected, and, if yes, select theme_change
		for (i = linked_objects.begin (), iend = linked_objects.end (); i != iend; i++)
			if ((*i)->CanSelect ())
				m_pData->SetSelected (*i);
		m_pData->SimplifySelection ();
	} else if (m_Rotate) {
		double dAngle;
		m_x-= m_cx;
		m_y -= m_cy;
		if (m_x == 0 && m_y == 0)
				return;
		else {
			dAngle = atan2 (-m_y, m_x) * 180. / M_PI - m_dAngleInit;
			if (!(m_nState & GDK_CONTROL_MASK))
				dAngle = rint(dAngle / 5) * 5;
		}
		if (dAngle < -180.)
			dAngle += 360.;
		if (dAngle > 180.)
			dAngle -= 360.;
		if (dAngle != m_dAngle) {
			// Rotate the selection
			std::set < gcu::Object * >::iterator i, end = m_pData->SelectedObjects.end ();
			std::set < gcu::Object * > dirty;
			gcu::Matrix2D m (dAngle - m_dAngle);
			for (i = m_pData->SelectedObjects.begin (); i != end; i++) {
				(*i)->Transform2D (m, m_cx / m_dZoomFactor, m_cy / m_dZoomFactor);
				if ((*i)->GetParent ()->GetType () == gcu::MoleculeType) {
					gcp::Molecule *mol = static_cast <gcp::Molecule *> ((*i)->GetParent ());
					std::list <gcu::Bond*>::const_iterator i;
					gcp::Bond const *bond = static_cast <gcp::Bond const *> (mol->GetFirstBond (i));
					while (bond) {
						const_cast <gcp::Bond *> (bond)->SetDirty ();
						bond = static_cast <gcp::Bond const *> (mol->GetNextBond (i));
					}
					dirty.insert (mol);
				} else
					m_pView->Update (*i);
			}
			std::set <gcu::Object *>::iterator j;
			while (!dirty.empty ()) {
				j = dirty.begin ();
				m_pView->Update (*j);
				dirty.erase (j);
			}
			m_dAngle = dAngle;
		}
		char tmp[32];
		snprintf (tmp, sizeof(tmp) - 1, _("Orientation: %g"), dAngle);
		m_pApp->SetStatusText (tmp);
	} else {
		// Translate the selection
		std::set < gcu::Object * >::iterator i, end = m_pData->SelectedObjects.end ();
		std::set < gcu::Object * > dirty;
		for (i = m_pData->SelectedObjects.begin (); i != end; i++) {
			(*i)->Move ((m_x - m_x0) / m_dZoomFactor, (m_y - m_y0) / m_dZoomFactor);
			if ((*i)->GetParent ()->GetType () == gcu::MoleculeType) {
				gcp::Molecule *mol = static_cast <gcp::Molecule *> ((*i)->GetParent ());
				std::list <gcu::Bond*>::const_iterator i;
				gcp::Bond const *bond = static_cast <gcp::Bond const *> (mol->GetFirstBond (i));
				while (bond) {
					const_cast <gcp::Bond *> (bond)->SetDirty ();
					bond = static_cast <gcp::Bond const *> (mol->GetNextBond (i));
				}
				dirty.insert (mol);
			} else
				m_pView->Update (*i);
		}
		std::set <gcu::Object *>::iterator j;
		while (!dirty.empty ()) {
			j = dirty.begin ();
			m_pView->Update (*j);
			dirty.erase (j);
		}
		m_x0 = m_x;
		m_y0 = m_y;
	}
}

void gcpLassoTool::OnRelease ()
{
	if (m_Item) {
		m_pData->SimplifySelection ();
		AddSelection (m_pData);
	} else {
		std::set < gcu::Object * > groups;
		std::set < gcu::Object * >::iterator j, jend;
		std::set < gcu::Object * >::iterator i, end = m_pData->SelectedObjects.end ();
		gcu::Object *group;
		for (i = m_pData->SelectedObjects.begin (); i != end; i++) {
			group = (*i)->GetGroup ();
			groups.insert ((group)? group: *i);
			(*i)->EmitSignal (gcp::OnChangedSignal);
		}
		jend = groups.end ();
		for (j = groups.begin (); j != jend; j++)
			m_pOp->AddObject (*j, 1);
		m_pView->GetDoc ()->FinishOperation ();
	}
}

void gcpLassoTool::Activate ()
{
	gcp::Document *pDoc = m_pApp->GetActiveDocument ();
	if (pDoc) {
		m_pView = m_pApp->GetActiveDocument ()->GetView ();
		GtkWidget *w = m_pView->GetWidget ();
		m_pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	}
}

bool gcpLassoTool::Deactivate ()
{
	std::map <gcp::WidgetData *, guint>::iterator i;
	while (!SelectedWidgets.empty ()) {
		i = SelectedWidgets.begin ();
		(*i).first->UnselectAll ();
		g_signal_handler_disconnect ((*i).first->Canvas, (*i).second);
		SelectedWidgets.erase (i);
	}
	return true;
}

void gcpLassoTool::AddSelection (gcp::WidgetData* data)
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
	}
}

void gcpLassoTool::OnWidgetDestroyed (GtkWidget *widget, gcpLassoTool *tool)
{
	tool->SelectedWidgets.erase (static_cast <gcp::WidgetData *> (g_object_get_data (G_OBJECT (widget), "data")));
}

void gcpLassoTool::OnFlip (bool horizontal)
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
	gcu::Matrix2D m (m_x, 0., 0., -m_x);
	std::set < gcu::Object * >::iterator i, end = m_pData->SelectedObjects.end ();
	gcp::Document* pDoc = m_pView->GetDoc ();
	m_pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
	std::set <gcu::Object *> dirty;
	for (i = m_pData->SelectedObjects.begin (); i != end; i++) {
		gcu::Object *group = (*i)->GetGroup ();
		if (group) {
			if (dirty.find (group) == dirty.end ()) {
				m_pOp->AddObject (group, 0);
				dirty.insert (group);
			}
			if ((*i)->GetType () == gcu::AtomType) {
				gcp::Atom *atom = static_cast <gcp::Atom *> (*i);
				std::map < gcu::Atom *, gcu::Bond * >::const_iterator i;
				gcp::Bond const *bond = static_cast <gcp::Bond const *> (atom->GetFirstBond (i));
				while (bond) {
					const_cast <gcp::Bond *> (bond)->SetDirty ();
					bond = static_cast <gcp::Bond const *> (atom->GetNextBond (i));
				}
			}
		} else
			m_pOp->AddObject (*i, 0);
		(*i)->Transform2D (m, m_cx / m_dZoomFactor, m_cy / m_dZoomFactor);
		if (!group) {
			m_pView->Update (*i);
			m_pOp->AddObject (*i, 1);
		}
	}
	std::set <gcu::Object *>::iterator j;
	while (!dirty.empty ()) {
		j = dirty.begin ();
		m_pOp->AddObject (*j, 1);
		m_pView->Update (*j);
		dirty.erase (j);
	}
	pDoc->FinishOperation ();
}

void gcpLassoTool::Rotate (bool rotate)
{
	m_Rotate = rotate;
}

static void on_flip (GtkWidget *btn, gcp::Application* App)
{
	gcpLassoTool *tool = static_cast <gcpLassoTool *> (App->GetTool ("Lasso"));
	if (GTK_IS_WIDGET (btn))
		tool->OnFlip (strcmp (gtk_widget_get_name (btn), "VertFlip"));
	else
		tool->OnFlip (strcmp (gtk_action_get_name (GTK_ACTION (btn)), "VertFlip"));
}

static void on_rotate (GtkWidget *btn, gcp::Application* App)
{
	gcpLassoTool *tool = static_cast <gcpLassoTool *> (App->GetTool ("Lasso"));
	if (GTK_IS_WIDGET (btn))
		tool->Rotate (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (btn)));
	else
		tool->Rotate (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (btn)));
}

GtkWidget *gcpLassoTool::GetPropertyPage ()
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
	gtk_widget_show_all (grid);
	return grid;
}

char const *gcpLassoTool::GetHelpTag ()
{
	if (m_Rotate)
		return "rotate";
	return "lasso";
}
