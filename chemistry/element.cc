/* 
 * Gnome Chemistry Utils
 * element.cc 
 *
 * Copyright (C) 2002
 *
 * Developed by Jean Br�fort <jean.brefort@ac-dijon.fr>
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

#include "config.h"
#include "element.h"
#include <stdio.h>//FIXME: should be removed
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include <glib.h>
#include <locale.h>
#include <vector>
#include <map>
#include <string>
#include <libintl.h>
#include <string.h>
#define _(String) gettext(String)

using namespace gcu;
using namespace std;

class EltTable
{
public:
	EltTable();
	virtual ~EltTable();
	
	Element* operator [] (int Z);
	Element* operator [] (string Symbol);
	
	void AddElement(Element* Elt);

private:
	vector<Element*> Elements;
	map <string, Element*> EltsMap;
};

EltTable Table;

EltTable::EltTable()
{
	bindtextdomain(PACKAGE, DATADIR"/locale");
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain(PACKAGE);
	xmlDocPtr xml;
	char *old_num_locale, *tmp, *num;
	if (!(xml = xmlParseFile(DATADIR"/gchemutils/elements.xml")))
	{
		g_error(_("Can't find and read elements.xml"));
	}
	old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	xmlNode* node = xml->children;
	if (strcmp((const char*)node->name, "gpdata")) g_error(_("Uncorrect file format: elements.xml"));
	node = node->children;
	Element* Elt;
	while (node)
	{
		if (strcmp((const char*)node->name, "text"))
		{
			if (strcmp((const char*)node->name, "element")) g_error(_("Uncorrect file format: elements.xml"));
			tmp = (char*) xmlGetProp(node, (xmlChar*)"symbol");
			num = (char*) xmlGetProp(node, (xmlChar*)"Z");
			Elt = new Element(atoi(num), tmp);
			AddElement(Elt);
		}
		node = node->next;
	}
	setlocale(LC_NUMERIC, old_num_locale);
	g_free(old_num_locale);
	xmlFreeDoc(xml);
}

EltTable::~EltTable()
{
	map<string, Element*>::iterator i;
	for (i = EltsMap.begin(); i != EltsMap.end(); i++)
		if ((*i).second) delete (*i).second;
	EltsMap.clear();
	Elements.clear();
}

Element* EltTable::operator[](int Z)
{
	return Elements[Z];
}

Element* EltTable::operator[](string Symbol)
{
	return EltsMap[Symbol];
}

void EltTable::AddElement(Element* Elt)
{
	if (Elt->GetZ() >= Elements.size()) Elements.resize(Elements.size() + 10);
	Elements[Elt->GetZ()] = Elt;
	EltsMap[Elt->GetSymbol()] = Elt;
}

Element::Element(int Z, const char* Symbol)
{
	m_Z = Z;
	strncpy(m_Symbol, Symbol, 3);
	m_Symbol[3] = 0;
	m_BestSide = true;
	switch (m_Z)
	{
		case 6:
		case 14:
		case 32:
			m_DefaultValence = 4;
			break;
		case 5:
		case 7:
		case 13:
		case 15:
		case 33:
		case 51:
			m_DefaultValence = 3;
			break;
		case 8:
		case 16:
		case 34:
		case 52:
			m_BestSide = false;
		case 4:
			m_DefaultValence = 2;
			break;
		case 9:
		case 17:
		case 35:
		case 53:
			m_BestSide = false;
			m_DefaultValence = 1;
			break;
		case 2:
		case 10:
		case 28:
		case 36:
		case 54:
			m_DefaultValence = 0;
			break;
		default:
			m_DefaultValence = -1;
	}
}

Element::~Element()
{
}

const gchar* Element::Symbol(gint Z)
{
	Element* Elt = Table[Z];
	return (Elt)? Elt->GetSymbol(): NULL;
}

bool Element::BestSide(gint Z)
{
	Element* Elt = Table[Z];
	return (Elt)? Elt->GetBestSide(): true;
}

gint Element::Z(const gchar* name)
{
	Element* Elt = Table[name];
	return (Elt)? Elt->GetZ(): 0;
}

Element* Element::GetElement(gint Z)
{
	return Table[Z];
}

Element* Element::GetElement(const gchar* name)
{
	return Table[name];
}
