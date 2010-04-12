// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * crystaldoc.cc 
 *
 * Copyright (C) 2002-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <cstring>
#include "crystaldoc.h"
#include "crystalview.h"
#include "cylinder.h"
#include "matrix.h"
#include "objprops.h"
#include "spacegroup.h"
#include "sphere.h"
#include "transform3d.h"
#include "vector.h"
#include "xml-utils.h"
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <libintl.h>
#include <clocale>
#include <cmath>
#include <set>
#include <vector>
#include <GL/gl.h>
#include <cstring>
#include <sstream>

#define __max(x,y)  ((x) > (y)) ? (x) : (y)
#define __min(x,y)  ((x) < (y)) ? (x) : (y)

using namespace std;

namespace gcu
{
	
gchar const *LatticeName[] = {
	"simple cubic",
	"body-centered cubic",
	"face-centered cubic",
	"hexagonal",
	"tetragonal",
	"body-centered tetragonal",
	"orthorhombic",
	"base-centered orthorhombic",
	"body-centered orthorhombic",
	"face-centered orthorhombic",
	"rhombohedral",
	"monoclinic",
	"base-centered monoclinic",
	"triclinic"
};

CrystalDoc::CrystalDoc (Application *App): GLDocument (App),
	m_SpaceGroup (NULL), m_AutoSpaceGroup (false)
{
	m_xmin = m_ymin = m_zmin = 0;
	m_xmax = m_ymax = m_zmax = 1;
}


CrystalDoc::~CrystalDoc()
{
	while (!AtomDef.empty ()) {
		delete AtomDef.front ();
		AtomDef.pop_front ();
	}
	while (!Atoms.empty ()) {
		delete Atoms.front ();
		Atoms.pop_front ();
	}
	while (!LineDef.empty ()) {
		delete LineDef.front ();
		LineDef.pop_front ();
	}
	while (!Lines.empty ()) {
		delete Lines.front ();
		Lines.pop_front ();
	}
	while (!Cleavages.empty ()) {
		delete Cleavages.front ();
		Cleavages.pop_front ();
	}
	while (!m_Views.empty ()) {
		m_Views.pop_back ();
	}
}

void CrystalDoc::Reinit()
{
	//destruction of lists
	while (!AtomDef.empty ()) {
		delete AtomDef.front ();
		AtomDef.pop_front ();
	}
	while (!Atoms.empty ()) {
		delete Atoms.front ();
		Atoms.pop_front ();
	}
	while (!LineDef.empty ()) {
		delete LineDef.front ();
		LineDef.pop_front ();
	}
	while (!Lines.empty ()) {
		delete Lines.front ();
		Lines.pop_front ();
	}
	while (!Cleavages.empty ()) {
		delete Cleavages.front ();
		Cleavages.pop_front ();
	}
	Init ();
}

void CrystalDoc::Init()
{
	m_a = m_b = m_c = 100;
	m_alpha = m_beta = m_gamma = 90;
	m_lattice = cubic;
	m_SpaceGroup = SpaceGroup::GetSpaceGroup (195);
	m_AutoSpaceGroup = false;
	m_xmin = m_ymin = m_zmin = 0;
	m_xmax = m_ymax = m_zmax = 1;
	m_bFixedSize = false;
	m_MaxDist = 0;
	if (m_Views.size() == 0)
	{
		CrystalView* pView = CreateNewView();
		m_Views.push_back(pView);
	}
}

void CrystalDoc::ParseXMLTree (xmlNode* xml)
{
	char *old_num_locale, *txt;
	xmlNodePtr node;
	bool bViewLoaded = false;

	Reinit ();
	old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	//look for generator node
	unsigned version = 0xffffff , major, minor, micro;
	node = xml->children;
	while (node) {
		if (!strcmp ((const char*)(node->name), "generator"))
			break;
		node = node->next;
	}
	if (node) {
		txt = (char*) xmlNodeGetContent (node);
		if (sscanf(txt, "Gnome Crystal %d.%d.%d", &major, &minor, &micro) == 3)
			version = micro + minor * 0x100 + major * 0x10000;
		xmlFree (txt);
	}
	node = xml->children;
	while(node) {
		if (!strcmp ((gchar*) node->name, "text"));
		else if (!strcmp ((gchar*) node->name, "lattice")) {
			txt = (char*) xmlNodeGetContent (node);
			int i = 0;
			while (strcmp (txt, LatticeName[i]) && (i < 14))
				i++;
			if (i < 14)
				m_lattice = (gcLattices)i;
			xmlFree (txt);
		} else if (!strcmp ((gchar*) node->name, "group")) {
			SpaceGroup *group = new SpaceGroup ();
			txt = (char*) xmlGetProp (node, (xmlChar*) "Hall");
			if (txt) {
				group->SetHallName (txt);
				xmlFree (txt);
			} else {
				txt = (char*) xmlGetProp (node, (xmlChar*) "HM");
				if (txt) {
					group->SetHMName (txt);
					xmlFree (txt);
				}
			}
			xmlNodePtr child = node->children;
			while (child) {
				if (!strcmp ((char const*) child->name, "transform")) {
					txt = (char*) xmlNodeGetContent (child);
					if (txt) {
						group->AddTransform (txt);
						xmlFree (txt);
					}
				}
				child = child->next;
			}
			m_SpaceGroup = SpaceGroup::Find (group);
			delete group;
		} else if (!strcmp ((gchar*) node->name, "cell")) {
			txt = (char*) xmlGetProp (node, (xmlChar*)"a");
			if (txt) {
				sscanf (txt, "%lg", &m_a);
				xmlFree (txt);
			}
			txt = (char*) xmlGetProp (node, (xmlChar*) "b");
			if (txt) {
				sscanf (txt, "%lg", &m_b);
				xmlFree (txt);
			}
			txt = (char*) xmlGetProp (node, (xmlChar*) "c");
			if (txt) {
				sscanf (txt, "%lg", &m_c);
				xmlFree (txt);
			}
			txt = (char*) xmlGetProp (node, (xmlChar*) "alpha");
			if (txt) {
				sscanf (txt, "%lg", &m_alpha);
				xmlFree (txt);
			}
			txt = (char*) xmlGetProp (node, (xmlChar*) "beta");
			if (txt) {
				sscanf(txt, "%lg", &m_beta);
				xmlFree (txt);
			}
			txt = (char*) xmlGetProp (node, (xmlChar*) "gamma");
			if (txt) {
				sscanf(txt, "%lg", &m_gamma);
				xmlFree (txt);
			}
		} else if (!strcmp ((gchar*) node->name, "size")) {
			ReadPosition (node, "start", &m_xmin, &m_ymin, &m_zmin);
			ReadPosition (node, "end", &m_xmax, &m_ymax, &m_zmax);
			txt = (char*) xmlGetProp (node, (xmlChar*) "fixed");
			if (txt) {
				if (!strcmp (txt, "true"))
					m_bFixedSize = true;
				xmlFree (txt);
			}
		} else if (!strcmp ((gchar*) node->name, "atom")) {
			CrystalAtom *pAtom = CreateNewAtom ();
			if (pAtom->Load (node))
				AddChild (pAtom);
			else
				delete pAtom;
		} else if (!strcmp ((gchar*) node->name, "line")) {
			CrystalLine *pLine = CreateNewLine ();
			if (pLine->Load (node))
				LineDef.push_back (pLine);
			else
				delete pLine;
		} else if (!strcmp ((gchar*) node->name, "cleavage")) {
			CrystalCleavage *pCleavage = CreateNewCleavage ();
			if (pCleavage->Load (node))
				Cleavages.push_back (pCleavage);
			else
				delete pCleavage;
		} else if (!strcmp ((gchar*) node->name, "view")) {
			if (!bViewLoaded) {
				m_Views.front ()->Load (node); //the first view is created with the document
				bViewLoaded = true;
			} else
				LoadNewView (node);
		}
		node = node->next;
	}
	setlocale (LC_NUMERIC, old_num_locale);
	g_free (old_num_locale);
	SetDirty (false);
	Update ();
}

bool CrystalDoc::LoadNewView(G_GNUC_UNUSED xmlNodePtr node)
{
	return true;
}

CrystalView *CrystalDoc::GetView()
{
	if (m_Views.size() == 0)
	{
		CrystalView* pView = CreateNewView();
		m_Views.push_back(pView);
	}
	return m_Views.front();
}

void CrystalDoc::Update()
{
	m_Empty = (AtomDef.empty() && LineDef.empty()) ? true : false;
	CrystalAtom Atom;
	CrystalLine Line;
	gdouble alpha = m_alpha * M_PI / 180;
	gdouble beta = m_beta * M_PI / 180;
	gdouble gamma = m_gamma * M_PI / 180;
	// Remove all atoms
	while (!Atoms.empty())
	{
		delete Atoms.front();
		Atoms.pop_front();
	}
	// Remove all bonds
	while (!Lines.empty())
	{
		delete Lines.front();
		Lines.pop_front();
	}

	// update space group
	m_SpaceGroup = FindSpaceGroup ();

	////////////////////////////////////////////////////////////
	//Establish list of atoms
	CrystalAtomList::iterator i, iend = AtomDef.end ();

	if (m_SpaceGroup) {
		Vector v;
		list<Vector> d;
		for (i = AtomDef.begin(); i != iend; i++) {
			v.SetX ((*i)->x ());
			v.SetY ((*i)->y ());
			v.SetZ ((*i)->z ());
			d = m_SpaceGroup->Transform (v);
			list<Vector>::iterator vi, viend = d.end();
			CrystalAtom atom (**i);
			for (vi=d.begin (); vi!= viend; vi++) {
				atom.SetCoords ((*vi).GetX(), (*vi).GetY(), (*vi).GetZ());
				Duplicate (atom);
			}
		}
	} else for (i = AtomDef.begin(); i != iend; i++) {
		Duplicate(**i);
		switch (m_lattice) {
		case body_centered_cubic:
		case body_centered_tetragonal:
		case body_centered_orthorhombic:
			Atom = **i;
			Atom.Move(0.5, 0.5, 0.5);
			Duplicate(Atom);
			break;
		case face_centered_cubic:
		case face_centered_orthorhombic:
			Atom = **i;
			Atom.Move(0.5, 0, 0.5);
			Duplicate(Atom);
			Atom = **i;
			Atom.Move(0, 0.5, 0.5);
			Duplicate(Atom);
		case base_centered_orthorhombic:
		case base_centered_monoclinic:
			Atom = **i;
			Atom.Move(0.5, 0.5, 0);
			Duplicate(Atom);
			break;
		default:
			break;
		}
	}
	
	////////////////////////////////////////////////////////////
	//Establish list of lines
	CrystalLineList::iterator j, jend = LineDef.end();
	for (j = LineDef.begin() ; j != jend ; j++)
	{
		switch ((*j)->Type())
		{
		case edges:
			Line = **j;
			Line.SetPosition(0 ,0, 0, 1, 0, 0);
			Duplicate(Line);
			Line.SetPosition(0 ,0, 0, 0, 1, 0);
			Duplicate(Line);
			Line.SetPosition(0 ,0, 0, 0, 0, 1);
			Duplicate(Line);
			break ;
		case diagonals:
			Line = **j;
			Line.SetPosition(0 ,0, 0, 1, 1, 1);
			Duplicate(Line);
			Line.SetPosition(1 ,0, 0, 0, 1, 1);
			Duplicate(Line);
			Line.SetPosition(0 ,1, 0, 1, 0, 1);
			Duplicate(Line);
			Line.SetPosition(1 ,1, 0, 0, 0, 1);
			Duplicate(Line);
			break ;
		case medians:
			Line = **j;
			Line.SetPosition(.5, .5, 0, .5, .5, 1);
			Duplicate(Line);
			Line.SetPosition(0, .5, .5, 1, .5, .5);
			Duplicate(Line);
			Line.SetPosition(.5, 0, .5, .5, 1, .5);
			Duplicate(Line);
			break ;
		case normal:
			Duplicate(**j) ;
			switch (m_lattice)
			{
			case body_centered_cubic:
			case body_centered_tetragonal:
			case body_centered_orthorhombic:
				Line = **j;
				Line.Move(0.5, 0.5, 0.5);
				Duplicate(Line);
				break;
			case face_centered_cubic:
			case face_centered_orthorhombic:
				Line = **j;
				Line.Move(0.5, 0, 0.5);
				Duplicate(Line);
				Line = **j;
				Line.Move(0, 0.5, 0.5);
				Duplicate(Line);
			case base_centered_orthorhombic:
			case base_centered_monoclinic:
				Line = **j;
				Line.Move(0.5, 0.5, 0);
				Duplicate(Line);
				break;
			default:
				break;
			}
			break ;
		case unique:
			if (((*j)->Xmin() >= m_xmin) && ((*j)->Xmax() <= m_xmax)
				&& ((*j)->Ymin() >= m_ymin) && ((*j)->Ymax() <= m_ymax)
				&& ((*j)->Zmin() >= m_zmin) && ((*j)->Zmax() <= m_zmax))
					Lines.push_back(new CrystalLine(**j)) ;
		}
	}

	//Manage cleavages
	CrystalCleavageList::iterator k;
	for (k = Cleavages.begin(); k != Cleavages.end(); k++)
	{
		double x;
		std::vector<double> ScalarProducts;
		std::vector<double>::iterator m;
		unsigned n;

		//scalar products calculus and storing
		for (i = Atoms.begin(); i != Atoms.end(); i++)
		{
			x = (*i)->ScalProd((*k)->h(), (*k)->k(), (*k)->l());
			for (m = ScalarProducts.begin(); m != (ScalarProducts.end()) && ((*m) > (x + PREC)); m++) ;
			if ((m == ScalarProducts.end()) || (fabs(*m - x) > PREC)) ScalarProducts.insert(m, x);
		}

		//cleave atoms and bonds
		if ((n = (*k)->Planes()) >= ScalarProducts.size())
		{
			GtkWidget* message = gtk_message_dialog_new(NULL, (GtkDialogFlags) 0, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, _("Everything has been cleaved"));
			g_signal_connect (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), NULL);
			gtk_widget_show(message);
			for (i = Atoms.begin(); i != Atoms.end(); i++) (*i)->Cleave();
			for (j = Lines.begin(); j != Lines.end(); j++) (*j)->Cleave();
		}
		else
		{
			x = ScalarProducts[n - 1];
			for (i = Atoms.begin(); i != Atoms.end(); i++)
				if (x < (*i)->ScalProd((*k)->h(), (*k)->k(), (*k)->l())+ PREC)
					(*i)->Cleave() ;

			//bonds cleavage
			for (j = Lines.begin(); j != Lines.end(); j++)
				if (x < (*j)->ScalProd((*k)->h(), (*k)->k(), (*k)->l())+ PREC)
					(*j)->Cleave();
		}

		ScalarProducts.clear() ;
	}
	
	//Transform coordinates to Cartesians and find center of visible view
	gdouble x, y, z, d,
		xmin = G_MAXDOUBLE, ymin = G_MAXDOUBLE, zmin = G_MAXDOUBLE,
		xmax = -G_MAXDOUBLE, ymax = -G_MAXDOUBLE, zmax = -G_MAXDOUBLE;
	iend = Atoms.end ();
	for (i = Atoms.begin(); i != iend; i++) {
		(*i)->NetToCartesian(m_a, m_b, m_c, alpha, beta, gamma);
		if ((*i)->IsCleaved ())
			continue;
		(*i)->GetCoords (&x, &y, &z);
		if (x < xmin)
			xmin = x;
		if (y < ymin)
			ymin = y;
		if (z < zmin)
			zmin = z;
		if (x > xmax)
			xmax = x;
		if (y > ymax)
			ymax = y;
		if (z > zmax)
			zmax = z;
	}
	jend = Lines.end ();
	for (j = Lines.begin (); j != jend; j++) {
		(*j)->NetToCartesian (m_a, m_b, m_c, alpha, beta, gamma);
		if ((*j)->IsCleaved ())
			continue;
		x = (*j)->Xmin ();
		y = (*j)->Ymin ();
		z = (*j)->Zmin ();
		if (x < xmin)
			xmin = x;
		if (y < ymin)
			ymin = y;
		if (z < zmin)
			zmin = z;
		x = (*j)->Xmax ();
		y = (*j)->Ymax ();
		z = (*j)->Zmax ();
		if (x > xmax)
			xmax = x;
		if (y > ymax)
			ymax = y;
		if (z > zmax)
			zmax = z;
	}

	//Searching the center of the crystal and find maximum distance from center
	x = (xmin + xmax) / 2.;
	y = (ymin + ymax) / 2.;
	z = (zmin + zmax) / 2.;
	m_MaxDist = 0;
	for (i = Atoms.begin(); i != iend; i++) {
		d =  (*i)->Distance (x, y, z, m_bFixedSize);
		m_MaxDist = __max (m_MaxDist, d);
		(*i)->Move (- x, - y, - z);
	}

	for (j = Lines.begin(); j != Lines.end(); j++) {
		d =  (*j)->Distance (x, y, z, m_bFixedSize);
		m_MaxDist = __max (m_MaxDist, d);
		(*j)->Move (- x, - y, - z);
	}
	m_SpaceGroup = FindSpaceGroup ();
}

void CrystalDoc::Duplicate (CrystalAtom& Atom)
{
	CrystalAtom AtomX, AtomY, AtomZ ;
	AtomX = Atom ;
	AtomX.Move (- floor (AtomX.x ()-m_xmin + 1e-7), - floor (AtomX.y ()-m_ymin + 1e-7), - floor (AtomX.z ()-m_zmin + 1e-7)) ;
	while (AtomX.x () <= m_xmax + 1e-7) {
		AtomY = AtomX ;
		while (AtomY.y () <= m_ymax + 1e-7) {
			AtomZ = AtomY ;
			while (AtomZ.z () <= m_zmax + 1e-7) {
				Atoms.push_back (new CrystalAtom (AtomZ)) ;
				AtomZ.Move (0,0,1) ;
			}
			AtomY.Move (0,1,0) ;
		}
		AtomX.Move (1,0,0) ;
	}
}

void CrystalDoc::Duplicate (CrystalLine& Line)
{
	CrystalLine LineX, LineY, LineZ ;
	LineX = Line ;
	LineX.Move (- floor (LineX.Xmin ()-m_xmin + 1e-7), - floor (LineX.Ymin ()-m_ymin + 1e-7), - floor (LineX.Zmin ()-m_zmin + 1e-7)) ;
	while (LineX.Xmax () <= m_xmax + 1e-7) {
		LineY = LineX ;
		while (LineY.Ymax () <= m_ymax + 1e-7) {
			LineZ = LineY ;
			while (LineZ.Zmax () <= m_zmax + 1e-7) {
				Lines.push_back (new CrystalLine (LineZ)) ;
				LineZ.Move (0,0,1) ;
			}
			LineY.Move (0,1,0) ;
		}
		LineX.Move (1,0,0) ;
	}
}
	
void CrystalDoc::Draw (Matrix const &m) const
{
	Vector v, v1;
	Sphere sp (10);
	glEnable (GL_RESCALE_NORMAL);
	CrystalAtomList::const_iterator i, iend = Atoms.end ();
	double red, green, blue, alpha;
	for (i = Atoms.begin (); i != iend; i++)
		if (!(*i)->IsCleaved ()) {
			v.SetZ ((*i)->x ());
			v.SetX ((*i)->y ());
			v.SetY ((*i)->z ());
			v = m.glmult (v);
			(*i)->GetColor (&red, &green, &blue, &alpha);
			glColor4d (red, green, blue, alpha) ;
			sp.draw (v, (*i)->r () * (*i)->GetEffectiveRadiusRatio ());
		}
	glEnable (GL_NORMALIZE);
	CrystalLineList::const_iterator j, jend = Lines.end ();
	Cylinder cyl (10);
	for (j = Lines.begin (); j != jend; j++)
		if (!(*j)->IsCleaved ()) {
			v.SetZ ((*j)->X1 ());
			v.SetX ((*j)->Y1 ());
			v.SetY ((*j)->Z1 ());
			v = m.glmult (v);
			v1.SetZ ((*j)->X2 ());
			v1.SetX ((*j)->Y2 ());
			v1.SetY ((*j)->Z2 ());
			v1 = m.glmult (v1);
			(*j)->GetColor (&red, &green, &blue, &alpha);
			glColor4d (red, green, blue, alpha) ;
			cyl.draw (v, v1, (*j)->GetRadius ());
		}
}

CrystalView* CrystalDoc::CreateNewView()
{
	return new CrystalView(this);
}

CrystalAtom* CrystalDoc::CreateNewAtom()
{
	return new CrystalAtom();
}

CrystalLine* CrystalDoc::CreateNewLine()
{
	return new CrystalLine();
}

CrystalCleavage* CrystalDoc::CreateNewCleavage()
{
	return new CrystalCleavage();
}

const char* CrystalDoc::GetProgramId() const
{
	return NULL;
}

xmlDocPtr CrystalDoc::BuildXMLTree () const
{
	gchar buf[256];
	xmlDocPtr xml;
	xmlNodePtr node;
	xmlNsPtr ns;
	char *old_num_locale;

	xml = xmlNewDoc((xmlChar*)"1.0");
	if (xml == NULL) {throw(1);}
	
	old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	xmlDocSetRootElement (xml,  xmlNewDocNode(xml, NULL, (xmlChar*)"crystal", NULL));
	ns = xmlNewNs (xml->children, (xmlChar*) "http://www.nongnu.org/gcrystal", (xmlChar*) "gcry");
	xmlSetNs (xml->children, ns);

	try
	{
		node = xmlNewDocNode(xml, NULL, (xmlChar*)"generator", (xmlChar*)GetProgramId());
		if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		
		node = xmlNewDocNode(xml, NULL, (xmlChar*)"lattice", (xmlChar*)LatticeName[m_lattice]);
		if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		if (m_SpaceGroup) {
			node = xmlNewDocNode (xml, NULL, (xmlChar*) "group", NULL);
			if (node)
				xmlAddChild (xml->children, node);
			else
				throw (int) 0;
			string name = m_SpaceGroup->GetHallName ();
			if (name.length () != 0)
				xmlNewProp (node, (xmlChar*) "Hall", (xmlChar*) name.c_str ());
			else {
				name = m_SpaceGroup->GetHMName ();
				if (name.length () != 0)
					xmlNewProp (node, (xmlChar*) "HM", (xmlChar*) name.c_str ());
			}
			xmlNodePtr child;
			list <Transform3d*>::const_iterator i;
			Transform3d const *t = m_SpaceGroup->GetFirstTransform (i);
			while (t) {
				child = xmlNewDocNode (xml, NULL, (xmlChar*) "transform", (xmlChar const*) t->DescribeAsString ().c_str ());
				if (child)
					xmlAddChild (node, child);
				else
					throw (int) 0;
				t = m_SpaceGroup->GetNextTransform (i);
			}
		}
		node = xmlNewDocNode(xml, NULL, (xmlChar*)"cell", NULL);
		if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		snprintf(buf, sizeof(buf), "%g", m_a);
		xmlNewProp(node, (xmlChar*)"a", (xmlChar*)buf);
		snprintf(buf, sizeof(buf), "%g", m_b);
		xmlNewProp(node, (xmlChar*)"b", (xmlChar*)buf);
		snprintf(buf, sizeof(buf), "%g", m_c);
		xmlNewProp(node, (xmlChar*)"c", (xmlChar*)buf);
		snprintf(buf, sizeof(buf), "%g", m_alpha);
		xmlNewProp(node, (xmlChar*)"alpha", (xmlChar*)buf);
		snprintf(buf, sizeof(buf), "%g", m_beta);
		xmlNewProp(node, (xmlChar*)"beta", (xmlChar*)buf);
		snprintf(buf, sizeof(buf), "%g", m_gamma);
		xmlNewProp(node, (xmlChar*)"gamma", (xmlChar*)buf);
	
		node = xmlNewDocNode(xml, NULL, (xmlChar*)"size", NULL);
		if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		WritePosition(xml, node, "start", m_xmin, m_ymin, m_zmin);
		WritePosition(xml, node, "end", m_xmax, m_ymax, m_zmax);
		if (m_bFixedSize)
			xmlNewProp (node, (xmlChar *) "fixed", (xmlChar *) "true");
		
		CrystalAtomList::const_iterator i;
		for (i = AtomDef.begin(); i != AtomDef.end(); i++)
		{
			node = (*i)->Save(xml);
			if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		}
	
		CrystalLineList::const_iterator j;
		for (j = LineDef.begin(); j != LineDef.end(); j++)
		{
			node = (*j)->Save(xml);
			if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		}
	
		CrystalCleavageList::const_iterator k;
		for (k = Cleavages.begin(); k != Cleavages.end(); k++)
		{
			node = (*k)->Save(xml);
			if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		}
		
		list<CrystalView*>::const_iterator view;
		for (view = m_Views.begin(); view != m_Views.end(); view++)
		{
			node = (*view)->Save(xml);
			if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		}
	
		setlocale(LC_NUMERIC, old_num_locale);
		g_free(old_num_locale);

		return xml;
	}
	catch (int num)
	{
		xmlFreeDoc(xml);
		setlocale(LC_NUMERIC, old_num_locale);
		g_free(old_num_locale);
		return NULL;
	}
}

bool CrystalDoc::Loaded () throw (LoaderError)
{
	if (m_NameCommon.length () > 0)
		SetTitle (m_NameCommon);
	else if (m_NameMineral.length () > 0)
		SetTitle (m_NameMineral);
	else if (m_NameSystematic.length () > 0)
		SetTitle (m_NameSystematic);
	else if (m_NameStructure.length () > 0)
		SetTitle (m_NameStructure);
	LineDef.push_back (new CrystalLine (edges, 0., 0., 0., 0., 0., 0., 10., .25 , .25, .25 , 1.));
	// set radii
	CrystalAtomList::iterator i, iend = AtomDef.end ();
	GcuAtomicRadius radius;
	radius.type = GCU_VAN_DER_WAALS;
	radius.charge = 0;
    radius.cn = -1;
    radius.spin = GCU_N_A_SPIN;
    radius.scale = NULL;
	/* use van der Waals radii for now in all cases, using ionic radii would need the
	evaluation of the coordination numbers */
	for (i = AtomDef.begin (); i != iend; i++) {
		radius.Z = (*i)->GetZ ();
		if (gcu_element_get_radius (&radius)) {
			(*i)->SetRadius (radius);
			(*i)->SetEffectiveRadiusRatio (.4);
		}
		(*i)->SetDefaultColor ();
	}
	switch (m_lattice) { // ensure that the paraleters are coherent
	case cubic:
	case body_centered_cubic:
	case face_centered_cubic:
		m_alpha = m_beta = m_gamma  = 90;
		m_b = m_c = m_a;
		break;
	case hexagonal:
		m_alpha = m_beta = 90;
		m_gamma  = 120;
		m_b = m_a;
		break;
	case tetragonal:
	case body_centered_tetragonal:
		m_alpha = m_beta = m_gamma  = 90;
		m_b = m_a;
		break;
	case orthorhombic:
	case base_centered_orthorhombic:
	case body_centered_orthorhombic:
	case face_centered_orthorhombic:
		m_alpha = m_beta = m_gamma  = 90;
		break;
	case rhombohedral:
		m_beta = m_gamma = m_alpha;
		m_b = m_c = m_a;
		break;
	case monoclinic:
	case base_centered_monoclinic:
		m_alpha = m_gamma  = 90;
		break;
	case triclinic:
		break;
	}

	Update ();
	return false;	// no pending reference updated
}

bool CrystalDoc::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_CELL_A:
		m_a = strtod (value, NULL) * GetScale ();
		break;
	case GCU_PROP_CELL_B:
		m_b = strtod (value, NULL) * GetScale ();
		break;
	case GCU_PROP_CELL_C:
		m_c = strtod (value, NULL) * GetScale ();
		break;
	case GCU_PROP_CELL_ALPHA:
		m_alpha = strtod (value, NULL);
		break;
	case GCU_PROP_CELL_BETA:
		m_beta = strtod (value, NULL);
		break;
	case GCU_PROP_CELL_GAMMA:
		m_gamma = strtod (value, NULL);
		break;
	case GCU_PROP_CHEMICAL_NAME_COMMON:
		m_NameCommon = value;
		break;
	case GCU_PROP_CHEMICAL_NAME_SYSTEMATIC:
		m_NameSystematic = value;
		break;
	case GCU_PROP_CHEMICAL_NAME_MINERAL:
		m_NameMineral = value;
		break;
	case GCU_PROP_CHEMICAL_NAME_STRUCTURE:
		m_NameStructure = value;
		break;
	case GCU_PROP_SPACE_GROUP: {
		m_SpaceGroup = SpaceGroup::GetSpaceGroup (value);
		char type = (*value == '-')? value[1]: value[0];
		int id = m_SpaceGroup->GetId ();
		if (id <= 2)
			m_lattice = triclinic;
		else if (id <= 15)
			m_lattice = (type == 'P')? monoclinic: base_centered_monoclinic;
		else if (id <= 74)
			switch (type) {
			case 'P':
				m_lattice = orthorhombic;
				break;
			case 'I':
				m_lattice = body_centered_orthorhombic;
				break;
			case 'F':
				m_lattice = face_centered_orthorhombic;
				break;
			default:
				m_lattice = base_centered_orthorhombic;
				break;
			}
		else if (id <= 142)
			m_lattice = (type == 'P')? tetragonal: body_centered_tetragonal;
		else if (id <= 167)
			m_lattice = rhombohedral;
		else if (id <= 194)
			m_lattice = hexagonal;
		else {
			switch (type) {
			case 'P':
				m_lattice = cubic;
				break;
			case 'I':
				m_lattice = body_centered_cubic;
				break;
			case 'F':
				m_lattice = face_centered_cubic;
				break;
			}
		}
		break;
	}
	default:
		return false;
	}
	return true;
}

std::string CrystalDoc::GetProperty (unsigned property) const
{
	ostringstream res;
	switch (property) {
	case GCU_PROP_DOC_TITLE:
		return m_Title;
	case GCU_PROP_CHEMICAL_NAME_COMMON:
		return m_NameCommon;
	case GCU_PROP_CHEMICAL_NAME_SYSTEMATIC:
		return m_NameSystematic;
	case GCU_PROP_CHEMICAL_NAME_MINERAL:
		return m_NameMineral;
	case GCU_PROP_CELL_A:
		res << m_a / GetScale ();
		break;
	case GCU_PROP_CELL_B:
		res << m_b / GetScale ();
		break;
	case GCU_PROP_CELL_C:
		res << m_c / GetScale ();
		break;
	case GCU_PROP_CELL_ALPHA:
		res << m_alpha;
		break;
	case GCU_PROP_CELL_BETA:
		res << m_beta;
		break;
	case GCU_PROP_CELL_GAMMA:
		res << m_gamma;
		break;
	case GCU_PROP_SPACE_GROUP:
		return m_SpaceGroup->GetHallName ();
	default:
		return GLDocument::GetProperty (property);
	}
	return res.str ();
}

void CrystalDoc::AddChild (Object* object)
{
	Object::AddChild (object);
	CrystalAtom *atom = dynamic_cast <CrystalAtom *> (object);
	if (atom) {
		AtomDef.remove (atom); // don't add more than needed (a set might be better than a list)
		AtomDef.push_back (atom);
	}
}

SpaceGroup const *CrystalDoc::FindSpaceGroup ()
{
	if (!AtomDef.size ())
		return NULL;
	if (m_SpaceGroup && !m_AutoSpaceGroup)
		return (m_SpaceGroup);
	unsigned start, end, id;
	switch (m_lattice) {
	case cubic:
	case body_centered_cubic:
	case face_centered_cubic:
		start = 195;
		end = 230;
		break;
	case hexagonal:
		start = 168;
		end = 194;
		break;
	case tetragonal:
	case body_centered_tetragonal:
		start = 75;
		end = 142;
		break;
	case orthorhombic:
	case base_centered_orthorhombic:
	case body_centered_orthorhombic:
	case face_centered_orthorhombic:
		start = 16;
		end = 74;
		break;
	case rhombohedral:
		start = 143;
		end = 167;
		break;
	case monoclinic:
	case base_centered_monoclinic:
		start = 3;
		end = 15;
		break;
	case triclinic:
		start = 1;
		end = 2;
		break;
	default:
		start = end = 0;
	}
	//make a list of all atoms inside the cell
	list <CrystalAtom *> atoms;
	CrystalAtom *a;
	double x, y, z;
	CrystalAtomList::iterator i, i0, iend = AtomDef.end ();
	for (i = AtomDef.begin (); i != iend; i++) {
		atoms.push_back (new CrystalAtom (**i));
		switch (m_lattice) {
		case body_centered_cubic:
		case body_centered_tetragonal:
		case body_centered_orthorhombic:
			a = new CrystalAtom (**i);
			a->GetCoords (&x, &y, &z);
			x = (x > .5 - PREC)? x - .5: x + .5;
			y = (y > .5 - PREC)? y - .5: y + .5;
			z = (z > .5 - PREC)? z - .5: z + .5;
			a->SetCoords (x, y, z);
			atoms.push_back (a);
			break;
		case face_centered_cubic:
		case face_centered_orthorhombic:
			a = new CrystalAtom (**i);
			a->GetCoords (&x, &y, &z);
			x = (x > .5 - PREC)? x - .5: x + .5;
			z = (z > .5 - PREC)? z - .5: z + .5;
			a->SetCoords (x, y, z);
			atoms.push_back (a);
			a = new CrystalAtom (**i);
			a->GetCoords (&x, &y, &z);
			y = (y > .5 - PREC)? y - .5: y + .5;
			z = (z > .5 - PREC)? z - .5: z + .5;
			a->SetCoords (x, y, z);
			atoms.push_back (a);
		case base_centered_orthorhombic:
		case base_centered_monoclinic:
			a = new CrystalAtom (**i);
			a->GetCoords (&x, &y, &z);
			x = (x > .5 - PREC)? x - .5: x + .5;
			y = (y > .5 - PREC)? y - .5: y + .5;
			a->SetCoords (x, y, z);
			atoms.push_back (a);
			break;
		default:
			break;
		}
	}
	iend = atoms.end ();
	SpaceGroup const *res = NULL;
	Vector v;
	std::list <Vector>::iterator j, jend;
	for (id = end; id >= start; id--) {
		std::list <SpaceGroup const *> &groups = SpaceGroup::GetSpaceGroups (id);
		std::list <SpaceGroup const *>::iterator g, gend = groups.end ();
		for (g = groups.begin (); g != gend; g++) {
			for (i = atoms.begin (); i != iend; i++) {
				a = new CrystalAtom (**i);
				v = a->GetVector ();
				std::list <Vector> vv = (*g)->Transform (v);
				jend = vv.end ();
				for (j = vv.begin (); j != jend; j++) {
					x = (*j).GetX ();
					y = (*j).GetY ();
					z = (*j).GetZ ();
					while (x > 1. - PREC)
						x -= 1.;
					while (y > 1. - PREC)
						y -= 1.;
					while (z > 1. - PREC)
						z -= 1.;
					a->SetCoords (x, y, z);
					for (i0 = atoms.begin (); i0 != iend; i0++)
						if (*a == **i0)
							break;
					if (i0 == iend)
						goto end_loop;
				}
				delete a;
				if (j != jend)
					goto end_loop;
			}
			if (i == iend)
				break;
end_loop:;
		}
		if (g != gend) {
			res = *g;
			break;
		}
	}
	// clean atoms
	for (i = atoms.begin (); i != iend; i++)
		delete *i;
	// now, search for duplicates in AtomDef
	if (!res)
		return NULL;
	set <CrystalAtom *> dups;
	iend = AtomDef.end ();
	for (i = AtomDef.begin (); i != iend; i++) {
		if (dups.find (*i) != dups.end ())
			continue;
		a = new CrystalAtom (**i);
		v = a->GetVector ();
		std::list <Vector> vv = res->Transform (v);
		for (j = vv.begin (); j != jend; j++) {
			x = (*j).GetX ();
			y = (*j).GetY ();
			z = (*j).GetZ ();
			while (x > 1. - PREC)
				x -= 1.;
			while (y > 1. - PREC)
				y -= 1.;
			while (z > 1. - PREC)
				z -= 1.;
			a->SetCoords (x, y, z);
			for (i0 = i, i0++; i0 != iend; i0++) {
				if (dups.find (*i0) != dups.end ())
					continue;
				if (*a == **i0) {
					if ((*i0)->x () < (*i)->x () || (*i0)->y () < (*i)->y () || (*i0)->z () < (*i)->z ()) {
						dups.insert (*i);
						goto end_loop1;
					}
					dups.insert (*i0);
				}
			}
		}
end_loop1:;
	}
	set <CrystalAtom *>::iterator k, kend = dups.end ();
	for (k = dups.begin (); k != kend; k++) {
		AtomDef.remove (*k);
		delete *k;
	}
	return res;
}

}	//	namespace gcu
