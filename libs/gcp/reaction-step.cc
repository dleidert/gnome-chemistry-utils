// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-step.cc
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
#include "reaction-step.h"
#include "reaction.h"
#include "reactant.h"
#include "reaction-arrow.h"
#include "reaction-operator.h"
#include "document.h"
#include "mechanism-arrow.h"
#include "molecule.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <gccv/item.h>
#include <glib/gi18n.h>

using namespace gcu;
using namespace std;

namespace gcp {

TypeId ReactionStepType = NoType;
TypeId DummyStepType = NoType;

ReactionStep::ReactionStep (): MechanismStep (ReactionStepType)
{
	SetId ("rs1");
	m_bLoading = false;
}

ReactionStep::ReactionStep (Reaction *reaction, map<double, Object*>& Children, map<Object*, gccv::Rect> Objects) throw (std::invalid_argument): MechanismStep (ReactionStepType)
{
	SetId ("rs1");
	reaction->AddChild (this);
	GetDocument ()->EmptyTranslationTable ();
	Document *pDoc = dynamic_cast <Document*> (GetDocument ());
	View *view = pDoc->GetView ();
	Theme *pTheme = pDoc->GetTheme ();
	map<double, Object*>::iterator im, endm;
	double x, y, x0, y0, x1, y1;
	gccv::Rect *rect;
	Object *cur;
	im = Children.begin ();
	try {
		if ((*im).second->GetType () == MechanismStepType) {
			if (Children.size () > 1)
				throw invalid_argument (_("A mechanism step must stay alone inside a reaction step"));
			AddChild ((*im).second);
		} else
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
			dynamic_cast <gccv::ItemClient *> (pOp)->GetItem ()->GetBounds (x0, y0, x1, y1);
			pOp->Move ((x - x0) / pTheme->GetZoomFactor (), 0);
			x += pTheme->GetSignPadding () + x1 - x0;
			cur = (*im).second;
			if ((*im).second->GetType () == MechanismStepType)
				throw invalid_argument (_("A mechanism step must stay alone inside a reaction step"));
			else
				new Reactant (this, cur);
			rect = &Objects[cur];
			y0 = cur->GetYAlign ();
			cur->Move ((x - rect->x0) / pTheme->GetZoomFactor (), y - y0);
			x+= rect->x1 - rect->x0;
		}
		view->Update (this);
		m_bLoading = false;
	}
	catch (invalid_argument& e) {
		m_bLoading = false;
		CleanChildren ();
		throw e;
	}
}

ReactionStep::~ReactionStep ()
{
	if (IsLocked ())
		return;
	Lock ();
	CleanChildren ();
}

xmlNodePtr ReactionStep::Save (xmlDocPtr xml) const
{
	xmlNodePtr node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "reaction-step", NULL);
	if (!node) return NULL;
	SaveId (node);
	map<string, Object*>::const_iterator i;
	Object const *obj = GetFirstChild (i);
	xmlNodePtr child;
	while (obj) {
		if (obj->GetType () != ReactionOperatorType) {
			if ((child = obj->Save (xml)))
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
	if (!Object::Load (node))
		return false;
	Document *pDoc = dynamic_cast <Document*> (GetDocument ());
	pDoc->NotifyDirty (this);
	pDoc->ObjectLoaded (this);
	return true;
}

void ReactionStep::OnLoaded ()
{
	m_bLoading = true;
	map<Object*, gccv::Rect> Objects;
	map<double, Object*> Children;
	gccv::Rect rect;
	map<string, Object*>::iterator i;
	Object *pObj = GetFirstChild (i);
	Document *pDoc = dynamic_cast <Document*> (GetDocument ());
	View *view = pDoc->GetView ();
	Theme *pTheme = pDoc->GetTheme ();
	view->Update (this);
	WidgetData  *pData= view->GetData ();
	map<double, Object*>::iterator im, endm;
	double x, y, x0, y0, x1, y1;
	// this method is called more than once (might consider this a bug) so we need to remove old operators
	std::list <gcu::Object *> Operators;
	// FIXME: add a dummy object if empty
	while (pObj) {
		if (pObj->GetType () == ReactionOperatorType)
			Operators.push_front (pObj);
		else if (pObj->GetType () != gcp::MechanismArrowType) {
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
		delete Operators.front ();
		Operators.pop_front ();
	}
	im = Children.begin ();
	endm = Children.end ();
	rect = Objects[(*im).second];
	x = rect.x1;
	y = (*im).second->GetYAlign ();
	for (im++; im != endm; im++) {
		pObj = (*im).second;
		rect = Objects[pObj];
		x = (x + rect.x0) / 2.;
		//Add a sign
		ReactionOperator* pOp = new ReactionOperator();
		AddChild (pOp);
		pOp->SetCoords(x / pTheme->GetZoomFactor (), y);
		pDoc->AddObject(pOp);
		x= rect.x1;
	}
	m_bLoading = false;
}

double ReactionStep::GetYAlign () const
{
	std::map < std::string, gcu::Object * >::const_iterator i;
	Object const *obj = GetFirstChild (i);
	while (obj && obj->GetType () != gcu::ReactantType && obj->GetType () != MechanismStepType)
		obj = GetNextChild (i);
	return (obj)? obj->GetYAlign (): 0.;
}

bool ReactionStep::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (Signal == OnChangedSignal) {
		if (m_bLoading)
			return false;
		map<Object*, gccv::Rect> Objects;
		map<double, Object*> Children;
		list<Object*> Operators;
		gccv::Rect rect;
		map<string, Object*>::iterator i;
		Object *pObj = GetFirstChild (i);
		Document *pDoc = dynamic_cast <Document*> (GetDocument ());
		Theme *pTheme = pDoc->GetTheme ();
		View *pView = pDoc->GetView ();
		WidgetData  *pData= (WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
		map<double, Object*>::iterator im, endm;
		double x, y, x0, y0, x1, y1;
		std::set <gcu::Object *> Invalid;
		if (!pObj) {
			delete this;
			return true;
		}
		while (pObj) {
			if (pObj->GetType () == ReactionOperatorType)
				Operators.push_front (pObj);
			else if (pObj->GetType () == MechanismArrowType); // don't do anything
			else if (pObj->GetType () == MesomeryArrowType) {
				delete this;
				return true;
			} else {
				if (pObj->GetChildrenNumber ()) {
					pData->GetObjectBounds (pObj, &rect);
					x = (rect.x0 + rect.x1) / 2;
					while (Children[x] != NULL)
						x += 1e-5;
					Children[x] = pObj;
					Objects[pObj] = rect;
				} else
					Invalid.insert (pObj);
			}
			pObj = GetNextChild (i);
		}
		// delete invalid children
		// first avoid new signals
		m_bLoading = true;
		// really delete
		std::set <gcu::Object *>::iterator j, jend = Invalid.end ();
		for (j = Invalid.begin (); j != jend; j++)
			delete *j;
		m_bLoading = false;
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
			dynamic_cast <gccv::ItemClient *> (pOp)->GetItem ()->GetBounds (x0, y0, x1, y1);
			pOp->Move ((x - x0) / pTheme->GetZoomFactor (), 0);
			x += pTheme->GetSignPadding () + x1 - x0;
			pObj = (*im).second;
			rect = Objects[pObj];
			pObj->Move ((x - rect.x0) / pTheme->GetZoomFactor (), y - pObj->GetYAlign ());
			x+= rect.x1 - rect.x0;
		}
		pDoc->GetView ()->Update (this);
		return true;
	} else
		return true;
}

std::string ReactionStep::Name ()
{
	return _("Reaction step");
}

void ReactionStep::CleanChildren ()
{
	map < Step *, Arrow * >::iterator i, end = m_Arrows.end();
	for (i = m_Arrows.begin (); i != end; i++)
		(*i).second->RemoveStep (this);
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
	list <MechanismArrow *> arrows;
	set <Object *> new_objects;
	while (HasChildren ()) {
		Child = GetFirstChild (j);
		if (Child->GetType () == ReactionOperatorType) {
			pDoc->Remove (Child);
			continue;
		} else if (Child->GetType () == MechanismArrowType) {
			Child->SetParent (pObj);
			arrows.push_back (static_cast <MechanismArrow *> (Child));
			continue;
		} else if (Child->GetType () == MechanismStepType || Child->GetType () == MesomeryArrowType) {
			Child->SetParent (pObj);
			if (pOp && !Group)
				new_objects.insert (Child);
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
				new_objects.insert (Child);
		}
		delete pReactant;
	}
	while (!arrows.empty ()) {
		MechanismArrow *arrow = arrows.front ();
		MechanismStep *step;
		Object *obj = arrow->GetSource (), *molecule = obj->GetMolecule (), *parent = molecule->GetParent ();
		if (parent->GetType () == MechanismStepType) {
			step = static_cast <MechanismStep *> (parent);
			step->AddChild (arrow);
		} else {
			step = dynamic_cast <MechanismStep *> (arrow->GetTarget ()->GetMolecule ()->GetParent ());
			if (!step)
				step = new MechanismStep ();
			step->SetParent (parent);
			step->AddChild (arrow);
			step->AddChild (molecule);
			if (pOp) {
				new_objects.erase (molecule);
				new_objects.insert (step);
			}
		}
		obj = arrow->GetTarget ();
		molecule = obj->GetMolecule ();
		parent = molecule->GetParent ();
		if (parent != step) {
			if (parent->GetType () == MechanismStepType) {
				map <string, Object *>::iterator it;
				obj = parent->GetFirstChild (it);
				while (obj) {
					if (pOp)
						new_objects.erase (obj);
					step->AddChild (obj);
					obj = parent->GetFirstChild (it);
				}

			} else
				step->AddChild (molecule);
		}
		arrows.pop_front ();
	}
	set <Object *>::iterator k, kend = new_objects.end ();
	for (k = new_objects.begin (); k != kend; k++)
		pOp->AddObject (*k, 1);
}

void ReactionStep::AddMolecule (Molecule *molecule, bool signal)
{
	new Reactant (this, molecule);
	if (signal)
		EmitSignal (OnChangedSignal);
}

bool ReactionStep::Validate ()
{
	if (m_Arrows.empty ())
		return false;
	// FIXME:
	return true;
}

}	//	namespace gcp
