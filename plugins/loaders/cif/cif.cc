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

ContentType CIFLoader::Read  (Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED IOContext *io)
{
	ContentType type = ContentTypeCrystal;
	GsfInputTextline *input = reinterpret_cast <GsfInputTextline *> (gsf_input_textline_new (in));
	char *buf;
	bool in_string = false, in_loop = false, waiting_value = false;
	string key, value;
	int size;
	char endstr;
	map <string, unsigned>::iterator prop;
	list <unsigned> loop_contents;
	list <unsigned>::iterator loop_prop;
	doc->SetScale (100.); // lentghs and positions pus be converted to pm
	while ((buf = reinterpret_cast <char *> (gsf_input_textline_utf8_gets (input)))) {
		char *cur = buf, *next;
		size = strlen (buf);
		// check for new data bloc
		if (!strncmp (cur, "data_", 5)) {
			if (!doc->GetEmpty ()) {
				// FIXME: implement multiple data blocs reading
				g_object_unref (input);
				return type;
			}
		}
		if (!strncmp (cur, "save_", 5)) // FIXME: implement
			continue;
		while (*cur == ' ')
			cur++;
		if (in_string) {
		} else {
			if (*cur == '#')
				continue;
			// skip unsupported global words
			if (!strcmp (cur, "stop_") || !strcmp (cur, "global_"))
			    continue;
			// now read the keyword
			if (in_loop && *cur != '_')
				waiting_value = true;
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
					} else {
						waiting_value = true;
						value.clear ();
					}
				} else {
					if (key == "loop_")
						in_loop = true;
				}
				if (cur - buf > size)
					continue;
			}
			if (in_loop) {
				if (*cur == 0) {
					in_loop = false;
				} else {
				}
				continue;
			}
			if (in_string) {
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
					value += cur;
					value += "\n";
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
			// read the value
			prop = KnownProps.find (key);
			if (prop != KnownProps.end ())
				doc->SetProperty ((*prop).second, value.c_str ());
			// unkown data are discarded
			waiting_value = false;
		}
	}
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
