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
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <cstring>
#include <sstream>
#include <iostream>

namespace gcu
{

SpectrumDocument::SpectrumDocument (): Document (NULL), m_Empty (true)
{
	m_View = new SpectrumView (this);
}

SpectrumDocument::SpectrumDocument (Application *App, SpectrumView *View): Document (App)
{
	m_View = (View)? View: new SpectrumView (this);
}

SpectrumDocument::~SpectrumDocument ()
{
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
		// Now, get the key value
		data = eq + 1;
printf("%s == ",key);
		while (*data != 0 && *data < '$')
			data++;
puts(data);
	}
	return JCAMP_UNKNOWN;
}

void SpectrumDocument::LoadJcampDx (char const *data)
{
	char key[KEY_LENGTH];
	char buf[VALUE_LENGTH];
	char line[300]; // should be enough
	istringstream s(data);
	s.getline (line, 300);
	ReadField (line, key, buf);
	while (!s.eof ()) {
		s.getline (line, 300);
		ReadField (line, key, buf);
	}
}

}	//	nampespace gcu
