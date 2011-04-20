// -*- C++ -*-

/* 
 * Gnome Crystal
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

#ifndef GCRYSTAL_DOC_PROP_H
#define GCRYSTAL_DOC_PROP_H

#include <gcugtk/dialog.h>
#include <gcu/macros.h>

class gcDocument;

class gcDocPropDlg: public gcugtk::Dialog
{
public:
	gcDocPropDlg (gcDocument* pDoc);
	virtual ~gcDocPropDlg ();
	
	void OnTitleChanged (char const *title);
	void OnNameChanged (char const *title);
	void OnMailChanged (char const *title);
	void OnCommentsChanged (char const *title);
	
private:
	gcDocument* m_pDoc;
	GtkEntry *Title, *Name, *Mail;
	GtkLabel *CreationDate, *RevisionDate;
	GtkTextView *Comments;
	gulong m_ChangedSignal;
};

#endif //GCRYSTAL_DOC_PROP_H
