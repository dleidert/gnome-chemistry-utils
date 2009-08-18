// -*- C++ -*-

/* 
 * CDXML files loader plugin
 * cdx.cc 
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

#include "config.h"
#include <gcu/application.h>
#include <gcu/atom.h>
#include <gcu/bond.h>
#include <gcu/document.h>
#include <gcu/element.h>
#include <gcu/formula.h>
#include <gcu/loader.h>
#include <gcu/molecule.h>
#include <gcu/objprops.h>
#include <gcu/residue.h>

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

class CDXLoader: public gcu::Loader
{
public:
	CDXLoader ();
	virtual ~CDXLoader ();

	ContentType Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Object *obj, GsfOutput *out, char const *mime_type, IOContext *io, ContentType type);

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
	guint16 ReadSize (GsfInput *in);
	bool ReadDate (GsfInput *in);

	bool WriteObject (GsfOutput *out, Object *object, IOContext *io);
	void WriteSimpleStringProperty (GsfOutput *out, unsigned id, unsigned length, char const *data);

private:
	char *buf;
	size_t bufsize;
	map<unsigned, CDXFont> m_Fonts;
	vector <string> colors;
	guint8 m_TextAlign, m_TextJustify;

	map <string, bool (*) (CDXLoader *, GsfOutput *, Object *, IOContext *)> m_WriteCallbacks;
	map<unsigned, GOColor> m_Colors;
};

/******************************************************************************
 *	Write callbacks															  *
 ******************************************************************************/
 
static bool cdx_write_atom (CDXLoader *loader, GsfOutput *out, Object *obj, IOContext *s)
{
	return true;
}

static bool cdx_write_bond (CDXLoader *loader, GsfOutput *out, Object *obj, IOContext *s)
{
	return true;
}

static bool cdx_write_molecule (CDXLoader *loader, GsfOutput *out, Object *obj, IOContext *s)
{
	return true;
}

CDXLoader::CDXLoader ():
	m_TextAlign (0),
	m_TextJustify (0)
{
	AddMimeType ("chemical/x-cdx");
	// Add write callbacks
	m_WriteCallbacks["atom"] = cdx_write_atom;
	m_WriteCallbacks["bond"] = cdx_write_bond;
	m_WriteCallbacks["molecule"] = cdx_write_molecule;
}

CDXLoader::~CDXLoader ()
{
	RemoveMimeType ("chemical/x-cdx");
}

ContentType CDXLoader::Read  (Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED IOContext *io)
{
	if (doc == NULL || in == NULL)
		return ContentTypeUnknown;
	ContentType result = ContentType2D;
	guint16 code;
	bufsize = 64;
	buf = new char [bufsize];
	// note that we read 28 bytes here while headers for recent cdx files have only 22 bytes, remaining are 0x8000 (document) and its id (0)
	if (!gsf_input_read (in, kCDX_HeaderLength, (guint8*) buf) || strncmp (buf, kCDX_HeaderString, kCDX_HeaderStringLen)) {
		result = ContentTypeUnknown;
		code = 0;
	} else if (!(READINT16 (in, code))) {
		result = ContentTypeUnknown;
		code = 0;
	}

	while (code) {
		if (code & kCDXTag_Object) {
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
				snprintf (buf, bufsize, "%u", length);
				doc->SetProperty (GCU_PROP_THEME_BOND_LENGTH, buf);
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
			}
			break;
			case kCDXProp_ColorTable: {
				colors.push_back ("red=\"1\" green=\"1\" blue=\"1\""); // white
				colors.push_back ("red=\"0\" green=\"0\" blue=\"0\""); // black
				unsigned nb = (size - 2) / 6;
				if (!(READINT16 (in,size)) || size != nb)
					return ContentTypeUnknown;
				guint16 red, blue, green;
				for (unsigned i = 0; i < nb; i++) {
				if (!(READINT16 (in,red)) || !(READINT16 (in,green)) || !(READINT16 (in,blue)))
					return ContentTypeUnknown;
					snprintf (buf, bufsize, "red=\"%g\" green=\"%g\" blue=\"%g\"", (double) red / 0xffff, (double) green / 0xffff, (double) blue / 0xffff);
					colors.push_back (buf);
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
	return result;
}

bool CDXLoader::WriteObject (GsfOutput *out, Object *object, IOContext *io)
{
	string name = Object::GetTypeName (object->GetType ());
	map <string, bool (*) (CDXLoader *, GsfOutput *, Object *, IOContext *)>::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (this, out, object, io);
	return true; /* loosing data is not considered an error, it is just a missing feature
					either in this code or in the cml schema */
}

void CDXLoader::WriteSimpleStringProperty (GsfOutput *out, unsigned id, unsigned length, char const *data)
{
	WRITEINT16 (out, id);
	WRITEINT16 (out, length);
	length = 0;
	WRITEINT16 (out, length); // number of runs in the string
	gsf_output_write (out, length, reinterpret_cast <guint8 const *> (data));
}

bool CDXLoader::Write  (Object *obj, GsfOutput *out, G_GNUC_UNUSED G_GNUC_UNUSED char const *mime_type, IOContext *io, G_GNUC_UNUSED ContentType type)
{
	Document *doc = dynamic_cast <Document *> (obj);
	int n;
	if (!doc || !out)
		return false;

	// Init colors
	m_Colors[2] = RGBA_WHITE;
	m_Colors[3] = RGBA_BLACK;
	m_Colors[4] = RGBA_RED;
	m_Colors[5] = RGBA_YELLOW;
	m_Colors[6] = RGBA_GREEN;
	m_Colors[7] = RGBA_CYAN;
	m_Colors[8] = RGBA_BLUE;
	m_Colors[9] = RGBA_VIOLET;

	// Init fonts, we always use Unknown as the charset, hoping it is not an issue
	m_Fonts[3] = (CDXFont) {3, kCDXCharSetUnknown, string ("Arial")};
	m_Fonts[4] = (CDXFont) {4, kCDXCharSetUnknown, string ("Times New Roman")};

	gsf_output_write (out, kCDX_HeaderStringLen, (guint8 const *) kCDX_HeaderString);
	gsf_output_write (out, kCDX_HeaderLength - kCDX_HeaderStringLen, (guint8 const *) "\x04\x03\x02\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00");
	std::string app = doc->GetApp ()->GetName () + " "VERSION;
	WriteSimpleStringProperty (out, kCDXProp_CreationProgram, app.length (), app.c_str ());
	// write the document contents
	// there is a need for a two paths procedure
	// in the first path, we collect fonts and colors
	// everything else is saved during the second path
	// write end of document and end of file
	GsfOutput *buf = gsf_output_memory_new ();
	std::map <std::string, Object *>::iterator i;
	Object *child = doc->GetFirstChild (i);
	while (child) {
		if (!WriteObject (buf, child, io)) {
			g_object_unref (buf);
			m_Colors.clear ();
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
		n = UINT_RGBA_R ((*color).second) * 0x101;
		WRITEINT16 (out, n);
		n = UINT_RGBA_G ((*color).second) * 0x101;
		WRITEINT16 (out, n);
		n = UINT_RGBA_B ((*color).second) * 0x101;
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
	n = 0; // say we are on a mac even if not true
	WRITEINT16 (out, n);
	n = m_Fonts.size ();
	WRITEINT16 (out, n);
	for (font = m_Fonts.begin (); font != end_font; font++) {
		WRITEINT16 (out, (*font).second.index);
		WRITEINT16 (out, (*font).second.encoding);
		gsf_output_write (out, (*font).second.name.length (),
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
	Object *mol = Object::CreateObject ("molecule", parent);
	guint32 Id;
	if (!(READINT32 (in,Id)))
		return false;
	snprintf (buf, bufsize, "m%d", Id);
	mol->SetId (buf);
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
	return true;
}

bool CDXLoader::ReadAtom (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Atom = Object::CreateObject ("atom", parent);
	Document *Doc = NULL;
	Atom->SetProperty (GCU_PROP_ATOM_Z, "6");
	guint32 Id;
	int type = 0;
	int Z = 6;
	if (!(READINT32 (in,Id)))
		return false;
	snprintf (buf, bufsize, "a%d", Id);
	Atom->SetId (buf);
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
								Atom = Object::CreateObject ("fragment", parent);
								Atom->SetProperty (GCU_PROP_TEXT_TEXT, buf);
								snprintf (buf, bufsize, "a%d", Id);
								Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, buf);
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
								Atom = Object::CreateObject ("fragment", parent);
								Atom->SetProperty (GCU_PROP_TEXT_TEXT, buf);
								snprintf (buf, bufsize, "a%d", Id);
								Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, buf);
								Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_START, "0");
								Atom->SetProperty (GCU_PROP_POS2D, pos.c_str ());
							} else {
								// FIXME: should the document care with the residues?
								g_warning (_("Unsupported feature, please report!"));
							}
						} else {
							// FIXME: Unkown residue: add it to the database? or just to the document?
							g_warning (_("Unsupported feature, please report!"));
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
							Atom = Object::CreateObject ("fragment", parent);
							Atom->SetProperty (GCU_PROP_TEXT_TEXT, buf);
							snprintf (buf, bufsize, "a%d", Id);
							Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, buf);
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
							Atom = Object::CreateObject ("fragment", parent);
							Atom->SetProperty (GCU_PROP_TEXT_TEXT, buf);
							snprintf (buf, bufsize, "a%d", Id);
							Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, buf);
							Atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_START, "0");
							Atom->SetProperty (GCU_PROP_POS2D, pos.c_str ());
						} else {
							// TODO: import it in the document
							g_warning (_("Unsupported feature, please report!"));
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
				snprintf (buf, bufsize, "%d %d", x, y);
				Atom->SetProperty (GCU_PROP_POS2D, buf);
				break;
			}
			case kCDXProp_Node_Element:
				if (size != 2 || !(READINT16 (in,size)))
					goto bad_exit;
				Z = size;
				snprintf (buf, bufsize, "%u", size);
				Atom->SetProperty (GCU_PROP_ATOM_Z, buf);
				break;
			case kCDXProp_Atom_Charge:
				gint8 charge;
				if (size!= 1 || !gsf_input_read (in, 1, (guint8*) &charge))
					goto bad_exit;
				snprintf (buf, bufsize, "%d", charge);
				Atom->SetProperty (GCU_PROP_ATOM_CHARGE, buf);
				break;
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
					Atom = Object::CreateObject ("pseudo-atom", parent);
					snprintf (buf, bufsize, "a%d", Id);
					Atom->SetId (buf);
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
	return true;
bad_exit:
	if (Doc)
		delete Doc;
	return false;
}

bool CDXLoader::ReadBond (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Bond = Object::CreateObject ("bond", parent);
	guint32 Id;
	if (!(READINT32 (in,Id)))
		return false;
	snprintf (buf, bufsize, "b%d", Id);
	Bond->SetId (buf);
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
				snprintf (buf, bufsize, "%u", Id);
				Bond->SetProperty (GCU_PROP_BOND_BEGIN, buf);
				break;
			}
			case kCDXProp_Bond_End: {
				if (size != 4 || !(READINT32 (in,Id)))
					return false;
				snprintf (buf, bufsize, "%u", Id);
				Bond->SetProperty (GCU_PROP_BOND_END, buf);
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
	Object *Text= Object::CreateObject ("text", parent);
	guint32 Id;
	guint8 TextAlign = 0xfe, TextJustify = 0xfe;
	if (!(READINT32 (in,Id)))
		return false;
	snprintf (buf, bufsize, "t%d", Id);
	Text->SetId (buf);
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
				snprintf (buf, bufsize, "%d %d", x, y);
				Text->SetProperty (GCU_PROP_POS2D, buf);
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
									while (cur < attrs0.index && (buf[cur] < '0' || buf[cur] > '9')){printf("cur=%d c=%c\n",cur,buf[cur]);
										str << buf[cur++];}
									if (cur < attrs0.index) {
										if (attrs0.face & 4)
											str << "</u>";
										if (attrs0.face & 2)
											str << "</i>";
										if (attrs0.face & 1)
											str << "</b>";
										str << "</fore></font><font name=\"" << m_Fonts[attrs.font].name << " " << (double) attrs.size / 30. << "\">";
										str << "<fore " << colors[attrs.color] << ">";
										str << "<sub height=\"" << (double) attrs.size / 60. << "\">";
										while (buf[cur] >= '0' && buf[cur] <= '9'){printf("cur=%d c=%c\n",cur,buf[cur]);
											str << buf[cur++];}
										str << "</sub></fore></font><font name=\"" << m_Fonts[attrs.font].name << " " << (double) attrs.size / 20. << "\">";
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
						str << "<font name=\"" << m_Fonts[attrs.font].name << " " << (double) attrs.size / 20. << "\">";
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
								str << "</fore></font><font name=\"" << m_Fonts[attrs.font].name << " " << (double) attrs.size / 30. << "\">";
								str << "<fore " << colors[attrs.color] << ">";
								str << "<sub height=\"" << (double) attrs.size / 60. << "\">";
								while (buf[cur] >= '0' && buf[cur] <= '9')
									str << buf[cur++];
								str << "</sub></fore></font>";
								if (cur < size) {
									str << "<font name=\"" << m_Fonts[attrs.font].name << " " << (double) attrs.size / 20. << "\">";
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
	return true;
}

bool CDXLoader::ReadGroup (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Group= Object::CreateObject ("group", parent);
	Group->Lock ();
	if (gsf_input_seek (in, 4, G_SEEK_CUR)) //skip the id
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
		switch (arrow_type) {
		case 1:
		case 2:
			obj = Object::CreateObject ("reaction-arrow", parent);
			snprintf (buf, bufsize, "ra%d", Id);
			break;
		case 4:
			obj = Object::CreateObject ("mesomery-arrow", parent);
			snprintf (buf, bufsize, "ma%d", Id);
			break;
		case 8:
			obj = Object::CreateObject ("reaction-arrow", parent);
			snprintf (buf, bufsize, "ra%d", Id);
			obj->SetProperty (GCU_PROP_REACTION_ARROW_TYPE, "double");
			break;
		case 32:
			obj = Object::CreateObject ("retrosynthesis-arrow", parent);
			snprintf (buf, bufsize, "rsa%d", Id);
			break;
		default:
			break;
		}
		if (obj) {
			obj->SetId (buf);
			snprintf (buf, bufsize, "%d %d %d %d", x0, y0, x1, y1);
			obj->SetProperty (GCU_PROP_ARROW_COORDS, buf);
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
			switch (code) {
/*			case kCDXProp_2DPosition: {
				if (size != 8)
					return false;
				gint32 x, y;
				if (!READINT32 (in, y))
					return false;
				if (!READINT32 (in, x))
					return false;
				snprintf (buf, bufsize, "%d %d", x, y);
//				Text->SetProperty (GCU_PROP_POS2D, buf);
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
