// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * macros.h 
 *
 * Copyright (C) 2001-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_MACROS_H
#define GCU_MACROS_H

#define GCU_PROP(type,member) \
public:	\
	void Set##member (type val) {m_##member = val;}	\
	type Get##member (void) {return m_##member;}	\
	type &GetRef##member (void) {return m_##member;}	\
private:	\
	type m_##member;

#define GCU_RO_PROP(type,member) \
public:	\
	type Get##member (void) {return m_##member;}	\
private:	\
	type m_##member;

#define GCU_PROT_PROP(type,member) \
public:	\
	type Get##member (void) {return m_##member;}	\
protected:	\
	type m_##member;

#define GCU_GCONF_GET(key,type,target,defaultval) \
	type target = gconf_client_get_##type (m_ConfClient, key, &error); \
	if (error) {	\
		target = defaultval;	\
		g_message ("GConf failed: %s", error->message);	\
		g_error_free (error);	\
		error = NULL;	\
	}	\
	if (target == (type) 0)	\
		target = defaultval;

#define GCU_GCONF_GET_NO_CHECK(key,type,target,defaultval) \
	target = gconf_client_get_##type (m_ConfClient, key, &error); \
	if (error) {	\
		target = defaultval;	\
		g_message ("GConf failed: %s", error->message);	\
		g_error_free (error);	\
		error = NULL;	\
	}

#define GCU_GCONF_GET_N_TRANSFORM(key,type,target,defaultval,func) \
	{	\
		type val = gconf_client_get_##type (m_ConfClient, key, &error); \
		if (error) {	\
			val = defaultval;	\
			g_message ("GConf failed: %s", error->message);	\
			g_error_free (error);	\
			error = NULL;	\
		}	\
		if (val == (type) 0)	\
			val = defaultval; \
		target = func (val);	\
	}

#define GCU_GCONF_GET_STRING(key,target,defaultval) \
	if (target) {	\
		g_free (target);	\
		target = NULL;	\
	}	\
	target = gconf_client_get_string (m_ConfClient, key, &error); \
	if (error) {	\
		if (defaultval)	\
			target = g_strdup (defaultval);	\
		g_message ("GConf failed: %s", error->message);	\
		g_error_free (error);	\
		error = NULL;	\
	}

#endif	//	GCU_MACROS_H
