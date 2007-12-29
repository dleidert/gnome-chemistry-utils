// -*- C++ -*-

/* 
 * CDXML files loader plugin
 * cdx.cc 
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <goffice/app/module-plugin-defs.h>
#include <libintl.h>
#include <cstdio>

using namespace std;
using namespace gcu;

class CDXLoader: public gcu::Loader
{
public:
	CDXLoader ();
	virtual ~CDXLoader ();

	bool Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io);
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
	return true;
}

bool CDXLoader::Write  (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io)
{
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
}

G_MODULE_EXPORT void
go_plugin_shutdown (GOPlugin *plugin, GOCmdContext *cc)
{
}

}
