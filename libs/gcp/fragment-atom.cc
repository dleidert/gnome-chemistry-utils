// -*- C++ -*-

/*
 * GChemPaint library
 * fragment-atom.cc
 *
 * Copyright (C) 2003-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "bond.h"
#include "fragment-atom.h"
#include "fragment.h"
#include "molecule.h"
#include "view.h"
#include <gccv/text.h>
#include <gcu/element.h>
#include <cstring>
#include <map>

using namespace gcu;
using namespace std;

namespace gcp {

FragmentAtom::FragmentAtom (): Atom ()
{
	SetId ("a1");
	SetShowSymbol (true);
}

FragmentAtom::FragmentAtom (Fragment *fragment, int Z): Atom ()
{
	m_Fragment = fragment;
	SetZ (Z);
	SetId ("a1");
	SetShowSymbol (true);
	BuildSymbolGeometry (0,0,0);
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
	BuildSymbolGeometry (0,0,0);
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
void FragmentAtom::AddItem ()
{
}

/*!
Overrided to avoid Atom::Update execution. Just call fragment Update method.
*/
void FragmentAtom::UpdateItem ()
{
	m_Fragment->UpdateItem ();
}


/*!
Overrided to avoid Atom::SetSelected execution. Just call fragment SetSelected method.
*/
void FragmentAtom::SetSelected (int state)
{
	m_Fragment->SetSelected (state);
}

xmlNodePtr FragmentAtom::Save (xmlDocPtr xml) const
{
	xmlNodePtr node;
	char buf[16];
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "atom", NULL);
	if (!node)
		return NULL;
	SaveId (node);

	strncpy (buf, GetSymbol (), sizeof (buf));
	xmlNodeSetContent (node, (xmlChar*) buf);

	SaveChildren (xml, node);
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
				case POSITION_NE:
					buf = "ne";
					break;
				case POSITION_NW:
					buf = "nw";
					break;
				case POSITION_N:
					buf = "n";
					break;
				case POSITION_SE:
					buf = "se";
					break;
				case POSITION_SW:
					buf = "sw";
					break;
				case POSITION_S:
					buf= "s";
					break;
				case POSITION_E:
					buf = "e";
					break;
				case POSITION_W:
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
	for (xmlNodePtr child = node->children; child; child = child->next)
		if (!strcmp (reinterpret_cast <char const *> (child->name), "orbital")) {
			Object *obj = CreateObject ("orbital", this);
			if (!obj->Load (child))
				return false;
		} else
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
				ChargePos = POSITION_NE;
				Angle = M_PI / 4.;
			} else if (! strcmp (buf, "nw")) {
				ChargePos = POSITION_NW;
				Angle = 3. * M_PI / 4.;
			} else if (! strcmp (buf, "n")) {
				ChargePos = POSITION_N;
				Angle = M_PI / 2.;
			} else if (! strcmp (buf, "se")) {
				ChargePos = POSITION_SE;
				Angle = 7. * M_PI / 4;
			} else if (! strcmp (buf, "sw")) {
				ChargePos = POSITION_SW;
				Angle = 5. * M_PI / 4;
			} else if (! strcmp (buf, "s")) {
				ChargePos = POSITION_S;
				Angle = 3 * M_PI / 2.;
			} else if (! strcmp (buf, "e")) {
				ChargePos = POSITION_E;
				Angle = 0.;
			} else if (! strcmp (buf, "w")) {
				ChargePos = POSITION_W;
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

gccv::Anchor FragmentAtom::GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y)
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
	m_Fragment->Update ();
}

bool FragmentAtom::Match (G_GNUC_UNUSED gcu::Atom *atom, G_GNUC_UNUSED AtomMatchState &state)
{
	return false; // not supported at the moment
}

void FragmentAtom::DoBuildSymbolGeometry (View *pView)
{
	// Building atom geometry if necessary
	double ascent;
	char const *symbol = GetSymbol ();
	if (!symbol)
		return;
	PangoContext* pc = gccv::Text::GetContext ();
	PangoLayout *layout = pango_layout_new (pc);
	pango_layout_set_font_description (layout, pView->GetPangoFontDesc ());
	PangoRectangle rect;
	if (m_CHeight == 0.) {
		pango_layout_set_text (layout, "C", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		m_CHeight =  double (rect.height / PANGO_SCALE) / 2.0;
	}
	pango_layout_set_text (layout, symbol, -1);
	PangoLayoutIter* iter = pango_layout_get_iter (layout);
	ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
	pango_layout_iter_free (iter);
	pango_layout_get_extents (layout, &rect, NULL);

	BuildSymbolGeometry ((double) rect.width / PANGO_SCALE, (double) rect.height / PANGO_SCALE, ascent - (double) rect.y / PANGO_SCALE - m_CHeight);
	g_object_unref (G_OBJECT (layout));
	map < gcu::Bondable *, gcu::Bond * >::iterator i;
	Bond *bond = (Bond*) GetFirstBond (i);
	if (bond)
		bond->SetDirty ();
}

}	//	namespace gcp
