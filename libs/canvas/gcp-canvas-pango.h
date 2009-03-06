/* libgcpcanvas/gcp-canvas-pango.h
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
 * Boston, MA 02110-1301, USA.
 */

#ifndef GNOME_CANVAS_PANGO_H
#define GNOME_CANVAS_PANGO_H

#include <libgnomecanvas/gnome-canvas.h>

G_BEGIN_DECLS

#define GNOME_TYPE_CANVAS_PANGO             (gnome_canvas_pango_get_type ())
#define GNOME_CANVAS_PANGO(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_TYPE_CANVAS_PANGO, GnomeCanvasPango))
#define GNOME_CANVAS_PANGO_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_PANGO, GnomeCanvasPangoClass))
#define GNOME_IS_CANVAS_PANGO(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_TYPE_CANVAS_PANGO))
#define GNOME_IS_CANVAS_PANGO_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_PANGO))
#define GNOME_CANVAS_PANGO_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GNOME_TYPE_CANVAS_PANGO, GnomeCanvasPangoClass))

typedef struct _GnomeCanvasPango             GnomeCanvasPango;
typedef struct _GnomeCanvasPangoPrivate      GnomeCanvasPangoPrivate;
typedef struct _GnomeCanvasPangoClass        GnomeCanvasPangoClass;

struct _GnomeCanvasPango {
	GnomeCanvasItem item;

    GnomeCanvasPangoPrivate *_priv;
};

struct GnomeCanvasPangoSelBounds {
	int start, cur;
};

struct _GnomeCanvasPangoClass {
	GnomeCanvasItemClass parent_class;

	/* signals */
	void (*changed) (GnomeCanvasPango *text);
	void (*sel_changed) (GnomeCanvasPango *text, struct GnomeCanvasPangoSelBounds *bounds);
};

GType gnome_canvas_pango_get_type(void) G_GNUC_CONST;

PangoLayout* gnome_canvas_pango_get_layout (GnomeCanvasPango *text);
void gnome_canvas_pango_set_layout (GnomeCanvasPango *text, PangoLayout *layout);
void gnome_canvas_pango_set_insert_attrs (GnomeCanvasPango *text, PangoAttrList *attr_list);
void gnome_canvas_pango_apply_attrs_to_selection (GnomeCanvasPango *text, PangoAttrList *attr_list);
int gnome_canvas_pango_get_cur_index (GnomeCanvasPango *text);
int gnome_canvas_pango_get_selection_start (GnomeCanvasPango *text);
void gnome_canvas_pango_set_selection_bounds (GnomeCanvasPango *text, unsigned start, unsigned end);
void gcp_pango_layout_replace_text (PangoLayout *layout, unsigned start, unsigned length, char const *new_str, PangoAttrList *l);

G_END_DECLS

#endif /* GNOME_CANVAS_PANGO_H */
