// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/application.cc 
 *
 * Copyright (C) 2005-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "cmd-context-gtk.h"
#include "document.h"
#include "loader.h"
#include "message.h"
#include "ui-builder.h"
#include <gsf/gsf-input-gio.h>
#include <gsf/gsf-output-gio.h>
#include <glib/gi18n-lib.h>
#include <sys/stat.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <set>
#include <sstream>

using namespace std;

namespace gcu
{

GOConfNode *Application::m_ConfDir = NULL;

static map<string, Application *> Apps;
WindowState Application::DefaultWindowState = NormalWindowState;
static Application *Default = NULL;

class ApplicationPrivate
{
public:
	static void MaximizeWindows ();
	static void FullScreenWindows ();
};

void ApplicationPrivate::MaximizeWindows ()
{
	Application::DefaultWindowState = MaximizedWindowState;
}

void ApplicationPrivate::FullScreenWindows ()
{
	Application::DefaultWindowState = FullScreenWindowState;
}

static GOptionEntry options[] = 
{
  {"full-screen", 'F', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void *)ApplicationPrivate::MaximizeWindows, N_("Open new windows full screen"), NULL},
  {"maximize", 'M', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void *) ApplicationPrivate::FullScreenWindows, N_("Maximize new windows"), NULL},
  {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

Application::Application (string name, string datadir, char const *help_name, char const *icon_name, CmdContext *cc)
{
	if (m_ConfDir == NULL) {
		libgoffice_init ();
		m_ConfDir = go_conf_get_node (NULL, GCU_CONF_DIR);
	}
	m_CmdContext = cc;
	if (m_CmdContext)
		m_CmdContext->m_App = this;
	Apps[name] = this;
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
	char *dir = g_get_current_dir ();
	char *uri = g_filename_to_uri (dir, NULL, NULL);
	g_free (dir);
	CurDir = uri;
	g_free (uri);
	if (first_call && !g_get_application_name ()) { // needed to create several applications in the same program instance
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
	RegisterOptions (options);
	if (Default == NULL)
		Default = this;
}

Application::~Application ()
{
	Apps.erase (Name);
	if (m_CmdContext)
		delete m_CmdContext;
	if (Apps.empty ()) {
		ClearDialogs (); // needed to cleanly stop goffice
		go_conf_free_node (m_ConfDir);
		m_ConfDir = NULL;
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
	UIBuilder *builder = new UIBuilder (UIDIR"/image-resolution.ui", GETTEXT_PACKAGE);
	GtkWidget *w = builder->GetWidget ("screen-lbl");
	char *buf = g_strdup_printf (_("(screen resolution is %u)"), m_ScreenResolution);
	gtk_label_set_text (GTK_LABEL (w), buf);
	g_free (buf);
	w = builder->GetWidget ("res-btn");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), m_ImageResolution);
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_res_changed), this);
	w = builder->GetWidget ("transparent-btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), m_TransparentBackground);
	g_signal_connect (G_OBJECT (w), "toggled", G_CALLBACK (on_transparency_changed), this);
	w = builder->GetRefdWidget ("res-table");
	delete builder;
	return w;
}

GtkWidget *Application::GetImageSizeWidget ()
{
	UIBuilder *builder = new UIBuilder (UIDIR"/image-size.ui", GETTEXT_PACKAGE);
	GtkWidget *w = builder->GetWidget ("width");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), m_ImageWidth);
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_width_changed), this);
	w = builder->GetWidget ("height");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), m_ImageHeight);
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_height_changed), this);
	w = builder->GetRefdWidget ("size-table");
	delete builder;
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
	GError *error = NULL;
	GsfInput *input = gsf_input_gio_new_for_uri (uri.c_str (), &error);
	if (error) {
		g_error_free (error);
		return ContentTypeUnknown;
	}
	GOIOContext *io = GetCmdContext ()->GetNewGOIOContext ();
	ContentType ret = l->Read (Doc, input, mime_type, io);
	g_object_unref (input);
	g_object_unref (io);
	char *dirname = g_path_get_dirname (uri.c_str ());
	SetCurDir (dirname);
	g_free (dirname);
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
			ostringstream str;
			str << _("Error while processing ") << unescaped << ":\n" << error->message;
			Message *Box = new Message (this, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE);
			g_free (unescaped);
			g_error_free (error);
			Box->Run ();
			g_object_unref (file);
			return false;
		}
	}
	g_object_unref (file);
	GError *error = NULL;
	GsfOutput *output = gsf_output_gio_new_for_uri (uri.c_str (), &error);
	if (error) {
		g_error_free (error);
	}
	GOIOContext *io = GetCmdContext ()->GetNewGOIOContext ();
	bool ret = l->Write (const_cast <Document *> (Doc), output, mime_type, io, type);
	g_object_unref (output);
	g_object_unref (io);
	return ret;
}

GOConfNode *Application::GetConfDir ()
{
	if (m_ConfDir == NULL) {
		libgoffice_init ();
		m_ConfDir = go_conf_get_node (NULL, GCU_CONF_DIR);
	}
	return m_ConfDir;
}

struct option_data {
	GOptionEntry const *entries;
	char const *translation_domain;
};

void Application::RegisterOptions (GOptionEntry const *entries, char const *translation_domain)
{
	struct option_data d;
	d.entries = entries;
	d.translation_domain = translation_domain;
	m_Options.push_back (d);
}

void Application::AddOptions (GOptionContext *context)
{
	list<option_data>::iterator i, end = m_Options.end ();
	for (i = m_Options.begin (); i != end; i++)
		g_option_context_add_main_entries (context, (*i).entries, (*i).translation_domain);
}

Application *Application::GetDefaultApplication ()
{
	if (!Default)
		Default = new Application ("gcu"); // the name is just arbitrary
	return Default;
}

Application *Application::GetApplication (char const *name)
{
	map <string, Application *>::iterator i = Apps.find (name);
	return (i != Apps.end ())? (*i).second: NULL;
}

Application *Application::GetApplication (string &name)
{
	map <string, Application *>::iterator i = Apps.find (name);
	return (i != Apps.end ())? (*i).second: NULL;
}

static TypeId NextType = OtherType;

TypeId Application::AddType (std::string TypeName, Object* (*Create) (), TypeId id)
{
	TypeId Id = Object::GetTypeId (TypeName);
	if (Id != NoType)
		id = Id;
	if (id == OtherType) {
		id = NextType;
		NextType = TypeId ((unsigned) NextType + 1);
	}
	if (Object::GetTypeId (TypeName) != id)
		Object::AddAlias (id, TypeName);
	TypeDesc &typedesc = m_Types[id];
	typedesc.Id = id;
	typedesc.Create = Create;
	return typedesc.Id;
}

Object* Application::CreateObject (const std::string& TypeName, Object* parent)
{
	map <TypeId, TypeDesc>::const_iterator it = m_Types.find (Object::GetTypeId (TypeName));
	if (it == m_Types.end ())
		return NULL;
	TypeDesc const *typedesc = &(*it).second;
	Object* obj = (typedesc->Create)? typedesc->Create (): NULL;
	if (obj) {
		if (parent) {
			char const *id = obj->GetId ();
			if (id) {
				char* newId = parent->GetDocument ()->GetNewId (id, false);
				obj->SetId (newId);
				delete [] newId;
			}
			parent->AddChild (obj);
		}
		obj->m_TypeDesc = typedesc;
	}
	return obj;
}

bool Application::BuildObjectContextualMenu (Object *target, GtkUIManager *UIManager, Object *object, double x, double y)
{
	bool result = false;
	TypeDesc const *typedesc = target->m_TypeDesc;
	if (!typedesc)
		return false;
	list<BuildMenuCb>::const_iterator i, end = typedesc->MenuCbs.end ();
	for (i = typedesc->MenuCbs.begin (); i != end; i++)
		result |= (*i) (target, UIManager, object, x, y);
	return result;
}

void Application::AddRule (const string& type1, RuleId rule, const string& type2)
{
	AddRule (Object::GetTypeId (type1), rule, Object::GetTypeId (type2));
}

void Application::AddRule (TypeId type1, RuleId rule, TypeId type2)
{
	TypeDesc& typedesc1 = m_Types[type1];
	if (typedesc1.Id == NoType) {
		m_Types.erase (type1);
		return;
	}
	TypeDesc& typedesc2 = m_Types[type2];
	if (typedesc2.Id == NoType) {
		m_Types.erase (type2);
		return;
	}
	switch (rule) {
	case RuleMustContain:
		typedesc1.RequiredChildren.insert (typedesc2.Id);
	case RuleMayContain:
		typedesc1.PossibleChildren.insert (typedesc2.Id);
		typedesc2.PossibleParents.insert (typedesc1.Id);
		break;
	case RuleMustBeIn:
		typedesc1.RequiredParents.insert (typedesc2.Id);
	case RuleMayBeIn:
		typedesc2.PossibleChildren.insert (typedesc1.Id);
		typedesc1.PossibleParents.insert (typedesc2.Id);
		break;
	}
}

const set<TypeId>& Application::GetRules (const string& type, RuleId rule)
{
	return GetRules (Object::GetTypeId (type), rule);
}

const set<TypeId>& Application::GetRules (TypeId type, RuleId rule)
{
	static set <TypeId> noId;
	TypeDesc& typedesc = m_Types[type];
	switch (rule) {
	case RuleMustContain:
		return typedesc.RequiredChildren;
	case RuleMayContain:
		return typedesc.PossibleChildren;
	case RuleMustBeIn:
		return typedesc.RequiredParents;
	case RuleMayBeIn:
		return typedesc.PossibleParents;
	default:
		return noId;
	}
	return noId;
}

void Application::SetCreationLabel (TypeId Id, string Label)
{
	TypeDesc &type = m_Types[Id];
	type.CreationLabel = Label;
}

const string& Application::GetCreationLabel (TypeId Id)
{
	return m_Types[Id].CreationLabel;
}

const string& Application::GetCreationLabel (const string& TypeName)
{
	return m_Types[Object::GetTypeId (TypeName)].CreationLabel;
}

void Application::AddMenuCallback (TypeId Id, BuildMenuCb cb)
{
	TypeDesc& typedesc = m_Types[Id];
	typedesc.MenuCbs.push_back (cb);
}

TypeDesc const *Application::GetTypeDescription (TypeId Id)
{
	map <TypeId, TypeDesc>::iterator i = m_Types.find (Id);
	return (i != m_Types.end ())? &(*i).second: NULL;
}

CmdContext *Application::GetCmdContext ()
{
	if (m_CmdContext == NULL)
		m_CmdContext = new CmdContextGtk (this);
	return m_CmdContext;
}

}	//	namespace gcu
