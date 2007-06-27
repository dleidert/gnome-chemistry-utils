/* 
 * gnome-print-pango.c
 *
 * Copyright (C) 2003-2004 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#define GTK_TEXT_USE_INTERNAL_UNSUPPORTED_API
#include <gtk/gtktextlayout.h>
#include "gnome-print-pango.h"
#include <string.h>

#ifdef GP_HAS_PANGO
#	include <libgnomeprint/gnome-print-pango.h>
#	define TEXT_ZOOM_FACTOR	(4. / 3.)

void pango_layout_print (GnomePrintContext *gpc, PangoLayout* pl)
{
	PangoAttrList *attrs = pango_layout_get_attributes (pl);
	PangoLayout *layout = (PangoLayout*) gnome_print_pango_create_layout (gpc);
	const char *text = pango_layout_get_text (pl);
	int w1, w2, h1, h2, l;
	double zoom, extra_space;
	PangoFontDescription const *desc = pango_layout_get_font_description (pl);
	PangoAttribute *pa;
	PangoAttrList *pal = (attrs)? pango_attr_list_copy (attrs): pango_attr_list_new ();
	l = strlen (text);
	pango_layout_set_text (layout, text, l);
	if (desc)
		pango_layout_set_font_description (layout, desc);
	pango_layout_set_attributes (layout, pal);
	pango_layout_get_size (pl, &w1, &h1);
	pango_layout_get_size (layout, &w2, &h2);
	zoom = (double) h1 / (double) h2;
	extra_space = w1 / zoom - w2;
	if (l > 1) {
		pa = pango_attr_letter_spacing_new ((int) extra_space / l);
		pa->start_index = 0;
		pa->end_index = l;
		pango_attr_list_insert (pal, pa);
		pango_layout_set_attributes (layout, pal);
	}
	pango_attr_list_unref (pal);
	gnome_print_gsave (gpc);
	gnome_print_scale (gpc, zoom, -zoom); /* not so good. Why is zoom necessary to get the same size ? Have a look at resolution */
	gnome_print_moveto (gpc, 0., 0.);
	gnome_print_pango_layout (gpc, layout);
	gnome_print_grestore (gpc);
	g_object_unref (layout);
}

#else

void gpc_print_pango_layout_print (GnomePrintContext *gpc, PangoLayout* pl) {
	gint i, top, bottom;
	GnomeFont *font;
	GnomeFontFace *face;
	GnomeGlyphList *glyph_list;
	GSList *extra_attrs_list;
	PangoFontDescription *desc;
	PangoGlyphItem *item;
	PangoRectangle logical_rect, ink_rect, rect;
	PangoLayout *layout;
	ArtDRect art_rect;
	const char *text = pango_layout_get_text (pl);
	gboolean foreground_set;
	guint16 foreground_red = 0, foreground_green = 0, foreground_blue = 0;
	gboolean background_set;
	guint16 background_red = 0, background_green = 0, background_blue = 0;
	PangoLayoutIter *iter = pango_layout_get_iter (pl);
	gdouble scale[6] = {1., 0., 0., 1., 0., 0.};
	gdouble space;
	gboolean strikethrough, underline, slant;
	GtkTextAppearance* appearance;
	int underline_type = PANGO_UNDERLINE_NONE;
	PangoAlignment align = pango_layout_get_alignment (pl);
	double zoom;
	
	pango_layout_get_extents (pl, NULL, &logical_rect);
	gnome_print_gsave (gpc);
	switch (align) {
	case PANGO_ALIGN_CENTER:
		gnome_print_translate (gpc, - (double) logical_rect.width / 2., 0.);
		break;
	case PANGO_ALIGN_RIGHT:
		gnome_print_translate (gpc, - (double) logical_rect.width, 0.);
		break;
	default:
		break;
	}
	do {
		item = pango_layout_iter_get_run (iter);
		if (!item) break;
		strikethrough = underline = foreground_set = background_set = FALSE;
		appearance = NULL;
		desc = pango_font_describe (item->item->analysis.font);
		face = gnome_font_face_find_closest_from_weight_slant (
								(unsigned char const*) pango_font_description_get_family (desc),
								(GnomeFontWeight) pango_font_description_get_weight (desc),
								(pango_font_description_get_style (desc) != PANGO_STYLE_NORMAL));
		font =	gnome_font_face_get_font (face,
					  pango_font_description_get_size(desc) / PANGO_SCALE, 72, 72);
		if (font == NULL)
		{
			g_warning ("No font file for font.");
			continue;
		}
		glyph_list = gnome_glyphlist_new ();
		extra_attrs_list = item->item->analysis.extra_attrs;
		top = pango_layout_iter_get_baseline(iter) / PANGO_SCALE;
		pango_layout_iter_get_char_extents (iter, &logical_rect);
		bottom = 0;
		while (extra_attrs_list)
		{
			PangoAttribute *attr = extra_attrs_list->data;
			PangoAttrType attr_type = attr->klass->type;
			switch (attr_type) {
			case PANGO_ATTR_STYLE:
				g_warning("style");
				break;
			case PANGO_ATTR_RISE:
				bottom = ((PangoAttrInt *) attr)->value / PANGO_SCALE;
				break;
			case PANGO_ATTR_FOREGROUND:
				foreground_set = TRUE;
				foreground_red = ((PangoAttrColor *) attr)->color.red;
				foreground_green = ((PangoAttrColor *) attr)->color.green;
				foreground_blue = ((PangoAttrColor *) attr)->color.blue;
				break;
			case PANGO_ATTR_BACKGROUND:
				background_set = TRUE;
				background_red = ((PangoAttrColor *) attr)->color.red;
				background_green = ((PangoAttrColor *) attr)->color.green;
				background_blue = ((PangoAttrColor *) attr)->color.blue;
				break;
			case PANGO_ATTR_UNDERLINE:
				underline_type = ((PangoAttrInt *) attr)->value;
				underline = TRUE;
				break;
			case PANGO_ATTR_STRIKETHROUGH:
				strikethrough = TRUE;
				break;
			case PANGO_ATTR_SHAPE:
				g_warning ("Pango attribute PANGO_ATTR_SHAPE not supported");
				break;
			case PANGO_ATTR_SCALE:
				g_warning ("Pango attribute PANGO_ATTR_SCALE not supported");
				break;
			default:
				if (attr_type == gtk_text_attr_appearance_type)
					appearance = &((GtkTextAttrAppearance *)attr)->appearance;
				break;
			}
			extra_attrs_list = extra_attrs_list->next;
		}
		pango_layout_iter_get_run_extents (iter, &ink_rect, &logical_rect);
		layout = pango_layout_new (pango_layout_get_context (pl));
		pango_layout_set_font_description (layout, desc);
		pango_layout_set_text (layout, text + item->item->offset, item->item->length);
		pango_layout_get_extents (layout, &rect, NULL);
		g_object_unref (layout);
		if (background_set)
		{
			gnome_print_setrgbcolor (gpc, (float) background_red / 0xFFFF,
						(float) background_green / 0xFFFF, (float) background_blue / 0xFFFF);
			gnome_print_rect_filled (gpc, logical_rect.x / PANGO_SCALE,
						logical_rect.y / PANGO_SCALE,
						(float) logical_rect.width / PANGO_SCALE,
						(float) + logical_rect.height / PANGO_SCALE);
		}
		gnome_print_setrgbcolor (gpc, (float) foreground_red / 0xFFFF,
						(float) foreground_green / 0xFFFF, (float) foreground_blue / 0xFFFF);
		if (strikethrough || (appearance && appearance->strikethrough)) {
			gnome_print_setlinewidth (gpc, .75);
			gnome_print_setlinecap (gpc, 0);
			gnome_print_moveto (gpc, logical_rect.x / PANGO_SCALE,
					 - bottom + top - (0.22 * logical_rect.height) / PANGO_SCALE);
			gnome_print_lineto (gpc, (logical_rect.x + logical_rect.width) / PANGO_SCALE,
					 - bottom + top - (0.22 * logical_rect.height) / PANGO_SCALE);
			gnome_print_stroke (gpc);
		}
		if (underline) {
			gnome_print_setlinewidth (gpc, .75);
			gnome_print_setlinecap (gpc, 0);
			switch (underline_type) {
			case PANGO_UNDERLINE_NONE:
				break;
			case PANGO_UNDERLINE_DOUBLE:
				gnome_print_moveto (gpc, logical_rect.x / PANGO_SCALE,
						 - bottom + top + 3.0);
				gnome_print_lineto (gpc, (logical_rect.x + logical_rect.width) / PANGO_SCALE,
						 - bottom + top + 3.0);
				gnome_print_stroke (gpc);
				  /* Fall through */
			case PANGO_UNDERLINE_SINGLE:
				gnome_print_moveto (gpc, logical_rect.x / PANGO_SCALE,
						 - bottom + top + 1.0);
				gnome_print_lineto (gpc, (logical_rect.x + logical_rect.width) / PANGO_SCALE,
						 - bottom + top + 1.0);
				gnome_print_stroke (gpc);
				break;
			case PANGO_UNDERLINE_LOW:
				gnome_print_moveto (gpc, logical_rect.x / PANGO_SCALE,
						 - bottom + top + logical_rect.y / PANGO_SCALE + 1.0);
				gnome_print_lineto (gpc, (logical_rect.x + logical_rect.width) / PANGO_SCALE,
						 - bottom + top + logical_rect.y / PANGO_SCALE + 1.0);
				gnome_print_stroke (gpc);
				break;
			}
		}
		gnome_print_moveto (gpc, logical_rect.x / PANGO_SCALE,
				 - bottom + top);
		gnome_glyphlist_font (glyph_list, font);
		for (i = 0; i < item->glyphs->num_glyphs; i++)
		{
			gnome_glyphlist_glyph (glyph_list, item->glyphs->glyphs[i].glyph);
		}
		gnome_glyphlist_bbox (glyph_list, scale, 0, &art_rect);
		slant = (pango_font_description_get_style (desc) != PANGO_STYLE_NORMAL) && !gnome_font_face_is_italic (face);
		zoom = ((double) ink_rect.height) / PANGO_SCALE / (art_rect.y1 - art_rect.y0);
		if (item->glyphs->num_glyphs > 1) {
			space = (((double) ink_rect.width / PANGO_SCALE / zoom - art_rect.x1 + art_rect.x0) - ((slant)? top * 0.15 / zoom: 0.))
						/ item->glyphs->num_glyphs;
			gnome_glyphlist_unref (glyph_list);
			glyph_list = gnome_glyphlist_new ();
			gnome_print_moveto (gpc, logical_rect.x / PANGO_SCALE,
					 - bottom + top);
			gnome_glyphlist_font (glyph_list, font);
			gnome_glyphlist_letterspace (glyph_list, space);
			for (i = 0; i < item->glyphs->num_glyphs; i++)
			{
				gnome_glyphlist_glyph (glyph_list, item->glyphs->glyphs[i].glyph);
			}
		}
		gnome_print_gsave(gpc);
		if (slant) {
		/*simulate slanted font*/
			double matrix [6] = {
				 zoom,
				0., .15 * zoom, -zoom, .0, .0};
			gnome_print_concat (gpc, matrix);
		} else
			gnome_print_scale(gpc, zoom, -zoom);
		gnome_print_glyphlist (gpc, glyph_list);
		gnome_print_grestore(gpc);
		gnome_glyphlist_unref (glyph_list);
	} while (pango_layout_iter_next_run (iter));
	pango_layout_iter_free (iter);
	gnome_print_grestore (gpc);
}
#endif
