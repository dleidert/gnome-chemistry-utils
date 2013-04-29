// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-arrow.cc
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "reaction-arrow.h"
#include "reaction.h"
#include "reaction-step.h"
#include "reaction-prop.h"
#include "reaction-prop-dlg.h"
#include "reaction-separator.h"
#include "document.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <gccv/arrow.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gcugtk/ui-manager.h>
#include <gcu/objprops.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

ReactionArrow::ReactionArrow (Reaction* react, unsigned Type): Arrow (ReactionArrowType)
{
	SetId ("ra1");
	m_Type = Type;
	if (react)
		react->AddChild (this);
	m_TypeChanged = false;
	m_NumberingScheme = NumberingSchemeArabic;
	m_MaxLinesAbove = 1;
}


ReactionArrow::~ReactionArrow ()
{
}

xmlNodePtr ReactionArrow::Save (xmlDocPtr xml) const
{
	xmlNodePtr node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "reaction-arrow", NULL);
	if (!node)
		return NULL;
	if (!Arrow::Save (xml, node)) {
		xmlFreeNode (node);
		return NULL;
	}
	xmlNewProp (node, (xmlChar*) "type", (xmlChar*) ((m_Type == SimpleArrow)? "single": "double"));
	if (m_Type == FullReversibleArrow)
		xmlNewProp (node, (xmlChar*) "heads", (xmlChar*) "full");
	if (GetStartStep ())
		xmlNewProp (node, (xmlChar*) "start",  (xmlChar*) GetStartStep ()->GetId ());
	if (GetEndStep ())
		xmlNewProp (node, (xmlChar*) "end",  (xmlChar*) GetEndStep ()->GetId ());
	if (GetLastStep () > 1 && m_NumberingScheme != NumberingSchemeArabic) {
		xmlNewProp (node, reinterpret_cast < xmlChar const * > ("numbering-scheme"),
		            ((m_NumberingScheme == NumberingSchemeRoman)?
		             	 reinterpret_cast < xmlChar const * > ("roman"):
			             reinterpret_cast < xmlChar const * > ("roman lower")));
	}
	if (m_MaxLinesAbove != 1) {
		char *buf = g_strdup_printf ("%u", m_MaxLinesAbove);
		xmlNewProp (node, reinterpret_cast < xmlChar const * > ("max-lines-above"), reinterpret_cast < xmlChar const * > (buf));
		g_free (buf);
	}
	SaveChildren (xml, node);
	return node;
}

bool ReactionArrow::Load (xmlNodePtr node)
{
	char *buf;
	Object *parent, *prop;
	gcu::Document *doc = GetDocument ();
	xmlNodePtr child;
	if (Arrow::Load (node)) {
		buf = (char*) xmlGetProp (node, (xmlChar*) "type");
		if (buf) {
			if (!strcmp (buf, "double")) {
				m_Type = ReversibleArrow;
				char *buf0 = (char*) xmlGetProp (node, (xmlChar*) "heads");
				if (buf0) {
					if (!strcmp (buf0, "full"))
						m_Type = FullReversibleArrow;
					xmlFree (buf0);
				}
				m_TypeChanged = true;
			}
			xmlFree (buf);
		}
		/* load children */
		child = GetNodeByName (node, "reaction-prop");
		while (child) {
			prop = CreateObject ("reaction-prop", this);
			if (prop) {
				if (!prop->Load (child))
					delete prop;
			}
			child = GetNextNodeByName (child->next, "reaction-prop");
		}
		buf = reinterpret_cast < char * > (xmlGetProp (node, (xmlChar*) "numbering-scheme"));
		if (buf) {
			if (!strcmp (buf, "roman"))
				m_NumberingScheme = NumberingSchemeRoman;
			else if (!strcmp (buf, "roman low"))
				m_NumberingScheme = NumberingSchemeRomanLow;
			xmlFree (buf);
		}
		buf = reinterpret_cast < char * > (xmlGetProp (node, (xmlChar*) "max-lines-above"));
		if (buf) {
			m_MaxLinesAbove = strtoul (buf, NULL, 10);
			xmlFree (buf);
		}

		// at this point, check if there are no duplicates for the (step, line, rank) triplet
		// and no missing lines steps or ranks
		// FIXME: write that code!

		parent = GetParent ();
		if (!parent)
			return true;
		buf = (char*) xmlGetProp (node, (xmlChar*) "start");
		if (buf) {
			doc->SetTarget (buf, reinterpret_cast <Object **> (GetStartStepPtr ()), GetParent (), this, ActionIgnore);
			xmlFree (buf);
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "end");
		if (buf) {
			doc->SetTarget (buf, reinterpret_cast <Object **> (GetEndStepPtr ()), GetParent (), this, ActionIgnore);
			xmlFree (buf);
		}
		// ensure that children are properly aligned
		PositionChildren ();
		return true;
	}
	return false;
}

void ReactionArrow::AddItem ()
{
	if (m_Item)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	switch(m_Type) {
	case SimpleArrow: {
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
		m_Item = arrow;
		break;
	}
	case ReversibleArrow: {
		double dAngle = atan2 (m_height, m_width);
		gccv::Group *group = new gccv::Group (view->GetCanvas ()->GetRoot (), this);
		gccv::Arrow *arrow = new gccv::Arrow (group,
											  m_x * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * sin (dAngle),
											  m_y * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * cos (dAngle),
											  (m_x + m_width) * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * sin (dAngle),
											  (m_y + m_height) * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * cos (dAngle),
											  this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		arrow = new gccv::Arrow (group,
								 (m_x + m_width) * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * sin (dAngle),
								 (m_y + m_height) * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * cos (dAngle),
								 m_x * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * sin (dAngle),
								 m_y * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * cos (dAngle),
								 this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		m_Item = group;
		break;
	}
	case FullReversibleArrow: {
		double dAngle = atan2 (m_height, m_width);
		gccv::Group *group = new gccv::Group (view->GetCanvas ()->GetRoot (), this);
		gccv::Arrow *arrow = new gccv::Arrow (group,
											  m_x * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * sin (dAngle),
											  m_y * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * cos (dAngle),
											  (m_x + m_width) * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * sin (dAngle),
											  (m_y + m_height) * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * cos (dAngle),
											  this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		arrow = new gccv::Arrow (group,
								 (m_x + m_width) * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * sin (dAngle),
								 (m_y + m_height) * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * cos (dAngle),
								 m_x * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * sin (dAngle),
								 m_y * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * cos (dAngle),
								 this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		m_Item = group;
		break;
	}
	}
}

void ReactionArrow::UpdateItem ()
{
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	}
	AddItem ();
}

class ReactionArrowProps: gcugtk::Dialog
{
public:
	ReactionArrowProps (ReactionArrow *arrow);
};

static void on_lines_changed (GtkSpinButton *btn, ReactionArrow *arrow)
{
	arrow->SetMaxLinesAbove (gtk_spin_button_get_value_as_int (btn));
	arrow->PositionChildren ();
}

static void on_numbering_changed (GtkComboBox *box, ReactionArrow *arrow)
{
	arrow->SetNumberingScheme (static_cast < NumberingScheme > (gtk_combo_box_get_active (box)));
	arrow->PositionChildren ();
}

ReactionArrowProps::ReactionArrowProps (ReactionArrow *arrow):
	gcugtk::Dialog (reinterpret_cast < gcugtk::Application * > (arrow->GetApplication ()), UIDIR"/reaction-arrow-prop.ui", "reaction-arrow-dlg", GETTEXT_PACKAGE, arrow)
{
	gtk_widget_show (GTK_WIDGET (dialog));
	GtkWidget *w = GetWidget ("lines-btn");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), arrow->GetMaxLinesAbove ());
	g_signal_connect (w, "value-changed", G_CALLBACK (on_lines_changed), arrow);
	w = GetWidget ("steps-combo");
	gtk_combo_box_set_active (GTK_COMBO_BOX (w), arrow->GetNumberingScheme ());
	g_signal_connect (w, "changed", G_CALLBACK (on_numbering_changed), arrow);
	if (arrow->GetLastStep () < 2) {
		gtk_widget_hide (GetWidget ("steps-lbl"));
		gtk_widget_hide (w);
	}
}
	

static void do_props (ReactionArrow *arrow)
{
	new ReactionArrowProps (arrow);
}

struct CallbackData {
	ReactionArrow *arrow;
	Object *child;
};

static void do_attach_object (struct CallbackData *data)
{
	data->arrow->AddProp (data->child);
}

static void do_free_data (struct CallbackData *data)
{
	delete data;
}

bool ReactionArrow::BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y)
{
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	Document *Doc = dynamic_cast<Document*> (GetDocument ());
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	// If there are several lines of objects attached to the arrow, add a property dialog
	bool result = false;
	GtkActionGroup *group = NULL;
	GtkAction *action;
	if (GetChildrenNumber () > 0) {
		group = gtk_action_group_new ("reaction-arrow");
		GtkAction *action = gtk_action_new ("Arrow", _("Arrow"), NULL, NULL);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		action = gtk_action_new ("props", _("Properties..."), _("Arrow properties"), NULL);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (do_props), this);
		gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Arrow'><menuitem action='props'/></menu></popup></ui>", -1, NULL);
		gtk_ui_manager_insert_action_group (uim, group, 0);
		result = true;
	}
	if (pData->SelectedObjects.size () == 1) {
		Object *obj = *pData->SelectedObjects.begin ();
		TypeId Id = obj->GetType ();
		if ((Id != MoleculeType && Id != TextType) || obj->GetGroup ())
			return Object::BuildContextualMenu (UIManager, object, x, y);
		if (group == NULL) {
			group = gtk_action_group_new ("reaction-arrow");
			action = gtk_action_new ("Arrow", _("Arrow"), NULL, NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
		}
		struct CallbackData *data = new struct CallbackData ();
		data->arrow = this;
		data->child = obj;
		action = gtk_action_new ("attach", _("Attach selection to arrow..."), NULL, NULL);
		g_object_set_data_full (G_OBJECT (action), "data", data, (GDestroyNotify) do_free_data);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (do_attach_object), data);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Arrow'><menuitem action='attach'/></menu></popup></ui>", -1, NULL);
		gtk_ui_manager_insert_action_group (uim, group, 0);
		g_object_unref (group);
		result = true;
	}
	return result || Object::BuildContextualMenu (UIManager, object, x, y);
}

void ReactionArrow::Move (double x, double y, double z)
{
	// Move the arrow
	Arrow::Move (x, y, z);
	// Move its children
	Object::Move (x, y, z);
}

void ReactionArrow::SetSelected (int state)
{
	// Select the arrow
	Arrow::SetSelected (state);
	// Select its children
//	Object::SetSelected (state);
}

void ReactionArrow::AddProp (Object *object)
{
	Document *Doc = dynamic_cast<Document*> (GetDocument ());
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	Operation *Op = Doc->GetNewOperation (GCP_MODIFY_OPERATION);
	unsigned step, line, rank;
	Op->AddObject (object, 0);
	Object *Group = GetGroup ();
	if (!Group)
		Group = this;
	Op->AddObject (Group, 0);
	ReactionProp *prop = new ReactionProp (this, object);
	// set a step: the last current one
	step = GetLastStep ();
	if (step == 0) {
		step++;
		line = 1;
		rank = 1;
	} else {
		line = GetLastLine (step);
		if (line == 0) {
			line = 1;
			rank = 1;
		} else {
			// add to a new line if last line has only one object
			rank = GetLastPos (step, line);
			if (rank < 2) {
				line++;
				rank = 1;
			} else
				rank++;
		}	}
	prop->SetStep (step);
	prop->SetLine (line);
	prop->SetRank (rank);
	// set a line: a new one if step is 0 and first line has one object or less
	line = GetLastLine (step);
	// add the child in the object tree
	AddChild (prop);
	// position the child
	PositionChild (prop);
	Op->AddObject (Group, 1);
	Doc->FinishOperation ();
	pData->UnselectAll ();
	EmitSignal (OnChangedSignal);
	new ReactionPropDlg (this, prop);
}

void ReactionArrow::PositionChild (ReactionProp *prop)
{
	// FIXME: this is experimental code
	Document *Doc = dynamic_cast<Document*> (GetDocument ());
	Theme *pTheme = Doc->GetTheme ();
	double xmin, xspan, ymin, yspan,
		length = sqrt (m_width * m_width + m_height * m_height),
		x = m_width / length, y = m_height / length;
	gccv::Rect rect;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	pData->GetObjectBounds (prop, &rect);
	if (m_width >=0) {
		if (m_height >=0) {
			xmin = (rect.x0 * x + rect.y0 * y) / pTheme->GetZoomFactor ();
			xspan = (rect.x1 * x + rect.y1 * y) / pTheme->GetZoomFactor () - xmin;
			ymin = (rect.x0 * y - rect.y1 * x) / pTheme->GetZoomFactor ();
			yspan = (rect.x1 * y - rect.y0 * x) / pTheme->GetZoomFactor () - ymin;
		} else {
			xmin = (rect.x0 * x + rect.y1 * y) / pTheme->GetZoomFactor ();
			xspan = (rect.x1 * x + rect.y0 * y) / pTheme->GetZoomFactor () - xmin;
			ymin = (rect.x0 * y - rect.y0 * x) / pTheme->GetZoomFactor ();
			yspan = (rect.x1 * y - rect.y1 * x) / pTheme->GetZoomFactor () - ymin;
		}
	} else {
		if (m_height >=0) {
			xmin = (rect.x1 * x + rect.y0 * y) / pTheme->GetZoomFactor ();
			xspan = (rect.x0 * x + rect.y1 * y) / pTheme->GetZoomFactor () - xmin;
			ymin = (rect.x1 * y - rect.y1 * x) / pTheme->GetZoomFactor ();
			yspan = (rect.x0 * y - rect.y0 * x) / pTheme->GetZoomFactor () - ymin;
		} else {
			xmin = (rect.x1 * x + rect.y1 * y) / pTheme->GetZoomFactor ();
			xspan = (rect.x0 * x + rect.y0 * y) / pTheme->GetZoomFactor () - xmin;
			ymin = (rect.x1 * y - rect.y0 * x) / pTheme->GetZoomFactor ();
			yspan = (rect.x0 * y - rect.y1 * x) / pTheme->GetZoomFactor () - ymin;
		}
	}
	xspan = fabs (xspan);
	yspan = fabs (yspan);
	// xmin and ymin will now be the current center of the object
	xspan += (2* pTheme->GetArrowObjectPadding () + pTheme->GetArrowHeadA ()) / pTheme->GetZoomFactor ();
	// adjust the arrow length if needed
	if (xspan > length) {
		m_width *= xspan / length;
		m_height *= xspan / length;
		length = xspan;
	}
	// now move the child to the right place
	length -= pTheme->GetArrowHeadA () / pTheme->GetZoomFactor ();
	length /= 2.;
	// FIXME: using GetArrowDist is a non-sense, we should have a new variable.
	yspan = yspan / 2. + pTheme->GetArrowDist () / pTheme->GetZoomFactor ();
	// calculate the vector of the needed move
	xmin = m_x + length * x + y * yspan - (rect.x0 + rect.x1) / 2. / pTheme->GetZoomFactor ();
	ymin = m_y + length * y - x * yspan - (rect.y0 + rect.y1) / 2. / pTheme->GetZoomFactor ();
	prop->Move (xmin, ymin);
	Doc->GetView ()->Update (this);
}

typedef struct {
	gcu::Object **objs;
	unsigned max;
	double width, height, ascent, descent;
} Line;

typedef struct {
	Line *lines;
	unsigned max;
} ArrowStep;

void ReactionArrow::PositionChildren ()
{
	std::set < ReactionSeparator * > separators;
	std::set < gcu::Object * > garbage;
	std::map < std::string, gcu::Object * >::iterator i;
	gcu::Object *obj = GetFirstChild (i);
	unsigned s, max_step = GetLastStep (), l, p, n;
	ArrowStep *steps = (max_step != 0)? new ArrowStep[max_step]: NULL;
	ReactionProp *prop;
	Document *doc = static_cast < Document * > (GetDocument ());
	WidgetData* data = reinterpret_cast < WidgetData * > ( g_object_get_data (G_OBJECT (doc->GetWidget ()), "data"));
	gccv::Rect rect;
	double y, ascent, descent, sep_width = 0., sep_ascent = 0., sep_descent = 0.;
	Line *line;
	bool needs_sep = false;
	ReactionSeparator *sep;
	double scale = doc->GetTheme ()->GetZoomFactor ();

	if (steps == NULL)
		return; /* there is nothing valid around there */
	// allocate lines
	for (s = 0; s < max_step; s++) {
		steps[s].max = GetLastLine (s + 1);
		if (steps[s].max > 0) {
			steps[s].lines = new Line[steps[s].max];
			for (l = 0; l < steps[s].max; l++) {
				steps[s].lines[l].max = GetLastPos (s + 1, l + 1);
				if (steps[s].lines[l].max > 0) {
					steps[s].lines[l].objs = new gcu::Object *[steps[s].lines[l].max];
					for (p = 0 ; p < steps[s].lines[l].max; p++)
						steps[s].lines[l].objs[p] = NULL;
					}
				else
					steps[s].lines[l].objs = NULL;
			}
		} else
			steps[s].lines = NULL;
	}
	while (obj) {
		if (obj->GetType () == ReactionSeparatorType)
			separators.insert (static_cast < ReactionSeparator * > (obj));
		else if (obj->GetType () == ReactionPropType) {
			prop = static_cast < ReactionProp * > (obj);
			s = prop->GetStep ();
			l = prop->GetLine ();
			p = prop->GetRank ();
			if (s * l * p == 0) // none should be nul
				garbage.insert (obj);
			s--;
			l--;
			p--;
			if (steps[s].lines[l].objs[p] == NULL)
				steps[s].lines[l].objs[p] = obj;
			else
				garbage.insert (obj);
		} else
			garbage.insert (obj);
		obj = GetNextChild (i);
	}
	// check if every object really exists to avoid crashes
	for (s = 0; s < max_step; s++) {
		for (l = 0; l < steps[s].max; l++) {
			line = steps[s].lines + l;
			for (p = 0; p < line->max; p++)
				if (line->objs[p] == NULL) {
					for (n = p + 1; n < line->max; n++)
						line->objs[n - 1] = line->objs[n];
					line->max --;
				}
			if (line->max == 0) {
				// remove the empty line
				if (line->objs)
					delete [] line->objs;
				for (n = l + 1; n < steps[s].max; n++)
					steps[s].lines[n - 1] = steps[s].lines[n];
				steps[s].max--;
			} else if (line->max > 1)
				needs_sep = true;
		}
		if (steps[s].max == 0) {
			// remove the empty step
			if (steps[s].lines)
				delete [] steps[s].lines;
			for (n = s + 1; n < max_step; n++)
				steps[n - 1] = steps[n];
			max_step--;
		}
	}
	
	// evaluate lines size
	if (needs_sep) {
		if (separators.empty ()) {
			sep = new ReactionSeparator ();
			separators.insert (sep);
			AddChild (sep);
			doc->GetView ()->AddObject (sep);
		} else
			sep = *separators.begin ();
		data->GetObjectBounds (sep, &rect);
		sep_width = (rect.x1 - rect.x0) / scale;
		y = sep->GetYAlign ();
		sep_ascent = y - rect.y0 / scale;
		sep_descent = rect.y1 / scale - y;
	}
	for (s = 0; s < max_step; s++) {
		for (l = 0; l < steps[s].max; l++) {
			line = steps[s].lines + l;
			line->width = line->ascent = line->descent = 0.;
			for (p = 0; p < line->max; p++) {
				data->GetObjectBounds (line->objs[p], &rect);
				line->width += (rect.x1 - rect. x0) / scale;
				y = line->objs[p]->GetYAlign ();
				ascent = y - rect.y0 / scale;
				descent = rect.y1 / scale - y;
				if (line->ascent < ascent)
					line->ascent = ascent;
				if (line->descent < descent)
					line->descent = descent;
			}
			if (line->max > 1) {
				line->width += sep_width * (line->max - 1);
				if (line->ascent < sep_ascent)
					line->ascent = sep_ascent;
				if (line->descent < sep_descent)
					line->descent = sep_descent;
			}
			line->height = line->ascent + line->descent;
			// FIXME: if there are several steps, add room for steps numbering
		}
	}
	// evaluate needed arrow size
	// clean memory
	for (s = 0; s < max_step; s++) {
		for (l = 0; l < steps[s].max; l++)
			delete [] steps[s].lines[l].objs;
		delete [] steps[s].lines;
	}
	delete [] steps;
	// FIXME: delete garbage if any
}

bool ReactionArrow::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (Signal == OnChangedSignal) {
		// for now we can have only one child property.
		map<string, Object*>::iterator i;
		ReactionProp *prop = dynamic_cast <ReactionProp *> (GetFirstChild (i));
		if (prop != NULL)
			PositionChild (prop);
	}
	return true;
}

bool ReactionArrow::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_REACTION_ARROW_TYPE:
		m_Type = (strcmp (value, "double"))? ReversibleArrow: SimpleArrow;
		break;
	default:
		return Arrow::SetProperty (property, value);
	}
	return true;
}

std::string ReactionArrow::Name ()
{
	return _("Reaction arrow");
}

char ReactionArrow::GetSymbolicPosition (double x, double y)
{
	x -= m_x;
	y -= m_y;
	double s = x * m_width + y * m_height;
	if (s < 0.)
		return 't';
	double l = m_width * m_width + m_height * m_height;
	return (s > l)? 'h': 'o';
}

unsigned ReactionArrow::GetLastStep () const
{
	unsigned res = 0, step;
	std::map < std::string, gcu::Object * >::const_iterator i;
	gcu::Object const *obj = GetFirstChild (i);
	ReactionProp const *prop;
	while (obj) {
		if (obj->GetType () == ReactionPropType) {
			prop = static_cast < ReactionProp const * > (obj);
			step = prop->GetStep ();
			if (step > res)
				res = step;
		}
		obj = GetNextChild (i);
	}
	return res;
}

unsigned ReactionArrow::GetLastLine (unsigned step) const
{
	unsigned res = 0, line;
	std::map < std::string, gcu::Object * >::const_iterator i;
	gcu::Object const *obj = GetFirstChild (i);
	ReactionProp const *prop;
	while (obj) {
		if (obj->GetType () == ReactionPropType) {
			prop = static_cast < ReactionProp const * > (obj);
			if (step == prop->GetStep ()) {
				line = prop->GetLine ();
				if (line > res)
					res = line;
			}
		}
		obj = GetNextChild (i);
	}
	return res;
}

unsigned ReactionArrow::GetLastPos (unsigned step, unsigned line) const
{
	unsigned res = 0, pos;
	std::map < std::string, gcu::Object * >::const_iterator i;
	gcu::Object const *obj = GetFirstChild (i);
	ReactionProp const *prop;
	while (obj) {
		if (obj->GetType () == ReactionPropType) {
			prop = static_cast < ReactionProp const * > (obj);
			if (step == prop->GetStep () && line == prop->GetLine ()) {
				pos = prop->GetRank ();
				if (pos > res)
					res = pos;
			}
		}
		obj = GetNextChild (i);
	}
	return res;
}

}	//	namespace gcp
