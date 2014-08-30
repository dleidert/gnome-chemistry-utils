// -*- C++ -*-

/*
 * GChemPaint library
 * theme.h
 *
 * Copyright (C) 2002-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_THEME_H
#define GCHEMPAINT_THEME_H

#include <gcu/macros.h>
#include <gcu/object.h>
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
/*!
@param name the name of the new theme.

Constructs a new theme according to the local settings.
*/
	Theme (char const *name);
/*!
The destructor.
*/
	~Theme ();

/*!
@return the theme name.
*/
	std::string &GetName () {return m_Name;}

/*!
@return the theme name.
*/
	std::string &GetFileName () {return (m_FileName.length ())? m_FileName: m_Name;}

/*!
@param xml the xml document used for serialization.

Builds an xml node containing the serialized theme.
@return the new xml node or NULL if an error occured.
*/
	bool Save (xmlDocPtr xml);
/*!
@param node the xml node containing the serialized theme.

Loads a theme in memory, either from the theme database or from a document file.
*/
	bool Load (xmlNodePtr node);
/*!
@param theme a theme to compare.

@return true if the two themes have identical settings.
*/
	bool operator== (const Theme &theme);
/*!
@param client the client to add.

Adds a new client to the list of this theme clients.
*/
	void AddClient (gcu::Object *client) {m_Clients.insert (client);}
/*!
@param client the client to remove.

Removes \a client to the list of this theme clients.
*/
	void RemoveClient (gcu::Object *client);
/*!
Notify all the theme clients that at least one setting changed.
*/
	void NotifyChanged ();

private:
	std::string m_Name, m_FileName;
	std::set <gcu::Object*> m_Clients;
	bool modified;
	bool locked;

/*!\fn GetBondLength()
@return the theme default bond length in picometers.
*/
GCU_RO_PROP (double, BondLength)
/*!\fn GetBondAngle()
@return the theme default angle between two consecutive bonds in a chain in degrees.
*/
GCU_RO_PROP (double, BondAngle)
/*!\fn GetBondDist()
@return the theme default distance between two lines in pixels (at 100% zoom) in a multiple bond.
*/
GCU_RO_PROP (double, BondDist)
/*!\fn GetBondWidth()
@return the theme default bond line width in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, BondWidth)
/*!\fn GetArrowLength()
@return the theme default arrow length in picometers.
*/
GCU_RO_PROP (double, ArrowLength)
/*!\fn GetHashWidth()
@return the theme default line width in pixels for hash bonds (at 100% zoom).
*/
GCU_RO_PROP (double, HashWidth)
/*!\fn GetHashDist()
@return the theme default distance between two lines in pixels for hash bonds (at 100% zoom).
*/
GCU_RO_PROP (double, HashDist)
/*!\fn GetStereoBondWidth()
@return the theme default largest width for hash or wedge bonds (at 100% zoom).
*/
GCU_RO_PROP (double, StereoBondWidth)
/*!\fn GetZoomFactor()
@return the theme default scale used to convert real distance to canvas distance expressed in pixel per pm (at 100% zoom).
*/
GCU_RO_PROP (double, ZoomFactor)
/*!\fn GetPadding()
@return the theme default padding used around text objects such as atoms, texts and other typographic signs in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, Padding)
/*!\fn GetArrowHeadA()
@return the theme default distance from tip of arrowhead to center in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, ArrowHeadA)
/*!\fn GetArrowHeadB()
@return the theme default distance from tip of arrowhead to trailing point, measured along shaft, in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, ArrowHeadB)
/*!\fn GetArrowHeadC()
@return the theme default distance of arrowhead trailing points from outside edge of shaft in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, ArrowHeadC)
/*!\fn GetArrowDist()
@return the theme default distance between two lines in pixels (at 100% zoom) for double arrows or for retrosynthesis arrows.
*/
GCU_RO_PROP (double, ArrowDist)
/*!\fn GetArrowWidth()
@return the theme default arrow line width in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, ArrowWidth)
/*!\fn GetArrowPadding()
@return the theme default padding between arrows and associated objects (reactants, mesomers,...) in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, ArrowPadding)
/*!\fn GetArrowObjectPadding()
@return the theme default padding between arrows ends and attached objects bounds in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, ArrowObjectPadding)
/*!\fn GetStoichiometryPadding()
@return the theme default extra padding between a stoichiometric coefficient and its associated molecule in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, StoichiometryPadding)
/*!\fn GetObjectPadding()
@return the theme default padding in pixels added between consecutive objects during an alignment operation (at 100% zoom).
*/
GCU_RO_PROP (double, ObjectPadding)
/*!\fn GetSignPadding()
@return the theme default padding between a '+' sign in a reaction equation and reactants symbols in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, SignPadding)
/*!\fn GetChargeSignSize()
@return the theme default size of the charge sign in pixels (at 100% zoom).
*/
GCU_RO_PROP (double, ChargeSignSize)
/*!\fn GetFontFamily()
@return the theme font family used for chemical formula such as atoms and fragments
*/
GCU_RO_PROP (gchar*, FontFamily)
/*!\fn GetFontStyle()
@return the theme font style used for chemical formula such as atoms and fragments.
*/
GCU_RO_PROP (PangoStyle, FontStyle)
/*!\fn GetFontWeight()
@return the theme font weight used for chemical formula such as atoms and fragments.
*/
GCU_RO_PROP (PangoWeight, FontWeight)
/*!\fn GetFontVariant()
@return the theme font variant used for chemical formula such as atoms and fragments.
*/
GCU_RO_PROP (PangoVariant, FontVariant)
/*!\fn GetFontStretch()
@return the theme font stretch used for chemical formula such as atoms and fragments.
*/
GCU_RO_PROP (PangoStretch, FontStretch)
/*!\fn GetFontSize()
@return the theme font size used for chemical formula such as atoms and fragments.
*/
GCU_RO_PROP (gint, FontSize)
/*!\fn GetTextFontFamily()
@return the theme default font family used for texts.
*/
GCU_RO_PROP (gchar*, TextFontFamily)
/*!\fn GetTextFontStyle()
@return the theme default font style used for texts.
*/
GCU_RO_PROP (PangoStyle, TextFontStyle)
/*!\fn GetTextFontWeight()
@return the theme default font weight used for texts.
*/
GCU_RO_PROP (PangoWeight, TextFontWeight)
/*!\fn GetTextFontVariant()
@return the theme default font variant used for texts.
*/
GCU_RO_PROP (PangoVariant, TextFontVariant)
/*!\fn GetTextFontStretch()
@return the theme default font stretch used for texts.
*/
GCU_RO_PROP (PangoStretch, TextFontStretch)
/*!\fn GetTextFontSize()
@return the theme default text font size.
*/
GCU_RO_PROP (gint, TextFontSize)
/*!\fn GetThemeType()
@return the type of the theme as definedby the ThemeType enumeration.
*/
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
/*!
Constructs a theme manager.
*/
	ThemeManager ();
/*!
The destructor.
*/
	~ThemeManager ();

/*!
@param name the name of the requested theme.

@return the theme corresponding to \a name if any, or NULL.
*/
	Theme *GetTheme (char const *name);
/*!
@param name the name of the requested theme.

@return the theme corresponding to \a name if any, or NULL.
*/
	Theme *GetTheme (std::string &name);
/*!
@return the list of all theme names currently in use.
*/
	std::list <std::string> const &GetThemesNames ();
/*!
@param node the GOConfNode for which a key value changed.
@param name the key whose value changed.

Called by the framework when the value associated with \a name changed in the
settings.
*/
	void OnConfigChanged (GOConfNode *node, gchar const *name);
/*!
@param theme a theme to duplicate.

Creates a new theme based on \a theme or on the default theme if \a theme is NULL.
*/
	Theme *CreateNewTheme (Theme *theme = NULL);
/*!
@param theme a theme imported from a document file.
@param label the document label.

Called when opening a file if the theme saved with the file has the same name
than an already registered theme, but different settings. The theme will not be
saved to the local database, and will be registered using \a label and the theme
name.
*/
	void AddFileTheme (Theme *theme, char const *label);
/*!
@param theme the themeto remove from the list.

Called for theme registered using AddFileName() typically when the associated
document file is closed.
*/
	void RemoveFileTheme (Theme *theme);
/*!
@param theme a theme.
@param name the new name for the theme.

Changes the name of the theme.
*/
	void ChangeThemeName (Theme *theme, char const *name);
/*!
@return the default theme.
*/
	Theme *GetDefaultTheme () {return m_DefaultTheme;}
/*!
@param name the name of the default theme.

Sets the default theme. This theme is used when creating new documents with no
explicit theme.
*/
	void SetDefaultTheme (char const *name);
/*!
Closes configuration change notification. Should be called once before the
destructionof the theme manager.
*/
	void Shutdown ();

private:
	void ParseDir (std::string &path, ThemeType type);

private:
	std::map <std::string, Theme*> m_Themes;
	std::list <std::string> m_Names;
	GOConfNode *m_ConfNode;
	unsigned m_NotificationId;
	Theme *m_DefaultTheme;
};

/*!
The themes manager unique instance.
*/
extern ThemeManager TheThemeManager;

}	//	namespace gcp

#endif	//	GCHEMPAINT_THEME_H
