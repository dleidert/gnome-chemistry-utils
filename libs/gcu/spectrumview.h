/* 
 * Gnome Chemisty Utils
 * spectrumview.h
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_SPECTRUM_VIEW_H
#define GCU_SPECTRUM_VIEW_H

#include <goffice/graph/gog-series.h>
#include <gtk/gtkwidget.h>

namespace gcu
{

class SpectrumDocument;

class SpectrumView
{
public:
//!Constructor.
/*!
@param pDoc: a pointer to the SpectrumDocument instance.

Creates a view for the document.
*/
	SpectrumView (SpectrumDocument *pDoc);

//!Destructor.
/*!
The destructor of SpectrumView.
*/
	virtual ~SpectrumView ();

/*!\fn GetDoc()
@return the associated document.
*/
GCU_RO_PROP (SpectrumDocument *, Doc)
GCU_RO_PROP (GtkWidget *, Widget)
GCU_RO_PROP (GogSeries *, Series)
};

}

#endif	//	GCU_SPECTRUM_VIEW_H
