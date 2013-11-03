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
#include "step-counter.h"
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
#include <vector>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

typedef struct {
	ReactionProp *obj;
	double x, y, width, height, ascent;
} ObjPos;

class ReactionArrowLine
{
public:
	unsigned m_nProps;
	double x, y, width, height, descent, ascent;
	std::list < ObjPos > m_Props;
};

class ReactionArrowStep
{
public:
	ReactionArrowStep ();
	~ReactionArrowStep ();

	StepCounter *m_Counter;
	unsigned m_nLines;
	std::list < ReactionArrowLine * > m_Lines;
};

ReactionArrowStep::ReactionArrowStep ():
	m_Counter (NULL)
{
}

ReactionArrowStep::~ReactionArrowStep ()
{
}

ReactionArrow::ReactionArrow (Reaction* react, unsigned Type): Arrow (ReactionArrowType)
{
	SetId ("ra1");
	m_Type = Type;
	if (react)
		react->AddChild (this);
	m_TypeChanged = false;
	m_NumberingScheme = NumberingSchemeArabic;
	m_MaxLinesAbove = 1;
	m_nSteps = 0;
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
	if (m_nSteps > 1 && m_NumberingScheme != NumberingSchemeArabic) {
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
		doc->NotifyDirty (this);
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

class ReactionArrowProps: public gcugtk::Dialog
{
public:
	ReactionArrowProps (ReactionArrow *arrow);
};

static void on_lines_changed (GtkSpinButton *btn, ReactionArrow *arrow)
{
	Document *doc = static_cast < Document * > (arrow->GetDocument ());
	Operation *op = doc->GetNewOperation (GCP_MODIFY_OPERATION);
	gcu::Object *obj = arrow->GetGroup ();
	if (obj == NULL)
		obj = arrow;
	op->AddObject (obj);
	arrow->SetMaxLinesAbove (gtk_spin_button_get_value_as_int (btn));
	arrow->PositionChildren ();
	op->AddObject (obj, 1);
	doc->FinishOperation ();
}

static void on_numbering_changed (GtkComboBox *box, ReactionArrow *arrow)
{
	Document *doc = static_cast < Document * > (arrow->GetDocument ());
	Operation *op = doc->GetNewOperation (GCP_MODIFY_OPERATION);
	op->AddObject (arrow);
	arrow->SetNumberingScheme (static_cast < NumberingScheme > (gtk_combo_box_get_active (box)));
	arrow->PositionChildren ();
	op->AddObject (arrow, 1);
	doc->FinishOperation ();
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
	gcu::Dialog *dialog = arrow->GetDialog ("reaction-arrow-dlg");
	if (dialog)
		dialog->Present ();
	else
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
	Op->AddObject (object, 0);
	Object *Group = GetGroup ();
	ReactionArrowStep *astep;
	ReactionArrowLine *aline;
	if (!Group)
		Group = this;
	Op->AddObject (Group, 0);
	ReactionProp *prop = new ReactionProp (this, object);
	// set a step: the last current one
	if (m_nSteps == 0) {
		astep = new ReactionArrowStep ();
		m_Steps.push_back (astep);
		m_nSteps++;
		aline = new ReactionArrowLine ();
		astep->m_Lines.push_back (aline);
		aline->m_nProps = 1;
	} else {
		astep = m_Steps.back ();
		aline = astep->m_Lines.back ();
		if (aline->m_Props.size () < 2) {
			// add to a new line if last line has only one object
			aline = new ReactionArrowLine ();
			astep->m_Lines.push_back (aline);
		}
	}
	prop->SetStep (m_nSteps);
	prop->SetLine (astep->m_Lines.size ());
	ObjPos pos = {prop, 0., 0., 0., 0., 0.};
	aline->m_Props.push_back (pos);
	prop->SetRank (aline->m_Props.size ());
	// add the child in the object tree
	AddChild (prop);
	// position the child
	PositionChildren ();
	Op->AddObject (Group, 1);
	Doc->FinishOperation ();
	pData->UnselectAll ();
	EmitSignal (OnChangedSignal);
	new ReactionPropDlg (this, prop);
}
/*
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
}*/
/*
typedef struct {
	ObjPos *objs;
	double *ascents;
	unsigned max;
	double width, height, ascent, descent;
} Line;

typedef struct {
	Line *lines;
	unsigned max;
} ArrowStep;*/

void ReactionArrow::PositionChildren ()
{
	std::vector < StepCounter * > counters;
	std::set < ReactionSeparator * > separators;
	std::set < gcu::Object * > garbage;
	std::map < std::string, gcu::Object * >::iterator i;
	gcu::Object *obj = GetFirstChild (i);
	unsigned max_step = m_nSteps, p, cur = 0;
	Document *doc = static_cast < Document * > (GetDocument ());
	Theme const *theme = doc->GetTheme ();
	WidgetData* data = reinterpret_cast < WidgetData * > ( g_object_get_data (G_OBJECT (doc->GetWidget ()), "data"));
	gccv::Rect rect;
	double y, ascent, descent, sep_width = 0., sep_ascent = 0., sep_descent = 0.,
		   counter_width = 0., counter_ascent = 0., counter_descent = 0.,
		   uwidth = 0., uheight = 0., lwidth = 0., lheight = 0.;
	bool needs_sep = false;
	ReactionSeparator *sep;
	double scale = theme->GetZoomFactor (), padding = theme->GetPadding ();
	StepCounter *counter;
	double lxspan, lyspan, uxspan, uyspan, x, length, xspan, yspan, xmin, ymin, width, xc, yc;
	unsigned cur_line;
	ReactionArrowStep *step;
	ReactionArrowLine *line;
	std::list < ReactionArrowStep * >::iterator is, isend = m_Steps.end ();
	std::list < ReactionArrowLine * >::iterator il, ilend;
	std::list < ObjPos >::iterator ip, ipend;

	if (max_step == 0)
		return; /* there is nothing valid around there */
	// TODO: add counters to first steps if needed
	counters.resize (max_step, NULL);
	while (obj) {
		if (obj->GetType () == ReactionSeparatorType)
			separators.insert (static_cast < ReactionSeparator * > (obj));
		else if (obj->GetType () == StepCounterType) {
			counter = static_cast < StepCounter * > (obj);
			counters[counter->GetStep ()] = counter;
		} else if (obj->GetType () == ReactionPropType) {
			if (!needs_sep && static_cast < ReactionProp * > (obj)->GetRank () > 1)
				needs_sep = true;
		}
		else
			garbage.insert (obj);
		obj = GetNextChild (i);
	}
	// FIXME: add counters
	
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
	// evaluate step counters size, and create them if needed
//	for (s = 0; s < max_step; s++) {
//		if (counters[s] == NULL) {
//			counters[s] = new StepCounter (s + 1, m_NumberingScheme);
//			AddChild (counters[s]);
//			doc->GetView ()->AddObject (counters[s]);
//		}
//		data->GetObjectBounds (counters[s], &rect);
//		y = (rect.x1 - rect.x0) / scale;
//		if (y > counter_width)
//			counter_width = y;
//		rect.y0 /= scale;
//		rect.y1 /= scale;
//		y = counters[s]->GetYAlign ();
//		if (y -rect.y0 > counter_ascent)
//			counter_ascent = y - rect.y0;
//		if (rect.y1 -y > counter_descent)
//			counter_descent = rect.y1 - y;
//	}
	
	for (is = m_Steps.begin (); is != isend; is++) {
		step = *is;
		ilend = step->m_Lines.end ();
		for (il = step->m_Lines.begin (); il != ilend; il++) {
			line = *il;
			line->width = line->ascent = line->descent = 0.;
			ipend = line->m_Props.end ();
			for (ip = line->m_Props.begin (); ip != ipend; ip++) {
				obj = (*ip).obj;
				data->GetObjectBounds (obj, &rect);
				(*ip).x = rect.x0 / scale;
				(*ip).y = rect.y0 / scale;
				(*ip).width = (rect.x1 - rect. x0) / scale;
				(*ip).height = (rect.y1 - rect. y0) / scale;
				line->width += (*ip).width;
				y = obj->GetYAlign ();
				ascent = y - (*ip).y;
				(*ip).y += ascent;
				descent = rect.y1 / scale - y;
				if (line->ascent < ascent)
					line->ascent = ascent;
				if (line->descent < descent)
					line->descent = descent;
			}
			if (line->m_Props.size () > 1) {
				line->width += sep_width * (line->m_Props.size () - 1);
				if (line->ascent < sep_ascent)
					line->ascent = sep_ascent;
				if (line->descent < sep_descent)
					line->descent = sep_descent;
			}
			if (max_step > 1) {
				// make room for steps numbering.
				line->width += counter_width;
				if (line->ascent < counter_ascent)
					line->ascent = counter_ascent;
				if (line->descent < counter_descent)
					line->descent = counter_descent;
			}
			line->height = line->ascent + line->descent;
			if (cur < m_MaxLinesAbove) {
				uheight += line->height;
				if (line->width > uwidth)
					uwidth = line->width;
			} else {
				lheight += line->height;
				if (line->width > lwidth)
					lwidth = line->width;
			}
			cur++;
		}
	}
	// evaluate needed arrow size
	if (cur > m_MaxLinesAbove) {
		uheight += (m_MaxLinesAbove - 1) * padding;
		cur ++;
		if (cur > m_MaxLinesAbove)
			lheight = (cur - m_MaxLinesAbove) * padding;
	} else
		uheight += (cur - 1) * padding;
	cur = 0;
	length = sqrt (m_width * m_width + m_height * m_height);
	x = fabs (m_width / length);
	y = fabs (m_height / length);
	lxspan = fabs (lwidth * x + lheight * y);
	lyspan = fabs (lwidth * y  + lheight * x);
	uxspan = fabs (uwidth * x + uheight * y);
	uyspan = fabs (uwidth * y  + uheight * x);
	xspan = (uxspan > lxspan)? uxspan: lxspan,
	xspan += (2* theme->GetArrowObjectPadding () + theme->GetArrowHeadA ()) / theme->GetZoomFactor ();
	// adjust the arrow length if needed
	if (xspan > length) {
		m_width *= xspan / length;
		m_height *= xspan / length;
		length = xspan;
	}
	m_MinLength = xspan;
	length -= theme->GetArrowHeadA () / theme->GetZoomFactor ();
	length /= 2.;
	// position children
	// FIXME: using GetArrowDist is a non-sense, we should have a new variable.
	yspan = uyspan / 2. + theme->GetArrowDist () / theme->GetZoomFactor ();
	if (x < 0 || (x < 1e-5 && y < 0))
		yspan = -yspan;
	// if the arrow is almost horizontal upper and lower blocks should be aligned
	if (x != 0. && fabs (y / x) < 0.05)
		uxspan = lxspan = MAX (uxspan, lxspan);
	// first evaluate upper block position (actually first line position)
	xmin = m_x + length * x + y * yspan - uxspan / 2.;
	ymin = m_y + length * y - x * yspan - uyspan / 2.;
	width = uxspan;
	cur_line = 0;
	for (is = m_Steps.begin (); is != isend; is++) {
		step = *is;
		ilend = step->m_Lines.end ();
		for (il = step->m_Lines.begin (); il != ilend; il++) {
			if (cur_line == m_MaxLinesAbove) {
				yspan = lyspan / 2. + theme->GetArrowDist () / theme->GetZoomFactor ();
				xmin = m_x + length * x - y * yspan - lxspan / 2.;
				ymin = m_y + length * y + x * yspan - lyspan / 2.;
				width = lxspan;
			}
			line = (*il);
			xc = xmin;
			if (max_step == 1) // center the lines instead of left align
				xc += (width - line->width) / 2.;
			yc = ymin + line->ascent;
			ipend = line->m_Props.end ();
			p = 0;
			for (ip = line->m_Props.begin (); ip != ipend; ip++) {
				if (p++ > 0) {
					// insert a separator
					if (separators.empty ()) {
						sep = new ReactionSeparator ();
						separators.insert (sep);
						AddChild (sep);
						doc->GetView ()->AddObject (sep);
					} else {
						sep = *separators.begin ();
						separators.erase (sep);
					}
					sep->SetCoords (xc, yc);
					xc += sep_width;
				}
				(*ip).obj->Move (xc - (*ip).x, yc - (*ip).y);
				xc += (*ip).width;
			}
			ymin += line->height;
			cur_line++;
		}
	}
	// FIXME: delete garbage if any
	doc->GetView ()->Update (this);
	// remove extra separators
	std::set < ReactionSeparator * >::iterator sp, spend = separators.end ();
	for (sp = separators.begin (); sp != spend; sp++)
		delete *sp;
}

void ReactionArrow::OnLoaded ()
{
	unsigned s, l, p;
	std::map < std::string, gcu::Object * >::iterator i;
	gcu::Object *obj = GetFirstChild (i);
	ReactionProp *prop;
	Document *doc = static_cast < Document * > (GetDocument ());
	ReactionArrowStep *step = NULL;
	ReactionArrowLine *line;
	std::list < ReactionArrowStep * >::iterator is, isend;
	std::list < ReactionArrowLine * >::iterator il, ilend;
	std::list < ObjPos >::iterator ip, ipend;
	std::set < gcu::Object * > garbage;
	ObjPos pos = {NULL, 0., 0., 0., 0., 0.};

	// we don't need to destroy extra steps at this point, since empty ones will be later
	while (obj) {
		if (obj->GetType () == ReactionPropType) {
			prop = static_cast < ReactionProp * > (obj);
			s = prop->GetStep ();
			l = prop->GetLine ();
			p = prop->GetRank ();
			if (s * l * p == 0) 
				garbage.insert (obj);
			while (s > m_nSteps) {
				step = new ReactionArrowStep ();
				m_Steps.push_back (step);
				m_nSteps++;
				step->m_nLines = 1;
				line = new ReactionArrowLine ();
				line->m_nProps = 0;
				step->m_Lines.push_back (line);
			}
			if (step == NULL) {
				is = m_Steps.begin ();
				while (s > 1) {
					s--;
					is++;
				}
				step = *is; // we are sure that the iterator is valid
			}
			line = NULL;
			while (l > step->m_nLines) {
				step->m_nLines++;
				line = new ReactionArrowLine ();
				line->m_nProps = 0;
				step->m_Lines.push_back (line);
			}
			if (line == NULL) {
				il = step->m_Lines.begin ();
				while (l > 1) {
					l--;
					il++;
				}
				line = *il;
			}
			if (p > line->m_nProps) {
				while (p > line->m_nProps) {
					line->m_Props.push_back (pos);
					line->m_nProps++;
				}
			}
			ip = line->m_Props.begin ();
			while (p > 1) {
				p--;
				ip++;
			}
			if ((*ip).obj == NULL)
				(*ip).obj = prop;
			else
				// we can't have two objects at the same position
				garbage.insert (obj);
		} else {
			// we can't have objects that are not ReactionProp instances
			garbage.insert (obj);
		}
		obj = GetNextChild (i);
	}
	// check if every object really exists to avoid crashes
	isend = m_Steps.end ();
	s = 1;
	for (is = m_Steps.begin (); is != isend; is++) {
		step = *is;
		ilend = step->m_Lines.end ();
		l = 1;
		for (il = step->m_Lines.begin (); il != ilend; il++) {
			line = *il;
			ipend = line->m_Props.end ();
			p = 1;
			for (ip = line->m_Props.begin (); ip != ipend; ip++) {
				if ((*ip).obj == NULL) {
					ip = line->m_Props.erase (ip);
					line->m_nProps--;
					ipend = line->m_Props.end ();
					if (ip == ipend)
						break;
				} else {
					if ((*ip).obj->GetRank () != p)
						(*ip).obj->SetRank (p);
					p++;
					if ((*ip).obj->GetLine () != l)
						(*ip).obj->SetLine (l);
					if ((*ip).obj->GetStep () != s)
						(*ip).obj->SetStep (s);
				}
			}
			if (line->m_Props.empty ()) {
				il = step->m_Lines.erase (il);
				delete line;
				step->m_nLines--;
				ilend = step->m_Lines.end ();
				if (il == ilend)
					break;
			} else
				l++;
		}
		if (step->m_Lines.empty ()) {
			is = m_Steps.erase (is);
			delete step;
			m_nSteps--;
			isend = m_Steps.end ();
			if (is == isend)
				break;
		} else
			s++;
	}
	// remove garbage
	std::set < gcu::Object * >::iterator j, jend = garbage.end ();
	for (j = garbage.begin (); j != jend; j++) {
		if ((*j)->GetType () == ReactionPropType)
			static_cast < ReactionProp * > (*j)->GetObject ()->SetParent (doc);
		else
			(*j)->SetParent (doc);
		delete (*j);
	}
	PositionChildren ();
}

bool ReactionArrow::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (Signal == OnChangedSignal)
		PositionChildren ();
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
	return m_nSteps;
}

unsigned ReactionArrow::GetLastLine (unsigned step) const
{
	if (step == 0 || step > m_nSteps)
		return 0;
	std::list < ReactionArrowStep * >::const_iterator is = m_Steps.begin ();
	while (step > 1) {
		step--;
		is++;
	}
	return (*is)->m_nLines;
}

unsigned ReactionArrow::GetLastPos (unsigned step, unsigned line) const
{
	if (step == 0 || step > m_nSteps || line == 0)
		return 0;
	std::list < ReactionArrowStep * >::const_iterator is = m_Steps.begin ();
	while (step > 1) {
		step--;
		is++;
	}
	if ((*is)->m_nLines < line)
		return 0;
	std::list < ReactionArrowLine * >::const_iterator il = (*is)->m_Lines.begin ();
	while (line > 1) {
		line--;
		il++;
	}
	return (*il)->m_nProps;
}

void ReactionArrow::SetChildPos (ReactionProp *prop, unsigned step, unsigned line, unsigned rank)
{
	unsigned s, l, r, cur;
	s = prop->GetStep ();
	l = prop->GetLine ();
	r = prop->GetRank ();
	if ((s == step && l == line && r == rank) || step < 1 || line < 1 || rank < 1)
		return;
	Document *doc = static_cast < Document * > (GetDocument ());
	Operation *op = doc->GetNewOperation (GCP_MODIFY_OPERATION);
	gcu::Object *obj = GetGroup ();
	if (obj == NULL)
		obj = this;
	op->AddObject (obj);
	if (step != s)
		line = 0; // we put the object into a new line
	else if (line != l)
		rank = 0; // we put the object at line end
	std::list < ReactionArrowStep * >::iterator is = m_Steps.begin (), isend; // origin step
	cur = s;
	while (cur > 1)  {
		cur--;
		is++;
	}
	std::list < ReactionArrowLine * >::iterator il = (*is)->m_Lines.begin (), ilend; // origin step
	cur = l;
	while (cur > 1)  {
		cur--;
		il++;
	}
	std::list < ObjPos >::iterator ir = (*il)->m_Props.begin (), irend; // origin step
	cur = r;
	while (cur > 1)  {
		cur--;
		ir++;
	}
	ReactionArrowStep *ars = *is;
	ReactionArrowLine *arl = *il;
	// remove object from its current position
	arl->m_nProps--;
	if (arl->m_nProps == 0) {
		if (line > l)
			line--;
		ars->m_nLines--;
		delete *il;
		if (ars->m_nLines == 0) {
			m_nSteps--;
			delete *is;
			is = m_Steps.erase (is);
			isend = m_Steps.end ();
			if (step > s)
				step--;
			while (is != isend) {
				ilend = (*is)->m_Lines.end ();
				for (il = (*is)->m_Lines.begin (); il != ilend; il++) {
					irend = (*il)->m_Props.end ();
					for (ir = (*il)->m_Props.begin (); ir != irend; ir++)
						(*ir).obj->SetStep (s);
				}
				s++;
			}
		} else {
			il = ars->m_Lines.erase (il);
			ilend = ars->m_Lines.end ();
			while (il != ilend) {
				irend = (*il)->m_Props.end ();
				for (ir = (*il)->m_Props.begin (); ir != irend; ir++)
					(*ir).obj->SetLine (l);
				l++;
			}
			
		}
	} else {
		ir = arl->m_Props.erase (ir);
		irend = arl->m_Props.end ();
		while (ir != irend) {
			(*ir).obj->SetRank (s++);
			ir++;
		}
	}
	// and now, insert the object at its new position
	if (step > m_nSteps) {
		step = ++m_nSteps;
		// create the new step
		ars = new ReactionArrowStep ();
		m_Steps.push_back (ars);
		arl = new ReactionArrowLine ();
		ars->m_nLines = 1;
		ars->m_Lines.push_front (arl);
		line = 1;
		rank = 1;
	} else {
		is = m_Steps.begin ();
		cur = step;
		while (cur > 1)  {
			cur--;
			is++;
		}
		ars = *is;
		if (line == 0)
			line = ars->m_nLines + 1;
	}
	prop->SetStep (step);
	if (line > ars->m_nLines) {
		line = ++ars->m_nLines;
		arl = new ReactionArrowLine ();
		ars->m_Lines.push_back (arl);
		rank = 1;
	} else {
		il = ars->m_Lines.begin ();
		cur = line;
		while (cur > 1)  {
			cur--;
			il++;
		}
		arl = *il;
		if (rank == 0)
			rank = arl->m_nProps + 1;
	}
	prop->SetLine (line);
	prop->SetRank (rank);
	ObjPos pos = {prop, 0., 0., 0., 0., 0.};
	if (rank > arl->m_nProps)
		arl->m_Props.push_back (pos);
	else {
		cur = rank;
		ir = arl->m_Props.begin ();
		while (cur > 1) {
			cur--;
			ir++;
		}
		ir = arl->m_Props.insert (ir, pos);
		irend = arl->m_Props.end ();
		ir++;
		while (ir != irend)
			(*ir++).obj->SetRank (++rank);
	}
	arl->m_nProps++;
	PositionChildren ();
	op->AddObject (obj, 1);
	doc->FinishOperation ();
	// update children properties dialogs if any
	std::map < std::string, gcu::Object * >::iterator i;
	obj = GetFirstChild (i);
	while (obj) {
		if (obj->GetType () == ReactionPropType) {
			gcu::Dialog *dialog = static_cast < ReactionProp * > (obj)->GetDialog ("reaction-prop");
			if (dialog)
				static_cast < ReactionPropDlg * > (dialog)->Update ();
		}
		obj = GetNextChild (i);
	}
}

}	//	namespace gcp
