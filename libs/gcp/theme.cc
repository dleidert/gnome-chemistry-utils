// -*- C++ -*-

/* 
 * GChemPaint library
 * theme.cc
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "theme.h"
#include "settings.h"
#include <gcu/application.h>
#ifdef HAVE_GO_CONF_SYNC
#	include <goffice/goffice.h>
#endif
#include <glib/gi18n-lib.h>
#include <sys/stat.h>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

double DefaultBondLength = 140.;
double DefaultBondAngle = 120.;
double DefaultBondDist = 5.;
double DefaultBondWidth = 1.0;
double DefaultArrowLength = 200;
double DefaultArrowHeadA = 6.0;
double DefaultArrowHeadB = 8.0;
double DefaultArrowHeadC = 4.0;
double DefaultArrowDist = 5.0;
double DefaultArrowWidth = 1.0;
double DefaultHashWidth = 1.0;
double DefaultHashDist = 2.0;
double DefaultStereoBondWidth = 5.0;
double DefaultZoomFactor = 0.25;
double DefaultPadding = 2.0;
double DefaultArrowPadding = 16.0;
double DefaultArrowObjectPadding = 16.0;
double DefaultStoichiometryPadding = 1.;
double DefaultObjectPadding = 16.0;
double DefaultSignPadding = 8.0;
//double DefaultChargeSignHeight = 12.;
double DefaultChargeSignSize = 9.;
//double DefaultChargeYAlign = 10.;
gchar *DefaultFontFamily = NULL;
PangoStyle DefaultFontStyle = PANGO_STYLE_NORMAL;
PangoWeight DefaultFontWeight = PANGO_WEIGHT_NORMAL;
PangoVariant DefaultFontVariant = PANGO_VARIANT_NORMAL;
PangoStretch DefaultFontStretch = PANGO_STRETCH_NORMAL;
gint DefaultFontSize = 12 * PANGO_SCALE;
gchar *DefaultTextFontFamily = NULL;
PangoStyle DefaultTextFontStyle = PANGO_STYLE_NORMAL;
PangoWeight DefaultTextFontWeight = PANGO_WEIGHT_NORMAL;
PangoVariant DefaultTextFontVariant = PANGO_VARIANT_NORMAL;
PangoStretch DefaultTextFontStretch = PANGO_STRETCH_NORMAL;
gint DefaultTextFontSize = 12 * PANGO_SCALE;

Theme::Theme (char const *name)
{
	m_ZoomFactor = DefaultZoomFactor;
	m_BondLength = DefaultBondLength;
	m_BondAngle = DefaultBondAngle;
	m_BondDist = DefaultBondDist;
	m_BondWidth = DefaultBondWidth;
	m_ArrowLength = DefaultArrowLength;
	m_HashWidth = DefaultHashWidth;
	m_HashDist = DefaultHashDist;
	m_StereoBondWidth = DefaultStereoBondWidth;
	m_Padding = DefaultPadding;
	m_ArrowHeadA = DefaultArrowHeadA;
	m_ArrowHeadB = DefaultArrowHeadB;
	m_ArrowHeadC = DefaultArrowHeadC;
	m_ArrowDist = DefaultArrowDist;
	m_ArrowPadding = DefaultArrowPadding;
	m_ArrowObjectPadding = DefaultArrowObjectPadding;
	m_ArrowWidth = DefaultArrowWidth;
	m_StoichiometryPadding = DefaultStoichiometryPadding;
	m_ObjectPadding = DefaultObjectPadding;
	m_SignPadding = DefaultSignPadding;
	m_ChargeSignSize = DefaultChargeSignSize;
	m_FontFamily = strdup (DefaultFontFamily);
	m_FontStyle = DefaultFontStyle;
	m_FontWeight = DefaultFontWeight;
	m_FontVariant = DefaultFontVariant;
	m_FontStretch = DefaultFontStretch;
	m_FontSize = DefaultFontSize;
	m_TextFontFamily = strdup (DefaultTextFontFamily);
	m_TextFontStyle = DefaultTextFontStyle;
	m_TextFontWeight = DefaultTextFontWeight;
	m_TextFontVariant = DefaultTextFontVariant;
	m_TextFontStretch = DefaultTextFontStretch;
	m_TextFontSize = DefaultTextFontSize;
	if (name)
		m_Name = name;
	m_ThemeType = DEFAULT_THEME_TYPE;
	modified = false;
}

Theme::~Theme ()
{
	if (m_FontFamily)
		g_free (m_FontFamily);
	if (m_TextFontFamily)
		g_free (m_TextFontFamily);
	// set all remaining document client theme to NULL
	// this is just a work around, but a ThemeClient class should be implemented
	Document *doc;
	std::set <gcu::Object*>::iterator i, end = m_Clients.end ();
	for (i = m_Clients.begin (); i != end; i++) {
		doc = dynamic_cast <Document *> (*i);
		if (doc)
			doc->SetTheme (NULL);
	}
}

ThemeManager TheThemeManager;

#ifdef HAVE_GO_CONF_SYNC
static void on_config_changed (GOConfNode *node, gchar const *key, ThemeManager *manager)
{
	manager->OnConfigChanged (node, key);
}
#else
static void on_config_changed (GConfClient *client, guint cnxn_id, GConfEntry *entry, ThemeManager *manager)
{
	manager->OnConfigChanged (client, cnxn_id, entry);
}
#endif

// transform functions for gconf key values

static double inv (double x) {return 1 / x;}

static PangoStyle set_fontstyle (int val)
{
	switch (val) {
		case 0: return PANGO_STYLE_NORMAL;
		case 1: return PANGO_STYLE_OBLIQUE;
		case 2: return PANGO_STYLE_ITALIC;
		default: return PANGO_STYLE_NORMAL;
	}
}

static PangoWeight set_fontweight (int val)
{
	switch (val) {
		case 2: return PANGO_WEIGHT_ULTRALIGHT;
		case 3: return PANGO_WEIGHT_LIGHT;
		case 4: return PANGO_WEIGHT_NORMAL;
		case 6: return PANGO_WEIGHT_SEMIBOLD;
		case 7: return PANGO_WEIGHT_BOLD;
		case 8: return PANGO_WEIGHT_ULTRABOLD;
		case 9: return PANGO_WEIGHT_HEAVY;
		default: return PANGO_WEIGHT_NORMAL;
	}
}

static PangoVariant set_fontvariant (int val)
{
	switch (val) {
		case 0: return PANGO_VARIANT_NORMAL;
		case 1: return PANGO_VARIANT_SMALL_CAPS;
		default: return PANGO_VARIANT_NORMAL;
	}
}

static PangoStretch set_fontstretch (int val)
{
	switch (val) {
		case 0: return PANGO_STRETCH_ULTRA_CONDENSED;
		case 1: return PANGO_STRETCH_EXTRA_CONDENSED;
		case 2: return PANGO_STRETCH_CONDENSED;
		case 3: return PANGO_STRETCH_SEMI_CONDENSED;
		case 4: return PANGO_STRETCH_NORMAL;
		case 5: return PANGO_STRETCH_SEMI_EXPANDED;
		case 6: return PANGO_STRETCH_EXPANDED;
		case 7: return PANGO_STRETCH_EXTRA_EXPANDED;
		case 8: return PANGO_STRETCH_ULTRA_EXPANDED;
		default: return PANGO_STRETCH_NORMAL;
	}
}

static int set_fontsize (double val) {return (int) (val * PANGO_SCALE);}

ThemeManager::ThemeManager ()
{
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	g_type_init ();
#ifdef HAVE_GO_CONF_SYNC
	libgoffice_init ();
	m_ConfNode = go_conf_get_node (gcu::Application::GetConfDir (), GCP_CONF_DIR_SETTINGS);
#else
	GError *error = NULL;
	m_ConfClient = gconf_client_get_default ();
	gconf_client_add_dir (m_ConfClient, "/apps/gchemutils/paint/settings", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
#endif
	GCU_GCONF_GET ("bond-length", float, DefaultBondLength, 140.)
	GCU_GCONF_GET ("bond-angle", float, DefaultBondAngle, 120.)
	GCU_GCONF_GET ("bond-dist", float, DefaultBondDist, 5.)
	GCU_GCONF_GET ("bond-width", float, DefaultBondWidth, 1.0)
	GCU_GCONF_GET ("arrow-length", float, DefaultArrowLength, 200)
	GCU_GCONF_GET ("arrow-headA", float, DefaultArrowHeadA, 6.0)
	GCU_GCONF_GET ("arrow-headB", float, DefaultArrowHeadB, 8.0)
	GCU_GCONF_GET ("arrow-headC", float, DefaultArrowHeadC, 4.0)
	GCU_GCONF_GET ("arrow-dist", float, DefaultArrowDist, 5.0)
	GCU_GCONF_GET ("arrow-width", float, DefaultArrowWidth, 1.0)
	GCU_GCONF_GET ("hash-width", float, DefaultHashWidth, 1.0)
	GCU_GCONF_GET ("hash-dist", float, DefaultHashDist, 2.0)
	GCU_GCONF_GET ("stereo-width", float, DefaultStereoBondWidth, 5.0)
	GCU_GCONF_GET_N_TRANSFORM ("scale", float, DefaultZoomFactor, 4., inv)
	GCU_GCONF_GET ("padding", float, DefaultPadding, 2.0)
	GCU_GCONF_GET ("arrow-padding", float, DefaultArrowPadding, 16.0)
	GCU_GCONF_GET ("arrow-object-padding", float, DefaultArrowObjectPadding, 16.0)
	GCU_GCONF_GET ("stoichiometry-padding", float, DefaultStoichiometryPadding, 1.)
	GCU_GCONF_GET ("object-padding", float, DefaultObjectPadding, 16.0)
	GCU_GCONF_GET ("sign-padding", float, DefaultSignPadding, 8.0)
	GCU_GCONF_GET ("charge-sign-size", float, DefaultChargeSignSize, 9.)
	GCU_GCONF_GET_STRING ("font-family", DefaultFontFamily, "Bitstream Vera Sans")
	GCU_GCONF_GET_N_TRANSFORM ("font-style", int, DefaultFontStyle, 0, set_fontstyle)
	GCU_GCONF_GET_N_TRANSFORM ("font-weight", int, DefaultFontWeight, 4, set_fontweight)
	GCU_GCONF_GET_N_TRANSFORM ("font-variant", int, DefaultFontVariant, 0, set_fontvariant)
	GCU_GCONF_GET_N_TRANSFORM ("font-stretch", int, DefaultFontStretch, 4, set_fontstretch)
	GCU_GCONF_GET_N_TRANSFORM ("font-size", float, DefaultFontSize, 12., set_fontsize)
	GCU_GCONF_GET_STRING ("text-font-family", DefaultTextFontFamily, "Bitstream Vera Serif")
	GCU_GCONF_GET_N_TRANSFORM ("text-font-style", int, DefaultTextFontStyle, 0, set_fontstyle)
	GCU_GCONF_GET_N_TRANSFORM ("text-font-weight", int, DefaultTextFontWeight, 4, set_fontweight)
	GCU_GCONF_GET_N_TRANSFORM ("text-font-variant", int, DefaultTextFontVariant, 0, set_fontvariant)
	GCU_GCONF_GET_N_TRANSFORM ("text-font-stretch", int, DefaultTextFontStretch, 4, set_fontstretch)
	GCU_GCONF_GET_N_TRANSFORM ("text-font-size", float, DefaultTextFontSize, 12., set_fontsize)
	// Build default theme from settings
#ifdef HAVE_GO_CONF_SYNC
	m_NotificationId = go_conf_add_monitor (m_ConfNode, NULL, (GOConfMonitorFunc) on_config_changed, this);
#else
	m_NotificationId = gconf_client_notify_add (m_ConfClient, "/apps/gchemutils/paint/settings", (GConfClientNotifyFunc) on_config_changed, this, NULL, NULL);
#endif
	m_Themes["GChemPaint"] = new Theme ("GChemPaint");
	m_Names.push_front ("GChemPaint");
	// load global themes
	string path = PKGDATADIR;
	path += "/paint/themes";
	ParseDir (path, GLOBAL_THEME_TYPE);
	// load local themes
	char *szhome = getenv ("HOME");
	if (szhome)
		path = szhome;
	path += "/.gchempaint/themes";
	ParseDir (path, LOCAL_THEME_TYPE);
	gchar *default_theme  =NULL;
	GCU_GCONF_GET_STRING ("default-theme", default_theme, "GChemPaint");
	m_DefaultTheme = m_Themes[default_theme];
	g_free (default_theme);
	if (!m_DefaultTheme)
		m_DefaultTheme = m_Themes["GChemPaint"];
}

ThemeManager::~ThemeManager ()
{
	// save themes if needed, then delete
	Theme *theme, *def = NULL;
	map <string, Theme*>::iterator i, iend = m_Themes.end ();
	for (i = m_Themes.begin (); i != iend; i++) {
		theme = (*i).second;
		if (!theme || (def && theme == def))
			continue; // this theme has already been deleted
		if (theme->modified && theme->m_ThemeType == LOCAL_THEME_TYPE) {
			xmlDocPtr doc = xmlNewDoc((xmlChar*)"1.0");
			xmlDocSetRootElement (doc,  xmlNewDocNode (doc, NULL, (xmlChar*) "chemistry", NULL));
			if (theme->Save (doc)) {
				char *szhome = getenv ("HOME");
				string home, path;
				if (szhome)
					home = szhome;
				path = home + "/.gchempaint/themes";
				GDir *dir = g_dir_open (path.c_str (), 0, NULL);
				if (!dir) {
					string path;
					path = home + "/.gchempaint";
					dir = g_dir_open (path.c_str (), 0, NULL);
					if (dir)
						g_dir_close (dir);
					else
						mkdir (path.c_str (), 0x1ed);
					mkdir (path.c_str (), 0x1ed);
				} else
					g_dir_close (dir);
				path += string ("/") + theme->GetName ();
				xmlSaveFormatFile (path.c_str (), doc, true);
			}
		} else if (theme->m_Name == "Default")
			def = theme;
		delete theme;
	}
	g_free (DefaultFontFamily);
	g_free (DefaultTextFontFamily);
}

void ThemeManager::Shutdown ()
{
#ifdef HAVE_GO_CONF_SYNC
	go_conf_remove_monitor (m_NotificationId);
	go_conf_free_node (m_ConfNode);
	m_ConfNode = NULL;
#else
	gconf_client_notify_remove (m_ConfClient, m_NotificationId);
	gconf_client_remove_dir (m_ConfClient, "/apps/gchemutils/paint/settings", NULL);
	g_object_unref (m_ConfClient);
	m_ConfClient = NULL;
#endif
}

Theme *ThemeManager::GetTheme (char const *name)
{
	if (!strcmp (_(name), _("Default")))
		return m_DefaultTheme;
	return m_Themes[name];
}

Theme *ThemeManager::GetTheme (string &name)
{
	if (name == "Default" || name == _("Default") )
		return m_DefaultTheme;
	return m_Themes[name.c_str ()];
}

list <string> const &ThemeManager::GetThemesNames ()
{
	return m_Names;
}

#ifdef HAVE_GO_CONF_SYNC
void ThemeManager::OnConfigChanged (GOConfNode *node, gchar const *name)
{
#else
void ThemeManager::OnConfigChanged (GConfClient *client, guint cnxn_id, GConfEntry *entry)
{
	if (client != m_ConfClient)
		return;	// we might want an error message?
	if (cnxn_id != m_NotificationId)
		return;	// we might want an error message?
	char const *name;
#endif
	Theme *theme = m_Themes["GChemPaint"];
	GCU_UPDATE_KEY ("bond-length", float, DefaultBondLength, theme->m_BondLength = DefaultBondLength;)
	GCU_UPDATE_KEY ("bond-angle", float, DefaultBondAngle, theme->m_BondAngle = DefaultBondAngle;)
	GCU_UPDATE_KEY ("bond-dist", float, DefaultBondDist, theme->m_BondDist = DefaultBondDist;)
	GCU_UPDATE_KEY ("bond-width", float, DefaultBondWidth, theme->m_BondWidth = DefaultBondWidth;)
	GCU_UPDATE_KEY ("arrow-length", float, DefaultArrowLength, theme->m_ArrowLength = DefaultArrowLength;)
	GCU_UPDATE_KEY ("arrow-headA", float, DefaultArrowHeadA, theme->m_ArrowHeadA = DefaultArrowHeadA;)
	GCU_UPDATE_KEY ("arrow-headB", float, DefaultArrowHeadB, theme->m_ArrowHeadB = DefaultArrowHeadB;)
	GCU_UPDATE_KEY ("arrow-headA", float, DefaultArrowHeadC, theme->m_ArrowHeadC = DefaultArrowHeadC;)
	GCU_UPDATE_KEY ("arrow-dist", float, DefaultArrowDist, theme->m_ArrowDist = DefaultArrowDist;)
	GCU_UPDATE_KEY ("arrow-width", float, DefaultArrowWidth, theme->m_ArrowWidth = DefaultArrowWidth;)
	GCU_UPDATE_KEY ("hash-width", float, DefaultHashWidth, theme->m_HashWidth = DefaultHashWidth;)
	GCU_UPDATE_KEY ("hash-dist", float, DefaultHashDist, theme->m_HashDist = DefaultHashDist;)
	GCU_UPDATE_KEY ("stereo-width", float, DefaultStereoBondWidth, theme->m_StereoBondWidth = DefaultStereoBondWidth;)
	double x;
	GCU_UPDATE_KEY ("scale", float, x,
		if (x > 1e-5) {
			DefaultZoomFactor = 1 / x;
			theme->m_ZoomFactor = DefaultZoomFactor;
		}
	)
	GCU_UPDATE_KEY ("padding", float, DefaultPadding, theme->m_Padding = DefaultPadding;)
	GCU_UPDATE_KEY ("stoichiometry-padding", float, DefaultStoichiometryPadding, theme->m_StoichiometryPadding = DefaultStoichiometryPadding;)
	GCU_UPDATE_KEY ("object-padding", float, DefaultObjectPadding, theme->m_ObjectPadding = DefaultObjectPadding;)
	GCU_UPDATE_KEY ("charge-sign-padding", float, DefaultSignPadding, theme->m_SignPadding = DefaultSignPadding;)
	GCU_UPDATE_KEY ("charge-sign-size", float, DefaultChargeSignSize, theme->m_ChargeSignSize = DefaultChargeSignSize;)
	GCU_UPDATE_KEY ("font-family", string, name,
		if (name) {
			if (DefaultFontFamily != NULL)
				g_free (DefaultFontFamily);
			DefaultFontFamily = g_strdup (name);
			theme->m_FontFamily = DefaultFontFamily;
		})
	int n;
	GCU_UPDATE_KEY ("font-style", int, n, {
		DefaultFontStyle = set_fontstyle (n);
		theme->m_FontStyle = DefaultFontStyle;
	})
	GCU_UPDATE_KEY ("font-weight", int, n, {
		DefaultFontWeight = set_fontweight (n);
		theme->m_FontWeight = DefaultFontWeight;
	})
	GCU_UPDATE_KEY ("font-variant", int, n, {
		DefaultFontVariant = set_fontvariant (n);
		theme->m_FontVariant = DefaultFontVariant;
	})
	GCU_UPDATE_KEY ("font-stretch", int, n, {
		DefaultFontStretch = set_fontstretch (n);
		theme->m_FontStretch = DefaultFontStretch;
	})
	GCU_UPDATE_KEY ("font-size", float, DefaultFontSize, theme->m_FontSize = DefaultFontSize;)
	GCU_UPDATE_KEY ("text-font-family", string, name,
		if (name) {
			if (DefaultTextFontFamily != NULL)
				g_free (DefaultTextFontFamily);
			DefaultTextFontFamily = g_strdup (name);
			theme->m_TextFontFamily = DefaultTextFontFamily;
		})
	GCU_UPDATE_KEY ("text-font-style", int, n, {
		DefaultTextFontStyle = set_fontstyle (n);
		theme->m_TextFontStyle = DefaultTextFontStyle;
	})
	GCU_UPDATE_KEY ("text-font-weight", int, n, {
		DefaultTextFontWeight = set_fontweight (n);
		theme->m_TextFontWeight = DefaultTextFontWeight;
	})
	GCU_UPDATE_KEY ("text-font-variant", int, n, {
		DefaultTextFontVariant = set_fontvariant (n);
		theme->m_TextFontVariant = DefaultTextFontVariant;
	})
	GCU_UPDATE_KEY ("text-font-stretch", int, n, {
		DefaultTextFontStretch = set_fontstretch (n);
		theme->m_TextFontStretch = DefaultTextFontStretch;
	})
	GCU_UPDATE_KEY ("text-font-size", float, DefaultTextFontSize, theme->m_TextFontSize = DefaultTextFontSize;)
	GCU_UPDATE_KEY ("default-theme", string, name, {
		theme = m_Themes[name];
		if (theme)
			m_DefaultTheme = theme;
	})
}

Theme *ThemeManager::CreateNewTheme (Theme *theme)
{
	gchar *name = g_strdup (_("NewTheme1"));
	int i = 2;
	while (m_Themes[name] != NULL) {
		g_free (name);
		name = g_strdup_printf (_("NewTheme%d"), i++);
	}
	Theme *pTheme = new Theme (name);
	m_Themes[name] = pTheme;
	m_Names.push_back (name);
	g_free (name);
	if (theme) {
		pTheme->m_ZoomFactor = theme->m_ZoomFactor;
		pTheme->m_BondLength = theme->m_BondLength;
		pTheme->m_BondAngle = theme->m_BondAngle;
		pTheme->m_BondDist = theme->m_BondDist;
		pTheme->m_BondWidth = theme->m_BondWidth;
		pTheme->m_ArrowLength = theme->m_ArrowLength;
		pTheme->m_HashWidth = theme->m_HashWidth;
		pTheme->m_HashDist = theme->m_HashDist;
		pTheme->m_StereoBondWidth = theme->m_StereoBondWidth;
		pTheme->m_Padding = theme->m_Padding;
		pTheme->m_ArrowHeadA = theme->m_ArrowHeadA;
		pTheme->m_ArrowHeadB = theme->m_ArrowHeadB;
		pTheme->m_ArrowHeadC = theme->m_ArrowHeadC;
		pTheme->m_ArrowDist = theme->m_ArrowDist;
		pTheme->m_ArrowPadding = theme->m_ArrowPadding;
		pTheme->m_ArrowWidth = theme->m_ArrowWidth;
		pTheme->m_StoichiometryPadding = theme->m_StoichiometryPadding;
		pTheme->m_ObjectPadding = theme->m_ObjectPadding;
		pTheme->m_SignPadding = theme->m_SignPadding;
		pTheme->m_ChargeSignSize = theme->m_ChargeSignSize;
		g_free (pTheme->m_FontFamily);
		pTheme->m_FontFamily = strdup (theme->m_FontFamily);
		pTheme->m_FontStyle = theme->m_FontStyle;
		pTheme->m_FontWeight = theme->m_FontWeight;
		pTheme->m_FontVariant = theme->m_FontVariant;
		pTheme->m_FontStretch = theme->m_FontStretch;
		pTheme->m_FontSize = theme->m_FontSize;
		g_free (pTheme->m_TextFontFamily);
		pTheme->m_TextFontFamily = strdup (theme->m_TextFontFamily);
		pTheme->m_TextFontStyle = theme->m_TextFontStyle;
		pTheme->m_TextFontWeight = theme->m_TextFontWeight;
		pTheme->m_TextFontVariant = theme->m_TextFontVariant;
		pTheme->m_TextFontStretch = theme->m_TextFontStretch;
		pTheme->m_TextFontSize = theme->m_TextFontSize;
	}
	pTheme->m_ThemeType = LOCAL_THEME_TYPE;
	pTheme->modified = true;
	return pTheme;
}

void ThemeManager::ParseDir (string &path, ThemeType type)
{
	char const *name;
	xmlDocPtr doc;
	xmlNodePtr node;
	Theme *theme;
	string filename;
	GDir *dir = g_dir_open (path.c_str (), 0, NULL);
	if (dir) {
		path += "/";
		while ((name = g_dir_read_name (dir))) {
			if (name[strlen (name) - 1] == '~')
				continue; // don't read backups
			filename = path + name;
			doc = xmlParseFile (filename.c_str ());
			node = doc->children;
			if (!strcmp ((char*)node->name, "chemistry")) {
				node = node->children;
				while (node && !strcmp ((char*)node->name, "text"))
					node = node->next;
				if (node && !strcmp ((char*)node->name, "theme")) {
					theme = new Theme ("");
					theme->Load (node);
					if (theme->GetName () != name) {
						theme->m_Name = name;
						theme->modified = true;
					}
					if (theme->m_ThemeType == DEFAULT_THEME_TYPE || theme->m_ThemeType == GLOBAL_THEME_TYPE)
						name = _(name);
					if (m_Themes.find (name) != m_Themes.end ()) {
						delete theme;
						continue;
					}
					theme->m_ThemeType = type;
					m_Themes[name] = theme;
					m_Names.push_back (name);
				}
			}
			xmlFree (doc);
		}
		g_dir_close (dir);
	}
}

void ThemeManager::AddFileTheme (Theme *theme, char const *label)
{
	string name = theme->GetName ().c_str ();
	if (name == "Default")
		name = "GChemPaint";
	string theme_name = name;
	if (m_Themes.find (name) != m_Themes.end ()) {
		name = string ((label)? label: _("Unknown")) + ":" + name;
	}
	int i = 0;
	while (m_Themes.find (name) != m_Themes.end ())
		name = string ((label)? label: _("Unknown")) + ((char) ('0' + i++)) + ":" + theme_name;
	m_Themes[name] = theme;
	m_Names.push_back (name);
}

void ThemeManager::RemoveFileTheme (Theme *theme)
{
	char const *name = NULL;
	map <string, Theme*>::iterator i, iend = m_Themes.end ();
	for (i = m_Themes.begin (); i != iend; i++)
		if ((*i).second == theme) {
			name = (*i).first.c_str ();
			break;
		}
	m_Names.remove (name);
	m_Themes.erase (name);
}

void ThemeManager::ChangeThemeName (Theme *theme, char const *name)
{
	m_Themes.erase (theme->m_Name);
	m_Names.remove (theme->m_Name);
	theme->m_Name = name;
	m_Themes[name] = theme;
	m_Names.push_back (name);
}
 
void ThemeManager::SetDefaultTheme (char const *name)
{
	Theme *theme = m_Themes[name];
	if (theme) {
		m_DefaultTheme = theme;
	}
}

bool Theme::Save (xmlDocPtr xml)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) "theme", NULL);
	char *buf;
	if (!node)
		return false;
	if (m_Name.length () > 0)
		xmlNewProp (node, (xmlChar*) "name", (xmlChar*) m_Name.c_str ());
	buf = g_strdup_printf("%g", m_BondLength);
	xmlNewProp (node, (xmlChar*) "bond-length", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_BondAngle);
	xmlNewProp (node, (xmlChar*) "bond-angle", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_BondDist);
	xmlNewProp (node, (xmlChar*) "bond-dist", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_BondWidth);
	xmlNewProp (node, (xmlChar*) "bond-width", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ArrowLength);
	xmlNewProp (node, (xmlChar*) "arrow-length", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ArrowHeadA);
	xmlNewProp (node, (xmlChar*) "arrow-head-a", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ArrowHeadB);
	xmlNewProp (node, (xmlChar*) "arrow-head-b", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ArrowHeadC);
	xmlNewProp (node, (xmlChar*) "arrow-head-c", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ArrowDist);
	xmlNewProp (node, (xmlChar*) "arrow-dist", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ArrowWidth);
	xmlNewProp (node, (xmlChar*) "arrow-width", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ArrowPadding);
	xmlNewProp (node, (xmlChar*) "arrow-padding", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_HashWidth);
	xmlNewProp (node, (xmlChar*) "hash-width", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_HashDist);
	xmlNewProp (node, (xmlChar*) "hash-dist", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_StereoBondWidth);
	xmlNewProp (node, (xmlChar*) "stereo-bond-width", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", 1 / m_ZoomFactor);
	xmlNewProp (node, (xmlChar*) "zoom-factor", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_Padding);
	xmlNewProp (node, (xmlChar*) "padding", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_StoichiometryPadding);
	xmlNewProp (node, (xmlChar*) "stoichiometry-padding", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ObjectPadding);
	xmlNewProp (node, (xmlChar*) "object-padding", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_SignPadding);
	xmlNewProp (node, (xmlChar*) "sign-padding", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf("%g", m_ChargeSignSize);
	xmlNewProp (node, (xmlChar*) "charge-sign-size", (xmlChar*) buf);
	g_free (buf);
	xmlNewProp (node, (xmlChar*) "font-family", (xmlChar*) m_FontFamily);
	buf = NULL;
	switch (m_FontStyle) {
	case PANGO_STYLE_NORMAL:
		buf = const_cast<char*> ("normal");
		break;
	case PANGO_STYLE_OBLIQUE:
		buf = const_cast<char*> ("oblique");
		break;
	case PANGO_STYLE_ITALIC:
		buf = const_cast<char*> ("italic");
		break;
	}
	if (buf)
		xmlNewProp (node, (xmlChar*) "font-style", (xmlChar*) buf);
	buf = NULL;
	switch (m_FontWeight) {
	case PANGO_WEIGHT_ULTRALIGHT:
		buf = const_cast<char*> ("ultra-light");
		break;
	case PANGO_WEIGHT_LIGHT:
		buf = const_cast<char*> ("light");
		break;
	case PANGO_WEIGHT_NORMAL:
		buf = const_cast<char*> ("normal");
		break;
	case PANGO_WEIGHT_SEMIBOLD:
		buf = const_cast<char*> ("semi-bold");
		break;
	case PANGO_WEIGHT_BOLD:
		buf = const_cast<char*> ("bold");
		break;
	case PANGO_WEIGHT_ULTRABOLD:
		buf = const_cast<char*> ("ultra-bold");
		break;
	case PANGO_WEIGHT_HEAVY:
		buf = const_cast<char*> ("heavy");
		break;
	}
	if (buf)
		xmlNewProp (node, (xmlChar*) "font-weight", (xmlChar*) buf);
	xmlNewProp (node, (xmlChar*) "font-variant", (xmlChar*) ((m_FontVariant == PANGO_VARIANT_SMALL_CAPS)? "small-caps": "normal"));
	buf = NULL;
	switch (m_FontStretch) {
	case PANGO_STRETCH_ULTRA_CONDENSED:
		buf = const_cast<char*> ("ultra-condensed");
		break;
	case PANGO_STRETCH_EXTRA_CONDENSED:
		buf = const_cast<char*> ("extra-condensed");
		break;
	case PANGO_STRETCH_CONDENSED:
		buf = const_cast<char*> ("condensed");
		break;
	case PANGO_STRETCH_SEMI_CONDENSED:
		buf = const_cast<char*> ("semi-condensed");
		break;
	case PANGO_STRETCH_NORMAL:
		buf = const_cast<char*> ("normal");
		break;
	case PANGO_STRETCH_SEMI_EXPANDED:
		buf = const_cast<char*> ("semi-expanded");
		break;
	case PANGO_STRETCH_EXPANDED:
		buf = const_cast<char*> ("expanded");
		break;
	case PANGO_STRETCH_EXTRA_EXPANDED:
		buf = const_cast<char*> ("extra-expanded");
		break;
	case PANGO_STRETCH_ULTRA_EXPANDED:
		buf = const_cast<char*> ("ultra-expanded");
		break;
	}
	if (buf)
		xmlNewProp (node, (xmlChar*) "font-stretch", (xmlChar*) buf);
	buf = g_strdup_printf("%d", m_FontSize);
	xmlNewProp (node, (xmlChar*) "font-size", (xmlChar*) buf);
	g_free (buf);
	xmlNewProp (node, (xmlChar*) "text-font-family", (xmlChar*) m_TextFontFamily);
	buf = NULL;
	switch (m_TextFontStyle) {
	case PANGO_STYLE_NORMAL:
		buf = const_cast<char*> ("normal");
		break;
	case PANGO_STYLE_OBLIQUE:
		buf = const_cast<char*> ("oblique");
		break;
	case PANGO_STYLE_ITALIC:
		buf = const_cast<char*> ("italic");
		break;
	}
	if (buf)
		xmlNewProp (node, (xmlChar*) "text-font-style", (xmlChar*) buf);
	buf = NULL;
	switch (m_TextFontWeight) {
	case PANGO_WEIGHT_ULTRALIGHT:
		buf = const_cast<char*> ("ultra-light");
		break;
	case PANGO_WEIGHT_LIGHT:
		buf = const_cast<char*> ("light");
		break;
	case PANGO_WEIGHT_NORMAL:
		buf = const_cast<char*> ("normal");
		break;
	case PANGO_WEIGHT_SEMIBOLD:
		buf = const_cast<char*> ("semi-bold");
		break;
	case PANGO_WEIGHT_BOLD:
		buf = const_cast<char*> ("bold");
		break;
	case PANGO_WEIGHT_ULTRABOLD:
		buf = const_cast<char*> ("ultra-bold");
		break;
	case PANGO_WEIGHT_HEAVY:
		buf = const_cast<char*> ("heavy");
		break;
	}
	if (buf)
		xmlNewProp (node, (xmlChar*) "text-font-weight", (xmlChar*) buf);
	xmlNewProp (node, (xmlChar*) "text-font-variant", (xmlChar*) ((m_TextFontVariant == PANGO_VARIANT_SMALL_CAPS)? "small-caps": "normal"));
	buf = NULL;
	switch (m_TextFontStretch) {
	case PANGO_STRETCH_ULTRA_CONDENSED:
		buf = const_cast<char*> ("ultra-condensed");
		break;
	case PANGO_STRETCH_EXTRA_CONDENSED:
		buf = const_cast<char*> ("extra-condensed");
		break;
	case PANGO_STRETCH_CONDENSED:
		buf = const_cast<char*> ("condensed");
		break;
	case PANGO_STRETCH_SEMI_CONDENSED:
		buf = const_cast<char*> ("semi-condensed");
		break;
	case PANGO_STRETCH_NORMAL:
		buf = const_cast<char*> ("normal");
		break;
	case PANGO_STRETCH_SEMI_EXPANDED:
		buf = const_cast<char*> ("semi-expanded");
		break;
	case PANGO_STRETCH_EXPANDED:
		buf = const_cast<char*> ("expanded");
		break;
	case PANGO_STRETCH_EXTRA_EXPANDED:
		buf = const_cast<char*> ("extra-expanded");
		break;
	case PANGO_STRETCH_ULTRA_EXPANDED:
		buf = const_cast<char*> ("ultra-expanded");
		break;
	}
	if (buf)
		xmlNewProp (node, (xmlChar*) "text-font-stretch", (xmlChar*) buf);
	buf = g_strdup_printf("%d", m_TextFontSize);
	xmlNewProp (node, (xmlChar*) "text-font-size", (xmlChar*) buf);
	g_free (buf);
	xmlAddChild (xml->children, node);
	return true;
}

#define READ_DOUBLE(var,name) \
	buf = (char*) xmlGetProp (node, (xmlChar*) name); \
	if (buf) { \
		var = strtod (buf, NULL); \
		xmlFree (buf); \
	}
#define READ_INT(var,name) \
	buf = (char*) xmlGetProp (node, (xmlChar*) name); \
	if (buf) { \
		var = strtol (buf, NULL, 10); \
		xmlFree (buf); \
	}

bool Theme::Load (xmlNodePtr node)
{
	char *buf = (char*) xmlGetProp (node, (xmlChar*) "name");
	if (buf) {
		m_Name = buf;
		xmlFree (buf);
	}
	READ_DOUBLE (m_BondLength, "bond-length");
	READ_DOUBLE (m_BondAngle, "bond-angle");
	READ_DOUBLE (m_BondDist, "bond-dist");
	READ_DOUBLE (m_BondWidth, "bond-width");
	READ_DOUBLE (m_ArrowLength, "arrow-length");
	READ_DOUBLE (m_ArrowHeadA, "arrow-head-a");
	READ_DOUBLE (m_ArrowHeadB, "arrow-head-b");
	READ_DOUBLE (m_ArrowHeadC, "arrow-head-c");
	READ_DOUBLE (m_ArrowDist, "arrow-dist");
	READ_DOUBLE (m_ArrowWidth, "arrow-width");
	READ_DOUBLE (m_ArrowPadding, "arrow-padding");
	READ_DOUBLE (m_HashWidth, "hash-width");
	READ_DOUBLE (m_HashDist, "hash-dist");
	READ_DOUBLE (m_StereoBondWidth, "stereo-bond-width");
	READ_DOUBLE (m_ZoomFactor, "zoom-factor");
	m_ZoomFactor = 1 / m_ZoomFactor;
	READ_DOUBLE (m_Padding, "padding");
	READ_DOUBLE (m_StoichiometryPadding, "stoichiometry-padding");
	READ_DOUBLE (m_ObjectPadding, "object-padding");
	READ_DOUBLE (m_SignPadding, "sign-padding");
	READ_DOUBLE (m_ChargeSignSize, "charge-sign-size");
	buf = (char*) xmlGetProp (node, (xmlChar*) "font-family");
	if (buf) {
		if (m_FontFamily)
			g_free (m_FontFamily);
		m_FontFamily = g_strdup (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "font-style");
	if (buf) {
		if (!strcmp (buf, "normal"))
			m_FontStyle = PANGO_STYLE_NORMAL;
		else if (!strcmp (buf, "oblique"))
			m_FontStyle = PANGO_STYLE_OBLIQUE;
		else if (!strcmp (buf, "italic"))
			m_FontStyle = PANGO_STYLE_ITALIC;
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "font-weight");
	if (buf) {
		if (!strcmp (buf, "ultra-light"))
			m_FontWeight= PANGO_WEIGHT_ULTRALIGHT;
		else if (!strcmp (buf, "light"))
			m_FontWeight = PANGO_WEIGHT_LIGHT;
		else if (!strcmp (buf, "normal"))
			m_FontWeight = PANGO_WEIGHT_NORMAL;
		else if (!strcmp (buf, "semi-bold"))
			m_FontWeight = PANGO_WEIGHT_SEMIBOLD;
		else if (!strcmp (buf, "bold"))
			m_FontWeight = PANGO_WEIGHT_BOLD;
		else if (!strcmp (buf, "ultra-bold"))
			m_FontWeight = PANGO_WEIGHT_ULTRABOLD;
		else if (!strcmp (buf, "heavy"))
			m_FontWeight = PANGO_WEIGHT_HEAVY;
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "font-variant");
	if (buf) {
		if (!strcmp (buf, "normal"))
			m_FontVariant = PANGO_VARIANT_NORMAL;
		else if (!strcmp (buf, "small-caps"))
			m_FontVariant = PANGO_VARIANT_SMALL_CAPS;
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "font-stretch");
	if (buf) {
		if (!strcmp (buf, "ultra-condensed"))
			m_FontStretch = PANGO_STRETCH_ULTRA_CONDENSED;
		else if (!strcmp (buf, "extra-condensed"))
			m_FontStretch = PANGO_STRETCH_EXTRA_CONDENSED;
		else if (!strcmp (buf, "condensed"))
			m_FontStretch = PANGO_STRETCH_CONDENSED;
		else if (!strcmp (buf, "semi-condensed"))
			m_FontStretch = PANGO_STRETCH_SEMI_CONDENSED;
		else if (!strcmp (buf, "normal"))
			m_FontStretch = PANGO_STRETCH_NORMAL;
		else if (!strcmp (buf, "semi-expanded"))
			m_FontStretch = PANGO_STRETCH_SEMI_EXPANDED;
		else if (!strcmp (buf, "expanded"))
			m_FontStretch = PANGO_STRETCH_EXPANDED;
		else if (!strcmp (buf, "extra-expanded"))
			m_FontStretch = PANGO_STRETCH_EXTRA_EXPANDED;
		else if (!strcmp (buf, "ultra-expanded"))
			m_FontStretch = PANGO_STRETCH_ULTRA_EXPANDED;
		xmlFree (buf);
	}
	READ_INT (m_FontSize, "font-size");
	buf = (char*) xmlGetProp (node, (xmlChar*) "text-font-family");
	if (buf) {
		if (m_TextFontFamily)
			g_free (m_TextFontFamily);
		m_TextFontFamily = g_strdup (buf);
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "text-font-style");
	if (buf) {
		if (!strcmp (buf, "normal"))
			m_TextFontStyle = PANGO_STYLE_NORMAL;
		else if (!strcmp (buf, "oblique"))
			m_TextFontStyle = PANGO_STYLE_OBLIQUE;
		else if (!strcmp (buf, "italic"))
			m_TextFontStyle = PANGO_STYLE_ITALIC;
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "text-font-weight");
	if (buf) {
		if (!strcmp (buf, "ultra-light"))
			m_TextFontWeight= PANGO_WEIGHT_ULTRALIGHT;
		else if (!strcmp (buf, "light"))
			m_TextFontWeight = PANGO_WEIGHT_LIGHT;
		else if (!strcmp (buf, "normal"))
			m_TextFontWeight = PANGO_WEIGHT_NORMAL;
		else if (!strcmp (buf, "semi-bold"))
			m_TextFontWeight = PANGO_WEIGHT_SEMIBOLD;
		else if (!strcmp (buf, "bold"))
			m_TextFontWeight = PANGO_WEIGHT_BOLD;
		else if (!strcmp (buf, "ultra-bold"))
			m_TextFontWeight = PANGO_WEIGHT_ULTRABOLD;
		else if (!strcmp (buf, "heavy"))
			m_TextFontWeight = PANGO_WEIGHT_HEAVY;
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "text-font-variant");
	if (buf) {
		if (!strcmp (buf, "normal"))
			m_TextFontVariant = PANGO_VARIANT_NORMAL;
		else if (!strcmp (buf, "small-caps"))
			m_TextFontVariant = PANGO_VARIANT_SMALL_CAPS;
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "text-font-stretch");
	if (buf) {
		if (!strcmp (buf, "ultra-condensed"))
			m_TextFontStretch = PANGO_STRETCH_ULTRA_CONDENSED;
		else if (!strcmp (buf, "extra-condensed"))
			m_TextFontStretch = PANGO_STRETCH_EXTRA_CONDENSED;
		else if (!strcmp (buf, "condensed"))
			m_TextFontStretch = PANGO_STRETCH_CONDENSED;
		else if (!strcmp (buf, "semi-condensed"))
			m_TextFontStretch = PANGO_STRETCH_SEMI_CONDENSED;
		else if (!strcmp (buf, "normal"))
			m_TextFontStretch = PANGO_STRETCH_NORMAL;
		else if (!strcmp (buf, "semi-expanded"))
			m_TextFontStretch = PANGO_STRETCH_SEMI_EXPANDED;
		else if (!strcmp (buf, "expanded"))
			m_TextFontStretch = PANGO_STRETCH_EXPANDED;
		else if (!strcmp (buf, "extra-expanded"))
			m_TextFontStretch = PANGO_STRETCH_EXTRA_EXPANDED;
		else if (!strcmp (buf, "ultra-expanded"))
			m_TextFontStretch = PANGO_STRETCH_ULTRA_EXPANDED;
		xmlFree (buf);
	}
	READ_INT (m_TextFontSize, "text-font-size");
	m_ThemeType = FILE_THEME_TYPE;
	return true;
}

#define TEST_FIELD(field) \
	if (field != theme.field) \
		return false;
#define TEST_FLOAT_FIELD(field) \
	if (fabs (1. - field / theme.field) > 1e-7) \
		return false;

bool Theme::operator== (const Theme &theme)
{
	TEST_FLOAT_FIELD (m_BondLength)
	TEST_FLOAT_FIELD (m_BondAngle)
	TEST_FLOAT_FIELD (m_BondDist)
	TEST_FLOAT_FIELD (m_BondWidth)
	TEST_FLOAT_FIELD (m_ArrowLength)
	TEST_FLOAT_FIELD (m_ArrowWidth)
	TEST_FLOAT_FIELD (m_ArrowDist)
	TEST_FLOAT_FIELD (m_ArrowHeadA)
	TEST_FLOAT_FIELD (m_ArrowHeadB)
	TEST_FLOAT_FIELD (m_ArrowHeadC)
	TEST_FLOAT_FIELD (m_ArrowPadding)
	TEST_FLOAT_FIELD (m_HashWidth)
	TEST_FLOAT_FIELD (m_HashDist)
	TEST_FLOAT_FIELD (m_StereoBondWidth)
	TEST_FLOAT_FIELD (m_ZoomFactor)
	TEST_FLOAT_FIELD (m_Padding)
	TEST_FLOAT_FIELD (m_StoichiometryPadding)
	TEST_FLOAT_FIELD (m_ObjectPadding)
	TEST_FLOAT_FIELD (m_SignPadding)
	TEST_FLOAT_FIELD (m_ChargeSignSize)
	if (strcmp (m_FontFamily, theme.m_FontFamily))
		return false;
	TEST_FIELD (m_FontStyle)
	TEST_FIELD (m_FontWeight)
	TEST_FIELD (m_FontVariant)
	TEST_FIELD (m_FontStretch)
	TEST_FIELD (m_FontSize)
	if (strcmp (m_TextFontFamily, theme.m_TextFontFamily))
		return false;
	TEST_FIELD (m_TextFontStyle)
	TEST_FIELD (m_TextFontWeight)
	TEST_FIELD (m_TextFontVariant)
	TEST_FIELD (m_TextFontStretch)
	TEST_FIELD (m_TextFontSize)
	return true;
}

void Theme::RemoveClient (Object *client)
{
	set <Object*>::iterator iter = m_Clients.find (client);
	if (iter != m_Clients.end ())
		m_Clients.erase (iter);
	if (m_ThemeType == FILE_THEME_TYPE && m_Clients.size () == 0) {
		TheThemeManager.RemoveFileTheme (this);
		delete this;
	}
}

void Theme::NotifyChanged ()
{
	set <Object*>::iterator iter, enditer = m_Clients.end ();
	for (iter = m_Clients.begin (); iter != enditer; iter++)
		(*iter)->OnSignal (OnThemeChangedSignal, NULL);
}

}	//	namespace gcp
