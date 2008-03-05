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
static void gnome_canvas_group_ext_print       (GPrintable *gprintable, GnomePrintContext *pc);
static void gnome_canvas_group_ext_export_svg  (GPrintable *gprintable, xmlDocPtr doc, xmlNodePtr node);
static void gnome_canvas_group_ext_draw_cairo  (GPrintable *gprintable, cairo_t *ct);

static void
gnome_canvas_group_print_init (GPrintableIface *iface)
{
	iface->print = gnome_canvas_group_ext_print;
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

		group_ext_type = g_type_register_static (GNOME_TYPE_CANVAS_GROUP_EXT, "GnomeCanvasGroupExt",
						    &object_info, 0);

		g_type_add_interface_static (group_ext_type, G_TYPE_PRINTABLE, &print_info);
	}

	return group_ext_type;
}

static void
gnome_canvas_group_ext_class_init (GnomeCanvasGroupExtClass *class)
{
}

static void
gnome_canvas_group_ext_init (GnomeCanvasGroupExt *group_ext)
{
}

void gnome_canvas_group_ext_print (GPrintable *printable, GnomePrintContext *pc)
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
			gnome_canvas_group_ext_print (G_PRINTABLE (item), pc);
		else if (G_IS_PRINTABLE (item))
		{
			gnome_canvas_item_i2w_affine (item, affine);
			gnome_print_gsave(pc);
			gnome_print_concat(pc, affine);
			g_printable_print (G_PRINTABLE (item), pc); 
			gnome_print_grestore(pc);
		}
	}
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