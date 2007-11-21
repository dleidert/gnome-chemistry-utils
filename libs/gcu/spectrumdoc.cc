/* 
 * Gnome Chemisty Utils
 * spectrumdoc.cc
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

#include "config.h"
#include "application.h"
#include "spectrumdoc.h"
#include "spectrumview.h"
#include <goffice/data/go-data-simple.h>
#include <goffice/math/go-math.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <glib/gi18n-lib.h>
#include <cstring>
#include <sstream>
#include <iostream>

using namespace std;

namespace gcu
{

SpectrumDocument::SpectrumDocument (): Document (NULL), m_Empty (true)
{
	m_View = new SpectrumView (this);
	x = y = NULL;
	npoints = 0;
	maxx = maxy = minx = miny = go_nan;
	firstx = lastx = deltax = firsty = go_nan;
}

SpectrumDocument::SpectrumDocument (Application *App, SpectrumView *View):
	Document (App),
	m_Empty (true)
{
	m_View = (View)? View: new SpectrumView (this);
	x = y = NULL;
	npoints = 0;
	maxx = maxy = minx = miny = go_nan;
	firstx = lastx = deltax = firsty = go_nan;
}

SpectrumDocument::~SpectrumDocument ()
{
	if (x)
		delete[] x;
	if (y)
		delete[] y;
}

void SpectrumDocument::Load (char const *uri, char const *mime_type)
{
	// Only supporting JCAMP-DX at the moment
	if (!mime_type || strcmp (mime_type, "chemical/x-jcamp-dx")) {
		return;
	}
	GnomeVFSHandle *handle;
	GnomeVFSFileInfo *info = gnome_vfs_file_info_new ();
	GnomeVFSResult result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);
	if (result != GNOME_VFS_OK) {
		gnome_vfs_file_info_unref (info);
		return;
	}
	gnome_vfs_get_file_info_from_handle (handle, info, (GnomeVFSFileInfoOptions) 0);
	gchar *buf = new gchar[info->size + 1];
	GnomeVFSFileSize n;
	gnome_vfs_read (handle, buf, info->size, &n);
	buf[info->size] = 0;
	if (n == info->size) {
		LoadJcampDx (buf);
		if (m_App) {
			char *dirname = g_path_get_dirname (uri);
			m_App->SetCurDir (dirname);
			g_free (dirname);
		}
	}
	gnome_vfs_file_info_unref (info);
	delete [] buf;
	g_free (handle);
	
}

char const *Types[] = {
	"INFRARED SPECTRUM",
	"RAMAN SPECTRUM",
	"INFRARED PEAK TABLE",
	"INFRARED INTERFEROGRAM",
	"INFRARED TRANSFORMED SPECTRUM",
	"UV-VISIBLE SPECTRUM",
	"NMR SPECTRUM",
	"MASS SPECTRUM"
};

char const *Units[] = {
	"1/CM",
	"TRANSMITTANCE",
	"ABSORBANCE",
};

int get_spectrum_data_from_string (char const *type, char const *names[], int max)
{
	int res = 0;
	while (res < max) {
		if (!strncmp (type, names[res], strlen (names[res])))
			return res;
		res++;
	}
	return res;
}

char const *Keys[] = {
	"TITLE",
	"JCAMPDX",
	"DATATYPE",
	"DATACLASS",
	"APPLICATION",
	"DICTIONARY",
	"BLOCKS",
	"BLOCKID",
	"END",
	"XYDATA",
	"XYPOINTS",
	"XYPAIRS",
	"PEAKTABLE",
	"PEAKASSIGNMENTS",
	"RADATA",
	"XUNITS",
	"YUNITS",
	"XLABEL",
	"YLABEL",
	"XFACTOR",
	"YFACTOR",
	"FIRSTX",
	"LASTX",
	"NPOINTS",
	"FIRSTY",
	"MAXX",
	"MINX",
	"MAXY",
	"MINY",
	"RUNITS",
	"AUNITS",
	"FIRSTR",
	"LASTR",
	"MAXA",
	"MINA",
	"RFACTOR",
	"AFACTOR",
	"FIRSTA",
	"ALIAS",
	"ZPD",
	"NTUPLES",
	"VARNAME",
	"SYMBOL",
	"VARTYPE",
	"VARFORM",
	"VARDIM",
	"UNITS",
	"FIRST",
	"LAST",
	"MIN",
	"MAX",
	"FACTOR",
	"PAGE",
	"DELTAX",
	"CLASS",
	"ORIGIN",
	"OWNER",
	"DATE",
	"TIME",
	"SOURCEREFERENCE",
	"CROSSREFERENCE",
	"SPECTROMETER",
	"DATASYSTEM",
	"INSTRUMENTPARAMETERS",
	"SAMPLEDESCRIPTION",
	"CASNAME",
	"NAMES",
	"MOLEFORM",
	"CASREGISTRYNO",
	"WISWESSER",
	"BEILSTEINLAWSONNO",
	"MP",
	"BP",
	"REFRACTIVEINDEX",
	"DENSITY",
	"MW",
	"CONCENTRATIONS",
	"SAMPLINGPROCEDURE",
	"STATE",
	"PATHLENGTH",
	"PRESSURE",
	"TEMPERATURE",
	"DATAPROCESSING",
	"SPECTROMETERTYPE",
	"INLET",
	"IONIZATIONMODE",
	"INLETTEMPERATURE",
	"SOURCETEMPERATURE",
	"IONIZATIONENERGY",
	"ACCELERATINGVOLTAGE",
	"TOTALIONCURRENT",
	"ACQUISITIONRANGE",
	"DETECTOR",
	"SCANNUMBER",
	"RETENTIONTIME",
	"BASEPEAK",
	"BASEPEAKINTENSITY",
	"RIC",
	"NOMINALMASS",
	"MONOISOTOPICMASS",
	"OBSERVEFREQUENCY",
	"OBSERVENUCLEUS",
	"SOLVENTREFERENCE",
	"DELAY",
	"ACQUISITIONMODE",
	"FIELD",
	"DECOUPLER",
	"FILTERWIDTH",
	"ACQUISITIONTIME",
	"ZEROFILL",
	"AVERAGES",
	"DIGITIZERRES",
	"SPINNINGRATE",
	"PHASE0",
	"PHASE1",
	"MININTENSITY",
	"MAXINTENSITY",
	"OBSERVE90",
	"COUPLINGCONSTANTS",
	"RELAXATIONTIMES",
	NULL
};

enum {
	JCAMP_INVALID = -10,
	JCAMP_UNKNOWN,
	JCAMP_CONTINUE,
	JCAMP_TITLE = 0,
	JCAMP_JCAMP_DX,
	JCAMP_DATA_TYPE,
	JCAMP_DATACLASS,
	JCAMP_APPLICATION,
	JCAMP_DICTIONARY,
	JCAMP_BLOCKS,
	JCAMP_BLOCK_ID,
	JCAMP_END,
	JCAMP_XYDATA,
	JCAMP_XYPOINTS,
	JCAMP_XYPAIRS,
	JCAMP_PEAK_TABLE,
	JCAMP_PEAK_ASSIGNMENTS,
	JCAMP_RADATA,
	JCAMP_XUNITS,
	JCAMP_YUNITS,
	JCAMP_XLABEL,
	JCAMP_YLABEL,
	JCAMP_XFACTOR,
	JCAMP_YFACTOR,
	JCAMP_FIRSTX,
	JCAMP_LASTX,
	JCAMP_NPOINTS,
	JCAMP_FIRSTY,
	JCAMP_MAXX,
	JCAMP_MINX,
	JCAMP_MAXY,
	JCAMP_MINY,
	JCAMP_RUNITS,
	JCAMP_AUNITS,
	JCAMP_FIRSTR,
	JCAMP_LASTR,
	JCAMP_MAXA,
	JCAMP_MINA,
	JCAMP_RFACTOR,
	JCAMP_AFACTOR,
	JCAMP_FIRSTA,
	JCAMP_ALIAS,
	JCAMP_ZPD,
	JCAMP_NTUPLES,
	JCAMP_VAR_NAME,
	JCAMP_SYMBOL,
	JCAMP_VAR_TYPE,
	JCAMP_VAR_FORM,
	JCAMP_VAR_DIM,
	JCAMP_UNITS,
	JCAMP_FIRST,
	JCAMP_LAST,
	JCAMP_MIN,
	JCAMP_MAX,
	JCAMP_FACTOR,
	JCAMP_PAGE,
	JCAMP_DELTAX,
	JCAMP_CLASS,
	JCAMP_ORIGIN,
	JCAMP_OWNER,
	JCAMP_DATE,
	JCAMP_TIME,
	JCAMP_SOURCE_REFERENCE,
	JCAMP_CROSS_REFERENCE,
	JCAMP_SPECTROMETER,
	JCAMP_DATASYSTEM,
	JCAMP_INSTRUMENT_PARAMETERS,
	JCAMP_SAMPLE_DESCRIPTION,
	JCAMP_CAS_NAME,
	JCAMP_NAMES,
	JCAMP_MOLEFORM,
	JCAMP_CAS_REGISTRY_NO,
	JCAMP_WISWESSER,
	JCAMP_BEILSTEIN_LAWSON_NO,
	JCAMP_MP,
	JCAMP_BP,
	JCAMP_REFRACTIVE_INDEX,
	JCAMP_DENSITY,
	JCAMP_MW,
	JCAMP_CONCENTRATIONS,
	JCAMP_SAMPLING_PROCEDURE,
	JCAMP_STATE,
	JCAMP_PATH_LENGTH,
	JCAMP_PRESSURE,
	JCAMP_TEMPERATURE,
	JCAMP_DATA_PROCESSING,
	JCAMP_SPECTROMETER_TYPE,
	JCAMP_INLET,
	JCAMP_IONIZATION_MODE,
	JCAMP_INLET_TEMPERATURE,
	JCAMP_SOURCE_TEMPERATURE,
	JCAMP_IONIZATION_ENERGY,
	JCAMP_ACCELERATING_VOLTAGE,
	JCAMP_TOTAL_ION_CURRENT,
	JCAMP_ACQUISITION_RANGE,
	JCAMP_DETECTOR,
	JCAMP_SCAN_NUMBER,
	JCAMP_RETENTION_TIME,
	JCAMP_BASE_PEAK,
	JCAMP_BASE_PEAK_INTENSITY,
	JCAMP_RIC,
	JCAMP_NOMINAL_MASS,
	JCAMP_MONOISOTOPIC_MASS,
	JCAMP_OBSERVE_FREQUENCY,
	JCAMP_OBSERVE_NUCLEUS,
	JCAMP_SOLVENT_REFERENCE,
	JCAMP_DELAY,
	JCAMP_ACQUISITION_MODE,
	JCAMP_FIELD,
	JCAMP_DECOUPLER,
	JCAMP_FILTER_WIDTH,
	JCAMP_ACQUISITION_TIME,
	JCAMP_ZERO_FILL,
	JCAMP_AVERAGES,
	JCAMP_DIGITIZER_RES,
	JCAMP_SPINNING_RATE,
	JCAMP_PHASE_0,
	JCAMP_PHASE_1,
	JCAMP_MIN_INTENSITY,
	JCAMP_MAX_INTENSITY,
	JCAMP_OBSERVE_90,
	JCAMP_COUPLING_CONSTANTS,
	JCAMP_RELAXATION_TIMES,
};

#define KEY_LENGTH 80
#define VALUE_LENGTH 128
int ReadField (char const *s, char *key, char *buf)
{
	char const *data = s, *eq;
	int i = 0;
	while (*data != 0 && *data < '#')
		data++;
	if (*data == 0)
		return JCAMP_INVALID;
	if (!strncmp (data, "$$", 2)) {
		// This is a comment, just skip
		return JCAMP_INVALID;
	}
	if (!strncmp (data, "##", 2)) {
		// found a field
		data += 2;
		eq = strchr (data, '=');
		i = 0;
		while (data < eq) {
			if (*data >= 'A' && *data <= 'Z')
				key[i++] = *data;
			else if (*data >= 'a' && *data <= 'z')
				key[i++] = *data - 0x20;
			data++;
		}
		key[i] = 0;
		if (i == 0)
			// This is a comment, just skip
			return JCAMP_INVALID;
		data = eq + 1;
		while (*data != 0 && *data < '$')
			data++;
		eq = strstr (data, "$$");
		if (eq && *eq) {
			strncpy (buf, data, MIN (eq - data, VALUE_LENGTH));
			buf[MIN (eq - data, VALUE_LENGTH - 1)] = 0;
		}
		else 
			strncpy (buf, data, VALUE_LENGTH);
		// strip trailing white spaces:
		i = strlen (buf);
		while (buf[i] <= ' ' || buf[i] == '"')
			buf[i--] = 0;
		
		// Now, get the key value
		i = JCAMP_TITLE;
		while (i <= JCAMP_RELAXATION_TIMES && strcmp (key, Keys[i]))
			i++;
		return i;
	}
	return JCAMP_UNKNOWN;
}

#define JCAMP_PREC 1e-5 // fully arbitrary

void SpectrumDocument::LoadJcampDx (char const *data)
{
	char key[KEY_LENGTH];
	char buf[VALUE_LENGTH];
	char line[300]; // should be enough
	int n;
	deltax = 0.;
	istringstream s(data);
	while (!s.eof ()) {
		s.getline (line, 300);
		n = ReadField (line, key, buf);
		switch (n) {
		case JCAMP_TITLE:
			SetTitle (buf);
			break;
		case JCAMP_JCAMP_DX:
			break;
		case JCAMP_DATA_TYPE:
			m_SpectrumType = (SpectrumType) get_spectrum_data_from_string (buf, Types, GCU_SPECTRUM_MAX);
			break;
		case JCAMP_DATACLASS:
		case JCAMP_APPLICATION:
		case JCAMP_DICTIONARY:
		case JCAMP_BLOCKS:
		case JCAMP_BLOCK_ID:
			break;
		case JCAMP_END:
			goto out;
		case JCAMP_XYDATA:
		case JCAMP_XYPOINTS: {
			unsigned read = 0;
			list<double> l;
			if (deltax == 0.)
				deltax = (lastx - firstx) / (npoints - 1);
			else {
				if (firstx > lastx && deltax > 0)
					deltax = -deltax;
				unsigned n = (unsigned) ((lastx - firstx) / deltax + .1) + 1;
				if (n && n != npoints) {
					if (x)
						delete[] x;
					x = new double[n];
					if (y)
						delete[] y;
					y = new double[n];
				}
				npoints = n;
			}
			// FIXME: we should implement a real parser for this value
			if (!strncmp (buf, "(X++(Y..Y))",strlen ("(X++(Y..Y))"))) {
				while (1) {
					if (s.eof ())
						break;	// this should not occur, but a corrupted or bad file is always possible
					s.getline (line, 300);
					if (strstr (line, "##")) {
						s.seekg (-strlen (line) -1, _S_cur);
						if (read > npoints) {
							g_warning (_("Found too many data!"));
							// FIXME: throw an exception
						} else
							npoints = read;
						break;
					}
					ReadDataLine (line, l);
					if (l.empty ())
						continue;
					list<double>::iterator i = l.begin (), end = l.end ();
					if (read > 0) {
						double x0 = firstx + deltax * read, x1 = (*i) * xfactor;
						if (fabs (x0 - x1) < fmax (fabs (deltax), fabs (x0)) * JCAMP_PREC) {
							// values are the same, no y reminder, and nothing to do
						} else if (fabs (x[read - 1] - x1) < fmax (fabs (deltax), fabs (x1)) * JCAMP_PREC) {
							i++;
							double y0 = (*i) * yfactor;
							if (fabs (y0 - y[read - 1]) > fmax (fabs (y0), fabs (y[read - 1])) * JCAMP_PREC)
							g_warning (_("Data check failed!"));
						} else if ((x0 - x1) * deltax < 0.) {
							unsigned missing = (unsigned) round ((x1 - x0) / deltax), n;
							for (n = 0; n < missing; n++) {
								if (read > npoints) // FIXME: Throw an exception
									;
								x[read] = firstx + deltax * read;
								y[read++] = go_nan;
							}
						} else {
							// FIXME: duplicate values, throw an exception
						}
					} else {
						x[read] = (*i) * xfactor;
						if (fabs (x[0] - firstx) > fabs (deltax * JCAMP_PREC)) {
							g_warning (_("Data check failed: FIRSTX!"));
							firstx = x[0]; // WARNING: hope that deltax is good
						}
						i++;
						y[read++] = (*i) * yfactor;
						if (fabs (firsty - y[0]) > fmax (fabs (firsty), fabs (y[0])) * JCAMP_PREC)
							g_warning (_("Data check failed!"));
					}
					for (i++; i !=	end; i++) {
						if (read > npoints) // FIXME: Throw an exception
							;
						x[read] = firstx + deltax * read;
						y[read++] = (*i) * yfactor;
					}
					l.clear ();
				}
				while (npoints > read) {
					// this should never occur, fill missing y values with nan
					x[read] = minx + deltax * read;
					y[read++] = go_nan;
				}
				if (isnan (maxx)) {
					maxx = MAX (firstx, lastx);
					minx = MIN (firstx, lastx);
				}
			} else if (!strncmp (buf, "(XY..XY)",strlen ("(XY..XY)"))) {
				while (1) {
					if (s.eof ())
						break;	// this should not occur, but a corrupted or bad file is always possible
					s.getline (line, 300);
					if (strstr (line, "##")) {
						s.seekg (-strlen (line) -1, _S_cur);
						if (read > npoints) {
							g_warning (_("Found too many data!"));
							// FIXME: throw an exception
						} else
							npoints = read;
						break;
					}
					if (sscanf (line, "%lg %lg\n", x + read, y + read) != 2)
						g_warning (_("Invalid line!"));
					read++;
				}
			}
			break;
		}
		case JCAMP_XYPAIRS:
		case JCAMP_PEAK_TABLE:
		case JCAMP_PEAK_ASSIGNMENTS:
		case JCAMP_RADATA:
		case JCAMP_XUNITS:
		case JCAMP_YUNITS:
		case JCAMP_XLABEL:
		case JCAMP_YLABEL:
			break;
		case JCAMP_XFACTOR:
			xfactor = strtod (buf, NULL);
			break;
		case JCAMP_YFACTOR:
			yfactor = strtod (buf, NULL);
			break;
		case JCAMP_FIRSTX:
			firstx = strtod (buf, NULL);
			break;
		case JCAMP_LASTX:
			lastx = strtod (buf, NULL);
			break;
		case JCAMP_NPOINTS: {
			unsigned n = (unsigned) atoi (buf);
			if (n && n != npoints) {
				if (x)
					delete[] x;
				x = new double[n];
				if (y)
					delete[] y;
				y = new double[n];
			}
			npoints = n;
			break;
		}
		case JCAMP_FIRSTY:
			firsty = strtod (buf, NULL);
			break;
		case JCAMP_MAXX:
			maxx = strtod (buf, NULL);
			break;
		case JCAMP_MINX:
			minx = strtod (buf, NULL);
			break;
		case JCAMP_MAXY:
			maxy = strtod (buf, NULL);
			break;
		case JCAMP_MINY:
			miny = strtod (buf, NULL);
			break;
		case JCAMP_RUNITS:
		case JCAMP_AUNITS:
		case JCAMP_FIRSTR:
		case JCAMP_LASTR:
		case JCAMP_MAXA:
		case JCAMP_MINA:
		case JCAMP_RFACTOR:
		case JCAMP_AFACTOR:
		case JCAMP_FIRSTA:
		case JCAMP_ALIAS:
		case JCAMP_ZPD:
		case JCAMP_NTUPLES:
		case JCAMP_VAR_NAME:
		case JCAMP_SYMBOL:
		case JCAMP_VAR_TYPE:
		case JCAMP_VAR_FORM:
		case JCAMP_VAR_DIM:
		case JCAMP_UNITS:
		case JCAMP_FIRST:
		case JCAMP_LAST:
		case JCAMP_MIN:
		case JCAMP_MAX:
		case JCAMP_FACTOR:
		case JCAMP_PAGE:
			break;
		case JCAMP_DELTAX:
			deltax = strtod (buf, NULL);
			break;
		case JCAMP_CLASS:
		case JCAMP_ORIGIN:
		case JCAMP_OWNER:
		case JCAMP_DATE:
		case JCAMP_TIME:
		case JCAMP_SOURCE_REFERENCE:
		case JCAMP_CROSS_REFERENCE:
		case JCAMP_SPECTROMETER:
		case JCAMP_DATASYSTEM:
		case JCAMP_INSTRUMENT_PARAMETERS:
		case JCAMP_SAMPLE_DESCRIPTION:
		case JCAMP_CAS_NAME:
		case JCAMP_NAMES:
		case JCAMP_MOLEFORM:
		case JCAMP_CAS_REGISTRY_NO:
		case JCAMP_WISWESSER:
		case JCAMP_BEILSTEIN_LAWSON_NO:
		case JCAMP_MP:
		case JCAMP_BP:
		case JCAMP_REFRACTIVE_INDEX:
		case JCAMP_DENSITY:
		case JCAMP_MW:
		case JCAMP_CONCENTRATIONS:
		case JCAMP_SAMPLING_PROCEDURE:
		case JCAMP_STATE:
		case JCAMP_PATH_LENGTH:
		case JCAMP_PRESSURE:
		case JCAMP_TEMPERATURE:
		case JCAMP_DATA_PROCESSING:
		case JCAMP_SPECTROMETER_TYPE:
		case JCAMP_INLET:
		case JCAMP_IONIZATION_MODE:
		case JCAMP_INLET_TEMPERATURE:
		case JCAMP_SOURCE_TEMPERATURE:
		case JCAMP_IONIZATION_ENERGY:
		case JCAMP_ACCELERATING_VOLTAGE:
		case JCAMP_TOTAL_ION_CURRENT:
		case JCAMP_ACQUISITION_RANGE:
		case JCAMP_DETECTOR:
		case JCAMP_SCAN_NUMBER:
		case JCAMP_RETENTION_TIME:
		case JCAMP_BASE_PEAK:
		case JCAMP_BASE_PEAK_INTENSITY:
		case JCAMP_RIC:
		case JCAMP_NOMINAL_MASS:
		case JCAMP_MONOISOTOPIC_MASS:
		case JCAMP_OBSERVE_FREQUENCY:
		case JCAMP_OBSERVE_NUCLEUS:
		case JCAMP_SOLVENT_REFERENCE:
		case JCAMP_DELAY:
		case JCAMP_ACQUISITION_MODE:
		case JCAMP_FIELD:
		case JCAMP_DECOUPLER:
		case JCAMP_FILTER_WIDTH:
		case JCAMP_ACQUISITION_TIME:
		case JCAMP_ZERO_FILL:
		case JCAMP_AVERAGES:
		case JCAMP_DIGITIZER_RES:
		case JCAMP_SPINNING_RATE:
		case JCAMP_PHASE_0:
		case JCAMP_PHASE_1:
		case JCAMP_MIN_INTENSITY:
		case JCAMP_MAX_INTENSITY:
		case JCAMP_OBSERVE_90:
		case JCAMP_COUPLING_CONSTANTS:
		case JCAMP_RELAXATION_TIMES:
		default:
			break;
		}
	}

out:
	m_Empty = npoints == 0;
	GOData *godata;
	GogSeries *series = m_View->GetSeries ();
	godata = go_data_vector_val_new (x, npoints, NULL);
	gog_series_set_dim (series, 0, godata, NULL);
	godata = go_data_vector_val_new (y, npoints, NULL);
	gog_series_set_dim (series, 1, godata, NULL);
	m_View->SetXAxisBounds (minx, maxx, true); // FIXME only invert if needed
}

void SpectrumDocument::ReadDataLine (char const *data, list<double> &l)
{
	int i = 1, j, max = strlen (data);
	char buf[32], c = data[0];
	double val = 0., newval = 0.;
	bool pos, diff = false;
	char const *eq = strstr (data, "$$");
	if (eq)
		max = eq - data + 1;
	pos = true;
	while (i < max) {
		switch (c) {
		case ' ':
			c = data[i++];
			continue;
		case '-':
			pos = false;
		case '+':
			c = data[i++];
			if ((c < '0' || c > '9') && c != '.') // FIXME: throw an exception
				;
			continue;
		case '.':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			buf[0] = c;
			diff = false;
			break;
		case '@':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
			buf[0] = c - 0x10;
			diff = false;
			break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
			pos = false;
			diff = false;
			buf[0] = c - 0x30;
			break;
		case '%':
			c = 'I';
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
			diff = true;
			buf[0] = c - 0x19;
			break;
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
			pos = false;
			diff = true;
			buf[0] = c - 0x39;
			break;
		case 's':
			c = '[';
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z': {
			buf[0] = c - 0x22;
			j = 1;
			while (c = data[i++], (c >= '0' && c <= '9')) {
				if (j == 31) {
					g_warning (_("Constant too long"));
					break;
				}
				buf[j++] = c;
			}
			buf[j] = 0;
			int m, n = atoi (buf);
			for (m = 1; m < n; m++) {
				if (diff)
					val += newval;
				l.push_back (val);
			}
			continue;
		}
		case '?':
			diff = false;
			val = go_nan;
			newval = 0.;
			l.push_back (go_nan);
			c = data[i++];
			continue;
		default:
			if (c > ' ')
				g_warning (_("Invalid character in data block"));
			c = data[i++];
			continue;
		}
		j = 1;
		while (c = data[i++], (c >= '0' && c <= '9') || c == '.') {
			if (j == 31) {
				g_warning (_("Constant too long"));
				break;
			}
			buf[j++] = c;
		}
		buf[j] = 0;
		newval = strtod (buf, NULL);
		if (!pos)
			newval = - newval;
		if (diff)
			val += newval;
		else
			val = newval;
		l.push_back (val);
		pos = true;
	}
}

}	//	nampespace gcu
