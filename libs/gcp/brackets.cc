// -*- C++ -*-

/* 
 * GChemPaint library
 * brackets.cc
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "brackets.h"
#include <gccv/canvas.h>
#include <gcp/document.h>
#include <gcp/mechanism-step.h>
#include <gcp/reaction-step.h>
#include <gcp/settings.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcu/application.h>
#include <gcu/document.h>
#include <gccv/canvas.h>
#include <gccv/structs.h>
#include <sstream>
#include <cstring>

namespace gcp {

gcu::TypeId BracketsType = gcu::NoType;
static gcu::Object *last_loaded;

Brackets::Brackets (gccv::BracketsTypes type): gcu::Object (BracketsType), ItemClient ()
{
	m_Type = type;
	m_Valid = false;
	m_Content = BracketContentInvalid;
}

Brackets::~Brackets ()
{
}

void Brackets::OnLoaded ()
{
	if (last_loaded) {
		// this is NOT thread safe
		m_EmbeddedObjects.insert (last_loaded);
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
	view->GetData ()->GetObjectsBounds (m_EmbeddedObjects, &rect);
	gccv::Brackets *item = new gccv::Brackets (view->GetCanvas ()->GetRoot (), m_Type, m_Used, m_FontDesc.c_str (), rect.x0, rect.y0, rect.x1, rect.y1);
	item->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
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
	return true;
}

xmlNodePtr Brackets::Save (xmlDocPtr xml) const
{
	if (m_EmbeddedObjects.size () == 0)
		return NULL;
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) "brackets", NULL);
	SaveId (node);
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
	std::set <gcu::Object const *>::iterator i, end = m_EmbeddedObjects.end ();
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
	static_cast <gccv::Brackets *> (m_Item)->SetColor (color);
}

void Brackets::SetEmbeddedObjects (std::set <gcu::Object const *> objects)
{
	// evaluate what objects are really there, and add links to them
	if (objects.size () == 0) // that case the brackets are not valid
		return;
	gcu::Object const *obj;
	std::set <gcu::Object const *>::iterator i = objects.begin (),
									   end = objects.end ();
	std::set <gcu::TypeId> const &rules = GetApplication ()->GetRules (BracketsType, gcu::RuleMayContain);


	if (objects.size () == 1) {
		obj = *i;
		gcu::TypeId type = obj->GetType ();
		if (type == gcu::MoleculeType)
			m_Content = BracketContentMolecule;
		else if (type == gcp::ReactionStepType || type == gcp::MechanismStepType || rules.find (type) != rules.end ())
			m_Content = BracketContentGroup;
		else
			return;
		m_Decorations = BracketSuperscript;
	} else {
		obj = (*i)->GetMolecule ();
		if (obj != NULL) {
			for (i++; i != end; i++)
				if ((*i)->GetMolecule () != obj)
					return;
			// now we need to test whether all selected atoms are connected (is this true?)
			m_Content = BracketContentFragment;
		} else
			return; // may be we are missing some cases where the enclosed group is valid
		m_Decorations = BracketSubscript;
	}
	m_EmbeddedObjects = objects;
	m_Valid = true;
}

}
