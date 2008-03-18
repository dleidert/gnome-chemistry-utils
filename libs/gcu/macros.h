// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * macros.h 
 *
 * Copyright (C) 2001-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifdef HAVE_GO_CONF_SYNC
#	include <goffice/app/go-conf.h>
#else
#	include <gconf/gconf-client.h>
#endif

/*!\file */ 
/*!\def GCU_PROP()
Defines a private member with appropriate get/set methods.
GCU_PROP((Type,Foo) expands to one private member:
\code
	Type m_Foo;
\endcode

and three public methods:
\code
	void SetFoo(Type val);
	Type GetFoo();
	Type& GetRefFoo();
\endcode

The last one allows code as:
\code
	obj.GetRefFoo() = val;
\endcode
*/
#define GCU_PROP(type,member) \
public:	\
	void Set##member (type val) {m_##member = val;}	\
	type Get##member (void) const {return m_##member;}	\
	type &GetRef##member (void) {return m_##member;}	\
private:	\
	type m_##member;

#define GCU_POINTER_PROP(type,member) \
public:	\
	void Set##member (type *val) {m_##member = val;}	\
	type *Get##member (void) {return m_##member;}	\
	type const *Get##member (void) const {return m_##member;}	\
private:	\
	type *m_##member;

/*!\def GCU_RO_PROP()
Defines a private member with appropriate get method. RO stands for Read Only. The member
can't be modified from outside the class it belongs too or a friend class.
GCU_RO_PROP(Type,Foo) expands to one private member:
\code
	Type m_Foo;
\endcode

and one public method:
\code
	Type GetFoo();
\endcode
*/
#define GCU_RO_PROP(type,member) \
public:	\
	type Get##member (void) {return m_##member;}	\
private:	\
	type m_##member;

#define GCU_RO_POINTER_PROP(type,member) \
public:	\
	type const *Get##member (void) const {return m_##member;}	\
private:	\
	type *m_##member;

/*!\def GCU_PROT_PROP()
Defines a protected member with appropriate get method. The member can be modified
the class it belongs too or a friend class or a derived class.
GCU_PROT_PROP(Type,Foo) expands to one protected member:
\code
	Type m_Foo;
\endcode

and one public method:
\code
	Type GetFoo();
\endcode
*/
#define GCU_PROT_PROP(type,member) \
public:	\
	type Get##member (void) {return m_##member;}	\
protected:	\
	type m_##member;

#define GCU_PROT_POINTER_PROP(type,member) \
public:	\
	type *Get##member (void) {return m_##member;}	\
	type const *Get##member (void) const {return m_##member;}	\
protected:	\
	type *m_##member;

/*!\def GCU_GCONF_GET()
This macro gets the numerical value of type \a type associated to \a key, and
copies it to \a target. If an error occurs or if the value is 0,
\a defaultval is used instead.\n
Calling class must have a GConfClient member called m_ConfClient, and the code
must provide a GError *error initially set to NULL.
*/
#ifdef HAVE_GO_CONF_SYNC
#define go_conf_get_float go_conf_get_double
#define GCU_GCONF_GET(key,type,target,defaultval) \
	target = go_conf_get_##type (m_ConfNode, key); \
	if (target == (type) 0)	\
		target = defaultval;
#else
#define GCU_GCONF_GET(key,type,target,defaultval) \
	target = gconf_client_get_##type (m_ConfClient, ROOTDIR key, &error); \
	if (error) {	\
		target = defaultval;	\
		g_message ("GConf failed: %s", error->message);	\
		g_error_free (error);	\
		error = NULL;	\
	}	\
	if (target == (type) 0)	\
		target = defaultval;
#endif
/*!\def GCU_GCONF_GET_NO_CHECK()
This macro gets the numerical value of type \a type associated to \a key, and
copies it to \a target. If an error occurs, \a defaultval is used instead.\n
Calling class must have a GConfClient member called m_ConfClient, and the code
must provide a GError *error initially set to NULL.
*/
#ifdef HAVE_GO_CONF_SYNC
#define GCU_GCONF_GET_NO_CHECK(key,type,target,defaultval) \
	target = go_conf_get_##type (m_ConfNode, key);
#else
#define GCU_GCONF_GET_NO_CHECK(key,type,target,defaultval) \
	target = gconf_client_get_##type (m_ConfClient, ROOTDIR key, &error); \
	if (error) {	\
		target = defaultval;	\
		g_message ("GConf failed: %s", error->message);	\
		g_error_free (error);	\
		error = NULL;	\
	}
#endif

/*!\def GCU_GCONF_GET_N_TRANSFORM()
This macro gets the numerical value of type \a type associated to \a key. If an error
occurs or if the value is 0, \a defaultval is used instead.\n
The resuting value (which might be the default value) is then passed
to \a func and the result is copied to \a target. \n
Calling class must have a GConfClient member called m_ConfClient, and the code
must provide a GError *error initially set to NULL.
*/
#ifdef HAVE_GO_CONF_SYNC
#define GCU_GCONF_GET_N_TRANSFORM(key,type,target,defaultval,func) \
	{	\
		type val = go_conf_get_##type (m_ConfNode, key); \
		if (val == (type) 0)	\
			val = defaultval; \
		target = func (val);	\
	}
#else
#define GCU_GCONF_GET_N_TRANSFORM(key,type,target,defaultval,func) \
	{	\
		type val = gconf_client_get_##type (m_ConfClient, ROOTDIR key, &error); \
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
#endif

/*!\def GCU_GCONF_GET_STRING()
This macro gets the string value associated to \a key, and
copies it to \a target. If an error occurs, \a defaultval is used instead.\n
If \a target is not NULL when entering the macro, it is deallocated using g_free
and set to NULL before calling gconf_client_get_string.\n
Calling class must have a GConfClient member called m_ConfClient, and the code
must provide a GError *error initially set to NULL.
*/
#ifdef HAVE_GO_CONF_SYNC
#define GCU_GCONF_GET_STRING(key,target,defaultval) \
	if (target) {	\
		g_free (target);	\
		target = NULL;	\
	}	\
	target = go_conf_get_string (m_ConfNode, key); \
	if (target == NULL && defaultval)	\
		target = g_strdup (defaultval);
#else
#define GCU_GCONF_GET_STRING(key,target,defaultval) \
	if (target) {	\
		g_free (target);	\
		target = NULL;	\
	}	\
	target = gconf_client_get_string (m_ConfClient, ROOTDIR key, &error); \
	if (error) {	\
		if (defaultval)	\
			target = g_strdup (defaultval);	\
		g_message ("GConf failed: %s", error->message);	\
		g_error_free (error);	\
		error = NULL;	\
	} else if (target == NULL && defaultval)	\
			target = g_strdup (defaultval);
#endif

#ifdef HAVE_GO_CONF_SYNC
#define GCU_UPDATE_KEY(key,type,target,action) \
	if (!strcmp (name, ROOTDIR key)) { \
		target = go_conf_get_##type (node, ((node)? key: ROOTDIR key)); \
		action \
		return; \
	}
#else
#define GCU_UPDATE_KEY(key,type,target,action) \
	if (!strcmp (gconf_entry_get_key (entry), ROOTDIR key)) { \
		target = gconf_value_get_##type (gconf_entry_get_value (entry)); \
		action \
		return; \
	}
#endif

#ifdef HAVE_GO_CONF_SYNC
#define GCU_UPDATE_STRING_KEY(key,target,action) \
	if (!strcmp (name, ROOTDIR key)) { \
		target = go_conf_get_string (node, ((node)? key: ROOTDIR key)); \
		action \
		return; \
	}
#else
#define GCU_UPDATE_STRING_KEY(key,target,action) \
	if (!strcmp (gconf_entry_get_key (entry), ROOTDIR key)) { \
		target = g_strdup (gconf_value_get_string (gconf_entry_get_value (entry))); \
		action \
		return; \
	}
#endif

#endif	//	GCU_MACROS_H
