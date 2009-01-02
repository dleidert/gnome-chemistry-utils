// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/application.cc 
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "cmd-context.h"
#include "document.h"
#include "loader.h"
#include <goffice/goffice.h>
#include <goffice/app/io-context.h>
#include <goffice/utils/go-file.h>
#include <gsf/gsf-input-gio.h>
#include <gsf/gsf-output-gio.h>
#include <glade/glade.h>
#ifndef HAVE_GO_CONF_SYNC
#include <gconf/gconf-client.h>
#endif
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <sys/stat.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <set>

using namespace std;

namespace gcu
{

#ifdef HAVE_GO_CONF_SYNC
GOConfNode *Application::m_ConfDir = NULL;
#endif

static set<Application *> Apps;

Application::Application (string name, string datadir, char const *help_name, char const *icon_name)
{
#ifdef HAVE_GO_CONF_SYNC
	if (m_ConfDir == NULL) {
		libgoffice_init ();
		m_ConfDir = go_conf_get_node (NULL, GCU_CONF_DIR);
	}
#else
	libgoffice_init ();
#endif
	Apps.insert (this);
	static bool first_call = true;
	Name = name;
	char const *szlang = getenv ("LANG");
	string lang = (szlang)? szlang: "C";
	HelpName = help_name? help_name: Name;
	HelpFilename = string ("file://") + datadir + string ("/gnome/help/") + HelpName + string ("-"API_VERSION"/") + lang + string ("/") + HelpName + "-"API_VERSION".xml";
	GFile *file = g_file_new_for_uri (HelpFilename.c_str ());
	bool exists = g_file_query_exists (file, NULL);
	g_object_unref (file);
	if (!exists) {
		HelpFilename = string ("file://") + datadir + string ("/gnome/help/") + HelpName + string ("-"API_VERSION"/C/") + HelpName + "-"API_VERSION".xml";
	}
	HelpBrowser = "yelp"; // there is no more key for that
	CurDir = g_get_current_dir ();
	if (first_call) { // needed to create several applications in the same program instance
		g_set_application_name (name.c_str ());
		first_call = false;
	}
	IconName = icon_name? icon_name: (help_name? help_name: Name.c_str ());
	GdkScreen *screen = gdk_screen_get_default ();
	m_ScreenResolution = (unsigned) rint (gdk_screen_get_width (screen) * 25.4 / gdk_screen_get_width_mm (screen));
	m_ImageResolution = m_ScreenResolution;
	m_ImageHeight = m_ImageWidth = 300;
	m_RecentManager = gtk_recent_manager_get_default ();

	// check supported pixbuf formats
	GSList *formats = gdk_pixbuf_get_formats ();
	GSList *l = formats;
	GdkPixbufFormat *format;
	char **mimes;
	while (l) {
		format = (GdkPixbufFormat*) l->data;
		if (gdk_pixbuf_format_is_writable (format)) {
			mimes = gdk_pixbuf_format_get_mime_types (format);
			m_SupportedPixbufFormats[*mimes] = format;
			g_strfreev (mimes);
		}
		l = l->next;
	}
	g_slist_free (formats);
}

Application::~Application ()
{
	Apps.erase (this);
	if (Apps.empty ()) {
		ClearDialogs (); // needed to cleanly stop goffice
#ifdef HAVE_GO_CONF_SYNC
		go_conf_free_node (m_ConfDir);
		m_ConfDir = NULL;
#endif
		libgoffice_shutdown ();
	}
}

void Application::OnHelp (string tag)
{
	if (!HasHelp ())
		return;
	char *argv[3] = {NULL, NULL, NULL};
	argv[0] = (char*) HelpBrowser.c_str();
	string path = HelpFilename;
	if (tag.length ())
		path += string("#") + HelpName + string ("-") + tag;
	argv[1] = (char*) path.c_str ();
	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
				   NULL, NULL, NULL, NULL);
}

bool Application::HasHelp ()
{
	if (!HelpBrowser.length () || !HelpFilename.length ())
		return false;
	GFile *file = g_file_new_for_uri (HelpFilename.c_str ());
	bool exists = g_file_query_exists (file, NULL);
	g_object_unref (file);
	return exists;	
}

void Application::SetCurDir (char const* dir)
{
	CurDir = dir;
}

void Application::SetCurDir (string const &dir)
{
	CurDir = dir;
}

void Application::OnMail (char const *MailAddress)
{
	go_url_show (MailAddress);
}

void Application::ShowURI (string& uri)
{
	go_url_show (uri.c_str ());
}

void Application::OnLiveAssistance ()
{
	go_url_show ("irc://irc.gimp.net/gchemutils");
}

static void on_res_changed (GtkSpinButton *btn, Application *app)
{
	app->SetImageResolution (gtk_spin_button_get_value_as_int (btn));
}

static void on_width_changed (GtkSpinButton *btn, Application *app)
{
	app->SetImageWidth (gtk_spin_button_get_value_as_int (btn));
}

static void on_height_changed (GtkSpinButton *btn, Application *app)
{
	app->SetImageHeight (gtk_spin_button_get_value_as_int (btn));
}

static void on_transparency_changed (GtkToggleButton *btn, Application *app)
{
	app->SetTransparentBackground (gtk_toggle_button_get_active (btn));
}

GtkWidget *Application::GetImageResolutionWidget ()
{
	GladeXML *xml = glade_xml_new (GLADEDIR"/image-resolution.glade", "res-table", NULL);
	GtkWidget *w = glade_xml_get_widget (xml, "screen-lbl");
	char *buf = g_strdup_printf (_("(screen resolution is %u)"), m_ScreenResolution);
	gtk_label_set_text (GTK_LABEL (w), buf);
	g_free (buf);
	w = glade_xml_get_widget (xml, "res-btn");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), m_ImageResolution);
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_res_changed), this);
	w = glade_xml_get_widget (xml, "transparent-btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), m_TransparentBackground);
	g_signal_connect (G_OBJECT (w), "toggled", G_CALLBACK (on_transparency_changed), this);
	w = glade_xml_get_widget (xml, "res-table");
	g_object_unref (G_OBJECT (xml));
	return w;
}

GtkWidget *Application::GetImageSizeWidget ()
{
	GladeXML *xml = glade_xml_new (GLADEDIR"/image-size.glade", "size-table", NULL);
	GtkWidget *w = glade_xml_get_widget (xml, "width");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), m_ImageWidth);
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_width_changed), this);
	w = glade_xml_get_widget (xml, "height");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), m_ImageHeight);
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_height_changed), this);
	w = glade_xml_get_widget (xml, "size-table");
	return w;
}

char const *Application::GetPixbufTypeName (string& filename, char const *mime_type)
{
	GdkPixbufFormat *format = m_SupportedPixbufFormats[mime_type];
	char **exts, **ext;
	bool found = false;
	int i;
	if (!format)
		return NULL;
	// ensure the file name has a valid extension and add the default one if not
	exts = gdk_pixbuf_format_get_extensions (format);
	ext = exts;
	while (*ext) {
		i = filename.length() - strlen (*ext);
		if ((i > 1) && (filename[i - 1] == '.') && !filename.compare (i, strlen (*ext), *ext)) {
			found = true;
			break;
		}
		ext++;
	}
	if (!found)
		filename += string (".") + *exts;
	g_strfreev (exts);
	return gdk_pixbuf_format_get_name (format);
}

void Application::RemoveDocument (Document *Doc)
{
	m_Docs.erase (Doc);
	if (m_Docs.size () == 0 && gtk_main_level ())
		NoMoreDocsEvent ();
}

ContentType Application::Load (std::string const &uri, const gchar *mime_type, Document* Doc)
{
	Loader *l = Loader::GetLoader (mime_type);
	if (!l)
		return ContentTypeUnknown;
	string old_num_locale = setlocale (LC_NUMERIC, NULL);
	setlocale (LC_NUMERIC, "C");
	GError *error = NULL;
	GsfInput *input = gsf_input_gio_new_for_uri (uri.c_str (), &error);
	if (error) {
		g_error_free (error);
		return ContentTypeUnknown;
	}
	IOContext *io = gnumeric_io_context_new (gcu_get_cmd_context ());
	ContentType ret = l->Read (Doc, input, mime_type, io);
	g_object_unref (input);
	g_object_unref (io);
	setlocale (LC_NUMERIC, old_num_locale.c_str ());
	return ret;
}

bool Application::Save (std::string const &uri, const gchar *mime_type, Document const *Doc, ContentType type)
{
	Loader *l = Loader::GetSaver (mime_type);
	if (!l)
		return false;
	GFile *file = g_file_new_for_uri (uri.c_str ());
	if (g_file_query_exists (file, NULL)) {
		GError *error = NULL;
		g_file_delete (file, NULL, &error);
		if (error) {
			char *unescaped = g_uri_unescape_string (uri.c_str (), NULL);
			gchar * message = g_strdup_printf (_("Error while processing %s:\n%s"), unescaped, error->message);
			g_free (unescaped);
			g_error_free (error);
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, message));
			gtk_window_set_icon_name (GTK_WINDOW (Box), IconName.c_str ());
			gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
			g_object_unref (file);
			return false;
		}
	}
	g_object_unref (file);
	string old_num_locale = setlocale (LC_NUMERIC, NULL);
	setlocale (LC_NUMERIC, "C");
	GError *error = NULL;
	GsfOutput *output = gsf_output_gio_new_for_uri (uri.c_str (), &error);
	if (error) {
		g_error_free (error);
	}
	IOContext*io  = gnumeric_io_context_new (gcu_get_cmd_context ());
	bool ret = l->Write (const_cast <Document *> (Doc), output, mime_type, io, type);
	g_object_unref (output);
	g_object_unref (io);
	setlocale (LC_NUMERIC, old_num_locale.c_str ());
	return ret;
}

#ifdef HAVE_GO_CONF_SYNC
GOConfNode *Application::GetConfDir ()
{
	if (m_ConfDir == NULL) {
		libgoffice_init ();
		m_ConfDir = go_conf_get_node (NULL, GCU_CONF_DIR);
	}
	return m_ConfDir;
}
#endif

}	//	namespace gcu
