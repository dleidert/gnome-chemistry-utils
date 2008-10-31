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

#include <goffice/app/module-plugin-defs.h>
#include <libintl.h>

using namespace gcu;

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
}

CMLLoader::~CMLLoader ()
{
	RemoveMimeType ("chemical/x-cml");
}
////////////////////////////////////////////////////////////////////////////////
// Reading code

bool CMLLoader::Read  (Document *doc, GsfInput *in, char const *mime_type, IOContext *io)
{
	bool  success = false;

	return success;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

bool CMLLoader::Write  (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io)
{
	if (NULL != out) {
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
