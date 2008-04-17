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
#include <gcu/document.h>
#include <gcu/formula.h>
#include <gcu/loader.h>
#include <gcu/molecule.h>
#include <gcu/objprops.h>
#include <gcu/residue.h>

#include <goffice/app/module-plugin-defs.h>
#include <openbabel/chemdrawcdx.h>
#include <map>
#include <string>
#include <vector>
#include <cstdio>
#include <libintl.h>

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

typedef struct {
	guint16 index;
	guint16 encoding;
	string name;
} CDXFont;

static map<guint16, string> Charsets;
static map<string, guint16> CharsetIDs;

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
	bool ReadText (GsfInput *in, Object *parent);
	bool ReadGroup (GsfInput *in, Object *parent);
	bool ReadGraphic (GsfInput *in, Object *parent);
	bool ReadFragmentText (GsfInput *in, Object *parent);
	guint16 ReadSize (GsfInput *in);
	bool ReadDate (GsfInput *in);

private:
	char *buf;
	size_t bufsize;
	map<unsigned, CDXFont> fonts;
	vector <string> colors;
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
	if (doc == NULL || in == NULL)
		return false;
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
			case kCDXProp_CreationUserName:
				gsf_input_read (in, size, (guint8*) buf);
				doc->SetProperty (GCU_PROP_DOC_CREATOR, buf);
				break;
			case kCDXProp_CreationDate: {
				if (size != 14 || !ReadDate (in)) {
					result = false;
					break;
				}
				doc->SetProperty (GCU_PROP_DOC_CREATION_TIME, buf);
				break;
			}
			case kCDXProp_ModificationDate:{ 
				if (size != 14 || !ReadDate (in)) {
					result = false;
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
					result = false;
					break;
				}
				guint32 length;
				if (!READINT32 (in, buf, length)) {
					result = false;
					break;
				}
				snprintf (buf, bufsize, "%u", length);
				doc->SetProperty (GCU_PROP_THEME_BOND_LENGTH, buf);
				break;
			}
			case kCDXProp_FontTable: {
				// skip origin platform and read fonts number
				guint16 nb;
				if (gsf_input_seek (in, 2, G_SEEK_CUR) || !READINT16 (in, buf, nb))
					return false;
				CDXFont font;
				for (int i = 0; i < nb; i++) {
					if (!READINT16 (in, buf, font.index) ||
						!READINT16 (in, buf, font.encoding) ||
						!READINT16 (in, buf, size))
						return false;
					gsf_input_read (in, size, (guint8*) buf);
					buf[size] = 0;
					font.name = buf;
					fonts[font.index] = font;
				}
			}
			break;
			case kCDXProp_ColorTable: {
				colors.push_back ("1 1 1"); // white
				colors.push_back ("0 0 0"); // black
				unsigned nb = (size - 2) / 6;
				if (!READINT16 (in, buf, size) || size != nb)
					return false;
				guint16 red, blue, green;
				for (unsigned i = 0; i < nb; i++) {
				if (!READINT16 (in, buf, red) || !READINT16 (in, buf, green) || !READINT16 (in, buf, blue))
					return false;
					snprintf (buf, bufsize, "%g %g %g", (double) red / 0xffff, (double) green / 0xffff, (double) blue / 0xffff);
					colors.push_back (buf);
				}
				break;
			}
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
	fonts.clear ();
	return result;
}

bool CDXLoader::Write  (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io)
{
	gsf_output_write (out, kCDX_HeaderStringLen, (guint8 const *) kCDX_HeaderString);
	gsf_output_write (out, kCDX_HeaderLength - kCDX_HeaderStringLen, (guint8 const *) "\x04\x03\x02\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00");
	// write the document contents
	// there is a need for a two paths procedure
	// in the first path, we collect fonts and colors
	// everything else is saved during the second path
	// write end of document and end of file
	gsf_output_write (out, 4, (guint8 const *) "\x00\x00\x00\x00");
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
		if (!READINT16 (in, buf, code))
			return false;
	}
	return true;
}

bool CDXLoader::ReadMolecule (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *mol = Object::CreateObject ("molecule", parent);
	guint32 Id;
	if (!READINT32 (in, buf, Id))
		return false;
	snprintf (buf, bufsize, "m%d", Id);
	mol->SetId (buf);
	if (!READINT16 (in, buf, code))
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
		if (!READINT16 (in, buf, code))
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
	if (!READINT32 (in, buf, Id))
		return false;
	snprintf (buf, bufsize, "a%d", Id);
	Atom->SetId (buf);
	if (!READINT16 (in, buf, code))
		return false;
	while (code) {
		if (code & kCDXTag_Object) {
			switch (code) {
			case kCDXObj_Fragment: {
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
						// now build a molecule from the formula
						Molecule *mol2 = Molecule::MoleculeFromFormula (Doc, form);
					}
					catch (parse_error &error) {
						return false;
					}
					break;
				case 4: {
					bool amb;
					Residue const *res = Residue::GetResidue (buf, &amb);
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
						}
					} else {
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
						gcu::Atom *a;
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
								// try adding a new residue
							}
						}
						catch (parse_error &error) {
							int start, length;
							puts (error.what (start, length));
						}
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
				default:
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
				if (size != 8 || !READINT32 (in, buf, y) || !READINT32 (in, buf, x))
					goto bad_exit;
				snprintf (buf, bufsize, "%d %d", x, y);
				Atom->SetProperty (GCU_PROP_POS2D, buf);
				break;
			}
			case kCDXProp_Node_Element:
				if (size != 2 || !READINT16 (in, buf, size))
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
				if (size != 2 || !READINT16 (in, buf, type))
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
		if (!READINT16 (in, buf, code))
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
				if (size != 4 || !READINT32 (in, buf, Id))
					return false;
				snprintf (buf, bufsize, "%u", Id);
				Bond->SetProperty (GCU_PROP_BOND_BEGIN, buf);
				break;
			}
			case kCDXProp_Bond_End: {
				if (size != 4 || !READINT32 (in, buf, Id))
					return false;
				snprintf (buf, bufsize, "%u", Id);
				Bond->SetProperty (GCU_PROP_BOND_END, buf);
				break;
			}
			case kCDXProp_Bond_Order:
				if (size != 2 || !READINT16 (in, buf, size))
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
				if (size != 2 || !READINT16 (in, buf, size))
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
		if (!READINT16 (in, buf, code))
			return false;
	}
	return true;
}

bool CDXLoader::ReadText (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Text= Object::CreateObject ("text", parent);
	guint32 Id;
	if (!READINT32 (in, buf, Id))
		return false;
	snprintf (buf, bufsize, "t%d", Id);
	Text->SetId (buf);
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
				gint32 x, y;
				if (!READINT32 (in, buf, y))
					return false;
				if (!READINT32 (in, buf, x))
					return false;
				snprintf (buf, bufsize, "%d %d", x, y);
				Text->SetProperty (GCU_PROP_POS2D, buf);
				break;
			}
			case kCDXProp_Text: {
				guint16 nb;
				if (!READINT16 (in, buf, nb))
					return false;
				size -=2;
				for (int i =0; i < nb; i++) {
					if (size < 10)
						return false;
					guint16 n[5];
					for (int j = 0; j < 5; j++)
						if (!READINT16 (in, buf, n[j]))
							return false;
					size -= 10;
				}
				if (size < 1)
					return false;
				if (!gsf_input_read (in, size, (guint8*) buf))
					return false;
				buf[size] = 0;
				Text->SetProperty (GCU_PROP_TEXT_TEXT, buf);
				break;
			}
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

bool CDXLoader::ReadGroup (GsfInput *in, Object *parent)
{
	guint16 code;
	Object *Group= Object::CreateObject ("group", parent);
	Group->Lock ();
	if (gsf_input_seek (in, 4, G_SEEK_CUR)) //skip the id
		return false;
	if (!READINT16 (in, buf, code))
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
		if (!READINT16 (in, buf, code))
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
	if (!READINT32 (in, buf, Id) || !READINT16 (in, buf, code))
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
				if (size != 16 || !READINT32 (in, buf, y1) || !READINT32 (in, buf, x1)
					|| !READINT32 (in, buf, y0) || !READINT32 (in, buf, x0))
					return false;
				break;
			case kCDXProp_Graphic_Type:
				if (size != 2 || !READINT16 (in, buf, type))
					return false;
				break;
			case kCDXProp_Arrow_Type:
				if (size != 2 || !READINT16 (in, buf, arrow_type))
					return false;
				break;
			default:
				if (size && !gsf_input_read (in, size, (guint8*) buf))
					return false;
			}
		}
		if (!READINT16 (in, buf, code))
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
	if (!READINT16 (in, buf, size))
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
		if (!READINT16 (in, buf, n[i]))
			return false;
	GDate *date = g_date_new_dmy (n[2], (GDateMonth) n[1], n[0]);
	g_date_strftime (buf, bufsize, "%m/%d/%Y", date);
	g_date_free (date);
	return true;
}

bool CDXLoader::ReadFragmentText (GsfInput *in, Object *parent)
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
			switch (code) {
			case kCDXProp_2DPosition: {
				if (size != 8)
					return false;
				gint32 x, y;
				if (!READINT32 (in, buf, y))
					return false;
				if (!READINT32 (in, buf, x))
					return false;
				snprintf (buf, bufsize, "%d %d", x, y);
//				Text->SetProperty (GCU_PROP_POS2D, buf);
				break;
			}
			case kCDXProp_Text: {
				guint16 nb;
				if (!READINT16 (in, buf, nb))
					return false;
				size -=2;
				for (int i =0; i < nb; i++) {
					if (size < 10)
						return false;
					guint16 n[5];
					for (int j = 0; j < 5; j++)
						if (!READINT16 (in, buf, n[j]))
							return false;
					size -= 10;
				}
				if (size < 1)
					return false;
				if (!gsf_input_read (in, size, (guint8*) buf))
					return false;
				buf[size] = 0;
//				Text->SetProperty (GCU_PROP_TEXT_TEXT, buf);
				break;
			}
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
go_plugin_shutdown (GOPlugin *plugin, GOCmdContext *cc)
{
}

}
