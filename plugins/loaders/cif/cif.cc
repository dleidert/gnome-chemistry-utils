// -*- C++ -*-

/* 
 * CIF files loader plugin
 * cif.cc 
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/spacegroup.h>
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

CIFLoader::CIFLoader ()
{
	AddMimeType ("chemical/x-cif");
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
	SpaceGroup *group = new SpaceGroup ();
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
							if (!key.compare (0, 10, "_atom_type_"))
								loop_type = LOOP_ATOM_TYPE;
							else if (!key.compare (0, 10, "_atom_site_"))
								loop_type = LOOP_ATOM;
							else if (!key.compare (0, 10, "_publ_author_"))
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
							case LOOP_ATOM:
								break;
							case LOOP_ATOM_TYPE:
								break;
							case LOOP_AUTHOR:
								break;
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
		if (group)
			doc->SetProperty (GCU_PROP_SPACE_GROUP, sp->GetHallName ().c_str ());
	}
read_exit:
	delete group;
	g_object_unref (input);
	return type;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

bool CIFLoader::Write  (Object *obj, GsfOutput *out, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED IOContext *io, G_GNUC_UNUSED ContentType type)
{
	if (NULL != out) {
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
