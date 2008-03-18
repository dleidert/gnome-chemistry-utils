// -*- C++ -*-

/* 
 * GChemPaint text plugin
 * fragmenttool.cc 
 *
 * Copyright (C) 2003-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcp/document.h>
#include <gcp/fragment.h>
#include <gcp/molecule.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/window.h>
#include <gdk/gdkkeysyms.h>

extern xmlDocPtr pXmlDoc;

using namespace gcu;

gcpFragmentTool::gcpFragmentTool (gcp::Application *App): gcpTextTool (App, "Fragment")
{
}

gcpFragmentTool::~gcpFragmentTool ()
{
	if (gcp::ClipboardData)
		xmlFree (gcp::ClipboardData);
}

static void on_get_data (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, gcpFragmentTool* tool)
{
	tool->OnGetData (clipboard, selection_data, info);
}

bool gcpFragmentTool::OnClicked ()
{
	if (m_Active && ((m_pObject == NULL) || (m_pObject->GetType() != FragmentType) ||
			(m_Active != g_object_get_data (G_OBJECT (m_pData->Items[m_pObject]), "fragment")))) {
		if (!Unselect ())
			return false;
	}
	gcp::Document* pDoc = m_pView->GetDoc ();
	if (!m_pObject) {
		gcp::Theme *pTheme = pDoc->GetTheme ();
		gcp::Fragment *fragment = new gcp::Fragment (m_x0 / pTheme->GetZoomFactor (), m_y0 / pTheme->GetZoomFactor ());
		pDoc->AddFragment (fragment);
		pDoc->AbortOperation ();
		pDoc->EmptyTranslationTable ();
		m_pObject = fragment;
	}
	struct GnomeCanvasPangoSelBounds bounds;
	bool need_update = false;
	gcp::Fragment *pFragment = NULL;
	if (m_pObject) {
		switch(m_pObject->GetType ()) {
			case AtomType: {
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
			}
			case BondType:
				return false;
			case FragmentType:
				break;
			default:
				return false;
		}
		m_pObject->SetSelected (m_pWidget, gcp::SelStateUpdating);
		m_Active = GNOME_CANVAS_PANGO (g_object_get_data (G_OBJECT (m_pData->Items[m_pObject]), "fragment"));
		if (need_update) {
			gnome_canvas_pango_set_selection_bounds (m_Active,  bounds.cur,  bounds.cur);
			pFragment->AnalContent ((unsigned) bounds.start, (unsigned&) bounds.cur);
			pFragment->OnChanged (false);
		}
		m_pView->SetGnomeCanvasPangoActive (m_Active);
		g_object_set (G_OBJECT (m_Active), "editing", true, NULL);
		m_CurNode = ((gcp::Fragment*) m_pObject)->SaveSelected ();
		m_InitNode = ((gcp::Fragment*) m_pObject)->SaveSelected ();
		pDoc->GetWindow ()->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", false);
	}
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

bool gcpFragmentTool::OnEvent (GdkEvent* event)
{
	if (m_Active) {
		if ((event->type == GDK_KEY_PRESS) || (event->type == GDK_KEY_RELEASE)) {
			if (event->key.state & GDK_CONTROL_MASK) {
				switch(event->key.keyval) {
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
						return false;
				}
			}
			if (event->key.keyval == GDK_KP_Enter || event->key.keyval == GDK_Return ||
								event->key.keyval == GDK_space)
				return true;
			if (!g_utf8_validate (((GdkEventKey*) event)->string, -1, NULL)) {
				gsize r, w;
				gchar* newstr = g_locale_to_utf8 (((GdkEventKey*) event)->string, ((GdkEventKey*) event)->length, &r, &w, NULL);
				g_free (((GdkEventKey*) event)->string);
				((GdkEventKey*) event)->string = newstr;
				((GdkEventKey*) event)->length = w;
			}
			gnome_canvas_item_grab_focus ((GnomeCanvasItem*) m_Active);
			GnomeCanvasItemClass* klass = GNOME_CANVAS_ITEM_CLASS (G_OBJECT_GET_CLASS (m_Active));
			klass->event ((GnomeCanvasItem*) m_Active, event);
			return true;
		}
	}
	return false;
}

bool gcpFragmentTool::CopySelection (GtkClipboard *clipboard)
{
	if (!m_Active)
		return false;
	unsigned start, end;
	gcp::Fragment *fragment = (gcp::Fragment*) g_object_get_data (G_OBJECT (m_Active), "object");
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
	gtk_clipboard_set_with_data (clipboard, gcp::targets, gcp::ClipboardFormats,
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
bool gcpFragmentTool::OnReceive (GtkClipboard *clipboard, GtkSelectionData *data, int type)
{
	return false;
}

bool gcpFragmentTool::Unselect ()
{
	if (!m_Active)
		return true;
	gcp::Fragment *fragment = (gcp::Fragment*) g_object_get_data (G_OBJECT (m_Active), "object");
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
