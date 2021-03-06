// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * xml-utils.h
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


#ifndef GCU_XML_UTILS_H
#define GCU_XML_UTILS_H

#include <libxml/parser.h>
#include "chemistry.h"
#include <goffice/goffice.h>

namespace gcu {

/*!\file
Some helper functions to load / save Object instances.
*/

/*!ReadPosition
@param node: a pointer to the current parsed XML Node.
@param name: the name of the searched node.
@param id: the id property of the node or NULL if not significant. This parameter might be omitted in the last case.

Helper method used to search the xmlNode instance of name Name and with id property equal to id
in the children of node. When id is NULL, the searched node does not have an id property.
When id is not NULL, the searched node should be unique.

@return the matching node or NULL.
*/
xmlNodePtr FindNodeByNameAndId (xmlNodePtr node, const char* name, const char* id = NULL);

/*!
@param node: a pointer to the xmlNode representing an Object instance.
@param id: the value of the id property of the position. This is used for example for arrows because both the head and the tail
of the arrow are positions.
@param x: a pointer to the x coordinate of the position.
@param y: a pointer to the y coordinate of the position.
@param z: a pointer to the z coordinate of the position or NULL for 2D objects.

This method can be used to load the coordinates of a point in an Object instance. It should be used in cunjunction with WritePosition() which
executes the inverse operation. If id is NULL, a position with no id property will be search and parsed.

@return true on success and false on failure.
*/
bool ReadPosition (xmlNodePtr node, const char* id, double* x, double* y, double* z = NULL);
/*!WritePosition
@param xml: a pointer to the xmlDoc used to serialize the document.
@param node: a pointer to the xmlNode representing an Object instance.
@param id: the value of the id property of the position. This is used for example for arrows because both the head and the tail
of the arrow are positions.
@param x: the x coordinate of the position.
@param y: the y coordinate of the position.
@param z: the z coordinate of the position.

Save a position as an XML node added to the children of node. This position can then be parsed using ReadPosition().

@return true on success and false on failure.
*/
bool WritePosition (xmlDocPtr xml, xmlNodePtr node, const char* id, double x, double y, double z = 0.0);

/*!
@param node: a pointer to the xmlNode representing an Object instance.
@param id: the value of the id property of the color. This is used for Object instances having at least two colors.
@param red: a pointer to the red component of the color.
@param green: a pointer to the green component of the color.
@param blue: a pointer to the blue component of the color.
@param alpha: a pointer to the alpha component of the color or NULL if tranparency is not managed.

This method can be used to load a color in an Object instance. It should be used in cunjunction with WriteColor() which
executes the inverse operation. If id is NULL, a color with no id property will be search and parsed.

@return true on success and false on failure.
*/
bool ReadColor (xmlNodePtr node, const char* id, float* red, float* green, float* blue, float* alpha = NULL);
/*!
@param xml: a pointer to the xmlDoc used to serialize the document.
@param node: a pointer to the xmlNode representing an Object instance.
@param id: the value of the id property of the color. This is used for Object instances having at least two colors.
of the arrow are positions.
@param red: the red component of the color.
@param green: the green component of the color.
@param blue: the blue component of the color.
@param alpha: the alpha component of the color.

Save a color as an XML node added to the children of node. This color can then be parsed using ReadColor().

@return true on success and false on failure.
*/
bool WriteColor (xmlDocPtr xml, xmlNodePtr node, const char* id, double red, double green, double blue, double alpha = 1.0);

/*!
@param node: a pointer to the xmlNode from which the color must be retrieved.
Builds a GOColor from the node properties.

@return the GOColor found or black (0xff).
*/
GOColor ReadColor (xmlNodePtr node);

/*!
@param node: a pointer to the xmlNode to which the color must be serialized.
@param color: a pointer to the xmlNode from which the color must be retrieved.
Serializes \a color to \a node.
*/
void WriteColor (xmlNodePtr node, GOColor color);

/*!
@param node: a pointer to the xmlNode representing an Object instance.
@param radius: a GcuAtomicRadius structure.

This function parses the XML node representing a radius (written using WriteRadius()) and fills the fields in the GcuAtomicRadius
structure with the data parsed.

@return true on success and false on failure.
*/
bool ReadRadius (xmlNodePtr node, GcuAtomicRadius& radius);

/*!
@param xml: a pointer to the xmlDoc used to serialize the document.
@param node: a pointer to the xmlNode representing an Object instance.
@param radius: a GcuAtomicRadius structure.

Save a radius as an XML node added to the children of node. This radius can then be parsed using ReadRadius().

@return true on success and false on failure.
*/
bool WriteRadius (xmlDocPtr xml, xmlNodePtr node, const GcuAtomicRadius& radius);

/*!
@param buf: a string to convert to a static instance.

This function is used to replace a dynamically allocated string by a static instance. It
works managing a set of string instances. It is used for scale names (as for radii and
electronegativity), hence its name.

@return a static version of buf or NULL if a new static string could not be created.
*/
char const *GetStaticScale (char *buf);

/*!
@param node a pointer to an XML node.
@param name the property name.
@param value the value for the property.

Adds \a value as a property with name \a name to \a node. This method is
locale independent.
*/
	void WriteFloat (xmlNodePtr node, char const *name, double value);

/*!
@param node a pointer to an XML node.
@param name the property name.
@param value where to store the result.
@param default_value the default value for the property if the property can't be retrieved.

Retrieves \a value from the \a node property with name \a name. Sets the default
value if somthing goes wrong. This method is locale independent.
@return true if the value could be retrieved.
*/
bool ReadFloat (xmlNodePtr node, char const *name, double &value, double default_value = 0.);

/*!
@param node a pointer to an XML node.
@param name the property name.
@param value the value for the property.

Adds \a value as a property with name \a name to \a node.
*/
void WriteInt (xmlNodePtr node, char const *name, int value);

/*!
@param node a pointer to an XML node.
@param name the property name.
@param value where to store the result.
@param default_value the default value for the property if the property can't be retrieved.

Retrieves \a value from the \a node property with name \a name. Sets the default
value if somthing goes wrong.
@return true if the value could be retrieved.
*/
bool ReadInt (xmlNodePtr node, char const *name, int &value, int default_value = 0);

/*!
@param node a pointer to an XML node.
@param name the property name.
@param date the date to serialize.

Adds \a date as a property with name \a name to \a node. This method is locale
independent. The date will be stored in the format MM/DD/YYYY.
*/
void WriteDate (xmlNodePtr node, char const *name, GDate const *date);

/*!
@param node a pointer to an XML node.
@param name the property name.
@param date where to store the result.

Retrieves \a date from the \a node property with name \a name. On failure,
 \a date will be cleared using g_date_clear().
@return true if the date could be retrieved. This method is locale
independent. The date must be stored in the format MM/DD/YYYY.
*/
bool ReadDate (xmlNodePtr node, char const *name, GDate *date);

/*!
@param node a pointer to an XML node.
@param name the property name.
@param dt the date to serialize.

Adds \a date as a property with name \a name to \a node. This method is locale
independent. The date will be stored in the format MM/DD/YYYY hh:mm::ss.
*/
void WriteDate (xmlNodePtr node, char const *name, GDateTime *dt);

/*!
@param node a pointer to an XML node.
@param name the property name.
@param dt where to store the result.

Retrieves \a date from the \a node property with name \a name. On failure,
 \a date will be cleared using g_date_clear().
@return true if the date could be retrieved. This method is locale
independent. The date must be stored in the format MM/DD/YYYY hh:mm::ss.
*/
bool ReadDate (xmlNodePtr node, char const *name, GDateTime **dt);

xmlDocPtr ReadXMLDocFromFile (GFile *file, char const *uri, char const *encoding, GOCmdContext *ctxt);
xmlDocPtr ReadXMLDocFromURI (char const *uri, char const *encoding, GOCmdContext *ctxt);
}	//	namespace gcu

#endif	// GCU_XML_UTILS_H
