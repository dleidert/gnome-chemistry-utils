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

extern xmlDocPtr pXmlDoc;

using namespace gcu;
extern GtkTargetEntry const text_targets[];

gcpFragmentTool::gcpFragmentTool (gcp::Application *App): gcpTextTool (App, "Fragment")
{
	m_ImContext = gtk_im_multicontext_new ();
	g_signal_connect (G_OBJECT (m_ImContext), "commit",
		G_CALLBACK (OnCommit), this);
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
	SetSize (m_pView->GetFontHeight ());
	if (!m_pObject) {
		gcp::Theme *pTheme = pDoc->GetTheme ();
		m_Fragment = new gcp::Fragment (m_x0 / pTheme->GetZoomFactor (), m_y0 / pTheme->GetZoomFactor ());
		pDoc->AddFragment (m_Fragment);
		pDoc->AbortOperation ();
		pDoc->EmptyTranslationTable ();
		m_pObject = m_Fragment;
	} else {
		m_Fragment = NULL;
	}
/*	gccv::TextSelBounds bounds;
	bool need_update = false;*/
	if (m_pObject) {
		switch (m_pObject->GetType ()) {
/*			case AtomType: {
				gcp::Atom* pAtom = (gcp::Atom*) m_pObject;
				if (pAtom->GetTotalBondsNumber () > 1)
					return false;
				double x, y;
				pAtom->GetCoords (&x, &y);
				gcp::Molecule *pMol = (gcp::Molecule*) pAtom->GetMolecule ();
				map<Atom*, Bond*>::iterator i;
				gcp::Bond *pBond = (gcp::Bond*) pAtom->GetFirstBond (i);
				pFragment = new gcp::Fragment (x, y);
				gcp::Atom* pFragAtom = (gcp::Atom*) pFragment->GetAtom ();
				map<string, Object*>::iterator ie;
				Object* electron = pAtom->GetFirstChild (ie);
				while (electron) {
					m_pView->Remove (electron);
					delete electron;
					electron = pAtom->GetNextChild (ie);
				}
				pMol->Remove (pAtom);
				pAtom->SetParent (NULL);
				pMol->AddFragment (pFragment);
				pDoc->AddFragment (pFragment);
				pDoc->AbortOperation ();
				gcp::Operation* pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
				pOp->AddObject (pAtom, 0);
				if (pBond)
					pOp->AddObject (pBond, 0);
				m_pView->Remove (pAtom);
				pFragAtom->SetZ (pAtom->GetZ ());
				pFragAtom->SetId ((gchar*) pAtom->GetId ());
				int n = pAtom->GetAttachedHydrogens ();
				if (n) {
					char* buf;
					if (n > 1)
						buf = g_strdup_printf ("H%d", n);
					else
						buf = g_strdup ("H");
					bounds.start = bounds.cur = ((pAtom->GetBestSide ())? strlen (pAtom->GetSymbol ()): 0);
					pFragment->OnSelChanged (&bounds);
					gcp_pango_layout_replace_text (pFragment->GetLayout (),
						bounds.cur,
						0, buf, pDoc->GetPangoAttrList ());
					bounds.cur +=  strlen (buf);
					need_update = true;
					g_free (buf);
				}
				delete pAtom;
				if (pBond) {
					pBond->ReplaceAtom (pAtom, pFragAtom);
					pFragAtom->AddBond (pBond);
					pOp->AddObject (pBond, 1);
				}
				pOp->AddObject (pFragment, 1);
				pDoc->FinishOperation ();
				pDoc->EmptyTranslationTable ();
				m_pObject = pFragment;
				break;
			}*/
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
		m_Active = m_Fragment->GetTextItem ();
/*		if (need_update) {
			m_Active->SetSelectionBounds (m_Active,  cur,  cur);
			pFragment->AnalContent (start, cur);
			pFragment->OnChanged (false);
		}*/
		m_pView->SetTextActive (m_Active);
		m_Active->SetEditing (true);
		m_Active->OnButtonPressed (m_x0, m_y0);
		m_CurNode = m_Fragment->SaveSelected ();
		m_InitNode = m_Fragment->SaveSelected ();
		pDoc->GetWindow ()->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", false);
		m_Fragment->SetEditor (this);
	}
	BuildTagsList ();
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
					BuildTagsList ();
					break;
				case GDK_underscore:
					// enter/quit subscript (not stoichiometric) mode
					m_CurMode = (m_CurMode == gcp::Fragment::SubscriptMode)? gcp::Fragment::AutoMode: gcp::Fragment::SubscriptMode;
					m_Fragment->SetMode (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_dead_circumflex:
					// enter/quit superscript (not charge) mode
					m_CurMode = (m_CurMode == gcp::Fragment::SuperscriptMode)? gcp::Fragment::AutoMode: gcp::Fragment::SuperscriptMode;
					m_Fragment->SetMode (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_n:
					// enter/quit stoichiometry mode
					m_CurMode = (m_CurMode == gcp::Fragment::StoichiometryMode)? gcp::Fragment::AutoMode: gcp::Fragment::StoichiometryMode;
					m_Fragment->SetMode (m_CurMode);
					BuildTagsList ();
					break;
				case GDK_space:
					// back to auto mode
					m_CurMode = gcp::Fragment::AutoMode;
					m_Fragment->SetMode (m_CurMode);
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
		if (gtk_im_context_filter_keypress (m_ImContext, event))
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
* This method does nothing and always return false because pasting something inside
* a fragment is somewhat unsafe and will not be implemented in a foreseable future.
*/
bool gcpFragmentTool::OnReceive (G_GNUC_UNUSED GtkClipboard *clipboard, G_GNUC_UNUSED GtkSelectionData *data, G_GNUC_UNUSED int type)
{
	return false;
}

bool gcpFragmentTool::Unselect ()
{
	if (!m_Active)
		return true;
	gcp::Fragment *fragment = dynamic_cast <gcp::Fragment*> (m_Active->GetClient ());;
	if (fragment->Validate ())
		return gcpTextTool::Unselect ();
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
	gcp::cleared = false;
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
		virtual void UpdateTagsList ();
	start = n;
	}
	tool->m_Active->ReplaceText (s, start, end - start);
}

void gcpFragmentTool::UpdateTagsList () {
	// FIXME: write this
}

