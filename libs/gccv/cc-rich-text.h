/* ccc/cc-rich-text-.h
 *
 * Copyright (c) 2005-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef CC_RICH_TEXT_H
#define CC_RICH_TEXT_H

#include <ccc/cc-shape.h>
#include <pango/pango-layout.h>

G_BEGIN_DECLS

typedef struct _CcRichText      CcRichText;
typedef struct _CcRichTextClass CcRichTextClass;

#define CC_TYPE_RICH_TEXT         (cc_rich_text_get_type())
#define CC_RICH_TEXT(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_RICH_TEXT, CcRichText))
#define CC_RICH_TEXT_CLASS(c)     (G_TYPE_ECHCK_CLASS_CAST((c), CC_TYPE_RICH_TEXT, CcRichTextClass))
#define CC_IS_RICH_TEXT(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_RICH_TEXT))
#define CC_IS_RICH_TEXT_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_RICH_TEXT))
#define CC_RICH_TEXT_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_RICH_TEXT, CcRichTextClass))

struct _CcRichText {
	CcShape        base_instance;

	PangoLayout *layout;
	/* Attributes to apply to inserted text */
	PangoAttrList *insert_attrs;
	/* Position at anchor */
	gdouble x, y;
	/* Dimensions */
	gdouble width, height; /* user set size */
	gdouble _width, _height; /* size of the pango layout */
	GtkAnchorType anchor;
	guint rgba;
	gchar *color_name;
	gboolean editing, cursor_visible;
	guint blink_timeout, clicks;
	/* current position and selection bounds */
	gint index, start_sel, xl;
	/* Current line */
	gint line;
	CcHashMap    * im_contexts;
};

struct CcRichTextBounds {
	gint start, cur;
};


struct _CcRichTextClass {
	CcShapeClass base_class;

	/* vtable */
	void (*changed) (CcRichText     * self);
	void (*sel_changed) (CcRichText     * self,
			struct CcRichTextBounds *bounds);
};

GType cc_rich_text_get_type(void) G_GNUC_CONST;

void cc_rich_text_set_anchor(CcRichText *self, gdouble x, gdouble y);
void cc_rich_text_set_anchor_type (CcRichText *self, GtkAnchorType anchor);
PangoLayout* cc_rich_text_get_layout (CcRichText *text);
void cc_rich_text_set_layout (CcRichText *text, PangoLayout *layout);
void cc_rich_text_set_insert_attrs (CcRichText *text, PangoAttrList *attr_list);
void cc_rich_text_apply_attrs_to_selection (CcRichText *text, PangoAttrList *attr_list);
int cc_rich_text_get_cur_index (CcRichText *text);
int cc_rich_text_get_selection_start (CcRichText *text);
void cc_rich_text_set_selection_bounds (CcRichText *text, unsigned start, unsigned end);
void pango_layout_replace_text (PangoLayout *layout, unsigned start, unsigned length, char const *new_str, PangoAttrList *l);

G_END_DECLS

#endif /* CC_RICH_TEXT_H */
