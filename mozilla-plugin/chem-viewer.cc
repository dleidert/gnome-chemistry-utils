/* 
 * Gnome Chemisty Utils
 * chem-viewer.c 
 *
 * Copyright (C) 2005-2006
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

#include "config.h"
#include "gcu/gtkchem3dviewer.h"
#include <gdk/gdkx.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkplug.h>
#include <libgnomevfs/gnome-vfs.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

using namespace std;

class ChemComp
{
public:
	ChemComp (void* instance, string& mime_type);
	~ChemComp ();

	void SetWindow (XID xid);
	void SetFilename (string& filename);
	void SetProperty (string& name, char const *value) {Params[name] = value;}

private:
	void* Instance;
	XID Xid;
	string Filename, MimeType;
	GtkWidget *Plug, *Viewer;
	GdkWindow *Parent;
	map<string, string> Params;
};

ChemComp::ChemComp (void* instance, string& mime_type)
{
	Instance = instance;
	MimeType = mime_type;
	Xid = 0;
	Plug = NULL;
}

ChemComp::~ChemComp ()
{
	gtk_widget_unrealize (Plug);
	gtk_widget_destroy (Plug);
	g_object_unref (Parent);
}

void ChemComp::SetWindow (XID xid)
{
	int width, height;
	if (Xid == xid) {
		//just resize and redraw
		gdk_window_get_geometry (Parent, NULL, NULL, &width, &height, NULL);
		gtk_window_resize (GTK_WINDOW (Plug), width, height);
	} else {
		if (Plug) // does this happen ?
			return;
		Xid = xid;
		Plug = gtk_plug_new (xid);    
		Parent = gdk_window_foreign_new (xid);
		gdk_window_get_geometry (Parent, NULL, NULL, &width, &height, NULL);
		gtk_window_set_default_size (GTK_WINDOW (Plug), width, height);
		gtk_widget_realize (Plug);
		XReparentWindow (GDK_WINDOW_XDISPLAY (Plug->window),
			GDK_WINDOW_XID (Plug->window), xid, 0, 0);
		XMapWindow (GDK_WINDOW_XDISPLAY (Plug->window),
			GDK_WINDOW_XID (Plug->window));
		Viewer = gtk_chem3d_viewer_new (NULL);
		gtk_container_add (GTK_CONTAINER (Plug), Viewer);
		gtk_widget_show_all (Plug);
	}
}

void ChemComp::SetFilename (string& filename)
{
	Filename = filename;
	gtk_chem3d_viewer_set_uri_with_mime_type (GTK_CHEM3D_VIEWER (Viewer),
			filename.c_str (), MimeType.c_str ());
	map<string, string>::iterator i, iend = Params.end ();
	for (i = Params.begin (); i != iend; i++) {
		if ((*i).first == "display3d") {
			if ((*i).second == "ball&stick")
				g_object_set (G_OBJECT (Viewer), "display3d", BALL_AND_STICK, NULL);
			else if ((*i).second == "spacefill")
				g_object_set (G_OBJECT (Viewer), "display3d", SPACEFILL, NULL);
		}
		else
			g_object_set (G_OBJECT (Viewer), (*i).first.c_str (), (*i).second.c_str (), NULL);
	}
	Params.clear ();
}

GIOChannel *in_channel;
map<void*, ChemComp*> components;

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
		iss >> hex >> instance;
		if (components[instance] != NULL) // this should not occur
			delete components[instance];
		components[instance] = new ChemComp (instance, mime_type);
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		buf = str;
		g_free (str);
		while (buf != "end") {
			g_io_channel_read_line (source, &str, &length, NULL, NULL);
			str[length - 1] = 0;
			components[instance]->SetProperty (buf, str);
			g_free (str);
			g_io_channel_read_line (source, &str, &length, NULL, NULL);
			str[length - 1] = 0;
			buf = str;
			g_free (str);
		}
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
		XID xid;
		iss >> hex >> instance;
		iss_ >> hex >> xid;
		if (components[instance] != NULL)
			components[instance]->SetWindow (xid);
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
		if (components[instance] != NULL)
			components[instance]->SetFilename (filename);
	} else if (buf == "kill") {
		string filename;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strinst = str;
		g_free (str);
		istringstream iss (strinst);
		iss >> hex >> instance;
		if (components[instance] != NULL)
			delete components[instance];
		components.erase (instance);
	} else if (buf == "halt") {
		gtk_main_quit ();
	}
	return true;
}

int main (int argc, char *argv[])
{
	GError *error = NULL;

	gtk_init (&argc, &argv);
	if (!gnome_vfs_init ()) {
		cerr << "Could not initialize GnomeVFS\n" << endl;
		return 1;
	}
	in_channel = g_io_channel_unix_new (fileno (stdin));
	g_io_add_watch (in_channel, G_IO_IN, io_func, &error);
	gtk_main ();
	g_io_channel_unref (in_channel);
	map <void*, ChemComp*>::iterator i, end = components.end ();
	for (i = components.begin (); i != end; i++)
		delete (*i).second;
	components.clear ();
	return 0;
}
