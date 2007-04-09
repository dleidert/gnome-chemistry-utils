// -*- C++ -*-

/* 
 * GChemPaint library
 * operation.h 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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


#ifndef GCHEMPAINT_OPERATION_H
#define GCHEMPAINT_OPERATION_H

#include <gcu/macros.h>
#include <gcu/object.h>

using namespace gcu; 

namespace gcp {

class Document;

typedef enum
{
	GCP_ADD_OPERATION,
	GCP_DELETE_OPERATION,
	GCP_MODIFY_OPERATION,
} OperationType;

class Operation
{
public:
	Operation (Document *pDoc, unsigned long ID);
	virtual ~Operation ();

	virtual void Undo () = 0;
	virtual void Redo () = 0;
	virtual void AddObject (Object* pObject, unsigned type = 0);
	virtual void AddNode (xmlNodePtr node, unsigned type = 0);

protected:
	void Add (unsigned type = 0);
	void Delete (unsigned type = 0);

protected:
	xmlNodePtr* m_Nodes;

private:
	gcp::Document* m_pDoc;

GCU_RO_PROP (unsigned long, ID);
};

class AddOperation: public Operation
{
public:
	AddOperation (gcp::Document *pDoc, unsigned long ID);
	virtual ~AddOperation ();

	virtual void Undo ();
	virtual void Redo ();
};

class DeleteOperation: public Operation
{
public:
	DeleteOperation (gcp::Document *pDoc, unsigned long ID);
	virtual ~DeleteOperation ();

	virtual void Undo ();
	virtual void Redo ();
};

class ModifyOperation: public Operation
{
public:
	ModifyOperation (gcp::Document *pDoc, unsigned long ID);
	virtual ~ModifyOperation ();

	virtual void Undo ();
	virtual void Redo ();
};

}	// namespace gcp

#endif //GCHEMPAINT_OPERATION_H
