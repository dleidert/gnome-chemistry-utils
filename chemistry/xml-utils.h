// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/xml-utils.h
 *
 * Copyright (C) 2002
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */


#ifndef GCU_XML_UTILS_H
#define GCU_XML_UTILS_H

#include <libxml/parser.h>

xmlNodePtr FindNodeByNameAndId(xmlNodePtr node, const char* name, const char* id = NULL);

bool ReadPosition(xmlNodePtr node, const char* id, double* x, double* y, double* z = NULL);
bool WritePosition(xmlDocPtr xml, xmlNodePtr node, const char* id, double x, double y, double z = 0.0);

bool ReadColor(xmlNodePtr node, const char* id, float* red, float* green, float* blue, float* alpha = NULL);
bool WriteColor(xmlDocPtr xml, xmlNodePtr node, const char* id, double red, double green, double blue, double alpha = 1.0);

#endif	// GCU_XML_UTILS_H
