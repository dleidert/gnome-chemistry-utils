// -*- C++ -*-

/* 
 * GChemPaint library
 * docprop.h 
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

#ifndef GCHEMPAINT_DOC_PROP_H
#define GCHEMPAINT_DOC_PROP_H

#include <gcu/dialog.h>
#include <gcu/object.h>
#include <gcu/macros.h>

namespace gcp {

class Document;
class Theme;

class DocPropDlg: public gcu::Dialog,gcu::Object
{
public:
	DocPropDlg (Document* pDoc);
	virtual ~DocPropDlg ();
	
	void OnThemeNamesChanged ();
	void OnThemeChanged (Theme *theme);
	void OnTitleChanged (char const *title);
	void OnNameChanged (char const *title);
	void OnMailChanged (char const *title);
	void OnCommentsChanged (char const *title);
	
private:
	Document* m_pDoc;
	GtkEntry *Title, *Name, *Mail;
	GtkLabel *CreationDate, *RevisionDate;
	GtkTextView *Comments;
	GtkTextBuffer *Buffer;
	GtkComboBox *m_Box;
	unsigned m_Lines;
	gulong m_ChangedSignal;

GCU_PROP (Theme*, m_Theme)
};

}	//	namespace gcp

#endif //GCHEMPAINT_DOC_PROP_H
