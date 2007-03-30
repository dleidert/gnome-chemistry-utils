/* Bpath item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@acm.org>
 *          Lauris Kaplinski <lauris@ximian.com>
 *          Miguel de Icaza <miguel@kernel.org>
 *          Cody Russell <bratsche@gnome.org>
 *          Rusty Conover <rconover@bangtail.net>
 */

/* These includes are set up for standalone compile. If/when this codebase
   is integrated into libgnomeui, the includes will need to change. */

#include "config.h"
#include <math.h>
#include <string.h>

#include <gtk/gtkobject.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>

#include "gcp-canvas-bpath.h"
#include "gcp-canvas-shape.h"
#include "gnome-canvas-shape-private.h"
#include <libgnomecanvas/gnome-canvas-path-def.h>

enum {
	PROP_0,
	PROP_BPATH,
};

static void gnome_canvas_bpath_ext_class_init   (GnomeCanvasBpathExtClass *class);
static void gnome_canvas_bpath_ext_init         (GnomeCanvasBpathExt      *bpath);
static void gnome_canvas_bpath_ext_destroy      (GtkObject               *object);
static void gnome_canvas_bpath_ext_set_property (GObject               *object,
					     guint                  param_id,
					     const GValue          *value,
                                             GParamSpec            *pspec);
static void gnome_canvas_bpath_ext_get_property (GObject               *object,
					     guint                  param_id,
					     GValue                *value,
                                             GParamSpec            *pspec);

static void   gnome_canvas_bpath_ext_update      (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);


static GnomeCanvasShapeExtClass *parent_class;

GtkType
gnome_canvas_bpath_ext_get_type (void)
{
	static GType bpath_ext_type;

	if (!bpath_ext_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasBpathExtClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_bpath_ext_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasBpathExt),
			0,			/* n_preallocs */
			(GInstanceInitFunc) gnome_canvas_bpath_ext_init,
			NULL			/* value_table */
		};

		bpath_ext_type = g_type_register_static (GNOME_TYPE_CANVAS_SHAPE_EXT, "GnomeCanvasBpathExt",
						     &object_info, 0);
	}

	return bpath_ext_type;
}

static void
gnome_canvas_bpath_ext_class_init (GnomeCanvasBpathExtClass *class)
{
	GObjectClass         *gobject_class;
	GtkObjectClass       *object_class;
	GnomeCanvasItemClass *item_class;

	gobject_class = (GObjectClass *) class;
	object_class = (GtkObjectClass *) class;
	item_class = (GnomeCanvasItemClass *) class;

	parent_class = g_type_class_peek_parent (class);

	/* when this gets checked into libgnomeui, change the
           GTK_TYPE_POINTER to GTK_TYPE_GNOME_CANVAS_BPATH, and add an
           entry to gnome-boxed.defs */

	gobject_class->set_property = gnome_canvas_bpath_ext_set_property;
	gobject_class->get_property = gnome_canvas_bpath_ext_get_property;

	object_class->destroy = gnome_canvas_bpath_ext_destroy;

	g_object_class_install_property (gobject_class,
                                         PROP_BPATH,
                                         g_param_spec_pointer ("bpath", NULL, NULL,
                                                               (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	item_class->update = gnome_canvas_bpath_ext_update;
}

static void
gnome_canvas_bpath_ext_init (GnomeCanvasBpathExt *bpath)
{

}

static void
gnome_canvas_bpath_ext_destroy (GtkObject *object)
{
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gnome_canvas_bpath_ext_set_property (GObject      *object,
                                 guint         param_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
	GnomeCanvasItem         *item;
	GnomeCanvasBpathExt        *bpath;
	GnomeCanvasPathDef      *gpp;

	item = GNOME_CANVAS_ITEM (object);
	bpath = GNOME_CANVAS_BPATH_EXT (object);

	switch (param_id) {
	case PROP_BPATH:
		gpp = g_value_get_pointer (value);

		gnome_canvas_shape_ext_set_path_def (GNOME_CANVAS_SHAPE_EXT (object), gpp);

		gnome_canvas_item_request_update (item);
		break;

	default:
		break;
	}
}


static void
gnome_canvas_bpath_ext_get_property (GObject     *object,
                                 guint        param_id,
                                 GValue      *value,
                                 GParamSpec  *pspec)
{
	GnomeCanvasShapeExt        *shape;

	shape = GNOME_CANVAS_SHAPE_EXT(object);

	switch (param_id) {
	case PROP_BPATH:
		if (shape->priv->path) {
			gnome_canvas_path_def_ref (shape->priv->path);
			g_value_set_pointer (value, shape->priv->path);
		} else
			g_value_set_pointer (value, NULL);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gnome_canvas_bpath_ext_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	if(GNOME_CANVAS_ITEM_CLASS(parent_class)->update) {
		(* GNOME_CANVAS_ITEM_CLASS(parent_class)->update)(item, affine, clip_path, flags);
	}
}
