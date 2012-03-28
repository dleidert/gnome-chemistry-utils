// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/glview.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_GL_VIEW_H
#define GCU_GTK_GL_VIEW_H

#include "printable.h"
#include <gcu/glview.h>
#include <gtk/gtk.h>
#include <GL/glx.h>

namespace gcugtk {

/*!
\class GLView gcu/glview.h
View class based on OpenGL for rendering. Used to display 3d chemical structures
such as molecules or crystals cells.
*/
class GLView: public gcu::GLView, public gcugtk::Printable
{
friend class GLViewPrivate;
public:
//!Constructor.
/*!
@param pDoc: a pointer to the GLDocument instance.

Creates a view for the document.
*/
	GLView (gcu::GLDocument* pDoc) throw (std::runtime_error);
//!Destructor.
/*!
The destructor of GLView.
*/
	virtual ~GLView ();

/*!
@return the associated GtkWidget.
*/
	GtkWidget *GetWidget () {return m_Widget;}

/*!
@return the top level GtkWindow containing the view.
*/
	GtkWindow *GetGtkWindow () {return GTK_WINDOW (gtk_widget_get_toplevel (m_Widget));}
/*!
Update the contents of the associated widget. This method must be called
each time the document or the view are modified.
*/
	void Update ();
/*!
@param print a GtkPrintOperation.
@param context a GtkPrintContext.
@param page the page to print.

Prints the current view at 300 dpi.
*/
	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context, int page) const;
/*!
@param width the width of the generated image.
@param height the height of the generated image.
@param use_bg whether to use the window background or a transparent background.

Generates a pixbuf from the current view.

@return the pixbuf containing the generated image
*/
	GdkPixbuf *BuildPixbuf (unsigned width, unsigned height, bool use_bg) const;

protected:
	virtual bool GLBegin ();
	virtual void GLEnd ();

private:
	void Reshape (int width, int height) ;

protected:
/*!
The associated widget.
*/
	GtkWidget *m_Widget;

private:
	bool m_bInit;
	GdkWindow *m_Window;
	GLXContext m_Context;
	XVisualInfo *m_VisualInfo;
};

}	//	namespace gcugtk

#endif	//	GCU_GTK_GL_VIEW_H
