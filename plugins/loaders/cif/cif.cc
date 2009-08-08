// -*- C++ -*-

/* 
 * CIF files loader plugin
 * cif.cc 
 *
 * Copyright (C) 2008-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/element.h>
#include <gcu/objprops.h>
#include <gcu/spacegroup.h>
#include <gcu/transform3d.h>
#include <goffice/app/module-plugin-defs.h>
#include <gsf/gsf-input-textline.h>
#include <glib/gi18n-lib.h>
#include <map>
#include <stack>
#include <string>
#include <libintl.h>

using namespace gcu;
using namespace std;

static map<string, unsigned> KnownProps;

class CIFLoader: public Loader
{
public:
	CIFLoader ();
	virtual ~CIFLoader ();

	ContentType Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Object *obj, GsfOutput *out, char const *mime_type, IOContext *io, ContentType type);
};

enum { // local only properties
	CIF_ATOM_SITE_SYMBOL = GCU_PROP_MAX + 1,
	CIF_ATOM_SITE_OXIDATION_NUMBER,
	CIF_ATOM_LABEL
};

CIFLoader::CIFLoader ()
{
	AddMimeType ("chemical/x-cif");

	KnownProps["_publ_contact_author_name"] = GCU_PROP_DOC_CREATOR;
	KnownProps["_publ_author_name"] = GCU_PROP_DOC_CREATOR;
	KnownProps["_publ_contact_author_email"] = GCU_PROP_DOC_CREATOR_EMAIL;
	KnownProps["_publ_author_email"] = GCU_PROP_DOC_CREATOR_EMAIL;

	KnownProps["_cell_length_a"] = GCU_PROP_CELL_A;
	KnownProps["_cell_length_b"] = GCU_PROP_CELL_B;
	KnownProps["_cell_length_c"] = GCU_PROP_CELL_C;
	KnownProps["_cell_angle_apha"] = GCU_PROP_CELL_ALPHA;
	KnownProps["_cell_angle_beta"] = GCU_PROP_CELL_BETA;
	KnownProps["_cell_angle_gamma"] = GCU_PROP_CELL_GAMMA;

	KnownProps["_chemical_name_common"] = GCU_PROP_CHEMICAL_NAME_COMMON;
	KnownProps["_chemical_name_systematic"] = GCU_PROP_CHEMICAL_NAME_SYSTEMATIC;
	KnownProps["_chemical_name_mineral"] = GCU_PROP_CHEMICAL_NAME_COMMON;
	KnownProps["_chemical_name_structure_type"] = GCU_PROP_CHEMICAL_NAME_STRUCTURE;

	KnownProps["_atom_type_symbol"] = CIF_ATOM_SITE_SYMBOL;
	KnownProps["_atom_type_oxidation_number"] = CIF_ATOM_SITE_OXIDATION_NUMBER;
	KnownProps["_atom_site_type_symbol"] = CIF_ATOM_SITE_SYMBOL;
	KnownProps["_atom_site_label"] = CIF_ATOM_LABEL;
	KnownProps["_atom_site_fract_x"] = GCU_PROP_XFRACT;
	KnownProps["_atom_site_fract_y"] = GCU_PROP_YFRACT;
	KnownProps["_atom_site_fract_z"] = GCU_PROP_ZFRACT;
}

CIFLoader::~CIFLoader ()
{
	RemoveMimeType ("chemical/x-cif");
}
////////////////////////////////////////////////////////////////////////////////
// Reading code

enum {
	LOOP_UNKNOWN,
	LOOP_ATOM,
	LOOP_ATOM_TYPE,
	LOOP_SYMMETRY,
	LOOP_AUTHOR
};

typedef struct {
	string elt;
	string charge;
} CIFAtomType;

ContentType CIFLoader::Read  (Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED IOContext *io)
{
	ContentType type = ContentTypeCrystal;
	GsfInputTextline *input = reinterpret_cast <GsfInputTextline *> (gsf_input_textline_new (in));
	char *buf;
	bool in_string = false, in_loop = false, waiting_value = false;
	string key, value;
	int size;
	char endstr = 0;
	map <string, unsigned>::iterator prop;
	unsigned loop_type = LOOP_UNKNOWN;
	list <unsigned> loop_contents;
	list <unsigned>::iterator loop_prop;
	list <string> loop_values;
	map <string, CIFAtomType> AtomTypes;
	SpaceGroup *group = new SpaceGroup ();
	bool author_found = false;
	doc->SetScale (100.); // lentghs and positions pus be converted to pm
	while ((buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (input)))) {
		char *cur = buf, *next;
		size = strlen (buf);
		// check for new data bloc
		if (!strncmp (cur, "data_", 5)) {
			if (!doc->GetEmpty ())
				// FIXME: implement multiple data blocs reading
				goto read_exit;
		}
		if (!strncmp (cur, "save_", 5)) // FIXME: implement
			continue;
		while (*cur == ' ')
			cur++;
		if (in_string) {
			next = strchr (cur, endstr);
			if (next) {
				*next = 0;
				if (value.length () > 0 && strlen (cur) > 0)
					value += '\n';
				value += cur;
				in_string = false;
				prop = KnownProps.find (key);
				if (prop != KnownProps.end ())
					doc->SetProperty ((*prop).second, value.c_str ());
				// unkown data are discarded
				waiting_value = false;
			} else {
				if (value.length () > 0 && strlen (cur) > 0)
					value += '\n';
				value += cur;
			}
		} else {
			if (*cur == '#')
				continue;
			// skip unsupported global words
			if (!strcmp (cur, "stop_") || !strcmp (cur, "global_"))
			    continue;
			// now read the keyword
			if (in_loop && *cur != '_') {
				waiting_value = true;
				loop_prop = loop_contents.begin ();
				if (loop_prop == loop_contents.end ()) {
					GtkWidget *w = gtk_message_dialog_new (doc->GetGtkWindow (), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Invalid loop in data."));
					g_signal_connect (G_OBJECT (w), "response", G_CALLBACK (gtk_widget_destroy), NULL);
					gtk_widget_show_all (w);
				}
			}
			if (in_loop && waiting_value) {
				if (*cur == '_' || !strncmp (cur, "loop_", 5)) { // FIXME: implement nested loops
					waiting_value = false;
					in_loop = false;
				}
			}
			if (!waiting_value) {
				next = cur;
				while (*next && *next != ' ')
					next++;
					
				*next = 0;
				key = cur;
				cur = next + 1;
				if (key[0] == '_') {
					if (in_loop) {
						prop = KnownProps.find (key);
						loop_contents.push_back ((prop == KnownProps.end ())? static_cast <unsigned> (GCU_PROP_MAX): (*prop).second);
						if (loop_type == LOOP_UNKNOWN) {
							if (!key.compare (0, 11, "_atom_type_", 11))
								loop_type = LOOP_ATOM_TYPE;
							else if (!key.compare (0, 11, "_atom_site_", 11))
								loop_type = LOOP_ATOM;
							else if (!key.compare (0, 13, "_publ_author_",13))
								loop_type = LOOP_AUTHOR;
							else if (key == "_symmetry_equiv_pos_as_xyz")
								loop_type = LOOP_SYMMETRY;
						}
					} else {
						waiting_value = true;
						value.clear ();
					}
				} else {
					if (key == "loop_") {
						loop_type = LOOP_UNKNOWN;
						loop_contents.clear ();
						in_loop = true;
					}
					continue;
				}
				if (cur - buf > size)
					continue;
			}
			// read the value(s)
			if (in_loop) {
				if (loop_type == LOOP_UNKNOWN)
					continue; // FIXME: this should not happen
				if (*cur == 0) {
					in_loop = false;
				} else {
					while (*cur) {
						while (*cur == ' ')
							cur++;
						if (*cur == '\'' || *cur == '"' || *cur == ';') {
							endstr = *cur;
							cur++;
							next = strchr (cur, endstr);
							if (next) {
								endstr = *next;
								*next = 0;
							} else
								// FIXME: does this happen? at the moment just exit the loop to avoid an infinite loop condition
								break;
						} else {
							next = cur;
							while (*next && *next != ' ')
								next++;
							endstr = *next;
							*next = 0;
						}
						value = cur;
						*next = endstr;
						cur = next;
						loop_prop++;
						loop_values.push_back (value);
						if (loop_prop == loop_contents.end ()) {
							// store the values
							switch (loop_type) {
							case LOOP_ATOM: {
								double scale = doc->GetScale ();
								doc->SetScale (1.);
								Object *atom = Object::CreateObject ("atom", doc);
								for (loop_prop = loop_contents.begin (); loop_prop != loop_contents.end (); loop_prop++) {
									std::string val = loop_values.front ();
									loop_values.pop_front ();
									switch (*loop_prop) {
									case CIF_ATOM_LABEL: {
										if (AtomTypes.empty ()) {
											int i = 0;
											while (g_ascii_isalpha (val[i]))
												i++;
											val = val.substr (0, i);
											atom->SetProperty (GCU_PROP_ATOM_SYMBOL, val.c_str ());
										}
										break;
									}
									case CIF_ATOM_SITE_SYMBOL: {
										CIFAtomType t = AtomTypes[val];
										atom->SetProperty (GCU_PROP_ATOM_Z, t.elt.c_str ());
										atom->SetProperty (GCU_PROP_ATOM_CHARGE, t.charge.c_str ());
										break;
									}
									default:
										if (*loop_prop != GCU_PROP_MAX)
											atom->SetProperty (*loop_prop, val.c_str ());
										break;
									}
								}
								doc->SetScale (scale);
								break;
							}
							case LOOP_ATOM_TYPE: {
								CIFAtomType t = {"", ""}; // make gcc happy
								std::string ident;
								for (loop_prop = loop_contents.begin (); loop_prop != loop_contents.end (); loop_prop++) {
									std::string val = loop_values.front ();
									loop_values.pop_front ();
									switch (*loop_prop) {
									case CIF_ATOM_SITE_SYMBOL: {
										ident = val;
										int i = 0;
										while (g_ascii_isalpha (val[i]))
											i++;
										val = val.substr (0, i);
										int z = Element::Z (val.c_str ());
										char *buf = g_strdup_printf ("%d", z);
										t.elt = buf;
										g_free (buf);
										break;
									}
									case CIF_ATOM_SITE_OXIDATION_NUMBER:
										t.charge = val;
										break;
      								default:
										break;
									}
								}
								AtomTypes[ident] = t;
								break;
							}
							case LOOP_AUTHOR: { // FIXME: support several authors
								if (author_found)
									break;
								author_found = true;
								for (loop_prop = loop_contents.begin (); loop_prop != loop_contents.end (); loop_prop++) {
									std::string val = loop_values.front ();
									loop_values.pop_front ();
									if (*loop_prop < GCU_PROP_MAX)
										doc->SetProperty (*loop_prop, val.c_str ());
								}
								break;
							}
							case LOOP_SYMMETRY: {
								group->AddTransform (loop_values.front ());
								break;
							}
							}
							loop_prop = loop_contents.begin ();
							loop_values.clear ();
						}
					}
				}
				continue;
			}
			while (*cur == ' ')
				cur++;
			if (!*cur)
				continue;
			if (*cur == '\'' || *cur == '"' || *cur == ';') {
				endstr = *cur;
				cur++;
				next = strchr (cur, endstr);
				if (next) {
					*next = 0;
					value = cur;
				} else {
					value = cur;
					in_string = true;
					continue;
				}
			} else {
				next = cur;
				while (*next && *next != ' ')
					next++;
				*next = 0;
				value = cur;
			}
			// check for a symetry property
			if (key == "_symmetry_space_group_name_H-M")
				group->SetHMName (value);
			else if (key == "_symmetry_space_group_name_Hall")
				group->SetHallName (value);
			else if (key == "_symmetry_Int_Tables_number")
				group->SetId (strtoul (value.c_str (), NULL, 10));
			else {
				// check if this concerns the author
				if (!author_found && (!key.compare (0, 13, "_publ_author_", 13) || !key.compare (0, 13, "_publ_contact_author_", 13)))
					author_found = true; // we don't allow several authors for now
				// otherwise set the property
				prop = KnownProps.find (key);
				if (prop != KnownProps.end ())
					doc->SetProperty ((*prop).second, value.c_str ());
				// unkown properties are discarded
			}
			waiting_value = false;
			continue;
		}
	}
	// check ymmetry
	if (!group->IsValid ()) {
		GtkWidget *w = gtk_message_dialog_new (doc->GetGtkWindow (), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Invalid symmetry group."));
		g_signal_connect (G_OBJECT (w), "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show_all (w);
	} else {
		SpaceGroup const *sp = SpaceGroup::Find (group);
		if (sp)
			doc->SetProperty (GCU_PROP_SPACE_GROUP, sp->GetHallName ().c_str ());
	}
read_exit:
	delete group;
	g_object_unref (input);
	return type;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

static void WriteStringField (GsfOutput *out, char const *propname, std::string &prop)
{
	if (!g_utf8_validate (prop.c_str (), -1, NULL)) // FIXME: try some conversion
		return;
	char const *separator = (strstr (prop.c_str (), "' "))? "\"": "'";
	std::string str = std::string (propname) + std::string (35 - strlen (propname), ' ') + separator + prop + separator + "\n";
	gsf_output_write (out, str.length (), reinterpret_cast <guint8 const *> (str.c_str ()));
}

typedef struct
{
	string symbol;
	string charge;
} Atom_Type;

typedef struct
{
	string symbol;
	string label;
	string XFract, YFract, ZFract; 
} AtomInstance;

bool CIFLoader::Write  (G_GNUC_UNUSED Object *obj, GsfOutput *out, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED IOContext *io, G_GNUC_UNUSED ContentType type)
{
	std::string prop;
	unsigned i;
	if (NULL != out) {
		Document *doc = dynamic_cast <Document *> (obj);
		if (!doc)
			doc = obj->GetDocument ();
		if (doc)
			doc->SetScale (100);
		prop = obj->GetProperty (GCU_PROP_DOC_TITLE);
		if (prop.length () == 0)
			prop = "0";
		for (i = 0; i < prop.length (); i++)
			if (!g_ascii_isalnum (prop[i]))
				prop[i] = '_';
		gsf_output_write (out, 5, reinterpret_cast <guint8 const *> ("data_"));
		gsf_output_write (out, prop.length (), reinterpret_cast <guint8 const *> (prop.c_str ()));
		gsf_output_write (out, 1, reinterpret_cast <guint8 const *> ("\n"));
		prop = obj->GetProperty (GCU_PROP_CHEMICAL_NAME_COMMON);
		if (prop.length ())
			WriteStringField (out, "_chemical_name_common", prop);
		prop = obj->GetProperty (GCU_PROP_CHEMICAL_NAME_SYSTEMATIC);
		if (prop.length ())
			WriteStringField (out, "_chemical_name_systematic", prop);
		prop = obj->GetProperty (GCU_PROP_CHEMICAL_NAME_MINERAL);
		if (prop.length ())
			WriteStringField (out, "_chemical_name_mineral", prop);
		prop = obj->GetProperty (GCU_PROP_DOC_CREATOR);
		if (prop.length ())
			WriteStringField (out, "_publ_author_name", prop);
		prop = obj->GetProperty (GCU_PROP_DOC_CREATOR_EMAIL);
		if (prop.length ())
			WriteStringField (out, "_publ_author_email", prop);
		// export space group
		prop = obj->GetProperty (GCU_PROP_SPACE_GROUP);
		if (prop.length ()) {
			WriteStringField (out, "_symmetry_space_group_name_Hall", prop);
			gsf_output_write (out, 6, reinterpret_cast <guint8 const *> ("loop_\n"));
			gsf_output_write (out, 27, reinterpret_cast <guint8 const *> ("_symmetry_equiv_pos_as_xyz\n"));
			std::list <Transform3d*>::const_iterator t;
			SpaceGroup const *group = SpaceGroup::GetSpaceGroup (prop);
			Transform3d const *tr = group->GetFirstTransform (t);
			while (tr) {
				prop = string ("  '") + tr->DescribeAsString() + "'\n";
				gsf_output_write (out, prop.length (), reinterpret_cast <guint8 const *> (prop.c_str ()));
				tr = group->GetNextTransform (t);
			}
		}
		// iterate through the children and prepare atoms export
		map <string, Object *>::iterator i;
		Object *child = doc->GetFirstChild (i);
		map <string, Atom_Type> AtomTypes;
		list <AtomInstance> AtomInstances;
		map <int, int> types;
		map <int, int> indices;
		map <string, Atom_Type>::iterator t, tend;
		char *buf;
		while (child) {
			// FIXME, using gcu::Atom is not conform to the spirit of the loader mechanism
			if (child->GetType () == AtomType) {
				string prop = child->GetProperty (GCU_PROP_ATOM_Z);
				int Z = atoi (prop.c_str ());
				int index = indices[Z] + 1;
				string symbol = (Z > 0)? child->GetProperty (GCU_PROP_ATOM_SYMBOL): "Xx";
				indices[Z] = index;
				AtomInstance instance;
				string charge = child->GetProperty (GCU_PROP_ATOM_CHARGE);
				string id = symbol + charge;
				buf = g_strdup_printf ("%d", index);
				instance.label = symbol + buf;
				g_free (buf);
				t = AtomTypes.find (id);
				if (t != AtomTypes.end ())
					instance.symbol = (*t).second.symbol;
				else {
					Atom_Type type;
					index = types[Z];
					types[Z] = index + 1;
					buf = g_strdup_printf ("%d", index);
					type.symbol = symbol + buf;
					g_free (buf);
					type.charge = charge;
					AtomTypes[id] = type;
					instance.symbol = type.symbol;
				}
				instance.XFract = child->GetProperty (GCU_PROP_XFRACT);
				instance.YFract = child->GetProperty (GCU_PROP_YFRACT);
				instance.ZFract = child->GetProperty (GCU_PROP_ZFRACT);
				AtomInstances.push_back (instance);
			}
			child = doc->GetNextChild (i);
		}
		// export atom types
		gsf_output_write (out, 6, reinterpret_cast <guint8 const *> ("loop_\n"));
		gsf_output_write (out, 18, reinterpret_cast <guint8 const *> ("_atom_type_symbol\n"));
		gsf_output_write (out, 28, reinterpret_cast <guint8 const *> ("_atom_type_oxidation_number\n"));
		tend = AtomTypes.end ();
		for (t = AtomTypes.begin (); t != tend; t++) {
			prop = string ("  ") + (*t).second.symbol + string (MAX (8 - static_cast <int> ((*t).second.symbol.length ()), 1), ' ') + (*t).second.charge + "\n";
			gsf_output_write (out, prop.length (), reinterpret_cast <guint8 const *> (prop.c_str ()));
		}
		//export atoms
		
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// Initialization

static CIFLoader loader;

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
