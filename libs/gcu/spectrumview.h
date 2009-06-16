/* 
 * Gnome Chemisty Utils
 * spectrumview.h
 *
 * Copyright (C) 2007-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "macros.h"

/*!\file*/
namespace gcu
{

class SpectrumDocument;

/*!\class SpectrumView gcu/spectrumview.h
The view class used for spectra. This API is still quite unstable and
might change in the future.
*/
class SpectrumView
{
friend class SpectrumViewPrivate;
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

/*!
@param target an axis type.
@param min the mimimum value to show.
@param max the maximum value to show.
@param inverted whether to invert the axis.

Sets the scale of the first axis of the selected type.
*/
	void SetAxisBounds (GogAxisType target, double min, double max, bool inverted);

/*!
@param target an axis type.
@param unit a text (might be a unit).

Sets the text for the label of the first axis of the selected type.
*/
	void SetAxisLabel (GogAxisType target, char const *unit);

/*!
@param target an axis type.
@param show whether to show the axis or not

Used to show or hide the first axis of either GOG_AXIS_X or GOG_AXIS_Y types.
*/
	void ShowAxis (GogAxisType target, bool show);

/*!
@param target an axis type.
@param inverted whether to invert the axis scale or not

Used to invert the first axis of either GOG_AXIS_X or GOG_AXIS_Y types.
*/
	void InvertAxis (GogAxisType target, bool inverted);

/*!
@param cr the cairo_t* to which render.
@param width the width of the rendering area.
@param height the height of the rendering area.

Renders the chart to \a cr.
*/
	void Render (cairo_t *cr, double width, double height);

/*!
Called by the framework if the minimum has changed for the x-axis.
*/
	void OnMinChanged ();

/*!
Called by the framework if the minimum has changed for the y-axis.
*/
	void OnYMinChanged ();

/*!
Called by the framework if the maximum has changed for the x-axis.
*/
	void OnMaxChanged ();

/*!
Called by the framework if the maximum has changed for the y-axis.
*/
	void OnYMaxChanged ();

/*!
Called by the framework if the range has changed for the x-axis.
*/
	void OnXRangeChanged ();

/*!
Called by the framework if the range has changed for the y-axis.
*/
	void OnYRangeChanged ();

/*!
@param new_plot if true, a new plot is created (this is not yet implemented).
Creates a new GogSeries for the chart.
*/
	GogSeries *NewSeries (bool new_plot);
			
/*!
@param filename the name of the file.
@param mime_type the requested mime type.
@param width the width of the generated image.
@param height the height of the generated image.

Export the view contents as an image. The size of the new image is defined by the width
and height parameters. Supported ilage file format include svg, png, jpeg, ps, eps,
and pdf, and possibly a few other bitmap formats.
*/
	void SaveAsImage (std::string const &filename, char const *mime_type, unsigned width, unsigned height) const;

private:
	GtkSpinButton *xminbtn, *xmaxbtn, *yminbtn, *ymaxbtn;
	GtkRange *xrange, *yrange;
	gulong minsgn, maxsgn, yminsgn, ymaxsgn, xrangesgn, yrangesgn;
	double xmin, xmax, xstep, ymin, ymax, ystep;

/*!\fn GetDoc()
@return the associated document.
*/
GCU_RO_PROP (SpectrumDocument *, Doc)
/*!\fn GetWidget()
@return the widget used to display the spectrum.
*/
GCU_RO_PROP (GtkWidget *, Widget)
/*!\fn GetOptionBox()
@return a GtkBox to which an optional user interface might be added.
*/
GCU_RO_PROP (GtkWidget *, OptionBox)
/*!\fn GetSeries()
@return the first GogSeries* created for the view.
*/
GCU_RO_PROP (GogSeries *, Series)
GCU_RO_PROP (int, Width)
GCU_RO_PROP (int, Height)
};

}

#endif	//	GCU_SPECTRUM_VIEW_H
