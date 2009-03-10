// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text-tag.h 
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

#ifndef GCCV_TEXT_TAG_H
#define GCCV_TEXT_TAG_H

#include <gcu/macros.h>
#include <goffice/utils/go-color.h>
#include <pango/pango.h>
#include <list>
#include <string>

/*!\file*/
namespace gccv {

// Text attributes support
typedef enum
{
	Invalid,
	Family,
	Size,
	Style,
	Weight,
	Variant,
	Strecth,
	Underline,
	Strikethrough,
	Foreground,
	Background,
	Rise,
	Subscript,
	Superscript,
	Max
} Tag;

typedef enum
{
	TagPriorityFirst,
	TagPriorityLast,
} TagPriority;

class TextTag
{
public:
	TextTag (Tag tag, TagPriority priority = TagPriorityFirst);
	virtual ~TextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end) = 0;
private:

GCU_RO_PROP (Tag, Tag)
GCU_RO_PROP (TagPriority, Priority)
GCU_PROP (unsigned, StartIndex)
GCU_PROP (unsigned, EndIndex)
};

class FamilyTextTag: public TextTag
{
public:
	FamilyTextTag (char const *family);
	virtual ~FamilyTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	std::string m_Family;
};

class SizeTextTag: public TextTag
{
public:
	SizeTextTag (double size);
	virtual ~SizeTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	double m_Size;
};

class StyleTextTag: public TextTag
{
public:
	StyleTextTag (PangoStyle style);
	virtual ~StyleTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	PangoStyle m_Style;
};

class WeightTextTag: public TextTag
{
public:
	WeightTextTag (PangoWeight weight);
	virtual ~WeightTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	PangoWeight m_Weight;
};

class VariantTextTag: public TextTag
{
public:
	VariantTextTag (PangoVariant variant);
	virtual ~VariantTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	PangoVariant m_Variant;
};

class StretchTextTag: public TextTag
{
public:
	StretchTextTag (PangoStretch stretch);
	virtual ~StretchTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	PangoStretch m_Stretch;
};

class UnderlineTextTag: public TextTag
{
public:
	UnderlineTextTag (PangoUnderline underline);
	virtual ~UnderlineTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	PangoUnderline m_Underline;
};

class StrikethroughTextTag: public TextTag
{
public:
	StrikethroughTextTag (bool strikethrough);
	virtual ~StrikethroughTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	bool m_Strikethrough;
};

class ForegroundTextTag: public TextTag
{
public:
	ForegroundTextTag (GOColor m_Color);
	virtual ~ForegroundTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	GOColor m_Color;
};

class BackgroundTextTag: public TextTag
{
public:
	BackgroundTextTag (GOColor m_Color);
	virtual ~BackgroundTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	GOColor m_Color;
};

class RiseTextTag: public TextTag
{
public:
	RiseTextTag (double size);
	virtual ~RiseTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	double m_Rise;
};

class SubscriptTextTag: public TextTag
{
public:
	SubscriptTextTag (double size, bool stacked = false);
	virtual ~SubscriptTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	double m_Size; // default size
	bool m_Stacked;
};

class SuperscriptTextTag: public TextTag
{
public:
	SuperscriptTextTag (double size, bool stacked = false);
	virtual ~SuperscriptTextTag ();

	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end);

private:
	double m_Size; // default size
	bool m_Stacked;
};

class TextTagList:public std::list <TextTag *>
{
public:
	TextTagList ();
	~TextTagList ();
};

}   //	namespace gccv

#endif	//	GCCV_TEXT_TAG_H
