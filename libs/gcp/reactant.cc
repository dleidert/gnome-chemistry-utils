// -*- C++ -*-

/* 
 * GChemPaint library
 * reactant.cc 
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
#include "reactant.h"
#include "reaction-step.h"
#include "application.h"
#include "document.h"
#include "text.h"
#include "theme.h"
#include "tool.h"
#include "view.h"
#include <glib/gi18n-lib.h>

namespace gcp {

extern xmlDocPtr pXmlDoc;

Reactant::Reactant (): Object (ReactantType)
{
	SetId ("r1");
	m_Stoich = 0;
	Child = NULL;
	Stoichiometry = NULL;
}

Reactant::Reactant (ReactionStep* step, Object *object)	throw (invalid_argument): Object (ReactantType)
{
	SetId ("r1");
	step->AddChild (this);
	GetDocument ()->EmptyTranslationTable();
	static const set<TypeId>& allowed_types = Object::GetRules ("reactant", RuleMayContain);
	if (allowed_types.find (object->GetType ()) == allowed_types.end ())
		throw invalid_argument ("invalid reactant");
	AddChild (object);
	Child = object;
	Stoichiometry = NULL;
	m_Stoich = 0;
}

Reactant::~Reactant ()
{
}

static void do_add_stoichiometry (Reactant *reactant)
{
	reactant->AddStoichiometry ();
}

bool Reactant::BuildContextualMenu (GtkUIManager *UIManager, Object *object)
{
	bool result = false;
	if (m_Stoich == 0 && !Stoichiometry) {
		GtkActionGroup *group = gtk_action_group_new ("reactant");
		GtkAction *action = gtk_action_new ("stoichiometry", _("Add a stoichiometry coefficient"), NULL, NULL);
		gtk_action_group_add_action (group, action);
		gtk_ui_manager_insert_action_group (UIManager, group, 0);
		char buf[] = "<ui><popup><menuitem action='stoichiometry'/></popup></ui>";
		gtk_ui_manager_add_ui_from_string (UIManager, buf, -1, NULL);
		GtkWidget *w = gtk_ui_manager_get_widget (UIManager, "/popup/stoichiometry");
		g_signal_connect_swapped (w, "activate", G_CALLBACK (do_add_stoichiometry), this);
		result = true;
	}
	return result | GetParent ()->BuildContextualMenu (UIManager, object);
}

xmlNodePtr Reactant::Save (xmlDocPtr xml)
{
	if (!Child)
		return NULL;
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (const xmlChar*) "reactant", NULL);
	SaveId (node);
	xmlNodePtr child = Child->Save (xml);
	xmlAddChild (node, child);
	if (Stoichiometry) {
		xmlNodePtr stoich = Stoichiometry->Save (xml);
		xmlNodeSetName (stoich, (const xmlChar*) "stoichiometry");
		xmlAddChild (node, stoich);
	}
	return node;
}

bool Reactant::Load (xmlNodePtr node)
{
	xmlChar* buf;
	xmlNodePtr child;

	Lock ();
	buf = xmlGetProp (node, (xmlChar*) "id");
	if (buf) {
		SetId ((char*) buf);
		xmlFree (buf);
	}
	child = node->children;
	Document *pDoc = (Document*) GetDocument ();
	while (child) {
		if (!strcmp ((const char*) child->name, "stoichiometry")) {
			if (Stoichiometry) {
				Lock (false);
				return false;
			}
			Stoichiometry = new Text ();
			AddChild (Stoichiometry);
			if (!Stoichiometry->Load (child)) {
				delete Stoichiometry;
				Lock (false);
				return false;
			};
			pDoc->AddObject (Stoichiometry);
		} else {
			if (Child) {
				if (strcmp ((const char*) child->name, "text")) {
					Lock (false);
					return false;
				} else {
					child = child->next;
					continue;
				}
			}
			Child = CreateObject ((const char*) child->name, this);
			if (Child) {
				AddChild (Child);
				if (!Child->Load (child)) {
					delete Child;
					Child = NULL;
				}
			}
		}
		child = child->next;
	}
	Lock (false);
	return (Child != NULL);
}

double Reactant::GetYAlign ()
{
	return (Child)? Child->GetYAlign (): 0.;
}

void Reactant::AddStoichiometry ()
{
	Document *pDoc = dynamic_cast <Document*> (GetDocument ());
	Application * pApp = pDoc->GetApplication ();
	View *pView = pDoc->GetView ();
	Theme *pTheme= pDoc->GetTheme ();
	WidgetData *pData = (WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
	ArtDRect rect;
	pData->GetObjectBounds (this, &rect);
	double x = rect.x0 / pTheme->GetZoomFactor ();
	Text *text = new Text (x, GetYAlign () + pView->GetBaseLineOffset ());
	Stoichiometry = text;
	AddChild (text);
	pDoc->AddObject (text);
	gnome_canvas_update_now (GNOME_CANVAS (pData->Canvas));
	pData->GetObjectBounds (text, &rect);
	Child->Move (rect.x1 / pTheme->GetZoomFactor () + pTheme->GetStoichiometryPadding () - x, 0.);
	Tool *tool = pApp->GetTool ("Text");
	GetParent ()->EmitSignal (OnChangedSignal);
	pApp->ActivateTool ("Text", true);
	tool->OnClicked (pView, text, rect.x0 * pTheme->GetZoomFactor (), GetYAlign () * pTheme->GetZoomFactor (), 0);
}

bool Reactant::OnSignal (SignalId Signal, Object *Obj)
{
	if (Signal == OnChangedSignal) {
		Document *pDoc = (Document*) GetDocument ();
		Theme *pTheme= pDoc->GetTheme ();
		WidgetData *pData = (WidgetData*) g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data");
		ArtDRect rect;
		unsigned n = GetChildrenNumber ();
		map<string, Object*>::iterator i;
		Object *pObj;
		if (n == 0)
			delete this;
		else if (n == 1) {
			if (Stoichiometry) {
				// Child or stoichiometry have been deleted
				pObj = GetFirstChild (i);
				if (pObj == Child)
					Stoichiometry = NULL;
				else {
					pDoc->Remove (Stoichiometry);
					delete this;
				}
			}
		} else if ((n == 2) && Stoichiometry) {
			// Just need to space the two children
			gnome_canvas_update_now (GNOME_CANVAS (pData->Canvas));
			pData->GetObjectBounds (Stoichiometry, &rect);
			double x = rect.x1 / pTheme->GetZoomFactor () + pTheme->GetStoichiometryPadding ();
			pData->GetObjectBounds (Child, &rect);
			Child->Move (x - rect.x0 / pTheme->GetZoomFactor (), 0.);
			PangoLayout *layout = ((TextObject*) Stoichiometry)->GetLayout ();
			const char *txt = pango_layout_get_text (layout);
			char *endptr;
			int n = strtol (txt, &endptr, 10);
			m_Stoich = (!*endptr)? n: 0;
		} else {
			// Most probably child has been splitted
			xmlNodePtr node = NULL;
			bool ChildFound = false;
			ReactionStep *step = reinterpret_cast<ReactionStep*> (GetParent ());
			if (Stoichiometry)
				node = Stoichiometry->Save (pXmlDoc);
			pObj = GetFirstChild (i);
			while (pObj) {
				if (pObj == Child)
					ChildFound = true;
				else if (pObj != Stoichiometry) {
					Reactant *reactant = new Reactant (step, pObj);
					if (Stoichiometry) {
						reactant->Stoichiometry = new Text ();
						reactant->AddChild (reactant->Stoichiometry);
						pDoc->AddObject (reactant->Stoichiometry);
						reactant->Stoichiometry->Load (node);
						reactant->EmitSignal (OnChangedSignal);
					}
					pObj = GetFirstChild (i);
					continue;
				}
				pObj = GetNextChild (i);
			}
			if (!ChildFound) {
				if (Stoichiometry)
					pDoc->Remove (Stoichiometry);
				delete this;
			}
			if (node)
				xmlFreeNode (node);
			EmitSignal (OnChangedSignal);
		}
	}
	return true;
}

}	//	namespace gcp
