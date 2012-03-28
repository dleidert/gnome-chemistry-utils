// -*- C++ -*-

/*
 * GChemPaint library
 * operation.h
 *
 * Copyright (C) 2002-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

/*!\file*/
#ifndef GCHEMPAINT_OPERATION_H
#define GCHEMPAINT_OPERATION_H

#include <gcu/macros.h>
#include <gcu/object.h>

/*!\file*/
namespace gcp {

class Document;

/*!\enum OperationType gcp/operation.h
Enumeration of the different operation types See gcp::Document::GetNewOeration()
for its use.
*/
typedef enum
{
/*!
Object addition operation, see the AddOperation class.
*/
	GCP_ADD_OPERATION,
/*!
Object deletion operation, see the DeleteOperation class.
*/
	GCP_DELETE_OPERATION,
/*!
Object modification operation, see the ModifyOperation class.
*/
	GCP_MODIFY_OPERATION,
} OperationType;

/*!\class Operation gcp/operation.h
Base operation class for the Undo/Redo framework.
This class is virtual since some methods are pure virtual.
*/
class Operation
{
public:
/*!
@param pDoc a document.
@param ID a unique operation ID for the document and the session.

Creates a new operation. Operations should always created by calls to
Document::GetNewOperation().
*/
	Operation (Document *pDoc, unsigned long ID);
	virtual ~Operation ();

/*!
Undo the changes represented by this operation.
*/
	virtual void Undo () = 0;
/*!
Redo the changes represented by this operation.
*/
	virtual void Redo () = 0;
/*!
@param pObject an Object affected by the changes.
@param type a number indicationg the role of the stored objects.

The \a type argument is only significant for the gcp::ModifyOperation class
where 0 represent the state of the objects before the operation, and 1 the
state of the objects after the operation.

Adds an object to the operation.
Typically, modifying an object whould need code like:
\code
	Object *obj;
	// Initialize the object pointer so that it points to a valid object
		...
	Document *doc = obj->GetDocument ();
	Operation *op = doc->GetNewOperation (GCP_MODIFY_OPERATION);
	op->AddObject (obj, 0);
	// Modify the object
		...
	op->AddObject (obj, 1);
	doc->FinishOperation ();
\endcode
*/
	virtual void AddObject (gcu::Object* pObject, unsigned type = 0);
/*!
@param node an xml node related to the changes.
@param type a number indicationg the role of the stored objects.

The \a type argument is only significant for the gcp::ModifyOperation class
where 0 represent the state of the objects before the operation, and 1 the
state of the objects after the operation.

Adds the node to the document owning the operation. This might be used when
Objects are not available such as when editing text.
*/
	virtual void AddNode (xmlNodePtr node, unsigned type = 0);

protected:
/*!
@param type a number indicationg the role of the stored objects.

The \a type argument is only significant for the gcp::ModifyOperation class
where 0 represent the state of the objects before the operation, and 1 the
state of the objects after the operation.

Adds the stored objects to the document owning the operation.
*/
	void Add (unsigned type = 0);
/*!
@param type a number indicationg the role of the stored objects.

The \a type argument is only significant for the gcp::ModifyOperation class
where 0 represent the state of the objects before the operation, and 1 the
state of the objects after the operation.

Deletes the stored objects to the document owning the operation.
*/
	void Delete (unsigned type = 0);

protected:
/*!
The xml nodes storing the changes.
*/
	xmlNodePtr* m_Nodes;

private:
	gcp::Document* m_pDoc;

GCU_RO_PROP (unsigned long, ID);
};

/*!\class AddOperation gcp/operation.h
Operation class representing objects additions.
*/
class AddOperation: public Operation
{
public:
/*!
@param pDoc a document.
@param ID a unique operation ID for the document and the session.

Creates a new AddOperation. Operations should always created by calls to
Document::GetNewOperation().
*/
	AddOperation (gcp::Document *pDoc, unsigned long ID);
	virtual ~AddOperation ();

/*!
Undo the additions represented by this operation.
*/
	void Undo ();
/*!
Redo the additions represented by this operation.
*/
	void Redo ();
};

/*!\class DeleteOperation gcp/operation.h
Operation class representing objects deletions.
*/
class DeleteOperation: public Operation
{
public:
/*!
@param pDoc a document.
@param ID a unique operation ID for the document and the session.

Creates a new DeleteOperation. Operations should always created by calls to
Document::GetNewOperation().
*/
	DeleteOperation (gcp::Document *pDoc, unsigned long ID);
	virtual ~DeleteOperation ();

/*!
Undo the deletions represented by this operation.
*/
	void Undo ();
/*!
Redo the deletions represented by this operation.
*/
	void Redo ();
};

/*!\class ModifyOperation gcp/operation.h
Operation class representing objects modifications.
*/
class ModifyOperation: public Operation
{
public:
/*!
@param pDoc a document.
@param ID a unique operation ID for the document and the session.

Creates a new ModifyOperation. Operations should always created by calls to
Document::GetNewOperation().
*/
	ModifyOperation (gcp::Document *pDoc, unsigned long ID);
	virtual ~ModifyOperation ();

/*!
Undo the modifications represented by this operation.
*/
	void Undo ();
/*!
Redo the modifications represented by this operation.
*/
	void Redo ();
};

}	// namespace gcp

#endif //GCHEMPAINT_OPERATION_H
