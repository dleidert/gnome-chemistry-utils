// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-step.cc 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "reaction-step.h"
#include "reaction.h"
#include "reactant.h"
#include "reaction-arrow.h"
#include "reaction-operator.h"
#include "document.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <libgnomecanvas/gnome-canvas.h>

using namespace gcu;
using namespace std;

namespace gcp {

TypeId ReactionStepType = NoType;

ReactionStep::ReactionStep (): Object (ReactionStepType)
{
	SetId ("rs1");
	m_bLoading = false;
}

ReactionStep::ReactionStep (Reaction *reaction, map<double, Object*>& Children, map<Object*, ArtDRect> Objects): Object (ReactionStepType)
{
	SetId ("rs1");
	reaction->AddChild (this);
	GetDocument ()->EmptyTranslationTable ();
	Document *pDoc = dynamic_cast <Document*> (GetDocument ());
	Theme *pTheme = pDoc->GetTheme ();
	WidgetData  *pData= (WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
	map<double, Object*>::iterator im, endm;
	double x, y, x0, y0, x1, y1;
	ArtDRect *rect;
	Object *cur;
	im = Children.begin ();
	new Reactant (this, (*im).second);
	endm = Children.end ();
	rect = &Objects[(*im).second];
	x = rect->x1;
	y = (*im).second->GetYAlign ();
	for (im++; im != endm; im++) {
		x += pTheme->GetSignPadding ();
		//Add a sign
		ReactionOperator* pOp = new ReactionOperator ();
		AddChild (pOp);
		pOp->SetCoords (x / pTheme->GetZoomFactor (), y);
		pDoc->AddObject (pOp);
		gnome_canvas_update_now (GNOME_CANVAS (pData->Canvas));
		gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (pData->Items[pOp]), &x0, &y0, &x1, &y1);
		pOp->Move ((x - x0) / pTheme->GetZoomFactor (), 0);
		x += pTheme->GetSignPadding () + x1 - x0;
		cur = (*im).second;
		new Reactant (this, cur);
		rect = &Objects[cur];
		y0 = cur->GetYAlign ();
		cur->Move ((x - rect->x0) / pTheme->GetZoomFactor (), y - y0);
		x+= rect->x1 - rect->x0;
	}
	Update (pData->Canvas);
	gnome_canvas_update_now (GNOME_CANVAS (pData->Canvas));
	m_bLoading = false;
}

ReactionStep::~ReactionStep ()
{
	if (IsLocked ())
		return;
	set<ReactionArrow *>::iterator i, end = m_Arrows.end();
	for (i = m_Arrows.begin (); i != end; i++)
		(*i)->RemoveStep (this);
	/* If there are no chidren, don't care and exit */
	if (!GetChildrenNumber ())
		return;
	/* Now, destroy children and add the reactant contents to the reaction parent,
	not to the reaction */
	Document *pDoc = reinterpret_cast<Document *> (GetDocument ());
	Operation *pOp = pDoc->GetCurrentOperation ();
	Reaction *pReaction = reinterpret_cast<Reaction *> (GetParent ());
	if (!pReaction)
		return;
	Object *pObj = pReaction->GetParent (), *Child, *Group = pReaction->GetGroup ();
	map<string, Object *>::iterator j;
	Reactant *pReactant;
	while (HasChildren ()) {
		Child = GetFirstChild (j);
		if (Child->GetType () == ReactionOperatorType) {
			pDoc->Remove (Child);
			continue;
		}
		pReactant = reinterpret_cast<Reactant *> (Child);
		Child = pReactant->GetStoichChild ();
		if (Child)
			pDoc->Remove (Child);
		Child = pReactant->GetChild ();
		if (Child) {
			Child->SetParent (pObj);
			if (pOp && !Group)
				pOp->AddObject (Child, 1);
		}
		delete pReactant;
	}
}
	
xmlNodePtr ReactionStep::Save (xmlDocPtr xml)
{
	xmlNodePtr node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "reaction-step", NULL);
	if (!node) return NULL;
	SaveId (node);
	map<string, Object*>::iterator i;
	Object *obj = GetFirstChild (i);
	xmlNodePtr child;
	while (obj) {
		if ((*i).second->GetType () != ReactionOperatorType) {
			if ((child = (*i).second->Save (xml)))
				xmlAddChild (node, child);
			else
				return NULL;
		}
		obj = GetNextChild (i);
	}
	return node;
}
	
bool ReactionStep::Load (xmlNodePtr node)
{
	m_bLoading = true;
	if (!Object::Load (node))
		return false;
	map<Object*, ArtDRect> Objects;
	map<double, Object*> Children;
	ArtDRect rect;
	map<string, Object*>::iterator i;
	Object *pObj = GetFirstChild (i);
	Document *pDoc = dynamic_cast <Document*> (GetDocument ());
	Theme *pTheme = pDoc->GetTheme ();
	WidgetData  *pData= (WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
	map<double, Object*>::iterator im, endm;
	double x, y, x0, y0, x1, y1;
	gnome_canvas_update_now (GNOME_CANVAS (pData->Canvas));
	while (pObj) {
		pData->GetObjectBounds (pObj, &rect);
		x = (rect.x0 + rect.x1) / 2;
		while (Children[x] != NULL)
			x += 1e-5;
		Children[x] = pObj;
		Objects[pObj] = rect;
		pObj = GetNextChild (i);
	}
	im = Children.begin ();
	endm = Children.end ();
	rect = Objects[(*im).second];
	x = rect.x1;
	y = (*im).second->GetYAlign ();
	for (im++; im != endm; im++) {
		x += pTheme->GetSignPadding ();
		//Add a sign
		ReactionOperator* pOp = new ReactionOperator();
		AddChild (pOp);
		pOp->SetCoords(x / pTheme->GetZoomFactor (), y);
		pDoc->AddObject(pOp);
		gnome_canvas_update_now (GNOME_CANVAS (pData->Canvas));
		gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (pData->Items[pOp]), &x0, &y0, &x1, &y1);
		pOp->Move ((x - x0) / pTheme->GetZoomFactor (), 0);
		x += pTheme->GetSignPadding () + x1 - x0;
		pObj = (*im).second;
		rect = Objects[pObj];
		x+= rect.x1 - rect.x0;
	}
	Update (pData->Canvas);
	m_bLoading = false;
	return true;
}

double ReactionStep::GetYAlign ()
{
	map<string, Object*>::iterator i;
	GetFirstChild (i);
	return ((*i).second)? (*i).second->GetYAlign (): 0.;
}

bool ReactionStep::OnSignal (SignalId Signal, Object *Child)
{
	if (Signal == OnChangedSignal) {
		if (m_bLoading)
			return false;
		map<Object*, ArtDRect> Objects;
		map<double, Object*> Children;
		list<Object*> Operators;
		ArtDRect rect;
		map<string, Object*>::iterator i;
		Object *pObj = GetFirstChild (i);
		Document *pDoc = dynamic_cast <Document*> (GetDocument ());
		Theme *pTheme = pDoc->GetTheme ();
		View *pView = pDoc->GetView ();
		WidgetData  *pData= (WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
		map<double, Object*>::iterator im, endm;
		double x, y, x0, y0, x1, y1;
//		gnome_canvas_update_now (GNOME_CANVAS (pData->Canvas));
		while (pObj) {
			if (pObj->GetType () == ReactionOperatorType)
				Operators.push_front (pObj);
			else {
				pData->GetObjectBounds (pObj, &rect);
				x = (rect.x0 + rect.x1) / 2;
				while (Children[x] != NULL)
					x += 1e-5;
				Children[x] = pObj;
				Objects[pObj] = rect;
			}
			pObj = GetNextChild (i);
		}
		while (!Operators.empty ()) {
			pObj = Operators.front ();
			pView->Remove (pObj);
			delete pObj;
			Operators.pop_front ();
		}
		im = Children.begin ();
		endm = Children.end ();
		rect = Objects[(*im).second];
		x = rect.x1;
		y = (*im).second->GetYAlign ();
		for (im++; im != endm; im++) {
			x += pTheme->GetSignPadding ();
			//Add a sign
			ReactionOperator* pOp = new ReactionOperator();
			AddChild (pOp);
			pOp->SetCoords(x / pTheme->GetZoomFactor (), y);
			pDoc->AddObject(pOp);
			gnome_canvas_update_now (GNOME_CANVAS (pData->Canvas));
			gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (pData->Items[pOp]), &x0, &y0, &x1, &y1);
			pOp->Move ((x - x0) / pTheme->GetZoomFactor (), 0);
			x += pTheme->GetSignPadding () + x1 - x0;
			pObj = (*im).second;
			rect = Objects[pObj];
			pObj->Move ((x - rect.x0) / pTheme->GetZoomFactor (), y - pObj->GetYAlign ());
			x+= rect.x1 - rect.x0;
		}
		Update (pData->Canvas);
		return true;
	} else
		return true;
}

void ReactionStep::RemoveArrow (ReactionArrow *arrow) {
	m_Arrows.erase (arrow);
	if (m_Arrows.empty ()) {
		// if there is no more arrows this is no more a reaction step
		delete this;
	}
}

void ReactionStep::Add (GtkWidget* w)
{
	/* Now that Object::Add adds children, we need to override this method
	because a molecule children (atoms, bonds, and fragments) are added to
	the widget by the document. FIXME! FIXME! FIXME! CHANGE THIS OLD WEIRD CODE!
	*/
}

}	//	namespace gcp
