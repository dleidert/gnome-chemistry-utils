// -*- C++ -*-

/* 
 * CML files loader plugin
 * cml.cc 
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
#include <gsf/gsf-libxml.h>
#include <glib/gi18n-lib.h>
#include <map>
#include <stack>
#include <string>
#include <libintl.h>

using namespace gcu;
using namespace std;

static map<string, unsigned> KnownProps;

typedef struct {
	Document *doc;
	IOContext *context;
	stack<Object*> cur;
} CMLReadState;

class CMLLoader: public Loader
{
public:
	CMLLoader ();
	virtual ~CMLLoader ();

	bool Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io);
};

CMLLoader::CMLLoader ()
{
	AddMimeType ("chemical/x-cml");
	KnownProps["title"] = GCU_PROP_DOC_TITLE;
}

CMLLoader::~CMLLoader ()
{
	RemoveMimeType ("chemical/x-cml");
}
////////////////////////////////////////////////////////////////////////////////
// Reading code

static void
cml_simple_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	state->cur.top ()->Lock (false);
	state->cur.top ()->OnLoaded ();
	state->cur.pop ();
}

static void
cml_doc (GsfXMLIn *xin, xmlChar const **attrs)
{
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	map<string, unsigned>::iterator it;
	while (*attrs) {
		if ((it = KnownProps.find ((char const *) *attrs++)) != KnownProps.end ()) {
			state->doc->SetProperty ((*it).second, (char const *) *attrs);}
		attrs++;
	}
	state->cur.push (state->doc);
}

////////////////////////////////////////////////////////////////////////////////
// Molecule code

static void
cml_mol_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("molecule", state->cur.top ());
	state->cur.push (obj);
}

////////////////////////////////////////////////////////////////////////////////
// Reading code
static GsfXMLInNode const cml_dtd[] = {
GSF_XML_IN_NODE (CML, CML, -1, "cml", GSF_XML_CONTENT, &cml_doc, NULL),
	GSF_XML_IN_NODE (CML, MOLECULE, -1, "molecule", GSF_XML_CONTENT, cml_mol_start, cml_simple_end),
};

bool CMLLoader::Read  (Document *doc, GsfInput *in, char const *mime_type, IOContext *io)
{
	CMLReadState state;
	bool  success = false;

	state.doc = doc;
	state.context = io;

	if (NULL != in) {
		GsfXMLInDoc *xml = gsf_xml_in_doc_new (cml_dtd, NULL);
		success = gsf_xml_in_doc_parse (xml, in, &state);

		if (!success)
			gnm_io_warning (state.context,
				_("'%s' is corrupt!"),
				gsf_input_name (in));
		gsf_xml_in_doc_free (xml);
	}
	return success;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

bool CMLLoader::Write  (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io)
{
	if (NULL != out) {
		GsfXMLOut *xml = gsf_xml_out_new (out);
		gsf_xml_out_start_element (xml, "cml");
		string title = doc->GetProperty (GCU_PROP_DOC_TITLE);
		if (title.length ())
			gsf_xml_out_add_cstr (xml, "title", title.c_str ());
		gsf_xml_out_end_element (xml);
		g_object_unref (xml);
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// Initialization

static CMLLoader loader;

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
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

G_MODULE_EXPORT void
go_plugin_shutdown (GOPlugin *plugin, GOCmdContext *cc)
{
}

}
