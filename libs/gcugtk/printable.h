/* 
 * Gnome Chemistry Utils
 * printable.h
 *
 * Copyright (C) 2008-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <gcu/dialog-owner.h>
#include <gcu/macros.h>
#include <gtk/gtkprintoperation.h>
#include <gtk/gtkprintcontext.h>
#include <gtk/gtkpagesetup.h>
#include <gtk/gtkprintsettings.h>

/*!\file*/

namespace gcugtk {

/*!\enum PrintScaleType gcu/printable.h
Represents how printing is scaled
*/
typedef enum {
/*!
No scaling is done.
*/
	GCU_PRINT_SCALE_NONE,
/*!
Explicit scaling.
*/
	GCU_PRINT_SCALE_FIXED,
/*!
Automatic scaling (will fill the print area).
*/
	GCU_PRINT_SCALE_AUTO,
} PrintScaleType;

/*!\class Printable gcu/printable.h
A base class for everything that might be printed.
*/
class Printable: virtual public gcu::DialogOwner
{
public:
/*!
The constructor.
*/
	Printable ();
/*!
The destructor.
*/
	virtual ~Printable ();

/*!
@param print a GtkPrintOperation*.
@param context a GtkPrintContext*.
@param page the page to print.

This is the method doing the real printing. It mustbe overloaded by derived classes
and take parameters defined in gcu::Printable into account. Default implementation
does not do anything.
*/
	virtual void DoPrint (GtkPrintOperation *print, GtkPrintContext *context, int page) const = 0;
/*!
Derived classes supporting headers and footers must overload this method.
@return true if the document headers and footers. Default implementation
returns false.
*/
	virtual bool SupportsHeaders () {return false;}
/*!
Derived classes able to print several pages must overload this method.
@return true if the document supports pagination. Default implementation
returns false.
*/
	virtual bool SupportMultiplePages () {return false;}
/*!
Derived classes need to overload this pure virtual method.
@return the top level GtkWindow containing the document.
*/
	virtual GtkWindow *GetGtkWindow () = 0;
/*!
Derived classes able to print several pages must overload this method.
@return the page number for the document. Default implementation returns 1.
*/
	virtual int GetPagesNumber () {return 1;}

/*!
@param preview whether preview or real printing is requested. 

Prints the documennt. It initializes printing, and then calls DoPrint.
*/
	void Print (bool preview);
/*!
@param PageSetup a GtkPageSetup*.

This methods sets the page setup for the current printable object. Note that
this does not increase the reference count of the GtkPageSetup*. The page setup
will be unrefed when the Printable is destroyed or when a new page setup is set.
*/
	void SetPageSetup (GtkPageSetup *PageSetup);

/*!\fn GetPrintSettings()
@return the GtkPrintSettings currently associated with the Printable instance.
*/
GCU_RO_PROP (GtkPrintSettings *, PrintSettings)
/*!\fn GetPrintSettings()
@return the GtkPageSetup currently associated with the Printable instance.
*/
GCU_RO_PROP (GtkPageSetup *, PageSetup)
/*!\fn SetUnit(GtkUnit Unit)
@param Unit a GtkUnit.

Sets the current unit used in settings.
*/
/*!\fn GetUnit()
@return the current GtkUnit.
*/
/*!\fn GetRefUnit()
@return the current GtkUnit as a reference.
*/
GCU_PROP (GtkUnit, Unit)
/*!\fn SetHeaderHeight(double HeaderHeight)
@param HeaderHeight the height of the header.

Sets the height of the header to be printed on top of each page.
*/
/*!\fn GetHeaderHeight()
@return the current page header height.
*/
/*!\fn GetRefHeaderHeight()
@return the current page header height as a reference.
*/
GCU_PROP (double, HeaderHeight)
/*!\fn SetFooterHeight(double FooterHeight)
@param FooterHeight the height of the header.

Sets the height of the footer to be printed at the bottom of each page.
*/
/*!\fn GetFooterHeight()
@return the current page footer height.
*/
/*!\fn GetRefFooterHeight()
@return the current page footer height as a reference.
*/
GCU_PROP (double, FooterHeight)
/*!\fn SetHorizCentered(bool HorizCentered)
@param HorizCentered whether to center horizontally or not.
*/
/*!\fn GetHorizCentered()
@return true if printing is horizontally centered.
*/
/*!\fn GetRefHorizCentered()
@return a reference to the boolean indicating if printing is horizontally
centered or not.
*/
GCU_PROP (bool, HorizCentered)
/*!\fn SetVertCentered(bool VertCentered)
@param VertCentered whether to center vertically or not.
*/
/*!\fn GetVertCentered()
@return true if printing is vertically centered.
*/
/*!\fn GetRefVertCentered()
@return a reference to the boolean indicating if printing is vertically
centered or not.
*/
GCU_PROP (bool, VertCentered)
/*!\fn SetScaleType(PrintScaleType ScaleType)
@param ScaleType how to scale when printing.

Sets the printing scale type.
*/
/*!\fn GetScaleType()
@return the current printing scale type.
*/
/*!\fn GetRefScaleType()
@return the current printing scale type as a reference.
*/
GCU_PROP (PrintScaleType, ScaleType)
/*!\fn SetScale(double Scale)
@param Scale the scale to use.

Sets the scale to use when the printing scale type is GCU_PRINT_SCALE_FIXED.
*/
/*!\fn GetScale()
@return the current scale.
*/
/*!\fn GetRefScale()
@return the current scale as a reference.
*/
GCU_PROP (double, Scale)
/*!\fn SetHorizFit(bool HorizFit)
@param HorizFit whether to fill the page horizontally.
*/
/*!\fn GetHorizFit()
@return true if printing fills the page horizontally.
*/
/*!\fn GetRefHorizFit()
@return a reference to the boolean indicating if printing fills the
page horizontally.
*/
GCU_PROP (bool, HorizFit)
/*!\fn SetVertFit(bool VertFit)
@param VertFit whether to fill the page vertically.
*/
/*!\fn GetVertFit()
vertically*/
/*!\fn GetRefVertFit()
@return a reference to the boolean indicating if printing fills the
page vertically.
*/
GCU_PROP (bool, VertFit)
/*!\fn SetHPages(int HPages)
@param HPages the number of pages to which horizontally spread the print output.

Sets the full width in pages of the printing, when printing to several pages.
*/
/*!\fn GetHPages()
@return the current number of pages to which printing is horizontally spread.
*/
/*!\fn GetRefHPages()
@return the current GtkUnnumber of pages to which printing is horizontally
spread as a reference.
*/
GCU_PROP (int, HPages)
/*!\fn SetVPages(int VPages)
@param VPages the number of pages to which vertically spread the print output.

Sets the full height in pages of the printing, when printing to several pages.
*/
/*!\fn GetVPages()
@return the current number of pages to which printing is vertically spread.
*/
/*!\fn GetRefVPages()
@return the current GtkUnnumber of pages to which printing is vertically
spread as a reference.
*/
GCU_PROP (int, VPages)
};

/*!
@param name a string representing a GtkUnit.
@return the GtkUnit represented by the string.
*/
GtkUnit gtk_unit_from_string (char const *name);
/*!
@param unit a  GtkUnit.
@return the sring representig the GtkUnit.
*/
char const *gtk_unit_to_string (GtkUnit unit);

}	//	namespace gcu

#endif	//	GCU_PRINTABLE_H
