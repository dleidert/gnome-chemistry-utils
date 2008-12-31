// -*- C++ -*-

/* 
 * GChemPaint library
 * molecule.cc 
 *
 * Copyright (C) 2001-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "atom.h"
#include "bond.h"
#include "document.h"
#include "molecule.h"
#include "stringdlg.h"
#include "tool.h"
#include "view.h"
#include <gcu/chain.h>
#include <glib/gi18n-lib.h>
#include <unistd.h>
#include <openbabel/obconversion.h>
#include <clocale>
#include <cmath>
#include <cstring>
#include <map>
#include <set>

using namespace gcu;
using namespace OpenBabel;
using namespace std;

namespace gcp {

static void do_export_to_ghemical (Molecule* pMol)
{
	pMol->ExportToGhemical ();
}

static void do_select_alignment (GObject *action, Molecule* pMol)
{
	Object *object = (Object*) g_object_get_data (action, "item");
	pMol->SelectAlignmentItem (object);
}

static void do_build_inchi (Molecule* pMol)
{
	pMol->ShowInChI ();
}

static void do_build_smiles (Molecule* pMol)
{
	pMol->BuildSMILES ();
}

static void do_show_webbook (Molecule* pMol)
{
	pMol->ShowWebBase ("http://webbook.nist.gov/cgi/cbook.cgi?Name=", "&Units=SI");
}

static void do_show_pubchem (Molecule* pMol)
{
	pMol->ShowWebBase ("http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?CMD=search&DB=pccompound&term=\"", "\"");
}

static void do_open_in_calc (Molecule* pMol)
{
	pMol->OpenCalc ();
}

Molecule::Molecule (TypeId Type): gcu::Molecule (Type)
{
	m_Alignment = NULL;
	m_Changed = true;
	m_IsResidue = false;
}

Molecule::Molecule (Atom* pAtom): gcu::Molecule  (pAtom)
{
	m_Alignment = NULL;
	m_Changed = true;
	m_IsResidue = false;
}

Molecule::~Molecule ()
{
}

/*void Molecule::Add (GtkWidget* w) const
{
	std::list<gcu::Atom*>::const_iterator i, iend = m_Atoms.end ();
	for (i = m_Atoms.begin (); i != iend; i++)
		(*i)->Add (w);
	std::list<Fragment*>::const_iterator j, jend = m_Fragments.end ();
	for (j = m_Fragments.begin (); j != jend; j++)
		(*j)->Add (w);
	std::list<gcu::Bond*>::const_iterator k, kend = m_Bonds.end ();
	for (k = m_Bonds.begin (); k != kend; k++)
		(*k)->Add (w);
}*/

void Molecule::AddChild (Object* object)
{
	switch (object->GetType ()) {
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
	if (!m_Atoms.empty ()) {
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
	}
	buf = (char*) xmlGetProp (node, (const xmlChar*) "valign");
	if (buf) {
		m_Alignment = GetDescendant (buf);
		xmlFree (buf);
		if (!m_Alignment)
			return false;
	}
	m_Changed = true;
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

Object* Molecule::GetAtomAt (double x, double y, double z)
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

bool Molecule::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	if (m_IsResidue)
		return false;
	GtkActionGroup *group = gtk_action_group_new ("molecule");
	GtkAction *action;
	action = gtk_action_new ("Molecule", _("Molecule"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	bool result = false;
	if (!m_Fragments.size ()) {
		if (((Document*) GetDocument ())->GetApplication ()->HaveGhemical ()) {
			action = gtk_action_new ("ghemical", _("Export molecule to Ghemical"), NULL, NULL);
			g_signal_connect_swapped (action, "activate", G_CALLBACK (do_export_to_ghemical), this);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Molecule'><menuitem action='ghemical'/></menu></popup></ui>", -1, NULL);
		}
		if (((Document*) GetDocument ())->GetApplication ()->HaveInChI ()) {
			action = gtk_action_new ("inchi", _("Generate InChI"), NULL, NULL);
			g_signal_connect_swapped (action, "activate", G_CALLBACK (do_build_inchi), this);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Molecule'><menuitem action='inchi'/></menu></popup></ui>", -1, NULL);
			action = gtk_action_new ("webbook", _("NIST WebBook page for this molecule"), NULL, NULL);
			g_signal_connect_swapped (action, "activate", G_CALLBACK (do_show_webbook), this);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Molecule'><menuitem action='webbook'/></menu></popup></ui>", -1, NULL);
			action = gtk_action_new ("pubchem", _("PubChem page for this molecule"), NULL, NULL);
			g_signal_connect_swapped (action, "activate", G_CALLBACK (do_show_pubchem), this);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Molecule'><menuitem action='pubchem'/></menu></popup></ui>", -1, NULL);
		}
		action = gtk_action_new ("smiles", _("Generate SMILES"), NULL, NULL);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (do_build_smiles), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Molecule'><menuitem action='smiles'/></menu></popup></ui>", -1, NULL);
		action = gtk_action_new ("calc", _("Open in Calculator"), NULL, NULL);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (do_open_in_calc), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Molecule'><menuitem action='calc'/></menu></popup></ui>", -1, NULL);
		result = true;
	}
	if (m_Bonds.size ()) {
		action = gtk_action_new ("select-align", _("Select alignment item"), NULL, NULL);
		g_signal_connect (action, "activate", G_CALLBACK (do_select_alignment), this);
		g_object_set_data (G_OBJECT (action), "item", object);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Molecule'><menuitem action='select-align'/></menu></popup></ui>", -1, NULL);
		result = true;
	}
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
	return result | Object::BuildContextualMenu (UIManager, object, x, y);
}

void Molecule::ExportToGhemical ()
{
	OBMol Mol;
	OBConversion Conv;
	OBFormat* pOutFormat = Conv.FindFormat ("gpr");
	Conv.SetInAndOutFormats (pOutFormat, pOutFormat);
	BuildOBMol (Mol);
	char *tmpname = g_strdup ("/tmp/2gprXXXXXX");
	int f = g_mkstemp (tmpname);
	gchar *old_num_locale;
	close (f);
	ofstream ofs;
	ofs.open(tmpname);
	if (!ofs) throw (int) 1;
	old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	Conv.Write (&Mol, &ofs);
	setlocale(LC_NUMERIC, old_num_locale);
	g_free(old_num_locale);
	ofs.close();
	char *command_line = g_strconcat ("ghemical -f ", tmpname, NULL);	
	g_free (tmpname);
	g_spawn_command_line_async (command_line, NULL);
	g_free (command_line);
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

void Molecule::BuildOBMol (OBMol &Mol)
{
	double xav = 0., yav = 0., zf;
	unsigned n = m_Atoms.size ();
	map<string, unsigned> AtomTable;
	list<gcu::Atom*>::iterator ia, enda = m_Atoms.end ();
	list<gcu::Bond*> BondList;
	double x, y, z;
	for (ia = m_Atoms.begin (); ia != enda; ia++) {
		(*ia)->GetCoords(&x, &y, &z);
		xav += x;
		yav += y;
	}
	xav /= n;
	yav /= n;
	Atom* pgAtom;
	OBAtom obAtom;
	unsigned index = 1;
	map<gcu::Atom*, gcu::Bond*>::iterator i;
	Bond *pBond;
	Mol.BeginModify ();
	Mol.ReserveAtoms (n);
	for (ia = m_Atoms.begin (); ia != enda; ia++) {
		pgAtom =  reinterpret_cast <Atom *> (*ia);
		AtomTable [pgAtom->GetId ()] = index;
		obAtom.SetIdx (index++);
		obAtom.SetAtomicNum (pgAtom->GetZ());
		pgAtom->GetCoords (&x, &y, &z);
		// Scans the atom bonds and change z to try conservation of stereochemistry
		pBond = (Bond*) pgAtom->GetFirstBond (i);
		while (pBond) {
			zf = (pBond->GetAtom (0) == pgAtom)? 1.: -1.;
			switch (pBond->GetType ()) {
			case UpBondType:
				z += zf * 50.;
				break;
			case DownBondType:
				z -= zf * 50.;
				break;
			default:
				break;
			}
			pBond = (Bond*) pgAtom->GetNextBond (i);
		}
		obAtom.SetVector ((xav - x) / 100, (yav - y) / 100, z / 100);
		Mol.AddAtom (obAtom);
		obAtom.Clear ();
	}
	list<gcu::Bond*>::iterator j, endb = m_Bonds.end ();
	int start, end, order;
	for (j = m_Bonds.begin (); j != endb; j++)
	{
		order = (*j)->GetOrder ();
		start = AtomTable[(*j)->GetAtom (0)->GetId ()];
		end = AtomTable[(*j)->GetAtom (1)->GetId ()];
		Mol.AddBond(start, end, order, 0);
	}
	Mol.EndModify ();
}

void Molecule::BuildOBMol2D (OBMol &Mol)
{
	double xav = 0., yav = 0.;
	unsigned n = m_Atoms.size ();
	map<string, unsigned> AtomTable;
	list<gcu::Atom*>::iterator ia, enda = m_Atoms.end ();
	list<gcu::Bond*> BondList;
	double x, y, z;
	for (ia = m_Atoms.begin (); ia != enda; ia++) {
		(*ia)->GetCoords (&x, &y, &z);
		xav += x;
		yav += y;
	}
	xav /= n;
	yav /= n;
	Atom* pgAtom;
	OBAtom obAtom;
	unsigned index = 1;
	map<gcu::Atom*, gcu::Bond*>::iterator i;
	Mol.BeginModify ();
	Mol.ReserveAtoms (n);
	Mol.SetDimension (2);
	for (ia = m_Atoms.begin (); ia != enda; ia++) {
		pgAtom =  reinterpret_cast <Atom *> (*ia);
		AtomTable [pgAtom->GetId ()] = index;
		obAtom.SetIdx (index++);
		obAtom.SetAtomicNum (pgAtom->GetZ());
		pgAtom->GetCoords (&x, &y, &z);
		// Scans the atom bonds and change z to try conservation of stereochemistry
		obAtom.SetVector ((x - xav) / 100, (yav - y) / 100, 0.);
		Mol.AddAtom (obAtom);
		obAtom.Clear ();
	}
	list<gcu::Bond*>::iterator j, endb = m_Bonds.end ();
	int start, end, order, flag;
	for (j = m_Bonds.begin (); j != endb; j++) {
		order = (*j)->GetOrder ();
		start = AtomTable[(*j)->GetAtom (0)->GetId ()];
		end = AtomTable[(*j)->GetAtom (1)->GetId ()];
		switch (reinterpret_cast <Bond *> (*j)->GetType ()) {
		case UpBondType:
			flag = OB_WEDGE_BOND;
			break;
		case DownBondType:
			flag = OB_HASH_BOND;
			break;
		default:
			flag = 0;
		}
		Mol.AddBond (start, end, order, flag);
	}
	Mol.EndModify ();
}

void Molecule::BuildInChI ()
{
	OBMol Mol;
	OBConversion Conv;
	BuildOBMol2D (Mol);
	OBFormat *pInChIFormat = Conv.FindFormat ("inchi"), *pMolFormat = Conv.FindFormat ("mol");
	if (pInChIFormat) {
		Conv.SetInAndOutFormats (pMolFormat, pInChIFormat);
		Conv.SetOptions ("xt", OpenBabel::OBConversion::OUTOPTIONS);
		ostringstream ofs;
		char *old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		Conv.Write (&Mol, &ofs);
		setlocale (LC_NUMERIC, old_num_locale);
		g_free (old_num_locale);
		// remove "INCHI=" and the new line char at the end
		m_InChI = ofs.str ().substr (0, ofs.str ().length () - 2);
	} else {
		Conv.SetInAndOutFormats (pMolFormat, pMolFormat);
		char *tmpname = g_strdup ("/tmp/inchiXXXXXX");
		int f = g_mkstemp (tmpname);
		close (f);
		ofstream ofs;
		ofs.open (tmpname);
		char *old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		Conv.Write (&Mol, &ofs);
		setlocale (LC_NUMERIC, old_num_locale);
		ofs.close ();
		// calling main_inchi -STDIO -AuxNone -NoLabels
		char *command = g_strdup_printf ("main_inchi %s -STDIO -AuxNone -NoLabels", tmpname);
		char *output, *errors;
		g_spawn_command_line_sync (command, &output, &errors, NULL, NULL);
		if (output) {
			// remove the new line char at the end
			output[strlen (output) - 1] = 0;
			m_InChI = output + 6;
			g_free (output);
		}
		if (errors)
			g_free (errors);
		g_free (command);
		g_free (old_num_locale);
		remove (tmpname);
		g_free (tmpname);
	}
	m_Changed = false;
}

void Molecule::BuildSMILES ()
{
	OBMol Mol;
	OBConversion Conv;
	OBFormat* pOutFormat = Conv.FindFormat ("smi");
	Conv.SetInAndOutFormats (pOutFormat, pOutFormat);
	BuildOBMol2D (Mol);
	ostringstream ofs;
	char *old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	Conv.Write (&Mol, &ofs);
	setlocale (LC_NUMERIC, old_num_locale);
	//TODO: do something with the string
	g_free (old_num_locale);
	string str = ofs.str ().substr (0, ofs.str ().length () - 2);
	new StringDlg (reinterpret_cast<Document *>(GetDocument ()), str, StringDlg::SMILES);
}

void Molecule::ShowInChI ()
{
	if (m_Changed)
		BuildInChI ();
	new StringDlg (reinterpret_cast<Document *>(GetDocument ()), m_InChI, StringDlg::INCHI);
}

void Molecule::ShowWebBase (char const* uri_start, char const *uri_end)
{
	if (m_Changed)
		BuildInChI ();
	if (m_InChI.length () == 0)
		return; //should emit at least a warning
	string::size_type t;
	while ((t = m_InChI.find ('+')) != string::npos)
		m_InChI.replace (t, 1, "%2b");
	string uri = string (uri_start) + m_InChI + uri_end;
	((Document*) GetDocument ())->GetApplication ()->ShowURI (uri);
}

bool Molecule::OnSignal (SignalId Signal, Object *Child)
{
	m_Changed = true;
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

char const *Molecule::GetInChI ()
{
	if (m_Changed)
		BuildInChI ();
	return m_InChI.c_str ();
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

void Molecule::OnLoaded ()
{
	UpdateCycles ();
}

unsigned Molecule::GetAtomsNumber () const
{
	return m_Atoms.size () + m_Fragments.size ();
}

}	//	namespace gcp
