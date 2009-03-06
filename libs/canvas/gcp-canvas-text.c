/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
/* Text item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas
 * widget.  Tk is copyrighted by the Regents of the University of California,
 * Sun Microsystems, and other parties.
 *
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 * Port to Pango co-done by Gergõ Érdi <cactus@cactus.rulez.org>
 */

#include "config.h"
#include <math.h>
#include "gcp-canvas-text.h"
#include "gprintable.h"
#include "gnome-print-pango.h"

static void gnome_canvas_text_ext_class_init   (GnomeCanvasTextExtClass *class);
static void gnome_canvas_text_ext_init         (GnomeCanvasTextExt     *text);
static void gnome_canvas_text_ext_print       (GPrintable *gprintable, GnomePrintContext *pc);
static void gnome_canvas_text_ext_export_svg   (GPrintable *gprintable, xmlDocPtr doc, xmlNodePtr node);
static void gnome_canvas_text_ext_ensure_rendered        (GnomeCanvasTextExt     *text);

static void
gnome_canvas_text_print_init (GPrintableIface *iface)
{
	iface->print = gnome_canvas_text_ext_print;
	iface->export_svg = gnome_canvas_text_ext_export_svg;
}

GType
gnome_canvas_text_ext_get_type (void)
{
	static GType text_ext_type;

	if (!text_ext_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasTextExtClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_text_ext_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasTextExt),
			0,			/* n_preallocs */
			(GInstanceInitFunc) gnome_canvas_text_ext_init,
			NULL			/* value_table */
		};

		static const GInterfaceInfo print_info = {
			(GInterfaceInitFunc) gnome_canvas_text_print_init,
			NULL, NULL
		};

		text_ext_type = g_type_register_static (GNOME_TYPE_CANVAS_TEXT_EXT, "GnomeCanvasTextExt",
						    &object_info, 0);

		g_type_add_interface_static (text_ext_type, G_TYPE_PRINTABLE, &print_info);
	}

	return text_ext_type;
}

static void
gnome_canvas_text_ext_class_init (GnomeCanvasTextExtClass *class)
{
}

static void
gnome_canvas_text_ext_init (GnomeCanvasTextExt *text)
{
	text->rendered = FALSE;
}

void gnome_canvas_text_ext_print (GPrintable *printable, GnomePrintContext *pc)
{
	GnomeCanvasText *text;
	GnomeCanvasTextExt *text_ext;
	double x , y;
	g_return_if_fail (GNOME_IS_CANVAS_TEXT_EXT (printable));
	text = GNOME_CANVAS_TEXT (printable);
	text_ext = GNOME_CANVAS_TEXT_EXT (printable);
	if (!text_ext->rendered)
		gnome_canvas_text_ext_ensure_rendered (text_ext);
	gnome_print_gsave (pc);
	x = text->x;
	y = text->y;
	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		x -= text->max_width / 2;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		x -= text->max_width;
		break;

	default:
		break;
	}

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		y -= text->height / 2;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		y -= text->height;
		break;

	default:
		break;
	}
	gnome_print_translate (pc, x, y);
#ifdef GP_HAS_PANGO
	pango_layout_print (pc, text->layout);
#else	
	gpc_print_pango_layout_print (pc, text->layout);
#endif
	gnome_print_grestore (pc);
}

extern void pango_layout_to_svg (PangoLayout* layout, xmlDocPtr doc, xmlNodePtr node, double x, double y);

static void
gnome_canvas_text_ext_export_svg (GPrintable *printable, xmlDocPtr doc, xmlNodePtr node)
{
	GnomeCanvasText *text;
	double x , y;
	GnomeCanvasTextExt *text_ext;

	g_return_if_fail (GNOME_IS_CANVAS_TEXT_EXT (printable));
	text = GNOME_CANVAS_TEXT (printable);
	text_ext = GNOME_CANVAS_TEXT_EXT (printable);
	if (!text_ext->rendered)
		gnome_canvas_text_ext_ensure_rendered (text_ext);
	x = text->x;
	y = text->y;
	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		x -= text->max_width / 2;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		x -= text->max_width;
		break;

	default:
		break;
	}

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		y -= text->height / 2;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		y -= text->height;
		break;

	default:
		break;
	}
	pango_layout_to_svg (text->layout, doc, node, x, y);
}

static void
gnome_canvas_text_ext_ensure_rendered (GnomeCanvasTextExt *text)
{
	double x1, y1, x2, y2;
	int w, h;
	GdkPixbuf *pixbuf;
	GnomeCanvasBuf buf;
	gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (text), &x1, &y1, &x2, &y2);
	w = (int) (ceil (x2) - floor (x1)) + 1, h = (int) (ceil (y2) - floor (y1)) + 1;
	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, w, h);
	buf.buf = gdk_pixbuf_get_pixels (pixbuf);
	buf.rect.x0 = (int) floor (x1);
	buf.rect.x1 = (int) ceil (x2);
	buf.rect.y0 = (int) floor (y1);
	buf.rect.y1 = (int) ceil (y2);
	buf.buf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	buf.bg_color = 0xffffff;
	buf.is_buf = 1;
	(* GNOME_CANVAS_ITEM_GET_CLASS (text)->render) (GNOME_CANVAS_ITEM (text), &buf);
	g_object_unref (pixbuf);
	text->rendered = TRUE;
}
