// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/object.h 
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

#ifndef GCU_OBJECT_H
#define GCU_OBJECT_H

#include <glib.h>
#include <libxml/parser.h>
#include <map>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print.h>

#define square(x) ((x)*(x))

using namespace std;

namespace gcu
{
	
enum TypeId
{
	AtomType,
	FragmentType,
	BondType,
	MoleculeType,
	ChainType,
	CycleType,
	ReactantType,
	ReactionArrowType,
	ReactionOperatorType,
	ReactionType,
	MesomeryType,
	DocumentType,
	TextType,
	OtherType
};

class Object
{
public:
	Object(TypeId Id = OtherType);
	virtual ~Object();
	
	TypeId GetType() {return m_Type;}
	void SetId(gchar* Id);
	const gchar* GetId() {return m_Id;}
	void AddChild(Object* object);
	Object* GetMolecule();
	Object* GetReaction();
	Object* GetDocument();
	Object* GetChild(const gchar* Id);
	Object* GetFirstChild(map<string, Object*>::iterator& i);
	Object* GetNextChild(map<string, Object*>::iterator& i);
	Object* GetDescendant(const gchar* Id);
	Object* GetParent() {return m_Parent;}
	void SetParent(Object* Parent);
	virtual xmlNodePtr Save(xmlDocPtr xml);
	virtual bool Load(xmlNodePtr);
	virtual void Move(double x, double y, double z = 0);
	bool SaveChildren(xmlDocPtr xml, xmlNodePtr node);
	xmlNodePtr GetNodeByProp(xmlNodePtr node, char* Property, char* Id);
	xmlNodePtr GetNextNodeByProp(xmlNodePtr node, char* Property, char* Id);
	xmlNodePtr GetNodeByName(xmlNodePtr node, char* Name);
	xmlNodePtr GetNextNodeByName(xmlNodePtr node, char* Name);
	void ShowContextualMenu(unsigned button, unsigned time);
	virtual void Add(GtkWidget* w);
	virtual void Print(GnomePrintContext *pc);
	bool HasChildren() {return m_Children.size() != 0;}

protected:
	virtual void BuildContextualMenu();
	guint m_nMolIndex;
	GtkMenu* m_Menu;
	
protected:
	gchar* m_Id;
private:
	TypeId m_Type;
	Object *m_Parent;
	map<string, Object*> m_Children; //string is Id of object, so each object must have an Id
};

}
#endif //GCU_OBJECT_H
