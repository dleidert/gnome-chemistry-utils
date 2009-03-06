// -*- C++ -*-

/* 
 * Gnome Crystal
 * document.cc 
 *
 * Copyright (C) 2000-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "gcrystal.h"
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "application.h"
#include "document.h"
#include "view.h"
#include "window.h"
#include "celldlg.h"
#include "atomsdlg.h"
#include "linesdlg.h"
#include "sizedlg.h"
#include "cleavagesdlg.h"
#include "globals.h"
#include <gcu/filechooser.h>
#include <glade/glade.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include <clocale>
#include <cmath>
#include <vector>
#include <map>
#include <fstream>
#include <ostream>
#include <sstream>
#include <gio/gio.h>
#include <glib/gi18n.h>
#ifdef HAVE_OPENBABEL_2_2
#	include <openbabel/format.h>
#	include <openbabel/obconversion.h>
#	include <openbabel/math/matrix3x3.h>
using namespace OpenBabel;
#endif
#include <cstring>

#define SAVE	1
#define LOAD	2
#define XML		3
#define FORMAT	4

#define PREC 1e-3

using namespace std;

gcDocument::gcDocument (gcApplication *pApp) :CrystalDoc (pApp)
{
	Init ();
	m_filename = NULL;
	m_title = NULL;
	m_bClosing = false;
	m_ReadOnly = false;
	m_Author = m_Mail = m_Label = m_Comment = NULL;
	g_date_set_time_t (&m_CreationDate, time (NULL));
	g_date_clear (&m_RevisionDate, 1);
}

gcDocument::~gcDocument()
{
	g_free(m_filename);
	g_free(m_title);
	Reinit();
	Dialog *dialog;
	while (!m_Dialogs.empty())
	{
		dialog = m_Dialogs.front();
		m_Dialogs.pop_front();
		dialog->Destroy();
	}
}

void gcDocument::Define (unsigned nPage)
{
	switch(nPage) {
	case 0:
		new gcCellDlg (dynamic_cast <gcApplication *> (m_App), this);
		break;
	case 1:
		new gcAtomsDlg (dynamic_cast <gcApplication *> (m_App), this);
		break;
	case 2:
		new gcLinesDlg (dynamic_cast <gcApplication *> (m_App), this);
		break;
	case 3:
		new gcSizeDlg (dynamic_cast <gcApplication *> (m_App), this);
		break;
	case 4:
		new gcCleavagesDlg (dynamic_cast <gcApplication *> (m_App), this);
		break;
	}
}

void gcDocument::Update()
{
	CrystalDoc::Update();
	UpdateAllViews();
}

void gcDocument::UpdateAllViews()
{
	list<CrystalView*>::iterator i;
	for (i = m_Views.begin(); i != m_Views.end(); i++) (*i)->Update();
}

void gcDocument::GetSize(gdouble* xmin, gdouble* xmax, gdouble* ymin, gdouble* ymax, gdouble* zmin, gdouble* zmax)
{
	*xmin = m_xmin;
	*xmax = m_xmax;
	*ymin = m_ymin;
	*ymax = m_ymax;
	*zmin = m_zmin;
	*zmax = m_zmax;
}

void gcDocument::SetSize(gdouble xmin, gdouble xmax, gdouble ymin, gdouble ymax, gdouble zmin, gdouble zmax)
{
	m_xmin = xmin;
	m_xmax = xmax;
	m_ymin = ymin;
	m_ymax = ymax;
	m_zmin = zmin;
	m_zmax = zmax;
}

void gcDocument::GetCell(gcLattices *lattice, gdouble *a, gdouble *b, gdouble *c, gdouble *alpha, gdouble *beta, gdouble *gamma)
{
	*lattice = m_lattice;
	*a = m_a;
	*b = m_b;
	*c = m_c;
	*alpha = m_alpha;
	*beta = m_beta;
	*gamma = m_gamma;
}

void gcDocument::SetCell(gcLattices lattice, gdouble a, gdouble b, gdouble c, gdouble alpha, gdouble beta, gdouble gamma)
{
	m_lattice = lattice;
	m_a = a;
	m_b = b;
	m_c = c;
	m_alpha = alpha;
	m_beta = beta;
	m_gamma = gamma;
}

void gcDocument::SetFileName (const string &filename)
{
	GFile *file = g_file_new_for_uri (filename.c_str ());
	GError *error = NULL;
	GFileInfo *info = g_file_query_info (file,
							  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
							  G_FILE_QUERY_INFO_NONE, NULL, &error);
	if (error) {
		g_warning ("GIO error: %s", error->message);
		g_error_free (error);
		m_ReadOnly = true;
	} else
		m_ReadOnly = !g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
	if (info)
		g_object_unref (info);
	g_object_unref (file);
	if (m_filename)
		g_free (m_filename);
	m_filename = g_strdup (filename.c_str ());
	char *dirname = g_path_get_dirname (m_filename);
	m_App->SetCurDir (dirname);
	g_free (dirname);
	int i = filename.length () - 1;
	while ((m_filename[i] != '/') && (i >= 0))
		i--;
	if (i >=0) {
		m_filename [i] = 0;
		chdir (m_filename);
		m_filename[i] = '/';
	}
	i++;
	int j = filename.length () - 1;
	while ((i < j) && (m_filename[j] != '.'))
		j--;
	if (!m_title) {
		g_free (m_Label);
		char *buf = (strcmp (m_filename + j, ".gcrystal"))? g_strdup (m_filename + i): g_strndup (m_filename + i, j - i);
		m_Label = g_uri_unescape_string (buf, NULL);
		g_free (buf);
	}
}

void gcDocument::SetTitle(const gchar* title)
{
	g_free(m_title);
	m_title = g_strdup(title);
	g_free(m_Label);
	m_Label = g_strdup(title);	
}

static int cb_xml_to_vfs (GOutputStream *output, const char* buf, int nb)
{
	GError *error = NULL;
	int n = g_output_stream_write (output, buf, nb, NULL, &error);
	if (error) {
		g_message ("GIO error: %s", error->message);
		g_error_free (error);
	}
	return n;
}

void gcDocument::Save() const
{
	if (!m_filename)
		return;
	xmlDocPtr xml = NULL;

	try {
		xml = BuildXMLTree();

		if (!g_date_valid (&m_CreationDate))
			g_date_set_time_t (&const_cast <gcDocument *> (this)->m_CreationDate, time (NULL));
		g_date_set_time_t (&const_cast <gcDocument *> (this)->m_RevisionDate, time (NULL));
		gchar tmp[64];
		g_date_strftime (tmp, sizeof (tmp), "%m/%d/%Y", &m_CreationDate);
		xmlNewProp (xml->children, (xmlChar*) "creation", (xmlChar*) tmp);
		g_date_strftime (tmp, sizeof (tmp), "%m/%d/%Y", &m_RevisionDate);
		xmlNewProp (xml->children, (xmlChar*) "revision", (xmlChar*) tmp);
		xmlNodePtr node;

		if (m_title && *m_title) {
			node = xmlNewDocNode (xml, NULL, (xmlChar*) "title", (xmlChar*) m_title);
			if (node)
				xmlAddChild (xml->children, node);
			else
				throw (int) 0;
		}
		if ((m_Author && *m_Author) || (m_Mail && *m_Mail)) {
			node = xmlNewDocNode (xml, NULL, (xmlChar*) "author", NULL);
			if (node) {
				if (m_Author && *m_Author)
					xmlNewProp (node, (xmlChar*) "name", (xmlChar*) m_Author);
				if (m_Mail && *m_Mail)
					xmlNewProp (node, (xmlChar*) "e-mail", (xmlChar*) m_Mail);
				xmlAddChild (xml->children, node);
			}
			else
				throw (int) 0;
		}
		if (m_Comment && *m_Comment) {
			node = xmlNewDocNode (xml, NULL, (xmlChar*) "comment", (xmlChar*) m_Comment);
			if (node)
				xmlAddChild (xml->children, node);
			else
				throw (int) 0;
		}
		xmlIndentTreeOutput = true;
		xmlKeepBlanksDefault (0);
	
		xmlOutputBufferPtr buf = xmlAllocOutputBuffer (NULL);
		GFile *file = g_file_new_for_uri (m_filename);
		GError *error = NULL;
			if (g_file_query_exists (file, NULL)) {
				// FIXME: for now, delete it, but we might make a backup?
				g_file_delete (file, NULL, &error);
				if (error) {
					g_message ("GIO error: %s", error->message);
					g_error_free (error);
					g_object_unref (file);
					throw (int) 1;
				}
			}
		GOutputStream *output = G_OUTPUT_STREAM (g_file_create (file, G_FILE_CREATE_NONE, NULL, &error));
		if (error) {
			g_message ("GIO error: %s", error->message);
			g_error_free (error);
			g_object_unref (file);
			throw (int) 1;
		}
		buf->context = output;
		buf->closecallback = NULL;
		buf->writecallback = (xmlOutputWriteCallback) cb_xml_to_vfs;
		int n = xmlSaveFormatFileTo (buf, xml, NULL, true);
		g_output_stream_close (output, NULL, NULL);
		g_object_unref (file);
		if (n < 0)
			throw 1;

		xmlFreeDoc (xml);
		const_cast <gcDocument *> (this)->SetDirty (false);
		const_cast <gcDocument *> (this)->m_ReadOnly = false;	// if saving succeded, the file is not read only...
	}
	catch (int num) {
		xmlFreeDoc (xml);
		Error (SAVE);
	}
}

void gcDocument::Error (int num) const
{
	gchar *mess = NULL;
	GtkWidget* message;
	char *unescaped = g_uri_unescape_string (m_filename, NULL);
	switch (num) {
	case SAVE:
		mess = g_strdup_printf (_("Could not save file\n%s"), unescaped);
		break;
	case LOAD:
		mess = g_strdup_printf (_("Could not load file\n%s"), unescaped);
		break;
	case XML:
		mess = g_strdup_printf (_("%s: invalid xml file.\nTree is empty?"), unescaped);
		break;
	case FORMAT:
		mess = g_strdup_printf (_("%s: invalid file format."), unescaped);
		break;
	}
	g_free (unescaped);
	message = gtk_message_dialog_new (NULL, (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, mess);
	g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
	gtk_widget_show (message);
	g_free (mess);
}

bool gcDocument::Load (const string &filename)
{
	xmlDocPtr xml = NULL;
	gchar *oldfilename, *oldtitle;
	if (m_filename)
		oldfilename = g_strdup (m_filename);
	else oldfilename = NULL;
	oldtitle = g_strdup (m_title);
	try {
		if (SetFileName (filename), !m_filename || !m_Label)
			throw (int) 1;
		if (!(xml = xmlParseFile (filename.c_str ())))
			throw (int) 2;
		if (xml->children == NULL)
			throw (int) 3;
		if (strcmp ((char*) xml->children->name, "crystal"))
			throw (int) 4;
		if (oldfilename)
			g_free(oldfilename);
		g_free (oldtitle);
		ParseXMLTree (xml->children);
		xmlFreeDoc (xml);
		return true;
	}
	catch (int num) {
		switch (num)
		{
		case 2:
		case 3:
			Error(XML);
			break;
		case 4:
			Error(FORMAT);
			break;
		default:
			Error(LOAD);
		}
		if (num > 0) {
			if (oldfilename)  {
				SetFileName (oldfilename);
				g_free (oldfilename);
			} else {
				g_free (m_filename);
				m_filename = NULL;
			}
			SetTitle (oldtitle);
			g_free (oldtitle);
		}
		if (num > 1)
			xmlFreeDoc(xml);
		return false;
	}
}

void gcDocument::ParseXMLTree(xmlNode* xml)
{
	char *old_num_locale, *txt;
	xmlNodePtr node;
	bool bViewLoaded = false;

	Reinit();
	old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	//look for generator node
	unsigned version = 0xffffff , major, minor, micro;
	node = xml->children;
	if (m_Author) {
		g_free (m_Author);
		m_Author = NULL;
	}
	if (m_Mail) {
		g_free (m_Mail);
		m_Mail = NULL;
	}
	if (m_Comment) {
		g_free (m_Comment);
		m_Comment = NULL;
	}
	g_date_clear (&m_CreationDate, 1);
	g_date_clear (&m_RevisionDate, 1);
	txt = (char*) xmlGetProp (xml, (xmlChar*) "creation");
	if (txt) {
		g_date_set_parse (&m_CreationDate, txt);
		if (!g_date_valid (&m_CreationDate))
			g_date_clear (&m_CreationDate, 1);
		xmlFree (txt);
	}
	txt = (char*) xmlGetProp (xml, (xmlChar*) "revision");
	if (txt) {
		g_date_set_parse (&m_RevisionDate, txt);
		if (!g_date_valid(&m_RevisionDate))
			g_date_clear(&m_RevisionDate, 1);
		xmlFree (txt);
	}

	node = GetNodeByName (xml, "title");
	if (node) {
		txt = (char*) xmlNodeGetContent (node);
		if (txt) {
			g_free (m_title);
			m_title = g_strdup (txt);
			xmlFree (txt);
		}
	}
	node = GetNodeByName (xml, "author");
	if (node) {
		txt = (char*) xmlGetProp (node, (xmlChar*) "name");
		if (txt) {
			m_Author = g_strdup (txt);
			xmlFree (txt);
		}
		txt = (char*) xmlGetProp (node, (xmlChar*) "e-mail");
		if (txt) {
			m_Mail = g_strdup (txt);
			xmlFree (txt);
		}
	}
	node = GetNodeByName (xml, "comment");
	if (node) {
		txt = (char*) xmlNodeGetContent (node);
		if (txt) {
			m_Comment = g_strdup (txt);
			xmlFree (txt);
		}
	}
	while (node)
	{
		if (!strcmp ((const char*)(node->name), "generator")) break;
		node = node->next;
	}
	if (node)
	{
		txt = (char*)xmlNodeGetContent(node);
		if (txt)
		{
			if (sscanf(txt, "Gnome Crystal %d.%d.%d", &major, &minor, &micro) == 3)
				version = micro + minor * 0x100 + major * 0x10000;
			xmlFree(txt);
		}
	}
	if (version >= 0x500)
	{
		CrystalDoc::ParseXMLTree(xml);
	}
	else
	{
		node = xml->children;
		while(node)
		{
			if (!strcmp((gchar*)node->name, "lattice"))
			{
				txt = (char*)xmlNodeGetContent(node);
				if (txt)
				{
					int i = 0;
					while (strcmp(txt, LatticeName[i]) && (i < 14)) i++;
					if (i < 14) m_lattice = (gcLattices)i;
					xmlFree(txt);
				}
			}
			else if (!strcmp((gchar*)node->name, "cell"))
			{
				txt = (char*)xmlNodeGetContent(node);
				if (txt)
				{
					sscanf(txt, "%lg %lg %lg %lg %lg %lg", &m_a, &m_b, &m_c, &m_alpha, &m_beta, &m_gamma);
					xmlFree(txt);
				}
			}
			else if (!strcmp((gchar*)node->name, "size"))
			{
				txt = (char*)xmlNodeGetContent(node);
				if (txt)
				{
					sscanf(txt, "%lg %lg %lg %lg %lg %lg", &m_xmin, &m_ymin, &m_zmin, &m_xmax, &m_ymax, &m_zmax);
					xmlFree(txt);
				}
				txt = (char*)xmlGetProp(node, (xmlChar*)"fixed");
				if (txt)
				{
					if (!strcmp(txt, "true")) m_bFixedSize = true;
					xmlFree(txt);
				}
			}
			else if (!strcmp((gchar*)node->name, "atom"))
			{
				gcAtom *pAtom = new gcAtom();
				if (pAtom->LoadOld(node, version)) AtomDef.push_back((CrystalAtom*)pAtom);
				else delete pAtom;
			}
			else if (!strcmp((gchar*)node->name, "line"))
			{
				gcLine *pLine = new gcLine();
				if (pLine->LoadOld(node, version)) LineDef.push_back((CrystalLine*)pLine);
				else delete pLine;
			}
			else if (!strcmp((gchar*)node->name, "cleavage"))
			{
				gcCleavage *pCleavage = new gcCleavage();
				if (pCleavage->LoadOld(node)) Cleavages.push_back((CrystalCleavage*)pCleavage);
				else delete pCleavage;
			}
			else if (!strcmp( (gchar*) node->name, "view")) {
				if (bViewLoaded) {
					gcWindow *pWindow = new gcWindow (dynamic_cast <gcApplication *> (m_App), this);
					gcView *pView = pWindow->GetView ();
					pView->LoadOld(node);
				} else {
					m_Views.front ()->Load (node); //the first view is created with the document
					bViewLoaded = true;
				}
			}
			node = node->next;
		}
	}
	setlocale(LC_NUMERIC, old_num_locale);
	g_free(old_num_locale);
	Update();
}

void gcDocument::OnNewDocument()
{
	Reinit();
	UpdateAllViews();
}

typedef struct {int n; std::list<CrystalAtom*> l;} sAtom;
typedef struct {int n; std::list<CrystalLine*> l;} sLine;

void gcDocument::OnExportVRML (const string &FileName) const
{
	char *old_num_locale, tmp[128];
	double x0, x1, x2, x3, x4, x5;
	int n = 0;
	try {
		ostringstream file;
		GError *error = NULL;
		GFile *stream = g_file_new_for_uri (FileName.c_str ());
		GOutputStream *output = G_OUTPUT_STREAM (g_file_create (stream, G_FILE_CREATE_NONE, NULL, &error));
		if (error) {
			cerr << "gio error: " << error->message << endl;
			g_error_free (error);
			g_object_unref (file);
			throw (int) 1;
		}
		std::map<std::string, sAtom>AtomsMap;
		std::map<std::string, sLine>LinesMap;
		old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
		setlocale(LC_NUMERIC, "C");

		file << "#VRML V2.0 utf8" << endl;
		
		//Create prototypes for atoms
		CrystalAtomList::const_iterator i;
		for (i = Atoms.begin(); i != Atoms.end(); i++)
		{
			(*i)->GetColor(&x0, &x1, &x2, &x3);
			snprintf(tmp, sizeof(tmp), "%g %g %g %g %g", (*i)->GetSize(), x0, x1, x2, x3);
			if (AtomsMap[tmp].l.empty())
			{
				AtomsMap[tmp].n = n;
				file << "PROTO Atom" << n++ << " [] {Shape {" << endl << "\tgeometry Sphere {radius " << (*i)->GetSize() / 100 << "}" << endl;
				file << "\tappearance Appearance {" << endl << "\t\tmaterial Material {" << endl << "\t\t\tdiffuseColor " << x0 << " " << x1 << " " << x2 << endl;
				if (x3 < 1) file << "\t\t\ttransparency " << (1 - x3) << endl;
				file << "\t\t\tspecularColor 1 1 1" << endl << "\t\t\tshininess 0.9" << endl << "\t\t}" << endl << "\t}\r\n}}" << endl;
			}
			AtomsMap[tmp].l.push_back(*i);
		}
	
		//Create prototypes for bonds
		CrystalLineList::const_iterator j;
		n = 0;
		for (j = Lines.begin(); j != Lines.end(); j++)
		{
			(*j)->GetColor(&x0, &x1, &x2, &x3);
			snprintf(tmp, sizeof(tmp), "%g %g %g %g %g %g", (*j)->Long(), (*j)->GetRadius(), x0, x1, x2, x3);
			if (LinesMap[tmp].l.empty())
			{
				LinesMap[tmp].n = n;
				file << "PROTO Bond" << n++ << " [] {Shape {" << endl << "\tgeometry Cylinder {radius " << (*j)->GetRadius() / 100 << "\theight " << (*j)->Long() / 100 << "}" << endl;
				file << "\tappearance Appearance {" << endl << "\t\tmaterial Material {" << endl << "\t\t\tdiffuseColor " << x0 << " " << x1 << " " << x2 << endl;
				if (x3 < 1) file << "\t\t\ttransparency " << (1 - x3) << endl;
				file << "\t\t\tspecularColor 1 1 1" << endl << "\t\t\tshininess 0.9" << endl << "\t\t}" << endl << "\t}\r\n}}" << endl;
			}
			LinesMap[tmp].l.push_back(*j);
		}
		
		//world begin
		m_pActiveView->GetBackgroundColor(&x0, &x1, &x2, &x3);
		file << "Background{skyColor " << x0 << " " << x1 << " " << x2 << "}" << endl;
		file << "Viewpoint {fieldOfView " << m_pActiveView->GetFoV()/90*1.570796326794897 <<"\tposition 0 0 " << m_pActiveView->GetPos() / 100 << "}" << endl;
		m_pActiveView->GetRotation(&x0, &x1, &x2);
		Matrix m(x0/90*1.570796326794897, x1/90*1.570796326794897, x2/90*1.570796326794897, euler);
		file << "Transform {" << endl << "\tchildren [" << endl;
	
		std::map<std::string, sAtom>::iterator k;
		for (k = AtomsMap.begin(); k != AtomsMap.end(); k++)
		{
			for (i = (*k).second.l.begin(); i != (*k).second.l.end(); i++)
			{
				if (!(*i)->IsCleaved())
				{
					x0 = (*i)->x();
					x1 = (*i)->y();
					x2 = (*i)->z();
					m.Transform(x0, x1, x2);
					file << "\t\tTransform {translation " << x1 / 100 << " " << x2 / 100 << " " << x0 / 100\
						<<  " children [Atom" << (*k).second.n << " {}]}" << endl;
				}
			}
			(*k).second.l.clear();
		}
		AtomsMap.clear();
		
		std::map<std::string, sLine>::iterator l;
		n = 0;
		for (l = LinesMap.begin(); l != LinesMap.end(); l++)
		{
			for (j = (*l).second.l.begin(); j != (*l).second.l.end(); j++)
			{
				if (!(*j)->IsCleaved())
				{
					x0 = (*j)->X1();
					x1 = (*j)->Y1();
					x2 = (*j)->Z1();
					m.Transform(x0, x1, x2);
					x3 = (*j)->X2();
					x4 = (*j)->Y2();
					x5 = (*j)->Z2();
					m.Transform(x3, x4, x5);
					CrystalLine line(gcu::unique, x0, x1, x2, x3, x4, x5, 0.0, 0.0, 0.0, 0.0, 0.0);
					line.GetRotation(x0, x1, x2, x3);
					file << "\t\tTransform {" << endl << "\t\t\trotation " << x1 << " " << x2 << " " << x0 << " " << x3 << endl;
					x0 = (line.X1() + line.X2()) / 200;
					x1 = (line.Y1() + line.Y2()) / 200;
					x2 = (line.Z1() + line.Z2()) / 200;
					file << "\t\t\ttranslation " << x1 << " " << x2  << " " << x0 <<  endl\
							<< "\t\t\tchildren [Bond" << (*l).second.n << " {}]}" << endl;
				}
			}
			n++;
			(*l).second.l.clear();
		}
		LinesMap.clear();
		
		//end of the world
		file << "\t]" << endl << "}" << endl;

		setlocale(LC_NUMERIC, old_num_locale);
		g_free(old_num_locale);
		g_output_stream_write (output, file.str ().c_str (), file.str ().size (), NULL, &error);
		if (error) {
			cerr << "gio error: " << error->message << endl;
			g_error_free (error);
			g_object_unref (stream);
			throw (int) 1;
		}
		g_output_stream_close (output, NULL, NULL);
		g_object_unref (stream);
	}
	catch (int n) {
		// TODO: implement a meaningful error handler.
	}
}

gcView *gcDocument::GetNewView()
{
	return NULL;
}

void gcDocument::AddView(gcView* pView)
{
	m_Views.push_back (pView);
	RenameViews ();
	if (!GetEmpty ())
		SetDirty (true);
}

bool gcDocument::RemoveView (gcView* pView)
{
	if (m_Views.size () > 1) {
		m_Views.remove (pView);
		RenameViews ();
		if (!m_bClosing && !GetEmpty ())
			SetDirty (true);
		return true;
	}
	if (GetDirty ()) {
		if (!VerifySaved ())
			return false;
	}
	dynamic_cast <gcApplication *> (m_App)->RemoveDocument (this);
	delete this;
	return true;
}

void gcDocument::RemoveAllViews ()
{
	while (m_Views.size () > 1)
		dynamic_cast<gcView*> (m_Views.front ())->GetWindow ()->Destroy ();
	// The last one is deleted separately since this will be destroyed !
	dynamic_cast<gcView*> (m_Views.front ())->GetWindow ()->Destroy ();
}

bool gcDocument::VerifySaved()
{
	m_bClosing = true;
	if (!GetDirty ())
		return true;
	gchar* str = g_strdup_printf(_("\"%s\" has been modified.  Do you wish to save it?"), m_Label);
	GtkWidget* mbox;
	int res;
	do
	{
		mbox = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, str);
		gtk_dialog_add_button(GTK_DIALOG(mbox),  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		res = gtk_dialog_run(GTK_DIALOG(mbox));
		gtk_widget_destroy(mbox);
		if (res == GTK_RESPONSE_YES) {
			if (m_filename == NULL) {
				list<string> l;
				l.push_front ("application/x-gcrystal");
				FileChooser (m_App, true, l, this);
			}
			if (m_filename)
				Save ();
		}
	}
	while ((res == GTK_RESPONSE_YES) && (m_filename == NULL));
	if (res == GTK_RESPONSE_NO)
		SetDirty (false);
	else if (res == GTK_RESPONSE_CANCEL) m_bClosing = false;
	g_free(str);
	return (res != GTK_RESPONSE_CANCEL);
}

CrystalView* gcDocument::CreateNewView()
{
	return new gcView(this);
}

CrystalAtom* gcDocument::CreateNewAtom()
{
	return (CrystalAtom*) new gcAtom();
}

CrystalLine* gcDocument::CreateNewLine()
{
	return (CrystalLine*) new gcLine();
}

CrystalCleavage* gcDocument::CreateNewCleavage()
{
	return (CrystalCleavage*) new gcCleavage();
}

const char* gcDocument::GetProgramId () const
{
	return "Gnome Crystal "VERSION;
}

void gcDocument::SaveAsImage (const string &filename, char const *type, map<string, string>& options)
{
	m_pActiveView->SaveAsImage (filename, type, options, GetApp ()->GetImageWidth (), GetApp ()->GetImageHeight ());
}

bool gcDocument::LoadNewView (xmlNodePtr node)
{
	gcWindow *pWindow = new gcWindow (dynamic_cast <gcApplication *> (m_App), this);
	gcView *pView = pWindow->GetView ();
	bool result = pView->Load (node);
	if (!result)
		delete pWindow;
	return result;
}

void gcDocument::RenameViews ()
{
	list <CrystalView *>::iterator i, iend = m_Views.end ();
	int n = 1, max = m_Views.size ();
	for (i = m_Views.begin (); i != iend; i++) {
		gcWindow *window = dynamic_cast <gcView*> (*i)->GetWindow ();
		GtkWindow *w = window->GetWindow ();
		if (!w)
			continue;
		if (max > 1) {
			char *t = g_strdup_printf ("%s (%i)", m_Label, n++);
			gtk_window_set_title (w, t);
			g_free (t);
		} else
			gtk_window_set_title (w, m_Label);
		window->ActivateActionWidget ("ui/MainMenu/FileMenu/Save", !m_ReadOnly);
		window->ActivateActionWidget ("ui/MainToolbar/Save", !m_ReadOnly);
	}
}

#ifdef HAVE_OPENBABEL_2_2
bool gcDocument::Import (const string &filename, const string& mime_type)
{
	gchar *oldfilename, *oldtitle;
	if (m_filename)
		oldfilename = g_strdup (m_filename);
	else oldfilename = NULL;
	oldtitle = g_strdup (m_title);
	char *old_num_locale;
	bool result = false, read_only = false;
	GFile *file;
	GFileInfo *info = NULL;
	GError *error = NULL;
	try {
		if (!filename.length ())
			throw (int) 0;
		file = g_file_new_for_uri (filename.c_str ());
		info = g_file_query_info (file,
								  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE","G_FILE_ATTRIBUTE_STANDARD_SIZE,
								  G_FILE_QUERY_INFO_NONE, NULL, &error);
		if (error) {
			g_warning ("GIO error: %s", error->message);
			g_error_free (error);
			if (info)
				g_object_unref (info);
			g_object_unref (file);
			throw (int) 1;
		}
		gsize size = g_file_info_get_size (info);
		read_only = !g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
		g_object_unref (info);
		if (SetFileName (filename), !m_filename)
			throw (int) 2;
		g_free (oldfilename);
		g_free (oldtitle);
		char *buf = new char[size + 1];
		GInputStream *input = G_INPUT_STREAM (g_file_read (file, NULL, &error));
		gsize n = 0;
		while (n < size) {
			n += g_input_stream_read (input, buf, size, NULL, &error);
			if (error) {
				g_message ("GIO could not read the file: %s", error->message);
				g_error_free (error);
				delete [] buf;
				g_object_unref (input);
				g_object_unref (file);
				throw (int) 3;
			}
		}
		g_object_unref (input);
		g_object_unref (file);
		buf[size] = 0;
		istringstream iss (buf);
		old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale(LC_NUMERIC, "C");
		OBMol Mol;
		OBConversion Conv;
		OBFormat* pInFormat = OBFormat::FormatFromMIME (mime_type.c_str ());
		if (pInFormat == NULL)
			throw (int) 4;
		Conv.SetInFormat (pInFormat);
		Conv.Read (&Mol, &iss);
		result = ImportOB (Mol);
		Mol.Clear ();
		setlocale (LC_NUMERIC, old_num_locale);
		g_free (old_num_locale);
		g_free (buf);
		if (!result)
			throw (int) 5;
		UpdateAllViews ();
		return true;
	}
	catch (int num) {
		switch (num)
		{
		default:
			Error(LOAD);
		}
		if (num >= 0) {
			if (oldfilename)  {
				SetFileName (oldfilename);
				g_free (oldfilename);
			} else {
				g_free (m_filename);
				m_filename = NULL;
			}
			SetTitle (oldtitle);
			g_free (oldtitle);
		}
		return false;
	}
	return false;
}
#endif

void gcDocument::SetAuthor (char const *author)
{
	g_free (m_Author);
	m_Author = g_strdup (author);
}

void gcDocument::SetMail (char const *mail)
{
	g_free (m_Mail);
	m_Mail = g_strdup (mail);
}

void gcDocument::SetComment (char const *comment)
{
	g_free (m_Comment);
	m_Comment = g_strdup (m_Comment);
}

void gcDocument::SetLabel (char const *label)
{
	g_free (m_Label);
	m_Label = g_strdup (label);
}
