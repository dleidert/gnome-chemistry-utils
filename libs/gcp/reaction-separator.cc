// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-separator.cc
 *
 * Copyright (C) 2013 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "reaction-separator.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <gccv/canvas.h>
#include <gccv/text.h>
#include <glib/gi18n.h>
#include <cmath>
#include <cstring>

using namespace gcu;

namespace gcp {

TypeId ReactionSeparatorType = NoType;

ReactionSeparator::ReactionSeparator ():
	Object (ReactionSeparatorType),
	gccv::ItemClient ()
{
	m_Text = ", ";
}

ReactionSeparator::ReactionSeparator (char const *text, gcu::TypeId type):
	Object (type),
	gccv::ItemClient ()
{
	m_Text = text;
}

ReactionSeparator::~ReactionSeparator ()
{
}

void ReactionSeparator::AddItem ()
{
	if (m_Item)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc ->GetTheme ();
	double x, y;
	GetCoords (&x, &y);
	x *= theme->GetZoomFactor ();
	y *= theme->GetZoomFactor ();
	gccv::Text *text = new gccv::Text (view->GetCanvas ()->GetRoot (), x, y, this);
	text->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: 0);
	text->SetFillColor (0);
	text->SetLineColor (0);
	text->SetLineWidth (0.);
	text->SetFontDescription (view->GetPangoFontDesc ()); // may be use the text font instead
	text->SetText (m_Text);
	text->SetLineOffset (view->GetCHeight ());
	m_Item = text;
}

void ReactionSeparator::SetSelected (int state)
{
	GOColor color;
	switch (state) {
	case SelStateUnselected:
		color = Color;
		break;
	case SelStateSelected:
		color = SelectColor;
		break;
	case SelStateUpdating:
		color = AddColor;
		break;
	case SelStateErasing:
		color = DeleteColor;
		break;
	default:
		color = Color;
		break;
	}
	dynamic_cast <gccv::Text *> (m_Item)->SetColor (color);
}

void ReactionSeparator::Move (double x, double y, G_GNUC_UNUSED double z)
{
	m_x += x;
	m_y += y;
}

void ReactionSeparator::SetCoords (double x, double y)
{
	m_x = x;
	m_y = y;
}

bool ReactionSeparator::GetCoords (double* x, double* y, double *z) const
{
	if (x == NULL || y == NULL)
		return false;
	*x = m_x;
	*y = m_y;
	if (z)
		*z = 0.;
	return true;
}

double ReactionSeparator::GetYAlign ()
{
	return m_y;
}

std::string ReactionSeparator::Name ()
{
	return _("Reaction separator");
}

xmlNodePtr ReactionSeparator::Save (xmlDocPtr) const
{
	return NULL;
}

}	//	namespace gcp
