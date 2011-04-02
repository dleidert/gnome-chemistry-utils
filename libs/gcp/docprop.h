// -*- C++ -*-

/* 
 * GChemPaint library
 * docprop.h 
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

/*!\file*/
namespace gcp {

class Document;
class Theme;

/*!\class DocPropDlg gcp/docprop.h
The document property dialog.
*/
class DocPropDlg: public gcu::Dialog, gcu::Object
{
public:
/*!
@param pDoc the document.

Constructs a property dialog for \a pDoc.
*/
	DocPropDlg (Document* pDoc);
/*!
The destructor.
*/
	virtual ~DocPropDlg ();
	
/*!
Called by the framework to update themes names.
*/
	void OnThemeNamesChanged ();
/*!
@param theme the new document theme.

Called by the framework when the theme changed.
*/
	void OnThemeChanged (Theme *theme);
/*!
@param title the new document title.

Called by the framework when the document title changed.
*/
	void OnTitleChanged (char const *title);
/*!
@param title the new author name.

Called by the framework when the author name changed.
*/
	void OnNameChanged (char const *title);
/*!
@param title the new author mail.

Called by the framework when the author mail changed.
*/
	void OnMailChanged (char const *title);
/*!
@param title the new document comments.

Called by the framework when the document comments changed.
*/
	void OnCommentsChanged (char const *title);

private:
	Document* m_pDoc;
	GtkEntry *Title, *NameEntry, *Mail;
	GtkLabel *CreationDate, *RevisionDate;
	GtkTextView *Comments;
	GtkTextBuffer *Buffer;
#if GTK_CHECK_VERSION (2, 24, 0)
	GtkComboBoxText *m_Box;
#else
	GtkComboBox *m_Box;
#endif
	unsigned m_Lines;
	gulong m_ChangedSignal;
};

}	//	namespace gcp

#endif //GCHEMPAINT_DOC_PROP_H
