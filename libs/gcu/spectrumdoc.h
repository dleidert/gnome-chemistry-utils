/* 
 * Gnome Chemisty Utils
 * spectrumdoc.h
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

#ifndef GCU_SPECTRUM_DOC_H
#define GCU_SPECTRUM_DOC_H

#include "document.h"
#include "macros.h"
#include "printable.h"
#include <string>
#include <vector>

/*!\file*/
namespace gcu
{

/*!\enum SpectrumType gcu/spectrumdoc.h
Represents the list of spectrum types supported (or which should be supported)
by the gcu::SpectrumDocument class.
*/
typedef enum {
/*!
Infrared spectrum.
*/
	GCU_SPECTRUM_INFRARED,
/*!
Raman spectrum.
*/
	GCU_SPECTRUM_RAMAN,
/*!
Infrared peaks table.
*/
	GCU_SPECTRUM_INFRARED_PEAK_TABLE,
/*!
Infrared iterferogram.
*/
	GCU_SPECTRUM_INFRARED_INTERFEROGRAM,
/*!
Transformed infrared spectrum.
*/
	GCU_SPECTRUM_INFRARED_TRANSFORMED,
/*!
UV-visible spectrum.
*/
	GCU_SPECTRUM_UV_VISIBLE,
/*!
NMR spectrum.
*/
	GCU_SPECTRUM_NMR,
/*!
NMR FID.
*/
	GCU_SPECTRUM_NMR_FID,
/*!
NMR peaks table.
*/
	GCU_SPECTRUM_NMR_PEAK_TABLE,
/*!
NMR peaks assignments.
*/
	GCU_SPECTRUM_NMR_PEAK_ASSIGNMENTS,
/*!
Mass spectrum.
*/
	GCU_SPECTRUM_MASS,
/*!
Last known value. This value does not represent a valid type, but is
equal to the number of supported types.
*/
	GCU_SPECTRUM_MAX
} SpectrumType;


/*!\enum SpectrumUnitType gcu/spectrumdoc.h
Represents the list of units supported by the gcu::SpectrumDocument class.
*/
typedef enum {
/*!
Wave number per cm.
*/
	GCU_SPECTRUM_UNIT_CM_1,
/*!
Transmittance
*/
	GCU_SPECTRUM_UNIT_TRANSMITTANCE,
/*!
Absorbance
*/
	GCU_SPECTRUM_UNIT_ABSORBANCE,
/*!
Part per million.
*/
	GCU_SPECTRUM_UNIT_PPM,
/*!
Nanometers
*/
	GCU_SPECTRUM_UNIT_NANOMETERS,
/*!
Micrometers
*/
	GCU_SPECTRUM_UNIT_MICROMETERS,
/*!
Seconds
*/
	GCU_SPECTRUM_UNIT_SECONDS,
/*!
Hz.
*/
	GCU_SPECTRUM_UNIT_HZ,
/*!
M/Z (for mass spectra).
*/
	GCU_SPECTRUM_UNIT_M_Z,
/*!
Relative abundance.
*/
	GCU_SPECTRUM_UNIT_REL_ABUNDANCE,
/*!
Last known value. This value does not represent a valid type, but is
equal to the number of supported units.
*/
	GCU_SPECTRUM_UNIT_MAX
} SpectrumUnitType;

/*!\enum SpectrumVarType gcu/spectrumdoc.h
Represents the list of varialble types supported by the
gcu::SpectrumDocument class.
*/
typedef enum {
/*!
Independent variable.
*/
	GCU_SPECTRUM_TYPE_INDEPENDENT,
/*!
Dependent variable.
*/
	GCU_SPECTRUM_TYPE_DEPENDENT,
/*!
Page number.
*/
	GCU_SPECTRUM_TYPE_PAGE,
/*!
Last known value. This value does not represent a valid type, but is
equal to the number of supported types.
*/
	GCU_SPECTRUM_TYPE_MAX
} SpectrumVarType;

/*!\enum SpectrumFormatType gcu/spectrumdoc.h
Represents the list of data formats from the JCAMP-DX specification supported
by the gcu::SpectrumDocument class. See the JCAMP-DX specification for
more information.
*/
typedef enum {
/*!
ASCII squeezed difference form.
*/
	GCU_SPECTRUM_FORMAT_ASDF,
/*!
ASCII free format numeric.
*/
	GCU_SPECTRUM_FORMAT_AFFN,
/*!
Packed form.
*/
	GCU_SPECTRUM_FORMAT_PAC,
/*!
Squeezed form.
*/
	GCU_SPECTRUM_FORMAT_SQZ,
/*!
Difference form.
*/
	GCU_SPECTRUM_FORMAT_DIF,
/*!
Last known value. This value does not represent a valid type, but is
equal to the number of supported formats.
*/
	GCU_SPECTRUM_FORMAT_MAX
} SpectrumFormatType;

class Application;
class SpectrumView;

/*!\struct JdxVar gcu/spectrumdoc.h
Used to store a series of data.
*/
typedef struct  {
/*!
The name of the series.
*/
	std::string Name;
/*!
The symbol associated to the series.
*/
	char Symbol;
/*!
The SpectrumVarType of the data.
*/
	SpectrumVarType Type;
/*!
The unit stored as a SpectrumUnitType value.
*/
	SpectrumUnitType Unit;
/*!
The format of the data when loaded from a JCAMP-DX file.
*/
	SpectrumFormatType Format;
/*!
The values number.
*/
	unsigned NbValues;
/*!
First vaue in the series.
*/
	double First;
/*!
Last value in the series 
*/
	double Last;
/*!
Smallest value in  the series. 
*/
	double Min;
/*!
Largest value in the series. 
*/
	double Max;
/*!
Constant value by which each value in the series must be multiplied after
loading from a JCAMP-DX file in order to get the real value.
*/
	double Factor;
/*!
The array of values.
*/
	double *Values;
/*!
The GogSeries used in the chart. See the GOffice documentation for more
information.
*/
	GogSeries *Series;
} JdxVar;

/*!\class SpectrumDocument gcu/spectrumdoc.h
The document class used for spectra. This API is still quite unstable and
might change in the future.
*/
class SpectrumDocument: public Document, public Printable
{
public:
/*!
Default constructor
*/
	SpectrumDocument ();
/*!
@param app the application.
@param view an optional already existing SpectrumView instance.
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
@param i the rank of the unit to set.

Called to change the displayed X axis unit. The values acceptable for the
parameter are spectrum type dependent. This is used by the framework after
a user made an appropriate request from the graphical interface.
*/
	void OnXUnitChanged (int i);

/*!
@param i the rank of the unit to set.

Called to change the displayed Y axis unit. The values acceptable for the
parameter are spectrum type dependent. This is used by the framework after
a user made an appropriate request from the graphical interface.
*/
	void OnYUnitChanged (int i);

/*!
@param inverted whether to invert the X axis scale.

Called to change the X axis scale order. This is used by the framework after
a user made an appropriate request from the graphical interface.
*/
	void OnXAxisInvert (bool inverted);

/*!
Used to show or hide the integral curve for an NMR spectrum.
*/
	void OnShowIntegral ();

/*!
@param btn the clicked GtkButton.

Called to transform an FID to the NMR spectrum. The label in the GtkButton
might be changed.
*/
	void OnTransformFID (GtkButton *btn);
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set properties to spectra.
@return true if the property could be set, or if the property is not relevant, false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);
/*!
Called by the application when the document has been loaded.
*/
	bool Loaded () throw (gcu::LoaderError);

private:
	void LoadJcampDx (char const *data);
	void ReadDataLine (char const *data, std::list<double> &l);
	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context, int page) const;
	GtkWindow *GetGtkWindow ();
	void ReadDataTable (std::istream &s, double *x, double *y);
	double (*GetConversionFunction (SpectrumUnitType oldu, SpectrumUnitType newu, double &factor, double &offset)) (double, double, double);

private:
	double *x, *y;
	unsigned npoints;
	double maxx, maxy, minx, miny;
	double firstx, lastx, deltax, firsty;
	double xfactor, yfactor;
	std::vector <JdxVar> variables;
	int X, Y, R, I, integral, Rt, It, Rp;
	double freq;
	double offset, refpoint;
	GtkWidget *m_XAxisInvertBtn;
	guint m_XAxisInvertSgn;

/*!\var m_View
The SpectrumView instance associated with the document.
*/
/*!\fn GetView()
@return the SpectrumView instance associated with the document.
*/
GCU_PROT_PROP (SpectrumView*, View)
/*!\fn GetEmpty()
@return true if the document does not have any data, false otherwise.
*/
GCU_RO_PROP (bool, Empty)
/*!\fn GetSpectrumType()
@return the gcu::SpectrumType of the document, or GCU_SPECTRUM_TYPE_MAX.
*/
GCU_RO_PROP (SpectrumType, SpectrumType)
/*!\fn GetXUnit()
@return the unit of the x-axis as gcu::SpectumUnitType, or
GCU_SPECTRUM_UNIT_MAX.
*/
GCU_RO_PROP (SpectrumUnitType, XUnit)
/*!\fn GetYUnit()
@return the unit of the y-axis as gcu::SpectumUnitType, or
GCU_SPECTRUM_UNIT_MAX.
*/
GCU_RO_PROP (SpectrumUnitType, YUnit)
/*!\fn GetIntegralVisible()
@return true if the integral of an NMR spectrum is visible, false in all
other cases.
*/
GCU_RO_PROP (bool, IntegralVisible)
};

}

#endif	//	GCU_SPECTRUM_DOC_H
