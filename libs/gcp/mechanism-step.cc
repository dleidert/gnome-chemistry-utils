// -*- C++ -*-

/*
 * GChemPaint library
 * mechanism-step.cc
 *
 * Copyright (C) 2009-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "mechanism-step.h"
#include "mechanism-arrow.h"
#include "molecule.h"
#include "operation.h"
#include "reaction-step.h"
#include <gcu/document.h>
#include <gcugtk/ui-manager.h>
#include <glib/gi18n.h>

namespace gcp {

class MechanismStepPrivate {
public:
	static void DoSelectAlignment (GObject *action, MechanismStep* step);
};

void MechanismStepPrivate::DoSelectAlignment (GObject *action, MechanismStep* step)
{
	step->m_Alignment = reinterpret_cast < gcu::Object * > (g_object_get_data (action, "item"));
	step->EmitSignal (OnChangedSignal);
}

gcu::TypeId MechanismStepType;
extern gcu::SignalId OnChangedSignal;

MechanismStep::MechanismStep (gcu::TypeId type):
	Step (type),
	m_bLoading (false),
	m_Alignment (NULL)
{
	SetId ("ms1");
}

MechanismStep::~MechanismStep ()
{
	Lock ();
}

std::string MechanismStep::Name ()
{
	return _("Mechanism step");
}

double MechanismStep::GetYAlign ()
{
	if (m_Alignment)
		return m_Alignment->GetYAlign ();
	unsigned nb = 0;
	double res = 0.;
	std::map <std::string, Object *>::iterator i;
	for (Object *obj = GetFirstChild (i); obj; obj = GetNextChild (i))
		if (obj->GetType () == gcu::MoleculeType) {
			res += obj->GetYAlign ();
			nb++;
		}
	return res / nb;
}

bool MechanismStep::OnSignal (gcu::SignalId Signal, G_GNUC_UNUSED gcu::Object *Child)
{
	if (Signal == OnChangedSignal) {
		if (m_bLoading)
			return false;
		std::map <std::string, Object *>::iterator i;
		std::set <Object *> molecules;
		for (Object *obj = GetFirstChild (i); obj; obj = GetNextChild (i))
			if (obj->GetType () == MechanismArrowType) {
				MechanismArrow *arrow = static_cast <MechanismArrow *> (obj);
				Object *mol = arrow->GetSource ()->GetMolecule ();
				if (mol)
					molecules.insert (mol);
				mol = arrow->GetTarget ()->GetMolecule ();
				if (mol)
					molecules.insert (mol);
			}
		Object *parent = GetParent (), *group = GetGroup ();
		Operation *op = static_cast <Document *> (GetDocument ())->GetCurrentOperation ();
		if (molecules.empty ()) {
			// if no arrow remains, delete this
			Object *obj;
			SetParent (NULL); //
			if (parent->GetType () == ReactionStepType) {
				ReactionStep *step = static_cast <ReactionStep *> (parent);
				while ((obj = GetFirstChild (i)))
					if (obj->GetType () == gcu::MoleculeType)
						step->AddMolecule (static_cast <Molecule *> (obj), false);
			} else
				while ((obj = GetFirstChild (i))) {
					parent->AddChild (obj);
					if (!group && op)
						op->AddObject (obj, 1);

				}
			delete this;
			parent->EmitSignal (OnChangedSignal);
			return false;
		} else {
			Object *obj;
			if (parent->GetType () == ReactionStepType) {
				ReactionStep *step = static_cast <ReactionStep *> (parent);
				std::set <Object *> orphans;
				std::set <Object *>::iterator j, end = molecules.end ();
				// search for molecules without a mechanism arrow
				for (obj = GetFirstChild (i); obj; obj = GetNextChild (i))
					if (obj->GetType () != MechanismArrowType && molecules.find (obj) == end)
						orphans.insert (obj);
				// now remove orphans from this
				end = orphans.end ();
				for (j = orphans.begin (); j != end; j++) {
					if (step)
						step->AddMolecule (static_cast <Molecule *> (*j), false);
					else
						parent->AddChild (*j);
				}
			}
		}
	}
	return true;
}

xmlNodePtr MechanismStep::Save (xmlDocPtr xml) const
{
	// don't save an empty step, note that this might not occur
	return (GetChildrenNumber ())? gcu::Object::Save (xml): NULL;
}

bool MechanismStep::Load (xmlNodePtr node)
{
	m_bLoading = true;
	bool res = gcu::Object::Load (node);
	m_bLoading = false;
	GetDocument ()->ObjectLoaded (this);
	return res;
}

void MechanismStep::NotifyEmpty ()
{
	// only delete when not a mesomer or a reaction step
	if (!HasChildren () && !IsLocked () && GetType () == MechanismStepType)
		delete this;
}

bool MechanismStep::BuildContextualMenu (gcu::UIManager *UIManager, gcu::Object *object, double, double)
{
	if (GetType () != MechanismStepType)
		return false;	// no need for a menu if it is a reaction step
	if (object->GetType () == MechanismArrowType)
		return false;	// don't align on a curved arrow
	if (object->GetMolecule () == NULL)
		return false;
	// now ensure that several molecules exist
	unsigned nb = 0;
	std::map <std::string, Object *>::iterator i;
	for (Object *obj = GetFirstChild (i); obj; obj = GetNextChild (i))
		if (obj->GetType () == gcu::MoleculeType)
			nb++;
	if (nb < 2)
		return false;
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	GtkActionGroup *group = gtk_action_group_new ("mstep");
	GtkAction *action;
	action = gtk_action_new ("Mechanism step", _("Mechanism step"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	action = gtk_action_new ("mstep-select-align", _("Align using this molecule"), NULL, NULL);
	g_signal_connect (action, "activate", G_CALLBACK (MechanismStepPrivate::DoSelectAlignment), this);
	g_object_set_data (G_OBJECT (action), "item", object->GetMolecule ());
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Mechanism step'><menuitem action='mstep-select-align'/></menu></popup></ui>", -1, NULL);
	gtk_ui_manager_insert_action_group (uim, group, 0);
	g_object_unref (group);
	return true;
}

}	//	namespace gcp
