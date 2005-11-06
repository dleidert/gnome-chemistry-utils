/* 
 * Gnome Chemisty Utils
 * chem-viewer.c 
 *
 * Copyright (C) 2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

#include <gdk/gdkx.h>
#include <gtk/gtkmain.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

GIOChannel *in_channel;

using namespace std;

static gboolean
io_func (GIOChannel *source, GIOCondition condition, gpointer data)
{
	string buf, strinst;
	void *instance;
	char *str;
	gsize length;
	g_io_channel_read_line (source, &str, &length, NULL, NULL);
	str[length - 1] = 0;
	buf = str;
	g_free (str);
	cerr << buf << endl;

	if (buf == "new") {
		string mime_type;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strinst = str;
		g_free (str);
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		mime_type = str;
		g_free (str);
		istringstream iss (strinst);
		if (mime_type != "chemical/x-xyz")
			return true; // only xyz files are allowed at the moment
		iss >> hex >> instance;
cerr << hex << instance << endl;
	} else if (buf == "win") {
		string strwin;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strinst = str;
		g_free (str);
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strwin = str;
		g_free (str);
		istringstream iss (strinst), iss_ (strwin);
		Window win;
		iss >> hex >> instance;
		iss_ >> hex >> win;
cerr << "window:" << strwin << "??" << hex << win << endl;
	} else if (buf == "file") {
		string filename;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strinst = str;
		g_free (str);
		istringstream iss (strinst);
		iss >> hex >> instance;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		filename = str;
		g_free (str);
		ifstream ifs (filename.c_str ());
cerr << filename << endl;
	} else if (buf == "kill") {
		string filename;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strinst = str;
		g_free (str);
		istringstream iss (strinst);
		iss >> hex >> instance;
cerr << "killing " << hex << instance << endl;
	} else if (buf == "halt") {
		gtk_main_quit ();
	}
	return true;
}

int main (int argc, char *argv[])
{
	GError *error = NULL;

	gtk_init (&argc, &argv);
	in_channel = g_io_channel_unix_new (fileno (stdin));
	g_io_add_watch (in_channel, G_IO_IN, io_func, &error);
	gtk_main ();
	return 0;
}
