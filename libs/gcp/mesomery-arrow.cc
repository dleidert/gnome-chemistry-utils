// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomery-arrow.cc 
 *
 * Copyright (C) 2004-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "mesomery-arrow.h"
#include "mesomery.h"
#include "mesomer.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <gccv/arrow.h>
#include <gccv/canvas.h>
#include <glib/gi18n-lib.h>

using namespace gcu;

namespace gcp {

MesomeryArrow::MesomeryArrow (Mesomery* mesomery): Arrow (MesomeryArrowType)
{
	SetId ("ma1");
	if (mesomery)
		mesomery->AddChild( this);
	m_Start = m_End = NULL;
}

MesomeryArrow::~MesomeryArrow ()
{
	if (IsLocked ())
		return;
	if (m_Start && m_End) {
		m_Start->RemoveArrow (this, m_End);
		m_End->RemoveArrow (this, m_Start);
	}
}

xmlNodePtr MesomeryArrow::Save (xmlDocPtr xml) const
{
	xmlNodePtr parent, node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "mesomery-arrow", NULL);
	if (!node)
		return NULL;
	if (!Arrow::Save (xml, node)) {
		xmlFreeNode (node);
		return NULL;
	}
	if (m_Start)
		xmlNewProp (node, (xmlChar*) "start",  (xmlChar*) m_Start->GetId ());
	if (m_End)
		xmlNewProp (node, (xmlChar*) "end",  (xmlChar*) m_End->GetId ());
	Mesomery* m = (Mesomery*) GetParentOfType (MesomeryType);
	if (!m)
	{
		//save the arrow as an object
		parent = xmlNewDocNode (xml, NULL, (xmlChar*) "object", NULL);
		if (node && parent)
			xmlAddChild (parent, node);
		else {
			xmlFreeNode (node);
			return NULL;
			}
	}
	else parent = node;
	return parent;
}

bool MesomeryArrow::Load (xmlNodePtr node)
{
	char *buf;
	Object *parent;
	if (Arrow::Load (node)) {
		parent = GetParent ();
		if (!parent)
			return true;
		buf = (char*) xmlGetProp (node, (xmlChar*) "start");
		if (buf) {
			m_Start = reinterpret_cast<Mesomer*> (parent->GetDescendant (buf));
			xmlFree (buf);
			if (!m_Start)
				return false;
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "end");
		if (buf) {
			m_End = reinterpret_cast<Mesomer*> (parent->GetDescendant (buf));
			xmlFree (buf);
			if (!m_End)
				return false;
			m_End->AddArrow (this, m_Start);
		}
		if (m_Start)
			m_Start->AddArrow (this, m_End);
		return true;
	}
	return false;
}

void MesomeryArrow::AddItem ()
{
	if (m_Item)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	gccv::Arrow *arrow = new gccv::Arrow (view->GetCanvas ()->GetRoot (),
										  m_x * theme->GetZoomFactor (),
										  m_y * theme->GetZoomFactor (),
										  (m_x + m_width) * theme->GetZoomFactor (),
										  (m_y + m_height) * theme->GetZoomFactor (),
										  this);
	arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
	arrow->SetLineWidth (theme->GetArrowWidth ());
	arrow->SetA (theme->GetArrowHeadA ());
	arrow->SetB (theme->GetArrowHeadB ());
	arrow->SetC (theme->GetArrowHeadC ());
	arrow->SetStartHead (gccv::ArrowHeadFull);
	m_Item = arrow;
}

void MesomeryArrow::UpdateItem ()
{
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	}
	AddItem ();
}

void MesomeryArrow::Reverse ()
{
	Mesomer *mesomer = m_Start;
	m_Start= m_End;
	m_End = mesomer;
	m_x = m_x + m_width;
	m_y = m_y + m_height;
	m_width = - m_width;
	m_height = - m_height;
}

std::string MesomeryArrow::Name ()
{
	return _("Mesomery arrow");
}

}	//	namespace gcp
