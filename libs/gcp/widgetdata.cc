// -*- C++ -*-

/* 
 * GChemPaint library
 * widgetdata.cc
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "widgetdata.h"
#include "view.h"
#include "settings.h"
#include "document.h"
#include "application.h"
#include "operation.h"
#include "theme.h"
#include "tool.h"
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/item-client.h>
#include <gccv/item.h>
#include <cstring>
#include <map>

using namespace gcu;

namespace gcp {

static xmlDocPtr pXmlDoc = NULL, pXmlDoc1 = NULL;
xmlChar* ClipboardData = NULL;
char *ClipboardTextData = NULL;
guint ClipboardDataType, ClipboardDataType1;

GtkTargetEntry const export_targets[] = {
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

void on_receive_targets (GtkClipboard *clipboard, GtkSelectionData *selection_data, Application *App)
{
	GtkClipboard* sel_clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	guint *DataType = (clipboard == sel_clipboard)? &ClipboardDataType: &ClipboardDataType1;
	if (gtk_selection_data_get_target (selection_data) == gdk_atom_intern ("TARGETS", FALSE)) {
		static char const *formats [] =
		{
			GCHEMPAINT_ATOM_NAME,
			"image/svg",
			"image/svg+xml",
			"image/x-eps",
			"image/png",
			"image/jpeg",
			"image/bmp",
			"UTF8_STRING",
			"STRING",
			NULL
		};

		GdkAtom const *targets = (GdkAtom *) gtk_selection_data_get_data (selection_data);
		int length = gtk_selection_data_get_length (selection_data);
		unsigned const atom_count = (length / sizeof (GdkAtom));
		unsigned i, j;
		/* Nothing on clipboard? */
		if (length < 0) {
			if (clipboard == sel_clipboard)
				App->ActivateWindowsActionWidget ("/MainMenu/EditMenu/Paste", false);
			return;
		}

		gchar* name;
		*DataType = GCP_CLIPBOARD_ALL;
		for ( j = 0; j < atom_count ; j++) {
			name = gdk_atom_name (targets [j]);
			for (i = 0; i < *DataType; i++)
				if (!strcmp (name, formats[i])) {
					*DataType = i;
					break;
				}
			g_free (name);
		}
	}
	if (clipboard == sel_clipboard && App != NULL)
		App->ActivateWindowsActionWidget ("/MainMenu/EditMenu/Paste",
			ClipboardDataType == GCP_CLIPBOARD_NATIVE || ClipboardDataType == GCP_CLIPBOARD_UTF8_STRING || ClipboardDataType == GCP_CLIPBOARD_STRING);
}

static void on_get_data (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, Application *App)
{
	xmlDocPtr pDoc = (clipboard == gtk_clipboard_get(GDK_SELECTION_CLIPBOARD))? pXmlDoc: pXmlDoc1;
	guint *DataType = (clipboard == gtk_clipboard_get(GDK_SELECTION_CLIPBOARD))? &ClipboardDataType: &ClipboardDataType1;
	g_return_if_fail (pDoc);
	if (ClipboardData)
		xmlFree (ClipboardData);
	ClipboardData = NULL;
	g_free (ClipboardTextData);
	ClipboardTextData = NULL;
	*DataType = info;
	int size;
	switch (info) {
	case GCP_CLIPBOARD_NATIVE:
		xmlDocDumpFormatMemory (pDoc, &ClipboardData, &size, info);
		gtk_selection_data_set(selection_data, gdk_atom_intern (GCHEMPAINT_ATOM_NAME, FALSE), 8, (const guchar*) ClipboardData, size);
		break;
	case GCP_CLIPBOARD_SVG:
	case GCP_CLIPBOARD_SVG_XML: {
		Document *Doc = new Document (NULL, true);
		View *pView = Doc->GetView ();
		pView->CreateNewWidget (); // force canvas creation
		Doc->ParseXMLTree (pDoc);
		ClipboardTextData = pView->BuildSVG ();
		gtk_selection_data_set_text (selection_data, ClipboardTextData, strlen (ClipboardTextData));
		delete Doc;
		break;
	}
	case GCP_CLIPBOARD_EPS: {
		Document *Doc = new Document (NULL, true);
		View *pView = Doc->GetView ();
		pView->CreateNewWidget (); // force canvas creation
		Doc->ParseXMLTree (pDoc);
		ClipboardTextData = pView->BuildEPS ();
		gtk_selection_data_set_text (selection_data, ClipboardTextData, strlen (ClipboardTextData));
		delete Doc;
		break;
	}
	case GCP_CLIPBOARD_PNG: {
		Document *Doc = new Document (NULL, true);
		View *pView = Doc->GetView ();
		gsize size;
		pView->CreateNewWidget (); // force canvas creation
		Doc->ParseXMLTree (pDoc);
		GdkPixbuf *pixbuf = pView->BuildPixbuf (-1); // copy with zoom == 1
		gdk_pixbuf_save_to_buffer (pixbuf, &ClipboardTextData, &size, "png", NULL, NULL);
		gtk_selection_data_set (selection_data, gdk_atom_intern (export_targets[info].target, FALSE), 8, (const guchar*) ClipboardTextData, size);
		g_object_unref (pixbuf);
		delete Doc;
		break;
	}
	case GCP_CLIPBOARD_JPEG: {
		Document *Doc = new Document (NULL, true);
		View *pView = Doc->GetView ();
		gsize size;
		pView->CreateNewWidget (); // force canvas creation
		Doc->ParseXMLTree (pDoc);
		GdkPixbuf *pixbuf = pView->BuildPixbuf (-1); // copy with zoom == 1
		gdk_pixbuf_save_to_buffer (pixbuf, &ClipboardTextData, &size, "jpg", NULL, NULL);
		gtk_selection_data_set (selection_data, gdk_atom_intern (export_targets[info].target, FALSE), 8, (const guchar*) ClipboardTextData, size);
		g_object_unref (pixbuf);
		delete Doc;
		break;
	}
	case GCP_CLIPBOARD_BMP: {
		Document *Doc = new Document (NULL, true);
		View *pView = Doc->GetView ();
		gsize size;
		pView->CreateNewWidget (); // force canvas creation
		Doc->ParseXMLTree (pDoc);
		GdkPixbuf *pixbuf = pView->BuildPixbuf (-1); // copy with zoom == 1
		gdk_pixbuf_save_to_buffer (pixbuf, &ClipboardTextData, &size, "bmp", NULL, NULL);
		gtk_selection_data_set (selection_data, gdk_atom_intern (export_targets[info].target, FALSE), 8, (const guchar*) ClipboardTextData, size);
		g_object_unref (pixbuf);
		delete Doc;
		break;
	}
	default:
		xmlDocDumpFormatMemory (pDoc, &ClipboardData, &size, info);
		gtk_selection_data_set_text (selection_data, (const gchar*) ClipboardData, size);
		break;
	}
	if (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))
		App->ActivateWindowsActionWidget ("/MainMenu/EditMenu/Paste", true);
}

void on_clear_data (GtkClipboard *clipboard, Object *obj)
{
	if (ClipboardData) {
		xmlFree (ClipboardData);
		ClipboardData = NULL;
	}
	g_free (ClipboardTextData);
	ClipboardTextData = NULL;
	Application *app = dynamic_cast <Application *> (obj);
	if (!app) {
		// the object might be a tool
		Tool *tool = dynamic_cast <Tool *> (obj);
		if (tool)
			app = tool->GetApplication ();
	}
	if (app)
		gtk_clipboard_request_contents (clipboard, gdk_atom_intern ("TARGETS", FALSE),  (GtkClipboardReceivedFunc) on_receive_targets, app);
}

bool WidgetData::IsSelected (Object const *obj) const
{
	std::list<Object*>::const_iterator i, end = SelectedObjects.end ();
	Object const *parent = obj->GetParent ();
	if (parent && IsSelected (parent))
		return true;
	for (i = SelectedObjects.begin (); i != end; i++)
		if (*i == obj)
			return true;
	
	return false;
}

bool WidgetData::ChildrenSelected (gcu::Object const *obj) const
{
	if (!obj->HasChildren ())
		return false;
	std::map <std::string, gcu::Object *>::const_iterator i;
	std::list<Object*>::const_iterator j,  end = SelectedObjects.end ();
	for (Object const *child = obj->GetFirstChild (i); child; child = obj->GetNextChild (i)) {
		for (j = SelectedObjects.begin (); j != end; j++)
			if (*j == child)
				break;
		if (j == end && !ChildrenSelected (child))
		    return false;
	}
	return true;
}

gcu::Object *WidgetData::GetSelectedAncestor (gcu::Object *child)
{
	Object *parent = child->GetParent ();
	if (parent->GetType () == DocumentType)
		return NULL;
	Object *ancestor = GetSelectedAncestor (parent);
	if (ancestor)
		return ancestor;
	// check if the object needs a specific parent and return NULL if so
	if (!parent->GetDocument ()->GetApplication ()->GetRules (parent->GetType (), RuleMustBeIn).empty ())
		return NULL;
	return (ChildrenSelected (parent))? parent: NULL;
}

void WidgetData::SimplifySelection ()
{
	std::list <Object *>::iterator i, end = SelectedObjects.end ();
	std::set <Object *> RealSelection;
	Object *parent;
	gcu::Application *app = m_View->GetDoc ()->GetApplication ();
	for (i = SelectedObjects.begin (); i != end; i++) {
		parent = GetSelectedAncestor (*i);
		if (parent)
			RealSelection.insert (parent);
		else if (app->GetRules ((*i)->GetType (), RuleMustBeIn).empty ())
			RealSelection.insert (*i);
	}
	UnselectAll ();
	std::set <Object *>::iterator j, jend = RealSelection.end ();
	for (j = RealSelection.begin (); j != jend; j++)
		SetSelected (*j);
}

void WidgetData::Unselect (Object *obj)
{
	SelectedObjects.remove (obj);
	m_View->SetSelectionState (obj, SelStateUnselected);
}

void WidgetData::UnselectAll ()
{
	Object* obj;
	while (!SelectedObjects.empty ()) {
		obj = SelectedObjects.front ();
		SelectedObjects.pop_front ();
		Unselect (obj);
	}
}

void WidgetData::SetSelected (Object *obj, int state)
{
	if (!IsSelected (obj)) {
		SelectedObjects.push_front (obj);
		m_View->SetSelectionState (obj, state);
	}
}

void WidgetData::MoveSelectedItems (double dx, double dy)
{
	std::list<Object*>::iterator i, end = SelectedObjects.end ();
	for (i = SelectedObjects.begin (); i != end; i++)
		MoveItems (*i, dx, dy);
}

void WidgetData::MoveItems (Object* obj, double dx, double dy)
{
	Object* pObject;
	gccv::ItemClient *client = dynamic_cast <gccv::ItemClient *> (obj);
	gccv::Item *item = (client)? client->GetItem (): NULL;
	if (item && item->GetParent ()->GetParent () == NULL) // move only if parent is root group
		item->Move (dx, dy);
	std::map<std::string, Object*>::iterator i;
	pObject = obj->GetFirstChild (i);
	while (pObject) {
		MoveItems (pObject, dx, dy);
		pObject = obj->GetNextChild (i);
	}
}

void WidgetData::MoveSelection (double dx, double dy)
{
	if (!SelectedObjects.size ())
		return;
	std::list<Object*>::iterator i, end = SelectedObjects.end ();
	Document* pDoc = m_View->GetDoc ();
	Operation* pOp = pDoc-> GetNewOperation (GCP_MODIFY_OPERATION);
	Theme *pTheme = pDoc->GetTheme ();
	for (i = SelectedObjects.begin (); i != end; i++) {
		pOp->AddObject (*i,0);
		(*i)->Move (dx / pTheme->GetZoomFactor (), dy / pTheme->GetZoomFactor ());
		m_View->Update (*i);
		pOp->AddObject (*i,1);
	}
	pDoc->FinishOperation ();
}

void WidgetData::RotateSelection (double dx, double dy, double angle)
{
	Theme *pTheme = m_View->GetDoc ()->GetTheme ();
	std::list<Object*>::iterator i, end = SelectedObjects.end ();
	Matrix2D m (angle);
	for (i = SelectedObjects.begin (); i != end; i++) {
		(*i)->Transform2D (m, dx / pTheme->GetZoomFactor (), dy / pTheme->GetZoomFactor ());
		m_View->Update (*i);
	}
}

void WidgetData::Copy (GtkClipboard* clipboard)
{
	/*First build the XML tree from current selection*/
	xmlDocPtr *pDoc = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &pXmlDoc: &pXmlDoc1;
	if (*pDoc)
		xmlFreeDoc (*pDoc);
	*pDoc = xmlNewDoc ((xmlChar*) "1.0");
	if (!*pDoc)
		return;
	if (SelectedObjects.empty ())
		return;
	xmlDocSetRootElement (*pDoc, xmlNewDocNode(*pDoc, NULL, (xmlChar*)"chemistry", NULL));
	xmlNsPtr ns = xmlNewNs ((*pDoc)->children, (xmlChar*) "http://www.nongnu.org/gchempaint", (xmlChar*) "gcp");
	xmlSetNs ((*pDoc)->children, ns);
//FIXME: implement exception handling
	std::list<Object*>::iterator i, end= SelectedObjects.end ();
	xmlNodePtr child;
	for (i = SelectedObjects.begin(); i != end; i++)
		if ((child = (*i)->Save (pXmlDoc)))
			xmlAddChild ((*pDoc)->children, child);
	Application* App = m_View->GetDoc ()->GetApplication ();
	gtk_clipboard_set_with_data (clipboard, export_targets, ClipboardFormats, (GtkClipboardGetFunc) on_get_data, (GtkClipboardClearFunc) on_clear_data, App);
	gtk_clipboard_request_contents (clipboard, gdk_atom_intern ("TARGETS", FALSE),  (GtkClipboardReceivedFunc) on_receive_targets, App);
}

void WidgetData::GetObjectBounds (Object const *obj, gccv::Rect &rect) const
{
	Object const *pObject;
	gccv::ItemClient const *client, *child_client;
	double x1, y1, x2,  y2;
	client = dynamic_cast <gccv::ItemClient const *> (obj);
	gccv::Item const *item;
	// will not work if the object item is not top level, but can this happen?
	if (client && (item = client->GetItem ()) && item->IsTopLevel ()) {
		item->GetBounds (x1, y1, x2, y2);
		if (x2 > 0.) {
			if (!go_finite (rect.x0)) {
				rect.x0 = x1;
				rect.y0 = y1;
				rect.x1 = x2;
				rect.y1 = y2;
			} else {
				if (rect.x0 > x1) rect.x0 = x1;
				if (rect.y0 > y1) rect.y0 = y1;
				if (rect.x1 < x2) rect.x1 = x2;
				if (rect.y1 < y2) rect.y1 = y2;
			}
		}
	}
	std::map<std::string, Object*>::const_iterator i;
	pObject = obj->GetFirstChild (i);
	while (pObject) {
		child_client = dynamic_cast <gccv::ItemClient const *> (pObject);
		if (!child_client || !child_client->GetItem () || !client || child_client->GetItem ()->GetParent () != client->GetItem ())
			GetObjectBounds (pObject, rect);
		pObject = obj->GetNextChild (i);
	}
}

void WidgetData::GetSelectionBounds (gccv::Rect &rect) const
{
	std::list<Object*>::const_iterator i, end = SelectedObjects.end ();
	rect.x0 = go_nan;
	for (i = SelectedObjects.begin (); i != end; i++)
		GetObjectBounds (*i, rect);
	if (!go_finite (rect.x0))
		rect.x0 = rect.y0 = rect.x1 = rect.y1 = 0.;
}

void WidgetData::GetObjectBounds (Object const *obj, gccv::Rect *rect) const
{
	rect->x0 = go_nan;
	GetObjectBounds (obj, *rect);
	if (!go_finite (rect->x0))
		rect->x0 = rect->y0 = rect->x1 = rect->y1 = 0.;
}

void WidgetData::GetObjectsBounds (std::set <gcu::Object const *> const &objects, gccv::Rect *rect) const
{
	rect->x0 = go_nan;
	std::set <gcu::Object const *>::iterator it, end = objects.end ();
	for (it = objects.begin (); it != end; it++)
		GetObjectBounds (*it, *rect);
	if (!go_finite (rect->x0))
		rect->x0 = rect->y0 = rect->x1 = rect->y1 = 0.;
}

void WidgetData::SelectAll ()
{
	std::list <gccv::Item *>::iterator it;
	gccv::Group *root = m_View->GetCanvas ()->GetRoot ();
	gccv::Item *item;
	Object *pObject, *pGroup;
	for (item = root->GetFirstChild (it); item; item = root->GetNextChild (it)) {
		if (item->GetClient ())
			pObject = dynamic_cast <Object *> (item->GetClient ());
		else
			continue;
		pGroup = pObject->GetGroup ();
		if (pGroup) {
			if (!IsSelected (pGroup))
				SetSelected (pGroup);
		} else if (!IsSelected (pObject))
			SetSelected (pObject);
	}
}

xmlDocPtr WidgetData::GetXmlDoc (GtkClipboard* clipboard)
{
	return (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? pXmlDoc: pXmlDoc1;
}

void WidgetData::ShowSelection (bool state)
{
	std::list<Object*>::iterator i, end = SelectedObjects.end ();
	for (i = SelectedObjects.begin (); i != end; i++)
		m_View->SetSelectionState (*i, (state)? SelStateSelected: SelStateUnselected);
}

}	//	namespace gcp
