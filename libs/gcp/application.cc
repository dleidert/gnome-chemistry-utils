// -*- C++ -*-

/*
 * GChemPaint library
 * application.cc
 *
 * Copyright (C) 2004-2014 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include "atom-residue.h"
#include "bond.h"
#include "brackets.h"
#include "document.h"
#include "electron.h"
#include "fragment.h"
#include "gcp-stock-pixbufs.h"
#include "text.h"
#include "plugin.h"
#include "mechanism-arrow.h"
#include "mechanism-step.h"
#include "mesomer.h"
#include "mesomery.h"
#include "mesomery-arrow.h"
#include "molecule.h"
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
#include "view.h"
#include "window.h"
#include "widgetdata.h"
#include "zoomdlg.h"
#include <gcugtk/ui-manager.h>
#include <gcugtk/filechooser.h>
#include <gcu/cmd-context.h>
#include <gcu/loader.h>
#include <gcu/loader-error.h>
#include <gcu/xml-utils.h>
#include <gccv/canvas.h>
#include <glib/gi18n-lib.h>
#include <fstream>
#include <cstring>
#include <sys/stat.h>

using namespace std;
using namespace gcu;

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

// class ApplicationPrivate: implements useful callbacks
class ApplicationPrivate {
public:
	static bool Init (Application *app);
	static Object* CreateBrackets ();
};

bool ApplicationPrivate::Init (Application *app)
{
#if GTK_CHECK_VERSION(3,20,0)
	// init a default style for background in main widget
	GtkCssProvider *provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_data	(provider, 	"canvas { color: #000000; background-color: white;} button canvas {background-color: rgba(0,0,0,0);}", -1, NULL);
	gtk_style_context_add_provider_for_screen (gdk_screen_get_default (), GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
#endif

	// Check for programs
	char *result = NULL, *errors = NULL;
	// check for ghemical
	app->m_HaveGhemical = (g_spawn_command_line_sync ("which ghemical", &result, &errors, NULL, NULL)
		&& result && strlen (result));
	if (result) {
		g_free (result);
		result = NULL;
	}
	if (errors) {
		g_free (errors);
		errors = NULL;
	}
	app->m_HaveGChem3D = (g_spawn_command_line_sync ("which gchem3d-" GCU_API_VER, &result, &errors, NULL, NULL)
		&& result && strlen (result));
	if (result) {
		g_free (result);
		result = NULL;
	}
	if (errors) {
		g_free (errors);
		errors = NULL;
	}
	app->m_HaveAvogadro = (g_spawn_command_line_sync ("which avogadro", &result, &errors, NULL, NULL)
		&& result && strlen (result));
	if (result) {
		g_free (result);
		result = NULL;
	}
	if (errors) {
		g_free (errors);
		errors = NULL;
	}
	return false;
}

// Objects creation static methods
Object* ApplicationPrivate::CreateBrackets ()
{
	return new Brackets ();
}

static Object* CreateAtom ()
{
	return new Atom ();
}

static Object* CreateResidue ()
{
	return new AtomResidue ();
}

static Object* CreateBond ()
{
	return new Bond ();
}

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

static Object* CreateMechanismArrow ()
{
	return new MechanismArrow ();
}

static Object* CreateMechanismStep ()
{
	return new MechanismStep (MechanismStepType);
}

static Object* CreateElectron ()
{
	return new Electron (NULL, false);
}

bool	Application::m_bInit = false;
bool	Application::m_Have_InChI = false;
bool	Application::m_HaveGhemical = false;
bool	Application::m_HaveGChem3D = false;
bool	Application::m_HaveAvogadro = false;

extern TypeId ReactionSeparatorType;
extern TypeId ReactionSubstractorType;
extern TypeId StepCounterType;

static void on_config_changed (GOConfNode *node, char const *key, Application *app)
{
	app->OnConfigChanged (node, key);
}

Application::Application (gcugtk::CmdContextGtk *cc):
	gcugtk::Application ("GChemPaint", DATADIR, "gchempaint", "gchempaint", cc)
{
	m_CurZ = 6;
	m_pActiveDoc = NULL;
	m_pActiveTool = NULL;
	m_pActiveTarget = NULL;
	m_NumWindow = 1;

	if (!m_bInit) {
		/* Initialize plugins manager */
		gcu::Loader::Init (this);
/*		OBConversion Conv;
		m_Have_InChI = Conv.FindFormat ("inchi") != NULL ||
			(g_spawn_command_line_sync ("which main_inchi", &result, &errors, NULL, NULL)
			&& result && strlen (result));
		if (result)
			g_free (result);
		if (errors) {
			g_free (errors);
			errors = NULL;
		}*/

		// Initialize types
		AddType ("atom", CreateAtom, AtomType);
		AddType ("residue", CreateResidue, ResidueType);
		AddType ("bond", CreateBond, gcu::BondType);
		AddType ("molecule", CreateMolecule, MoleculeType);
		AddType ("reaction", CreateReaction, ReactionType);
		SetCreationLabel (ReactionType, _("Create a new reaction"));
		ReactionStepType = AddType ("reaction-step", CreateReactionStep);
		AddType ("reactant", CreateReactant, ReactantType);
		AddType ("reaction-arrow", CreateReactionArrow, ReactionArrowType);
		AddType ("reaction-operator", NULL, ReactionOperatorType);
		ReactionSeparatorType = AddType ("reaction-separator", NULL, ReactionSeparatorType);
		ReactionSubstractorType = AddType ("reaction-substractor", NULL, ReactionSubstractorType);
		StepCounterType = AddType ("step-counter", NULL, StepCounterType);
		ReactionPropType = AddType ("reaction-prop", CreateReactionProp);
		MesomerType = AddType ("mesomer", CreateMesomer);
		AddType ("mesomery", CreateMesomery, MesomeryType);
		SetCreationLabel (MesomeryType, _("Create a new mesomery relationship"));
		AddType ("mesomery-arrow", CreateMesomeryArrow, MesomeryArrowType);
		AddType ("text", CreateText, TextType);
		AddType ("fragment", CreateFragment, FragmentType);
		ElectronType = AddType ("electron", CreateElectron);
		Object::AddAlias (ElectronType, "electron-pair");
		BracketsType = AddType ("brackets", ApplicationPrivate::CreateBrackets);
		// Add rules
		AddRule ("reaction", RuleMustContain, "reaction-step");
		AddRule ("reaction-step", RuleMustContain, "reactant");
		AddRule ("reactant", RuleMustBeIn, "reaction-step");
		AddRule ("reaction-step", RuleMustBeIn, "reaction");
		AddRule ("reaction", RuleMustContain, "reaction-arrow");
		AddRule ("reaction-arrow", RuleMayContain, "reaction-prop");
		AddRule ("reaction-arrow", RuleMayContain, "reaction-separator");
		AddRule ("reaction-arrow", RuleMayContain, "reaction-substractor");
		AddRule ("reaction-arrow", RuleMayContain, "step-counter");
		AddRule ("reaction-prop", RuleMustBeIn, "reaction-arrow");
		AddRule ("reaction-prop", RuleMayContain, "molecule");
		AddRule ("reaction-prop", RuleMayContain, "text");
		AddRule ("reaction-operator", RuleMustBeIn, "reaction-step");
		AddRule ("reaction-separator", RuleMustBeIn, "reaction-arrow");
		AddRule ("reaction-substractor", RuleMustBeIn, "reaction-arrow");
		AddRule ("step-counter", RuleMustBeIn, "reaction-arrow");
		AddRule ("reactant", RuleMayContain, "molecule");
		AddRule ("reactant", RuleMayContain, "text");
		AddRule ("reactant", RuleMayContain, "mesomery");
		AddRule ("mesomer", RuleMayContain, "molecule");
		AddRule ("mesomer", RuleMustBeIn, "mesomery");
		AddRule ("mesomery", RuleMustContain, "mesomer");
		AddRule ("mesomery", RuleMustContain, "mesomery-arrow");
		MechanismArrowType = AddType ("mechanism-arrow", CreateMechanismArrow);
		MechanismStepType = AddType ("mechanism-step", CreateMechanismStep);
		AddRule ("reaction-step", RuleMayContain, "mechanism-step");
		AddRule ("mesomer", RuleMayContain, "mechanism-step");

		// Create global signal ids
		OnChangedSignal = Object::CreateNewSignalId ();
		OnDeleteSignal = Object::CreateNewSignalId ();
		OnThemeChangedSignal = Object::CreateNewSignalId ();

#if 0
		/* get the theme style for labels so that tools buttons colors might
		be adapted to the current theme */
		GtkWidget *w = gtk_label_new ("");
		GtkStyleContext *ctxt = gtk_widget_get_style_context (w);
		g_object_ref_sink (w);
		g_object_unref (w);
#endif

		// load settings before plugins
		m_ConfNode = go_conf_get_node (GetConfDir (), GCP_CONF_DIR_SETTINGS);
		GCU_GCONF_GET ("compression", int, CompressionLevel, 0)
		GCU_GCONF_GET ("invert-wedge-hashes", bool, InvertWedgeHashes, false)
		GCU_GCONF_GET ("use-atom-colors", bool, m_UseAtomColors, false)
		bool CopyAsText;
		GCU_GCONF_GET ("copy-as-text", bool, CopyAsText, false)
		ClipboardFormats = CopyAsText? GCP_CLIPBOARD_ALL: GCP_CLIPBOARD_NO_TEXT;
		m_NotificationId = go_conf_add_monitor (m_ConfNode, NULL, (GOConfMonitorFunc) on_config_changed, this);

		// load plugins
		Plugin::LoadPlugins ();
		m_bInit = true;
		g_idle_add (reinterpret_cast <GSourceFunc> (ApplicationPrivate::Init), this);
	}
	set<Plugin*>::iterator i = Plugins.begin (), end = Plugins.end ();
	while (i != end) (*i++)->Populate (this);
	XmlDoc = xmlNewDoc ((xmlChar const*) "1.0");
	visible_windows = 0;
	load_globs ();
	m_SupportedMimeTypes.push_back ("application/x-gchempaint");
	m_WriteableMimeTypes.push_back ("application/x-gchempaint");
	// browse available loaders
	map<string, LoaderStruct>::iterator it;
	bool found = Loader::GetFirstLoader (it);
	while (found) {
		if ((*it).second.supports2D) {
			if ((*it).second.read)
				AddMimeType (m_SupportedMimeTypes, (*it).first);
			if ((*it).second.write)
				AddMimeType (m_WriteableMimeTypes, (*it).first);
		}
		found = Loader::GetNextLoader (it);
	}
	// test if OpenBabel supports some extra types
//	TestSupportedType ("chemical/x-mdl-molfile");
	TestSupportedType ("chemical/x-pdb", "pdb", true);
	TestSupportedType ("chemical/x-xyz", "xyz", true);
//	TestSupportedType ("chemical/x-ncbi-asn1");
//	TestSupportedType ("chemical/x-ncbi-asn1-binary");
	TestSupportedType ("chemical/x-ncbi-asn1-xml", "pc", false);
	TestSupportedType ("chemical/x-inchi", "inchi", true);
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

	// make themes permanent with this as a dummy client
	list <string> Names = TheThemeManager.GetThemesNames ();
	list <string>::iterator j, jend = Names.end ();
	Theme *pTheme;
	m_Dummy = new Object (0);
	for (j = Names.begin (); j != jend; j++) {
		pTheme = TheThemeManager.GetTheme (*j);
		pTheme->AddClient (m_Dummy);
	}
	// Create cursors
	GdkPixbuf *unallowed = gdk_pixbuf_new_from_inline (-1, gcp_unallowed, false, NULL);
	m_Cursors[CursorUnallowed] = gdk_cursor_new_from_pixbuf (gdk_display_get_default (), unallowed, 3, 3);
	g_object_unref (unallowed);
	m_Cursors[CursorPencil] = gdk_cursor_new (GDK_PENCIL);
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
	// remove themes permanency with this as a dummy client
	list <string> Names = TheThemeManager.GetThemesNames ();
	list <string>::iterator j, jend = Names.end ();
	Theme *pTheme;
	for (j = Names.begin (); j != jend; j++) {
		pTheme = TheThemeManager.GetTheme (*j);
		pTheme->RemoveClient (m_Dummy);
	}
	delete m_Dummy;
	go_conf_remove_monitor (m_NotificationId);
	go_conf_free_node (m_ConfNode);
	m_ConfNode = NULL;
	TheThemeManager.Shutdown ();
	// unref cursors
	for (int i = 0; i < CursorMax; i++)
		g_object_unref (m_Cursors[i]);
	// unload plugins
	Plugin::UnloadPlugins ();
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
				Tools *ToolsBox = dynamic_cast<Tools*> (GetDialog ("tools"));
				if (ToolsBox)
					ToolsBox->OnSelectTool (m_pActiveTool);
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
		Window *Win = static_cast < Window * > (m_pActiveDoc->GetWindow ());
		if (Win)
			Win->ClearStatus ();
	}
}

void Application::SetStatusText (const char* text)
{
	if (m_pActiveDoc) {
		Window *Win = static_cast < Window * > (m_pActiveDoc->GetWindow ());
		if (Win)
			Win->SetStatusText (text);
	}
}

void Application::OnSaveAs ()
{
	gcugtk::FileChooser (this, true, m_WriteableMimeTypes, m_pActiveDoc);
}

enum {
	CHEMISTRY,
	SVG,
	EPS,
	PDF,
	PS,
	PIXBUF
};

bool Application::FileProcess (char const *filename, char const *mime_type, bool bSave, G_GNUC_UNUSED GtkWindow *window, gcu::Document *Doc)
{
	char const *ext;
	Document *pDoc = static_cast<Document*> (Doc);
	if (!filename || !strlen (filename) || g_file_test (filename, G_FILE_TEST_IS_DIR)) {
		GetCmdContext ()->Message (_("Please enter a file name,\nnot a directory"), CmdContext::SeverityWarning, true);
		return true;
	}
	int file_type = -1;
	int n = strlen (filename), i = n - 1;
	char const *pixbuf_type = NULL;
	string filename2 = filename;
	while ((i > 0) && (filename[i] != '.') && (filename[i] != '/'))
		i--;
	if (filename[i] == '/')
		i = 0;
	ext = (i > 0)? filename + i + 1: NULL;
	if (!mime_type) // to be really sure we don't crash
		mime_type = "application/x-gchempaint";
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
		else if (!strcmp (mime_type, "application/pdf"))
			file_type = PDF;
		else if (!strcmp (mime_type, "application/ps"))
			file_type = PS;
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
		char *unescaped = g_uri_unescape_string (filename, NULL);
		char *mess = bSave?
					 g_strdup_printf (_("Sorry, format %s not supported!\nFailed to save %s."), mime_type, unescaped):
					 g_strdup_printf (_("Sorry, format %s not supported!\nFailed to load %s."), mime_type, unescaped);
		g_free (unescaped);
		GetCmdContext ()->Message (mess, CmdContext::SeverityError, true);
		g_free (mess);
		return true;
	}
	list<string> &exts = globs[mime_type];
	bool err;
	GFile *file;
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
		file = g_file_new_for_uri (filename2.c_str ());
		err = g_file_query_exists (file, NULL);
		CmdContext::Response result = CmdContext::ResponseYes;
		if (err) {
			char *unescaped = g_uri_unescape_string (filename2.c_str (), NULL);
			char *message = g_strdup_printf (_("File %s\nexists, overwrite?"), unescaped);
			g_free (unescaped);
			result = GetCmdContext ()->GetResponse (message, CmdContext::ResponseYes | CmdContext::ResponseNo);
			g_free (message);
		}
		if (result == CmdContext::ResponseYes) {
			// destroy the old file if needed
			if (err) {
				GError *error = NULL;
				g_file_delete (file, NULL, &error);
				if (error) {
					char *unescaped = g_uri_unescape_string (filename2.c_str (), NULL);
					char *message = g_strdup_printf (_("Error while processing %s:\n%s"), unescaped, error->message);
					g_free (unescaped);
					g_error_free (error);
					GetCmdContext ()->Message (message, CmdContext::SeverityError, false);
					g_free (message);
					g_object_unref (file);
					return false;
				}
			}
			switch (file_type) {
			case SVG:
				m_pActiveDoc->ExportImage (filename2, "svg");
				break;
			case EPS:
				m_pActiveDoc->ExportImage (filename2, "eps");
				break;
			case PDF:
				m_pActiveDoc->ExportImage (filename2, "pdf");
				break;
			case PS:
				m_pActiveDoc->ExportImage (filename2, "ps");
				break;
			case PIXBUF:
				m_pActiveDoc->ExportImage (filename2, pixbuf_type, GetImageResolution ());
				break;
			default:
				if (!strcmp (mime_type, "application/x-gchempaint"))
					SaveGcp (filename2, pDoc);
				else
					Save (filename2, mime_type, pDoc, ContentType2D);
			}
		}
		g_object_unref (file);
	} else  try { //loading
		file = g_file_new_for_uri (filename);
		err = g_file_query_exists (file, NULL);
		g_object_unref (file);
		if (err) {
			if (!ext) {
				list<string>::iterator cur, end = exts.end ();
				for (cur = exts.begin (); cur != end; cur++) {
					filename2 = string (filename) + "." + *cur;
					file = g_file_new_for_uri (filename2.c_str ());
					err = !g_file_query_exists (file, NULL);
					g_object_unref (file);
					if (!err)
						break;
				}
			}
			if (err) {
				filename2 = filename;
			}
		}
		bool create = false;
		if (!pDoc || !pDoc->GetEmpty () || pDoc->GetDirty ()) {
			create = true;
			OnFileNew ();
			pDoc = m_pActiveDoc;
		}
		pDoc->SetFileName(filename2, mime_type);
		if (!strcmp (mime_type, "application/x-gchempaint"))
			OpenGcp (filename2, pDoc);
		else {
			// FIXME: rewrite this code
			ContentType type = Load (filename2, mime_type, pDoc, "--gen2D");
			if (type != ContentTypeUnknown) {
				switch (type) {
				case ContentType3D: {
					// open in gchem3d instead
					string command = string ("gchem3d-") + API_VERSION + " " + filename2;
					g_spawn_command_line_async (command.c_str (), NULL);
					if (create) {
						pDoc->GetWindow ()->Destroy ();
						pDoc = NULL;
						} else
						pDoc->Clear ();
					return false;
				}
				case ContentTypeCrystal:
					break;
				case ContentTypeSpectrum:
					break;
				default: {
					pDoc->Loaded ();
					double l = pDoc->GetMedianBondLength ();
					if (l > 0.) {
						double r = pDoc->GetBondLength () / l;
						if (fabs (r - 1.) > .1) { // might not work properly in some rare cases when there are a lot of short or long bonds
							Matrix2D m (r, 0., 0., r);
							// FIXME: this would not work for reactions
							pDoc->Transform2D (m, 0., 0.);
						}
					}
					break;
				}
				};
				pDoc->GetView ()->AddObject (pDoc);
				pDoc->GetView ()->Update (pDoc);
				pDoc->GetView ()->EnsureSize ();
				if (pDoc->GetWindow ())
					static_cast < Window * > (m_pActiveDoc->GetWindow ())->ActivateActionWidget ("/MainMenu/FileMenu/SaveAsImage", pDoc->HasChildren ());
			}
		}
	}
	catch (LoaderError &e) {
		if (!pDoc)
			pDoc = m_pActiveDoc;
		pDoc->Clear ();
		char *unescaped = g_uri_unescape_string (filename, NULL);
		string mess = _("Error in ");
		mess += unescaped;
		// Note to translator: add a space if needed before the semicolon
		mess += _(":\n");
		mess += e.what ();
		GetCmdContext ()->Message (mess.c_str (), CmdContext::SeverityError, false);
		g_free (unescaped);
	}
	return false;
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

void Application::OpenGcp (string const &filename, Document* pDoc)
{
	xmlDocPtr xml = NULL;
	GError *error = NULL;
	GFileInfo *info = NULL;
	bool create = false;
	pDoc->SetUseAtomColors (false); // the default for a document
	try
	{
		if (!filename.length ())
			throw (int) 0;

		// try opening with write access to see if it is readonly
		// use xmlReadIO for non local files.
		GFile *file = g_file_new_for_uri (filename.c_str ());
		info = g_file_query_info (file,
								  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
								  G_FILE_QUERY_INFO_NONE, NULL, &error);
		if (error) {
			g_object_unref (file);
			g_message ("GIO error: %s\n", error->message);
			g_error_free (error);
			throw 1;
		}
		if (!(xml = ReadXMLDocFromFile (file, filename.c_str (), NULL, NULL))) {
			g_object_unref (file);
			throw 1;
		}
		g_object_unref (file);
		if (xml->children == NULL)
			throw (int) 2;
		if (strcmp((char*)xml->children->name, "chemistry"))
			throw (int) 3;	//FIXME: that could change when a dtd is available
		if (!pDoc || !pDoc->GetEmpty () || pDoc->GetDirty ()) {
			create = true;
			OnFileNew ();
			pDoc = m_pActiveDoc;
		}
		pDoc->SetFileName(filename, "application/x-gchempaint");
		bool result = pDoc->Load(xml->children);
		if (!result) {
			if (create)
				pDoc->GetWindow ()->Destroy ();
			throw (int) 4;
		}
		pDoc->SetReadOnly (!g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE));
		g_object_unref (info);
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
		if (num > 1)
			xmlFreeDoc(xml);
		if (info)
			g_object_unref (info);
		char *mess = NULL;
		GtkWidget* message;
		switch (num)
		{
		case 0:
			mess = _("No filename");
			break;
		case 1:
			mess = _("Could not load file\n%s");
			break;
		case 2:
			mess = _("%s: invalid xml file.\nTree is empty?");
			break;
		case 3:
			mess = _("%s: invalid file format.");
			break;
		case 4:
			mess = _("%s: parse error.");
			break;
		default:
			throw (num); //this should not occur
		}
		char *unescaped = g_uri_unescape_string (filename.c_str (), NULL);
		message = gtk_message_dialog_new (NULL, (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, mess, unescaped, NULL);
		g_free (unescaped);
		gtk_window_set_icon_name (GTK_WINDOW (message), "gchempaint");
		g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
		gtk_widget_show(message);
		pDoc->SetUseAtomColors (m_UseAtomColors); // the file is new, so it should follow the app default
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
	l.push_front ("application/ps");
	l.push_front ("application/pdf");
	l.push_front ("image/x-eps");
	l.push_front ("image/svg+xml");
	gcugtk::FileChooser (this, true, l, m_pActiveDoc, _("Save as image"), GetImageResolutionWidget ());
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
			pDialog->Present ();
		else
			new ZoomDlg (m_pActiveDoc);
	}
}

void Application::OnToolChanged (char const *new_tool_name)
{
	Tools *ToolsBox = dynamic_cast<Tools*> (GetDialog ("tools"));
	Tool *new_tool = m_Tools[new_tool_name];
	if (m_pActiveTool) {
		if (m_pActiveTool->GetName () == new_tool_name)
			return;
		if (new_tool == NULL || !m_pActiveTool->Activate(false)) {
			if (ToolsBox)
				ToolsBox->OnSelectTool (m_pActiveTool);
			return;
		}
	} else if (new_tool == NULL)
		return;
	m_pActiveTool = new_tool;
	if (ToolsBox)
		ToolsBox->OnSelectTool (m_pActiveTool);
	if (m_pActiveTool)
		m_pActiveTool->Activate(true);
}

void Application::BuildTools () throw (std::runtime_error)
{
	Tools *ToolsBox = new Tools (this, m_ToolDescriptions);
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
	} else if (GetActiveDocument ())
		ToolsBox->Show (visible, GetActiveDocument ()->GetGtkWindow ());
}

void Application::AddTools (ToolDesc const *tools)
{
	// just storing at this point
	m_ToolDescriptions.push_back (tools);
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

void Application::UpdateAllTargets ()
{
	set <Target *>::iterator target, tend = m_Targets.end ();
	for (target = m_Targets.begin (); target != tend; target++) {
		Document *doc = (*target)->GetDocument ();
		doc->GetView ()->Update (doc);
	}
	// now propagate to tools
	std::map <std::string, Tool*>::iterator i, iend = m_Tools.end ();
	for (i = m_Tools.begin (); i != iend; i++)
		(*i).second->OnConfigChanged ();
}

void Application::OnConfigChanged (GOConfNode *node, char const *name)
{
	GCU_UPDATE_KEY ("compression", int, CompressionLevel, {})
	bool CopyAsText;
	GCU_UPDATE_KEY ("invert-wedge-hashes", bool, InvertWedgeHashes, UpdateAllTargets ();)
	GCU_UPDATE_KEY ("use-atom-colors", bool, m_UseAtomColors, {})
	GCU_UPDATE_KEY ("copy-as-text", bool, CopyAsText, ClipboardFormats = CopyAsText?GCP_CLIPBOARD_ALL: GCP_CLIPBOARD_NO_TEXT;)
}

void Application::AddMimeType (list<string> &l, string const& mime_type)
{
	list<string>::iterator i, iend = l.end ();
	for (i = l.begin (); i != iend; i++)
		if (*i == mime_type)
			break;
	if (i == iend)
		l.push_back (mime_type);
	else
		g_warning ("Duplicate mime type: %s", mime_type.c_str ());
}

void Application::TestSupportedType (char const *mime_type, char const* babel_type, bool writeable)
{
	AddMimeType (m_SupportedMimeTypes, mime_type);
	if (babel_type)
		RegisterBabelType (mime_type, babel_type);
	if (writeable)
		AddMimeType (m_WriteableMimeTypes, mime_type);
/*	OBFormat *f = OBConversion::FormatFromMIME (mime_type);
	if (f != NULL) {
		AddMimeType (m_SupportedMimeTypes, mime_type);
		if (!(f->Flags () & NOTWRITABLE))
			AddMimeType (m_WriteableMimeTypes, mime_type);
	}*/
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

void Application::BuildMenu (gcu::UIManager *manager)
{
	list<BuildMenuCb>::iterator i, end = m_MenuCbs.end ();
	for (i = m_MenuCbs.begin (); i != end; i++)
		(*i) (manager);
}

gcu::Document *Application::CreateNewDocument ()
{
	return new Document (this, true, NULL);
}

void Application::ReceiveTargets (GtkClipboard *clipboard, GtkSelectionData *selection_data)
{
	on_receive_targets (clipboard, selection_data, this);
}


}	//	namespace gcp
