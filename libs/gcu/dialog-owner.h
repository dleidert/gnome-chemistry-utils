// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/dialog-owner.h
 *
 * Copyright (C) 2005-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_DIALOG_OWNER_H
#define GCU_DIALOG_OWNER_H

#include <string>
#include <map>

namespace gcu {

class Dialog;

/*!\class DialogOwner gcu/dialog-owner.h
This class is the base class for objects owning dialogs. It is aimed at ensuring
that each dialog is unique and that when the owner is destroyed, its dialogs
are closed.
*/

class DialogOwner
{
friend class Dialog;
public:
/*!
The constructor.
*/
	DialogOwner ();
/*!
The destructor.
*/
	virtual ~DialogOwner ();

/*!
@param name the name associated to the Dialog.

@return the Dialog instance associated with name or NULL if there is none.
*/
	Dialog *GetDialog (std::string name);

/*!
	Destroys all dialogs associated with this instance.
*/
	void ClearDialogs ();

private:
	bool AddDialog (std::string name, Dialog *dialog) ;
	void RemoveDialog (std::string name)  {Dialogs.erase (name);}

private:
	std::map <std::string, Dialog*> Dialogs;
};

}	// namespace gcu

#endif	// GCU_DIALOG_OWNER_H
