/*
 * Gnome Chemisty Utils
 * spectrumdoc.cc
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include <gcu/objprops.h>
#include <glib/gi18n-lib.h>
#include <cstring>
#include <sstream>
#include <iostream>

#ifndef HAVE_EXP10
#	define exp10(x) pow(10.,(x))
#endif

using namespace std;

namespace gcugtk
{

SpectrumDocument::SpectrumDocument ():
	gcu::Document (NULL),
	Printable (),
	m_XAxisInvertBtn (NULL),
	m_Empty (true)
{
	m_View = new SpectrumView (this);
	x = y = NULL;
	X = Y = R = I = integral = Rt = It = Rp = -1;
	npoints = 0;
	maxx = maxy = minx = miny = go_nan;
	firstx = lastx = deltax = firsty = go_nan;
	freq = offset = refpoint = go_nan;
	gtk_page_setup_set_orientation (GetPageSetup (), GTK_PAGE_ORIENTATION_LANDSCAPE);
	SetScaleType (GCU_PRINT_SCALE_AUTO);
	SetHorizFit (true);
	SetVertFit (true);
	m_IntegralVisible = false;
}

SpectrumDocument::SpectrumDocument (Application *App, SpectrumView *View):
	Document (App),
	Printable (),
	m_XAxisInvertBtn (NULL),
	m_Empty (true)
{
	m_View = (View)? View: new SpectrumView (this);
	x = y = NULL;
	X = Xt = Y = R = I = integral = Rt = It = Rp = -1;
	npoints = 0;
	maxx = maxy = minx = miny = go_nan;
	firstx = lastx = deltax = firsty = go_nan;
	freq = offset = refpoint = go_nan;
	gtk_page_setup_set_orientation (GetPageSetup (), GTK_PAGE_ORIENTATION_LANDSCAPE);
	SetScaleType (GCU_PRINT_SCALE_AUTO);
	SetHorizFit (true);
	SetVertFit (true);
	m_IntegralVisible = false;
}

SpectrumDocument::~SpectrumDocument ()
{
	if (x && X < 0)
		delete[] x;
	if (y && Y < 0)
		delete[] y;
	for (unsigned i = 0; i < variables.size (); i++)
		if (variables[i].Values)
			delete [] variables[i].Values;
	if (m_View)
		delete m_View;
}

void SpectrumDocument::Load (char const *uri, char const *mime_type)
{
	// Only supporting JCAMP-DX at the moment
	if (!mime_type || strcmp (mime_type, "chemical/x-jcamp-dx")) {
		return;
	}
	GVfs *vfs = g_vfs_get_default ();
	GFile *file = g_vfs_get_file_for_uri (vfs, uri);
	GError *error = NULL;
	GFileInfo *info = g_file_query_info (file,
										 G_FILE_ATTRIBUTE_STANDARD_SIZE,
										 G_FILE_QUERY_INFO_NONE,
										 NULL, &error);
	if (error) {
		g_message ("GIO querry failed: %s", error->message);
		g_error_free (error);
		g_object_unref (file);
		return;
	}
	gsize size = g_file_info_get_size (info);
	g_object_unref (info);
	GInputStream *input = G_INPUT_STREAM (g_file_read (file, NULL, &error));
	if (error) {
		g_message ("GIO could not create the stream: %s", error->message);
		g_error_free (error);
		g_object_unref (file);
		return;
	}
	char *buf = new char[size + 1];
	gsize n = size;
	while (n) {
		n -= g_input_stream_read (input, buf, size, NULL, &error);
		if (error) {
			g_message ("GIO could not read the file: %s", error->message);
			g_error_free (error);
			delete [] buf;
			g_object_unref (input);
			g_object_unref (file);
			return;
		}
	}
	buf[size] = 0;
	LoadJcampDx (buf);
	if (m_App) {
		char *dirname = g_path_get_dirname (uri);
		m_App->SetCurDir (dirname);
		g_free (dirname);
	}
	delete [] buf;
	g_object_unref (file);

}

struct data_type_struct {
	int n; // number of variables
	bool comma_sep; // whether the separator is a comma
	bool one_per_line; // whether each line has only one data set
	bool explicit_indep; // whether the independent variable is explicit
	char *variables; // each letter is a variable symbol, must be freed
};

static void parse_data_type (char const *type, struct data_type_struct &s)
{
	// initialize s
	s.variables = NULL; // if still NULL when returning, an error occurred
	s.n = 0;
	s.one_per_line = false;
	s.comma_sep = false;
	s.explicit_indep = true;
	string variables;
	if (*type != '(')
		return;
	type++;
	if (*type < 'A' || *type > 'Z')
		return;
	variables += *type; //first variable symbol
	type++;
	switch (*type) {
	case ',':
		s.comma_sep = true;
		type++;
		break;
	case '+':
		if (type[1] != '+')
			return;
		s.explicit_indep = false;
		type += 2;
		while (*type == '(' || *type == ')')
			type++;
		break;
	default:
		break;
	}
	// at this point *type should be the second variable symbol
	if (*type < 'A' || *type > 'Z')
		return;
	variables += *type; //second variable symbol
	type++;
	// add additional variables if any
	while (*type == ',' || (*type >= 'A' && *type <= 'Z')) {
		if (*type != ',')
			variables += *type;
		type++;
	}
	while (*type == ' ')
		type++;
	s.one_per_line = !(*type == '.');
	s.n = variables.length ();
	if (s.n > 0)
		s.variables = g_strdup (variables.c_str ());
}

static struct {char const *name; SpectrumType type;} Types[] = {
	{"INFRARED SPECTRUM", GCU_SPECTRUM_INFRARED},
	{"RAMAN SPECTRUM", GCU_SPECTRUM_RAMAN},
	{"INFRARED PEAK TABLE", GCU_SPECTRUM_INFRARED_PEAK_TABLE},
	{"INFRARED INTERFEROGRAM", GCU_SPECTRUM_INFRARED_INTERFEROGRAM},
	{"INFRARED TRANSFORMED SPECTRUM", GCU_SPECTRUM_INFRARED_TRANSFORMED},
	{"UV-VISIBLE SPECTRUM", GCU_SPECTRUM_UV_VISIBLE},
	{"NMR SPECTRUM", GCU_SPECTRUM_NMR},
	{"NMR FID", GCU_SPECTRUM_NMR_FID},
	{"NMR PEAK TABLE", GCU_SPECTRUM_NMR_PEAK_TABLE},
	{"NMR PEAK ASSIGNMENTS", GCU_SPECTRUM_NMR_PEAK_ASSIGNMENTS},
	{"MASS SPECTRUM", GCU_SPECTRUM_MASS},
	{"UV-VIS SPECTRUM", GCU_SPECTRUM_UV_VISIBLE},
	{"UV/VISIBLE SPECTRUM", GCU_SPECTRUM_UV_VISIBLE},
	{"UV/VIS SPECTRUM", GCU_SPECTRUM_UV_VISIBLE}
};

char const *Units[] = {
	"1/CM",
	"TRANSMITTANCE",
	"ABSORBANCE",
	"PPM",
	"NANOMETERS",
	"MICROMETERS",
	"SECONDS",
	"HZ",
	"M/Z",
	"RELATIVE ABUNDANCE",
	"RF"
};

static struct {char const *name; SpectrumUnitType unit;} OtherUnits[] = {
	{"NM", GCU_SPECTRUM_UNIT_NANOMETERS}
};

char const *UnitNames[] = {
	N_("Wavenumber (cm<sup>−1</sup>)"),
	N_("Transmittance"),
	N_("Absorbance"),
	N_("Chemical shift (ppm)"),
	N_("Wavelength (nm)"),
	N_("Wavelength (µm)"),
	N_("Time (s)"),
	N_("Frequency (Hz)"),
	N_("Mass/charge ratio"),
	N_("Relative abundance"),
	N_("Response factor")
};

char const *VarTypes[] = {
	"INDEPENDENT",
	"DEPENDENT",
	"PAGE"
};

char const *Formats[] = {
	"ASDF",
	"AFFN",
	"PAC",
	"SQZ",
	"DIF"
};

int get_spectrum_data_from_string (char const *type, char const *names[], int max)
{
	int res = 0;
	char *up = g_ascii_strup (type, -1);
	while (res < max) {
		if (!strncmp (up, names[res], strlen (names[res]))) {
			g_free (up);
			return res;
		}
		res++;
	}
	g_free (up);
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
	"DATATABLE",
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
/* Brücker specific data */
	"$OFFSET",
	"$REFERENCEPOINT",
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
	JCAMP_DATA_TABLE,
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
	BRUCKER_OFFSET,
	VARIAN_OFFSET,
	JCAMP_MAX_VALID
};

#define KEY_LENGTH 80
#define VALUE_LENGTH 128
static int ReadField (char const *s, char *key, char *buf)
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
/*	if (!strncmp (data, "##$", 3)) {
		// This is a vendor specific tag, just skip for now
		return JCAMP_INVALID;
	}*/
	if (!strncmp (data, "##", 2)) {
		// found a field
		data += 2;
		eq = strchr (data, '=');
		i = 0;
		if (*data == '$' && strncmp (data, "$$", 2)) {
			key[i++] = '$';
			data++;
		}
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
		else {
			strncpy (buf, data, VALUE_LENGTH);
			buf[VALUE_LENGTH - 1] = 0;
		}
		// strip trailing white spaces:
		i = strlen (buf) - 1;
		if (i < 0)
			return JCAMP_UNKNOWN;
		while (buf[i] <= ' ' || buf[i] == '"')
			buf[i--] = 0;

		// Now, get the key value
		i = JCAMP_TITLE;
		while (i < JCAMP_MAX_VALID && strcmp (key, Keys[i]))
			i++;
		return i;
	}
	return JCAMP_UNKNOWN;
}

static void on_xunit_changed (GtkComboBox *box, SpectrumDocument *doc)
{
	doc->OnXUnitChanged (gtk_combo_box_get_active (box));
}

static void on_yunit_changed (GtkComboBox *box, SpectrumDocument *doc)
{
	doc->OnYUnitChanged (gtk_combo_box_get_active (box));
}

static void on_xaxis_invert (GtkToggleButton *btn, SpectrumDocument *doc)
{
	doc->OnXAxisInvert (gtk_toggle_button_get_active (btn));
}

static void on_show_integral (GtkButton *btn, SpectrumDocument *doc)
{
	gtk_button_set_label (btn, (doc->GetIntegralVisible ()?
								_("Show integral"): _("Hide integral")));
	doc->OnShowIntegral ();
}


static void on_transform_fid (GtkButton *btn, SpectrumDocument *doc)
{
	doc->OnTransformFID (btn);
}

static SpectrumType get_spectrum_type_from_string (char const *buf)
{
	unsigned i;
	char *up = g_ascii_strup (buf, -1);
	for (i = 0; i < G_N_ELEMENTS (Types); i++)
		if (!strcmp (Types[i].name, up)) {
			g_free (up);
			return Types[i].type;
		}
	g_free (up);
	return GCU_SPECTRUM_MAX;
}

#define JCAMP_PREC 1e-2 // fully arbitrary

void SpectrumDocument::LoadJcampDx (char const *data)
{
	char key[KEY_LENGTH];
	char buf[VALUE_LENGTH];
	char line[300]; // should be enough
	int n;
	deltax = 0.;
	istringstream s(data);
	JdxVar var;
	var.NbValues = 0;
	var.Symbol = 0;
	var.Type = GCU_SPECTRUM_TYPE_MAX;
	var.Unit = GCU_SPECTRUM_UNIT_MAX;
	var.Format = GCU_SPECTRUM_FORMAT_MAX;
	var.First = var.Last = var.Min = var.Max = var.Factor = 0.;
	var.Values = NULL;
	GString *utf8_str = NULL;
	while (!s.eof ()) {
		s.getline (line, 300);
parse_line:
		n = ReadField (line, key, buf);
		switch (n) {
		case JCAMP_TITLE:
			go_guess_encoding (buf, strlen (buf), "ASCII", &utf8_str, NULL);
			if (utf8_str) {
				SetTitle (utf8_str->str);
				g_string_free (utf8_str, TRUE);
				utf8_str = NULL;
			} else
				SetTitle (buf);
			break;
		case JCAMP_JCAMP_DX:
			break;
		case JCAMP_DATA_TYPE:
			m_SpectrumType = get_spectrum_type_from_string (buf);
			break;
		case JCAMP_DATACLASS:
		case JCAMP_APPLICATION:
		case JCAMP_DICTIONARY:
		case JCAMP_BLOCKS:
		case JCAMP_BLOCK_ID:
			break;
		case JCAMP_END:
			goto out;
		case JCAMP_PEAK_TABLE: {
			// in that case, add drop lines ans remove the normal line
			GogSeries *series = m_View->GetSeries ();
			gog_object_add_by_name (GOG_OBJECT (series), "Vertical drop lines", GOG_OBJECT (g_object_new (GOG_TYPE_SERIES_LINES, NULL)));
			GOStyle *style = go_styled_object_get_style (GO_STYLED_OBJECT (series));
			style->line.dash_type = GO_LINE_NONE;
			style->line.auto_dash = false;
		}
		case JCAMP_XYPAIRS:
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
			if (!strncmp (buf, "(X++(Y..Y))",strlen ("(X++(Y..Y))")))
				ReadDataTable (s, x, y);
			else if (!strncmp (buf, "(XY..XY)",strlen ("(XY..XY)"))) {
				char *cur;
				while (1) {
					if (s.eof ())
						break;	// this should not occur, but a corrupted or bad file is always possible
					s.getline (line, 300);
					if (strstr (line, "##")) {
						if (read > npoints) {
							g_warning (_("Found too many data!"));
							// FIXME: throw an exception
						} else
							npoints = read;
						if (!go_finite (minx))
							go_range_min (x, read, &minx);
						if (!go_finite (maxx))
							go_range_max (x, read, &maxx);
						if (!go_finite (miny))
							go_range_min (y, read, &miny);
						if (!go_finite (maxy))
							go_range_max (y, read, &maxy);
						goto parse_line;
					}
					cur = line;
					while (*cur) {
						while (*cur && (*cur < '0' || *cur > '9') && *cur != '-' && *cur !='+')
							cur++;
						if (*cur == 0)
							break;
						x[read] = g_ascii_strtod (cur, &cur);
						while (*cur && (*cur < '0' || *cur > '9') && *cur != '-' && *cur !='+')
							cur++;
						y[read] = g_ascii_strtod (cur, &cur);
						read++;
					}
				}
			}
			break;
		}
		case JCAMP_PEAK_ASSIGNMENTS:
		case JCAMP_RADATA:
			break;
		case JCAMP_XUNITS:
			m_XUnit = (SpectrumUnitType) get_spectrum_data_from_string (buf, Units, GCU_SPECTRUM_UNIT_MAX);
			if (m_XUnit == GCU_SPECTRUM_UNIT_MAX) {
				for (unsigned i = 0; i < G_N_ELEMENTS (OtherUnits); i++)
					if (!strcmp (buf, OtherUnits[i].name)) {
						m_XUnit = OtherUnits[i].unit;
						break;
					}
			}
			break;
		case JCAMP_YUNITS:
			m_YUnit = (SpectrumUnitType) get_spectrum_data_from_string (buf, Units, GCU_SPECTRUM_UNIT_MAX);
			if (m_YUnit == GCU_SPECTRUM_UNIT_MAX) {
				for (unsigned i = 0; i < G_N_ELEMENTS (OtherUnits); i++)
					if (!strcmp (buf, OtherUnits[i].name)) {
						m_YUnit = OtherUnits[i].unit;
						break;
					}
			}
			if (m_YUnit == GCU_SPECTRUM_UNIT_TRANSMITTANCE)
				m_View->SetAxisBounds (GOG_AXIS_Y, 0., 1., false);
			break;
		case JCAMP_XLABEL:
		case JCAMP_YLABEL:
			break;
		case JCAMP_XFACTOR:
			xfactor = g_ascii_strtod (buf, NULL);
			break;
		case JCAMP_YFACTOR:
			yfactor = g_ascii_strtod (buf, NULL);
			break;
		case JCAMP_FIRSTX:
			firstx = g_ascii_strtod (buf, NULL);
			break;
		case JCAMP_LASTX:
			lastx = g_ascii_strtod (buf, NULL);
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
			firsty = g_ascii_strtod (buf, NULL);
			break;
		case JCAMP_MAXX:
			maxx = g_ascii_strtod (buf, NULL);
			break;
		case JCAMP_MINX:
			minx = g_ascii_strtod (buf, NULL);
			break;
		case JCAMP_MAXY:
			maxy = g_ascii_strtod (buf, NULL);
			break;
		case JCAMP_MINY:
			miny = g_ascii_strtod (buf, NULL);
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
			break;
		case JCAMP_VAR_NAME: {
			size_t i = 0;
			char *cur = buf, *end;
			while (*cur) {
				while (*cur && *cur == ' ')
					cur++;
				if (!*cur)
					break;
				end = strchr (cur, ',');
				if (end)
					*end = 0;
				if (i < variables.size ())
					variables[i].Name = cur;
				else {
					var.Name = cur;
					variables.push_back (var);
					var.Name.clear ();
				}
				cur = (end)? end + 1: cur + strlen (cur);
				i++;
			}
			break;
		}
		case JCAMP_SYMBOL: {
			size_t i = 0;
			char *cur = buf, *end;
			while (*cur) {
				while (*cur && *cur == ' ')
					cur++;
				if (!*cur)
					break;
				end = strchr (cur, ',');
				if (end)
					*end = 0;
				if (i < variables.size ())
					variables[i].Symbol = *cur;
				else {
					var.Symbol = *cur;
					variables.push_back (var);
					var.Symbol = 0;
				}
				switch (*cur) {
				case 'I':
					I = i;
					break;
				case 'R':
					R = i;
					break;
				case 'X':
					X = i;
					break;
				case 'Y':
					Y = i;
					break;
				default:
					break;
				}
				cur = (end)? end + 1: cur + strlen (cur);
				i++;
			}
			break;
		}
		case JCAMP_VAR_TYPE: {
			size_t i = 0;
			char *cur = buf, *end;
			SpectrumVarType Type;
			while (*cur) {
				while (*cur && *cur == ' ')
					cur++;
				if (!*cur)
					break;
				end = strchr (cur, ',');
				if (end)
					*end = 0;
				Type = (SpectrumVarType) get_spectrum_data_from_string (cur, VarTypes, GCU_SPECTRUM_TYPE_MAX);
				if (i < variables.size ())
					variables[i].Type = Type;
				else {
					var.Type = Type;
					variables.push_back (var);
					var.Type = GCU_SPECTRUM_TYPE_MAX;
				}
				cur = (end)? end + 1: cur + strlen (cur);
				i++;
			}
			break;
			break;
		}
		case JCAMP_VAR_FORM: {
			size_t i = 0;
			char *cur = buf, *end;
			SpectrumFormatType Format;
			while (*cur) {
				while (*cur && *cur == ' ')
					cur++;
				if (!*cur)
					break;
				end = strchr (cur, ',');
				if (end)
					*end = 0;
				Format = (SpectrumFormatType) get_spectrum_data_from_string (cur, Formats, GCU_SPECTRUM_FORMAT_MAX);
				if (i < variables.size ())
					variables[i].Format = Format;
				else {
					var.Format = Format;
					variables.push_back (var);
					var.Format = GCU_SPECTRUM_FORMAT_MAX;
				}
				cur = (end)? end + 1: cur + strlen (cur);
				i++;
			}
			break;
		}
		case JCAMP_VAR_DIM: {
			size_t i = 0;
			char *cur = buf;
			unsigned dim;
			while (*cur) {
				dim = strtoul (cur, &cur, 10);
				if (*cur)
					cur++;
				if (i < variables.size ())
					variables[i].NbValues = dim;
				else {
					var.NbValues = dim;
					variables.push_back (var);
					var.NbValues = 0;
				}
				i++;
			}
			break;
		}
		case JCAMP_UNITS: {
			size_t i = 0;
			char *cur = buf, *end;
			SpectrumUnitType Unit;
			while (*cur) {
				while (*cur && *cur == ' ')
					cur++;
				if (!*cur)
					break;
				end = strchr (cur, ',');
				if (end)
					*end = 0;
				Unit = (SpectrumUnitType) get_spectrum_data_from_string (cur, Units, GCU_SPECTRUM_UNIT_MAX);
				if (i < variables.size ())
					variables[i].Unit = Unit;
				else {
					var.Unit = Unit;
					variables.push_back (var);
					var.Unit = GCU_SPECTRUM_UNIT_MAX;
				}
				cur = (end)? end + 1: cur + strlen (cur);
				i++;
			}
			break;
		}
		case JCAMP_FIRST: {
			size_t i = 0;
			char *cur = buf;
			double x;
			while (*cur) {
				x = g_ascii_strtod (cur, &cur);
				if (*cur)
					cur++;
				if (i < variables.size ())
					variables[i].First = x;
				else {
					var.First = x;
					variables.push_back (var);
					var.First = 0.;
				}
				i++;
			}
			break;
		}
		case JCAMP_LAST: {
			size_t i = 0;
			char *cur = buf;
			double x;
			while (*cur) {
				x = g_ascii_strtod (cur, &cur);
				if (*cur)
					cur++;
				if (i < variables.size ())
					variables[i].Last = x;
				else {
					var.Last = x;
					variables.push_back (var);
					var.Last = 0.;
				}
				i++;
			}
			break;
		}
		case JCAMP_MIN: {
			size_t i = 0;
			char *cur = buf;
			double x;
			while (*cur) {
				x = g_ascii_strtod (cur, &cur);
				if (*cur)
					cur++;
				if (i < variables.size ())
					variables[i].Min = x;
				else {
					var.Min = x;
					variables.push_back (var);
					var.Min = 0.;
				}
				i++;
			}
			break;
		}
		case JCAMP_MAX: {
			size_t i = 0;
			char *cur = buf;
			double x;
			while (*cur) {
				x = g_ascii_strtod (cur, &cur);
				if (*cur)
					cur++;
				if (i < variables.size ())
					variables[i].Max = x;
				else {
					var.Max = x;
					variables.push_back (var);
					var.Max = 0.;
				}
				i++;
			}
			break;
		}
		case JCAMP_FACTOR: {
			size_t i = 0;
			char *cur = buf;
			double x;
			while (*cur) {
				x = g_ascii_strtod (cur, &cur);
				if (*cur)
					cur++;
				if (i < variables.size ())
					variables[i].Factor = x;
				else {
					var.Factor = x;
					variables.push_back (var);
					var.Factor = 0.;
				}
				i++;
			}
			break;
		}
		case JCAMP_PAGE: {
			unsigned num = atoi (strchr (buf, '=') + 1);
			if (num == 1) {
				unsigned max = 0;
				for (unsigned i = 0; i < variables.size (); i++) {
					JdxVar &v = variables[i];
					if (v.Name == "PAGE NUMBER")
						continue;
					if (v.NbValues > max)
						max = v.NbValues;
					switch (v.Type) {
					case GCU_SPECTRUM_TYPE_INDEPENDENT:
						if (v.Symbol == 'X') {
							firstx = v.First;
							lastx = v.Last;
							minx = v.Min;
							maxx = v.Max;
							xfactor = v.Factor;
							deltax = (lastx - firstx) / (v.NbValues - 1);
						}
						break;
					case GCU_SPECTRUM_TYPE_DEPENDENT:
						break;
					default:
						break;
					}
				}
				npoints = max;
			}
			break;
		}
		case JCAMP_DATA_TABLE: {
			// first split fields, we might have two, but the second is optional
			char *vlist = buf, *desc;
			while (*vlist && *vlist == ' ')
				vlist++;
			if (!*vlist)
				break;
			desc = strchr (vlist, ',');
			if (desc)
				*desc = 0;
			desc++;
			while (*desc && *desc == ' ')
				desc++;
			struct data_type_struct dts;
			parse_data_type (vlist, dts);
			if (!strncmp (desc, "XYDATA", 6)) {
				if (dts.n == 2 && !dts.one_per_line && !dts.explicit_indep) {
					int first = -1, second = -1;
					switch (*dts.variables) {
					case 'I':
						first = I;
						break;
					case 'R':
						first = R;
						break;
					case 'X':
						first = X;
						break;
					case 'Y':
						first = Y;
						break;
					}
					switch (dts.variables[1]) {
					case 'I':
						second = I;
						break;
					case 'R':
						second = R;
						break;
					case 'X':
						second = X;
						break;
					case 'Y':
						second = Y;
						break;
					}
					if (first >=0 && second >=0 && first != second) {
						if (variables[first].Values)
							delete [] variables[first].Values;
						variables[first].Values = new double[variables[first].NbValues];
						if (variables[second].Values)
							delete [] variables[second].Values;
						variables[second].Values = new double[variables[second].NbValues];
						yfactor = variables[second].Factor;
						firsty = variables[second].First;
						ReadDataTable (s, variables[first].Values, variables[second].Values);
					}
				}
			} //what should be done for PROFILE, PEAKS and COUTOUR?
			g_free (dts.variables);
			break;
		}
		case JCAMP_DELTAX:
			deltax = g_ascii_strtod (buf, NULL);
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
			break;
		case JCAMP_OBSERVE_FREQUENCY:
			freq = g_ascii_strtod (buf, NULL);
			break;
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
			break;
		case BRUCKER_OFFSET:
			offset = g_ascii_strtod (buf, NULL);
			break;
		case VARIAN_OFFSET:
			refpoint = g_ascii_strtod (buf, NULL);
			break;
		default:
			break;
		}
	}

out:
	Loaded ();
}

void SpectrumDocument::ReadDataLine (char const *data, list<double> &l)
{
	int i = 1, j;
	char buf[32], c = data[0];
	double val = 0., newval = 0.;
	bool pos, diff = false;
	char *eq = strstr (const_cast <char *> (data), "$$");
	if (eq)
		*eq = 0;
	pos = true;
	while (c) {
		switch (c) {
		case ' ':
			c = data[i++];
			continue;
		case '-':
			pos = false;
		case '+':
			c = data[i++];
			if ((c < '0' || c > '9') && c != '.') // FIXME: throw an exception
				{;}
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
		newval = g_ascii_strtod (buf, NULL);
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

void SpectrumDocument::DoPrint (G_GNUC_UNUSED GtkPrintOperation *print, GtkPrintContext *context, G_GNUC_UNUSED int page) const
{
	cairo_t *cr;
	double width, height;

	cr = gtk_print_context_get_cairo_context (context);
	width = gtk_print_context_get_width (context);
	height = gtk_print_context_get_height (context);
	int w, h; // size in points
	w = m_View->GetWidth ();
	h = m_View->GetHeight ();
	switch (GetScaleType ()) {
	case GCU_PRINT_SCALE_NONE:
		break;
	case GCU_PRINT_SCALE_FIXED:
		w *= Printable::GetScale ();
		h *= Printable::GetScale ();
		break;
	case GCU_PRINT_SCALE_AUTO:
		if (GetHorizFit ())
			w = width;
		if (GetVertFit ())
			h = height;
		break;
	}
	double x = 0., y = 0.;
	if (GetHorizCentered ())
		x = (width - w) / 2.;
	if (GetVertCentered ())
		y = (height - h) / 2.;
	cairo_save (cr);
	cairo_translate (cr, x, y);
	m_View->Render (cr, w, h);
	cairo_restore (cr);
}

GtkWindow *SpectrumDocument::GetGtkWindow ()
{
	GtkWidget *w = m_View->GetWidget ();
	return (GtkWindow*) ((w)? gtk_widget_get_toplevel (m_View->GetWidget ()): NULL);
}

void SpectrumDocument::ReadDataTable (istream &s, double *x, double *y)
{
	char line[300]; // should be enough
	unsigned read = 0;
	list<double> l;
	int previous = 0;
	double previousx = firstx;
	while (1) {
		if (s.eof ())
			break;	// this should not occur, but a corrupted or bad file is always possible
		s.getline (line, 300);
		if (strstr (line, "##")) {
			s.seekg (-strlen (line) -1, s.cur);
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
			double x1 = (*i) * xfactor;
			int n = read - previous - (int)round((x1-previousx)/deltax);
			previous = read;
			previousx = x1;
			if (n == 0) {
				// values are the same, no y reminder, and nothing to do
			} else if (n == 1) {
				i++;
				previous--;
				double y0 = (*i) * yfactor;
				if (fabs (y0 - y[read - 1]) > fmax (fabs (y0), fabs (y[read - 1])) * JCAMP_PREC)
					g_warning (_("Data check failed!"));
			} else if (previousx - x1 < 0.) {
				unsigned missing = (unsigned) round ((x1 - previousx) / deltax), n;
				for (n = 0; n < missing; n++) {
					if (read > npoints) // FIXME: Throw an exception
						break;
					x[read] = firstx + deltax * read;
					y[read++] = go_nan;
				}
			} else {
				// FIXME: duplicate values, throw an exception
			}
		} else {
			x[read] = (*i) * xfactor;
			if (fabs (x[0] - firstx) > fabs (deltax * JCAMP_PREC)) {
				xfactor = firstx / (*i);
				deltax = (lastx - firstx) / (npoints - 1);
				g_warning (_("Data check failed: FIRSTX!"));
			}
			i++;
			y[read++] = (*i) * yfactor;
			if (fabs (firsty - y[0]) > fmax (fabs (firsty), fabs (y[0])) * JCAMP_PREC)
				g_warning (_("Data check failed: FIRSTY!"));
		}
		for (i++; i !=	end; i++) {
			if (read >= npoints) { // FIXME: Throw an exception
				g_warning (_("Found too many data"));
				break;
			}
			x[read] = firstx + deltax * read;
			y[read++] = (*i) * yfactor;
		}
		l.clear ();
	}
	if (!go_finite (minx))
		go_range_min (x, read, &minx);
	if (!go_finite (maxx))
		go_range_max (x, read, &maxx);
	if (!go_finite (miny))
		go_range_min (y, read, &miny);
	if (!go_finite (maxy))
		go_range_max (y, read, &maxy);
	while (npoints > read) {
		// this should never occur, fill missing y values with nan
		x[read] = minx + deltax * read;
		y[read++] = go_nan;
	}
	if (isnan (maxx)) {
		maxx = MAX (firstx, lastx);
		minx = MIN (firstx, lastx);
	}
}

void SpectrumDocument::OnXUnitChanged (int i)
{
	SpectrumUnitType unit = GCU_SPECTRUM_UNIT_MAX;
	bool invert_axis = false;
	switch (m_SpectrumType) {
	case GCU_SPECTRUM_NMR:
		unit = (i == 0)? GCU_SPECTRUM_UNIT_PPM: GCU_SPECTRUM_UNIT_HZ;
		invert_axis = true;
		break;
	case GCU_SPECTRUM_INFRARED:
	case GCU_SPECTRUM_RAMAN:
		if (i == 1) {
			unit = GCU_SPECTRUM_UNIT_CM_1;
			invert_axis = true;
		} else
			unit = GCU_SPECTRUM_UNIT_MICROMETERS;
		break;
	case GCU_SPECTRUM_UV_VISIBLE:
		if (i == 1) {
			unit = GCU_SPECTRUM_UNIT_CM_1;
			invert_axis = true;
		} else
			unit = GCU_SPECTRUM_UNIT_NANOMETERS;
		break;
	default:
		break;
	}
	if (unit == GCU_SPECTRUM_UNIT_MAX)
		return;
	GOData *godata;
	GogSeries *series = m_View->GetSeries ();
	if (x && m_XUnit == unit) {
		X = -1;
		godata = go_data_vector_val_new (x, npoints, NULL);
		gog_series_set_dim (series, 0, godata, NULL);
		m_View->SetAxisBounds (GOG_AXIS_X, minx, maxx, invert_axis);
		m_View->SetAxisLabel (GOG_AXIS_X, _(UnitNames[m_XUnit]));
		if (m_XAxisInvertBtn) {
			g_signal_handler_block (m_XAxisInvertBtn, m_XAxisInvertSgn);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_XAxisInvertBtn), invert_axis);
			g_signal_handler_unblock (m_XAxisInvertBtn, m_XAxisInvertSgn);
		}
	} else {
		unsigned i, j;
		double (*conv) (double, double, double);
		double f, o;
		for (i = 0; i < variables.size (); i++)
			if (variables[i].Symbol == 'X' && variables[i].Unit == unit)
				break;
		if (i == variables.size ()) {
			// Add new data vector
			JdxVar v;
			if (X >=0) {
				conv = GetConversionFunction (variables[X].Unit, unit, f, o);
				if (!conv)
					return;
				v.Name = _(UnitNames[variables[X].Unit]);
				v.Symbol = variables[X].Symbol;
				v.Type = variables[X].Type;
				v.Unit = unit;
				v.Format = variables[X].Format;
				v.NbValues = variables[X].NbValues;
					v.First = conv (variables[X].First, f, o);
					v.Last = conv (variables[X].Last, f, o);
					v.Min = conv (variables[X].Min, f, o);
					v.Max = conv (variables[X].Max, f, o);
					v.Factor = 1.;
					v.Values = new double[variables[X].NbValues];
					for (j = 0; j < variables[X].NbValues; j++)
						v.Values[j] = conv (variables[X].Values[j], f, o);
			} else {
				conv = GetConversionFunction (m_XUnit, unit, f, o);
				if (!conv)
					return;
				v.Name = _(UnitNames[unit]);
				v.Symbol = 'X';
				v.Type = GCU_SPECTRUM_TYPE_DEPENDENT;
				v.Unit = unit;
				v.Format = GCU_SPECTRUM_FORMAT_MAX;
				v.NbValues = npoints;
					v.First = conv (firstx, f, o);
					v.Last = conv (lastx, f, o);
					v.Min = conv (minx, f, o);
					v.Max = conv (maxx, f, o);
					v.Factor = 1.;
					v.Values = new double[npoints];
					for (j = 0; j < npoints; j++)
						v.Values[j] = conv (x[j], f, o);
			}
			if (v.Min > v.Max) {
				f = v.Min;
				v.Min = v.Max;
				v.Max = f;
			}
			variables.push_back (v);
		}
		X = i;
		godata = go_data_vector_val_new (variables[i].Values, variables[i].NbValues, NULL);
		gog_series_set_dim (series, 0, godata, NULL);
		m_View->SetAxisBounds (GOG_AXIS_X, variables[i].Min, variables[i].Max, invert_axis);
		m_View->SetAxisLabel (GOG_AXIS_X, _(UnitNames[variables[i].Unit]));
		if (m_XAxisInvertBtn) {
			g_signal_handler_block (m_XAxisInvertBtn, m_XAxisInvertSgn);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_XAxisInvertBtn), invert_axis);
			g_signal_handler_unblock (m_XAxisInvertBtn, m_XAxisInvertSgn);
		}
		if (integral > 0) {
			g_object_ref (godata);
		}
	}
	if (integral > 0) {
		g_object_ref (godata);
		gog_series_set_dim (variables[integral].Series, 0, godata, NULL);
	}
}

void SpectrumDocument::OnYUnitChanged (int i)
{
	SpectrumUnitType unit = GCU_SPECTRUM_UNIT_MAX;
	bool invert_axis = false;
	switch (m_SpectrumType) {
	case GCU_SPECTRUM_INFRARED:
	case GCU_SPECTRUM_RAMAN:
	case GCU_SPECTRUM_UV_VISIBLE:
		unit = (i == 0)? GCU_SPECTRUM_UNIT_ABSORBANCE: GCU_SPECTRUM_UNIT_TRANSMITTANCE;
		break;
	default:
		break;
	}
	if (unit == GCU_SPECTRUM_UNIT_MAX)
		return;
	GOData *godata;
	GogSeries *series = m_View->GetSeries ();
	if (m_YUnit == unit) {
		Y = -1;
		godata = go_data_vector_val_new (y, npoints, NULL);
		gog_series_set_dim (series, 1, godata, NULL);
		m_View->SetAxisBounds (GOG_AXIS_Y, miny, maxy, invert_axis);
		m_View->SetAxisLabel (GOG_AXIS_Y, _(UnitNames[m_YUnit]));
	} else {
		unsigned i, j;
		double (*conv) (double, double, double);
		double f, o;
		for (i = 0; i < variables.size (); i++)
			if (variables[i].Symbol == 'Y' && variables[i].Unit == unit)
				break;
		if (i == variables.size ()) {
			// Add new data vector
			JdxVar v;
			if (Y >=0) {
				conv = GetConversionFunction (variables[Y].Unit, unit, f, o);
				if (!conv)
					return;
				v.Name = _(UnitNames[variables[Y].Unit]);
				v.Symbol = variables[Y].Symbol;
				v.Type = variables[Y].Type;
				v.Unit = unit;
				v.Format = variables[Y].Format;
				v.NbValues = variables[Y].NbValues;
				v.First = conv (variables[Y].First, f, o);
				v.Last = conv (variables[Y].Last, f, o);
				v.Min = conv (variables[Y].Min, f, o);
				v.Max = conv (variables[Y].Max, f, o);
				v.Factor = 1.;
				v.Values = new double[variables[Y].NbValues];
				for (j = 0; j < variables[Y].NbValues; j++)
					v.Values[j] = conv (variables[Y].Values[j], f, o);
			} else {
				conv = GetConversionFunction (m_YUnit, unit, f, o);
				if (!conv)
					return;
				v.Name = _(UnitNames[unit]);
				v.Symbol = 'Y';
				v.Type = GCU_SPECTRUM_TYPE_DEPENDENT;
				v.Unit = unit;
				v.Format = GCU_SPECTRUM_FORMAT_MAX;
				v.NbValues = npoints;
				v.First = conv (firsty, f, o);
				v.Last = 0.; // not important
				v.Min = conv (miny, f, o);
				v.Max = conv (maxy, f, o);
				v.Factor = 1.;
				v.Values = new double[npoints];
				for (j = 0; j < npoints; j++)
					v.Values[j] = conv (y[j], f, o);
			}
			if (v.Min > v.Max) {
				f = v.Min;
				v.Min = v.Max;
				v.Max = f;
			}
			variables.push_back (v);
		}
		Y = i;
		godata = go_data_vector_val_new (variables[i].Values, variables[i].NbValues, NULL);
		gog_series_set_dim (series, 1, godata, NULL);
		m_View->SetAxisBounds (GOG_AXIS_Y, variables[i].Min, variables[i].Max, invert_axis);
		m_View->SetAxisLabel (GOG_AXIS_Y, _(UnitNames[variables[i].Unit]));
	}
}

static double mult (double val, double f, double offset)
{
	return val * f + offset;
}

static double inv (double val, double f, double offset)
{
	return f / val + offset;
}

static double logm (double val, double f, double offset)
{
	return -log10 (val * f + offset);
}

static double expm (double val, double f, double offset)
{
	return exp10 (-val) * f + offset;
}

double (*SpectrumDocument::GetConversionFunction (SpectrumUnitType oldu, SpectrumUnitType newu, double &factor, double &shift)) (double, double, double)
{
	switch (oldu) {
	case GCU_SPECTRUM_UNIT_CM_1:
		if (newu == GCU_SPECTRUM_UNIT_NANOMETERS) {
			factor = 1.e7;
			shift = 0;
			return inv;
		}
		if (newu == GCU_SPECTRUM_UNIT_MICROMETERS) {
			factor = 1.e4;
			shift = 0;
			return inv;
		}
		break;
	case GCU_SPECTRUM_UNIT_TRANSMITTANCE:
		if (newu == GCU_SPECTRUM_UNIT_ABSORBANCE) {
			factor = 1.;
			shift = 0.;
			return logm;
		}
		break;
	case GCU_SPECTRUM_UNIT_ABSORBANCE:
		if (newu == GCU_SPECTRUM_UNIT_TRANSMITTANCE) {
			factor = 1.;
			shift = 0.;
			return expm;
		}
		break;
	case GCU_SPECTRUM_UNIT_PPM:
		if (go_finite (freq) && newu == GCU_SPECTRUM_UNIT_HZ) {
			factor = freq;
			shift = 0;
			return mult;
		}
		break;
	case GCU_SPECTRUM_UNIT_NANOMETERS:
		if (newu == GCU_SPECTRUM_UNIT_CM_1) {
			factor = 1.e7;
			shift = 0;
			return inv;
		}
		break;
	case GCU_SPECTRUM_UNIT_MICROMETERS:
		if (newu == GCU_SPECTRUM_UNIT_CM_1) {
			factor = 1.e4;
			shift = 0;
			return inv;
		}
		break;
	case GCU_SPECTRUM_UNIT_HZ:
		if (go_finite (freq) && newu == GCU_SPECTRUM_UNIT_PPM)
			factor = 1. / freq;
		shift = 0.;
		return mult;
	default:
		break;
	}
	return NULL;
}

void SpectrumDocument::OnShowIntegral ()
{
	m_IntegralVisible = !m_IntegralVisible;
	GOStyle *style;
	if (m_IntegralVisible) {
		if (integral < 0) {
			integral = variables.size ();
			JdxVar v;
			double *xo, *xn[5], *yb, cur, acc;
			v.Name = _("Integral");
			v.Symbol = 'i';
			v.Type = GCU_SPECTRUM_TYPE_DEPENDENT;
			v.Unit = GCU_SPECTRUM_UNIT_MAX;
			v.Format = GCU_SPECTRUM_FORMAT_MAX;
			v.Factor = 1.;
			v.NbValues = (X >= 0)? variables[X].NbValues: npoints;
			xn[0] = new double[v.NbValues];
			xn[1] = new double[v.NbValues];
			xn[2] = new double[v.NbValues];
			xn[3] = new double[v.NbValues];
			xn[4] = new double[v.NbValues];
			yb = new double[v.NbValues];
			v.First = 0.;
			v.Values = new double[v.NbValues];
			unsigned i;
			double *z;
			if (Rp >= 0)
				z = variables[Rp].Values;
			else if (R >= 0)
				z = variables[R].Values;
			else if (Y >= 0)
				z = variables[Y].Values;
			else
				z = y;
			xo = (X >= 0 && variables[X].Values != NULL)? variables[X].Values: x;
			double max, delta;
			unsigned used = 0;
			go_range_max (z, v.NbValues, &max);
			max *= 0.005;
			v.Values[0] = 0.;
			for (i = 1; i < v.NbValues; i++) {
				delta = 0.5 * (z[i - 1] + z[i]);
				v.Values[i] = v.Values[i - 1] + delta;
				if (delta < max) {
					cur = xn[0][used] = xo[i];
					acc = xn[1][used] = cur * cur;
					xn[2][used] = (acc *= cur);
					xn[3][used] = (acc *= cur);
					xn[4][used] = acc * cur;
					yb[used] = (used > 0)? yb[used - 1] + delta: delta;
					used++;
				}
			}
			go_regression_stat_t reg;
			double res[6];
			go_linear_regression (xn, 5, yb, used, true, res, &reg);
			for (i = 0; i < v.NbValues; i++) {
				cur = xo[i];
				acc = cur * cur;
				v.Values[i] -= res[0] + res[1] * cur + res[2] * acc;
				v.Values[i] -= res[3] * (acc *= cur);
				v.Values[i] -= res[4] * (acc *= cur);
				v.Values[i] -= res[5] * cur * acc;
			}
			if (xo[1] > xo[0])
				for (i = 0; i < v.NbValues; i++)
					v.Values[i] = -v.Values[i];
			g_free (reg.se);
			g_free (reg.t);
			g_free (reg.xbar);
			v.Last = v.Max = v.Values[v.NbValues - 1];
			v.Min = 0.;
			v.Series = m_View->NewSeries (true);
			GOData *godata;
			godata = go_data_vector_val_new (xo, npoints, NULL);
			gog_series_set_dim (v.Series, 0, godata, NULL);
			godata = go_data_vector_val_new (v.Values, v.NbValues, NULL);
			gog_series_set_dim (v.Series, 1, godata, NULL);
			GOStyledObject *axis = GO_STYLED_OBJECT (g_object_new (GOG_TYPE_AXIS, "major-tick-labeled", false, NULL));
			GogPlot	*plot = gog_series_get_plot (v.Series);
			GogObject *chart = GOG_OBJECT (gog_object_get_parent (GOG_OBJECT (plot)));
			gog_object_add_by_name (chart, "Y-Axis", GOG_OBJECT (axis));
			gog_plot_set_axis (plot, GOG_AXIS (axis));
			style = go_styled_object_get_style (axis);
			style->line.auto_dash = false;
			style->line.dash_type = GO_LINE_NONE;
			style = go_styled_object_get_style (GO_STYLED_OBJECT (v.Series));
			style->line.auto_dash = false;
			style->line.auto_color = false;
			style->line.color = GO_COLOR_RED;
			variables.push_back (v);
			delete [] xn[0];
			delete [] xn[1];
			delete [] xn[2];
			delete [] xn[3];
			delete [] xn[4];
			delete [] yb;
		} else
			style = go_styled_object_get_style (GO_STYLED_OBJECT (variables[integral].Series));
		// show the series
		style->line.dash_type = GO_LINE_SOLID;
		gog_object_request_update (GOG_OBJECT (variables[integral].Series));
	} else {
		// hide the series
		style = go_styled_object_get_style (GO_STYLED_OBJECT (variables[integral].Series));
		style->line.dash_type = GO_LINE_NONE;
		gog_object_request_update (GOG_OBJECT (variables[integral].Series));
	}
}

void SpectrumDocument::OnTransformFID (G_GNUC_UNUSED GtkButton *btn)
{
	double *re = variables[R].Values, *im = variables[I].Values;
	unsigned n = 2;
	while (n < npoints)
		n <<= 1;
	go_complex *fid = new go_complex[n], *sp;
	unsigned i;
	for (i = 0; i < npoints; i++) {
		// assuming we have as many real, imaginary and time values
		fid[i].re = re[i];
		fid[i].im = im[i];
	}
	for (; i < n; i++) {
		// fill with zeros
		fid[i].re =  fid[i].im = 0.;
	}
	//we make no apodization at the moment
	go_fourier_fft (fid, n, 1, &sp, false);
	delete [] fid;
	// copy the unphased data to Rt and It (t for transformed)
	JdxVar vr, vi, rp, xt;
	vr.Name = _("Real transformed data");
	vr.Symbol = 't';
	vr.Type = GCU_SPECTRUM_TYPE_DEPENDENT;
	vr.Unit = GCU_SPECTRUM_UNIT_MAX;
	vr.Format = GCU_SPECTRUM_FORMAT_MAX;
	vr.Factor = 1.;
	vr.NbValues = n;
	vr.Values = new double[n];
	vi.Name = _("Imaginary transformed data");
	vi.Symbol = 'u';
	vi.Type = GCU_SPECTRUM_TYPE_DEPENDENT;
	vi.Unit = GCU_SPECTRUM_UNIT_MAX;
	vi.Format = GCU_SPECTRUM_FORMAT_MAX;
	vi.Factor = 1.;
	vi.NbValues = n;
	vi.Values = new double[n];
	unsigned n2 = n / 2 - 1, j;
	for (i = 0, j = n2; i < n2; i++, j--) {
		vr.Values[i] = sp[j].re;
		vi.Values[i] = sp[j].im;
	}
	// the value at 0 must be skipped, doing a linear interpolation
	vr.Values[i] = (sp[1].re + sp[n - 1].re) / 2.;
	vi.Values[i++] = (sp[1].im + sp[n - 1].im) / 2.;
	for (j = n - 1; i < n; i++, j--) {
		vr.Values[i] = sp[j].re;
		vi.Values[i] = sp[j].im;
	}
	vr.First = vr.Values[0];
	vr.Last = vr.Values[n - 1];
	go_range_min (vr.Values, n, &vr.Min);
	go_range_max (vr.Values, n, &vr.Max);
	vr.Series = NULL;
	Rt = variables.size ();
	variables.push_back (vr);
	vi.First = vi.Values[0];
	vi.Last = vi.Values[n - 1];
	go_range_min (vi.Values, n, &vi.Min);
	go_range_max (vi.Values, n, &vi.Max);
	vi.Series = NULL;
	It = variables.size ();
	variables.push_back (vi);
	// Now we need to adjust the phase (see http://www.ebyte.it/stan/Poster_EDISPA.html)
	double phi = 0., tau = 0., *z, maxz = 0., phiopt = 0., tauopt = 0.;
	z = new double[n];
	for (i = 0; i < n; i++) {
		// copy reordered data to sp
		sp[i].re = vr.Values[i];
		sp[i].im = vi.Values[i];
		z[i] = go_complex_mod (sp + i);
		if (z[i] > maxz)
			maxz = z[i];
	}
	// normalize the z values
	for (i = 0; i < n; i++)
		z[i] /= maxz;
	double c = 0.1;
	unsigned nmax, nmin;
	while (c > 0.) {
		nmin = n, nmax = 0;
		for (i = 0; i < n; i++)
			if (z[i] > c) {
				nmin = i;
				break;
			}
		for (i = n - 1; i > 0; i--)
			if (z[i] > c) {
				nmax = i;
				break;
			}
		if ((nmax - nmin) > n / 10) // 10 is arbitrary, but should be not too large
			break;
		c /= 2.;
	}
	// make a list of indices with z greater than c
	std::list <unsigned> restricted;
	for (i = 0; i < n; i++)
		if (z[i] > c)
			restricted.push_back (i);
	// evaluate the predictor in c, since the value is not used anymore
	double maxc = 0., maxk = 0., p, phis[41];
	unsigned k;
	std::list <unsigned>::iterator it, itend = restricted.end ();
	for (k = 0; k < 41; k++) {
		tau = -1. + k * .1;
		maxk = 0.;
		for (phi = 0; phi <  2 * M_PI; phi += 10. / 180. * M_PI) {
			c = 0.;
			for (it = restricted.begin (); it != itend; it++) {
				i = *it;
				p = phi - 2. * M_PI * tau * (n - 1 - i) / n;
				c += z[i] * z[i] * (sp[i].re * cos (p) - sp[i].im * sin (p)) * exp (-fabs (4. * i / n -2));
			}
			if (c > maxk) {
				if (c > maxc) {
					maxc = c;
					tauopt = tau;
					phiopt = phi;
				}
				maxk = c;
				phis[k] = phi;
			}
		}
	}
	// let's search more finely around the maximum
	// using the fact that phis[k] is a(n almost) linear function of tau
	// first, reevaluate the optimum phi for tauopt and previous value and next value with a step of 1°
	double phase = phiopt;
	for (phi = phase - 9. / 180. * M_PI; phi < phase + 10. / 180. * M_PI; phi += 1. / 180. * M_PI) {
			c = 0.;
			for (it = restricted.begin (); it != itend; it++) {
				i = *it;
				p = phi - 2. * M_PI * tauopt * (n - 1 - i) / n;
				c += z[i] * z[i] * (sp[i].re * cos (p) - sp[i].im * sin (p)) * exp (-fabs (4. * i / n -2));
			}
			if (c > maxc) {
				maxc = c;
				phiopt = phi;
			}
	}
	for (k = 1; k < 10; k++) {
		// search with lower values of tau
		tau = tauopt - 0.01 * k;
		for (phi = 0; phi <  2 * M_PI; phi += 10. / 180. * M_PI) {
			c = 0.;
			maxk = 0;
			for (it = restricted.begin (); it != itend; it++) {
				i = *it;
				p = phi - 2. * M_PI * tau * (n - 1 - i) / n;
				c += z[i] * z[i] * (sp[i].re * cos (p) - sp[i].im * sin (p)) * exp (-fabs (4. * i / n -2));
			}
			if (c > maxk) {
				phis[0] = phi;
				maxk = c;
			}
		}
		phase = phis[0];
		for (phi = phase - 9. / 180. * M_PI; phi < phase + 10. / 180. * M_PI; phi += 1. / 180. * M_PI) {
				c = 0.;
				for (it = restricted.begin (); it != itend; it++) {
					i = *it;
					p = phi - 2. * M_PI * tau * (n - 1 - i) / n;
					c += z[i] * z[i] * (sp[i].re * cos (p) - sp[i].im * sin (p)) * exp (-fabs (4. * i / n -2));
				}
				if (c > maxk) {
					maxk = c;
					phis[0] = phi;
				}
		}
		if (maxk < maxc)
			break;
		maxc = maxk;
		phiopt = phis[0];
		tauopt = tau;
	}
	if (k == 1) {
		for (k = 1; k < 10; k++) {
			// search with higher values of tau
			tau = tauopt + 0.01 * k;
			for (phi = 0; phi <  2 * M_PI; phi += 10. / 180. * M_PI) {
				c = 0.;
				maxk = 0;
				for (it = restricted.begin (); it != itend; it++) {
					i = *it;
					p = phi - 2. * M_PI * tau * (n - 1 - i) / n;
					c += z[i] * z[i] * (sp[i].re * cos (p) - sp[i].im * sin (p)) * exp (-fabs (4. * i / n -2));
				}
				if (c > maxk) {
					phis[0] = phi;
					maxk = c;
				}
			}
			phase = phis[0];
			for (phi = phase - 9. / 180. * M_PI; phi < phase + 10. / 180. * M_PI; phi += 1. / 180. * M_PI) {
					c = 0.;
					for (it = restricted.begin (); it != itend; it++) {
						i = *it;
						p = phi - 2. * M_PI * tau * (n - 1 - i) / n;
						c += z[i] * z[i] * (sp[i].re * cos (p) - sp[i].im * sin (p)) * exp (-fabs (4. * i / n -2));
					}
					if (c > maxk) {
						maxk = c;
						phis[0] = phi;
					}
			}
			if (maxk < maxc)
				break;
			maxc = maxk;
			phiopt = phis[0];
			tauopt = tau;
		}
	}
	g_free (sp);
	// set the phase real values
	// free what needs to be freed
	delete [] z;
	// set the corrected values
	rp.Values = new double[n];
	//store phi and tau as first and last, respectively
	double step = M_PI * 2. * tauopt / n;
	phase = phiopt - M_PI * 2. * tauopt;
	rp.First = phase + step;
	rp.Last = phiopt + n * step;
	for (i = 0; i < n; i++) {
		phase += step;
		rp.Values[i] = vr.Values[i] * cos (phase) - vi.Values[i] * sin (phase);
	}
	go_range_min (rp.Values, n, &rp.Min);
	go_range_max (rp.Values, n, &rp.Max);
	Rp = variables.size ();
	variables.push_back (rp);
	// add Hz and ppm variables (0 for last point, user will have to choose a reference peak)
	// first Hz
	// if we are there, we have R and I values, we should have also X, but let's check
	double freq, shift;
	if (X >= 0 && variables[X].Values != NULL)
		freq = 1 / (variables[X].Last - variables[X].First);
	else
		freq = 1 / (lastx - firstx);
	if (!go_finite (offset))
		offset = 0.;
	shift = offset - freq * (npoints - 1) / 2.;
	//now display the spectrum
	variables[R].Series = NULL;
	rp.Series = m_View->GetSeries ();
	GOData *godata = go_data_vector_val_new (rp.Values, n, NULL);
	gog_series_set_dim (rp.Series, 1, godata, NULL);
	m_View->SetAxisBounds (GOG_AXIS_Y, rp.Min, rp.Max, false);
	if (Xt < 0) {
		xt.Name = _("Chemical shift");
		xt.Symbol = 'X';
		xt.Type = GCU_SPECTRUM_TYPE_INDEPENDENT;
		xt.Unit = GCU_SPECTRUM_UNIT_HZ;
		xt.Format = GCU_SPECTRUM_FORMAT_MAX;
		xt.Factor = 1.;
		xt.NbValues = n;
		xt.Values = new double[n];
		for (i = 0; i < n; i++)
			xt.Values[i] = i * freq + shift;
		xt.Min = xt.First = xt.Values[0];
		xt.Max = xt.Last = xt.Values[n - 1];
		xt.Series = NULL;
		X = variables.size ();
		variables.push_back (xt);
	}
	godata = go_data_vector_val_new (variables[X].Values, variables[X].NbValues, NULL);
	gog_series_set_dim (rp.Series, 0, godata, NULL);
	m_SpectrumType = GCU_SPECTRUM_NMR;
	m_View->SetAxisBounds (GOG_AXIS_X, variables[X].Min, variables[X].Max, true);
	m_View->SetAxisLabel (GOG_AXIS_X, _(UnitNames[variables[X].Unit]));
	OnXUnitChanged (0);
	// remove the last widget from the option box
	m_View->DestroyExtraWidget ();
	// now add the widgets appropriate for an NMR spectrum
	GtkWidget *grid = gtk_grid_new (), *w;
	if (!gtk_check_version (3, 2, 0))
		gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
	else
		gtk_grid_set_row_spacing (GTK_GRID (grid), 12);
	if (go_finite (freq)) {
		w = gtk_label_new (_("X unit:"));
		gtk_container_add (GTK_CONTAINER (grid), w);
		w = gtk_combo_box_text_new ();
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Chemical shift (ppm)"));
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Frequency (Hz)"));
		SpectrumUnitType unit = (X >= 0)? variables[X].Unit: m_XUnit;
		gtk_combo_box_set_active (GTK_COMBO_BOX (w), ((unit == GCU_SPECTRUM_UNIT_PPM)? 0: 1));
		g_signal_connect (w, "changed", G_CALLBACK (on_xunit_changed), this);
		gtk_container_add (GTK_CONTAINER (grid), w);
	}
	w = gtk_button_new_with_label (_("Show integral"));
	g_signal_connect (w, "clicked", G_CALLBACK (on_show_integral), this);
	gtk_container_add (GTK_CONTAINER (grid), w);
	gtk_widget_show_all (grid);
	m_View->AddToOptionBox (grid);
}

void SpectrumDocument::OnXAxisInvert (bool inverted)
{
	m_View->InvertAxis (GOG_AXIS_X, inverted);
}

bool SpectrumDocument::Loaded () throw (gcu::LoaderError)
{
	bool hide_y_axis = false;
	SpectrumUnitType unit = GCU_SPECTRUM_UNIT_MAX;
	// doon't do anything for unsupported spectra
	switch (m_SpectrumType) {
	case GCU_SPECTRUM_NMR: {
		if (x == NULL && X >= 0 && variables[X].Values == NULL)
			return false;
		// fix origin
		if (go_finite (offset)) {
			unsigned i;
			if (x) {
				double d = offset * freq - maxx;
				maxx += d;
				minx += d;
				firstx += d;
				lastx += d;
				for (i = 0; i < npoints; i++)
					x[i] += d;
			} else if (X < 0) {
				JdxVar xt;
				xt.Name = _("Chemical shift");
				xt.Symbol = 'X';
				xt.Type = GCU_SPECTRUM_TYPE_INDEPENDENT;
				xt.Unit = GCU_SPECTRUM_UNIT_HZ;
				xt.Format = GCU_SPECTRUM_FORMAT_MAX;
				xt.Factor = 1.;
				xt.NbValues = npoints;
				xt.Values = new double[npoints];
				double freq = (maxx - minx) / npoints, shift = (maxx - minx) / 2. - offset;
				maxx -= shift;
				minx -= shift;
				for (i = 0; i < npoints; i++)
					xt.Values[i] = i * freq + minx;
				xt.Min = xt.First = xt.Values[0];
				xt.Max = xt.Last = xt.Values[npoints - 1];
				xt.Series = NULL;
				X = variables.size ();
				variables.push_back (xt);
				OnXUnitChanged (0);
				minx = variables[X].Min;
				maxx = variables[X].Max;
			} else {
				double d = offset * freq - variables[X].Max;
				maxx = variables[X].Max += d;
				minx = variables[X].Min += d;
				variables[X].First += d;
				variables[X].Last += d;
				for (i = 0; i < npoints; i++)
					variables[X].Values[i] += d;
			}
		} else if (go_finite (refpoint)) {
			unsigned i;
			double d = -refpoint;
			if (x) {
				maxx += d;
				minx += d;
				firstx += d;
				lastx += d;
				for (i = 0; i < npoints; i++)
					x[i] += d;
			} else {
				maxx = variables[X].Max += d;
				minx = variables[X].Min += d;
				variables[X].First += d;
				variables[X].Last += d;
				for (i = 0; i < npoints; i++)
					variables[X].Values[i] += d;
			}
		}
		// add some widgets to the option box
		GtkWidget *grid = gtk_grid_new (), *w;
		if (!gtk_check_version (3, 2, 0))
			gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
		else
			gtk_grid_set_row_spacing (GTK_GRID (grid), 12);
		if (go_finite (freq)) {
			w = gtk_label_new (_("X unit:"));
			gtk_container_add (GTK_CONTAINER (grid), w);
			w = gtk_combo_box_text_new ();
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Chemical shift (ppm)"));
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Frequency (Hz)"));
			SpectrumUnitType unit = (X >= 0)? variables[X].Unit: m_XUnit;
			gtk_combo_box_set_active (GTK_COMBO_BOX (w), ((unit == GCU_SPECTRUM_UNIT_PPM)? 0: 1));
			g_signal_connect (w, "changed", G_CALLBACK (on_xunit_changed), this);
			gtk_container_add (GTK_CONTAINER (grid), w);
		}
		w = gtk_button_new_with_label (_("Show integral"));
		g_signal_connect (w, "clicked", G_CALLBACK (on_show_integral), this);
		gtk_container_add (GTK_CONTAINER (grid), w);
		gtk_widget_show_all (grid);
		m_View->AddToOptionBox (grid);
		hide_y_axis = true;
		break;
	}
	case GCU_SPECTRUM_NMR_FID: {
		if (x == NULL && X >= 0 && variables[X].Values == NULL)
			return false;
		else if (x == NULL && X < 0) {
			x = new double[npoints];
			deltax = (maxx - minx) / (npoints - 1);
			for (unsigned i = 0; i < npoints;i++)
				x[i] = minx + i * deltax;
			firstx = 0;
			lastx = x[npoints - 1];
		}
		if (R >= 0 && I >= 0) {
			GtkWidget *grid = gtk_grid_new ();
			if (!gtk_check_version (3, 2, 0))
				gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
			else
				gtk_grid_set_row_spacing (GTK_GRID (grid), 12);
			GtkWidget *w = gtk_button_new_with_label (_("Transform to spectrum"));
			g_signal_connect (w, "clicked", G_CALLBACK (on_transform_fid), this);
			if (!go_finite (offset))
				gtk_widget_set_sensitive (w, false);
			gtk_container_add (GTK_CONTAINER (grid), w);
			gtk_widget_show_all (grid);
			m_View->AddToOptionBox (grid);
		}
		hide_y_axis = true;
		break;
	}
	case GCU_SPECTRUM_INFRARED:
	case GCU_SPECTRUM_RAMAN:
		unit = GCU_SPECTRUM_UNIT_MICROMETERS;
//	case GCU_SPECTRUM_INFRARED_PEAK_TABLE:
//	case GCU_SPECTRUM_INFRARED_INTERFEROGRAM:
//	case GCU_SPECTRUM_INFRARED_TRANSFORMED:
	case GCU_SPECTRUM_UV_VISIBLE:
		if (x == NULL && X > 0 && variables[X].Values == NULL)
			return false;
		else {
			if (unit == GCU_SPECTRUM_UNIT_MAX)
				unit = GCU_SPECTRUM_UNIT_NANOMETERS;
			// add some widgets to the option box
			GtkWidget *grid = gtk_grid_new (), *w;
			if (!gtk_check_version (3, 2, 0))
				gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
			else
				gtk_grid_set_row_spacing (GTK_GRID (grid), 12);
			w = gtk_label_new (_("X unit:"));
			gtk_container_add (GTK_CONTAINER (grid), w);
			w = gtk_combo_box_text_new ();
			GList *cells = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (w));
			if (cells && cells->data) {
				/* set "markup" as the target property */
				gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (w), GTK_CELL_RENDERER (cells->data),
								"markup", 0, NULL);
			}
			if  (unit == GCU_SPECTRUM_UNIT_NANOMETERS)
				gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Wave length (nm)"));
			else
				gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Wave length (µm)"));
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Wavenumber (cm<sup>−1</sup>)"));
			SpectrumUnitType unit = (X >= 0)? variables[X].Unit: m_XUnit;
			gtk_combo_box_set_active (GTK_COMBO_BOX (w), ((unit == GCU_SPECTRUM_UNIT_CM_1)? 1: 0));
			g_signal_connect (w, "changed", G_CALLBACK (on_xunit_changed), this);
			gtk_container_add (GTK_CONTAINER (grid), w);
			m_XAxisInvertBtn = gtk_check_button_new_with_label (_("Invert X Axis"));
			m_XAxisInvertSgn = g_signal_connect (m_XAxisInvertBtn, "toggled", G_CALLBACK (on_xaxis_invert), this);
			gtk_container_add (GTK_CONTAINER (grid), m_XAxisInvertBtn);
			unit = (Y >= 0)? variables[Y].Unit: m_YUnit;
			if (unit == GCU_SPECTRUM_UNIT_ABSORBANCE || unit == GCU_SPECTRUM_UNIT_TRANSMITTANCE) {
				w = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
				gtk_container_add (GTK_CONTAINER (grid), w);
				w = gtk_label_new (_("Y unit:"));
				gtk_container_add (GTK_CONTAINER (grid), w);
				w = gtk_combo_box_text_new ();
				gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Absorbance"));
				gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), _("Transmittance"));
				gtk_combo_box_set_active (GTK_COMBO_BOX (w), ((unit == GCU_SPECTRUM_UNIT_ABSORBANCE)? 0: 1));
				g_signal_connect (w, "changed", G_CALLBACK (on_yunit_changed), this);
				gtk_container_add (GTK_CONTAINER (grid), w);
			}
			gtk_widget_show_all (grid);
			m_View->AddToOptionBox (grid);
		}
		break;
//	case GCU_SPECTRUM_NMR_PEAK_TABLE:
//	case GCU_SPECTRUM_NMR_PEAK_ASSIGNMENTS:
	case GCU_SPECTRUM_MASS:
		break;
	default:
		return false;
	}
	m_Empty = npoints == 0;
	GOData *godata;
	GogSeries *series = m_View->GetSeries ();
	if (X >= 0 && variables[X].Values != NULL) {
		godata = go_data_vector_val_new (variables[X].Values, npoints, NULL);
		m_XUnit = variables[X].Unit;
	}
	else
		godata = go_data_vector_val_new (x, npoints, NULL);
	gog_series_set_dim (series, 0, godata, NULL);
	if (Y >= 0 && variables[Y].Values != NULL) {
		godata = go_data_vector_val_new (variables[Y].Values, npoints, NULL);
		m_YUnit = variables[Y].Unit;
	} else if (R >= 0 && variables[R].Values != NULL) {
		godata = go_data_vector_val_new (variables[R].Values, npoints, NULL);
		m_YUnit = variables[R].Unit;
	} else
		godata = go_data_vector_val_new (y, npoints, NULL);
	gog_series_set_dim (series, 1, godata, NULL);
	/* invert X-axis if needed */
	bool invert_axis = false;
	switch (m_XUnit) {
	case GCU_SPECTRUM_UNIT_CM_1:
	case GCU_SPECTRUM_UNIT_PPM:
		invert_axis = true;
		break;
	case GCU_SPECTRUM_UNIT_HZ:
		if (m_SpectrumType == GCU_SPECTRUM_NMR)
			invert_axis = true;
		break;
	default:
		break;
	}
	if (m_XAxisInvertBtn) {
		g_signal_handler_block (m_XAxisInvertBtn, m_XAxisInvertSgn);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_XAxisInvertBtn), invert_axis);
		g_signal_handler_unblock (m_XAxisInvertBtn, m_XAxisInvertSgn);
	}
	m_View->SetAxisBounds (GOG_AXIS_X, minx, maxx, invert_axis);
	m_View->SetAxisBounds (GOG_AXIS_Y, miny, maxy, false);
	if (hide_y_axis)
		m_View->ShowAxis (GOG_AXIS_Y, false);
	/* Add axes labels */
	if (m_XUnit < GCU_SPECTRUM_UNIT_MAX)
		m_View->SetAxisLabel (GOG_AXIS_X, _(UnitNames[m_XUnit]));
	if (m_YUnit < GCU_SPECTRUM_UNIT_MAX)
		m_View->SetAxisLabel (GOG_AXIS_Y, _(UnitNames[m_YUnit]));
	return true;
}

bool SpectrumDocument::SetProperty (unsigned property, char const *value)
{
	istringstream is (value);
	switch (property) {
	case GCU_PROP_DOC_TITLE:
		SetTitle (value);
		break;
	case GCU_PROP_SPECTRUM_TYPE:
		m_SpectrumType = get_spectrum_type_from_string (value);
		break;
	case GCU_PROP_SPECTRUM_NPOINTS:
		is >> npoints;
		break;
	case GCU_PROP_SPECTRUM_DATA_X:
		break;
	case GCU_PROP_SPECTRUM_DATA_Y:
		break;
	case GCU_PROP_SPECTRUM_DATA_REAL: {
		if (npoints == 0 || R >= 0)
			return false;
		JdxVar vr;
		vr.Name = _("Real data");
		vr.Symbol = 'r';
		vr.Type = GCU_SPECTRUM_TYPE_DEPENDENT;
		vr.Unit = GCU_SPECTRUM_UNIT_MAX;
		vr.Format = GCU_SPECTRUM_FORMAT_MAX;
		vr.Factor = 1.;
		vr.NbValues = npoints;
		vr.Values = new double[npoints];
		for (unsigned i = 0; i < npoints; i++)
			is >> vr.Values[i];
		vr.First = vr.Values[0];
		vr.Last = vr.Values[npoints - 1];
		go_range_min (vr.Values, npoints, &vr.Min);
		go_range_max (vr.Values, npoints, &vr.Max);
		vr.Series = NULL;
		Y = R = variables.size ();
		variables.push_back (vr);
		break;
	}
	case GCU_PROP_SPECTRUM_DATA_IMAGINARY: {
		if (npoints == 0 || I >= 0)
			return false;
		JdxVar vi;
		vi.Name = _("Imaginary data");
		vi.Symbol = 'i';
		vi.Type = GCU_SPECTRUM_TYPE_DEPENDENT;
		vi.Unit = GCU_SPECTRUM_UNIT_MAX;
		vi.Format = GCU_SPECTRUM_FORMAT_MAX;
		vi.Factor = 1.;
		vi.NbValues = npoints;
		vi.Values = new double[npoints];
		for (unsigned i = 0; i < npoints; i++)
			is >> vi.Values[i];
		vi.First = vi.Values[0];
		vi.Last = vi.Values[npoints - 1];
		go_range_min (vi.Values, npoints, &vi.Min);
		go_range_max (vi.Values, npoints, &vi.Max);
		vi.Series = NULL;
		I = variables.size ();
		variables.push_back (vi);
		break;
	}
	case GCU_PROP_SPECTRUM_X_UNIT:
			m_XUnit = (SpectrumUnitType) get_spectrum_data_from_string (value, Units, GCU_SPECTRUM_UNIT_MAX);
		break;
	case GCU_PROP_SPECTRUM_X_MIN:
		is >> minx;
		break;
	case GCU_PROP_SPECTRUM_X_MAX:
		is >> maxx;
		break;
	case GCU_PROP_SPECTRUM_X_OFFSET:
		is >> offset;
		break;
	case GCU_PROP_SPECTRUM_NMR_FREQ:
		is >> freq;
		break;
	default:
		return false;
	}
	return true;
}

}	//	nampespace gcu
