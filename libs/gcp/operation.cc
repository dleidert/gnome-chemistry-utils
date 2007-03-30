// -*- C++ -*-

/* 
 * GChemPaint library
 * operation.cc 
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

#include "config.h"
#include "operation.h"
#include "document.h"

namespace gcp {

xmlDocPtr pXmlDoc = xmlNewDoc ((const xmlChar*) "1.0"); // Needed to create xmlNodes

Operation::Operation (gcp::Document* pDoc, unsigned long ID):
	m_pDoc (pDoc),
	m_ID (ID)
{
}

Operation::~Operation ()
{
	if (m_Nodes) 
		delete[] m_Nodes;
}

void Operation::Add (unsigned type)
{
	m_pDoc->LoadObjects (m_Nodes[type]);
}

void Operation::Delete (unsigned type)
{
	xmlNodePtr node = m_Nodes[type]->children;
	char* Id;
	while (node) {
		Id = (strcmp ((const char*) node->name, "object"))?
			 (char*) xmlGetProp (node, (xmlChar*) "id"):
			 (char*) xmlGetProp (node->children, (xmlChar*) "id");
		m_pDoc->Remove (Id);
		xmlFree (Id);
		node = node->next;
	}
}

void Operation::AddObject (Object* pObject, unsigned type)
{
	xmlNodePtr node = pObject->Save (pXmlDoc);
	if (node)
		xmlAddChild (m_Nodes[type], node);
}

void Operation::AddNode(xmlNodePtr node, unsigned type)
{
	if (node) xmlAddChild(m_Nodes[type], node);
}

 AddOperation:: AddOperation (gcp::Document* pDoc, unsigned long ID):  Operation(pDoc, ID)
{
	m_Nodes = new xmlNodePtr[1];
	*m_Nodes = xmlNewDocNode (pXmlDoc, NULL, (const xmlChar*) "add", NULL);
}

 AddOperation::~ AddOperation ()
{
	if (*m_Nodes)
		xmlFreeNode (*m_Nodes);
}

void  AddOperation::Undo ()
{
	Delete ();
}

void  AddOperation::Redo ()
{
	Add ();
}

DeleteOperation::DeleteOperation (gcp::Document* pDoc, unsigned long ID): Operation (pDoc, ID)
{
	m_Nodes = new xmlNodePtr[1];
	*m_Nodes = xmlNewDocNode (pXmlDoc, NULL, (const xmlChar*) "delete", NULL);
}

DeleteOperation::~DeleteOperation ()
{
	if (*m_Nodes)
		xmlFreeNode (*m_Nodes);
}

void DeleteOperation::Undo ()
{
	Add ();
}

void DeleteOperation::Redo ()
{
	Delete ();
}

ModifyOperation::ModifyOperation (gcp::Document* pDoc, unsigned long ID): Operation (pDoc, ID)
{
	m_Nodes = new xmlNodePtr[2];
	m_Nodes[0] = xmlNewDocNode (pXmlDoc, NULL, (const xmlChar*) "before", NULL);
	m_Nodes[1] = xmlNewDocNode (pXmlDoc, NULL, (const xmlChar*) "after", NULL);
}

ModifyOperation::~ModifyOperation ()
{
	if (!m_Nodes)
		return;
	if (m_Nodes[0])
		xmlFreeNode (m_Nodes[0]);
	if (m_Nodes[1])
		xmlFreeNode (m_Nodes[1]);
}

void ModifyOperation::Undo ()
{
	Delete (1);
	Add (0);
}

void ModifyOperation::Redo ()
{
	Delete (0);
	Add (1);
}

} // namespace gcp
