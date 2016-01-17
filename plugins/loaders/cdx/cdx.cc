// -*- C++ -*-

/*
 * CDXML files loader plugin
 * cdx.cc
 *
 * Copyright (C) 2007-2016 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/application.h>
#include <gcu/bond.h>
#include <gcu/document.h>
#include <gcu/element.h>
#include <gcu/formula.h>
#include <gcu/loader.h>
#include <gcu/molecule.h>
#include <gcu/objprops.h>
#include <gcu/residue.h>
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/fragment.h>
#include <gcp/theme.h>

#include <goffice/app/module-plugin-defs.h>
#include <gsf/gsf-output-memory.h>
#include <glib/gi18n-lib.h>
#include <openbabel/chemdrawcdx.h>
#include <map>
#include <string>
#include <vector>
#include <cstdio>
#include <libintl.h>
#include <cstring>
#include <sstream>

#include <libxml/tree.h>

using namespace std;
using namespace gcu;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define READINT16(input,i) gsf_input_read (input, 2, (guint8*) &i)
#define READINT32(input,i) gsf_input_read (input, 4, (guint8*) &i)
#define WRITEINT16(output,i) gsf_output_write  (output, 2, (guint8*) &i)
#define WRITEINT32(output,i) gsf_output_write  (output, 4, (guint8*) &i)
#else
unsigned char buffer[4];
bool readint_res;
#define READINT16(input,i) \
	readint_res = gsf_input_read (input, 2, (guint8*) buffer), \
	i = buffer[0] + (buffer[1] << 8), readint_res
#define READINT32(input,i) \
	readint_res = gsf_input_read (input, 4, (guint8*) buffer), \
	i = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24), readint_res
#define WRITEINT16(output,i) \
	gsf_output_write  (output, 1, (guint8*) (&i) + 1);\
	gsf_output_write  (output, 1, (guint8*) &i)
#define WRITEINT32(output,i) \
	gsf_output_write  (output, 1, (guint8*) (&i) + 3);\
	gsf_output_write  (output, 1, (guint8*) (&i) + 2);\
	gsf_output_write  (output, 1, (guint8*) (&i) + 1);\
	gsf_output_write  (output, 1, (guint8*) &i)
#endif

typedef struct {
	guint16 index;
	guint16 encoding;
	string name;
} CDXFont;

static map<guint16, string> Charsets;
static map<string, guint16> CharsetIDs;

static gint32 ReadInt (GsfInput *input, int size)
{
	gint32 res = 0;
	switch (size) {
	case 1:
		gsf_input_read (input, 1, (guint8*) &res);
		break;
	case 2:
		READINT16 (input, res);
		break;
	case 4:
		READINT32 (input, res);
		break;
	}
	return res;
}

/*static guint32 ReadUInt (GsfInput *input, int size)
{
	guint32 res = 0;
	switch (size) {
	case 1:
		gsf_input_read (input, 1, (guint8*) &res);
		break;
	case 2:
		READINT16 (input, res);
		break;
	case 4:
		READINT32 (input, res);
		break;
	}
	return res;
}*/

typedef struct {
	guint32 Arrow;
	std::list < unsigned > Reagents, Products, ObjectsAbove, ObjectsBelow;
} StepData;

class CDXLoader: public gcu::Loader
{
public:
	CDXLoader ();
	virtual ~CDXLoader ();

	ContentType Read (Document *doc, GsfInput *in, char const *mime_type, GOIOContext *io);
	bool Write (Object const *obj, GsfOutput *out, char const *mime_type, GOIOContext *io, ContentType type);

private:
	bool ReadGenericObject (GsfInput *in);
	bool ReadPage (GsfInput *in, Object *parent);
	bool ReadMolecule (GsfInput *in, Object *parent);
	bool ReadAtom (GsfInput *in, Object *parent);
	bool ReadBond (GsfInput *in, Object *parent);
	bool ReadText (GsfInput *in, Object *parent);
	bool ReadGroup (GsfInput *in, Object *parent);
	bool ReadGraphic (GsfInput *in, Object *parent);
	bool ReadFragmentText (GsfInput *in, Object *parent);
	bool ReadScheme (GsfInput *in, Object *parent);
	bool ReadStep (GsfInput *in, Object *parent);
	bool ReadArrow (GsfInput *in, Object *parent);
	guint16 ReadSize (GsfInput *in);
	bool ReadDate (GsfInput *in);

	bool WriteObject (GsfOutput *out, Object const *object, GOIOContext *io);
	static void AddInt16Property (GsfOutput *out, gint16 prop, gint16 value);
	static void AddInt32Property (GsfOutput *out, gint16 prop, gint32 value);
	static void WriteSimpleStringProperty (GsfOutput *out, gint16 id, gint16 length, char const *data);
	static bool WriteAtom (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteBond (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteMolecule (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	void WriteId (Object const *obj, GsfOutput *out);

private:
	char *buf;
	size_t bufsize;
	map<unsigned, CDXFont> m_Fonts;
	vector <string> colors;
	guint8 m_TextAlign, m_TextJustify;

	map <string, bool (*) (CDXLoader *, GsfOutput *, Object const *, GOIOContext *)> m_WriteCallbacks;
	map<unsigned, GOColor> m_Colors;
	map <string, gint32> m_SavedIds;
	std::map <gint32, std::string> m_LoadedIds;
	std::map <gint32, gint32> m_Superseded;
	std::list <StepData > m_Scheme;
	gint32 m_MaxId;
	unsigned m_Z;
};

CDXLoader::CDXLoader ():
	m_TextAlign (0),
	m_TextJustify (0)
{
	AddMimeType ("chemical/x-cdx");
	// Add write callbacks
	m_WriteCallbacks["atom"] = WriteAtom;
	m_WriteCallbacks["bond"] = WriteBond;
	m_WriteCallbacks["molecule"] = WriteMolecule;
}

CDXLoader::~CDXLoader ()
{
	RemoveMimeType ("chemical/x-cdx");
}

ContentType CDXLoader::Read  (Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io)
{
	if (doc == NULL || in == NULL)
		return ContentTypeUnknown;
	ContentType result = ContentType2D;
	guint16 code, labelfont = 0, captionfont = 0;
	bufsize = 64;
	buf = new char [bufsize];
	double bond_dist_ratio = -1., bond_length = 30.;
	ostringstream themedesc;
	gcp::Theme *theme = NULL;
	themedesc << "<?xml version=\"1.0\"?>" << std::endl << "<theme name=\"ChemDraw\"";
	// note that we read 28 bytes here while headers for recent cdx files have only 22 bytes, remaining are 0x8000 (document) and its id (0)
	if (!gsf_input_read (in, kCDX_HeaderLength, (guint8*) buf) || strncmp (buf, kCDX_HeaderString, kCDX_HeaderStringLen)) {
		result = ContentTypeUnknown;
		code = 0;
	} else if (!(READINT16 (in, code))) {
		result = ContentTypeUnknown;
		code = 0;
	}

	// set a default scale
	doc->SetProperty (GCU_PROP_THEME_BOND_LENGTH, "1966080");

	while (code) {
		if (code & kCDXTag_Object) {
			if (theme == NULL) { // time to set the theme
				gcp::Document *cpDoc = dynamic_cast <gcp::Document *> (doc);
				if (cpDoc != NULL) {
					if (bond_dist_ratio > 0.) 
						themedesc << " bond-dist=\"" << bond_length * bond_dist_ratio / 3. << "\"";
					themedesc << "/>";
					xmlDocPtr xml = xmlParseMemory (themedesc.str().c_str(), themedesc.str().length());
					theme = new gcp::Theme (NULL);
					theme->Load (xml->children);
					xmlFreeDoc (xml);
					gcp::Theme *LocalTheme = gcp::TheThemeManager.GetTheme (theme->GetName ().c_str ());
					if (LocalTheme && *LocalTheme == *theme) {
						cpDoc->SetTheme (LocalTheme);
						delete theme;
					} else {
						gcp::TheThemeManager.AddFileTheme (theme, doc->GetTitle ().c_str ());
						cpDoc->SetTheme (theme);
					}
				}
			}
			switch (code) {
			case kCDXObj_Page:
				if (!ReadPage (in, doc))
					result = ContentTypeUnknown;
				break;
			default:
				if (!ReadGenericObject (in))
					result = ContentTypeUnknown;
			}
		} else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff) {
				result = ContentTypeUnknown;
				break;
			}
			switch (code) {
			case kCDXProp_CreationUserName:
				gsf_input_read (in, size, (guint8*) buf);
				doc->SetProperty (GCU_PROP_DOC_CREATOR, buf);
				break;
			case kCDXProp_CreationDate: {
				if (size != 14 || !ReadDate (in)) {
					result = ContentTypeUnknown;
					break;
				}
				doc->SetProperty (GCU_PROP_DOC_CREATION_TIME, buf);
				break;
			}
			case kCDXProp_ModificationDate:{
				if (size != 14 || !ReadDate (in)) {
					result = ContentTypeUnknown;
					break;
				}
				gsf_input_read (in, size, (guint8*) buf);
				doc->SetProperty (GCU_PROP_DOC_MODIFICATION_TIME, buf);
				break;
			}
			case kCDXProp_Name:
				gsf_input_read (in, size, (guint8*) buf);
				doc->SetProperty (GCU_PROP_DOC_TITLE, buf);
				break;
			case kCDXProp_Comment:
				gsf_input_read (in, size, (guint8*) buf);
				doc->SetProperty (GCU_PROP_DOC_COMMENT, buf);
				break;
			case kCDXProp_BondLength: {
				if (size != 4) {
					result = ContentTypeUnknown;
					break;
				}
				guint32 length;
				if (!(READINT32 (in,length))) {
					result = ContentTypeUnknown;
					break;
				}
				ostringstream str;
				str << length;
				doc->SetProperty (GCU_PROP_THEME_BOND_LENGTH, str.str ().c_str ());
				bond_length = length / 16384.;
				themedesc << " bond-length=\"" << bond_length << "\" zoom-factor=\"3\"";
				break;
			}
			case kCDXProp_BondSpacing: {
				if (size != 2) {
					result = ContentTypeUnknown;
					break;
				}
				guint16 val;
				if (!(READINT16 (in, val))) {
					result = ContentTypeUnknown;
					break;
				}
				bond_dist_ratio = val / 1000.;
				break;
			}
			case kCDXProp_LineWidth: {
				if (size != 4) {
					result = ContentTypeUnknown;
					break;
				}
				guint32 length;
				if (!(READINT32 (in,length))) {
					result = ContentTypeUnknown;
					break;
				}
				double x = length / 16384. / 3.;
				themedesc << " bond-width=\"" << x << "\" arrow-width=\"" << x << "\" hash-width=\"" << x << "\"";
				break;
			}
			case kCDXProp_BoldWidth: {
				if (size != 4) {
					result = ContentTypeUnknown;
					break;
				}
				guint32 length;
				if (!(READINT32 (in,length))) {
					result = ContentTypeUnknown;
					break;
				}
				// for wedges chemdraw uses 1,5 times this value, but this is not the case for GChemPaint, at least for now.
				themedesc << " stereo-bond-width=\"" << length / 16384. / 3. << "\"";
				break;
			}
			case kCDXProp_HashSpacing: {
				if (size != 4) {
					result = ContentTypeUnknown;
					break;
				}
				guint32 length;
				if (!(READINT32 (in,length))) {
					result = ContentTypeUnknown;
					break;
				}
				// for wedges chemdraw uses 1,5 times this value, but this is not the case for GChemPaint, at least for now.
				themedesc << " hash-dist=\"" << length / 16384. / 3. << "\"";
				break;
			}
			case kCDXProp_ChainAngle: {
				if (size != 4) {
					result = ContentTypeUnknown;
					break;
				}
				guint32 length;
				if (!(READINT32 (in, length))) {
					result = ContentTypeUnknown;
					break;
				}
				themedesc << " bond-angle=\"" << (length / 65536) << "\"";
				break;
			}
			case kCDXProp_MarginWidth: {
				if (size != 4) {
					result = ContentTypeUnknown;
					break;
				}
				guint32 length;
				if (!(READINT32 (in,length))) {
					result = ContentTypeUnknown;
					break;
				}
				double x = length / 16384. / 3.;
				themedesc << " padding=\"" << x << "\" arrow-padding=\"" << x << "\" object-padding=\"" << x << "\" sign-padding=\"" << x << "\"";
				break;
			}
			case kCDXProp_CaptionStyle: {
				guint16 i;
				if (size != 8) {
					result = ContentTypeUnknown;
					break;
				}
				if (!(READINT16 (in, captionfont))) {
					result = ContentTypeUnknown;
					break;
				}
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				switch (i & 3) { // we do not support anything else
				default:
				case 0:
					themedesc << " text-font-style=\"normal\" text-font_weight=\"normal\"";
					break;
				case 1:
					themedesc << " text-font-style=\"normal\" text-font_weight=\"bold\"";
					break;
				case 2:
					themedesc << " text-font-style=\"italic\" text-font_weight=\"normal\"";
					break;
				case 3:
					themedesc << " text-font-style=\"italic\" text-font_weight=\"bold\"";
					break;
				}
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				themedesc << " text-font-size=\"" << i * PANGO_SCALE / 20 << "\"";
				// read the color, but don't use it for now since GChemPaint does not support
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				break;
			}
			case kCDXProp_CaptionStyleFont:
				if (size != 2) {
					result = ContentTypeUnknown;
					break;
				}
				if (!(READINT16 (in, captionfont))) {
					result = ContentTypeUnknown;
					break;
				}
				break;
			case kCDXProp_CaptionStyleSize: {
				if (size != 2) {
					result = ContentTypeUnknown;
					break;
				}
				int i;
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				themedesc << " text-font-size=\"" << i * PANGO_SCALE / 20 << "\"";
				break;
			}
			case kCDXProp_CaptionStyleFace: {
				if (size != 2) {
					result = ContentTypeUnknown;
					break;
				}
				int i;
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				switch (i & 3) { // we do not support anything else
				default:
				case 0:
					themedesc << " text-font-style=\"normal\" text-font_weight=\"normal\"";
					break;
				case 1:
					themedesc << " text-font-style=\"normal\" text-font_weight=\"bold\"";
					break;
				case 2:
					themedesc << " text-font-style=\"italic\" text-font_weight=\"normal\"";
					break;
				case 3:
					themedesc << " text-font-style=\"italic\" text-font_weight=\"bold\"";
					break;
				}
				break;
			}
			case kCDXProp_LabelStyle: {
				guint16 i;
				if (size != 8) {
					result = ContentTypeUnknown;
					break;
				}
				if (!(READINT16 (in, labelfont))) {
					result = ContentTypeUnknown;
					break;
				}
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				switch (i & 3) { // we do not support anything else
				default:
				case 0:
					themedesc << " font-style=\"normal\" font_weight=\"normal\"";
					break;
				case 1:
					themedesc << " font-style=\"normal\" font_weight=\"bold\"";
					break;
				case 2:
					themedesc << " font-style=\"italic\" font_weight=\"normal\"";
					break;
				case 3:
					themedesc << " font-style=\"italic\" font_weight=\"bold\"";
					break;
				}
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				themedesc << " font-size=\"" << i * PANGO_SCALE / 20 << "\"";
				// read the color, but don't use it for now since GChemPaint does not support
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				break;
			}
			case kCDXProp_LabelStyleFont:
				if (size != 2) {
					result = ContentTypeUnknown;
					break;
				}
				if (!(READINT16 (in, labelfont))) {
					result = ContentTypeUnknown;
					break;
				}
				break;
			case kCDXProp_LabelStyleSize: {
				if (size != 2) {
					result = ContentTypeUnknown;
					break;
				}
				int i;
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				themedesc << " font-size=\"" << i * PANGO_SCALE / 20 << "\"";
				break;
			}
			case kCDXProp_LabelStyleFace: {
				if (size != 2) {
					result = ContentTypeUnknown;
					break;
				}
				int i;
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				switch (i & 3) { // we do not support anything else
				default:
				case 0:
					themedesc << " font-style=\"normal\" font_weight=\"normal\"";
					break;
				case 1:
					themedesc << " font-style=\"normal\" font_weight=\"bold\"";
					break;
				case 2:
					themedesc << " font-style=\"italic\" font_weight=\"normal\"";
					break;
				case 3:
					themedesc << " font-style=\"italic\" font_weight=\"bold\"";
					break;
				}
				break;
			}
			case kCDXProp_FontTable: {
				// skip origin platform and read fonts number
				guint16 nb;
				if (gsf_input_seek (in, 2, G_SEEK_CUR) || !(READINT16 (in,nb)))
					return ContentTypeUnknown;
				CDXFont font;
				for (int i = 0; i < nb; i++) {
					if (!(READINT16 (in,font.index)) ||
						!(READINT16 (in,font.encoding)) ||
						!(READINT16 (in,size)))
						return ContentTypeUnknown;
					gsf_input_read (in, size, (guint8*) buf);
					buf[size] = 0;
					font.name = buf;
					m_Fonts[font.index] = font;
				}
				std::map < unsigned, CDXFont>::iterator i = m_Fonts.find (labelfont);
				if (i != m_Fonts.end ())
					themedesc << " font-family=\"" << (*i).second.name.c_str() << "\"";
				i = m_Fonts.find (captionfont);
				if (i != m_Fonts.end ())
					themedesc << " text-font-family=\"" << (*i).second.name.c_str() << "\"";
			}
			break;
			case kCDXProp_ColorTable: {
				colors.push_back ("red=\"0\" green=\"0\" blue=\"0\""); // black
				colors.push_back ("red=\"1\" green=\"1\" blue=\"1\""); // white
				unsigned nb = (size - 2) / 6;
				if (!(READINT16 (in,size)) || size != nb)
					return ContentTypeUnknown;
				guint16 red, blue, green;
				for (unsigned i = 0; i < nb; i++) {
				if (!(READINT16 (in,red)) || !(READINT16 (in,green)) || !(READINT16 (in,blue)))
					return ContentTypeUnknown;
					ostringstream str;
					str << "red=\"" << (double) red / 0xffff << "\" green=\"" << (double) green / 0xffff << "\" blue=\"" << (double) blue / 0xffff << "\"";
					colors.push_back (str.str ());
				}
				break;
			}
			case kCDXProp_CaptionJustification: {
				if (!gsf_input_read (in, 1, &m_TextAlign))
					return ContentTypeUnknown;
				break;
			}
			default:
				if (size)
					if (!gsf_input_read (in, size, (guint8*) buf))
						result = ContentTypeUnknown;
			}
		}
		if (result != ContentType2D)
			break;
		if (!(READINT16 (in,code))) {
			result = ContentTypeUnknown;
			break;
		}
	}
	delete [] buf;
	m_Fonts.clear ();
	m_LoadedIds.clear ();
	m_Superseded.clear ();
	return result;
}

/******************************************************************************
 *	Write callbacks															  *
 ******************************************************************************/

bool CDXLoader::WriteAtom (CDXLoader *loader, GsfOutput *out, Object const *obj, G_GNUC_UNUSED GOIOContext *s)
{
	gint16 n = kCDXObj_Node;
	double x, y;
	gint32 x_, y_;
	WRITEINT16 (out, n);
	loader->WriteId (obj, out);
	string prop = obj->GetProperty (GCU_PROP_POS2D);
	if (prop.length ()) {
		istringstream str (prop);
		str >> x >> y;
		x_ = x;
		y_ = y;
		n = kCDXProp_2DPosition;
		WRITEINT16 (out, n);
		gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x08\x00"));
		// write y first
		WRITEINT32 (out, y_);
		WRITEINT32 (out, x_);
	}
	AddInt16Property (out, kCDXProp_ZOrder, loader->m_Z++);
	prop = obj->GetProperty (GCU_PROP_ATOM_Z);
	if (prop != "6") {
		n = kCDXProp_Node_Element;
		WRITEINT16 (out, n);
		gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x02\x00"));
		n = strtol (prop.c_str (), NULL, 10);
		WRITEINT16 (out, n);
	}
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of atom
	return true;
}

bool CDXLoader::WriteBond (CDXLoader *loader, GsfOutput *out, Object const *obj, G_GNUC_UNUSED GOIOContext *s)
{
	gint16 n = kCDXObj_Bond;
	WRITEINT16 (out, n);
	loader->WriteId (obj, out);
	AddInt16Property (out, kCDXProp_ZOrder, loader->m_Z++);
	string prop = obj->GetProperty (GCU_PROP_BOND_BEGIN);
	AddInt32Property (out, kCDXProp_Bond_Begin, loader->m_SavedIds[prop]);
	prop = obj->GetProperty (GCU_PROP_BOND_END);
	AddInt32Property (out, kCDXProp_Bond_End, loader->m_SavedIds[prop]);
	prop = obj->GetProperty (GCU_PROP_BOND_ORDER);
	if (prop == "3")
		AddInt16Property (out, kCDXProp_Bond_Order, 4);
	else if (prop == "2")
		AddInt16Property (out, kCDXProp_Bond_Order, 2);
	prop = obj->GetProperty (GCU_PROP_BOND_TYPE);
	if (prop == "wedge")
		AddInt16Property (out, kCDXProp_Bond_Display, 6);
	else if (prop == "hash")
		AddInt16Property (out, kCDXProp_Bond_Display, 3);
	else if (prop == "squiggle")
		AddInt16Property (out, kCDXProp_Bond_Display, 8);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of bond
	return true;
}

bool CDXLoader::WriteMolecule (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s)
{
	gint16 n = kCDXObj_Fragment;
	WRITEINT16 (out, n);
	loader->WriteId (obj, out);
	// save atoms
	std::map <std::string, Object *>::const_iterator i;
	Object const *child = obj->GetFirstChild (i);
	while (child) {
		if (child->GetType () == AtomType && !loader->WriteObject (out, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	// save fragments
	child = obj->GetFirstChild (i);
	while (child) {
		if (child->GetType () == FragmentType && !loader->WriteObject (out, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	// save bonds
	child = obj->GetFirstChild (i);
	while (child) {
		if (child->GetType () == BondType && !loader->WriteObject (out, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of molecule
	return true;
}

bool CDXLoader::WriteObject (GsfOutput *out, Object const *object, GOIOContext *io)
{
	string name = Object::GetTypeName (object->GetType ());
	map <string, bool (*) (CDXLoader *, GsfOutput *, Object const *, GOIOContext *)>::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (this, out, object, io);
	// if we don't save the object iself, try tosave its children
	std::map <std::string, Object *>::const_iterator j;
	Object const *child = object->GetFirstChild (j);
	while (child) {
		if (!WriteObject (out, child, io))
			return false;
		child = object->GetNextChild (j);
	}
	return true; /* loosing data is not considered an error, it is just a missing feature
					either in this code or in the cml schema */
}

void CDXLoader::WriteSimpleStringProperty (GsfOutput *out, gint16 id, gint16 length, char const *data)
{
	WRITEINT16 (out, id);
	gint16 l = length + 2;
	WRITEINT16 (out, l);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00"));
	gsf_output_write (out, length, reinterpret_cast <guint8 const *> (data));
}

void CDXLoader::WriteId (Object const *obj, GsfOutput *out)
{
	m_SavedIds[obj->GetId ()] = m_MaxId;
	gint32 n = m_MaxId++;
	WRITEINT32 (out, n);
}

void CDXLoader::AddInt16Property (GsfOutput *out, gint16 prop, gint16 value)
{
	WRITEINT16 (out, prop);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x02\x00"));
	WRITEINT16 (out, value);
}

void CDXLoader::AddInt32Property (GsfOutput *out, gint16 prop, gint32 value)
{
	WRITEINT16 (out, prop);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x04\x00"));
	WRITEINT32 (out, value);
}

bool CDXLoader::Write  (Object const *obj, GsfOutput *out, G_GNUC_UNUSED G_GNUC_UNUSED char const *mime_type, GOIOContext *io, G_GNUC_UNUSED ContentType type)
{
	Document const *doc = dynamic_cast <Document const *> (obj);
	gint16 n;
	gint32 l;
	// FIXME: should be able to export a molecule or any object actually
	if (!doc || !out)
		return false;

	m_MaxId = m_Z = 1;

	// Init colors
	m_Colors[2] = GO_COLOR_WHITE;
	m_Colors[3] = GO_COLOR_BLACK;
	m_Colors[4] = GO_COLOR_RED;
	m_Colors[5] = GO_COLOR_YELLOW;
	m_Colors[6] = GO_COLOR_GREEN;
	m_Colors[7] = GO_COLOR_CYAN;
	m_Colors[8] = GO_COLOR_BLUE;
	m_Colors[9] = GO_COLOR_VIOLET;

	// Init fonts, we always use Unknown as the charset, hoping it is not an issue
	m_Fonts[3] = (CDXFont) {3, kCDXCharSetUnknown, string ("Arial")};
	m_Fonts[4] = (CDXFont) {4, kCDXCharSetUnknown, string ("Times New Roman")};

	gsf_output_write (out, kCDX_HeaderStringLen, (guint8 const *) kCDX_HeaderString);
	gsf_output_write (out, kCDX_HeaderLength - kCDX_HeaderStringLen, (guint8 const *) "\x04\x03\x02\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00");
	std::string app = doc->GetApp ()->GetName () + " "VERSION;
	WriteSimpleStringProperty (out, kCDXProp_CreationProgram, app.length (), app.c_str ());
	// determine the bond length and scale the document appropriately
	string prop = doc->GetProperty (GCU_PROP_THEME_BOND_LENGTH);
	double scale = strtod (prop.c_str (), NULL);
	const_cast <Document *> (doc)->SetScale (scale / 1966080.);
	n = kCDXProp_BondLength;
	WRITEINT16 (out, n);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x04\x00"));
	l = 30;
	WRITEINT32 (out, l);
	// write the document contents
	// there is a need for a two paths procedure
	// in the first path, we collect fonts and colors
	// everything else is saved during the second path
	// write end of document and end of file
	GsfOutput *buf = gsf_output_memory_new ();
	n = kCDXObj_Page;
	WRITEINT16 (buf, n);
	l = 0;
	WRITEINT32 (buf, l); // id = 0 for the page
	std::map <std::string, Object *>::const_iterator i;
	Object const *child = doc->GetFirstChild (i);
	while (child) {
		if (!WriteObject (buf, child, io)) {
			g_object_unref (buf);
			m_Colors.clear ();
			m_Fonts.clear ();
			m_SavedIds.clear ();
			return false;
		}
		child = doc->GetNextChild (i);
	}
	// write colors
	n = kCDXProp_ColorTable;
	WRITEINT16 (out, n);
	n = m_Colors.size () * 6 + 2;
	WRITEINT16 (out, n);
	n = m_Colors.size ();
	WRITEINT16 (out, n);
	map <unsigned, GOColor>::iterator color, end_color = m_Colors.end ();
	for (color = m_Colors.begin (); color != end_color; color++) {
		n = GO_COLOR_UINT_R ((*color).second) * 0x101;
		WRITEINT16 (out, n);
		n = GO_COLOR_UINT_G ((*color).second) * 0x101;
		WRITEINT16 (out, n);
		n = GO_COLOR_UINT_B ((*color).second) * 0x101;
		WRITEINT16 (out, n);
	}

	// write fonts
	n = kCDXProp_FontTable;
	WRITEINT16 (out, n);
	n = 4;
	map <unsigned, CDXFont>::iterator font, end_font = m_Fonts.end ();
	for (font = m_Fonts.begin (); font != end_font; font++)
		n += 6 + (*font).second.name.length ();
	WRITEINT16 (out, n);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // say we are on a mac even if not true
	n = m_Fonts.size ();
	WRITEINT16 (out, n);
	for (font = m_Fonts.begin (); font != end_font; font++) {
		WRITEINT16 (out, (*font).second.index);
		WRITEINT16 (out, (*font).second.encoding);
		n = (*font).second.name.length ();
		WRITEINT16 (out, n);
		gsf_output_write (out, n,
		                  reinterpret_cast <guint8 const *> ((*font).second.name.c_str ()));
	}

	// write the objects
	gint64 size;
	g_object_get (buf, "size", &size, NULL);
	if (size > 0)
		gsf_output_write (out, size, gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (buf)));
	g_object_unref (buf);
	gsf_output_write (out, 4, (guint8 const *) "\x00\x00\x00\x00");
	m_Colors.clear ();
	m_Fonts.clear ();
	m_SavedIds.clear ();
	return true;
}

bool CDXLoader::ReadGenericObject  (GsfInput *in)
{
	guint16 code;
	if (gsf_input_seek (in, 4, G_SEEK_CUR)) //skip the id
		return false;
	if (!(READINT16 (in,code)))
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
		if (!(READINT16 (in,code)))
			return false;
	}
	return true;
}

bool CDXLoader::ReadPage (GsfInput *in, Object *parent)
{
	guint16 code;
	if (gsf_input_seek (in, 4, G_SEEK_CUR)) //skip the id
		return false;
	if (!(READINT16 (in,code)))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			switch (code) {
			case kCDXObj_Group:
				if (!ReadGroup (in, parent))
					return false;
				break;
			case kCDXObj_Fragment:
				if (!ReadMolecule (in, parent))
					return false;
				break;
			case kCDXObj_Text:
				if (!ReadText (in, parent))
					return false;
				break;
			case kCDXObj_Graphic:
				if (!ReadGraphic (in, parent))
					return false;
				break;
			case kCDXObj_ReactionScheme:
				if (!ReadScheme (in, parent))
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
		if (!(READINT16 (in,code)))
			return false;
	}
	return true;
}

bool CDXLoader::ReadMolecule (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *mol = parent->GetApplication ()->CreateObject ("molecule", parent);
	guint32 Id;
	if (!(READINT32 (in,Id)))
		return false;
	ostringstream str;
	str << "m" << Id;
	mol->SetId (str.str ().c_str ());
	m_LoadedIds[Id] = mol->GetId ();
	if (!(READINT16 (in,code)))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			switch (code) {
			case kCDXObj_Node:
				if (!ReadAtom (in, mol))
					return false;
				break;
			case kCDXObj_Bond:
				if (!ReadBond (in, mol))
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
		if (!(READINT16 (in,code)))
			return false;
	}
		static_cast <Molecule*> (mol)->UpdateCycles ();
	parent->GetDocument ()->ObjectLoaded (mol);
	return true;
}

bool CDXLoader::ReadAtom (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Atom = parent->GetApplication ()->CreateObject ("atom", parent);
	Document *Doc = NULL;
	Atom->SetProperty (GCU_PROP_ATOM_Z, "6");
	guint32 Id;
	int type = 0;
	int Z = 6;
	if (!(READINT32 (in,Id)))
		return false;
	ostringstream str;
	str << "a" << Id;
	Atom->SetId (str.str ().c_str ());
	m_LoadedIds[Id] = Atom->GetId ();
	if (!(READINT16 (in,code)))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			switch (code) {
			case kCDXObj_Fragment: {
				if (!Doc)
					Doc = parent->GetDocument ()->GetApp ()->CreateNewDocument ();
				Doc->SetProperty (GCU_PROP_THEME_BOND_LENGTH, "943718");
				if (!ReadMolecule (in, Doc)) {
					delete Doc;
					return false;
				}
				break;
			}
			case kCDXObj_Text:
				if (Z == 6) {
					if (!ReadFragmentText (in, Atom))
						goto bad_exit;
					switch (type) {
					case 0:
						// Parse the formula.
						try {
							Formula form (buf, GCU_FORMULA_PARSE_RESIDUE);
							if (Doc) {
								map< string, Object * >::iterator i;
								Molecule *mol = dynamic_cast <Molecule *> (Doc->GetFirstChild (i));
								if (Doc->GetChildrenNumber () != 1 || mol == NULL)
									goto bad_exit;
								// compare the formula as interpreted with the document contents
								// TODO: write this code
							}
							// now build a molecule from the formula
							Molecule *mol2 = NULL;
							if (Doc)
								mol2 = Molecule::MoleculeFromFormula (Doc, form);
							bool replace = true;
							if (mol2) {
							} else {
								// check if the formula contains only one atom
								std::list<FormulaElt *> const &items = form.GetElements ();
								if (items.size () == 1 && dynamic_cast <FormulaAtom const *> (items.front ()))
									replace = false;
							}
							if (replace) {
								string pos = Atom->GetProperty (GCU_PROP_POS2D);
								Molecule *mol = dynamic_cast <Molecule *> (parent);
								if (mol)
									mol->Remove (Atom);
								delete Atom;
								Atom = parent->GetApplication ()->CreateObject ("fragment", parent);
								Atom->SetProperty (GCU_PROP_TEXT_TEXT, buf);
								ostringstream str;
								str << "a" << Id;
								Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, str.str ().c_str ());
								Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_START, "0");
								Atom->SetProperty (GCU_PROP_POS2D, pos.c_str ());
							}
							if (!Doc)
								Doc = parent->GetDocument ()->GetApp ()->CreateNewDocument ();
						}
						catch (parse_error &error) {
							return false;
						}
						break;
					case 4: {
						bool amb;
						Residue const *res = parent->GetDocument ()->GetResidue (buf, &amb);
						if (res != NULL) {
							map< string, Object * >::iterator i;
							Molecule *mol = dynamic_cast <Molecule *> (Doc->GetFirstChild (i));
							if (mol == NULL)
								goto bad_exit;
							if (*res == *mol) {
								// Residue has been identified to the known one
								string pos = Atom->GetProperty (GCU_PROP_POS2D);
								Molecule *mol = dynamic_cast <Molecule *> (parent);
								if (mol)
									mol->Remove (Atom);
								delete Atom;
								Atom = parent->GetApplication ()->CreateObject ("fragment", parent);
								Atom->SetProperty (GCU_PROP_TEXT_TEXT, buf);
								ostringstream str;
								str << "a" << Id;
								Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, str.str ().c_str ());
								Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_START, "0");
								Atom->SetProperty (GCU_PROP_POS2D, pos.c_str ());
							} else {
								g_warning (_("Unsupported feature, please report!"));
								// FIXME: should the document care with the residues?
							}
						} else {
							g_warning (_("Unsupported feature, please report!"));
							// FIXME: Unkown residue: add it to the database? or just to the document?
						}
						break;
					}
					case 5:
						// First, parse the formula.
						{
							map< string, Object * >::iterator i;
							Molecule *mol = dynamic_cast <Molecule *> (Doc->GetFirstChild (i));
							// Do the molecule have a pseudo-atom?
							bool have_pseudo = false;
							Object *obj = mol->GetFirstChild (i);
							gcu::Atom *a = NULL;
							while (obj) {
								a = dynamic_cast <gcu::Atom *> (obj);
								if (a && ! a->GetZ ()) {
									have_pseudo = true;
									break;
								}
								obj = mol->GetNextChild (i);
							}
							if (mol == NULL)
								goto bad_exit;
							try {
								// First, parse the formula.
								Formula form (buf, GCU_FORMULA_PARSE_RESIDUE);
								// now build a molecule from the formula
								Molecule *mol2 = Molecule::MoleculeFromFormula (Doc, form, have_pseudo);
								// Now see if it matches with the molecule
								if (!mol2 || !(*mol == *mol2)) {
									if (have_pseudo) {
										// try adding a new residue
										// first examine the first atom
										map <gcu::Atom*, gcu::Bond*>::iterator i;
										gcu::Bond *b = a->GetFirstBond (i);
										int residue_offset = 0;
										if (!b)
											goto fragment_error;
										gcu::Atom *a2 = b->GetAtom (a);
										if (!a2)
											goto fragment_error;
										list<FormulaElt *> const &elts = form.GetElements ();
										list<FormulaElt *>::const_iterator j = elts.begin ();
										FormulaAtom *fatom = dynamic_cast <FormulaAtom *> (*j);
										int valence;
										if (!fatom || fatom->elt != a2->GetZ ())
											goto fragment_add;
										valence = Element::GetElement (fatom->elt)->GetDefaultValence ();
										switch (valence) {
										case 2: {
											/* remove the first atom and replace it by a pseudo-atom, then add the residue
											this helps with things begining with an oxygen or a sulfur, but might be
											not enough n other cases */
											double x, y;
											a2->GetCoords (&x, &y);
											a->SetCoords (x, y);
											a->RemoveBond (b);
											a2->RemoveBond (b);
											mol->Remove (b);
											delete b;
											if (a2->GetBondsNumber () > 1)
												goto fragment_error;
											b = a2->GetFirstBond (i);
											if (b->GetOrder () != 1)
												goto fragment_error;
											b->ReplaceAtom (a2, a);
											a->AddBond (b);
											mol->Remove (a2);
											delete a2;
											// now remove the atom from the new residue symbol
											residue_offset += fatom->end;
											break;
										}
										case 3:
											// we do not support that at the moment
											goto fragment_error;
											break;
										default:
											// we do not support that at the moment
											goto fragment_error;
										}
fragment_add:
										// Try create a new document, using the symbol as name
										parent->GetDocument ()->CreateResidue (buf + residue_offset, buf + residue_offset, mol);
										goto fragment_success;
									}
fragment_error:
									g_warning (_("failed for %s\n"),buf);
								}
							}
							catch (parse_error &error) {
								int start, length;
								puts (error.what (start, length));
							}
fragment_success:
							string pos = Atom->GetProperty (GCU_PROP_POS2D);
							mol = dynamic_cast <Molecule *> (parent);
							if (mol)
								mol->Remove (Atom);
							delete Atom;
							Atom = parent->GetApplication ()->CreateObject ("fragment", parent);
							Atom->SetProperty (GCU_PROP_TEXT_TEXT, buf);
							ostringstream str;
							str << "a" << Id;
							Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, str.str ().c_str ());
							Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_START, "0");
							Atom->SetProperty (GCU_PROP_POS2D, pos.c_str ());
						}
						break;
					case 7: {
						bool amb;
						Residue const *res = parent->GetDocument ()->GetResidue (buf, &amb);
						if (res != NULL && res->GetGeneric ()) {
							string pos = Atom->GetProperty (GCU_PROP_POS2D);
							Molecule *mol = dynamic_cast <Molecule *> (parent);
							if (mol)
								mol->Remove (Atom);
							delete Atom;
							Atom = parent->GetApplication ()->CreateObject ("fragment", parent);
							Atom->SetProperty (GCU_PROP_TEXT_TEXT, buf);
							ostringstream str;
							str << "a" << Id;
							Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, str.str ().c_str ());
							Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_START, "0");
							Atom->SetProperty (GCU_PROP_POS2D, pos.c_str ());
							parent->GetDocument ()->ObjectLoaded (static_cast <gcp::Fragment *> (Atom)->GetAtom ());
						} else {
							g_warning (_("Unsupported feature, please report!"));
							// TODO: import it in the document
						}
						break;
					}
					default:
						g_warning (_("Unsupported feature, please report!"));
						break;
					}
					break;
				}
			default:
				if (!ReadGenericObject (in))
					goto bad_exit;
			}
		} else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff)
				goto bad_exit;
			switch (code) {
			case kCDXProp_2DPosition: {
				gint32 x, y;
				if (size != 8 || !(READINT32 (in,y)) || !(READINT32 (in,x)))
					goto bad_exit;
				ostringstream str;
				str <<  x << " " << y;
				Atom->SetProperty (GCU_PROP_POS2D, str.str ().c_str ());
				break;
			}
			case kCDXProp_Node_Element: {
				if (size != 2 || !(READINT16 (in,size)))
					goto bad_exit;
				Z = size;
				ostringstream str;
				str <<  size;
				Atom->SetProperty (GCU_PROP_ATOM_Z, str.str ().c_str ());
				break;
			}
			case kCDXProp_Atom_Charge: {
				gint8 charge;
				if (size!= 1 || !gsf_input_read (in, 1, (guint8*) &charge))
					goto bad_exit;
				ostringstream str;
				str <<  charge;
				Atom->SetProperty (GCU_PROP_ATOM_CHARGE, str.str ().c_str ());
				break;
			}
			case kCDXProp_Node_Type:
				if (size != 2 || !(READINT16 (in,type)))
					goto bad_exit;
				if (type == 12) {
					// convert the atom to a pseudo atom.
					string pos = Atom->GetProperty (GCU_PROP_POS2D);
					Molecule *mol = dynamic_cast <Molecule *> (parent);
					if (mol)
						mol->Remove (Atom);
					delete Atom;
					Atom = parent->GetApplication ()->CreateObject ("pseudo-atom", parent);
					ostringstream str;
					str << "a" << Id;
					Atom->SetId (str.str ().c_str ());
					Atom->SetProperty (GCU_PROP_POS2D, pos.c_str ());
				}
				break;
			default:
				if (size && !gsf_input_read (in, size, (guint8*) buf))
					goto bad_exit;
			}
		}
		if (!(READINT16 (in,code)))
			goto bad_exit;
	}
	if (Doc)
		delete Doc;
	parent->GetDocument ()->ObjectLoaded (Atom);
	return true;
bad_exit:
	if (Doc)
		delete Doc;
	return false;
}

bool CDXLoader::ReadBond (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Bond = parent->GetApplication ()->CreateObject ("bond", parent);
	guint32 Id;
	if (!(READINT32 (in,Id)))
		return false;
	ostringstream str;
	str << "b" << Id;
	Bond->SetId (str.str ().c_str ());
	m_LoadedIds[Id] = Bond->GetId ();
	Bond->SetProperty (GCU_PROP_BOND_ORDER, "1");
	if (!(READINT16 (in,code)))
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
				if (size != 4 || !(READINT32 (in,Id)))
					return false;
				ostringstream str;
				str << Id;
				Bond->SetProperty (GCU_PROP_BOND_BEGIN, str.str ().c_str ());
				break;
			}
			case kCDXProp_Bond_End: {
				if (size != 4 || !(READINT32 (in,Id)))
					return false;
				ostringstream str;
				str << Id;
				Bond->SetProperty (GCU_PROP_BOND_END, str.str ().c_str ());
				break;
			}
			case kCDXProp_Bond_Order:
				if (size != 2 || !(READINT16 (in,size)))
					return false;
				switch (size) {
				case 2:
					Bond->SetProperty (GCU_PROP_BOND_ORDER, "2");
					break;
				case 4:
					Bond->SetProperty (GCU_PROP_BOND_ORDER, "3");
					break;
				default:
					Bond->SetProperty (GCU_PROP_BOND_ORDER, "1");
					break;
				}
				break;
			case kCDXProp_Bond_Display:
				if (size != 2 || !(READINT16 (in,size)))
					return false;
				switch (size) {
				case 1:
				case 2:
				case 3:
					Bond->SetProperty (GCU_PROP_BOND_TYPE, "hash");
					break;
				case 4:
					Bond->SetProperty (GCU_PROP_BOND_TYPE, "hash-invert");
					break;
				case 5:
					Bond->SetProperty (GCU_PROP_BOND_TYPE, "large");
					break;
				case 6:
					Bond->SetProperty (GCU_PROP_BOND_TYPE, "wedge");
					break;
				case 7:
					Bond->SetProperty (GCU_PROP_BOND_TYPE, "wedge-invert");
					break;
				case 8:
					Bond->SetProperty (GCU_PROP_BOND_TYPE, "squiggle");
					break;
				default:
					Bond->SetProperty (GCU_PROP_BOND_TYPE, "normal");
				}
				break;
			default:
				if (size && !gsf_input_read (in, size, (guint8*) buf))
					return false;
			}
		}
		if (!(READINT16 (in,code)))
			return false;
	}
	parent->GetDocument ()->ObjectLoaded (Bond);
	return true;
}

typedef struct {
	guint16 index;
	guint16 font;
	guint16 face;
	guint16 size;
	guint16 color;
} attribs;

bool CDXLoader::ReadText (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Text= parent->GetApplication ()->CreateObject ("text", parent);
	guint32 Id;
	guint8 TextAlign = 0xfe, TextJustify = 0xfe;
	if (!(READINT32 (in,Id)))
		return false;
	ostringstream str;
	str << "t" << Id;
	Text->SetId (str.str ().c_str ());
	m_LoadedIds[Id] = Text->GetId ();
	if (!(READINT16 (in,code)))
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
				gint32 x, y;
				if (!(READINT32 (in,y)))
					return false;
				if (!(READINT32 (in,x)))
					return false;
				ostringstream str;
				str <<  x << " " << y;
				Text->SetProperty (GCU_PROP_POS2D, str.str ().c_str ());
				break;
			}
			case kCDXProp_Text: {
				guint16 nb;
				bool interpret = false;
				attribs attrs, attrs0;
				attrs0.index = 0; // makes gcc happy
				attrs0.face = 0; // ditto
				if (!(READINT16 (in,nb)))
					return false;
				list <attribs> attributes;
				size -=2;
				guint16 *n = &attrs.index;
				for (int i =0; i < nb; i++) {
					if (size < 10)
						return false;
					for (int j = 0; j < 5; j++)
						if (!(READINT16 (in,n[j])))
							return false;
					attributes.push_back (attrs);
					size -= 10;
				}
				if (size < 1)
					return false;
				if (attributes.empty ()) {
					if (!gsf_input_read (in, size, (guint8*) buf))
						return false;
					buf[size] = 0;
					Text->SetProperty (GCU_PROP_TEXT_TEXT, buf);
				} else {
					ostringstream str;
					str << "<text>";
					while (!attributes.empty ()) {
						attrs = attributes.front ();
						attributes.pop_front ();
						if (attrs.index > 0) {
							attrs0.index = attrs.index - attrs0.index;
							if (!gsf_input_read (in, attrs0.index, (guint8*) buf))
								return false;
							buf[attrs0.index] = 0;
							// supposing the text is ASCII !!
							if (interpret) {
								// for now put all numbers as subscripts
								// FIXME: fix this kludgy code
								int cur = 0;
								while (cur < attrs0.index) {
									while (cur < attrs0.index && (buf[cur] < '0' || buf[cur] > '9'))
										str << buf[cur++];
									if (cur < attrs0.index) {
										if (attrs0.face & 4)
											str << "</u>";
										if (attrs0.face & 2)
											str << "</i>";
										if (attrs0.face & 1)
											str << "</b>";
										str << "</fore></font><font name=\"" << m_Fonts[attrs.font].name << ", " << (double) attrs.size / 30. << "\">";
										str << "<fore " << colors[attrs.color] << ">";
										str << "<sub height=\"" << (double) attrs.size / 60. << "\">";
										while (buf[cur] >= '0' && buf[cur] <= '9')
											str << buf[cur++];
										str << "</sub></fore></font><font name=\"" << m_Fonts[attrs.font].name << ", " << (double) attrs.size / 20. << "\">";
										str << "<fore " << colors[attrs.color] << ">";
										if (attrs0.face & 1)
											str << "<b>";
										if (attrs0.face & 2)
											str << "<i>";
										if (attrs0.face & 4)
											str << "<u>";
									}
								}
							} else
								str << buf;
							size -= attrs0.index;
							if ((attrs0.face & 0x60) == 0x60)
								interpret = false;
							else if (attrs0.face & 0x40)
								str << "</sup>";
							else if (attrs0.face & 0x20)
								str << "</sub>";
							if (attrs0.face & 4)
								str << "</u>";
							if (attrs0.face & 2)
								str << "</i>";
							if (attrs0.face & 1)
								str << "</b>";
							str << "</fore>";
							str << "</font>";
						}
						if ((attrs.face & 0x60) != 0 && (attrs.face & 0x60) != 0x60)
							attrs.size = attrs.size * 2 / 3;
						str << "<font name=\"" << m_Fonts[attrs.font].name << ", " << (double) attrs.size / 20. << "\">";
						str << "<fore " << colors[attrs.color] << ">";
						if (attrs.face & 1)
							str << "<b>";
						if (attrs.face & 2)
							str << "<i>";
						if (attrs.face & 4)
							str << "<u>";
						// skip 0x08 == outline since it is not supported
						// skip 0x10 == shadow since it is not supported
						if ((attrs.face & 0x60) == 0x60)
							interpret = true;
						else if (attrs.face & 0x20)
							str << "<sub height=\"" << (double) attrs.size / 40. << "\">";
						else if (attrs.face & 0x40)
							str << "<sup height=\"" << (double) attrs.size / 20. << "\">";
						attrs0 = attrs;
					}
					if (!gsf_input_read (in, size, (guint8*) buf))
						return false;
					buf[size] = 0;
					bool opened = true;
					// supposing the text is ASCII!!
					if (interpret) {
						// for now put all numbers as subscripts
						// FIXME: fix this kludgy code
						int cur = 0;
						while (cur < size) {
							while (cur < size && (buf[cur] < '0' || buf[cur] > '9'))
								str << buf[cur++];
							if (cur < size) {
								if (attrs0.face & 4)
									str << "</u>";
								if (attrs0.face & 2)
									str << "</i>";
								if (attrs0.face & 1)
									str << "</b>";
								str << "</fore></font><font name=\"" << m_Fonts[attrs.font].name << ", " << (double) attrs.size / 30. << "\">";
								str << "<fore " << colors[attrs.color] << ">";
								str << "<sub height=\"" << (double) attrs.size / 60. << "\">";
								while (buf[cur] >= '0' && buf[cur] <= '9')
									str << buf[cur++];
								str << "</sub></fore></font>";
								if (cur < size) {
									str << "<font name=\"" << m_Fonts[attrs.font].name << ", " << (double) attrs.size / 20. << "\">";
									str << "<fore " << colors[attrs.color] << ">";
									if (attrs0.face & 1)
										str << "<b>";
									if (attrs0.face & 4)
										str << "<u>";
									if (attrs0.face & 2)
										str << "<i>";
								} else
									opened = false;
							}
						}
					} else
						str << buf;
					if (opened) {
						if ((attrs0.face & 0x60) != 0x60) {
							if (attrs0.face & 0x40)
								str << "</sup>";
							else if (attrs0.face & 0x20)
								str << "</sub>";
						}
						if (attrs0.face & 4)
							str << "</u>";
						if (attrs0.face & 2)
							str << "</i>";
						if (attrs0.face & 1)
							str << "</b>";
						str << "</fore>";
						str << "</font>";
						}
					str << "</text>";
					Text->SetProperty (GCU_PROP_TEXT_MARKUP, str.str().c_str());
				}
				break;
			}
			case kCDXProp_Justification:
				if (!gsf_input_read (in, 1, &TextJustify))
					return false;
				break;
			case kCDXProp_CaptionJustification:
				if (!gsf_input_read (in, 1, &TextAlign))
					return false;
				break;
			default:
				if (size && !gsf_input_read (in, size, (guint8*) buf))
					return false;
			}
		}
		if (!(READINT16 (in,code)))
			return false;
	}
	if (TextAlign == 0xfe)
		TextAlign = m_TextAlign;
	switch (TextAlign) {
	case 0xff:
		Text->SetProperty (GCU_PROP_TEXT_ALIGNMENT, "right");
		break;
	case 0:
		Text->SetProperty (GCU_PROP_TEXT_ALIGNMENT, "left");
		break;
	case 1:
		Text->SetProperty (GCU_PROP_TEXT_ALIGNMENT, "center");
		break;
	default:
	// Other cases are not currently supported
		break;
	}
	if (TextJustify == 0xfe)
		TextJustify = m_TextJustify;
	switch (TextJustify) {
	case 0xff:
		Text->SetProperty (GCU_PROP_TEXT_JUSTIFICATION, "right");
		break;
	case 0:
		Text->SetProperty (GCU_PROP_TEXT_JUSTIFICATION, "left");
		break;
	case 1:
		Text->SetProperty (GCU_PROP_TEXT_JUSTIFICATION, "center");
		break;
	case 2:
		Text->SetProperty (GCU_PROP_TEXT_JUSTIFICATION, "justify");
		break;
	default:
	// Other cases are not currently supported
		break;
	}
	parent->GetDocument ()->ObjectLoaded (Text);
	return true;
}

bool CDXLoader::ReadGroup (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Group= parent->GetApplication ()->CreateObject ("group", parent);
	Group->Lock ();
	if (gsf_input_seek (in, 4, G_SEEK_CUR)) //skip the id, unless we need to store in in m_LoadedIds
		return false;
	if (!(READINT16 (in,code)))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			switch (code) {
			case kCDXObj_Fragment:
				if (!ReadMolecule (in, Group))
					return false;
				break;
			case kCDXObj_Text:
				if (!ReadText (in, Group))
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
		if (!(READINT16 (in,code)))
			return false;
	}
	Group->Lock (false);
	Group->OnLoaded ();
	parent->GetDocument ()->ObjectLoaded (Group);
	return true;
}

bool CDXLoader::ReadGraphic  (GsfInput *in, Object *parent)
{
	guint16 code;
	guint32 Id;
	guint16 type = 0xffff, arrow_type = 0xffff;
	gint32 x0, y0, x1, y1;
	if (!(READINT32 (in,Id)) || !(READINT16 (in,code)))
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
			case kCDXProp_BoundingBox:
				if (size != 16 || !(READINT32 (in,y1)) || !(READINT32 (in,x1))
					|| !(READINT32 (in,y0)) || !(READINT32 (in,x0)))
					return false;
				break;
			case kCDXProp_Graphic_Type:
				type = ReadInt (in, size);
				break;
			case kCDXProp_Arrow_Type:
				arrow_type = ReadInt (in, size);
				break;
			default:
				if (size && !gsf_input_read (in, size, (guint8*) buf))
					return false;
			}
		}
		if (!(READINT16 (in,code)))
			return false;
	}
	if (type == 1) {
		Object *obj = NULL;
		ostringstream str;
		switch (arrow_type) {
		case 1:
		case 2:
			obj = parent->GetApplication ()->CreateObject ("reaction-arrow", parent);
			str << "ra" << Id;
			break;
		case 4:
			obj = parent->GetApplication ()->CreateObject ("mesomery-arrow", parent);
			str << "ma" << Id;
			break;
		case 8:
			obj = parent->GetApplication ()->CreateObject ("reaction-arrow", parent);
			str << "ra" << Id;
			obj->SetProperty (GCU_PROP_REACTION_ARROW_TYPE, "double");
			break;
		case 32:
			obj = parent->GetApplication ()->CreateObject ("retrosynthesis-arrow", parent);
			str << "rsa" << Id;
			break;
		default:
			break;
		}
		if (obj) {
			obj->SetId (str.str ().c_str ());
			m_LoadedIds[Id] = obj->GetId ();
			ostringstream str_;
			str_ << x0 << " " << y0 << " " << x1 << " " << y1;
			obj->SetProperty (GCU_PROP_ARROW_COORDS, str_.str ().c_str ());
			parent->GetDocument ()->ObjectLoaded (obj);
		}
	}
	return true;
}

guint16 CDXLoader::ReadSize  (GsfInput *in)
{
	guint16 size;
	if (!(READINT16 (in,size)))
		return 0xffff;
	if ((unsigned) size + 1 > bufsize) {
		do
			bufsize <<= 1;
		while ((unsigned) size + 1 > bufsize);
		delete [] buf;
		buf = new char [bufsize];
	}
	return size;
}

bool CDXLoader::ReadDate  (GsfInput *in)
{
	guint16 n[7];
	for (int i = 0; i < 7; i++)
		if (!(READINT16 (in,n[i])))
			return false;
	GDate *date = g_date_new_dmy (n[2], (GDateMonth) n[1], n[0]);
	g_date_strftime (buf, bufsize, "%m/%d/%Y", date);
	g_date_free (date);
	return true;
}

bool CDXLoader::ReadFragmentText (GsfInput *in, G_GNUC_UNUSED Object *parent)
{
	guint16 code;
	if (gsf_input_seek (in, 4, G_SEEK_CUR)) //skip the id, unless we need it in m_LoadedIds
		return false;
	if (!(READINT16 (in,code)))
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
/*			case kCDXProp_2DPosition: {
				if (size != 8)
					return false;
				gint32 x, y;
				if (!READINT32 (in, y))
					return false;
				if (!READINT32 (in, x))
					return false;
				ostringstream str;
				str <<  x << " " << y;
				Text->SetProperty (GCU_PROP_POS2D, str.str ().c_str ());
				break;
			}*/
			case kCDXProp_Text: {
				guint16 nb;
				if (!(READINT16 (in,nb)))
					return false;
				size -=2;
				for (int i =0; i < nb; i++) {
					if (size < 10)
						return false;
					guint16 n[5];
					for (int j = 0; j < 5; j++)
						if (!(READINT16 (in,n[j])))
							return false;
					size -= 10;
				}
				if (size < 1)
					return false;
				if (!gsf_input_read (in, size, (guint8*) buf))
					return false;
				buf[size] = 0;
				break;
			}
			default:
				if (size && gsf_input_seek (in, size, G_SEEK_CUR))
					return false;
			}
		}
		if (!(READINT16 (in,code)))
			return false;
	}
	return true;
}

bool CDXLoader::ReadScheme (GsfInput *in, Object *parent)
{
	guint16 code;
	guint32 Id;
	m_Scheme.clear ();
	if (!(READINT32 (in,Id)))
		return false;
	ostringstream str;
	str << "r" << Id;
	gcu::Object *obj = parent->GetApplication ()->CreateObject ("reaction", parent);
	obj->SetId (str.str ().c_str ());
	m_LoadedIds[Id] = obj->GetId ();
	if (!(READINT16 (in,code)))
		return false;
	while (code) {
		if (code == kCDXObj_ReactionStep) {
			if (!ReadStep (in, obj))
				return false;
		} else 
			return false;
		if (!(READINT16 (in,code)))
			return false;
	}
	return true;
}

bool CDXLoader::ReadStep (GsfInput *in, Object *parent)
{
	unsigned i, max;
	guint16 code;
	guint32 id;
	StepData data;
	if (dynamic_cast < gcp::Document * > (parent) == NULL)
	    parent = parent->GetDocument (); // we don't support anything else for now
	// we don't need the Id, so skip it;
	if (gsf_input_seek (in, 4, G_SEEK_CUR))
		return false;
	if (!(READINT16 (in,code)))
		return false;
	while (code) {
		if (code & kCDXTag_Object)
			return false; // this should not happen
		else {
			guint16 size;
			if ((size = ReadSize (in)) == 0xffff)
				return false;
			switch (code) {
			case kCDXProp_ReactionStep_Reactants:
				if ((size % 4) != 0)
					return false;
				max = size / 4;
				for (i = 0; i < max; i++) {
					if (!(READINT32 (in, id)))
						return false;
					data.Reagents.push_back (id);
				}
				break;
			case kCDXProp_ReactionStep_Products:
				if ((size % 4) != 0)
					return false;
				max = size / 4;
				for (i = 0; i < max; i++) {
					if (!(READINT32 (in, id)))
						return false;
					data.Products.push_back (id);
				}
				break;
			case kCDXProp_ReactionStep_ObjectsAboveArrow:
				if ((size % 4) != 0)
					return false;
				max = size / 4;
				for (i = 0; i < max; i++) {
					if (!(READINT32 (in, id)))
						return false;
					data.ObjectsAbove.push_back (id);
				}
				break;
			case kCDXProp_ReactionStep_ObjectsBelowArrow:
				if ((size % 4) != 0)
					return false;
				max = size / 4;
				for (i = 0; i < max; i++) {
					if (!(READINT32 (in, id)))
						return false;
				}
					data.ObjectsBelow.push_back (id);
				break;
			case kCDXProp_ReactionStep_Arrows:
				if ((size % 4) != 0)
					return false;
				// reading only the firt arrow for now
				if (!(READINT32 (in, data.Arrow)))
						return false;
				if (size > 4 && gsf_input_seek (in, size - 4, G_SEEK_CUR))
					return false;
				break;
			default:
				if (size && gsf_input_seek (in, size, G_SEEK_CUR))
					return false;
			}
		}
		if (!(READINT16 (in,code)))
			return false;
	}
	m_Scheme.push_back (data);
	return true;
}

bool CDXLoader::ReadArrow (GsfInput *in, Object *parent)
{
	return true;
}

static CDXLoader loader;

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
	// initialize charsets names
	Charsets[kCDXCharSetUnknown] = "Unknown";
	Charsets[kCDXCharSetEBCDICOEM] = "EBCDICOEM";
	Charsets[kCDXCharSetMSDOSUS] = "MSDOSUS";
	Charsets[kCDXCharSetEBCDIC500V1] = "EBCDIC500V1";
	Charsets[kCDXCharSetArabicASMO708] = "ASMO-708";
	Charsets[kCDXCharSetArabicASMO449P] = "ArabicASMO449P";
	Charsets[kCDXCharSetArabicTransparent] = "ArabicTransparent";
	Charsets[kCDXCharSetArabicTransparentASMO] = "DOS-720";
	Charsets[kCDXCharSetGreek437G] = "Greek437G";
	Charsets[kCDXCharSetBalticOEM] = "cp775";
	Charsets[kCDXCharSetMSDOSLatin1] = "windows-850";
	Charsets[kCDXCharSetMSDOSLatin2] = "ibm852";
	Charsets[kCDXCharSetIBMCyrillic] = "cp855";
	Charsets[kCDXCharSetIBMTurkish] = "cp857";
	Charsets[kCDXCharSetMSDOSPortuguese] = "cp860";
	Charsets[kCDXCharSetMSDOSIcelandic] = "cp861";
	Charsets[kCDXCharSetHebrewOEM] = "DOS-862";
	Charsets[kCDXCharSetMSDOSCanadianFrench] = "cp863";
	Charsets[kCDXCharSetArabicOEM] = "cp864";
	Charsets[kCDXCharSetMSDOSNordic] = "cp865";
	Charsets[kCDXCharSetMSDOSRussian] = "cp866";
	Charsets[kCDXCharSetIBMModernGreek] = "cp869";
	Charsets[kCDXCharSetThai] = "windows-874";
	Charsets[kCDXCharSetEBCDIC] = "EBCDIC";
	Charsets[kCDXCharSetJapanese] = "shift_jis";
	Charsets[kCDXCharSetChineseSimplified] = "gb2312";
	Charsets[kCDXCharSetKorean] = "ks_c_5601-1987";
	Charsets[kCDXCharSetChineseTraditional] = "big5";
	Charsets[kCDXCharSetUnicodeISO10646] = "iso-10646";
	Charsets[kCDXCharSetWin31EasternEuropean] = "windows-1250";
	Charsets[kCDXCharSetWin31Cyrillic] = "windows-1251";
	Charsets[kCDXCharSetWin31Latin1] = "iso-8859-1";
	Charsets[kCDXCharSetWin31Greek] = "iso-8859-7";
	Charsets[kCDXCharSetWin31Turkish] = "iso-8859-9";
	Charsets[kCDXCharSetHebrew] = "windows-1255";
	Charsets[kCDXCharSetArabic] = "windows-1256";
	Charsets[kCDXCharSetBaltic] = "windows-1257";
	Charsets[kCDXCharSetVietnamese] = "windows-1258";
	Charsets[kCDXCharSetKoreanJohab] = "windows-1361";
	Charsets[kCDXCharSetMacRoman] = "x-mac-roman";
	Charsets[kCDXCharSetMacJapanese] = "x-mac-japanese";
	Charsets[kCDXCharSetMacTradChinese] = "x-mac-tradchinese";
	Charsets[kCDXCharSetMacKorean] = "x-mac-korean";
	Charsets[kCDXCharSetMacArabic] = "x-mac-arabic";
	Charsets[kCDXCharSetMacHebrew] = "x-mac-hebrew";
	Charsets[kCDXCharSetMacGreek] = "x-mac-greek";
	Charsets[kCDXCharSetMacCyrillic] = "x-mac-cyrillic";
	Charsets[kCDXCharSetMacReserved] = "x-mac-reserved";
	Charsets[kCDXCharSetMacDevanagari] = "x-mac-devanagari";
	Charsets[kCDXCharSetMacGurmukhi] = "x-mac-gurmukhi";
	Charsets[kCDXCharSetMacGujarati] = "x-mac-gujarati";
	Charsets[kCDXCharSetMacOriya] = "x-mac-oriya";
	Charsets[kCDXCharSetMacBengali] = "x-mac-nengali";
	Charsets[kCDXCharSetMacTamil] = "x-mac-tamil";
	Charsets[kCDXCharSetMacTelugu] = "x-mac-telugu";
	Charsets[kCDXCharSetMacKannada] = "x-mac-kannada";
	Charsets[kCDXCharSetMacMalayalam] = "x-mac-Malayalam";
	Charsets[kCDXCharSetMacSinhalese] = "x-mac-sinhalese";
	Charsets[kCDXCharSetMacBurmese] = "x-mac-burmese";
	Charsets[kCDXCharSetMacKhmer] = "x-mac-khmer";
	Charsets[kCDXCharSetMacThai] = "x-mac-thai";
	Charsets[kCDXCharSetMacLao] = "x-mac-lao";
	Charsets[kCDXCharSetMacGeorgian] = "x-mac-georgian";
	Charsets[kCDXCharSetMacArmenian] = "x-mac-armenian";
	Charsets[kCDXCharSetMacSimpChinese] = "x-mac-simpChinese";
	Charsets[kCDXCharSetMacTibetan] = "x-mac-tibetan";
	Charsets[kCDXCharSetMacMongolian] = "x-mac-mongolian";
	Charsets[kCDXCharSetMacEthiopic] = "x-mac-ethiopic";
	Charsets[kCDXCharSetMacCentralEuroRoman] = "x-mac-ce";
	Charsets[kCDXCharSetMacVietnamese] = "x-mac-vietnamese";
	Charsets[kCDXCharSetMacExtArabic] = "x-mac-extArabic";
	Charsets[kCDXCharSetMacUninterpreted] = "x-mac-uninterpreted";
	Charsets[kCDXCharSetMacIcelandic] = "x-mac-icelandic";
	Charsets[kCDXCharSetMacTurkish] = "x-mac-turkish";
	CharsetIDs["Unknown"] = kCDXCharSetUnknown;
	CharsetIDs["EBCDICOEM"] = kCDXCharSetEBCDICOEM;
	CharsetIDs["MSDOSUS"] = kCDXCharSetMSDOSUS;
	CharsetIDs["EBCDIC500V1"] = kCDXCharSetEBCDIC500V1;
	CharsetIDs["ASMO-708"] = kCDXCharSetArabicASMO708;
	CharsetIDs["ArabicASMO449P"] = kCDXCharSetArabicASMO449P;
	CharsetIDs["ArabicTransparent"] = kCDXCharSetArabicTransparent;
	CharsetIDs["DOS-720"] = kCDXCharSetArabicTransparentASMO;
	CharsetIDs["Greek437G"] = kCDXCharSetGreek437G;
	CharsetIDs["cp775"] = kCDXCharSetBalticOEM;
	CharsetIDs["windows-850"] = kCDXCharSetMSDOSLatin1;
	CharsetIDs["ibm852"] = kCDXCharSetMSDOSLatin2;
	CharsetIDs["cp855"] = kCDXCharSetIBMCyrillic;
	CharsetIDs["cp857"] = kCDXCharSetIBMTurkish;
	CharsetIDs["cp860"] = kCDXCharSetMSDOSPortuguese;
	CharsetIDs["cp861"] = kCDXCharSetMSDOSIcelandic;
	CharsetIDs["DOS-862"] = kCDXCharSetHebrewOEM;
	CharsetIDs["cp863"] = kCDXCharSetMSDOSCanadianFrench;
	CharsetIDs["cp864"] = kCDXCharSetArabicOEM;
	CharsetIDs["cp865"] = kCDXCharSetMSDOSNordic;
	CharsetIDs["cp866"] = kCDXCharSetMSDOSRussian;
	CharsetIDs["cp869"] = kCDXCharSetIBMModernGreek;
	CharsetIDs["windows-874"] = kCDXCharSetThai;
	CharsetIDs["EBCDIC"] = kCDXCharSetEBCDIC;
	CharsetIDs["shift_jis"] = kCDXCharSetJapanese;
	CharsetIDs["gb2312"] = kCDXCharSetChineseSimplified;
	CharsetIDs["ks_c_5601-1987"] = kCDXCharSetKorean;
	CharsetIDs["big5"] = kCDXCharSetChineseTraditional;
	CharsetIDs["iso-10646"] = kCDXCharSetUnicodeISO10646;
	CharsetIDs["windows-1250"] = kCDXCharSetWin31EasternEuropean;
	CharsetIDs["windows-1251"] = kCDXCharSetWin31Cyrillic;
	CharsetIDs["iso-8859-1"] = kCDXCharSetWin31Latin1;
	CharsetIDs["iso-8859-7"] = kCDXCharSetWin31Greek;
	CharsetIDs["iso-8859-9"] = kCDXCharSetWin31Turkish;
	CharsetIDs["windows-1255"] = kCDXCharSetHebrew;
	CharsetIDs["windows-1256"] = kCDXCharSetArabic;
	CharsetIDs["windows-1257"] = kCDXCharSetBaltic;
	CharsetIDs["windows-1258"] = kCDXCharSetVietnamese;
	CharsetIDs["windows-1361"] = kCDXCharSetKoreanJohab;
	CharsetIDs["x-mac-roman"] = kCDXCharSetMacRoman;
	CharsetIDs["x-mac-japanese"] = kCDXCharSetMacJapanese;
	CharsetIDs["x-mac-tradchinese"] = kCDXCharSetMacTradChinese;
	CharsetIDs["x-mac-korean"] = kCDXCharSetMacKorean;
	CharsetIDs["x-mac-arabic"] = kCDXCharSetMacArabic;
	CharsetIDs["x-mac-hebrew"] = kCDXCharSetMacHebrew;
	CharsetIDs["x-mac-greek"] = kCDXCharSetMacGreek;
	CharsetIDs["x-mac-cyrillic"] = kCDXCharSetMacCyrillic;
	CharsetIDs["x-mac-reserved"] = kCDXCharSetMacReserved;
	CharsetIDs["x-mac-devanagari"] = kCDXCharSetMacDevanagari;
	CharsetIDs["x-mac-gurmukhi"] = kCDXCharSetMacGurmukhi;
	CharsetIDs["x-mac-gujarati"] = kCDXCharSetMacGujarati;
	CharsetIDs["x-mac-oriya"] = kCDXCharSetMacOriya;
	CharsetIDs["x-mac-nengali"] = kCDXCharSetMacBengali;
	CharsetIDs["x-mac-tamil"] = kCDXCharSetMacTamil;
	CharsetIDs["x-mac-telugu"] = kCDXCharSetMacTelugu;
	CharsetIDs["x-mac-kannada"] = kCDXCharSetMacKannada;
	CharsetIDs["x-mac-Malayalam"] = kCDXCharSetMacMalayalam;
	CharsetIDs["x-mac-sinhalese"] = kCDXCharSetMacSinhalese;
	CharsetIDs["x-mac-burmese"] = kCDXCharSetMacBurmese;
	CharsetIDs["x-mac-khmer"] = kCDXCharSetMacKhmer;
	CharsetIDs["x-mac-thai"] = kCDXCharSetMacThai;
	CharsetIDs["x-mac-lao"] = kCDXCharSetMacLao;
	CharsetIDs["x-mac-georgian"] = kCDXCharSetMacGeorgian;
	CharsetIDs["x-mac-armenian"] = kCDXCharSetMacArmenian;
	CharsetIDs["x-mac-simpChinese"] = kCDXCharSetMacSimpChinese;
	CharsetIDs["x-mac-tibetan"] = kCDXCharSetMacTibetan;
	CharsetIDs["x-mac-mongolian"] = kCDXCharSetMacMongolian;
	CharsetIDs["x-mac-ethiopic"] = kCDXCharSetMacEthiopic;
	CharsetIDs["x-mac-ce"] = kCDXCharSetMacCentralEuroRoman;
	CharsetIDs["x-mac-vietnamese"] = kCDXCharSetMacVietnamese;
	CharsetIDs["x-mac-extArabic"] = kCDXCharSetMacExtArabic;
	CharsetIDs["x-mac-uninterpreted"] = kCDXCharSetMacUninterpreted;
	CharsetIDs["x-mac-icelandic"] = kCDXCharSetMacIcelandic;
	CharsetIDs["x-mac-turkish"] = kCDXCharSetMacTurkish;
}

G_MODULE_EXPORT void
go_plugin_shutdown (G_GNUC_UNUSED GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
}

}
