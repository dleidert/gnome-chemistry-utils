/* 
 * Gnome Chemistry Utils
 * object.cc 
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "object.h"
#include "objprops.h"
#include "application.h"
#include "dialog.h"
#include "document.h"
#include <glib/gi18n.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

namespace gcu
{

TypeDesc::TypeDesc ()
{
	Id = NoType;
	Create = NULL;
}

static map<string, TypeId> Types;
static vector<string> TypeNames;

Object::Object (TypeId Id):
	m_Dirty (false)
{
	m_Type = Id;
	m_Id = NULL;
	m_Parent = NULL;
	m_Locked = 0;
	m_TypeDesc = NULL;
}

Object::~Object ()
{
	if (m_Id) {
		if (m_Parent) {
			Document *doc = GetDocument ();
			if (doc)
				doc->m_DirtyObjects.erase (this);
			m_Parent->m_Children.erase (m_Id);
		}
		g_free (m_Id);
	}
	map<string, Object*>::iterator i;
	while (!m_Children.empty ()) {
		i = m_Children.begin ();
		if (m_Parent)
			m_Parent->AddChild ((*i).second);
		else {
			(*i).second->m_Parent = NULL;
			delete (*i).second;
			m_Children.erase ((*i).first);
		}
	}
	while (!m_Links.empty ())
		Unlink (*(m_Links.begin ()));
	if (m_Parent && !m_Parent->HasChildren ())
		m_Parent->NotifyEmpty ();
}

void Object::Clear ()
{
	map<string, Object*>::iterator i;
	while (!m_Children.empty ()) {
		(*i).second->m_Parent = NULL;
		delete (*i).second;
		m_Children.erase ((*i).first);
	}
}

void Object::SetId (gchar const *Id)
{
	if (!Id)
		return;
	if (m_Id) {
		if (!strcmp (Id, m_Id))
			return;
		if (m_Parent)
			m_Parent->m_Children.erase (m_Id);
		g_free(m_Id);
	}
	m_Id = g_strdup (Id);
	if (m_Parent) {
		Object *parent = m_Parent;
		m_Parent = NULL;
		parent->AddChild (this);
	}
}

Object* Object::GetMolecule () const
{
	Object const *object = this;
	while (object && (object->m_Type != MoleculeType))
		object = object->m_Parent;
	return const_cast <Object *> (object);
}

Object* Object::GetReaction () const
{
	Object const *object = this;
	while (object && (object->m_Type != ReactionType))
		object = object->m_Parent;
	return const_cast <Object *> (object);
}

Object* Object::GetGroup () const
{
	if (!m_Parent || m_Parent->GetType () == DocumentType)
		return NULL;
	Object const *object = this;
	while (object->m_Parent->GetType () != DocumentType)
		object = object->m_Parent;
	return const_cast <Object *> (object);
}

Document* Object::GetDocument () const
{
	Object const *object = this;
	while (object && (object->m_Type != DocumentType))
		object = object->m_Parent;
	return const_cast <Document *> (reinterpret_cast <Document const *> (object));
}

Application* Object::GetApplication () const
{
	Document *doc = GetDocument ();
	return (doc)? doc->GetApp (): Application::GetDefaultApplication ();
}

Object* Object::GetParentOfType (TypeId Id) const
{
	Object const *object = this;
	while (object && (object->m_Type != Id))
		object = object->m_Parent;
	return const_cast <Object *> (object);
}

void Object::AddChild (Object* object)
{
	if (this == object->m_Parent)
		return;
	Document* pDoc = GetDocument ();
	if (!pDoc)
		cerr << "Cannot add an object outside a document" << endl;
	if (object->m_Id == NULL) {
		int i = 1;
		char szId[16];
		while (snprintf (szId, sizeof(szId), "o%d", i++), pDoc->GetDescendant (szId) != NULL) ;
		object->m_Id = g_strdup (szId);
	} else {
		Object* o = pDoc->RealGetDescendant (object->m_Id);
		if (o && ((pDoc != object->GetDocument()) || (object != o))) {
			gchar *buf = pDoc->GetNewId (object->m_Id);
			 if (object->m_Parent) {
				object->m_Parent->m_Children.erase (object->m_Id);
				object->m_Parent = NULL;
			}
			g_free (object->m_Id);
			object->m_Id = g_strdup (buf);
			delete [] buf;
		}
	}
	if (object->m_Parent) {
		object->m_Parent->m_Children.erase (object->m_Id);
		object->m_Parent = NULL;
	}
	object->m_Parent = this;
	m_Children[object->m_Id] = object;
	if (object->m_TypeDesc == NULL) {
		Application *App = pDoc->GetApp ();
		if (App)
			object->m_TypeDesc = App->GetTypeDescription (object->m_Type);
	}
}

void Object::SetParent (Object* Parent)
{
	if (Parent)
		Parent->AddChild(this);
	else {
		if (m_Parent) {
			Document *doc = GetDocument ();
			if (doc)
				doc->m_DirtyObjects.erase (this);
			m_Parent->m_Children.erase (m_Id);
		}
		m_Parent = NULL;
	}
}

Object* Object::GetChild (const gchar* Id) const
{
	if (Id == NULL)
		return NULL;
	map<string, Object*>::const_iterator i;
	i = m_Children.find (Id);
	return (i != m_Children.end ())? (*i).second: NULL;
}

Object* Object::GetDescendant (const gchar* Id) const
{
	if (Id == NULL)
		return NULL;
	Document* pDoc = GetDocument();
	string sId = pDoc->GetTranslatedId (Id);
	if (sId.size ())
		Id = sId.c_str ();
	else
		pDoc->EraseTranslationId (Id);
	return RealGetDescendant (Id);
}

Object* Object::RealGetDescendant (const gchar* Id) const
{
	map<string, Object*>::const_iterator i;
	Object *object = NULL;
	i = m_Children.find (Id);
	if (i == m_Children.end ()) {
		map<string, Object*>::const_iterator i, end = m_Children.end ();
		for (i = m_Children.begin (); i != end; i++)
			if ((*i).second->HasChildren () && (object = (*i).second->RealGetDescendant (Id)))
				break;
	} else
		object = (*i).second;
	return object;
}

Object *Object::GetFirstChild (map<string, Object*>::iterator& i)
{
	i = m_Children.begin ();
	if (i == m_Children.end ())
		return NULL;
	return (*i).second;
}

Object const *Object::GetFirstChild (map<string, Object*>::const_iterator& i) const
{
	i = m_Children.begin ();
	if (i == m_Children.end ())
		return NULL;
	return (*i).second;
}
	
Object *Object::GetNextChild (map<string, Object*>::iterator& i)
{
	i++;
	if (i == m_Children.end ())
		return NULL;
	return (*i).second;
}
	
Object const *Object::GetNextChild (map<string, Object*>::const_iterator& i) const
{
	i++;
	if (i == m_Children.end ())
		return NULL;
	return (*i).second;
}

xmlNodePtr Object::Save (xmlDocPtr xml) const
{
	xmlNodePtr node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) GetTypeName (m_Type).c_str (), NULL);
	if (!node)
		return NULL;
	SaveId (node);
	
	if (!SaveChildren (xml, node)) {
		xmlFreeNode (node);
		return NULL;
	}
	return node;
}

void Object::SaveId (xmlNodePtr node) const
{
	if (m_Id && *m_Id)
		xmlNewProp (node, (xmlChar*) "id", (xmlChar*) m_Id);
}

bool Object::Load (xmlNodePtr node)
{
	xmlChar* tmp;
	xmlNodePtr child;
	Object* pObject;

	m_Locked++;
	tmp = xmlGetProp (node, (xmlChar*) "id");
	if (tmp) {
		SetId ((char*) tmp);
		xmlFree (tmp);
	}
	child = node->children;
	while (child) {
		if (!strcmp ((const char*) child->name, "text") && !child->children) {
			child = child->next;
			continue;
		}
		node = (strcmp ((const char*) child->name, "object"))? child: child->children;
		pObject = CreateObject ((const char*) node->name, this);
		if (pObject) {
			if (!pObject->Load (node))
				delete pObject;
		} else {
			m_Locked--;
			return false;
		}
		child = child->next;
	}
	m_Locked--;
	return true;
}

bool Object::SaveChildren (xmlDocPtr xml, xmlNodePtr node) const
{
	map<string, Object*>::const_iterator i, end = m_Children.end ();
	xmlNodePtr child;
	for (i = m_Children.begin (); i != end; i++) {
		if ((child = (*i).second->Save (xml)))
			xmlAddChild (node, child);
		else
			return false;
	}
	return true;
}

xmlNodePtr Object::GetNodeByProp (xmlNodePtr root, char const *Property, char const *Id)
{
	return GetNextNodeByProp (root->children, Property, Id);
}

xmlNodePtr Object::GetNextNodeByProp (xmlNodePtr node, char const *Property, char const *Id)
{
	char *txt;
	while (node) {
		txt = (char*) xmlGetProp (node, (xmlChar*) Property);
		if (!strcmp (txt, Id))
			break;
		node = node->next;
	}
	return node;
}

xmlNodePtr Object::GetNodeByName (xmlNodePtr root, char const *Name)
{
	return GetNextNodeByName (root->children, Name);
}

xmlNodePtr Object::GetNextNodeByName (xmlNodePtr node, char const *Name)
{
	while (node) {
		if (!strcmp ((char*) node->name, Name))
			break;
		node = node ->next;
	}
	return node;
}

void Object::Move (double x, double y, double z)
{
	map<string, Object*>::iterator i, end = m_Children.end ();
	for (i = m_Children.begin (); i != end; i++)
		(*i).second->Move (x, y, z);
}

bool Object::GetCoords (G_GNUC_UNUSED double *x, G_GNUC_UNUSED double *y, G_GNUC_UNUSED double *z) const
{
	return false;
}

void Object::Transform2D(Matrix2D& m, double x, double y)
{
	map<string, Object*>::iterator i, end = m_Children.end ();
	for (i = m_Children.begin (); i != end; i++)
		(*i).second->Transform2D (m, x, y);
}

bool Object::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	Application *app = GetApplication ();
	bool result = (app)? app->BuildObjectContextualMenu (this, UIManager, object, x, y): false;
	return result | ((m_Parent)? m_Parent->BuildContextualMenu (UIManager, object, x, y): false);
}

char const *Object::HasPropertiesDialog () const
{
	return NULL;
}

Dialog *Object::BuildPropertiesDialog ()
{
	return NULL;
}

void Object::ShowPropertiesDialog ()
{
	const char *dlg_name = HasPropertiesDialog ();
	if (!dlg_name)
		return;
	DialogOwner *owner = dynamic_cast <DialogOwner *> (this);
	if (!owner)
		return;
	Dialog *dlg = owner->GetDialog (dlg_name);
	if (!dlg)
		dlg = BuildPropertiesDialog ();
	if (dlg)
		dlg->Present ();
	
}

Object* Object::GetAtomAt (G_GNUC_UNUSED double x, G_GNUC_UNUSED double y, G_GNUC_UNUSED double z)
{
	return NULL;
}

bool Object::Build (G_GNUC_UNUSED list<Object*>& Children) throw (invalid_argument)
{
	return false;
}

double Object::GetYAlign ()
{
	return 0.0;
}

TypeId Object::AddType (string TypeName, Object* (*Create) (), TypeId id)
{
	return Application::GetDefaultApplication ()->AddType (TypeName, Create, id);
}

void Object::AddAlias (TypeId id, std::string TypeName)
{
	if (TypeNames.size () <= id) {
		size_t max = (((size_t) id / 10) + 1) * 10;
		TypeNames.resize (max);
		TypeNames[id] = TypeName;
	} else {
		string &name = TypeNames[id];
		if (name.length () == 0)
			TypeNames[id] = TypeName;
	}
	Types[TypeName] = id;
}

Object* Object::CreateObject (const string& TypeName, Object* parent)
{
	Application *app = (parent)? parent->GetApplication (): NULL;
	return (app)? app->CreateObject (TypeName, parent): Application::GetDefaultApplication ()->CreateObject (TypeName, parent);
}

TypeId Object::GetTypeId (const string& Name)
{
	TypeId res = Types[Name];
	if (res == NoType)
		Types.erase (Name);
	return res;
}

string Object::GetTypeName (TypeId Id)
{
	return TypeNames[Id];
}

void Object::AddRule (TypeId type1, RuleId rule, TypeId type2)
{
	AddRule (TypeNames[type1], rule, TypeNames[type2]);
}

void Object::AddRule (const string& type1, RuleId rule, const string& type2)
{
	Application::GetDefaultApplication ()->AddRule (type1, rule, type2);
}

const set<TypeId>& Object::GetRules (TypeId type, RuleId rule)
{
	return GetRules (TypeNames[type], rule);
}

const set<TypeId>& Object::GetRules (const string& type, RuleId rule)
{
	return Application::GetDefaultApplication ()->GetRules (type, rule);
}

static void AddAncestorTypes (Application *app, TypeId type, set<TypeId>& types)
{
	const set<TypeId>& new_types = app->GetRules (type, RuleMayBeIn);
	set<TypeId>::iterator i = new_types.begin (), end = new_types.end ();
	for (; i != end; i++) {
		types.insert (*i);
		AddAncestorTypes (app, *i, types);
	}
}

void Object::GetPossibleAncestorTypes (set<TypeId>& types) const
{
	AddAncestorTypes (GetApplication (), m_Type, types);
}

void Object::SetCreationLabel (TypeId Id, string Label)
{
	Application::GetDefaultApplication ()->SetCreationLabel (Id, Label);
}

const string& Object::GetCreationLabel (TypeId Id)
{
	return Application::GetDefaultApplication ()->GetCreationLabel (Id);
}

const string& Object::GetCreationLabel (const string& TypeName)
{
	return Application::GetDefaultApplication ()->GetCreationLabel (TypeName);
}

static SignalId NextSignal = 0;

SignalId Object::CreateNewSignalId ()
{
	return NextSignal++;
}

void Object::EmitSignal (SignalId Signal)
{
	Object *obj = NULL;
	Object *ancestor = this;
	while (ancestor && !ancestor->IsLocked () && ancestor->OnSignal (Signal, obj)) {
		obj = ancestor;
		ancestor = obj->m_Parent;
	}
}

bool Object::OnSignal (G_GNUC_UNUSED SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	return true;
}

Object* Object::GetFirstLink (set<Object*>::iterator& i)
{
	i = m_Links.begin ();
	if (i == m_Links.end ())
		return NULL;
	return *i;
}

Object* Object::GetNextLink (set<Object*>::iterator& i)
{
	i++;
	if (i == m_Links.end ())
		return NULL;
	return *i;
}

void Object::Link (Object *object)
{
	m_Links.insert (object);
}

void Object::Unlink (Object *object)
{
	m_Links.erase (object);
	object->OnUnlink (this);
}

void Object::OnUnlink (G_GNUC_UNUSED Object *object)
{
}

void Object::Lock (bool state)
{
	if (state)
		m_Locked++;
	else if (m_Locked > 0)
		m_Locked--;
}

void Object::AddMenuCallback (TypeId Id, BuildMenuCb cb)
{
	Application::GetDefaultApplication ()->AddMenuCallback (Id, cb);
}

bool Object::SetProperty (G_GNUC_UNUSED unsigned property, G_GNUC_UNUSED char const *value)
{
	return true;
}

string Object::GetProperty (unsigned property) const
{
	switch (property) {
	case GCU_PROP_ID:
		return (m_Id)? m_Id: "xxx";
	default:
		break;
	}
	return "";
}

void Object::OnLoaded ()
{
}

void Object::SetDirty (bool dirty)
{
	m_Dirty = dirty;
	if (dirty) {
		Document *doc = GetDocument ();
		if (doc)
			doc->NotifyDirty (this);
	}
}

std::string Object::Identity ()
{
	return Name () + " " + GetId ();
}

std::string Object::Name ()
{
	return _("Object");
}

}	//	namespace gcu
