// -*- C++ -*-

/*
 * GChemPaint library
 * reaction.cc
 *
 * Copyright (C) 2002-2012 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "reaction.h"
#include "reaction-arrow.h"
#include "reaction-step.h"
#include "document.h"
#include "mechanism-step.h"
#include "molecule.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <gcugtk/ui-manager.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

Reaction::Reaction (): Scheme (ReactionType)
{
	SetId ("rxn1");
}

Reaction::~Reaction ()
{
	if (IsLocked ())
		return;
	map<string, Object*>::iterator i;
	Object *pObj;
	ReactionArrow *arrow;
	Document *pDoc = reinterpret_cast<Document *> (GetDocument ());
	Operation *pOp = pDoc->GetCurrentOperation ();
	while ((pObj = GetFirstChild (i))) {
		if (pObj->GetType () == ReactionArrowType) {
			arrow = reinterpret_cast<ReactionArrow*> (pObj);
			arrow->SetStartStep (NULL);
			arrow->SetEndStep (NULL);
			arrow->SetParent (GetParent ());
			if (pOp)
				pOp->AddObject (arrow, 1);

		} else
			delete pObj;
	}
}

bool Reaction::Build (std::set < Object * > const &Children) throw (invalid_argument)
{
	Document *pDoc = (Document*) GetDocument ();
	Theme *pTheme = pDoc->GetTheme ();
	WidgetData  *pData= reinterpret_cast<WidgetData *> (g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data"));
	map<Object*, gccv::Rect> Objects;
	list < Arrow * >::iterator a, a1, aend;
	set < Object * >::iterator i, iend = Children.end ();
	list < Arrow * > Arrows;
	set < Object * > Others;
	map < double, Object * > Cur, Left, Right;
	double x0, y0, x1, y1, x, xpos;
	double zf = pTheme->GetZoomFactor ();
	gccv::Rect *rect;
	for (i = Children.begin (); i != iend; i++) {
		// It might be better to use the objects coordinates there
		pData->GetObjectBounds (*i, &Objects[*i]);
		// Search arrows
		if ((*i)->GetType() == ReactionArrowType)
			Arrows.push_front ((ReactionArrow*) (*i));
		else if ((*i)->GetType () == MoleculeType || (*i)->GetType () == MechanismStepType
		         || (*i)->GetType () == TextType)
			Others.insert (*i);
		else return false;
	}
	/* sort objects according their position relative to each arrow, using string, with
	 * one char per arrow, 't' means on tail side, 'h', header side, and 'o' other.
	 * If we support arrows with several tails or heads, just add a figure
	 * (there should not be more than 10 heads or tails! */ 
	map < string, set < Object * > > steps;
	iend = Others.end ();
	aend = Arrows.end ();
	for (i = Others.begin (); i != iend; i++) {
		string s;
		gccv::Rect rect = Objects[*i];
		double x = (rect.x0 + rect.x1) / 2. / zf, y = (rect.y0 + rect.y1) / 2. / zf;
		for (a = Arrows.begin (); a != aend; a++)
			s += static_cast < ReactionArrow * > (*a)->GetSymbolicPosition (x, y);
		steps[s].insert (*i);
	}
	map < string, ReactionStep * > rsteps;
	map < string, set < Object * > >::iterator istep, step_end = steps.end ();
	set < ReactionStep * > unused_steps;
	ReactionStep *step;
	for (istep = steps.begin (); istep != step_end; istep++) {
		set < Object * > &cur_set = (*istep).second;
		set < Object * >::iterator i, end = cur_set.end ();
		for (i = cur_set.begin (); i != end; i++) {
			rect = &Objects[*i];
			xpos = x = (rect->x0 + rect->x1) / 2;
			while (Cur[xpos] != NULL)
				xpos += 1e-5;
			Cur[xpos] = *i;
		}
		step = new ReactionStep (this, Cur, Objects);
		rsteps[(*istep).first] = step;
		unused_steps.insert (step);
		Cur.clear ();
	}
	// now we must find the relations between the various steps and the arrows
	// for each arrow, determine the position of head and tail relative to each other arrow
	for (a = Arrows.begin (); a != aend; a++) {
		AddChild (*a);
		(*a)->GetCoords (&x0, &y0, &x1, &y1);
		// add some padding to the arrow bounds
		double dx, dy, dl;
		dx = x1 - x0;
		dy = y1 - y0;
		dl = sqrt (dx * dx + dy * dy) / pTheme->GetArrowPadding () * pTheme->GetZoomFactor ();
		dx /= dl;
		dy /= dl;
		x0 -= dx;
		y0 -= dy;
		x1 += dx;
		y1 += dy;
		std::string st, sh;
		// search step at tail and head
		for (a1 = Arrows.begin (); a1 != aend; a1++) {
			if (a == a1) {
				st += 't';
				sh += 'h';
			} else {
				st += static_cast < ReactionArrow * > (*a1)->GetSymbolicPosition (x0, y0);
				sh += static_cast < ReactionArrow * > (*a1)->GetSymbolicPosition (x1, y1);
			}
		}
		ReactionStep *start = rsteps[st], *end = rsteps[sh];
		if (start == NULL) {
			if (Arrows.size () > 1)
				throw  invalid_argument (_("Error: arrow without a reactant or product."));
			// FIXME: add a dummy step
		} else {
			unused_steps.erase (start);
			start->AddArrow (*a, end);
			(*a)->SetStartStep (start);
		}
		if (end == NULL) {
			if (Arrows.size () > 1)
				throw  invalid_argument (_("Error: arrow without a reactant or product."));
			// FIXME: add a dummy step
		} else {
			unused_steps.erase (end);
			end->AddArrow (*a, start);
			(*a)->SetEndStep (end);
		}
	}

	// check if all steps are valid
	map < string, ReactionStep * >::iterator rsi, rsiend = rsteps.end ();
	for (rsi = rsteps.begin (); rsi != rsiend; rsi++) {
		if ((*rsi).second && !(*rsi).second->Validate ()) {
			// FIXME: select molecules from this step (should be done in ReactionStep::Validate())
			throw  invalid_argument (_("Error: isolated molecules."));
		}
	}

	// FIXME: remove dummy steps
	// if two arrows have no reactant between them, insert an empty step if the arrows are
	// consecutive or return an error otherwise, that is -> -> is ok -> <- or <- -> are forbidden.
	// FIXME: if we have an isolated arrow, return false

	// now, position the various steps and arrows
	Align ();
	return true;
}

void Reaction::Transform2D (G_GNUC_UNUSED Matrix2D& m, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y)
{
}

bool Reaction::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Obj)
{
	if (IsLocked ())
		return false;
	if (Signal == OnChangedSignal) {
		Document *pDoc = (Document*) GetDocument ();
		View *pView = pDoc->GetView ();
		Theme *pTheme = pDoc->GetTheme ();
		WidgetData  *pData= (WidgetData*)g_object_get_data(G_OBJECT(pDoc->GetWidget ()), "data");
		map<Object*, gccv::Rect> Objects;
		map<double, Object*> Children;
		list<Object*> Operators;
		gccv::Rect rect;
		map<string, Object*>::iterator i;
		Object *pObj = GetFirstChild (i);
		ReactionArrow *arrow;
		ReactionStep *step;
		list<ReactionArrow *> IsolatedArrows;
		double x0, y0, x1, y1, x, y, x2, y2, xpos, ypos, l;
		bool horiz, has_start;
		// It is assume we'll find only one arrow!
		while (pObj) {
			if (pObj->GetType () == ReactionArrowType) {
				arrow = reinterpret_cast<ReactionArrow*> (pObj);
				has_start = false;
				arrow->GetCoords (&x0, &y0, &x1, &y1);
				x = x1 - x0;
				y = y1 - y0;
				l = sqrt (x * x + y * y);
				x /= l;
				y /= l;
				if ((fabs (x) > 1e-5) && (fabs (y) > 1e-5))
					horiz = (fabs (x) > fabs (y));
				else if (fabs (x) > 1e-5)
					horiz = true;
				else
					horiz = false;
				step = static_cast < ReactionStep * > (arrow->GetStartStep ());
				if (step) {
					has_start = true;
					pData->GetObjectBounds (step, &rect);
					x2 = (rect.x0 + rect.x1) / 2;
					y2 = step->GetYAlign () * pTheme->GetZoomFactor ();
					xpos = rect.x1 - x2;
					ypos = rect.y1 - y2;
					if (horiz) {
						l = xpos + pTheme->GetArrowPadding ();
						if (x < 0)
							l = -l;
						x2 += l;
						y2 += l * y / x;
					} else {
						l = ypos + pTheme->GetArrowPadding ();
						if (y < 0)
							l = -l;
						x2 += l * x / y;
						y2 += l;
					}
					x1 += xpos = x2 / pTheme->GetZoomFactor () - x0;
					y1 += ypos = y2 / pTheme->GetZoomFactor () - y0;
					arrow->Move (xpos, ypos);
					pView->Update (arrow);
				}
				step = static_cast < ReactionStep * > (arrow->GetEndStep ());
				if (step) {
					pData->GetObjectBounds (step, &rect);
					x2 = (rect.x0 + rect.x1) / 2;
					y2 = step->GetYAlign () * pTheme->GetZoomFactor ();
					if (horiz) {
						xpos = rect.x1 - x2;
						l = xpos + pTheme->GetArrowPadding ();
						if (x < 0)
							l = -l;
						x2 -= l;
						y2 -= l * y / x;
					} else {
						ypos = rect.y1 - y2;
						l = ypos + pTheme->GetArrowPadding ();
						if (y < 0)
							l = -l;
						x2 -= l * x / y;
						y2 -= l;
					}
					step->Move (x1 - x2 / pTheme->GetZoomFactor (), y1 - y2 / pTheme->GetZoomFactor ());
					pView->Update (step);
				} else if (!has_start)
					IsolatedArrows.push_front (arrow);
			}
			pObj = GetNextChild (i);
		}
		while (!IsolatedArrows.empty ()) {
			IsolatedArrows.front ()->SetParent (GetParent ());
			IsolatedArrows.pop_front ();
		}
		if (!HasChildren ())
			delete this;
	}
	return true;
}

bool Reaction::Load (xmlNodePtr node)
{
	xmlChar* tmp;
	xmlNodePtr child;
	Object* pObject;
	list<xmlNodePtr> arrows;

	Lock ();
	tmp = xmlGetProp (node, (xmlChar*) "id");
	if (tmp) {
		SetId ((char*) tmp);
		xmlFree (tmp);
	}
	child = node->children;
	while (child) {
		if (!strcmp ((const char*) child->name, "reaction-arrow"))
			arrows.push_front (child);
		else {
			pObject = CreateObject ((const char*) child->name, this);
			if (pObject) {
				if (!pObject->Load (child))
					delete pObject;
			} else {
				Lock (false);
				return false;
			}
		}
		child = child->next;
	}
	while (!arrows.empty ()) {
		child = arrows.back ();
		pObject = CreateObject ("reaction-arrow", this);
		if (pObject) {
			if (!pObject->Load (child))
				delete pObject;
		} else {
			Lock (false);
			return false;
		}
		arrows.pop_back ();
	}
	Lock (false);
	return true;
}

static void do_destroy_reaction (void *data)
{
	Reaction *reaction = reinterpret_cast<Reaction *> (data);
	Document *pDoc = reinterpret_cast<Document *> (reaction->GetDocument ());
	WidgetData *pData = reinterpret_cast<WidgetData *> (g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data"));
	pData->Unselect (reaction);
	Operation *pOp = pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
	pOp->AddObject (reaction, 0);
	delete reaction;
	pDoc->FinishOperation ();
}

bool Reaction::BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y)
{
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	GtkActionGroup *group = gtk_action_group_new ("reaction");
	GtkAction *action = gtk_action_new ("destroy", _("Destroy the reaction"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_insert_action_group (uim, group, 0);
	g_object_unref (group);
	char buf[] = "<ui><popup><menuitem action='destroy'/></popup></ui>";
	gtk_ui_manager_add_ui_from_string (uim, buf, -1, NULL);
	GtkWidget *w = gtk_ui_manager_get_widget (uim, "/popup/destroy");
	g_signal_connect_swapped (w, "activate", G_CALLBACK (do_destroy_reaction), this);
	Object::BuildContextualMenu (UIManager, object, x, y);
	return true;
}

double Reaction::GetYAlign ()
{
	map<string, Object*>::iterator i;
	Object *pObj;
	pObj = GetFirstChild (i);
	double y = DBL_MAX, new_y;
	while (pObj) {
		if (pObj->GetType () == ReactionStepType)
			if ((new_y = pObj->GetYAlign ()) < y)
				y = new_y;
		pObj = GetNextChild (i);
	}
	return y;
}

std::string Reaction::Name ()
{
	return _("Reaction");
}

}	//	namespace gcp
