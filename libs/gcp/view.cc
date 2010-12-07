// -*- C++ -*-

/* 
 * GChemPaint library
 * view.cc
 *
 * Copyright (C) 2001-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "atom.h"
#include "bond.h"
#include "document.h"
#include "settings.h"
#include "text.h"
#include "theme.h"
#include "tool.h"
#include "tools.h"
#include "view.h"
#include "widgetdata.h"
#include "window.h"
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/text.h>
#include <gsf/gsf-output-gio.h>
#include <gsf/gsf-output-memory.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>
#include <pango/pango-context.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>
#include <clocale>
#include <cmath>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>

using namespace gcu;
using namespace std;

namespace gcp {

static bool on_destroy (GtkWidget *widget, View * pView)
{
	pView->OnDestroy (widget);
	return true;
}

void on_receive (GtkClipboard *clipboard, GtkSelectionData *selection_data, View * pView)
{
	pView->OnReceive (clipboard, selection_data);
}

View::View (Document *pDoc, bool Embedded):
	Client ()
{
	m_pDoc = pDoc;
	Theme *pTheme = pDoc->GetTheme ();
	m_PangoFontDesc = pango_font_description_new ();
	pango_font_description_set_family (m_PangoFontDesc, pTheme->GetFontFamily ());
	pango_font_description_set_style (m_PangoFontDesc, pTheme->GetFontStyle ());
	pango_font_description_set_weight (m_PangoFontDesc, pTheme->GetFontWeight ());
	pango_font_description_set_variant (m_PangoFontDesc, pTheme->GetFontVariant ());
	pango_font_description_set_stretch (m_PangoFontDesc, pTheme->GetFontStretch ());
	pango_font_description_set_size (m_PangoFontDesc, pTheme->GetFontSize ());
	m_sFontName = pango_font_description_to_string (m_PangoFontDesc);
	m_PangoSmallFontDesc = pango_font_description_new ();
	pango_font_description_set_family (m_PangoSmallFontDesc, pTheme->GetFontFamily ());
	pango_font_description_set_style (m_PangoSmallFontDesc, pTheme->GetFontStyle ());
	pango_font_description_set_weight (m_PangoSmallFontDesc, pTheme->GetFontWeight ());
	pango_font_description_set_variant (m_PangoSmallFontDesc, pTheme->GetFontVariant ());
	pango_font_description_set_stretch (m_PangoSmallFontDesc, pTheme->GetFontStretch ());
	pango_font_description_set_size (m_PangoSmallFontDesc, pTheme->GetFontSize () * 2 / 3);
	m_sSmallFontName = pango_font_description_to_string (m_PangoSmallFontDesc);
	m_width = 400;
	m_height = 300;
	m_ActiveRichText = NULL;
	m_bEmbedded = Embedded;
	m_UIManager = gtk_ui_manager_new ();
	m_Dragging = false;
	m_pWidget = NULL;
	m_CurObject = NULL;
	m_CurAtom = NULL;
	PangoLayout *layout = pango_layout_new (gccv::Text::GetContext ());
	pango_layout_set_text (layout, "C", 1);
	pango_layout_set_font_description (layout, m_PangoFontDesc);
	PangoRectangle rect;
	pango_layout_get_extents (layout, &rect, NULL);
	m_CHeight =  double (rect.height) / PANGO_SCALE / 2.0;
	m_BaseLineOffset =  m_CHeight / m_pDoc->GetTheme ()->GetZoomFactor ();
	pango_layout_set_text (layout, "H", 1);
	pango_layout_get_extents (layout, &rect, NULL);
	m_HWidth = (double (rect.width) / 2.0 + rect.x) / PANGO_SCALE;
	g_object_unref (layout);
}

View::~View ()
{
	if (m_sFontName)
		g_free (m_sFontName);
	if (m_sSmallFontName)
		g_free (m_sSmallFontName);
	pango_font_description_free (m_PangoFontDesc);
	pango_font_description_free (m_PangoSmallFontDesc);
	g_object_unref (m_UIManager);
	// we don't need to delete the canvas, since destroying the widget does the job.
}

void View::AddObject (Object *pObject)
{
	gccv::ItemClient *client = dynamic_cast <gccv::ItemClient *> (pObject);
	if (client)
		client->AddItem ();
	// now, add the children
	map<string, Object*>::iterator i;
	Object *child = pObject->GetFirstChild (i);
	while (child) {
		AddObject (child);
		child = pObject->GetNextChild (i);
	}
}

GtkWidget* View::CreateNewWidget ()
{
	if (m_Canvas)
		return m_Canvas->GetWidget ();
	m_Canvas = new gccv::Canvas (this);
	m_Canvas->SetBackgroundColor (GO_COLOR_WHITE);
	m_pWidget = m_Canvas->GetWidget ();
	m_Canvas->SetGap (3.); // FIXME: make this configurable
	if (m_pWidget) {
		g_object_set_data (G_OBJECT (m_pWidget), "view", this);
		g_object_set_data (G_OBJECT (m_pWidget), "doc", m_pDoc);
		m_pData = new WidgetData ();
		m_pData->Canvas = m_pWidget;
		g_object_set_data (G_OBJECT (m_pWidget), "data", m_pData);
		m_pData->m_View = this;
		m_pData->Zoom = 1.0;
		g_signal_connect (G_OBJECT (m_pWidget), "destroy", G_CALLBACK (on_destroy), this);
		gtk_widget_show (m_pWidget);
		UpdateFont ();
	}
	return m_pWidget;
}

void View::OnDestroy (GtkWidget* widget)
{
	if (m_bEmbedded) {
		m_Widgets.remove (widget);
	} else
		delete m_pDoc;
	delete (WidgetData*) g_object_get_data (G_OBJECT (widget), "data");
}

void View::Update (Object *pObject)
{
	if (!m_pWidget)
		return;
	gccv::ItemClient *client = dynamic_cast <gccv::ItemClient *> (pObject);
	if (client)
		client->UpdateItem ();
	// now, add the children
	map<string, Object*>::iterator i;
	Object *child = pObject->GetFirstChild (i);
	while (child) {
		Update (child);
		child = pObject->GetNextChild (i);
	}
}

double View::GetZoomFactor ()
{
	return m_pDoc->GetTheme ()->GetZoomFactor ();
}

void View::UpdateFont ()
{
	PangoLayout* pl = pango_layout_new (gccv::Text::GetContext ());
	pango_layout_set_font_description (pl, m_PangoFontDesc);
	PangoRectangle rect;
	pango_layout_set_text (pl, "lj", 2);
	pango_layout_get_extents (pl, &rect, NULL);
	m_dFontHeight = rect.height / PANGO_SCALE;
	pango_layout_set_text (pl, "C", 1);
	pango_layout_get_extents (pl, &rect, NULL);
	m_CHeight = (double) rect.height / PANGO_SCALE / 2.0;
	m_BaseLineOffset =  m_CHeight / m_pDoc->GetTheme ()->GetZoomFactor ();
	pango_layout_set_text (pl, "H", 1);
	pango_layout_get_extents (pl, &rect, NULL);
	m_HWidth = (double (rect.width) / 2.0 + rect.x) / PANGO_SCALE;
	g_object_unref (G_OBJECT (pl));
}
	
void View::Remove (Object* pObject)
{
	if (!m_pWidget)
		return;
	Object* pObj = pObject->GetMolecule ();
	if (pObj)
		m_pData->SelectedObjects.remove (pObj);
	else
		m_pData->SelectedObjects.remove (pObject);
	gccv::ItemClient *client = dynamic_cast <gccv::ItemClient *> (pObject);
	if (client && client->GetItem ())
		delete client->GetItem ();
}

void View::OnDeleteSelection (GtkWidget* w)
{
	m_pWidget = w;
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	Object *parent;
	if (!pActiveTool->DeleteSelection ()) {
		m_pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
		Object *pObject, *Group;
		set<string> ModifiedObjects;
		bool modify = false;
		list<Object *>::iterator i, iend = m_pData->SelectedObjects.end ();
		// first search if we are deleting top objects or not
		for (i = m_pData->SelectedObjects.begin (); i != iend; i++)
			if ((*i)->GetGroup ()) {
				modify = true;
				break;
			}
		Operation *Op = m_pDoc->GetNewOperation (((modify)?
								GCP_MODIFY_OPERATION: GCP_DELETE_OPERATION));
		while (!m_pData->SelectedObjects.empty ()) {
			pObject = m_pData->SelectedObjects.front ();
			Group = pObject->GetGroup ();
			if (Group && 
				(ModifiedObjects.find (Group->GetId ()) == ModifiedObjects.end ())) {
				Op->AddObject (Group);
				ModifiedObjects.insert (Group->GetId ());
			} else
				Op->AddObject (pObject);
			pObject->Lock ();
			parent = pObject->GetParent ();
			m_pData->Unselect (pObject);
			m_pDoc->Remove (pObject);
			if (parent)
				parent->EmitSignal (OnChangedSignal);
		}
		m_pData->ClearSelection ();
		set<string>::iterator k, kend = ModifiedObjects.end();
		for (k = ModifiedObjects.begin(); k != kend; k++) {
			pObject = m_pDoc->GetDescendant ((*k).c_str ());
			if (pObject)
				Op->AddObject (pObject, 1);
		}
	}
	m_pDoc->FinishOperation ();
	Window *Win = m_pDoc->GetWindow ();
	if (Win) {
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", false);
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", false);
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", false);
	}
}

GtkTargetEntry const targets[] = {
	{(char *) GCHEMPAINT_ATOM_NAME,  0, GCP_CLIPBOARD_NATIVE},
	{(char *) "image/svg",  0, GCP_CLIPBOARD_SVG},
	{(char *) "image/svg+xml",  0, GCP_CLIPBOARD_SVG_XML},
	{(char *) "image/x-eps",  0, GCP_CLIPBOARD_EPS},
	{(char *) "image/png",  0, GCP_CLIPBOARD_PNG},
	{(char *) "image/jpeg",  0, GCP_CLIPBOARD_JPEG},
	{(char *) "image/bmp",  0, GCP_CLIPBOARD_BMP},
	{(char *) "UTF8_STRING", 0, GCP_CLIPBOARD_UTF8_STRING},
	{(char *) "STRING", 0, GCP_CLIPBOARD_STRING}
};

void View::OnCopySelection (GtkWidget* w, GtkClipboard* clipboard)
{
	if (m_pDoc->GetAllowClipboard ()) {
		Application * pApp = m_pDoc->GetApplication ();
		Tool* pActiveTool = pApp->GetActiveTool ();
		m_pWidget = w;
		m_pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
		if (!pActiveTool->CopySelection (clipboard))
			m_pData->Copy (clipboard);
	}
}

void View::OnReceive (GtkClipboard* clipboard, GtkSelectionData* selection_data)
{
	int length = gtk_selection_data_get_length (selection_data);
	char const *data = reinterpret_cast <char const *> (gtk_selection_data_get_data (selection_data));
	if ((length <= 0) || !data)
		return;
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &ClipboardDataType: &ClipboardDataType1;
	g_return_if_fail (gtk_selection_data_get_target (selection_data) == gdk_atom_intern (targets[*DataType].target, FALSE));
	if (pActiveTool->OnReceive (clipboard, selection_data, *DataType))
		return;
	else if (pActiveTool) {
		pApp->ActivateTool ("Select", true);
		pActiveTool = pApp->GetActiveTool ();
	}
	if (!pActiveTool || (pActiveTool != pApp->GetTool ("Select")))
		return;
	xmlDocPtr xml;
	m_pData->UnselectAll ();
	switch (*DataType) {
	case GCP_CLIPBOARD_NATIVE:
		xml = xmlParseMemory (data, length);
		m_pDoc->AddData (xml->children->children);
		xmlFreeDoc (xml);
		break;
	case GCP_CLIPBOARD_UTF8_STRING: {
			Text* text = new Text ();
			text->SetText (data);
			text->OnChanged (true);
			m_pDoc->AddObject (text);
			m_pData->SetSelected (text);
		}
		break;
	case GCP_CLIPBOARD_STRING: {
			Text* text = new Text ();
			if (!g_utf8_validate (data, length, NULL)) {
				gsize r, w;
				gchar* newstr = g_locale_to_utf8 ((const char*) data, length, &r, &w, NULL);
				text->SetText (newstr);
				g_free (newstr);
			} else
				text->SetText (data);
			text->OnChanged (true);
			m_pDoc->AddObject (text);
			m_pData->SetSelected (text);
		}
		break;
	}
	gccv::Rect rect;
	double dx, dy;
	while (gtk_events_pending ())
		gtk_main_iteration ();
	m_pDoc->AbortOperation ();
	m_pData->GetSelectionBounds (rect);
	if (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD)) {
		//center the pasted data at the center of the visible area
		if (m_bEmbedded) {
			// might this still occur? what does happen if not everything is visible?
			dx = m_width / 2. - (rect.x0 + rect.x1) / 2.;
			dy = m_height / 2. - (rect.y0 + rect.y1) / 2.;
		} else {
			GtkAdjustment *horiz, *vert;
			GtkWidget* parent = gtk_widget_get_parent (m_pWidget);
			horiz = gtk_viewport_get_hadjustment (GTK_VIEWPORT (parent));
			vert = gtk_viewport_get_vadjustment (GTK_VIEWPORT (parent));
			dx = gtk_adjustment_get_value (horiz) + gtk_adjustment_get_page_size (horiz) / 2.  - (rect.x0 + rect.x1) / 2.;
			dy = gtk_adjustment_get_value (vert) + gtk_adjustment_get_page_size (vert) / 2.  - (rect.y0 + rect.y1) / 2.;
		}
	} else {
		//center the pasted data at the mouse click position
		dx = m_lastx - (rect.x0 + rect.x1) / 2.;
		dy = m_lasty - (rect.y0 + rect.y1) / 2.;
	}
	m_pData->MoveSelection (dx, dy);
	Tool *pTool = pApp->GetTool ("Select");
	if (pTool)
		pTool->AddSelection (m_pData);
	m_pDoc->PopOperation ();
	Operation* pOp = m_pDoc->GetNewOperation (GCP_ADD_OPERATION);
	std::list<Object*>::iterator i, end = m_pData->SelectedObjects.end();
	for (i = m_pData->SelectedObjects.begin(); i != end; i++) 
		pOp->AddObject (*i);
	m_pDoc->FinishOperation ();
}

void View::OnPasteSelection (GtkWidget* w, GtkClipboard* clipboard)
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	if (pActiveTool->PasteSelection (clipboard))
		return;
	m_pWidget = w;
	m_pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &ClipboardDataType: &ClipboardDataType1;
	GdkAtom targets_atom  = gdk_atom_intern (targets[*DataType].target, FALSE);
	gtk_clipboard_request_contents (clipboard, targets_atom,  (GtkClipboardReceivedFunc) on_receive, this);
}

void View::OnCutSelection (GtkWidget* w, GtkClipboard* clipboard)
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	if (!pActiveTool->CutSelection(clipboard)) {
		OnCopySelection (w, clipboard);
		OnDeleteSelection (w);
	}
	Window *Win = m_pDoc->GetWindow ();
	if (Win) {
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", false);
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", false);
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", false);
	}
}

static void do_set_symbol (GtkAction *action, Object *obj)
{
	Document* pDoc = static_cast<Document*> (obj->GetDocument ());
	Application *App = static_cast<Application*> (pDoc->GetApplication ());
	Tools *tools = static_cast<Tools*> (App->GetDialog ("tools"));
	int Z = Element::Z (gtk_action_get_name(action));
	tools->SetElement (Z);
	if (obj->GetType () == AtomType) {
		Atom *atom = static_cast<Atom*> (obj);
		if (atom->GetZ () == Z || atom->GetZ () == 0)
			return;
		Object *group = obj->GetGroup ();
		Operation *op = pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
		op->AddObject (group);
		atom->SetZ (Z);
		// set all bonds as dirty
		map<gcu::Atom*, gcu::Bond*>::iterator i;
		Bond *bond = reinterpret_cast <Bond *> (atom->GetFirstBond (i));
		while (bond) {
			bond->SetDirty ();
			bond = reinterpret_cast <Bond *> (atom->GetNextBond (i));
		}
		pDoc->GetView ()->Update (obj);
		op->AddObject (group, 1);
		pDoc->FinishOperation ();
	}
}

bool View::OnKeyPress (GtkWidget* w, GdkEventKey* event)
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	if (pActiveTool->OnKeyPress (event))
		return true;
	switch (event->keyval) {
	case GDK_Delete:
	case GDK_Clear:
	case GDK_BackSpace:
		OnDeleteSelection(w);
		return true;
	case GDK_Shift_L:
	case GDK_Shift_R:
		if (pActiveTool)
			pActiveTool->OnKeyPressed (GDK_SHIFT_MASK);
		return true;
	case GDK_Control_L:
	case GDK_Control_R:
		if (pActiveTool)
			pActiveTool->OnKeyPressed (GDK_CONTROL_MASK);
		return true;
	case GDK_Alt_L:
	case GDK_Alt_R:
		if (pActiveTool)
			pActiveTool->OnKeyPressed (GDK_MOD1_MASK);//FIXME: might be not portable
		return true;
	case GDK_ISO_Level3_Shift:
		if (pActiveTool)
			pActiveTool->OnKeyPressed (GDK_MOD5_MASK);//FIXME: might be not portable
		return true;
	case GDK_Caps_Lock:
		if (pActiveTool) {
			if (event->state & GDK_LOCK_MASK)
				pActiveTool->OnKeyReleased (GDK_LOCK_MASK);
			else
				pActiveTool->OnKeyPressed (GDK_LOCK_MASK);
		}
		return true;
	default: {
			if (m_Dragging || (event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0 || event->keyval > 127)
				break;
			// Now try to get the atom at the cursor
			unsigned min_bonds = (m_CurAtom)? m_CurAtom->GetTotalBondsNumber (): 0;
			int Z = 0;
			switch (event->keyval) {
			case GDK_a:
				Z = 13;
				break;
			case GDK_b:
				Z = 5;
				break;
			case GDK_c:
				Z = 6;
				break;
			case GDK_d:
				Z = 11;
				break;
			case GDK_e:
				Z = 34;
				break;
			case GDK_f:
				Z = 9;
				break;
			case GDK_g:
				Z = 32;
				break;
			case GDK_h:
				Z = 1;
				break;
			case GDK_i:
				Z = 53;
				break;
			case GDK_j:
				Z = 22;
				break;
			case GDK_k:
				Z = 19;
				break;
			case GDK_l:
				Z = 3;
				break;
			case GDK_m:
				Z = 12;
				break;
			case GDK_n:
				Z = 7;
				break;
			case GDK_o:
				Z = 8;
				break;
			case GDK_p:
				Z = 15;
				break;
			case GDK_q:
				Z = 14;
				break;
			case GDK_r:
				Z = 35;
				break;
			case GDK_s:
				Z = 16;
				break;
			case GDK_t:
				Z = 78;
				break;
			case GDK_u:
				Z = 29;
				break;
			case GDK_v:
				Z = 23;
				break;
			case GDK_w:
				Z = 74;
				break;
			case GDK_x:
				Z = 17;
				break;
			case GDK_y:
				Z = 39;
				break;
			case GDK_z:
				Z = 40;
				break;
			}
			if (Z) {
				if (!m_CurAtom) {
					Tools *tools = static_cast<Tools*> (pApp->GetDialog ("tools"));
					tools->SetElement (Z);
				} else if (m_CurAtom->GetZ () != Z && Element::GetElement (Z)->GetMaxBonds () >= min_bonds) {
					Object *group = m_CurAtom->GetGroup ();
					Operation *op = m_pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
					op->AddObject (group);
					m_CurAtom->SetZ (Z);
					Update (m_CurAtom);
					// set all bonds as dirty
					map<gcu::Atom*, gcu::Bond*>::iterator i;
					Bond *bond = reinterpret_cast <Bond *> (m_CurAtom->GetFirstBond (i));
					while (bond) {
						bond->SetDirty ();
						bond = reinterpret_cast <Bond *> (m_CurAtom->GetNextBond (i));
					}
					op->AddObject (group, 1);
					m_pDoc->FinishOperation ();
				}
				return true;
			}
			// build a contextual menu of all known symbols starting with the pressed key.
			map<string, Element*> entries;
			string symbol;
			unsigned key = gdk_keyval_to_upper (event->keyval);
			for (int i = 1; i <= 128; i++) {
				Element *elt = Element::GetElement (i);
				if (!elt || elt->GetMaxBonds () < min_bonds)
					continue;
				symbol = elt->GetSymbol ();
				if ((unsigned char) symbol[0] == key) {
					entries[symbol] = elt;
				}
			}
			if (entries.empty ())
				break;
			map<string, Element*>::iterator i, end = entries.end ();
			g_object_unref (m_UIManager);
			m_UIManager = gtk_ui_manager_new ();
			GtkActionGroup *group = gtk_action_group_new ("element");
			GtkAction *action;
			string ui;
			for (i = entries.begin (); i != end; i++) {
				symbol = (*i).first;
				symbol.insert (((symbol.length () > 1)? 1: 0), "_");
				action = GTK_ACTION (gtk_action_new ((*i).second->GetSymbol (), symbol.c_str (), (*i).second->GetName (), NULL));
				g_signal_connect (action, "activate", G_CALLBACK (do_set_symbol), (m_CurAtom)? static_cast<Object*> (m_CurAtom): static_cast<Object*> (m_pDoc));
				gtk_action_group_add_action (group, action);
				g_object_unref (action);
				ui = string ("<ui><popup><menuitem action='") + (*i).second->GetSymbol () + "'/></popup></ui>";
				gtk_ui_manager_add_ui_from_string (m_UIManager, ui.c_str (), -1, NULL);
			}
			gtk_ui_manager_insert_action_group (m_UIManager, group, 0);
			g_object_unref (group);
			GtkWidget *w = gtk_ui_manager_get_widget (m_UIManager, "/popup");
			gtk_menu_popup (GTK_MENU (w), NULL, NULL, NULL, NULL, 3,  gtk_get_current_event_time ());
			break;
		}
	}
	return false;
}

bool View::OnKeyRelease (G_GNUC_UNUSED GtkWidget* w, GdkEventKey* event)
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	if (pActiveTool->OnKeyRelease (event))
		return true;
	switch(event->keyval) {
	case GDK_Shift_L:
	case GDK_Shift_R:
		if (pActiveTool)
			pActiveTool->OnKeyReleased (GDK_SHIFT_MASK);
		return true;
	case GDK_Control_L:
	case GDK_Control_R:
		if (pActiveTool)
			pActiveTool->OnKeyReleased (GDK_CONTROL_MASK);
		return true;
	case GDK_Alt_L:
	case GDK_Alt_R:
		if (pActiveTool)
			pActiveTool->OnKeyReleased (GDK_MOD1_MASK);//FIXME: might be not portable
		return true;
	case 0:
		// Seems that when releasing the AltGr key, the keyval is 0 !!!
		if (!(event->state & GDK_MOD5_MASK))
			break;
	case GDK_ISO_Level3_Shift:
		if (pActiveTool)
			pActiveTool->OnKeyReleased (GDK_MOD5_MASK);//FIXME: might be not portable
		return true;
	default:
		break;
	}
	return false;
}

void View::SetTextActive (gccv::Text* item)
{
	m_ActiveRichText = item;
	m_Dragging = false;
}

bool View::PrepareUnselect ()
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	return (pActiveTool)? pActiveTool->NotifyViewChange (): true;
}

void View::OnSelectAll ()
{
	Application *pApp = m_pDoc->GetApplication();
	Tool *pActiveTool = pApp->GetTool ("Select");
	if (pActiveTool)
		pApp->ActivateTool ("Select", true);
	m_pData->SelectAll ();
	if (pActiveTool)
		pActiveTool->AddSelection (m_pData);
}

static gboolean do_save_image (const gchar *buf, gsize count, GError **error, gpointer data)
{
	GOutputStream *output = (GOutputStream *) data;
	while (count) {
		count -= g_output_stream_write (output, buf, count, NULL, error);
		if (*error)
			return false;
	}
	return true;
}

static cairo_status_t cairo_write_func (void *closure, const unsigned char *data, unsigned int length)
{
	gboolean result;
	GsfOutput *output = GSF_OUTPUT (closure);

	result = gsf_output_write (output, length, data);

	return result ? CAIRO_STATUS_SUCCESS : CAIRO_STATUS_WRITE_ERROR;
}

void View::ExportImage (string const &filename, const char* type, int resolution)
{
	gccv::Rect rect;
	m_pData->GetObjectBounds (m_pDoc, &rect);
	m_pData->ShowSelection (false);
	int w = (int) (ceil (rect.x1) - floor (rect.x0)), h = (int) (ceil (rect.y1) - floor (rect.y0));
	if (!strcmp (type, "eps") || !strcmp (type, "ps") || !strcmp (type, "pdf")) {
		GError *error = NULL;
		GsfOutput *output = gsf_output_gio_new_for_uri (filename.c_str (), &error);
		if (error) {
			GtkWidget* message = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (m_pWidget)), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Could not create stream!\n%s"), error->message);
			gtk_dialog_run (GTK_DIALOG (message));
			gtk_widget_destroy (message);
			g_error_free (error);
		}
		gccv::Rect rect;
		m_pData->GetObjectBounds (m_pDoc, &rect);
		cairo_surface_t *surface = NULL;
		if (!strcmp (type, "pdf"))
			surface = cairo_pdf_surface_create_for_stream (cairo_write_func, output, w * .75, h * .75);
		else {
			surface = cairo_ps_surface_create_for_stream (cairo_write_func, output, w * .75, h * .75);
			if (!strcmp (type, "eps"))
				cairo_ps_surface_set_eps (surface, TRUE);
		}
		cairo_t *cr = cairo_create (surface);
		cairo_scale (cr, .75, .75);
		cairo_translate (cr, -rect.x0, -rect.y0);
		cairo_surface_destroy (surface);
		Render (cr);
		cairo_destroy (cr);
		g_object_unref (output);
	} else if (!strcmp (type, "svg")) {
		GError *error = NULL;
		GsfOutput *output = gsf_output_gio_new_for_uri (filename.c_str (), &error);
		if (error) {
			GtkWidget* message = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (m_pWidget)), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Could not create stream!\n%s"), error->message);
			gtk_dialog_run (GTK_DIALOG (message));
			gtk_widget_destroy (message);
			g_error_free (error);
		}
		gccv::Rect rect;
		m_pData->GetObjectBounds (m_pDoc, &rect);
		cairo_surface_t *surface = cairo_svg_surface_create_for_stream (cairo_write_func, output, w, h);
		cairo_t *cr = cairo_create (surface);
		cairo_translate (cr, -rect.x0, -rect.y0);
		cairo_surface_destroy (surface);
		Render (cr);
		cairo_destroy (cr);
		g_object_unref (output);
	} else {
		GdkPixbuf *pixbuf = BuildPixbuf (resolution);
		GFile *file = g_vfs_get_file_for_uri (g_vfs_get_default (), filename.c_str ());
		GError *error = NULL;
		GFileOutputStream *output = g_file_create (file, G_FILE_CREATE_NONE, NULL, &error);
		if (!error)
			gdk_pixbuf_save_to_callbackv (pixbuf, do_save_image, output, type, NULL, NULL, &error);
		if (error) {
			fprintf (stderr, _("Unable to save image file: %s\n"), error->message);
			g_error_free (error);
		}
		g_object_unref (file);
		g_object_unref (pixbuf);
	}
	m_pData->ShowSelection (true);
}

char *View::BuildSVG ()
{
	gccv::Rect rect;
	m_pData->GetObjectBounds (m_pDoc, &rect);
	int w = (int) (ceil (rect.x1) - floor (rect.x0)), h = (int) (ceil (rect.y1) - floor (rect.y0));
	GsfOutput *output = gsf_output_memory_new ();
	cairo_surface_t *surface = cairo_svg_surface_create_for_stream (cairo_write_func, output, w, h);
	cairo_t *cr = cairo_create (surface);
	cairo_translate (cr, -rect.x0, -rect.y0);
	cairo_surface_destroy (surface);
	Render (cr);
	cairo_destroy (cr);
	m_pData->ShowSelection (true);
	char *m = g_strdup (reinterpret_cast <char const*> (gsf_output_memory_get_bytes (reinterpret_cast <GsfOutputMemory *> (output))));
	g_object_unref (output);
	return m;
}

char *View::BuildEPS ()
{
	gccv::Rect rect;
	m_pData->GetObjectBounds (m_pDoc, &rect);
	int w = (int) (ceil (rect.x1) - floor (rect.x0)), h = (int) (ceil (rect.y1) - floor (rect.y0));
	GsfOutput *output = gsf_output_memory_new ();
	cairo_surface_t *surface = cairo_ps_surface_create_for_stream (cairo_write_func, output, w, h);
	cairo_ps_surface_set_eps (surface, TRUE);
	cairo_t *cr = cairo_create (surface);
	cairo_translate (cr, -rect.x0, -rect.y0);
	cairo_surface_destroy (surface);
	Render (cr);
	cairo_destroy (cr);
	m_pData->ShowSelection (true);
	char *m = g_strdup (reinterpret_cast <char const*> (gsf_output_memory_get_bytes (reinterpret_cast <GsfOutputMemory *> (output))));
	g_object_unref (output);
	return m;
}

static void destroy_surface (G_GNUC_UNUSED guchar *pixels, gpointer data)
{
	cairo_surface_destroy (reinterpret_cast <cairo_surface_t *> (data));
}

GdkPixbuf *View::BuildPixbuf (int resolution)
{
	gccv::Rect rect;
	m_pData->GetObjectBounds (m_pDoc, &rect);
	m_pData->ShowSelection (false);
	int w = (int) (ceil (rect.x1) - floor (rect.x0)), h = (int) (ceil (rect.y1) - floor (rect.y0));
	double zoom;
	if (resolution > 0) {
		int screenres = m_pDoc->GetApp ()->GetScreenResolution ();
		zoom = (double) resolution / screenres;
		w = (int) rint ((double) w * zoom);
		h = (int) rint ((double) h * zoom);
	} else
		zoom = 1.;
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
	cairo_t *cr = cairo_create (surface);
	if (m_pDoc->GetApp () && !m_pDoc->GetApp ()->GetTransparentBackground ()) {
		cairo_set_source_rgb (cr, 1., 1., 1.);
		cairo_paint (cr);
	}
	cairo_scale (cr, zoom, zoom);
	cairo_translate (cr, -floor (rect.x0), -floor (rect.y0));
	m_Canvas->Render (cr, false);
	int rowstride = cairo_image_surface_get_stride (surface);
	unsigned char *data = cairo_image_surface_get_data (surface);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data (data, GDK_COLORSPACE_RGB, TRUE, 8, w, h, rowstride, destroy_surface, surface);
	go_cairo_convert_data_to_pixbuf (data, NULL, w, h, rowstride);
	cairo_destroy (cr);
	return pixbuf;
}

void View::EnsureSize ()
{
	double x1, y1, x2, y2;
	m_Canvas->GetRoot ()->GetBounds (x1, y1, x2, y2);
	if (x1 < 0.) x2 -= x1;
	if (y1 < 0.) y2 -= y1;
	if (x2 <= 0. || y2 <= 0.)
		return;
	if ((x2 != m_width) || (y2 != m_height)) {
		m_width = x2;
		m_height = y2;
		gtk_widget_set_size_request (m_pWidget, (int) ceil (x2 * m_Canvas->GetZoom ()), (int) ceil (y2 * m_Canvas->GetZoom ()));
	}
	if ((x1 < 0.) || (y1 < 0.))
	{
		Theme* pTheme = m_pDoc->GetTheme ();
		x1 = (x1 < 0.)? -x1 / pTheme->GetZoomFactor (): 0.;
		y1 = (y1 < 0.)? -y1 / pTheme->GetZoomFactor (): 0.;
		m_pDoc->Move(x1, y1);
		Update (m_pDoc);
	}
}

void View::Zoom (double zoom)
{
	m_pData->Zoom = zoom;
	m_Canvas->SetZoom (zoom);
	EnsureSize ();
}

void View::ShowCursor (bool show)
{
	if (m_ActiveRichText)
		m_ActiveRichText->SetEditing (show);
}

void View::UpdateTheme ()
{
	if (m_sFontName)
		g_free (m_sFontName);
	if (m_sSmallFontName)
		g_free (m_sSmallFontName);
	pango_font_description_free (m_PangoFontDesc);
	pango_font_description_free (m_PangoSmallFontDesc);
	Theme *pTheme = m_pDoc->GetTheme ();
	m_PangoFontDesc = pango_font_description_new ();
	pango_font_description_set_family (m_PangoFontDesc, pTheme->GetFontFamily ());
	pango_font_description_set_style (m_PangoFontDesc, pTheme->GetFontStyle ());
	pango_font_description_set_weight (m_PangoFontDesc, pTheme->GetFontWeight ());
	pango_font_description_set_variant (m_PangoFontDesc, pTheme->GetFontVariant ());
	pango_font_description_set_stretch (m_PangoFontDesc, pTheme->GetFontStretch ());
	pango_font_description_set_size (m_PangoFontDesc, pTheme->GetFontSize ());
	m_sFontName = pango_font_description_to_string (m_PangoFontDesc);
	m_PangoSmallFontDesc = pango_font_description_new ();
	pango_font_description_set_family (m_PangoSmallFontDesc, pTheme->GetFontFamily ());
	pango_font_description_set_style (m_PangoSmallFontDesc, pTheme->GetFontStyle ());
	pango_font_description_set_weight (m_PangoSmallFontDesc, pTheme->GetFontWeight ());
	pango_font_description_set_variant (m_PangoSmallFontDesc, pTheme->GetFontVariant ());
	pango_font_description_set_stretch (m_PangoSmallFontDesc, pTheme->GetFontStretch ());
	pango_font_description_set_size (m_PangoSmallFontDesc, pTheme->GetFontSize () * 2 / 3);
	m_sSmallFontName = pango_font_description_to_string (m_PangoSmallFontDesc);
	Update (m_pDoc);
}

void View::Render (cairo_t *cr)
{
	m_pData->ShowSelection(false);
	m_Canvas->Render (cr, true);
	m_pData->ShowSelection (true);
}

// Events

bool View::OnButtonPressed (gccv::ItemClient *client, unsigned button, double x, double y, unsigned state)
{
	Application *App = m_pDoc->GetApplication ();
	Theme *pTheme = m_pDoc->GetTheme ();
	Tool* pActiveTool = App? App->GetActiveTool (): NULL;
	Object *pAtom;
	m_CurObject = dynamic_cast <Object *> (client);
	if (m_CurObject && ((pAtom = m_CurObject->GetAtomAt (x / pTheme->GetZoomFactor (), y / pTheme->GetZoomFactor ()))))
			m_CurObject = pAtom;
	if ((!m_pDoc->GetEditable ()) || (!pActiveTool))
		return true;
	switch (button) {
	case 1: {
		if (m_Dragging) break;
		bool result = pActiveTool->OnClicked (this, m_CurObject, x, y, state);
		m_Dragging = result;
		return true;
	}
	case 2: {
		m_lastx = x;
		m_lasty = y;
		GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		OnPasteSelection (m_pWidget, clipboard);
		return true;
	}
	case 3: {
		bool result;
		g_object_unref (m_UIManager);
		m_UIManager = gtk_ui_manager_new ();
		result = pActiveTool->OnRightButtonClicked (this, m_CurObject, x, y, m_UIManager);
		if (m_CurObject)
			result |= m_CurObject->BuildContextualMenu (m_UIManager, m_CurObject, x / GetZoomFactor (), y / GetZoomFactor ());
		if (result) {
			GtkWidget *w = gtk_ui_manager_get_widget (m_UIManager, "/popup");
			gtk_menu_popup (GTK_MENU (w), NULL, NULL, NULL, NULL, 3,  gtk_get_current_event_time ());
			return true;
		}
	}
	}
	return true;
}

bool View::OnButtonReleased (G_GNUC_UNUSED gccv::ItemClient *client, unsigned button, double x, double y, unsigned state)
{
	Application *App = m_pDoc->GetApplication ();
	Tool* pActiveTool = App? App->GetActiveTool (): NULL;
	if ((!m_pDoc->GetEditable ()) || (!pActiveTool))
		return true;
	switch (button) {
	case 1:
		if (!m_Dragging)
			break;
		m_Dragging = false;
		pActiveTool->OnRelease (x, y, state);
		if (!pActiveTool->GetOwnStatus ())
			m_pDoc->GetApplication ()->ClearStatus ();
		return true;
	}
	return true;
}

bool View::OnDrag (G_GNUC_UNUSED gccv::ItemClient *client, double x, double y, unsigned state)
{
	Application *App = m_pDoc->GetApplication ();
	Tool* pActiveTool = App? App->GetActiveTool (): NULL;	
	if (m_pDoc->GetEditable () && pActiveTool && m_Dragging)
		pActiveTool->OnDrag (x, y, state);
	return true;
}

bool View::OnMotion (gccv::ItemClient *client, double x, double y, unsigned state)
{
	m_CurObject = dynamic_cast <Object *> (client);
	if (m_CurObject) {
		m_CurAtom = dynamic_cast <Atom *> (m_CurObject);
		if (!m_CurAtom) {
			Theme *theme = m_pDoc->GetTheme ();
			m_CurAtom = reinterpret_cast <Atom *> (m_CurObject->GetAtomAt (x / theme->GetZoomFactor (), y / theme->GetZoomFactor ()));
		}
	} else
		m_CurAtom = NULL;
	Application *App = m_pDoc->GetApplication ();
	Tool* pActiveTool = App? App->GetActiveTool (): NULL;	
	if (m_pDoc->GetEditable () && pActiveTool)
		pActiveTool->OnMotion (this, ((m_CurAtom)? m_CurAtom: m_CurObject), x, y, state);
	return true;
}

bool View::OnLeaveNotify (unsigned state)
{
	Application *App = m_pDoc->GetApplication ();
	Tool* pActiveTool = App? App->GetActiveTool (): NULL;	
	if (m_pDoc->GetEditable () && pActiveTool)
		pActiveTool->OnLeaveNotify (this, state);
	return true;
}

void View::SetSelectionState (Object *object, int state)
{
	gccv::ItemClient *client = dynamic_cast <gccv::ItemClient *> (object);
	if (client)
		client->SetSelected (state);	
	map<string, Object*>::iterator i;
	Object *child = object->GetFirstChild (i);
	while (child) {
		SetSelectionState (child, state);
		child = object->GetNextChild (i);
	}
}

}	//	namespace gcp
