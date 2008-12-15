// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text-tag.cc
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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
 #include "text-tag.h"
 
namespace gccv {

////////////////////////////////////////////////////////////////////////////////
// Base tag class

TextTag::TextTag (Tag tag):
	m_Tag (tag),
	m_StartIndex (0),
	m_EndIndex (0)
{
}

TextTag::~TextTag ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Font family tag class

FamilyTextTag::FamilyTextTag (char const *family):
	TextTag (Family),
	m_Family (family)
{
}

FamilyTextTag::~FamilyTextTag ()
{
}

void FamilyTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
}

////////////////////////////////////////////////////////////////////////////////
// Font size tag class

SizeTextTag::SizeTextTag (double size):
	TextTag (Size),
	m_Size (size)
{
}

SizeTextTag::~SizeTextTag ()
{
}

void SizeTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
}

////////////////////////////////////////////////////////////////////////////////
// Subscript tag class

SubscriptTextTag::SubscriptTextTag ():
	TextTag (Subscript)
{
}

SubscriptTextTag::~SubscriptTextTag ()
{
}

void SubscriptTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
}

}
