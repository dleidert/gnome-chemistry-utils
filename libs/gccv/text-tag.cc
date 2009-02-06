// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text-tag.cc
 *
 * Copyright (C) 2008-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <list>
#include <map>

namespace gccv {

////////////////////////////////////////////////////////////////////////////////
// static helper callbacks

static gboolean cb_splice_true (G_GNUC_UNUSED PangoAttribute *attr, G_GNUC_UNUSED gpointer data)
{
	return TRUE;
}

	
////////////////////////////////////////////////////////////////////////////////
// Base tag class

TextTag::TextTag (Tag tag, TagPriority priority):
	m_Tag (tag),
	m_Priority (priority),
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

SubscriptTextTag::SubscriptTextTag (double size):
	TextTag (Subscript, TagPriorityLast),
	m_Size (size)
{
}

SubscriptTextTag::~SubscriptTextTag ()
{
}

struct subscript_data {
	guint start, end;
	std::list <PangoAttribute *>extra;
	std::map <unsigned, int> sizes, rises;
};

static gboolean
subscript_filter (PangoAttribute *attr, gpointer _data)
{
	struct subscript_data *data = static_cast <struct subscript_data *> (_data);

	if (attr->end_index <= data->start || attr->start_index >= data->end)
		return false;
	switch (attr->klass->type) {
	case PANGO_ATTR_SIZE:
		data->sizes[attr->start_index] = reinterpret_cast <PangoAttrSize *> (attr)->size;
		break;
	case PANGO_ATTR_RISE:
		data->rises[attr->start_index] = reinterpret_cast <PangoAttrInt *> (attr)->value;
		break;
	default:
		return false;
	}
	if (attr->end_index > data->end || attr->start_index < data->start) {
		// add a new attribute
		PangoAttribute *new_attr = pango_attribute_copy (attr);
		new_attr->start_index = data->end;
		new_attr->end_index = attr->end_index;
		attr->end_index = data->start;
		data->extra.push_back (new_attr);
	} else if (attr->start_index < data->start)
		attr->end_index = data->start;
	else
		attr->start_index = data->end;
	return false;
}

void SubscriptTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	struct subscript_data data;
	data.start = start;
	data.end = end;
	data.sizes[start] = m_Size * PANGO_SCALE;
	data.rises[start] = 0.;
	pango_attr_list_filter (l, subscript_filter, NULL);
	// Build and apply the new attributes
	std::map <unsigned, int>::iterator i, j, iend = data.sizes.end (), jend = data.rises.end (), nexti, nextj;
	unsigned cur_start = start, cur_end;
	j =  data.rises.begin ();
	for (i =  data.sizes.begin (); i != iend; i++) {
		nexti = i;
		nexti++;
		for (; j!= jend && (*j).first < ((nexti == iend)? end: (*nexti).first); j++) {
			nextj = j;
			nextj++;
			cur_end = MIN (((nexti == iend)? end: (*nexti).first), ((nextj == jend)? end: (*nextj).first));
			PangoAttribute *attr = pango_attr_size_new ((*i).second * 2 / 3);
			attr->start_index = cur_start;
			attr->end_index = cur_end;
			pango_attr_list_insert (l, attr);
			attr = pango_attr_rise_new ((*j).second - (*i).second / 3);
			attr->start_index = cur_start;
			attr->end_index = cur_end;
			pango_attr_list_insert (l, attr);
			cur_start = cur_end;
		}
		//WARNING: might be buggy
	}
	// Apply the extra attributes.
	std::list <PangoAttribute *>::iterator k, kend = data.extra.end ();
	for (k = data.extra.begin (); k != kend; k++)
		pango_attr_list_insert (l, *k);
}

}
