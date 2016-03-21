// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * chemistry/xml-utils.cc
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

#include "config.h"
#include "xml-utils.h"
#include "macros.h"
#include <glib/gi18n-lib.h>
#include <cstring>
#include <set>
#include <sstream>
#include <string>

using namespace std;

namespace gcu {

xmlNodePtr FindNodeByNameAndId (xmlNodePtr node, const char* name, const char* id)
{
	xmlNodePtr child = node->children;
	char* tmp;
	while (child)
	{
		if (!strcmp ((char*) child->name, name))
		{
			tmp = (char*) xmlGetProp (child, (xmlChar*) "id");
			if ((!id && !tmp) || (id && tmp && !strcmp (tmp, id))) {
				if (tmp)
					xmlFree (tmp);
				break;
			} else if (tmp)
				xmlFree (tmp);
		}
		child = child->next;
	}
	return child;
}

bool ReadPosition (xmlNodePtr node, const char* id, double* x, double* y, double* z)
{
	xmlNodePtr child = FindNodeByNameAndId (node, "position", id);
	if (!child) return false;
	char* buf;
	buf = reinterpret_cast <char *> (xmlGetProp (child, (xmlChar*) "x"));
	if (buf) {
		*x = g_ascii_strtod (buf, NULL);
		xmlFree (buf);
	} else
		return false;
	buf = reinterpret_cast <char *> (xmlGetProp (child, (xmlChar*) "y"));
	if (buf) {
		*y = g_ascii_strtod (buf, NULL);
		xmlFree (buf);
	} else
		return false;
	if (z)
	{
		buf = reinterpret_cast <char *> (xmlGetProp (child, (xmlChar*) "z"));
		if (buf) {
			*z = g_ascii_strtod (buf, NULL);
			xmlFree (buf);
		} else
			*z = 0.0;
	}
	return true;
}

bool WritePosition (xmlDocPtr xml, xmlNodePtr node, const char* id, double x, double y, double z)
{
	xmlNodePtr child;
	child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("position"), NULL);
	if (child)
		xmlAddChild (node, child);
	else
		return false;
	if (id)
		xmlNewProp (child, reinterpret_cast <xmlChar const *> ("id"), reinterpret_cast <xmlChar const *> (id));
	WriteFloat (child, "x", x);
	WriteFloat (child, "y", y);
	if (z != 0.0)
		WriteFloat (child, "z", z);
	return true;
}

bool ReadColor (xmlNodePtr node, const char* id, float* red, float* green, float* blue, float* alpha)
{
	xmlNodePtr child = FindNodeByNameAndId (node, "color", id);
	double buf;
	if (!child)
		return false;
	if (!ReadFloat (child, "red", buf))
		return false;
	*red = buf;
	if (!ReadFloat (child, "green", buf))
		return false;
	*green = buf;
	if (!ReadFloat (child, "blue", buf))
		return false;
	*blue = buf;
	if (alpha)
		*alpha = (ReadFloat (child, "alpha", buf))? buf: 1.0;
	return true;
}

bool WriteColor (xmlDocPtr xml, xmlNodePtr node, const char* id, double red, double green, double blue, double alpha)
{
	xmlNodePtr child;
	child = xmlNewDocNode (xml, NULL, (xmlChar*) "color", NULL);
	if (child)
		xmlAddChild (node, child);
	else
		return false;
	if (id)
		xmlNewProp (child, (xmlChar*) "id", (xmlChar*) id);
	WriteFloat (child, "red", red);
	WriteFloat (child, "green", green);
	WriteFloat (child, "blue", blue);
	if (alpha != 1.0)
		WriteFloat (child, "alpha", alpha);
	return true;
}

GOColor ReadColor (xmlNodePtr node)
{
	double buf;
	unsigned char red = 0, green = 0, blue = 0, alpha = 0xff;
	if (ReadFloat (node, "red", buf))
		red = buf * 0xff;
	if (ReadFloat (node, "green", buf))
		green = buf * 0xff;
	if (ReadFloat (node, "blue", buf))
		blue = buf * 0xff;
	if (ReadFloat (node, "alpha", buf))
		alpha = buf * 0xff;
	return GO_COLOR_FROM_RGBA (red, green, blue, alpha);
}

void WriteColor (xmlNodePtr node, GOColor color)
{
	unsigned field = GO_COLOR_UINT_R (color);
	if (field)
		WriteFloat (node, "red", static_cast <double> (field) / 0xff);
	field = GO_COLOR_UINT_G (color);
	if (field)
		WriteFloat (node, "green", static_cast <double> (field) / 0xff);
	field = GO_COLOR_UINT_B (color);
	if (field)
		WriteFloat (node, "blue", static_cast <double> (field) / 0xff);
	field = GO_COLOR_UINT_A (color);
	if (field != 0xff)
		WriteFloat (node, "alpha", static_cast <double> (field) / 0xff);
}

bool ReadRadius (xmlNodePtr node, GcuAtomicRadius& radius)
{
	char *tmp, *dot, *end;
	tmp = (char*) xmlGetProp (node, (xmlChar*) "type");
	if (!tmp ||
		((!((!strcmp (tmp, "unknown")) && (radius.type = GCU_RADIUS_UNKNOWN))) &&
		(!((!strcmp (tmp, "covalent")) && (radius.type = GCU_COVALENT))) &&
		(!((!strcmp (tmp, "vdW")) && (radius.type = GCU_VAN_DER_WAALS))) &&
		(!((!strcmp (tmp, "ionic")) && (radius.type = GCU_IONIC))) &&
		(!((!strcmp (tmp, "metallic")) && (radius.type = GCU_METALLIC))) &&
		(!((!strcmp (tmp, "atomic")) && (radius.type = GCU_ATOMIC)))))
			radius.type = GCU_RADIUS_UNKNOWN;
	if (tmp)
		xmlFree (tmp);
	tmp = (char*) xmlGetProp (node, (xmlChar*) "scale");
	if (tmp) {
		radius.scale = GetStaticScale (tmp);
		xmlFree (tmp);
	} else
		radius.scale = NULL;
	tmp = (char*) xmlGetProp (node, (xmlChar*) "charge");
	if (tmp) {
		radius.charge = strtol (tmp, NULL, 10);
		xmlFree (tmp);
	} else
		radius.charge = 0;
	tmp = (char*) xmlGetProp (node, (xmlChar*)"cn");
	if (tmp) {
		radius.cn = strtol (tmp, NULL, 10);
		xmlFree (tmp);
	} else
		radius.cn = -1;
	tmp = (char*) xmlGetProp(node, (xmlChar*) "spin");
	if ((!tmp) ||
		((!((!strcmp (tmp, "low")) && (radius.spin = GCU_LOW_SPIN))) &&
		(!((!strcmp (tmp, "high")) && (radius.spin = GCU_HIGH_SPIN)))))
	radius.spin = GCU_N_A_SPIN;
	if (tmp)
		xmlFree(tmp);
	if (((tmp = (char*) xmlGetProp (node, (xmlChar*) "value")) ||
		(tmp = (char*) xmlNodeGetContent (node))) && *tmp) {
		radius.value.value = strtod (tmp, &end);
		dot = strchr (tmp, '.');
		radius.value.prec = (dot)? end - dot - 1: 0;
		radius.scale = "custom";
		xmlFree(tmp);
	} else {
		if (tmp)
			xmlFree(tmp);
		if (radius.scale && (!strcmp (radius.scale, "custom")))
			return false;
		else if (!gcu_element_get_radius (&radius))
			return false;
	}
	if (radius.value.value <= 0.0)
		return false;
	return true;
}

bool WriteRadius (xmlDocPtr xml, xmlNodePtr node, const GcuAtomicRadius& radius)
{
	xmlNodePtr child;
	char buf[256];
	char const *tmp;

	child = xmlNewDocNode (xml, NULL, (xmlChar*) "radius", NULL);
	if (child)
		xmlAddChild (node, child);
	else return false;
	switch (radius.type) {
	case GCU_RADIUS_UNKNOWN:
		tmp = NULL;
		break;
	case GCU_ATOMIC:
		tmp = "atomic";
		break;
	case GCU_IONIC:
		tmp = "ionic";
		break;
	case GCU_METALLIC:
		tmp = "metallic";
		break;
	case GCU_COVALENT:
		tmp = "covalent";
		break;
	case GCU_VAN_DER_WAALS:
		tmp = "vdW";
		break;
	default:
		tmp = NULL;
	}
	if (tmp)
		xmlNewProp (child, (xmlChar*) "type", (xmlChar*) tmp);
	if ((radius.type == GCU_RADIUS_UNKNOWN) || (radius.scale && (!strcmp (radius.scale, "custom")))) {
		char *format = g_strdup_printf ("%%0.%df",radius.value.prec);
		g_snprintf (buf, sizeof (buf) - 1, format, radius.value.value);
		g_free (format);
		xmlNewProp (child, (xmlChar*) "value", (xmlChar*) buf);
	}
	if (radius.scale &&  strcmp (radius.scale, "custom"))
		xmlNewProp (child, (xmlChar*) "scale", (xmlChar*) radius.scale);
	if (radius.charge) {
		g_snprintf (buf, sizeof (buf) - 1, "%d", radius.charge);
		xmlNewProp (child, (xmlChar*) "charge", (xmlChar*) buf);
	}
	if (radius.cn != -1) {
		g_snprintf (buf, sizeof (buf) - 1, "%d", radius.cn);
		xmlNewProp (child, (xmlChar*) "cn", (xmlChar*) buf);
	}
	if (radius.spin != GCU_N_A_SPIN)
		xmlNewProp (child, (xmlChar*) "spin", (xmlChar*)((radius.spin == GCU_LOW_SPIN)? "low": "high"));
	return true;
}

// we must have static strings
static set <string> ScaleNames;

char const *GetStaticScale (char *buf)
{
	set <string>::iterator i = ScaleNames.find (buf);
	if (i == ScaleNames.end ()) {
		std::pair<set <string>::iterator,bool> res = ScaleNames.insert (buf);
		if (res.second)
			return (*res.first).c_str ();
		else
			return NULL;
	}
	return (*i).c_str ();
}

void WriteFloat (xmlNodePtr node, char const *name, double value)
{
	static char buf[G_ASCII_DTOSTR_BUF_SIZE];
	g_ascii_dtostr (buf, G_ASCII_DTOSTR_BUF_SIZE, value);
	xmlNewProp (node, reinterpret_cast <xmlChar const *> (name), reinterpret_cast <xmlChar const *> (buf));
}

bool ReadFloat (xmlNodePtr node, char const *name, double &value, double default_value)
{
	xmlChar *buf = xmlGetProp (node, reinterpret_cast <xmlChar const *> (name));
	char *end;
	if (!buf) {
		value = default_value;
		return false;
	}
	value = g_ascii_strtod (reinterpret_cast <char *> (buf), &end);
	if (end && (*end)) {
		xmlFree (buf);
		value = default_value;
		return false;
	}
	xmlFree (buf);
	return true;
}

void WriteInt (xmlNodePtr node, char const *name, int value)
{
	char *buf = g_strdup_printf ("%d", value);
	xmlNewProp (node, CC2XML(name), CC2XML (buf));
	g_free (buf);
}

bool ReadInt (xmlNodePtr node, char const *name, int &value, int default_value)
{
	xmlChar *buf = xmlGetProp (node, CC2XML (name));
	char *end;
	if (!buf) {
		value = default_value;
		return false;
	}
	value = strtol (XML2CC (buf), &end, 10);
	if (end && (*end)) {
		xmlFree (buf);
		value = default_value;
		return false;
	}
	xmlFree (buf);
	return true;
}

void WriteDate (xmlNodePtr node, char const *name, GDate const *date)
{
	char buf[64];
	if (!g_date_valid (date))
		return;
	g_date_strftime (buf, sizeof (buf), "%m/%d/%Y", date);
	xmlNewProp (node, reinterpret_cast <xmlChar const *> (name), reinterpret_cast <xmlChar const *> (buf));
}

bool ReadDate (xmlNodePtr node, char const *name, GDate *date)
{
	xmlChar *buf = xmlGetProp (node, reinterpret_cast <xmlChar const *> (name));
	unsigned d, m, y;
	if (buf && sscanf (reinterpret_cast <char const *> (buf), "%2u/%2u/%4u", &m, &d, &y)) {
		xmlFree (buf);
		g_date_set_dmy (date, d, static_cast <GDateMonth> (m), y);
		bool res = g_date_valid (date);
		if (!res)
			g_date_clear (date, 1);
		return res;
	} else
		return false;
}

void WriteDate (xmlNodePtr node, char const *name, GDateTime *dt)
{
	if (dt == NULL)
		return;
	char *buf = g_date_time_format (dt, "%m/%d/%Y %H:%M:%S");
	xmlNewProp (node, reinterpret_cast <xmlChar const *> (name), reinterpret_cast <xmlChar const *> (buf));
}

bool ReadDate (xmlNodePtr node, char const *name, GDateTime **dt)
{
	xmlChar *buf = xmlGetProp (node, reinterpret_cast <xmlChar const *> (name));
	unsigned d, m, y, hh = 0, mm = 0, res;
	float ss  =0.;
	if (buf && (res = sscanf (reinterpret_cast <char const *> (buf), "%2u/%2u/%4u %2u:%2u:%f", &m, &d, &y, &hh, &mm, &ss))) {
		xmlFree (buf);
		GTimeZone *tz = g_time_zone_new_utc ();
		*dt = g_date_time_new (tz, y, m, d, hh, mm, ss);
		g_time_zone_unref (tz);
		return res >= 3;
	} else
		return false;
}

struct XmlReadState {
	GInputStream *input;
	GOCmdContext *ctxt;
};

static int	cb_vfs_to_xml (struct XmlReadState *state, char* buf, int nb)
{
	GError *error = NULL;
	int n = g_input_stream_read (state->input, buf, nb, NULL, &error);
	if (error) {
		if (state->ctxt)
			go_cmd_context_error (state->ctxt, error);
		else
			g_message ("GIO error: %s\n", error->message);
		g_error_free (error);
	}
	return n;
}

static int	cb_state_release (struct XmlReadState *state)
{
	g_input_stream_close (state->input, NULL, NULL);
	return 0;
}

xmlDocPtr ReadXMLDocFromFile (GFile *file, char const *uri, char const *encoding, GOCmdContext *ctxt)
{
	GError *error = NULL;
	GInputStream *input = G_INPUT_STREAM (g_file_read (file, NULL, &error));
	xmlDocPtr xml = NULL;
	struct XmlReadState state;
	if (error) {
		go_cmd_context_error (ctxt, error);
		g_error_free (error);
		return NULL;
	}
	state.input = input;
	state.ctxt = ctxt;
	xmlKeepBlanksDefault (1); // to be sure we don't loose significant spaces.
	if (!(xml = xmlReadIO ((xmlInputReadCallback) cb_vfs_to_xml,
	                       (xmlInputCloseCallback) cb_state_release, &state,
	                       uri, encoding, 0))) {
		if (ctxt)
			go_cmd_context_error_import (ctxt, _("Error while parsing XML"));
		else
			g_message (_("Error while parsing XML"));
	}
	return xml;
}

xmlDocPtr ReadXMLDocFromURI (char const *uri, char const *encoding, GOCmdContext *ctxt)
{
	GFile *file = g_file_new_for_uri (uri);
	xmlDocPtr xml = ReadXMLDocFromFile (file, uri, encoding, ctxt);
	g_object_unref (file);
	return xml;
}

}	//	namespace gcu
