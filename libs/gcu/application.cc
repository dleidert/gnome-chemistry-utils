// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/application.cc 
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "ui-manager.h"
#include <gsf/gsf-input-gio.h>
#include <gsf/gsf-output-gio.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-memory.h>
#include <glib/gi18n-lib.h>
#include <sys/stat.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <set>
#include <sstream>

using namespace std;

namespace gcu
{

GOConfNode *Application::m_ConfDir = NULL;

static map<string, Application *> Apps;
static Application *Default = NULL;

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
	if (Default == NULL)
		Default = this;
	RegisterBabelType ("chemical/x-xyz", "xyz");
#if 0
	g_idle_add (reinterpret_cast <GSourceFunc> (ApplicationPrivate::LoadDatabases), this);
#endif
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

ContentType Application::Load (std::string const &uri, const gchar *mime_type, Document* Doc, const char *options)
{
	Loader *l = Loader::GetLoader (mime_type);
	GsfInput *input;
	if (!l) {
		l = Loader::GetLoader ("chemical/x-cml");
		if (!l)
			return ContentTypeUnknown;
		char *cml = ConvertToCML (uri, mime_type, options);
		if (!cml)
			return ContentTypeUnknown;
		input = gsf_input_memory_new (const_cast <guint8 const *> (reinterpret_cast <guint8 *> (cml)), strlen (cml), true);
		mime_type = "chemical/x-cml";
	} else {
		GError *error = NULL;
		input = gsf_input_gio_new_for_uri (uri.c_str (), &error);
		if (error) {
			g_error_free (error);
			return ContentTypeUnknown;
		}
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

ContentType Application::Load (GsfInput *input, const gchar *mime_type, Document* Doc, const char *options)
{
	Loader *l = Loader::GetLoader (mime_type);
	bool needs_free = false;
	if (!l) {
		l = Loader::GetLoader ("chemical/x-cml");
		if (!l)
			return ContentTypeUnknown;
		char *cml = ConvertToCML (input, mime_type, options);
		if (!cml)
			return ContentTypeUnknown;
		input = gsf_input_memory_new (const_cast <guint8 const *> (reinterpret_cast <guint8 *> (cml)), strlen (cml), true);
		mime_type = "chemical/x-cml";
		needs_free = true;
	}
	GOIOContext *io = GetCmdContext ()->GetNewGOIOContext ();
	ContentType ret = l->Read (Doc, input, mime_type, io);
	g_object_unref (io);
	if (needs_free)
		g_object_unref (input);
	return ret;
}

bool Application::Save (std::string const &uri, const gchar *mime_type, Object const *Obj, ContentType type, const char *options)
{
	Loader *l = Loader::GetSaver (mime_type);
	GError *error = NULL;
	GOIOContext *io = GetCmdContext ()->GetNewGOIOContext ();
	if (!l) {
		l = Loader::GetSaver ("chemical/x-cml");
		if (!l) {
			g_object_unref (io);
			return false;
		}
		GsfOutput *output = gsf_output_memory_new ();
		l->Write (Obj, output, "chemical/x-cml", io, type);
		guint8 const* cml = gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (output));
		g_object_unref (io);
		if (cml)
			ConvertFromCML (reinterpret_cast < char const * > (cml), uri, mime_type, options);
		g_object_unref (output);		
		return true;
	}
	GFile *file = g_file_new_for_uri (uri.c_str ());
	if (g_file_query_exists (file, NULL)) {
		GError *error = NULL;
		g_file_delete (file, NULL, &error);
		if (error) {
			char *unescaped = g_uri_unescape_string (uri.c_str (), NULL);
			ostringstream str;
			str << _("Error while processing ") << unescaped << ":\n" << error->message;
			m_CmdContext->Message (str.str ().c_str (), CmdContext::SeverityError, false);
			g_free (unescaped);
			g_error_free (error);
			g_object_unref (file);
			return false;
		}
	}
	g_object_unref (file);
	GsfOutput *output = gsf_output_gio_new_for_uri (uri.c_str (), &error);
	if (error) {
		g_error_free (error);
	}
	bool ret = l->Write (Obj, output, mime_type, io, type);
	g_object_unref (output);
	g_object_unref (io);
	return ret;
}

bool Application::Save (GsfOutput *output, const gchar *mime_type, Object const *Obj, ContentType type, const char *options)
{
	bool ret;
	Loader *l = Loader::GetSaver (mime_type);
	GOIOContext *io = GetCmdContext ()->GetNewGOIOContext ();
	if (!l) {
		l = Loader::GetSaver ("chemical/x-cml");
		if (!l)
			return false;
		GsfOutput *cml = gsf_output_memory_new ();
		ret = l->Write (const_cast <Object *> (Obj), cml, "chemical/x-cml", io, type);
		if (ret) {
			ConvertFromCML (reinterpret_cast <char const *> (gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (cml))), output, mime_type, options);
			ret = gsf_output_size (output) > 0;
		}
		g_object_unref (cml);
	} else
		ret = l->Write (const_cast <Object *> (Obj), output, mime_type, io, type);
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

bool Application::BuildObjectContextualMenu (Object *target, UIManager *uim, Object *object, double x, double y)
{
	bool result = false;
	TypeDesc const *typedesc = target->m_TypeDesc;
	if (!typedesc)
		return false;
	list<BuildMenuCb>::const_iterator i, end = typedesc->MenuCbs.end ();
	for (i = typedesc->MenuCbs.begin (); i != end; i++)
		result |= (*i) (target, uim, object, x, y);
	return result;
}

CmdContext *Application::GetCmdContext ()
{
	if (m_CmdContext == NULL)
		CreateDefaultCmdContext ();
	return m_CmdContext;
}

void Application::RegisterBabelType (const char *mime_type, const char *type)
{
	std::map <std::string, std::string>::iterator it = m_BabelTypes.find (mime_type);
	if (it == m_BabelTypes.end ())
		m_BabelTypes[mime_type] = type;
}

char const *Application::MimeToBabelType (char const *mime_type)
{
	std::map <std::string, std::string>::iterator it = m_BabelTypes.find (mime_type);
	return (it == m_BabelTypes.end ())? mime_type: (*it).second.c_str ();
}


int Application::OpenBabelSocket ()
{
	struct stat statbuf;
	static std::string socket_path = "/tmp/babelsocket-";
	if (socket_path.length () == 17)
		socket_path += getenv ("USER");
	if (stat (socket_path.c_str (), &statbuf)) {
		char *args[] = {const_cast <char *> (LIBEXECDIR"/babelserver"), NULL};
		GError *error = NULL;
		g_spawn_async (NULL, (char **) args, NULL, static_cast <GSpawnFlags> (0), NULL, NULL, NULL, &error);
		if (error) {
			g_error_free (error);
			error = NULL;
			return -1;
		}
		time_t endtime = time (NULL) + 15; // hoping the server will have started within 15 seconds
		while (stat (socket_path.c_str (), &statbuf))
			if (time (NULL) > endtime)
				return -1; // timeout
	}
	int res = socket (AF_UNIX, SOCK_STREAM, 0);
	if (res == -1) {
		perror ("Could not create the socket");
		return -1;
	}
	struct sockaddr_un adr_serv;
	adr_serv.sun_family = AF_UNIX;
	strcpy (adr_serv.sun_path, socket_path.c_str ());
	if (connect (res, (const struct sockaddr*) &adr_serv, sizeof (struct sockaddr_un)) == -1) {
		perror ("Connexion failed");
		return -1;
	}
	return res;
}

/*!
@param uri the uri of the document to convert.
@param mime_type the mime type of the document.
@param options options to pass to OpenBabel.
 
This method converts the source to CML.
@return the converted text as a newly allocate string or NULL.
*/
char* Application::ConvertToCML (std::string const &uri, const char *mime_type, const char *options)
{
	int sock = OpenBabelSocket ();
	if (sock <= 0)
		return NULL;
	GVfs *vfs = g_vfs_get_default ();
	GFile *file = g_vfs_get_file_for_uri (vfs, uri.c_str ());
	char *path = g_file_get_path (file);
	std::string buf = "-i ";
	buf += MimeToBabelType (mime_type);
	if (path) {
		buf += " ";
		buf += path;
		buf += " -o cml";
		if (options) {
			buf += " ";
			buf += options;
		}
		buf += " -D";
		write (sock, buf.c_str (), buf.length ());
		g_free (path);
	} else {
		buf += " -o cml";
		if (options) {
			buf += " ";
			buf += options;
		}
		// load the data in memory and write them to the socket
		GError *error = NULL;
		GFileInfo *info = g_file_query_info (file,
											 ((mime_type)? G_FILE_ATTRIBUTE_STANDARD_SIZE:
											 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","G_FILE_ATTRIBUTE_STANDARD_SIZE),
											 G_FILE_QUERY_INFO_NONE,
											 NULL, &error);
		if (error) {
			g_message ("GIO querry failed: %s", error->message);
			g_error_free (error);
			g_object_unref (file);
			return NULL;
		}
		gsize size = g_file_info_get_size (info);
		g_object_unref (info);
		GInputStream *input = G_INPUT_STREAM (g_file_read (file, NULL, &error));
		if (error) {
			g_message ("GIO could not create the stream: %s", error->message);
			g_error_free (error);
			g_object_unref (file);
			return NULL;
		}
		gchar *szbuf = new gchar[size];
		gsize n = g_input_stream_read (input, szbuf, size, NULL, &error);
		if (error) {
			g_message ("GIO could not read the file: %s", error->message);
			g_error_free (error);
			delete [] szbuf;
			return NULL;
		}
		g_object_unref (input);
		g_object_unref (file);
		if (n != size) {
			delete [] szbuf;
			return NULL;
		}
		char *szsize = g_strdup_printf (" -l %lu -D", size);
		buf += szsize;
		g_free (szsize);
		write (sock, buf.c_str (), buf.length ());
		write (sock, szbuf, size);
		delete [] szbuf;
	}
	time_t timeout = time (NULL) + 60;
	char inbuf[256], *start = inbuf, *end;
	unsigned cur, index = 0, length = 0;
	while (time (NULL) < timeout) {
		if ((cur = read (sock, start + index, ((length)? length: 255) - index))) {
			index += cur;
			start[index] = 0;
			if (start == inbuf) {
				if ((end = strchr (inbuf, ' '))) {
					length = strtoul (inbuf, NULL, 10);
					start = reinterpret_cast <char *> (g_malloc (length + 1));
					if (!start)
						break;
					strcpy (start, end + 1);
					index = strlen (start);
				}
			}
			if (index == length)
				goto ok_exit;
		} else
			break;
	}
	if (start != inbuf)
		g_free (start);
	start = NULL;
ok_exit:
	g_object_unref (file);
	close (sock);
	return start;
}

char* Application::ConvertToCML (GsfInput *input, const char *mime_type, const char *options)
{
	int sock = OpenBabelSocket ();
	if (sock <= 0)
		return NULL;
	size_t n = gsf_input_size (input);
	char const *outbuf = reinterpret_cast <char const *> (gsf_input_read (input, n, NULL)); 
	std::string buf = "-i ";
	buf += MimeToBabelType (mime_type);
	buf += " -o cml";
	if (options) {
		buf += " ";
		buf += options;
	}
	char *szsize = g_strdup_printf (" -l %lu -D", n);
	buf += szsize;
	g_free (szsize);
	write (sock, buf.c_str (), buf.length ());
	write (sock, outbuf, n);
	time_t timeout = time (NULL) + 60;
	char inbuf[256], *start = inbuf, *end;
	unsigned cur, index = 0, length = 0;
	while (time (NULL) < timeout) {
		if ((cur = read (sock, start + index, ((length)? length: 255) - index))) {
			index += cur;
			start[index] = 0;
			if (start == inbuf) {
				if ((end = strchr (inbuf, ' '))) {
					length = strtoul (inbuf, NULL, 10);
					start = reinterpret_cast <char *> (g_malloc (length + 1));
					if (!start)
						break;
					strcpy (start, end + 1);
					index = strlen (start);
				}
			}
			if (index == length)
				goto ok_exit;
		} else
			break;
	}
	if (start != inbuf)
		g_free (start);
	start = NULL;
ok_exit:
	close (sock);
	return start;
}

void Application::ConvertFromCML (char const *cml, std::string const &uri, const char *mime_type, const char *options)
{
	int sock = OpenBabelSocket ();
	if (sock <= 0)
		return;
	GVfs *vfs = g_vfs_get_default ();
	GFile *file = g_vfs_get_file_for_uri (vfs, uri.c_str ());
	char *path = g_file_get_path (file);
	std::ostringstream os;
	size_t l = strlen (cml);
	os << "-i cml -o ";
	os << MimeToBabelType (mime_type);
	if (path) {
		os << " " << path;
		if (options)
			os << " " << options;
		os << " -l" << l << " -D";
		write (sock, os.str ().c_str (), os.str ().length ());
		write (sock, cml, l);
		g_free (path);
		g_object_unref (file);
		return; // no need to wait, direct output
	}
	// if we are there, this means that we have a distant file to write
	os << "" -l << l << " -D";
	write (sock, os.str ().c_str (), os.str ().length ());
	write (sock, cml, l);
	time_t timeout = time (NULL) + 60;
	char inbuf[256], *start = inbuf, *end;
	unsigned cur, index = 0, length = 0;
	while (time (NULL) < timeout) {
		if ((cur = read (sock, start + index, ((length)? length: 255) - index))) {
			index += cur;
			start[index] = 0;
			if (start == inbuf) {
				if ((end = strchr (inbuf, ' '))) {
					length = strtoul (inbuf, NULL, 10);
					start = reinterpret_cast <char *> (g_malloc (length + 1));
					if (!start)
						break;
					strcpy (start, end + 1);
					index = strlen (start);
				}
			}
			if (index == length)
				break;
		} else // something failed, we should post an error message
			goto exit_point;
	}
	// save to distant file
exit_point:
	if (start != inbuf)
		g_free (start);
	g_object_unref (file);
	close (sock);
}

void Application::ConvertFromCML (char const *cml, GsfOutput *output, const char *mime_type, const char *options)
{
	int sock = OpenBabelSocket ();
	if (sock <= 0)
		return;
	std::ostringstream os;
	size_t l = strlen (cml);
	os << "-i cml -o ";
	os << MimeToBabelType (mime_type);
	if (options)
		os << " " << options;
	os << " -l " << l << " -D";
	write (sock, os.str ().c_str (), os.str ().length ());
	write (sock, cml, l);
	time_t timeout = time (NULL) + 60;
	char inbuf[256], *start = inbuf, *end;
	int cur, index = 0, length = 0;
	while (time (NULL) < timeout) {
		if ((cur = read (sock, start + index, ((length)? length: 255) - index)) > 0) {
			index += cur;
			start[index] = 0;
			if (start == inbuf) {
				if ((end = strchr (inbuf, ' '))) {
					length = strtoul (inbuf, NULL, 10);
					start = reinterpret_cast <char *> (g_malloc (length + 1));
					if (!start)
						break;
					strcpy (start, end + 1);
					index = strlen (start);
				}
			}
			if (index == length) {
				gsf_output_write (output, length, reinterpret_cast <guint8 *> (start));
				break;
			}
		} else // something failed, we should post an error message
			break;
	}
}

}	//	namespace gcu
