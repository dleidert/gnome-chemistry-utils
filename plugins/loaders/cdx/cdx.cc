// -*- C++ -*-

/* 
 * CDXML files loader plugin
 * cdx.cc 
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
#include <gcu/document.h>
#include <gcu/loader.h>
#include <gcu/objprops.h>

#include <goffice/app/module-plugin-defs.h>
#include <openbabel/chemdrawcdx.h>
#include <libintl.h>
#include <cstdio>

using namespace std;
using namespace gcu;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define READINT16(input,buf,i) gsf_input_read (input, 2, (guint8*) &i)
#define READINT32(input,buf,i) gsf_input_read (input, 4, (guint8*) &i)
#else
#define READINT16(input,buf,i) \
	bool res = gsf_input_read (input, 2, (guint8*) buf), \
	(guint16) i = buf[0] + buf[1] << 8, res
#define READINT32(input,buf,i) \
	bool res = gsf_input_read (input, 4, (guint8*) buf), \
	(guint32) i = buf[0] + buf[1] << 8 + buf[2] << 16 + buf[3] << 24, res
#endif

class CDXLoader: public gcu::Loader
{
public:
	CDXLoader ();
	virtual ~CDXLoader ();

	bool Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io);

private:
	bool ReadGenericObject (GsfInput *in);
	bool ReadPage (GsfInput *in, Object *parent);
	bool ReadMolecule (GsfInput *in, Object *parent);
	bool ReadAtom (GsfInput *in, Object *parent);
	bool ReadBond (GsfInput *in, Object *parent);
	guint16 ReadSize (GsfInput *in);

private:
	char *buf;
	size_t bufsize;
};

CDXLoader::CDXLoader ()
{
	AddMimeType ("chemical/x-cdx");
}

CDXLoader::~CDXLoader ()
{
	RemoveMimeType ("chemical/x-cdx");
}

bool CDXLoader::Read  (Document *doc, GsfInput *in, char const *mime_type, IOContext *io)
{
	bool result = true;
	guint16 code;
	bufsize = 64;
	buf = new char [bufsize];
	// note that we read 28 bytes here while headers for recent cdx files have only 22 bytes, remaining are 0x8000 (document) and its id (0)
	if (!gsf_input_read (in, kCDX_HeaderLength, (guint8*) buf) || strncmp (buf, kCDX_HeaderString, kCDX_HeaderStringLen)) {
		result = false;
		code = 0;
	} else if (!READINT16 (in, buf, code)) {
		result = false;
		code = 0;
	}

	while (code) {
		if (code & kCDXTag_Object) {
			switch (code) {
			case kCDXObj_Page:
				result = ReadPage (in, doc);
				break;
			default:
				result = ReadGenericObject (in);
			}
		} else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff) {
				result = false;
				break;
			}
			switch (code) {
			default:
				if (size)
					result = (gsf_input_read (in, size, (guint8*) buf));
			}
		}
		if (!result)
			break;
		if (!READINT16 (in, buf, code)) {
			result = false;
			break;
		}
	}
	delete [] buf;
	return result;
}

bool CDXLoader::Write  (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io)
{
	return true;
}

bool CDXLoader::ReadGenericObject  (GsfInput *in)
{
	guint16 code;
	if (gsf_input_seek (in, 4, G_SEEK_CUR)) //skip the id
		return false;
	if (!READINT16 (in, buf, code))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			if (!ReadGenericObject (in))
				return false;
		} else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff)
				return false;
			if (size && !gsf_input_read (in, size, (guint8*) buf))
				return false;
		}
		if (!READINT16 (in, buf, code))
			return false;
	}
	return true;
}

bool CDXLoader::ReadPage (GsfInput *in, Object *parent)
{
	guint16 code;
	if (gsf_input_seek (in, 4, G_SEEK_CUR)) //skip the id
		return false;
	if (!READINT16 (in, buf, code))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			switch (code) {
			case kCDXObj_Fragment:
				if (!ReadMolecule (in, parent))
					return false;
				break;
			default:
				if (!ReadGenericObject (in))
					return false;
			}
		} else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff)
				return false;
			if (size && !gsf_input_read (in, size, (guint8*) buf))
				return false;
		}
		if (!READINT16 (in, buf, code))
			return false;
	}
	return true;
}

bool CDXLoader::ReadMolecule (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Molecule = Object::CreateObject ("molecule", parent);
	guint32 Id;
	if (!READINT32 (in, buf, Id))
		return false;
	snprintf (buf, bufsize, "m%d", Id);
	Molecule->SetId (buf);
	if (!READINT16 (in, buf, code))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			switch (code) {
			case kCDXObj_Node:
				if (!ReadAtom (in, Molecule))
					return false;
				break;
			case kCDXObj_Bond:
				if (!ReadBond (in, Molecule))
					return false;
				break;
			default:
				if (!ReadGenericObject (in))
					return false;
			}
		} else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff)
				return false;
			if (size && !gsf_input_read (in, size, (guint8*) buf))
				return false;
		}
		if (!READINT16 (in, buf, code))
			return false;
	}
	return true;
}

bool CDXLoader::ReadAtom (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Atom = Object::CreateObject ("atom", parent);
	Atom->SetProperty (GCU_PROP_ATOM_Z, "6");
	guint32 Id;
	if (!READINT32 (in, buf, Id))
		return false;
	snprintf (buf, bufsize, "a%d", Id);
	Atom->SetId (buf);
	if (!READINT16 (in, buf, code))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			if (!ReadGenericObject (in))
				return false;
		} else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff)
				return false;
			switch (code) {
			case kCDXProp_2DPosition: {
				if (size != 8)
					return false;
				float x, y;
				if (!READINT32 (in, buf, x))
					return false;
				if (!READINT32 (in, buf, y))
					return false;
				snprintf (buf, bufsize, "%g %g", x, y);
				if (!Atom->SetProperty (GCU_PROP_POS2D, buf))
				break;
			}
			case kCDXProp_Node_Element:
				if (size != 2)
					return false;
				if (!READINT16 (in, buf, size))
					return false;
				snprintf (buf, bufsize, "%u", size);
				Atom->SetProperty (GCU_PROP_ATOM_Z, buf);
				break;
			default:
				if (size && !gsf_input_read (in, size, (guint8*) buf))
					return false;
			}
		}
		if (!READINT16 (in, buf, code))
			return false;
	}
	return true;
}

bool CDXLoader::ReadBond (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Bond = Object::CreateObject ("bond", parent);
	guint32 Id;
	if (!READINT32 (in, buf, Id))
		return false;
	snprintf (buf, bufsize, "b%d", Id);
	Bond->SetId (buf);
	Bond->SetProperty (GCU_PROP_BOND_ORDER, "1");
	if (!READINT16 (in, buf, code))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			if (!ReadGenericObject (in))
				return false;
		} else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff)
				return false;
			switch (code) {
			case kCDXProp_Bond_Begin: {
				if (size != 4)
					return false;
				if (!READINT32 (in, buf, Id))
					return false;
				snprintf (buf, bufsize, "a%d", Id);
				Bond->SetProperty (GCU_PROP_BOND_BEGIN, buf);
				break;
			}
			case kCDXProp_Bond_End: {
				if (size != 4)
					return false;
				if (!READINT32 (in, buf, Id))
					return false;
				snprintf (buf, bufsize, "a%d", Id);
				Bond->SetProperty (GCU_PROP_BOND_END, buf);
				break;
			}
			case kCDXProp_Bond_Order:
				if (size != 2)
					return false;
				if (!READINT16 (in, buf, size))
					return false;
				snprintf (buf, bufsize, "%u", size);
				Bond->SetProperty (GCU_PROP_BOND_ORDER, buf);
				break;
			default:
				if (size && !gsf_input_read (in, size, (guint8*) buf))
					return false;
			}
		}
		if (!READINT16 (in, buf, code))
			return false;
	}
	return true;
}

guint16 CDXLoader::ReadSize  (GsfInput *in)
{
	guint16 size;
	if (!READINT16 (in, buf, size))
		return 0xffff;
	if (size > bufsize) {
		do
			bufsize <<= 1;
		while (size > bufsize);
		delete [] buf;
		buf = new char [bufsize];	
	}
	return size;
}

static CDXLoader loader;

extern "C" {

extern GOPluginModuleDepend const go_plugin_depends [] = {
    { "goffice", GOFFICE_API_VERSION }
};
extern GOPluginModuleHeader const go_plugin_header =
	{ GOFFICE_MODULE_PLUGIN_MAGIC_NUMBER, G_N_ELEMENTS (go_plugin_depends) };

G_MODULE_EXPORT void
go_plugin_init (GOPlugin *plugin, GOCmdContext *cc)
{
	bindtextdomain (GETTEXT_PACKAGE, DATADIR"/locale");
#ifdef ENABLE_NLS
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
}

G_MODULE_EXPORT void
go_plugin_shutdown (GOPlugin *plugin, GOCmdContext *cc)
{
}

}
