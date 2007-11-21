// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction.cc 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "theme.h"
#include "view.h"
#include <glib/gi18n-lib.h>
#include <cmath>

using namespace gcu;
using namespace std;

namespace gcp {

Reaction::Reaction (): Object (ReactionType)
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

bool Reaction::Build (list<Object*>& Children) throw (invalid_argument)
{
	Document *pDoc = (Document*) GetDocument ();
	Theme *pTheme = pDoc->GetTheme ();
	WidgetData  *pData= (WidgetData*)g_object_get_data(G_OBJECT(pDoc->GetWidget ()), "data");
	map<Object*, ArtDRect> Objects;
	list<Object*>::iterator i, end = Children.end ();
	list<ReactionArrow*> Arrows;
	list<Object*> Others;
	map<double, Object*> Left, Right;
	double x0, y0, x1, y1, x, y, xpos, ypos, l;
	bool horiz = true;;
	ArtDRect *rect, srect;
	for (i = Children.begin (); i != end; i++) {
		// It might be better to use the objects coordinates there
		pData->GetObjectBounds (*i, &Objects[*i]);
		// Search arrows
		if ((*i)->GetType() == ReactionArrowType)
			Arrows.push_front ((ReactionArrow*) (*i));
		else if ((*i)->GetType() == MoleculeType)
			Others.push_front ((Molecule*) (*i));
		else return false;
	}
	if (Arrows.size () == 1) {
	// FIXME: only simple reactions schemes with one arrow are supported in this version
		ReactionArrow *arrow = Arrows.front ();
		AddChild (arrow);
		arrow->GetCoords (&x0, &y0, &x1, &y1);
		//x0 and y0 should be the center of the arrow, not the beginning, so we must transform them
		x0 = (x0 + x1) / 2;
		y0 = (y0 + y1) / 2;
		// x1, y1 will now be the coordinates of a normalized vector:
		x1 -= x0;
		y1 -= y0;
		x0 *= pTheme->GetZoomFactor ();
		y0 *= pTheme->GetZoomFactor ();
		l = sqrt (x1 * x1 + y1 * y1);
		x1 /= l;
		y1 /= l;
		// Now, group objects depending of their position relative to the arrow
		// FIXME: objects above or below an arrow are not supported.
		end = Others.end ();
		for (i = Others.begin (); i != end; i++) {
			rect = &Objects[*i];
			xpos = x = (rect->x0 + rect->x1) / 2;
			y = (rect->y0 + rect->y1) / 2;
			// search the direction from the center of the arrow to the center of the object
			x -= x0;
			y -= y0;
			l = sqrt (x * x + y * y);
			x /= l;
			y /= l;
			// scalar product:
			l = x * x1 + y * y1;
			if (l > 0.71) {
				while (Right[xpos] != NULL)
					xpos += 1e-5;
				Right[xpos] = *i;
			}
			else if (l < -0.71) {
				while (Left[xpos] != NULL)
					xpos += 1e-5;
				Left[xpos] = *i;
			}
			else //Object too far from the arrow direction
				throw  invalid_argument(_("Error could not build a reaction\nfrom the selected objects."));
		}
		// We have one or two sets of objects. We must transform them in reaction steps
		ReactionStep *step;
		l= 0.;
		if (Left.size ()) {
			step = new ReactionStep (this, Left, Objects);
			// Link fisrt step to the arrow
			arrow->SetStartStep (step);
			// Move the arrow to its new position
			pData->GetObjectBounds (step, &srect);
			x0 = (srect.x0 + srect.x1) / 2;
			y0 = step->GetYAlign () * pTheme->GetZoomFactor ();
			x = srect.x1 - x0;
			y = srect.y1 - y0;
			if ((fabs (x1) > 1e-5) && (fabs (y1) > 1e-5))
				horiz = (fabs (x1) > fabs (y1));
			else if (fabs (x1) > 1e-5)
				horiz = true;
			else
				horiz = false;
			if (horiz) {
				l = x + pTheme->GetArrowPadding ();
				if (x1 < 0)
					l = -l;
				x0 += l;
				y0 += l * y1 / x1;
			} else {
				l = y + pTheme->GetArrowPadding ();
				if (y1 < 0)
					l = -l;
				x0 += l * x1 / y1;
				y0 += l;
			}
			arrow->GetCoords (&srect.x0, &srect.y0, &srect.x1, &srect.y1);
			arrow->Move (x0 / pTheme->GetZoomFactor () - srect.x0, y0 / pTheme->GetZoomFactor () - srect.y0);
		}
		arrow->GetCoords (&srect.x0, &srect.y0, &srect.x1, &srect.y1);
		xpos = srect.x1;
		ypos = srect.y1;
		// Create second step
		if (Right.size ()) {
			step = new ReactionStep (this, Right, Objects);
			arrow->SetEndStep (step);
			pData->GetObjectBounds (step, &srect);
			x0 = (srect.x0 + srect.x1) / 2;
			y0 = step->GetYAlign () * pTheme->GetZoomFactor ();
			if (l == 0.) {
				if ((fabs (x1) > 1e-5) && (fabs (y1) > 1e-5))
					horiz = (fabs (x1) > fabs (y1));
				else if (fabs (x1) > 1e-5)
					horiz = true;
				else
					horiz = false;
			}
			if (horiz) {
				x = srect.x1 - x0;
				l = x + pTheme->GetArrowPadding ();
				if (x1 < 0)
					l = -l;
				x0 -= l;
				y0 -= l * y1 / x1;
			} else {
				y = srect.y1 - y0;
				l = y + pTheme->GetArrowPadding ();
				if (y1 < 0)
					l = -l;
				x0 -= l * x1 / y1;
				y0 -= l;
			}
			step->Move (xpos - x0 / pTheme->GetZoomFactor (), ypos - y0 / pTheme->GetZoomFactor ());
		}
	} else
		throw  invalid_argument (_("Error could not build a reaction\nfrom the selected objects."));
	return true;
}

void Reaction::Transform2D (Matrix2D& m, double x, double y)
{
}

bool Reaction::OnSignal (SignalId Signal, Object *Obj)
{
	if (IsLocked ())
		return false;
	if (Signal == OnChangedSignal) {
		Document *pDoc = (Document*) GetDocument ();
		View *pView = pDoc->GetView ();
		Theme *pTheme = pDoc->GetTheme ();
		WidgetData  *pData= (WidgetData*)g_object_get_data(G_OBJECT(pDoc->GetWidget ()), "data");
		map<Object*, ArtDRect> Objects;
		map<double, Object*> Children;
		list<Object*> Operators;
		ArtDRect rect;
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
				step = arrow->GetStartStep ();
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
				step = arrow->GetEndStep ();
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

bool Reaction::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	GtkActionGroup *group = gtk_action_group_new ("reaction");
	GtkAction *action = gtk_action_new ("destroy", _("Destroy the reaction"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
	char buf[] = "<ui><popup><menuitem action='destroy'/></popup></ui>";
	gtk_ui_manager_add_ui_from_string (UIManager, buf, -1, NULL);
	GtkWidget *w = gtk_ui_manager_get_widget (UIManager, "/popup/destroy");
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

}	//	namespace gcp
