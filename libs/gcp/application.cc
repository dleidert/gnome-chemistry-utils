// -*- C++ -*-

/* 
 * GChemPaint library
 * application.cc 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "electron.h"
#include "text.h"
#include "plugin.h"
#include "mesomer.h"
#include "mesomery.h"
#include "mesomery-arrow.h"
#include "newfiledlg.h"
#include "reaction.h"
#include "reactant.h"
#include "reaction-step.h"
#include "reaction-arrow.h"
#include "reaction-prop.h"
#include "settings.h"
#include "theme.h"
#include "tool.h"
#include "tools.h"
#include "target.h"
#include "window.h"
#include "zoomdlg.h"
#include <gcu/filechooser.h>
#include <goffice/utils/go-file.h>
#include <sys/stat.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs-mime.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-mime-info.h>
#include <glib/gi18n-lib.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <clocale>
#include <fstream>

// following code is needed to get file extensions, it as been essentially copied from gnome-vfs
static map<string, list<string> > globs;

static void load_globs_from_dir (char const *directory)
{
  char *file_name = (char*) malloc (strlen (directory) + strlen ("/mime/globs") + 1);
  struct stat st;
  strcpy (file_name, directory); strcat (file_name, "/mime/globs");
  if (stat (file_name, &st) == 0)
    {
  FILE *glob_file;
  char line[255];

  glob_file = fopen (file_name, "r");

  if (glob_file == NULL)
    return;

  /* FIXME: Not UTF-8 safe.  Doesn't work if lines are greater than 255 chars.
   * Blah */
  while (fgets (line, 255, glob_file) != NULL)
    {
      char *colon;
      if (line[0] == '#')
	continue;

      colon = strchr (line, ':');
      if (colon == NULL)
	continue;
      *(colon++) = '\000';
      colon[strlen (colon) -1] = '\000';
      colon = strchr (colon, '.');
       if (colon == NULL)
		continue;
     colon++;
      if (!*colon)
      	continue;
      globs[line].push_back (colon);
    }

  fclose (glob_file);
    }
    free (file_name);
}

static void load_globs ()
{
  const char *xdg_data_home;
  const char *xdg_data_dirs;
  const char *ptr;

  xdg_data_home = getenv ("XDG_DATA_HOME");
  if (xdg_data_home)
    {
      load_globs_from_dir (xdg_data_home);
    }
  else
    {
      const char *home;

      home = getenv ("HOME");
      if (home != NULL)
	{
	  char *guessed_xdg_home;

	  guessed_xdg_home = (char*) malloc (strlen (home) + strlen ("/.local/share/") + 1);
	  strcpy (guessed_xdg_home, home);
	  strcat (guessed_xdg_home, "/.local/share/");
	  load_globs_from_dir (guessed_xdg_home);
	  free (guessed_xdg_home);
	}
    }

  xdg_data_dirs = getenv ("XDG_DATA_DIRS");
  if (xdg_data_dirs == NULL)
    xdg_data_dirs = "/usr/local/share/:/usr/share/";

  ptr = xdg_data_dirs;

  while (*ptr != '\000')
    {
      const char *end_ptr;
      char *dir;
      int len;
 
      end_ptr = ptr;
      while (*end_ptr != ':' && *end_ptr != '\000')
	end_ptr ++;

      if (end_ptr == ptr)
	{
	  ptr++;
	  continue;
	}

      if (*end_ptr == ':')
	len = end_ptr - ptr;
      else
	len = end_ptr - ptr + 1;
      dir = (char*) malloc (len + 1);
      strncpy (dir, ptr, len);
      dir[len] = '\0';
      load_globs_from_dir (dir);
      free (dir);

      ptr = end_ptr;
    }
}

namespace gcp {

// Objects creation static methods
static Object* CreateMolecule ()
{
	return new Molecule ();
}

static Object* CreateReaction ()
{
	return new Reaction ();
}

static Object* CreateReactant ()
{
	return new Reactant ();
}

static Object* CreateReactionStep ()
{
	return new ReactionStep ();
}

static Object* CreateReactionArrow ()
{
	return new ReactionArrow (NULL);
}

static Object* CreateReactionProp ()
{
	return new ReactionProp ();
}

static Object* CreateMesomery ()
{
	return new Mesomery ();
}

static Object* CreateMesomeryArrow ()
{
	return new MesomeryArrow (NULL);
}

static Object* CreateMesomer ()
{
	return new Mesomer ();
}

static Object* CreateText ()
{
	return new Text ();
}

static Object* CreateFragment ()
{
	return new Fragment ();
}

bool	Application::m_bInit = false;
bool	Application::m_Have_Ghemical = false;
bool	Application::m_Have_InChI = false;

// FIXME: Remove for stable versions
#undef PACKAGE
#define PACKAGE "gchemutils-unstable" 

static void on_config_changed (GConfClient *client, guint cnxn_id, GConfEntry *entry, Application *app)
{
	app->OnConfigChanged (client, cnxn_id, entry);
}

Application::Application ():
	gcu::Application ("GChemPaint", DATADIR, "gchempaint-unstable", "gchempaint")
{
	m_CurZ = 6;
	m_pActiveDoc = NULL;
	m_pActiveTool = NULL;
	m_NumWindow = 1;

	if (!m_bInit) {
		// Check for programs
		char *result = NULL, *errors = NULL;
		// check for ghemical
		m_Have_Ghemical = (g_spawn_command_line_sync ("which ghemical", &result, &errors, NULL, NULL)
			&& result && strlen (result));
		if (result) {
			g_free (result);
			result = NULL;
		}
		if (errors) {
			g_free (errors);
			errors = NULL;
		}
		OBConversion Conv;
		m_Have_InChI = Conv.FindFormat ("inchi") != NULL || 
			(g_spawn_command_line_sync ("which main_inchi", &result, &errors, NULL, NULL)
			&& result && strlen (result));
		if (result)
			g_free (result);
		if (errors) {
			g_free (errors);
			errors = NULL;
		}

		// Initialize types
		Object::AddType ("molecule", CreateMolecule, MoleculeType);
		Object::AddType ("reaction", CreateReaction, ReactionType);
		Object::SetCreationLabel (ReactionType, _("Create a new reaction"));
		ReactionStepType = Object::AddType ("reaction-step", CreateReactionStep);
		Object::AddType ("reactant", CreateReactant, ReactantType);
		Object::AddType ("reaction-arrow", CreateReactionArrow, ReactionArrowType);
		ReactionPropType = Object::AddType ("reaction-prop", CreateReactionProp);
		MesomerType = Object::AddType ("mesomer", CreateMesomer);
		Object::AddType ("mesomery", CreateMesomery, MesomeryType);
		Object::SetCreationLabel (MesomeryType, _("Create a new mesomery relationship"));
		Object::AddType ("mesomery-arrow", CreateMesomeryArrow, MesomeryArrowType);
		Object::AddType ("text", CreateText, TextType);
		Object::AddType ("fragment", CreateFragment, FragmentType);
		ElectronType = Object::AddType ("electron", NULL);
		// Add rules
		Object::AddRule ("reaction", RuleMustContain, "reaction-step");
		Object::AddRule ("reaction-step", RuleMustContain, "reactant");
		Object::AddRule ("reactant", RuleMustBeIn, "reaction-step");
		Object::AddRule ("reaction-step", RuleMustBeIn, "reaction");
		Object::AddRule ("reaction", RuleMustContain, "reaction-arrow");
		Object::AddRule ("reaction-arrow", RuleMustBeIn, "reaction");
		Object::AddRule ("reaction-arrow", RuleMayContain, "reaction-prop");
		Object::AddRule ("reaction-prop", RuleMustBeIn, "reaction-arrow");
		Object::AddRule ("reaction-prop", RuleMayContain, "molecule");
		Object::AddRule ("reaction-prop", RuleMayContain, "text");
		Object::AddRule ("reactant", RuleMayContain, "molecule");
		Object::AddRule ("mesomer", RuleMustContain, "molecule");
		Object::AddRule ("mesomer", RuleMustBeIn, "mesomery");
		Object::AddRule ("mesomery", RuleMustContain, "mesomer");
		Object::AddRule ("mesomery", RuleMustContain, "mesomery-arrow");
		Object::AddRule ("mesomery-arrow", RuleMustBeIn, "mesomery");

		// Create global signal ids
		OnChangedSignal = Object::CreateNewSignalId ();
		OnDeleteSignal = Object::CreateNewSignalId ();
		OnThemeChangedSignal = Object::CreateNewSignalId ();

		Plugin::LoadPlugins ();
		m_bInit = true;
	}
	RadioActions = NULL;
	m_entries = 0;
	IconFactory = gtk_icon_factory_new ();
	set<Plugin*>::iterator i = Plugins.begin (), end = Plugins.end ();
	while (i != end) (*i++)->Populate (this);
	gtk_icon_factory_add_default (IconFactory);
	g_object_unref (G_OBJECT (IconFactory));
	XmlDoc = xmlNewDoc ((xmlChar const*) "1.0");
	visible_windows = 0;
	load_globs ();
	m_SupportedMimeTypes.push_back ("application/x-gchempaint");
	m_WriteableMimeTypes.push_back ("application/x-gchempaint");
	// test if OpenBabel supports some extra types
	TestSupportedType ("chemical/x-cml");
	TestSupportedType ("chemical/x-mdl-molfile");
	TestSupportedType ("chemical/x-pdb");
	TestSupportedType ("chemical/x-xyz");
	TestSupportedType ("chemical/x-cdx");
	TestSupportedType ("chemical/x-cdxml");
	TestSupportedType ("chemical/x-ncbi-asn1");
	TestSupportedType ("chemical/x-ncbi-asn1-binary");
	TestSupportedType ("chemical/x-ncbi-asn1-xml");
	// now read extra types declared by the user
	char *home = getenv ("HOME");
	if (home) {
		string path = home;
		path += "/.gchempaint/mime-types";
		char line[255];
		ifstream f (path.c_str ());
		while (!f.fail ()) {
			f.getline (line, 255);
			if (*line)
				TestSupportedType (line);
		}
	}
	
	m_ConfClient = gconf_client_get_default ();
	gconf_client_add_dir (m_ConfClient, "/apps/gchempaint/settings", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	GError *error = NULL;
	GCU_GCONF_GET (ROOTDIR"compression", int, CompressionLevel, 0)
	GCU_GCONF_GET (ROOTDIR"tearable-mendeleiev", bool, TearableMendeleiev, false)
	bool CopyAsText;
	GCU_GCONF_GET (ROOTDIR"copy-as-text", bool, CopyAsText, false)
	ClipboardFormats = CopyAsText? GCP_CLIPBOARD_ALL: GCP_CLIPBOARD_NO_TEXT;
	m_NotificationId = gconf_client_notify_add (m_ConfClient, "/apps/gchempaint/settings", (GConfClientNotifyFunc) on_config_changed, this, NULL, NULL);
	// make themes permanent with this as a dummy client
	list <string> Names = TheThemeManager.GetThemesNames ();
	list <string>::iterator j, jend = Names.end ();
	Theme *pTheme;
	m_Dummy = new Object (0);
	for (j = Names.begin (); j != jend; j++) {
		pTheme = TheThemeManager.GetTheme ("Default");
		pTheme->AddClient (m_Dummy);
	}
}

Application::~Application ()
{
	map<string, Tool*>::iterator tool = m_Tools.begin(), endtool = m_Tools.end();
	for (; tool!= endtool; tool++)
		delete (*tool).second;
	m_Tools.clear ();
	if (XmlDoc)
		xmlFreeDoc (XmlDoc);
	m_SupportedMimeTypes.clear ();
	delete m_Dummy;
}

void Application::ActivateTool (const string& toolname, bool activate)
{
	if (m_Tools[toolname]) {
		if (activate) {
			if (m_pActiveTool != m_Tools[toolname]) {
				if (m_pActiveTool)
					m_pActiveTool->Activate (false);
				m_pActiveTool = m_Tools[toolname];
				m_pActiveTool->Activate (true);
				GtkToggleToolButton* button = (GtkToggleToolButton*) ToolItems[toolname];
				if (button && !gtk_toggle_tool_button_get_active (button))
					gtk_toggle_tool_button_set_active (button, true);
			}
		} else {
			if (m_pActiveTool == m_Tools[toolname])
				m_pActiveTool = NULL;
			m_Tools[toolname]->Activate (false);
		}
	}
}

void Application::ClearStatus ()
{
	if (m_pActiveDoc) {
		Window *Win = m_pActiveDoc->GetWindow ();
		if (Win)
			Win->ClearStatus ();
	}
}

void Application::SetStatusText (const char* text)
{
	if (m_pActiveDoc) {
		Window *Win = m_pActiveDoc->GetWindow ();
		if (Win)
			Win->SetStatusText (text);
	}
}

void Application::OnSaveAs ()
{
	FileChooser (this, true, m_WriteableMimeTypes, m_pActiveDoc);
}

enum {
	CHEMISTRY,
	SVG,
	EPS,
	PIXBUF
};

bool Application::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, gcu::Document *Doc)
{
	const gchar* ext;
	Document *pDoc = static_cast<Document*> (Doc);
	if (!filename || !strlen(filename) || filename[strlen(filename) - 1] == '/')
	{
		GtkWidget* message = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
															_("Please enter a file name,\nnot a directory"));
		gtk_window_set_icon_name (GTK_WINDOW (message), "gchempaint");
		gtk_dialog_run (GTK_DIALOG (message));
		gtk_widget_destroy (message);
		return true;
	}
	int file_type = -1;
	int n = strlen (filename), i = n - 1;
	char const *pixbuf_type = NULL;
	string filename2 = filename;
	while ((i > 0) && (filename[i] != '.') && (filename[i] != '/')) i--;
	if (filename[i] == '/') i = 0;
	ext = (i > 0)? filename + i + 1: NULL;
	list<string>::iterator it, itend = m_SupportedMimeTypes.end ();
	for (it =  m_SupportedMimeTypes.begin (); it != itend; it++)
		if (*it == mime_type) {
			file_type = CHEMISTRY;
			break;
		}
	if (file_type != CHEMISTRY) {
		if (!strcmp (mime_type, "image/svg+xml"))
			file_type = SVG;
		else if (!strcmp (mime_type, "image/x-eps"))
			file_type = EPS;
		else if ((pixbuf_type = GetPixbufTypeName (filename2, mime_type))) {
			file_type = PIXBUF;
			if (!ext) {
				filename = filename2.c_str ();
				i = strlen (filename) - 1;
				while ((i > 0) && (filename[i] != '.') && (filename[i] != '/')) i--;
				if (filename[i] == '/') i = 0;
				ext = (i > 0)? filename + i + 1: NULL;
			}
		}
	}
	if (file_type < 0 || (!bSave && (file_type > CHEMISTRY))) {
		GtkWidget* message = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
													_("Sorry, format not supported!"));
		gtk_dialog_run (GTK_DIALOG (message));
		gtk_widget_destroy (message);
		return true;
	}
	list<string> &exts = globs[mime_type];
	bool err;
	GnomeVFSURI *uri;
	if (bSave) {
		char const *default_ext = (exts.size ())? exts.front ().c_str (): NULL;
		if (ext) {
			list<string>::iterator cur, end = exts.end ();
			for (cur = exts.begin (); cur != end; cur++)
				if (*cur != ext) {
					default_ext = ext;
					break;
				}
			if (default_ext && strcmp (ext, default_ext))
				ext = NULL;
		}
		if (default_ext && !ext)
				filename2 += string(".") + default_ext;
		uri = gnome_vfs_uri_new (filename2.c_str ());
		err = gnome_vfs_uri_exists (uri);
		gnome_vfs_uri_unref (uri);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), filename2.c_str ());
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
			gtk_window_set_icon_name (GTK_WINDOW (Box), "gchempaint");
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
		}
		if (result == GTK_RESPONSE_YES) {
			// destroy the old file
			gnome_vfs_unlink (filename2.c_str ());
			switch (file_type) {
			case SVG:
				m_pActiveDoc->ExportImage (filename2, "svg");
				break;
			case EPS:
				m_pActiveDoc->ExportImage (filename2, "eps");
				break;
			case PIXBUF:
				m_pActiveDoc->ExportImage (filename2, pixbuf_type, GetImageResolution ());
				break;
			default:
				if (!strcmp (mime_type, "application/x-gchempaint"))
					SaveGcp (filename2, pDoc);
				else
					SaveWithBabel (filename2, mime_type, pDoc);
			}
		}
	} else  { //loading
		uri = gnome_vfs_uri_new (filename);
		err = !gnome_vfs_uri_exists (uri);
		gnome_vfs_uri_unref (uri);
		if (err) {
			if (!ext) {
				list<string>::iterator cur, end = exts.end ();
				for (cur = exts.begin (); cur != end; cur++) {
					filename2 = string (filename) + "." + *cur;
					uri = gnome_vfs_uri_new (filename2.c_str ());
					err = !gnome_vfs_uri_exists (uri);
					gnome_vfs_uri_unref (uri);
					if (!err)
						break;
				}
			}
			if (err) {
				filename2 = filename;
			}
		}
		if (!strcmp (mime_type, "application/x-gchempaint"))
			OpenGcp (filename2, pDoc);
		else
			OpenWithBabel (filename2, mime_type, pDoc);
	}
	return false;
}

void Application::SaveWithBabel (string const &filename, const gchar *mime_type, Document* pDoc)
{
	pDoc->SetFileName (filename, mime_type);
	pDoc->Save ();
	GtkRecentData data;
	data.display_name = (char*) pDoc->GetTitle ();
	data.description = NULL;
	data.mime_type = (char*) mime_type;
	data.app_name = const_cast<char*> ("gchempaint");
	data.app_exec = const_cast<char*> ("gchempaint %u");
	data.groups = NULL;
	data.is_private =  FALSE;
	gtk_recent_manager_add_full (GetRecentManager (), filename.c_str (), &data);
}

void Application::OpenWithBabel (string const &filename, const gchar *mime_type, Document* pDoc)
{
	char *old_num_locale;
	bool bNew = (pDoc == NULL || (!pDoc->GetEmpty () && ! pDoc->GetDirty ())), local;
	GnomeVFSFileInfo *info = NULL;
	bool result = false, read_only = false;
	try {
		if (!filename.length ())
			throw (int) 0;
		info = gnome_vfs_file_info_new ();
		gnome_vfs_get_file_info (filename.c_str (), info, GNOME_VFS_FILE_INFO_DEFAULT);
		local = GNOME_VFS_FILE_INFO_LOCAL (info);
		if (!(info->permissions & (GNOME_VFS_PERM_USER_WRITE | GNOME_VFS_PERM_GROUP_WRITE)))
			read_only = true;
		gnome_vfs_file_info_unref (info);
		if (bNew) {
			OnFileNew ();
			pDoc = m_pActiveDoc;
		}
		if (local) {
			ifstream ifs;
			GnomeVFSURI *uri = gnome_vfs_uri_new (filename.c_str ());
			ifs.open (gnome_vfs_uri_get_path (uri));
			gnome_vfs_uri_unref (uri);
			if (ifs.fail ())
				throw (int) 1;
			old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
			setlocale(LC_NUMERIC, "C");
			OBMol Mol;
			OBConversion Conv;
			OBFormat* pInFormat = Conv.FormatFromMIME (mime_type);
			if (pInFormat == NULL)
				throw 1;
			Conv.SetInFormat (pInFormat);
			while (!ifs.eof () && Conv.Read (&Mol, &ifs)) {
				result = pDoc->ImportOB(Mol);
				Mol.Clear ();
				if (!result)
					break;
			}
			setlocale (LC_NUMERIC, old_num_locale);
			g_free (old_num_locale);
			ifs.close ();
		} else {
			char *buf;
			int size;
			if (gnome_vfs_read_entire_file (filename.c_str (), &size, &buf) != GNOME_VFS_OK)
				throw 1;
			istringstream iss (buf);
			old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
			setlocale(LC_NUMERIC, "C");
			OBMol Mol;
			OBConversion Conv;
			OBFormat* pInFormat = Conv.FormatFromExt (filename.c_str ());
			if (pInFormat == NULL)
				throw 1;
			Conv.SetInFormat (pInFormat);
			while (!iss.eof () && Conv.Read (&Mol, &iss)) {
				result = pDoc->ImportOB(Mol);
				Mol.Clear ();
				if (!result)
					break;
			}
			setlocale (LC_NUMERIC, old_num_locale);
			g_free (old_num_locale);
			g_free (buf);
		}
		if (!result)
		{
			if (bNew)
				pDoc->GetWindow ()->Destroy ();
			throw (int) 2;
		}
		pDoc->SetFileName (filename, mime_type);
		pDoc->SetReadOnly (read_only);
		double l = pDoc->GetMedianBondLength ();
		if (l > 0.) {
			double r = pDoc->GetBondLength () / l;
			if (fabs (r - 1.) > .0001) { 
				Matrix2D m (r, 0., 0., r);
				// FIXME: this would not work for reactions
				pDoc->Transform2D (m, 0., 0.);
			}
		}
		View *pView = pDoc->GetView ();
		pView->Update (pDoc);
		pDoc->Update ();
		pView->EnsureSize ();
		Window *win = pDoc->GetWindow ();
		if (win)
			win->SetTitle (pDoc->GetTitle ());
		GtkRecentData data;
		data.display_name = (char*) pDoc->GetTitle ();
		data.description = NULL;
		data.mime_type = (char*) mime_type;
		data.app_name = const_cast<char*> ("gchempaint");
		data.app_exec = const_cast<char*> ("gchempaint %u");
		data.groups = NULL;
		data.is_private =  FALSE;
		gtk_recent_manager_add_full (GetRecentManager (), filename.c_str (), &data);
	}
	catch (int num)
	{
		gchar *mess = NULL;
		GtkWidget* message;
		switch (num)
		{
		case 0:
			mess = g_strdup_printf(_("No filename"));
			break;
		case 1:
			mess = g_strdup_printf(_("Could not open file\n%s"), filename.c_str ());
			break;
		case 2:
			mess = g_strdup_printf(_("%s: parse error."), filename.c_str ());
			break;
		default:
			throw (num); //this should not occur
		}
		message = gtk_message_dialog_new(NULL, (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, mess);
		gtk_window_set_icon_name (GTK_WINDOW (message), "gchempaint");
		g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
		gtk_widget_show(message);
		g_free(mess);
	}
}

void Application::SaveGcp (string const &filename, Document* pDoc)
{
	pDoc->SetFileName (filename, "application/x-gchempaint");
	pDoc->Save ();
	GtkRecentData data;
	data.display_name = (char*) pDoc->GetTitle ();
	data.description = NULL;
	data.mime_type = const_cast<char*> ("application/x-gchempaint");
	data.app_name = const_cast<char*> ("gchempaint");
	data.app_exec = const_cast<char*> ("gchempaint %u");
	data.groups = NULL;
	data.is_private =  FALSE;
	gtk_recent_manager_add_full (GetRecentManager (), filename.c_str (), &data);
}

static int	cb_vfs_to_xml (GnomeVFSHandle *handle, char* buf, int nb)
{
	GnomeVFSFileSize ndone;
	return  (gnome_vfs_read (handle, buf, nb, &ndone) == GNOME_VFS_OK)? (int) ndone: -1;
}

void Application::OpenGcp (string const &filename, Document* pDoc)
{
	xmlDocPtr xml = NULL;
	char *old_num_locale, *old_time_locale;
	GnomeVFSFileInfo *info = NULL;
	bool create = false;
	try
	{
		if (!filename.length ())
			throw (int) 0;

		// try opening with write access to see if it is readonly
		// use xmlReadIO for non local files.
		info = gnome_vfs_file_info_new ();
		gnome_vfs_get_file_info (filename.c_str (), info, GNOME_VFS_FILE_INFO_DEFAULT);

		if (GNOME_VFS_FILE_INFO_LOCAL (info)) {
			if (!(xml = xmlParseFile(filename.c_str ())))
				throw (int) 1;
		} else {
			GnomeVFSHandle *handle;
			GnomeVFSResult result = gnome_vfs_open (&handle, filename.c_str (), GNOME_VFS_OPEN_READ);
			if (result != GNOME_VFS_OK)
				throw 1;
			if (!(xml = xmlReadIO ((xmlInputReadCallback) cb_vfs_to_xml, 
					 (xmlInputCloseCallback) gnome_vfs_close, handle, filename.c_str (), NULL, 0)))
				throw 1;
		}
		if (xml->children == NULL) throw (int) 2;
		if (strcmp((char*)xml->children->name, "chemistry")) throw (int) 3;	//FIXME: that could change when a dtd is available
		old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
		setlocale(LC_NUMERIC, "C");
		old_time_locale = g_strdup(setlocale(LC_TIME, NULL));
		setlocale(LC_TIME, "C");
		if (!pDoc || !pDoc->GetEmpty () || pDoc->GetDirty ()) {
			create = true;
			OnFileNew ();
			pDoc = m_pActiveDoc;
		}
		pDoc->SetFileName(filename, "application/x-gchempaint");
		bool result = pDoc->Load(xml->children);
		setlocale(LC_NUMERIC, old_num_locale);
		g_free(old_num_locale);
		setlocale(LC_TIME, old_time_locale);
		g_free(old_time_locale);
		if (!result) {
			if (create)
				pDoc->GetWindow ()->Destroy ();
			throw (int) 4;
		}
		if (!(info->permissions & (GNOME_VFS_PERM_USER_WRITE | GNOME_VFS_PERM_GROUP_WRITE)))
			pDoc->SetReadOnly (true);
		gnome_vfs_file_info_unref (info);
		xmlFreeDoc(xml);
		GtkRecentData data;
		data.display_name = (char*) pDoc->GetTitle ();
		data.description = NULL;
		data.mime_type = const_cast<char*> ("application/x-gchempaint");
		data.app_name = const_cast<char*> ("gchempaint");
		data.app_exec = const_cast<char*> ("gchempaint %u");
		data.groups = NULL;
		gtk_recent_manager_add_full (GetRecentManager (), filename.c_str (), &data);
	}
	catch (int num)
	{
		if (info)
			gnome_vfs_file_info_unref (info);
		if (num > 1) xmlFreeDoc(xml);
		gchar *mess = NULL;
		GtkWidget* message;
		switch (num)
		{
		case 0:
			mess = g_strdup_printf(_("No filename"));
			break;
		case 1:
			mess = g_strdup_printf(_("Could not load file\n%s"), filename.c_str ());
			break;
		case 2:
			mess = g_strdup_printf(_("%s: invalid xml file.\nTree is empty?"), filename.c_str ());
			break;
		case 3:
			mess = g_strdup_printf(_("%s: invalid file format."), filename.c_str ());
			break;
		case 4:
			mess = g_strdup_printf(_("%s: parse error."), filename.c_str ());
			break;
		default:
			throw (num); //this should not occur
		}
		message = gtk_message_dialog_new(NULL, (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, mess);
		gtk_window_set_icon_name (GTK_WINDOW (message), "gchempaint");
		g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
		gtk_widget_show(message);
		g_free(mess);
	}
}

void Application::InitTools()
{
	map<string, Tool*>::iterator i = m_Tools.begin (), end = m_Tools.end ();
	for (; i != end; i++)
		if ((*i).second)
			(*i).second->Activate ((*i).first == "Select");
}

void Application::OnSaveAsImage ()
{
	if (!m_pActiveDoc)
		return;
	list<string> l;
	map<string, GdkPixbufFormat*>::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	l.push_front ("image/x-eps");
	l.push_front ("image/svg+xml");
	FileChooser (this, true, l, m_pActiveDoc, _("Save as image"), GetImageResolutionWidget ());
}

void Application::Zoom (double zoom)
{
	View *pView = m_pActiveDoc->GetView ();
	// authorized zooms: 20% to 800% all other values will open the zoom dialog.
	if (zoom >= 0.2 && zoom <= 8.)
		pView->Zoom (zoom);
	else {
		Dialog *pDialog = GetDialog ("Zoom");
		if (pDialog)
			gtk_window_present (pDialog->GetWindow ()); 
		else
			new ZoomDlg (m_pActiveDoc);
	}
}

static void on_tool_changed (GtkAction *action, GtkAction *current, Application* App)
{
	App->OnToolChanged (current);
}

void Application::OnToolChanged (GtkAction *current)
{
	if (m_pActiveTool)
		m_pActiveTool->Activate(false);
	m_pActiveTool = m_Tools[gtk_action_get_name (current)];
	Tools *ToolsBox = dynamic_cast<Tools*> (GetDialog ("tools"));
	if (ToolsBox)
		ToolsBox->OnSelectTool (m_pActiveTool);		
	if (m_pActiveTool)
		m_pActiveTool->Activate(true);
}

void Application::BuildTools ()
{
	Tools *ToolsBox = new Tools (this);
	map<int, string>::iterator i, iend = ToolbarNames.end ();
	list<char const*>::iterator j, jend = UiDescs.end ();
	string s;
	GError *error = NULL;
	GtkUIManager *ToolsManager = gtk_ui_manager_new ();
	ToolsBox->SetUIManager (ToolsManager);
	GtkActionGroup *action_group = gtk_action_group_new ("Tools");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_radio_actions (action_group, RadioActions, m_entries, 0, G_CALLBACK (on_tool_changed), this);
	gtk_ui_manager_insert_action_group (ToolsManager, action_group, 0);
	for (j = UiDescs.begin (); j != jend; j++)
		if (!gtk_ui_manager_add_ui_from_string (ToolsManager, *j, -1, &error)) {
			g_message ("building user interface failed: %s", error->message);
			g_error_free (error);
			exit (EXIT_FAILURE);
		}
	for (i = ToolbarNames.begin (); i != iend; i++) {
		s = "ui/";
		s += (*i).second;
		ToolsBox->AddToolbar (s);
	}
	g_object_unref (ToolsManager);
	m_pActiveTool = m_Tools["Select"];
	if (m_pActiveTool)
		m_pActiveTool->Activate(true);
	ToolsBox->OnSelectTool (m_pActiveTool);	
	ToolsBox->OnElementChanged (m_CurZ);	
}

void Application::ShowTools (bool visible)
{
	Tools *ToolsBox = dynamic_cast<Tools*> (GetDialog ("tools"));
	if (!ToolsBox) {
		if (visible)
			BuildTools ();
	} else
	ToolsBox->Show (visible);
}

void Application::AddActions (GtkRadioActionEntry const *entries, int nb, char const *ui_description, IconDesc const *icons)
{
	static int cur_entry = 1;
	if (nb > 0) {
		if (m_entries)
			RadioActions = g_renew (GtkRadioActionEntry, RadioActions, m_entries + nb);
		else
			RadioActions = g_new (GtkRadioActionEntry, nb);
		memcpy (RadioActions + m_entries, entries, nb * sizeof (GtkRadioActionEntry));
		for (int i = 0; i < nb; i++)
			if (strcmp (RadioActions[i + m_entries].name, "Select"))
				RadioActions[i + m_entries].value = cur_entry++;
			else
				RadioActions[i + m_entries].value = 0;
		m_entries += nb;
	}
	if (ui_description)
		UiDescs.push_back (ui_description);
	if (icons) {
		GtkIconSet *set;
		GtkIconSource *src;
		while (icons->name) {
			set = gtk_icon_set_new ();
			src = gtk_icon_source_new ();
		
			gtk_icon_source_set_size_wildcarded (src, TRUE);
			gtk_icon_source_set_pixbuf (src,
				gdk_pixbuf_new_from_inline (-1, icons->data_24, FALSE, NULL));
			gtk_icon_set_add_source (set, src);	/* copies the src */
		
			gtk_icon_factory_add (IconFactory, icons->name, set);	/* keeps reference to set */
			gtk_icon_set_unref (set);
			gtk_icon_source_free (src);
			icons++;
		}
	}
}

void Application::RegisterToolbar (char const *name, int index)
{
	if (ToolbarNames[index] == "")
		ToolbarNames[index] = name;
}

void Application::AddTarget (Target *target)
{
	m_Targets.insert (target);
	NotifyIconification (false);
}

void Application::DeleteTarget (Target *target)
{
	m_Targets.erase (target);
	ShowTools (false);
}

void Application::NotifyIconification (bool iconified)
{
	if (iconified) {
		ShowTools (false);
	}
}

void Application::NotifyFocus (bool has_focus, Target *target)
{
	if (target) {
		m_pActiveTarget = target;
		m_pActiveDoc = target->GetDocument ();
		m_pActiveTool->Activate ();
		if (has_focus)
			ShowTools (true);
	}
}

void Application::CloseAll ()
{
	while (!m_Targets.empty ())
		if (!(*m_Targets.begin ())->Close ())
			return;
}

void Application::ActivateWindowsActionWidget (const char *path, bool activate)
{
	std::set<Target*>::iterator i, iend = m_Targets.end ();
	for (i = m_Targets.begin (); i != iend; i++) {
		Window *window = dynamic_cast<Window*> (*i);
		if (window)
			window->ActivateActionWidget (path, activate);
	}
}

void Application::OnConfigChanged (GConfClient *client, guint cnxn_id, GConfEntry *entry)
{
	if (client != m_ConfClient)
		return;	// we might want an error message?
	if (cnxn_id != m_NotificationId)
		return;	// we might want an error message?
	if (!strcmp (gconf_entry_get_key (entry),ROOTDIR"compression")) {
		CompressionLevel = gconf_value_get_int (gconf_entry_get_value (entry));
	} else 	if (!strcmp (gconf_entry_get_key (entry),ROOTDIR"tearable-mendeleiev")) {
		TearableMendeleiev = gconf_value_get_bool (gconf_entry_get_value (entry));
		Tools *ToolsBox = dynamic_cast<Tools*> (GetDialog ("tools"));
		if (ToolsBox)
			ToolsBox->Update ();
	} else 	if (!strcmp (gconf_entry_get_key (entry),ROOTDIR"copy-as-text"))
		ClipboardFormats = (gconf_value_get_bool (gconf_entry_get_value (entry)))? GCP_CLIPBOARD_ALL: GCP_CLIPBOARD_NO_TEXT;
}

void Application::TestSupportedType (char const *mime_type)
{
	OBFormat *f = OBConversion::FormatFromMIME (mime_type);
	if (f != NULL) {
		m_SupportedMimeTypes.push_back (mime_type);
		if (!(f->Flags () & NOTWRITABLE))
			m_WriteableMimeTypes.push_back (mime_type);
	}
}

list<string> &Application::GetExtensions(string &mime_type)
{
	return globs[mime_type];
}

void Application::OnThemeNamesChanged ()
{
	NewFileDlg *dlg = dynamic_cast <NewFileDlg *> (GetDialog ("newfile"));
	if (dlg)
		dlg->OnThemeNamesChanged ();
	set <gcu::Document*>::iterator i, end = m_Docs.end ();
	for (i = m_Docs.begin (); i != end; i++)
		dynamic_cast <Document *> (*i)->OnThemeNamesChanged ();
}

void Application::AddMenuCallback (BuildMenuCb cb)
{
	m_MenuCbs.push_back (cb);
}

void Application::BuildMenu (GtkUIManager *manager)
{
	list<BuildMenuCb>::iterator i, end = m_MenuCbs.end ();
	for (i = m_MenuCbs.begin (); i != end; i++)
		(*i) (manager);
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

}	//	namespace gcp