/* 
 * Gnome Chemisty Utils
 * chem-viewer.c 
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/element.h>
#include <gcu/gcuchem3dviewer.h>
#include <gcr/gcrcrystalviewer.h>
#include <gcu/gcuspectrumviewer.h>
#include <gcu/chem3ddoc.h>
#include <gcu/loader.h>
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/structs.h>
#include <goffice/goffice.h>
#include <goffice/app/go-plugin.h>
#include <goffice/app/go-plugin-loader-module.h>
#include <gdk/gdkx.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkplug.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <string>
#include <cstring>

using namespace std;
using namespace gcu;

static bool loaded_radii = false;

class MozPaintApp: public gcp::Application
{
public:
	MozPaintApp ();
	virtual ~MozPaintApp ();

	void OnFileNew (G_GNUC_UNUSED char const *Theme = NULL) {}
	GtkWindow* GetWindow() {return NULL;}
};

MozPaintApp::MozPaintApp (): gcp::Application ()
{
}

MozPaintApp::~MozPaintApp ()
{
}

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
	gcp::Document *Doc;
	map<string, string> Params;
	MozPaintApp *gcpApp;
	bool Loaded;
};

ChemComp::ChemComp (void* instance, string& mime_type)
{
	Instance = instance;
	MimeType = mime_type;
	Xid = 0;
	Plug = NULL;
	Doc = NULL;
	gcpApp = NULL;
	Viewer = NULL;
	Loaded = false;
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
	if (Viewer)
		return;
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
		GdkWindow *window = gtk_widget_get_window (Plug);
		XReparentWindow (GDK_WINDOW_XDISPLAY (window),
			GDK_WINDOW_XID (window), xid, 0, 0);
		XMapWindow (GDK_WINDOW_XDISPLAY (window),
			GDK_WINDOW_XID (window));
		if (MimeType == "application/x-gcrystal" || MimeType == "chemical/x-cif")
			Viewer = gcr_crystal_viewer_new (NULL);
		else if (MimeType == "application/x-gchempaint" || MimeType == "chemical/x-cdx" || MimeType == "chemical/x-cdxml") {
			if (!gcpApp)
				gcpApp = new MozPaintApp ();
			Doc = new gcp::Document (gcpApp, true, NULL);
			Doc->SetEditable (false);
			Viewer = Doc->GetView ()->CreateNewWidget ();
		} else 	if (MimeType == "chemical/x-jcamp-dx")
			Viewer = gcu_spectrum_viewer_new (NULL);
		else
			Viewer = gcu_chem3d_viewer_new (NULL);
		gtk_container_add (GTK_CONTAINER (Plug), Viewer);
		gtk_widget_show_all (Plug);
	}
}

void ChemComp::SetFilename (string& filename)
{
	if (Loaded)
		return;
	Loaded = true;
	if (Filename.length ())
		return;
	Filename = filename;
	if (MimeType == "application/x-gcrystal") {
		if (!loaded_radii) {
			Element::LoadRadii ();
			loaded_radii = true;
		}
		xmlDocPtr xml = xmlParseFile (filename.c_str ());
		if (!xml || !xml->children || strcmp ((char*) xml->children->name, "crystal"))
			return;
		gcr_crystal_viewer_set_data (GCR_CRYSTAL_VIEWER (Viewer), xml->children);
		xmlFree (xml);
	} else if (MimeType == "chemical/x-cif") {
		if (!loaded_radii) {
			Element::LoadRadii ();
			loaded_radii = true;
		}
		if (filename[0] == '/') {
			char *uri = go_filename_to_uri (filename.c_str ());
			filename = uri;
			g_free (uri);
		}
		gcr_crystal_viewer_set_uri_with_mime_type (GCR_CRYSTAL_VIEWER (Viewer), filename.c_str (), "chemical/x-cif");
	} else 	if (MimeType == "application/x-gchempaint") {
		xmlDocPtr xml = xmlParseFile (filename.c_str ());
		if (!xml || !xml->children || strcmp ((char*) xml->children->name, "chemistry"))
			return;
		Doc->Load (xml->children);
		gccv::Rect r;
		gcp::WidgetData *pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (Viewer), "data");
		Doc->GetView ()->Update (Doc);
		pData->GetObjectBounds (Doc, &r);
		gcp::Theme *pTheme = Doc->GetTheme ();
		if (r.x0 || r.y0)
			Doc->Move (- r.x0 / pTheme->GetZoomFactor (), - r.y0 / pTheme->GetZoomFactor ());
		Doc->GetView ()->Update (Doc);
		xmlFree (xml);
	} else if (MimeType == "chemical/x-cdx" || MimeType == "chemical/x-cdxml") {
		char *uri = g_filename_to_uri (filename.c_str (), NULL, NULL);
		gcpApp->Load (uri, MimeType.c_str (), Doc);
		g_free (uri);
		gccv::Rect r;
		gcp::WidgetData *pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (Viewer), "data");
		gcp::Theme *pTheme = Doc->GetTheme ();
		Doc->GetView ()->Update (Doc);
		pData->GetObjectBounds (Doc, &r);
		if (r.x0 || r.y0)
			Doc->Move (- r.x0 / pTheme->GetZoomFactor (), - r.y0 / pTheme->GetZoomFactor ());
		Doc->GetView ()->Update (Doc);
	} else 	if (MimeType == "chemical/x-jcamp-dx") {
		char *uri = g_filename_to_uri (filename.c_str (), NULL, NULL);
		gcu_spectrum_viewer_set_uri (GCU_SPECTRUM_VIEWER (Viewer), uri);
		g_free (uri);
	} else {
		char *uri = g_filename_to_uri (filename.c_str (), NULL, NULL);
		gcu_chem3d_viewer_set_uri_with_mime_type (GCU_CHEM3D_VIEWER (Viewer),
				uri, MimeType.c_str ());
		g_free (uri);
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
}

GIOChannel *in_channel;
map<void*, ChemComp*> components;

static gboolean
io_func (GIOChannel *source, G_GNUC_UNUSED GIOCondition condition, G_GNUC_UNUSED gpointer data)
{
	string buf, strinst;
	void *instance;
	char *str;
	gsize length;
	g_io_channel_read_line (source, &str, &length, NULL, NULL);
	str[length - 1] = 0;
	buf = str;
	g_free (str);
	str = NULL;

	if (buf == "new") {
		string mime_type;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strinst = str;
		g_free (str);
		str = NULL;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		mime_type = str;
		g_free (str);
		str = NULL;
		istringstream iss (strinst);
		iss >> hex >> instance;
		if (components[instance] != NULL) // this should not occur
			delete components[instance];
		components[instance] = new ChemComp (instance, mime_type);
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		buf = str;
		g_free (str);
		str = NULL;
		while (buf != "end") {
			g_io_channel_read_line (source, &str, &length, NULL, NULL);
			str[length - 1] = 0;
			components[instance]->SetProperty (buf, str);
			g_free (str);
			str = NULL;
			g_io_channel_read_line (source, &str, &length, NULL, NULL);
			str[length - 1] = 0;
			buf = str;
			g_free (str);
			str = NULL;
		}
	} else if (buf == "win") {
		string strwin;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strinst = str;
		g_free (str);
		str = NULL;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strwin = str;
		g_free (str);
		str = NULL;
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
		str = NULL;
		istringstream iss (strinst);
		iss >> hex >> instance;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		filename = str;
		g_free (str);
		str = NULL;
		if (components[instance] != NULL)
			components[instance]->SetFilename (filename);
	} else if (buf == "kill") {
		string filename;
		g_io_channel_read_line (source, &str, &length, NULL, NULL);
		str[length - 1] = 0;
		strinst = str;
		g_free (str);
		str = NULL;
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
	libgoffice_init ();
	go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
	Loader::Init (Application::GetDefaultApplication ());
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
