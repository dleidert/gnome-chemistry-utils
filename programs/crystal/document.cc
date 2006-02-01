// -*- C++ -*-

/* 
 * Gnome Crystal
 * document.cc 
 *
 * Copyright (C) 2000-2004
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "gcrystal.h"
#include <unistd.h>
#include <locale.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "document.h"
#include "view.h"
#include "celldlg.h"
#include "atomsdlg.h"
#include "linesdlg.h"
#include "sizedlg.h"
#include "cleavagesdlg.h"
#include "globals.h"
#include "filesel.h"
#include <libgnome/libgnome.h>
#include <glade/glade.h>
#include <math.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include <vector>
#include <map>
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

#define SAVE	1
#define LOAD	2
#define XML		3
#define FORMAT	4

#define PREC 1e-3

using namespace std;

char *LatticeName[] = {"simple cubic",
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
	"triclinic"};

static void do_save_as(const gchar* filename, gcView* pView)
{
	gcDocument *pDoc = pView->GetDocument();
	pDoc->SetFileName(filename);
}

gcDocument::gcDocument():CrystalDoc()
{
	Init();
	m_filename = NULL;
	m_title = NULL;
	m_bEmpty = true;
	m_bDirty = false;
	m_bClosing = false;
	m_ps = NULL;
}

/*gcDocument::gcDocument(bool create_view)
{
	Init();
	if (create_view) pView = m_pView = new gcView(this);
}*/

gcDocument::~gcDocument()
{
	if (m_filename != NULL) g_free(m_filename);
	if (m_title) g_free(m_title);
	Reinit();
	gcDialog *dialog;
	while (!m_Dialogs.empty())
	{
		dialog = m_Dialogs.front();
		m_Dialogs.pop_front();
		dialog->Destroy();
	}
}

void gcDocument::Define(unsigned nPage)
{
	switch(nPage)
	{
		case 0:
			new gcCellDlg(this);
			break;
		case 1:
			new gcAtomsDlg(this);
			break;
		case 2:
			new gcLinesDlg(this);
			break;
		case 3:
			new gcSizeDlg(this);
			break;
		case 4:
			new gcCleavagesDlg(this);
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

void gcDocument::SetFileName(const gchar* filename)
{
	if (m_filename) g_free(m_filename);
	m_filename = g_strdup(filename);
	int i = strlen(filename) - 1;
	while ((m_filename[i] != '/') && (i >= 0)) i--;
	if (i >=0) 
	{
		m_filename[i] = 0;
		chdir(m_filename);
		m_filename[i] = '/';
	}
	i++;
	int j = strlen(filename) - 1;
	while ((i < j) && (m_filename[j] != '.')) j--;
	gchar* title = (strcmp(m_filename + j, ".gcrystal"))? g_strdup(m_filename + i):g_strndup(m_filename + i, j - i);
	SetTitle(title);
	g_free(title);
}

void gcDocument::SetTitle(const gchar* title)
{
	if (m_title) g_free(m_title);
	m_title = g_strdup(title);
	list<CrystalView*>::iterator view;
	GtkLabel *label;
	for (view = m_Views.begin(); view != m_Views.end(); view++)
	{
		label = ((gcView*)(*view))->GetLabel();
		if (label) gtk_label_set_text(label, title);
	}
}

void gcDocument::Save()
{
	if (!m_filename) return;
	gchar buf[256];
	xmlDocPtr xml;
	xmlNodePtr node;
	char *old_num_locale;

	try
	{
		xml = BuildXMLTree();
	
		if (xmlSaveFile(m_filename, xml) < 0) Error(SAVE);
			
		xmlFreeDoc(xml);
		m_bDirty = false;
	}
	catch (int num)
	{
		xmlFreeDoc(xml);
		setlocale(LC_NUMERIC, old_num_locale);
		g_free(old_num_locale);
		Error(SAVE);
	}
}

void gcDocument::Error(int num)
{
	gchar *mess;
	GtkWidget* message;
	switch (num)
	{
	case SAVE:
		mess = g_strdup_printf(_("Could not save file\n%s"),m_filename);
		break;
	case LOAD:
		mess = g_strdup_printf(_("Could not load file\n%s"),m_filename);
		break;
	case XML:
		mess = g_strdup_printf(_("%s: invalid xml file.\nTree is empty?"),m_filename);
		break;
	case FORMAT:
		mess = g_strdup_printf(_("%s: invalid file format."),m_filename);
		break;
	}
	message = gtk_message_dialog_new(NULL, (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, mess);
	g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
	gtk_widget_show(message);
	g_free(mess);
}

bool gcDocument::Load(const gchar* filename)
{
	xmlDocPtr xml;
	gchar *oldfilename, *oldtitle;
	if (m_filename) oldfilename = g_strdup(m_filename);
	else oldfilename = NULL;
	oldtitle = g_strdup(m_title);
	try
	{
		if (SetFileName(filename),!m_filename || !m_title) throw (int) 0;
		if (!(xml = xmlParseFile(filename))) throw (int) 1;
		if (xml->children == NULL) throw (int) 2;
		if (strcmp((char*)xml->children->name, "crystal")) throw (int) 3;
		if (oldfilename) g_free(oldfilename);
		g_free(oldtitle);
		ParseXMLTree(xml->children);
		xmlFreeDoc(xml);
		return true;
	}
	catch (int num)
	{
		switch (num)
		{
			case 2: Error(XML); break;
			case 3: Error(FORMAT); break;
			default: Error(LOAD);
		}
		if (num > 0)
		{
			if (oldfilename) 
			{
				SetFileName(oldfilename);
				g_free(oldfilename);
			}
			else
			{
				g_free(m_filename);
				m_filename = NULL;
			}
			SetTitle(oldtitle);
			g_free(oldtitle);
		}
		if (num > 1) xmlFreeDoc(xml);
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
			else if (!strcmp((gchar*)node->name, "view"))
			{
				if (bViewLoaded && !IsEmbedded())
				{
					gcView* pView = new gcView(this);
					pView->LoadOld(node);
					m_Views.push_back(pView);
					if (!RequestApp(pView))
					{
						delete pView;
					}
				}
				else
				{
					m_Views.front()->Load(node); //the first view is created with the document
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

void gcDocument::OnExportVRML(const gchar* FileName, gcView* pView)
{
	char *old_num_locale, tmp[128];
	double x0, x1, x2, x3, x4, x5;
	int n = 0;
	ofstream file(FileName);
	std::map<std::string, sAtom /*list<gcAtom*>*/ >AtomsMap;
	std::map<std::string, sLine>LinesMap;
	if (!file)
	{
		gchar* mess = g_strdup_printf(_("Can't create file %s"), FileName);
		GtkWidget* message = gtk_message_dialog_new(NULL, (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, mess);
		gtk_widget_show(message);
		g_free(mess);
	}
	old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	file << "#VRML V2.0 utf8" << endl;
	
	//Create prototypes for atoms
	CrystalAtomList::iterator i;
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
	CrystalLineList::iterator j;
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
	pView->GetBackgroundColor(&x0, &x1, &x2, &x3);
	file << "Background{skyColor " << x0 << " " << x1 << " " << x2 << "}" << endl;
	file << "Viewpoint {fieldOfView " << pView->GetFoV()/90*1.570796326794897 <<"\tposition 0 0 " << pView->GetPos() / 100 << "}" << endl;
	pView->GetRotation(&x0, &x1, &x2);
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

	file.close();
	setlocale(LC_NUMERIC, old_num_locale);
	g_free(old_num_locale);
}

gcView *gcDocument::GetNewView()
{
}

void gcDocument::SetDirty()
{
	m_bDirty = true;
	if (m_ps) bonobo_persist_set_dirty(m_ps, m_bDirty);
}

void gcDocument::AddView(gcView* pView)
{
	m_Views.push_back(pView);
	if (!m_bEmpty) m_bDirty = true;
}

bool gcDocument::RemoveView(gcView* pView)
{
	if (pView->IsLocked()) return false;
	if (m_Views.size() > 1)
	{
		m_Views.remove(pView);
		if (!m_bClosing && !m_bEmpty) m_bDirty = true;
		return true;
	}
	if (IsDirty())
	{
		if (!VerifySaved()) return false;
	}
	RemoveDocument(this);
	return true;
}

bool gcDocument::VerifySaved()
{
	m_bClosing = true;
	if (!m_bDirty) return true;
	gchar* str = g_strdup_printf(_("\"%s\" has been modified.  Do you wish to save it?"), m_title);
	GtkWidget* mbox;
	int res;
	do
	{
		mbox = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, str);
		gtk_dialog_add_button(GTK_DIALOG(mbox),  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		res = gtk_dialog_run(GTK_DIALOG(mbox));
		gtk_widget_destroy(mbox);
		if (res == GTK_RESPONSE_YES)
		{
			if (m_filename == NULL)
			{
				gcFileSel* FileSel = new gcFileSel(_("Save model as..."), do_save_as, true, ".gcrystal", (gcView*)m_Views.front(), true);
				while (((gcView*)(m_Views.front()))->IsLocked())
					if (gtk_events_pending()) gtk_main_iteration();
			}
			if (m_filename) Save();
		}
	}
	while ((res == GTK_RESPONSE_YES) && (m_filename == NULL));
	if (res == GTK_RESPONSE_NO) m_bDirty = false;
	else if (res == GTK_RESPONSE_CANCEL) m_bClosing = false;
	g_free(str);
	return (res != GTK_RESPONSE_CANCEL);
}

void gcDocument::NotifyDialog(gcDialog* dialog)
{
	m_Dialogs.push_front(dialog);
}

void gcDocument::RemoveDialog(gcDialog* dialog)
{
	m_Dialogs.remove(dialog);
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

const char* gcDocument::GetProgramId()
{
	return "Gnome Crystal "VERSION;
}
