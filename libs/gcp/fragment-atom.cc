// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment-atom.cc 
 *
 * Copyright (C) 2003-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "fragment-atom.h"
#include "fragment.h"
#include "molecule.h"
#include <gcu/element.h>

namespace gcp {

FragmentAtom::FragmentAtom (): Atom ()
{
	SetId ("a1");
}

FragmentAtom::FragmentAtom (Fragment *fragment, int Z): Atom ()
{
	m_Fragment = fragment;
	SetZ (Z);
	SetId ("a1");
}

FragmentAtom::~FragmentAtom () {}

void FragmentAtom::SetZ (int Z)
{
	static bool setting = false;
	if (setting)
		return;
	setting = true;
	gcu::Atom::SetZ (Z);
	if (Z != 0)
		m_Fragment->OnChangeAtom ();
	setting = false;
}

/*!
Only accept a new bond if none exists. So only one bond.
*/
bool FragmentAtom::AcceptNewBonds (int nb)
{
	return (nb > 1)? false: GetBondsNumber () == 0;
}

/*!
Overrided to avoid Atom::Add execution. Don't do anything.
*/
void FragmentAtom::Add (GtkWidget* w)
{
}

/*!
Overrided to avoid Atom::Update execution. Just call fragment Update method.
*/
void FragmentAtom::Update (GtkWidget* w)
{
	m_Fragment->Update (w);
}


/*!
Overrided to avoid Atom::SetSelected execution. Just call fragment SetSelected method.
*/
void FragmentAtom::SetSelected (GtkWidget* w, int state)
{
	m_Fragment->SetSelected (w, state);
}

xmlNodePtr FragmentAtom::Save (xmlDocPtr xml)
{
	xmlNodePtr node;
	gchar buf[16];
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "atom", NULL);
	if (!node)
		return NULL;
	SaveId (node);

	strncpy (buf, GetSymbol (), sizeof (buf));
	xmlNodeSetContent (node, (xmlChar*) buf);
	
	if (m_Charge) {
		snprintf (buf, sizeof (buf), "%d", m_Charge);
		xmlNewProp (node, (xmlChar*) "charge", (xmlChar*) buf);
		double Angle, Dist;
		unsigned char ChargePos = Atom::GetChargePosition (&Angle, &Dist);
		if (ChargePos != 0xff) {
			char *buf;
			if (ChargePos) {
				char const *buf;
				switch (ChargePos) {
				case CHARGE_NE:
					buf = "ne";
					break;
				case CHARGE_NW:
					buf = "nw";
					break;
				case CHARGE_N:
					buf = "n";
					break;
				case CHARGE_SE:
					buf = "se";
					break;
				case CHARGE_SW:
					buf = "sw";
					break;
				case CHARGE_S:
					buf= "s";
					break;
				case CHARGE_E:
					buf = "e";
					break;
				case CHARGE_W:
					buf = "w";
					break;
				default:
					buf = "def"; // should not occur
				}
				xmlNewProp (node, (xmlChar*) "charge-position", (xmlChar*) buf);
			} else {
				buf = g_strdup_printf ("%g", Angle * 180. / M_PI);
				xmlNewProp (node, (xmlChar*) "charge-angle", (xmlChar*) buf);
				g_free (buf);
			}
			if (Dist != 0.) {
				buf = g_strdup_printf ("%g", Dist);
				xmlNewProp (node, (xmlChar*) "charge-dist", (xmlChar*) buf);
				g_free (buf);
			}
		}
	}
	return node;
}

bool FragmentAtom::Load (xmlNodePtr node)
{
	char* buf;
	unsigned char ChargePos = 0xff;
	double Angle = 0., Dist = 0.;
	buf = (char*) xmlGetProp (node, (xmlChar*) "id");
	if (buf) {
		SetId (buf);
		xmlFree (buf);
	}
	buf = (char*) xmlNodeGetContent (node);
	if (buf) {
		m_Z = Element::Z (buf);
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "charge");
	m_Charge = (buf)? (char) atoi (buf): 0;
	if (buf)
		xmlFree (buf);
	if (m_Charge) {
		char *buf = (char*) xmlGetProp (node, (xmlChar*) "charge-position");
		if (buf) {
			if (! strcmp (buf, "ne")) {
				ChargePos = CHARGE_NE;
				Angle = M_PI / 4.;
			} else if (! strcmp (buf, "nw")) {
				ChargePos = CHARGE_NW;
				Angle = 3. * M_PI / 4.;
			} else if (! strcmp (buf, "n")) {
				ChargePos = CHARGE_N;
				Angle = M_PI / 2.;
			} else if (! strcmp (buf, "se")) {
				ChargePos = CHARGE_SE;
				Angle = 7. * M_PI / 4;
			} else if (! strcmp (buf, "sw")) {
				ChargePos = CHARGE_SW;
				Angle = 5. * M_PI / 4;
			} else if (! strcmp (buf, "s")) {
				ChargePos = CHARGE_S;
				Angle = 3 * M_PI / 2.;
			} else if (! strcmp (buf, "e")) {
				ChargePos = CHARGE_E;
				Angle = 0.;
			} else if (! strcmp (buf, "w")) {
				ChargePos = CHARGE_W;
				Angle = M_PI;
			}
			xmlFree (buf);
		} else {
			buf = (char*) xmlGetProp (node, (xmlChar*)"charge-angle");
			if (buf) {
				sscanf (buf, "%lg", &Angle);
				Angle *= M_PI / 180.;
				xmlFree (buf);
				ChargePos = 0;
			}
		}
		buf = (char*) xmlGetProp (node, (xmlChar*)"charge-dist");
		if (buf) {
			sscanf (buf, "%lg", &Dist);
			xmlFree (buf);
		}
		SetChargePosition (ChargePos, ChargePos == 0xff, Angle, Dist);
	}
	return true;
}

void FragmentAtom::AddToMolecule (Molecule* Mol)
{
	Mol->AddFragment (m_Fragment);
}

int FragmentAtom::GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y)
{
	return m_Fragment->GetChargePosition (this, Pos, Angle, x, y);
}


int FragmentAtom::GetAvailablePosition (double& x, double& y)
{
	return m_Fragment->GetAvailablePosition (x, y);
}
	
bool FragmentAtom::GetPosition (double angle, double& x, double& y)
{
	return m_Fragment->GetPosition (angle, x, y);
}
	
bool FragmentAtom::AcceptCharge (int charge)
{
	return (charge >= -1 && charge <= 1);
}

void FragmentAtom::Update ()
{
}

}	//	namespace gcp