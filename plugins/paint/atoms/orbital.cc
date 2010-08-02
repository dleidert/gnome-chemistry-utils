// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * orbital.cc 
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "orbital.h"
#include <gcu/dialog.h>
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <gccv/group.h>
#include <gccv/leaf.h>
#include <glib/gi18n.h>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////
// class gcpOrbitalProps: the Orbital properties dialog class

namespace gcp {
extern xmlDocPtr pXmlDoc;
}

class gcpOrbitalProps: public gcu::Dialog
{
public:
	friend class gcpOrbital;
	gcpOrbitalProps (gcp::Document *doc, gcpOrbital *orbital);
	~gcpOrbitalProps ();

	static void OnStartEditing (gcpOrbitalProps *dlg);
	static void OnEndEditing (gcpOrbitalProps *dlg);
	static void OnTypeChanged (gcpOrbitalProps *dlg, GtkToggleButton *btn);
	static void OnCoefChanged (gcpOrbitalProps *dlg, GtkSpinButton *btn);
	static void OnRotationChanged (gcpOrbitalProps *dlg, GtkSpinButton *btn);

private:
	gcpOrbital *m_Orbital;
	gcp::Document *m_Doc;
	//values on edition start
	gcpOrbitalType m_Type;
	double m_Coef, m_Rotation;
	xmlNodePtr m_Node;
};

gcpOrbitalProps::gcpOrbitalProps (gcp::Document *doc, gcpOrbital *orbital):
	gcu::Dialog (doc? doc->GetApplication (): NULL, UIDIR"/orbital-prop.ui", "orbital-properties", GETTEXT_PACKAGE, orbital)
{
	m_Orbital = orbital;
	m_Doc = doc;
	SetTransientFor (doc->GetGtkWindow ());
	m_Type = GCP_ORBITAL_INVALID;
	m_Rotation = m_Coef = 0.;
	m_Node = NULL;
	g_signal_connect_swapped (dialog, "focus-in-event", G_CALLBACK (gcpOrbitalProps::OnStartEditing), this);
	g_signal_connect_swapped (dialog, "focus-out-event", G_CALLBACK (gcpOrbitalProps::OnEndEditing), this);
	GtkWidget *w = GetWidget ("s-btn");
	g_object_set_data (G_OBJECT (w), "orbital-type", GUINT_TO_POINTER (GCP_ORBITAL_TYPE_S));
	if (m_Orbital->GetType () == GCP_ORBITAL_TYPE_S) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), true);
		gtk_widget_set_sensitive (GetWidget ("rotation-btn"), false);
		gtk_widget_set_sensitive (GetWidget ("rotation-btn"), false);
	}
	g_signal_connect_swapped (w, "toggled", G_CALLBACK (gcpOrbitalProps::OnTypeChanged), this);
	w = GetWidget ("p-btn");
	g_object_set_data (G_OBJECT (w), "orbital-type", GUINT_TO_POINTER (GCP_ORBITAL_TYPE_P));
	if (m_Orbital->GetType () == GCP_ORBITAL_TYPE_P)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), true);
	g_signal_connect_swapped (w, "toggled", G_CALLBACK (gcpOrbitalProps::OnTypeChanged), this);
	w = GetWidget ("dxy-btn");
	g_object_set_data (G_OBJECT (w), "orbital-type", GUINT_TO_POINTER (GCP_ORBITAL_TYPE_DXY));
	if (m_Orbital->GetType () == GCP_ORBITAL_TYPE_DXY)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), true);
	g_signal_connect_swapped (w, "toggled", G_CALLBACK (gcpOrbitalProps::OnTypeChanged), this);
	w = GetWidget ("dz2-btn");
	g_object_set_data (G_OBJECT (w), "orbital-type", GUINT_TO_POINTER (GCP_ORBITAL_TYPE_DZ2));
	if (m_Orbital->GetType () == GCP_ORBITAL_TYPE_DZ2)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), true);
	g_signal_connect_swapped (w, "toggled", G_CALLBACK (gcpOrbitalProps::OnTypeChanged), this);
	w = GetWidget ("coef-btn");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), m_Orbital->GetCoef ());
	g_signal_connect_swapped (w, "value-changed", G_CALLBACK (gcpOrbitalProps::OnCoefChanged), this);
	w = GetWidget ("rotation-btn");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), m_Orbital->GetRotation ());
	g_signal_connect_swapped (w, "value-changed", G_CALLBACK (gcpOrbitalProps::OnRotationChanged), this);
}

gcpOrbitalProps::~gcpOrbitalProps ()
{
	OnEndEditing (this);
	if (m_Node)
		xmlFree (m_Node);
}

void gcpOrbitalProps::OnStartEditing (gcpOrbitalProps *dlg)
{
	if (dlg->m_Node)
		xmlFree (dlg->m_Node);
	dlg->m_Coef = dlg->m_Orbital->GetCoef ();
	dlg->m_Rotation = dlg->m_Orbital->GetRotation ();
	dlg->m_Type = dlg->m_Orbital->GetType ();
	gcu::Object *obj = dlg->m_Orbital->GetGroup ();
	dlg->m_Node = obj->Save (gcp::pXmlDoc);
}

void gcpOrbitalProps::OnEndEditing (gcpOrbitalProps *dlg)
{
	if (dlg->m_Orbital == NULL)
		return;
	bool changed = dlg->m_Coef != dlg->m_Orbital->GetCoef () || dlg->m_Type != dlg->m_Orbital->GetType () ||
					(dlg->m_Type != GCP_ORBITAL_TYPE_S && dlg->m_Rotation != dlg->m_Orbital->GetRotation ());
	if (changed) {
		gcp::Operation *op = dlg->m_Doc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
		op->AddNode (dlg->m_Node, 0);
		gcu::Object *obj = dlg->m_Orbital->GetGroup ();
		op->AddObject (obj, 1);
		dlg->m_Doc->FinishOperation ();
		OnStartEditing (dlg); // updates everything so that we really do incremental undo/redo
	}
	dlg->m_Node = NULL;
}

void gcpOrbitalProps::OnTypeChanged (gcpOrbitalProps *dlg, GtkToggleButton *btn)
{
	if (gtk_toggle_button_get_active (btn)) {
		gcpOrbitalType type = static_cast <gcpOrbitalType> (GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (btn), "orbital-type")));
		gtk_widget_set_sensitive (dlg->GetWidget ("rotation-btn"), type != GCP_ORBITAL_TYPE_S);
		gtk_widget_set_sensitive (dlg->GetWidget ("rotation-btn"), type != GCP_ORBITAL_TYPE_S);
		dlg->m_Orbital->SetType (type);
		dlg->m_Doc->GetView ()->Update (dlg->m_Orbital);
	}
}

void gcpOrbitalProps::OnCoefChanged (gcpOrbitalProps *dlg, GtkSpinButton *btn)
{
	dlg->m_Orbital->SetCoef (gtk_spin_button_get_value (btn));
	dlg->m_Doc->GetView ()->Update (dlg->m_Orbital);
}

void gcpOrbitalProps::OnRotationChanged (gcpOrbitalProps *dlg, GtkSpinButton *btn)
{
	dlg->m_Orbital->SetRotation (gtk_spin_button_get_value (btn));
	dlg->m_Doc->GetView ()->Update (dlg->m_Orbital);
}

////////////////////////////////////////////////////////////////////////////////
// class gcpOrbital: atomic orbitals

gcu::TypeId OrbitalType;

gcpOrbital::gcpOrbital (gcp::Atom *parent, gcpOrbitalType type):
	gcu::Object (OrbitalType),
	gcu::DialogOwner (),
	gccv::ItemClient (),
	m_Atom (parent),
	m_Type (type),
	m_Coef (1.),
	m_Rotation (0.)
{
	SetId ("orb1");
	if (parent)
		parent->AddChild (this);
}

gcpOrbital::~gcpOrbital ()
{
	gcu::Dialog *dlg = GetDialog ("orbital-properties");
	if (dlg)
		static_cast <gcpOrbitalProps *> (dlg)->m_Orbital = NULL;
}

void gcpOrbital::AddItem ()
{
	if (!m_Atom || m_Item)
		return;
	gcp::Document *doc = static_cast <gcp::Document *> (GetDocument ());
	gcp::Theme *theme = doc->GetTheme ();
	gcp::View *view = doc->GetView ();
	double zoom = theme->GetZoomFactor ();
	gccv::Group *group = static_cast <gccv::Group *> (m_Atom->GetItem ());
	if (!group) { // the atom is inside a fragment
		gcu::Object *parent = m_Atom->GetParent ();
		group = static_cast <gccv::Group *> (dynamic_cast <gccv::ItemClient *> (parent)->GetItem ());
	}
	switch (m_Type) {
	case GCP_ORBITAL_TYPE_S: {
		gccv::Circle *circle = new gccv::Circle (group, 0., 0., theme->GetBondLength () * fabs (m_Coef) * zoom / 2., this);
		circle->SetLineWidth (1.);
		circle->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		circle->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		m_Item = circle;
		group->MoveToBack (m_Item);
		break;
	}
	case GCP_ORBITAL_TYPE_P: {
		gccv::Group *new_group = new gccv::Group (group, this);
		gccv::Leaf *leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetRotation (m_Rotation / 180. * M_PI);
		leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
		leaf->SetRotation (m_Rotation / 180. * M_PI + M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (GO_COLOR_WHITE);
		m_Item = new_group;
		group->MoveToBack (m_Item);
		break;
	}
	case GCP_ORBITAL_TYPE_DXY: {
		gccv::Group *new_group = new gccv::Group (group, this);
		gccv::Leaf *leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetRotation ((m_Rotation / 180. + .25) * M_PI);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.25) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .75) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.75) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		m_Item = new_group;
		group->MoveToBack (m_Item);
		break;
	}
	case GCP_ORBITAL_TYPE_DZ2: {
		gccv::Group *new_group = new gccv::Group (group, this);
		gccv::Leaf *leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetRotation (m_Rotation / 180. * M_PI);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * GCP_ORBITAL_DZ2_FACTOR * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .5) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * GCP_ORBITAL_DZ2_FACTOR * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.5) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		m_Item = new_group;
		group->MoveToBack (m_Item);
		break;
	}
	default:
		// we might throw an exception here.
		break;
	}
}

xmlNodePtr gcpOrbital::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("orbital"), NULL);
	char *buf;
	switch (m_Type) {
	case GCP_ORBITAL_TYPE_S:
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> ("s"));
		break;
	case GCP_ORBITAL_TYPE_P:
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> ("p"));
		break;
	case GCP_ORBITAL_TYPE_DXY:
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> ("dxy"));
		break;
	case GCP_ORBITAL_TYPE_DZ2:
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> ("dz2"));
		break;
	default:
		break;
	}
	buf = g_strdup_printf ("%g", m_Coef);
	xmlNewProp (node, reinterpret_cast <xmlChar const *> ("coef"), reinterpret_cast <xmlChar *> (buf));
	g_free (buf);
	if (m_Rotation != 0. && m_Type != GCP_ORBITAL_TYPE_S) {
		buf = g_strdup_printf ("%g", m_Rotation);
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("rotation"), reinterpret_cast <xmlChar *> (buf));
		g_free (buf);
	}
	return node;
}

bool gcpOrbital::Load (xmlNodePtr node)
{
	m_Atom = dynamic_cast <gcp::Atom *> (GetParent ());
	char *buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("type")));
	if (buf) {
		if (!strcmp (buf, "s"))
			m_Type = GCP_ORBITAL_TYPE_S;
		else if (!strcmp (buf, "p"))
			m_Type = GCP_ORBITAL_TYPE_P;
		else if (!strcmp (buf, "dxy"))
			m_Type = GCP_ORBITAL_TYPE_DXY;
		else if (!strcmp (buf, "dz2"))
			m_Type = GCP_ORBITAL_TYPE_DZ2;
		xmlFree (buf);
	}
	buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("coef")));
	if (buf) {
		m_Coef = g_strtod (buf, NULL);
		xmlFree (buf);
	}
	buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("rotation")));
	if (buf) {
		m_Rotation = g_strtod (buf, NULL);
		xmlFree (buf);
	}
	GetDocument ()->ObjectLoaded (this);
	return true;
}

void gcpOrbital::SetSelected (int state)
{
	GOColor color;
	
	switch (state) {	
	case gcp::SelStateUnselected:
		color = GO_COLOR_BLACK;
		break;
	case gcp::SelStateSelected:
		color = gcp::SelectColor;
		break;
	case gcp::SelStateUpdating:
		color = gcp::AddColor;
		break;
	case gcp::SelStateErasing:
		color = gcp::DeleteColor;
		break;
	default:
		color = GO_COLOR_BLACK;
		break;
	}
	if (m_Type == GCP_ORBITAL_TYPE_S)
		static_cast <gccv::LineItem *> (m_Item)->SetLineColor (color);
	else {
		gccv::Group *group = static_cast <gccv::Group *> (m_Item);
		std::list<gccv::Item *>::iterator it;
		for (gccv::Item *item = group->GetFirstChild (it); item; item = group->GetNextChild (it))
			static_cast <gccv::LineItem *> (item)->SetLineColor (color);
	}
}

std::string gcpOrbital::Name ()
{
	return _("Orbital");
}

static void do_orbital_properties (gcu::Object *obj)
{
	obj->ShowPropertiesDialog ();
}

bool gcpOrbital::BuildContextualMenu (GtkUIManager *UIManager, gcu::Object *object, double x, double y)
{
	GtkActionGroup *group = gtk_action_group_new ("orbital");
	GtkAction *action = gtk_action_new ("Orbital", _("Orbital"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	action = gtk_action_new ("orbital-properties", _("Properties"), _("Orbital properties"), NULL);
	g_signal_connect_swapped (action, "activate", G_CALLBACK (do_orbital_properties), this);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Orbital'><menuitem action='orbital-properties'/></menu></popup></ui>", -1, NULL);
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
	gcu::Object::BuildContextualMenu (UIManager, object, x, y);
	return true;
}

char const *gcpOrbital::HasPropertiesDialog () const
{
	return "orbital-properties";
}

gcu::Dialog *gcpOrbital::BuildPropertiesDialog ()
{
	gcp::Document *doc = static_cast <gcp::Document *> (GetDocument ());
	gcu::Dialog *dlg = new gcpOrbitalProps (doc, this);
	return dlg;
}
