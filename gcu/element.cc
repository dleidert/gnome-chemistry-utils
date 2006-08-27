// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * element.cc 
 *
 * Copyright (C) 2002-2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "element.h"
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include <glib.h>
#include <locale.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <libintl.h>
#include <math.h>
#include <string.h>
#include <glib/gi18n-lib.h>
#include <goffice/utils/go-math.h>

static set<string>units;

static void ReadValue (char const *source, GcuValue &value)
{
	char *buf, *dot;
	value.value = strtod (source, &buf);
	dot = strchr (source, '.');
	value.prec = (dot)? buf - dot - 1: 0;
	value.delta = (*buf == '(')? strtol (buf + 1, NULL, 10): 0;
}

static void ReadDimensionalValue (char const *source, GcuDimensionalValue &value)
{
	char *buf, *dot;
	value.value = strtod (source, &buf);
	dot = strchr (source, '.');
	value.prec = (dot)? buf - dot - 1: 0;
	value.delta = (*buf == '(')? strtol (buf + 1, NULL, 10): 0;
}

namespace gcu
{

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

} // namespace gcu

using namespace gcu;

EltTable Table;

EltTable::EltTable()
{
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#ifdef ENABLE_NLS
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
	xmlDocPtr xml;
	char* DefaultName;
	char *lang = getenv ("LANG");
	setlocale (LC_ALL, lang);
	char *old_num_locale, *buf, *num, *dot;
	unsigned char Z;
	map <string, string> Langs;
	Langs["de"] = _("German");
	Langs["fr"] = _("French");
	Langs["it"] = _("Italian");
	Langs["pl"] = _("Polish");
	Langs["ru"] = _("Russian");
	if (!(xml = xmlParseFile (PKGDATADIR"/elements.xml")))
	{
		g_error (_("Can't find and read elements.xml"));
	}
	old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	xmlNode* node = xml->children, *child;
	if (strcmp ((const char*) node->name, "gpdata")) g_error (_("Incorrect file format: elements.xml"));
	node = node->children;
	Element* Elt;
	while (node)
	{
		if (strcmp ((const char*) node->name, "text"))
		{
			if (strcmp ((const char*) node->name, "element")) g_error (_("Incorrect file format: elements.xml"));
			buf = (char*) xmlGetProp (node, (xmlChar*) "symbol");
			num = (char*) xmlGetProp (node, (xmlChar*) "Z");
			Elt = new Element (Z = atoi (num), buf);
			xmlFree (num);
			xmlFree (buf);
			num = (char*) xmlGetProp (node, (xmlChar*) "max_bonds");
			Elt->m_MaxBonds = atoi (num);
			xmlFree (num);
			num = (char*) xmlGetProp (node, (xmlChar*) "weight");
			Elt->m_Weight = strtod (num, &buf);
			dot = strchr (num, '.');
			Elt->m_WeightPrec = (dot)? buf - dot - 1: 0;
			child = node->children;
			DefaultName = NULL;
			while (child)
			{
				if (!strcmp ((const char*) child->name, "text")) {
					child = child->next;
					continue;
				}
				if (!strcmp((const char*)child->name, "name")) {
					buf = (char*) xmlNodeGetLang (child);
					if ((buf) && (lang)){
						string Lang = Langs[buf];
						char *Name = (char*) xmlNodeGetContent (child);
						if (Lang.length ())
							Elt->names[Lang] = Name;
						if (!strncmp (lang, buf, 2))
							Elt->name = Name;
						xmlFree (Name);
					} else if (!buf) {
						DefaultName = (char*) xmlNodeGetContent (child);
						Elt->names[_("English")] = DefaultName;
					}
					xmlFree (buf);
				} else if (!strcmp ((const char*) child->name, "color")) {
					buf = (char*) xmlGetProp (child, (xmlChar*) "red");
					if (buf) {
						Elt->m_DefaultColor[0] = strtod (buf, NULL);
						xmlFree (buf);
					}
					buf = (char*) xmlGetProp (child, (xmlChar*) "green");
					if (buf) {
						Elt->m_DefaultColor[1] = strtod (buf, NULL);
						xmlFree (buf);
					}
					buf = (char*) xmlGetProp (child, (xmlChar*) "blue");
					if (buf) {
						Elt->m_DefaultColor[2] = strtod (buf, NULL);
						xmlFree (buf);
					}
				}				child = child->next;
			}
			if ((Elt->name.length () == 0) && DefaultName) Elt->name = DefaultName;
			if (DefaultName)
				xmlFree (DefaultName);
			AddElement (Elt);
		}
		node = node->next;
	}
	setlocale (LC_NUMERIC, old_num_locale);
	g_free (old_num_locale);
	xmlFreeDoc (xml);
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
	if ((unsigned) Elt->GetZ() >= Elements.size()) Elements.resize(Elements.size() + 10);
	Elements[Elt->GetZ()] = Elt;
	EltsMap[Elt->GetSymbol()] = Elt;
}

Element::Element(int Z, const char* Symbol)
{
	m_Z = Z;
	strncpy(m_Symbol, Symbol, 3);
	m_Symbol[3] = 0;
	m_MaxBonds = 0;
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
	m_DefaultColor[0] = m_DefaultColor[1] = m_DefaultColor[2] = 0.0;
	if (m_Z <= 2) {
		m_nve = m_tve = m_Z;
		m_maxve = 2;
	} else if (m_Z <= 10) {
		m_nve = m_tve = m_Z - 2;
		m_maxve = 8;
	} else if (m_Z <= 18) {
		m_nve = m_tve = m_Z - 10;
		m_maxve = 8;
	} else if (m_Z <= 29) {
		m_nve = m_tve = m_Z - 18;
		m_maxve = 18;
	} else if (m_Z <= 36) {
		m_tve = m_Z - 18;
		m_nve = m_tve - 10;
		m_maxve = 18;
	} else if (m_Z <= 47) {
		m_nve = m_tve = m_Z - 36;
		m_maxve = 18;
	} else if (m_Z <= 54) {
		m_tve = m_Z - 36;
		m_nve = m_tve - 10;
		m_maxve = 18;
	} else if (m_Z <= 70) {
		m_nve = m_tve = m_Z - 54;
		m_maxve = 32;
	} else if (m_Z <= 79) {
		m_tve = m_Z - 54;
		m_nve = m_tve - 14;
		m_maxve = 32;
	} else if (m_Z <= 86) {
		m_tve = m_Z - 54;
		m_nve = m_tve - 24;
		m_maxve = 32;
	} else if (m_Z <= 102) {
		m_nve = m_tve = m_Z - 86;
		m_maxve = 32;
	} else if (m_Z <= 111) {
		m_tve = m_Z - 86;
		m_nve = m_tve - 14;
		m_maxve = 32;
	} else { // Assume m_Z <= 118
		m_tve = m_Z - 86;
		m_nve = m_tve - 24;
		m_maxve = 32;
	}
}

Element::~Element()
{
	while (!m_radii.empty()) {
		GcuAtomicRadius *radius = m_radii.back();
		if (radius) {
			if (radius->scale) g_free(radius->scale);
			delete radius;
		}
		m_radii.pop_back();
	}
	while (!m_en.empty()) {
		GcuElectronegativity* en = m_en.back();
		if (en) {
			if (en->scale) g_free(en->scale);
			delete en;
		}
		m_en.pop_back();
	}
	while (!m_isotopes.empty ()) {
		delete (m_isotopes.back ());
		m_isotopes.pop_back ();
	}
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

gint Element::Z(const gchar* symbol)
{
	Element* Elt = Table[symbol];
	return (Elt)? Elt->GetZ(): 0;
}

Element* Element::GetElement(gint Z)
{
	return Table[Z];
}

Element* Element::GetElement(const gchar* symbol)
{
	return Table[symbol];
}

unsigned Element::GetMaxBonds(gint Z)
{
	Element* Elt = Table[Z];
	return (Elt)? Elt->GetMaxBonds(): 0;
}

bool Element::GetRadius(GcuAtomicRadius* radius)
{
	Element* Elt = Table[radius->Z];
	if (!Elt || !Elt->m_radii.size ())
		return false;
	for (int i = 0; Elt->m_radii[i]; i++)
	{
		if (radius->type != Elt->m_radii[i]->type) continue;
		if (radius->charge != Elt->m_radii[i]->charge) continue;
		if ((radius->cn >= 0) &&(radius->cn != Elt->m_radii[i]->cn)) continue;
		if ((radius->spin != GCU_N_A_SPIN) &&(radius->spin != Elt->m_radii[i]->spin)) continue;
		if (!radius->scale)
		{
			*radius = *Elt->m_radii[i];
			return true;
		}
		else if (!strcmp(radius->scale, Elt->m_radii[i]->scale))
		{
			radius->value = Elt->m_radii[i]->value;
			return true;
		}
	}
	return false;
}

bool Element::GetElectronegativity(GcuElectronegativity* en)
{
	Element* Elt = Table[en->Z];
	if (!Elt) return false;
	if (!en->scale)
	{
		*en = *Elt->m_en[0];
		return true;
	}
	for (int i = 0; Elt->m_en[i]; i++)
		if (!strcmp(en->scale, Elt->m_en[i]->scale))
		{
			en->value = Elt->m_en[i]->value;
			return true;
		}

		return false;
}

const GcuAtomicRadius** Element::GetRadii()
{
	return (const GcuAtomicRadius**) &m_radii.front();
}

const GcuElectronegativity** Element::GetElectronegativities()
{
	return (const GcuElectronegativity**) &m_en.front();
}

double Element::GetWeight (int Z, int &prec)
{
	Element* Elt = Table[Z];
	return (Elt)? Elt->GetWeight(prec): 0.;
}

void Element::LoadRadii ()
{
	xmlDocPtr xml;
	char *old_num_locale, *buf, *num;
	unsigned char Z;
	static bool loaded = false;
	if (loaded)
		return;
	if (!(xml = xmlParseFile (PKGDATADIR"/radii.xml")))
	{
		g_error (_("Can't find and read radii.xml"));
	}
	old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	xmlNode* node = xml->children, *child;
	if (strcmp ((const char*) node->name, "gpdata")) g_error (_("Incorrect file format: radii.xml"));
	node = node->children;
	Element* Elt;
	set<string>::iterator it = units.find ("pm");
	if (it == units.end ()) {
		units.insert ("pm");
		it = units.find ("pm");
	}
	while (node) {
		if (strcmp ((const char*) node->name, "text"))
		{
			if (strcmp ((const char*) node->name, "element")) g_error (_("Incorrect file format: radii.xml"));
			num = (char*) xmlGetProp (node, (xmlChar*) "Z");
			Elt = Table[Z = atoi (num)];
			child = node->children;
			while (child)
			{
				if (!strcmp ((const char*) child->name, "text")) {
					child = child->next;
					continue;
				}
				if (!strcmp ((const char*) child->name, "radius")) {
					GcuAtomicRadius* radius = new GcuAtomicRadius;
					radius->Z = Z;	//FIXME: is it really useful there?
					buf = (char*) xmlGetProp (child, (xmlChar*) "type");
					if (!buf ||
						((!((!strcmp (buf, "covalent")) && (radius->type = GCU_COVALENT))) &&
						(!((!strcmp (buf, "vdW")) && (radius->type = GCU_VAN_DER_WAALS))) &&
						(!((!strcmp (buf, "ionic")) && (radius->type = GCU_IONIC))) &&
						(!((!strcmp (buf, "metallic")) && (radius->type = GCU_METALLIC))) &&
						(!((!strcmp (buf, "atomic")) && ((radius->type = GCU_ATOMIC) || true))))) {
						//invalid radius
						delete radius;
						if (buf)
							xmlFree (buf);
						continue;
					}
					buf = (char*) xmlGetProp (child, (xmlChar*) "scale");
					if (buf) {
						radius->scale = g_strdup (buf);
						xmlFree (buf);
					} else
						radius->scale = NULL;
					buf = (char*) xmlGetProp (child, (xmlChar*) "charge");
					if (buf) {
						radius->charge = strtol (buf, NULL, 10);
						xmlFree (buf);
					} else
						radius->charge = 0;
					buf = (char*) xmlGetProp (child, (xmlChar*) "cn");
					if (buf) {
						radius->cn = strtol (buf, NULL, 10);
						xmlFree (buf);
					} else
						radius->cn = -1;
					buf = (char*) xmlGetProp (child, (xmlChar*)"spin");
					if ((!buf) ||
						(!((!strcmp (buf, "low")) && (radius->spin = GCU_LOW_SPIN))) &&
						(!((!strcmp (buf, "high")) && (radius->spin = GCU_HIGH_SPIN))))
						radius->spin = GCU_N_A_SPIN;
					if (buf)
						xmlFree (buf);
					buf = (char*) xmlGetProp (child, (xmlChar*) "value");
					if (buf) {
						ReadDimensionalValue (buf, radius->value) ;
						radius->value.unit = (*it).c_str ();
						Elt->m_radii.push_back (radius);
						xmlFree (buf);
					} else
						delete radius;
				} else
					g_error ("Invalid radius node");
				child = child->next;
			}
			Elt->m_radii.push_back (NULL);
		}
		node = node->next;
	}
	setlocale (LC_NUMERIC, old_num_locale);
	g_free (old_num_locale);
	xmlFreeDoc (xml);
	loaded = true;
}

void Element::LoadElectronicProps ()
{
	xmlDocPtr xml;
	char *old_num_locale, *buf, *num, *dot, *end;
	unsigned char Z;
	unsigned i;
	static bool loaded = false;
	if (loaded)
		return;
	if (!(xml = xmlParseFile (PKGDATADIR"/elecprops.xml")))
	{
		g_error (_("Can't find and read elecprops.xml"));
	}
	old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	xmlNode* node = xml->children, *child;
	if (strcmp ((const char*) node->name, "gpdata")) g_error (_("Incorrect file format: elecprops.xml"));
	node = node->children;
	Element* Elt;
	while (node) {
		if (strcmp ((const char*) node->name, "text")) {
			if (strcmp ((const char*) node->name, "element")) g_error (_("Incorrect file format: elecprops.xml"));
			num = (char*) xmlGetProp (node, (xmlChar*) "Z");
			Elt = Table[Z = atoi (num)];
			child = node->children;
			while (child) {
				if (!strcmp ((const char*) child->name, "text")) {
					child = child->next;
					continue;
				}
				if (!strcmp ((const char*) child->name, "en")) {
					GcuElectronegativity* en = new GcuElectronegativity;
					en->Z = Z;	//FIXME: is it really useful there?
					buf = (char*) xmlGetProp (child, (xmlChar*) "scale");
					if (buf) {
						en->scale = g_strdup (buf);
						xmlFree (buf);
					} else
						en->scale = NULL;
					buf = (char*) xmlGetProp (child, (xmlChar*) "value");
					if (buf) {
						en->value.value = strtod (buf, &end);
						dot = strchr (buf, '.');
						en->value.prec = (dot)? end - dot - 1: 0;
						en->value.delta = 0; // we should use a generic parser
						Elt->m_en.push_back (en);
						xmlFree (buf);
					} else
						delete en;	//without a value, the structure is useless and is discarded
				} else if (!strcmp ((const char*) child->name, "config")) {
					buf = (char*) xmlNodeGetContent (child);
					char *cur = buf;
					bool nonvoid = false;
					if (buf[0] == '[') {
						Elt->ElecConfig.append (buf, 4);
						cur += 4;
						nonvoid = true;
					}
					while (cur && *cur) {
						if (nonvoid) {
							cur++;
							Elt->ElecConfig.append (" ");
						}
						Elt->ElecConfig.append (cur, 2);
						cur += 2;
						i = 1;
						while (cur[i] > ' ')
							i++;
						Elt->ElecConfig.append ("<sup>");
						Elt->ElecConfig.append (cur, i);
						Elt->ElecConfig.append ("</sup>");
						cur += i;
					}
					Elt->ElecConfig.append (" ");
					xmlFree (buf);
				} else if (!strcmp ((const char*) child->name, "ei")) {
					unsigned rank;
					buf = (char*) xmlGetProp (child, (xmlChar*) "rank");
					if (buf) {
						rank = strtol (buf, NULL, 10);
						xmlFree (buf);
					} else
						rank = 1;
					if ((i = Elt->m_ei.size ()) < rank) {
						Elt->m_ei.resize (rank);
						for (; i < rank; i++)
							Elt->m_ei[i].value = go_nan;
					}
					rank--;
					buf = (char*) xmlGetProp (child, (xmlChar*) "value");
					if (buf) {
						ReadValue (buf, (GcuValue&)Elt->m_ei[rank]) ;
						xmlFree (buf);
					} else {
						//no need to read the unit
						Elt->m_ei[rank].value = go_nan;
						break;
					}
					buf = (char*) xmlGetProp (child, (xmlChar*) "unit");
					if (buf) {
						string str(buf);
						set<string>::iterator it = units.find (str);
						if (it == units.end ()) {
							units.insert (str);
							it = units.find (str);
						}
						Elt->m_ei[rank].unit = (*it).c_str ();
						xmlFree (buf);
					} else
						Elt->m_ei[rank].unit = "MJ.mol<sup>-1</sup>";
				} else if (!strcmp ((const char*) child->name, "ae")) {
					unsigned rank;
					buf = (char*) xmlGetProp (child, (xmlChar*) "rank");
					if (buf) {
						rank = strtol (buf, NULL, 10);
						xmlFree (buf);
					} else
						rank = 1;
					if ((i = Elt->m_ae.size ()) < rank) {
						Elt->m_ae.resize (rank);
						for (; i < rank; i++)
							Elt->m_ae[i].value = go_nan;
					}
					rank--;
					buf = (char*) xmlGetProp (child, (xmlChar*) "value");
					if (buf) {
						ReadValue (buf, (GcuValue&)Elt->m_ae[rank]) ;
						xmlFree (buf);
					} else {
						//no need to read the unit
						Elt->m_ae[rank].value = go_nan;
						break;
					}
					buf = (char*) xmlGetProp (child, (xmlChar*) "unit");
					if (buf) {
						string str(buf);
						set<string>::iterator it = units.find (str);
						if (it == units.end ()) {
							units.insert (str);
							it = units.find (str);
						}
						Elt->m_ae[rank].unit = (*it).c_str ();
						xmlFree (buf);
					} else
						Elt->m_ae[rank].unit = "kJ.mol<sup>-1</sup>";
				} else
					g_error ("Invalid property node");
				child = child->next;
			}
			Elt->m_en.push_back (NULL);
		}
		node = node->next;
	}
	setlocale (LC_NUMERIC, old_num_locale);
	g_free (old_num_locale);
	xmlFreeDoc (xml);
	loaded = true;
}

void Element::LoadIsotopes ()
{
	xmlDocPtr xml;
	char *old_num_locale, *num;
	unsigned char Z;
	static bool loaded = false;
	if (loaded)
		return;
	if (!(xml = xmlParseFile (PKGDATADIR"/isotopes.xml")))
	{
		g_error (_("Can't find and read isotopes.xml"));
	}
	old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	xmlNode* node = xml->children, *child;
	if (strcmp ((const char*) node->name, "gpdata")) g_error (_("Incorrect file format: isotopes.xml"));
	node = node->children;
	Element *Elt;
	Isotope *Is;
	int minA, maxA, niso;
	while (node) {
		if (strcmp ((const char*) node->name, "text")) {
			if (strcmp ((const char*) node->name, "element")) g_error (_("Incorrect file format: isotopes.xml"));
			minA = maxA = niso = 0;
			num = (char*) xmlGetProp (node, (xmlChar*) "Z");
			Elt = Table[Z = atoi (num)];
			xmlFree (num);
			if (Elt == NULL)	// This should not occur
				continue;
			child = node->children;
			while (child) {
				if (!strcmp ((const char*) child->name, "text")) {
					child = child->next;
					continue;
				}
				if (!strcmp ((const char*) child->name, "isotope")) {
					Is = new Isotope ();
					num = (char*) xmlGetProp (child, (xmlChar*) "A");
					if (num) {
						Is->A = strtol (num, NULL, 10);
						xmlFree (num);
					}
					num = (char*) xmlGetProp (child, (xmlChar*) "weight");
					if (num) {
						ReadValue (num, Is->mass);
						xmlFree (num);
					}
					num = (char*) xmlGetProp (child, (xmlChar*) "abundance");
					if (num) {
						ReadValue (num, Is->abundance);
						xmlFree (num);
						niso++;
						if (minA == 0)
							minA = maxA = Is->A;
						else {
							if (minA > Is->A)
								minA = Is->A;
							else if (maxA < Is->A)
								maxA = Is->A;
						}
					}
					Elt->m_isotopes.push_back (Is);
				}
				child = child->next;
			}
			if (minA > 0) {
				IsotopicPattern *pattern = new IsotopicPattern (minA, maxA);
				vector<Isotope*>::iterator i, iend = Elt->m_isotopes.end ();
				for (i = Elt->m_isotopes.begin (); i != iend; i++) {
					if ((*i)->abundance.value != 0.)
						pattern->SetValue ((*i)->A, (*i)->abundance.value);
				}
				pattern->Normalize ();
				niso = pattern->GetMonoNuclNb ();
				i = Elt->m_isotopes.begin ();
				while ((*i)->A != niso)
					i++;
				pattern->SetMonoMass ((*i)->mass.value);
				Elt->m_patterns.push_back (pattern);
			}
		}
		node = node->next;
	}
	setlocale (LC_NUMERIC, old_num_locale);
	g_free (old_num_locale);
	xmlFreeDoc (xml);
	loaded = true;
}

void Element::LoadAllData ()
{
	LoadRadii ();
	LoadElectronicProps ();
	LoadIsotopes ();
}

IsotopicPattern *Element::GetIsotopicPattern (unsigned natoms)
{
	if (m_patterns.size () == 0)
		return NULL;
	IsotopicPattern *pat, *pattern, *result = NULL;
	if (natoms == 0)
		return NULL;
	unsigned i = 1;
	while ((natoms & 1) == 0) {
		natoms >>= 1;
		i++;
	}
	while (natoms) {
		if (i == 1) {
			result = m_patterns[0];
			result->Ref ();
		} else if (natoms & 1) {
			while (m_patterns.size () < i) {
				pat = m_patterns[m_patterns.size () - 1]->Square ();
				pattern = pat->Simplify ();
				pat->Unref ();
				m_patterns.push_back (pattern);
			}
			pattern = m_patterns[i - 1];
			if (result) {
				pat = result->Multiply (*pattern);
				result->Unref ();
				result = pat->Simplify ();
				pat->Unref ();
			} else {
				result = pattern;
				result->Ref ();
			}
		}
		natoms >>= 1;
		i++;
	}
	return result;
}

GcuDimensionalValue const *Element::GetIonizationEnergy (unsigned rank)
{
	return (rank <= m_ei.size ())? &m_ei[rank - 1]: NULL;
}

GcuDimensionalValue const *Element::GetElectronAffinity (unsigned rank)
{
	return (rank <= m_ae.size ())? &m_ae[rank - 1]: NULL;
}
