// -*- C++ -*-

/* 
 * GChemPaint library
 * theme.h
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

#ifndef GCHEMPAINT_THEME_H
#define GCHEMPAINT_THEME_H

#include <gcu/macros.h>
#include <gcu/object.h>
#include <glib/gtypes.h>
#include <pango/pango.h>
#ifdef HAVE_GO_CONF_SYNC
#include <goffice/app/go-conf.h>
#else
#include <gconf/gconf-client.h>
#endif
#include <libxml/tree.h>
#include <list>
#include <map>
#include <string>
#include <set>

/*!\file*/

namespace gcp {

/*!\enum ThemeType gcp/theme.h
Enumerates the various storage classes for themes.
*/
typedef enum {
/*!
The default theme.
*/
	DEFAULT_THEME_TYPE,
/*!
A user defined theme which might be modified on the fly.
*/
	LOCAL_THEME_TYPE,
/*!
A global theme which can't be edited.
*/
	GLOBAL_THEME_TYPE,
/*!
A theme loaded from a data file. Such themes can be modified.
*/
	FILE_THEME_TYPE
} ThemeType;

/*!\class Theme gcp/theme.h
\brief themes class.

Represent an edition settings in GChemPaint.
*/
class Theme
{
friend class ThemeManager;
friend class PrefsDlg;

public:
	Theme (char const *name);
	~Theme ();

	std::string &GetName () {return m_Name;}
	bool Save (xmlDocPtr);
	bool Load (xmlNodePtr);
	bool operator== (const Theme &theme);
	void AddClient (gcu::Object *client) {m_Clients.insert (client);}
	void RemoveClient (gcu::Object *client);
	void NotifyChanged ();

private:
	std::string m_Name;
	std::set <gcu::Object*> m_Clients;
	bool modified;

GCU_RO_PROP (double, BondLength)
GCU_RO_PROP (double, BondAngle)
GCU_RO_PROP (double, BondDist)
GCU_RO_PROP (double, BondWidth)
GCU_RO_PROP (double, ArrowLength)
GCU_RO_PROP (double, HashWidth)
GCU_RO_PROP (double, HashDist)
GCU_RO_PROP (double, StereoBondWidth)
GCU_RO_PROP (double, ZoomFactor)
GCU_RO_PROP (double, Padding)
GCU_RO_PROP (double, ArrowHeadA)
GCU_RO_PROP (double, ArrowHeadB)
GCU_RO_PROP (double, ArrowHeadC)
GCU_RO_PROP (double, ArrowDist)
GCU_RO_PROP (double, ArrowWidth)
GCU_RO_PROP (double, ArrowPadding)
GCU_RO_PROP (double, ArrowObjectPadding)
GCU_RO_PROP (double, StoichiometryPadding)
GCU_RO_PROP (double, ObjectPadding)
GCU_RO_PROP (double, SignPadding)
GCU_RO_PROP (double, ChargeSignSize)
GCU_RO_PROP (gchar*, FontFamily)
GCU_RO_PROP (PangoStyle, FontStyle)
GCU_RO_PROP (PangoWeight, FontWeight)
GCU_RO_PROP (PangoVariant, FontVariant)
GCU_RO_PROP (PangoStretch, FontStretch)
GCU_RO_PROP (gint, FontSize)
GCU_RO_PROP (gchar*, TextFontFamily)
GCU_RO_PROP (PangoStyle, TextFontStyle)
GCU_RO_PROP (PangoWeight, TextFontWeight)
GCU_RO_PROP (PangoVariant, TextFontVariant)
GCU_RO_PROP (PangoStretch, TextFontStretch)
GCU_RO_PROP (gint, TextFontSize)
GCU_RO_PROP (ThemeType, ThemeType);
};

/*!\class ThemeManager gcp/theme.h
\brief themes engine class.

Represent the themes set. Only one global object of this class exists
in GChemPaint.
*/
class ThemeManager
{
public:
	ThemeManager ();
	~ThemeManager ();

	Theme *GetTheme (char const *name);
	Theme *GetTheme (std::string &name);
	std::list <std::string> const &GetThemesNames ();
#ifdef HAVE_GO_CONF_SYNC
	void OnConfigChanged (GOConfNode *node, gchar const *name);
#else
	void OnConfigChanged (GConfClient *client,  guint cnxn_id, GConfEntry *entry);
#endif
	Theme *CreateNewTheme (Theme *theme = NULL);
	void AddFileTheme (Theme *theme, char const *label);
	void RemoveFileTheme (Theme *theme);
	void ChangeThemeName (Theme *theme, char const *name);
	Theme *GetDefaultTheme () {return m_DefaultTheme;}
	void SetDefaultTheme (char const *name);
	void Shutdown ();

private:
	void ParseDir (std::string &path, ThemeType type);

private:
	std::map <std::string, Theme*> m_Themes;
	std::list <std::string> m_Names;
#ifdef HAVE_GO_CONF_SYNC
	GOConfNode *m_ConfNode;
#else
	GConfClient *m_ConfClient;
#endif
	guint m_NotificationId;
	Theme *m_DefaultTheme;
};

/*!
The themes manager unique instance.
*/
extern ThemeManager TheThemeManager;

}	//	namespace gcp

#endif	//	GCHEMPAINT_THEME_H
