// -*- C++ -*-

/*
 * GChemPaint library
 * brackets.cc
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "brackets.h"
#include <gccv/canvas.h>
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/fragment.h>
#include <gcp/mechanism-step.h>
#include <gcp/reaction-step.h>
#include <gcp/settings.h>
#include <gcp/text.h>
#include <gcp/theme.h>
#include <gcp/tool.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcu/application.h>
#include <gcu/document.h>
#include <gcugtk/ui-manager.h>
#include <gccv/canvas.h>
#include <gccv/structs.h>
#include <sstream>
#include <cstring>
#include <glib/gi18n-lib.h>

namespace gcp {

extern xmlDocPtr pXmlDoc;

gcu::TypeId BracketsType = gcu::NoType;
static gcu::Object *last_loaded;

Brackets::Brackets (gccv::BracketsTypes type): gcu::Object (BracketsType), ItemClient ()
{
	SetId ("bk1");
	m_Type = type;
	m_Valid = false;
	m_Used = gccv::BracketsBoth;
	m_Content = BracketContentInvalid;
	m_Decorations = BracketDecorationNone;
}

Brackets::~Brackets ()
{
	while (!m_EmbeddedObjects.empty ())
		(*m_EmbeddedObjects.begin ())->Unlink (this);
}

void Brackets::OnUnlink (Object *object)
{
	m_EmbeddedObjects.erase (object);
}

void Brackets::OnLoaded ()
{
	if (last_loaded) {
		// this is NOT thread safe
		m_EmbeddedObjects.insert (last_loaded);
		last_loaded->Link (this);
		last_loaded = NULL;
	}
}

void Brackets::AddItem ()
{
	if (m_Item || m_EmbeddedObjects.empty ())
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	if (!m_FontDesc.length ()) {
		char *desc = pango_font_description_to_string (view->GetPangoFontDesc ());
		m_FontDesc = desc;
		g_free (desc);
	}
	gccv::Rect rect;
	if (m_EmbeddedObjects.size () == 1 && GetParent () == *m_EmbeddedObjects.begin ()) {
		std::map < std::string, gcu::Object * >::iterator i;
		rect.x0 = go_nan;
		gcu::Object *parent = GetParent (), *child;
		for (child = parent->GetFirstChild (i); child; child = parent->GetNextChild (i)) {
			if (child == this)
				continue;
			Brackets *br = dynamic_cast < Brackets * > (child);
			if (br && br->m_EmbeddedObjects.size () == 1 && parent == *br->m_EmbeddedObjects.begin ())
					continue;
			view->GetData ()->GetObjectBounds (child, rect);
		}
	} else
		view->GetData ()->GetObjectsBounds (m_EmbeddedObjects, &rect);
	gccv::Brackets *item = new gccv::Brackets (view->GetCanvas ()->GetRoot (), m_Type, m_Used, m_FontDesc.c_str (), rect.x0, rect.y0, rect.x1, rect.y1, this);
	item->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
	m_Item = item;
}

bool Brackets::Load (xmlNodePtr node)
{
	char *buf;
	gcu::Document *doc = GetDocument ();
	buf = reinterpret_cast <char *> (xmlGetProp (node, (xmlChar*) "type"));
	if (!buf)
		m_Type = gccv::BracketsTypeNormal;
	else if (!strcmp (buf, "square"))
		m_Type = gccv::BracketsTypeSquare;
	else if (!strcmp (buf, "curly"))
		m_Type = gccv::BracketsTypeCurly;
	else
		m_Type = gccv::BracketsTypeNormal;
	if (buf)
		xmlFree (buf);
	buf = reinterpret_cast <char *> (xmlGetProp (node, (xmlChar*) "objects"));
	if (buf) {
		char **ids = g_strsplit (buf, ",", -1);
		unsigned i = 0;
		while (ids[i])
			doc->SetTarget (ids[i++], &last_loaded, doc, this);
		g_strfreev (ids);
		xmlFree (buf);
	}
	return gcu::Object::Load (node);
}

xmlNodePtr Brackets::Save (xmlDocPtr xml) const
{
	if (m_EmbeddedObjects.size () == 0)
		return NULL;
	xmlNodePtr node = gcu::Object::Save (xml);//xmlNewDocNode (xml, NULL, (xmlChar*) "brackets", NULL);
	char const *type = NULL;
	switch (m_Type) {
	case gccv::BracketsTypeNormal:
	default:
		break;
	case gccv::BracketsTypeSquare:
		type = "square";
		break;
	case gccv::BracketsTypeCurly:
		type = "curly";
		break;
	}
	if (type)
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> (type));
	// now save embedded objects as a list of Ids
	std::set <gcu::Object *>::iterator i, end = m_EmbeddedObjects.end ();
	i = m_EmbeddedObjects.begin ();
	std::ostringstream str;
	str << (*i)->GetId ();
	for (i++ ; i != end; i++)
		str << "," << (*i)->GetId ();
	xmlNewProp (node, reinterpret_cast <xmlChar const *> ("objects"), reinterpret_cast <xmlChar const *> (str.str ().c_str ()));
	return node;
}

void Brackets::SetSelected (int state)
{
	GOColor color;
	switch (state) {
	case SelStateUnselected:
		color = GO_COLOR_BLACK;
		break;
	case SelStateSelected:
		color = SelectColor;
		break;
	case SelStateUpdating:
		color = AddColor;
		break;
	case SelStateErasing:
		color = DeleteColor;
		break;
	default:
		color = GO_COLOR_BLACK;
		break;
	}
	static_cast <gccv::Brackets *> (m_Item)->SetLineColor (color);
}

void Brackets::SetEmbeddedObjects (std::set < gcu::Object * > objects)
{
	// evaluate what objects are really there, and add links to them
	if (objects.size () == 0) // that case the brackets are not valid
		return;
	gcu::Object *obj;
	std::set  < gcu::Object * >::iterator i = objects.begin (),
									   end = objects.end ();
	std::set <gcu::TypeId> const &rules = GetApplication ()->GetRules (BracketsType, gcu::RuleMayContain);

	bool ok = true;
	if ((ok = objects.size () == 1)) {
		obj = *i;
		gcu::TypeId type = obj->GetType ();
		if (type == gcu::MoleculeType)
			m_Content = BracketContentMolecule;
		else if (type == gcp::ReactionStepType || type == gcp::MechanismStepType || rules.find (type) != rules.end ())
			m_Content = BracketContentGroup;
		else
			ok = false;
		if (m_Used == gccv::BracketsBoth)
			m_Decorations = BracketSuperscript;
	}
	if (!ok) {
		obj = (*i)->GetMolecule ();
		if (obj != NULL) {
			for (i++; i != end; i++)
				if ((*i)->GetMolecule () != obj)
					return;
			// now we need to test whether all selected atoms are connected
			if (!ConnectedAtoms (objects))
				return;
			m_Content = BracketContentFragment;
		} else
			return; // may be we are missing some cases where the enclosed group is valid
		if (m_Used == gccv::BracketsBoth)
			m_Decorations = BracketSubscript;
	}

	SetParent (obj);
	// unset existing links
	for (i= m_EmbeddedObjects.begin (), end = m_EmbeddedObjects.end (); i != end; i++)
		(*i)->Unlink (this);
	m_EmbeddedObjects = objects;
	// set new links
	for (i= m_EmbeddedObjects.begin (), end = m_EmbeddedObjects.end (); i != end; i++)
		(*i)->Link (this);
	m_Valid = true;
}

static void AddAtom (gcu::Atom const *atom, std::set < gcu::Object * > const &objects, std::set < gcu::Object const * > &test)
{
	test.insert (atom);
	std::map < gcu::Atom *, gcu::Bond *>::const_iterator i;
	gcu::Bond const *bond = atom->GetFirstBond (i);
	gcu::Atom const *atom1;
	std::set < gcu::Object * >::const_iterator end = objects.end ();
	while (bond) {
		atom1 = bond->GetAtom (atom);
		if (objects.find (const_cast < gcu::Bond * > (bond)) != end && test.find (bond) == test.end ()) {
			test.insert (bond);
			AddAtom (atom1, objects, test);
		}
		bond = atom->GetNextBond (i);
	}
}

bool Brackets::ConnectedAtoms (std::set < gcu::Object * > const &objects)
{
	// first find an atom
	std::set  < gcu::Object * >::iterator i, end = objects.end ();
	Atom const *atom = NULL;
	for (i = objects.begin (); i != end && atom == NULL; i++)
		switch ((*i)->GetType ()) {
		case gcu::AtomType:
			atom = static_cast < Atom const * > (*i);
			break;
		case gcu::FragmentType:
			atom = static_cast < Fragment const * > (*i)->GetAtom ();
			break;
		default:
			break;
		}
	if (atom == NULL)
		return false; // not really important
	std::set < gcu::Object const * > test;
	AddAtom (atom, objects, test);
	// now count brackets
	int nb = 0;
	for (i = objects.begin (); i != end; i++)
		if ((*i)->GetType () == BracketsType)
			nb++;
	return objects.size () == test.size () + nb;
}

static void on_stoichiometry_add (Brackets *brackets) {
	Document *pDoc = dynamic_cast <Document*> (brackets->GetDocument ());
	Application * pApp = pDoc->GetApplication ();
	View *pView = pDoc->GetView ();
	Theme *pTheme= pDoc->GetTheme ();
	WidgetData *pData = (WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
	gccv::Rect rect;
	Operation *op = pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
	op->AddNode (brackets->GetGroup ()->Save (pXmlDoc), 0);
	pData->GetObjectBounds (brackets, &rect);
	double x = rect.x1 / pTheme->GetZoomFactor (),
		y = (rect.y1 + pTheme->GetFontSize () / 3. / PANGO_SCALE) / pTheme->GetZoomFactor ();
	Text *text = new Text ((StoichiometryTag)? StoichiometryTag:(StoichiometryTag = gccv::TextTag::RegisterTagType ()), x, y);
	text->SetAnchor (gccv::AnchorSouthWest);
	brackets->AddChild (text);
	pDoc->AddObject (text);
	Tool *tool = pApp->GetTool ("Text");
	brackets->GetParent ()->EmitSignal (OnChangedSignal);
	pApp->ActivateTool ("Text", true);
	tool->OnClicked (pView, text, rect.x1, rect.y1, 0);
}

static void on_superscript_add (Brackets *brackets) {
	Document *pDoc = dynamic_cast <Document*> (brackets->GetDocument ());
	Application * pApp = pDoc->GetApplication ();
	View *pView = pDoc->GetView ();
	Theme *pTheme= pDoc->GetTheme ();
	WidgetData *pData = (WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
	gccv::Rect rect;
	Operation *op = pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
	op->AddNode (brackets->GetGroup ()->Save (pXmlDoc), 0);
	pData->GetObjectBounds (brackets, &rect);
	double x = rect.x1 / pTheme->GetZoomFactor (),
		y = rect.y0 / pTheme->GetZoomFactor ();
	Text *text = new Text (x, y);
	text->SetAnchor (gccv::AnchorNorthWest);
	brackets->AddChild (text);
	pDoc->AddObject (text);
	Tool *tool = pApp->GetTool ("Text");
	brackets->GetParent ()->EmitSignal (OnChangedSignal);
	pApp->ActivateTool ("Text", true);
	tool->OnClicked (pView, text, rect.x1, rect.y0, 0);
}

bool Brackets::BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y)
{
	bool result = false;
	if (!HasChildren () && m_Decorations != BracketDecorationNone && m_Used == gccv::BracketsBoth) {
		result = true;
		if (m_Decorations & BracketSubscript) {
			GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
			GtkActionGroup *group = gtk_action_group_new ("bracket");
			GtkAction *action;
			action = gtk_action_new ("Brackets", _("Brackets"), NULL, NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			action = gtk_action_new ("Stoich", _("Add stoichiometry"), NULL, NULL);
			g_signal_connect_swapped (action, "activate", G_CALLBACK (on_stoichiometry_add), this);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Brackets'><menuitem action='Stoich'/></menu></popup></ui>", -1, NULL);
			gtk_ui_manager_insert_action_group (uim, group, 0);
			g_object_unref (group);
		} else if (m_Decorations & BracketSuperscript) {
			GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
			GtkActionGroup *group = gtk_action_group_new ("bracket");
			GtkAction *action;
			action = gtk_action_new ("Brackets", _("Brackets"), NULL, NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			action = gtk_action_new ("Super", _("Add superscript"), NULL, NULL);
			g_signal_connect_swapped (action, "activate", G_CALLBACK (on_superscript_add), this);
//			gtk_action_group_add_action (group, action);
			g_object_unref (action);
//			gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Brackets'><menuitem action='Super'/></menu></popup></ui>", -1, NULL);
			gtk_ui_manager_insert_action_group (uim, group, 0);
			g_object_unref (group);
			result = false; // remove when we support superscripts
		}
	}
	return result || Object::BuildContextualMenu (UIManager, object, x, y);
}

}
