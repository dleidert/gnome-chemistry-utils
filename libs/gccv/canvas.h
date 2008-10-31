// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * canvas/canvas.h 
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

#ifndef GCCV_CANVAS_H
#define GCCV_CANVAS_H

#include <gcu/macros.h>
#include <gtk/gtk.h>

namespace gccv {

class Group;
class Client;

class Canvas
{
public:
	Canvas (Client *client);
	virtual ~Canvas();

	GtkWidget *GetWidget () {return m_Widget;}
	void SetScrollRegion (double xmin, double ymin, double xmax, double ymax);
	void Invalidate (double x0, double y0, double x1, double y1);

	// Event related functions
	bool OnButtonPressed (GdkEventButton *event);
	bool OnButtonReleased (GdkEventButton *event);
	bool OnMotion (GdkEventMotion *event);
	bool OnExpose (GdkEventExpose *event);
	void UpdateBounds ();

private:
	GtkWidget *m_Widget;
	Client *m_Client;
	bool m_Dragging;

GCU_RO_PROP (Group *, Root)
GCU_PROP (double, Gap)
};

}

#endif	//	 GCCV_CANVAS_H
