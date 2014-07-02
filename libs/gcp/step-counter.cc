// -*- C++ -*-

/*
 * GChemPaint library
 * step-counter.cc
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
#include "reaction-arrow.h"
#include "step-counter.h"
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

static char const *urn[] = {
	"Ⅰ. ",
	"Ⅱ. ",
	"Ⅲ. ",
	"Ⅳ. ",
	"Ⅴ. ",
	"Ⅵ. ",
	"Ⅶ. ",
	"Ⅷ. ",
	"Ⅸ. ",
	"Ⅹ. ",
	"Ⅺ. ",
	"Ⅻ. "
};

static char const *lrn[] = {
	"ⅰ. ",
	"ⅱ. ",
	"ⅲ. ",
	"ⅳ. ",
	"ⅴ. ",
	"ⅵ. ",
	"ⅶ. ",
	"ⅷ. ",
	"ⅸ. ",
	"ⅹ. ",
	"ⅺ. ",
	"ⅻ. "
};

static char const *arn[] = {
	"1. ",
	"2. ",
	"3. ",
	"4. ",
	"5. ",
	"6. ",
	"7. ",
	"8. ",
	"9. ",
	"10. ",
	"11. ",
	"12. "
};

static const char **schemes[] = {
	arn, urn, lrn
};

TypeId StepCounterType = NoType;

StepCounter::StepCounter (unsigned step, NumberingScheme scheme):
	Object (StepCounterType),
	gccv::ItemClient (),
	m_Step (step),
	m_Scheme (scheme)
{
	Changed ();
}

StepCounter::~StepCounter ()
{
}

void StepCounter::AddItem ()
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
	text->SetText (m_str.c_str ());
	text->SetLineOffset (view->GetCHeight ());
	m_Item = text;
}

void StepCounter::SetSelected (int state)
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

void StepCounter::Move (double x, double y, G_GNUC_UNUSED double z)
{
	m_x += x;
	m_y += y;
}

void StepCounter::SetCoords (double x, double y)
{
	m_x = x;
	m_y = y;
}

bool StepCounter::GetCoords (double* x, double* y, double *z) const
{
	if (x == NULL || y == NULL)
		return false;
	*x = m_x;
	*y = m_y;
	if (z)
		*z = 0.;
	return true;
}

double StepCounter::GetYAlign ()
{
	return m_y;
}

std::string StepCounter::Name ()
{
	return _("Reaction separator");
}

xmlNodePtr StepCounter::Save (xmlDocPtr) const
{
	return NULL;
}

void StepCounter::Changed ()
{
	if (m_Step == 0)
		m_Step = 1; // this would be a bug if it happened
	m_str = std::string (schemes [m_Scheme][m_Step - 1]);
}

}	//	namespace gcp
