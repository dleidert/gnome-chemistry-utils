// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * object.h 
 *
 * Copyright (C) 2002-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_OBJECT_H
#define GCU_OBJECT_H

#include "matrix2d.h"
#include <glib.h>
#include <libxml/parser.h>
#include <map>
#include <set>
#include <list>
#include <string>
#include <stdexcept>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print.h>

#define square(x) ((x)*(x))

using namespace std;

namespace gcu
{

/*!\enum TypeId
This enumeration is used to determine the type of an Object instance.
Possible values are:
	- NoType: invalid type
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
enum
{
	NoType,
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

typedef unsigned TypeId;

/*!\enum RuleId
This enumeration is used to maintain a set of rules about the possible
hierarchical of the document. They are used with two class names or ids.
Possible values are:
	- RuleMayContain: an instance of the first class may contain an instance of the second.
This implies that an instance of the second class may be in an instance of the first (see RuleMayBeIn);
	- RuleMustContain: an instance of the first class may contain an instance of the second class (implies RuleMayContain);
if no instance of the first class is present, the object is not valid.
	- RuleMayBeIn: an instance of the first class may be the child of an instance of the second class (see also RuleMayContain);
	- RuleMustBeIn:an instance of the first class must be the child of an instance of the second class, otherwise
it is not valid.
*/
enum RuleId
{
	RuleMayContain,
	RuleMustContain,
	RuleMayBeIn,
	RuleMustBeIn
};

typedef unsigned SignalId;

class Document;

/*!\class Object gcu/object.h
This is the base class for most other objects in the gcu namespace.
*/
class Object
{
public:
/*!
Used to create an object of type Id. Shold only be called from the constructor of a derived class.
*/
	Object (TypeId Id = OtherType);
/*!
The standard destructor of Object instances. Automatically called when the object is destroyed.
*/
	virtual ~Object ();
	
/*!
@return the type of the object. If the type is at least equal to OtherType, it is a dynamically created type returned by
the Object::AddType method.
*/
	TypeId GetType () {return m_Type;}
/*!
	@param Id: the id of the Object instance.
	
	Every object must have an Id, since searches in the document tree uses it.
*/
	void SetId (gchar* Id);
/*!
	@return the Id of the Object instance.
*/
	const gchar* GetId () {return m_Id;}
/*!
	@param object*: the Object instance to add as a child.
	
	Each Object instance maintains a list of its children. If object has already a parent, it will be removed from its
	parent children list. The new parent Object must have a Document ancestor to ensure that Ids are unique.
*/
	void AddChild (Object* object);
/*!
	Used to get the Molecule in the Object instances ancestors. 
	
	@return the first Object of type MoleculeType encountered when exploring
	the Objects tree or NULL if none is found.
*/
	Object* GetMolecule ();
/*!
	Used to get the Reaction in the Object instances ancestors. 
	
	@return the first Object of type ReactionType encountered when exploring
	the Objects tree or NULL if none is found.
*/
	Object* GetReaction ();
/*!
	Used to get the highest ancestor just before the document
	in the Object instances ancestors. 
	
	@return the last Object of type ReactionType encountered before the document when exploring
	the Objects tree or NULL if the object's parent is the document itself.
*/
	Object* GetGroup ();
/*!
	Used to get the Document in the Object instances ancestors. 
	
	@return the first Object of type DocumentType encountered when exploring
	the Objects tree (only one should be found) or NULL if none is found.
*/
	Document* GetDocument ();
/*!
@param Id: the type of the ancestor searched.

	Used to get the first ancestor of type Id in the Object instances ancestors.
	GetDocument, GetMolecule and GetReaction are special cases of this method.
	
	@return the first Object of type Id encountered when exploring
	the Objects tree (only one should be found) or NULL if none is found.
*/
	Object* GetParentOfType (TypeId Id);
/*!
@param Id: the Id of the child searched.

To search the Object in lower shells of the tree, use the Object::GetDescendant method.
@return the Object instance of type Id if found in the children list or NULL if not found.
*/
	Object* GetChild (const gchar* Id);
/*!
@param i: a C++ std::map iterator.

Use this function to retrieve the first child of the object and initialize the iterator.
@return the first child of the object or NULL.
*/
	Object* GetFirstChild (map<string, Object*>::iterator& i);
/*!
@param i: a C++ std::map iterator initialized by Object::GetFirstChild.

Use this method to iterate through the list of the Object children.
@return the next child of the object or NULL.
*/
	Object* GetNextChild (map<string, Object*>::iterator& i);
/*!
@param Id: the Id of the descendant searched.

This method searches the Object in its children and if not found calls the GetDescendant method for its children. 
@return the Object instance of type Id if found in the decendants or NULL if not found.
*/
	Object* GetDescendant (const gchar* Id);
/*!
@return the parent of the Object.
*/
	Object* GetParent () {return m_Parent;}
/*!
@param Parent: the new parent of the Object or NULL.
	
	When Parent is not NULL, this is equivalent to \code Parent->AddChild(this);\endcode
	Otherwise, it removes the Object from the Document tree.
*/
	void SetParent (Object* Parent);
/*!
	@param xml: the xmlDoc used to save the document.
	
	Used to save the Object to the xmlDoc. Each serializable Object should implement this virtual method.
	@return the xmlNode containing the serialized object. The name of the node should be the name of the 
	corresponding type used as first parameter to the Object::AddType method. The
	default method just saves the id and children.
*/
	virtual xmlNodePtr Save (xmlDocPtr xml);
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
	virtual bool Load (xmlNodePtr node);
/*!
@param x: the x component of the transation vector.
@param y: the y component of the transation vector.
@param z: the z component of the transation vector.

Used to move an object. This virtual method should most often be overrided by Object derived classes for which it makes sense.
The base Object class has no coordinates and the default method only loads its id and children.
*/
	virtual void Move (double x, double y, double z = 0.);
/*!
@param m: the 2D Matrix of the transformation.
@param x: the x component of the center of the transformation.
@param y: the y component of the center of the transformation.

Used to move and/or transform an object.
This virtual method must be overrided by Object derived classes for which it makes sense.
The base Object class has no coordinates and the default method calls the corresponding method
for every child.
*/
	virtual void Transform2D (Matrix2D& m, double x, double y);
/*!
@param xml: the xmlDoc used to save the document.
@param node: the node representing the Object.

This method calls Object::Save fo each child of the Object instance and add the xmlNode returned to the children of node.
It might be called from the Save method of objects having serializable children.
@return true on succes, false otherwise.
*/
	bool SaveChildren (xmlDocPtr xml, xmlNodePtr node);
/*!
@param node: the node representing the Object.

This helper method saves the Id of the node as a property of the xmlNode.
*/
	void SaveId (xmlNodePtr node);
/*!
@param node: the node where the search is to be done.
@param Property: the name of the property used in the search.
@param Id: the value to match to the property.

Helper method used in conjunction with Object::GetNextNodeByProp to search xmlNode instances having a property Property
whose value is Id in the children of node.

@return the node corresponding to the first match. This value is to be passed to Object::GetNextNodeByProp to iterate in the list
*/
	xmlNodePtr GetNodeByProp (xmlNodePtr node, char* Property, char* Id);
/*!
@param node: the xmlNodePtr returned by Object::GetNodeByProp or the last call to Object::GetNextNodeByProp.
@param Property: the name of the property used in the search.
@param Id: the value to match to the property.

Helper method used to iterate through a list of xmlNodePtr searching for a special value of a property.
Generally, the iteration is initialized by a call to Object::GetNodeByProp.
@return the next matching node.
*/
	xmlNodePtr GetNextNodeByProp (xmlNodePtr node, char* Property, char* Id);
/*!
@param node: the node where the search is to be done.
@param Name: the name of the xmlNode searched.

Helper method used in conjunction with Object::GetNextNodeByProp to search xmlNode instances of name Name
in the children of node.

@return the node corresponding to the first match. This value is to be passed to Object::GetNextNodeByName to iterate in the list.
*/
	xmlNodePtr GetNodeByName (xmlNodePtr node, char* Name);
/*!
@param node: the xmlNodePtr returned by Object::GetNodeByName or the last call to Object::GetNextNodeByName.
@param Name: the name of the xmlNode searched.

Helper method used to iterate through a list of xmlNodePtr searching for nodes whose name is Name.
Generally, the iteration is initialized by a call to Object::GetNodeByName.
@return the next matching node.
*/
	xmlNodePtr GetNextNodeByName (xmlNodePtr node, char* Name);
/*!
@param w: the GtkWidget inside which the Object will be displayed.

Used to add a representation of the Object in the widget. This method might be overrided for displayable Object classes
unless the application uses another mechanism.
*/
	virtual void Add (GtkWidget* w);
/*!
@param pc: the GnomePrintContext to which the document is printed.

This method might be used to print a document from an application using the Gnome Chemistry Utils.
*/
	virtual void Print (GnomePrintContext *pc);
/*!
@param w: the GtkWidget inside which the Object is displayed.

Used to update the representation of the Object in the widget. This method might be overrided for displayable Object classes
unless the application uses another mechanism.
*/
	virtual void Update (GtkWidget* w);
/*!
@param w: the GtkWidget inside which the Object is displayed.
@param state: the selection state of the Object.

Used to set the selection state of the Object inside the widget. The values of state are application dependant and have no
default value.
*/
	virtual void SetSelected (GtkWidget* w, int state);
/*!
@return true if the Object has at least a child an false if it has none.
*/
	bool HasChildren () {return m_Children.size () != 0;}

/*!
@return the children number of the Object.
*/
	unsigned GetChildrenNumber () {return m_Children.size ();}

/*!
@param x: the x coordinate
@param y: the y coordinate
@param z: the z coordinate

@return a pointer to a child of type Atomtype at or near position defined by the coordinates
passed as parameters. Default implementation returns NULL.
*/
	virtual Object* GetAtomAt (double x, double y, double z = 0.);

/*!
@param Children: the list of objects used as children to build the object

This method is called to build a parent object from its children. The object must already exist.
@return true in case of success and false if failed.
*/
	virtual bool Build (list<Object*>& Children) throw (invalid_argument);

/*!
Used to retreive the y coordinate for alignment. The default implementation returns 0.0 and
every derived class for which alignment has a meaning should implement this method.
@return y coordinate used for objects alignment.
*/
	virtual double GetYAlign ();

/*!
@param UIManager: the GtkUI%anager to populate.
@param object: the Object on which occured the mouse click.

This method is called to build a contextual menu for the object. It is called by Object::ShowContextualMenu, so
it should not be necessary to call it directly. It should be overrided by derived classes when a contextual menu
is needed. Typically, each class adds a submenu and calls the same method for its parent.
Default implementation just calls the parent's method.
@return true if something is added to the UIManager, false otherwise.
*/
	virtual bool BuildContextualMenu (GtkUIManager *UIManager, Object *object);

/*!
@param Signal: the appropriate SignalId

Sends a signal to the object parent. The signal may be propagated to the ancestors (see
Object::OnSignal ()).
*/
	void EmitSignal (SignalId Signal);

/*!
@param Signal: the appropriate SignalId
@param Child: the child which emitted the signal or NULL

This function is called by the framework when a signal has been emitted for the object.
It should not be called by a program; call Object::EmitSignal instead.

@return true if the signal should be propagated to the parent, false otherwise.
*/
	virtual bool OnSignal (SignalId Signal, Object *Child);

/*!
@param state: whether to block signals or not

Blocks signals if State is true and unblocs if state is false.

Since 0.4.2
*/
	void Lock (bool state = true);

/*!

@return true if signals are locked, false otherwise

Since 0.4.2
*/
	bool IsLocked () {return m_Locked > 0;}

/*!
@param i: a C++ std::set<Object*> iterator.

Use this function to retrieve the first object linked to the object and initialize the iterator.
Links can be used when the relation between the objects is not a parent to child one.
@return the first object linked to the object or NULL.
*/
	Object* GetFirstLink (set<Object*>::iterator& i);

/*!
@param i: a C++ std::set<Object*> iterator initialized by Object::GetFirstLink.

Use this method to iterate through the list of Object instances linked to the object.
@return the next object linked to the object or NULL.
*/
	Object* GetNextLink (set<Object*>::iterator& i);

/*!
@param object: the object to unlink.

Unlinks object and calls Object::OnUnlink.
*/
	void Unlink (Object *object);

/*!
@param object: the object just unlinked by Object::Unlink.

Virtual method called when an object hs been unlinked. Programs should not call it
directly, but should call Object::OnUnlink instead.
*/
	virtual void OnUnlink (Object *object);

/*!
@param types: the list of TypeId values to fill

Fills types with all valid ancestor types for the object as defined by rules created with AddRule
*/
	void GetPossibleAncestorTypes (set<TypeId>& types);

/*!
@param TypeName: the name of the new type.
@param CreateFunc: a pointer to a function returning a pointer to a new object of the new type.
@param id: the Id of the type to create if a standard one or OtherType for a new type. In this last case, this parameter
can be omitted.

This method is used to register a new type derived from Object.
@return the Id of the new type.
*/
	static TypeId AddType (string TypeName, Object*(*CreateFunc)(), TypeId id = OtherType);

/*!
@param TypeName: the name of the new type.
@param parent: the parent of the newly created object or NULL. if NULL, the parameter can be omitted.

Used to create an object of type name TypeName. The Object::AddType method must have been called with the same
TypeName parameter. if parent is given and not NULL, the new Object will be a child of parent.
It will also be given a default Id.

@return a pointer to the newly created Object or NULL if the Object could not be created.
*/
	static Object* CreateObject (const string& TypeName, Object* parent = NULL);

/*!
@param Name: the name of the Object derived class

@return the TypeId corresponding to Name
*/
	static TypeId GetTypeId (const string& Name);

/*!
@param Id: the TypeId of the Object derived class

@return the name of the type.
*/
	static string GetTypeName (TypeId Id);

/*!
@param type1: the TypeId of the first class in the rule
@param rule: the new rule value
@param type2: the TypeId of the second class in the rule

Adds a rule.
*/
	static void AddRule (TypeId type1, RuleId rule, TypeId type2);

/*!
@param type1: the name of the first class in the rule
@param rule: the new rule value
@param type2: the name of the second class in the rule

Adds a rule.
*/
	static void AddRule (const string& type1, RuleId rule, const string& type2);

/*!
@param type: the TypeId of a class
@param rule: a RuleId value

@return the set of rules correponding to the RuleId value for this class.
*/
	static  const set<TypeId>& GetRules (TypeId type, RuleId rule);

/*!
@param type: the name of a class
@param rule: a RuleId value

@return the set of rules correponding to the RuleId value for this class.
*/
	static const set<TypeId>& GetRules (const string& type, RuleId rule);

/*!
@param Id: the TypeId of a class
@param Label: the string to display in a contextual menu

Used to give a label for contextual menus used when the creation of an instance of
the class seems possible.
*/
	static void SetCreationLabel (TypeId Id, string Label);

/*!
@param Id: the TypeId of a class

@return the string defined by SetCreationLabel.
*/
	static const string& GetCreationLabel (TypeId Id);

/*!
@param TypeName: the name of a class

@return the string defined by SetCreationLabel.
*/
	static const string& GetCreationLabel (const string& TypeName);

/*!
@return a new SignalId.
*/
	static SignalId CreateNewSignalId ();

private:
	Object* RealGetDescendant (const gchar* Id);

private:
	gchar* m_Id;
	TypeId m_Type;
	Object *m_Parent;
	map<string, Object*> m_Children; //string is Id of object, so each object must have an Id
	set<Object*> m_Links; //objects linked to this but outside of the hierarchy

private:
/*!
Set to true while loading to avoid signal propagation.
*/
	int m_Locked;
};

}
#endif //GCU_OBJECT_H