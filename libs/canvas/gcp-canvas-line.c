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
 * Boston, MA 02111-1307, USA.
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

#include "config.h"
#include <math.h>
#include <string.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>
#include <libgnomecanvas/libgnomecanvas.h>
#include "gcp-canvas-line.h"
#include "gprintable.h"

#define noVERBOSE

#define DEFAULT_SPLINE_STEPS 12		/* this is what Tk uses */
#define NUM_ARROW_POINTS     6		/* number of points in an arrowhead */
#define NUM_HALF_ARROW_POINTS     5		/* number of points in a half arrowhead */
#define NUM_STATIC_POINTS    256	/* number of static points to use to avoid allocating arrays */


#define GROW_BOUNDS(bx1, by1, bx2, by2, x, y) \
{	\
	if (x < bx1)				\
		bx1 = x;			\
						\
	if (x > bx2)				\
		bx2 = x;			\
						\
	if (y < by1)				\
		by1 = y;			\
						\
	if (y > by2)				\
		by2 = y;			\
}


enum {
	PROP_0,
	PROP_FIRST_ARROWHEAD_STYLE,
	PROP_LAST_ARROWHEAD_STYLE,
};


static void gnome_canvas_line_ext_class_init   (GnomeCanvasLineExtClass *class);
static void gnome_canvas_line_ext_init         (GnomeCanvasLineExt     *line);
static void gnome_canvas_line_ext_set_property (GObject              *object,
					    guint                 param_id,
					    const GValue         *value,
					    GParamSpec           *pspec);
static void gnome_canvas_line_ext_get_property (GObject              *object,
					    guint                 param_id,
					    GValue               *value,
					    GParamSpec           *pspec);

static void   gnome_canvas_line_ext_update      (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
//static void   gnome_canvas_line_ext_realize     (GnomeCanvasItem *item);
//static void   gnome_canvas_line_ext_unrealize   (GnomeCanvasItem *item);
static void   gnome_canvas_line_ext_draw        (GnomeCanvasItem *item, GdkDrawable *drawable,
					     int x, int y, int width, int height);
static double gnome_canvas_line_ext_point       (GnomeCanvasItem *item, double x, double y,
					     int cx, int cy, GnomeCanvasItem **actual_item);
static void   gnome_canvas_line_ext_bounds      (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2);
static void   gnome_canvas_line_ext_render      (GnomeCanvasItem *item, GnomeCanvasBuf *buf);
static void   gnome_canvas_line_ext_print       (GPrintable *gprintable, GnomePrintContext *pc);
static void   gnome_canvas_line_ext_export_svg (GPrintable *printable, xmlDocPtr doc, xmlNodePtr node);
static void   gnome_canvas_line_ext_draw_cairo (GPrintable *gprintable, cairo_t *cr);

static GnomeCanvasItemClass *parent_class;

static void
gnome_canvas_line_print_init (GPrintableIface *iface)
{
	iface->print = gnome_canvas_line_ext_print;
	iface->export_svg = gnome_canvas_line_ext_export_svg;
	iface->draw_cairo = gnome_canvas_line_ext_draw_cairo;
}

GType
gnome_canvas_line_ext_get_type (void)
{
	static GType line_ext_type;

	if (!line_ext_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasLineExtClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_line_ext_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasLineExt),
			0,			/* n_preallocs */
			(GInstanceInitFunc) gnome_canvas_line_ext_init,
			NULL			/* value_table */
		};

		static const GInterfaceInfo print_info = {
			(GInterfaceInitFunc) gnome_canvas_line_print_init,
			NULL, NULL
		};

		line_ext_type = g_type_register_static (GNOME_TYPE_CANVAS_LINE, "GnomeCanvasLineExt",
						    &object_info, 0);

		g_type_add_interface_static (line_ext_type, G_TYPE_PRINTABLE, &print_info);
	}

	return line_ext_type;
}

static void
gnome_canvas_line_ext_class_init (GnomeCanvasLineExtClass *class)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	gobject_class = (GObjectClass *) class;
	object_class = (GtkObjectClass *) class;
	item_class = (GnomeCanvasItemClass *) class;

	parent_class = g_type_class_peek_parent (class);

	gobject_class->set_property = gnome_canvas_line_ext_set_property;
	gobject_class->get_property = gnome_canvas_line_ext_get_property;

        g_object_class_install_property
                (gobject_class,
                 PROP_FIRST_ARROWHEAD_STYLE,
                 g_param_spec_uchar ("first_arrowhead_style", NULL, NULL,
				       0, 3, 0,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_LAST_ARROWHEAD_STYLE,
                 g_param_spec_uchar ("last_arrowhead_style", NULL, NULL,
				       0, 3, 0,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	item_class->update = gnome_canvas_line_ext_update;
	item_class->draw = gnome_canvas_line_ext_draw;
	item_class->point = gnome_canvas_line_ext_point;
	item_class->bounds = gnome_canvas_line_ext_bounds;

	item_class->render = gnome_canvas_line_ext_render;
}

static void
gnome_canvas_line_ext_init (GnomeCanvasLineExt *line_ext)
{
	GnomeCanvasLine *line = (GnomeCanvasLine*) line_ext;
	line->width = 0.0;
	line->cap = GDK_CAP_BUTT;
	line->join = GDK_JOIN_MITER;
	line->line_style = GDK_LINE_SOLID;
	line->shape_a = 0.0;
	line->shape_b = 0.0;
	line->shape_c = 0.0;
	line->spline_steps = DEFAULT_SPLINE_STEPS;
	line_ext->first_arrow_head_style = ARROW_HEAD_BOTH;
	line_ext->last_arrow_head_style = ARROW_HEAD_BOTH;
}

/* Computes the bounding box of the line, including its arrow points.  Assumes that the number of
 * points in the line is not zero.
 */
static void
get_bounds (GnomeCanvasLine *line, double *bx1, double *by1, double *bx2, double *by2)
{
	double *coords;
	double x1, y1, x2, y2;
	double width;
	int i;

	if (!line->coords) {
	    *bx1 = *by1 = *bx2 = *by2 = 0.0;
	    return;
	}
	
	/* Find bounding box of line's points */

	x1 = x2 = line->coords[0];
	y1 = y2 = line->coords[1];

	for (i = 1, coords = line->coords + 2; i < line->num_points; i++, coords += 2)
		GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	/* Add possible over-estimate for wide lines */

	if (line->width_pixels)
		width = line->width / line->item.canvas->pixels_per_unit;
	else
		width = line->width;

	x1 -= width;
	y1 -= width;
	x2 += width;
	y2 += width;

	/* For mitered lines, make a second pass through all the points.  Compute the location of
	 * the two miter vertex points and add them to the bounding box.
	 */

	if (line->join == GDK_JOIN_MITER)
		for (i = line->num_points, coords = line->coords; i >= 3; i--, coords += 2) {
			double mx1, my1, mx2, my2;

			if (gnome_canvas_get_miter_points (coords[0], coords[1],
							   coords[2], coords[3],
							   coords[4], coords[5],
							   width,
							   &mx1, &my1, &mx2, &my2)) {
				GROW_BOUNDS (x1, y1, x2, y2, mx1, my1);
				GROW_BOUNDS (x1, y1, x2, y2, mx2, my2);
			}
		}

	/* Add the arrow points, if any */

	if (line->first_arrow && line->first_coords)
		for (i = 0, coords = line->first_coords; i < NUM_ARROW_POINTS; i++, coords += 2)
			GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	if (line->last_arrow && line->last_coords)
		for (i = 0, coords = line->last_coords; i < NUM_ARROW_POINTS; i++, coords += 2)
			GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	/* Done */

	*bx1 = x1;
	*by1 = y1;
	*bx2 = x2;
	*by2 = y2;
}

/* Computes the bounding box of the line, in canvas coordinates.  Assumes that the number of points in the polygon is
 * not zero. Affine is the i2c transformation.
 */
static void
get_bounds_canvas (GnomeCanvasLine *line, double *bx1, double *by1, double *bx2, double *by2, double affine[6])
{
	GnomeCanvasItem *item;

	/* It would be possible to tighten the bounds somewhat by transforming the individual points before
	   aggregating them into the bbox. But it hardly seems worth it. */
	ArtDRect bbox_world;
	ArtDRect bbox_canvas;

	item = GNOME_CANVAS_ITEM (line);

	get_bounds (line, &bbox_world.x0, &bbox_world.y0, &bbox_world.x1, &bbox_world.y1);

	art_drect_affine_transform (&bbox_canvas, &bbox_world, affine);
	/* include 1 pixel of fudge */
	*bx1 = bbox_canvas.x0 - 1;
	*by1 = bbox_canvas.y0 - 1;
	*bx2 = bbox_canvas.x1 + 1;
	*by2 = bbox_canvas.y1 + 1;
}

/* Recalculates the arrow polygons for the line */
static void
reconfigure_arrows (GnomeCanvasLineExt *lineext)
{
	double *poly, *coords;
	double dx, dy, length;
	double sin_theta, cos_theta, tmp;
	double frac_height;	/* Line width as fraction of arrowhead width */
	double backup;		/* Distance to backup end points so the line ends in the middle of the arrowhead */
	double shape_a, shape_b, shape_c;
	double width;
	int i;
	GnomeCanvasLine* line = GNOME_CANVAS_LINE(lineext);
	if (line->num_points == 0)
		return;

	/* Set up things */

	if (lineext->first_arrow_head_style !=ARROW_HEAD_BOTH)
	{
		if (line->first_arrow && lineext->first_arrow_head_style) {
			if (line->first_coords) {
				line->coords[0] = lineext->saved_coords[0];
				line->coords[1] = lineext->saved_coords[1];
			} else
				line->first_coords = g_new (double, 2 * NUM_ARROW_POINTS);
		} else if (line->first_coords) {
			line->coords[0] = lineext->saved_coords[0];
			line->coords[1] = lineext->saved_coords[1];
	
			g_free (line->first_coords);
			line->first_coords = NULL;
		}
	}

	
	if  (lineext->last_arrow_head_style !=ARROW_HEAD_BOTH){ 
		i = 2 * (line->num_points - 1);
		if (line->last_arrow && lineext->last_arrow_head_style) {
			if (line->last_coords) {
				line->coords[i] = lineext->saved_coords[2];
				line->coords[i + 1] = lineext->saved_coords[3];
			} else
				line->last_coords = g_new (double, 2 * NUM_ARROW_POINTS);
		} else if (line->last_coords) {
			line->coords[i] = lineext->saved_coords[2];
			line->coords[i + 1] = lineext->saved_coords[3];
	
			g_free (line->last_coords);
			line->last_coords = NULL;
		}
	}

	if (!(line->first_arrow && lineext->first_arrow_head_style) && !(line->last_arrow && lineext->last_arrow_head_style))
		return;

	if (line->width_pixels)
		width = line->width / line->item.canvas->pixels_per_unit;
	else
		width = line->width;

	/* Add fudge value for better-looking results */

	shape_a = line->shape_a;
	shape_b = line->shape_b;
	shape_c = line->shape_c + width;

	if (line->width_pixels) {
		shape_a /= line->item.canvas->pixels_per_unit;
		shape_b /= line->item.canvas->pixels_per_unit;
		shape_c /= line->item.canvas->pixels_per_unit;
	}

	shape_a += 0.001;
	shape_b += 0.001;
	shape_c += 0.001;

	/* Compute the polygon for the first arrowhead and adjust the first point in the line so
	 * that the line does not stick out past the leading edge of the arrowhead.
	 */
	frac_height = line->width / shape_c;
	backup = frac_height * shape_b + shape_a * (1.0 - frac_height) / 2.0;

	if (line->first_arrow && (lineext->first_arrow_head_style != ARROW_HEAD_BOTH)) {
		poly = line->first_coords;

		/* save first coords in unsed first_coords */
		line->first_coords[10] = line->coords[0];
		line->first_coords[11] = line->coords[1];

		dx = line->coords[0] - line->coords[2];
		dy = line->coords[1] - line->coords[3];
		length = sqrt (dx * dx + dy * dy);
		if (length < GNOME_CANVAS_EPSILON)
			sin_theta = cos_theta = 0.0;
		else {
			sin_theta = dy / length;
			cos_theta = dx / length;
		}

		tmp = shape_c * sin_theta;
		
		switch(lineext->first_arrow_head_style)
		{
			case ARROW_HEAD_LEFT:
				poly[8] = poly[0] = line->coords[0] - line->width / 2.0 * sin_theta;
				poly[9] = poly[1] = line->coords[1] + line->width / 2.0 * cos_theta;
				poly[2] = poly[0] - shape_b * cos_theta + tmp;
				poly[6] = poly[0] - shape_a * cos_theta;
				poly[4] = poly[6] + line->width * sin_theta;
		
				tmp = shape_c * cos_theta;
		
				poly[3] = poly[1] - shape_b * sin_theta - tmp;
				poly[7] = poly[1] - shape_a * sin_theta;
				poly[5] = poly[7] - line->width * cos_theta;
				break;
			case ARROW_HEAD_RIGHT:
				poly[8] = poly[0] = line->coords[0] + line->width / 2.0 * sin_theta;
				poly[9] = poly[1] = line->coords[1] - line->width / 2.0 * cos_theta;
				poly[2] = poly[0] - shape_b * cos_theta - tmp;
				poly[6] = poly[0] - shape_a * cos_theta;
				poly[4] = poly[6] - line->width * sin_theta;
		
				tmp = shape_c * cos_theta;
		
				poly[3] = poly[1] - shape_b * sin_theta + tmp;
				poly[7] = poly[1] - shape_a * sin_theta;
				poly[5] = poly[7] + line->width * cos_theta;
				break;
			default:
				break;
		}

		/* Move the first point towards the second so that the corners at the end of the
		 * line are inside the arrowhead.
		 */

		line->coords[0] -= backup * cos_theta;
		line->coords[1] -= backup * sin_theta;
	}

	/* Same process for last arrowhead */

	if (line->last_arrow && (lineext->last_arrow_head_style != ARROW_HEAD_BOTH)) {
		coords = line->coords + 2 * (line->num_points - 2);
		poly = line->last_coords;

		/* save first coords in unsed first_coords */
		line->last_coords[10] = coords[2];
		line->last_coords[11] = coords[3];

		dx = line->coords[2] - coords[0];
		dy = line->coords[3] - coords[1];
		length = sqrt (dx * dx + dy * dy);
		if (length < GNOME_CANVAS_EPSILON)
			sin_theta = cos_theta = 0.0;
		else {
			sin_theta = dy / length;
			cos_theta = dx / length;
		}

		tmp = shape_c * sin_theta;
		
		switch(lineext->last_arrow_head_style)
		{
			case ARROW_HEAD_LEFT:
				poly[8] = poly[0] = coords[2] - line->width / 2.0 * sin_theta;
				poly[9] = poly[1] = coords[3] + line->width / 2.0 * cos_theta;
				poly[2] = poly[0] - shape_b * cos_theta + tmp;
				poly[6] = poly[0] - shape_a * cos_theta;
				poly[4] = poly[6] + line->width * sin_theta;
		
				tmp = shape_c * cos_theta;
		
				poly[3] = poly[1] - shape_b * sin_theta - tmp;
				poly[7] = poly[1] - shape_a * sin_theta;
				poly[5] = poly[7] - line->width * cos_theta;
				break;
			case ARROW_HEAD_RIGHT:
				poly[8] = poly[0] = coords[2] + line->width / 2.0 * sin_theta;
				poly[9] = poly[1] = coords[3] - line->width / 2.0 * cos_theta;
				poly[2] = poly[0] - shape_b * cos_theta - tmp;
				poly[6] = poly[0] - shape_a * cos_theta;
				poly[4] = poly[6] - line->width * sin_theta;
		
				tmp = shape_c * cos_theta;
		
				poly[3] = poly[1] - shape_b * sin_theta + tmp;
				poly[7] = poly[1] - shape_a * sin_theta;
				poly[5] = poly[7] + line->width * cos_theta;
				break;
			default:
				break;
		}

		coords[2] -= backup * cos_theta;
		coords[3] -= backup * sin_theta;
	}
}

/* Convenience function to set the line's GC's foreground color */
static void
set_line_gc_foreground (GnomeCanvasLine *line)
{
	GdkColor c;

	if (!line->gc)
		return;

	c.pixel = line->fill_pixel;
	gdk_gc_set_foreground (line->gc, &c);
}

/* Recalculate the line's width and set it in its GC */
static void
set_line_gc_width (GnomeCanvasLine *line)
{
	int width;

	if (!line->gc)
		return;

	if (line->width_pixels)
		width = (int) line->width;
	else
		width = (int) (line->width * line->item.canvas->pixels_per_unit + 0.5);

	gdk_gc_set_line_attributes (line->gc,
				    width,
				    line->line_style,
				    (line->first_arrow || line->last_arrow) ? GDK_CAP_BUTT : line->cap,
				    line->join);
}

/* Sets the stipple pattern for the line */
static void
set_stipple (GnomeCanvasLine *line, GdkBitmap *stipple, int reconfigure)
{
	if (line->stipple && !reconfigure)
		g_object_unref (line->stipple);

	line->stipple = stipple;
	if (stipple && !reconfigure)
		g_object_ref (stipple);

	if (line->gc) {
		if (stipple) {
			gdk_gc_set_stipple (line->gc, stipple);
			gdk_gc_set_fill (line->gc, GDK_STIPPLED);
		} else
			gdk_gc_set_fill (line->gc, GDK_SOLID);
	}
}

static void
gnome_canvas_line_ext_set_property (GObject              *object,
				guint                 param_id,
				const GValue         *value,
				GParamSpec           *pspec)
{
	GnomeCanvasItem *item;
	GnomeCanvasLineExt *line;
	gboolean color_changed;
	int have_pixel;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_LINE_EXT (object));

	item = GNOME_CANVAS_ITEM (object);
	line = GNOME_CANVAS_LINE_EXT (object);

	color_changed = FALSE;
	have_pixel = FALSE;

	switch (param_id) {
	case PROP_FIRST_ARROWHEAD_STYLE:
		line->first_arrow_head_style = g_value_get_uchar (value);
		gnome_canvas_item_request_update (item);
		break;

	case PROP_LAST_ARROWHEAD_STYLE:
		line->last_arrow_head_style = g_value_get_uchar (value);
		gnome_canvas_item_request_update (item);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gnome_canvas_line_ext_get_property (GObject              *object,
				guint                 param_id,
				GValue               *value,
				GParamSpec           *pspec)
{
	GnomeCanvasLineExt *line;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_LINE (object));

	line = GNOME_CANVAS_LINE_EXT (object);

	switch (param_id) {
	case PROP_FIRST_ARROWHEAD_STYLE:
		g_value_set_uchar (value, (guchar)line->first_arrow_head_style);
		break;

	case PROP_LAST_ARROWHEAD_STYLE:
		g_value_set_boolean (value, (guchar)line->last_arrow_head_style);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gnome_canvas_line_ext_render (GnomeCanvasItem *item,
			     GnomeCanvasBuf *buf)
{
	GnomeCanvasLine *line;

	line = GNOME_CANVAS_LINE (item);

	if (line->fill_svp != NULL)
		gnome_canvas_render_svp (buf, line->fill_svp, line->fill_rgba);

	if (line->first_svp != NULL)
		gnome_canvas_render_svp (buf, line->first_svp, line->fill_rgba);

	if (line->last_svp != NULL)
		gnome_canvas_render_svp (buf, line->last_svp, line->fill_rgba);
}


static ArtSVP *
svp_from_points (const double *item_coords, int num_points, const double affine[6])
{
	ArtVpath *vpath;
	ArtSVP *svp;
	double x, y;
	int i;

	vpath = art_new (ArtVpath, num_points + 2);

	for (i = 0; i < num_points; i++) {
		vpath[i].code = i == 0 ? ART_MOVETO : ART_LINETO;
		x = item_coords[i * 2];
		y = item_coords[i * 2 + 1];
		vpath[i].x = x * affine[0] + y * affine[2] + affine[4];
		vpath[i].y = x * affine[1] + y * affine[3] + affine[5];
	}
#if 0
	vpath[i].code = ART_LINETO;
	vpath[i].x = vpath[0].x;
	vpath[i].y = vpath[0].y;
	i++;
#endif
	vpath[i].code = ART_END;
	vpath[i].x = 0;
	vpath[i].y = 0;

	svp = art_svp_from_vpath (vpath);

	art_free (vpath);

	return svp;
}

static void
gnome_canvas_line_ext_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GnomeCanvasLineExt *lineext;
	GnomeCanvasLine *line;
	int i;
	ArtVpath *vpath;
	ArtPoint pi, pc;
	double width;
	ArtSVP *svp;
	double x1, y1, x2, y2;

	lineext = GNOME_CANVAS_LINE_EXT (item);
	line = GNOME_CANVAS_LINE (item);

	if (line->first_coords) {
		lineext->saved_coords[0] = line->first_coords[10];
		lineext->saved_coords[1] = line->first_coords[11];
	} else {
		lineext->saved_coords[0] = line->coords[0];
		lineext->saved_coords[1] = line->coords[1];
	}
	if (line->last_coords) {
		lineext->saved_coords[2] = line->last_coords[10];
		lineext->saved_coords[3] = line->last_coords[11];
	} else {
		i = (line->num_points - 1) * 2;
		lineext->saved_coords[2] = line->coords[i];
		lineext->saved_coords[3] = line->coords[i + 1];
	}

	if (parent_class->update)
		(* parent_class->update) (item, affine, clip_path, flags);

	reconfigure_arrows (lineext);

	if (item->canvas->aa) {
		gnome_canvas_item_reset_bounds (item);

		vpath = art_new (ArtVpath, line->num_points + 2);

		for (i = 0; i < line->num_points; i++) {
			pi.x = line->coords[i * 2];
			pi.y = line->coords[i * 2 + 1];
			art_affine_point (&pc, &pi, affine);
			vpath[i].code = i == 0 ? ART_MOVETO : ART_LINETO;
			vpath[i].x = pc.x;
			vpath[i].y = pc.y;
		}
		vpath[i].code = ART_END;
		vpath[i].x = 0;
		vpath[i].y = 0;

		if (line->width_pixels)
			width = line->width;
		else
			width = line->width * art_affine_expansion (affine);

		if (width < 0.5)
			width = 0.5;

		svp = art_svp_vpath_stroke (vpath,
					    gnome_canvas_join_gdk_to_art (line->join),
					    gnome_canvas_cap_gdk_to_art (line->cap),
					    width,
					    4,
					    0.25);
		art_free (vpath);

		gnome_canvas_item_update_svp_clip (item, &line->fill_svp, svp, clip_path);

		if (line->first_arrow)
			svp = svp_from_points (line->first_coords,
							(lineext->first_arrow_head_style == ARROW_HEAD_BOTH)? NUM_ARROW_POINTS: NUM_HALF_ARROW_POINTS,
							affine);
		else
			svp = NULL;

		gnome_canvas_item_update_svp_clip (item, &line->first_svp, svp, clip_path);

		if (line->last_arrow)
			svp = svp_from_points (line->last_coords,
							(lineext->last_arrow_head_style == ARROW_HEAD_BOTH)? NUM_ARROW_POINTS: NUM_HALF_ARROW_POINTS,
							affine);
		else
			svp = NULL;

		gnome_canvas_item_update_svp_clip (item, &line->last_svp, svp, clip_path);

	} else {
		set_line_gc_foreground (line);
		set_line_gc_width (line);
		set_stipple (line, line->stipple, TRUE);

		get_bounds_canvas (line, &x1, &y1, &x2, &y2, affine);
		gnome_canvas_update_bbox (item, x1, y1, x2, y2);
	}
}

static void
item_to_canvas (GnomeCanvas *canvas, double *item_coords, GdkPoint *canvas_coords, int num_points,
		int *num_drawn_points, double i2c[6], int x, int y)
{
	int i;
	int old_cx, old_cy;
	int cx, cy;
	ArtPoint pi, pc;

#ifdef VERBOSE
	{
		char str[128];
		art_affine_to_string (str, i2c);
		g_print ("line item_to_canvas %s\n", str);
	}
#endif

	/* the first point is always drawn */

	pi.x = item_coords[0];
	pi.y = item_coords[1];
	art_affine_point (&pc, &pi, i2c);
	cx = floor (pc.x + 0.5);
	cy = floor (pc.y + 0.5);
	canvas_coords->x = cx - x;
	canvas_coords->y = cy - y;
	canvas_coords++;
	old_cx = cx;
	old_cy = cy;
	*num_drawn_points = 1;

	for (i = 1; i < num_points; i++) {
		pi.x = item_coords[i * 2];
		pi.y = item_coords[i * 2 + 1];
		art_affine_point (&pc, &pi, i2c);
		cx = floor (pc.x + 0.5);
		cy = floor (pc.y + 0.5);
		if (old_cx != cx || old_cy != cy) {
			canvas_coords->x = cx - x;
			canvas_coords->y = cy - y;
			old_cx = cx;
			old_cy = cy;
			canvas_coords++;
			(*num_drawn_points)++;
		}
	}
}

static void
gnome_canvas_line_ext_draw (GnomeCanvasItem *item, GdkDrawable *drawable,
			int x, int y, int width, int height)
{
	GnomeCanvasLineExt *lineext;
	GnomeCanvasLine *line;
	GdkPoint static_points[NUM_STATIC_POINTS];
	GdkPoint *points;
	int actual_num_points_drawn;
	double i2c[6];

	lineext = GNOME_CANVAS_LINE_EXT (item);
	line = GNOME_CANVAS_LINE (item);

	if (line->num_points == 0)
		return;

	/* Build array of canvas pixel coordinates */

	if (line->num_points <= NUM_STATIC_POINTS)
		points = static_points;
	else
		points = g_new (GdkPoint, line->num_points);


	gnome_canvas_item_i2c_affine (item, i2c);

	item_to_canvas (item->canvas, line->coords, points, line->num_points,
			&actual_num_points_drawn, i2c, x, y);

	if (line->stipple)
		gnome_canvas_set_stipple_origin (item->canvas, line->gc);

	gdk_draw_lines (drawable, line->gc, points, actual_num_points_drawn);

	if (points != static_points)
		g_free (points);

	/* Draw arrowheads */

	points = static_points;

	if (line->first_arrow) {
		item_to_canvas (item->canvas, line->first_coords, points,
				((lineext->first_arrow_head_style == ARROW_HEAD_BOTH)? NUM_ARROW_POINTS: NUM_HALF_ARROW_POINTS),
				&actual_num_points_drawn, i2c, x, y);
		gdk_draw_polygon (drawable, line->gc, TRUE, points, actual_num_points_drawn );
	}

	if (line->last_arrow) {
		item_to_canvas (item->canvas, line->last_coords, points,
				((lineext->last_arrow_head_style == ARROW_HEAD_BOTH)? NUM_ARROW_POINTS: NUM_HALF_ARROW_POINTS),
				&actual_num_points_drawn, i2c, x, y);
		gdk_draw_polygon (drawable, line->gc, TRUE, points, actual_num_points_drawn );
	}
}

static double
gnome_canvas_line_ext_point (GnomeCanvasItem *item, double x, double y,
			 int cx, int cy, GnomeCanvasItem **actual_item)
{
	GnomeCanvasLine *line;
	GnomeCanvasLineExt *lineext;
	double *line_points = NULL, *coords;
	double static_points[2 * NUM_STATIC_POINTS];
	double poly[10];
	double best, dist;
	double dx, dy;
	double width;
	int num_points = 0, i;
	int changed_miter_to_bevel;

#ifdef VERBOSE
	g_print ("gnome_canvas_line_ext_point x, y = (%g, %g); cx, cy = (%d, %d)\n", x, y, cx, cy);
#endif

	lineext = GNOME_CANVAS_LINE_EXT (item);
	line = GNOME_CANVAS_LINE (item);

	*actual_item = item;

	best = 1.0e36;

	/* Handle smoothed lines by generating an expanded set ot points */

	if (line->smooth && (line->num_points > 2)) {
		/* FIXME */
	} else {
		num_points = line->num_points;
		line_points = line->coords;
	}

	/* Compute a polygon for each edge of the line and test the point against it.  The effective
	 * width of the line is adjusted so that it will be at least one pixel thick (so that zero
	 * pixel-wide lines can be pickedup as well).
	 */

	if (line->width_pixels)
		width = line->width / item->canvas->pixels_per_unit;
	else
		width = line->width;

	if (width < (1.0 / item->canvas->pixels_per_unit))
		width = 1.0 / item->canvas->pixels_per_unit;

	changed_miter_to_bevel = 0;

	for (i = num_points, coords = line_points; i >= 2; i--, coords += 2) {
		/* If rounding is done around the first point, then compute distance between the
		 * point and the first point.
		 */

		if (((line->cap == GDK_CAP_ROUND) && (i == num_points))
		    || ((line->join == GDK_JOIN_ROUND) && (i != num_points))) {
			dx = coords[0] - x;
			dy = coords[1] - y;
			dist = sqrt (dx * dx + dy * dy) - width / 2.0;
			if (dist < GNOME_CANVAS_EPSILON) {
				best = 0.0;
				goto done;
			} else if (dist < best)
				best = dist;
		}

		/* Compute the polygonal shape corresponding to this edge, with two points for the
		 * first point of the edge and two points for the last point of the edge.
		 */

		if (i == num_points)
			gnome_canvas_get_butt_points (coords[2], coords[3], coords[0], coords[1],
						      width, (line->cap == GDK_CAP_PROJECTING),
						      poly, poly + 1, poly + 2, poly + 3);
		else if ((line->join == GDK_JOIN_MITER) && !changed_miter_to_bevel) {
			poly[0] = poly[6];
			poly[1] = poly[7];
			poly[2] = poly[4];
			poly[3] = poly[5];
		} else {
			gnome_canvas_get_butt_points (coords[2], coords[3], coords[0], coords[1],
						      width, FALSE,
						      poly, poly + 1, poly + 2, poly + 3);

			/* If this line uses beveled joints, then check the distance to a polygon
			 * comprising the last two points of the previous polygon and the first two
			 * from this polygon; this checks the wedges that fill the mitered point.
			 */

			if ((line->join == GDK_JOIN_BEVEL) || changed_miter_to_bevel) {
				poly[8] = poly[0];
				poly[9] = poly[1];

				dist = gnome_canvas_polygon_to_point (poly, 5, x, y);
				if (dist < GNOME_CANVAS_EPSILON) {
					best = 0.0;
					goto done;
				} else if (dist < best)
					best = dist;

				changed_miter_to_bevel = FALSE;
			}
		}

		if (i == 2)
			gnome_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
						      width, (line->cap == GDK_CAP_PROJECTING),
						      poly + 4, poly + 5, poly + 6, poly + 7);
		else if (line->join == GDK_JOIN_MITER) {
			if (!gnome_canvas_get_miter_points (coords[0], coords[1],
							    coords[2], coords[3],
							    coords[4], coords[5],
							    width,
							    poly + 4, poly + 5, poly + 6, poly + 7)) {
				changed_miter_to_bevel = TRUE;
				gnome_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
							      width, FALSE,
							      poly + 4, poly + 5, poly + 6, poly + 7);
			}
		} else
			gnome_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
						      width, FALSE,
						      poly + 4, poly + 5, poly + 6, poly + 7);

		poly[8] = poly[0];
		poly[9] = poly[1];

		dist = gnome_canvas_polygon_to_point (poly, 5, x, y);
		if (dist < GNOME_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else if (dist < best)
			best = dist;
	}

	/* If caps are rounded, check the distance to the cap around the final end point of the line */

	if (line->cap == GDK_CAP_ROUND) {
		dx = coords[0] - x;
		dy = coords[1] - y;
		dist = sqrt (dx * dx + dy * dy) - width / 2.0;
		if (dist < GNOME_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

	/* sometimes the GnomeCanvasItem::update signal will not have
           been processed between deleting the arrow points and a call
           to this routine -- this can cause a segfault here */
	if ((line->first_arrow && !line->first_coords) ||
	    (line->last_arrow && !line->last_coords))
		reconfigure_arrows(lineext);

	/* If there are arrowheads, check the distance to them */

	if (line->first_arrow && line->first_coords) {
		dist = gnome_canvas_polygon_to_point (line->first_coords, ((lineext->first_arrow_head_style == ARROW_HEAD_BOTH)? NUM_ARROW_POINTS: NUM_HALF_ARROW_POINTS), x, y);
		if (dist < GNOME_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

	if (line->last_arrow && line->last_coords) {
		dist = gnome_canvas_polygon_to_point (line->last_coords, ((lineext->last_arrow_head_style == ARROW_HEAD_BOTH)? NUM_ARROW_POINTS: NUM_HALF_ARROW_POINTS), x, y);
		if (dist < GNOME_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

done:

	if ((line_points != static_points) && (line_points != line->coords))
		g_free (line_points);

	return best;
}

static void
gnome_canvas_line_ext_bounds (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GnomeCanvasLine *line;

	line = GNOME_CANVAS_LINE (item);

	if (line->num_points == 0) {
		*x1 = *y1 = *x2 = *y2 = 0.0;
		return;
	}

	get_bounds (line, x1, y1, x2, y2);
}

static void
gnome_canvas_line_ext_print (GPrintable *gprintable, GnomePrintContext *pc)
{
	gdouble width;
	gint i;
	GnomeCanvasLine *line;
	GnomeCanvasLineExt *lineext;
	gdouble dashes[2] = {3.0, 2.0};
	
	line = GNOME_CANVAS_LINE (gprintable);
	lineext = GNOME_CANVAS_LINE_EXT (gprintable);
	
	if (line->num_points == 0)
		return;

	gnome_print_setrgbcolor (pc, ((double)(line->fill_rgba >> 24)) / 255.0,
								    ((double)((line->fill_rgba >> 16) & 0xff)) / 255.0,
								    ((double)((line->fill_rgba >> 8) & 0xff)) / 255.0);
	gnome_print_setopacity(pc, ((double) (line->fill_rgba & 0xff)) / 255.0);

	if (line->width_pixels)
		width = (double) line->width / line->item.canvas->pixels_per_unit;
	else
		width = line->width;
	gnome_print_setlinewidth (pc, width);

	if (line->first_arrow || line->last_arrow) gnome_print_setlinecap (pc, ART_PATH_STROKE_CAP_BUTT);
	else switch (line->cap)
	{
		case GDK_CAP_ROUND: gnome_print_setlinecap (pc, ART_PATH_STROKE_CAP_ROUND); break;
		case GDK_CAP_PROJECTING: gnome_print_setlinecap (pc, ART_PATH_STROKE_CAP_SQUARE); break;
		default: gnome_print_setlinecap (pc, ART_PATH_STROKE_CAP_BUTT); break;
	}
	
	gnome_print_setlinejoin (pc, (gint) line->join);
	
	gnome_print_setdash(pc, (line->line_style == GDK_LINE_ON_OFF_DASH)? 2: 0, dashes, 0);
	
	gnome_print_moveto (pc, line->coords[0], line->coords[1]);
	for (i = 1; i < line->num_points; i++)
		gnome_print_lineto (pc, line->coords[2 * i], line->coords[2 * i + 1]);/*FIXME: Change that for spline lines!*/
		
	gnome_print_stroke (pc);
	gnome_print_setlinewidth (pc, 0.0);

	if (line->first_arrow && line->first_coords)
	{
		gnome_print_newpath (pc);
		gnome_print_moveto(pc, line->first_coords[0], line->first_coords[1]);
		gnome_print_lineto(pc, line->first_coords[2], line->first_coords[3]);
		gnome_print_lineto(pc, line->first_coords[4], line->first_coords[5]);
		gnome_print_lineto(pc, line->first_coords[6], line->first_coords[7]);
		gnome_print_lineto(pc, line->first_coords[8], line->first_coords[9]);
		if (lineext->first_arrow_head_style == ARROW_HEAD_BOTH)
			gnome_print_lineto(pc, line->first_coords[10], line->first_coords[11]);
		gnome_print_closepath (pc);
		gnome_print_fill (pc);
	}
	
	if (line->last_arrow && line->last_coords)
	{
		gnome_print_newpath (pc);
		gnome_print_moveto(pc, line->last_coords[0], line->last_coords[1]);
		gnome_print_lineto(pc, line->last_coords[2], line->last_coords[3]);
		gnome_print_lineto(pc, line->last_coords[4], line->last_coords[5]);
		gnome_print_lineto(pc, line->last_coords[6], line->last_coords[7]);
		gnome_print_lineto(pc, line->last_coords[8], line->last_coords[9]);
		if (lineext->last_arrow_head_style == ARROW_HEAD_BOTH)
			gnome_print_lineto(pc, line->last_coords[10], line->last_coords[11]);
		gnome_print_closepath (pc);
		gnome_print_fill (pc);
	}
}

static void
gnome_canvas_line_ext_export_svg (GPrintable *printable, xmlDocPtr doc, xmlNodePtr node)
{
	GnomeCanvasLine *line;
	GnomeCanvasLineExt *lineext;
	GString *string;
	xmlNodePtr child;
	char *buf;
	double width;
	int opacity, i;

	line = GNOME_CANVAS_LINE (printable);
	lineext = GNOME_CANVAS_LINE_EXT (printable);
	
	if (line->num_points == 0)
		return;

	child = xmlNewDocNode (doc, NULL, (const xmlChar*) "path", NULL);
	xmlAddChild (node, child);
	string = g_string_new ("");
	g_string_append_printf (string, "M%g %g", line->coords[0], line->coords[1]);
	for (i = 1; i < line->num_points; i++)
		g_string_append_printf (string, "L%g %g", line->coords[2 * i], line->coords[2 * i + 1]);/*FIXME: Change that for spline lines!*/
	xmlNewProp (child, (const xmlChar*)"d", (const xmlChar*)string->str);
	g_string_free (string, TRUE);
	xmlNewProp (child, (const xmlChar*)"fill", (const xmlChar*)"none");
	buf = g_strdup_printf ("#%06x", line->fill_rgba >> 8);
	xmlNewProp (child, (const xmlChar*)"stroke", (const xmlChar*)buf);
	g_free (buf);
	opacity = line->fill_rgba & 0xff;
	if (opacity != 255) {
		buf = g_strdup_printf ("%g", (double) opacity / 255.);
		xmlNewProp (child, (const xmlChar*)"stroke-opacity", (const xmlChar*)buf);
		g_free (buf);
	}
	if (line->width_pixels)
		width = (double) line->width / line->item.canvas->pixels_per_unit;
	else
		width = line->width;
	buf = g_strdup_printf ("%g", width);
	xmlNewProp (child, (const xmlChar*)"stroke-width", (const xmlChar*)buf);
	g_free (buf);

	switch (line->cap) {
	case GDK_CAP_ROUND:
		xmlNewProp (child, (const xmlChar*)"stroke-linecap", (const xmlChar*)"round");
		break;
	case GDK_CAP_PROJECTING:
		xmlNewProp (child, (const xmlChar*)"stroke-linecap", (const xmlChar*)"square");
		break;
	default:
		xmlNewProp (child, (const xmlChar*)"stroke-linecap", (const xmlChar*)"butt");
		break;
	}

	switch (line->join) {
	case GDK_JOIN_ROUND:
		xmlNewProp (child, (const xmlChar*)"stroke-join", (const xmlChar*)"round");
		break;
	case GDK_JOIN_BEVEL:
		xmlNewProp (child, (const xmlChar*)"stroke-linejoin", (const xmlChar*)"bevel");
		break;
	default:
		xmlNewProp (child, (const xmlChar*)"stroke-linejoin", (const xmlChar*)"miter");
		break;
	}

	if (line->line_style == GDK_LINE_ON_OFF_DASH)
		xmlNewProp (child, (const xmlChar*)"stroke-dasharray", (const xmlChar*)"3,2");

	if (line->first_arrow && line->first_coords) {
		child = xmlNewDocNode (doc, NULL, (const xmlChar*) "path", NULL);
		xmlAddChild (node, child);
		string = g_string_new ("");
		g_string_append_printf (string, "M%g %g", line->first_coords[0], line->first_coords[1]);
		g_string_append_printf (string, "L%g %g", line->first_coords[2], line->first_coords[3]);
		g_string_append_printf (string, "L%g %g", line->first_coords[4], line->first_coords[5]);
		g_string_append_printf (string, "L%g %g", line->first_coords[6], line->first_coords[7]);
		g_string_append_printf (string, "L%g %g", line->first_coords[8], line->first_coords[9]);
		if (lineext->first_arrow_head_style == ARROW_HEAD_BOTH)
			g_string_append_printf (string, "L%g %g", line->first_coords[10], line->first_coords[11]);
		xmlNewProp (child, (const xmlChar*)"d", (const xmlChar*)string->str);
		g_string_free (string, TRUE);
		xmlNewProp (child, (const xmlChar*)"stroke", (const xmlChar*)"none");
		buf = g_strdup_printf ("#%06x", line->fill_rgba >> 8);
		xmlNewProp (child, (const xmlChar*)"fill", (const xmlChar*)buf);
		g_free (buf);
		opacity = line->fill_rgba & 0xff;
		if (opacity != 255) {
			buf = g_strdup_printf ("%g", (double) opacity / 255.);
			xmlNewProp (child, (const xmlChar*)"fill-opacity", (const xmlChar*)buf);
			g_free (buf);
		}
	}

	if (line->last_arrow && line->last_coords) {
		child = xmlNewDocNode (doc, NULL, (const xmlChar*) "path", NULL);
		xmlAddChild (node, child);
		string = g_string_new ("");
		g_string_append_printf (string, "M%g %g", line->last_coords[0], line->last_coords[1]);
		g_string_append_printf (string, "L%g %g", line->last_coords[2], line->last_coords[3]);
		g_string_append_printf (string, "L%g %g", line->last_coords[4], line->last_coords[5]);
		g_string_append_printf (string, "L%g %g", line->last_coords[6], line->last_coords[7]);
		g_string_append_printf (string, "L%g %g", line->last_coords[8], line->last_coords[9]);
		if (lineext->last_arrow_head_style == ARROW_HEAD_BOTH)
			g_string_append_printf (string, "L%g %g", line->last_coords[10], line->last_coords[11]);
		xmlNewProp (child, (const xmlChar*)"d", (const xmlChar*)string->str);
		g_string_free (string, TRUE);
		xmlNewProp (child, (const xmlChar*)"stroke", (const xmlChar*)"none");
		buf = g_strdup_printf ("#%06x", line->fill_rgba >> 8);
		xmlNewProp (child, (const xmlChar*)"fill", (const xmlChar*)buf);
		g_free (buf);
		opacity = line->fill_rgba & 0xff;
		if (opacity != 255) {
			buf = g_strdup_printf ("%g", (double) opacity / 255.);
			xmlNewProp (child, (const xmlChar*)"fill-opacity", (const xmlChar*)buf);
			g_free (buf);
		}
	}
}

void
gnome_canvas_line_ext_draw_cairo (GPrintable *gprintable, cairo_t *cr)
{

	gdouble width;
	gint i;
	GnomeCanvasLine *line;
	GnomeCanvasLineExt *lineext;
	gdouble dashes[2] = {3.0, 2.0};
	
	line = GNOME_CANVAS_LINE (gprintable);
	lineext = GNOME_CANVAS_LINE_EXT (gprintable);
	
	if (line->num_points == 0)
		return;

	cairo_set_source_rgba (cr, ((double)(line->fill_rgba >> 24)) / 255.0,
							((double)((line->fill_rgba >> 16) & 0xff)) / 255.0,
							((double)((line->fill_rgba >> 8) & 0xff)) / 255.0,
							((double) (line->fill_rgba & 0xff)) / 255.0);

	if (line->width_pixels)
		width = (double) line->width / line->item.canvas->pixels_per_unit;
	else
		width = line->width;
	cairo_set_line_width (cr, width);

	if (line->first_arrow || line->last_arrow)
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	else switch (line->cap) {
	case GDK_CAP_ROUND:
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
		break;
	case GDK_CAP_PROJECTING:
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
		break;
	default:
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
		break;
	}
	
	cairo_set_line_join (cr, (cairo_line_join_t) line->join);
	
	cairo_set_dash (cr, dashes, (line->line_style == GDK_LINE_ON_OFF_DASH)? 2: 0, 0.);
	
	cairo_move_to (cr, line->coords[0], line->coords[1]);
	for (i = 1; i < line->num_points; i++)
		cairo_line_to (cr, line->coords[2 * i], line->coords[2 * i + 1]);/*FIXME: Change that for spline lines!*/
		
	cairo_stroke (cr);
	cairo_set_line_width (cr, 0.);

	if (line->first_arrow && line->first_coords) {
		cairo_new_path (cr);
		cairo_move_to (cr, line->first_coords[0], line->first_coords[1]);
		cairo_line_to (cr, line->first_coords[2], line->first_coords[3]);
		cairo_line_to (cr, line->first_coords[4], line->first_coords[5]);
		cairo_line_to (cr, line->first_coords[6], line->first_coords[7]);
		cairo_line_to (cr, line->first_coords[8], line->first_coords[9]);
		if (lineext->first_arrow_head_style == ARROW_HEAD_BOTH)
			cairo_line_to (cr, line->first_coords[10], line->first_coords[11]);
		cairo_close_path (cr);
		cairo_fill (cr);
	}
	
	if (line->last_arrow && line->last_coords) {
		cairo_new_path (cr);
		cairo_move_to (cr, line->last_coords[0], line->last_coords[1]);
		cairo_line_to (cr, line->last_coords[2], line->last_coords[3]);
		cairo_line_to (cr, line->last_coords[4], line->last_coords[5]);
		cairo_line_to (cr, line->last_coords[6], line->last_coords[7]);
		cairo_line_to (cr, line->last_coords[8], line->last_coords[9]);
		if (lineext->last_arrow_head_style == ARROW_HEAD_BOTH)
			cairo_line_to (cr, line->last_coords[10], line->last_coords[11]);
		cairo_close_path (cr);
		cairo_fill (cr);
	}

}
