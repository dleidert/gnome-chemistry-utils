// -*- C++ -*-

/* 
 * NUTS files loader plugin
 * nuts.cc 
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/application.h>
#include <gcu/document.h>
#include <gcu/loader.h>
#include <gcu/objprops.h>
#include <sstream>

#include <goffice/app/module-plugin-defs.h>
#include <glib/gi18n-lib.h>

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define READINT16(input,i) gsf_input_read (input, 2, (guint8*) &i)
#define READINT32(input,i) gsf_input_read (input, 4, (guint8*) &i)
#define READFLOAT(input,i) gsf_input_read (input, 4, (guint8*) &i)
#else
unsigned char buffer[4];
bool readint_res;
#define READINT16(input,i) \
	readint_res = gsf_input_read (input, 2, (guint8*) buffer), \
	i = buffer[0] + (buffer[1] << 8), readint_res
#define READINT32(input,i) \
	readint_res = gsf_input_read (input, 4, (guint8*) buffer), \
	i = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24), readint_res
#define READFLOAT(input,i) \
	readint_res = gsf_input_read (input, 4, (guint8*) buffer), \
	*((int32_t *) &i) = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24), readint_res
#endif

class NUTSLoader: public gcu::Loader
{
public:
	NUTSLoader ();
	virtual ~NUTSLoader ();

	gcu::ContentType Read (gcu::Document *doc, GsfInput *in, char const *mime_type, GOIOContext *io);
};

NUTSLoader::NUTSLoader ()
{
	AddMimeType ("application/x-nuts-fid");
}

NUTSLoader::~NUTSLoader ()
{
	RemoveMimeType ("application/x-nuts-fid");
}

gcu::ContentType NUTSLoader::Read (gcu::Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io)
{
	int32_t i32;
	int32_t header_size, dims, frame_size, nb2D, npts1, data_type1, domain1, axis1, decim, npts2, data_type2, domain2, axis2, nbacq;
	bool use_ints, tailer, ver2;
	float sw, sf, sw1, sf1, of1, rp1, pp1, tba1, tpb1, tlb1, time[64], sw2, sf2, of2, rp2, pp2, tba2, tpb2, tlb2, temp, pw, rd, f;
	int i;
	char desc[41], uname[41], date[33], comment[129], pgname[33], nucleus[33], solvent[33];
	char *buf, sbuf[G_ASCII_DTOSTR_BUF_SIZE];
	if (!READINT32 (in, i32) || i32 != 0x4030201)
		return gcu::ContentTypeUnknown;
	READINT32 (in, header_size);
	READINT32 (in, dims);
	if (dims != 1)
		return gcu::ContentTypeUnknown; // only 1D supported for now
	doc->SetProperty (GCU_PROP_SPECTRUM_TYPE, "NMR FID");
	READINT32 (in, i32);
	use_ints = i32 != 0;
	READINT32 (in, i32);
	ver2 = i32 == 2;
	READINT32 (in, frame_size);
	READINT32 (in, i32); //Program version
	READINT32 (in, nb2D);
	READINT32 (in, i32);
	tailer = i32 != 0;
	for (i = 0; i < 9; i++)
		READINT32 (in, i32); // unused
	READFLOAT (in, sw); // not used
	READFLOAT (in, sf); // not used
	for (i = 0; i < 64; i++)
		READFLOAT (in, time[i]); // what are these for?
	for (i = 0; i < 12; i++)
		READINT32 (in, i32); // unassigned
	READINT32 (in, npts1);
	buf = g_strdup_printf ("%u", npts1);
	doc->SetProperty (GCU_PROP_SPECTRUM_NPOINTS, buf);
	g_free (buf);
	READINT32 (in, data_type1);
	READINT32 (in, domain1);
	READINT32 (in, axis1);
	READINT32 (in, decim);
	for (i = 0; i < 11; i++)
		READINT32 (in, i32); // unassigned
	READFLOAT (in, sw1);
	READFLOAT (in, sf1);
	READFLOAT (in, of1);
	if (domain1) {
		doc->SetProperty (GCU_PROP_SPECTRUM_TYPE, "NMR SPECTRUM");
		// HZ is for axis1 == 2, what about 0 and 1?
		doc->SetProperty (GCU_PROP_SPECTRUM_X_UNIT, (axis1 == 3)? "PPM": "HZ");
		// not tested, no sample available
		g_ascii_dtostr (sbuf, G_ASCII_DTOSTR_BUF_SIZE, sw1);
	} else {
		doc->SetProperty (GCU_PROP_SPECTRUM_TYPE, "NMR FID");
		doc->SetProperty (GCU_PROP_SPECTRUM_X_UNIT, "SECONDS");
		double max = npts1 / sw1;
		g_ascii_dtostr (sbuf, G_ASCII_DTOSTR_BUF_SIZE, max);
		doc->SetProperty (GCU_PROP_SPECTRUM_X_MAX, sbuf);
	}
	doc->SetProperty (GCU_PROP_SPECTRUM_X_MIN, "0.");
	g_ascii_dtostr (sbuf, G_ASCII_DTOSTR_BUF_SIZE, of1);
	doc->SetProperty (GCU_PROP_SPECTRUM_X_OFFSET, sbuf);
	g_ascii_dtostr (sbuf, G_ASCII_DTOSTR_BUF_SIZE, sf1);
	doc->SetProperty (GCU_PROP_SPECTRUM_NMR_FREQ, sbuf);
	READFLOAT (in, rp1);
 	READFLOAT (in, pp1);
 	READFLOAT (in, tba1);
 	READFLOAT (in, tpb1);
 	READFLOAT (in, tlb1);
	for (i = 0; i < 16; i++)
		READINT32 (in, i32); // unassigned
	READINT32 (in, npts2);
	READINT32 (in, data_type2);
	READINT32 (in, domain2);
	READINT32 (in, axis2);
	for (i = 0; i < 12; i++)
		READINT32 (in, i32); // unassigned
	READFLOAT (in, sw2);
	READFLOAT (in, sf2);
	READFLOAT (in, of2);
	READFLOAT (in, rp2);
 	READFLOAT (in, pp2);
 	READFLOAT (in, tba2);
 	READFLOAT (in, tpb2);
 	READFLOAT (in, tlb2);
	if (ver2 == 2) {
		for (i = 0; i < 96; i++) // doc says 42, but two values are undocumented
			READINT32 (in, i32); // unassigned
		READFLOAT (in, temp);
		desc[0] = 0;	// deprecated field
		READFLOAT (in, pw);
		READFLOAT (in, rd);
		READINT32 (in, nbacq);
		gsf_input_read (in, 32, reinterpret_cast <guint8 *> (pgname));
		for (i = 32; i >= 0; i--)
			if (pgname[i]!= ' ')
				break;
		pgname[i + 1] = 0;
		gsf_input_read (in, 32, reinterpret_cast <guint8 *> (nucleus));
		for (i = 32; i >= 0; i--)
			if (nucleus[i]!= ' ')
				break;
		nucleus[i + 1] = 0;
		gsf_input_read (in, 32, reinterpret_cast <guint8 *> (solvent));
		for (i = 32; i >= 0; i--)
			if (solvent[i]!= ' ')
				break;
		solvent[i + 1] = 0;
		gsf_input_read (in, 32, reinterpret_cast <guint8 *> (uname));
		for (i = 32; i >= 0; i--)
			if (uname[i]!= ' ')
				break;
		uname[i + 1] = 0;
		gsf_input_read (in, 32, reinterpret_cast <guint8 *> (date));
		for (i = 32; i >= 0; i--)
			if (date[i]!= ' ')
				break;
		date[i + 1] = 0;
		gsf_input_read (in, 128, reinterpret_cast <guint8 *> (comment));
		for (i = 128; i >= 0; i--)
			if (comment[i]!= ' ')
				break;
		comment[i + 1] = 0;
		gsf_input_read (in, 2776, NULL);
	} else {
		for (i = 0; i < 44; i++) // doc says 42, but two values are undocumented
			READINT32 (in, i32); // unassigned
		READFLOAT (in, temp);
		gsf_input_read (in, 40, reinterpret_cast <guint8 *> (desc));
		for (i = 40; i >= 0; i--)
			if (desc[i]!= ' ' && desc[i] != 0)
				break;
		desc[i + 1] = 0;
		READFLOAT (in, pw);
		READFLOAT (in, rd);
		READINT32 (in, nbacq);
		gsf_input_read (in, 40, reinterpret_cast <guint8 *> (uname));
		for (i = 40; i >= 0; i--)
			if (uname[i]!= ' ')
				break;
		uname[i + 1] = 0;
		gsf_input_read (in, 32, reinterpret_cast <guint8 *> (date));
		for (i = 32; i >= 0; i--)
			if (date[i]!= ' ')
				break;
		date[i + 1] = 0;
		gsf_input_read (in, 84, reinterpret_cast <guint8 *> (comment));
		for (i = 84; i >= 0; i--)
			if (comment[i]!= ' ')
				break;
		comment[i + 1] = 0;
		gsf_input_read (in, 4, NULL); // undocumented word
	}
	// now read the data, since we support only 1D spectra, we should find only one slice
	READINT32 (in, i32); // data number
	if (use_ints) {
		switch (data_type1) {
		case 0: {
			std::ostringstream re;
			READINT32 (in, i32);
			re << i32;
			for (i = 1; i < npts1; i++) {
				READINT32 (in, i32);
				re << " " << i32;
			}
			doc->SetProperty (GCU_PROP_SPECTRUM_DATA_Y, re.str ().c_str ());
			break;
		}
		case 1: {
			std::ostringstream re, im;
			READINT32 (in, i32);
			re << i32;
			READINT32 (in, i32);
			im << i32;
			for (i = 1; i < npts1; i++) {
				READINT32 (in, i32);
				re << " " << i32;
				READINT32 (in, i32);
				im << " " << i32;
			}
			doc->SetProperty (GCU_PROP_SPECTRUM_DATA_REAL, re.str ().c_str ());
			doc->SetProperty (GCU_PROP_SPECTRUM_DATA_IMAGINARY, im.str ().c_str ());
			break;
		}
		case 2: // unsupported for now, needs more info and at least one sample
		default:
			return gcu::ContentTypeUnknown;
		}
	} else {
		switch (data_type1) {
		case 0: {
			std::ostringstream re;
			READFLOAT (in, f);
			re << f;
			for (i = 1; i < npts1; i++) {
				READFLOAT (in, f);
				re << " " << f;
			}
			doc->SetProperty (GCU_PROP_SPECTRUM_DATA_Y, re.str ().c_str ());
			break;
		}
		case 1: {
			std::ostringstream re, im;
			READFLOAT (in, f);
			re << f;
			READFLOAT (in, f);
			im << f;
			for (i = 1; i < npts1; i++) {
				READFLOAT (in, f);
				re << " " << f;
				READFLOAT (in, f);
				im << " " << f;
			}
			doc->SetProperty (GCU_PROP_SPECTRUM_DATA_REAL, re.str ().c_str ());
			doc->SetProperty (GCU_PROP_SPECTRUM_DATA_IMAGINARY, im.str ().c_str ());
			break;
		}
		case 2: // unsupported for now, needs more info and at least one sample
		default:
			return gcu::ContentTypeUnknown;
		}
	}
	doc->Loaded ();
	return gcu::ContentTypeSpectrum;
}

////////////////////////////////////////////////////////////////////////////////
// Initialization

static NUTSLoader loader;

extern "C" {

extern GOPluginModuleDepend const go_plugin_depends [] = {
    { "goffice", GOFFICE_API_VERSION }
};
extern GOPluginModuleHeader const go_plugin_header =
	{ GOFFICE_MODULE_PLUGIN_MAGIC_NUMBER, G_N_ELEMENTS (go_plugin_depends) };

G_MODULE_EXPORT void
go_plugin_init (G_GNUC_UNUSED GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
	bindtextdomain (GETTEXT_PACKAGE, DATADIR"/locale");
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

G_MODULE_EXPORT void
go_plugin_shutdown (G_GNUC_UNUSED GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
}

}
