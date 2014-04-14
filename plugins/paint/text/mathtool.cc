// -*- C++ -*-

/*
 * GChemPaint text plugin
 * mathtool.cc
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "equation.h"
#include "mathtool.h"
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>

gcpMathTool::gcpMathTool (gcp::Application *App):
	gcp::Tool (App, "Equation")
{
	gcp::Theme *theme = gcp::TheThemeManager.GetTheme ("Default");
	m_Font = pango_font_description_new ();
	pango_font_description_set_family (m_Font, theme->GetTextFontFamily ());
	pango_font_description_set_size (m_Font, theme->GetTextFontSize ());
	pango_font_description_set_style (m_Font, theme->GetTextFontStyle ());
	pango_font_description_set_weight (m_Font, theme->GetTextFontWeight ());
	pango_font_description_set_stretch (m_Font, theme->GetTextFontStretch ());
	pango_font_description_set_variant (m_Font, theme->GetTextFontVariant ());
}

gcpMathTool::~gcpMathTool ()
{
	pango_font_description_free (m_Font);
}

bool gcpMathTool::OnClicked ()
{
	gcp::Theme *theme = m_pView->GetDoc ()->GetTheme ();
	if (!m_pObject) {
		gcpEquation *eq = new gcpEquation (m_x0 / theme->GetZoomFactor (), m_y0 / theme->GetZoomFactor ());
		eq->SetFontDesc (m_Font);
		m_pView->GetDoc ()->AddObject (eq);
		m_pView->GetDoc ()->AbortOperation ();
		m_pObject = eq;
	}
	if (m_pObject) {
		if (m_pObject->GetType () != EquationType)
			return false;
		gcpEquation *eq = static_cast <gcpEquation *> (m_pObject);
		eq->SetSelected (gcp::SelStateUpdating);
		eq->ShowPropertiesDialog ();
		// FIXME: open the property dialog
	}
	return true;
}
