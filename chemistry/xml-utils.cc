// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/xml-utils.cc 
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

#include "xml-utils.h"
#include <string.h>

xmlNodePtr FindNodeByNameAndId(xmlNodePtr node, const char* name, const char* id)
{
	xmlNodePtr child = node->children;
	char* tmp;
	while (child)
	{
		if (!strcmp((char*)child->name, name))
		{
			tmp = (char*)xmlGetProp(child, (xmlChar*)"id");
			if ((!id && !tmp) || (id && tmp && !strcmp(tmp, id))) break;
		}
		child = child->next;
	}
	return child;
}

bool ReadPosition(xmlNodePtr node, const char* id, double* x, double* y, double* z)
{
	xmlNodePtr child = FindNodeByNameAndId(node, "position", id);
	if (!child) return false;
	char* tmp;
	tmp = (char*)xmlGetProp(child, (xmlChar*)"x");
	if (tmp) sscanf(tmp, "%lg", x); else return false;
	tmp = (char*)xmlGetProp(child, (xmlChar*)"y");
	if (tmp) sscanf(tmp, "%lg", y); else return false;
	if (z) 
	{
		tmp = (char*)xmlGetProp(child, (xmlChar*)"z");
		if (tmp) sscanf(tmp, "%lg", z); else *z = 0.0;
	}
	return true;
}

bool WritePosition(xmlDocPtr xml, xmlNodePtr node, const char* id, double x, double y, double z)
{
	xmlNodePtr child;
	char buf[16];
	child = xmlNewDocNode(xml, NULL, (xmlChar*)"position", NULL);
	if (child) xmlAddChild(node, child); else return false;
	if (id) xmlNewProp(child, (xmlChar*)"id", (xmlChar*)id);
	snprintf(buf, sizeof(buf), "%g", x);
	xmlNewProp(child, (xmlChar*)"x", (xmlChar*)buf);
	snprintf(buf, sizeof(buf), "%g", y);
	xmlNewProp(child, (xmlChar*)"y", (xmlChar*)buf);
	if (z != 0.0)
	{
		snprintf(buf, sizeof(buf), "%g", z);
		xmlNewProp(child, (xmlChar*)"z", (xmlChar*)buf);
	}
	return true;
}

bool ReadColor(xmlNodePtr node, const char* id, float* red, float* green, float* blue, float* alpha)
{
	xmlNodePtr child = FindNodeByNameAndId(node, "color", id);
	if (!child) return false;
	char* tmp;
	tmp = (char*)xmlGetProp(child, (xmlChar*)"red");
	if (tmp) sscanf(tmp, "%g", red); else return false;
	tmp = (char*)xmlGetProp(child, (xmlChar*)"green");
	if (tmp) sscanf(tmp, "%g", green); else return false;
	tmp = (char*)xmlGetProp(child, (xmlChar*)"blue");
	if (tmp) sscanf(tmp, "%g", blue); else return false;
	if (alpha) 
	{
		tmp = (char*)xmlGetProp(child, (xmlChar*)"alpha");
		if (tmp) sscanf(tmp, "%g", alpha); else *alpha = 1.0;
	}
	return true;
}

bool WriteColor(xmlDocPtr xml, xmlNodePtr node, const char* id, double red, double green, double blue, double alpha)
{
	xmlNodePtr child;
	char buf[16];
	child = xmlNewDocNode(xml, NULL, (xmlChar*)"color", NULL);
	if (child) xmlAddChild(node, child); else return false;
	if (id) xmlNewProp(child, (xmlChar*)"id", (xmlChar*)id);
	snprintf(buf, sizeof(buf), "%g", red);
	xmlNewProp(child, (xmlChar*)"red", (xmlChar*)buf);
	snprintf(buf, sizeof(buf), "%g", green);
	xmlNewProp(child, (xmlChar*)"green", (xmlChar*)buf);
	snprintf(buf, sizeof(buf), "%g", blue);
	xmlNewProp(child, (xmlChar*)"blue", (xmlChar*)buf);
	if (alpha != 1.0)
	{
		snprintf(buf, sizeof(buf), "%g", alpha);
		xmlNewProp(child, (xmlChar*)"alpha", (xmlChar*)buf);
	}
	return true;
}
