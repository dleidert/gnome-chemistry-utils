// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/dialog.h
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

#ifndef GCU_GTK_DIALOG_H
#define GCU_GTK_DIALOG_H
#include "ui-builder.h"
#include <gcu/dialog.h>
#include <gtk/gtk.h>
#include <string>

/*!\file*/

namespace gcu {
class DialogOwner;
}

namespace gcugtk {

/** CheckType
* CheckType describes how numbers entered in entries might be checked.
* Possible values are:
* - NoCheck: no check is performed.
* - Min: value > min.
* - Max:  value < max.
* - MinMax: min < value < max.
* - MinEq:  value >= min.
* - MaxEq:  value <= max.
* - MinEqMax:  min <= value < max.
* - MinMaxEq:  min < value <= max.
* - MinEqMaxEq:  min <= value <= max.
*
*  This enumeration is used in Dialog::GetNumber.
*/
enum CheckType
{
	NoCheck,
	Min,
	Max,
	MinMax,
	MinEq,
	MaxEq,
	MinEqMax,
	MinMaxEq,
	MinEqMaxEq
};

class Application;

/*!\class Dialog gcugtk/dialog.h
This class is base class for dialog boxes. It provides some basic services.
*/
class Dialog: public UIBuilder, public gcu::Dialog
{
public:
friend class DialogPrivate;
/*!
@param App: the Application which owns the dialog.
@param filename: the name of the ui file which contains the description of
the dialog.
@param windowname: the name of the top level GtkWidget of the dialog box in
the glade file. This name should be unique for the application. It is used to access
the contextual help and to ensure the uniqueness of the dialog (in some cases).
@param domainname: the translation domain.
@param owner: the address of an owner object, might be App or a document
or NULL (the default). when owner is not NULL, the dialog will be unique for it.
@param extra_destroy: a callback to be called when the dialog is detroyed
by calling Dialog::Destroy. Useful to perform non standard cleaning operations
before calling gtk_widget_destroy. The destructor being called afterwards, it
cannot access the widget.
@param data: the data to be passed to extra_destroy.

If the ui file declares buttons with names "OK", "apply", "cancel" and "help",
default actions will be associated with these buttons.
If the Application does not provide help support, the Help button will be hidden.
*/
	Dialog (Application* App, char const *filename, const char* windowname, char const *domainname, gcu::DialogOwner *owner = NULL, void (*extra_destroy)(gpointer) = NULL, gpointer data = NULL) throw (std::runtime_error);
	virtual ~Dialog ();

/*!
	Called when closing the dialog box after a click on the OK or Cancel buttons.
	If a child class implements this method, it should call Dialog::Destroy after
	performing its task or it must destroy the window.
	The defaut implementation calls extra_destroy and gtk_widget_destroy.
*/
	virtual void Destroy ();

/*!
	Called after a click on the OK or Apply buttons. After clicking the OK button
	and if the method returns true, Destroy will be called to close the dialog box.
	A derived class should implement this method, as the default just returns true.

@return true if everything worked, false if something when wrong and the dialog
should not be closed.
*/
	virtual bool Apply ();
/*!
	Called when the dialog closes. Default does nothing.
*/
	virtual void Closed ();

/*!
/return the top level window of the dialog box.
*/
	GtkWindow* GetWindow () {return dialog;}

/*!
Brings the dialog to top of the windows stack.
*/
	void Present () {gtk_window_present (dialog);}

/*!
@param window the parent window.

Make the diaolg box transient for \a window.
*/
	void SetTransientFor (GtkWindow *window);

protected:
/*!
@param Entry: the GtkEntry from which the number should be retrieved.
@param x: a pointer to the value which will be replaced by the result.
@param c: the type of check to perform on the value.
@param min: the minimum accepted value, if needed.
@param max: the maximum accepted value, if needed.

This method retrieves the text displayed in Entry, converts it to a number
and perform bounds tests if needed. If an error occurs, a message box is
displayed which let the user know why the value is not correct.
@return true if the value is valid, false if something went wrong.
*/
	bool GetNumber (GtkEntry *Entry, double *x, CheckType c = NoCheck, double min = 0, double max = 0);

protected:
/*!
The associated GtkWindow instance.
*/
	GtkWindow *dialog;

private:
	void (*m_extra_destroy) (gpointer);
	gpointer m_data;
	char m_buf[64];
};

}	// namespace gcu

#endif // GCU_GTK_DIALOG_H
