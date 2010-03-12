// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalatom.cc 
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
#include "element.h"
#include "crystalatom.h"
#include "document.h"
#include "element.h"
#include "objprops.h"
#include "xml-utils.h"
#include <GL/glu.h>
#include <cmath>
#include <string>
#include <sstream>

namespace gcu
{

CrystalAtom::CrystalAtom(): Atom()
{
	m_Radius.Z = (unsigned char) GetZ();
	m_Radius.type = GCU_RADIUS_UNKNOWN;
	m_Radius.scale = NULL;
	m_Radius.spin = GCU_N_A_SPIN,
	m_Radius.charge = 0;
	m_Radius.value.value = 0.0;
	m_Radius.value.prec = 0;
	m_Radius.cn = -1;
	m_bCustomColor = false;
	m_fRed = m_fBlue = m_fGreen = 0;
	m_fAlpha = 1;
	m_nCleave = 0;
	m_EffectiveRadiusRatio = 1.;
}

CrystalAtom::~CrystalAtom()
{
}

CrystalAtom::CrystalAtom(int Z, double x, double y, double z): Atom(Z, x, y, z)
{
	m_Radius.Z = (unsigned char) GetZ();
	m_Radius.type = GCU_RADIUS_UNKNOWN;
	m_Radius.scale = NULL;
	m_Radius.spin = GCU_N_A_SPIN,
	m_Radius.charge = 0;
	m_Radius.value.value = 0.0;
	m_Radius.value.prec = 0;
	m_Radius.cn = -1;
	m_fAlpha = 1.;
	if (Z) SetDefaultColor();
	m_nCleave = 0;
	m_EffectiveRadiusRatio = 1.;
}

CrystalAtom::CrystalAtom(CrystalAtom& caAtom): Atom(caAtom)
{
	m_Radius.scale = NULL;
	SetRadius(caAtom.m_Radius);
	m_bCustomColor = caAtom.m_bCustomColor;
	m_fRed = caAtom.m_fRed;
	m_fGreen = caAtom.m_fGreen;
	m_fBlue = caAtom.m_fBlue;
	m_fAlpha = caAtom.m_fAlpha;
	m_EffectiveRadiusRatio = caAtom.m_EffectiveRadiusRatio;
	m_nCleave = 0;
}

CrystalAtom& CrystalAtom::operator=(CrystalAtom& caAtom)
{
	SetZ(caAtom.GetZ());
	double x, y, z;
	caAtom.GetCoords(&x, &y, &z);
	SetCoords(x, y, z);
	SetRadius(caAtom.m_Radius);
	m_bCustomColor = caAtom.m_bCustomColor;
	m_fRed = caAtom.m_fRed ;
	m_fGreen = caAtom.m_fGreen ;
	m_fBlue = caAtom.m_fBlue ;
	m_fAlpha = caAtom.m_fAlpha ;
	m_EffectiveRadiusRatio = caAtom.m_EffectiveRadiusRatio;
	m_nCleave = 0 ;
	SetCharge (caAtom.GetCharge ());
	return *this ;
}

void CrystalAtom::SetColor(float red, float green, float blue, float alpha)
{
	m_bCustomColor = true;
	m_fRed = red ;
	m_fGreen = green ;
	m_fBlue = blue ;
	m_fAlpha = alpha ;
}

void CrystalAtom::SetDefaultColor()
{
	if (!GetZ())
		return;
	m_bCustomColor = false;
	double *Colors = Element::GetElement(GetZ())->GetDefaultColor();
	m_fRed = (float) Colors[0];
	m_fGreen = (float) Colors[1];
	m_fBlue = (float) Colors[2];
}

void CrystalAtom::GetColor(double *red, double *green, double *blue, double *alpha)
{
	*red = m_fRed ;
	*green = m_fGreen ;
	*blue = m_fBlue ;
	*alpha = m_fAlpha ;
}

void CrystalAtom::SetSize(double r)
{
	m_Radius.Z = (unsigned char) GetZ();
	m_Radius.type = GCU_RADIUS_UNKNOWN;
	m_Radius.scale = NULL;
	m_Radius.spin = GCU_N_A_SPIN,
	m_Radius.charge = 0;
	m_Radius.value.value = r;
	m_Radius.value.prec = 0;
	m_Radius.cn = -1;
	m_Radius.value.value = 0.0;
	m_Radius.value.prec = 0;
}


double CrystalAtom::GetSize()
{
	return m_Radius.value.value;
}

bool CrystalAtom::operator==(CrystalAtom& caAtom)
{
	// original version only checked x, y and z, we might need a SameCoords method.
	return ((fabs (x() - caAtom.x()) < PREC) &&
			(fabs (y() - caAtom.y()) < PREC) &&
			(fabs (z() - caAtom.z()) < PREC) &&
			(GetZ () == caAtom.GetZ ()) &&
			(GetCharge () == caAtom.GetCharge ())) ;
}

double CrystalAtom::ScalProd(int h, int k, int l)
{
	return x() * h + y() * k + z() * l ;
}

double CrystalAtom::Distance(double dx, double dy, double dz, bool bFixed)
{
	if ((m_nCleave > 0) && ! bFixed) return 0. ;
	dx -= x() ;
	dy -= y() ;
	dz -= z() ;
	return sqrt(dx * dx + dy * dy + dz * dz) + m_Radius.value.value ;
}

void CrystalAtom::NetToCartesian(double a, double b, double c, double alpha, double beta, double gamma)
{
	double dx = x() * a ;
	double dy = y() * b ;
	double dz = z() * c ;
	SetCoords(dx * sqrt(1-square(cos(beta)) - square((cos(gamma) - cos(beta)*cos(alpha))/sin(alpha))),
		dx * (cos(gamma) - cos(beta)*cos(alpha))/sin(alpha) + dy * sin(alpha),
		(dx * cos(beta) + dy * cos(alpha) + dz));
}

bool CrystalAtom::SaveNode (xmlDocPtr xml, xmlNodePtr node) const
{
	if (!WriteRadius (xml, node, m_Radius))
		return false;

	if (m_bCustomColor && !WriteColor (xml, node, NULL, m_fRed, m_fGreen, m_fBlue, m_fAlpha))
		return false;

	return true;
}

bool CrystalAtom::LoadNode(xmlNodePtr node)
{
	xmlNodePtr child = FindNodeByNameAndId(node, "color");
	if (!child) SetDefaultColor();
	else
	{
		if (!ReadColor(node, NULL, &m_fRed, &m_fGreen, &m_fBlue, &m_fAlpha)) return false;
		m_bCustomColor = true;
	}
	child = FindNodeByNameAndId(node, "radius");
	if (!child) return false;
	m_Radius.Z = GetZ();
	bool result = ReadRadius(child, m_Radius);
	return result;
}

void CrystalAtom::SetRadius(const GcuAtomicRadius& r)
{
	m_Radius.type = r.type;
	m_Radius.value = r.value;
	m_Radius.charge = r.charge;
	m_Radius.scale = r.scale;
	m_Radius.cn = r.cn;	//coordination number: -1: unspecified
	m_Radius.spin = r.spin;
}

bool CrystalAtom::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_POS2D:
	case GCU_PROP_X:
	case GCU_PROP_Y:
	case GCU_PROP_Z:
		break;
	case GCU_PROP_XFRACT:
		m_x = strtod (value, NULL) * GetDocument ()->GetScale ();
		break;
	case GCU_PROP_YFRACT:
		m_y = strtod (value, NULL) * GetDocument ()->GetScale ();
		break;
	case GCU_PROP_ZFRACT:
		m_z = strtod (value, NULL) * GetDocument ()->GetScale ();
		break;
	default:
		return Atom::SetProperty (property, value);
	}
	return  true;
}

std::string CrystalAtom::GetProperty (unsigned property) const
{
	std::ostringstream res;
	switch (property) {
	case GCU_PROP_ATOM_CHARGE:
		res << static_cast <int> (m_Radius.charge);
		break;
	case GCU_PROP_XFRACT:
		res << x ();
		break;
	case GCU_PROP_YFRACT:
		res << y ();
		break;
	case GCU_PROP_ZFRACT:
		res << z ();
		break;
	default:
		return Atom::GetProperty (property);
	}
	return res.str ();
};

}	//	namespace gcu
