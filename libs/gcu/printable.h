/* 
 * Gnome Chemistry Utils
 * printable.h
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_PRINTABLE_H
#define GCU_PRINTABLE_H

#include "dialog-owner.h"
#include "macros.h"
#include <gtk/gtkprintoperation.h>
#include <gtk/gtkprintcontext.h>
#include <gtk/gtkpagesetup.h>
#include <gtk/gtkprintsettings.h>

namespace gcu {

class Printable: public DialogOwner
{
public:
	Printable ();
	virtual ~Printable ();

	virtual void DoPrint (GtkPrintOperation *print, GtkPrintContext *context) = 0;
	virtual bool SupportsHeaders () {return false;}
	virtual GtkWindow *GetWindow () = 0;
	virtual int GetPagesNumber () {return 1;}

	void Print (bool preview);
	void SetPageSetup (GtkPageSetup *PageSetup);

GCU_RO_PROP (GtkPrintSettings *, PrintSettings)
GCU_RO_PROP (GtkPageSetup *, PageSetup)
GCU_PROP (GtkUnit, Unit);
};

}	//	namespace gcu

#endif	//	GCU_PRINTABLE_H
