// -*- C++ -*-

/*
 * GChemPaint arrows plugin
 * retrosynthesisstep.cc
 *
 * Copyright (C) 2005-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "retrosynthesisstep.h"
#include "retrosynthesis.h"
#include "retrosynthesisarrow.h"
#include <gcp/application.h>
#include <gcp/molecule.h>
#include <gcp/document.h>
#include <gcu/objprops.h>
#include <glib/gi18n-lib.h>

TypeId RetrosynthesisStepType;

gcpRetrosynthesisStep::gcpRetrosynthesisStep (): gcp::Step (RetrosynthesisStepType)
{
	SetId ("rss1");
	Molecule = NULL;
}

gcpRetrosynthesisStep::~gcpRetrosynthesisStep ()
{
	if (IsLocked ())
		return;
	gcp::Document *pDoc = reinterpret_cast<gcp::Document *> (GetDocument ());
	gcp::Operation *pOp = pDoc->GetCurrentOperation ();
	gcpRetrosynthesis *rs = reinterpret_cast<gcpRetrosynthesis *> (GetParent ());
	if (!rs)
		return;
	map<string, Object *>::iterator i;
	Object *Child, *Group = rs->GetGroup ();
	while (HasChildren ()) {
		Child = GetFirstChild (i);
		GetParent ()->GetParent ()->AddChild (Child);
		if (pOp && !Group)
			pOp->AddObject (Child, 1);
	}
}

gcpRetrosynthesisStep::gcpRetrosynthesisStep (gcpRetrosynthesis *synthesis, gcp::Molecule* molecule) throw (std::invalid_argument): gcp::Step (RetrosynthesisStepType)
{
	if (!synthesis || !molecule)
		throw invalid_argument ("NULL argument to gcpRetrosynthesisStep constructor!");
	SetId ("rss1");
	synthesis->AddChild (this);
	GetDocument ()->EmptyTranslationTable();
	AddChild (molecule);
	Molecule = molecule;
	Arrow = NULL;
}

void gcpRetrosynthesisStep::AddArrow (gcpRetrosynthesisArrow *arrow, gcpRetrosynthesisStep *step, bool start) throw (std::invalid_argument)
{
	if (start) {
		if (Arrows[step])
			throw invalid_argument (_("Only one arrow can link two given steps."));
		Arrows[step] = arrow;
	} else {
		Arrow = arrow;
		Precursor = step;
	}
}

bool gcpRetrosynthesisStep::Load(xmlNodePtr node)
{
	if (Object::Load (node)) {
		if (GetChildrenNumber () != 1)
			return false;
		map<string, Object*>::iterator i;
		Molecule = reinterpret_cast<gcp::Molecule *> (GetFirstChild (i));
		GetDocument ()->ObjectLoaded (this);
		return true;
	}
	return false;
}

double gcpRetrosynthesisStep::GetYAlign ()
{
	return (Molecule)? Molecule->GetYAlign (): 0.;
}

void gcpRetrosynthesisStep::RemoveArrow (G_GNUC_UNUSED gcpRetrosynthesisArrow *arrow, gcpRetrosynthesisStep *step)
{
	if (step == Precursor) {
		Precursor = NULL;
		Arrow = NULL;
	} else
		Arrows.erase (step);
}

bool gcpRetrosynthesisStep::OnSignal (G_GNUC_UNUSED SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (GetChildrenNumber () != 1) {
		delete GetParent ();
		return false;
	}
	return true;
}

bool gcpRetrosynthesisStep::SetProperty (unsigned property, char const *value)
{
	gcu::Document *doc = GetDocument ();
	switch (property) {
	case GCU_PROP_MOLECULE: {
		if (doc == NULL)
			return false;
		if (Molecule != NULL && !strcmp (Molecule->GetId (), value)) {
			break;
		}
		gcu::Object *new_child = doc->GetDescendant (value);
		gcp::Application *app = static_cast <gcp::Application * > (doc->GetApplication ());
		std::set < TypeId > const &rules = app->GetRules (RetrosynthesisStepType, RuleMayContain);
		if (new_child != NULL && rules.find (new_child->GetType ()) != rules.end ()) {
			if (Molecule != NULL)
				Molecule->SetParent (doc);
			Molecule = dynamic_cast < gcp::Molecule * > (new_child);
			if (Molecule)
				AddChild (Molecule);
		}
		break;
	}
	}
	return true;
}

std::string gcpRetrosynthesisStep::Name ()
{
	return _("Retrosynthesis step");
}
