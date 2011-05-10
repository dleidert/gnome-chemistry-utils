// -*- C++ -*-

/* 
 * GChemPaint library
 * zoomdlg.h
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_ZOOM_DLG_H
#define GCHEMPAINT_ZOOM_DLG_H

#include <gcugtk/dialog.h>
#include <gtk/gtk.h>

/*!\file*/
namespace gcp {

class Document;

/*!\class ZoomDlg gcp/zoomdlg.h
The zoom level dialog.
*/
class ZoomDlg: public gcugtk::Dialog
{
public:
/*!
@param pDoc the document to zoom.

Constructs a dialog to change the zoom level for \a pDoc.
*/
	ZoomDlg (gcp::Document* pDoc);
/*!
The destructor.
*/
	virtual ~ZoomDlg ();
	
/*!
Called by the framework when the dialog gets the focus and update the zoom level
which might have changed.
*/
	void OnFocusIn ();
	
private:
	GtkSpinButton *btn;
	gcp::Document *m_Doc;
	gulong m_ZoomSignal;
};

}	// namespace gcp

#endif //GCHEMPAINT_ZOOM_DLG_H
