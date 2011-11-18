/*
 * GChemPaint selection plugin
 * group.cc
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "group.h"
#include "groupdlg.h"
#include <gcp/brackets.h>
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcugtk/ui-manager.h>
#include <gccv/structs.h>
#include <glib/gi18n-lib.h>
#include <cerrno>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

TypeId GroupType = NoType;

static void on_group_properties (gcpGroup* group)
{
	Dialog *dlg = group->GetDialog ("group");
	if (dlg)
		dlg->Present ();
	else 
		new gcpGroupDlg ((gcp::Document*) group->GetDocument (), group);
}

gcpGroup::gcpGroup (): Object(GroupType), DialogOwner ()
{
	SetId ("gr1");
	m_Align = false;
	m_Spaced = false;
}

gcpGroup::~gcpGroup ()
{
}

bool gcpGroup::BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y)
{
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	GtkActionGroup *group = gtk_action_group_new ("group");
	GtkAction *action = gtk_action_new ("group_properties", _("Group properties..."), NULL, NULL);
	g_signal_connect_swapped (action, "activate", G_CALLBACK (on_group_properties), this);
	gtk_action_group_add_action (group, action);
	gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menuitem action='group_properties'/></popup></ui>", -1, NULL);
	gtk_ui_manager_insert_action_group (uim, group, 0);
	Object::BuildContextualMenu (UIManager, object, x, y);
	return true;
}

void gcpGroup::SetAligned (gcpAlignType type)
{
	if (!m_Align || m_AlignType != type) {
		m_Align = true;
		m_AlignType = type;
		Align ();
	}
}

void gcpGroup::UnAlign ()
{
	m_Align = false;
	m_Spaced = false;
}

bool gcpGroup::GetAlignType (gcpAlignType& align)
{
	align = m_AlignType;
	return m_Align;
}

void gcpGroup::SetPadding (double padding)
{
	if (!m_Spaced || m_Padding != padding) {
		m_Spaced = true;
		m_Padding = padding;
		Space ();
	}
}

void gcpGroup::UnSpace ()
{
	m_Spaced = false;
}

bool gcpGroup::GetPadding (double& padding)
{
	padding = m_Padding;
	return m_Spaced;
}

void gcpGroup::Align ()
{
	if (!m_Align)
		return;
	map<Object*, double> Children;
	map<string, Object*>::iterator i;
	gcu::Object *bracket = NULL;
	Object* obj = GetFirstChild (i);
	gccv::Rect rect;
	gcp::Document *pDoc = dynamic_cast <gcp::Document*> (GetDocument ());
	gcp::View *View = pDoc->GetView ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	gcp::WidgetData *Data = reinterpret_cast <gcp::WidgetData*> (g_object_get_data (G_OBJECT (View->GetWidget ()), "data"));
	double x = 0., t = 0.;
	int nb = 0;
	gcu::Object *child = GetFirstChild (i);
	for (; child; child = GetNextChild (i))
		if (child->GetType () != gcp::BracketsType)
			nb++;
	child = GetFirstChild (i);
	while (obj->GetType () == gcp::BracketsType)
		obj = GetNextChild (i);
	if ((m_AlignType == GCP_ALIGN_TOP)
		|| (m_AlignType == GCP_ALIGN_LEFT))
		t = DBL_MAX;
	while (obj) {
		if (obj->GetType () != gcp::BracketsType) {
			if (m_AlignType == GCP_ALIGN_NORMAL) {
				x = obj->GetYAlign ();
				t += x;
			} else {
				Data->GetObjectBounds (obj, &rect);
				switch (m_AlignType) {
				case GCP_ALIGN_TOP:
					x = rect.y0 / pTheme->GetZoomFactor ();
					if (t > x)
						t = x;
					break;
				case GCP_ALIGN_MID_HEIGHT:
					x = (rect.y0 + rect.y1) / 2. / pTheme->GetZoomFactor ();
					t += x;
					break;
				case GCP_ALIGN_BOTTOM:
					x = rect.y1 / pTheme->GetZoomFactor ();
					if (t < x)
						t = x;
					break;
				case GCP_ALIGN_LEFT:
					x = rect.x0 / pTheme->GetZoomFactor ();
					if (t > x)
						t = x;
					break;
				case GCP_ALIGN_CENTER:
					x = (rect.x0 + rect.x1) / 2. / pTheme->GetZoomFactor ();
					t += x;
					break;
				case GCP_ALIGN_RIGHT:
					x = rect.x1 / pTheme->GetZoomFactor ();
					if (t < x)
						t = x;
					break;
				default:
					break;
				}
			}
			Children[obj] = x;
		}
		obj = GetNextChild (i);
	}
	if ((m_AlignType == GCP_ALIGN_NORMAL)
		|| (m_AlignType == GCP_ALIGN_MID_HEIGHT)
		|| (m_AlignType == GCP_ALIGN_CENTER))
		t /= nb;
	obj = GetFirstChild (i);
	while (obj) {
		if (obj->GetType () != gcp::BracketsType) {
			if ((m_AlignType == GCP_ALIGN_LEFT)
				|| (m_AlignType == GCP_ALIGN_CENTER)
				|| (m_AlignType == GCP_ALIGN_RIGHT))
				obj->Move (t - Children[obj], 0);
			else
				obj->Move (0, t - Children[obj]);
			View->Update (obj);
		} else
			bracket = obj;
		obj = GetNextChild (i);
	}
	Space ();
	if (bracket)
		View->Update (bracket);
}

void gcpGroup::Space ()
{
	if (!m_Align || !m_Spaced)
		return;
	map<string, Object*>::iterator i;
	map<Object*, gccv::Rect> rects;
	map<double, Object*> Children;
	map<double, Object*>::iterator im, endm;
	Object* obj = GetFirstChild (i);
	gccv::Rect rect;
	double x;
	gcp::Document *pDoc = dynamic_cast <gcp::Document*> (GetDocument ());
	gcp::View *View = pDoc->GetView ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	gcp::WidgetData *Data = (gcp::WidgetData*) g_object_get_data (G_OBJECT (View->GetWidget ()), "data");
	while (obj) {
		if (obj->GetType () != gcp::BracketsType) {
			Data->GetObjectBounds (obj, &rect);
			rects[obj] = rect;
			x = (m_AlignType <= GCP_ALIGN_BOTTOM)? rect.x0: rect.y0;
			while (Children[x])
				x += 1e-5;
			Children[x] = obj;
		}
		obj = GetNextChild (i);
	}
	endm = Children.end ();
	im = Children.begin();
	rect = rects[(*im).second];
	x = (m_AlignType <= GCP_ALIGN_BOTTOM)? rect.x1: rect.y1;
	x /= pTheme->GetZoomFactor ();
	for (im++; im != endm; im++) {
		x += m_Padding;
		obj = (*im).second;
		rect = rects[obj];
		if (m_AlignType <= GCP_ALIGN_BOTTOM) {
			obj->Move (x - rect.x0 / pTheme->GetZoomFactor (), 0);
			x += (rect.x1 - rect.x0) / pTheme->GetZoomFactor ();
		} else {
			obj->Move (0, x - rect.y0 / pTheme->GetZoomFactor ());
			x += (rect.y1 - rect.y0) /pTheme->GetZoomFactor ();
		}
		View->Update (obj);
	}
}

bool gcpGroup::Load (xmlNodePtr node)
{
	if (!Object::Load (node))
		return false;
	Lock ();
	char *buf = (char*) xmlGetProp (node, (const xmlChar*) "align");
	if (buf) {
		if (!strcmp (buf, "normal")) {
			m_Align = true;
			m_AlignType = GCP_ALIGN_NORMAL;
		} else if (!strcmp (buf, "top")) {
			m_Align = true;
			m_AlignType = GCP_ALIGN_TOP;
		} else if (!strcmp (buf, "mid-height")) {
			m_Align = true;
			m_AlignType = GCP_ALIGN_MID_HEIGHT;
		} else if (!strcmp (buf, "bottom")) {
			m_Align = true;
			m_AlignType = GCP_ALIGN_BOTTOM;
		} else if (!strcmp (buf, "left")) {
			m_Align = true;
			m_AlignType = GCP_ALIGN_LEFT;
		} else if (!strcmp (buf, "center")) {
			m_Align = true;
			m_AlignType = GCP_ALIGN_CENTER;
		} else if (!strcmp (buf, "right")) {
			m_Align = true;
			m_AlignType = GCP_ALIGN_RIGHT;
		} else
			m_Align = false;
		xmlFree (buf);
		if (m_Align) {
			m_Padding = false;
			buf = (char*) xmlGetProp (node, (const xmlChar*) "padding");
			if (buf) {
				char *endptr;
				m_Padding = strtod (buf, &endptr);
				if (!*endptr && (errno != ERANGE))
					m_Spaced = true;
				xmlFree (buf);
			}
			((gcp::Document*) GetDocument ())->GetView ()->AddObject (this);
			Align ();
		}
	}
	Lock (false);
	GetDocument ()->ObjectLoaded (this);
	return true;
}

xmlNodePtr gcpGroup::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = Object::Save (xml);
	if (m_Align) {
		char const *align_type = NULL;
		switch (m_AlignType) {
		case GCP_ALIGN_NORMAL:
			align_type = "normal";
			break;
		case GCP_ALIGN_TOP:
			align_type = "top";
			break;
		case GCP_ALIGN_MID_HEIGHT:
			align_type = "mid-height";
			break;
		case GCP_ALIGN_BOTTOM:
			align_type = "bottom";
			break;
		case GCP_ALIGN_LEFT:
			align_type = "left";
			break;
		case GCP_ALIGN_CENTER:
			align_type = "center";
			break;
		case GCP_ALIGN_RIGHT:
			align_type = "right";
			break;
		}
		xmlNewProp (node, (const xmlChar*) "align", (const xmlChar*) align_type);
		if (m_Spaced) {
			char *buf = g_strdup_printf ("%g", m_Padding);
			xmlNewProp (node, (const xmlChar*) "padding", (const xmlChar*) buf);
			g_free (buf);
		}
	}
	return node;
}

bool gcpGroup::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (IsLocked ())
		return false;
	if (Signal == gcp::OnChangedSignal) {
		// evaluate children number excluding brackets
		int nb = 0;
		std::map < std::string, gcu::Object * >::iterator i;
		gcu::Object *child = GetFirstChild (i);
		for (child = GetFirstChild (i); child; child = GetNextChild (i))
			if (child->GetType () != gcp::BracketsType)
				nb++;
		if (nb < 2)
			delete this;
		else
			Align ();
	}
	return true;
}

void gcpGroup::Transform2D (G_GNUC_UNUSED Matrix2D& m, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y)
{
}

double gcpGroup::GetYAlign ()
{
	gcp::Document *pDoc = reinterpret_cast<gcp::Document*> (GetDocument ());
	gcp::WidgetData* pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
	gccv::Rect rect;
	pData->GetObjectBounds (this, &rect);
	return (rect.y1 - rect.y0) / 2.;
}

void gcpGroup::OnLoaded ()
{
	int nb = 0;
	std::map < std::string, gcu::Object * >::iterator i;
	gcu::Object *child = GetFirstChild (i);
	for (child = GetFirstChild (i); child; child = GetNextChild (i))
		if (child->GetType () != gcp::BracketsType)
			nb++;
	if (nb < 2)
		delete this;
	else
		Align ();
}

std::string gcpGroup::Name ()
{
	return _("Group");
}
