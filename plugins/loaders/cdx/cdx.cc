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
#include <gcu/xml-utils.h>
#include <gcp/arrow.h>
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/fragment.h>
#include <gcp/reaction-step.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>

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
	std::list < unsigned > Arrows, Reagents, Products, ObjectsAbove, ObjectsBelow;
} StepData;

typedef struct {
	unsigned Id;
	std::list < StepData > Steps;
} SchemeData;

typedef struct {
	guint8 *buf;
	guint size;
	guint capacity;
} ByteBuf;

typedef struct {
	GsfOutput *out;
	GOIOContext *s;
	ByteBuf *buf;
	bool italic;
	bool bold;
	bool underline;
	guint16 font;
	guint16 size;
	int position;
	guint16 color;
	guint16 index;
} WriteTextState;

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
	guint16 ReadSize (GsfInput *in);
	bool ReadDate (GsfInput *in);
	void BuildScheme (gcu::Document *doc, SchemeData &scheme);

	bool WriteObject (GsfOutput *out, Object const *object, GOIOContext *io);
	static void AddInt8Property (GsfOutput *out, gint16 prop, gint8 value);
	static void AddInt16Property (GsfOutput *out, gint16 prop, gint16 value);
	static void AddInt32Property (GsfOutput *out, gint16 prop, gint32 value);
	static void AddBoundingBox (GsfOutput *out, gint32 x0, gint32 y0, gint32 x1, gint32 y1);
	static void WriteSimpleStringProperty (GsfOutput *out, gint16 id, gint16 length, char const *data);
	static void WriteDateProperty (GsfOutput *out, gint16 id, GDateTime *date);
	static bool WriteArrow (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteAtom (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteBond (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteFragment (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteMesomery(CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteMolecule (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteReaction(CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteReactionStep(CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteRetrosynthesis(CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	static bool WriteText(CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s);
	bool WriteNode (xmlNodePtr node, WriteTextState *state);
	bool WriteScheme (GsfOutput *out, Object const *obj, std::string const &arrow_type, GOIOContext *s);
	void WriteId (Object const *obj, GsfOutput *out);

private:
	char *buf;
	size_t bufsize;
	map<unsigned, CDXFont> m_Fonts;
	vector <string> colors;
	guint8 m_TextAlign, m_TextJustify;
	unsigned m_Charset;
	double padding; // needed to detect stoichiometry coefficients

	map <string, bool (*) (CDXLoader *, GsfOutput *, Object const *, GOIOContext *)> m_WriteCallbacks;
	map<unsigned, GOColor> m_Colors;
	map <string, guint32> m_SavedIds;
	std::map <guint32, std::string> m_LoadedIds;
	std::map <guint32, guint32> m_Superseded;
	SchemeData m_Scheme;
	std::list < SchemeData > m_Schemes;
	gint32 m_MaxId;
	unsigned m_Z;
	int m_CHeight, m_FontSize;
	gint16 m_LabelFont, m_LabelFontSize, m_LabelFontFace, m_LabelFontColor;
	double m_Scale, m_Zoom;
	bool m_WriteScheme;
};

CDXLoader::CDXLoader ():
	m_TextAlign (0),
	m_TextJustify (0)
{
	AddMimeType ("chemical/x-cdx");
	// Add write callbacks
	m_WriteCallbacks["atom"] = WriteAtom;
	m_WriteCallbacks["fragment"] = WriteFragment;
	m_WriteCallbacks["bond"] = WriteBond;
	m_WriteCallbacks["molecule"] = WriteMolecule;
	m_WriteCallbacks["reaction"] = WriteReaction;
	m_WriteCallbacks["reaction-arrow"] = WriteArrow;
	m_WriteCallbacks["mesomery"] = WriteMesomery;
	m_WriteCallbacks["mesomery-arrow"] = WriteArrow;
	m_WriteCallbacks["retrosynthesis"] = WriteRetrosynthesis;
	m_WriteCallbacks["retrosynthesis-arrow"] = WriteArrow;
	m_WriteCallbacks["text"] = WriteText;
	m_WriteScheme = true;
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
	doc->SetProperty (GCU_PROP_DOC_CREATOR, ""); // Chemdraw does not use it for now.
	themedesc << "<?xml version=\"1.0\"?>" << std::endl << "<theme name=\"ChemDraw\"";
	// note that we read 28 bytes here while headers for recent cdx files have only 22 bytes, remaining are 0x8000 (document) and its id (0)
	if (!gsf_input_read (in, kCDX_HeaderLength, (guint8*) buf) || strncmp (buf, kCDX_HeaderString, kCDX_HeaderStringLen)) {
		result = ContentTypeUnknown;
		code = 0;
	} else if (!(READINT16 (in, code))) {
		result = ContentTypeUnknown;
		code = 0;
	}

	// set the scale
	doc->SetProperty (GCU_PROP_THEME_SCALE, "16384");
	m_CHeight = 0.;

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
						theme = LocalTheme; // don't point to an invalid object
					} else {
						gcp::TheThemeManager.AddFileTheme (theme, doc->GetTitle ().c_str ());
						cpDoc->SetTheme (theme);
					}
					m_CHeight = cpDoc->GetView ()->GetCHeight () * 16384 * 3;
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
				padding = x * 2.; // the 2. factor is arbitrary
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
					themedesc << " text-font-style=\"normal\" text-font-weight=\"normal\"";
					break;
				case 1:
					themedesc << " text-font-style=\"normal\" text-font-weight=\"bold\"";
					break;
				case 2:
					themedesc << " text-font-style=\"italic\" text-font-weight=\"normal\"";
					break;
				case 3:
					themedesc << " text-font-style=\"italic\" text-font-weight=\"bold\"";
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
				int i = 0;
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
				guint16 i;
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				switch (i & 3) { // we do not support anything else
				default:
				case 0:
					themedesc << " text-font-style=\"normal\" text-font-weight=\"normal\"";
					break;
				case 1:
					themedesc << " text-font-style=\"normal\" text-font-weight=\"bold\"";
					break;
				case 2:
					themedesc << " text-font-style=\"italic\" text-font-weight=\"normal\"";
					break;
				case 3:
					themedesc << " text-font-style=\"italic\" text-font-weight=\"bold\"";
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
					themedesc << " font-style=\"normal\" font-weight=\"normal\"";
					break;
				case 1:
					themedesc << " font-style=\"normal\" font-weight=\"bold\"";
					break;
				case 2:
					themedesc << " font-style=\"italic\" font-weight=\"normal\"";
					break;
				case 3:
					themedesc << " font-style=\"italic\" font-weight=\"bold\"";
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
				int i = 0;
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
				guint16 i;
				if (!(READINT16 (in, i))) {
					result = ContentTypeUnknown;
					break;
				}
				switch (i & 3) { // we do not support anything else
				default:
				case 0:
					themedesc << " font-style=\"normal\" font-weight=\"normal\"";
					break;
				case 1:
					themedesc << " font-style=\"normal\" font-weight=\"bold\"";
					break;
				case 2:
					themedesc << " font-style=\"italic\" font-weight=\"normal\"";
					break;
				case 3:
					themedesc << " font-style=\"italic\" font-weight=\"bold\"";
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
				if (size != 1) {
					result = ContentTypeUnknown;
					break;
				}
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

	// Update the view so that the positions are correct
	static_cast <gcp::Document *> (doc)->GetView ()->Update (doc);

	// time to build reaction schemes and the like
	std::list < SchemeData >::iterator i, iend = m_Schemes.end ();
	for (i = m_Schemes.begin (); i != iend; i++) {
		if ((*i).Steps.empty ())
			continue;
		BuildScheme (doc, *i);
	}
	delete [] buf;
	m_Fonts.clear ();
	m_LoadedIds.clear ();
	m_Superseded.clear ();
	m_Colors.clear ();
	m_Schemes.clear ();
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
	prop = obj->GetProperty (GCU_PROP_TEXT_TEXT);
	if (prop.length () > 0) {
		n = kCDXObj_Text;
		WRITEINT16 (out, n);
		loader->WriteId (NULL, out);
		string prop2 = obj->GetProperty (GCU_PROP_TEXT_POSITION);
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
		n = kCDXProp_Text;
		WRITEINT16 (out, n);
		n = 12 + prop.length ();
		WRITEINT16 (out, n);
		gsf_output_write (out, 4, reinterpret_cast <guint8 const *> ("\x01\x00\x00\x00")); // runs count and first index
		WRITEINT16 (out, loader->m_LabelFont);
		WRITEINT16 (out, loader->m_LabelFontFace);
		WRITEINT16 (out, loader->m_LabelFontSize);
		WRITEINT16 (out, loader->m_LabelFontColor);
		gsf_output_write (out, prop.length (), reinterpret_cast <guint8 const *> (prop.c_str ()));
		gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of text
	}
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of atom
	return true;
}

bool CDXLoader::WriteFragment (CDXLoader *loader, GsfOutput *out, Object const *obj, G_GNUC_UNUSED GOIOContext *s)
{
	gint16 n = kCDXObj_Node;
	double x, y;
	gint32 x_, y_;
	WRITEINT16 (out, n);
	std::string prop = obj->GetProperty (GCU_PROP_FRAGMENT_ATOM_ID);
	gcu::Object *atom = obj->GetChild (prop.c_str ());
	loader->m_SavedIds[atom->GetId ()] = loader->m_MaxId;
	loader->WriteId (obj, out);
	prop = obj->GetProperty (GCU_PROP_POS2D);
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
	AddInt16Property (out, kCDXProp_Node_Type, 5);
	prop = obj->GetProperty (GCU_PROP_TEXT_TEXT);
	std::string pos = obj->GetProperty (GCU_PROP_FRAGMENT_ATOM_START);
	unsigned as = atoi (pos.c_str ());
	if (as > 0) {
		char const *symbol = static_cast < gcu::Atom * > (atom)->GetSymbol ();
		unsigned ae = as + strlen (symbol);
		if (ae < prop.length () - 1) {
			// attachment point is in the middle of the string, we need to bring it to start,
			// and add put the left part inside brackets
			std::string left = prop.substr (0, as), right = prop.substr (ae);
			prop = symbol;
			prop += "(";
			gcu::Formula *formula = new gcu::Formula (left, GCU_FORMULA_PARSE_RESIDUE);
			std::list < FormulaElt * > const &elts = formula->GetElements ();
			std::list< FormulaElt * >::const_reverse_iterator i, end = elts.rend ();
			for (i = elts.rbegin (); i!= end; i++)
				prop += (*i)->Text ();
			prop += ")";
			prop += right;
			delete formula;
		} else {
			// atom is at end, we need to revert the formula
			gcu::Formula *formula = new gcu::Formula (prop, GCU_FORMULA_PARSE_RESIDUE);
			std::list < FormulaElt * > const &elts = formula->GetElements ();
			prop.clear ();
			std::list< FormulaElt * >::const_reverse_iterator i, end = elts.rend ();
			for (i = elts.rbegin (); i!= end; i++)
				prop += (*i)->Text ();
			delete formula;
		}
	}
	if (prop.length () > 0) {
		n = kCDXObj_Text;
		WRITEINT16 (out, n);
		loader->WriteId (NULL, out);
		string prop2 = obj->GetProperty (GCU_PROP_TEXT_POSITION);
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
		n = kCDXProp_Text;
		WRITEINT16 (out, n);
		n = 12 + prop.length ();
		WRITEINT16 (out, n);
		gsf_output_write (out, 4, reinterpret_cast <guint8 const *> ("\x01\x00\x00\x00")); // runs count and first index
		WRITEINT16 (out, loader->m_LabelFont);
		WRITEINT16 (out, loader->m_LabelFontFace);
		WRITEINT16 (out, loader->m_LabelFontSize);
		WRITEINT16 (out, loader->m_LabelFontColor);
		gsf_output_write (out, prop.length (), reinterpret_cast <guint8 const *> (prop.c_str ()));
		gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of text
	}
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of atom
	return true;
}

bool CDXLoader::WriteBond (CDXLoader *loader, GsfOutput *out, Object const *obj, G_GNUC_UNUSED GOIOContext *s)
{
	gint16 n = kCDXObj_Bond;
	WRITEINT16 (out, n);
	// the id might already exist because of forward references for crossing bonds
	std::string ids = obj->GetId ();
	std::map < std::string, unsigned >::iterator it = loader->m_SavedIds.find (ids);
	if (it == loader->m_SavedIds.end ())
		loader->WriteId (obj, out);
	else
		WRITEINT32 (out, (*it).second);
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
	prop = obj->GetProperty (GCU_PROP_BOND_DOUBLE_POSITION);
	if (prop == "center")
		AddInt16Property (out, kCDXProp_Bond_DoublePosition, 256);
	else if (prop == "right")
		AddInt16Property (out, kCDXProp_Bond_DoublePosition, 257);
	else if (prop == "left")
		AddInt16Property (out, kCDXProp_Bond_DoublePosition, 258);
	prop = obj->GetProperty (GCU_PROP_BOND_CROSSING);
	if (prop.length () > 0) {
		std::istringstream is (prop);
		std::set < unsigned > crossing;
		std::set < unsigned >::iterator j, jend;
		guint32 id;
		while (!is.eof ()) {
			is >> ids;
			it = loader->m_SavedIds.find (ids);
			if (it == loader->m_SavedIds.end ()) {
				id = loader->m_MaxId++;
				loader->m_SavedIds[ids] = id;
			} else
				id = loader->m_SavedIds[ids];
			crossing.insert (id);
		}
		n = kCDXProp_Bond_CrossingBonds;
		WRITEINT16 (out, n);
		n = crossing.size () * 4;
		WRITEINT16 (out, n);
		jend = crossing.end ();
		for (j = crossing.begin (); j != jend; j++) {
			id = *j;
			WRITEINT32 (out, id);
		}
	}
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
	std::map < int, std::set < gcu::Object const *> > crossing;
	std::set < gcu::Object const *>::const_iterator j, jend;
	int level, minl = G_MAXINT, maxl = G_MININT;
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
	// save non crossing bonds and sort others by level
	child = obj->GetFirstChild (i);
	while (child) {
		if (child->GetType () == BondType) {
			std::string prop (child->GetProperty (GCU_PROP_BOND_CROSSING));
			if (prop.length () > 0) {
				prop = child->GetProperty (GCU_PROP_BOND_LEVEL);
				level = atoi (prop.c_str ());
				if (level < minl)
					minl = level;
				if (level > maxl)
					maxl = level;
				crossing[level].insert (child);
			} else if (!loader->WriteObject (out, child, s))
				return false;
		}
		child = obj->GetNextChild (i);
	}
	// now save crossing bond starting with deeper ones.
	for (level = minl; level <= maxl; level++) {
		jend = crossing[level].end ();
		for (j = crossing[level].begin (); j != jend; j++)
			if (!loader->WriteObject (out, *j, s))
				return false;
	}
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of molecule
	return true;
}

bool CDXLoader::WriteArrow (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	while (child) {
		if (!loader->WriteObject (out, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	gint16 n = kCDXObj_Graphic;
	WRITEINT16 (out, n);
	loader->WriteId (obj, out);
	std::istringstream str (obj->GetProperty (GCU_PROP_ARROW_COORDS));
	double x0, y0, x1, y1;
	str >> x0 >> y0 >> x1 >> y1;
	loader->AddBoundingBox (out, x0, y0, x1, y1);
	AddInt16Property (out, kCDXProp_ZOrder, loader->m_Z++);
	loader->AddInt16Property (out, kCDXProp_Graphic_Type, 1);
	std::string type = gcu::Object::GetTypeName (obj->GetType ());
	if (type == "reaction-arrow")
		loader->AddInt16Property (out, kCDXProp_Arrow_Type, obj->GetProperty (GCU_PROP_REACTION_ARROW_TYPE) == "double"? 8: 2);
    else if (type == "mesomery-arrow")
		loader->AddInt16Property (out, kCDXProp_Arrow_Type, 4);
	else if (type == "retrosynthesis-arrow")
		loader->AddInt16Property (out, kCDXProp_Arrow_Type, 32);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of plus
	return true;
}

bool CDXLoader::WriteScheme (GsfOutput *out, Object const *obj, std::string const &arrow_type, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	std::list < gcu::Object const * > arrows;
	std::list < gcu::Object const * >::const_iterator it, itend;
	while (child) {
		std::string name = Object::GetTypeName (child->GetType ());
		if (name == arrow_type)
			arrows.push_back (child);
		else if (!WriteObject (out, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	itend = arrows.end ();
	for (it = arrows.begin (); it != itend; it++)
		// save the graphic object.
		if (!WriteArrow (this, out, *it, s))
			return false;
	// Now, save the reaction
	if (!m_WriteScheme)
		return true;
	gint16 n = kCDXObj_ReactionScheme;
	guint32 id;
	WRITEINT16 (out, n);
	WriteId (obj, out);
	for (it = arrows.begin (); it != itend; it++) {
		// save the associated step
		n = kCDXObj_ReactionStep;
		WRITEINT16 (out, n);
		std::list < guint32 > Ids, Ids_;
		WriteId (NULL, out);
		// reactants
		gcu::Object const *arrow = *it;
		gcu::Object const *cur = obj->GetDescendant (arrow->GetProperty (GCU_PROP_ARROW_START_ID).c_str ());
		if (cur) {
			child = cur->GetFirstChild (i);
			if (child) {
				n = kCDXProp_ReactionStep_Reactants;
				WRITEINT16 (out, n);
				n = 4;
				WRITEINT16 (out, n);
				id = m_SavedIds[child->GetId ()];
				WRITEINT32 (out, id);
			}
		}
		// products
		cur = obj->GetDescendant (arrow->GetProperty (GCU_PROP_ARROW_END_ID).c_str ());
		if (cur) {
			child = cur->GetFirstChild (i);
			if (child) {
				n = kCDXProp_ReactionStep_Products;
				WRITEINT16 (out, n);
				n = 4;
				WRITEINT16 (out, n);
				id = m_SavedIds[child->GetId ()];
				WRITEINT32 (out, id);
			}
		}
		// arrow
		n = kCDXProp_ReactionStep_Arrows;
		WRITEINT16 (out, n);
		gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x04\x00"));
		id = m_SavedIds[arrow->GetId ()];
		WRITEINT32 (out, id);
		gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of step
	}
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of scheme
	return true;
}

bool CDXLoader::WriteMesomery(CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s)
{
	return loader->WriteScheme (out, obj, "mesomery-arrow", s);
}

bool CDXLoader::WriteReactionStep(CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	while (child) {
		std::string name = Object::GetTypeName (child->GetType ());
		if (name == "reaction-operator") {
			gint16 n = kCDXObj_Graphic;
			WRITEINT16 (out, n);
			loader->WriteId (obj, out);
			// Write the bounding box
			std::istringstream str(child->GetProperty (GCU_PROP_POS2D));
			double x, y;
			str >> x >> y;
			x -= loader->m_FontSize / 3;
			y += loader->m_CHeight + loader->m_FontSize / 2;
			AddBoundingBox (out, x, y, x, y - loader->m_FontSize);
			AddInt16Property (out, kCDXProp_ZOrder, loader->m_Z++);
			loader->AddInt16Property (out, kCDXProp_Graphic_Type, 7);
			loader->AddInt16Property (out, kCDXProp_Symbol_Type, 8);
			gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of plus
		} else {
			// search for a mesomery inside the reaction
			std::string id = child->GetProperty (GCU_PROP_MOLECULE);
			gcu::Object *mol = child->GetChild (id.c_str ());
			if (gcu::Object::GetTypeName (mol->GetType ()) == "mesomery")
				loader->m_WriteScheme = false;
			if (!loader->WriteObject (out, child, s))
				return false;
		}
		child = obj->GetNextChild (i);
	}
	return true;
}

bool CDXLoader::WriteReaction (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	std::list < gcu::Object const * > arrows;
	std::list < gcu::Object const * >::const_iterator it, itend;
	while (child) {
		std::string name = Object::GetTypeName (child->GetType ());
		if (name == "reaction-step") {
			if (!loader->WriteReactionStep (loader, out, child, s))
				return false;
		} else if (name == "reaction-arrow")
			arrows.push_back (child);
		child = obj->GetNextChild (i);
	}
	itend = arrows.end ();
	for (it = arrows.begin (); it != itend; it++)
		// save the graphic object.
		if (!WriteArrow (loader, out, *it, s))
			return false;
	// Now, save the reaction
	if (loader->m_WriteScheme) {
		gint16 n = kCDXObj_ReactionScheme;
		guint32 id;
		WRITEINT16 (out, n);
		loader->WriteId (obj, out);
		for (it = arrows.begin (); it != itend; it++) {
			// save the associated step
			n = kCDXObj_ReactionStep;
			WRITEINT16 (out, n);
			std::list < guint32 > Ids, Ids_;
			loader->WriteId (NULL, out);
			// reactants
			gcp::Arrow const *arrow = static_cast < gcp::Arrow const * > (*it);
			gcu::Object const *cur = obj->GetDescendant (arrow->GetProperty (GCU_PROP_ARROW_START_ID).c_str ());
			if (cur) {
				child = cur->GetFirstChild (i);
				while (child) {
					if (child->GetType () == gcu::ReactantType)
						Ids.push_back (loader->m_SavedIds[child->GetProperty (GCU_PROP_MOLECULE)]);
					child = cur->GetNextChild (i);
				}
				if (!Ids.empty ()) {
					n = kCDXProp_ReactionStep_Reactants;
					WRITEINT16 (out, n);
					n = 4 * Ids.size ();
					WRITEINT16 (out, n);
					while (!Ids.empty ()) {
						id = Ids.front ();
						WRITEINT32 (out, id);
						Ids.pop_front ();
					}
				}
			}
			// products
			cur = obj->GetDescendant (arrow->GetProperty (GCU_PROP_ARROW_END_ID).c_str ());
			if (cur) {
				child = cur->GetFirstChild (i);
				while (child) {
					if (child->GetType () == gcu::ReactantType)
						Ids.push_back (loader->m_SavedIds[child->GetProperty (GCU_PROP_MOLECULE)]);
					child = cur->GetNextChild (i);
				}
				if (!Ids.empty ()) {
					n = kCDXProp_ReactionStep_Products;
					WRITEINT16 (out, n);
					n = 4 * Ids.size ();
					WRITEINT16 (out, n);
					while (!Ids.empty ()) {
						id = Ids.front ();
						WRITEINT32 (out, id);
						Ids.pop_front ();
					}
				}
			}
			// arrow
			n = kCDXProp_ReactionStep_Arrows;
			WRITEINT16 (out, n);
			gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x04\x00"));
			id = loader->m_SavedIds[arrow->GetId ()];
			WRITEINT32 (out, id);
			// attached objects
			// quick implementation, if y is lower than arrow y, then above, otherwise under
			child = arrow->GetFirstChild (i);
			double y = arrow->GetYAlign ();
			while (child) {
				if (y > child->GetYAlign ())
					Ids_.push_back (loader->m_SavedIds[child->GetProperty (GCU_PROP_ARROW_OBJECT)]);
				else
					Ids.push_back (loader->m_SavedIds[child->GetProperty (GCU_PROP_ARROW_OBJECT)]);
				child = arrow->GetNextChild (i);
			}
			// objects above the arrow
			if (!Ids.empty ()) {
				n = kCDXProp_ReactionStep_ObjectsAboveArrow;
				WRITEINT16 (out, n);
				n = 4 * Ids.size ();
				WRITEINT16 (out, n);
				while (!Ids.empty ()) {
					id = Ids.front ();
					WRITEINT32 (out, id);
					Ids.pop_front ();
				}
			}
			// objects under the arrow
			if (!Ids_.empty ()) {
				n = kCDXProp_ReactionStep_ObjectsBelowArrow;
				WRITEINT16 (out, n);
				n = 4 * Ids_.size ();
				WRITEINT16 (out, n);
				while (!Ids_.empty ()) {
					id = Ids_.front ();
					WRITEINT32 (out, id);
					Ids_.pop_front ();
				}
			}
			gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of step
		}
		gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of scheme
	}
	loader->m_WriteScheme = true;
	return true;
}

bool CDXLoader::WriteRetrosynthesis (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s)
{
	return loader->WriteScheme (out, obj, "retrosynthesis-arrow", s);
}

bool CDXLoader::WriteNode (xmlNodePtr node, WriteTextState *state)
{
	std::string name (reinterpret_cast < char const * > (node->name));
	WriteTextState child_state = *state;
	if (name == "i")
		child_state.italic = true;
	else if (name == "b")
		child_state.bold = true;
	else if (name == "u")
		child_state.underline = true;
	else if (name == "font") {
		xmlChar *buf = xmlGetProp (node, reinterpret_cast < xmlChar const * > ("name"));
		PangoFontDescription *desc = pango_font_description_from_string (reinterpret_cast < char * > (buf));
		xmlFree (buf);
		child_state.size = pango_font_description_get_size (desc) * 20 / PANGO_SCALE;
		std::string family = pango_font_description_get_family (desc);
		if (family == "Arial")
			child_state.font = 3;
		else if (family == "Times New Roman")
			child_state.font = 4;
		else {
			guint16 i = 5;
			std::map < unsigned, CDXFont >::iterator it, itend = m_Fonts.end ();
			for (it = m_Fonts.find (i); it != itend; it++, i++)
				if (family == (*it).second.name)
					break;
			if (it == itend)
				m_Fonts[i] = (CDXFont) {i, kCDXCharSetUnicodeISO10646, family};
			child_state.font = i;
		}
	} else if (name == "sub")
		child_state.position = -1;
	else if (name == "sup")
		child_state.position = 1;
	else if (name == "br") {
		gsize written;
		guint8 const *new_text;
		char *converted = g_convert ("\r", 1,
		                             Charsets[m_Fonts[state->font].encoding].c_str (),
		                             "utf-8", NULL, &written, NULL);
		if (converted)
			new_text = reinterpret_cast < guint8 * > (converted);
		else { // copying raw text and crossing fingers
			new_text = reinterpret_cast < guint8 const * > ("\r");
			written = 1;
		}
		if (written > 0) {
			if (state->buf->size + written > state->buf->capacity) {
				state->buf->capacity += 100 * ((written % 100) + 1);
				state->buf->buf = reinterpret_cast < guint8 * > (g_realloc (state->buf->buf, state->buf->capacity));
			}
			memcpy (state->buf->buf + state->buf->size, new_text, written);
			state->buf->size += written;
			return true;
		}
	} else if (name == "fore") {
		GOColor color = ReadColor (node);
		guint i = 2;
		std::map < unsigned, GOColor >::iterator it, itend = m_Colors.end ();
			for (it = m_Colors.find (i); it != itend; it++, i++)
				if (color == (*it).second)
					break;
			if (it == itend)
				m_Colors[i] = color;
		child_state.color = i;
		
	}
	xmlNodePtr child = node->children;
	if (child == NULL) {
		xmlChar *buf = xmlNodeGetContent (node);
		guint16 bufsize = strlen (reinterpret_cast < char * > (buf));
		if (bufsize > 0) {
			// write the font style run
			WRITEINT16 (state->out, state->index);
			WRITEINT16 (state->out, child_state.font);
			guint16 font_type = 0;
			if (child_state.bold)
				font_type |= 1;
			if (child_state.italic)
				font_type |= 2;
			if (child_state.underline)
				font_type |= 4;
			switch (child_state.position) {
			case -1:
				font_type |= 0x20;
				break;
			case 1:
				font_type |= 0x40;
				break;
			default:
				break;
			}
			WRITEINT16 (state->out, font_type);
			guint16 size =  (child_state.position == 0)? child_state.size: child_state.size * 3 / 2;
			WRITEINT16 (state->out, size);
			WRITEINT16 (state->out, child_state.color);
			guint8 *new_text;
			gsize written;
			char *converted = g_convert (reinterpret_cast < char * > (buf),
				                         bufsize,
				                         Charsets[m_Fonts[child_state.font].encoding].c_str (),
				                         "utf-8", NULL, &written, NULL);
			if (converted)
				new_text = reinterpret_cast < guint8 * > (converted);
			else { // copying raw text and crossing fingers
				new_text = reinterpret_cast < guint8 * > (buf);
				written = bufsize;
			}
			if (written > 0) {
				if (state->buf->size + written > state->buf->capacity) {
					state->buf->capacity += 100 * ((written % 100) + 1);
					state->buf->buf = reinterpret_cast < guint8 * > (g_realloc (state->buf->buf, state->buf->capacity));
				}
				memcpy (state->buf->buf + state->buf->size, new_text, written);
				state->buf->size += written;
			}
			child_state.index = state->buf->size;
			g_free (converted);
		}
		xmlFree (buf);
	} else while (child) {
		WriteNode (child, &child_state);
		child = child->next;
	}
	state->index = child_state.index;
	return true;
}

bool CDXLoader::WriteText (CDXLoader *loader, GsfOutput *out, Object const *obj, GOIOContext *s)
{
	gint16 n = kCDXObj_Text;
	WRITEINT16 (out, n);
	loader->WriteId (obj, out);
	std::string prop = obj->GetProperty (GCU_PROP_POS2D);
	if (prop.length ()) {
		istringstream str (prop);
		double x, y;
		gint32 x_, y_;
		str >> x >> y;
		x_ = x;
		y_ = y + loader->m_CHeight;
		n = kCDXProp_2DPosition;
		WRITEINT16 (out, n);
		gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x08\x00"));
		// write y first
		WRITEINT32 (out, y_);
		WRITEINT32 (out, x_);
	}
	AddInt16Property (out, kCDXProp_ZOrder, loader->m_Z++);
	prop = obj->GetProperty (GCU_PROP_TEXT_ALIGNMENT);
	if (prop == "right")
		AddInt8Property (out, kCDXProp_Justification, 0xff);
	else if (prop == "left")
		AddInt8Property (out, kCDXProp_Justification, 0);
	else if (prop == "center")
		AddInt8Property (out, kCDXProp_Justification, 1);
	prop = obj->GetProperty (GCU_PROP_TEXT_JUSTIFICATION);
	if (prop == "right")
		AddInt8Property (out, kCDXProp_CaptionJustification, 0xff);
	else if (prop == "left")
		AddInt8Property (out, kCDXProp_CaptionJustification, 0);
	else if (prop == "center")
		AddInt8Property (out, kCDXProp_CaptionJustification, 1);
	else if (prop == "justify")
		AddInt8Property (out, kCDXProp_CaptionJustification, 2);
	double inl;
	std::istringstream in (obj->GetProperty (GCU_PROP_TEXT_INTERLINE));
	in >> inl;
	if (inl <= 0.) {
		prop = obj->GetProperty (GCU_PROP_TEXT_VARIABLE_LINE_HEIGHT);
		AddInt16Property (out, kCDXProp_CaptionLineHeight, (prop =="true")? 0: 1);
	} else {
		std::istringstream in (obj->GetProperty (GCU_PROP_TEXT_MAX_LINE_HEIGHT));
		double lh;
		in >> lh;
		AddInt16Property (out, kCDXProp_CaptionLineHeight, inl + lh);
	}
	prop = obj->GetProperty (GCU_PROP_TEXT_MARKUP);
	xmlDocPtr xml = xmlParseMemory (prop.c_str(), prop.length ());
	xmlNodePtr node = xml->children->children;
	GsfOutput *buf = gsf_output_memory_new ();
	ByteBuf contents;
	contents.buf = reinterpret_cast < guint8 * > (g_malloc (100));
	contents.size = 0;
	contents.capacity = 100;
	WriteTextState state;
	state.out = buf;
	state.s = s;
	state.buf = &contents;
	state.italic = false;
	state.bold = false;
	state.underline = false;
	state.font = 3;
	state.size = 10;
	state.position = 0;
	state.color = 3;
	state.index = 0;
	while (node) {
		if (strcmp (reinterpret_cast < char const *>(node->name), "position"))
			loader->WriteNode (node, &state);
		node = node->next;
	}
	gsf_off_t count = gsf_output_size (buf);
	size_t size = contents.size + count + 2;
	n = kCDXProp_Text;
	WRITEINT16 (out, n);
	n = size;
	WRITEINT16 (out, n);
	n = count / 10;
	WRITEINT16 (out, n);
	gsf_output_write (out, count, gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (buf)));
	g_object_unref (buf);
	gsf_output_write (out, contents.size, contents.buf);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x00\x00")); // end of text
	xmlFreeDoc (xml);
	return true;
}

bool CDXLoader::WriteObject (GsfOutput *out, Object const *object, GOIOContext *io)
{
	string name = Object::GetTypeName (object->GetType ());
	map <string, bool (*) (CDXLoader *, GsfOutput *, Object const *, GOIOContext *)>::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (this, out, object, io);
	// if we don't save the object iself, try to save its children
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

void CDXLoader::WriteDateProperty (GsfOutput *out, gint16 id,  GDateTime *date)
{
	WRITEINT16 (out, id);
	guint16 n = 14;
	WRITEINT16 (out, n);
	n = g_date_time_get_year (date);
	WRITEINT16 (out, n);
	n = g_date_time_get_month (date);
	WRITEINT16 (out, n);
	n = g_date_time_get_day_of_month (date);
	WRITEINT16 (out, n);
	n = g_date_time_get_hour (date);
	WRITEINT16 (out, n);
	n = g_date_time_get_minute (date);
	WRITEINT16 (out, n);
	n = g_date_time_get_second (date);
	WRITEINT16 (out, n);
	n = g_date_time_get_microsecond (date) / 1000;
	WRITEINT16 (out, n);
}

void CDXLoader::WriteId (Object const *obj, GsfOutput *out)
{
	if (obj)
		m_SavedIds[obj->GetId ()] = m_MaxId;
	gint32 n = m_MaxId++;
	WRITEINT32 (out, n);
}

void CDXLoader::AddInt8Property (GsfOutput *out, gint16 prop, gint8 value)
{
	WRITEINT16 (out, prop);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x01\x00"));
	gsf_output_write (out, 1, reinterpret_cast <guint8 const *> (&value)); 
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

void CDXLoader::AddBoundingBox (GsfOutput *out, gint32 x0, gint32 y0, gint32 x1, gint32 y1)
{
	gint16 n = kCDXProp_BoundingBox;
	WRITEINT16 (out, n);
	gsf_output_write (out, 2, reinterpret_cast <guint8 const *> ("\x10\x00"));
	WRITEINT32 (out, y1);
	WRITEINT32 (out, x1);
	WRITEINT32 (out, y0);
	WRITEINT32 (out, x0);
}

bool CDXLoader::Write  (Object const *obj, GsfOutput *out, G_GNUC_UNUSED G_GNUC_UNUSED char const *mime_type, GOIOContext *io, G_GNUC_UNUSED ContentType type)
{
	Document const *doc = dynamic_cast <Document const *> (obj);
	gint16 n;
	gint32 l;
	std::string str;
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
	m_Fonts[3] = (CDXFont) {3, kCDXCharSetWin31Latin1, string ("Arial")};
	m_Fonts[4] = (CDXFont) {4, kCDXCharSetWin31Latin1, string ("Times New Roman")};

	gsf_output_write (out, kCDX_HeaderStringLen, (guint8 const *) kCDX_HeaderString);
	gsf_output_write (out, kCDX_HeaderLength - kCDX_HeaderStringLen, (guint8 const *) "\x04\x03\x02\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00");
	str = doc->GetApp ()->GetName () + " " VERSION;
	WriteSimpleStringProperty (out, kCDXProp_CreationProgram, str.length (), str.c_str ());
	// save title authors and the like
	str = doc->GetProperty (GCU_PROP_DOC_CREATOR);
	if (str.length () > 0)
		WriteSimpleStringProperty (out, kCDXProp_CreationUserName, str.length (), str.c_str ());
/* Don't write dates for now since chemdraw does not use them for now
	str = doc->GetProperty (GCU_PROP_DOC_CREATION_TIME);
	unsigned d, m, Y;
	if (sscanf (str.c_str (), "%u-%u-%u", &Y,&m, &d) == 3) {
		GDateTime *date = g_date_time_new_utc (Y, m, d, 0, 0, 0.);
		WriteDateProperty (out, kCDXProp_CreationDate, date);
		g_date_time_unref (date);
	}
	GDateTime *dt = g_date_time_new_now_utc ();
	WriteDateProperty (out, kCDXProp_ModificationDate, dt);
	g_date_time_unref (dt);
*/
	str = doc->GetProperty (GCU_PROP_DOC_TITLE);
	if (str.length () > 0)
		WriteSimpleStringProperty (out, kCDXProp_Name, str.length (), str.c_str ());
	str = doc->GetProperty (GCU_PROP_DOC_COMMENT);
	if (str.length () > 0)
		WriteSimpleStringProperty (out, kCDXProp_Comment, str.length (), str.c_str ());
	// Get the theme (we need a gcp::Document there)
	gcp::Document const *cpDoc = dynamic_cast < gcp::Document const * > (doc);
	gcp::Theme const *theme = cpDoc->GetTheme ();
	m_Zoom = 1. / theme->GetZoomFactor();
	m_Scale = 16384. / m_Zoom;
	m_CHeight = const_cast < gcp::Document * > (cpDoc)->GetView ()->GetCHeight () * 16384. * 3.;
	// determine the bond length and scale the document appropriately
	const_cast <Document *> (doc)->SetScale (1. / 3. / m_Scale);
	l = theme->GetBondLength () * m_Scale * 3.;
	AddInt32Property (out, kCDXProp_BondLength, l);
	n = theme->GetBondDist () * 1000. * m_Zoom / theme->GetBondLength ();
	AddInt16Property (out, kCDXProp_BondSpacing, n);
	l = theme->GetBondWidth () * 16384. * 3.;
	AddInt32Property (out, kCDXProp_LineWidth, l);
	l = theme->GetStereoBondWidth () * 16384. * 3.;
	AddInt32Property (out, kCDXProp_BoldWidth, l);
	l = theme->GetHashDist () * 16384. * 3.;
	AddInt32Property (out, kCDXProp_HashSpacing, l);
	l = theme->GetBondAngle () * 65536.;
	AddInt32Property (out, kCDXProp_ChainAngle, l);
	l = theme->GetPadding () * 16384. * 3.;
	AddInt32Property (out, kCDXProp_MarginWidth, l);

	str = theme->GetTextFontFamily ();
	if (str == "Arial")
		n = 3;
	else if (str == "Times New Roman")
		n = 4;
	else {
		n = 5;
		std::map < unsigned, CDXFont >::iterator it, itend = m_Fonts.end ();
		for (it = m_Fonts.find (n); it != itend; it++, n++)
			if (str == (*it).second.name)
				break;
		if (it == itend)
			m_Fonts[n] = (CDXFont) {static_cast < guint16 > (n), kCDXCharSetUnicodeISO10646, str};
	}
	AddInt16Property (out, kCDXProp_CaptionStyleFont, n);
	n = theme->GetTextFontSize () * 20 / PANGO_SCALE;
	m_FontSize = n * 16384 * 3 / 20;
	AddInt16Property (out, kCDXProp_CaptionStyleSize, n);
	n = 0;
	if (theme->GetTextFontWeight () > PANGO_WEIGHT_NORMAL)
		n |= 1;
	if (theme->GetTextFontStyle () != PANGO_STYLE_NORMAL)
		n |= 2;
	AddInt16Property (out, kCDXProp_CaptionStyleFace, n);
	str = theme->GetFontFamily ();
	if (str == "Arial")
		m_LabelFont = 3;
	else if (str == "Times New Roman")
		m_LabelFont = 4;
	else {
		m_LabelFont = 5;
		std::map < unsigned, CDXFont >::iterator it, itend = m_Fonts.end ();
		for (it = m_Fonts.find (n); it != itend; it++, m_LabelFont++)
			if (str == (*it).second.name)
				break;
		if (it == itend)
			m_Fonts[m_LabelFont] = (CDXFont) {static_cast < guint16 > (m_LabelFont), kCDXCharSetUnicodeISO10646, str};
	}
	AddInt16Property (out, kCDXProp_LabelStyleFont, m_LabelFont);
	m_LabelFontSize = theme->GetFontSize () * 20 / PANGO_SCALE;
	AddInt16Property (out, kCDXProp_LabelStyleSize, m_LabelFontSize);
	m_LabelFontFace = 0x60;
	if (theme->GetFontWeight () > PANGO_WEIGHT_NORMAL)
		m_LabelFontFace |= 1;
	if (theme->GetFontStyle () != PANGO_STYLE_NORMAL)
		m_LabelFontFace |= 2;
	AddInt16Property (out, kCDXProp_LabelStyleFace, m_LabelFontFace);
	m_LabelFontColor = 0; // black
	AddInt16Property (out, kCDXProp_LabelStyleColor, m_LabelFontColor);
	AddInt8Property (out, kCDXProp_CaptionJustification, 0);
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
				if (Z == 6) { // Why???
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
							Molecule *mol = (Doc && Doc->HasChildren ())? dynamic_cast <Molecule *> (Doc->GetFirstChild (i)): NULL;
							if (mol) {
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
											map < gcu::Bondable *, gcu::Bond * >::iterator i;
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
							}
fragment_success:
							if (mol)
								delete mol;
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
						char saved;
						int i = 0;
						// remove modifiers like ' from the buffer
						while (buf[i] >= 'A' && buf[i] < 'z')
							i++;
						saved = buf[i];
						buf[i] = 0;
						Residue const *res = parent->GetDocument ()->GetResidue (buf, &amb);
						// restore the buffer
						buf[i] = saved;
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
				} else if (!ReadGenericObject (in))
					goto bad_exit;
				break;
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
			case kCDXProp_Bond_DoublePosition: {
				gint16 pos;
				if (size != 2 || !(READINT16 (in, pos)))
					return false;
				switch (pos) {
				case 256:
					Bond->SetProperty (GCU_PROP_BOND_DOUBLE_POSITION, "center");
					break;
				case 257:
					Bond->SetProperty (GCU_PROP_BOND_DOUBLE_POSITION, "right");
					break;
				case 258:
					Bond->SetProperty (GCU_PROP_BOND_DOUBLE_POSITION, "left");
					break;
				default:
					Bond->SetProperty (GCU_PROP_BOND_DOUBLE_POSITION, "auto");
					break;
				}
				break;
			}
			case kCDXProp_Bond_CrossingBonds: {
				std::ostringstream out;
				bool first = true;
				size /= 4;
				for (int i = 0; i < size; i++) {
					READINT32 (in, Id);
					if (first)
						first = false;
					else
						out << ' ';
					out << Id;
				}
				Bond->SetProperty (GCU_PROP_BOND_CROSSING, out.str ().c_str ());
				break;
			}
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
	char *utf8str;
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
				str <<  x << " " << y - m_CHeight;
				Text->SetProperty (GCU_PROP_POS2D, str.str ().c_str ());
				break;
			}
			case kCDXProp_Text: {
				guint16 nb;
				bool interpret = false;
				attribs attrs, attrs0;
				if (!(READINT16 (in,nb)))
					return false;
				list <attribs> attributes;
				size -=2;   
				guint16 *n = &attrs.index;
				for (int i = 0; i < nb; i++) {
					if (size < 10)
						return false;
					for (int j = 0; j < 5; j++)
						if (!(READINT16 (in, n[j])))
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
					utf8str = g_convert (buf, size, "utf-8", Charsets[m_Fonts[attrs.font].encoding].c_str (),
					                           NULL, NULL, NULL);
					if (utf8str != NULL)
						Text->SetProperty (GCU_PROP_TEXT_TEXT, utf8str);
					g_free (utf8str);
				} else {
					ostringstream str;
					str << "<text>";
					attrs0 = attributes.front ();
					attributes.pop_front ();
					while (!attributes.empty ()) {
						attrs = attributes.front ();
						attributes.pop_front ();
						if (attrs.index > attrs0.index) {
							if ((attrs0.face & 0x60) != 0 && (attrs0.face & 0x60) != 0x60)
								attrs0.size = attrs0.size * 2 / 3;
							str << "<font name=\"" << m_Fonts[attrs0.font].name << ", " << (double) attrs0.size / 20. << "\">";
							str << "<fore " << colors[attrs0.color] << ">";
							if (attrs0.face & 1)
								str << "<b>";
							if (attrs0.face & 2)
								str << "<i>";
							if (attrs0.face & 4)
								str << "<u>";
							// skip 0x08 == outline since it is not supported
							// skip 0x10 == shadow since it is not supported
							if ((attrs0.face & 0x60) == 0x60)
								interpret = true;
							else if (attrs0.face & 0x20)
								str << "<sub height=\"" << (double) attrs0.size / 40. << "\">";
							else if (attrs0.face & 0x40)
							str << "<sup height=\"" << (double) attrs0.size / 20. << "\">";
							attrs0.index = attrs.index - attrs0.index;
							if (!gsf_input_read (in, attrs0.index, (guint8*) buf))
								return false;
							buf[attrs0.index] = 0;
							std::string encoding = Charsets[m_Fonts[attrs0.font].encoding];
							if (encoding != "Unknown")
									utf8str = g_convert (buf, attrs0.index, "utf-8", encoding.c_str (),
													           NULL, NULL, NULL);
							else { // just copy the string and cross fingers
								utf8str = reinterpret_cast < char * > (g_malloc (attrs0.index + 1));
								strncpy (utf8str, buf, attrs0.index);
								utf8str[attrs0.index] = 0;
							}
							if (interpret) {
								// for now put all numbers as subscripts
								// FIXME: fix this kludgy code
								int cur = 0;
								while (cur < attrs0.index) {
									while (cur < attrs0.index && (utf8str[cur] < '0' || utf8str[cur] > '9'))
										str << utf8str[cur++];
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
										while (utf8str[cur] >= '0' && utf8str[cur] <= '9')
											str << utf8str[cur++];
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
								str << utf8str;
							g_free (utf8str);
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
						attrs0 = attrs;
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
					bool opened = true;
					if (size) {
						if (!gsf_input_read (in, size, (guint8*) buf))
							return false;
						buf[size] = 0;
						std::string encoding = Charsets[m_Fonts[attrs.font].encoding];
						if (encoding != "Unknown")
							utf8str = g_convert (buf, size, "utf-8", encoding.c_str (),
									                   NULL, NULL, NULL);
						else { // just copy the string and cross fingers
							utf8str = reinterpret_cast < char * > (g_malloc (size + 1));
							strncpy (utf8str, buf, size);
							utf8str[size] = 0;
						}
						// supposing the text is ASCII!!
						if (interpret) {
							// for now put all numbers as subscripts
							// FIXME: fix this kludgy code
							int cur = 0;
							while (cur < size) {
								while (cur < size && (utf8str[cur] < '0' || utf8str[cur] > '9'))
									str << utf8str[cur++];
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
									while (utf8str[cur] >= '0' && utf8str[cur] <= '9')
										str << utf8str[cur++];
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
							str << utf8str;
						g_free (utf8str);
					}
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
					Text->SetProperty (GCU_PROP_TEXT_MARKUP, str.str ().c_str ());
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
			case kCDXProp_LineHeight:
			case kCDXProp_CaptionLineHeight: {
				if (size != 2)
					return false;
				gint16 height;
				if (!(READINT16 (in,height)))
					return false;
				if (height == 0)
					Text->SetProperty (GCU_PROP_TEXT_VARIABLE_LINE_HEIGHT, "true");
				else if (height == 1) // we need a better support for line heights
					Text->SetProperty (GCU_PROP_TEXT_VARIABLE_LINE_HEIGHT, "false");
				else if (height > 1) {
					Text->SetProperty (GCU_PROP_TEXT_VARIABLE_LINE_HEIGHT, "false");
					std::istringstream in (Text->GetProperty (GCU_PROP_TEXT_MAX_LINE_HEIGHT));
					double lh;
					in >> lh;
					std::ostringstream out;
					out << height - lh;
					Text->SetProperty (GCU_PROP_TEXT_INTERLINE, out.str ().c_str ());
				}
				break;
			}
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
	m_Scheme.Steps.clear ();
	if (!(READINT32 (in,m_Scheme.Id)))
		return false;
	if (!(READINT16 (in,code)))
		return false;
	while (code) {
		if (code == kCDXObj_ReactionStep) {
			if (!ReadStep (in, parent))
				return false;
		} else 
			return false;
		if (!(READINT16 (in,code)))
			return false;
	}
	m_Schemes.push_back (m_Scheme);
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
					data.ObjectsBelow.push_back (id);
				}
				break;
			case kCDXProp_ReactionStep_Arrows:
				if ((size % 4) != 0)
					return false;
				max = size / 4;
				for (i = 0; i < max; i++) {
					if (!(READINT32 (in, id)))
						return false;
					data.Arrows.push_back (id);
				}
				break;
			default:
				if (size && gsf_input_seek (in, size, G_SEEK_CUR))
					return false;
			}
		}
		if (!(READINT16 (in,code)))
			return false;
	}
	m_Scheme.Steps.push_back (data);
	return true;
}

void CDXLoader::BuildScheme (gcu::Document *doc, SchemeData &scheme)
{
	std::list < StepData >::iterator i, iend = scheme.Steps.end ();
	std::list < unsigned >::iterator j, jend;
	int IsReaction = 0, IsMesomery = 0, IsRetrosynthesis = 0;
	bool HasMesomeryArrows = false;
	gcu::Object *parent, *arrow, *obj, *step, *reactant;
	for (i = scheme.Steps.begin (); i != iend; i++) {
		if ((*i).Arrows.size () != 1)
			return; // unsupported feature, don't load the scheme
		obj = doc->GetChild ((m_LoadedIds[*((*i).Arrows.begin())]).c_str ());
		if (obj == NULL)
			return;
		std::string klass = gcu::Object::GetTypeName (obj->GetType ());
		if (klass == "retrosynthesis-arrow") {
			if (IsRetrosynthesis == -1)
				return;
			IsRetrosynthesis = 1;
			IsReaction = IsMesomery = -1;
		} else if (klass ==  "mesomery-arrow") {
			if (IsMesomery == -1)
				return;
			IsRetrosynthesis = -1;
			if (IsMesomery == 0 && IsReaction == 0)
				IsMesomery = 1;
			HasMesomeryArrows = true;
		} else if (klass ==  "reaction-arrow") {
			if (IsReaction == -1 || IsMesomery == -1)
				return;
			IsReaction = 1;
			IsRetrosynthesis = -1;
			IsMesomery = 0;
		} else
			return;
	}
	if (IsRetrosynthesis == 1) {
		gcu::Object *retrosynthesis = doc->CreateObject ("retrosynthesis", doc);
		std::set < std::string > targets;
		std::set < std::string >::iterator target;
		ostringstream str;
		str << "rsy" << scheme.Id;
		retrosynthesis->SetId (str.str ().c_str ());
		m_LoadedIds[scheme.Id] = retrosynthesis->GetId ();
		// now, add the objects to the retrosynthesis
		for (i = scheme.Steps.begin (); i != iend; i++) {
			if ((*i).Reagents.size () != 1 || (*i).Products.size () != 1) {
				delete retrosynthesis;
				return;
			}
			// first the arrow
			arrow = doc->GetChild ((m_LoadedIds[*((*i).Arrows.begin())]).c_str ());
			obj = doc->GetDescendant (m_LoadedIds[*(*i).Reagents.begin ()].c_str ());
			parent = obj->GetParent ();
			if (parent == doc)
				parent = doc->CreateObject ("retrosynthesis-step", retrosynthesis);
			else if (parent->GetParent () != retrosynthesis) {
				delete retrosynthesis;
				return;
			}
			parent->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
			arrow->SetProperty (GCU_PROP_ARROW_START_ID, parent->GetId ());
			targets.insert (parent->GetId ());
			obj = doc->GetDescendant (m_LoadedIds[*(*i).Products.begin ()].c_str ());
			parent = obj->GetParent ();
			if (parent == doc)
				parent = doc->CreateObject ("retrosynthesis-step", retrosynthesis);
			else if (parent->GetParent () != retrosynthesis) {
				delete retrosynthesis;
				return;
			}
			parent->AddChild (obj);
			arrow->SetProperty (GCU_PROP_ARROW_END_ID, parent->GetId ());
			target = targets.find (parent->GetId ());
			if (target != targets.end ())
				targets.erase (target);
			retrosynthesis->AddChild (arrow);
		}
		if (targets.size () != 1) {
			delete retrosynthesis;
			return;
		}
		// using GCU_PROP_MOLECULE even if not ideal (the target is a step, not the molecule inside)
		retrosynthesis->SetProperty (GCU_PROP_MOLECULE, (*targets.begin()).c_str ());
		// Ignore objects over and under the arrows for now
	} else if (IsMesomery == 1) {
		gcu::Object *mesomery = doc->CreateObject ("mesomery", doc);
		ostringstream str;
		str << "msy" << scheme.Id;
		mesomery->SetId (str.str ().c_str ());
		m_LoadedIds[scheme.Id] = mesomery->GetId ();
		// now, add the objects to the reaction
		for (i = scheme.Steps.begin (); i != iend; i++) {
			if ((*i).Reagents.size () != 1 || (*i).Products.size () != 1) {
				delete mesomery;
				return;
			}
			// first the arrow
			arrow = doc->GetChild ((m_LoadedIds[*((*i).Arrows.begin())]).c_str ());
			obj = doc->GetDescendant (m_LoadedIds[*(*i).Reagents.begin ()].c_str ());
			parent = obj->GetParent ();
			if (parent == doc)
				parent = doc->CreateObject ("mesomer", mesomery);
			else if (parent->GetParent () != mesomery) {
				delete mesomery;
				return;
			}
			parent->SetProperty (GCU_PROP_MESOMER, obj->GetId ());
			arrow->SetProperty (GCU_PROP_ARROW_START_ID, parent->GetId ());
			obj = doc->GetDescendant (m_LoadedIds[*(*i).Products.begin ()].c_str ());
			parent = obj->GetParent ();
			if (parent == doc)
				parent = doc->CreateObject ("mesomer", mesomery);
			else if (parent->GetParent () != mesomery) {
				delete mesomery;
				return;
			}
			parent->AddChild (obj);
			arrow->SetProperty (GCU_PROP_ARROW_END_ID, parent->GetId ());
			mesomery->AddChild (arrow);
		}
		// Ignore objects over and under the arrows for now
	} else if (IsReaction ==1) {
		if (HasMesomeryArrows) {
			// build mesomeries inside reactions
			// FIXME: nots supported for now
			return;
		}
		gcu::Object *reaction = doc->CreateObject ("reaction", doc);
		ostringstream str;
		str << "r" << scheme.Id;
		reaction->SetId (str.str ().c_str ());
		m_LoadedIds[scheme.Id] = reaction->GetId ();
		// now, add the objects to the reaction
		for (i = scheme.Steps.begin (); i != iend; i++) {
			// first the arrow
			arrow = doc->GetChild ((m_LoadedIds[*((*i).Arrows.begin())]).c_str ());
			reaction->AddChild (arrow);
			// then reagents
			jend = (*i).Reagents.end ();
			parent = NULL;
			gcu::Object *rs = NULL; // make g++ happy
			for (j = (*i).Reagents.begin (); j != jend; j++) {
				obj = doc->GetDescendant (m_LoadedIds[*j].c_str ());
				if (obj == NULL) {
					delete reaction;
					return;
				}
				parent = obj->GetParent ();
				if (rs == NULL) {
					if (parent == doc) {
						rs = reaction->CreateObject ("reaction-step", reaction);
						arrow->SetProperty (GCU_PROP_ARROW_START_ID, rs->GetId ());
						reactant = rs->CreateObject ("reactant", rs);
						reactant->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
					} else {
						rs = parent->GetParent ();
						if (rs->GetParent () != reaction) {
							delete reaction;
							return;
						}
					}
				} else {
					if (parent == doc) {
						reactant = rs->CreateObject ("reactant", rs);
						reactant->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
					} else if (rs != parent->GetParent ()) {
						delete reaction;
						return;
					}
				}
				// search for potential stoichiometry coefficients
				arrow->SetProperty (GCU_PROP_ARROW_START_ID, rs->GetId ());
				rs->OnLoaded ();
			}
			// same treatment for products
			jend = (*i).Products.end ();
			rs = NULL;
			for (j = (*i).Products.begin (); j != jend; j++) {
				obj = doc->GetDescendant (m_LoadedIds[*j].c_str ());
				if (obj == NULL) {
					delete reaction;
					return;
				}
				parent = obj->GetParent ();
				if (rs == NULL) {
					if (parent == doc) {
						rs = reaction->CreateObject ("reaction-step", reaction);
						arrow->SetProperty (GCU_PROP_ARROW_END_ID, rs->GetId ());
						reactant = rs->CreateObject ("reactant", rs);
						reactant->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
					} else {
						rs = parent->GetParent ();
						if (rs->GetParent () != reaction) {
							delete reaction;
							return;
						}
					}
				} else {
					if (parent == doc) {
						reactant = rs->CreateObject ("reactant", rs);
						reactant->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
					} else if (rs != parent->GetParent ()) {
						delete reaction;
						return;
					}
				}
				// search for potential stoichiometry coefficients
				arrow->SetProperty (GCU_PROP_ARROW_END_ID, rs->GetId ());
				rs->OnLoaded ();
			}
			// last, the objects attached above and below the arrow
			if (!(*i).ObjectsAbove.empty () || !(*i).ObjectsBelow.empty ()) {
				ostringstream str;
				str << (*i).ObjectsAbove.size ();
				arrow->SetProperty (GCU_PROP_REACTION_ARROW_MAX_LINES_ABOVE, str.str ().c_str ());
				str.str ("");
				unsigned n = 1;
				jend = (*i).ObjectsAbove.end ();
				for (j = (*i).ObjectsAbove.begin (); j != jend; j++) {
					obj = doc->GetDescendant (m_LoadedIds[*j].c_str ());
					if (obj == NULL) // we should emit at least a warning
						continue;
					parent = arrow->CreateObject ("reaction-prop", arrow);
					parent->SetProperty (GCU_PROP_ARROW_OBJECT, obj->GetId ());
					str << n;
					parent->SetProperty (GCU_PROP_REACTION_ARROW_PROP_LINE, str.str ().c_str ());
					str.str ("");
					n++;
				}
				jend = (*i).ObjectsBelow.end ();
				for (j = (*i).ObjectsBelow.begin (); j != jend; j++) {
					obj = doc->GetDescendant (m_LoadedIds[*j].c_str ());
					if (obj == NULL) // we should emit at least a warning
						continue;
					parent = arrow->CreateObject ("reaction-prop", arrow);
					parent->SetProperty (GCU_PROP_ARROW_OBJECT, obj->GetId ());
					str << n;
					parent->SetProperty (GCU_PROP_REACTION_ARROW_PROP_LINE, str.str ().c_str ());
					str.str ("");
					n++;
				}
			}
		}
		// now search for stoichiometry coefficients if any
		gcp::WidgetData *data = static_cast <gcp::Document * > (doc)->GetView ()->GetData ();
		gccv::Rect rect;
		double x0, y0, x1;
		std::map < std::string, Object * >::iterator k, l, r;
		std::pair <gcu::Object *, gcu::Object * > couple;
		std::list < std::pair <gcu::Object *, gcu::Object * > > couples;
		obj = doc->GetFirstChild (k);
		while (obj) {
			// assuming that only text object can be stoichiometric coefs
			if (obj->GetType () == gcu::TextType) {
				data->GetObjectBounds (obj, rect);
				x0 = rect.x0;
				y0 = (rect.y0 + rect.y1) / 2.;
				x1 = rect.x1 + padding;
				for (step = reaction->GetFirstChild (l); step; step = reaction->GetNextChild (l)) {
					if (step->GetType () != gcp::ReactionStepType)
						continue;
					data->GetObjectBounds (step, rect);
					if (x0 > rect.x1 || x1 < rect.x0 || y0 > rect.y1 || y0 < rect.y0)
						continue;
					for (reactant = step->GetFirstChild (r); reactant; reactant = step->GetNextChild (r)) {
						if (reactant->GetType () != gcu::ReactantType)
							continue;
						data->GetObjectBounds (reactant, rect);
						if (x0 > rect.x0 || x1 < rect.x0 || y0 > rect.y1 || y0 < rect.y0)
							continue;
						// if we get there, we got it
						// we must not set it now to avoid an invalid iterator at this point, so store in couples.
						couple.first = reactant;
						couple.second = obj;
						couples.push_back (couple);
						goto next_text;
					}
				}
			}
next_text:
			obj = doc->GetNextChild (k);
		}
		std::list < std::pair <gcu::Object *, gcu::Object * > >::iterator c, cend = couples.end ();
		for (c = couples.begin (); c != cend; c++) {
			(*c).first->SetProperty (GCU_PROP_STOICHIOMETRY, (*c).second->GetId ());
		}
	}
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
