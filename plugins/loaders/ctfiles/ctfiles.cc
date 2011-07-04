// -*- C++ -*-

/*
 * CTfiles loader plugin
 * ctfiles.cc
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/document.h>
#include <gcu/loader.h>
#include <gcu/molecule.h>
#include <goffice/app/module-plugin-defs.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-output.h>
#include <glib/gi18n-lib.h>
#include <map>
#include <string>

class CTfilesLoader: public gcu::Loader
{
public:
	CTfilesLoader ();
	virtual ~CTfilesLoader ();

	gcu::ContentType Read (gcu::Document *doc, GsfInput *in, char const *mime_type, GOIOContext *io);
	bool Write (gcu::Object const *obj, GsfOutput *out, char const *mime_type, GOIOContext *io, gcu::ContentType type);

	bool WriteObject (GsfXMLOut *out, gcu::Object const *object, GOIOContext *io, gcu::ContentType type);

private:
	std::map <std::string, bool (*) (CTfilesLoader *, GsfXMLOut *, gcu::Object const *, GOIOContext *s, gcu::ContentType)> m_WriteCallbacks;
};

CTfilesLoader::CTfilesLoader ()
{
	AddMimeType ("chemical/x-mdl-molfile");
}

CTfilesLoader::~CTfilesLoader ()
{
	RemoveMimeType ("chemical/x-mdl-molfile");
}

////////////////////////////////////////////////////////////////////////////////
// Reading code

gcu::ContentType CTfilesLoader::Read  (G_GNUC_UNUSED gcu::Document *doc, G_GNUC_UNUSED GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io)
{
	// for now, not implemented
	return gcu::ContentTypeUnknown;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

bool CTfilesLoader::WriteObject (GsfXMLOut *xml, gcu::Object const *object, GOIOContext *io, gcu::ContentType type)
{
	std::string name = gcu::Object::GetTypeName (object->GetType ());
	std::map <std::string, bool (*) (CTfilesLoader *, GsfXMLOut *, gcu::Object const *, GOIOContext *, gcu::ContentType)>::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (this, xml, object, io, type);
	// if we don't save the object iself, try to save its children
	std::map <std::string, gcu::Object *>::const_iterator j;
	gcu::Object const *child = object->GetFirstChild (j);
	while (child) {
		if (!WriteObject (xml, child, io, type))
			return false;
		child = object->GetNextChild (j);
	}
	return true; /* loosing data is not considered an error, it is just a missing feature
					either in this code or in the cml schema */
}

bool CTfilesLoader::Write  (gcu::Object const *obj, GsfOutput *out, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io, G_GNUC_UNUSED gcu::ContentType type)
{
	if (NULL != out) {
		gcu::Document const *doc = dynamic_cast <gcu::Document const *> (obj);
		if (!doc)
			doc = obj->GetDocument ();
		if (obj->GetType () == gcu::MoleculeType) {
			gcu::Molecule const *mol = static_cast < gcu::Molecule const * > (obj);
			// we don't use the three first lines at least for now
			gsf_output_write (out, 3, reinterpret_cast < guint8 const * > ("\n\n\n"));
			char buf[] = "                                 V2000\n";
			// fill the various fields
			snprintf (buf, 3, "%3d", mol->GetAtomsNumber ());
			gsf_output_write (out, strlen (buf), reinterpret_cast < guint8 const * > (buf));
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
