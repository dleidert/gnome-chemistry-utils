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
/* Rectangle and ellipse item types for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Rusty Conover <rconover@bangtail.net>
 */

#include <config.h>
#include <math.h>
#include "gcp-canvas-rect-ellipse.h"
#include "gcp-canvas-shape.h"
#include <libgnomecanvas/gnome-canvas-util.h>


#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_rgb_svp.h>

/* Base class for rectangle and ellipse item types */

#define noVERBOSE

enum {
	PROP_0,
	PROP_X1,
	PROP_Y1,
	PROP_X2,
	PROP_Y2,
};


static void gnome_canvas_re_ext_class_init (GnomeCanvasREExtClass *class);
static void gnome_canvas_re_ext_init       (GnomeCanvasREExt      *re);
static void gnome_canvas_re_ext_destroy    (GtkObject          *object);
static void gnome_canvas_re_ext_set_property (GObject              *object,
					  guint                 param_id,
					  const GValue         *value,
					  GParamSpec           *pspec);
static void gnome_canvas_re_ext_get_property (GObject              *object,
					  guint                 param_id,
					  GValue               *value,
					  GParamSpec           *pspec);

static void gnome_canvas_rect_ext_update      (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
static void gnome_canvas_ellipse_ext_update      (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);

static GnomeCanvasItemClass *re_ext_parent_class;


GType
gnome_canvas_re_ext_get_type (void)
{
	static GType re_ext_type;

	if (!re_ext_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasREExtClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_re_ext_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasREExt),
			0,			/* n_preallocs */
			(GInstanceInitFunc) gnome_canvas_re_ext_init,
			NULL			/* value_table */
		};

		re_ext_type = g_type_register_static (GNOME_TYPE_CANVAS_SHAPE_EXT, "GnomeCanvasREExt",
						  &object_info, 0);
	}

	return re_ext_type;
}

static void
gnome_canvas_re_ext_class_init (GnomeCanvasREExtClass *class)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	gobject_class = (GObjectClass *) class;
	object_class = (GtkObjectClass *) class;
	item_class = (GnomeCanvasItemClass *) class;

	re_ext_parent_class = g_type_class_peek_parent (class);

	gobject_class->set_property = gnome_canvas_re_ext_set_property;
	gobject_class->get_property = gnome_canvas_re_ext_get_property;

        g_object_class_install_property
                (gobject_class,
                 PROP_X1,
                 g_param_spec_double ("x1", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_Y1,
                 g_param_spec_double ("y1", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_X2,
                 g_param_spec_double ("x2", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_Y2,
                 g_param_spec_double ("y2", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	object_class->destroy = gnome_canvas_re_ext_destroy;
}

static void
gnome_canvas_re_ext_init (GnomeCanvasREExt *re)
{
	re->x1 = 0.0;
	re->y1 = 0.0;
	re->x2 = 0.0;
	re->y2 = 0.0;
	re->path_dirty = 0;
}

static void
gnome_canvas_re_ext_destroy (GtkObject *object)
{
	GnomeCanvasREExt *re;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_RE_EXT (object));

	re = GNOME_CANVAS_RE_EXT (object);

	if (GTK_OBJECT_CLASS (re_ext_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (re_ext_parent_class)->destroy) (object);
}

static void
gnome_canvas_re_ext_set_property (GObject              *object,
			      guint                 param_id,
			      const GValue         *value,
			      GParamSpec           *pspec)
{
	GnomeCanvasItem *item;
	GnomeCanvasREExt *re;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_RE_EXT (object));

	item = GNOME_CANVAS_ITEM (object);
	re = GNOME_CANVAS_RE_EXT (object);

	switch (param_id) {
	case PROP_X1:
		re->x1 = g_value_get_double (value);
		re->path_dirty = 1;
		gnome_canvas_item_request_update (item);
		break;

	case PROP_Y1:
		re->y1 = g_value_get_double (value);
		re->path_dirty = 1;
		gnome_canvas_item_request_update (item);
		break;

	case PROP_X2:
		re->x2 = g_value_get_double (value);
		re->path_dirty = 1;
		gnome_canvas_item_request_update (item);
		break;

	case PROP_Y2:
		re->y2 = g_value_get_double (value);
		re->path_dirty = 1;
		gnome_canvas_item_request_update (item);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gnome_canvas_re_ext_get_property (GObject              *object,
			      guint                 param_id,
			      GValue               *value,
			      GParamSpec           *pspec)
{
	GnomeCanvasREExt *re;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_RE_EXT (object));

	re = GNOME_CANVAS_RE_EXT (object);

	switch (param_id) {
	case PROP_X1:
		g_value_set_double (value,  re->x1);
		break;

	case PROP_Y1:
		g_value_set_double (value,  re->y1);
		break;

	case PROP_X2:
		g_value_set_double (value,  re->x2);
		break;

	case PROP_Y2:
		g_value_set_double (value,  re->y2);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

/* Rectangle item */
static void gnome_canvas_rect_ext_class_init (GnomeCanvasRectExtClass *class);



GType
gnome_canvas_rect_ext_get_type (void)
{
	static GType rect_ext_type;

	if (!rect_ext_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasRectExtClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_rect_ext_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasRectExt),
			0,			/* n_preallocs */
			(GInstanceInitFunc) NULL,
			NULL			/* value_table */
		};

		rect_ext_type = g_type_register_static (GNOME_TYPE_CANVAS_RE_EXT, "GnomeCanvasRectExt",
						    &object_info, 0);
	}

	return rect_ext_type;
}

static void
gnome_canvas_rect_ext_class_init (GnomeCanvasRectExtClass *class)
{
	GnomeCanvasItemClass *item_class;

	item_class = (GnomeCanvasItemClass *) class;

	item_class->update = gnome_canvas_rect_ext_update;
}

static void
gnome_canvas_rect_ext_update (GnomeCanvasItem *item, double affine[6], ArtSVP *clip_path, gint flags)
{	GnomeCanvasREExt *re;	

	GnomeCanvasPathDef *path_def;

	re = GNOME_CANVAS_RE_EXT(item);

	if (re->path_dirty) {		
		path_def = gnome_canvas_path_def_new ();
		
		gnome_canvas_path_def_moveto(path_def, re->x1, re->y1);
		gnome_canvas_path_def_lineto(path_def, re->x2, re->y1);
		gnome_canvas_path_def_lineto(path_def, re->x2, re->y2);
		gnome_canvas_path_def_lineto(path_def, re->x1, re->y2);
		gnome_canvas_path_def_lineto(path_def, re->x1, re->y1);		
		gnome_canvas_path_def_closepath_current(path_def);		
		gnome_canvas_shape_ext_set_path_def (GNOME_CANVAS_SHAPE_EXT (item), path_def);
		gnome_canvas_path_def_unref(path_def);
		re->path_dirty = 0;
	}

	if (re_ext_parent_class->update)
		(* re_ext_parent_class->update) (item, affine, clip_path, flags);
}

/* Ellipse item */


static void gnome_canvas_ellipse_ext_class_init (GnomeCanvasEllipseExtClass *class);


GType
gnome_canvas_ellipse_ext_get_type (void)
{
	static GType ellipse_ext_type;

	if (!ellipse_ext_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasEllipseExtClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_ellipse_ext_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasEllipseExt),
			0,			/* n_preallocs */
			(GInstanceInitFunc) NULL,
			NULL			/* value_table */
		};

		ellipse_ext_type = g_type_register_static (GNOME_TYPE_CANVAS_RE_EXT, "GnomeCanvasEllipseExt",
						       &object_info, 0);
	}

	return ellipse_ext_type;
}

static void
gnome_canvas_ellipse_ext_class_init (GnomeCanvasEllipseExtClass *class)
{
	GnomeCanvasItemClass *item_class;

	item_class = (GnomeCanvasItemClass *) class;

	item_class->update = gnome_canvas_ellipse_ext_update;
}

#define N_PTS 90

static void
gnome_canvas_ellipse_ext_update (GnomeCanvasItem *item, double affine[6], ArtSVP *clip_path, gint flags) {
	GnomeCanvasPathDef *path_def;
	GnomeCanvasREExt *re;

	re = GNOME_CANVAS_RE_EXT(item);

	if (re->path_dirty) {
		gdouble cx, cy, rx, ry;
		gdouble beta = 0.26521648983954400922; /* 4*(1-cos(pi/8))/(3*sin(pi/8)) */
		gdouble sincosA = 0.70710678118654752440; /* sin (pi/4), cos (pi/4) */
		gdouble dx1, dy1, dx2, dy2;
		gdouble mx, my;

		path_def = gnome_canvas_path_def_new();

		cx = (re->x2 + re->x1) * 0.5;
		cy = (re->y2 + re->y1) * 0.5;
		rx = re->x2 - cx;
		ry = re->y2 - cy;

		dx1 = beta * rx;
		dy1 = beta * ry;
		dx2 = beta * rx * sincosA;
		dy2 = beta * ry * sincosA;
		mx = rx * sincosA;
		my = ry * sincosA;

		gnome_canvas_path_def_moveto (path_def, cx + rx, cy);
		gnome_canvas_path_def_curveto (path_def,
					       cx + rx, cy - dy1,
					       cx + mx + dx2, cy - my + dy2,
					       cx + mx, cy - my);
		gnome_canvas_path_def_curveto (path_def,
					       cx + mx - dx2, cy - my - dy2,
					       cx + dx1, cy - ry,
					       cx, cy - ry);
		gnome_canvas_path_def_curveto (path_def,
					       cx - dx1, cy - ry,
					       cx - mx + dx2, cy - my - dy2,
					       cx - mx, cy - my);
		gnome_canvas_path_def_curveto (path_def,
					       cx - mx - dx2, cy - my + dy2,
					       cx - rx, cy - dy1,
					       cx - rx, cy);
		
		gnome_canvas_path_def_curveto (path_def,
					       cx - rx, cy + dy1,
					       cx - mx - dx2, cy + my - dy2,
					       cx - mx, cy + my);
		gnome_canvas_path_def_curveto (path_def,
					       cx - mx + dx2, cy + my + dy2,
					       cx - dx1, cy + ry,
					       cx, cy + ry);
		gnome_canvas_path_def_curveto (path_def,
					       cx + dx1, cy + ry,
					       cx + mx - dx2, cy + my + dy2,
					       cx + mx, cy + my);
		gnome_canvas_path_def_curveto (path_def,
					       cx + mx + dx2, cy + my - dy2,
					       cx + rx, cy + dy1,
					       cx + rx, cy);
		
		gnome_canvas_path_def_closepath_current(path_def);
		
		gnome_canvas_shape_ext_set_path_def (GNOME_CANVAS_SHAPE_EXT (item), path_def);
		gnome_canvas_path_def_unref(path_def);
		re->path_dirty = 0;
	}

	if (re_ext_parent_class->update)
		(* re_ext_parent_class->update) (item, affine, clip_path, flags);
}
