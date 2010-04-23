// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text.h 
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

#ifndef GCCV_TEXT_H
#define GCCV_TEXT_H

#include "rectangle.h"
#include "structs.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pango/pango.h>
#include <list>
#include <string>

/*!\file*/
namespace gccv {

/*!\def GCCV_TEXT_PROP()
Defines a private member with appropriate get/set methods. This macro should
be used when a property change imples a size and/or position change of the text
item.
GCCV_TEXT_PROP((Type,Foo) expands to one private member:
\code
	Type m_Foo;
\endcode

and three public methods:
\code
	void SetFoo(Type val);
	Type GetFoo();
\endcode

Calling SetFoo(val) will set the member, reevaluate the size and position, and
redraw the text item.
*/
#define GCCV_TEXT_PROP(type,member) \
public:	\
	void Set##member (type val) {	\
		m_##member = val;	\
		SetPosition (m_x, m_y);	\
	}	\
	type Get##member (void) const {return m_##member;}	\
private:	\
	type m_##member;

// These are private classes
class TextPrivate;
class TextLine;
class TextRun;
class TextTag;
class TextTagList;

/*!
@brief Text.

The Text item class is used to display text in the canvas.
*/
class Text: public Rectangle
{
friend class TextPrivate;
public:
/*!
@param canvas a Canvas.
@param x the horizontal position.
@param y the vertical position.

Creates a new Text sets it as a child of the root Group of \a canvas.
Interpretation of the values \a x and \a y relies on the text anchor.
*/
	Text (Canvas *canvas, double x, double y);
/*!
@param parent the Group to which the new Text will be added.
@param x the top left corner horizontal position.
@param y the top left corner vertical position.
@param client the ItemClient for the new Text if any.

Creates a new Text inside \a parent and sets \a client as its associated
ItemClient. Interpretation of the values \a x and \a y relies on the text anchor.
*/
	Text (Group *parent, double x, double y, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Text ();

/*!
@param x the top left corner new horizontal position.
@param y the top left corner new vertical position.

Sets the position of the Text. Interpretation of the values \a x and \a y relies
on the text anchor.
*/
	void SetPosition (double x, double y);
/*!
@param text the text to display.

Sets the text displayed by the item.
*/
	void SetText (char const *text);
/*!
@param text the text to display.

Sets the text displayed by the item.
*/
	void SetText (std::string const &text);
/*!
@param desc a font description.

Sets the default font description for the item.
*/
	void SetFontDescription (PangoFontDescription *desc);
/*!
@param color the default text color.

Sets the default text color for the item.
*/
	void SetColor (GOColor color);
/*!
@param editing whether to show the cursor or not

When editing a blinking cursor is displayed.
*/
	void SetEditing (bool editing);

/*!
@param ink where to store the ink bounds, might be NULL.
@param logical where to store the logical bounds, might be NULL.

Retrieved the ink and logical bounds for the text as defined by Pango.
*/
	void GetBounds (Rect *ink, Rect *logical);
/*!
@return the text displayed by the item.
*/
	char const *GetText ();

/*!
@param tag a TextTag
@param rebuild_attributes whether to rebuild the PangoAttrList for the text.

Inserts a new TextTag.
*/
	void InsertTextTag (TextTag *tag, bool rebuild_attributes = true);
/*!
@param tag a TextTag
@param rebuild_attributes whether to rebuild the PangoAttrList for the text.

Removes an existing TextTag.
*/
	void DeleteTextTag (TextTag *tag, bool rebuild_attributes = true);
/*!
@return the current TextTag list for the Text item.
*/
	std::list <TextTag *> const *GetTags () {return &m_Tags;}

/*!
@param l text tags.

Sets the list of TextTag instances to be applied when text is inserted.
*/
	void SetCurTagList (TextTagList *l);

/*!
@param l text tags.

Apply the tags list to the current selection.
*/
	void ApplyTagsToSelection (TextTagList const *l);

/*!
@param str the text to insert
@param pos the insertion position
@param length the length (in bytes) of the substring to replace

Replaces (or inserts if \a length is nul) text in the text item. If position is
-1, the text will be inserted at the current cursor position. The cursor will
be moved to the end of the inserted text.
*/
	void ReplaceText (std::string &str, int pos, unsigned length);
	unsigned GetCursorPosition () {return m_CurPos;}
	void GetSelectionBounds (unsigned &start, unsigned &end);
	void SetSelectionBounds (unsigned start, unsigned end);
	unsigned GetIndexAt (double x, double y);
	bool GetPositionAtIndex (unsigned index, Rect &rect);

	// virtual methods
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Text to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Text.
*/
	void Move (double x, double y);

	// events related methods
/*!
@param event a GdkEventKey.

Notifies the Text item that a relevant "key-press" event has occured.
*/
	bool OnKeyPressed (GdkEventKey *event);
/*!
@param event a GdkEventKey.

Notifies the Text item that a relevant "key-release" event has occured.
*/
	bool OnKeyReleased (GdkEventKey *event);
/*!
@param x the cursor horizontal position.
@param y the cursor vertical position.

Notifies the Text item that a relevant "button-press" event has occured. This
moves the cursor to the new position.
*/
	void OnButtonPressed (double x, double y);
/*!
@param x the cursor horizontal position.
@param y the cursor vertical position.

Notifies the Text item that a relevant "mouse-move" event has occured while a
button is pressed to extend the selection.
*/
	void OnDrag (double x, double y);

	// static methods
/*!
@return the PangoContext used to evaluate sizes and positions for all Text
instances.
*/
	static PangoContext *GetContext ();

/*!
@return the default font sioze for the item.
*/
	unsigned GetDefaultFontSize () { return (m_FontDesc)? (double) pango_font_description_get_size (m_FontDesc) / PANGO_SCALE: 0; }
/*!
Rebuils the Pango attributes lists used by the item.
*/
	void RebuildAttributes ();
/*!
@param interline the new interline.
@param emit_changed whether to call TextClient::InterlineChanged for the client.

Sets the interline spacing and notifies the client if requested, and if there is
actually any client.
*/
	void SetInterline (double interline, bool emit_changed = false);
/*!
@param justification the new justification.
@param emit_changed whether to call TextClient::JustificationChanged for the client.

Sets the justification and notifies the client if requested, and if there is
actually any client. The justification is meaningful only for multiline texts.
*/
	void SetJustification (GtkJustification justification, bool emit_changed = false);

private:
	double m_x, m_y;
	unsigned long m_BlinkSignal;
	bool m_CursorVisible;
	unsigned m_CurPos, m_StartSel;
	std::list <TextRun *> m_Runs;
	std::list <TextTag *> m_Tags;
	std::string m_Text;
	GtkIMContext *m_ImContext;
	PangoFontDescription *m_FontDesc;
	TextTagList *m_CurTags;
	TextLine *m_Lines;
	unsigned m_LinesNumber;
	GOColor m_Color;

/*!\fn SetPadding(double padding)
@param padding the new padding around the text.

Adds \a padding in all directions around the text logical area.
*/
/*!\fn GetPadding()
@return the Text padding.
*/
GCCV_TEXT_PROP (double, Padding)
/*!\fn SetAnchor(Anchor anchor)
@param anchor the new Anchor.

Sets the Anchor foir the Text.
*/
/*!\fn GetAnchor()
@return the Text Anchor.
*/
GCCV_TEXT_PROP (Anchor, Anchor)
/*!\fn SetLineOffset(double offset)
@param offset the distance between the base line and the position used for
alignment.

Sets a vertical offset for the text alignment relative to other items. Typically
the half width of a 'C' character in the default font.
*/
/*!\fn GetLineOffset()
@return the Text line offset.
*/
GCCV_TEXT_PROP (double, LineOffset)
/*!\fn GetJustification()
@return the Text justification.
*/
GCU_RO_PROP (GtkJustification, Justification)
/*!\fn GetInterline()
@return the Text interline spacing.
*/
GCU_RO_PROP (double, Interline)
/*!\fn GetWidth()
@return the Text width.
*/
GCU_RO_PROP (double, Width)
/*!\fn GetHeight()
@return the Text height.
*/
GCU_RO_PROP (double, Height)
/*!\fn GetAscent()
@return the Text first line ascent.
*/
GCU_RO_PROP (double, Ascent)
/*!\fn GetY()
@return the vertical alignment position.
*/
GCU_RO_PROP (double, Y)
};

}   //	namespace gccv

#endif	//	GCCV_TEXT_H
