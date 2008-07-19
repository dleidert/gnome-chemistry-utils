/* -*- Mode: C; tab-width: 8; indent-tabs-mode: 8; c-basic-offset: 8 -*- */
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
/*
 * GnomeCanvas widget - Tk-like canvas widget for Gnome
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@gimp.org>
 */

#include "config.h"
#include "gcp-canvas-group.h"
#include "gprintable.h"
#include <string.h>

static void gnome_canvas_group_ext_class_init   (GnomeCanvasGroupExtClass *class);
static void gnome_canvas_group_ext_init         (GnomeCanvasGroupExt     *group_ext);
static void gnome_canvas_group_ext_export_svg  (GPrintable *gprintable, xmlDocPtr doc, xmlNodePtr node);
static void gnome_canvas_group_ext_draw_cairo  (GPrintable *gprintable, cairo_t *ct);

static void
gnome_canvas_group_print_init (GPrintableIface *iface)
{
	iface->export_svg = gnome_canvas_group_ext_export_svg;
	iface->draw_cairo = gnome_canvas_group_ext_draw_cairo;
}

GType
gnome_canvas_group_ext_get_type (void)
{
	static GType group_ext_type;

	if (!group_ext_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasGroupExtClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_group_ext_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasGroupExt),
			0,			/* n_preallocs */
			(GInstanceInitFunc) gnome_canvas_group_ext_init,
			NULL			/* value_table */
		};

		static const GInterfaceInfo print_info = {
			(GInterfaceInitFunc) gnome_canvas_group_print_init,
			NULL, NULL
		};

		group_ext_type = g_type_register_static (GNOME_TYPE_CANVAS_GROUP, "GnomeCanvasGroupExt",
						    &object_info, 0);

		g_type_add_interface_static (group_ext_type, G_TYPE_PRINTABLE, &print_info);
	}

	return group_ext_type;
}

/* Bounds handler for canvas groups */
static void
gnome_canvas_group_ext_bounds (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GnomeCanvasGroup *group;
	GnomeCanvasItem *child;
	GList *list;
	double tx1, ty1, tx2, ty2;
	double minx, miny, maxx, maxy;
	int set;

	group = GNOME_CANVAS_GROUP (item);

	/* Get the bounds of the first visible item */

	child = NULL; /* Unnecessary but eliminates a warning. */

	set = FALSE;

	for (list = group->item_list; list; list = list->next) {
		child = list->data;

		if (child->object.flags & GNOME_CANVAS_ITEM_VISIBLE) {
			set = TRUE;
			gnome_canvas_item_get_bounds (child, &minx, &miny, &maxx, &maxy);
			break;
		}
	}


	if (!set) {
		*x1 = *y1 = G_MAXDOUBLE;
		*x2 = *y2 = -G_MAXDOUBLE;
		return;
	}

	/* Now we can grow the bounds using the rest of the items */

	list = list->next;

	for (; list; list = list->next) {
		child = list->data;

		if (!(child->object.flags & GNOME_CANVAS_ITEM_VISIBLE))
			continue;

		gnome_canvas_item_get_bounds (child, &tx1, &ty1, &tx2, &ty2);

		if (tx1 < minx)
			minx = tx1;

		if (ty1 < miny)
			miny = ty1;

		if (tx2 > maxx)
			maxx = tx2;

		if (ty2 > maxy)
			maxy = ty2;
	}

	*x1 = minx;
	*y1 = miny;
	*x2 = maxx;
	*y2 = maxy;
}

static void
gnome_canvas_group_ext_class_init (GnomeCanvasGroupExtClass *klass)
{
	GnomeCanvasItemClass *item_class = (GnomeCanvasItemClass *) klass;
	item_class->bounds = gnome_canvas_group_ext_bounds;
}

static void
gnome_canvas_group_ext_init (GnomeCanvasGroupExt *group_ext)
{
}

static void
gnome_canvas_group_ext_export_svg (GPrintable *printable, xmlDocPtr doc, xmlNodePtr node)
{
	GList *list;
	double affine[6];
	GnomeCanvasItem *item;
	g_return_if_fail (GNOME_IS_CANVAS_GROUP_EXT (printable));
	for (list = GNOME_CANVAS_GROUP (printable) ->item_list; list; list = list->next) {
		item = GNOME_CANVAS_ITEM (list->data);
		if (!(item->object.flags & GNOME_CANVAS_ITEM_VISIBLE))
			continue;
		if (GNOME_IS_CANVAS_GROUP_EXT(item))
			gnome_canvas_group_ext_export_svg (G_PRINTABLE (item), doc, node);
		else if (G_IS_PRINTABLE (item))
		{
			char *buf;
			gnome_canvas_item_i2w_affine (item, affine);
			buf = g_strdup_printf ("matrix(%g,%g,%g,%g,%g,%g)",
									affine[0],
									affine[1],
									affine[2],
									affine[3],
									affine[4],
									affine[5]);
			if (strcmp (buf,"matrix(1,0,0,1,0,0)")) { 
				xmlNodePtr child = xmlNewDocNode (doc, NULL, (const xmlChar*) "g", NULL);
				xmlAddChild (node, child);
				xmlNewProp (child, (const xmlChar*) "transform", (const xmlChar*) buf);
				g_free (buf);
				g_printable_export_svg (G_PRINTABLE (item), doc, child);
			} else
				g_printable_export_svg (G_PRINTABLE (item), doc, node);
		}
	}
}

void gnome_canvas_group_ext_draw_cairo (GPrintable *printable, cairo_t *cr)
{
	GList *list;
	double affine[6];
	GnomeCanvasItem *item;
	g_return_if_fail (GNOME_IS_CANVAS_GROUP_EXT (printable));
	for (list = GNOME_CANVAS_GROUP (printable) ->item_list; list; list = list->next) {
		item = GNOME_CANVAS_ITEM (list->data);
		if (!(item->object.flags & GNOME_CANVAS_ITEM_VISIBLE))
			continue;
		if (GNOME_IS_CANVAS_GROUP_EXT(item))
			gnome_canvas_group_ext_draw_cairo (G_PRINTABLE (item), cr);
		else if (G_IS_PRINTABLE (item))
		{
			gnome_canvas_item_i2w_affine (item, affine);
			cairo_save (cr);
			cairo_transform (cr ,(cairo_matrix_t*) affine);
			g_printable_draw_cairo (G_PRINTABLE (item), cr); 
			cairo_restore (cr);
		}
	}
}
