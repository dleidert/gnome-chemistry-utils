/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
/*
  @NOTATION@
 */

/* Line/curve item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */


#ifndef GNOME_CANVAS_LINE_EXT_H
#define GNOME_CANVAS_LINE_EXT_H


#include <libgnomecanvas/gnome-canvas-line.h>


G_BEGIN_DECLS

/* The following comments are for gnome-canvas-line. The new possiblity here is to have half arrowheads.
 * the new variables are fisrt_arrow_head_style and last_arrow_head_style
 */
 
/* Line item for the canvas.  This is a polyline with configurable width, cap/join styles, and arrowheads.
 * If arrowheads are enabled, then three values are used to specify their shape:
 *
 *	arrow_shape_a:  Distance from tip of arrowhead to the center point.
 *	arrow_shape_b:  Distance from tip of arrowhead to trailing point, measured along the shaft.
 *	arrow_shape_c:	Distance of trailing point from outside edge of shaft.
 *
 * The following object arguments are available:
 *
 * name			type			read/write	description
 * ------------------------------------------------------------------------------------------
 * points		GnomeCanvasPoints*	RW		Pointer to a GnomeCanvasPoints structure.
 *								This can be created by a call to
 *								gnome_canvas_points_new() (in gnome-canvas-util.h).
 *								X coordinates are in the even indices of the
 *								points->coords array, Y coordinates are in
 *								the odd indices.
 * fill_color		string			W		X color specification for line
 * fill_color_gdk	GdkColor*		RW		Pointer to an allocated GdkColor
 * fill_stipple		GdkBitmap*		RW		Stipple pattern for the line
 * width_pixels		uint			R		Width of the line in pixels.  The line width
 *								will not be scaled when the canvas zoom factor changes.
 * width_units		double			R		Width of the line in canvas units.  The line width
 *								will be scaled when the canvas zoom factor changes.
 * cap_style		GdkCapStyle		RW		Cap ("endpoint") style for the line.
 * join_style		GdkJoinStyle		RW		Join ("vertex") style for the line.
 * line_style		GdkLineStyle		RW		Line dash style
 * first_arrowhead	boolean			RW		Specifies whether to draw an arrowhead on the
 *								first point of the line.
 * last_arrowhead	boolean			RW		Specifies whether to draw an arrowhead on the
 *								last point of the line.
 * smooth		boolean			RW		Specifies whether to smooth the line using
 *								parabolic splines.
 * spline_steps		uint			RW		Specifies the number of steps to use when rendering curves.
 * arrow_shape_a	double			RW		First arrow shape specifier.
 * arrow_shape_b	double			RW		Second arrow shape specifier.
 * arrow_shape_c	double			RW		Third arrow shape specifier.
 */


#define GNOME_TYPE_CANVAS_LINE_EXT				(gnome_canvas_line_ext_get_type ())
#define GNOME_CANVAS_LINE_EXT(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_TYPE_CANVAS_LINE_EXT, GnomeCanvasLineExt))
#define GNOME_CANVAS_LINE_EXT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_LINE_EXT, GnomeCanvasLineExtClass))
#define GNOME_IS_CANVAS_LINE_EXT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_TYPE_CANVAS_LINE_EXT))
#define GNOME_IS_CANVAS_LINE_EXT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_LINE_EXT))
#define GNOME_CANVAS_LINE_EXT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GNOME_TYPE_CANVAS_LINE_EXT, GnomeCanvasLineExtClass))

typedef enum {
	ARROW_HEAD_NONE,
	ARROW_HEAD_LEFT,
	ARROW_HEAD_RIGHT,
	ARROW_HEAD_BOTH
} ArrowHeadStyle;

typedef struct _GnomeCanvasLineExt GnomeCanvasLineExt;
typedef struct _GnomeCanvasLineExtClass GnomeCanvasLineExtClass;

struct _GnomeCanvasLineExt {
	GnomeCanvasLine line;

	ArrowHeadStyle first_arrow_head_style;
	ArrowHeadStyle last_arrow_head_style;
	double saved_coords[4];
};

struct _GnomeCanvasLineExtClass {
	GnomeCanvasLineClass parent_class;
};


/* Standard Gtk function */
GType gnome_canvas_line_ext_get_type (void) G_GNUC_CONST;


G_END_DECLS
#endif
