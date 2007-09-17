// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * crystaldoc.cc 
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
#include "crystaldoc.h"
#include "crystalview.h"
#include "cylinder.h"
#include "matrix.h"
#include "sphere.h"
#include "xml-utils.h"
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <libintl.h>
#include <clocale>
#include <cmath>
#include <cstring>
#include <vector>
#ifdef HAVE_OPENBABEL_2_2
#	include <list>
#else
#	include <openbabel/math/vector3.h>
#endif
#include <GL/gl.h>

#define __max(x,y)  ((x) > (y)) ? (x) : (y)
#define __min(x,y)  ((x) < (y)) ? (x) : (y)
#define PREC 1e-3

using namespace OpenBabel;

namespace gcu {
	
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

}

using namespace gcu;

#ifdef HAVE_OPENBABEL_2_2
CrystalDoc::CrystalDoc (Application *App): GLDocument (App),
	m_SpaceGroup (NULL)
#else
CrystalDoc::CrystalDoc (Application *App): GLDocument (App)
#endif
{
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
#ifdef HAVE_OPENBABEL_2_2
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
			m_SpaceGroup = group;
#endif
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
				AtomDef.push_back (pAtom);
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

bool CrystalDoc::LoadNewView(xmlNodePtr node)
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

	////////////////////////////////////////////////////////////
	//Establish list of atoms
	CrystalAtomList::iterator i, iend = AtomDef.end ();
	
#ifdef HAVE_OPENBABEL_2_2
	if (m_SpaceGroup) {
		vector3 v;
		list<vector3> d;
		for (i = AtomDef.begin(); i != iend; i++) {
			v.x () = (*i)->x ();
			v.y () = (*i)->y ();
			v.z () = (*i)->z ();
			d = m_SpaceGroup->Transform (v);
			list<vector3>::iterator vi, viend = d.end();
			CrystalAtom atom (**i);
			for (vi=d.begin (); vi!= viend; vi++) {
				atom.SetCoords ((*vi).x(), (*vi).y(), (*vi).z());
				Duplicate (atom);
			}
		}
	} else
#endif
	for (i = AtomDef.begin(); i != iend; i++) {
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
	
void CrystalDoc::Draw (Matrix &m)
{
	vector3 v, v1;
	Sphere sp (10);
	glEnable (GL_RESCALE_NORMAL);
	CrystalAtomList::iterator i, iend = Atoms.end ();
	double red, green, blue, alpha;
	for (i = Atoms.begin (); i != iend; i++) {
		(*i)->GetCoords (&v.x (), &v.y (), &v.z ());
		v = m * v;
		(*i)->GetColor (&red, &green, &blue, &alpha);
		glColor4d (red, green, blue, alpha) ;
		sp.draw (v, (*i)->r ());
	}
	glEnable (GL_NORMALIZE);
	CrystalLineList::iterator j, jend = Lines.end ();
	Cylinder cyl (10);
	for (j = Lines.begin (); j != jend; j++) {
		v.x () = (*j)->X1 ();
		v.y () = (*j)->Y1 ();
		v.z () = (*j)->Z1 ();
		v = m * v;
		v1.x () = (*j)->X2 ();
		v1.y () = (*j)->Y2 ();
		v1.z () = (*j)->Z2 ();
		v1 = m * v1;
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

const char* CrystalDoc::GetProgramId()
{
	return NULL;
}

xmlDocPtr CrystalDoc::BuildXMLTree()
{
	gchar buf[256];
	xmlDocPtr xml;
	xmlNodePtr node;
	char *old_num_locale;

	xml = xmlNewDoc((xmlChar*)"1.0");
	if (xml == NULL) {throw(1);}
	
	old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	xmlDocSetRootElement (xml,  xmlNewDocNode(xml, NULL, (xmlChar*)"crystal", NULL));
	
	try
	{
		node = xmlNewDocNode(xml, NULL, (xmlChar*)"generator", (xmlChar*)GetProgramId());
		if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		
		node = xmlNewDocNode(xml, NULL, (xmlChar*)"lattice", (xmlChar*)LatticeName[m_lattice]);
		if (node) xmlAddChild(xml->children, node); else throw (int) 0;
#ifdef HAVE_OPENBABEL_2_2
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
			transform3dIterator i;
			transform3d const *t = m_SpaceGroup->BeginTransform (i);
			while (t) {
				child = xmlNewDocNode (xml, NULL, (xmlChar*) "transform", (xmlChar const*) t->DescribeAsString ().c_str ());
				if (child)
					xmlAddChild (node, child);
				else
					throw (int) 0;
				t = m_SpaceGroup->NextTransform (i);
			}
		}
#endif
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
		
		CrystalAtomList::iterator i;
		for (i = AtomDef.begin(); i != AtomDef.end(); i++)
		{
			node = (*i)->Save(xml);
			if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		}
	
		CrystalLineList::iterator j;
		for (j = LineDef.begin(); j != LineDef.end(); j++)
		{
			node = (*j)->Save(xml);
			if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		}
	
		CrystalCleavageList::iterator k;
		for (k = Cleavages.begin(); k != Cleavages.end(); k++)
		{
			node = (*k)->Save(xml);
			if (node) xmlAddChild(xml->children, node); else throw (int) 0;
		}
		
		list<CrystalView*>::iterator view;
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

#ifdef HAVE_OPENBABEL_2_2
bool CrystalDoc::ImportOB (OBMol &mol)
{
	OBUnitCell *cell = dynamic_cast<OBUnitCell*> (mol.GetData (OBGenericDataType::UnitCell));
	if (cell == NULL)
		return false;
	m_a = cell->GetA () * 100;
	m_b = cell->GetB () * 100;
	m_c = cell->GetC () * 100;
	m_alpha = cell ->GetAlpha ();
	m_beta = cell->GetBeta ();
	m_gamma = cell->GetGamma ();
	string const group_name = cell->GetSpaceGroupName ();
	m_SpaceGroup = cell->GetSpaceGroup ();
    if (!m_SpaceGroup)
		return false;
	int lattice = cell->GetLatticeType ();
	switch (lattice) {
	case OBUnitCell::Triclinic:
		m_lattice = triclinic;
		break;
	case OBUnitCell::Monoclinic:
		switch (group_name[0]) {
		case 'C':
			m_lattice = base_centered_monoclinic;
			break;
		default:
			m_lattice = monoclinic;
			break;
		}
		break;
	case OBUnitCell::Orthorhombic:
		switch (group_name[0]) {
		case 'C':
			m_lattice = base_centered_orthorhombic;
			break;
		case 'I':
			m_lattice = body_centered_orthorhombic;
			break;
		case 'F':
			m_lattice = face_centered_orthorhombic;
			break;
		default:
			m_lattice = orthorhombic;
			break;
		}
		break;
	case OBUnitCell::Tetragonal:
		switch (group_name[0]) {
		case 'I':
			m_lattice = body_centered_tetragonal;
			break;
		default:
			m_lattice = tetragonal;
			break;
		}
		break;
	case OBUnitCell::Rhombohedral:
		m_lattice = rhombohedral;
		break;
	case OBUnitCell::Hexagonal:
		m_lattice = hexagonal;
		break;
	case OBUnitCell::Cubic:
		switch (group_name[0]) {
		case 'I':
			m_lattice = body_centered_cubic;
			break;
		case 'F':
			m_lattice = face_centered_cubic;
			break;
		default:
			m_lattice = cubic;
			break;
		}
		break;
	}
	matrix3x3 m = cell->GetFractionalMatrix ();
	vector3 v;
	// now get the atoms
	OBAtomIterator it;
	OBAtom *atom = mol.BeginAtom (it);
	CrystalAtom *catom;
	GcuAtomicRadius radius;
	radius.type = GCU_VAN_DER_WAALS;
	radius.charge = 0;
    radius.cn = -1;
    radius.spin = GCU_N_A_SPIN;
    radius.scale = NULL;
	while (atom) {
		v.x () = atom->GetX();
		v.y () = atom->GetY();
		v.z () = atom->GetZ();
		v *= m;
		radius.Z = atom->GetAtomicNum ();
		catom = new CrystalAtom (radius.Z, v.x (), v.y (), v.z ());
		if (gcu_element_get_radius (&radius)) {
			catom->SetRadius (radius);
			catom->SetEffectiveRadiusRatio (.25);
		}
		AtomDef.push_back (catom);
		atom = mol.NextAtom (it);
	}

	Update ();
	return true;
}
#endif
