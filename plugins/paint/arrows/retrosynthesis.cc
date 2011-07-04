// -*- C++ -*-

/*
 * GChemPaint arrows plugin
 * retrosynthesis.cc
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "retrosynthesis.h"
#include "retrosynthesisarrow.h"
#include "retrosynthesisstep.h"
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/widgetdata.h>
#include <gcp/view.h>
#include <gcugtk/ui-manager.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <cstring>

TypeId RetrosynthesisType = NoType;

gcpRetrosynthesis::gcpRetrosynthesis (): Object (RetrosynthesisType)
{
	SetId ("rsy1");
	Target = NULL;
}

gcpRetrosynthesis::~gcpRetrosynthesis ()
{
	if (IsLocked ())
		return;
	map<string, Object*>::iterator i;
	Object *pObj;
	gcpRetrosynthesisArrow *arrow;
	gcp::Document *pDoc = reinterpret_cast<gcp::Document *> (GetDocument ());
	gcp::Operation *pOp = pDoc->GetCurrentOperation ();
	while ((pObj = GetFirstChild (i))) {
		if ((pObj->GetType () == RetrosynthesisArrowType)) {
			arrow = reinterpret_cast<gcpRetrosynthesisArrow *> (pObj);
			arrow->SetStartStep (NULL);
			arrow->SetEndStep (NULL);
			arrow->SetParent (GetParent ());
			if (pOp)
				pOp->AddObject (arrow, 1);

		} else
			delete pObj;
	}
}

xmlNodePtr gcpRetrosynthesis::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = Object::Save (xml);
	xmlNewProp (node, (xmlChar const*) "target", (xmlChar const*) Target->GetId ());
	return node;
}

bool gcpRetrosynthesis::Load (xmlNodePtr node)
{
	xmlChar* buf;
	xmlNodePtr child;
	Object* pObject;
	list<xmlNodePtr> arrows;

	Lock ();
	buf = xmlGetProp (node, (xmlChar*) "id");
	if (buf) {
		SetId ((char*) buf);
		xmlFree (buf);
	}
	child = node->children;
	while (child) {
		if (!strcmp ((const char*) child->name, "retrosynthesis-arrow"))
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
		pObject = CreateObject ("retrosynthesis-arrow", this);
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
	buf = xmlGetProp (node, (xmlChar*) "target");
	if (!buf)
		return false;
	Target = reinterpret_cast<gcpRetrosynthesisStep*> (GetChild ((const char*) buf));
	xmlFree (buf);
	if (!Target)
		return false;
	GetDocument ()->ObjectLoaded (this);
	return true;
}

typedef struct
{
	double x, y;
	gccv::Rect r;
	gcpRetrosynthesisStep *step;
} ObjectData;

bool gcpRetrosynthesis::Build (std::set < Object * > const &Children) throw (invalid_argument)
{
	gcp::Document *pDoc = reinterpret_cast<gcp::Document *> (GetDocument ());
	gcp::Theme *pTheme = pDoc->GetTheme ();
	gcp::WidgetData  *pData= reinterpret_cast<gcp::WidgetData *> (g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data"));
	set < Object * >::iterator i, iend = Children.end ();
	map < Object *, ObjectData > Objects;
	list < Object  * > Arrows;
	double minright, minleft, x, y, x0, y0, x1, y1, l, d, ps;
	Object *Left, *Right;
	ObjectData od;
	gcpRetrosynthesisArrow *arrow;
	unsigned narrows = 0, nmol = 0;
	TypeId Id;
	for (i = Children.begin (); i != iend; i++) {
		Id = (*i)->GetType ();
		if (Id == MoleculeType) {
			pData->GetObjectBounds (*i, &od.r);
			od.x = (od.r.x0 + od.r.x1) / 2.;
			od.y = (*i)->GetYAlign () * pTheme->GetZoomFactor ();
			od.step = new gcpRetrosynthesisStep (this, reinterpret_cast<gcp::Molecule *>(*i));
			nmol++;
			Objects[*i] = od;
		} else if (Id == RetrosynthesisArrowType) {
			narrows++;
			Arrows.push_back (*i);
			AddChild (*i);
		} else
			throw  invalid_argument (_("Something wrong happened, please file a bug report."));
	}
	// now, for each arrow, search closiest object on both sides and verify it's a molecule
	list<Object *>::iterator j, jend = Arrows.end ();
	map<Object *, ObjectData>::iterator k, kend = Objects.end ();
	for (j = Arrows.begin (); j != jend; j++) {
		arrow = reinterpret_cast<gcpRetrosynthesisArrow *>(*j);
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
		l *= pTheme->GetZoomFactor (); // half length of the arrow on the screen
		// No molecule should be nearer than that
		minright = minleft = DBL_MAX;
		Left = Right = NULL;
		for (k = Objects.begin (); k != kend; k++) {
			od = (*k).second;
			x = od.x - x0;
			y = od.y - y0;
			d = sqrt (x * x + y * y);
			ps = (x * x1 + y * y1) / d;
			if (ps >= -.71 && ps <= .71)
				continue;
			if (d < l) {
				Left = (*k).first;
				Right = *j;
				pData->UnselectAll ();
				pData->SetSelected (Left);
				pData->SetSelected (Right);
				throw invalid_argument (_("No space left between molecule and arrow!"));
			}
			if (ps < 0) {
				if (d < minleft) {
					Left = od.step;
					minleft = d;
				}
			} else {
				if (d < minright) {
					Right = od.step;
					minright = d;
				}
			}
		}
		if (!Left || !Right) { // Do not accept arrows with only one step (?)
			Left = *j;
			pData->UnselectAll ();
			pData->SetSelected (Left);
			throw invalid_argument (_("Isolated arrows are not allowed!"));
		}
		reinterpret_cast<gcpRetrosynthesisArrow *> (*j)->SetStartStep (reinterpret_cast<gcpRetrosynthesisStep *> (Left));
		reinterpret_cast<gcpRetrosynthesisArrow *> (*j)->SetEndStep (reinterpret_cast<gcpRetrosynthesisStep *> (Right));
		reinterpret_cast<gcpRetrosynthesisStep *> (Left)->AddArrow (reinterpret_cast<gcpRetrosynthesisArrow *> (*j), reinterpret_cast<gcpRetrosynthesisStep *> (Right), true);
		reinterpret_cast<gcpRetrosynthesisStep *> (Right)->AddArrow (reinterpret_cast<gcpRetrosynthesisArrow *> (*j), reinterpret_cast<gcpRetrosynthesisStep *> (Left), false);
	}
	// now, check if each step has at least one arrow, may be we should add missing arrows?
	// also check that there are no cyclic relationships and only one start point
	for (k = Objects.begin (); k != kend; k++) {
		od = (*k).second;
		if (!od.step->Validate ()) {
			Left = (*k).first;
			pData->UnselectAll ();
			pData->SetSelected (Left);
			throw invalid_argument (_("Isolated molecule!\n Please add missing arrows."));
		}
	}
	switch (Validate (false)) {
	case 0:
		break;
	case 1:
		throw  invalid_argument (_("No target molecule!"));
	case 2:
		throw invalid_argument (_("Multiple target molecules or missing arrows."));
	case 3:
		throw invalid_argument (_("Sorry, cyclic retrosynthesis paths are not supported."));
	}
	Align ();
	return true;
}

static int BuildConnectivity ( set<Object *> &Objects, gcpRetrosynthesisStep* Step)
{
	map<gcpRetrosynthesisStep *, gcpRetrosynthesisArrow *> *Arrows = Step->GetArrows ();
	map<gcpRetrosynthesisStep *, gcpRetrosynthesisArrow *>::iterator i, end = Arrows->end ();
	for (i = Arrows->begin (); i != end; i++) {
		Objects.insert ((*i).second);
		if (Objects.find ((*i).first) == Objects.end ()) {
			Objects.insert ((*i).first);
			if (BuildConnectivity (Objects, (*i).first))
				return 1;
		} else
			return 1;
	}
	return 0;
}

int gcpRetrosynthesis::Validate (bool split)
{
	map<string, Object*>::iterator i;
	Object *pObj = GetFirstChild (i);
	while (pObj && (pObj->GetType () != RetrosynthesisStepType ||
		(reinterpret_cast<gcpRetrosynthesisStep *> (pObj))->GetArrow ()))
		pObj = GetNextChild (i);
	if (pObj == NULL)
		return 1;
	Target = reinterpret_cast<gcpRetrosynthesisStep *> (pObj);
	set<Object *> Objects;
	Objects.insert (pObj);
	if (BuildConnectivity (Objects, Target))
		return 3;
	while (Objects.size () < GetChildrenNumber ()) {
		if (!split)
			return 2;
		pObj = GetFirstChild (i);
		while (pObj && (pObj->GetType () != RetrosynthesisStepType ||
			(reinterpret_cast<gcpRetrosynthesisStep *> (pObj))->GetArrow () ||
			pObj == Target))
			pObj = GetNextChild (i);
		if (reinterpret_cast<gcpRetrosynthesisStep *> (pObj)->Validate ()) {
			gcpRetrosynthesis *rs = new gcpRetrosynthesis (GetParent (),
								reinterpret_cast<gcpRetrosynthesisStep *> (pObj));
			gcp::Document *pDoc = reinterpret_cast<gcp::Document*> (GetDocument ());
			gcp::Operation *pOp = pDoc->GetCurrentOperation ();
			pOp->AddObject (rs, 1);
		} else
			delete pObj;
	}
	return 0;
}

typedef struct
{
	double x, y;
	gccv::Rect r;
} StepData;

static void AlignStep (map<Object*, StepData> &data, gcpRetrosynthesisStep *step, gcp::View *pView, gcp::WidgetData  *pData)
{
	double x0, y0, x1, y1, x, y, l, dx, dy;
	bool horiz;
	StepData sd = data[step], sd1;
	gcp::Theme *pTheme = pView->GetDoc ()->GetTheme ();
	map<gcpRetrosynthesisStep *, gcpRetrosynthesisArrow *> *Arrows = step->GetArrows ();
	map<gcpRetrosynthesisStep *, gcpRetrosynthesisArrow *>::iterator i, end = Arrows->end ();
	for (i = Arrows->begin (); i != end; i++) {
		(*i).second->GetCoords (&x0, &y0, &x1, &y1);
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
		if (horiz) {
			if (x > 0)
				dx = sd.r.x1 - sd.x + pTheme->GetArrowPadding ();
			else
				dx = sd.r.x0 - sd.x - pTheme->GetArrowPadding ();
			dy = dx * y / x;
		} else {
			if (y > 0)
				dy = sd.r.y1 - sd.y + pTheme->GetArrowPadding ();
			else
				dy = sd.r.y0 - sd.y - pTheme->GetArrowPadding ();
			dx = dy * x / y;
		}
		x1 += x0 = (dx + sd.x) / pTheme->GetZoomFactor () - x0;
		y1 += y0 = (dy + sd.y) / pTheme->GetZoomFactor () - y0;
		(*i).second->Move (x0, y0);
		pView->Update ((*i).second);
		sd1 = data[(*i).first];
		if (horiz) {
			if (x > 0)
				dx = sd1.x - sd1.r.x0 + pTheme->GetArrowPadding ();
			else
				dx = sd1.x - sd1.r.x1 +- pTheme->GetArrowPadding ();
			dy = dx * y / x;
		} else {
			if (y > 0)
				dy = sd1.y - sd1.r.y0 + pTheme->GetArrowPadding ();
			else
				dy = sd1.y - sd1.r.y1 - pTheme->GetArrowPadding ();
			dx = dy * x / y;
		}
		dx = x1 * pTheme->GetZoomFactor () - (sd1.x - dx);
		dy = y1 * pTheme->GetZoomFactor () - (sd1.y - dy);
		(*i).first->Move (dx / pTheme->GetZoomFactor (), dy / pTheme->GetZoomFactor ());
		pView->Update ((*i).first);
		sd1.r.x0 += dx;
		sd1.r.x1 += dx;
		sd1.x += dx;
		sd1.r.y0 += dy;
		sd1.r.y1 += dy;
		sd1.y += dy;
		data[(*i).first] = sd1;
		AlignStep (data, (*i).first, pView, pData);
	}
}

void gcpRetrosynthesis::Align ()
{
	gcp::Document *pDoc = reinterpret_cast<gcp::Document *> (GetDocument ());
	gcp::Theme *pTheme = pDoc->GetTheme ();
	gcp::View *pView = pDoc->GetView ();
	gcp::WidgetData  *pData = reinterpret_cast<gcp::WidgetData *> (g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data"));
	map<string, Object*>::iterator i;
	Object *pObj = GetFirstChild (i);
	map<Object*, StepData> positions;
	StepData sd;
	while (pObj) {
		if (pObj->GetType () == RetrosynthesisStepType) {
			pData->GetObjectBounds (pObj, &sd.r);
			sd.x = (sd.r.x0 + sd.r.x1) / 2.;
			sd.y = pObj->GetYAlign () * pTheme->GetZoomFactor ();
			positions[pObj] = sd;
		}
		pObj = GetNextChild (i);
	}
	AlignStep (positions, Target, pView, pData);
}

double gcpRetrosynthesis::GetYAlign ()
{
	return (Target)? Target->GetYAlign (): 0.;
}

static void do_destroy_retrosynthesis (void *data)
{
	gcpRetrosynthesis *rs = reinterpret_cast<gcpRetrosynthesis *> (data);
	gcp::Document *pDoc = reinterpret_cast<gcp::Document *> (rs->GetDocument ());
	gcp::WidgetData *pData = reinterpret_cast<gcp::WidgetData *> (g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data"));
	pData->Unselect (rs);
	gcp::Operation *pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
	pOp->AddObject (rs, 0);
	delete rs;
	pDoc->FinishOperation ();
}

bool gcpRetrosynthesis::BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y)
{
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	GtkActionGroup *group = gtk_action_group_new ("retrosynthesis");
	GtkAction *action = gtk_action_new ("destroy-rs", _("Destroy the retrosynthesis path"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_insert_action_group (uim, group, 0);
	g_object_unref (group);
	char buf[] = "<ui><popup><menuitem action='destroy-rs'/></popup></ui>";
	gtk_ui_manager_add_ui_from_string (uim, buf, -1, NULL);
	GtkWidget *w = gtk_ui_manager_get_widget (uim, "/popup/destroy-rs");
	g_signal_connect_swapped (w, "activate", G_CALLBACK (do_destroy_retrosynthesis), this);
	Object::BuildContextualMenu (UIManager, object, x, y);
	return true;
}

bool gcpRetrosynthesis::OnSignal (G_GNUC_UNUSED SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	Validate (true);
	Align ();
	if (GetChildrenNumber () == 1)
		delete this;
	return true;
}

gcpRetrosynthesis::gcpRetrosynthesis (Object* parent, gcpRetrosynthesisStep *step): Object (RetrosynthesisType)
{
	SetId ("rsy1");
	SetParent (parent);
	Target = step;
	AddChild (Target);
	set<Object *> Objects;
	BuildConnectivity (Objects, Target);
	set<Object *>::iterator i, end = Objects.end ();
	for (i = Objects.begin (); i != end; i++)
		AddChild (*i);
	Align ();
}

void gcpRetrosynthesis::Transform2D (G_GNUC_UNUSED Matrix2D& m, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y)
{
}

std::string gcpRetrosynthesis::Name ()
{
	return _("Retrosynthesis");
}
