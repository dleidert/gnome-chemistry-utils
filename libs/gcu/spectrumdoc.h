/* 
 * Gnome Chemisty Utils
 * spectrumdoc.h
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "printable.h"
#include <string>
#include <vector>

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
	GCU_SPECTRUM_NMR_FID,
	GCU_SPECTRUM_NMR_PEAK_TABLE,
	GCU_SPECTRUM_NMR_PEAK_ASSIGNMENTS,
	GCU_SPECTRUM_MASS,
	GCU_SPECTRUM_MAX
} SpectrumType;

typedef enum {
	GCU_SPECTRUM_UNIT_CM_1,
	GCU_SPECTRUM_UNIT_TRANSMITTANCE,
	GCU_SPECTRUM_UNIT_ABSORBANCE,
	GCU_SPECTRUM_UNIT_PPM,
	GCU_SPECTRUM_UNIT_NANOMETERS,
	GCU_SPECTRUM_UNIT_MICROMETERS,
	GCU_SPECTRUM_UNIT_SECONDS,
	GCU_SPECTRUM_UNIT_HZ,
	GCU_SPECTRUM_UNIT_M_Z,
	GCU_SPECTRUM_UNIT_REL_ABUNDANCE,
	GCU_SPECTRUM_UNIT_MAX
} SpectrumUnitType;

typedef enum {
	GCU_SPECTRUM_TYPE_INDEPENDENT,
	GCU_SPECTRUM_TYPE_DEPENDENT,
	GCU_SPECTRUM_TYPE_PAGE,
	GCU_SPECTRUM_TYPE_MAX
} SpectrumVarType;

typedef enum {
	GCU_SPECTRUM_FORMAT_ASDF,
	GCU_SPECTRUM_FORMAT_AFFN,
	GCU_SPECTRUM_FORMAT_PAC,
	GCU_SPECTRUM_FORMAT_SQZ,
	GCU_SPECTRUM_FORMAT_DIF,
	GCU_SPECTRUM_FORMAT_MAX
} SpectrumFormatType;

class Application;
class SpectrumView;

typedef struct  {
	std::string Name;
	char Symbol;
	SpectrumVarType Type;
	SpectrumUnitType Unit;
	SpectrumFormatType Format;
	unsigned NbValues;
	double First, Last, Min, Max, Factor;
	double *Values;
} JdxVar;

class SpectrumDocument: public Document, public Printable
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

/*!
*/
	void OnUnitChanged (int i);

private:
	void LoadJcampDx (char const *data);
	void ReadDataLine (char const *data, std::list<double> &l);
	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context);
	GtkWindow *GetGtkWindow ();
	void ReadDataTable (std::istream &s, double *x, double *y);
	double GetConversionFactor (SpectrumUnitType oldu, SpectrumUnitType newu);

private:
	double *x, *y;
	unsigned npoints;
	double maxx, maxy, minx, miny;
	double firstx, lastx, deltax, firsty;
	double xfactor, yfactor;
	std::vector <JdxVar> variables;
	int X, Y, R, I;
	double freq;

GCU_PROT_PROP (SpectrumView*, View)
GCU_RO_PROP (bool, Empty)
GCU_RO_PROP (SpectrumType, SpectrumType)
GCU_RO_PROP (SpectrumUnitType, XUnit)
GCU_RO_PROP (SpectrumUnitType, YUnit)
};

}

#endif	//	GCU_SPECTRUM_DOC_H
