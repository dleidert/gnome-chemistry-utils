// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/dialog.h 
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_DIALOG_H
#define GCU_DIALOG_H
#include <string>
#include <stdexcept>

/*!\file*/
namespace gcu {

class Application;
class DialogOwner;

/*!\class Dialog gcu/dialog.h
This class is base class for dialog boxes. It provides some basic services.
*/
class Dialog
{
public:
/*!
@param App: the Application which owns the dialog.
@param windowname: the name of the top level GtkWidget of the dialog box in
the glade file. This name should be unique for the application. It is used to access
the contextual help and to ensure the uniqueness of the dialog (in some cases).
@param owner: the address of an owner object, might be App or a document
or NULL (the default). when owner is not NULL, the dialog will be unique for it.
*/
	Dialog (Application* App, const char* windowname, DialogOwner *owner = NULL) throw (std::runtime_error);
	virtual ~Dialog ();

/*!
@param name a new name for the window.
@param owner the address of an dialog owner object.

Usually, the Dialog is registered using its window name as passed to the
constructor. If several occurences of the same Dialog are acceptable, another
unique name is needed. \a owner might be NULL if it has already been set in the
constructor.
The existence of a Dialog with the same name should be checked before calling this
method, because that would throw an error.
*/
	void SetRealName (char const *name, DialogOwner *owner) throw (std::runtime_error);

	virtual void Present () = 0;
	virtual void Destroy () = 0;
	std::string &GetWindowName () {return m_windowname;}

protected:
/*!
The Application instance owning the dialog.
*/
	Application *m_App;
	std::string m_windowname;

private:
	DialogOwner *m_Owner;
};

}	// namespace gcu

#endif // GCU_DIALOG_H
