// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text-tag.h 
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

/*!
Text attributes support.
*/
typedef enum
{
/*!
Invalid tag.
*/
	Invalid,
/*!
Font family.
*/
	Family,
/*!
Font size.
*/
	Size,
/*!
Font style.
*/
	Style,
/*!
Font weight.
*/
	Weight,
/*!
Font variant.
*/
	Variant,
/*!
Font stretch.
*/
	Stretch,
/*!
Text underline mode.
*/
	Underline,
/*!
Text overline mode.
*/
	Overline,
/*!
Text strikethrough mode.
*/
	Strikethrough,
/*!
Foreground color.
*/
	Foreground,
/*!
Background color.
*/
	Background,
/*!
Vertical position.
*/
	Rise,
/*!
Normal, Subscript or Superscript.
*/
	Position,
/*!
Insert a new line.
*/
	NewLine,
/*!
First unused value. Larger values might be used by applications.
*/
	MaxTag
} Tag;

/*!
Used for ordering tags.
*/
typedef enum
{
/*!
Puts the Tag on the front of the list so that it will be considered before
other tags. The default value.
*/
	TagPriorityFirst,
/*!
Puts the Tag on the back of the list. Only PositionTextTag has this priority.
*/
	TagPriorityLast,
} TagPriority;

/*!
@brief class for text attributes

Used to set various attributes to a Text item. This class is virtual.
*/
class TextTag
{
public:
/*!
@param tag the tag type.
@param priority the tag priority.

Creates a new tag.
*/
	TextTag (Tag tag, TagPriority priority = TagPriorityFirst);
/*!
The destructor.
*/
	virtual ~TextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Derived classes must implement this pure virtual method. Used to add appropriate
PangoAttribute corresponding to the TextTag in the given bounds. In some cases
already existing attributes might have to be restricted in scope.
*/
	virtual void Filter (PangoAttrList *l, unsigned start, unsigned end) = 0;
/*!
@param tag a TextTag.

Equality operator. Derived classes must implement this pure virtual method.
@return true if the two tags have the same type and same attributes, e.g., two
color attributes are identical if they have the same color.
*/
	virtual bool operator== (TextTag const& tag) const = 0;
/*!
Creates a copy of the TextTag. Derived classes must implement this pure virtual
method.
@return the new TextTag.
*/
	virtual TextTag *Duplicate () const = 0;
/*!
@return true if the TextTag needs to create a new PangoLayout.
*/
	virtual bool NeedsNewRun () {return false;}
/*!
@param tag a TextTag.

Used to avoid overlapping of TextTag instances of the same Tag type. \a this
will have its bounds updated accordingly and might be split.
@return the new TextTag if \a this split or NULL.
*/
	virtual TextTag *Restrict (TextTag *tag);

/*!
Creates a new registered Tag value equal to current MaxTag value and increments
MaxTag.
@return the new Tag value.
*/
	static Tag RegisterTagType ();
/*!
The current first available Tag value for new TextTag types.
*/
	static Tag MaxTag;
/*!
@param first a TextTag.
@param last a TextTag.

Used to sort TextTag instances according to their start and end indices.
@return true if \a first should come before \a last.
*/
	static bool Order (TextTag *first, TextTag *last);

/*!\fn GetTag()
@return the Tag for the TextTag.
*/
GCU_RO_PROP (Tag, Tag)
/*!\fn GetPriority()
@return the TextPriority for the TextTag.
*/
GCU_RO_PROP (TagPriority, Priority)
/*!\fn SetStartIndex(unsigned index)
@param index the start index

Sets the start index in bytes for the TexTag.
*/
/*!\fn GetStartIndex()
@return the start index in bytes for the TexTag.
*/
/*!\fn GetRefStartIndex()
@return the start index in bytes for the TexTag as a reference.
*/
GCU_PROP (unsigned, StartIndex)
/*!\fn SetEndIndex(unsigned index)
@param index the end index

Sets the end index in bytes for the TexTag.
*/
/*!\fn GetEndIndex()
@return the end index in bytes for the TexTag.
*/
/*!\fn GetRefEndIndex()
@return the end index in bytes for the TexTag as a reference.
*/
GCU_PROP (unsigned, EndIndex)
/*!\var m_Stacked
true if the TextTag begins a stacked text run. This is used to have two text
runs start at the same horizontal position.
*/
/*!\fn GetStacked()
true if the TextTag begins a stacked text run.
*/
GCU_PROT_PROP (bool, Stacked)
/*!\var m_NewLine
true if the TextTag begins a new text line.
*/
/*!\fn GetNewLine()
@return true if the TextTag begins a new text line. 
*/
GCU_PROT_PROP (bool, NewLine)
};

/*!
@brief Font family.

TextTag class for font family.
*/
class FamilyTextTag: public TextTag
{
public:
/*!
@param family a font family name.

Constructs a FamilyTextTag with \a family as font family name.
*/
	FamilyTextTag (std::string const &family);
/*!
@param family a font family name.

Constructs a FamilyTextTag with \a family as font family name.
*/
	FamilyTextTag (char const *family);
/*!
The destructor.
*/
	virtual ~FamilyTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the FamilyTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a FamilyTextTag and has the same font family name
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the FamilyTextTag with the same font family name. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the font family name associated with the FamilyTextTag.
*/
	std::string const &GetFamily () const {return m_Family;}

private:
	std::string m_Family;
};

/*!
@brief Font size.

TextTag class for font size.
*/
class SizeTextTag: public TextTag
{
public:
/*!
@param size the font size

Constructs a SizeTextTag with \a size as font size.
*/
	SizeTextTag (double size);
/*!
The destructor.
*/
	virtual ~SizeTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the SizeTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a SizeTextTag and has the same font size
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the SizeTextTag with the same font size. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the font size.
*/
	double GetSize () const {return m_Size;}

private:
	double m_Size;
};


/*!
@brief Font style.

TextTag class for font style.
*/
class StyleTextTag: public TextTag
{
public:
/*!
@param style a font style.

Constructs a StyleTextTag with \a style as font style.
*/
	StyleTextTag (PangoStyle style);
/*!
The destructor.
*/
	virtual ~StyleTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the StyleTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a StyleTextTag and has the same font style
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the StyleTextTag with the same font style. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the font style.
*/
	PangoStyle GetStyle () const {return m_Style;}

private:
	PangoStyle m_Style;
};


/*!
@brief Font weight.

TextTag class for font weight.
*/
class WeightTextTag: public TextTag
{
public:
/*!
@param weight a font weight.

Constructs a WeightTextTag with \a weight as font weight.
*/
	WeightTextTag (PangoWeight weight);
/*!
The destructor.
*/
	virtual ~WeightTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the WeightTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a WeightTextTag and has the same font weight
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the WeightTextTag with the same font weight. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the font weight.
*/
	PangoWeight GetWeight () const {return m_Weight;}

private:
	PangoWeight m_Weight;
};


/*!
@brief Font variant.

TextTag class for font variant.
*/
class VariantTextTag: public TextTag
{
public:
/*!
@param variant a font variant.

Constructs a VariantTextTag with \a variant as font variant.
*/
	VariantTextTag (PangoVariant variant);
/*!
The destructor.
*/
	virtual ~VariantTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the VariantTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a VariantTextTag and has the same font variant
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the VariantTextTag with the same font variant. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the font variant.
*/
	PangoVariant GetVariant () const {return m_Variant;}

private:
	PangoVariant m_Variant;
};


/*!
@brief Font stretch.

TextTag class for font stretch.
*/
class StretchTextTag: public TextTag
{
public:
/*!
@param stretch a font stretch.

Constructs a StretchTextTag with \a stretch as font stretch.
*/
	StretchTextTag (PangoStretch stretch);
/*!
The destructor.
*/
	virtual ~StretchTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the StretchTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a StretchTextTag and has the same font stretch
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the StretchTextTag with the same font stretch. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the font stretch.
*/
	PangoStretch GetStretch () const {return m_Stretch;}

private:
	PangoStretch m_Stretch;
};


/*!
@brief Underline.

TextTag class for text underline.
*/
class UnderlineTextTag: public TextTag
{
public:
/*!
@param underline a TextDecoration.
@param color the line color.

Constructs a UnderlineTextTag with \a underline as underline mode and \a color
than line color, default color being black.
*/
	UnderlineTextTag (TextDecoration underline, GOColor color = GO_COLOR_BLACK);
/*!
The destructor.
*/
	virtual ~UnderlineTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the UnderlineTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a UnderlineTextTag and has the same underline mode
as \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the UnderlineTextTag with the same underline mode. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the underline mode.
*/
	TextDecoration GetUnderline () const {return m_Underline;}

private:
	TextDecoration m_Underline;

GCU_PROP (GOColor, Color)
};

/*!
@brief Overline.

TextTag class for text overline.
*/
class OverlineTextTag: public TextTag
{
public:
/*!
@param overline a TextDecoration.
@param color the line color.

Constructs a OverlineTextTag with \a overline as overline mode and \a color
than line color, default color being black.
*/
	OverlineTextTag (TextDecoration overline, GOColor color = GO_COLOR_BLACK);
/*!
The destructor.
*/
	virtual ~OverlineTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the OverlineTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a OverlineTextTag and has the same overline mode
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the OverlineTextTag with the same overline mode. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the overline mode.
*/
	TextDecoration GetOverline () const {return m_Overline;}

private:
	TextDecoration m_Overline;

GCU_PROP (GOColor, Color)
};

/*!
@brief Strikethrough.

TextTag class for text strikethrough.
*/
class StrikethroughTextTag: public TextTag
{
public:
/*!
@param strikethrough a TextDecoration.
@param color the line color.

Constructs a StrikethroughTextTag with \a strikethrough as strikethrough mode
 and \a color as line color, default color being black.
*/
	StrikethroughTextTag (TextDecoration strikethrough, GOColor color = GO_COLOR_BLACK);
/*!
The destructor.
*/
	virtual ~StrikethroughTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the StrikethroughTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a StrikethroughTextTag and has the same strikethrough mode
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the StrikethroughTextTag with the same strikethrough mode. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the strikethrough mode.
*/
	TextDecoration GetStrikethrough () const {return m_Strikethrough;}

private:
	TextDecoration m_Strikethrough;

GCU_PROP (GOColor, Color)
};

/*!
@brief Foreground color.

TextTag class for text color.
*/
class ForegroundTextTag: public TextTag
{
public:
/*!
@param color the text color.

Constructs a ForegroundTextTag with \a color as text color.
*/
	ForegroundTextTag (GOColor color);
/*!
The destructor.
*/
	virtual ~ForegroundTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the ForegroundTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a ForegroundTextTag and has the same text color
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the ForegroundTextTag with the same text color. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the text color.
*/
	GOColor GetColor () const {return m_Color;}

private:
	GOColor m_Color;
};

/*!
@brief Background color.

TextTag class for background color.
*/
class BackgroundTextTag: public TextTag
{
public:
/*!
@param color the background color.

Constructs a BackgroundTextTag with \a color as background color.
*/
	BackgroundTextTag (GOColor color);
/*!
The destructor.
*/
	virtual ~BackgroundTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the BackgroundTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a BackgroundTextTag and has the same background color
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the BackgroundTextTag with the same background color. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the background color.
*/
	GOColor GetColor () const {return m_Color;}

private:
	GOColor m_Color;
};

/*!
@brief Rise.

TextTag class for text vertical position.
*/
class RiseTextTag: public TextTag
{
public:
/*!
@param rise the vertical offset.

Constructs a RiseTextTag with \a rise as vertical offset above the base line.
Negative values puts the text under the base line.
*/
	RiseTextTag (double rise);
/*!
The destructor.
*/
	virtual ~RiseTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the RiseTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a RiseTextTag and has the same vertical offset above the base line
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the RiseTextTag with the same vertical offset above the base line. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@return the vertical offset above the base line.
*/
	double GetRise () const {return m_Rise;}

private:
	double m_Rise;
};

/*!
@brief Position.

TextTag class for normal, subscript or superscript. The exact position and sizes
are evaluated according to the font size, but are smaller than it.
*/
class PositionTextTag: public TextTag
{
public:
/*!
@param position a TextPosition.
@param size a default size.
@param stacked whether the text should be stacked.
@param tag an optional Tag argument to allow subclassing.

Constructs a PositionTextTag with \a position as text position (normal,
subscript or superscript). \a size is used to evaluate the size and rise if no
other TextTag defines the size between the TextTag bounds. If \a stacked is true,
a subscript and a superscript will appear at the same horizontal position.
*/
	PositionTextTag (TextPosition position, double size, bool stacked = false, Tag tag = Position);
/*!
The destructor.
*/
	virtual ~PositionTextTag ();

/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the PositionTextTag class.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return true if \a tag is a PositionTextTag and has the position
than \a this.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the PositionTextTag with the same position, size, stacked and
Tag attributes. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
/*!
@param stacked where to store whether the text should be stacked.
@param size where to store the default size.

@return the TextPosition for this PositionTextTag.
*/
	TextPosition GetPosition (bool &stacked, double &size) const {stacked = m_Stacked; size = m_Size; return m_Position;}
/*!
@return true if the PositionTextTag is stacked.
*/
	virtual bool NeedsNewRun () {return false;}
/*!
@param tag a TextTag.

Implementation of TextTag::Restrict for the PositionTextTag class.
@return the new TextTag if \a this split or NULL.
*/
	TextTag *Restrict (G_GNUC_UNUSED TextTag *tag) {return NULL;}

private:
	TextPosition m_Position;
	double m_Size; // default size
};

/*!
@brief New line.

TextTag class to start a new line.
*/
class NewLineTextTag: public TextTag
{
public:
/*!
Constructs a new NewLineTextTag.
*/
	NewLineTextTag ();
/*!
The destructor.
*/
	virtual ~NewLineTextTag ();
	
/*!
@param l a PangoAttrList.
@param start the start index of the relevant text.
@param end the end index of the relevant text.

Filter method for the NewLineTextTag class. Do not do anything.
*/
	void Filter (PangoAttrList *l, unsigned start, unsigned end);
/*!
@param tag a TextTag.

Equality operator.
@return false.
*/
	bool operator== (TextTag const& tag) const;
/*!
Creates a copy of the NewLineTextTag. 
@return the new TextTag.
*/
	TextTag *Duplicate () const;
};

/*!
@brief TextTag list.

List of TextTag instances.
*/
class TextTagList:public std::list <TextTag *>
{
public:
/*!
Constructs an empty list.
*/
	TextTagList ();
/*!
Destructs the list and all the TextTag instances inside. If the TextTag must not
be destroyed, call the clear() method before destroying the list.
*/
	~TextTagList ();
};

}   //	namespace gccv

#endif	//	GCCV_TEXT_TAG_H
