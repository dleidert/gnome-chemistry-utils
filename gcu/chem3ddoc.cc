// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/chem3ddoc.cc
 *
 * Copyright (C) 2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "glview.h"
#include <gcu/chemistry.h>
#include <gcu/element.h>
#include <libgnomevfs/gnome-vfs.h>
#include <sstream>
#include <libintl.h>
#include <locale.h>
#include <openbabel/obconversion.h>
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef HAVE_FSTREAM
#	include <fstream>
#else
#	include <fstream.h>
#endif
#ifdef HAVE_OSTREAM
#	include <ostream>
#else
#	include <ostream.h>
#endif
#ifdef HAVE_SSTREAM
#	include <sstream>
#else
#	include <sstream.h>
#endif

using namespace gcu;

Chem3dDoc::Chem3dDoc (): GLDocument (NULL)
{
	m_View = new GLView (this);
	m_Display3D = BALL_AND_STICK;
}

Chem3dDoc::Chem3dDoc (Application *App, GLView *View): GLDocument (App)
{
	m_View = (View)? View: new GLView (this);
	m_Display3D = BALL_AND_STICK;
}

Chem3dDoc::~Chem3dDoc ()
{
}

void Chem3dDoc::Draw ()
{
	std::vector < OBNodeBase * >::iterator i;
	OBAtom* atom = m_Mol.BeginAtom (i);
	guint Z;
	gdouble R, w, x, y, z, x0, y0, z0, dist;
	x0 = y0 = z0 = 0.0;
	const gdouble* color;
	while (atom) {
		Z = atom->GetAtomicNum ();
		x0 += atom->GetX ();
		y0 += atom->GetY ();
		z0 += atom->GetZ ();
		atom = m_Mol.NextAtom (i);
	}
	x0 /= m_Mol.NumAtoms ();
	y0 /= m_Mol.NumAtoms ();
	z0 /= m_Mol.NumAtoms ();
	atom = m_Mol.BeginAtom (i);
	GLUquadricObj *quadObj ;
	dist = 0;
	while (atom) {
		Z = atom->GetAtomicNum ();
		R = etab.GetVdwRad (Z);
		if (m_Display3D == BALL_AND_STICK)
			R *= 0.2;
		x = atom->GetX () - x0;
		y = atom->GetY () - y0;
		z = atom->GetZ () - z0;
		color = gcu_element_get_default_color (Z);
		if ((w = sqrt (x * x + y * y + z * z)) > dist - R)
			dist = w + R;
		glPushMatrix () ;
		glTranslated (x, y, z) ;
		glColor3d (color[0], color[1], color[2]) ;
		quadObj = gluNewQuadric () ;
		gluQuadricDrawStyle (quadObj, GL_FILL);
		gluQuadricNormals (quadObj, GL_SMOOTH) ;
		gluSphere (quadObj, R, 20, 10) ;
		gluDeleteQuadric (quadObj) ;
		glPopMatrix () ;
		atom = m_Mol.NextAtom (i);
	}
	m_MaxDist = dist * 1.05;
	std::vector < OBEdgeBase * >::iterator j;
	OBBond* bond = m_Mol.BeginBond (j);
	double x1, y1, z1, arot, xrot, yrot;
	if (m_Display3D == BALL_AND_STICK)
		while (bond) {
			atom = bond->GetBeginAtom ();
			x = atom->GetX () - x0;
			y = atom->GetY () - y0;
			z = atom->GetZ () - z0;
			atom = bond->GetEndAtom ();
			x1 = atom->GetX () - x0 - x;
			y1 = atom->GetY () - y0 - y;
			z1 = atom->GetZ () - z0 - z;
			dist = sqrt (x1 * x1 + y1 * y1 + z1 * z1);
			w = sqrt (x1 * x1 + y1 * y1);
			if (w > 0) {
				xrot = - y1 / w ;
				yrot = x1 / w ;
				arot = atan2 (w, z1) * 180. / M_PI ;
			} else {
				xrot = 0;
				if (z1 > 0)
					yrot = arot = 0.0;
				else
				{
					yrot = 1.0;
					arot = 180.0;
				}
			}
			glPushMatrix ();
			glTranslated (x, y, z);
			glRotated (arot, xrot, yrot, 0.0f);
			glColor3f (0.75, 0.75, 0.75);
			quadObj = gluNewQuadric ();
			gluQuadricDrawStyle (quadObj, GL_FILL);
			gluQuadricNormals (quadObj, GL_SMOOTH);
			gluCylinder (quadObj, 0.12, 0.12, dist, 20, 10);
			gluDeleteQuadric (quadObj);
			glPopMatrix ();
			bond = m_Mol.NextBond (j);
		}
}

void Chem3dDoc::Load (char const *uri, char const *mime_type)
{
	GnomeVFSHandle *handle;
	GnomeVFSFileInfo *info = gnome_vfs_file_info_new ();
	GnomeVFSResult result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);
	if (result != GNOME_VFS_OK) {
		gnome_vfs_file_info_unref (info);
		return;
	}
	if (mime_type)
		gnome_vfs_get_file_info_from_handle (handle, info, (GnomeVFSFileInfoOptions) 0);
	else {
		gnome_vfs_get_file_info_from_handle (handle, info,
			(GnomeVFSFileInfoOptions)(GNOME_VFS_FILE_INFO_GET_MIME_TYPE |
								GNOME_VFS_FILE_INFO_FORCE_SLOW_MIME_TYPE));
		if (!strncmp (info->mime_type, "text", 4))
			gnome_vfs_get_file_info_from_handle (handle, info,
				(GnomeVFSFileInfoOptions)GNOME_VFS_FILE_INFO_GET_MIME_TYPE);
	}
	gchar *buf = new gchar[info->size + 1];
	GnomeVFSFileSize n;
	gnome_vfs_read (handle, buf, info->size, &n);
	buf[info->size] = 0;
	if (n == info->size) {
		if (!mime_type)
			mime_type = info->mime_type;
		LoadData (buf, mime_type);
		if (m_App) {
			char *dirname = g_path_get_dirname (uri);
			m_App->SetCurDir (dirname);
			g_free (dirname);
		}
	}
	gnome_vfs_file_info_unref (info);
	delete [] buf;
	g_free (handle);
	
}

void Chem3dDoc::LoadData (char const *data, char const *mime_type)
{
	istringstream is (data);
	m_Mol.Clear ();
	char *old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	OBConversion Conv;
	OBFormat* pInFormat = Conv.FormatFromMIME (mime_type);
	if (pInFormat) {
		Conv.SetInAndOutFormats (pInFormat, pInFormat);
		Conv.Read (&m_Mol,&is);
	}
	setlocale (LC_NUMERIC, old_num_locale);
	m_View->Update ();
	g_free (old_num_locale);
}

typedef struct {int n; list<OBAtom*> l;} sAtom;

void Chem3dDoc::OnExportVRML (char const *filename)
{
	char *old_num_locale;
	double R, w, x, y, z, x0, y0, z0, dist;
	int n = 0, Z;
	const gdouble* color;
	char const *symbol;
	try {
		ostringstream file;
		GnomeVFSHandle *handle = NULL;
		GnomeVFSFileSize fs;
		GnomeVFSResult res;
		map<string, sAtom> AtomsMap;
		if ((res = gnome_vfs_create (&handle, filename, GNOME_VFS_OPEN_WRITE, true, 0644)) != GNOME_VFS_OK)
			throw (int) res;
		old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");

		file << "#VRML V2.0 utf8" << endl;
		
		x0 = y0 = z0 = 0.0;
		std::vector < OBNodeBase * >::iterator i;
		OBAtom* atom = m_Mol.BeginAtom (i);
		while (atom) {
			Z = atom->GetAtomicNum ();
			x0 += atom->GetX ();
			y0 += atom->GetY ();
			z0 += atom->GetZ ();
			atom = m_Mol.NextAtom (i);
		}
		x0 /= m_Mol.NumAtoms ();
		y0 /= m_Mol.NumAtoms ();
		z0 /= m_Mol.NumAtoms ();

		//Create prototypes for atoms
		for (atom = m_Mol.BeginAtom (i); atom; atom = m_Mol.NextAtom (i)) {
			Z = atom->GetAtomicNum ();
			symbol = Element::Symbol (Z);
			if (AtomsMap[symbol].l.empty()) {
				AtomsMap[symbol].n = n;
				R = etab.GetVdwRad (Z);
				if (m_Display3D == BALL_AND_STICK)
					R *= 0.2;
				color = gcu_element_get_default_color (Z);
				file << "PROTO Atom" << n++ << " [] {Shape {" << endl << "\tgeometry Sphere {radius " << R << "}" << endl;
				file << "\tappearance Appearance {" << endl << "\t\tmaterial Material {" << endl << "\t\t\tdiffuseColor " << color[0] << " " << color[1] << " " << color[2] << endl;
				file << "\t\t\tspecularColor 1 1 1" << endl << "\t\t\tshininess 0.9" << endl << "\t\t}" << endl << "\t}\r\n}}" << endl;
			}
			AtomsMap[symbol].l.push_back(atom);
		}

		//world begin
		double conv = M_PI / 180;
		file << "Background{skyColor " << m_View->GetRed () << " " << m_View->GetBlue () << " " << m_View->GetGreen () << "}" << endl;
		file << "Viewpoint {fieldOfView " << m_View->GetAngle () / 90*1.570796326794897 << "\tposition 0 0 " << m_View->GetRadius () << "}" << endl;
		Matrix m (m_View->GetPsi () * conv, m_View->GetTheta () * conv, m_View->GetPhi () * conv, euler);
		file << "Transform {" << endl << "\tchildren [" << endl;
	
		map<std::string, sAtom>::iterator k, kend = AtomsMap.end ();
		list<OBAtom*>::iterator j, jend;
		for (k = AtomsMap.begin (); k != kend; k++) {
			jend = (*k).second.l.end ();
			for (j = (*k).second.l.begin (); j != jend; j++) {
				x = (*j)->GetX ();
				y = (*j)->GetY ();
				z = (*j)->GetZ ();
				m.Transform(x, y, z);
				file << "\t\tTransform {translation " << x << " " << y << " " << z <<  " children [Atom" << (*k).second.n << " {}]}" << endl;
			}
			(*k).second.l.clear();
		}
		AtomsMap.clear();

		//end of the world
		file << "\t]" << endl << "}" << endl;

		setlocale(LC_NUMERIC, old_num_locale);
		g_free(old_num_locale);
		if ((res = gnome_vfs_write (handle, file.str ().c_str (), (GnomeVFSFileSize) file.str ().size (), &fs)) != GNOME_VFS_OK)
			throw (int) res;
		gnome_vfs_close (handle);
	}
	catch (int n) {
		cerr <<"gnome-vfs error" << n << endl;
	}
}
