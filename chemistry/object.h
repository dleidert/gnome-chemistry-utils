// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/object.h 
 *
 * Copyright (C) 2002-2003
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
#include <string>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print.h>

#define square(x) ((x)*(x))

using namespace std;

namespace gcu
{

/*!\enum TypeId
This enumeration is used to determine the type of an Object instance.
Possible values are:
	- AtomType: an atom
	- FragmentType: several atoms linked and represented by a text such as COOH (only in GChemPaint).
	- BondType: a bond between two (or more) atoms.
	- MoleculeType: a molecule.
	- ChainType: a chain of atoms (only in GChemPaint)
	- CycleType: a cycle (only in GChemPaint)
	- ReactantType: a molecule involved in a reaction (only in GChemPaint).
	- ReactionArrowType: a reaction arrow (only in GChemPaint).
	- ReactionOperatorType: a + sign in a reaction (only in GChemPaint).
	- ReactionType: a reaction.
	- MesomeryType: a mesomery representation (only in GChemPaint).
	- MesomeryArrowType: a double headed arrow used to represent mesomery (only in GChemPaint).
	- DocumentType: a document, generally the top node in the objects tree.
	- TextType: some text (only in GChemPaint).
	- OtherType: if the type of an object is at least equal to OtherType, then it is a dynamically created type returned
	by the static Object::AddType method.
	.
Some types are not used in  the Gnome Chemistry Utils, but only in GChemPaint
and might disappear from this list in future versions and replaced by dynamically created types.
*/	
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
	MesomeryArrowType,
	DocumentType,
	TextType,
	OtherType
};

/*!\class Object chemistry/object.h
This is the base class for most other objects in the gcu namespace.
*/
class Object
{
public:
/*!
Used to create an object of type Id. Shold only be called from the constructor of a derived class.
*/
	Object(TypeId Id = OtherType);
/*!
The standard destructor of Object instances. Automatically called when the object is destroyed.
*/
	virtual ~Object();
	
/*!
@return the type of the object. If the type is at least equal to OtherType, it is a dynamically created type returned by
the Object::AddType method.
*/
	TypeId GetType() {return m_Type;}
/*!
	@param Id: the id of the Object instance.
	
	Every object must have an Id, since searches in the document tree uses it.
*/
	void SetId(gchar* Id);
/*!
	@return the Id of the Object instance.
*/
	const gchar* GetId() {return m_Id;}
/*!
	@param object*: the Object instance to add as a child.
	
	Each Object instance maintains a list of its children. If object has already a parent, it will be removed from its
	parent children list. The new parent Object must have a Document ancestor to ensure that Ids are unique.
*/
	void AddChild(Object* object);
/*!
	Used to get the Molecule in the Object instances ancestors. 
	
	@return the first Object of type MoleculeType encountered when exploring
	the Objects tree or NULL if none is found.
*/
	Object* GetMolecule();
/*!
	Used to get the Reaction in the Object instances ancestors. 
	
	@return the first Object of type ReactionType encountered when exploring
	the Objects tree or NULL if none is found.
*/
	Object* GetReaction();
/*!
	Used to get the Document in the Object instances ancestors. 
	
	@return the first Object of type DocumentType encountered when exploring
	the Objects tree (only one should be found) or NULL if none is found.
*/
	Object* GetDocument();
/*!
@param Id: the type of the ancestor searched.

	Used to get the first ancestor of type Id in the Object instances ancestors.
	GetDocument, GetMolecule and GetReaction are special cases of this method.
	
	@return the first Object of type Id encountered when exploring
	the Objects tree (only one should be found) or NULL if none is found.
*/
	Object* GetParentOfType(TypeId Id);
/*!
@param Id: the Id of the child searched.

To search the Object in lower shells of the tree, use the Object::GetDescendant method.
@return the Object instance of type Id if found in the children list or NULL if not found.
*/
	Object* GetChild(const gchar* Id);
/*!
@param i: a C++ std::map iterator.

Use this function to retrieve the first child of the object and initialize the iterator.
@return the first child of the object or NULL.
*/
	Object* GetFirstChild(map<string, Object*>::iterator& i);
/*!
@param i: a C++ std::map iterator initialized by Object::GetFirstChild.

Use this method to iterate through the list of the Object children.
@return the next child of the object or NULL.
*/
	Object* GetNextChild(map<string, Object*>::iterator& i);
/*!
@param Id: the Id of the descendant searched.

This method searches the Object in its children and if not found calls the GetDescendant method for its children. 
@return the Object instance of type Id if found in the decendants or NULL if not found.
*/
	Object* GetDescendant(const gchar* Id);
/*!
@return the parent of the Object.
*/
	Object* GetParent() {return m_Parent;}
/*!
@param Parent: the new parent of the Object or NULL.
	
	When Parent is not NULL, this is equivalent to \code Parent->AddChild(this);\endcode
	Otherwise, it removes the Object from the Document tree.
*/
	void SetParent(Object* Parent);
/*!
	@param xml: the xmlDoc used to save the document.
	
	Used to save the Object to the xmlDoc. Each serializable Object should implement this virtual method.
	@return the xmlNode containing the serialized object. The name of the node should be the name of the 
	corresponding type used as first parameter to the Object::AddType method.
*/
	virtual xmlNodePtr Save(xmlDocPtr xml);
/*!
@param node: a pointer to the xmlNode containing the serialized object.

Used to load an Object in memory. The Object must already exist.

Example: \code
	std::string str = (const char*)node->name;
	Object* pObject = Object::CreateObject(str, this);
	if (pObject) {
		if (!pObject->Load(node)) delete Object; 
	} else
		cerr << "Warning: unknown object: " << str << endl;
\endcode

@return true on succes, false otherwise.
*/
	virtual bool Load(xmlNodePtr node);
/*!
@param x: the x component of the transation vector.
@param y: the y component of the transation vector.
@param z: the z component of the transation vector.

Used to move an object. This virtual method must be overrided by Object derived classes for which it makes sense.
The base Object class has no coordinates and the default method does nothing.
*/
	virtual void Move(double x, double y, double z = 0.);
/*!
@param xml: the xmlDoc used to save the document.
@param node: the node representing the Object.

This method calls Object::Save fo each child of the Object instance and add the xmlNode returned to the children of node.
It might be called from the Save method of objects having serializable children.
@return true on succes, false otherwise.
*/
	bool SaveChildren(xmlDocPtr xml, xmlNodePtr node);
/*!
@param node: the node representing the Object.

This helper method saves the Id of the node as a property of the xmlNode.
*/
	void SaveId(xmlNodePtr node);
/*!
@param node: the node where the search is to be done.
@param Property: the name of the property used in the search.
@param Id: the value to match to the property.

Helper method used in conjunction with Object::GetNextNodeByProp to search xmlNode instances having a property Property
whose value is Id in the children of node.

@return the node corresponding to the first match. This value is to be passed to Object::GetNextNodeByProp to iterate in the list
*/
	xmlNodePtr GetNodeByProp(xmlNodePtr node, char* Property, char* Id);
/*!
@param node: the xmlNodePtr returned by Object::GetNodeByProp or the last call to Object::GetNextNodeByProp.
@param Property: the name of the property used in the search.
@param Id: the value to match to the property.

Helper method used to iterate through a list of xmlNodePtr searching for a special value of a property.
Generally, the iteration is initialized by a call to Object::GetNodeByProp.
@return the next matching node.
*/
	xmlNodePtr GetNextNodeByProp(xmlNodePtr node, char* Property, char* Id);
/*!
@param node: the node where the search is to be done.
@param Name: the name of the xmlNode searched.

Helper method used in conjunction with Object::GetNextNodeByProp to search xmlNode instances of name Name
in the children of node.

@return the node corresponding to the first match. This value is to be passed to Object::GetNextNodeByName to iterate in the list.
*/
	xmlNodePtr GetNodeByName(xmlNodePtr node, char* Name);
/*!
@param node: the xmlNodePtr returned by Object::GetNodeByName or the last call to Object::GetNextNodeByName.
@param Name: the name of the xmlNode searched.

Helper method used to iterate through a list of xmlNodePtr searching for nodes whose name is Name.
Generally, the iteration is initialized by a call to Object::GetNodeByName.
@return the next matching node.
*/
	xmlNodePtr GetNextNodeByName(xmlNodePtr node, char* Name);
/*!
@param button: the mouse button which was pressed to initiate the event.
@param time: the time at which the activation event occurred.

Shows the contextual menu correspondig to the Object if this menu exists. The menu must be built in the derived class
BuildContextualMenu method.
*/
	void ShowContextualMenu(unsigned button, unsigned time);
/*!
@param w: the GtkWidget inside which the Object will be displayed.

Used to add a representation of the Object in the widget. This method might be overrided for displayable Object classes
unless the application uses another mechanism.
*/
	virtual void Add(GtkWidget* w);
/*!
@param pc: the GnomePrintContext to which the document is printed.

This method might be used to print a document from an application using the Gnome Chemistry Utils.
*/
	virtual void Print(GnomePrintContext *pc);
/*!
@param w: the GtkWidget inside which the Object is displayed.

Used to update the representation of the Object in the widget. This method might be overrided for displayable Object classes
unless the application uses another mechanism.
*/
	virtual void Update(GtkWidget* w);
/*!
@param w: the GtkWidget inside which the Object is displayed.
@param state: the selection state of the Object.

Used to set the selection state of the Object inside the widget. The values of state are application dependant and have no
default value.
*/
	virtual void SetSelected(GtkWidget* w, int state);
/*!
@return true if the Object has at least a child an false if it has none.
*/
	bool HasChildren() {return m_Children.size() != 0;}
/*!
@param TypeName: the name of the new type.
@param CreateFunc: a pointer to a function returning a pointer to a new object of the new type.
@param id: the Id of the type to create if a standard one or OtherType for a new type. In this last case, this parameter
can be omitted.

This method is used to register a new type derived from Object.
@return the Id of the new type.
*/
	static unsigned AddType(string TypeName, Object*(*CreateFunc)(), TypeId id = OtherType);
/*!
@param TypeName: the name of the new type.
@param parent: the parent of the newly created object or NULL. if NULL, the parameter can be omitted.

Used to create an object of type name TypeName. The Object::AddType method must have been called with the same
TypeName parameter. if parent is given and not NULL, the new Object will be a child of parent.
It will also be given a default Id.

@return a pointer to the newly created Object or NULL if the Object could not be created.
*/
	static Object* CreateObject(string& TypeName, Object* parent = NULL);
/*!
When pasting, objects added to the document might have the same Id as objects already existing. In such cases, the document
maintains a table to update links using Ids as identifiers. The EmptyTranslationTable method should be called then to reinitialize the table
to avoid errors on the next paste event.
*/
	void EmptyTranslationTable();
/*!
@param x: the x coordinate
@param y: the y coordinate
@param z: the z coordinate

@return a pointer to a child of type Atomtype at or near position defined by the coordinates
passed as parameters. Default implementation returns NULL.
*/
	virtual Object* GetAtomAt(double x, double y, double z = 0.);

protected:
/*!
This method is called to build a contextual menu for the object. It is called by Object::ShowContextualMenu, so
it should not be necessary to call it directly. It should be overrided by derived classes when a contextual menu
is needed.
*/
	virtual void BuildContextualMenu();

private:
	gchar* m_Id;
	TypeId m_Type;
	Object *m_Parent;
	map<string, Object*> m_Children; //string is Id of object, so each object must have an Id
	map <string, string> m_TranslationTable;//used when Ids translations are necessary (on pasting...)
	GtkMenu* m_Menu;
};

}
#endif //GCU_OBJECT_H
