// -*- C++ -*-

/* 
 * GChemPaint text plugin
 * fragmenttool.cc 
 *
 * Copyright (C) 2003-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "fragmenttool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/molecule.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcp/window.h>
#include <gccv/structs.h>
#include <gccv/text.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>

extern xmlDocPtr pXmlDoc;

using namespace gcu;
extern GtkTargetEntry const text_targets[];

gcpFragmentTool::gcpFragmentTool (gcp::Application *App): gcpTextTool (App, "Fragment")
{
	m_ImContext = gtk_im_multicontext_new ();
	g_signal_connect (G_OBJECT (m_ImContext), "commit",
		G_CALLBACK (OnCommit), this);
	m_OwnStatus = true;
}

gcpFragmentTool::~gcpFragmentTool ()
{
	if (gcp::ClipboardData) {
		xmlFree (gcp::ClipboardData);
		gcp::ClipboardData = NULL;
	}
}

static void on_get_data (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, gcpFragmentTool* tool)
{
	tool->OnGetData (clipboard, selection_data, info);
}

bool gcpFragmentTool::OnClicked ()
{
	if (m_Active && ((m_pObject == NULL) || (m_pObject->GetType() != FragmentType) ||
			(static_cast <gccv::Item *> (m_Active) != static_cast <gcp::Fragment *> (m_pObject)->GetTextItem ()))) {
		if (!Unselect ())
			return false;
	}
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	if (!m_pObject) {
		m_Fragment = new gcp::Fragment (m_x0 / pTheme->GetZoomFactor (), m_y0 / pTheme->GetZoomFactor ());
		pDoc->AddFragment (m_Fragment);
		pDoc->AbortOperation ();
		pDoc->EmptyTranslationTable ();
		m_pObject = m_Fragment;
	} else {
		m_Fragment = NULL;
	}
	unsigned start, end;
	if (m_pObject) {
		switch (m_pObject->GetType ()) {
			case AtomType: {
				gcp::Atom* pAtom = (gcp::Atom*) m_pObject;
				if (pAtom->GetTotalBondsNumber () > 1)
					return false;
				double x, y;
				pAtom->GetCoords (&x, &y);
				gcp::Molecule *pMol = (gcp::Molecule*) pAtom->GetMolecule ();
				map<Atom*, Bond*>::iterator i;
				gcp::Bond *pBond = (gcp::Bond*) pAtom->GetFirstBond (i);
				m_Fragment = new gcp::Fragment (x, y);
				gcp::Atom* pFragAtom = (gcp::Atom*) m_Fragment->GetAtom ();
				map<string, Object*>::iterator ie;
				Object* electron = pAtom->GetFirstChild (ie);
				while (electron) {
					m_pView->Remove (electron);
					delete electron;
					electron = pAtom->GetNextChild (ie);
				}
				pMol->Remove (pAtom);
				pAtom->SetParent (NULL);
				pMol->AddFragment (m_Fragment);
				pDoc->AddFragment (m_Fragment);
				pDoc->AbortOperation ();
				gcp::Operation* pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
				pOp->AddObject (pAtom, 0);
				if (pBond)
					pOp->AddObject (pBond, 0);
				m_pView->Remove (pAtom);
				pFragAtom->SetZ (pAtom->GetZ ());
				pFragAtom->SetId ((gchar*) pAtom->GetId ());
				m_Fragment->OnChanged (false);
				int n = pAtom->GetAttachedHydrogens ();
				if (n) {
					ostringstream stream;
					stream << "H";
					if (n > 1)
						stream << n;
					string buf = stream.str ();
					start = ((pAtom->GetBestSide ())? strlen (pAtom->GetSymbol ()): 0);
					m_Fragment->GetTextItem ()->SetSelectionBounds (start, start);
					m_Fragment->GetTextItem ()->ReplaceText (buf, start, 0);
					end = start + buf.length ();
					m_Fragment->OnChanged (false);
					m_Fragment->AnalContent (start, end);
					m_Fragment->GetTextItem ()->SetSelectionBounds (start, end);
				}
				delete pAtom;
				if (pBond) {
					pBond->ReplaceAtom (pAtom, pFragAtom);
					pFragAtom->AddBond (pBond);
					pOp->AddObject (pBond, 1);
					pBond->SetDirty (); // force redraw
				}
				pOp->AddObject (m_Fragment, 1);
				pDoc->FinishOperation ();
				pDoc->EmptyTranslationTable ();
				m_pObject = m_Fragment;
				break;
			}
			case BondType:
				return false;
			case FragmentType:
				break;
			default:
				return false;
		}
		if (!m_Fragment)
			m_Fragment = static_cast <gcp::Fragment *> (m_pObject);
		m_Fragment->SetSelected (gcp::SelStateUpdating);
		m_Fragment->SetMode (m_CurMode = gcp::Fragment::AutoMode);
		m_Active = m_Fragment->GetTextItem ();
		m_pView->SetTextActive (m_Active);
		m_Active->SetEditing (true);
		m_Active->OnButtonPressed (m_x0, m_y0);
		m_CurNode = m_Fragment->SaveSelected ();
		m_InitNode = m_Fragment->SaveSelected ();
		pDoc->GetWindow ()->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", false);
		m_Fragment->SetEditor (this);
	}
	BuildTagsList ();
	SetStatusText (gcp::Fragment::AutoMode);
	SetSize (pTheme->GetFontSize () / PANGO_SCALE);
	return true;
}

bool gcpFragmentTool::Deactivate ()
{
	if (m_Active && !Unselect ())
		return false;
	return true;
}

void gcpFragmentTool::Activate ()
{
}

bool gcpFragmentTool::OnKeyPress (GdkEventKey *event)
{
	if (m_Active) {
		if (event->state & GDK_CONTROL_MASK) {
			switch(event->keyval) {
				case GDK_Right:
				case GDK_Left:
				case GDK_Up:
				case GDK_Down:
				case GDK_End:
				case GDK_Home:
				case GDK_Delete:
				case GDK_KP_Delete:
				case GDK_BackSpace:
					break;
				case GDK_KP_Add:
				case GDK_plus:
					// enter/quit charge mode
					m_CurMode = (m_CurMode == gcp::Fragment::ChargeMode)? gcp::Fragment::AutoMode: gcp::Fragment::ChargeMode;
					m_Fragment->SetMode (m_CurMode);
					SetStatusText (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_underscore:
					// enter/quit subscript (not stoichiometric) mode
					m_CurMode = (m_CurMode == gcp::Fragment::SubscriptMode)? gcp::Fragment::AutoMode: gcp::Fragment::SubscriptMode;
					m_Fragment->SetMode (m_CurMode);
					SetStatusText (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_dead_circumflex:
				case GDK_asciicircum:
					// enter/quit superscript (not charge) mode
					m_CurMode = (m_CurMode == gcp::Fragment::SuperscriptMode)? gcp::Fragment::AutoMode: gcp::Fragment::SuperscriptMode;
					m_Fragment->SetMode (m_CurMode);
					SetStatusText (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_n:
					// enter/quit stoichiometry mode
					m_CurMode = (m_CurMode == gcp::Fragment::StoichiometryMode)? gcp::Fragment::AutoMode: gcp::Fragment::StoichiometryMode;
					m_Fragment->SetMode (m_CurMode);
					SetStatusText (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_space:
					// back to auto mode
					m_CurMode = gcp::Fragment::AutoMode;
					m_Fragment->SetMode (m_CurMode);
					SetStatusText (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_equal:
					// enter/quit normal mode
					m_CurMode = (m_CurMode == gcp::Fragment::NormalMode)? gcp::Fragment::AutoMode: gcp::Fragment::NormalMode;
					m_Fragment->SetMode (m_CurMode);
					SetStatusText (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_z:
					m_pView->GetDoc ()->OnUndo ();
					return true;
				case GDK_Z:
					m_pView->GetDoc ()->OnRedo ();
					return true;
				case GDK_c:
					CopySelection (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
					return true;
				case GDK_v:
					PasteSelection (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
					return true;
				case GDK_x:
					CutSelection (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
					return true;
				default:
					break;
			}
		}
		if (event->keyval == GDK_KP_Enter || event->keyval == GDK_Return ||
							event->keyval == GDK_space) // not allowed in fragments
			return true;
		m_Active->OnKeyPressed (event);
		return true;
	}
	return false;
}

bool gcpFragmentTool::CopySelection (GtkClipboard *clipboard)
{
	if (!m_Active)
		return false;
	unsigned start, end;
	gcp::Fragment *fragment = dynamic_cast <gcp::Fragment *> (m_Active->GetClient ());
	fragment->GetSelectionBounds (start, end);
	if (start == end)
		return false;
	m_pData->Copy (clipboard); //To clean the xmlDoc
	xmlDocPtr pDoc = m_pData->GetXmlDoc (clipboard);
	if (!pDoc)
		return false;
	pDoc->children = xmlNewDocNode (pDoc, NULL, (xmlChar*) "chemistry", NULL);
	xmlNsPtr ns = xmlNewNs (pDoc->children, (xmlChar*) "http://www.nongnu.org/gchempaint", (xmlChar*) "gcp");
	xmlSetNs (pDoc->children, ns);
	xmlNodePtr node = fragment->SaveSelection (pDoc);
	if (node)
		xmlAddChild (pDoc->children, node);
	else
		return false;
	gtk_clipboard_set_with_data (clipboard, text_targets, 3,
				(GtkClipboardGetFunc) on_get_data,
				(GtkClipboardClearFunc) gcp::on_clear_data, this);
	gtk_clipboard_request_contents (clipboard,
			gdk_atom_intern ("TARGETS", FALSE),
			(GtkClipboardReceivedFunc) gcp::on_receive_targets,
			m_pApp);
	return true;
}

bool gcpFragmentTool::CutSelection (GtkClipboard *clipboard)
{
	if (!CopySelection (clipboard))
		return false;
	return DeleteSelection ();
}

/**
* Accept only unformated text since pasting something else inside
* a fragment is somewhat unsafe and will not be implemented in a foreseable future.
*/
bool gcpFragmentTool::OnReceive (GtkClipboard *clipboard, GtkSelectionData *selection_data, G_GNUC_UNUSED int type)
{
	if (!m_Active)
		return false;
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &gcp::ClipboardDataType: &gcp::ClipboardDataType1;
	g_return_val_if_fail ((gtk_selection_data_get_target (selection_data) == gdk_atom_intern (gcp::targets[*DataType].target, FALSE)), FALSE);
	int length = gtk_selection_data_get_length (selection_data);
	char const *data = reinterpret_cast <char const *> (gtk_selection_data_get_data (selection_data));
	gcp::Fragment *fragment = dynamic_cast <gcp::Fragment*> (m_Active->GetClient ());
	unsigned start, end;
	fragment->GetSelectionBounds (start, end);
	switch (*DataType) {
		case gcp::GCP_CLIPBOARD_UTF8_STRING: {
			string s (data);
			m_Active->ReplaceText (s, static_cast <int> (start), start - end);
			break;
		}
		case gcp::GCP_CLIPBOARD_STRING: {
			if (!g_utf8_validate (data, length, NULL)) {
				gsize r, w;
				char* newstr = g_locale_to_utf8 (data, length, &r, &w, NULL);
				string s (newstr);
				m_Active->ReplaceText (s, static_cast <int> (start), start - end);
				g_free (newstr);
			} else {
				string s (data);
				m_Active->ReplaceText (s, static_cast <int> (start), start - end);
			}
			break;
		}
	}
	fragment->OnChanged (true);
	return true;
}

bool gcpFragmentTool::Unselect ()
{
	if (!m_Active)
		return true;
	gcp::Fragment *fragment = dynamic_cast <gcp::Fragment*> (m_Active->GetClient ());
	if (fragment->Validate ()) {
		bool result = gcpTextTool::Unselect ();
		if (result)
			m_pApp->ClearStatus ();
		return result;
	}
	return false;
}

void gcpFragmentTool::OnGetData (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info)
{
	xmlDocPtr pDoc = gcp::WidgetData::GetXmlDoc (clipboard);
	if (gcp::ClipboardData) {
		xmlFree (gcp::ClipboardData);
		gcp::ClipboardData = NULL;
	} 
	gcp::ClipboardDataType = info;
	gint size;
	if (info) {
		gcp::ClipboardData = xmlNodeGetContent (pDoc->children->children);
		size = strlen ((char*) gcp::ClipboardData);
		gtk_selection_data_set_text (selection_data, (const gchar*) gcp::ClipboardData, size);
	} else {
		xmlDocDumpFormatMemory (pDoc, &gcp::ClipboardData, &size, info);
		gtk_selection_data_set (selection_data, gdk_atom_intern (GCHEMPAINT_ATOM_NAME, FALSE), 8,  (const guchar*) gcp::ClipboardData, size);
	}
	if (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))
		m_pApp->ActivateWindowsActionWidget ("/MainMenu/EditMenu/Paste", true);
}

void gcpFragmentTool::BuildTagsList ()
{
	if (!m_Active)
		return;
	gccv::TextTagList *l = new gccv::TextTagList ();
	switch (m_CurMode) {
	case gcp::Fragment::AutoMode:
	case gcp::Fragment::NormalMode:
		break;
	case gcp::Fragment::SubscriptMode:
		l->push_back (new gccv::PositionTextTag (gccv::Subscript, GetSize (), false));
		break;
	case gcp::Fragment::SuperscriptMode:
		l->push_back (new gccv::PositionTextTag (gccv::Superscript, GetSize (), false));
		break;
	case gcp::Fragment::ChargeMode:
		l->push_back (new gcp::ChargeTextTag (GetSize ()));
		break;
	case gcp::Fragment::StoichiometryMode:
		l->push_back (new gcp::StoichiometryTextTag (GetSize ()));
		break;
	}
	m_Active->SetCurTagList (l);
	if (m_pView)
		gtk_window_present (m_pView->GetDoc ()->GetWindow ()->GetWindow ());
}

void gcpFragmentTool::OnCommit (G_GNUC_UNUSED GtkIMContext *context, const gchar *str, gcpFragmentTool *tool)
{
	std::string s = (!strcmp (str, "-") && (tool->m_CurMode == gcp::Fragment::AutoMode || tool->m_CurMode == gcp::Fragment::ChargeMode))? "−": str;
	unsigned start, end;
	tool->m_Active->GetSelectionBounds (start, end);
	if (start > end) {
		unsigned n = end;
		end = start;
		start = n;
	}
	tool->m_Active->ReplaceText (s, start, end - start);
}

void gcpFragmentTool::UpdateTagsList () {
	if (m_Active) {
		list <gccv::TextTag *> const *cur = m_Active->GetTags ();
		unsigned start, end;
		m_Active->GetSelectionBounds (start, end);
		list <gccv::TextTag *>::const_iterator i, iend = cur->end ();
		for (i = cur->begin (); i != iend; i++)
			if ((*i)->GetStartIndex () < end && (*i)->GetEndIndex () >= end) {
				gccv::Tag tag = (*i)->GetTag ();
				if (tag == gccv::Position) {
					bool stacked;
					double size;
					switch (static_cast <gccv::PositionTextTag *> (*i)->GetPosition (stacked, size)) {
					case gccv::Normalscript:
						break;
					case gccv::Subscript:
						m_CurMode = gcp::Fragment::SubscriptMode;
						SetStatusText (m_CurMode);
						break;
					case gccv::Superscript:
						m_CurMode = gcp::Fragment::SuperscriptMode;
						SetStatusText (m_CurMode);
						break;
					}
				}
			}
		m_Fragment->SetMode (m_CurMode);
		BuildTagsList ();
	}
}

void gcpFragmentTool::SetStatusText (gcp::Fragment::FragmentMode mode)
{
	string status = _("Mode: ");
	switch (mode) {
	case gcp::Fragment::AutoMode:
		status += _("auto");
		break;
	case gcp::Fragment::NormalMode:
		status += _("normal");
		break;
	case gcp::Fragment::SubscriptMode:
		status += _("subscript");
		break;
	case gcp::Fragment::SuperscriptMode:
		status += _("superscript");
		break;
	case gcp::Fragment::ChargeMode:
		status += _("charge");
		break;
	case gcp::Fragment::StoichiometryMode:
		status += _("stoichiometry");
		break;
	}
	m_pApp->SetStatusText (status.c_str ());
}
