// -*- C++ -*-

/* 
 * GChemPaint text plugin
 * texttool.cc 
 *
 * Copyright (C) 2002-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcp/document.h>
#include <gcp/fragment.h>
#include <gcp/settings.h>
#include <gcp/text.h>
#include <gcp/theme.h>
#include <gcp/window.h>
#include <goffice/gtk/go-color-selector.h>
#include <gdk/gdkkeysyms.h>
#include <unistd.h>
#include <cstring>

using namespace gcu;
extern GtkTargetEntry const text_targets[];

GtkTargetEntry const text_targets[] = {
	{(char *) GCHEMPAINT_ATOM_NAME,  0, gcp::GCP_CLIPBOARD_NATIVE},
	{(char *) "UTF8_STRING", 0, gcp::GCP_CLIPBOARD_UTF8_STRING},
	{(char *) "STRING", 0, gcp::GCP_CLIPBOARD_STRING}
};

static void on_get_data (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, gcpTextTool* tool)
{
	tool->OnGetData (clipboard, selection_data, info);
}

static void on_sel_changed (gcpTextTool *tool)
{
	tool->UpdateAttributeList ();
}

static bool filter_attribute (PangoAttribute *attribute, gcpTextTool *tool)
{
	int index = tool->GetIndex ();
	if (index < 0)
		return false;
	if ((index)? (attribute->start_index < (unsigned) index && attribute->end_index >= (unsigned) index):
		attribute->start_index == 0) {
			switch (attribute->klass->type) {
				case PANGO_ATTR_FAMILY:
					tool->SetFamilyName (((PangoAttrString*)attribute)->value);
					break;
				case PANGO_ATTR_STYLE:
					tool->SetStyle ((PangoStyle) ((PangoAttrInt*)attribute)->value);
					break;
				case PANGO_ATTR_WEIGHT:
					tool->SetWeight ((PangoWeight) ((PangoAttrInt*)attribute)->value);
					break;
				case PANGO_ATTR_VARIANT:
					tool->SetVariant ((PangoVariant) ((PangoAttrInt*)attribute)->value);
					break;
				case PANGO_ATTR_STRETCH:
					tool->SetStretch ((PangoStretch) ((PangoAttrInt*)attribute)->value);
					break;
				case PANGO_ATTR_SIZE:
					tool->SetSize (((PangoAttrInt*)attribute)->value);
					break;
				case PANGO_ATTR_UNDERLINE:
					tool->SetUnderline ((PangoUnderline) ((PangoAttrInt*)attribute)->value);
					break;
				case PANGO_ATTR_STRIKETHROUGH:
					tool->SetStrikethrough (((PangoAttrInt*)attribute)->value);
					break;
				case PANGO_ATTR_RISE:
					tool->SetRise (((PangoAttrInt*)attribute)->value);
					break;
				case PANGO_ATTR_FOREGROUND: {
					PangoColor color = ((PangoAttrColor*)attribute)->color;
					tool->SetColor (RGBA_TO_UINT(color.red >> 8, color.green >> 8, color.blue >> 8, 0xff));
					break;
				}
				default:
					break;
			}
		}
	return false;
}

gcpTextTool::gcpTextTool (gcp::Application* App, string Id):
	gcp::Tool (App, Id),
	m_FamilyList (NULL)
{
	m_Active = NULL;
	m_bUndo = true;
	m_CurNode = m_InitNode = NULL;
	m_Strikethrough = false;
	m_FontDesc = NULL;
	gcp::Theme *pTheme = gcp::TheThemeManager.GetTheme ("Default");
	m_FamilyName = pTheme->GetTextFontFamily ();
	m_Style = pTheme->GetFontStyle ();
	m_Weight = pTheme->GetFontWeight ();
	m_Stretch = pTheme->GetFontStretch ();
	m_Variant = pTheme->GetFontVariant ();
	m_Size = pTheme->GetFontSize ();
	m_Underline = PANGO_UNDERLINE_NONE;
	m_Rise = 0;
	m_Color = RGBA_BLACK;
	m_SelSignal = 0;
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
			(m_Active != g_object_get_data (G_OBJECT (m_pData->Items[m_pObject]), "text"))))
		Unselect ();
	bool create = false;
	if (!m_pObject) {
		gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
		gcp::Text *text = new gcp::Text(m_x0 / pTheme->GetZoomFactor (), m_y0 / pTheme->GetZoomFactor ());
		m_pView->GetDoc ()->AddObject (text);
		m_pView->GetDoc ()->AbortOperation ();
		m_pObject = text;
		create = true;
	}
	if (m_pObject) {
		if (m_pObject->GetType () != TextType)
			return false;
		m_pObject->SetSelected (m_pWidget, gcp::SelStateUpdating);
		m_Active = GNOME_CANVAS_PANGO (g_object_get_data (G_OBJECT (m_pData->Items[m_pObject]), "text"));
		m_pView->SetGnomeCanvasPangoActive (m_Active);
		g_object_set (G_OBJECT (m_Active), "editing", true, NULL);
		m_CurNode = ((gcp::Text*) m_pObject)->SaveSelected ();
		m_InitNode = ((gcp::Text*) m_pObject)->SaveSelected ();
		m_pView->GetDoc ()->GetWindow ()->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", false);
		if (m_SelSignal == 0)
			m_SelSignal = g_signal_connect_swapped (m_Active, "sel-changed", G_CALLBACK (on_sel_changed), this);
		if (create)
			BuildAttributeList ();
		else
			UpdateAttributeList ();
	}
	return true;
}

bool gcpTextTool::OnEvent (GdkEvent* event)
{
	if (m_Active) {
		if ((event->type == GDK_KEY_PRESS) || (event->type == GDK_KEY_RELEASE)) {
			if (event->key.state & GDK_CONTROL_MASK) {
				switch (event->key.keyval) {
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
					BuildAttributeList ();
					return true;
				case GDK_u:
					gtk_combo_box_set_active (m_UnderlineBox, ((m_Underline == PANGO_UNDERLINE_SINGLE)? PANGO_UNDERLINE_NONE: PANGO_UNDERLINE_SINGLE));
					return true;
				case GDK_b:
					m_Weight = (m_Weight == PANGO_WEIGHT_NORMAL)? PANGO_WEIGHT_BOLD: PANGO_WEIGHT_NORMAL;
					SelectBestFontFace ();
					BuildAttributeList ();
					return true;
				case GDK_k:
					gtk_toggle_button_set_active (m_StrikethroughBtn, !m_Strikethrough);
					return true;
				case GDK_plus:
				case GDK_dead_circumflex:
					if (m_Rise == 0) {
						m_Size = m_Size * 2 / 3;
						m_Rise = m_Size / PANGO_SCALE;
					} else if (m_Rise < 0)
						m_Rise = m_Size / PANGO_SCALE;
					else {
						m_Size = m_Size * 3 / 2;
						m_Rise = 0;
					}
					g_signal_handler_block (m_RiseButton, m_RiseSignal);
					gtk_spin_button_set_value (m_RiseButton, m_Rise);
					m_Rise *= PANGO_SCALE;
					g_signal_handler_unblock (m_RiseButton, m_RiseSignal);
					SetSizeFull (true);
					return true;
				case GDK_equal:
				case GDK_underscore:
					if (m_Rise == 0) {
						m_Size = m_Size * 2 / 3;
						m_Rise = - m_Size / PANGO_SCALE / 2;
					} else if (m_Rise > 0)
						m_Rise = - m_Size / PANGO_SCALE/ 2;
					else {
						m_Size = m_Size * 3 / 2;
						m_Rise = 0;
					}
					g_signal_handler_block (m_RiseButton, m_RiseSignal);
					gtk_spin_button_set_value (m_RiseButton, m_Rise);
					m_Rise *= PANGO_SCALE;
					g_signal_handler_unblock (m_RiseButton, m_RiseSignal);
					SetSizeFull (true);
					return true;
				case GDK_space: {
					GnomeCanvasPango* saved = m_Active;
					m_Active = NULL;
					UpdateAttributeList ();
					m_Active = saved;
					BuildAttributeList ();
					return true;
				}
				default:
					break;
				}
			}
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
		} else if (event->type == GDK_BUTTON_PRESS) {
			 switch (event->button.button) {
				case 2:
				return true;
			}
		} else if (event->type == GDK_MOTION_NOTIFY) {
			GnomeCanvasItemClass* klass = GNOME_CANVAS_ITEM_CLASS (G_OBJECT_GET_CLASS (m_Active));
			klass->event ((GnomeCanvasItem*) m_Active, event);
		}
	}
	return false;
}

void gcpTextTool::Activate ()
{
	if (!m_Active)
		UpdateAttributeList ();
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
	if (m_SelSignal) {
		g_signal_handler_disconnect (m_Active, m_SelSignal);
		m_SelSignal = 0;
	}
	g_object_set (G_OBJECT (m_Active), "editing", false, NULL);
	m_pView->SetGnomeCanvasPangoActive (NULL);
	Object *pObj = (Object*) g_object_get_data (G_OBJECT (m_Active), "object");
	pObj->SetSelected (m_pWidget, gcp::SelStateUnselected);
	char const *text = pango_layout_get_text (gnome_canvas_pango_get_layout (m_Active));
	m_Active = NULL;
	while (!m_UndoList.empty ()) {
		xmlFree(m_UndoList.front ());
		m_UndoList.pop_front ();
	}
	while (!m_RedoList.empty ()) {
		xmlFree(m_RedoList.front ());
		m_RedoList.pop_front ();
	}
	xmlBufferPtr initbuf = xmlBufferCreate ();
	xmlBufferPtr endbuf = xmlBufferCreate ();
	xmlNodeDump (initbuf, m_pApp->GetXmlDoc (), m_InitNode, 0, 0);
	xmlNodeDump (endbuf, m_pApp->GetXmlDoc (), m_CurNode, 0, 0);
	if (strcmp ((char*) initbuf->content, (char*) endbuf->content))
	{
		char* initval = (char*) xmlNodeGetContent (m_InitNode);
		char* endval = (char*) xmlNodeGetContent (m_CurNode);
		gcp::Operation *pOp = NULL;
		if ((initval && strlen (initval))) {
			if (endval && strlen (endval)) {
				pOp = m_pView->GetDoc ()->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
				pOp->AddNode (m_InitNode, 0);
				pOp->AddNode (m_CurNode, 1);
				m_CurNode = m_InitNode = NULL;
			} else {
				pOp = m_pView->GetDoc ()->GetNewOperation (gcp::GCP_DELETE_OPERATION);
				pOp->AddNode (m_InitNode);
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
		if (pOp)
			m_pView->GetDoc ()->PushOperation (pOp, m_bUndo);
		m_bUndo = true;
	}
	xmlBufferFree (initbuf);
	xmlBufferFree (endbuf);
	if (m_CurNode)
		xmlFree (m_CurNode);
	if (m_InitNode)
		xmlFree (m_InitNode);
	m_CurNode = m_InitNode = NULL;
	if (!*text) {
		Object* pMol = pObj->GetMolecule ();	//if pObj is a fragment
		if (pMol)
			pObj = pMol;
		m_pView->GetDoc ()->Remove (pObj);
		m_pView->GetDoc ()->AbortOperation ();
	}
	m_pView->GetDoc ()->GetWindow ()->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", m_pView->GetDoc ()->HasChildren ());
	return true;
}

bool gcpTextTool::DeleteSelection ()
{
	if (!m_Active)
		return false;
	unsigned start, end;

	gcp::TextObject *text = dynamic_cast<gcp::TextObject*> ((Object*) g_object_get_data (G_OBJECT (m_Active), "object"));
	if (!text)
		return false;
	text->GetSelectionBounds (start, end);
	gcp_pango_layout_replace_text (gnome_canvas_pango_get_layout (m_Active),
												start, end - start, "", NULL);
	gnome_canvas_pango_set_selection_bounds (m_Active, start, start);
	text->OnChanged (true);
	return true;
}

bool gcpTextTool::CopySelection (GtkClipboard *clipboard)
{
	if (!m_Active)
		return false;
	unsigned start, end;
	gcp::Text *text = (gcp::Text*) g_object_get_data (G_OBJECT (m_Active), "object");
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

struct FragState {
	PangoAttrList *l;
	unsigned start;
};

static bool filter_fragment (PangoAttribute *attr, struct FragState *s)
{
	PangoAttribute *new_attr = pango_attribute_copy (attr);
	new_attr->start_index += s->start;
	new_attr->end_index += s->start;
	pango_attr_list_change (s->l, new_attr);
	return false;
}

bool gcpTextTool::OnReceive (GtkClipboard *clipboard, GtkSelectionData *data, int type)
{
	if (!m_Active)
		return false;
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &gcp::ClipboardDataType: &gcp::ClipboardDataType1;
	g_return_val_if_fail ((data->target == gdk_atom_intern (gcp::targets[*DataType].target, FALSE)), FALSE);
	gcp::Text *text = (gcp::Text*) g_object_get_data (G_OBJECT (m_Active), "object");
	unsigned start, end;
	text->GetSelectionBounds (start, end);
	PangoLayout *layout = gnome_canvas_pango_get_layout (m_Active);
	switch (*DataType) {
		case gcp::GCP_CLIPBOARD_NATIVE: {
			xmlDocPtr xml = xmlParseMemory ((const char*) data->data, data->length);
			xmlNodePtr node = xml->children;
			if ((strcmp((char*)node->name, "chemistry")) || (node->children->next)) {
				xmlFreeDoc (xml);
				return false;
			}
			node = node->children;
			if (!strcmp ((char*) node->name, "text")) {
				gcp_pango_layout_replace_text (layout, start, end - start, "", NULL);
				text->LoadSelection (node, start);
				xmlFreeDoc (xml);
				return true; // otherwise, we'd call OnChange(true) twice.
			} else if (!strcmp((char*)node->name, "fragment")) {
				gcp::Fragment* fragment = new gcp::Fragment ();
				gcp::Document *pDoc = m_pView->GetDoc ();
				gcp::Theme *pTheme = pDoc->GetTheme ();
				pDoc->AddChild (fragment);
				fragment->Load (node);
				string buf = fragment->GetBuffer ();
				PangoAttrList *l = pango_attr_list_new ();
				PangoAttribute *attr = pango_attr_family_new (pTheme->GetFontFamily ());
				pango_attr_list_insert (l, attr);
				attr = pango_attr_size_new (pTheme->GetFontSize ());
				pango_attr_list_insert (l, attr);
				gcp_pango_layout_replace_text (layout, start, end - start, buf.c_str (), l);
				pango_attr_list_unref (l);
				l = fragment->GetAttrList ();
				struct FragState s;
				s.l = pango_layout_get_attributes (layout);
				s.start = start;
				pango_attr_list_filter (l, (PangoAttrFilterFunc) filter_fragment, &s);
				delete fragment;
				start += buf.length ();
				gnome_canvas_pango_set_selection_bounds (m_Active, start, start);
			} else {
				xmlFreeDoc (xml);
				return false;
			}
			xmlFreeDoc (xml);
			break;
		}
		case gcp::GCP_CLIPBOARD_UTF8_STRING: {
			PangoAttrList *l = pango_attr_list_new ();
			gcp_pango_layout_replace_text (layout, start, end - start, (char const *) data->data, l);
			pango_attr_list_unref (l);
			break;
		}
		case gcp::GCP_CLIPBOARD_STRING: {
			PangoAttrList *l = pango_attr_list_new ();
			if (!g_utf8_validate ((const char*) data->data, data->length, NULL)) {
				gsize r, w;
				gchar* newstr = g_locale_to_utf8 ((const char*) data->data, data->length, &r, &w, NULL);
				gcp_pango_layout_replace_text (layout, start, end - start, (char const *) data->data, l);
				g_free (newstr);
			} else
				gcp_pango_layout_replace_text (layout, start, end - start, (char const *) data->data, l);
			pango_attr_list_unref (l);
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
				if (m_CurNode) xmlFree(m_CurNode);
				m_CurNode = m_RedoList.back();
				m_RedoList.pop_back();
			}
			m_bUndo = false;
			Unselect();
		}
		return false;
	}
	xmlNodePtr node = m_UndoList.front();
	gcp::TextObject *text = (gcp::TextObject*) g_object_get_data (G_OBJECT (m_Active), "object");
	text->LoadSelected (node);
	m_UndoList.pop_front ();
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Window *pWin = pDoc->GetWindow ();
	if (m_UndoList.empty() && !pDoc->CanUndo ())
		pWin->ActivateActionWidget ("/MainMenu/EditMenu/Undo", false);
	m_RedoList.push_front(m_CurNode);
	pWin->ActivateActionWidget ("/MainMenu/EditMenu/Redo", true);
	unsigned start, end;
	char* tmp = (char*) xmlGetProp (node, (xmlChar*) "start-sel");
	start = (int) strtoul (tmp, NULL, 10);
	xmlFree (tmp);
	tmp = (char*) xmlGetProp (node, (xmlChar*) "end-sel");
	end = (int) strtoul (tmp, NULL, 10);
	xmlFree (tmp);
	gnome_canvas_pango_set_selection_bounds (m_Active, start, end);
	m_CurNode = node;
	return true;
}

bool gcpTextTool::OnRedo ()
{
	if (m_RedoList.empty ())
		return false;
	xmlNodePtr node = m_RedoList.front ();
	gcp::TextObject *text = (gcp::TextObject*) g_object_get_data (G_OBJECT (m_Active), "object");
	text->LoadSelected (node);
	m_RedoList.pop_front ();
	gcp::Window *pWin = m_pView->GetDoc ()->GetWindow ();
	if (m_RedoList.empty ())
		pWin->ActivateActionWidget ("/MainMenu/EditMenu/Redo", false);
	m_UndoList.push_front (m_CurNode);
	pWin->ActivateActionWidget ("/MainMenu/EditMenu/Undo", true);
	unsigned start, end;
	char* tmp = (char*) xmlGetProp (node, (xmlChar*) "start-sel");
	start = (int) strtoul (tmp, NULL, 10);
	xmlFree (tmp);
	tmp = (char*) xmlGetProp (node, (xmlChar*) "end-sel");
	end = (int) strtoul (tmp, NULL, 10);
	xmlFree (tmp);
	gnome_canvas_pango_set_selection_bounds (m_Active, start, end);
	m_CurNode = node;
	return true;
}

void gcpTextTool::PushNode (xmlNodePtr node)
{
	gcp::Window *pWin = m_pView->GetDoc ()->GetWindow ();
	while (!m_RedoList.empty ()) {
		xmlFree (m_RedoList.front ());
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
	gcp::cleared = false;
	if (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))
			m_pView->GetDoc ()->GetWindow ()->ActivateActionWidget ("/MainMenu/EditMenu/Paste", true);
}

void gcpTextTool::UpdateAttributeList ()
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
	m_Underline = PANGO_UNDERLINE_NONE;
	m_Strikethrough = false;
	m_Color = RGBA_BLACK;
	if (m_Active) {
		PangoLayout *layout;
		g_object_get (m_Active, "layout", &layout, NULL);
		PangoAttrList *l = pango_layout_get_attributes (layout);
		if (l)
			pango_attr_list_filter (l, (PangoAttrFilterFunc) filter_attribute, this);
	}
	// select the found face
	GtkTreeIter iter;
	char const *buf;
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_FamilyList), &iter);
	do {
		gtk_tree_model_get (GTK_TREE_MODEL (m_FamilyList), &iter, 0, &buf, -1);
		if (!strcmp (m_FamilyName, buf)) {
			m_Dirty = true;
			GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (m_FamilyList), &iter);
			gtk_tree_view_set_cursor (m_FamilyTree, path, NULL, FALSE);
			gtk_tree_path_free (path);
			break;
		}
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (m_FamilyList), &iter));
	// Update other widgets
	SetSizeFull (true);
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
	BuildAttributeList ();
}

void gcpTextTool::BuildAttributeList ()
{
	if (!m_Active)
		return;
	PangoAttrList *l = pango_attr_list_new ();
	pango_attr_list_insert (l, pango_attr_family_new (m_FamilyName));
	pango_attr_list_insert (l, pango_attr_style_new (m_Style));
	pango_attr_list_insert (l, pango_attr_weight_new (m_Weight));
	pango_attr_list_insert (l, pango_attr_stretch_new (m_Stretch));
	pango_attr_list_insert (l, pango_attr_variant_new (m_Variant));
	pango_attr_list_insert (l, pango_attr_size_new (m_Size));
	pango_attr_list_insert (l, pango_attr_underline_new (m_Underline));
	pango_attr_list_insert (l, pango_attr_strikethrough_new (m_Strikethrough));
	pango_attr_list_insert (l, pango_attr_rise_new (m_Rise));
	pango_attr_list_insert (l, pango_attr_foreground_new (UINT_RGBA_R (m_Color) * 0x101, UINT_RGBA_G (m_Color) * 0x101, UINT_RGBA_B (m_Color) * 0x101));
	gnome_canvas_pango_set_insert_attrs (m_Active, l);
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

static void on_size_activate (GtkEntry *entry, gcpTextTool *tool)
{
	tool->OnSizeChanged ();
}

static void on_size_focus_out (GtkEntry *entry, GdkEventFocus *event, gcpTextTool *tool)
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
	GladeXML *xml = glade_xml_new (GLADEDIR"/fontsel.glade", "fontsel", GETTEXT_PACKAGE);
	PangoFontFamily **families;
	int i, nb;
	gcp::Theme *pTheme = m_pApp->GetActiveDocument ()->GetTheme ();
	// Initialize faces list
	m_FaceList = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (m_FaceList), 0, GTK_SORT_ASCENDING);
	m_FacesTree = (GtkTreeView *) glade_xml_get_widget (xml, "style");
	gtk_tree_view_set_model (m_FacesTree, GTK_TREE_MODEL (m_FaceList));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (m_FacesTree, column);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (m_FacesTree);
	m_FaceSel = selection;
	m_FaceSignal = g_signal_connect (m_FaceSel, "changed", G_CALLBACK (on_select_face), this);
	// Initialize sizes list
	m_SizeList = gtk_list_store_new (1, G_TYPE_INT);
	m_SizesTree = (GtkTreeView *) glade_xml_get_widget (xml, "size-list");
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
	m_SizeEntry = (GtkEntry*) glade_xml_get_widget (xml, "size-entry");
	g_signal_connect (m_SizeEntry, "activate", G_CALLBACK (on_size_activate), this);
	g_signal_connect_after (m_SizeEntry, "focus_out_event", G_CALLBACK (on_size_focus_out), this);
	SetSizeFull (true);
	PangoContext *pc = gtk_widget_get_pango_context (GTK_WIDGET (m_SizeEntry));
	pango_context_list_families (pc, &families, &nb);
	m_FamilyList = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (m_FamilyList), 0, GTK_SORT_ASCENDING);
	m_FamilyTree = (GtkTreeView *) glade_xml_get_widget (xml, "family");
	gtk_tree_view_set_model (m_FamilyTree, GTK_TREE_MODEL (m_FamilyList));
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (m_FamilyTree, column);
	GtkTreePath *path = NULL;
	string name;
	bool default_found = false;
	for (i = 0; i < nb; i++) {
		PangoFontFace **faces;
		int *sizes, n;
		pango_font_family_list_faces (families[i], &faces, &n);
		if (n <= 0)
			continue;
		pango_font_face_list_sizes (faces[0], &sizes, &n);
		if (n > 0) // Do not use bitmap fonts
			continue;
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
	}
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
	m_UnderlineBox = GTK_COMBO_BOX (glade_xml_get_widget (xml, "underline"));
	gtk_combo_box_set_active (m_UnderlineBox, 0);
	m_UnderlineSignal = g_signal_connect (G_OBJECT (m_UnderlineBox), "changed", G_CALLBACK (on_underline_changed), this);
	m_StrikethroughBtn = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "strikethrough"));
	m_StrikethroughSignal = g_signal_connect (G_OBJECT (m_StrikethroughBtn), "toggled", G_CALLBACK (on_strikethrough_toggled), this);
	m_RiseButton = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "rise"));
	m_RiseSignal = g_signal_connect (G_OBJECT (m_RiseButton), "value-changed", G_CALLBACK (on_rise_changed), this);
	m_ColorSelector = GO_SELECTOR (go_color_selector_new (RGBA_BLACK, RGBA_BLACK, "fore"));
	go_color_selector_set_allow_alpha (m_ColorSelector, false);
	m_ForeSignal = g_signal_connect (G_OBJECT (m_ColorSelector), "activate", G_CALLBACK (on_fore_color_changed), this);
	gtk_widget_show (GTK_WIDGET (m_ColorSelector));
	gtk_table_attach (GTK_TABLE (glade_xml_get_widget (xml, "table2")), GTK_WIDGET (m_ColorSelector), 1, 2, 0, 1, (GtkAttachOptions) 0, (GtkAttachOptions) 0, 0, 0);
	return glade_xml_get_widget (xml, "fontsel");
}

void gcpTextTool::OnSelectFamily (GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter, selected;
	char const *name;
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
	gtk_tree_model_get (model, &iter, 0, &m_FamilyName, -1);
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
	g_signal_handler_unblock (m_FaceSel, m_FaceSignal);
	GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (m_FaceList), &selected);
	if (path) {
		gtk_tree_selection_select_path (GTK_TREE_SELECTION (m_FaceSel), path);
		gtk_tree_path_free (path);
	} else {
		//TODO: choose a face when default is not available
	}
	if (m_Active) {
		PangoAttrList *l = pango_attr_list_new ();
		pango_attr_list_insert (l, pango_attr_family_new (m_FamilyName));
		pango_attr_list_insert (l, pango_attr_style_new (m_Style));
		pango_attr_list_insert (l, pango_attr_weight_new (m_Weight));
		pango_attr_list_insert (l, pango_attr_stretch_new (m_Stretch));
		pango_attr_list_insert (l, pango_attr_variant_new (m_Variant));
		gnome_canvas_pango_apply_attrs_to_selection (m_Active, l);
		pango_attr_list_unref (l);
	}
}

void gcpTextTool::OnSelectFace (GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	char const *name;
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
	gtk_tree_model_get (model, &iter, 0, &name, -1);
	PangoFontFace *face = m_Faces[name];
	PangoFontDescription *desc = pango_font_face_describe (face);
	m_Style = pango_font_description_get_style (desc);
	m_Weight = pango_font_description_get_weight (desc);
	m_Variant = pango_font_description_get_variant (desc);
	m_Stretch = pango_font_description_get_stretch (desc);
	pango_font_description_free (desc);
	BuildAttributeList ();
	if (m_Active) {
		PangoAttrList *l = pango_attr_list_new ();
		pango_attr_list_insert (l, pango_attr_style_new (m_Style));
		pango_attr_list_insert (l, pango_attr_weight_new (m_Weight));
		pango_attr_list_insert (l, pango_attr_stretch_new (m_Stretch));
		pango_attr_list_insert (l, pango_attr_variant_new (m_Variant));
		gnome_canvas_pango_apply_attrs_to_selection (m_Active, l);
		pango_attr_list_unref (l);
	}
}

void gcpTextTool::OnSelectSize (int size)
{
	m_Size = size;
	SetSizeFull (false);
}

void gcpTextTool::OnSizeChanged ()
{
	char const *text = gtk_entry_get_text (m_SizeEntry);
	m_Size = (int) (MAX (0.1, atof (text) * PANGO_SCALE + 0.5));
	SetSizeFull (true);
}

void gcpTextTool::SetSizeFull (bool update_list)
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
	BuildAttributeList ();
	if (m_Active) {
		PangoAttrList *l = pango_attr_list_new ();
		pango_attr_list_insert (l, pango_attr_size_new (m_Size));
		gnome_canvas_pango_apply_attrs_to_selection (m_Active, l);
		pango_attr_list_unref (l);
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

int gcpTextTool::GetIndex ()
{
	return gnome_canvas_pango_get_cur_index (m_Active);
}

void gcpTextTool::OnUnderlineChanged (unsigned underline)
{
	m_Underline = (PangoUnderline) underline;
	BuildAttributeList ();
	if (m_Active) {
		PangoAttrList *l = pango_attr_list_new ();
		pango_attr_list_insert (l, pango_attr_underline_new (m_Underline));
		gnome_canvas_pango_apply_attrs_to_selection (m_Active, l);
		pango_attr_list_unref (l);
	}
}

void gcpTextTool::OnStriketroughToggled (bool strikethrough)
{
	m_Strikethrough = strikethrough;
	BuildAttributeList ();
	if (m_Active) {
		PangoAttrList *l = pango_attr_list_new ();
		pango_attr_list_insert (l, pango_attr_strikethrough_new (m_Strikethrough));
		gnome_canvas_pango_apply_attrs_to_selection (m_Active, l);
		pango_attr_list_unref (l);
	}
}

void gcpTextTool::OnPositionChanged (int position)
{
	m_Rise = position * PANGO_SCALE;
	BuildAttributeList ();
}

void gcpTextTool::OnForeColorChanged (GOColor color)
{
	m_Color = color;
	BuildAttributeList ();
	if (m_Active) {
		PangoAttrList *l = pango_attr_list_new ();
		pango_attr_list_insert (l, pango_attr_foreground_new (UINT_RGBA_R (m_Color) * 0x101, UINT_RGBA_G (m_Color) * 0x101, UINT_RGBA_B (m_Color) * 0x101));
		gnome_canvas_pango_apply_attrs_to_selection (m_Active, l);
		pango_attr_list_unref (l);
	}
}
