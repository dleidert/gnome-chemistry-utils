/* libgcpcanvas/gcp-canvas-pango.c
 *
 * Copyright (c) 2005 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "gcp-canvas-pango.h"
#include "gprintable.h"
#include <glib/gi18n-lib.h>
#include <cairo/cairo.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkimmulticontext.h>

#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>

#include <math.h>
#include <string.h>

struct _GnomeCanvasPangoPrivate {
	PangoLayout *layout;
	/* Position at anchor */
	double x, y;
	/* Dimensions */
	double width, height; /* user set size */
	double _width, _height; /* size of the pango layout */
	GtkAnchorType anchor;
	guint rgba;
	char *color_name;
	gboolean editing, cursor_visible;
	guint blink_timeout, clicks;
	/* current position and selection bounds */
	gint index, start_sel, xl;
	/* Current line */
	gint line;
	/*  IM */
	guint      reseting_im :1;	/* quick hack to keep gtk_im_context_reset from starting an edit */
	guint      mask_state;
	guint      preedit_length;
	GtkIMContext  *im_context;
	PangoAttrList *insert_attrs;
};

enum {
	PROP_0,
	PROP_LAYOUT,
	PROP_X,
	PROP_Y,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_ANCHOR,
	PROP_FILL_COLOR,
	PROP_EDITING,
};

enum {
	CHANGED,
	SEL_CHANGED,
	LAST_SIGNAL
};
static gulong gnome_canvas_pango_signals [LAST_SIGNAL] = { 0, };

static GnomeCanvasItemClass *parent_class;

static void gnome_canvas_pango_class_init (GnomeCanvasPangoClass *klass);
static void gnome_canvas_pango_init(GnomeCanvasPango *text);
static void gnome_canvas_pango_finalize(GObject *object);
static void gnome_canvas_pango_set_property(GObject *object, guint property_id,
						const GValue *value, GParamSpec *pspec);
static void gnome_canvas_pango_get_property(GObject *object, guint property_id,
						GValue *value, GParamSpec *pspec);
static void gnome_canvas_pango_update(GnomeCanvasItem *item, double *affine,
					  ArtSVP *clip_path, int flags);
static void gnome_canvas_pango_realize(GnomeCanvasItem *item);
static void gnome_canvas_pango_unrealize(GnomeCanvasItem *item);
static double gnome_canvas_pango_point(GnomeCanvasItem *item, 
					   double x, double y,
					   int cx, int cy, 
					   GnomeCanvasItem **actual_item);
static void gnome_canvas_pango_draw(GnomeCanvasItem *item, 
					GdkDrawable *drawable,
					int x, int y, int width, int height);
static void gnome_canvas_pango_render(GnomeCanvasItem *item,
					  GnomeCanvasBuf *buf);
static gint gnome_canvas_pango_event(GnomeCanvasItem *item, 
					 GdkEvent *event);
static void gnome_canvas_pango_get_bounds(GnomeCanvasItem *text, double *px1, double *py1,
	   double *px2, double *py2);
static void gnome_canvas_pango_export_svg   (GPrintable *gprintable, xmlDocPtr doc, xmlNodePtr node);
static void gnome_canvas_pango_draw_cairo (GPrintable *gprintable, cairo_t *cr);
/* some code imported from gnumeric/src/workbook-edit.c */

static gboolean
cb_set_attr_list_len (PangoAttribute *a, gpointer len_bytes)
{
	a->start_index = 0;
	a->end_index = GPOINTER_TO_INT (len_bytes);
	return FALSE;
}

struct cb_splice {
	guint pos, len;
	PangoAttrList *result;
};

static gboolean
cb_splice (PangoAttribute *attr, gpointer _data)
{
	struct cb_splice *data = _data;

	if (attr->start_index >= data->pos) {
		PangoAttribute *new_attr = pango_attribute_copy (attr);
		new_attr->start_index += data->len;
		new_attr->end_index += data->len;
		pango_attr_list_insert (data->result, new_attr);
	} else if (attr->end_index <= data->pos) {
		PangoAttribute *new_attr = pango_attribute_copy (attr);
		pango_attr_list_insert (data->result, new_attr);
	} else {
		PangoAttribute *new_attr = pango_attribute_copy (attr);
		new_attr->end_index = data->pos;
		pango_attr_list_insert (data->result, new_attr);

		new_attr = pango_attribute_copy (attr);
		new_attr->start_index = data->pos + data->len;
		new_attr->end_index += data->len;
		pango_attr_list_insert (data->result, new_attr);
	}

	return FALSE;
}

static gboolean
cb_splice_true (G_GNUC_UNUSED PangoAttribute *attr, G_GNUC_UNUSED gpointer data)
{
	return TRUE;
}

static void
gnome_canvas_pango_attr_list_splice (PangoAttrList *tape,
			    PangoAttrList *piece,
			    guint pos, guint len)
{
	struct cb_splice data;
	PangoAttrList *tape2;

	data.result = tape;
	data.pos = pos;
	data.len = len;

	/* Clean out the tape.  */
	tape2 = pango_attr_list_filter (tape, cb_splice_true, NULL);

	if (tape2) {
		(void)pango_attr_list_filter (tape2, cb_splice, &data);
		pango_attr_list_unref (tape2);
	}

	/* Apply the new attributes.  */
	pango_attr_list_splice (data.result, piece, pos, 0);
}

typedef struct {
	unsigned start_pos, end_pos, len; /* in bytes not chars */
} EntryDeleteTextClosure;

/*
 *
 * |       +-------------------+            The attribute
 *
 *                                +----+    (1)
 *                  +------------------+    (2)
 *                  +------+                (3)
 *   +--+                                   (4)
 *   +--------------+                       (5)
 *   +---------------------------------+    (6)
 *
 */
static gboolean
cb_delete_filter (PangoAttribute *a, EntryDeleteTextClosure *change)
{
	if (change->start_pos >= a->end_index)
		return FALSE;  /* (1) */

	if (change->start_pos <= a->start_index) {
		if (change->end_pos >= a->end_index)
			return TRUE; /* (6) */

		a->end_index -= change->len;
		if (change->end_pos >= a->start_index)
			a->start_index = change->start_pos; /* (5) */
		else
			a->start_index -= change->len; /* (4) */
	} else {
		if (change->end_pos >= a->end_index)
			a->end_index = change->start_pos;  /* (2) */
		else
			a->end_index -= change->len; /* (3) */
	}

	return FALSE;
}

static void
delete_text (PangoAttrList *list, int start, int length)
{
	PangoAttrList *gunk;
	EntryDeleteTextClosure change;

	change.start_pos = start;
	change.end_pos   = start + length;
	change.len = length;
	gunk = pango_attr_list_filter (list,
					   (PangoAttrFilterFunc) cb_delete_filter, &change);
	if (gunk != NULL)
		pango_attr_list_unref (gunk);
}

/* IM Context Callbacks
 */

static void
gnome_canvas_pango_commit_cb (GtkIMContext *context, const gchar *str, GnomeCanvasPango *text)
{
	GString *string = g_string_new (pango_layout_get_text (text->_priv->layout));
	int sel_length = abs (text->_priv->index - text->_priv->start_sel);
	int len = strlen (str);
	if (sel_length > 0) {
		text->_priv->index = text->_priv->start_sel =
			MIN (text->_priv->index, text->_priv->start_sel);
		g_string_erase (string, text->_priv->index, sel_length);
		delete_text (pango_layout_get_attributes (text->_priv->layout),
			text->_priv->index, sel_length);
	}
	g_string_insert (string, text->_priv->index, str);
	pango_layout_set_text (text->_priv->layout, string->str, -1);
	pango_attr_list_filter (text->_priv->insert_attrs,
					      cb_set_attr_list_len,
					      GINT_TO_POINTER (len));

	gnome_canvas_pango_attr_list_splice (pango_layout_get_attributes (text->_priv->layout),
					    text->_priv->insert_attrs,
					    text->_priv->index, len);
	text->_priv->start_sel = text->_priv->index += len;
	g_string_free (string, TRUE);
	g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [CHANGED], 0);
	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (text));
}

static void
gnome_canvas_pango_preedit_changed_cb (GtkIMContext *context, GnomeCanvasPango *text)
{
	/* TODO: write this function */
}

static gboolean
gnome_canvas_pango_retrieve_surrounding_cb (GtkIMContext *context, GnomeCanvasPango *text)
{
	/* TODO: write this function */
	return TRUE;
}

static gboolean
gnome_canvas_pango_delete_surrounding_cb (GtkIMContext *context,
                                  gint         offset,
                                  gint         n_chars,
                                  GnomeCanvasPango *text)
{
	/* TODO: write this function */
	return TRUE;
}

#define PREBLINK_TIME 300
#define CURSOR_ON_TIME 800
#define CURSOR_OFF_TIME 400

static gint
blink_cb (gpointer data)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (data);

	if (text->_priv->cursor_visible)
		text->_priv->blink_timeout = g_timeout_add (
			CURSOR_OFF_TIME, blink_cb, text);
	else
		text->_priv->blink_timeout = g_timeout_add (
			CURSOR_ON_TIME, blink_cb, text);

	text->_priv->cursor_visible = !text->_priv->cursor_visible;
	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (text));
	/* Remove ourself */
	return FALSE;
}

static void
gnome_canvas_pango_print_init (GPrintableIface *iface)
{
	iface->export_svg = gnome_canvas_pango_export_svg;
	iface->draw_cairo = gnome_canvas_pango_draw_cairo;
}

GType
gnome_canvas_pango_get_type(void)
{
	static GType pango_type;

	if (!pango_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasPangoClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_pango_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasPango),
			0,			/* n_preallocs */
			(GInstanceInitFunc) gnome_canvas_pango_init,
			NULL			/* value_table */
		};

		static const GInterfaceInfo print_info = {
			(GInterfaceInitFunc) gnome_canvas_pango_print_init,
			NULL, NULL
		};

		pango_type = g_type_register_static (GNOME_TYPE_CANVAS_ITEM, "GnomeCanvasPango",
							 &object_info, 0);

		g_type_add_interface_static (pango_type, G_TYPE_PRINTABLE, &print_info);
	}

	return pango_type;
}

static void
gnome_canvas_pango_class_init (GnomeCanvasPangoClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
/*	GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);	*/
	GnomeCanvasItemClass *item_class = GNOME_CANVAS_ITEM_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	gobject_class->set_property = gnome_canvas_pango_set_property;
	gobject_class->get_property = gnome_canvas_pango_get_property;
	gobject_class->finalize = gnome_canvas_pango_finalize;

	g_object_class_install_property (
		gobject_class,
		PROP_LAYOUT,
		g_param_spec_object ("layout",
				     _("Layout"),
				     _("Pango layout"),
				     PANGO_TYPE_LAYOUT,
				     G_PARAM_READWRITE));
	g_object_class_install_property (
		gobject_class,
		PROP_X,
		g_param_spec_double ("x",
				     _("X"),
				     _("X position"),
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				     G_PARAM_READWRITE));
	g_object_class_install_property (
		gobject_class,
		PROP_Y,
		g_param_spec_double ("y",
				     _("Y"),
				     _("Y position"),
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				     G_PARAM_READWRITE));
	g_object_class_install_property (
		gobject_class,
		PROP_WIDTH,
		g_param_spec_double ("width",
				     _("Width"),
				     _("Width for text box"),
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				     G_PARAM_READWRITE));
	g_object_class_install_property (
		gobject_class,
		PROP_HEIGHT,
		g_param_spec_double ("height",
				     _("Height"),
				     _("Height for text box"),
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				     G_PARAM_READWRITE));
	g_object_class_install_property (
		gobject_class,
		PROP_ANCHOR,
		g_param_spec_enum ("anchor",
				   _("Anchor"),
				   _("Anchor point for text"),
				   GTK_TYPE_ANCHOR_TYPE,
				   GTK_ANCHOR_NW,
				   G_PARAM_READWRITE));
	g_object_class_install_property (
		gobject_class,
		PROP_FILL_COLOR,
		g_param_spec_string ("fill_color",
					_("Color"),
					_("Text color, as string"),
					NULL,
					(G_PARAM_READWRITE)));
	g_object_class_install_property (
		gobject_class,
		PROP_EDITING,
		g_param_spec_boolean ("editing",
				      _("Editing"),
				      _("Is this rich text item currently edited?"),
				      FALSE,
				      G_PARAM_READWRITE));

	gnome_canvas_pango_signals [CHANGED] = g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GnomeCanvasPangoClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);
	gnome_canvas_pango_signals [SEL_CHANGED] = g_signal_new ("sel-changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GnomeCanvasPangoClass, sel_changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__POINTER,
		G_TYPE_NONE, 1, G_TYPE_POINTER);

	item_class->update = gnome_canvas_pango_update;
	item_class->realize = gnome_canvas_pango_realize;
	item_class->unrealize = gnome_canvas_pango_unrealize;
	item_class->draw = gnome_canvas_pango_draw;
	item_class->point = gnome_canvas_pango_point;
	item_class->render = gnome_canvas_pango_render;
	item_class->event = gnome_canvas_pango_event;
	item_class->bounds = gnome_canvas_pango_get_bounds;
}

static void
gnome_canvas_pango_init (GnomeCanvasPango *text)
{
	text->_priv = g_new0 (GnomeCanvasPangoPrivate, 1);
	text->_priv->anchor = GTK_ANCHOR_NW;
	text->_priv->im_context = gtk_im_multicontext_new ();
	text->_priv->preedit_length = 0;
	text->_priv->insert_attrs = pango_attr_list_new ();
	text->_priv->reseting_im = FALSE;
	text->_priv->clicks = 0;
	g_signal_connect (G_OBJECT (text->_priv->im_context), "commit",
		G_CALLBACK (gnome_canvas_pango_commit_cb), text);
	g_signal_connect (G_OBJECT (text->_priv->im_context), "preedit_changed",
		G_CALLBACK (gnome_canvas_pango_preedit_changed_cb), text);
	g_signal_connect (G_OBJECT (text->_priv->im_context), "retrieve_surrounding",
		G_CALLBACK (gnome_canvas_pango_retrieve_surrounding_cb), text);
	g_signal_connect (G_OBJECT (text->_priv->im_context), "delete_surrounding",
		G_CALLBACK (gnome_canvas_pango_delete_surrounding_cb), text);
}

static void
gnome_canvas_pango_finalize (GObject *object)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (object);
	g_return_if_fail (text);

	if (text->_priv->blink_timeout != 0) {
		g_source_remove (text->_priv->blink_timeout);
		text->_priv->blink_timeout = 0;
	}
	/*remove idle calls */
	while (g_idle_remove_by_data (object));

	if (text->_priv->layout)
		g_object_unref (text->_priv->layout);
	if (text->_priv->insert_attrs)
		pango_attr_list_unref (text->_priv->insert_attrs);
	if (text->_priv->color_name != NULL)
		g_free (text->_priv->color_name);
	g_object_unref (text->_priv->im_context);
	g_free (text->_priv);
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnome_canvas_pango_set_property (GObject *object, guint property_id,
						const GValue *value, GParamSpec *pspec)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (object);

	switch (property_id) {
	case PROP_LAYOUT:
		if (text->_priv->layout)
			g_object_unref (text->_priv->layout);
		text->_priv->layout = g_value_get_object (value);
		g_object_ref (text->_priv->layout);
		text->_priv->line = 0;
		text->_priv->index = text->_priv->start_sel = 0;
		break;
	case PROP_X:
		text->_priv->x = g_value_get_double (value);
		break;
	case PROP_Y:
		text->_priv->y = g_value_get_double (value);
		break;
	case PROP_WIDTH:
		text->_priv->width = g_value_get_double (value);
		break;
	case PROP_HEIGHT:
		text->_priv->height = g_value_get_double (value);
		break;
	case PROP_ANCHOR:
		text->_priv->anchor = g_value_get_enum (value);
		break;
	case PROP_FILL_COLOR: {
		const char *color_name;
		GdkColor color;

		if (text->_priv->color_name != NULL) {
			g_free (text->_priv->color_name);
			text->_priv->color_name = NULL;
		}

		color_name = g_value_get_string (value);
		if (color_name) {
			text->_priv->color_name = g_strdup (color_name);
			gdk_color_parse (color_name, &color);
			text->_priv->rgba = ((color.red & 0xff00) << 16 |
			(color.green & 0xff00) << 8 |
			(color.blue & 0xff00) |
			0xff);
		} else
			text->_priv->rgba = 0xff;
		break;
	}
	case PROP_EDITING: {
		gboolean editing = g_value_get_boolean (value);
		if (editing == text->_priv->editing)
			break;
		text->_priv->editing = editing;
		if (editing) {
			text->_priv->cursor_visible = TRUE;
			text->_priv->blink_timeout = g_timeout_add (
				CURSOR_ON_TIME, blink_cb, text);
		} else {
			text->_priv->cursor_visible = FALSE;
			if (text->_priv->blink_timeout != 0) {
				g_source_remove (text->_priv->blink_timeout);
				text->_priv->blink_timeout = 0;
			}
			while (	g_idle_remove_by_data (object));
		}
		break;
	}
		       
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (text));
}

static void
gnome_canvas_pango_get_property (GObject *object, guint property_id,
						GValue *value, GParamSpec *pspec)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (object);

	switch (property_id) {
	case PROP_LAYOUT:
		g_value_set_object (value, text->_priv->layout);
		break;
	case PROP_X:
		g_value_set_double (value, text->_priv->x);
		break;
	case PROP_Y:
		g_value_set_double (value, text->_priv->y);
		break;
	case PROP_WIDTH:
		g_value_set_double (value, text->_priv->width);
		break;
	case PROP_HEIGHT:
		g_value_set_double (value, text->_priv->height);
		break;
	case PROP_ANCHOR:
		g_value_set_enum (value, text->_priv->anchor);
		break;
	case PROP_FILL_COLOR:
		g_value_set_string (value, text->_priv->color_name);
		break;
	case PROP_EDITING:
		g_value_set_boolean (value, text->_priv->editing);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void
adjust_for_anchors(GnomeCanvasPango *text, double *ax, double *ay)
{
	double x, y;
	double width = (text->_priv->width > 0)? text->_priv->width: text->_priv->_width;
	double height = (text->_priv->height > 0)? text->_priv->height: text->_priv->_height;

	x = text->_priv->x;
	y = text->_priv->y;

	/* Anchor text */
	/* X coordinates */
	switch (text->_priv->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		x -= width / 2;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		x -= width;
		break;
	default:
		break;
	}

	/* Y coordinates */
	switch (text->_priv->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		y -= height / 2;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		y -= height;
		break;
	default:
		break;
	}

	if (ax)
		*ax = x;
	if (ay)
		*ay = y;
} /* adjust_for_anchors */

static void
gnome_canvas_pango_update (GnomeCanvasItem *item, double *affine,
					  ArtSVP *clip_path, int flags)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);
	double i2w[6], w2c[6], i2c[6];
	double x1, y1, x2, y2;
	double width = (text->_priv->width > 0)? text->_priv->width: text->_priv->_width;
	ArtPoint ip, cp;

	GNOME_CANVAS_ITEM_CLASS (parent_class)->update ( item, affine, clip_path, flags);

	gnome_canvas_item_i2w_affine (item, i2w);
	gnome_canvas_w2c_affine (item->canvas, w2c);
	art_affine_multiply (i2c, i2w, w2c);

	if (text->_priv->layout) {
		PangoRectangle rect;
		pango_layout_get_extents (text->_priv->layout, NULL, &rect);
		text->_priv->_width = rect.width / PANGO_SCALE;
		text->_priv->_height = rect.height / PANGO_SCALE;
	}

	adjust_for_anchors (text, &x1, &y1);
	if (width < 1.)
		width = 1.;
	x2 = x1 + width;
	y2 = y1 + ((text->_priv->height > 0)? text->_priv->height: text->_priv->_height);

	ip.x = x1;
	ip.y = y1;
	art_affine_point (&cp, &ip, i2c);
	x1 = cp.x;
	y1 = cp.y;

	ip.x = x2;
	ip.y = y2;
	art_affine_point (&cp, &ip, i2c);
	x2 = cp.x;
	y2 = cp.y;

	gnome_canvas_update_bbox (item, x1, y1, x2 + 1, y2);
}

static void
gnome_canvas_pango_realize (GnomeCanvasItem *item)
{
}

static void
gnome_canvas_pango_unrealize (GnomeCanvasItem *item)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);

	if (text->_priv->blink_timeout != 0) {
		g_source_remove (text->_priv->blink_timeout);
		text->_priv->blink_timeout = 0;
	}
	/*remove idle calls */
	while (g_idle_remove_by_data (item));

	(* GNOME_CANVAS_ITEM_CLASS(parent_class)->unrealize)(item);
}

static double
gnome_canvas_pango_point (GnomeCanvasItem *item, 
					   double x, double y,
					   int cx, int cy, 
					   GnomeCanvasItem **actual_item)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);
	double ax, ay;
	double x1, x2, y1, y2;
	double dx, dy;

	*actual_item = item;

	/* This is a lame cop-out. Anywhere inside of the bounding box. */

	adjust_for_anchors(text, &ax, &ay);

	x1 = ax;
	y1 = ay;
	x2 = ax + ((text->_priv->width > 0)? text->_priv->width: text->_priv->_width);
	y2 = ay + ((text->_priv->height > 0)? text->_priv->height: text->_priv->_height);

	if ((x > x1) && (y > y1) && (x < x2) && (y < y2))
		return 0.0;

	if (x < x1)
		dx = x1 - x;
	else if (x > x2)
		dx = x - x2;
	else
		dx = 0.0;

	if (y < y1)
		dy = y1 - y;
	else if (y > y2)
		dy = y - y2;
	else
		dy = 0.0;

	return sqrt(dx * dx + dy * dy);
}

static void
gnome_canvas_pango_draw (GnomeCanvasItem *item, 
					GdkDrawable *drawable,
					int x, int y, int width, int height)
{
}

static void
gnome_canvas_pango_render (GnomeCanvasItem *item,
					  GnomeCanvasBuf *buf)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);
	double x0, y0;
	double i2w[6], w2c[6], i2c[6];
	double ax, ay;
	double x1, y1, x2, y2;
	int x, y, px, py, h, w, dw, i, j;
	ArtPoint ip, cp;
	guint8 r, g, b, *dst, *src;
	guchar *data;
	cairo_surface_t *surf;
	cairo_t *cr;
	cairo_matrix_t matrix;
	PangoAttrList *attrs = pango_layout_get_attributes (text->_priv->layout);
	if (attrs)
		pango_attr_list_ref (attrs);

	g_return_if_fail (text);
	g_return_if_fail (text->_priv->layout);

	adjust_for_anchors (text, &ax, &ay);

	gnome_canvas_buf_ensure_buf (buf);

	gnome_canvas_item_i2w_affine (item, i2w);
	gnome_canvas_w2c_affine (item->canvas, w2c);
	art_affine_multiply (i2c, i2w, w2c);

	matrix.xx = i2c[0];
	matrix.xy = i2c[1];
	matrix.yx = i2c[2];
	matrix.yy = i2c[3];

	ip.x = ax;
	ip.y = ay;
	art_affine_point (&cp, &ip, i2c);
	x0 = x1 = floor (cp.x + 0.5);
	y0 = y1 = floor (cp.y + 0.5);

	ip.x = ax + ((text->_priv->width > 0)? text->_priv->width: text->_priv->_width);
	ip.y = ay + ((text->_priv->height > 0)? text->_priv->height: text->_priv->_height);
	art_affine_point (&cp, &ip, i2c);
	x2 = floor (cp.x + 0.5);
	y2 = floor (cp.y + 0.5);

	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	if (x0 < buf->rect.x0) {
		w -= buf->rect.x0 - x0;
		x = 0;
		px = x0 - buf->rect.x0;
		x0 = buf->rect.x0;
	} else  {
		x = x0 - buf->rect.x0;
		px = 0;
	}
	if (x0 + w >= buf->rect.x1)
		w = buf->rect.x1 - x0;
	if (y0 < buf->rect.y0)
	{
		h -= buf->rect.y0 - y0;
		y = 0;
		py = y0 - buf->rect.y0;
		y0 = buf->rect.y0;
	} else {
		y = y0 - buf->rect.y0;
		py = 0;
	}
	if (y0 + h >= buf->rect.y1)
		h = buf->rect.y1 - y0;
	if (((int) w <= 0) || ((int) h <= 0))
		return;

	matrix.x0 = px;
	matrix.y0 = py;
	/* cairo image surfaces must have width that are multiples of 4 */
	dw = (w / 4 + 1) * 4;
	data = g_malloc0 (w * h * 4);
	/* copy existing buffer */
	src = data;
	dst = buf->buf + (int) y * buf->buf_rowstride + (int) x * 3;
	j = h;

	while (j-- > 0) {
		for (i = w; i-- > 0 ; dst += 3, src += 4) {
			src[2] = dst[0];
			src[1] = dst[1];
			src[0] = dst[2];
		}
		dst += buf->buf_rowstride - w * 3;
	}
	surf = cairo_image_surface_create_for_data (data, CAIRO_FORMAT_RGB24,
				w, h, w * 4);
	cr = cairo_create (surf);
	cairo_set_matrix (cr, &matrix);
	/* change colors for selected text */
	if (text->_priv->editing && text->_priv->index != text->_priv->start_sel) {
		PangoAttribute *fg, *bg;
		PangoAttrList *list = (attrs)? pango_attr_list_copy (attrs): pango_attr_list_new ();
		fg = pango_attr_foreground_new (0xffff, 0xffff, 0xffff);
		bg = pango_attr_background_new (0x8000, 0x8000, 0x8000);
		if (text->_priv->index > text->_priv->start_sel) {
			fg->start_index = bg->start_index = text->_priv->start_sel;
			fg->end_index = bg->end_index = text->_priv->index;
		} else {
			fg->start_index = bg->start_index = text->_priv->index;
			fg->end_index = bg->end_index = text->_priv->start_sel;
		}
		pango_attr_list_insert (list, bg);
		pango_attr_list_insert (list, fg);
		pango_layout_set_attributes (text->_priv->layout, list);
		pango_attr_list_unref (list);
	}
	cairo_set_source_rgb (cr,
			(double) (text->_priv->rgba >> 24) /255,
			(double) ((text->_priv->rgba >> 16) & 0xff) / 255.,
			(double) ((text->_priv->rgba >> 8) & 0xff) / 255.);
	pango_cairo_update_layout (cr, text->_priv->layout);
	pango_cairo_show_layout (cr, text->_priv->layout);
	pango_context_set_matrix (pango_layout_get_context (text->_priv->layout), NULL);

	if (text->_priv->cursor_visible) {
		PangoRectangle rect;
		cairo_set_source_rgb (cr, 0., 0., 0.);
		pango_layout_get_cursor_pos (text->_priv->layout,
								text->_priv->index, &rect, NULL);
		cairo_new_path (cr);
		cairo_move_to (cr, rect.x / PANGO_SCALE, rect.y / PANGO_SCALE);
		cairo_rel_line_to (cr, 0, rect.height / PANGO_SCALE);
		cairo_stroke (cr);
	}

	r = (guint8) text->_priv->rgba >> 24;
	g = (guint8) (text->_priv->rgba >> 16) & 0xff;
	b = (guint8) (text->_priv->rgba >> 8) & 0xff;

	src = data;
	dst = buf->buf + (int) y * buf->buf_rowstride + (int) x * 3;

	while (h-- > 0) {
		for (i = w; i-- > 0 ; dst += 3, src += 4) {
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
		}
		dst += buf->buf_rowstride - w * 3;
	}

	cairo_destroy (cr);
	cairo_surface_destroy (surf);
	g_free (data);
	pango_layout_set_attributes (text->_priv->layout, attrs);
	if (attrs)
		pango_attr_list_unref (attrs);
}

static gboolean
get_event_coordinates(GdkEvent *event, gint *x, gint *y)
{
	g_return_val_if_fail(event, FALSE);
	
	switch (event->type) {
	case GDK_MOTION_NOTIFY:
		*x = event->motion.x;
		*y = event->motion.y;
		return TRUE;
		break;
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		*x = event->button.x;
		*y = event->button.y;
		return TRUE;
		break;

	default:
		return FALSE;
		break;
	}
} /* get_event_coordinates */

static void
gnome_canvas_pango_update_line_pos (GnomeCanvasPango *text)
{
	int i = 0;
	char const *cts = pango_layout_get_text (text->_priv->layout);
	PangoLayoutLine *line = pango_layout_get_line (text->_priv->layout, i);
	if (text->_priv->index > strlen (cts))
		text->_priv->index = strlen (cts);
	else if (text->_priv->index < 0)
		text->_priv->index =  0;
	while (line) {
		if (text->_priv->index >= line->start_index &&
				text->_priv->index <= line->start_index + line->length)
			break;
		line = pango_layout_get_line (text->_priv->layout, ++i);
	}
	pango_layout_line_index_to_x (line, text->_priv->index, 0, &text->_priv->xl);
	if (!line) {
		/* Should not occur */
		g_warning ("How did we get there?");
		i--;
	}
	text->_priv->line = i;
}

static gint
gnome_canvas_pango_button_press_event (GnomeCanvasItem *item, 
				       GdkEventButton *event)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);
	double newx, newy;
	int index, trailing, sel_start, sel_end;

	newx = (event->x - text->_priv->x) * item->canvas->pixels_per_unit;
	newy = (event->y - text->_priv->y) * item->canvas->pixels_per_unit;

	text->_priv->clicks++;
	if (text->_priv->index >= text->_priv->start_sel) {
		sel_start = text->_priv->start_sel;
		sel_end = text->_priv->index;
	} else {
		sel_end = text->_priv->start_sel;
		sel_start = text->_priv->index;
	}
	pango_layout_xy_to_index (text->_priv->layout, newx * PANGO_SCALE,
		newy * PANGO_SCALE, &index, &trailing);
	index += trailing;
	if (index >= sel_start && index <= sel_end) {
		/* TODO: start drag and drop, at the moment, just position the cursor */
		text->_priv->index = text->_priv->start_sel = index;
	} else {
		text->_priv->index = text->_priv->start_sel = index;
	}
	gnome_canvas_pango_update_line_pos (text);
	text->_priv->start_sel = text->_priv->index;
	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (text));
	return TRUE;
}

static gint
gnome_canvas_pango_button_release_event (GnomeCanvasItem *item, 
				       GdkEventButton *event)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);
	/* TODO: use a timeout to reset clicks */
	text->_priv->clicks = 0;
	struct GnomeCanvasPangoSelBounds bounds;
	bounds.start = text->_priv->start_sel;
	bounds.cur = text->_priv->index;
	g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
	return TRUE;
}

static gint
gnome_canvas_pango_drag_event (GnomeCanvasItem *item, 
				       GdkEventButton *event)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);
	double newx, newy;
	int trailing;

	newx = (event->x - text->_priv->x) * item->canvas->pixels_per_unit;
	newy = (event->y - text->_priv->y) * item->canvas->pixels_per_unit;
	pango_layout_xy_to_index (text->_priv->layout, newx * PANGO_SCALE,
		newy * PANGO_SCALE, &text->_priv->index, &trailing);
	text->_priv->index += trailing;
	gnome_canvas_pango_update_line_pos (text);
	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (text));
	/* TODO: implement DnD */
	return TRUE;
}

static gint
gnome_canvas_pango_key_press_event (GnomeCanvasItem *item, 
				       GdkEventKey *event)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);
	PangoLayoutLine *line;
	struct GnomeCanvasPangoSelBounds bounds;

	if (!text->_priv->layout)
		return FALSE;

	if (gtk_im_context_filter_keypress (text->_priv->im_context, event)) {
		gnome_canvas_pango_update_line_pos (text);
		return TRUE;
	}

	switch (event->keyval) {
	case GDK_Control_L:
	case GDK_Control_R:
		return FALSE;
	case GDK_Return:
	case GDK_KP_Enter:
		gnome_canvas_pango_commit_cb (text->_priv->im_context, "\n", text);
		break;

	case GDK_Tab:
		gnome_canvas_pango_commit_cb (text->_priv->im_context, "\t", text);
		break;

	/* MOVEMENT */
	case GDK_Right:
		if (text->_priv->index == strlen (pango_layout_get_text (text->_priv->layout)))
			break;
		if (event->state & GDK_CONTROL_MASK) {
			/* move to end of word */
			/* TODO: write this code */
			/* PangoLanguage *language = pango_context_get_language (pango_layout_get_context (text->_priv->layout)); */
		} else {
			int trailing;
			pango_layout_move_cursor_visually (text->_priv->layout, TRUE,
				text->_priv->index, 0, 1, &text->_priv->index, &trailing);
			text->_priv->index += trailing;
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_Left:
		if (text->_priv->index == 0)
			break;
		if (event->state & GDK_CONTROL_MASK) {
			/* move to start of word */
			/* TODO: write this code */
		} else {
			int trailing;
			pango_layout_move_cursor_visually (text->_priv->layout, TRUE,
				text->_priv->index, 0, -1, &text->_priv->index, &trailing);
			text->_priv->index += trailing;
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_f:
		if (event->state & GDK_CONTROL_MASK) {
			int trailing;
			pango_layout_move_cursor_visually (text->_priv->layout, TRUE,
				text->_priv->index, 0, 1, &text->_priv->index, &trailing);
			text->_priv->index += trailing;
		} else if (event->state & GDK_MOD1_MASK) {
			/* move to end of word */
			/* TODO: write this code */
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_b:
		if (event->state & GDK_CONTROL_MASK) {
			int trailing;
			pango_layout_move_cursor_visually (text->_priv->layout, TRUE,
				text->_priv->index, 0, -1, &text->_priv->index, &trailing);
			text->_priv->index += trailing;
		} else if (event->state & GDK_MOD1_MASK) {
			/* move to start of word */
			/* TODO: write this code */
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_p:
		if (!(event->state & GDK_CONTROL_MASK))
			break;
	case GDK_Up:
		if (text->_priv->line > 0) {
			int trailing;
			text->_priv->line--;
			line = pango_layout_get_line (text->_priv->layout, text->_priv->line);
			pango_layout_line_x_to_index (line, text->_priv->xl, &text->_priv->index, &trailing);
			if (trailing)
				text->_priv->index++;
			pango_layout_line_index_to_x (line, text->_priv->index, 0, &text->_priv->xl);
			bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
			bounds.cur = text->_priv->index;
			g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		}
		break;
	case GDK_n:
		if (!(event->state & GDK_CONTROL_MASK))
			break;
	case GDK_Down:
		if (text->_priv->line < pango_layout_get_line_count (text->_priv->layout) - 1) {
			int trailing;
			text->_priv->line++;
			line = pango_layout_get_line (text->_priv->layout, text->_priv->line);
			pango_layout_line_x_to_index (line, text->_priv->xl, &text->_priv->index, &trailing);
			if (trailing)
				text->_priv->index++;
			pango_layout_line_index_to_x (line, text->_priv->index, 0, &text->_priv->xl);
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_Home:
		if (event->state & GDK_CONTROL_MASK) {
			text->_priv->index = text->_priv->line = 0;
		} else {
			PangoLayoutLine *line = pango_layout_get_line (text->_priv->layout,
				text->_priv->line);
			text->_priv->index = line->start_index;
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_End:
		if (event->state & GDK_CONTROL_MASK) {
			char const *cts = pango_layout_get_text (text->_priv->layout);
			text->_priv->index = strlen (cts);
			text->_priv->line = pango_layout_get_line_count (text->_priv->layout);
		} else {
			PangoLayoutLine *line = pango_layout_get_line (text->_priv->layout,
				text->_priv->line);
			text->_priv->index = line->start_index + line->length;
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_a:
		if (event->state & GDK_CONTROL_MASK) {
			PangoLayoutLine *line = pango_layout_get_line (text->_priv->layout,
				text->_priv->line);
			text->_priv->index = line->start_index;
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_e:
		if (event->state & GDK_CONTROL_MASK) {
			PangoLayoutLine *line = pango_layout_get_line (text->_priv->layout,
				text->_priv->line);
			text->_priv->index = line->start_index + line->length;
		}
		bounds.start = (event->state & GDK_SHIFT_MASK)? text->_priv->start_sel: text->_priv->index;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;

	/* DELETING TEXT */
	case GDK_Delete:
	case GDK_KP_Delete: {
		GString *string = g_string_new (pango_layout_get_text (text->_priv->layout));
		if (event->state & GDK_CONTROL_MASK) {
			/* delete from cursor to end of word */
			/* TODO: write this code */
		} else {
			int sel_length = abs (text->_priv->index - text->_priv->start_sel);
			if (sel_length > 0) {
				text->_priv->index = text->_priv->start_sel =
					MIN (text->_priv->index, text->_priv->start_sel);
				g_string_erase (string, text->_priv->index, sel_length);
				delete_text (pango_layout_get_attributes (text->_priv->layout),
					text->_priv->index, sel_length);
			} else if (text->_priv->index < string->len) {
				char const* end = g_utf8_find_next_char (string->str + text->_priv->index, NULL);
				int delta = end - string->str - text->_priv->index;
				g_string_erase (string, text->_priv->index,delta);
				delete_text (pango_layout_get_attributes (text->_priv->layout),
					text->_priv->index, delta);
			}
		}
		pango_layout_set_text (text->_priv->layout, string->str, -1);
		g_string_free (string, TRUE);
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [CHANGED], 0);
		bounds.start = text->_priv->start_sel;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	}
	case GDK_d:
		if (event->state & GDK_CONTROL_MASK) {
		} else if (event->state & GDK_MOD1_MASK) {
			/* delete from cursor to end of word */
			/* TODO: write this code */
		}
		bounds.start = text->_priv->start_sel;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	case GDK_BackSpace: {
		GString *string = g_string_new (pango_layout_get_text (text->_priv->layout));
		if (event->state & GDK_CONTROL_MASK) {
			/* delete from cursor to start of word */
			/* TODO: write this code */
		} else {
			int sel_length = abs (text->_priv->index - text->_priv->start_sel);
			if (sel_length > 0) {
				text->_priv->index = text->_priv->start_sel =
					MIN (text->_priv->index, text->_priv->start_sel);
				g_string_erase (string, text->_priv->index, sel_length);
				delete_text (pango_layout_get_attributes (text->_priv->layout),
					text->_priv->index, sel_length);
			} else if (text->_priv->index > 0) {
				char const* end = g_utf8_find_prev_char (string->str, string->str + text->_priv->index);
				int delta = string->str + text->_priv->index - end;
				text->_priv->start_sel = text->_priv->index -= delta;
				g_string_erase (string, text->_priv->index, delta);
				delete_text (pango_layout_get_attributes (text->_priv->layout),
					text->_priv->index, delta);
			}
		}
		pango_layout_set_text (text->_priv->layout, string->str, -1);
		g_string_free (string, TRUE);
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [CHANGED], 0);
		bounds.start = text->_priv->start_sel;
		bounds.cur = text->_priv->index;
		g_signal_emit (G_OBJECT (text),	gnome_canvas_pango_signals [SEL_CHANGED], 0, &bounds);
		break;
	}
	case GDK_k:
		if (event->state & GDK_CONTROL_MASK) {
			/* delete from cursor to end of paragraph */
			/* TODO: write this code */
		}
		break;
	case GDK_u:
		if (event->state & GDK_CONTROL_MASK) {
			/* delete whole paragraph */
			/* TODO: write this code */
		}
		break;
	case GDK_backslash:
		if (event->state & GDK_MOD1_MASK) {
			/* delete all white spaces around the cursor */
			/* TODO: write this code */
		}
		break;
	default:
		break;
	}

	gnome_canvas_pango_update_line_pos (text);
	if (!(event->state & GDK_SHIFT_MASK))
		text->_priv->start_sel = text->_priv->index;

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (text));
	
	return TRUE;
} /* gnome_canvas_rich_text_ext_key_press_event */

static gint
gnome_canvas_pango_event (GnomeCanvasItem *item, 
					 GdkEvent *event)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);
	int x, y;

	if (get_event_coordinates (event, &x, &y)) {

		x -= text->_priv->x;
		y -= text->_priv->y;

/*		emit_event_on_tags(text, event, &iter);*/
	}
	else if (event->type == GDK_KEY_PRESS ||
		 event->type == GDK_KEY_RELEASE) {
/*		GtkTextMark *insert;
		GtkTextIter iter;

		insert = gtk_text_buffer_get_mark(get_buffer(text), "insert");
		gtk_text_buffer_get_iter_at_mark(
			get_buffer(text), &iter, insert);
		emit_event_on_tags(text, event, &iter);*/
	}

	switch (event->type) {
	case GDK_KEY_PRESS:
		return gnome_canvas_pango_key_press_event(
			item, (GdkEventKey *) event);
	case GDK_KEY_RELEASE:
/*		return gnome_canvas_rich_text_ext_key_release_event(
			item, (GdkEventKey *) event);*/
		return FALSE;
	case GDK_BUTTON_PRESS:
		return gnome_canvas_pango_button_press_event(
			item, (GdkEventButton *) event);
	case GDK_BUTTON_RELEASE:
		return gnome_canvas_pango_button_release_event(
			item, (GdkEventButton *) event);
	case GDK_MOTION_NOTIFY:
		if (!text->_priv->clicks)
			return FALSE;
		return gnome_canvas_pango_drag_event (
			item, (GdkEventButton *) event);
	case GDK_FOCUS_CHANGE:
/*		if (((GdkEventFocus *) event)->window !=
		    item->canvas->layout.bin_window)
			return FALSE;

		if (((GdkEventFocus *) event)->in)
			return gnome_canvas_rich_text_ext_focus_in_event(
				item, (GdkEventFocus *) event);
		else
			return gnome_canvas_rich_text_ext_focus_out_event(
				item, (GdkEventFocus *) event);*/
	default:
		break;
	}
	return FALSE;
}

static void
gnome_canvas_pango_get_bounds (GnomeCanvasItem *item, double *px1, double *py1,
	   double *px2, double *py2)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (item);

	adjust_for_anchors(text, px1, py1);

	*px2 = *px1 + ((text->_priv->width > 0)? text->_priv->width: text->_priv->_width);
	*py2 = *py1 + ((text->_priv->height > 0)? text->_priv->height: text->_priv->_height);
}

PangoLayout*
gnome_canvas_pango_get_layout (GnomeCanvasPango *text)
{
	g_return_val_if_fail (GNOME_IS_CANVAS_PANGO (text), NULL);
	return text->_priv->layout;
}

void
gnome_canvas_pango_set_layout (GnomeCanvasPango *text, PangoLayout* layout)
{
	g_return_if_fail (GNOME_IS_CANVAS_PANGO (text));
	if (text->_priv->layout)
		g_object_unref (text->_priv->layout);
	text->_priv->layout = layout;
}

struct MergeStruct {
	PangoAttrList *l;
	unsigned start, end;
};

static gboolean
merge_cb (PangoAttribute *attribute, gpointer data)
{
	struct MergeStruct *s = data;
	PangoAttribute *attr = pango_attribute_copy (attribute);
	attr->start_index = s->start;
	attr->end_index = s->end;
	pango_attr_list_change (s->l, attr);
	return FALSE;
}

void
gnome_canvas_pango_set_insert_attrs (GnomeCanvasPango *text,
	PangoAttrList *attr_list)
{
	g_return_if_fail (GNOME_IS_CANVAS_PANGO (text));
	if (text->_priv->insert_attrs)
		pango_attr_list_unref (text->_priv->insert_attrs);
	text->_priv->insert_attrs = attr_list;
}

void
gnome_canvas_pango_apply_attrs_to_selection (GnomeCanvasPango *text,
	PangoAttrList *attr_list)
{
	g_return_if_fail (GNOME_IS_CANVAS_PANGO (text));
	if (text->_priv->index != text->_priv->start_sel) {
		struct MergeStruct s;
		if (text->_priv->index < text->_priv->start_sel) {
			s.start = text->_priv->index;
			s.end = text->_priv->start_sel;
		} else {
			s.start = text->_priv->start_sel;
			s.end = text->_priv->index;
		}
		s.l = pango_layout_get_attributes (text->_priv->layout);
		if (s.l)
			pango_attr_list_filter (attr_list, merge_cb, &s);
	}
}

extern void pango_layout_to_svg (PangoLayout* layout, xmlDocPtr doc, xmlNodePtr node, double x, double y);

static void
gnome_canvas_pango_export_svg (GPrintable *gprintable, xmlDocPtr doc, xmlNodePtr node)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (gprintable);
	double ax, ay;
	g_return_if_fail (text);
	adjust_for_anchors (text, &ax, &ay);
	pango_layout_to_svg (text->_priv->layout, doc, node, ax, ay);
}

static void
gnome_canvas_pango_draw_cairo (GPrintable *gprintable, cairo_t *cr)
{
	GnomeCanvasPango *text = GNOME_CANVAS_PANGO (gprintable);
	double ax, ay;
	g_return_if_fail (text);
	adjust_for_anchors (text, &ax, &ay);
	cairo_translate (cr, ax, ay);
	pango_cairo_show_layout (cr, text->_priv->layout);
}

int gnome_canvas_pango_get_cur_index (GnomeCanvasPango *text)
{
	g_return_val_if_fail (GNOME_IS_CANVAS_PANGO (text), -1);
	return text->_priv->index;
}

int gnome_canvas_pango_get_selection_start (GnomeCanvasPango *text)
{
	g_return_val_if_fail (GNOME_IS_CANVAS_PANGO (text), -1);
	return text->_priv->start_sel;
}

void gcp_pango_layout_replace_text (PangoLayout *layout, unsigned start, unsigned length, char const *new_str, PangoAttrList *l)
{
	GString *string = g_string_new (pango_layout_get_text (layout));
	int len = strlen (new_str);
	if (length > 0) {
		g_string_erase (string, start, length);
		delete_text (pango_layout_get_attributes (layout), start, length);
	}
	if (len > 0) {
		g_string_insert (string, start, new_str);
		pango_attr_list_filter (l, cb_set_attr_list_len, GINT_TO_POINTER (len));
	
		gnome_canvas_pango_attr_list_splice (pango_layout_get_attributes (layout),
							l, start, len);
	}
	pango_layout_set_text (layout, string->str, -1);
	g_string_free (string, TRUE);
}

void gnome_canvas_pango_set_selection_bounds (GnomeCanvasPango *text, unsigned start, unsigned end)
{
	text->_priv->start_sel = start;
	text->_priv->index = end;
	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (text));
}
