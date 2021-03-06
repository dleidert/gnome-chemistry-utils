// -*- C++ -*-

/*
 * GChemPaint library
 * document.cc
 *
 * Copyright (C) 2001-2014 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include "atom.h"
#include "bond.h"
#include "docprop.h"
#include "document.h"
#include "fragment.h"
#include "mesomery.h"
#include "molecule.h"
#include "reaction.h"
#include "residue.h"
#include "settings.h"
#include "text.h"
#include "theme.h"
#include "tool.h"
#include "view.h"
#include "widgetdata.h"
#include "window.h"
#include <gcu/loader.h>
#include <gcu/objprops.h>
#include <gcu/xml-utils.h>
#include <glib/gi18n-lib.h>
#include <stack>
#include <cstring>
#include <iostream>
#include <sstream>
#include <libgen.h>
#include <unistd.h>

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
	m_bWriteable = true;
	m_PangoAttrList = pango_attr_list_new ();
	m_Theme = NULL;
	SetTheme (TheThemeManager.GetTheme ("Default"));
	m_pView = new View (this, !StandAlone);
	m_bIsLoading = m_bUndoRedo = false;
	CreationDate = g_date_time_new_now_utc ();
	RevisionDate = NULL;
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
	m_pCurOp = NULL;
	m_bReadOnly = false;
	SetActive ();
	m_AllowClipboard = true;
	m_SoftwareVersion = 0;
	m_UseAtomColors = false;
}

Document::~Document ()
{
	Clear ();
	if (m_pView)
		delete m_pView;
	pango_attr_list_unref (m_PangoAttrList);
	if (m_Theme)
		m_Theme->RemoveClient (this);
	if (m_App && static_cast < Application * > (m_App)->GetActiveDocument () == this)
		static_cast < Application * > (m_App)->SetActiveDocument (NULL);
	if (CreationDate)
		g_date_time_unref (CreationDate);
	if (RevisionDate)
		g_date_time_unref (RevisionDate);
}

void Document::Clear ()
{
	m_bIsLoading = true;
	if (m_pCurOp)
		delete m_pCurOp;
	m_pCurOp = NULL;
	g_free (m_filename);
	m_filename = NULL;
	SetTitle ("");
	g_free (m_label);
	m_label = NULL;
	g_free (m_author);
	m_author = NULL;
	g_free (m_mail);
	m_mail = NULL;
	SetComment ("");
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
	if (CreationDate) {
		g_date_time_unref (CreationDate);
		CreationDate = NULL;
	}
	if (RevisionDate) {
		g_date_time_unref (RevisionDate);
		RevisionDate = NULL;
	}
}

GtkWidget* Document::GetWidget () const
{
	return (m_pView)? m_pView->GetWidget (): NULL;
}


char const *Document::GetTitle () const
{
	if (gcu::Document::GetTitle ().length ())
		return gcu::Document::GetTitle ().c_str ();
	else if (m_label)
		return m_label;
	return (m_Window)? m_Window->GetDefaultTitle (): NULL;
}

char const *Document::GetLabel () const
{
	return m_label;
}

void Document::SetFileName (string const &Name, char const *mime_type)
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
	g_free (m_label);
	m_label = NULL;
	char *ext = m_filename + j + 1;
	list<string> &exts = m_pApp->GetExtensions (m_FileType);
	list<string>::iterator cur, end = exts.end ();
	for (cur = exts.begin (); cur != end; cur++)
		if (*cur == ext) {
			char *buf = g_strndup (m_filename + i, j - i);
			m_label = g_uri_unescape_string (buf, NULL);
			g_free (buf);
			break;
		}
	if (!m_label)
		m_label = g_uri_unescape_string (m_filename + i, NULL);
}

void Document::DoPrint (G_GNUC_UNUSED GtkPrintOperation *print, GtkPrintContext *context, G_GNUC_UNUSED int page) const
{
	cairo_t *cr;
	double width, height, x, y, w, h;

	// TODO: support multiple pages
	cr = gtk_print_context_get_cairo_context (context);
	width = gtk_print_context_get_width (context);
	height = gtk_print_context_get_height (context);
	// adjust position and size
	GtkWidget* widget = m_pView->GetWidget ();
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (widget), "data");
	gccv::Rect rect;
	pData->GetObjectBounds (this, &rect);
	double scale = .75; // FIXME: should we use the real screen resolution?
	x = rect.x0;
	y = rect.y0;
	w = rect.x1 - rect.x0;
	h = rect.y1 - rect.y0;
	cairo_save (cr);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_clip (cr);
	switch (GetScaleType ()) {
	case gcugtk::GCU_PRINT_SCALE_NONE:
		break;
	case gcugtk::GCU_PRINT_SCALE_FIXED:
		scale *= Printable::GetScale ();
		break;
	case gcugtk::GCU_PRINT_SCALE_AUTO:
		if (GetHorizFit () && GetVertFit ())
			scale *= min (width / w, height / h);
		else if (GetHorizFit ())
			scale *= width / w;
		else if (GetVertFit ())
			scale *= height / h;
		break;
	}
	x *= scale;
	y *= scale;
	w *= scale;
	h *= scale;
	if (GetHorizCentered ())
		x -= (width - w) / 2;
	if (GetVertCentered ())
		y -= (height - h) / 2;
	cairo_translate (cr, -x, -y);
	cairo_scale (cr, scale, scale);
	m_pView->Render (cr);
	cairo_restore (cr);
}

void Document::AddAtom (Atom* pAtom)
{
	int i = 1;
	char id[8];
	char const *Id;
	Id = pAtom->GetId ();
	if (Id == NULL) {
		id[0] = 'a';
		do
			snprintf (id+1, 7, "%d", i++);
		while (GetDescendant (id) != NULL);
		pAtom->SetId (id);
		Id = id;
	}
	if (!pAtom->GetParent ())
		AddChild (pAtom);
	if (m_pView->GetCanvas ())
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
	char const *Id;
	Id = pFragment->GetId ();
	if (Id == NULL)  {
		id[0] = 'f';
		do
			snprintf( id+1, 7, "%d", i++);
		while (GetDescendant (id) != NULL);
		pFragment->SetId (id);
		Id = id;
	}
	AddObject(pFragment);
	m_pView->AddObject (pFragment);
	if (m_bIsLoading)
		return;
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
	char const *Id;
	Id = pBond->GetId ();
	if (Id == NULL) {
		id[0] = 'b';
		do
			snprintf (id+1, 7, "%d", i++);
		while (GetDescendant(id) != NULL);
		pBond->SetId (id);
	}
	if (pBond->GetParent () == NULL)
		AddChild (pBond);
	Atom *pAtom0 = (Atom*) pBond->GetAtom (0), *pAtom1 = (Atom*) pBond->GetAtom (1);
	if (m_pView->GetCanvas () && pAtom0 && pAtom1) {
		pAtom0->UpdateItem ();
		pAtom1->UpdateItem ();
		pBond->AddItem ();
	}
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
			if (pMol1->GetParent () != this) {
				pMol1->Merge (pMol0);
				pMol0 = pMol1;
			} else
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

void Document::Save () const
{
	if (m_bReadOnly)
		const_cast <Document *> (this)->SetReadOnly (false);
	if (!m_filename || !m_bWriteable || m_bReadOnly)
		return;
	xmlDocPtr xml = NULL;
	const_cast <Document *> (this)->m_SavedResidues.clear ();

	try {
		if (m_FileType.length () && m_FileType != "application/x-gchempaint") {
			if (!m_pApp || !m_pApp->Save (m_filename, m_FileType.c_str (), this, ContentType2D))
				throw (int) -1; // FIXME: really display an error message
			return;
		}
		xml = BuildXMLTree ();
		xmlSetDocCompressMode (xml, CompressionLevel);

		if (!CompressionLevel) {
			xmlIndentTreeOutput = true;
			xmlKeepBlanksDefault (0);
		}

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
		const_cast <Document *> (this)->SetReadOnly (false);
		const_cast <Document *> (this)->SetDirty (false);
		const_cast <Document *> (this)->m_LastStackSize = m_UndoList.size ();
		if (m_UndoList.size () > 0)
			const_cast <Document *> (this)->m_OpID = m_UndoList.front ()->GetID ();
	}
	catch (int num) {
		if (xml)
			xmlFreeDoc (xml);
		xml = NULL;
//		Error (SAVE);
	}
	const_cast <Document *> (this)->m_SavedResidues.clear ();
}

bool Document::Load (xmlNodePtr root)
{
	SetTitle ("");
	if (m_author) {
		g_free(m_author);
		m_author = NULL;
	}
	if (m_mail) {
		g_free(m_mail);
		m_mail = NULL;
	}
	SetComment ("");
	xmlNodePtr node, child;
	char* tmp;
	Object* pObject;
	tmp = (char*) xmlGetProp (root, (xmlChar*) "id");
	if (tmp) {
		SetId (tmp);
		xmlFree (tmp);
	}
	tmp = reinterpret_cast < char * > (xmlGetProp (root, reinterpret_cast < xmlChar const * > ("use-atom-colors")));
	if (tmp) {
		m_UseAtomColors = !strcmp (tmp, "true");
		xmlFree (tmp);
	}

	gcu::ReadDate (root, "creation", &CreationDate);
	gcu::ReadDate (root, "revision", &RevisionDate);

	node = GetNodeByName (root, "generator");
	if (node) {
		tmp = reinterpret_cast <char*> (xmlNodeGetContent (node));
		if (tmp) {
			char software[strlen (tmp) + 1];
			unsigned Major = 0, minor = 0, micro = 0, read;
			if (4 == (read=sscanf (tmp, "%s %u.%u.%u", software, &Major, &minor, &micro)) &&
			    Major < 1000 && minor < 1000 && micro < 1000)
				m_SoftwareVersion = Major * 1000000 + minor * 1000 + micro;
			xmlFree (tmp);
		}
	}
	node = GetNodeByName (root, "title");
	if (node) {
		tmp = (char*) xmlNodeGetContent (node);
		if (tmp) {
			SetTitle (tmp);
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
			SetComment (tmp);
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
		while (pObject) {
			if (!pObject->Load (child))
				Remove (pObject);
			else
				m_pView->AddObject (pObject);
			if (child != node) {
				child = child->next;
				pObject = (child) ?CreateObject ((const char*) child->name, this): NULL;
			} else
				pObject = NULL;
		}
		node = node->next;
	}
	Loaded ();
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

xmlDocPtr Document::BuildXMLTree () const
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
	if (m_UseAtomColors)
		xmlNewProp (xml->children, reinterpret_cast < xmlChar const * > ("use-atom-colors"), reinterpret_cast < xmlChar const * > ("true"));
	if (CreationDate == NULL)
		const_cast < Document * > (this)->CreationDate = g_date_time_new_now_utc ();
	if (RevisionDate != NULL)
		const_cast < Document * > (this)->RevisionDate = g_date_time_new_now_utc ();
	gcu::WriteDate (xml->children, "creation", CreationDate);
	gcu::WriteDate (xml->children, "revision", RevisionDate);
	node = xmlNewDocNode (xml, NULL, 
	                      reinterpret_cast < xmlChar const * > ("generator"),
	                      reinterpret_cast < xmlChar const * > ("GChemPaint " VERSION));
	if (node)
		xmlAddChild (xml->children, node);
	else
		throw (int) 0;

	if (gcu::Document::GetTitle ().length ()) {
		node = xmlNewDocNode (xml, NULL, (xmlChar*) "title", (xmlChar*) gcu::Document::GetTitle ().c_str ());
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
	if (GetComment ().length () > 0) {
		node = xmlNewDocNode (xml, NULL, (xmlChar*) "comment", (xmlChar*) GetComment ().c_str ());
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
	std::set <Object *>::iterator i,  end = m_DirtyObjects.end ();
	std::set <Object *> Deleted;
	TypeId Id;
	for (i = m_DirtyObjects.begin (); i != end; i++) {
		Id = (*i)->GetType ();
		switch (Id) {
		case gcu::BondType:
			m_pView->Update ((Bond*) (*i));
			break;
		case gcu::MoleculeType:
			if ((*i)->GetChildrenNumber() == 0)
					Deleted.insert (*i);
			break;
		default:
			break;
		}
	}
	end = Deleted.end ();
	for (i = Deleted.begin (); i != end; i++)
		delete *i;
	m_DirtyObjects.clear ();
}

void Document::RemoveAtom (Atom* pAtom)
{
	std::map < gcu::Bondable *, gcu::Bond * >::iterator i;
	Bond* pBond;
	while ((pBond = (Bond*) pAtom->GetFirstBond (i)))
	{
		// save only for delete operations since otherwise, the whole group has already been saved
		if (!m_bUndoRedo && !m_bIsLoading && m_pCurOp && dynamic_cast <DeleteOperation *> (m_pCurOp))
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
	std::map < gcu::Bondable *, gcu::Bond * >::iterator i;
	Atom* pAtom = pFragment->GetAtom ();
	Bond* pBond;
	while ((pBond = (Bond*) pAtom->GetFirstBond (i))) {
		if (!m_bUndoRedo && !m_bIsLoading && m_pCurOp)
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
	if (pMol != NULL)
		pMol->Lock (true);
	pAtom0->RemoveBond (pBond);
	m_pView->Update (pAtom0);
	pAtom1->RemoveBond (pBond);
	m_pView->Update (pAtom1);
	if (pMol == NULL) {
		delete pBond;
		return;
	}
	pMol->Lock (false);
	if (pBond->IsCyclic ()) {
		pBond->RemoveAllCycles ();
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
		m_NewObjects.insert (id);
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
		m_NewObjects.insert (id);
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
					object = pObject->GetFirstChild (i);
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

void Document::SetAuthor (char const *author)
{
	g_free (m_author);
	m_author = (author && *author)? g_strdup (author): NULL;
}

void Document::SetMail (char const *mail)
{
	g_free (m_mail);
	m_mail = (mail && *mail)? g_strdup (mail): NULL;
}

void Document::AddObject (Object* pObject)
{
	if (!pObject->GetParent ())
		AddChild (pObject);
	m_pView->AddObject (pObject);
	if (m_bIsLoading || m_bUndoRedo)
		return;
}

void Document::OnUndo ()
{
	if (m_pApp->GetActiveTool ()->OnUndo ())
		return;
	m_pView->GetData ()->UnselectAll ();
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
	Loaded ();
	Update ();
	EmptyTranslationTable ();
	SetDirty (m_LastStackSize != m_UndoList.size () || (m_LastStackSize > 0 && m_OpID != m_UndoList.front ()->GetID ()));
	m_Empty = !HasChildren ();
}

void Document::OnRedo ()
{
	if (m_pApp->GetActiveTool ()->OnRedo ())
		return;
	m_pView->GetData ()->UnselectAll ();
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
	Loaded ();
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
	m_NewObjects.clear ();
	EmptyTranslationTable ();
	m_pView->EnsureSize ();
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
			Object* pObject = GetApp ()->CreateObject (str, this);
			pObject->Load (child1);
			AddObject (pObject);
			m_bIsLoading = false;
		}
		child = child->next;
	}
	//Now, add bonds
	if (!m_bUndoRedo) /* this looks unsafe but needed at least when undoing partial selection ops */
		m_bIsLoading = true;
	child = GetNodeByName (node, "bond");
	while (child) {
		Bond* pBond = new Bond ();
		AddChild (pBond);
		if (pBond->Load (child))
			AddBond (pBond);
		else
			delete pBond;
		child = GetNextNodeByName (child->next, "bond");
	}
	m_bIsLoading = false;
	Loaded ();
	// FIXME: we might just update what needs update, not the whole document
	m_pView->Update (this);
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

void Document::PasteData (xmlNodePtr node)
{
	xmlNodePtr child;
	string str;
	Object* pObject;
	m_bIsLoading = true;
	EmptyTranslationTable ();
	GtkWidget* w = m_pView->GetWidget ();
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	gcu::Application *app = GetApp ();
	if (!app)
		app = Application::GetApplication ("GChemPaint");
	if (!app)
		return; // TODO: may be an exception there
	while (node) {
		child = (strcmp ((const char*) node->name, "object"))? node: node->children;
		str = (const char*) child->name;
		pObject = app->CreateObject (str, this);
		if (pObject) {
			AddObject (pObject);
			if (!pObject->Load (child))
				Remove (pObject);
			else {
				m_pView->Update (pObject);//FIXME: should not be necessary, but solve problem with cyclic double bonds
				pData->SetSelected (pObject);
			}
		}
		node = node->next;
	}
	m_bIsLoading = false;
	Loaded ();
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
	m_bUndoRedo = true;
	if (m_Window) {
		m_Window->ActivateActionWidget ("/MainMenu/FileMenu/Save", !m_bReadOnly);
		m_Window->ActivateActionWidget ("/MainToolbar/Save", !m_bReadOnly);
	}
}

double Document::GetYAlign () const
{
	if (GetChildrenNumber () == 1) {
		map<string, Object*>::const_iterator i;
		Object const *Child = GetFirstChild (i);
		return Child->GetYAlign ();
	} else {
		WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (GetWidget ()), "data");
		gccv::Rect rect;
		pData->GetObjectBounds (this, &rect);
		return (rect.y1 - rect.y0) / 2. / m_Theme->GetZoomFactor ();
	}
}

void Document::SetTheme (Theme *theme)
{
	if (m_Theme)
		m_Theme->RemoveClient (this);
	if (!theme) {
		m_Theme = NULL;
		return;
	}
	m_Theme = theme;
	m_Theme->AddClient (this);
	m_BondLength = theme->GetBondLength ();
	m_BondAngle = theme->GetBondAngle ();
	m_ArrowLength = theme->GetArrowLength ();
	m_BracketsFontFamily = m_TextFontFamily = theme->GetTextFontFamily ();
	m_TextFontStyle = theme->GetTextFontStyle ();
	m_TextFontWeight = theme->GetTextFontWeight ();
	m_TextFontVariant = theme->GetTextFontVariant ();
	m_TextFontStretch = theme->GetTextFontStretch ();
	m_BracketsFontSize = m_TextFontSize = theme->GetTextFontSize ();
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

static void PropagateThemeChange (Object * obj)
{
	map< string, Object * >::iterator i;
	Object *Obj = obj->GetFirstChild (i);
	while (Obj) {
		Obj->ParentChanged ();
		PropagateThemeChange (Obj);
		Obj = obj->GetNextChild (i);
	}
}

bool Document::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Child)
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
		PropagateThemeChange (this);
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

void Document::SetLabel(char const *label)
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
	int failed = true;
	lengths.reserve (128);
	double result = 0.;
	stack<map< string, Object * >::iterator> iters;
	map< string, Object * >::iterator m;
	Object *Cur = this, *Ob = GetFirstChild (m);
	while (Ob) {
		if (Ob->GetType () == gcu::BondType)
			lengths.push_back (static_cast <Bond *> (Ob)->Get2DLength ());
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
	if (lengths.size () > 0)
		failed = go_range_median_inter_nonconst (&lengths[0], lengths.size (), &result);
	return (failed)? 0: result;
}

static GDateTime *convert_to_date (char const *value)
{
	unsigned Y = 0, M = 0, D = 0, h = 0, m = 0;
	float s = 0.;
	sscanf (value, "%u\%u\%u %u:%uù%f\n", &Y, &M, &D, &h, &m, &s);
	return g_date_time_new_utc (Y,M, D, h, m, s);
}

bool Document::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_DOC_FILENAME:
		SetFileName (value, m_FileType.c_str ());
		break;
	case GCU_PROP_DOC_MIMETYPE:
		m_FileType = value;
		break;
	case GCU_PROP_DOC_TITLE:
		SetTitle (value);
		if (m_Window)
			m_Window->SetTitle (GetTitle ());
		break;
	case GCU_PROP_DOC_COMMENT:
		SetComment (value);
		break;
	case GCU_PROP_DOC_CREATOR:
		g_free (m_author);
		m_author = g_strdup (value);
		break;
	case GCU_PROP_DOC_CREATION_TIME:
		if (CreationDate)
			g_date_time_unref (CreationDate);
		CreationDate = convert_to_date (value);
		break;
	case GCU_PROP_DOC_MODIFICATION_TIME:
		if (RevisionDate)
			g_date_time_unref (RevisionDate);
		RevisionDate = convert_to_date (value);
		break;
	case GCU_PROP_THEME_BOND_LENGTH: {
		char *end;
		double length = strtod (value, &end);
		if (*end != 0)
			return false;
		gcu::Document::SetScale (m_Theme->GetBondLength () / length);
		break;
	}
	case GCU_PROP_THEME_SCALE: {
		char *end;
		double length = strtod (value, &end);
		if (*end != 0)
			return false;
		gcu::Document::SetScale (1. / length);
		break;
	}
	}
	return true;
}

std::string Document::GetProperty (unsigned property) const
{
	ostringstream res;
	switch (property) {
	case GCU_PROP_DOC_FILENAME:
		res << GetFileName ();
		break;
	case GCU_PROP_DOC_MIMETYPE:
		res << m_FileType;
		break;
	case GCU_PROP_DOC_TITLE:
		res << gcu::Document::GetTitle ();
		break;
	case GCU_PROP_DOC_COMMENT:
		res << GetComment ().c_str ();
		break;
	case GCU_PROP_DOC_CREATOR:
		res << m_author;
		break;
	case GCU_PROP_DOC_CREATION_TIME: {
		char *buf = g_date_time_format (CreationDate, "%Y/%m/%d %H:%M:%S");
		res << buf;
		break;
	}
	case GCU_PROP_DOC_MODIFICATION_TIME: {
		char buf[16];
		*buf = 0;
		g_date_time_format (RevisionDate, "%Y/%m/%d %H:%M:%S");
		res << buf;
		break;
	}
	case GCU_PROP_THEME_BOND_LENGTH: {
		res << m_Theme->GetBondLength ();
		break;
	default:
		return gcu::Document::GetProperty (property);
	}
	}
	return res.str ();
}

GtkWindow *Document::GetGtkWindow ()
{
	return (m_Window)? m_Window->GetWindow (): NULL;
}

void Document::SaveResidue (Residue const *r, xmlNodePtr node)
{
	if (m_SavedResidues.find (r) == m_SavedResidues.end ()) {
		m_SavedResidues.insert (r);
		xmlNewProp (node, (xmlChar const *) "raw", (xmlChar const *) reinterpret_cast <Molecule const*> (r->GetMolecule ())->GetRawFormula ().c_str ());
		xmlNewProp (node, (xmlChar const *) "generic", (xmlChar const *) (r->GetGeneric ()? "true": "false"));
		map<string, bool> const &symbols = r->GetSymbols ();
		map<string, bool>::const_iterator i = symbols.begin (), iend = symbols.end ();
		string s = (*i).first;
		for (i++; i != iend; i++) {
			s += ";";
			s += (*i).first;
		}
		xmlNodePtr child = xmlNewDocNode (node->doc, NULL, (xmlChar const *) "symbols", (xmlChar const *) s.c_str ());
		xmlAddChild (node, child);
		map<string, string> const &names = r->GetNames ();
		map<string, string>::const_iterator j, jend = names.end ();
		j = names.find ("C");
		if (j != jend) {
			child = xmlNewDocNode (node->doc, NULL, (xmlChar const *) "name", (xmlChar const *) (*j).second.c_str ());
			xmlAddChild (node, child);
		}
		for (j = names.begin (); j != jend; j++) {
			if ((*j).first == "C")
				continue;
			child = xmlNewDocNode (node->doc, NULL, (xmlChar const *) "name", (xmlChar const *) (*j).second.c_str ());
			xmlNodeSetLang (child, (xmlChar const *) (*j).first.c_str ());
			xmlAddChild (node, child);
		}
		child = reinterpret_cast <Molecule const*> (r->GetMolecule ())->Save (node->doc);
		if (child) {
			xmlAddChild (node, child);
		}
	}
}

gcu::Residue const *Document::GetResidue (char const *symbol, bool *ambiguous)
{
	map<string, SymbolResidue>::iterator i = m_Residues.find (symbol);
	if (i != m_Residues.end ()) {
		if (ambiguous)
			*ambiguous = (*i).second.ambiguous;
		return (*i).second.res;
	} else
		return gcu::Document::GetResidue (symbol, ambiguous);
}

gcu::Residue *Document::CreateResidue (char const *name, char const *symbol, gcu::Molecule *molecule)
{
	Residue *res = NULL;
	Residue const *r;
	bool ambiguous;
	map< string, Object * >::iterator i;
	Object *obj = molecule->GetFirstChild (i);
	gcu::Atom *a = NULL;
	while (obj) {
		a = dynamic_cast <gcu::Atom *> (obj);
		if (a && ! a->GetZ ())
			break;
		a = NULL;
		obj = molecule->GetNextChild (i);
	}
	if (!a || a->GetBondsNumber () != 1)
		return NULL;
	if (strcmp (a->GetId (), "a1")) {
		Object *o = molecule->GetChild ("a1");
		if (o) {
			o->SetId ("at1");
			a->SetId ("a1");
			o->SetId ("a1");
		} else
			a->SetId ("a1");
	}
	double x, y;
	a->GetCoords (&x, &y);
	molecule->Move (-x, -y);
	map < gcu::Bondable *, gcu::Bond * >::iterator j;
	gcu::Bond *b = a->GetFirstBond (j);
	double angle = b->GetAngle2DRad (a);
	Matrix2D m (-angle, false);
	molecule->Transform2D (m, 0., 0.);
	// does a global residue exists for that symbol?
	r = static_cast <Residue const*> (Residue::GetResidue (symbol, &ambiguous));
	if (!r)
		res = new Residue (name, symbol, dynamic_cast <Molecule *> (molecule), NULL);
	else {
		// TODO: add residues specific to the document
	}
	return res;
}

gcu::Window *Document::GetWindow ()
{
	return m_Window;
}

}	//	namespace gcp
