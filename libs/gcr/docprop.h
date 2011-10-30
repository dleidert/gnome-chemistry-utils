// -*- C++ -*-

/*
 * Gnome Crystal library
 * docprop.h
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_DOC_PROP_H
#define GCR_DOC_PROP_H

#include <gcugtk/dialog.h>
#include <gcu/macros.h>

namespace gcr {

class Document;

class DocPropDlg: public gcugtk::Dialog
{
friend class DocProDlgPrivate;
public:
	DocPropDlg (Document* pDoc);
	virtual ~DocPropDlg ();

private:
	Document* m_pDoc;
	GtkEntry *Title, *Name, *Mail;
	GtkLabel *CreationDate, *RevisionDate;
	GtkTextView *Comments;
	gulong m_ChangedSignal;
};

}	//	namespace gcr
#endif //GCR_DOC_PROP_H
