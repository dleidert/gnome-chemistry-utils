// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text.h 
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

#define GCCV_TEXT_PROP(type,member) \
public:	\
	void Set##member (type val) {	\
		m_##member = val;	\
		SetPosition (m_x, m_y);	\
	}	\
	type Get##member (void) const {return m_##member;}	\
	type &GetRef##member (void) {return m_##member;}	\
private:	\
	type m_##member;

// These are private classes
class TextPrivate;
class TextRun;
class TextTag;

class Text: public Rectangle
{
friend class TextPrivate;
public:
	Text (Canvas *canvas, double x, double y);
	Text (Group *parent, double x, double y, ItemClient *client = NULL);
	virtual ~Text ();

	void SetPosition (double x, double y);
	void SetText (char const *text);
	void SetFontDescription (PangoFontDescription *desc);
	void SetEditing (bool editing);

	void GetBounds (Rect *ink, Rect *logical);
	char const *GetText ();

	void InsertTextTag (TextTag *tag);

/*!
@param str the text to insert
@param pos the insertion position
@param length the length (in bytes) of the substring to replace

Replaces (or inserts if \a length is nul) text in the text item. If position is
-1, the text will be inserted at the current cursor position. Tne cursor will
be moved to the end of the inserted text.
*/
	void ReplaceText (std::string &str, int pos, unsigned length);
	unsigned GetCursorPosition () {return m_CurPos;}

	// virtual methods
	void Draw (cairo_t *cr, bool is_vector) const;
	void Move (double x, double y);

	// events related methods
	bool OnKeyPressed (GdkEventKey *event);

	// static methods
	static PangoContext *GetContext ();

unsigned GetDefaultFontSize () { return (m_FontDesc)? (double) pango_font_description_get_size (m_FontDesc) / PANGO_SCALE: 0; }

private:
	double m_x, m_y;
	unsigned long m_BlinkSignal;
	bool m_CursorVisible;
	unsigned m_CurPos;
	std::list <TextRun *> m_Runs;
	std::list <TextTag *> m_Tags;
	std::string m_Text;
	GtkIMContext *m_ImContext;
	PangoFontDescription *m_FontDesc;


GCCV_TEXT_PROP (double, Padding)
GCCV_TEXT_PROP (Anchor, Anchor)
GCCV_TEXT_PROP (double, LineOffset)
GCU_RO_PROP (double, Width)
GCU_RO_PROP (double, Height)
GCU_RO_PROP (double, Ascent)
GCU_RO_PROP (double, Y)
};

}   //	namespace gccv

#endif	//	GCCV_TEXT_H
