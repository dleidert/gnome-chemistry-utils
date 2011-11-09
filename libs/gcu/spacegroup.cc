// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * spacegroup.cc - Handle Crystallographic Space Groups.
 *  
 * Copyright (C) 2007-2009 by Jean Bréfort
 * 
 * This file was originally part of the Open Babel project.
 * For more information, see <http://openbabel.sourceforge.net/>
 *  
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "config.h"

#include "spacegroup.h"
#include "transform3d.h"

#include <gsf/gsf-input-gio.h>
#include <gsf/gsf-libxml.h>
#include <glib/gi18n-lib.h>

#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <locale>

#include <cstdarg>
#include <cstdlib>

using namespace std;

namespace gcu
{

class SpaceGroups
{
public:
	SpaceGroups ();
	virtual ~SpaceGroups ();

	unsigned int GetSize () {return sgs.size();}
	bool Inited () {return m_Init;}
	void Init ();

	map <string, SpaceGroup const*> sgbn;
	vector <list<SpaceGroup const*> > sgbi;
	set <SpaceGroup*> sgs;
	bool m_Init;
};

SpaceGroups::SpaceGroups ()
{
	sgbi.assign (230, list <SpaceGroup const*> ());
	m_Init = false;
}

SpaceGroups::~SpaceGroups ()
{
	set <SpaceGroup *>::iterator i, end = sgs.end ();
	for (i = sgs.begin (); i != end; i++)
		delete (*i);
}

static SpaceGroups _SpaceGroups;

typedef struct {
	SpaceGroup *group;
} SGReadState;

static void
transform_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	SGReadState	*state = (SGReadState *) xin->user_state;
	state->group->AddTransform (xin->content->str);
}

static void
group_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	SGReadState	*state = (SGReadState *) xin->user_state;
	state->group = new SpaceGroup();
	string HMs;
	while (*attrs) {
		if (!strcmp (reinterpret_cast <char const *> (*attrs), "id")) {
			state->group->SetId (atoi (reinterpret_cast <char const *> (attrs[1])));
		} else if (!strcmp (reinterpret_cast <char const *> (*attrs), "HM")) {
			state->group->SetHMName (reinterpret_cast <char const *> (attrs[1]));
		} else if (!strcmp (reinterpret_cast <char const *> (*attrs), "HMs")) {
			HMs = reinterpret_cast <char const *> (attrs[1]);
		} else if (!strcmp (reinterpret_cast <char const *> (*attrs), "Hall")) {
			state->group->SetHallName (reinterpret_cast <char const *> (attrs[1]));
		}
		attrs += 2;
	}
	if (HMs.length() > 0)
		state->group->RegisterSpaceGroup(1, HMs.c_str());
	else
		state->group->RegisterSpaceGroup();
}

static GsfXMLInNode const sg_dtd[] = {
GSF_XML_IN_NODE (GROUPS, GROUPS, -1, "list", GSF_XML_NO_CONTENT, NULL, NULL),
	GSF_XML_IN_NODE (GROUPS, GROUP, -1, "group", GSF_XML_NO_CONTENT, group_start, NULL),
		GSF_XML_IN_NODE (GROUP, TRANSFORM, -1, "transform", GSF_XML_CONTENT, NULL, transform_end),
GSF_XML_IN_NODE_END
};

void SpaceGroups::Init ()
{
	GError *error = NULL;
	// do not use BODR space groups database for now until it is fully verified and fixed
	GsfInput *in = gsf_input_gio_new_for_path (PKGDATADIR"/space-groups.xml", &error);
	if (error) {
		cerr << _("Could not find space groups definitions in ") << BODR_PKGDATADIR"/space-groups.xml" << endl;
		cerr << _("Error is: ") << error->message << endl;
		g_error_free (error);
		return;
	}
	SGReadState state;

	state.group = NULL;

	GsfXMLInDoc *xml = gsf_xml_in_doc_new (sg_dtd, NULL);
	if (!gsf_xml_in_doc_parse (xml, in, &state))
		cerr << gsf_input_name (in) << _(" is corrupt!"),
	gsf_xml_in_doc_free (xml);
	m_Init = true;
}

SpaceGroup::SpaceGroup ():
	m_Id(0)
{
}

SpaceGroup::~SpaceGroup ()
{
	list <Transform3d*>::iterator i, end = m_Transforms.end();
	for (i = m_Transforms.begin(); i != end; ++i)
		delete *i;
}

/*! 
*/
void SpaceGroup::AddTransform(const string &s)
{
	Matrix m (0.);
	Vector v;
	istringstream iss(s);
	locale cLocale ("C");
	iss.imbue (cLocale);

	if (s.find (',') != string::npos) {
		string row;
		size_t i, j;
		bool neg;
		for (i = 0; i < 3; i++) {
			getline (iss, row, ',');
			j = 0;
			neg = false;
			while (j < row.length ()) {
				switch (row[j]) {
				case '.':
				case '0': { // anticipating something like 0.5 or .33333
					char *end;
					double *t = NULL;
					switch (i) {
					case 0:
						t = &v.GetRefX ();
						break;
					case 1:
						t = &v.GetRefY ();
						break;
					case 2:
						t = &v.GetRefZ ();
						break;
					}
					*t = g_ascii_strtod (row.c_str () + j, &end);
 					j = end - row.c_str() - 1;
					if (neg)
						*t = -*t;
					break;
				}
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
					if (row[j+1] == '/') {
						double *t = NULL;
						switch (i) {
						case 0:
							t = &v.GetRefX ();
							break;
						case 1:
							t = &v.GetRefY ();
							break;
						case 2:
							t = &v.GetRefZ ();
							break;
						}
						*t = ((double) (row[j] - '0')) / (row[j+2] - '0');
						if (neg)
							*t = -*t;
					}
					j +=2;
					break;
				case '-':
					neg = true;
					break;
				case '+':
					neg = false;
					break;
				case 'x':
					m (i, 0) = (neg)? -1.: 1.;
					break;
				case 'y':
					m (i, 1) = (neg)? -1.: 1.;
					break;
				case 'z':
					m (i, 2) = (neg)? -1.: 1.;
					break;
				}
				j++;
			}
		}
	} else if (s.find (' ') != string::npos) {
		/* supposing the string is a list of at least 12 float values. If there are
		16, the last four are 0., 0., 0. and 1. and are not needed */
		iss >> m (0,0) >> m (0,1) >> m (0,2) >> v.GetRefX ();
		iss >> m (1,0) >> m (1,1) >> m (1,2) >> v.GetRefY ();
		iss >> m (2,0) >> m (2,1) >> m (2,2) >> v.GetRefZ ();
	}
	if (v.GetX () < 0)
		v.GetRefX () += 1.;
	else if (v.GetX () >= 1.)
		v.GetRefX () -= 1.;
	if (v.GetY () < 0)
		v.GetRefY () += 1.;
	else if (v.GetY () >= 1.)
		v.GetRefY () -= 1.;
	if (v.GetZ () < 0)
		v.GetRefZ () += 1.;
	else if (v.GetZ () >= 1.)
		v.GetRefZ () -= 1.;
	m_Transforms.push_back (new Transform3d (m, v));
}

/*! 
*/
list<Vector> SpaceGroup::Transform (const Vector &v) const
{
	static double prec = 2e-5;
	list<Vector> res;
	list <Transform3d*>::const_iterator i, iend = m_Transforms.end ();
	for (i = m_Transforms.begin (); i!= iend; i++) {
		Vector t;
		t = *(*i) * v;
		if (t.GetX () < 0.)
			t.GetRefX () += 1.;
		else if (t.GetX () >= 1.)
			t.GetRefX () -= 1.;
		if (t.GetY () < 0.)
			t.GetRefY () += 1.;
		else if (t.GetY () >= 1.)
			t.GetRefY () -= 1.;
		if (t.GetZ () < 0.)
			t.GetRefZ () += 1.;
		else if (t.GetZ () >= 1.)
			t.GetRefZ () -= 1.;
		list <Vector>::iterator j, jend = res.end ();
		bool duplicate = false;
		for (j = res.begin (); j != jend; j++)
			if (fabs(t.GetX () - (*j).GetX ()) < prec &&
				fabs(t.GetY () - (*j).GetY ()) < prec &&
				fabs(t.GetZ () - (*j).GetZ ()) < prec) {
				duplicate = true;
				break;
			}
		if (!duplicate)
			res.push_back (t);
	}
	return res;
}

/*! 
*/
Transform3d const * SpaceGroup::GetFirstTransform (list <Transform3d*>::const_iterator &i) const
{
	i = m_Transforms.begin ();
	return (i == m_Transforms.end())? reinterpret_cast <Transform3d *> (NULL): *i;
}

/*! 
*/
Transform3d const * SpaceGroup::GetNextTransform (list <Transform3d*>::const_iterator &i) const
{
	i++;
	return (i == m_Transforms.end ())? reinterpret_cast <Transform3d *> (NULL): *i;
}

/*! 
*/
SpaceGroup const *SpaceGroup::GetSpaceGroup (char const *name)
{
	if (!_SpaceGroups.Inited ())
		_SpaceGroups.Init ();
	if (!name)
		return NULL;
	return (_SpaceGroups.sgbn.find (name)!= _SpaceGroups.sgbn.end ())? _SpaceGroups.sgbn[name]: NULL;
}

/*! 
*/
SpaceGroup const *SpaceGroup::GetSpaceGroup (string const &name)
{
	if (!_SpaceGroups.Inited ())
		_SpaceGroups.Init ();
	return (_SpaceGroups.sgbn.find (name) != _SpaceGroups.sgbn.end ())? _SpaceGroups.sgbn[name]: NULL;
}

/*! 
*/
SpaceGroup const *SpaceGroup::GetSpaceGroup (unsigned id)
{
	if (!_SpaceGroups.Inited ())
		_SpaceGroups.Init ();
	return (id > 0 && id <= 230)? _SpaceGroups.sgbi[id - 1].front (): NULL;
}

/*! 
*/
void SpaceGroup::RegisterSpaceGroup (int nb, ...)
{
	_SpaceGroups.sgs.insert (this);
	if (m_Id > 0 && m_Id <= 230)
		_SpaceGroups.sgbi[m_Id - 1].push_back (this);
	if (m_HMName.length () > 0 && _SpaceGroups.sgbn[m_HMName] == NULL)
		_SpaceGroups.sgbn[m_HMName] = this;
	if (m_HallName.length () > 0 && _SpaceGroups.sgbn[m_HallName] == NULL)
		_SpaceGroups.sgbn[m_HallName] = this;
	if (nb == 0)
		return;
	va_list args;
	va_start (args, nb);
	string name;
	for (int i = 0; i < nb; i++) {
		name=va_arg (args, const char *);
		if (name.length () > 0 && _SpaceGroups.sgbn[name] == NULL)
			_SpaceGroups.sgbn[name] = this;
	}
	va_end (args);
}

/*! 
*/
bool SpaceGroup::operator== (SpaceGroup const &sg) const
{
	if (m_Transforms.size () != sg.m_Transforms.size ())
		return false;
	set <string> s0, s1;
	list <Transform3d*>::const_iterator i, iend = m_Transforms.end();
	for (i = m_Transforms.begin (); i != iend; i++)
		s0.insert ((*i)->DescribeAsString ());
	iend = sg.m_Transforms.end ();
	for (i = sg.m_Transforms.begin (); i != iend; i++)
		s1.insert ((*i)->DescribeAsString ());
	if (s0.size () != s1.size ())
		return false;
	set <string>::iterator j, jend = s0.end ();
	for (j = s0.begin(); j != jend; j++)
		if (s1.find (*j) == s1.end ())
			return false;
	return true;
}

/*! 
*/
bool SpaceGroup::IsValid () const
{
	if (!m_Transforms.size ())
		return false;
	list <Transform3d *>::const_iterator i, iend = m_Transforms.end ();
	map <string, Transform3d *> T;
	for (i = m_Transforms.begin (); i != iend; i++) {
		if (T.find((*i)->DescribeAsString()) != T.end()) {
			cerr << _("Duplicated transform: ") << (*i)->DescribeAsString() << endl;
			return false;
		}
		T[(*i)->DescribeAsString ()] = *i;
	}
	// calculate all products and check if they are in the group
	map <string, Transform3d *>::iterator j, k, end = T.end ();
	string s;
	bool has_inverse;
	for (j = T.begin (); j != end; j++) {
		has_inverse = false;
		for (k = T.begin (); k != end; k++) {
			s = (*(*j).second * *(*k).second).DescribeAsString ();
			if (T.find (s) == end) {
				cerr << _("Invalid transform: ") << (*j).first << " * " << (*k).first << " = " << s << endl;
				return false;
			}
			if (!has_inverse && s == "x,y,z")
				has_inverse = true;
		}
		if (!has_inverse) {
			cerr << _("Transform with no inverse: ") << (*j).first << endl;
			return false;
		}
	}
	return true;
}

/*! 
*/
SpaceGroup const *SpaceGroup::Find (SpaceGroup* group)
{
	if (!_SpaceGroups.Inited ())
		_SpaceGroups.Init ();
	SpaceGroup const *found = NULL;
	if (group->m_HallName.length () > 0 && _SpaceGroups.sgbn.find (group->m_HallName) != _SpaceGroups.sgbn.end ()) {
		found = _SpaceGroups.sgbn[group->m_HallName];
		if (!found)
			cerr << _("Unknown space group error, please file a bug report.") << endl;
		if (group->m_Transforms.size () && *found  != *group) {
			unsigned id = group->GetId ();
			if (id == 3 || id == 68)
				goto find_by_id; // theses groups have duplicates
			cerr << _("Space group error, please file a bug report.") << endl;
		}
		/* even if there is an error (this should not occur) return the found group, since
		Hall names are secure */
		return found;
	}
	if (group->m_HMName.length () > 0 &&
		_SpaceGroups.sgbn.find (group->m_HMName) != _SpaceGroups.sgbn.end () &&
		(found = _SpaceGroups.sgbn[group->m_HMName])) {
		if (*found == *group)
			return found;
		if (group->m_Transforms.size ()) {
			list <SpaceGroup const *>::const_iterator i, end = _SpaceGroups.sgbi[found->m_Id - 1].end ();
			for (i = _SpaceGroups.sgbi[found->m_Id - 1].begin (); i!= end; i++)
				if ((**i) == *group)
					return *i;
			cerr << _("Unknown space group error, please file a bug report.") << endl;
			return NULL;
		} else if (group->m_Transforms.size() == 0) {
			int n = 0;
			list <SpaceGroup const *>::const_iterator i, end = _SpaceGroups.sgbi[group->m_Id].end ();
			for (i = _SpaceGroups.sgbi[group->m_Id].begin (); i!= end; i++)
				if ((*i)->m_HMName == group->m_HMName)
					n++;
			if (n > 1)
				cerr << _("Ambiguous space group with incomplete definition.") << endl;
			return found;
		}
	} else if (group->m_Id > 0 && group->m_Id <= 230) {
find_by_id:
		if (group->m_Transforms.size ()) { 
			list <SpaceGroup const *>::const_iterator i, end = _SpaceGroups.sgbi[group->m_Id - 1].end ();
			for (i = _SpaceGroups.sgbi[group->m_Id - 1].begin (); i!= end; i++)
				if ((**i) == *group)
					return *i;
		} else if (group->m_Transforms.size () == 0) {
			if (_SpaceGroups.sgbi[group->m_Id - 1].size () > 1)
				cerr << _("Ambiguous space group with incomplete definition.") << endl;
			return _SpaceGroups.sgbi[group->m_Id - 1].front ();
		}
	}
	// If we are there, we need to make a hard search through the whole collection
	if (!group->IsValid()) {
		g_warning (_("Unknown space group with incomplete or wrong definition."));
		return NULL;
	}
	set<SpaceGroup*>::iterator i, end = _SpaceGroups.sgs.end();
	for (i = _SpaceGroups.sgs.begin(); i != end; i++)
		if (**i == *group)
			return *i;
	cerr << _("Unknown space group error, please file a bug report.") << endl;;
	return NULL;
}

std::list <SpaceGroup const *> &SpaceGroup::GetSpaceGroups (unsigned id)
{
	if (!_SpaceGroups.Inited ())
		_SpaceGroups.Init ();
	return _SpaceGroups.sgbi[id - 1];
}

}   //  namespace gcu
