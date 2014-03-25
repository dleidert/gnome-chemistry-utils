// -*- C++ -*-

/*
 * CTfiles loader plugin
 * ctfiles.cc
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/document.h>
#include <gcu/loader.h>
#include <gcu/molecule.h>
#include <gcu/objprops.h>
#include <goffice/app/module-plugin-defs.h>
#include <gsf/gsf-input-textline.h>
#include <gsf/gsf-output.h>
#include <glib/gi18n-lib.h>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

typedef enum {
	MOLfile, // actually might be MOL, SDfile or RGroup
	RXNfile,
	RGfile,
	SDfile,
	RDfile
} CTfileType;

typedef struct {
	gcu::Document *doc;
	gcu::Application *app;
	GOIOContext *context;
	GsfInputTextline *input;
	std::stack < gcu::Object * > cur;
	gcu::ContentType type;
	bool v3000;
	unsigned na, nb, nsg, n3d;
	bool chiral;
	CTfileType cttype;
	std::vector < gcu::Object * > atoms;
} CTReadState;

class CTfilesLoader;
typedef struct {
	CTfilesLoader *loader;
	GsfOutput *out;
	GOIOContext *io;
	gcu::ContentType type;
	std::map < gcu::Object const * , unsigned> indices;
	unsigned cur;
} CTWriteState;

////////////////////////////////////////////////////////////////////////////////
// CTfilesLoader definition

class CTfilesLoader: public gcu::Loader
{
public:
	CTfilesLoader ();
	virtual ~CTfilesLoader ();

	gcu::ContentType Read (gcu::Document *doc, GsfInput *in, char const *mime_type, GOIOContext *io);
	bool Write (gcu::Object const *obj, GsfOutput *out, char const *mime_type, GOIOContext *io, gcu::ContentType type);

	bool WriteObject (CTWriteState *state, gcu::Object const *object);

private:
	std::map <std::string, bool (*) (CTWriteState *, gcu::Object const *)> m_WriteCallbacks;
	bool ReadHeader (CTReadState *state);
	bool ReadCounts (CTReadState *state, char const *source);
	bool ReadMolecule (CTReadState *state);
	bool ReadAtom (CTReadState *state, unsigned i);
	bool ReadBond (CTReadState *state);
};

////////////////////////////////////////////////////////////////////////////////
// Write callbacks

bool ct_write_atom (CTWriteState *state, gcu::Object const *object)
{
	return true;
}

bool ct_write_fragment (CTWriteState *state, gcu::Object const *object)
{
	return true;
}

bool ct_write_bond (CTWriteState *state, gcu::Object const *object)
{
	return true;
}

bool ct_write_molecule (CTWriteState *state, gcu::Object const *object)
{
	std::map < std::string, gcu::Object * >::const_iterator i;
	gcu::Object const *child = object->GetFirstChild (i);
	std::list < gcu::Object const * > bonds, fragments;
	while (child) {
		switch (child->GetType ()) {
		case gcu::FragmentType:
			fragments.push_back (child); 
		case gcu::AtomType:
			state->loader->WriteObject (state, child);
			state->indices[child] = ++state->cur;
			break;
		case gcu::BondType:
			bonds.push_back (child);
			break;
		default:
			break;
		}
		child = object->GetNextChild (i);
	}
	// now save bonds
	if (bonds.size () > 0) {
		std::list < gcu::Object const * >::iterator it, end = bonds.end ();
		for (it = bonds.begin (); it != end; ++it)
			state->loader->WriteObject (state, *it);
	}
	state->cur = 0;
	state->indices.clear ();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// CTfilesLoader implementation

CTfilesLoader::CTfilesLoader ()
{
	AddMimeType ("chemical/x-mdl-molfile");
	m_WriteCallbacks["molecule"] = ct_write_molecule;
	m_WriteCallbacks["atom"] = ct_write_molecule;
	m_WriteCallbacks["fragment"] = ct_write_molecule;
	m_WriteCallbacks["bond"] = ct_write_molecule;
}

CTfilesLoader::~CTfilesLoader ()
{
	RemoveMimeType ("chemical/x-mdl-molfile");
}

////////////////////////////////////////////////////////////////////////////////
// Reading code

bool CTfilesLoader::ReadHeader (CTReadState *state)
{
	char *buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (state->input));
	if (!strncmp (buf, "$RDFILE", 6)) {
		// RDfile
		state->cttype = RDfile;
	} else if (!strncmp (buf, "$RXN", 4)) {
		// RXN file
		state->cttype = RXNfile;
	} else if (!strncmp (buf, "$MDL", 4)) {
		// V2000 RGroup
		state->cttype = RGfile;
	} else {
		// either MOL SDfile or RGroup
		state->cttype = MOLfile;
		state->doc->SetTitle (buf); // note that this is wrong if we have more than one molecule
		// line 2
		buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (state->input));
		state->type = (strncmp (buf + 20, "3D", 2))? gcu::ContentType2D: gcu::ContentType3D;
		// FIXME, we might retrieve author initials and date from there if available
		// line 3
		buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (state->input));
		state->doc->SetComment (buf);
		// line 4
		buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (state->input));
		if (strlen (buf) < 37) {
			// FIXME: send an error message
			return false;
		}
		char *ver = buf + 33;
		while (*ver == ' ')
			ver++;
		if (!strncmp (ver, "V3000", 5))
			state->v3000 = true;
		else if (strncmp (ver, "V2000", 5)) {
			// unknown version
			// FIXME: send an error message
			return false;
		}
		if (!state->v3000)
			if (!ReadCounts (state, buf)) { // we know that we have enough characters in the buffer
				// FIXME: send an error message
				return false;
			}
	}
	return true;
}

bool CTfilesLoader::ReadCounts (CTReadState *state, char const *source)
{
	// oly applies to V2000
	char buf[4], *end;
	buf[3] = 0;
	strncpy (buf, source, 3);
	state->na = strtoul (buf, &end, 10);
	if (*end != 0) {
		return false;
	}
	strncpy (buf, source + 3, 3);
	state->nb = strtoul (buf, &end, 10);
	if (*end != 0) {
		return false;
	}
	strncpy (buf, source + 6, 3);
	unsigned atom_lists = atoi (buf);
	if (atom_lists != 0) {
		// Not sure we are able to support atoms lists at all, but we might
		go_io_warning (state->context, _("We do not support atoms lists for now, please file a bug report and attach your file"));
		return false;
	}
	strncpy (buf, source + 12, 3);
	state->chiral = strtoul (buf, &end, 10) != 0;
	if (*end != 0) {
		return false;
	}
	// ignore stext for now, seems it is not fully supported
	return true;
}

bool CTfilesLoader::ReadMolecule (CTReadState *state)
{
	char *buf;
	unsigned i;
	// first if V3000, we need to read some extra lines
	if (state->v3000) {
		buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (state->input));
		if (strncmp (buf, "M  V30 BEGIN CTAB", 17)) {
			// FIXME: send an error message
			return false;
		}
		// FIXME: what should we do with the name if any?
		// counts line
		buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (state->input));
		if (strncmp (buf, "M  V30 COUNTS", 13)) {
			// FIXME: send an error message
			return false;
		}
		unsigned chiral;
		if (sscanf (buf + 13, "%u %u %u %u %u\n", &state->na, &state->nb, &state->nsg, &state->n3d, &chiral) != 5) {
			// FIXME: send an error message
			return false;
		}
		state->chiral = chiral != 0;
	}
	// we can now create the molecule and push it on the stack
	gcu::Object *molecule = state->app->CreateObject ("molecule", state->cur.top ());
	state->cur.push (molecule);
	state->atoms.resize (state->na, NULL);
	// now read atoms
	for (i = 0; i < state->na; i++)
		if (!ReadAtom (state, i))
			return false;
	state->doc->EmptyTranslationTable();
	for (i = 0; i < state->nb; i++)
		if (!ReadBond (state))
			return false;
	// and now properties if any
	state->cur.pop ();
	return true;
}

bool CTfilesLoader::ReadAtom (CTReadState *state, unsigned i)
{
	char *buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (state->input));
	gcu::Object *atom = NULL;
	if (state->v3000) {
	} else {
		char coord[11], *end;
		coord[10] = 0;
		double x;
		if (strlen (buf) < 69) {
			// FIXME: send an error message
			return false;
		}
		// first the atom symbo
		strncpy (coord, buf + 31, 3);
		coord[3] = 0;
		if (coord[1] == ' ')
			coord[1] = 0;
		else if (coord[2] == ' ')
			coord[2] = 0;
		if (!strcmp (coord, "R#")) {
			// Create an R group
			atom = state->app->CreateObject ("fragment", state->cur.top ());
			atom->SetProperty (GCU_PROP_TEXT_TEXT, "R"); // FIXME
			atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_START, "0");
		} else if (!strcmp (coord, "L")) {
			// ??
			return false;
		} else if (!strcmp (coord, "A")) {
			// ??
			return false;
		} else if (!strcmp (coord, "Q")) {
			// ??
			return false;
		} else if (!strcmp (coord, "*")) {
			// ??
			return false;
		} else if (!strcmp (coord, "LP")) {
			// ??
			return false;
		} else {
			atom = state->app->CreateObject ("atom", state->cur.top ());
			atom->SetProperty (GCU_PROP_ATOM_SYMBOL, coord);
		}
		strncpy (coord, buf, 10);
		x = g_ascii_strtod (coord, &end);
		if (*end != ' ' && *end != 0) {
			// FIXME: send an error message
			return false;
		}
		atom->SetProperty (GCU_PROP_X, coord);
		strncpy (coord, buf + 10, 10);
		x = -g_ascii_strtod (coord, &end);
		if (*end != ' ' && *end != 0) {
			// FIXME: send an error message
			return false;
		}
		std::ostringstream res;
		res << x;
		atom->SetProperty (GCU_PROP_Y, res.str ().c_str ());
		strncpy (coord, buf + 20, 10);
		x = g_ascii_strtod (coord, &end);
		if (*end != ' ' && *end != 0) {
			// FIXME: send an error message
			return false;
		}
		if (x != 0.) {
			state->type = gcu::ContentType3D;
			atom->SetProperty (GCU_PROP_Z, coord);
		}
	}
	state->atoms[i] = atom;
	return true;
}

bool CTfilesLoader::ReadBond (CTReadState *state)
{
	char *buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (state->input));
	gcu::Object *bond = state->app->CreateObject ("bond", state->cur.top ());
	unsigned i;
	std::string id;
	if (state->v3000) {
		return false; // FIXME
	} else {
		char field[4], *end;
		field[3] = 0;
		// first atom
		strncpy (field, buf, 3);
		i = strtoul (field, &end, 10) - 1;
		if (*end != 0) {
			return false;
		}
		id = state->atoms[i]->GetProperty (GCU_PROP_FRAGMENT_ATOM_ID);
		if (id.length () == 0)
			id = state->atoms[i]->GetProperty (GCU_PROP_ID);
		bond->SetProperty (GCU_PROP_BOND_BEGIN, id.c_str ());
		// first atom
		strncpy (field, buf + 3, 3);
		i = strtoul (field, &end, 10) - 1;
		if (*end != 0) {
			return false;
		}
		id = state->atoms[i]->GetProperty (GCU_PROP_FRAGMENT_ATOM_ID);
		if (id.length () == 0)
			id = state->atoms[i]->GetProperty (GCU_PROP_ID);
		bond->SetProperty (GCU_PROP_BOND_END, id.c_str ());
		strncpy (field, buf + 6, 3);
		bond->SetProperty (GCU_PROP_BOND_ORDER, field);
		strncpy (field, buf + 9, 3);
		i = strtoul (field, &end, 10) - 1;
		if (*end != 0) {
			return false;
		}
		switch (i) {
		default:
		case 0:
			bond->SetProperty (GCU_PROP_BOND_TYPE, "normal");
			break;
		case 1:
			bond->SetProperty (GCU_PROP_BOND_TYPE, "wedge");
			break;
		case 4:
			bond->SetProperty (GCU_PROP_BOND_TYPE, "unkown");
			break;
		case 6:
			bond->SetProperty (GCU_PROP_BOND_TYPE, "hash");
			break;
		}
	}
	return true;
}

gcu::ContentType CTfilesLoader::Read  (G_GNUC_UNUSED gcu::Document *doc, G_GNUC_UNUSED GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io)
{
	CTReadState state;
	state.input = reinterpret_cast <GsfInputTextline *> (gsf_input_textline_new (in));
	doc->SetScale (100.);
	// read the first line
	// initialize the state
	state.doc = doc;
	state.app = doc->GetApplication ();
	state.context = io;
	state.cur.push (doc);
	state.v3000 = false;
	state.type = gcu::ContentTypeUnknown; // may be 2D
	if (!ReadHeader (&state))
		return gcu::ContentTypeUnknown;
	switch (state.cttype) {
	case MOLfile:
	case RGfile:
	case SDfile:
		if (!ReadMolecule (&state))
			return gcu::ContentTypeUnknown;
		break;
	case RXNfile:
		break;
	case RDfile:
		break;
	}
	return state.type;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

bool CTfilesLoader::WriteObject (CTWriteState *state, gcu::Object const *object)
{
	std::string name = gcu::Object::GetTypeName (object->GetType ());
	std::map < std::string, bool (*) (CTWriteState *, gcu::Object const *) >::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (state, object);
	// if we don't save the object iself, try to save its children
	std::map <std::string, gcu::Object *>::const_iterator j;
	gcu::Object const *child = object->GetFirstChild (j);
	while (child) {
		if (!WriteObject (state, child))
			return false;
		child = object->GetNextChild (j);
	}
	return true; /* loosing data is not considered an error, it is just a missing feature
					either in this code or in the cml schema */
}

bool CTfilesLoader::Write  (gcu::Object const *obj, GsfOutput *out, char const *, GOIOContext *ctxt, gcu::ContentType type)
{
	if (NULL != out) {
		gcu::Document const *doc = dynamic_cast <gcu::Document const *> (obj);
		CTWriteState state;
		state.loader = this;
		state.out = out;
		state.io = ctxt;
		state.type = type;
		state.cur = 0;
		if (!doc)
			doc = obj->GetDocument ();
		if (obj->GetType () == gcu::MoleculeType) {
			gcu::Molecule const *mol = static_cast < gcu::Molecule const * > (obj);
			// we don't use the three first lines at least for now
			gsf_output_write (out, 3, reinterpret_cast < guint8 const * > ("\n\n\n"));
			// only support V3000 on export
			char buf[] = "  0  0  0     0  0           999 V3000\n";
			gsf_output_write (out, strlen (buf), reinterpret_cast < guint8 const * > (buf));
			WriteObject (&state, obj);
			gsf_output_write (out, 6, reinterpret_cast < guint8 const * > ("M END\n"));
		} else
			return false;
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// Initialization

static CTfilesLoader loader;

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
