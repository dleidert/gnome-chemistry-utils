// -*- C++ -*-

/* 
 * GChemPaint library
 * document.cc 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "view.h"
#include "document.h"
#include "settings.h"
#include "docprop.h"
#include "reaction.h"
#include "mesomery.h"
#include "text.h"
#include "theme.h"
#include "tool.h"
#include "window.h"
#include <gcu/objprops.h>
#include <goffice/math/go-rangefunc.h>
#include <unistd.h>
#include <libgen.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <glib/gi18n-lib.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <clocale>
#include <stack>
#include <cstring>

using namespace OpenBabel;
using namespace std;
using namespace gcu;

namespace gcp {

SignalId OnChangedSignal;
SignalId OnDeleteSignal;
SignalId OnThemeChangedSignal;

Document::Document (Application *App, bool StandAlone, Window *window):
	gcu::Document (App),
	m_FileType ("application/x-gchempaint"),
	m_OpID (0),
	m_LastStackSize (0)
{
	m_pApp = App;
	m_pView = NULL;
	m_Window = window;
	m_filename = NULL;
	m_label = NULL;
	m_title = NULL;
	m_bWriteable = true;
	m_PangoAttrList = pango_attr_list_new ();
	m_Theme = NULL;
	SetTheme (TheThemeManager.GetTheme ("Default"));
	m_pView = new View (this, !StandAlone);
	m_bIsLoading = m_bUndoRedo = false;
	g_date_set_time_t (&CreationDate, time (NULL));
	g_date_clear (&RevisionDate, 1);
	const char* chn = getenv ("REAL_NAME");
	if (!chn)
		chn = getenv ("USERNAME");
	if (chn)
		m_author = g_strdup (chn);
	else
		m_author = NULL;
	chn = getenv ("E_MAIL");
	if (!chn)
		chn = getenv ("EMAIL_ADDRESS");
	if (chn)
		m_mail = g_strdup (chn);
	else
		m_mail = NULL;
	m_comment = NULL;
	m_pCurOp = NULL;
	m_bReadOnly = false;
	SetActive ();
	m_AllowClipboard = true;
}

Document::~Document ()
{
	Clear ();
	if (m_pView)
		delete m_pView;
	pango_attr_list_unref (m_PangoAttrList);
	if (m_Theme)
		m_Theme->RemoveClient (this);
	if (m_App)
		static_cast<Application*> (m_App)->SetActiveDocument (NULL);
}

void Document::Clear ()
{
	m_bIsLoading = true;
	if (m_pCurOp)
		delete m_pCurOp;
	m_pCurOp = NULL;
	g_free (m_filename);
	m_filename = NULL;
	g_free (m_title);
	m_title = NULL;
	g_free (m_label);
	m_label = NULL;
	g_free (m_author);
	m_author = NULL;
	g_free (m_mail);
	m_mail = NULL;
	g_free (m_comment);
	m_comment = NULL;
	map<string, Object *>::iterator it;
	Object *obj;
	while (HasChildren ()) {
		obj = GetFirstChild (it);
		obj->Lock ();
		Remove (obj);
	}
	while (!m_RedoList.empty ()) {
		delete m_RedoList.front ();
		m_RedoList.pop_front ();
	}
	while (!m_UndoList.empty ()) {
		delete m_UndoList.front ();
		m_UndoList.pop_front ();
	}
}

GtkWidget* Document::GetWidget ()
{
	return (m_pView) ? m_pView->GetWidget () : NULL;
}


const gchar* Document::GetTitle ()
{
	if (m_title)
		return m_title;
	else
		return GetLabel ();
}

const gchar* Document::GetLabel ()
{
	return m_label;
}

void Document::SetFileName (string const &Name, const gchar* mime_type)
{
	if (m_filename)
		g_free (m_filename);
	m_filename = g_strdup (Name.c_str ());
	m_FileType = mime_type;
	char *path = g_path_get_dirname (m_filename);
	m_pApp->SetCurDir (path);
	g_free (path);
	int i = strlen (m_filename) - 1;
	while ((m_filename[i] != '/') && (i >= 0))
		i--;
	i++;
	int j = strlen (m_filename) - 1;
	while ((i < j) && (m_filename[j] != '.'))
		j--;
	if (m_label)
		g_free (m_label);
	char *ext = m_filename + j + 1;
	list<string> &exts = m_pApp->GetExtensions (m_FileType);
	list<string>::iterator cur, end = exts.end ();
	for (cur = exts.begin (); cur != end; cur++)
		if (*cur == ext) {
			m_label = g_strndup (m_filename + i, j - i);
			break;
		}
	if (!m_label)
		m_label = g_strdup (m_filename + i);
}

void Document::BuildBondList (list<Bond*>& BondList, Object* obj)
{
	Object* pObject;
	map<string, Object*>::iterator i;
	for (pObject = obj->GetFirstChild (i); pObject; pObject = obj->GetNextChild (i))
		if (pObject->GetType () == gcu::BondType)
			BondList.push_back ((Bond*)(*i).second);
		else BuildBondList (BondList, pObject);
}


bool Document::ImportOB (OBMol& Mol)
{
	//Title, dates, author and so on are not imported and so are invalid
	if (m_title) {
		g_free (m_title);
		m_title = NULL;
	}
	if (m_author) {
		g_free (m_author);
		m_author = NULL;
	}
	if (m_mail) {
		g_free (m_mail);
		m_mail = NULL;
	}
	if (m_comment) {
		g_free (m_comment);
		m_comment = NULL;
	}
	g_date_clear (&CreationDate, 1);
	g_date_clear (&RevisionDate, 1);
	m_title = g_strdup (Mol.GetTitle ()); // Hmm, and if there are several molecules?
	OBAtom *atom;
	Atom* pAtom;
	vector<OBNodeBase*>::iterator i;
	for (atom = Mol.BeginAtom (i); atom; atom = Mol.NextAtom (i))
	{
		if (atom->GetAtomicNum () == 0)
			continue;
		AddAtom (pAtom = new Atom (atom));
	}
	Atom *begin, *end;
	Bond *pBond;
	unsigned char order;
	gchar* Id;
	OBBond *bond;
	vector<OBEdgeBase*>::iterator j;
	for (bond = Mol.BeginBond (j); bond; bond = Mol.NextBond (j)) {
		Id = g_strdup_printf ("a%d", bond->GetBeginAtomIdx ());
		begin = (Atom*) GetDescendant (Id);//Don't verify it is really an atom?
		g_free (Id);
		Id = g_strdup_printf ("a%d", bond->GetEndAtomIdx ());
		end = (Atom*) GetDescendant (Id);//Don't verify it is really an atom?
		g_free (Id);
		if (!end)
			continue;
		order = (unsigned char) (bond->GetBO ());
		if ((pBond = (Bond*) begin->GetBond (end)) != NULL) {
			pBond->IncOrder (order);
			m_pView->Update (pBond);
			m_pView->Update (begin);
			m_pView->Update (end);
		} else {
			Id = g_strdup_printf ("b%d", bond->GetIdx());
			pBond = new Bond (begin, end, order);
			if (bond->IsWedge ())
				pBond->SetType (UpBondType);
			else if (bond->IsHash ())
				pBond->SetType (DownBondType);
			pBond->SetId (Id);
			g_free (Id);
			AddBond (pBond);
		}
	}
	m_Empty = !HasChildren ();
	if (m_Window)
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", HasChildren ());
	return true;
}

void Document::BuildAtomTable (map<string, unsigned>& AtomTable, Object* obj, unsigned& index)
{
	Object* pObject;
	map<string, Object*>::iterator i;
	for (pObject = obj->GetFirstChild (i); pObject; pObject = obj->GetNextChild (i))
		if (pObject->GetType() == AtomType)
			AtomTable[(*i).second->GetId ()] = index++;
		else
			BuildAtomTable (AtomTable, pObject, index);
}

void Document::ExportOB ()
{
	OBMol Mol;
	map<string, unsigned>::iterator i;
	map<string, unsigned> AtomTable;
	list<Bond*> BondList;
	OBAtom obAtom;
	Atom* pgAtom;
	unsigned index = 1;
	double x, y, z;
	gchar *old_num_locale;
	map< string, Object * >::iterator m;
	stack<map< string, Object * >::iterator> iters;
	set<Object *> Mols;
	Object *Cur = this, *Ob;
	try {
		ostringstream ofs;
		GnomeVFSHandle *handle = NULL;
		GnomeVFSFileSize n;
		GnomeVFSResult res;
		res = gnome_vfs_open (&handle, m_filename, GNOME_VFS_OPEN_WRITE);
		if (res == GNOME_VFS_ERROR_NOT_FOUND)
			res = gnome_vfs_create (&handle, m_filename, GNOME_VFS_OPEN_WRITE, true, 0666);
		if (res != GNOME_VFS_OK)
			throw (int) res;
		old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		OBConversion Conv;
		OBFormat* pOutFormat = Conv.FormatFromMIME (m_FileType.c_str ());
		if (pOutFormat != NULL) {
			Conv.SetOutFormat (pOutFormat);
			Ob = GetFirstChild (m);
			while (Ob) {
				if (Ob->GetType () == MoleculeType)
					Mols.insert (Ob);
				else if (Ob->HasChildren ()) {
					Cur = Ob;
					iters.push (m);
					Ob = Cur->GetFirstChild (m);
					continue;
				}
				Ob = Cur->GetNextChild (m);
				while (!Ob && !iters.empty ()) {
					m = iters.top ();
					iters.pop ();
					Cur = Cur->GetParent ();
					Ob = Cur->GetNextChild (m);
				}
			}
			set<Object *>::iterator mi, mend = Mols.end ();
			unsigned nb = 1;
			Conv.SetOneObjectOnly (false);
			for (mi = Mols.begin (); mi != mend; mi++)
			{
				Ob = *mi;
				if (nb == Mols.size ())
					Conv.SetOneObjectOnly (true);
				Mol.BeginModify ();
				index = 1;
				BuildAtomTable (AtomTable, Ob, index);
				Mol.ReserveAtoms (AtomTable.size ());
				Mol.SetTitle ((char*) GetTitle ());
				Mol.SetDimension (2);
				for (i = AtomTable.begin (); i != AtomTable.end (); i++) {
					pgAtom = (Atom*) Ob->GetDescendant ((*i).first.data ());
					obAtom.SetIdx ((*i).second);
					obAtom.SetAtomicNum (pgAtom->GetZ ());
					pgAtom->GetCoords (&x, &y, &z);
					obAtom.SetVector (x / 100., 4. - y / 100., z / 100.);
					obAtom.SetFormalCharge (pgAtom->GetCharge ());
					Mol.AddAtom (obAtom);
					obAtom.Clear ();
				}
				BuildBondList (BondList, Ob);
				list<Bond*>::iterator j;
				int start, end, order, flag;
				for (j = BondList.begin (); j != BondList.end (); j++)
				{
					order = (*j)->GetOrder ();
					start = AtomTable[(*j)->GetAtom (0)->GetId ()];
					end = AtomTable[(*j)->GetAtom (1)->GetId ()];
					switch ((*j)->GetType ()) {
					case UpBondType:
						flag = OB_WEDGE_BOND;
						break;
					case DownBondType:
						flag = OB_HASH_BOND;
						break;
					default:
						flag = 0;
					}
					Mol.AddBond (start, end, order, flag);
				}
				Mol.EndModify ();
				Conv.SetOutputIndex (nb++);
				Conv.Write (&Mol, &ofs);
				Mol.Clear ();
				AtomTable.clear ();
				BondList.clear ();
			}
		}
		setlocale (LC_NUMERIC, old_num_locale);
		g_free (old_num_locale);
		if ((res = gnome_vfs_write (handle, ofs.str ().c_str (), (GnomeVFSFileSize) ofs.str ().size (), &n)) != GNOME_VFS_OK)
			throw (int) res;
		gnome_vfs_close (handle);
		SetReadOnly (false);
	}
	catch (int n) {
		fprintf (stderr, "gnome-vfs error #%d\n",n);
	}
}

void Document::Print (GnomePrintContext *pc, gdouble width, gdouble height)
{
	m_pView->Print (pc, width, height);
}

void Document::AddAtom (Atom* pAtom)
{
	int i = 1;
	char id[8];
	const gchar *Id;
	Id = pAtom->GetId ();
	if (Id == NULL) {
		id[0] = 'a';
		do
			snprintf (id+1, 7, "%d", i++);
		while (GetDescendant (id) != NULL);
		pAtom->SetId (id);
		Id = id;
	}
	m_pView->AddObject (pAtom);
	if (m_bIsLoading)
		return;
	Molecule* mol = new Molecule ();
	i = 1;
	id[0] = 'm';
	do
		snprintf (id+1, 7, "%d", i++);
	while (GetDescendant(id) != NULL);
	mol->SetId (id);
	AddChild (mol);
	mol->AddAtom (pAtom);
}

void Document::AddFragment (Fragment* pFragment)
{
	int i = 1;
	char id[8];
	const gchar *Id;
	Id = pFragment->GetId ();
	if (Id == NULL)  {
		id[0] = 'f';
		do
			snprintf( id+1, 7, "%d", i++);
		while (GetDescendant (id) != NULL);
		pFragment->SetId (id);
		Id = id;
	}
	m_pView->AddObject (pFragment);
	if (m_bIsLoading)
		return;
//	AddObject(pFragment);
	if (!pFragment->GetMolecule ()) {
		Molecule* mol = new Molecule ();
		i = 1;
		id[0] = 'm';
		do
			snprintf (id+1, 7, "%d", i++);
		while (GetDescendant (id) != NULL);
		mol->SetId (id);
		AddChild (mol);
		mol->AddFragment (pFragment);
	}
	pFragment->AnalContent ();
}

void Document::AddBond (Bond* pBond)
{
	int i = 1;
	char id[8];
	const gchar *Id;
	Id = pBond->GetId ();
	if (Id == NULL) {
		id[0] = 'b';
		do
			snprintf (id+1, 7, "%d", i++);
		while (GetDescendant(id) != NULL);
		pBond->SetId (id);
	}
	AddChild (pBond);
	Atom *pAtom0 = (Atom*) pBond->GetAtom (0), *pAtom1 = (Atom*) pBond->GetAtom (1);
	m_pView->Update (pAtom0);
	m_pView->Update (pAtom1);
	m_pView->AddObject (pBond);
	if (m_bIsLoading)
		return;
	//search molecules
	Molecule * pMol0 = (Molecule*) pAtom0->GetMolecule (),  *pMol1 = (Molecule*) pAtom1->GetMolecule ();
	if (pMol0 && pMol1) {
		if (pMol0 == pMol1) {
			//new cycle
			pMol0->UpdateCycles (pBond);
			m_pView->Update (pBond);
		} else {
			//merge two molecules
			pMol0->Merge (pMol1);
		}
		pMol0->AddBond (pBond);
	}
	else if (pMol0 || pMol1) {
		//add bond and atom to existing molecule
		if (!pMol0)
			pMol0 = pMol1;
		pMol0->AddAtom (pAtom0);
		pMol0->AddBond (pBond);
	}
	else {
		//new molecule
		i = 1;
		id[0] = 'm';
		do
			snprintf (id+1, 7, "%d", i++);
		while (GetDescendant (id) != NULL);
		pMol0 = new Molecule (pAtom0);
		pMol0->SetId (id);
		AddChild (pMol0);
	}
}

static int cb_xml_to_vfs (GnomeVFSHandle *handle, const char* buf, int nb)
{
	GnomeVFSFileSize ndone;
	return (int) gnome_vfs_write (handle, buf, nb, &ndone);
}

void Document::Save ()
{
	if (!m_filename || !m_bWriteable || m_bReadOnly)
		return;
	xmlDocPtr xml = NULL;
	char *old_num_locale, *old_time_locale;
	
	old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	old_time_locale = g_strdup (setlocale (LC_TIME, NULL));
	setlocale (LC_TIME, "C");

	try {
		if (m_FileType != "application/x-gchempaint")
			ExportOB ();
		else {
			xml = BuildXMLTree ();
			xmlSetDocCompressMode (xml, CompressionLevel);
			
			xmlIndentTreeOutput = true;
			xmlKeepBlanksDefault (0);
		
			GnomeVFSFileInfo *info = gnome_vfs_file_info_new ();
			gnome_vfs_get_file_info (m_filename, info, GNOME_VFS_FILE_INFO_DEFAULT);

			if (GNOME_VFS_FILE_INFO_LOCAL (info)) {
				gnome_vfs_file_info_unref (info);
				if (xmlSaveFormatFile (m_filename, xml, true) < 0) /*Error(SAVE)*/;
			} else {
				gnome_vfs_file_info_unref (info);
				xmlOutputBufferPtr buf = xmlAllocOutputBuffer (NULL);
				GnomeVFSHandle *handle;
				GnomeVFSResult result = gnome_vfs_open (&handle, m_filename, GNOME_VFS_OPEN_WRITE);
				if (result == GNOME_VFS_ERROR_NOT_FOUND)
					result = gnome_vfs_create (&handle, m_filename, GNOME_VFS_OPEN_WRITE, true, 0666);
				if (result != GNOME_VFS_OK)
					throw 1;
				buf->context = handle;
				buf->closecallback = (xmlOutputCloseCallback) gnome_vfs_close;
				buf->writecallback = (xmlOutputWriteCallback) cb_xml_to_vfs;
				int n = xmlSaveFormatFileTo (buf, xml, NULL, true);
				if (n < 0)
					throw 1;
				SetReadOnly (false);
			}
		}
		SetDirty (false);
		m_LastStackSize = m_UndoList.size ();
		m_OpID = m_UndoList.front ()->GetID ();
	}
	catch (int num) {
		if (xml)
			xmlFreeDoc (xml);
		xml = NULL;
//		Error (SAVE);
	}
	setlocale (LC_NUMERIC, old_num_locale);
	g_free (old_num_locale);
	setlocale (LC_TIME, old_time_locale);
	g_free (old_time_locale);
}

bool Document::Load (xmlNodePtr root)
{
	if (m_title) {
		g_free(m_title);
		m_title = NULL;
	}
	if (m_author) {
		g_free(m_author);
		m_author = NULL;
	}
	if (m_mail) {
		g_free(m_mail);
		m_mail = NULL;
	}
	if (m_comment) {
		g_free(m_comment);
		m_comment = NULL;
	}
	g_date_clear (&CreationDate, 1);
	g_date_clear (&RevisionDate, 1);
	xmlNodePtr node, child;
	char* tmp;
	Object* pObject;
	tmp = (char*) xmlGetProp (root, (xmlChar*) "id");
	if (tmp) {
		SetId (tmp);
		xmlFree (tmp);
	}
	tmp = (char*) xmlGetProp (root, (xmlChar*) "creation");
	if (tmp) {
		g_date_set_parse(&CreationDate, tmp);
		if (!g_date_valid(&CreationDate)) g_date_clear(&CreationDate, 1);
		xmlFree (tmp);
	}
	tmp = (char*) xmlGetProp (root, (xmlChar*) "revision");
	if (tmp) {
		g_date_set_parse(&RevisionDate, tmp);
		if (!g_date_valid(&RevisionDate)) g_date_clear(&RevisionDate, 1);
		xmlFree (tmp);
	}

	node = GetNodeByName (root, "title");
	if (node) {
		tmp = (char*) xmlNodeGetContent (node);
		if (tmp) {
			m_title = g_strdup (tmp);
			xmlFree (tmp);
		}
	}
	if (m_Window)
		m_Window->SetTitle (GetTitle ());
	node = GetNodeByName (root, "author");
	if (node) {
		tmp = (char*) xmlGetProp (node, (xmlChar*) "name");
		if (tmp) {
			m_author = g_strdup (tmp);
			xmlFree (tmp);
		}
		tmp = (char*) xmlGetProp (node, (xmlChar*) "e-mail");
		if (tmp) {
			m_mail = g_strdup (tmp);
			xmlFree (tmp);
		}
	}
	node = GetNodeByName (root, "comment");
	if (node) {
		tmp = (char*) xmlNodeGetContent (node);
		if (tmp) {
			m_comment = g_strdup (tmp);
			xmlFree (tmp);
		}
	}

	node = GetNodeByName (root, "theme");
	if (node) {
		Theme *pTheme = new Theme (NULL), *pLocalTheme;
		pTheme->Load (node);
		pLocalTheme = TheThemeManager.GetTheme (_(pTheme->GetName ().c_str ()));
		if (!pLocalTheme)
			pLocalTheme = TheThemeManager.GetTheme (pTheme->GetName ().c_str ());
		if (pLocalTheme && *pLocalTheme == *pTheme) {
			SetTheme (pLocalTheme);
			delete pTheme;
		} else {
			TheThemeManager.AddFileTheme (pTheme, GetTitle ());
			SetTheme (pTheme);
		}
	}

	m_bIsLoading = true;
	node = root->children;
	while (node) {
		child = (strcmp ((const char*) node->name, "object"))? node: node->children;
		pObject = CreateObject ((const char*) child->name, this);
		if (pObject) {
			if (!pObject->Load (child))
				delete pObject;
			else
				m_pView->AddObject (pObject);
		}
		node = node->next;
	}
	m_pView->Update (this);
	Update ();
	m_Empty = !HasChildren ();
	m_bIsLoading = false;
	if (m_Window)
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", HasChildren ());
	m_pView->EnsureSize ();
	return true;
}
	
void Document::ParseXMLTree (xmlDocPtr xml)
{
	Load (xml->children);
}

xmlDocPtr Document::BuildXMLTree ()
{
	xmlDocPtr xml;
	xmlNodePtr node;
	xmlNsPtr ns;

	xml = xmlNewDoc ((xmlChar*) "1.0");
//FIXME: do something if an error occurs 
	if (xml == NULL)
		throw (int) 0;
	
	xmlDocSetRootElement (xml,  xmlNewDocNode (xml, NULL, (xmlChar*) "chemistry", NULL));
	ns = xmlNewNs (xml->children, (xmlChar*) "http://www.nongnu.org/gchempaint", (xmlChar*) "gcp");
	xmlSetNs (xml->children, ns);
	if (!g_date_valid (&CreationDate))
		g_date_set_time_t (&CreationDate, time (NULL));
	g_date_set_time_t (&RevisionDate, time (NULL));
	gchar buf[64];
	g_date_strftime (buf, sizeof (buf), "%m/%d/%Y", &CreationDate);
	xmlNewProp (xml->children, (xmlChar*) "creation", (xmlChar*) buf);
	g_date_strftime (buf, sizeof (buf), "%m/%d/%Y", &RevisionDate);
	xmlNewProp (xml->children, (xmlChar*) "revision", (xmlChar*) buf);
	node = xmlNewDocNode (xml, NULL, (xmlChar*)"generator", (xmlChar*)"GChemPaint "VERSION);
	if (node)
		xmlAddChild (xml->children, node);
	else
		throw (int) 0;

	if (m_title && *m_title) {
		node = xmlNewDocNode (xml, NULL, (xmlChar*) "title", (xmlChar*) m_title);
		if (node)
			xmlAddChild (xml->children, node);
		else
			throw (int) 0;
	}
	if ((m_author && *m_author) || (m_mail && *m_mail)) {
		node = xmlNewDocNode (xml, NULL, (xmlChar*) "author", NULL);
		if (node) {
			if (m_author && *m_author)
				xmlNewProp (node, (xmlChar*) "name", (xmlChar*) m_author);
			if (m_mail && *m_mail)
				xmlNewProp (node, (xmlChar*) "e-mail", (xmlChar*) m_mail);
			xmlAddChild (xml->children, node);
		}
		else
			throw (int) 0;
	}
	if (m_comment && *m_comment) {
		node = xmlNewDocNode (xml, NULL, (xmlChar*) "comment", (xmlChar*) m_comment);
		if (node)
			xmlAddChild (xml->children, node);
		else
			throw (int) 0;
	}

	if (!m_Theme->Save (xml))
		throw (int) 0;
	if (!SaveChildren (xml, xml->children))
		throw 1;

	return xml;
}

void Document::Update ()
{
	std::set<Object*>::iterator i,  end = m_DirtyObjects.end ();
	TypeId Id;
	for (i = m_DirtyObjects.begin (); i != end; i++) {
		Id = (*i)->GetType ();
		switch (Id) {
		case gcu::BondType:
			m_pView->Update ((Bond*) (*i));
			break;
		}
	}
	m_DirtyObjects.clear ();
}
	
void Document::RemoveAtom (Atom* pAtom)
{
	std::map<gcu::Atom*, gcu::Bond*>::iterator i;
	Bond* pBond;
	while ((pBond = (Bond*) pAtom->GetFirstBond (i)))
	{
		if (!m_bUndoRedo)
			m_pCurOp->AddObject (pBond);
		RemoveBond (pBond);
	}
	Molecule *pMol = (Molecule*) pAtom->GetMolecule ();
	delete pMol;
	m_pView->Remove (pAtom);
	delete pAtom;
}

void Document::RemoveFragment (Fragment* pFragment)
{
	std::map<gcu::Atom*, gcu::Bond*>::iterator i;
	Atom* pAtom = pFragment->GetAtom ();
	Bond* pBond;
	while ((pBond = (Bond*) pAtom->GetFirstBond (i))) {
		if (!m_bUndoRedo)
			m_pCurOp->AddObject (pBond);
		RemoveBond (pBond);
	}
	Molecule *pMol = (Molecule*) pFragment->GetMolecule ();
	delete pMol;
	m_pView->Remove (pFragment);
	delete pFragment;
}

void Document::RemoveBond (Bond* pBond)
{
	m_pView->Remove (pBond);
	Atom *pAtom0, *pAtom1;
	pAtom0 = (Atom*) pBond->GetAtom (0);
	pAtom1 = (Atom*) pBond->GetAtom (1);
	Molecule *pMol = (Molecule*) pBond->GetMolecule ();
	char id[16];
	pMol->Lock (true);
	pAtom0->RemoveBond (pBond);
	m_pView->Update (pAtom0);
	pAtom1->RemoveBond (pBond);
	m_pView->Update (pAtom1);
	pMol->Lock (false);
	if (pBond->IsCyclic ()) {
		pMol->Remove (pBond);
		pMol->UpdateCycles ();
		Update ();
	} else {
		Object *Parent = pMol->GetParent ();
		Parent->Lock ();
		int i0 = 1;
		string align_id = pMol->GetAlignmentId ();
		delete pMol;
		Molecule * pMol = new Molecule ();
		pMol->Lock (true);
		do
			snprintf (id, sizeof (id), "m%d", i0++);
		while (GetDescendant (id) != NULL);
		pMol->SetId (id);
		Parent->AddChild (pMol); /* do not update at this point it's the caller responsibility */
		Object* pObject = pAtom0->GetParent ();
		if (pObject->GetType () == FragmentType)
			pMol->AddFragment ((Fragment*) pObject);
		else
			pMol->AddAtom (pAtom0);
		pMol->UpdateCycles ();
		if (align_id.size ()) {
			Object *obj = pMol->GetDescendant (align_id.c_str ());
			if (obj)
				pMol->SelectAlignmentItem (obj);
			align_id = "";
		}
		pMol->Lock (false);
		do
			snprintf (id, sizeof (id), "m%d", i0++);
		while (GetDescendant (id) != NULL);
		pMol = new Molecule ();
		pMol->Lock (true);
		pMol->SetId (id);
		Parent->AddChild (pMol); /* do not update at this point it's the caller responsibility */
		pObject = pAtom1->GetParent ();
		if (pObject->GetType () == FragmentType)
			pMol->AddFragment ((Fragment*) pObject);
		else
			pMol->AddAtom (pAtom1);
		pMol->UpdateCycles ();
		if (align_id.size ()) {
			Object *obj = pMol->GetDescendant (align_id.c_str ());
			if (obj)
				pMol->SelectAlignmentItem (obj);
		}
		pMol->Lock (false);
		if ((pAtom0->GetZ () == 6) && (pAtom0->GetBondsNumber () == 0))
			m_pView->Update (pAtom0);
		if ((pAtom1->GetZ () == 6) && (pAtom1->GetBondsNumber () == 0))
			m_pView->Update (pAtom1);
		Parent->Lock (false);
	}
	m_DirtyObjects.erase (pBond);
	delete pBond;
}
	
void Document::Remove (Object* pObject)
{
	switch(pObject->GetType ())
	{
		case AtomType:
			RemoveAtom ((Atom*) pObject);
			break;
		case FragmentType:
			RemoveFragment ((Fragment*) pObject);
			break;
		case gcu::BondType:
			RemoveBond ((Bond*) pObject);
			break;
		case MoleculeType: {
				((Molecule*) pObject)->Clear ();
				m_pView->Remove (pObject);
				map<string, Object*>::iterator i;
				Object* object = pObject->GetFirstChild (i);
				while (object) {
					m_pView->Remove (object);
					delete object;
					object = pObject->GetNextChild (i);
				}
				delete pObject;
			}
			break;
		default: {
				m_pView->Remove (pObject);
				map<string, Object*>::iterator i;
				Object* object = pObject->GetFirstChild (i);
				while (object) {
					if (pObject->IsLocked ())
						object->Lock ();
					Remove (object);
					object = pObject->GetFirstChild (i);
				}
				delete pObject;
			}
			break;
	}
}
	
void Document::Remove (const char* Id)
{
	Object* pObj = GetDescendant (Id);
	if (pObj) {
		pObj->Lock ();
		Remove(pObj);
	}
}

void Document::OnProperties ()
{
	new DocPropDlg (this);
}

void Document::SetTitle (const gchar* title)
{
	g_free (m_title);
	m_title = (title && *title)? g_strdup (title): NULL;
}

void Document::SetAuthor (const gchar* author)
{
	g_free (m_author);
	m_author = (author && *author)? g_strdup (author): NULL;
}

void Document::SetMail (const gchar* mail)
{
	g_free (m_mail);
	m_mail = (mail && *mail)? g_strdup (mail): NULL;
}

void Document::SetComment (const gchar* comment)
{
	g_free (m_comment);
	m_comment = (comment && *comment)? g_strdup (comment): NULL;
}
	
void Document::AddObject (Object* pObject)
{
	if (!pObject->GetParent ())
		AddChild (pObject);
	m_pView->AddObject (pObject);
	if (m_bIsLoading || m_bUndoRedo)
		return;
	if (!m_pCurOp) {
		m_pCurOp = new AddOperation (this, ++m_OpID);
		m_pCurOp->AddObject (pObject);
	}
}

void Document::OnUndo ()
{
	if (m_pApp->GetActiveTool ()->OnUndo ())
		return;
	m_bUndoRedo = true;
	if (!m_UndoList.empty ()) {
		Operation* Op = m_UndoList.front ();
		Op->Undo ();
		m_UndoList.pop_front ();
		m_RedoList.push_front (Op);
		if (m_Window)
			m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Redo", true);
	}
	if (m_Window) {
		if (m_UndoList.empty ())
			m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Undo", false);
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", HasChildren ());
	}
	m_bUndoRedo = false;
	Update ();
	EmptyTranslationTable ();
	SetDirty (m_LastStackSize != m_UndoList.size () || (m_LastStackSize > 0 && m_OpID != m_UndoList.front ()->GetID ()));
	m_Empty = !HasChildren ();
}

void Document::OnRedo ()
{
	if (m_pApp->GetActiveTool ()->OnRedo ())
		return;
	m_bUndoRedo = true;
	if (!m_RedoList.empty ()) {
		Operation* Op = m_RedoList.front ();
		Op->Redo ();
		m_RedoList.pop_front ();
		m_UndoList.push_front (Op);
		if (m_Window)
			m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Undo", true);
	}
	if (m_Window) {
		if (m_RedoList.empty ())
			m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Redo", false);
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", HasChildren ());
	}
	m_bUndoRedo = false;
	EmptyTranslationTable ();
	SetDirty (m_LastStackSize != m_UndoList.size () || (m_LastStackSize > 0 && m_OpID != m_UndoList.front ()->GetID ()));
	m_Empty = !HasChildren ();
}

void Document::FinishOperation ()
{
	if (!m_pCurOp)
		return;//FIXME: should at least emit a warning
	m_UndoList.push_front (m_pCurOp);
	while (!m_RedoList.empty ()) {
		delete m_RedoList.front ();
		m_RedoList.pop_front ();
	}
	m_pCurOp = NULL;
	SetDirty (true);
	m_Empty = !HasChildren ();
	if (m_Window) {
		m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Undo", true);
		m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Redo", false);
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", HasChildren ());
	}
	Update ();
}

void Document::AbortOperation()
{ 
	if (m_pCurOp)
		delete m_pCurOp;
	m_pCurOp = NULL;
} 

void Document::PopOperation ()
{
	if (!m_UndoList.empty ()) {
		delete m_UndoList.front ();
		m_UndoList.pop_front ();
		if (m_UndoList.empty () && m_Window)
			m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Undo", false);
	}
	SetDirty (m_LastStackSize != m_UndoList.size () || (m_LastStackSize > 0 && m_OpID != m_UndoList.front ()->GetID ()));
}

void Document::PushOperation (Operation* operation, bool undo)
{
	if (!m_pCurOp || (operation != m_pCurOp)) {
		cerr << "Warning: Incorrect operation" << endl;
		return;
	}
	if (undo)
		FinishOperation ();
	else
	{
		while (!m_RedoList.empty ()) {
			delete m_RedoList.front ();
			m_RedoList.pop_front ();
		}
		m_RedoList.push_front (operation);
		m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Redo", true);
	}
	m_pCurOp = NULL;
}

void Document::SetActive ()
{
	if (m_Window) {
		m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Undo", !m_UndoList.empty ());
		m_Window->ActivateActionWidget ("/MainMenu/EditMenu/Redo", !m_RedoList.empty ());
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", HasChildren ());
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/Save", !m_bReadOnly);
		m_Window->ActivateActionWidget ("/MainToolbar/Save", !m_bReadOnly);
	}
}

extern xmlDocPtr pXmlDoc;

void Document::LoadObjects (xmlNodePtr node)
{
	xmlNodePtr child = node->children, child1;
	string str;
	while (child) {
		//Add everything except bonds
		if (!strcmp ((const char*)child->name, "atom")) {
			Atom* pAtom = new Atom ();
			AddChild (pAtom);
			pAtom->Load (child);
			AddAtom (pAtom);
		} else if (!strcmp ((const char*) child->name, "fragment")) {
			Fragment* pFragment = new Fragment ();
			AddChild (pFragment);
			pFragment->Load (child);
			AddFragment (pFragment);
		} else if (!strcmp ((const char*) child->name, "bond"));
		else {
			m_bIsLoading = true;
			child1 = (strcmp ((const char*) child->name, "object"))? child: child->children;
			str = (const char*) child1->name;
			Object* pObject = Object::CreateObject (str, this);
			pObject->Load (child1);
			AddObject (pObject);
			m_pView->Update (pObject);//FIXME: should not be necessary, but solve problem with cyclic double bonds
			m_bIsLoading = false;
		}
		child = child->next;
	}
	//Now, add bonds
	child = GetNodeByName (node, "bond");
	while (child) {
		Bond* pBond = new Bond ();
		AddChild (pBond);
		if (pBond->Load (child))
			AddBond (pBond);
		else delete pBond;
		child = GetNextNodeByName (child->next, "bond");
	}
}

Operation* Document::GetNewOperation (OperationType type)
{
	m_OpID++;
	switch (type) {
		case GCP_ADD_OPERATION:
			return m_pCurOp = new AddOperation (this, m_OpID);
		case GCP_DELETE_OPERATION:
			return m_pCurOp = new DeleteOperation (this, m_OpID);
		case GCP_MODIFY_OPERATION:
			return m_pCurOp = new ModifyOperation (this, m_OpID);
		default:
			return NULL;
	}
}

void Document::AddData (xmlNodePtr node)
{
	xmlNodePtr child;
	string str;
	Object* pObject;
	m_bIsLoading = true;
	EmptyTranslationTable ();
	GtkWidget* w = m_pView->GetWidget ();
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	while (node) {
		child = (strcmp ((const char*) node->name, "object"))? node: node->children;
		str = (const char*) child->name;
		pObject = Object::CreateObject (str, this);
		AddObject (pObject);
		if (!pObject->Load (child))
			Remove (pObject);
		else {
			m_pView->Update (pObject);//FIXME: should not be necessary, but solve problem with cyclic double bonds
			pData->SetSelected (pObject);
		}
		node = node->next;
	}
	m_bIsLoading = false;
	EmptyTranslationTable ();
	FinishOperation ();
}

void Document::ExportImage (string const &filename, const char* type, int resolution)
{
	m_pView->ExportImage (filename, type, resolution);
}

void Document::SetReadOnly (bool ro)
{
	m_bReadOnly = ro;
	if (!ro && (m_FileType != "application/x-gchempaint")) {
		OBFormat *f = OBConversion::FormatFromMIME (m_FileType.c_str ());
		m_bReadOnly = (f == NULL)? true: f->Flags () & NOTWRITABLE;
	}
	m_bUndoRedo = true;
	if (m_Window) {
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/Save", !m_bReadOnly);
		m_Window->ActivateActionWidget ("/MainToolbar/Save", !m_bReadOnly);
	}
}

double Document::GetYAlign ()
{
	if (GetChildrenNumber () == 1) {
		map<string, Object*>::iterator i;
		Object *Child = GetFirstChild (i);
		return Child->GetYAlign ();
	} else {
		WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (GetWidget ()), "data");
		ArtDRect rect;
		pData->GetObjectBounds (this, &rect);
		return (rect.y1 - rect.y0) / 2.;
	}
}

void Document::SetTheme (Theme *theme)
{
	if (!theme)
		return; // or use the default theme?
	if (m_Theme)
		m_Theme->RemoveClient (this);
	m_Theme = theme;
	m_Theme->AddClient (this);
	m_BondLength = theme->GetBondLength ();
	m_BondAngle = theme->GetBondAngle ();
	m_ArrowLength = theme->GetArrowLength ();
	m_TextFontFamily = theme->GetTextFontFamily ();
	m_TextFontStyle = theme->GetTextFontStyle ();
	m_TextFontWeight = theme->GetTextFontWeight ();
	m_TextFontVariant = theme->GetTextFontVariant ();
	m_TextFontStretch = theme->GetTextFontStretch ();
	m_TextFontSize = theme->GetTextFontSize ();
	pango_attr_list_unref (m_PangoAttrList);
	m_PangoAttrList = pango_attr_list_new ();
	pango_attr_list_insert (m_PangoAttrList, pango_attr_family_new (theme->GetFontFamily ()));
	pango_attr_list_insert (m_PangoAttrList, pango_attr_style_new (theme->GetFontStyle ()));
	pango_attr_list_insert (m_PangoAttrList, pango_attr_weight_new (theme->GetFontWeight ()));
	pango_attr_list_insert (m_PangoAttrList, pango_attr_stretch_new (theme->GetFontStretch ()));
	pango_attr_list_insert (m_PangoAttrList, pango_attr_variant_new (theme->GetFontVariant ()));
	if (m_pView)
		m_pView->UpdateTheme ();
}

bool Document::OnSignal (SignalId Signal, Object *Child)
{
	if (Signal == OnThemeChangedSignal) {
		m_BondLength = m_Theme->GetBondLength ();
		m_BondAngle = m_Theme->GetBondAngle ();
		m_ArrowLength = m_Theme->GetArrowLength ();
		m_TextFontFamily = m_Theme->GetTextFontFamily ();
		m_TextFontStyle = m_Theme->GetTextFontStyle ();
		m_TextFontWeight = m_Theme->GetTextFontWeight ();
		m_TextFontVariant = m_Theme->GetTextFontVariant ();
		m_TextFontStretch = m_Theme->GetTextFontStretch ();
		m_TextFontSize = m_Theme->GetTextFontSize ();
		pango_attr_list_unref (m_PangoAttrList);
		m_PangoAttrList = pango_attr_list_new ();
		pango_attr_list_insert (m_PangoAttrList, pango_attr_family_new (m_Theme->GetFontFamily ()));
		pango_attr_list_insert (m_PangoAttrList, pango_attr_style_new (m_Theme->GetFontStyle ()));
		pango_attr_list_insert (m_PangoAttrList, pango_attr_weight_new (m_Theme->GetFontWeight ()));
		pango_attr_list_insert (m_PangoAttrList, pango_attr_stretch_new (m_Theme->GetFontStretch ()));
		pango_attr_list_insert (m_PangoAttrList, pango_attr_variant_new (m_Theme->GetFontVariant ()));
		m_pView->UpdateTheme ();
	}
	return false;
}

void Document::SetDirty (bool isDirty)
{
	if (!m_Window)
		return;
	char *title = g_strdup_printf ((isDirty? "*%s": "%s"), GetTitle ());
	m_Window->SetTitle (title);
	g_free (title);
	gcu::Document::SetDirty (isDirty);
}

void Document::SetLabel(const gchar* label)
{
	m_label = g_strdup (label);
	m_Window->SetTitle (label);
}

void Document::OnThemeNamesChanged ()
{
	DocPropDlg *dlg = dynamic_cast <DocPropDlg *> (GetDialog ("properties"));
	if (dlg)
		dlg->OnThemeNamesChanged ();
}

double Document::GetMedianBondLength ()
{
	vector <double> lengths;
	int max = 128, nb = 0;
	lengths.reserve (max);
	double result = 0.;
	stack<map< string, Object * >::iterator> iters;
	map< string, Object * >::iterator m;
	Object *Cur = this, *Ob = GetFirstChild (m);
	while (Ob) {
		if (Ob->GetType () == gcu::BondType) {
			if (nb == max) {
				max += 128;
				lengths.resize (max);
			}
			lengths[nb++] = static_cast <Bond *> (Ob)->Get2DLength ();
		} else if (Ob->HasChildren ()) {
			Cur = Ob;
			iters.push (m);
			Ob = Cur->GetFirstChild (m);
			continue;
		}
		Ob = Cur->GetNextChild (m);
		while (!Ob && !iters.empty ()) {
			m = iters.top ();
			iters.pop ();
			Cur = Cur->GetParent ();
			Ob = Cur->GetNextChild (m);
		}
	}
	if (nb > 0)
		go_range_median_inter_nonconst (lengths.data (), nb, &result);
	return result;
}

bool Document::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_DOC_TITLE:
		SetTitle (value);
		if (m_Window)
			m_Window->SetTitle (GetTitle ());
		break;
	case GCU_PROP_DOC_COMMENT:
		g_free (m_comment);
		m_comment = g_strdup (value);
		break;
	case GCU_PROP_DOC_CREATOR:
		g_free (m_author);
		m_author = g_strdup (value);
		break;
	case GCU_PROP_DOC_CREATION_TIME:
		g_date_set_parse (&CreationDate, value);
		break;
	case GCU_PROP_DOC_MODIFICATION_TIME:
		g_date_set_parse (&RevisionDate, value);
		break;
	case GCU_PROP_THEME_BOND_LENGTH: {
		char *end;
		double length = strtod (value, &end);
		if (*end != 0)
			return false;
		m_Scale = m_Theme->GetBondLength () / length; 
	}
	}
	return true;
}


}	//	namespace gcp
