/*
 * Gnome Chemistry Utils
 * cmd-context.h
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_CMD_CONTEXT_H
#define GCU_CMD_CONTEXT_H

#include "macros.h"
#include <goffice/goffice.h>

namespace gcu {

class Application;

/*!
\class CmdContext gcu/cmd-context.h

\brief error output channel.

This class is used to display errors. It uses a <a href="http://developer.gnome.org/goffice/unstable/goffice-0.10-go-cmd-context.html">GOCmdContext</a> from Goffice
internally. This class is actually a virtual base class.
*/

class CmdContext {
public:
	friend class Application;
/*!
@param App the application for which the CmdContext is created.

Initializes a CmdContext and attach it to \a App.
*/
	CmdContext (Application *App);

/*!
The destructor.
*/
	virtual ~CmdContext ();

/*!\enum Response gcu/cmd-context.h
\brief Responses which might be proposed to the user.

This enum list the various answers that might be proposed to the user in case
an error occured.
*/
	typedef enum {
/*!
The default response if none of the others applies.
*/
		ResponseDefault,
/*!
Answer to the question is 'OK'.
*/
		ResponseOK = 1 << 0,
/*!
Answer to the question is 'Cancel'.
*/
		ResponseCancel = 1 << 1,
/*!
Answer to the question is 'Yes'.
*/
		ResponseYes = 1 << 2,
/*!
Answer to the question is 'No'.
*/
		ResponseNo = 1 << 3,
/*!
Close the message and don't do anything else.
*/
		ResponseClose = 1 << 4
	} Response;

/*!\enum Severity gcu/cmd-context.h
\brief Error severity.

This enum list the severity of the errors that might occur.
*/
	typedef enum {
/*!
A simple information.
*/
		SeverityMessage,
/*!
A warning: something wrong occured but the program can proceed anyway.
*/
		SeverityWarning,
/*!
An error: the program can't do what was requested.
*/
		SeverityError
	} Severity;

/*!
@param message the message string.
@param responses the responses that are meaningful such as 'ResponseYes | ResponseNo'.

Ask a question to the user about what should be done.
@return the response chosen by the user. ResponseDefault means that the message
was aborted without choosing one of the proposed answers.
This method is pure virtual and must be implemented in derived classes.
*/
	virtual Response GetResponse (char const *message, int responses) = 0;

/*!
@param message the message string.
@param severity the Severity of the situation.
@param modal whether the message should block the program execution or not.

Inform the user of an abnormal situation.
This method is pure virtual and must be implemented in derived classes.
*/
	virtual void Message (char const *message, Severity severity, bool modal) = 0;

/*!
Just calls <a href="http://developer.gnome.org/goffice/unstable/goffice-0.10-io-context.html#go-io-context-new">go_io_context_new()</a>.
@return a new <a href="http://developer.gnome.org/goffice/unstable/goffice-0.10-io-context.html">GOIOContext</a>
associated with the internal <a href="http://developer.gnome.org/goffice/unstable/goffice-0.10-go-cmd-context.html">GOCmdContext</a></a>.
*/
	GOIOContext *GetNewGOIOContext ();

/*!\var m_GOCmdContext
The associated <a href="http://developer.gnome.org/goffice/unstable/goffice-0.10-go-cmd-context.html">GOCmdContext</a></a>.
*/
/*!\fn GetGOCmdContext()
@return the associated <a href="http://developer.gnome.org/goffice/unstable/goffice-0.10-go-cmd-context.html">GOCmdContext</a></a>.
*/
GCU_PROT_PROP (GOCmdContext *, GOCmdContext)

/*!\var m_App
The Application instance associated with the CmdContext.
*/
/*!\fn GetApp()
@return a pointer to the Application instance associated with the CmdContext.
*/
GCU_PROT_POINTER_PROP (Application, App)
};

}	//	namespace gcu

#endif //	GCU_CMD_CONTEXT_H
