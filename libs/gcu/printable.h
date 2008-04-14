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

typedef enum {
	GCU_PRINT_SCALE_NONE,
	GCU_PRINT_SCALE_FIXED,
	GCU_PRINT_SCALE_AUTO,
} PrintScaleType;

class Printable: virtual public DialogOwner
{
public:
	Printable ();
	virtual ~Printable ();

	virtual void DoPrint (GtkPrintOperation *print, GtkPrintContext *context) const = 0;
	virtual bool SupportsHeaders () {return false;}
	virtual bool SupportMultiplePages () {return false;}
	virtual GtkWindow *GetGtkWindow () = 0;
	virtual int GetPagesNumber () {return 1;}

	void Print (bool preview);
	void SetPageSetup (GtkPageSetup *PageSetup);

GCU_RO_PROP (GtkPrintSettings *, PrintSettings)
GCU_RO_PROP (GtkPageSetup *, PageSetup)
GCU_PROP (GtkUnit, Unit)
GCU_PROP (double, HeaderHeight)
GCU_PROP (double, FooterHeight)
GCU_PROP (bool, HorizCentered)
GCU_PROP (bool, VertCentered)
GCU_PROP (PrintScaleType, ScaleType)
GCU_PROP (double, Scale)
GCU_PROP (bool, HorizFit)
GCU_PROP (bool, VertFit)
GCU_PROP (int, HPages)
GCU_PROP (int, VPages)
};

GtkUnit gtk_unit_from_string (char const *name);
char const *gtk_unit_to_string (GtkUnit unit);

}	//	namespace gcu

#endif	//	GCU_PRINTABLE_H
