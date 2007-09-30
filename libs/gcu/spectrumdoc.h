/* 
 * Gnome Chemisty Utils
 * spectrumdoc.h
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

#ifndef GCU_SPECTRUM_DOC_H
#define GCU_SPECTRUM_DOC_H

#include "document.h"
#include "macros.h"

namespace gcu
{

typedef enum {
	GCU_SPECTRUM_INFRARED,
	GCU_SPECTRUM_RAMAN,
	GCU_SPECTRUM_INFRARED_PEAK_TABLE,
	GCU_SPECTRUM_INFRARED_INTERFEROGRAM,
	GCU_SPECTRUM_INFRARED_TRANSFORMED,
	GCU_SPECTRUM_UV_VISIBLE,
	GCU_SPECTRUM_NMR,
	GCU_SPECTRUM_MASS,
	GCU_SPECTRUM_MAX
} SpectrumType;

class Application;
class SpectrumView;

class SpectrumDocument: public Document
{
public:
/*!
Default constructor
*/
	SpectrumDocument ();
/*!
@param App the application.
@param View: an optional already existing SpectrumView instance.
*/
	SpectrumDocument (Application *app, SpectrumView *view = NULL);

/*!
Default destructor
*/
	~SpectrumDocument ();

/*!
@param uri the uri of the spectrum file.
@param mime_type the mime type of the spectrum file.

Loads a spaectrum from the provided uri. Default mime type is NULL,
"chemical/x-jcamp-dx" is the only one supported at the moment.
*/
	void Load (char const *uri, char const *mime_type = NULL);

private:
	void LoadJcampDx (char const *data);
	void ReadDataLine (char const *data, std::list<double> &l);

private:
	double *x, *y;
	unsigned npoints;
	double maxx, maxy, minx, miny;
	double firstx, lastx, deltax, firsty;
	double xfactor, yfactor;

GCU_PROT_PROP (SpectrumView*, View);
GCU_RO_PROP (bool, Empty);
GCU_RO_PROP (SpectrumType, SpectrumType);
};

}

#endif	//	GCU_SPECTRUM_DOC_H
