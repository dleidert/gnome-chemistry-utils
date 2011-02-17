// -*- C++ -*-

/* 
 * GChemPaint text plugin
 * texttool.cc 
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "texttool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/fragment.h>
#include <gcp/settings.h>
#include <gcp/text.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcp/window.h>
#include <gccv/text.h>
#include <gccv/text-tag.h>
#include <gcu/ui-builder.h>
#include <gdk/gdkkeysyms.h>
#include <unistd.h>
#include <cstring>

using namespace gcu;
extern GtkTargetEntry const text_targets[];
namespace gcp {
extern xmlDocPtr pXmlDoc;
};


GtkTargetEntry const text_targets[] = {
	{(char *) GCHEMPAINT_ATOM_NAME,  0, gcp::GCP_CLIPBOARD_NATIVE},
	{(char *) "UTF8_STRING", 0, gcp::GCP_CLIPBOARD_UTF8_STRING},
	{(char *) "STRING", 0, gcp::GCP_CLIPBOARD_STRING}
};

static void on_get_data (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, gcpTextTool* tool)
{
	tool->OnGetData (clipboard, selection_data, info);
}

gcpTextTool::gcpTextTool (gcp::Application* App, string Id):
	gcp::Tool (App, Id),
	m_FamilyList (NULL)
{
	m_Active = NULL;
	m_bUndo = true;
	m_CurNode = m_InitNode = m_GroupNode = NULL;
	m_Strikethrough = gccv::TextDecorationNone;
	m_FontDesc = NULL;
	gcp::Theme *pTheme = gcp::TheThemeManager.GetTheme ("Default");
	m_FamilyName = pTheme->GetTextFontFamily ();
	m_Style = pTheme->GetFontStyle ();
	m_Weight = pTheme->GetFontWeight ();
	m_Stretch = pTheme->GetFontStretch ();
	m_Variant = pTheme->GetFontVariant ();
	m_Size = pTheme->GetTextFontSize ();
	m_Underline = gccv::TextDecorationNone;
	m_Rise = 0;
	m_Color = GO_COLOR_BLACK;
	m_Position = gccv::Normalscript;
	m_Group = NULL;
}

gcpTextTool::~gcpTextTool ()
{
	map<string, PangoFontFamily*>::iterator i, iend = m_Families.end ();
	for (i = m_Families.begin (); i != iend; i++) {
		g_object_unref ((*i).second);
	}
	map<string, PangoFontFace*>::iterator j, jend = m_Faces.end ();
	for (j = m_Faces.begin (); j != jend; j++) {
		g_object_unref ((*j).second);
	}
	m_FontDesc = NULL;
}

bool gcpTextTool::OnClicked ()
{
	if (m_Active && ((m_pObject == NULL) || (m_pObject->GetType () != TextType) ||
			(static_cast <gccv::Item *> (m_Active) != dynamic_cast <gccv::ItemClient *> (m_pObject)->GetItem ())))
		Unselect ();
	bool create = false;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	if (!m_pObject) {
		gcp::Text *text = new gcp::Text(m_x0 / pTheme->GetZoomFactor (), m_y0 / pTheme->GetZoomFactor ());
		m_pView->GetDoc ()->AddObject (text);
		m_pObject = text;
		create = true;
	}
	if (m_pObject) {
		if (m_pObject->GetType () != TextType)
			return false;
		gcp::Text *text = static_cast <gcp::Text *> (m_pObject);
		text->SetSelected (gcp::SelStateUpdating);
		m_Active = static_cast <gccv::Text *> (dynamic_cast <gccv::ItemClient *> (m_pObject)->GetItem ());
		m_pView->SetTextActive (m_Active);
		m_Active->SetEditing (true);
		m_Active->OnButtonPressed (m_x0, m_y0);
		m_CurNode = ((gcp::Text*) m_pObject)->SaveSelected ();
		m_InitNode = ((gcp::Text*) m_pObject)->SaveSelected ();
		m_pView->GetDoc ()->GetWindow ()->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", false);
		text->SetEditor (this);
		if (create)
			BuildTagsList ();
		else
			UpdateTagsList ();
		m_Group = m_pObject->GetGroup ();
		if (m_pView->GetDoc ()->GetCurrentOperation () == NULL && m_Group)
			m_GroupNode = m_Group->Save (gcp::pXmlDoc);
	}
	m_Size = pTheme->GetTextFontSize ();
	return true;
}

void gcpTextTool::OnDrag ()
{
	if (m_Active)
		m_Active->OnDrag (m_x, m_y);
}

bool gcpTextTool::OnKeyPress (GdkEventKey* event)
{
	if (m_Active) {
		if (event->state & GDK_CONTROL_MASK) {
			switch (event->keyval) {
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
			case GDK_a:
				m_pView->OnSelectAll ();
				return true;
			case GDK_z:
				m_pView->GetDoc()->OnUndo ();
				return true;
			case GDK_Z:
				m_pView->GetDoc()->OnRedo ();
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
			case GDK_i:
				m_Style = (m_Style == PANGO_STYLE_NORMAL)? PANGO_STYLE_ITALIC: PANGO_STYLE_NORMAL;
				SelectBestFontFace ();
				BuildTagsList ();
				return true;
			case GDK_u:
				gtk_combo_box_set_active (m_UnderlineBox, ((m_Underline == gccv::TextDecorationDefault)? 0: 1));
				return true;
			case GDK_b:
				m_Weight = (m_Weight == PANGO_WEIGHT_NORMAL)? PANGO_WEIGHT_BOLD: PANGO_WEIGHT_NORMAL;
				SelectBestFontFace ();
				BuildTagsList ();
				return true;
			case GDK_k:
				gtk_toggle_button_set_active (m_StrikethroughBtn, !m_Strikethrough);
				return true;
			case GDK_plus:
			case GDK_dead_circumflex:
			case GDK_KP_Add:
			case GDK_asciicircum:
				m_Position = (m_Position == gccv::Superscript)? gccv::Normalscript: gccv::Superscript;
				BuildTagsList ();
				return true;
			case GDK_equal:
			case GDK_underscore:
				m_Position = (m_Position == gccv::Subscript)? gccv::Normalscript: gccv::Subscript;
				BuildTagsList ();
				return true;
			case GDK_space: {
				gccv::Text *saved = m_Active;
				m_Active = NULL;
				UpdateTagsList ();
				m_Active = saved;
				BuildTagsList ();
				return true;
			}
			case GDK_l:
				if (m_Active)
					m_Active->SetJustification (GTK_JUSTIFY_LEFT, true);
				return true;
			case GDK_r:
				if (m_Active)
					m_Active->SetJustification (GTK_JUSTIFY_RIGHT, true);
				return true;
			case GDK_j:
				if (m_Active)
					m_Active->SetJustification (GTK_JUSTIFY_FILL, true);
				return true;
			case GDK_m:
				if (m_Active)
					m_Active->SetJustification (GTK_JUSTIFY_CENTER, true);
				return true;
			case GDK_w:
				if (m_Active) {
					double w = m_Active->GetInterline ();
					if (w == 0.)
						return true;
					w -= 1.;
					m_Active->SetInterline ((w > 0.? w: 0.), true);
				}
				return true;
			case GDK_W:
				if (m_Active) 
					m_Active->SetInterline (m_Active->GetInterline () + 1., true);
				return true;
			default:
				break;
			}
		}
		m_Active->OnKeyPressed (event);
		return true;
	}
	return false;
}

bool gcpTextTool::OnKeyRelease (GdkEventKey* event)
{
	if (m_Active)
		m_Active->OnKeyReleased (event);
	return true;
}

void gcpTextTool::Activate ()
{
	if (!m_Active)
		UpdateTagsList ();
}

bool gcpTextTool::Deactivate ()
{
	if (m_Active)
		Unselect ();
	return true;
}

bool gcpTextTool::NotifyViewChange ()
{
	return (m_Active)? Unselect (): true;
}

bool gcpTextTool::Unselect ()
{
	if (!m_Active)
		return true;
	m_pView->SetTextActive (NULL);
	m_Active->SetEditing (false);
	m_Active->GetClient ()->SetSelected (gcp::SelStateUnselected);
	Object *pObj = dynamic_cast <Object *> (m_Active->GetClient ());
	char const *text = m_Active->GetText ();
	m_Active = NULL;
	while (!m_UndoList.empty ()) {
		xmlUnlinkNode (m_UndoList.front ());
		xmlFreeNode (m_UndoList.front ());
		m_UndoList.pop_front ();
	}
	while (!m_RedoList.empty ()) {
		xmlUnlinkNode (m_RedoList.front ());
		xmlFreeNode (m_RedoList.front ());
		m_RedoList.pop_front ();
	}
	xmlBufferPtr initbuf = xmlBufferCreate ();
	xmlBufferPtr endbuf = xmlBufferCreate ();
	xmlNodeDump (initbuf, m_pApp->GetXmlDoc (), m_InitNode, 0, 0);
	xmlNodeDump (endbuf, m_pApp->GetXmlDoc (), m_CurNode, 0, 0);
	gcp::Operation *pOp = NULL;
	if (strcmp ((char*) initbuf->content, (char*) endbuf->content)) {
		if (m_Group) {
			pOp = m_pView->GetDoc ()->GetCurrentOperation ();
			if (pOp && dynamic_cast <gcp::ModifyOperation *> (pOp) == NULL) {
				m_pView->GetDoc ()->AbortOperation ();
				pOp = NULL;
			}
			if (!pOp) {
				pOp = m_pView->GetDoc ()->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
				pOp->AddNode (m_GroupNode, 0);
			}
			pOp->AddNode (m_Group->Save (gcp::pXmlDoc), 1);
			m_Group = NULL;
			m_GroupNode = NULL;
		} else {
			char* endval = (char*) xmlNodeGetContent (m_CurNode);
			char* initval = (char*) xmlNodeGetContent (m_InitNode);
			gcp::Fragment *fragment = dynamic_cast <gcp::Fragment *> (pObj);
			map<Atom*, Bond*>::iterator i;
			gcp::Bond *pBond = (fragment)? reinterpret_cast <gcp::Bond *> (fragment->GetAtom ()->GetFirstBond (i)): NULL;
			if ((initval && strlen (initval))) {
				if (endval && strlen (endval)) {
					pOp = m_pView->GetDoc ()->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
					pOp->AddNode (m_InitNode, 0);
					pOp->AddNode (m_CurNode, 1);
					if (pBond) {
						pOp->AddObject (pBond, 0);
						pOp->AddObject (pBond, 1);
					}
					m_CurNode = m_InitNode = NULL;
				} else {
					pOp = m_pView->GetDoc ()->GetNewOperation (gcp::GCP_DELETE_OPERATION);
					pOp->AddNode (m_InitNode);
					if (pBond)
						pOp->AddObject (pBond);
					m_InitNode = NULL;
				}
			} else if (endval && strlen (endval)) {
				pOp = m_pView->GetDoc ()->GetNewOperation (gcp::GCP_ADD_OPERATION);
				pOp->AddNode (m_CurNode);
				m_CurNode = NULL;
			}
			if (initval)
				xmlFree (initval);
			if (endval)
				xmlFree (endval);
		}
		if (pOp)
			m_pView->GetDoc ()->PushOperation (pOp, m_bUndo);
		m_bUndo = true;
	} else {
		if (m_Group) {
			if (m_GroupNode) {
				xmlFree (m_GroupNode);
				m_GroupNode = NULL;
			}
			m_Group = NULL;
		}
		pOp = m_pView->GetDoc ()->GetCurrentOperation ();
		if (pOp)
			m_pView->GetDoc ()->AbortOperation ();
	}
	xmlBufferFree (initbuf);
	xmlBufferFree (endbuf);
	if (m_CurNode) {
 		xmlUnlinkNode (m_CurNode);
		xmlFreeNode (m_CurNode);
	}
	if (m_InitNode) {
		xmlUnlinkNode (m_InitNode);
		xmlFreeNode (m_InitNode);
	}
	m_CurNode = m_InitNode = NULL;
	if (!*text) {
		Object* pMol = pObj->GetMolecule ();	//if pObj is a fragment
		if (pMol)
			pObj = pMol;
		gcu::Object *parent = pObj->GetParent ();
		m_pView->GetDoc ()->Remove (pObj);
		m_pView->GetDoc ()->AbortOperation ();
		parent->EmitSignal (gcp::OnChangedSignal);
	}
	m_pView->GetDoc ()->FinishOperation ();
	m_pView->GetDoc ()->GetWindow ()->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", m_pView->GetDoc ()->HasChildren ());
	return true;
}

bool gcpTextTool::DeleteSelection ()
{
	if (!m_Active)
		return false;
	unsigned start, end;
	std::string s = "";

	gcp::TextObject *text = dynamic_cast <gcp::TextObject*> (m_Active->GetClient ());
	if (!text)
		return false;
	text->GetSelectionBounds (start, end);
	m_Active->ReplaceText (s, start, end - start);
	text->OnChanged (true);
	return true;
}

bool gcpTextTool::CopySelection (GtkClipboard *clipboard)
{
	if (!m_Active)
		return false;
	unsigned start, end;
	gcp::Text *text = dynamic_cast <gcp::Text*> (m_Active->GetClient ());
	text->GetSelectionBounds (start, end);
	if (start == end)
		return false;
	m_pData->Copy (clipboard); //To clean the xmlDoc
	xmlDocPtr pDoc = m_pData->GetXmlDoc (clipboard);
	if (!pDoc)
		return false;
	pDoc->children = xmlNewDocNode (pDoc, NULL, (xmlChar*) "chemistry", NULL);
	xmlNsPtr ns = xmlNewNs (pDoc->children, (xmlChar*) "http://www.nongnu.org/gchempaint", (xmlChar*) "gcp");
	xmlSetNs (pDoc->children, ns);
	xmlNodePtr node = text->SaveSelection (pDoc);
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

bool gcpTextTool::CutSelection (GtkClipboard *clipboard)
{
	if (!CopySelection (clipboard))
		return false;
	return DeleteSelection ();
}

bool gcpTextTool::PasteSelection (GtkClipboard *clipboard)
{
	if (!m_Active)
		return false;
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &gcp::ClipboardDataType: &gcp::ClipboardDataType1;
	GdkAtom targets_atom  = gdk_atom_intern (gcp::targets[*DataType].target, FALSE);
	gtk_clipboard_request_contents (clipboard, targets_atom,  (GtkClipboardReceivedFunc) gcp::on_receive, m_pView);
	return true;
}

bool gcpTextTool::OnReceive (GtkClipboard *clipboard, GtkSelectionData *selection_data, G_GNUC_UNUSED int type)
{
	if (!m_Active)
		return false;
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &gcp::ClipboardDataType: &gcp::ClipboardDataType1;
	g_return_val_if_fail ((gtk_selection_data_get_target (selection_data) == gdk_atom_intern (gcp::targets[*DataType].target, FALSE)), FALSE);
	int length = gtk_selection_data_get_length (selection_data);
	char const *data = reinterpret_cast <char const *> (gtk_selection_data_get_data (selection_data));
	gcp::Text *text = dynamic_cast <gcp::Text*> (m_Active->GetClient ());
	unsigned start, end;
	text->GetSelectionBounds (start, end);
	switch (*DataType) {
		case gcp::GCP_CLIPBOARD_NATIVE: {
			xmlDocPtr xml = xmlParseMemory (data, length);
			xmlNodePtr node = xml->children;
			if ((strcmp((char*)node->name, "chemistry")) || (node->children->next)) {
				xmlFreeDoc (xml);
				return false;
			}
			node = node->children;
			if (!strcmp ((char*) node->name, "text")) {
				text->LoadSelection (node, start);
				xmlFreeDoc (xml);
				return true; // otherwise, we'd call OnChanged (true) twice.
			} else if (!strcmp((char*)node->name, "fragment")) {
				gcp::Fragment* fragment = new gcp::Fragment ();
				gcp::Document *pDoc = m_pView->GetDoc ();
				pDoc->AddChild (fragment);
				fragment->Load (node);
				string buf = fragment->GetBuffer ();
				m_Active->ReplaceText (buf, static_cast <int> (start), start - end);
				gccv::TextTagList tags = fragment->GetTagList ();
				gccv::TextTagList::iterator it, end = tags.end ();
				gccv::TextTag *tag;
				for (it = tags.begin (); it != end; it++) {
					switch ((*it)->GetTag ()) {
					case gccv::Family:
					case gccv::Size:
					case gccv::Style:
					case gccv::Weight:
					case gccv::Variant:
					case gccv::Stretch:
					case gccv::Underline:
					case gccv::Overline:
					case gccv::Strikethrough:
					case gccv::Foreground:
					case gccv::Background:
					case gccv::Rise:
					case gccv::NewLine:
						tag = (*it)->Duplicate ();
						break;
					default: {
						gccv::PositionTextTag *ptag = dynamic_cast <gccv::PositionTextTag *> (*it);
						if (!ptag)
							continue; // should not happen
						bool stacked;
						double size;
						if (!ptag)
							continue; // should not happen
						gccv::TextPosition position = ptag->GetPosition (stacked, size);
						tag = new gccv::PositionTextTag (position, stacked, size);
						break;
					}
					}
					tag->SetStartIndex ((*it)->GetStartIndex () + start);
					tag->SetEndIndex ((*it)->GetEndIndex () + start);
					m_Active->InsertTextTag (tag);
				}
				tags.clear ();
				delete fragment;
			} else {
				xmlFreeDoc (xml);
				return false;
			}
			xmlFreeDoc (xml);
			break;
		}
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
	text->OnChanged (true);
	return true;
}

bool gcpTextTool::OnUndo ()
{
	if (m_UndoList.empty ()) {
		if (m_pView->GetDoc()->CanUndo())
		{
			if (!m_RedoList.empty())
			{
				if (m_CurNode) {
					xmlUnlinkNode (m_CurNode);
					xmlFreeNode (m_CurNode);
				}
				m_CurNode = m_RedoList.back();
				m_RedoList.pop_back();
			}
			m_bUndo = false;
			Unselect();
		}
		return false;
	}
	xmlNodePtr node = m_UndoList.front();
	gcp::TextObject *text = dynamic_cast <gcp::TextObject*> (m_Active->GetClient ());
	text->LoadSelected (node);
	m_UndoList.pop_front ();
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Window *pWin = pDoc->GetWindow ();
	if (m_UndoList.empty() && !pDoc->CanUndo ())
		pWin->ActivateActionWidget ("/MainMenu/EditMenu/Undo", false);
	m_RedoList.push_front(m_CurNode);
	pWin->ActivateActionWidget ("/MainMenu/EditMenu/Redo", true);
	m_CurNode = node;
	return true;
}

bool gcpTextTool::OnRedo ()
{
	if (m_RedoList.empty ())
		return false;
	xmlNodePtr node = m_RedoList.front ();
	gcp::TextObject *text = dynamic_cast <gcp::TextObject*> (m_Active->GetClient ());
	text->LoadSelected (node);
	m_RedoList.pop_front ();
	gcp::Window *pWin = m_pView->GetDoc ()->GetWindow ();
	if (m_RedoList.empty ())
		pWin->ActivateActionWidget ("/MainMenu/EditMenu/Redo", false);
	m_UndoList.push_front (m_CurNode);
	pWin->ActivateActionWidget ("/MainMenu/EditMenu/Undo", true);
	m_CurNode = node;
	return true;
}

void gcpTextTool::PushNode (xmlNodePtr node)
{
	gcp::Window *pWin = m_pView->GetDoc ()->GetWindow ();
	while (!m_RedoList.empty ()) {
		xmlUnlinkNode (m_RedoList.front ());
		xmlFreeNode (m_RedoList.front ());
		m_RedoList.pop_front ();
		pWin->ActivateActionWidget ("/MainMenu/EditMenu/Redo", false);
	}
	m_UndoList.push_front (m_CurNode);
	m_CurNode = node;
	pWin->ActivateActionWidget ("/MainMenu/EditMenu/Undo", true);
}

void gcpTextTool::OnGetData (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info)
{
	xmlDocPtr pDoc = gcp::WidgetData::GetXmlDoc (clipboard);
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &gcp::ClipboardDataType: &gcp::ClipboardDataType1;
	*DataType = info;
	gint size;
	if (info) {
		if (gcp::ClipboardTextData) {
			g_free (gcp::ClipboardTextData);
		} 
		gcp::Text *text = new gcp::Text ();
		text->Load (pDoc->children->children);
		gcp::ClipboardTextData = g_strdup (text->GetBuffer ().c_str ());
		delete text;
		size = strlen ((char*) gcp::ClipboardTextData);
		gtk_selection_data_set_text (selection_data, (const gchar*) gcp::ClipboardTextData, size);
	} else {
		xmlDocDumpFormatMemory (pDoc, &gcp::ClipboardData, &size, info);
		gtk_selection_data_set (selection_data, gdk_atom_intern (GCHEMPAINT_ATOM_NAME, FALSE), 8,  (const guchar*) gcp::ClipboardData, size);
	}
	if (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))
			m_pView->GetDoc ()->GetWindow ()->ActivateActionWidget ("/MainMenu/EditMenu/Paste", true);
}

void gcpTextTool::UpdateTagsList ()
{
	if (!m_FamilyList)
		return;
	gcp::Theme *pTheme = m_pApp->GetActiveDocument ()->GetTheme ();
	m_FamilyName = pTheme->GetTextFontFamily ();
	m_Style = pTheme->GetFontStyle ();
	m_Weight = pTheme->GetFontWeight ();
	m_Stretch = pTheme->GetFontStretch ();
	m_Variant = pTheme->GetFontVariant ();
	m_Size = pTheme->GetFontSize ();
	m_Rise = 0;
	m_Underline = gccv::TextDecorationNone;
	m_Strikethrough = gccv::TextDecorationNone;
	m_Color = GO_COLOR_BLACK;
	if (m_Active) {
		unsigned index = GetIndex ();
		std::list <gccv::TextTag *> const *tags = m_Active->GetTags ();
		std::list <gccv::TextTag *>::const_iterator i, end = tags->end ();
		for (i = tags->begin (); i != end; i++) {
			if ((index)? ((*i)->GetStartIndex () < index && (*i)->GetEndIndex () >=  index):
				(*i)->GetStartIndex () == 0) {
				switch ((*i)->GetTag ()) {
				case gccv::Family:
					m_FamilyName = static_cast <gccv::FamilyTextTag const *> (*i)->GetFamily ();
					break;
				case gccv::Size:
					m_Size = static_cast <gccv::SizeTextTag const *> (*i)->GetSize ();
					break;
				case gccv::Style:
					m_Style = static_cast <gccv::StyleTextTag const *> (*i)->GetStyle ();
					break;
				case gccv::Weight:
					m_Weight = static_cast <gccv::WeightTextTag const *> (*i)->GetWeight ();
					break;
				case gccv::Variant:
					m_Variant = static_cast <gccv::VariantTextTag const *> (*i)->GetVariant ();
					break;
				case gccv::Stretch:
					m_Stretch = static_cast <gccv::StretchTextTag const *> (*i)->GetStretch ();
					break;
				case gccv::Underline:
					m_Underline = static_cast <gccv::UnderlineTextTag const *> (*i)->GetUnderline ();
					break;
				case gccv::Strikethrough:
					m_Strikethrough = static_cast <gccv::StrikethroughTextTag const *> (*i)->GetStrikethrough ();
					break;
				case gccv::Foreground:
					m_Color = static_cast <gccv::ForegroundTextTag const *> (*i)->GetColor ();
					break;
				case gccv::Rise:
					m_Rise = static_cast <gccv::RiseTextTag const *> (*i)->GetRise ();
					break;
				case gccv::Position: {
					bool stacked;
					double size;
					m_Position = static_cast <gccv::PositionTextTag const *> (*i)->GetPosition (stacked, size);
					break;
				}
				default:
					break;
				}
			}
		}
	} else {
		m_FamilyName = pTheme->GetTextFontFamily ();
		m_Style = pTheme->GetFontStyle ();
		m_Weight = pTheme->GetFontWeight ();
		m_Stretch = pTheme->GetFontStretch ();
		m_Variant = pTheme->GetFontVariant ();
		m_Size = pTheme->GetTextFontSize ();
		m_Underline = gccv::TextDecorationNone;
		m_Rise = 0;
		m_Color = GO_COLOR_BLACK;
		m_Position = gccv::Normalscript;
	}
	// select the found face
	GtkTreeIter iter;
	char *buf;
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_FamilyList), &iter);
	do {
		gtk_tree_model_get (GTK_TREE_MODEL (m_FamilyList), &iter, 0, &buf, -1);
		if (m_FamilyName == buf) {
			m_Dirty = true;
			GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (m_FamilyList), &iter);
			g_signal_handler_block (m_FamilySel, m_FamilySignal);
			g_signal_handler_block (m_FaceSel, m_FaceSignal);
			gtk_tree_view_set_cursor (m_FamilyTree, path, NULL, FALSE);
			g_signal_handler_unblock (m_FaceSel, m_FaceSignal);
			g_signal_handler_unblock (m_FamilySel, m_FamilySignal);
			gtk_tree_path_free (path);
			g_free (buf);
			break;
		}
		g_free (buf);
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (m_FamilyList), &iter));
	// Update other widgets
	SetSizeFull (true, false);
	g_signal_handler_block (m_UnderlineBox, m_UnderlineSignal);
	gtk_combo_box_set_active (m_UnderlineBox, m_Underline);
	g_signal_handler_unblock (m_UnderlineBox, m_UnderlineSignal);
	g_signal_handler_block (m_StrikethroughBtn, m_StrikethroughSignal);
	gtk_toggle_button_set_active (m_StrikethroughBtn, m_Strikethrough);
	g_signal_handler_unblock (m_StrikethroughBtn, m_StrikethroughSignal);
	g_signal_handler_block (m_RiseButton, m_RiseSignal);
	gtk_spin_button_set_value (m_RiseButton, m_Rise / PANGO_SCALE);
	g_signal_handler_unblock (m_RiseButton, m_RiseSignal);
	g_signal_handler_block (m_ColorSelector, m_ForeSignal);
	go_color_selector_set_color (m_ColorSelector, m_Color);
	g_signal_handler_unblock (m_ColorSelector, m_ForeSignal);
	BuildTagsList ();
}

void gcpTextTool::BuildTagsList ()
{
	if (!m_Active)
		return;
	gccv::TextTagList *l = new gccv::TextTagList ();
	l->push_front (new gccv::FamilyTextTag (m_FamilyName));
	l->push_front (new gccv::StyleTextTag (m_Style));
	l->push_front (new gccv::WeightTextTag (m_Weight));
	l->push_front (new gccv::StretchTextTag (m_Stretch));
	l->push_front (new gccv::VariantTextTag (m_Variant));
	l->push_front (new gccv::SizeTextTag (m_Size));
	l->push_front (new gccv::UnderlineTextTag (m_Underline));
	l->push_front (new gccv::StrikethroughTextTag (m_Strikethrough));
	l->push_front (new gccv::RiseTextTag (m_Rise));
	l->push_front (new gccv::ForegroundTextTag (m_Color));
	l->push_front (new gccv::PositionTextTag (m_Position, m_Size));
	m_Active->SetCurTagList (l);
	m_Dirty = false;
	if (m_pView)
		gtk_window_present (m_pView->GetDoc ()->GetWindow ()->GetWindow ());
}

static void on_select_family (GtkTreeSelection *selection, gcpTextTool *tool)
{
	tool->OnSelectFamily (selection);
}

static void on_select_face (GtkTreeSelection *selection, gcpTextTool *tool)
{
	tool->OnSelectFace (selection);
}

static void on_select_size (GtkTreeSelection *selection, gcpTextTool *tool)
{
	int size;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gtk_tree_selection_get_selected (selection, &model, &iter);
	gtk_tree_model_get (model, &iter, 0, &size, -1);
	tool->OnSelectSize (size * PANGO_SCALE);
}

static void on_size_activate (G_GNUC_UNUSED GtkEntry *entry, gcpTextTool *tool)
{
	tool->OnSizeChanged ();
}

static void on_size_focus_out (G_GNUC_UNUSED GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, gcpTextTool *tool)
{
	tool->OnSizeChanged ();
}

static void on_underline_changed (GtkComboBox *box, gcpTextTool *tool)
{
	tool->OnUnderlineChanged (gtk_combo_box_get_active (box));
}

static void on_strikethrough_toggled (GtkToggleButton *btn, gcpTextTool *tool)
{
	tool->OnStriketroughToggled (gtk_toggle_button_get_active (btn));
}

static void on_fore_color_changed (GOSelector *sel, gcpTextTool *tool)
{
	gboolean auto_color;
	tool->OnForeColorChanged (go_color_selector_get_color (sel, &auto_color));
}

static void on_rise_changed (GtkSpinButton *btn, gcpTextTool *tool)
{
	tool->OnPositionChanged (gtk_spin_button_get_value_as_int (btn));
}

/* These are what we use as the standard font sizes, for the size list.
 */
static const guint16 font_sizes[] = {
  8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 22, 24, 26, 28,
  32, 36, 40, 48, 56, 64, 72
};

GtkWidget *gcpTextTool::GetPropertyPage ()
{
	gcu::UIBuilder *builder = new gcu::UIBuilder (UIDIR"/fontsel.ui", GETTEXT_PACKAGE);
	PangoFontFamily **families;
	int i, nb;
	gcp::Theme *pTheme = m_pApp->GetActiveDocument ()->GetTheme ();
	// Initialize faces list
	m_FaceList = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (m_FaceList), 0, GTK_SORT_ASCENDING);
	m_FacesTree = GTK_TREE_VIEW (builder->GetWidget ("style"));
	gtk_tree_view_set_model (m_FacesTree, GTK_TREE_MODEL (m_FaceList));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (m_FacesTree, column);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (m_FacesTree);
	m_FaceSel = selection;
	m_FaceSignal = g_signal_connect (m_FaceSel, "changed", G_CALLBACK (on_select_face), this);
	// Initialize sizes list
	m_SizeList = gtk_list_store_new (1, G_TYPE_INT);
	m_SizesTree = GTK_TREE_VIEW (builder->GetWidget ("size-list"));
	gtk_tree_view_set_model (m_SizesTree, GTK_TREE_MODEL (m_SizeList));
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (m_SizesTree, column);
	GtkTreeIter iter, selected;
	for (i = 0; i < (int) G_N_ELEMENTS (font_sizes); i++) {
		gtk_list_store_append (m_SizeList, &iter);
		gtk_list_store_set (m_SizeList, &iter,
				  0, font_sizes[i],
				  -1);
	}
	selection = gtk_tree_view_get_selection (m_SizesTree);
	m_SizeSel = selection;
	m_SizeSignal = g_signal_connect (m_SizeSel, "changed", G_CALLBACK (on_select_size), this);
	// Size entry
	m_SizeEntry = GTK_ENTRY (builder->GetWidget ("size-entry"));
	g_signal_connect (m_SizeEntry, "activate", G_CALLBACK (on_size_activate), this);
	g_signal_connect_after (m_SizeEntry, "focus_out_event", G_CALLBACK (on_size_focus_out), this);
	SetSizeFull (true, false);
	PangoContext *pc = gtk_widget_get_pango_context (GTK_WIDGET (m_SizeEntry));
	pango_context_list_families (pc, &families, &nb);
	m_FamilyList = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (m_FamilyList), 0, GTK_SORT_ASCENDING);
	m_FamilyTree = GTK_TREE_VIEW (builder->GetWidget ("family"));
	gtk_tree_view_set_model (m_FamilyTree, GTK_TREE_MODEL (m_FamilyList));
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (m_FamilyTree, column);
	GtkTreePath *path = NULL;
	string name;
	bool default_found = false;
	for (i = 0; i < nb; i++) {
		PangoFontFace **faces = NULL;
		int *sizes = NULL, n;
		pango_font_family_list_faces (families[i], &faces, &n);
		if (n <= 0) {
			g_free (faces);
			continue;
		}
		pango_font_face_list_sizes (faces[0], &sizes, &n);
		if (n > 0) {// Do not use bitmap fonts
			g_free (faces);
			g_free (sizes);
			continue;
		}
		name = pango_font_family_get_name (families[i]);
		m_Families[name] = (PangoFontFamily*) g_object_ref (families[i]);
		gtk_list_store_append (m_FamilyList, &iter);
		gtk_list_store_set (m_FamilyList, &iter,
				  0, name.c_str (),
				  -1);
		if (name == pTheme->GetTextFontFamily ()) {
			selected = iter;
			default_found = true;
		}
		g_free (faces);
		g_free (sizes);
	}
	g_free (families);
	if (!default_found) {
		if	(!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_FamilyList), &iter))
			return NULL;
		selected = iter;
	}
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (m_FamilyList), &selected);
	selection = gtk_tree_view_get_selection (m_FamilyTree);
	m_FamilySel = selection;
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
	m_FamilySignal = g_signal_connect (G_OBJECT (selection), "changed", G_CALLBACK (on_select_family), this);
	// Select default font
	if (path) {
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_view_scroll_to_cell (m_FamilyTree, path, column, FALSE, 0., 0.);
		gtk_tree_path_free (path);
	}
	m_UnderlineBox = GTK_COMBO_BOX (builder->GetWidget ("underline"));
	gtk_combo_box_set_active (m_UnderlineBox, 0);
	m_UnderlineSignal = g_signal_connect (G_OBJECT (m_UnderlineBox), "changed", G_CALLBACK (on_underline_changed), this);
	m_StrikethroughBtn = GTK_TOGGLE_BUTTON (builder->GetWidget ("strikethrough"));
	m_StrikethroughSignal = g_signal_connect (G_OBJECT (m_StrikethroughBtn), "toggled", G_CALLBACK (on_strikethrough_toggled), this);
	m_RiseButton = GTK_SPIN_BUTTON (builder->GetWidget ("rise"));
	m_RiseSignal = g_signal_connect (G_OBJECT (m_RiseButton), "value-changed", G_CALLBACK (on_rise_changed), this);
	m_ColorSelector = GO_SELECTOR (go_color_selector_new (GO_COLOR_BLACK, GO_COLOR_BLACK, "fore"));
	go_color_selector_set_allow_alpha (m_ColorSelector, false);
	m_ForeSignal = g_signal_connect (G_OBJECT (m_ColorSelector), "activate", G_CALLBACK (on_fore_color_changed), this);
	gtk_widget_show (GTK_WIDGET (m_ColorSelector));
	gtk_table_attach (GTK_TABLE (builder->GetWidget ("table2")), GTK_WIDGET (m_ColorSelector), 1, 2, 0, 1, (GtkAttachOptions) 0, (GtkAttachOptions) 0, 0, 0);
	GtkWidget *res = builder->GetRefdWidget ("fontsel");
	delete builder;
	return res;
}

void gcpTextTool::OnSelectFamily (GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter, selected;
	char const *name;
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
	gtk_tree_model_get (model, &iter, 0, &name, -1);
	m_FamilyName = name;
	g_free (const_cast <char *> (name));
	PangoFontFamily *family = m_Families[m_FamilyName];
	PangoFontFace **faces;
	int i, besti, nb;
	g_signal_handler_block (m_FaceSel, m_FaceSignal);
	pango_font_family_list_faces (family, &faces, &nb);
	gtk_list_store_clear (m_FaceList);
	map<string, PangoFontFace*>::iterator j, jend = m_Faces.end ();
	for (j = m_Faces.begin (); j != jend; j++) {
		g_object_unref ((*j).second);
	}
	m_Faces.clear ();
	PangoFontDescription *desc;
	int distance, best;
	PangoStyle Style;
	PangoWeight Weight;
	PangoVariant Variant;
	PangoStretch Stretch;

	best = 32000; // This should be enough
	for (i = 0; i < nb; i++) {
		name = pango_font_face_get_face_name (faces[i]);
		desc = pango_font_face_describe (faces[i]);
		m_Faces[name] = (PangoFontFace*) g_object_ref (faces[i]);
		gtk_list_store_append (m_FaceList, &iter);
		gtk_list_store_set (m_FaceList, &iter,
				  0, name,
				  -1);
		// Try to select the best available face
		Style = pango_font_description_get_style (desc);
		Weight = pango_font_description_get_weight (desc);
		Variant = pango_font_description_get_variant (desc);
		Stretch = pango_font_description_get_stretch (desc);
		distance = abs (Weight - m_Weight)
						+ abs ((Style? Style + 2: 0) - (m_Style? m_Style + 2: 0)) * 1000
						+ abs (Variant - m_Variant) * 10 + abs (Stretch - m_Stretch);
		if (distance < best) {
			best = distance;
			selected = iter;
			besti = i;
		}
		// TODO: write this code
		pango_font_description_free (desc);
	}
	g_free (faces);
	g_signal_handler_unblock (m_FaceSel, m_FaceSignal);
	GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (m_FaceList), &selected);
	if (path) {
		gtk_tree_selection_select_path (GTK_TREE_SELECTION (m_FaceSel), path);
		gtk_tree_path_free (path);
	} else {
		//TODO: choose a face when default is not available
	}
	if (m_Active) {
		gccv::TextTagList l;
		l.push_front (new gccv::FamilyTextTag (m_FamilyName));
		l.push_front (new gccv::StyleTextTag (m_Style));
		l.push_front (new gccv::WeightTextTag (m_Weight));
		l.push_front (new gccv::StretchTextTag (m_Stretch));
		l.push_front (new gccv::VariantTextTag (m_Variant));
		m_Active->ApplyTagsToSelection (&l);
	}
}

void gcpTextTool::OnSelectFace (GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *name;
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
	gtk_tree_model_get (model, &iter, 0, &name, -1);
	PangoFontFace *face = m_Faces[name];
	g_free (name);
	PangoFontDescription *desc = pango_font_face_describe (face);
	m_Style = pango_font_description_get_style (desc);
	m_Weight = pango_font_description_get_weight (desc);
	m_Variant = pango_font_description_get_variant (desc);
	m_Stretch = pango_font_description_get_stretch (desc);
	pango_font_description_free (desc);
	BuildTagsList ();
	if (m_Active) {
		gccv::TextTagList l;
		l.push_front (new gccv::StyleTextTag (m_Style));
		l.push_front (new gccv::WeightTextTag (m_Weight));
		l.push_front (new gccv::StretchTextTag (m_Stretch));
		l.push_front (new gccv::VariantTextTag (m_Variant));
		m_Active->ApplyTagsToSelection (&l);
	}
}

void gcpTextTool::OnSelectSize (int size)
{
	m_Size = size;
	SetSizeFull (false, true);
}

void gcpTextTool::OnSizeChanged ()
{
	char const *text = gtk_entry_get_text (m_SizeEntry);
	m_Size = (int) (MAX (0.1, atof (text) * PANGO_SCALE + 0.5));
	SetSizeFull (true, true);
}

void gcpTextTool::SetSizeFull (bool update_list, bool apply)
{
	char *buf = g_strdup_printf ("%.1f", (double) m_Size / PANGO_SCALE);
	gtk_entry_set_text (m_SizeEntry, buf);
	g_free (buf);
	if (update_list) {
		GtkTreeIter iter;
		bool found = false;
		GtkTreeSelection *selection = gtk_tree_view_get_selection (m_SizesTree);
		g_signal_handler_block (selection, m_SizeSignal);
		
		gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_SizeList), &iter);
		for (unsigned i = 0; i < G_N_ELEMENTS (font_sizes) && !found; i++) {
			if (font_sizes[i] * PANGO_SCALE == m_Size) {
				GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (m_SizeList), &iter);
				gtk_tree_view_set_cursor (m_SizesTree, path, NULL, FALSE);
				gtk_tree_path_free (path);
				found = true;
			}

			gtk_tree_model_iter_next (GTK_TREE_MODEL (m_SizeList), &iter);
		}
		
		if (!found)
			gtk_tree_selection_unselect_all (selection);
		g_signal_handler_unblock (selection, m_SizeSignal);
	}
	BuildTagsList ();
	if (apply && m_Active) {
		gccv::TextTagList l;
		l.push_front (new gccv::SizeTextTag (m_Size));
		m_Active->ApplyTagsToSelection (&l);
	}
}

void gcpTextTool::SelectBestFontFace ()
{
	PangoFontDescription *desc;
	int distance, best;
	PangoStyle Style;
	PangoWeight Weight;
	PangoVariant Variant;
	PangoStretch Stretch;
	map <string, PangoFontFace*>::iterator i, iend = m_Faces.end ();
	char const *name = NULL, *buf;
	GtkTreeIter iter;

	best = 32000; // This should be enough
	for (i = m_Faces.begin (); i != iend; i++) {
		desc = pango_font_face_describe ((*i).second);
		// Try to select the best available face
		Style = pango_font_description_get_style (desc);
		Weight = pango_font_description_get_weight (desc);
		Variant = pango_font_description_get_variant (desc);
		Stretch = pango_font_description_get_stretch (desc);
		distance = abs (Weight - m_Weight)
						+ abs ((Style? Style + 2: 0) - (m_Style? m_Style + 2: 0)) * 1000
						+ abs (Variant - m_Variant) * 10 + abs (Stretch - m_Stretch);
		if (distance < best) {
			best = distance;
			name = (*i).first.c_str ();
		}
		pango_font_description_free (desc);
	}
	// select the found face
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_FaceList), &iter);
	do {
		gtk_tree_model_get (GTK_TREE_MODEL (m_FaceList), &iter, 0, &buf, -1);
		if (!strcmp (name, buf)) {
			m_Dirty = true;
			GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (m_FaceList), &iter);
			gtk_tree_view_set_cursor (m_FacesTree, path, NULL, FALSE);
			gtk_tree_path_free (path);
			if (m_Dirty)
				OnSelectFace ((GtkTreeSelection*) m_FaceSel);
			break;
		}
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (m_FaceList), &iter));
}

unsigned gcpTextTool::GetIndex ()
{
	return (m_Active)? m_Active->GetCursorPosition (): 0;
}

void gcpTextTool::OnUnderlineChanged (unsigned underline)
{
	switch (underline) {
	default:
		m_Underline = gccv::TextDecorationNone;
		break;
	case 1:
		m_Underline = gccv::TextDecorationDefault;
		break;
	case 2:
		m_Underline = gccv::TextDecorationDouble;
		break;
	case 3:
		m_Underline = gccv::TextDecorationLow;
		break;
	}
	BuildTagsList ();
	if (m_Active) {
		gccv::TextTagList l;
		l.push_front (new gccv::UnderlineTextTag (m_Underline));
		m_Active->ApplyTagsToSelection (&l);
	}
}

void gcpTextTool::OnStriketroughToggled (bool strikethrough)
{
	m_Strikethrough = strikethrough? gccv::TextDecorationDefault: gccv::TextDecorationNone;
	BuildTagsList ();
	if (m_Active) {
		gccv::TextTagList l;
		l.push_front (new gccv::StrikethroughTextTag (m_Strikethrough));
		m_Active->ApplyTagsToSelection (&l);
	}
}

void gcpTextTool::OnPositionChanged (int position)
{
	m_Rise = position * PANGO_SCALE;
	BuildTagsList ();
	if (m_Active) {
		gccv::TextTagList l;
		l.push_front (new gccv::RiseTextTag (m_Rise));
		m_Active->ApplyTagsToSelection (&l);
	}
}

void gcpTextTool::OnForeColorChanged (GOColor color)
{
	m_Color = color;
	BuildTagsList ();
	if (m_Active) {
		gccv::TextTagList l;
		l.push_front (new gccv::ForegroundTextTag (m_Color));
		m_Active->ApplyTagsToSelection (&l);
	}
}

void gcpTextTool::SelectionChanged ()
{
	UpdateTagsList ();
}
