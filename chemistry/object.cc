/* 
 * Gnome Chemistry Utils
 * chemistry/object.cc 
 *
 * Copyright (C) 2002
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
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
#include <string>

using namespace gcu;

Object::Object(TypeId Id)
{
	m_Type = Id;
	m_Id = NULL;
	m_Parent = NULL;
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
		else (*i).second->m_Parent = NULL;
	}
}


void Object::SetId(gchar* Id)
{
	if (!Id) return;
	if (m_Id)
	{
		if (m_Parent) m_Parent->m_Children.erase(m_Id);
		g_free(m_Id);
	}
	m_Id = g_strdup(Id);
	if (m_Parent) m_Parent->m_Children[m_Id] = this;
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

Object* Object::GetDocument()
{
	Object* object = this;
	while (object && (object->m_Type != DocumentType)) object = object->m_Parent;
	return object;
}

void Object::AddChild(Object* object)
{
	if (object->m_Id == NULL)
	{
		int i = 0;
		char szId[16];
		while (snprintf(szId, sizeof(szId), "o%d", i++), m_Children[szId] != NULL);
		object->m_Id = g_strdup(szId);
	}
	else if (object->m_Parent) object->m_Parent->m_Children.erase(object->m_Id);
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
	Object* object = m_Children[Id];
	if (!object)
	{
		m_Children.erase(Id);
		map<string, Object*>::iterator i;
		for (i = m_Children.begin(); i != m_Children.end(); i++)
			if (object = (*i).second->GetDescendant(Id)) break;
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

xmlNodePtr Object::Save(xmlDocPtr xml)	//FIXME:Should save every child
{
	return NULL;
}

bool Object::Load(xmlNodePtr node)
{
	return false;
}

bool Object::SaveChildren(xmlDocPtr xml, xmlNodePtr node)
{
	map<string, Object*>::iterator i;
	xmlNodePtr child;
	for (i = m_Children.begin(); i != m_Children.end(); i++)
	{
		if (child = (*i).second->Save(xml)) xmlAddChild(node, child);
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

void Object::ShowContextualMenu(unsigned button, unsigned time)
{
	BuildContextualMenu();
	if (m_Menu)
	{
		gtk_menu_popup(m_Menu, NULL, NULL, NULL, NULL, button, time);
        g_object_unref(G_OBJECT(m_Menu));
		m_Menu = NULL;
	}
}

void Object::BuildContextualMenu()
{
}

void Object::Add(GtkWidget* w)
{
}

void Object::Print(GnomePrintContext *pc)
{
}
