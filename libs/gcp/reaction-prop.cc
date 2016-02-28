// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-prop.cc
 *
 * Copyright (C) 2007-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "reaction-prop.h"
#include "reaction-prop-dlg.h"
#include "reaction-arrow.h"
#include <gcugtk/ui-manager.h>
#include <gcu/objprops.h>
#include <glib/gi18n-lib.h>
#include <sstream>
#include <cstring>

using namespace gcu;

namespace gcp {

TypeId ReactionPropType = NoType;

char const *ReactionPropRoles[] = {
	N_("Unknown"),
	N_("Catalyst"),
	N_("Reactant"),
	N_("Product"),
	N_("Solvent"),
	N_("Temperature"),
	N_("Pressure"),
	N_("Time"),
	N_("Enthalpy"),
};

static unsigned RoleFromString (char const *role)
{
	unsigned res = REACTION_PROP_MAX;
	while (res > REACTION_PROP_UNKNOWN &&
		   strcmp (ReactionPropRoles[--res], role)) ;
	return res;
}

ReactionProp::ReactionProp ():
	Object (ReactionPropType),
	DialogOwner (),
	m_Object (NULL),
	m_Role (REACTION_PROP_UNKNOWN),
	m_Step (0),
	m_Line (0),
	m_Rank (0)
{
}

ReactionProp::ReactionProp (ReactionArrow *parent, Object *child):
	Object (ReactionPropType),
	DialogOwner (),
	m_Object (child),
	m_Role (REACTION_PROP_UNKNOWN),
	m_Step (0),
	m_Line (0),
	m_Rank (0)
{
	SetParent (parent);
	AddChild (child);
}

ReactionProp::~ReactionProp ()
{
}

xmlNodePtr ReactionProp::Save (xmlDocPtr xml) const
{
	xmlNodePtr node;
	node = Object::Save (xml);
	if (!node)
		return NULL;
	xmlNewProp (node, (xmlChar*) "role",  (xmlChar*) ReactionPropRoles[m_Role]);
	char *buf;
	if (m_Step > 1) {
			buf = g_strdup_printf ("%u", m_Step);
			xmlNewProp (node, (xmlChar*) "step", (xmlChar*) buf);
			g_free (buf);
	}
	if (m_Line > 1) {
			buf = g_strdup_printf ("%u", m_Line);
			xmlNewProp (node, (xmlChar*) "line", (xmlChar*) buf);
			g_free (buf);
	}
	if (m_Rank > 1) {
			buf = g_strdup_printf ("%u", m_Rank);
			xmlNewProp (node, (xmlChar*) "rank", (xmlChar*) buf);
			g_free (buf);
	}
	return node;
}

bool ReactionProp::Load (xmlNodePtr node)
{
	bool res = Object::Load (node);
	if (GetChildrenNumber () != 1)
		return false;
	std::map < std::string, gcu::Object * >::iterator i;
	m_Object = GetFirstChild (i);
	if (res) {
		char *buf = (char*) xmlGetProp (node, (xmlChar*) "role");
		if (buf) {
			m_Role = RoleFromString (buf);
			xmlFree (buf);
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "step");
		if (buf) {
			m_Step = strtoul (buf, NULL, 10);
			xmlFree (buf);
		} else
			m_Step = 1;
		buf = (char*) xmlGetProp (node, (xmlChar*) "line");
		if (buf) {
			m_Line = strtoul (buf, NULL, 10);
			xmlFree (buf);
		} else
			m_Line = 1;
		buf = (char*) xmlGetProp (node, (xmlChar*) "rank");
		if (buf) {
			m_Rank = strtoul (buf, NULL, 10);
			xmlFree (buf);
		} else
			m_Rank = 1;
	}
	return res;
}

bool ReactionProp::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (Signal == OnChangedSignal && !HasChildren ()) {
		static_cast < ReactionArrow *> (GetParent ())->RemoveProp (this);
		delete this;
	}
	return true;
}

std::string ReactionProp::Name ()
{
	return _("Reaction property");
}

double ReactionProp::GetYAlign () const
{
	return (m_Object)? m_Object->GetYAlign (): go_nan;
}

static void do_props (ReactionProp *prop)
{
	gcu::Dialog *dialog = prop->GetDialog ("reaction-prop");
	if (dialog)
		dialog->Present ();
	else
		new ReactionPropDlg (static_cast < ReactionArrow * > (prop->GetParent ()), prop);
}

bool ReactionProp::BuildContextualMenu (gcu::UIManager *UIManager, gcu::Object *, double, double)
{
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	GtkActionGroup *group = NULL;
	GtkAction *action;
	group = gtk_action_group_new ("reaction-prop");
	action = gtk_action_new ("props", _("Role and position..."), _("Attached object properties"), NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	g_signal_connect_swapped (action, "activate", G_CALLBACK (do_props), this);
	gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menuitem action='props'/></popup></ui>", -1, NULL);
	gtk_ui_manager_insert_action_group (uim, group, 0);
	return false;
}

std::string ReactionProp::GetProperty (unsigned property) const
{
	std::ostringstream res;
	switch (property) {
	case GCU_PROP_REACTION_ARROW_PROP_STEP:
		res << m_Step;
		break;
	case GCU_PROP_REACTION_ARROW_PROP_LINE:
		res << m_Line;
		break;
	case GCU_PROP_REACTION_ARROW_PROP_POSITION:
		res << m_Rank;
		break;
	case GCU_PROP_ARROW_OBJECT:
		res << m_Object->GetId ();
		break;
	default:
		return Object::GetProperty (property);
	}
	return res.str ();
}

bool ReactionProp::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_REACTION_ARROW_PROP_STEP:
		m_Step = atoi (value);
		break;
	case GCU_PROP_REACTION_ARROW_PROP_LINE:
		m_Line = atoi (value);
		break;
	case GCU_PROP_REACTION_ARROW_PROP_POSITION:
		m_Rank = atoi (value);
		break;
	case GCU_PROP_ARROW_OBJECT:
		SetChild (GetDocument ()->GetDescendant (value));
		break;
	default:
		return Object::SetProperty (property, value);
	}
	return true;
}

void ReactionProp::SetChild (gcu::Object *child)
{
	if (child == NULL)
		return;
	if (m_Object)
		delete m_Object;
	m_Object = child;
	AddChild (child);
}

}	//	namespace gcp
