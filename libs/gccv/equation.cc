// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/equation.cc
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
#include "canvas.h"
#include "equation.h"

namespace gccv {

Equation::Equation (Canvas *canvas, double x, double y):
	Rectangle (canvas, x, y, 0., 0.),
	m_x (x), m_y (y), m_View (NULL),
	m_Math (NULL),
	m_Anchor (AnchorLine)
{
}

Equation::Equation (Group *parent, double x, double y, ItemClient *client):
	Rectangle (parent, x, y, 0., 0., client),
	m_x (x), m_y (y), m_View (NULL),
	m_Math (NULL),
	m_Anchor (AnchorLine)
{
}

Equation::~Equation ()
{
	if (m_View)
		g_object_unref (m_View);
}

static GOColor last_color = 0;
static std::string last_font;

void Equation::Draw (cairo_t *cr, bool is_vector) const
{
	Rectangle::Draw (cr, is_vector);
	if (m_Math && (m_AutoFont || m_AutoTextColor)) {
		LsmDomNode *node = lsm_dom_node_get_first_child (LSM_DOM_NODE (m_Math));
		LsmDomElement *style = LSM_DOM_ELEMENT (lsm_dom_node_get_first_child (node));
		char *value;
		bool changed = false;
		if (m_AutoFont) {
			PangoFontDescription *font = GetCanvas ()->GetFont ();
			if (font) {
				char *desc = pango_font_description_to_string (font);
				if (last_font != desc) {
					last_font = desc;
					if (pango_font_description_get_weight (font) >= PANGO_WEIGHT_BOLD) {
						if (pango_font_description_get_style (font) == PANGO_STYLE_NORMAL)
							lsm_dom_element_set_attribute (style, "mathvariant", "bold");
						else
							lsm_dom_element_set_attribute (style, "mathvariant", "bold-italic");
					} else {
						if (pango_font_description_get_style (font) == PANGO_STYLE_NORMAL)
							lsm_dom_element_set_attribute (style, "mathvariant", "normal");
						else
							lsm_dom_element_set_attribute (style, "mathvariant", "italic");
					}

					lsm_dom_element_set_attribute (style, "mathfamily",
									   pango_font_description_get_family (font));

					value = g_strdup_printf ("%gpt", pango_units_to_double (
							pango_font_description_get_size (font)));
					lsm_dom_element_set_attribute (style, "mathsize", value);
					g_free (value);
				}
				g_free (desc);
			}
		}
		if (m_AutoTextColor) {
			GOColor color = GetCanvas ()->GetColor ();
			if (color != last_color) {
				last_color = color;
				changed = true;
				value = g_strdup_printf ("#%02x%02x%02x",
							 GO_COLOR_UINT_R (color),
							 GO_COLOR_UINT_G (color),
							 GO_COLOR_UINT_B (color));
				lsm_dom_element_set_attribute (style, "mathcolor", value);
				g_free (value);
			}
		}
		if (changed)
			const_cast < gccv::Equation * > (this)->SetPosition (m_x, m_y);
	}
	if (m_View) {
		double x, y;
		GetPosition (x, y);
		lsm_dom_view_render (m_View, cr, x, y);
	}
}

void Equation::SetPosition (double x, double y)
{
	m_x = x;
	m_y = y;
	if (m_View)
		g_object_unref (m_View);
	m_View = (m_Math)? lsm_dom_document_create_view (const_cast < LsmDomDocument * > (m_Math)): NULL;
	double w, h, bl;
	if (m_View)
		lsm_dom_view_get_size (m_View, &w, &h, &bl);
	else {
		w = 2.;
		h = bl = 10.;
	}
	// Horizontal position
	switch (m_Anchor) {
	default:
	case AnchorNorth:
	case AnchorLine:
	case AnchorCenter:
	case AnchorSouth:
		x -= w / 2.;
		break;
	case AnchorNorthWest:
	case AnchorLineWest:
	case AnchorWest:
	case AnchorSouthWest:
		break;
	case AnchorNorthEast:
	case AnchorLineEast:
	case AnchorEast:
	case AnchorSouthEast:
		x-= w;
		break;
	}
	// Vertical position
	switch (m_Anchor) {
	default:
	case AnchorLine:
	case AnchorLineWest:
	case AnchorLineEast: {
		y -= bl;
		break;
	}
	case AnchorCenter:
	case AnchorWest:
	case AnchorEast:
		y -= h / 2.;
		break;
	case AnchorNorth:
	case AnchorNorthWest:
	case AnchorNorthEast:
		break;
	case AnchorSouth:
	case AnchorSouthWest:
	case AnchorSouthEast:
		y -= h;
		break;
	}
	Rectangle::SetPosition (x, y, w, h);
}

}   // namespace gccv
