/* 
 * Gnome Chemistry Utils
 * printable.h
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
#include "printable.h"

namespace gcu {

class PrintSettings
{
public:
	PrintSettings ();
	virtual ~PrintSettings ();
	GtkPrintSettings *settings;
	GtkPageSetup *setup;
};

PrintSettings::PrintSettings ()
{
	settings = NULL;
	setup = NULL;
}

PrintSettings::~PrintSettings ()
{
	g_object_unref (setup);
	g_object_unref (settings);
}

static PrintSettings DefaultSettings;

Printable::Printable ():
	DialogOwner ()
{
	if (DefaultSettings.settings == NULL) {
		DefaultSettings.settings = gtk_print_settings_new ();
		DefaultSettings.setup = gtk_page_setup_new ();
		// TODO: import default values from conf keys
	}
	m_PrintSettings = gtk_print_settings_copy (DefaultSettings.settings);
	m_PageSetup = gtk_page_setup_copy (DefaultSettings.setup);
}

Printable::~Printable ()
{
	g_object_unref (m_PageSetup);
	g_object_unref (m_PrintSettings);
}

}	//	namespace gcu
