// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text-tag.h 
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

#ifndef GCCV_TEXT_TAG_H
#define GCCV_TEXT_TAG_H

#include "structs.h"
#include <gcu/macros.h>
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
	Stretch,
	Underline,
	Overline,
	Strikethrough,
	Foreground,
	Background,
	Rise,
	Position,
	NewLine,
	MaxTag
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
	virtual bool operator== (TextTag const& tag) const = 0;
	virtual TextTag *Duplicate () const = 0;
	virtual bool NeedsNewRun () {return false;}
	virtual TextTag *Restrict (TextTag *tag);

	static Tag RegisterTagType ();
	static Tag MaxTag;
	static bool Order (TextTag *first, TextTag *last);

GCU_RO_PROP (Tag, Tag)
GCU_RO_PROP (TagPriority, Priority)
GCU_PROP (unsigned, StartIndex)
GCU_PROP (unsigned, EndIndex)
GCU_PROT_PROP (bool, Stacked)
GCU_PROT_PROP (bool, NewLine)
};

class FamilyTextTag: public TextTag
{
public:
	FamilyTextTag (std::string const &family);
	FamilyTextTag (char const *family);
	virtual ~FamilyTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	std::string const &GetFamily () const {return m_Family;}

private:
	std::string m_Family;
};

class SizeTextTag: public TextTag
{
public:
	SizeTextTag (double size);
	virtual ~SizeTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	double GetSize () const {return m_Size;}

private:
	double m_Size;
};

class StyleTextTag: public TextTag
{
public:
	StyleTextTag (PangoStyle style);
	virtual ~StyleTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	PangoStyle GetStyle () const {return m_Style;}

private:
	PangoStyle m_Style;
};

class WeightTextTag: public TextTag
{
public:
	WeightTextTag (PangoWeight weight);
	virtual ~WeightTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	PangoWeight GetWeight () const {return m_Weight;}

private:
	PangoWeight m_Weight;
};

class VariantTextTag: public TextTag
{
public:
	VariantTextTag (PangoVariant variant);
	virtual ~VariantTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	PangoVariant GetVariant () const {return m_Variant;}

private:
	PangoVariant m_Variant;
};

class StretchTextTag: public TextTag
{
public:
	StretchTextTag (PangoStretch stretch);
	virtual ~StretchTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	PangoStretch GetStretch () const {return m_Stretch;}

private:
	PangoStretch m_Stretch;
};

class UnderlineTextTag: public TextTag
{
public:
	UnderlineTextTag (TextDecoration underline, GOColor color = 0xff);
	virtual ~UnderlineTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	TextDecoration GetUnderline () const {return m_Underline;}

private:
	TextDecoration m_Underline;

GCU_PROP (GOColor, Color)
};

class OverlineTextTag: public TextTag
{
public:
	OverlineTextTag (TextDecoration overline, GOColor color = 0xff);
	virtual ~OverlineTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	TextDecoration GetOverline () const {return m_Overline;}

private:
	TextDecoration m_Overline;

GCU_PROP (GOColor, Color)
};

class StrikethroughTextTag: public TextTag
{
public:
	StrikethroughTextTag (TextDecoration strikethrough, GOColor color = 0xff);
	virtual ~StrikethroughTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	TextDecoration GetStrikethrough () const {return m_Strikethrough;}

private:
	TextDecoration m_Strikethrough;

GCU_PROP (GOColor, Color)
};

class ForegroundTextTag: public TextTag
{
public:
	ForegroundTextTag (GOColor m_Color);
	virtual ~ForegroundTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	GOColor GetColor () const {return m_Color;}

private:
	GOColor m_Color;
};

class BackgroundTextTag: public TextTag
{
public:
	BackgroundTextTag (GOColor m_Color);
	virtual ~BackgroundTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	GOColor GetColor () const {return m_Color;}

private:
	GOColor m_Color;
};

class RiseTextTag: public TextTag
{
public:
	RiseTextTag (double size);
	virtual ~RiseTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	double GetRise () const {return m_Rise;}

private:
	double m_Rise;
};

class PositionTextTag: public TextTag
{
public:
	PositionTextTag (TextPosition position, double size, bool stacked = false, Tag tag = Position);
	virtual ~PositionTextTag ();

	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
	TextPosition GetPosition (bool &stacked, double &size) const {stacked = m_Stacked; size = m_Size; return m_Position;}
	bool NeedsNewRun () {return m_Stacked;}
	TextTag *Restrict (G_GNUC_UNUSED TextTag *tag) {return NULL;}

private:
	TextPosition m_Position;
	double m_Size; // default size
};

class NewLineTextTag: public TextTag
{
public:
	NewLineTextTag ();
	virtual ~NewLineTextTag ();
	
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
	bool operator== (TextTag const& tag) const;
	TextTag *Duplicate () const;
};

class TextTagList:public std::list <TextTag *>
{
public:
	TextTagList ();
	~TextTagList ();
};

}   //	namespace gccv

#endif	//	GCCV_TEXT_TAG_H