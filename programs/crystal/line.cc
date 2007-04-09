// -*- C++ -*-

/* 
 * Gnome Crystal
 * line.cc 
 *
 * Copyright (C) 2000-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "gcrystal.h"
#include "line.h"
#include <glib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>

gcLine::gcLine(): CrystalLine()
{
}

gcLine::~gcLine()
{
}


bool gcLine::LoadOld(xmlNodePtr node, unsigned version)
{
	char *txt;
	txt = (char*)xmlGetProp(node, (xmlChar*)"type");
	if (txt)
	{
		int i = 0;
		while (strcmp(txt, LineTypeName[i]) && (i < 5)) i++;
		xmlFree(txt);
		if (i < 5) m_nType = (CrystalLineType)i;
		else return false;
	}
	else return false;
	xmlNodePtr child = node->children;
	while(child)
	{
		if (!strcmp((gchar*)child->name, "position"))
		{
			txt = (char*)xmlNodeGetContent(child);
			if (txt)
			{
				sscanf(txt, "%lg %lg %lg %lg %lg %lg", &m_dx, &m_dy, &m_dz, &m_dx2, &m_dy2, &m_dz2);
				xmlFree(txt);
			}
		}
		else if (!strcmp((gchar*)child->name, "color"))
		{
			txt = (char*)xmlNodeGetContent(child);
			if (txt)
			{
				if (version < 0x200) sscanf(txt, "%g %g %g %g", &m_fBlue, &m_fRed, &m_fGreen, &m_fAlpha);
				else sscanf(txt, "%g %g %g %g", &m_fRed, &m_fGreen, &m_fBlue, &m_fAlpha);
				xmlFree(txt);
			}
		}
		else if (!strcmp((gchar*)child->name, "radius"))
		{
			txt = (char*)xmlNodeGetContent(child);
			if (txt)
			{
				sscanf(txt, "%lg", &m_dr);
				xmlFree(txt);
			}
		}
		child = child->next;
	}
	if (m_dr == 0) return false;
	return true;
}
