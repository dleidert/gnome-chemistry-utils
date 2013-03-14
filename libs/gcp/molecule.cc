// -*- C++ -*-

/*
 * GChemPaint library
 * molecule.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "atom.h"
#include "bond.h"
#include "document.h"
#include "molecule.h"
#include "tool.h"
#include "view.h"
#include <gcugtk/stringdlg.h>
#include <gcugtk/ui-manager.h>
#include <gcu/chain.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-output-memory.h>
#include <glib/gi18n-lib.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <map>
#include <set>
#include <sstream>

using namespace gcu;
using namespace std;

namespace gcp {

class MoleculePrivate
{
public:
	static void ShowInChIKey (Molecule *mol);
	static void ShowInChI (Molecule *mol);
	static void ShowSMILES (Molecule *mol);
	static void ExportToGhemical (Molecule *mol);
	static void ExportTo3D (Molecule *mol);
	static void ExportToAvogadro (Molecule *mol);
	static char *Build3D (Molecule *mol);
};

void MoleculePrivate::ShowInChIKey (Molecule *mol)
{

	new gcugtk::StringDlg (reinterpret_cast<Document *>(mol->GetDocument ()), mol->GetInChIKey (), gcugtk::StringDlg::INCHIKEY);
}

void MoleculePrivate::ShowInChI (Molecule *mol)
{

	new gcugtk::StringDlg (reinterpret_cast<Document *>(mol->GetDocument ()), mol->GetInChI (), gcugtk::StringDlg::INCHI);
}

void MoleculePrivate::ShowSMILES (Molecule *mol)
{

	new gcugtk::StringDlg (reinterpret_cast<Document *>(mol->GetDocument ()), mol->GetSMILES (), gcugtk::StringDlg::SMILES);
}

char *MoleculePrivate::Build3D (Molecule *mol)
{
	std::string const &InChI = mol->GetInChI ();
	GsfInput *in = gsf_input_memory_new (reinterpret_cast <guint8 const *> (InChI.c_str ()), InChI.length (), false);
	char *cml = mol->GetDocument ()->GetApp ()->ConvertToCML (in, "inchi", "-hc --gen3D");
	g_object_unref (in);
	return cml;
}

void MoleculePrivate::ExportToGhemical (Molecule *mol)
{
	char *cml = Build3D (mol);
	if (!cml) // how does this happen?
		return;
	char *tmpname = g_strdup ("/tmp/gprXXXXXX.gpr");
	int f = g_mkstemp (tmpname);
	close (f);
	std::string uri = "file://";
	uri += tmpname;
	mol->GetDocument ()->GetApp ()->ConvertFromCML (cml, uri, "gpr");
	g_free (cml);
	char *command_line = g_strconcat ("ghemical -f ", tmpname, NULL);
	g_free (tmpname);
	g_spawn_command_line_async (command_line, NULL);
	g_free (command_line);
}

void MoleculePrivate::ExportTo3D (Molecule *mol)
{
	char *cml = Build3D (mol);
	if (!cml) // how does this happen?
		return;
	char *tmpname = g_strdup ("/tmp/cmlXXXXXX.cml");
	int f = g_mkstemp (tmpname);
	write (f, cml, strlen (cml));
	close (f);
	g_free (cml);
	char *command_line = g_strconcat ("gchem3d-"GCU_API_VER" ", tmpname, NULL);
	g_free (tmpname);
	g_spawn_command_line_async (command_line, NULL);
	g_free (command_line);
}

void MoleculePrivate::ExportToAvogadro (Molecule *mol)
{
	char *cml = Build3D (mol);
	if (!cml) // how does this happen?
		return;
	char *tmpname = g_strdup ("/tmp/cmlXXXXXX.cml");
	int f = g_mkstemp (tmpname);
	write (f, cml, strlen (cml));
	close (f);
	g_free (cml);
	char *command_line = g_strconcat ("avogadro ", tmpname, NULL);
	g_free (tmpname);
	g_spawn_command_line_async (command_line, NULL);
	g_free (command_line);
}

static void do_select_alignment (GObject *action, Molecule* pMol)
{
	Object *object = (Object*) g_object_get_data (action, "item");
	pMol->SelectAlignmentItem (object);
}

static void do_open_in_calc (Molecule* pMol)
{
	pMol->OpenCalc ();
}

Molecule::Molecule (TypeId Type): gcugtk::Molecule (Type, gcu::ContentType2D)
{
	m_Alignment = NULL;
	m_IsResidue = false;
}

Molecule::Molecule (Atom* pAtom): gcugtk::Molecule (pAtom, gcu::ContentType2D)
{
	m_Alignment = NULL;
	m_IsResidue = false;
}

Molecule::~Molecule ()
{
}

void Molecule::AddChild (Object* object)
{
	switch (object->GetType ()) {
	case gcu::AtomType:
			if (object->GetParent () && object->GetParent ()->GetType () == FragmentType)
				object = object->GetParent ();
			else {
				gcu::Molecule::AddChild (object);
				break;
			}
	case FragmentType: {
		Fragment *fragment = reinterpret_cast<Fragment *> (object);
		m_Fragments.remove (fragment);
		AddFragment (fragment);
		break;
	}
	default:
		gcu::Molecule::AddChild (object);
		break;
	}
}

void Molecule::AddAtom (gcu::Atom* pAtom)
{
	gcu::Molecule::AddAtom (pAtom);
	if (!pAtom->GetZ ())
		m_IsResidue = true;
}

void Molecule::AddFragment (Fragment* pFragment)
{
	m_Fragments.remove (pFragment); // just in case
	m_Fragments.push_back (pFragment);
	Object::AddChild (pFragment);
}

void Molecule::AddBond (gcu::Bond* pBond)
{
	if (pBond->GetAtom (0) && pBond->GetAtom (1))
		CheckCrossings (reinterpret_cast <Bond *> (pBond));
	gcu::Molecule::AddBond (pBond);
	EmitSignal (OnChangedSignal);
}

void Molecule::Remove (Object* pObject)
{
	if (pObject == m_Alignment)
		m_Alignment = NULL;
	switch (pObject->GetType ()) {
	case FragmentType:
		m_Fragments.remove ((Fragment*) pObject);
		break;
	default:
		gcu::Molecule::Remove (pObject);
		break;
	}
	pObject->SetParent (GetParent ());
}

typedef struct {
	Atom *pAtom;
	int sb; // number of shared bonds
	int so0, so1; // total bond order of shared bonds for each atom
} MergedAtom;

bool Molecule::Merge (Molecule* pMolecule, bool RemoveDuplicates)
{
	Atom* pAtom;
	Fragment* pFragment;
	Bond* pBond;
	Chain* pChain;
	Cycle* pCycle;
	if (RemoveDuplicates) {
		std::list<gcu::Atom*>::iterator i = m_Atoms.begin (), end = m_Atoms.end ();
		double x, y, x0, y0, x1, y1;
		MergedAtom *ma;
		Bond *b0, *b1;
		bool DoMerge;
		int n;
		map<Atom*, MergedAtom*> AtomMap;
		map<Atom*, MergedAtom*>::iterator j, endj;
		map<Bond*, Bond*> BondMap;
		map<Bond*, Bond*>::iterator b, endb;
		for (; i != end; i++) {
			(*i)->GetCoords (&x, &y);
			pAtom = (Atom*) pMolecule->GetAtomAt (x, y);
			if (pAtom) {
				if ((*i)->GetZ()!= pAtom->GetZ())
				{
					// Cannot merge atoms which are not of the same element.
					endj = AtomMap.end ();
					for (j = AtomMap.begin (); j != endj; j++)
						delete (*j).second;
					return false;
				}
				ma = new MergedAtom;
				ma->sb = ma->so0 = ma->so1 = 0;
				ma->pAtom = pAtom;
				endj = AtomMap.end ();
				for (j = AtomMap.begin (); j != endj; j++) {
					if ((b1 = (Bond*) pAtom->GetBond ((*j).second->pAtom))) {
						b0 = (Bond*) (*i)->GetBond ((*j).first);
						if (b0) {
							ma->sb++;
							ma->so0 += b0->GetOrder ();
							ma->so1 += b1->GetOrder ();
							(*j).second->sb++;
							(*j).second->so0 += b0->GetOrder ();
							(*j).second->so1 += b1->GetOrder ();
							BondMap[b0] = b1;
						}
					}
				}
				AtomMap[reinterpret_cast <Atom *> (*i)] =  ma;
			}
		}
		// Now check if merging is possible for each shared atom.
		DoMerge = AtomMap.size () != 0;
		if (DoMerge) {
			x = y = 0.;
			endj = AtomMap.end ();
			for (j = AtomMap.begin (); j != endj; j++) {
				ma = (*j).second;
				n = ma->pAtom->GetTotalBondsNumber () - ma->so0 - ma->so1 + ma->sb;
				if (!(*j).first->AcceptNewBonds (n)) {
					DoMerge = false;
					break;
				}
				n = (*j).first->GetTotalBondsNumber () - ma->so0 - ma->so1 + ma->sb;
				if (!ma->pAtom->AcceptNewBonds (n)) {
					DoMerge = false;
					break;
				}
				(*j).first->GetCoords (&x0, &y0);
				ma->pAtom->GetCoords (&x1, &y1);
				x += x1 - x0;
				y += y1 - y0;
			}
		}
		if (DoMerge) {
			//First align molecules
			x /= 2.* AtomMap.size ();
			y /= 2.* AtomMap.size ();
			Move (x, y);
			pMolecule->Move (-x, -y);

			//Then align each atom individually
			endj = AtomMap.end ();
			for (j = AtomMap.begin (); j != endj; j++) {
				(*j).first->GetCoords (&x0, &y0);
				(*j).second->pAtom->GetCoords (&x1, &y1);
				(*j).first->Move ((x1 - x0) / 2.,(y1 - y0) / 2.);
			}
			View *pView = ((Document*) GetDocument ())->GetView ();

			/* Treat shared bonds (set order to 1, store max order in b1 and remove the bond
			 * from pMolecule. */
			endb = BondMap.end ();
			for (b = BondMap.begin (); b != endb; b++) {
				b1 = (*b).second;
				n = (*b).first->GetOrder ();
				pView->Remove (b1);
				pMolecule->Remove (b1);
				(*b).first->SetOrder (1);
				pAtom = (Atom*) b1->GetAtom (0);
				pAtom->RemoveBond (b1);
				b1->ReplaceAtom (pAtom, NULL);
				pAtom = (Atom*) b1->GetAtom (1);
				pAtom->RemoveBond (b1);
				b1->ReplaceAtom (pAtom, NULL);
				if (n > b1->GetOrder ())
					b1->SetOrder (n);
			}

			// Treat shared atoms and delete from pMolecule
			map< gcu::Atom *, gcu::Bond * >::iterator ai;
			endj = AtomMap.end ();
			for (j = AtomMap.begin (); j != endj; j++) {
				b0 = (Bond*) (*j).second->pAtom->GetFirstBond (ai);
				while (b0) {
					b0->ReplaceAtom ((*j).second->pAtom, (*j).first);
					(*j).first->AddBond (b0);
					b0 = (Bond*) (*j).second->pAtom->GetNextBond (ai);
				}
				pMolecule->Remove ((*j).second->pAtom);
				pView->Remove ((*j).second->pAtom);
				delete (*j).second->pAtom;
			}

			// Try to restore max bond order for shared bonds and destroy old bonds
			endb = BondMap.end ();
			for (b = BondMap.begin (); b != endb; b++) {
				n = (*b).second->GetOrder () - 1;
				b0 = (*b).first;
				while ((n > 0) &&
						(!((Atom*) b0->GetAtom (0))->AcceptNewBonds (n) ||
						!((Atom*) b0->GetAtom(1))->AcceptNewBonds (n)))
					n--;
				if (n > 0)
					b0->SetOrder (n + 1);
				delete (*b).second;
			}
		}

		// Clean memory
		endj = AtomMap.end ();
		for (j = AtomMap.begin (); j != endj; j++)
			delete (*j).second;
		//return if merging is not possible
		if (!DoMerge) return false;
	}
	while (!pMolecule->m_Atoms.empty ()) {
		pAtom =  reinterpret_cast <Atom *> (pMolecule->m_Atoms.front ());
		AddAtom (pAtom);
		pMolecule->m_Atoms.pop_front ();
	}
	while (!pMolecule->m_Fragments.empty ()) {
		pFragment = pMolecule->m_Fragments.front ();
		AddFragment (pFragment);
		pMolecule->m_Fragments.pop_front ();
	}
	while (!pMolecule->m_Bonds.empty ()) {
		pBond =  reinterpret_cast <Bond *> (pMolecule->m_Bonds.front ());
		AddBond (pBond);
		pMolecule->m_Bonds.pop_front ();
	}
	while (!pMolecule->m_Chains.empty ()) {
		//FIXME: Chains should change
		pChain = pMolecule->m_Chains.front ();
		m_Chains.push_back (pChain);
		pMolecule->m_Chains.pop_front ();
	}
	while (!pMolecule->m_Cycles.empty()) {
		pCycle = pMolecule->m_Cycles.front ();
		m_Cycles.push_back (pCycle);
		pMolecule->m_Cycles.pop_front ();
	}
	Object* pObj = pMolecule->GetParent ();
	delete pMolecule;
	pObj->EmitSignal (OnChangedSignal);
	if (RemoveDuplicates)
		UpdateCycles ();
	EmitSignal (OnChangedSignal);
	return true;
}

bool Molecule::Load (xmlNodePtr node)
{
	char* buf;
	xmlNodePtr child;
	Object* pObject;
	Document* pDoc = (Document*) GetDocument ();

	buf = (char*) xmlGetProp (node, (xmlChar*) "id");
	if (buf) {
		SetId (buf);
		xmlFree (buf);
	}
	child = GetNodeByName (node, "atom");
	while (child) {
		pObject = new Atom ();
		if (pDoc)
			AddChild (pObject);
		if (!pObject->Load (child)) {
			delete pObject;
			return false;
		}
		if (pDoc)
			pDoc->AddAtom ((Atom*) pObject);
		AddAtom ((Atom*) pObject);
		child = GetNextNodeByName (child->next, "atom");
	}
	// FIXME, the following looks like a kludge
	child = GetNodeByName (node, "pseudo-atom");
	while (child) {
		pObject = CreateObject ("pseudo-atom", pDoc);
		if (pDoc)
			AddChild (pObject);
		if (!pObject->Load (child)) {
			delete pObject;
			return false;
		}
		if (pDoc)
			pDoc->AddAtom ((Atom*) pObject);
		AddAtom ((Atom*) pObject);
		child = GetNextNodeByName (child->next, "pseudo-atom");
	}

	child = GetNodeByName (node, "fragment");
	while (child) {
		pObject = new Fragment ();
		if (pDoc)
			AddChild (pObject);
		if (!pObject->Load (child))  {
			delete pObject;
			return false;
		}
		if (pDoc)
			pDoc->AddFragment ((Fragment*) pObject);
		child = GetNextNodeByName (child->next, "fragment");
	}

	child = GetNodeByName (node, "bond");
	while (child) {
		pObject = new Bond ();
		AddBond ((Bond*) pObject);
		if (!pObject->Load (child)) {
			delete pObject;
			m_Bonds.remove ((Bond*) pObject);
			return false;
		}
		if (pDoc)
			pDoc->AddBond ((Bond*) pObject);
		child = GetNextNodeByName (child->next, "bond");
		CheckCrossings ((Bond*) pObject);
	}
/*	if (!m_Atoms.empty ()) {
		Atom* pAtom =  reinterpret_cast <Atom *> (m_Atoms.front ());
		list<gcu::Atom*>::iterator i = m_Atoms.begin ();
		i++;
		for (; i != m_Atoms.end (); i++)
			(*i)->SetParent (NULL);
		// erase cycles
		list<gcu::Bond*>::iterator j, jend = m_Bonds.end ();
		for (j = m_Bonds.begin (); j != jend; j++)
			(*j)->RemoveAllCycles ();
		Chain* pChain = new Chain (this, pAtom); //will find the cycles
		delete pChain;
	}*/

	child = GetNodeByName (node, "brackets");
	while (child) {
		pObject = CreateObject ((const char*) child->name, this);
		if (pDoc)
			AddChild (pObject);
		if (!pObject->Load (child))  {
			delete pObject;
			return false;
		}
		child = GetNextNodeByName (child->next, "brackets");
	}
	buf = (char*) xmlGetProp (node, (const xmlChar*) "valign");
	if (buf) {
		pDoc->SetTarget (buf, reinterpret_cast <Object **> (&m_Alignment), this, this, ActionDelete);
		xmlFree (buf);
	}
	pDoc->ObjectLoaded (this);
	return true;
}

void Molecule::Clear ()
{
	m_Bonds.clear ();
	m_Atoms.clear ();
	m_Fragments.clear ();
}

void Molecule::Transform2D (Matrix2D& m, double x, double y)
{
	Object::Transform2D (m, x, y);
	std::list<gcu::Atom*>::iterator i = m_Atoms.begin ();
	for (; i != m_Atoms.end (); i++)
	if (((*i)->GetZ () != 6) &&  reinterpret_cast <Atom *> (*i)->GetAttachedHydrogens () &&
		(*i)->GetBondsNumber ())  reinterpret_cast <Atom *> (*i)->Update ();
}

Object* Molecule::GetAtomAt (double x, double y, G_GNUC_UNUSED double z)
{
	// Make use of Bond::GetAtomAt
	std::list<gcu::Bond*>::iterator n, end = m_Bonds.end ();
	Object* pObj = NULL;
	for (n = m_Bonds.begin(); n != end; n++)
		if ((pObj = (*n)->GetAtomAt (x, y)))
			break;
	return pObj;
}

double Molecule::GetYAlign ()
{
	if (m_Alignment)
		return m_Alignment->GetYAlign ();
	double y, maxy = - DBL_MAX, miny = DBL_MAX;
	std::list<gcu::Atom*>::iterator i = m_Atoms.begin (), end = m_Atoms.end ();
	for (; i != end; i++) {
		y =  reinterpret_cast <Atom *> (*i)->GetYAlign ();
		if (y < miny)
			miny = y;
		if (y > maxy)
			maxy = y;
	}
	std::list<Fragment*>::iterator ig = m_Fragments.begin (), endg = m_Fragments.end ();
	for (; ig != endg; ig++) {
		y = (*ig)->GetYAlign ();
		if (y < miny)
			miny = y;
		if (y > maxy)
			maxy = y;
	}
	return (miny + maxy) / 2.0;
}

bool Molecule::BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y)
{
	if (m_IsResidue)
		return false;
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	GtkActionGroup *group = gtk_action_group_new ("molecule");
	GtkAction *action;
	action = gtk_action_new ("Molecule", _("Molecule"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	bool result = false;
	if (!m_Fragments.size ()) {
		Application *app = static_cast <Document *> (GetDocument ())->GetApplication ();
		if (app->Have3DSupport ()) {
			action = gtk_action_new ("open3d", _("Open 3D model in"), NULL, NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			if (app->GetHaveGhemical ()) {
				action = gtk_action_new ("ghemical", _("Ghemical"), NULL, NULL);
				g_signal_connect_swapped (action, "activate", G_CALLBACK (MoleculePrivate::ExportToGhemical), this);
				gtk_action_group_add_action (group, action);
				g_object_unref (action);
				gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Molecule'><menu action='open3d'><menuitem action='ghemical'/></menu></menu></popup></ui>", -1, NULL);
			}
			if (app->GetHaveGChem3D ()) {
				action = gtk_action_new ("gchem3d", _("GChem3D"), NULL, NULL);
				g_signal_connect_swapped (action, "activate", G_CALLBACK (MoleculePrivate::ExportTo3D), this);
				gtk_action_group_add_action (group, action);
				g_object_unref (action);
				gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Molecule'><menu action='open3d'><menuitem action='gchem3d'/></menu></menu></popup></ui>", -1, NULL);
			}
			if (app->GetHaveAvogadro ()) {
				action = gtk_action_new ("avogadro", _("Avogadro"), NULL, NULL);
				g_signal_connect_swapped (action, "activate", G_CALLBACK (MoleculePrivate::ExportToAvogadro), this);
				gtk_action_group_add_action (group, action);
				g_object_unref (action);
				gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Molecule'><menu action='open3d'><menuitem action='avogadro'/></menu></menu></popup></ui>", -1, NULL);
			}
		}
		BuildDatabasesMenu (uim, "<ui><popup><menu action='Molecule'>", "</menu></popup></ui>");
		action = gtk_action_new ("inchi", _("Generate InChI"), NULL, NULL);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (MoleculePrivate::ShowInChI), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Molecule'><menuitem action='inchi'/></menu></popup></ui>", -1, NULL);
		action = gtk_action_new ("inchikey", _("Generate InChIKey"), NULL, NULL);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (MoleculePrivate::ShowInChIKey), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Molecule'><menuitem action='inchikey'/></menu></popup></ui>", -1, NULL);
		action = gtk_action_new ("smiles", _("Generate SMILES"), NULL, NULL);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (MoleculePrivate::ShowSMILES), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Molecule'><menuitem action='smiles'/></menu></popup></ui>", -1, NULL);
		action = gtk_action_new ("calc", _("Open in Calculator"), NULL, NULL);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (do_open_in_calc), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Molecule'><menuitem action='calc'/></menu></popup></ui>", -1, NULL);
		result = true;
	}
	if (m_Bonds.size ()) {
		action = gtk_action_new ("select-align", _("Select alignment item"), NULL, NULL);
		g_signal_connect (action, "activate", G_CALLBACK (do_select_alignment), this);
		g_object_set_data (G_OBJECT (action), "item", object);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Molecule'><menuitem action='select-align'/></menu></popup></ui>", -1, NULL);
		result = true;
	}
	gtk_ui_manager_insert_action_group (uim, group, 0);
	g_object_unref (group);
	return result | Object::BuildContextualMenu (UIManager, object, x, y);
}

void Molecule::SelectAlignmentItem (Object *child)
{
	m_Alignment = (m_Alignment != child)? child: NULL;
	EmitSignal (OnChangedSignal);
}

xmlNodePtr Molecule::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = Object::Save (xml);
	if (!node)
		return NULL;
	if (m_Alignment)
		xmlNewProp (node, (const xmlChar*) "valign", (const xmlChar*) m_Alignment->GetId ());
	return node;
}

bool Molecule::OnSignal (G_GNUC_UNUSED SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	View *view = static_cast < Document * > (GetDocument ())->GetView ();
	gcu::Object *obj;
	std::set < gcu::Object * >::iterator j;
	for (obj = GetFirstLink (j); obj; obj = GetNextLink (j))
		view->Update (obj);
	ResetIndentifiers ();
	return true;
}

void Molecule::OpenCalc ()
{
	list<gcu::Atom*>::iterator ia, enda = m_Atoms.end ();
	ostringstream ofs;
	int nH;
	ofs << "gchemcalc-"API_VERSION" ";
	for (ia = m_Atoms.begin(); ia != enda; ia++) {
		ofs << (*ia)->GetSymbol();
		nH = reinterpret_cast <Atom *> (*ia)->GetAttachedHydrogens ();
		if (nH > 0) {
			ofs << "H" << nH;
		}
	}
	g_spawn_command_line_async (ofs.str ().c_str (), NULL);
}

void Molecule::CheckCrossings (Bond *pBond)
{
	View *pView = reinterpret_cast<Document*> (GetDocument ())->GetView ();
	list<gcu::Bond*>::iterator i, iend = m_Bonds.end ();
	for (i = m_Bonds.begin (); i != iend; i++)
		if (((*i) != pBond) && reinterpret_cast <Bond *> (*i)->IsCrossing (pBond)) {
			pView->Update (pBond);
			pView->Update (*i);
		}
}

std::string Molecule::GetRawFormula () const
{
	ostringstream ofs;

	if (!m_Fragments.size ()) {
		// we do not support fragments at the moment
		map<string, int> elts;
		list<gcu::Atom*>::const_iterator ia, enda = m_Atoms.end ();
		for (ia = m_Atoms.begin(); ia != enda; ia++) {
			if ((*ia)->GetZ () == 0)
				continue;
			elts[(*ia)->GetSymbol ()]++;
			elts["H"] += reinterpret_cast <Atom *> (*ia)->GetAttachedHydrogens ();
		}
		if (elts["C"] > 0) {
			ofs << "C" << elts["C"];
			elts.erase ("C");
		}
		if (elts["H"] > 0) {
			ofs << "H" << elts["H"];
			elts.erase ("H");
		}
		map<string, int>::iterator is, isend = elts.end ();
		for (is = elts.begin (); is != isend; is++)
			ofs << (*is).first << (*is).second;
	}

	return ofs.str ();
}

static void BuildConnectivity (gcu::Atom *atom, set < gcu::Atom * > &ConnectedAtoms)
{
	std::map < gcu::Atom *, gcu::Bond * >::iterator i;
	gcu::Bond *bond;
	ConnectedAtoms.insert (atom);
	for (bond = atom->GetFirstBond (i); bond; bond = atom->GetNextBond (i)) {
		gcu::Atom *end = bond->GetAtom (atom);
		if (ConnectedAtoms.find (end) == ConnectedAtoms.end ())
			BuildConnectivity (end, ConnectedAtoms);
	}
}

void Molecule::OnLoaded ()
{
	// First check if we have only one molecule or split, this might happen with
	// cml (root node being molecule) or malformed files.
	if (m_Atoms.size () > 1) {
		/* FIXME: what should be done with fragments? */
		std::set < gcu::Atom * > ConnectedAtoms;
		std::list < gcu::Atom * >:: iterator i;
		while (1) {
			BuildConnectivity (GetFirstAtom (i), ConnectedAtoms);
			if (m_Atoms.size () + m_Fragments.size () == ConnectedAtoms.size ())
				break;
			// now split the molecule
			Molecule *new_mol = new Molecule (static_cast < Atom * > (GetFirstAtom (i)));
			GetParent ()->AddChild (new_mol);
			// move chiral atoms to new mol if needed
			std::set < gcu::Atom * >::iterator i, iend = ConnectedAtoms.end ();
			for (i = ConnectedAtoms.begin (); i != iend; i++) {
				Atom *atom = static_cast < Atom * > (*i);
				if (m_ChiralAtoms.find (atom) != m_ChiralAtoms.end ()) {
					m_ChiralAtoms.erase (atom);
					new_mol->m_ChiralAtoms.insert (atom);
				}
			}
			// remove new_mol children from there
			std::list < gcu::Atom * >::iterator a, aend = new_mol->m_Atoms.end ();
			std::list < gcu::Bond * >::iterator b, bend = new_mol->m_Bonds.end ();
			std::list < Fragment * >::iterator f, fend = new_mol->m_Fragments.end ();
			for (a = new_mol->m_Atoms.begin (); a != aend; a++)
				m_Atoms.remove (*a);
			for (b = new_mol->m_Bonds.begin (); b != bend; b++)
				m_Bonds.remove (*b);
			for (f = new_mol->m_Fragments.begin (); f != fend; f++)
				m_Fragments.remove (*f);
			// clean connected atoms for the next round
			ConnectedAtoms.clear ();
		}
	}
	UpdateCycles ();
	// now we need to consider atoms with parity (essentially on import)
	std::set < Atom * >::iterator it, end = m_ChiralAtoms.end ();
	std::set < Atom * > done;
	for (it = m_ChiralAtoms.begin (); it != end; it++)
		if ((*it)->UpdateStereoBonds ())
			done.insert (*it);
	end = done.end ();
	// for now, we don't keep them
	for (it = done.begin (); it != end; it++)
		m_ChiralAtoms.erase (*it);
	// check for crossing bonds
	std::list < gcu::Bond * >::const_iterator i;
	Bond const *bond = static_cast < Bond const * > (GetFirstBond (i));
	while (bond) {
		CheckCrossings (const_cast <Bond *> (bond));
		bond = static_cast < Bond const * > (GetNextBond (i));
	}
	GetParent ()->OnLoaded (); // FIXME: shouldn't needed.
}

unsigned Molecule::GetAtomsNumber () const
{
	return m_Atoms.size () + m_Fragments.size ();
}

double Molecule::GetMeanBondLength () const
{
	unsigned n = 0;
	double l = 0;
	std::list < gcu::Bond * >::const_iterator i, end = m_Bonds.end ();
	for (i = m_Bonds.begin (); i != end; i++) {
		n++;
		l += (*i)->Get2DLength ();
	}
	return l / n;
}

bool Molecule::AtomIsChiral (Atom *atom) const {
	std::set < Atom * >::const_iterator it = m_ChiralAtoms.find (atom);
	if (it == m_ChiralAtoms.end ())
		return false;
	return atom->HasStereoBond ();
}

}	//	namespace gcp
