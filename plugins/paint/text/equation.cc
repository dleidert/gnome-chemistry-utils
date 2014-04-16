// -*- C++ -*-

/*
 * GChemPaint text plugin: math equation support
 * equation.cc
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
#include <gccv/canvas.h>
#include <gccv/equation.h>
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcugtk/dialog.h>
#include <gcu/xml-utils.h>
#include <glib/gi18n.h>

#ifndef HAVE_LSM_ITEX_TO_MATHML
/* Lasem - SVG and Mathml library
 *
 * Copyright © 2013 Emmanuel Pacaud
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author:
 * 	Emmanuel Pacaud <emmanuel@gnome.org>
 */

extern "C" char * itex2MML_parse (const char * buffer, unsigned long length);
extern "C" void   itex2MML_free_string (char * str);

static char *
lsm_itex_to_mathml (const char *itex, int size)
{
	char *mathml;

	if (itex == NULL)
		return NULL;

	if (size < 0)
		size = strlen (itex);

	mathml = itex2MML_parse (itex, size);
	if (mathml == NULL)
		return NULL;

	if (mathml[0] == '\0') {
		itex2MML_free_string (mathml);
		return NULL;
	}

	return mathml;
}

static void
lsm_itex_free_mathml_buffer (char *mathml)
{
	if (mathml == NULL)
		return;

	itex2MML_free_string (mathml);
}
#endif

gcu::TypeId EquationType;

static void on_itex_changed (GoMathEditor *me, gcpEquation *eq)
{
	char *itex = go_math_editor_get_itex (me);
	eq->ItexChanged (itex, go_math_editor_get_inline (me));
	g_free (itex);
}

static void on_font_changed (GOFontSel *fs, gpointer, gcpEquation *eq)
{
	PangoFontDescription *desc = go_font_sel_get_font_desc (fs);
	GOColor color = go_font_sel_get_color (fs);
	if ((eq->GetITeX ().length () != 0) && (!pango_font_description_equal (eq->GetFont (), desc) || (eq->GetColor () != color))) {
		gcp::Document *doc = static_cast < gcp::Document * > (eq->GetDocument ());
		gcp::Operation *op = doc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
		op->AddObject (eq);
		eq->SetColor (color); // color first to force the update (kludge)
		eq->SetFontDesc (desc);
		op->AddObject (eq, 1);
		doc->FinishOperation ();
		doc->GetView ()->Update (eq);
	}
	pango_font_description_free (desc);
}

static bool on_unselect (gcpEquation *equation) {
	equation->SetSelected (gcp::SelStateUnselected);
	return false;
}

static bool on_delete (gcpEquation *equation)
{
	if (equation->IsEmpty ())
		delete equation;
	else
		g_idle_add ((GSourceFunc) on_unselect, equation);
	return false;
}

// Properties dialog
class gcpEquationProps: public gcugtk::Dialog
{
public:
	friend class gcpEquation;
	gcpEquationProps (gcp::Document *doc, gcpEquation *equation);
	~gcpEquationProps ();

	void Update ();

private:
	gcpEquation *m_Equation;
	gcp::Document *m_Doc;
	GoMathEditor *m_Me;
};

gcpEquationProps::gcpEquationProps (gcp::Document *doc, gcpEquation *equation):
	gcugtk::Dialog (doc? doc->GetApplication (): NULL, UIDIR"/eq-props.ui", "equation-properties", GETTEXT_PACKAGE, equation),
	m_Equation (equation),
	m_Doc (doc)

{
	GtkNotebook *nb = GTK_NOTEBOOK (GetWidget ("notebook"));
	GtkWidget *me = go_math_editor_new ();
	m_Me = GO_MATH_EDITOR (me);
	go_math_editor_set_itex (m_Me, equation->GetITeX ().c_str ());
	g_signal_connect (me, "itex-changed", G_CALLBACK (on_itex_changed), equation);
	g_signal_connect (me, "inline-changed", G_CALLBACK (on_itex_changed), equation);
	gtk_notebook_append_page (nb, me, gtk_label_new (_("ITeX string")));
	GtkWidget *fs = GTK_WIDGET (g_object_new (GO_TYPE_FONT_SEL, "show-color", true, NULL));
	GOFont const *font = go_font_new_by_desc (pango_font_description_copy (equation->GetFont ()));
	go_font_sel_set_font (GO_FONT_SEL (fs), font);
	go_font_sel_set_color (GO_FONT_SEL (fs), equation->GetColor (), false);
	go_font_unref (font);
	g_signal_connect (fs, "font-changed", G_CALLBACK (on_font_changed), equation);
	gtk_notebook_append_page (nb, fs, gtk_label_new (_("Font")));
	gtk_widget_show_all (GTK_WIDGET (nb));
	g_signal_connect_swapped (GetWindow (), "delete-event", G_CALLBACK (on_delete), equation);
	g_signal_connect_swapped (GetWindow (), "response", G_CALLBACK (on_delete), equation);
}

gcpEquationProps::~gcpEquationProps ()
{
}

void gcpEquationProps::Update ()
{
}

// gcpEquation class implementation

gcpEquation::gcpEquation (double x, double y):
	gcu::Object (EquationType),
	gcu::DialogOwner (),
	gccv::ItemClient (),
	m_x (x), m_y (y),
	m_Math (NULL),
	m_AutoFont (true),
	m_Font (NULL),
	m_Color (GO_COLOR_BLACK),
	m_Inline (false)
{
	SetId ("eq1");
	m_Math = lsm_dom_implementation_create_document (NULL, "math");
	LsmDomNode *math_element = LSM_DOM_NODE (lsm_dom_document_create_element (m_Math, "math"));
	m_StyleElement = LSM_DOM_NODE (lsm_dom_document_create_element (m_Math, "mstyle"));
	LsmDomNode *itex_element = LSM_DOM_NODE (lsm_dom_document_create_element (m_Math, "lasem:itex"));
	m_ItexString = LSM_DOM_NODE (lsm_dom_document_create_text_node (m_Math, ""));

	lsm_dom_node_append_child (LSM_DOM_NODE (m_Math), math_element);
	lsm_dom_node_append_child (math_element, m_StyleElement);
	lsm_dom_node_append_child (m_StyleElement, itex_element);
	lsm_dom_node_append_child (itex_element, m_ItexString);
}

gcpEquation::~gcpEquation ()
{
	if (m_Math)
		g_object_unref (m_Math);
	if (m_Font)
		pango_font_description_free (m_Font);
}

void gcpEquation::AddItem ()
{
	if (m_Item)
		return;
	gcp::Document *doc = static_cast <gcp::Document*> (GetDocument ());
	gcp::View *view = doc->GetView ();
	gcp::Theme *theme = doc->GetTheme ();
	gccv::Equation *eq = new gccv::Equation (view->GetCanvas ()->GetRoot (),
	                                         m_x * theme->GetZoomFactor (),
	                                         m_y * theme->GetZoomFactor (),
	                                         this);
	m_Item = eq;
	eq->SetAnchor (gccv::AnchorLineWest);
	eq->SetMath ((m_Itex.length ())? m_Math: NULL);
}

void gcpEquation::UpdateItem ()
{
	if (!m_Item)
		return;
	gccv::Equation *eq = static_cast < gccv::Equation * > (m_Item);
	gcp::Document *doc = static_cast <gcp::Document*> (GetDocument ());
	gcp::Theme *theme = doc->GetTheme ();
	eq->SetMath ((m_Itex.length ())? m_Math: NULL);
	eq->SetPosition (m_x * theme->GetZoomFactor (), m_y * theme->GetZoomFactor ());
}

xmlNodePtr gcpEquation::Save (xmlDocPtr xml) const
{
	if (m_Itex.length () == 0)
		return NULL;
	char *buf;
	xmlNodePtr node = xmlNewDocNode (xml, NULL,
	                                 reinterpret_cast <xmlChar const *> ("equation"),
	                                 reinterpret_cast <xmlChar const *> (m_Itex.c_str ()));
	SaveId (node);
	// save the position
	gcu::WritePosition (xml, node, NULL, m_x, m_y);	
	// save the style
	if (!m_AutoFont) {
		buf = pango_font_description_to_string (m_Font);
		xmlNewProp (node, reinterpret_cast < xmlChar const * > ("font"), reinterpret_cast < xmlChar * > (buf));
		g_free (buf);
	}
	if (m_Color != GO_COLOR_BLACK) {
		buf = go_color_as_str (m_Color);
		xmlNewProp (node, reinterpret_cast < xmlChar const * > ("color"), reinterpret_cast < xmlChar * > (buf));
		g_free (buf);
	}

	return node;
}

bool gcpEquation::Load (xmlNodePtr node)
{
	xmlChar *buf = xmlGetProp (node, (xmlChar*) "id");
	if (buf) {
		SetId (reinterpret_cast <char *> (buf));
		xmlFree (buf);
	}
	if (!gcu::ReadPosition (node, NULL, &m_x, &m_y))
		return false;
	buf = xmlGetProp (node, reinterpret_cast < xmlChar const * > ("color"));
	if (buf) {
		if (!go_color_from_str (reinterpret_cast <char *> (buf), &m_Color))
			m_Color = GO_COLOR_BLACK;
		xmlFree (buf);
	}
	buf = xmlGetProp (node, reinterpret_cast < xmlChar const * > ("font"));
	if (buf) {
		PangoFontDescription *desc = pango_font_description_from_string (reinterpret_cast <char *> (buf));
		if (desc) {
			SetFontDesc (desc);
			pango_font_description_free (desc);
		}
		xmlFree (buf);
	}
	buf = xmlNodeGetContent (node);
	if (buf) {
		m_Itex = reinterpret_cast <char *> (buf);
		lsm_dom_node_set_node_value (m_ItexString, m_Itex.c_str ());
		xmlFree (buf);
	}
	// FIXME: populate editor if opened (might happen on undo/redo operations)
	gcpEquationProps *dlg = static_cast < gcpEquationProps * > (GetDialog ("equation-properties"));
	if (dlg) {
	}
	return m_Itex.length () > 0;
}

void gcpEquation::SetSelected (int state)
{
	if (!m_Item)
		return;
	GOColor color;
	switch (state) {
	case gcp::SelStateUnselected:
		color = (GetDialog ("equation-properties"))? gcp::AddColor: 0;
		break;
	case gcp::SelStateSelected:
		color = gcp::SelectColor;
		break;
	case gcp::SelStateUpdating:
		color = gcp::AddColor;
		break;
	case gcp::SelStateErasing:
		color = gcp::DeleteColor;
		break;
	default:
		color = 0;
		break;
	}
	static_cast <gccv::LineItem * > (m_Item)->SetLineColor (color);
}

std::string gcpEquation::Name ()
{
	return _("Equation");
}

char const *gcpEquation::HasPropertiesDialog () const
{
	return "equation-properties";
}

gcu::Dialog *gcpEquation::BuildPropertiesDialog ()
{
	gcp::Document *doc = static_cast <gcp::Document *> (GetDocument ());
	gcu::Dialog *dlg = new gcpEquationProps (doc, this);
	return dlg;
}

void gcpEquation::ParentChanged ()
{
	if ((m_Font == NULL) || m_AutoFont) {
		if (m_Font)
			pango_font_description_free (m_Font);
		gcp::Document *doc = static_cast <gcp::Document *> (GetDocument ());
		gcp::Theme *theme = doc->GetTheme ();
		m_Font = pango_font_description_new ();
		pango_font_description_set_family (m_Font, theme->GetTextFontFamily ());
		pango_font_description_set_size (m_Font, theme->GetTextFontSize ());
		pango_font_description_set_style (m_Font, theme->GetTextFontStyle ());
		pango_font_description_set_weight (m_Font, theme->GetTextFontWeight ());
		pango_font_description_set_stretch (m_Font, theme->GetTextFontStretch ());
		pango_font_description_set_variant (m_Font, theme->GetTextFontVariant ());
		if (m_Math)
			UpdateFont ();
	}
}

void gcpEquation::SetFontDesc (PangoFontDescription const *desc)
{
	if (pango_font_description_equal (m_Font, desc))
		return;
	m_Font = pango_font_description_copy (desc);
	m_AutoFont = false;
	if (m_Math)
		UpdateFont ();
}

void gcpEquation::UpdateFont ()
{
	char *value;
	LsmDomElement *style = LSM_DOM_ELEMENT (m_StyleElement);
	if (pango_font_description_get_weight (m_Font) >= PANGO_WEIGHT_BOLD) {
		if (pango_font_description_get_style (m_Font) == PANGO_STYLE_NORMAL)
			lsm_dom_element_set_attribute (style, "mathvariant", "bold");
		else
			lsm_dom_element_set_attribute (style, "mathvariant", "bold-italic");
	} else {
		if (pango_font_description_get_style (m_Font) == PANGO_STYLE_NORMAL)
			lsm_dom_element_set_attribute (style, "mathvariant", "normal");
		else
			lsm_dom_element_set_attribute (style, "mathvariant", "italic");
	}

	lsm_dom_element_set_attribute (style, "mathfamily",
					   pango_font_description_get_family (m_Font));

	value = g_strdup_printf ("%gpt", pango_units_to_double (
			pango_font_description_get_size (m_Font)));
	lsm_dom_element_set_attribute (style, "mathsize", value);
	g_free (value);
	value = g_strdup_printf ("#%02x%02x%02x",
				 GO_COLOR_UINT_R (m_Color),
				 GO_COLOR_UINT_G (m_Color),
				 GO_COLOR_UINT_B (m_Color));
	lsm_dom_element_set_attribute (style, "mathcolor", value);
	g_free (value);
}

void gcpEquation::ItexChanged (char const *itex, bool compact)
{
	if (m_Itex == itex && compact == m_Inline)
		return;
	char *mathml = NULL;
	if (*itex) {
		std::string full_itex = (compact)? "$": "\\[";
		full_itex += itex;
		full_itex += (compact)? "$": "\\]";
		mathml = lsm_itex_to_mathml (full_itex.c_str (), -1);
		if (mathml == NULL)
			return;
	}
	gcp::Operation *op;
	gcp::Document *doc = static_cast < gcp::Document * > (GetDocument ());
	unsigned target = 0;
	if (m_Itex.length () != 0) {
		op = doc->GetNewOperation ((*itex)? gcp::GCP_MODIFY_OPERATION: gcp::GCP_DELETE_OPERATION);
		op->AddObject (this);
		target = 1;
	} else
		op = doc->GetNewOperation (gcp::GCP_ADD_OPERATION);
	if (m_Itex != itex) {
		m_Itex = itex;
		g_object_unref (m_Math);
		m_Math = lsm_dom_implementation_create_document (NULL, "math");
		LsmDomNode *math_element = LSM_DOM_NODE (lsm_dom_document_create_element (m_Math, "math"));
		m_StyleElement = LSM_DOM_NODE (lsm_dom_document_create_element (m_Math, "mstyle"));
		LsmDomNode *itex_element = LSM_DOM_NODE (lsm_dom_document_create_element (m_Math, "lasem:itex"));
		m_ItexString = LSM_DOM_NODE (lsm_dom_document_create_text_node (m_Math, itex));

		lsm_dom_node_append_child (LSM_DOM_NODE (m_Math), math_element);
		lsm_dom_node_append_child (math_element, m_StyleElement);
		lsm_dom_node_append_child (m_StyleElement, itex_element);
		lsm_dom_node_append_child (itex_element, m_ItexString);
	}
	if (compact != m_Inline) {
		m_Inline = compact;
		lsm_dom_element_set_attribute (LSM_DOM_ELEMENT (m_StyleElement),
		                               "displaystyle", compact ? "false" : "true");
	}
	UpdateFont ();
	if (*itex)
		op->AddObject (this, target);
	doc->FinishOperation ();
	lsm_itex_free_mathml_buffer (mathml);
	doc->GetView ()->Update (this);
}

void gcpEquation::Move (double x, double y, double)
{
	m_x += x;
	m_y += y;
}
