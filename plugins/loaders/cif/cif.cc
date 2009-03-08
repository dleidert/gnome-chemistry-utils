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
#include <glib/gi18n-lib.h>
#include <map>
#include <stack>
#include <string>
#include <libintl.h>

using namespace gcu;
using namespace std;

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
