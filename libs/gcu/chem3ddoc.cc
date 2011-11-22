// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/chem3ddoc.cc
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "chem3ddoc.h"
#include "atom.h"
#include "application.h"
#include "bond.h"
#include "cylinder.h"
#include "glview.h"
#include "loader.h"
#include "sphere.h"
#include "vector.h"
#include <gcu/chemistry.h>
#include <gcu/element.h>
#include <gsf/gsf-input-memory.h>
#include <goffice/goffice.h>
#include <gio/gio.h>
#include <GL/gl.h>
#include <libintl.h>
#include <clocale>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <cstring>

using namespace std;

namespace gcu
{

static char const *Display3DModeNames[] = {
	"ball&stick",
	"spacefill",
	"cylinders",
	"wireframe"
};

Display3DMode Chem3dDoc::Display3DModeFromString (char const *name)
{
	if (name == NULL)
		return  gcu::BALL_AND_STICK;
	// first ensure the string is in lower case
	char lcname[16];
	int i, max = strlen (name), res = gcu::WIREFRAME;
	if (max > 15)
		return  gcu::BALL_AND_STICK;
	for (i = 0; i < max; i++)
		lcname[i] = tolower (name[i]);
	lcname[i] = 0;
	while (res >=  gcu::BALL_AND_STICK && strcmp (lcname, Display3DModeNames[res]))
		res--;
	return static_cast < gcu::Display3DMode > (res);
}

char const *Chem3dDoc::Display3DModeAsString (Display3DMode mode)
{
	return Display3DModeNames[mode];
}

Chem3dDoc::Chem3dDoc (): GLDocument (Application::GetDefaultApplication ())
{
	m_View = NULL;
	m_Display3D = BALL_AND_STICK;
	m_Mol = NULL;
}

Chem3dDoc::Chem3dDoc (Application *App, GLView *View): GLDocument (App)
{
	m_View = View;
	m_Display3D = BALL_AND_STICK;
	m_Mol = NULL;
}

Chem3dDoc::~Chem3dDoc ()
{
	delete m_View;
}

static Object* CreateAtom ()
{
	return new Atom ();
}

static Object* CreateBond ()
{
	return new Bond ();
}

static Object* CreateMolecule ()
{
	return new Molecule ();
}

void Chem3dDoc::Load (char const *uri, char const *mime_type)
{
	GVfs *vfs = g_vfs_get_default ();
	GFile *file = g_vfs_get_file_for_uri (vfs, uri);
	GError *error = NULL;
	GFileInfo *info = g_file_query_info (file,
										 ((mime_type)? G_FILE_ATTRIBUTE_STANDARD_SIZE:
										 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","G_FILE_ATTRIBUTE_STANDARD_SIZE),
										 G_FILE_QUERY_INFO_NONE,
										 NULL, &error);
	if (error) {
		g_message ("GIO querry failed: %s", error->message);
		g_error_free (error);
		g_object_unref (file);
		error = NULL;
		return;
	}
	if (!mime_type)
		mime_type = g_file_info_get_content_type (info);
	g_object_unref (info);
	if (!mime_type) {
		// should not happen
		g_object_unref (file);
		return;
	}
	// try using the loader mechanism
	Application *app = GetApp ();
	Object *obj = app->CreateObject ("atom", this);
	if (obj)
		delete obj;
	else {
		gcu::Loader::Init (app); // can be called many times
		app->AddType ("atom", CreateAtom, AtomType);
		app->AddType ("bond", CreateBond, BondType);
		app->AddType ("molecule", CreateMolecule, MoleculeType);
	}
	string filename = uri;
	Clear ();
	ContentType type = app->Load (filename, mime_type, this);
	Loaded ();
	if (type == ContentType3D) {
		// center the scene around 0,0,0
		std::map<std::string, Object*>::iterator it;
		obj =  GetFirstChild (it);
		while (obj) {
			m_Mol = dynamic_cast <Molecule *> (obj);
			if (m_Mol)
				break;
			obj = GetNextChild (it);
		}
		// FIXME: we show only one molecule
		double x0, y0, z0;
		x0 = y0 = z0 = 0.0;
		std::list <Atom *>::const_iterator i;
		Atom const *atom = m_Mol->GetFirstAtom (i);
		while (atom) {
			x0 += atom->x ();
			y0 += atom->y ();
			z0 += atom->z ();
			atom = m_Mol->GetNextAtom (i);
		}
		m_Mol->Move (-x0 / m_Mol->GetAtomsNumber (), -y0 / m_Mol->GetAtomsNumber (), -z0 / m_Mol->GetAtomsNumber ());
		char const *title = m_Mol->GetName ();
		if (title)
			SetTitle (title);
		else {
			char *fn = g_file_get_basename (file);
			SetTitle (fn);
			g_free (fn);
		}
		ChangedDisplay3D ();
		m_View->Update ();
	} else if (type != ContentTypeUnknown) {
		Clear ();
		// TODO: process the error (display a message at least or open with an appropriate application)
	}
	g_object_unref (file);
}

ContentType Chem3dDoc::LoadData (char const *data, char const *mime_type, size_t size)
{
	bool need_free = false;
	if (!mime_type) {
		mime_type = go_get_mime_type_for_data (data, size);
		need_free = true;
	}
	if (!mime_type) {
		// should not happen
		return ContentTypeUnknown;
	}
	if (size == 0)
		size = strlen (data);
	GsfInput *input = gsf_input_memory_new (reinterpret_cast <guint8 const *> (data), size, false);
	// try using the loader mechanism
	Application *app = GetApp ();
	Object *obj = app->CreateObject ("atom", this);
	if (obj)
		delete obj;
	else {
		gcu::Loader::Init (app); // can be called many times
		app->AddType ("atom", CreateAtom, AtomType);
		app->AddType ("bond", CreateBond, BondType);
		app->AddType ("molecule", CreateMolecule, MoleculeType);
	}
	Clear ();
	ContentType type = app->Load (input, mime_type, this);
	Loaded ();
	if (type == ContentType3D) {
		// center the scene around 0,0,0
		std::map<std::string, Object*>::iterator it;
		obj =  GetFirstChild (it);
		while (obj) {
			m_Mol = dynamic_cast <Molecule *> (obj);
			if (m_Mol)
				break;
			obj = GetNextChild (it);
		}
		// FIXME: we show only one molecule
		double x0, y0, z0;
		x0 = y0 = z0 = 0.0;
		std::list <Atom *>::const_iterator i;
		Atom const *atom = m_Mol->GetFirstAtom (i);
		while (atom) {
			x0 += atom->x ();
			y0 += atom->y ();
			z0 += atom->z ();
			atom = m_Mol->GetNextAtom (i);
		}
		m_Mol->Move (-x0 / m_Mol->GetAtomsNumber (), -y0 / m_Mol->GetAtomsNumber (), -z0 / m_Mol->GetAtomsNumber ());
		char const *title = m_Mol->GetName ();
		if (title)
			SetTitle (title);
		ChangedDisplay3D ();
		m_View->Update ();
	} else if (type != ContentTypeUnknown) {
		Clear ();
		// TODO: process the error
	}
	if (need_free)
		g_free (const_cast <char *> (mime_type));
	g_object_unref (input);
	return type;
}

struct VrmlBond {
	double x, y, z;
	double xrot, zrot, arot;
};
typedef struct {int n; list <Atom const *> l;} sAtom;
typedef struct {int n; list <struct VrmlBond> l;} sBond;

void Chem3dDoc::OnExportVRML (string const &filename)
{
	if (!m_Mol)
		return;
	double R, w, x, y, z, x0, y0, z0, dist;
	int n = 0, Z;
	const gdouble* color;
	char const *symbol;
	ostringstream file;
	map<string, sAtom> AtomsMap;
	map<string, sBond> BondsMap;
	GError *error = NULL;
	GFile *stream = g_file_new_for_uri (filename.c_str ());
	GFileOutputStream *output = g_file_create (stream, G_FILE_CREATE_NONE, NULL, &error);
	if (error) {
		cerr << "gio error: " << error->message << endl;
		g_error_free (error);
		g_object_unref (stream);
		return;
	}

	file << "#VRML V2.0 utf8" << endl;

	x0 = y0 = z0 = 0.0;
	std::list <Atom *>::const_iterator i;
	Atom const *atom = m_Mol->GetFirstAtom (i);
	while (atom) {
		Z = atom->GetZ ();
		x0 += atom->x ();
		y0 += atom->y ();
		z0 += atom->z ();
		atom = m_Mol->GetNextAtom (i);
	}
	x0 /= m_Mol->GetAtomsNumber ();
	y0 /= m_Mol->GetAtomsNumber ();
	z0 /= m_Mol->GetAtomsNumber ();

	//Create prototypes for atoms
	GcuAtomicRadius radius;
	radius.type = GCU_VAN_DER_WAALS;
	radius.charge = 0;
	radius.cn = -1;
	radius.spin = GCU_N_A_SPIN;
	radius.scale = NULL;
	for (atom = m_Mol->GetFirstAtom (i); atom; atom = m_Mol->GetNextAtom (i)) {
		Z = atom->GetZ ();
		if (!Z)
			continue;
		symbol = Element::Symbol (Z);
		if (AtomsMap[symbol].l.empty()) {
			AtomsMap[symbol].n = n;
			radius.Z = Z;
			gcu_element_get_radius (&radius);
			R = radius.value.value / 100; // convert from pm to Angstrom (supposing that the unit is actually pm).
			if (m_Display3D == BALL_AND_STICK)
				R *= 0.2;
			color = gcu_element_get_default_color (Z);
			file << "PROTO Atom" << n++ << " [] {Shape {" << endl << "\tgeometry Sphere {radius " << R << "}" << endl;
			file << "\tappearance Appearance {" << endl << "\t\tmaterial Material {" << endl << "\t\t\tdiffuseColor " << color[0] << " " << color[1] << " " << color[2] << endl;
			file << "\t\t\tspecularColor 1 1 1" << endl << "\t\t\tshininess 0.9" << endl << "\t\t}" << endl << "\t}\r\n}}" << endl;
		}
		AtomsMap[symbol].l.push_back (atom);
	}

	//Create prototypes for bonds
	double conv = M_PI / 180;
	Matrix m (m_View->GetPsi () * conv, m_View->GetTheta () * conv, m_View->GetPhi () * conv, euler);
	if (m_Display3D == BALL_AND_STICK) {
		std::list <Bond *>::const_iterator b;
		Bond const *bond = m_Mol->GetFirstBond (b);
		double x1, y1, z1;
		struct VrmlBond vb;
		n = 0;
		while (bond) {
			atom = bond->GetAtom (0);
			if (atom->GetZ () == 0) {
				bond = m_Mol->GetNextBond (b);
				continue;
			}
			vb.x = atom->x () - x0;
			vb.y = atom->y () - y0;
			vb.z = atom->z () - z0;
			atom = bond->GetAtom (1);
			if (atom->GetZ () == 0) {
				bond = m_Mol->GetNextBond (b);
				continue;
			}
			x1 = atom->x () - x0 - vb.x;
			y1 = atom->y () - y0 - vb.y;
			z1 = atom->z () - z0 - vb.z;
			vb.x += x1 / 2;
			vb.y += y1 / 2;
			vb.z += z1 / 2;
			m.Transform(vb.x, vb.y, vb.z);
			m.Transform(x1, y1, z1);
			dist = sqrt (x1 * x1 + y1 * y1 + z1 * z1);
			w = sqrt (x1 * x1 + z1 * z1);
			if (w > 0) {
				vb.xrot = z1 / w;
				vb.zrot = -x1 / w;
				vb.arot = atan2 (w, y1);
			} else {
				vb.zrot = 0.;
				vb.xrot = 0.;
				vb.arot = 0.0;
			}
			char *buf = g_strdup_printf ("%g", dist);
			if (BondsMap[buf].l.empty()) {
				BondsMap[buf].n = n;
				file << "PROTO Bond" << n++ << " [] {Shape {" << endl << "\tgeometry Cylinder {radius " << 0.12 << "\theight " << dist << "}" << endl;
				file << "\tappearance Appearance {" << endl << "\t\tmaterial Material {" << endl << "\t\t\tdiffuseColor " << .75 << " " << .75 << " " << .75 << endl;
				file << "\t\t\tspecularColor 1 1 1" << endl << "\t\t\tshininess 0.9" << endl << "\t\t}" << endl << "\t}\r\n}}" << endl;
			}
			BondsMap[buf].l.push_back (vb);
			bond = m_Mol->GetNextBond (b);
		}
	}

	//world begin
	file << "Background{skyColor " << m_View->GetRed () << " " << m_View->GetBlue () << " " << m_View->GetGreen () << "}" << endl;
	file << "Viewpoint {fieldOfView " << m_View->GetAngle () / 90*1.570796326794897 << "\tposition 0 0 " << m_View->GetRadius () << "}" << endl;
	file << "Transform {" << endl << "\tchildren [" << endl;

	map<std::string, sAtom>::iterator k, kend = AtomsMap.end ();
	list<Atom const *>::const_iterator j, jend;
	for (k = AtomsMap.begin (); k != kend; k++) {
		jend = (*k).second.l.end ();
		for (j = (*k).second.l.begin (); j != jend; j++) {
			x = (*j)->x ();
			y = (*j)->y ();
			z = (*j)->z ();
			m.Transform(x, y, z);
			file << "\t\tTransform {translation " << x << " " << y << " " << z <<  " children [Atom" << (*k).second.n << " {}]}" << endl;
		}
		(*k).second.l.clear();
	}
	AtomsMap.clear();

	map<std::string, sBond>::iterator l, lend = BondsMap.end ();
	list<struct VrmlBond>::iterator mc, mend;
	for (l = BondsMap.begin (); l != lend; l++) {
		mend = (*l).second.l.end ();
		for (mc = (*l).second.l.begin (); mc != mend; mc++) {
			file << "\t\tTransform {" << endl << "\t\t\trotation " << (*mc).xrot << " " << 0. << " " << (*mc).zrot << " " << (*mc).arot << endl;
			file << "\t\t\ttranslation " << (*mc).x << " " << (*mc).y  << " " << (*mc).z <<  endl\
					<< "\t\t\tchildren [Bond" << (*l).second.n << " {}]}" << endl;
		}
	}

	//end of the world
	file << "\t]" << endl << "}" << endl;

	g_output_stream_write (reinterpret_cast <GOutputStream *> (output), file.str ().c_str (), file.str ().size (), NULL, &error);
	if (error) {
		cerr << "gio error: " << error->message << endl;
		g_error_free (error);
	}
	g_object_unref (stream);
}

void Chem3dDoc::Draw (Matrix const &m) const
{
	if (!m_Mol)
		return;
	std::list <Atom *>::const_iterator i;
	Atom const *atom = m_Mol->GetFirstAtom (i);
	unsigned int Z;
	gdouble R;
	map<Atom const *, Vector> atomPos;
	const gdouble* color;
	Vector v, normal (0., 0., 1.);
	Sphere sp (10);
	GcuAtomicRadius rad;
	rad.type = GCU_VAN_DER_WAALS;
	rad.charge = 0;
	rad.cn = -1;
	rad.spin = GCU_N_A_SPIN;
	rad.scale = NULL;
	if (m_Display3D == WIREFRAME) {
		float light_ambient[] = {1.0, 1.0, 1.0, 1.0};
		glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
	} else {
		glEnable (GL_RESCALE_NORMAL);
		float light_ambient[] = {.0, .0, .0, 1.0};
		glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
	}
	while (atom) {
		atomPos[atom] = v = m.glmult (atom->GetVector ());
		Z = atom->GetZ ();
		if (Z > 0) {
			if (m_Display3D == CYLINDERS) {
				R = 12.;
			} else if (m_Display3D == WIREFRAME) {
				R = 0.;
			} else {
				rad.Z = Z;
				Element::GetElement (Z)->GetRadius (&rad);
				R = rad.value.value;
				if (m_Display3D == BALL_AND_STICK)
					R *= 0.2;
			}
			color = gcu_element_get_default_color (Z);
			if (m_Display3D != WIREFRAME) {
				glColor3d (color[0], color[1], color[2]);
				sp.draw (v, R);
			}
		}
		atom = m_Mol->GetNextAtom (i);
	}
	if (m_Display3D != SPACEFILL) {
		Cylinder cyl (10);
		std::list <Bond *>::const_iterator j;
		Bond const *bond = m_Mol->GetFirstBond (j);
		Vector v, v0, v1;
		double R1;
		unsigned int Z1;
		if (m_Display3D == WIREFRAME)
			sp.draw (v, 0.); // weird, this initializes something needed to see colors but what?
		else
			glEnable (GL_NORMALIZE);
		while (bond) {
			atom = bond->GetAtom (0);
			v = m.glmult (atom->GetVector ());
			Z = atom->GetZ ();
			if (Z == 0) {
				bond = m_Mol->GetNextBond (j);
				continue;
			}
			atom = bond->GetAtom (1);
			rad.Z = Z;
			Element::GetElement (Z)->GetRadius (&rad);
			R = rad.value.value;
			Z1 = atom->GetZ ();
			if (Z1 == 0) {
				bond = m_Mol->GetNextBond (j);
				continue;
			}
			rad.Z = Z1;
			Element::GetElement (Z1)->GetRadius (&rad);
			R1 = rad.value.value;
			v1 = m.glmult (atom->GetVector ());
			v0 = v + (v1 - v) * (R / (R + R1));
			color = gcu_element_get_default_color (Z);
			glColor3d (color[0], color[1], color[2]);
			if (m_Display3D == WIREFRAME) {
				glBegin (GL_LINES);
				glVertex3d (v.GetX (), v.GetY (), v.GetZ ());
				glVertex3d (v0.GetX (), v0.GetY (), v0.GetZ ());
				glEnd ();
			} else if (m_Display3D == BALL_AND_STICK && bond->GetOrder () > 1)
				cyl.drawMulti (v, v0, ((bond->GetOrder () > 2)? 7.: 10.),
							   static_cast <int> (bond->GetOrder ()), 15., normal);
			else
				cyl.draw (v, v0, 12.);
			color = gcu_element_get_default_color (Z1);
			glColor3d (color[0], color[1], color[2]);
			if (m_Display3D == WIREFRAME) {
				glBegin (GL_LINES);
				glVertex3d (v0.GetX (), v0.GetY (), v0.GetZ ());
				glVertex3d (v1.GetX (), v1.GetY (), v1.GetZ ());
				glEnd ();
			} else if (m_Display3D == BALL_AND_STICK && bond->GetOrder () > 1)
				cyl.drawMulti (v0, v1, ((bond->GetOrder () > 2)? 7.: 10.),
							   static_cast <int> (bond->GetOrder ()), 15., normal);
			else
				cyl.draw (v0, v1, 12.);
			bond = m_Mol->GetNextBond (j);
		}
	}
}

void Chem3dDoc::Clear ()
{
	Object::Clear ();
	m_Mol = NULL;
}

void Chem3dDoc::ChangedDisplay3D ()
{
	if (!m_Mol)
		return;
	std::list <Atom *>::const_iterator i;
	Atom const *atom = m_Mol->GetFirstAtom (i);
	unsigned int Z;
	gdouble R, w, x, y, z;
	GcuAtomicRadius rad;
	rad.type = GCU_VAN_DER_WAALS;
	rad.charge = 0;
	rad.cn = -1;
	rad.spin = GCU_N_A_SPIN;
	rad.scale = NULL;
	m_MaxDist = 0;
	while (atom) {
		Z = atom->GetZ ();
		if (Z > 0) {
			if (m_Display3D == CYLINDERS) {
				R = 12.;
			} else if (m_Display3D == WIREFRAME) {
				R = 0.;
			} else {
				rad.Z = Z;
				Element::GetElement (Z)->GetRadius (&rad);
				R = rad.value.value;
				if (m_Display3D == BALL_AND_STICK)
					R *= 0.2;
			}
			atom->GetCoords (&x, &y, &z);
			if ((w = sqrt (x * x + y * y + z * z)) > m_MaxDist - R)
				m_MaxDist = w + R;
		}
		atom = m_Mol->GetNextAtom (i);
	}
}

}	//	namespace gcu
