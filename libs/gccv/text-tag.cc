// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/text-tag.cc
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "text-tag.h"
#include <list>
#include <map>

namespace gccv {

////////////////////////////////////////////////////////////////////////////////
// Base tag class

TextTag::TextTag (Tag tag, TagPriority priority):
	m_Tag (tag),
	m_Priority (priority),
	m_StartIndex (0),
	m_EndIndex (0),
	m_Stacked (false),
	m_NewLine (false)
{
}

TextTag::~TextTag ()
{
}

Tag TextTag::MaxTag = gccv::MaxTag;

Tag TextTag::RegisterTagType ()
{
	Tag result = MaxTag;
	MaxTag = static_cast <Tag> (MaxTag + 1);
	return result;
}

bool TextTag::Order (TextTag *first, TextTag *last)
{
	if (first->GetStartIndex () < last->GetStartIndex ())
		return true;
	else if (first->GetStartIndex () > last->GetStartIndex ())
		return false;
	if (first->GetEndIndex () > last->GetEndIndex ())
		return true;
	else if (first->GetEndIndex () < last->GetEndIndex ())
		return false;
	return first->GetTag () < last->GetTag ();
}

TextTag *TextTag::Restrict (TextTag *tag)
{
	if (tag->GetTag () == GetTag () && tag->GetEndIndex () > GetStartIndex () && tag->GetStartIndex () < GetEndIndex ()) {
		if (*tag == *this) {
			if (m_StartIndex > tag->GetStartIndex ())
				m_StartIndex = tag->GetStartIndex ();
			if (m_EndIndex < tag->GetEndIndex ())
				m_EndIndex = tag->GetEndIndex ();
			tag->SetEndIndex (m_StartIndex); // makes tag invalid
			return NULL;
		}
		if (tag->GetEndIndex () > GetEndIndex ()) {
			if (tag->GetStartIndex () < GetStartIndex ()) {
				// split tag
				TextTag *new_tag = tag->Duplicate ();
				new_tag->SetStartIndex (m_EndIndex);
				new_tag->SetEndIndex (tag->GetEndIndex ());
				tag->SetEndIndex (m_StartIndex);
				return new_tag;
			}
			tag->SetStartIndex (m_EndIndex);
			return NULL;
		} else {
			tag->SetEndIndex (m_StartIndex);
			return NULL;
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Font family tag class

FamilyTextTag::FamilyTextTag (std::string const &family):
	TextTag (Family),
	m_Family (family)
{
}

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
	PangoAttribute *attr = pango_attr_family_new (m_Family.c_str ());
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool FamilyTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Family)
		return false;
	return static_cast <FamilyTextTag const&> (tag).m_Family == m_Family;
}

TextTag *FamilyTextTag::Duplicate () const
{
	return new FamilyTextTag (m_Family);
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
	PangoAttribute *attr = pango_attr_size_new (m_Size);
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool SizeTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Size)
		return false;
	return static_cast <SizeTextTag const&> (tag).m_Size == m_Size;
}

TextTag *SizeTextTag::Duplicate () const
{
	return new SizeTextTag (m_Size);
}

////////////////////////////////////////////////////////////////////////////////
// Font style tag class

StyleTextTag::StyleTextTag (PangoStyle style):
	TextTag (Style),
	m_Style (style)
{
}

StyleTextTag::~StyleTextTag ()
{
}

void StyleTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	PangoAttribute *attr = pango_attr_style_new (m_Style);
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool StyleTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Style)
		return false;
	return static_cast <StyleTextTag const&> (tag).m_Style == m_Style;
}

TextTag *StyleTextTag::Duplicate () const
{
	return new StyleTextTag (m_Style);
}

////////////////////////////////////////////////////////////////////////////////
// Font weight tag class

WeightTextTag::WeightTextTag (PangoWeight weight):
	TextTag (Weight),
	m_Weight (weight)
{
}

WeightTextTag::~WeightTextTag ()
{
}

void WeightTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	PangoAttribute *attr = pango_attr_weight_new (m_Weight);
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool WeightTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Weight)
		return false;
	return static_cast <WeightTextTag const&> (tag).m_Weight == m_Weight;
}

TextTag *WeightTextTag::Duplicate () const
{
	return new WeightTextTag (m_Weight);
}

////////////////////////////////////////////////////////////////////////////////
// Font variant tag class

VariantTextTag::VariantTextTag (PangoVariant variant):
	TextTag (Variant),
	m_Variant (variant)
{
}

VariantTextTag::~VariantTextTag ()
{
}

void VariantTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	PangoAttribute *attr = pango_attr_variant_new (m_Variant);
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool VariantTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Variant)
		return false;
	return static_cast <VariantTextTag const&> (tag).m_Variant == m_Variant;
}

TextTag *VariantTextTag::Duplicate () const
{
	return new VariantTextTag (m_Variant);
}

////////////////////////////////////////////////////////////////////////////////
// Font strecth tag class

StretchTextTag::StretchTextTag (PangoStretch stretch):
	TextTag (Stretch),
	m_Stretch (stretch)
{
}

StretchTextTag::~StretchTextTag ()
{
}

void StretchTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	PangoAttribute *attr = pango_attr_stretch_new (m_Stretch);
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool StretchTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Stretch)
		return false;
	return static_cast <StretchTextTag const&> (tag).m_Stretch == m_Stretch;
}

TextTag *StretchTextTag::Duplicate () const
{
	return new StretchTextTag (m_Stretch);
}

////////////////////////////////////////////////////////////////////////////////
// Font underline tag class

UnderlineTextTag::UnderlineTextTag (TextDecoration underline, GOColor color):
	TextTag (Underline),
	m_Underline (underline),
	m_Color (color)
{
}

UnderlineTextTag::~UnderlineTextTag ()
{
}

void UnderlineTextTag::Filter (G_GNUC_UNUSED PangoAttrList *l, G_GNUC_UNUSED unsigned start, G_GNUC_UNUSED unsigned end)
{
}

bool UnderlineTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Underline)
		return false;
	return static_cast <UnderlineTextTag const&> (tag).m_Underline == m_Underline;
}

TextTag *UnderlineTextTag::Duplicate () const
{
	return new UnderlineTextTag (m_Underline);
}

////////////////////////////////////////////////////////////////////////////////
// Font overline tag class

OverlineTextTag::OverlineTextTag (TextDecoration overline, GOColor color):
	TextTag (Overline),
	m_Overline (overline),
	m_Color (color)
{
}

OverlineTextTag::~OverlineTextTag ()
{
}

void OverlineTextTag::Filter (G_GNUC_UNUSED PangoAttrList *l, G_GNUC_UNUSED unsigned start, G_GNUC_UNUSED unsigned end)
{
}

bool OverlineTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Overline)
		return false;
	return static_cast <OverlineTextTag const&> (tag).m_Overline == m_Overline;
}

TextTag *OverlineTextTag::Duplicate () const
{
	return new OverlineTextTag (m_Overline);
}

////////////////////////////////////////////////////////////////////////////////
// Font strikethrough tag class

StrikethroughTextTag::StrikethroughTextTag (TextDecoration strikethrough, GOColor color):
	TextTag (Strikethrough),
	m_Strikethrough (strikethrough),
	m_Color (color)
{
}

StrikethroughTextTag::~StrikethroughTextTag ()
{
}

void StrikethroughTextTag::Filter (G_GNUC_UNUSED PangoAttrList *l, G_GNUC_UNUSED unsigned start, G_GNUC_UNUSED unsigned end)
{
}

bool StrikethroughTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Strikethrough)
		return false;
	return static_cast <StrikethroughTextTag const&> (tag).m_Strikethrough == m_Strikethrough;
}

TextTag *StrikethroughTextTag::Duplicate () const
{
	return new StrikethroughTextTag (m_Strikethrough);
}

////////////////////////////////////////////////////////////////////////////////
// Font foreground tag class

ForegroundTextTag::ForegroundTextTag (GOColor color):
	TextTag (Foreground),
	m_Color (color)
{
}

ForegroundTextTag::~ForegroundTextTag ()
{
}

void ForegroundTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	// this might be not enough since we use a global attribute with SetColor().
	PangoAttribute *attr = pango_attr_foreground_new (GO_COLOR_UINT_R (m_Color) * 0x101, GO_COLOR_UINT_G (m_Color) * 0x101, GO_COLOR_UINT_B (m_Color) * 0x101);
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool ForegroundTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Foreground)
		return false;
	return static_cast <ForegroundTextTag const&> (tag).m_Color == m_Color;
}

TextTag *ForegroundTextTag::Duplicate () const
{
	return new ForegroundTextTag (m_Color);
}

////////////////////////////////////////////////////////////////////////////////
// Font background tag class

BackgroundTextTag::BackgroundTextTag (GOColor color):
	TextTag (Background),
	m_Color (color)
{
}

BackgroundTextTag::~BackgroundTextTag ()
{
}

void BackgroundTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	PangoAttribute *attr = pango_attr_background_new (GO_COLOR_UINT_R (m_Color) * 0x101, GO_COLOR_UINT_G (m_Color) * 0x101, GO_COLOR_UINT_B (m_Color) * 0x101);
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool BackgroundTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Background)
		return false;
	return static_cast <BackgroundTextTag const&> (tag).m_Color == m_Color;
}

TextTag *BackgroundTextTag::Duplicate () const
{
	return new BackgroundTextTag (m_Color);
}

////////////////////////////////////////////////////////////////////////////////
// Font rise tag class

RiseTextTag::RiseTextTag (double rise):
	TextTag (Rise),
	m_Rise (rise)
{
}

RiseTextTag::~RiseTextTag ()
{
}

void RiseTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	PangoAttribute *attr = pango_attr_rise_new (m_Rise);
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert (l, attr);
}

bool RiseTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != Rise)
		return false;
	return static_cast <RiseTextTag const&> (tag).m_Rise == m_Rise;
}

TextTag *RiseTextTag::Duplicate () const
{
	return new RiseTextTag (m_Rise);
}

////////////////////////////////////////////////////////////////////////////////
// Position tag class (normal, subscript, superscript,...)

PositionTextTag::PositionTextTag (TextPosition position, double size, bool stacked, Tag tag):
	TextTag (tag, TagPriorityLast),
	m_Position (position),
	m_Size (size)
{
	m_Stacked = stacked;
}

PositionTextTag::~PositionTextTag ()
{
}

struct position_data {
	guint start, end;
	std::list <PangoAttribute *>extra;
	std::map <unsigned, int> sizes, rises;
};

static gboolean
position_filter (PangoAttribute *attr, gpointer _data)
{
	struct position_data *data = static_cast <struct position_data *> (_data);
	int start = MAX (data->start, attr->start_index);

	if (attr->end_index <= data->start || attr->start_index >= data->end)
		return false;
	switch (attr->klass->type) {
	case PANGO_ATTR_SIZE:
		data->sizes[start] = reinterpret_cast <PangoAttrSize *> (attr)->size;
		break;
	case PANGO_ATTR_RISE:
		data->rises[start] = reinterpret_cast <PangoAttrInt *> (attr)->value;
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

void PositionTextTag::Filter (PangoAttrList *l, unsigned start, unsigned end)
{
	if (m_Position == Normalscript)
		return;
	struct position_data data;
	data.start = start;
	data.end = end;
	data.sizes[start] = m_Size * PANGO_SCALE;
	data.rises[start] = 0.;
	pango_attr_list_filter (l, position_filter, &data);
	// Build and apply the new attributes
	std::map <unsigned, int>::iterator i, j, iend = data.sizes.end (), jend = data.rises.end (), nexti, nextj;
	unsigned cur_start = start, cur_end;
	j =  data.rises.begin ();
	double rise_f = 1.;
	switch (m_Position) {
	case Subscript:
		rise_f = -3.;
		break;
	case Superscript:
		rise_f = +3/2.;
		break;
	case Normalscript:
		break;
	}
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
			attr = pango_attr_rise_new ((*j).second + (*i).second / rise_f);
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

bool PositionTextTag::operator== (TextTag const& tag) const
{
	if (tag.GetTag () != GetTag ())
		return false;
	return static_cast <PositionTextTag const&> (tag).m_Position == m_Position;
}

TextTag *PositionTextTag::Duplicate () const
{
	return new PositionTextTag (m_Position, m_Size, m_Stacked, GetTag ());
}

////////////////////////////////////////////////////////////////////////////////
// Position tag class (normal, subscript, superscript,...)

NewLineTextTag::NewLineTextTag ():
	TextTag (NewLine)
{
	m_NewLine = true;
}

NewLineTextTag::~NewLineTextTag ()
{
}

void NewLineTextTag::Filter (G_GNUC_UNUSED PangoAttrList *l, G_GNUC_UNUSED unsigned start, G_GNUC_UNUSED unsigned end)
{
	// Nothing to do
}

bool NewLineTextTag::operator== (G_GNUC_UNUSED TextTag const& tag) const
{
	return false;
}

TextTag *NewLineTextTag::Duplicate () const
{
	return new NewLineTextTag ();
}

////////////////////////////////////////////////////////////////////////////////
// Tags list class
TextTagList::TextTagList (): std::list <TextTag *> ()
{
}

TextTagList::~TextTagList ()
{
	std::list <TextTag *>::iterator i, iend = end ();
	for (i = begin (); i != iend; i++)
		delete (*i);
}

}
