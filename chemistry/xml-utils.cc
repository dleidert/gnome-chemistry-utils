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

bool ReadRadius(xmlNodePtr node, GcuAtomicRadius& radius)
{
	char *tmp;
	tmp = (char*) xmlGetProp(node, (xmlChar*)"type");
	if (!tmp ||
		((!((!strcmp(tmp, "unknown")) && (radius.type = GCU_RADIUS_UNKNOWN))) &&
		(!((!strcmp(tmp, "covalent")) && (radius.type = GCU_COVALENT))) &&
		(!((!strcmp(tmp, "vdW")) && (radius.type = GCU_VAN_DER_WAALS))) &&
		(!((!strcmp(tmp, "ionic")) && (radius.type = GCU_IONIC))) &&
		(!((!strcmp(tmp, "metallic")) && (radius.type = GCU_METALLIC))) &&
		(!((!strcmp(tmp, "atomic")) && (radius.type = GCU_ATOMIC)))))
			radius.type = GCU_RADIUS_UNKNOWN;
	tmp = (char*) xmlGetProp(node, (xmlChar*)"scale");
	if (tmp) radius.scale = g_strdup(tmp);
	else radius.scale = NULL;
	tmp = (char*) xmlGetProp(node, (xmlChar*)"charge");
	if (tmp) radius.charge = strtol(tmp, NULL, 10);
	else radius.charge = 0;
	tmp = (char*) xmlGetProp(node, (xmlChar*)"cn");
	if (tmp) radius.cn = strtol(tmp, NULL, 10);
	else radius.cn = -1;
	tmp = (char*) xmlGetProp(node, (xmlChar*)"spin");
	if ((!tmp) ||
		(!((!strcmp(tmp, "low")) && (radius.spin = GCU_LOW_SPIN))) ||
		(!((!strcmp(tmp, "high")) && (radius.spin = GCU_HIGH_SPIN))))
	radius.spin = GCU_N_A_SPIN;
	if (((tmp = (char*) xmlGetProp(node, (xmlChar*)"value")) ||
		(tmp = (char*)xmlNodeGetContent(node))) && *tmp)
	{
		radius.value = strtod(tmp, NULL);
		radius.scale = g_strdup("custom");
	}
	else if (radius.scale && (!strcmp(radius.scale, "custom")))
	{
		return false;
	}
	else if (!gcu_element_get_radius(&radius))
	{
		return false;
	}
	if (radius.value <= 0.0) return false;
	return true;
}

bool WriteRadius(xmlDocPtr xml, xmlNodePtr node, const GcuAtomicRadius& radius)
{
	xmlNodePtr child;
	gchar buf[256], *tmp;

	child = xmlNewDocNode(xml, NULL, (xmlChar*)"radius", NULL);
	if (child) xmlAddChild(node, child);
	else return false;
	switch (radius.type)
	{
		case GCU_RADIUS_UNKNOWN:
			tmp = NULL;
			break;
		case GCU_ATOMIC:
			tmp = "atomic";
			break;
		case GCU_IONIC:
			tmp = "ionic";
			break;
		case GCU_METALLIC:
			tmp = "metallic";
			break;
		case GCU_COVALENT:
			tmp = "covalent";
			break;
		case GCU_VAN_DER_WAALS:
			tmp = "vdW";
			break;
	}
	if (tmp) xmlNewProp(child, (xmlChar*)"type", (xmlChar*)tmp);
	if ((radius.type == GCU_RADIUS_UNKNOWN) || (radius.scale && (!strcmp(radius.scale, "custom"))))
	{
		g_snprintf(buf, sizeof(buf) - 1, "%g", radius.value);
		xmlNewProp(child, (xmlChar*)"value", (xmlChar*)buf);
	}
	if (radius.scale &&  strcmp(radius.scale, "custom"))
		xmlNewProp(child, (xmlChar*)"scale", (xmlChar*)radius.scale);
	if (radius.charge)
	{
		g_snprintf(buf, sizeof(buf) - 1, "%d", radius.charge);
		xmlNewProp(child, (xmlChar*)"charge", (xmlChar*)buf);
	}
	if (radius.cn != -1)
	{
		g_snprintf(buf, sizeof(buf) - 1, "%d", radius.cn);
		xmlNewProp(child, (xmlChar*)"cn", (xmlChar*)buf);
	}
	if (radius.spin != GCU_N_A_SPIN)
		xmlNewProp(child, (xmlChar*)"spin", (xmlChar*)((radius.spin == GCU_LOW_SPIN)? "low": "high"));
	return true;
}
