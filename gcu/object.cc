/* 
 * Gnome Chemistry Utils
 * object.cc 
 *
 * Copyright (C) 2002-2004
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "object.h"
#include "document.h"
#include <string>
#include <iostream>
#include <vector>

using namespace gcu;

Object::Object(TypeId Id)
{
	m_Type = Id;
	m_Id = NULL;
	m_Parent = NULL;
	m_Locked = 0;
}

Object::~Object()
{
	if (m_Id)
	{
		if (m_Parent) m_Parent->m_Children.erase(m_Id);
		g_free(m_Id);
	}
	map<string, Object*>::iterator i;
	while(!m_Children.empty())
	{
		i = m_Children.begin();
		if (m_Parent) m_Parent->AddChild((*i).second);
		else {
			(*i).second->m_Parent = NULL;
			delete (*i).second;
			m_Children.erase ((*i).first);
		}
	}
}


void Object::SetId (gchar* Id)
{
	if (!Id)
		return;
	if (m_Id) {
		if (!strcmp (Id, m_Id))
			return;
		if (m_Parent) m_Parent->m_Children.erase (m_Id);
		g_free(m_Id);
	}
	m_Id = g_strdup (Id);
	if (m_Parent) {
		Object *parent = m_Parent;
		m_Parent = NULL;
		parent->AddChild (this);
	}
}

Object* Object::GetMolecule()
{
	Object* object = this;
	while (object && (object->m_Type != MoleculeType)) object = object->m_Parent;
	return object;
}

Object* Object::GetReaction()
{
	Object* object = this;
	while (object && (object->m_Type != ReactionType)) object = object->m_Parent;
	return object;
}

Object* Object::GetGroup()
{
	if (!m_Parent || m_Parent->GetType () == DocumentType) return NULL;
	Object* object = m_Parent;
	while (object->m_Parent->GetType () != DocumentType)
		object = object->m_Parent;
	return object;
}

Document* Object::GetDocument()
{
	Object* object = this;
	while (object && (object->m_Type != DocumentType)) object = object->m_Parent;
	return (Document*) object;
}

Object* Object::GetParentOfType(TypeId Id)
{
	Object* object = this;
	while (object && (object->m_Type != Id)) object = object->m_Parent;
	return object;
}

void Object::AddChild(Object* object)
{
	Document* pDoc = GetDocument();
	if (!pDoc)
	{
		cerr << "Cannot add an object outside a document" << endl;
	}
	if (object->m_Id == NULL)
	{
		int i = 1;
		char szId[16];
		while (snprintf(szId, sizeof(szId), "o%d", i++), pDoc->GetDescendant(szId) != NULL);
		object->m_Id = g_strdup(szId);
	}
	else
	{
		Object* o = pDoc->RealGetDescendant(object->m_Id);
		if (o && ((pDoc != object->GetDocument()) || (object != o)))
		{
			gchar *buf = pDoc->GetNewId (object->m_Id);
			 if (object->m_Parent) {
				object->m_Parent->m_Children.erase (object->m_Id);
				object->m_Parent = NULL;
			}
			g_free(object->m_Id);
			object->m_Id = g_strdup(buf);
			delete [] buf;
		}
	}
	if (object->m_Parent)
	{
		object->m_Parent->m_Children.erase(object->m_Id);
		object->m_Parent = NULL;
	}
	object->m_Parent = this;
	m_Children[object->m_Id] = object;
}

void Object::SetParent(Object* Parent)
{
	if (Parent) Parent->AddChild(this);
	else
	{
		if (m_Parent) m_Parent->m_Children.erase(m_Id); 
		m_Parent = NULL;
	}
}

Object* Object::GetChild(const gchar* Id)
{
	if (Id == NULL) return NULL;
	Object* object = m_Children[Id];
	if (!object) m_Children.erase(Id);
	return object;
}

Object* Object::GetDescendant(const gchar* Id)
{
	if (Id == NULL) return NULL;
	Document* pDoc = GetDocument();
	string sId = pDoc->GetTranslatedId (Id);
	if (sId.size()) Id = sId.c_str();
	else pDoc->EraseTranslationId(Id);
	return RealGetDescendant (Id);
}

Object* Object::RealGetDescendant(const gchar* Id)
{
	Object* object = m_Children[Id];
	if (!object)
	{
		m_Children.erase (Id);
		map<string, Object*>::iterator i;
		for (i = m_Children.begin (); i != m_Children.end (); i++)
			if ((*i).second->HasChildren () && (object = (*i).second->RealGetDescendant (Id)))
				break;
	}
	return object;
}

Object* Object::GetFirstChild(map<string, Object*>::iterator& i)
{
	i = m_Children.begin();
	if (i == m_Children.end()) return NULL;
	return (*i).second;
}
	
Object* Object::GetNextChild(map<string, Object*>::iterator& i)
{
	i++;
	if (i == m_Children.end()) return NULL;
	return (*i).second;
}

xmlNodePtr Object::Save (xmlDocPtr xml)
{
	xmlNodePtr node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) GetTypeName (m_Type).c_str (), NULL);
	if (!node) return NULL;
	SaveId (node);
	
	if (!SaveChildren (xml, node))
	{
		xmlFreeNode (node);
		return NULL;
	}
	return node;
}

void Object::SaveId(xmlNodePtr node)
{
	if (m_Id && *m_Id) xmlNewProp(node, (xmlChar*)"id", (xmlChar*)m_Id);
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
		pObject = CreateObject ((const char*) child->name, this);
		if (pObject) {
			if (!pObject->Load (child))
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

bool Object::SaveChildren(xmlDocPtr xml, xmlNodePtr node)
{
	map<string, Object*>::iterator i;
	xmlNodePtr child;
	for (i = m_Children.begin(); i != m_Children.end(); i++)
	{
		if ((child = (*i).second->Save(xml))) xmlAddChild(node, child);
		else return false;
	}
	return true;
}

xmlNodePtr Object::GetNodeByProp(xmlNodePtr root, char* Property, char* Id)
{
	return GetNextNodeByProp(root->children, Property, Id);
}

xmlNodePtr Object::GetNextNodeByProp(xmlNodePtr node, char* Property, char* Id)
{
	char *txt;
	while (node)
	{
		txt = (char*)xmlGetProp(node, (xmlChar*)Property);
		if (!strcmp(txt, Id)) break;
		node = node ->next;
	}
	return node;
}

xmlNodePtr Object::GetNodeByName(xmlNodePtr root, char* Name)
{
	return GetNextNodeByName(root->children, Name);
}

xmlNodePtr Object::GetNextNodeByName(xmlNodePtr node, char* Name)
{
	while (node)
	{
		if (!strcmp((char*)node->name, Name)) break;
		node = node ->next;
	}
	return node;
}

void Object::Move(double x, double y, double z)
{
	map<string, Object*>::iterator i;
	for (i = m_Children.begin(); i != m_Children.end(); i++) (*i).second->Move(x, y, z);
}

void Object::Transform2D(Matrix2D& m, double x, double y)
{
	map<string, Object*>::iterator i;
	for (i = m_Children.begin(); i != m_Children.end(); i++) (*i).second->Transform2D(m, x, y);
}

bool Object::BuildContextualMenu (GtkUIManager *UIManager, Object *object)
{
	return (m_Parent)? m_Parent->BuildContextualMenu (UIManager, object): false;
}

void Object::Add(GtkWidget* w)
{
}

void Object::Print(GnomePrintContext *pc)
{
}

void Object::Update(GtkWidget* w)
{
	map<string, Object*>::iterator i;
	for (i = m_Children.begin(); i != m_Children.end(); i++) (*i).second->Update(w);
}

void Object::SetSelected(GtkWidget* w, int state)
{
	map<string, Object*>::iterator i;
	for (i = m_Children.begin(); i != m_Children.end(); i++) (*i).second->SetSelected(w, state);
}

Object* Object::GetAtomAt(double x, double y, double z)
{
	return NULL;
}

bool Object::Build (list<Object*>& Children) throw (invalid_argument)
{
	return false;
}

double Object::GetYAlign ()
{
	return 0.0;
}

static TypeId NextType = OtherType;

class TypeDesc
{
public:
	TypeDesc ();

	TypeId Id;
	Object* (*Create) ();
	set <TypeId> PossibleChildren;
	set <TypeId> PossibleParents;
	set <TypeId> RequiredChildren;
	set <TypeId> RequiredParents;
	string CreationLabel;
};

TypeDesc::TypeDesc ()
{
	Id = NoType;
	Create = NULL;
}

static map<string, TypeDesc> Types;
static vector<string> TypeNames;

TypeId Object::AddType(string TypeName, Object*(*Create)(), TypeId id)
{
	TypeDesc& typedesc = Types[TypeName];
	typedesc.Create = Create;
	if (id == OtherType) {
		typedesc.Id = NextType;
		NextType = TypeId ((unsigned) NextType + 1);
	} else
		typedesc.Id = id;
	if (TypeNames.capacity() <=  typedesc.Id) {
		size_t max = (((size_t)  typedesc.Id / 10) + 1) * 10;
		TypeNames.reserve (max);
		while (max > TypeNames.size())
			TypeNames.push_back ("");
	}
#if HAS_VECTOR_AT
	TypeNames.at (id) = TypeName;
#else
	vector<string>::iterator it;
	it = TypeNames.begin ();
	it += typedesc.Id;
	TypeNames.insert (it, TypeName);
#endif
	return typedesc.Id;
}

Object* Object::CreateObject(const string& TypeName, Object* parent)
{
	TypeDesc& typedesc = Types[TypeName];
	Object* pObj = (typedesc.Create)? typedesc.Create(): NULL;
	if (parent && pObj) {
		if (pObj->m_Id) {
			char* newId = parent->GetDocument()->GetNewId (pObj->m_Id, false);
			pObj->SetId (newId);
			delete [] newId;
		}
		parent->AddChild(pObj);
	}
	return pObj;
}

TypeId Object::GetTypeId (const string& Name)
{
	TypeDesc& typedesc = Types[Name];
	TypeId res = typedesc.Id;
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
	if (!type1.size() || !type2.size ())
		return;
	TypeDesc& typedesc1 = Types[type1];
	if (typedesc1.Id == NoType) {
		Types.erase (type1);
		return;
	}
	TypeDesc& typedesc2 = Types[type2];
	if (typedesc2.Id == NoType) {
		Types.erase (type2);
		return;
	}
	switch (rule) {
		case RuleMustContain:
			typedesc1.RequiredChildren.insert (typedesc2.Id);
		case RuleMayContain:
			typedesc1.PossibleChildren.insert (typedesc2.Id);
			typedesc2.PossibleParents.insert (typedesc1.Id);
			break;
		case RuleMustBeIn:
			typedesc1.RequiredParents.insert (typedesc2.Id);
		case RuleMayBeIn:
			typedesc2.PossibleChildren.insert (typedesc1.Id);
			typedesc1.PossibleParents.insert (typedesc2.Id);
			break;
	}
}

const set<TypeId>& Object::GetRules (TypeId type, RuleId rule)
{
	return GetRules (TypeNames[type], rule);
}

const set<TypeId>& Object::GetRules (const string& type, RuleId rule)
{
	static set<TypeId> noId;
	TypeDesc& typedesc = Types[type];
	switch (rule) {
	case RuleMustContain:
		return typedesc.RequiredChildren;
	case RuleMayContain:
		return typedesc.PossibleChildren;
	case RuleMustBeIn:
		return typedesc.RequiredParents;
	case RuleMayBeIn:
		return typedesc.PossibleParents;
	default:
		return noId;
	}
	return noId;
}

static void AddAncestorTypes (TypeId type, set<TypeId>& types)
{
	const set<TypeId>& new_types = Object::GetRules (type, RuleMayBeIn);
	set<TypeId>::iterator i = new_types.begin (), end = new_types.end ();
	for (; i != end; i++) {
		types.insert (*i);
		AddAncestorTypes (*i, types);
	}
}

void Object::GetPossibleAncestorTypes (set<TypeId>& types)
{
	AddAncestorTypes (m_Type, types);
}

void Object::SetCreationLabel (TypeId Id, string Label)
{
	TypeDesc& type = Types[TypeNames[Id]];
	type.CreationLabel = Label;
}

const string& Object::GetCreationLabel (TypeId Id)
{
	return Types[TypeNames[Id]].CreationLabel;
}

const string& Object::GetCreationLabel (const string& TypeName)
{
	return Types[TypeName].CreationLabel;
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

bool Object::OnSignal (SignalId Signal, Object *Child)
{
	return true;
}

Object* Object::GetFirstLink (set<Object*>::iterator& i)
{
	i = m_Links.begin();
	if (i == m_Links.end()) return NULL;
	return *i;
}

Object* Object::GetNextLink(set<Object*>::iterator& i)
{
	i++;
	if (i == m_Links.end()) return NULL;
	return *i;
}

void Object::Unlink (Object *object)
{
	m_Links.erase (object);
	OnUnlink (object);
}

void Object::OnUnlink (Object *object)
{
}

void Object::Lock (bool state)
{
	if (state)
		m_Locked++;
	else if (m_Locked > 0)
		m_Locked--;
}
