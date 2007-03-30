/* 
 * pango-layout-to-svg.c 
 *
 * Copyright (C) 2004-2005 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include <pango/pango-layout.h>
#include <libxml/tree.h>
#include <math.h>

void pango_layout_to_svg (PangoLayout* layout, xmlDocPtr doc, xmlNodePtr node, double x, double y)
{
	xmlNodePtr child, run;
	char *buf;
	int i, k;
	GSList *extra_attrs_list;
	PangoFontDescription *desc;
	PangoGlyphItem *item;
	const char *text = pango_layout_get_text (layout);
	GString *str;
	PangoLayoutIter *iter = pango_layout_get_iter (layout);
	/*PangoAlignment align = pango_layout_get_alignment (layout);*/

	if (*text == 0)
		return;

	do {
		child = xmlNewDocNode (doc, NULL, (const xmlChar*) "text", NULL);
		xmlAddChild (node, child);
		buf = g_strdup_printf ("%g", x);
		xmlNewProp (child, (const xmlChar*) "x", (const xmlChar*) buf);
		g_free (buf);
		buf = g_strdup_printf ("%g", y + pango_layout_iter_get_baseline (iter) / PANGO_SCALE);
		xmlNewProp (child, (const xmlChar*) "y", (const xmlChar*) buf);
		g_free (buf);
	
		do {
			item = pango_layout_iter_get_run (iter);
			if (!item) break;
			desc = pango_font_describe (item->item->analysis.font);
			str = g_string_new ("");
			for (i = 0; i < item->glyphs->num_glyphs; i++)
			{
				k = g_utf8_get_char (text);
				if (k < 128)
					g_string_append_printf (str, "%c", k);
				else
					g_string_append_printf (str, "&#x%x;", k);
				text = g_utf8_next_char (text);
			}
			run = xmlNewDocNode (doc, NULL, (const xmlChar*) "tspan", (const xmlChar*) str->str);
			g_string_free (str, TRUE);
			xmlAddChild (child, run);
			xmlNewProp (run, (xmlChar*) "font-family", (xmlChar*) pango_font_description_get_family (desc));
			buf = g_strdup_printf ("%d", (int)(rint (pango_font_description_get_size (desc) / PANGO_SCALE)));
			xmlNewProp (run, (xmlChar*) "font-size", (xmlChar*) buf);
			g_free (buf);
			switch (pango_font_description_get_weight (desc)) {
			case PANGO_WEIGHT_BOLD:
				xmlNewProp (run, (xmlChar*) "font-weight", (xmlChar*) "bold");
				break;
			case PANGO_WEIGHT_NORMAL: break;
			default:
				buf = g_strdup_printf ("%d", pango_font_description_get_weight (desc));
				xmlNewProp (run, (xmlChar*) "font-weight", (xmlChar*) buf);
				g_free (buf);
				break;
			}
			switch (pango_font_description_get_style (desc)) {
			case PANGO_STYLE_ITALIC:
				xmlNewProp (run,(xmlChar*) "font-syle", (xmlChar*) "italic");
				break;
			case PANGO_STYLE_OBLIQUE:
				xmlNewProp (run, (xmlChar*) "font-syle", (xmlChar*) "oblique");
				break;
			default: break;
			}
			extra_attrs_list = item->item->analysis.extra_attrs;
			while (extra_attrs_list)
			{
				gboolean decorated = FALSE; /* only supports underline OR strikethrough
				so  that we export only the first encountered one */
				PangoAttribute *attr = extra_attrs_list->data;
				PangoAttrType attr_type = attr->klass->type;
				switch (attr_type) {
				case PANGO_ATTR_STYLE:
					g_warning("style");
					break;
				case PANGO_ATTR_RISE:
					k = ((PangoAttrInt *) attr)->value / PANGO_SCALE;
					buf = g_strdup_printf ("%d", - k);
					xmlNewProp (run, (xmlChar*) "dy", (xmlChar*) buf);
					g_free (buf);
					break;
				case PANGO_ATTR_FOREGROUND:
					buf = g_strdup_printf ("rgb(%d,%d,%d)",
								((PangoAttrColor *) attr)->color.red / 0xFF,
								((PangoAttrColor *) attr)->color.green / 0xFF,
								((PangoAttrColor *) attr)->color.blue / 0xFF);
					xmlNewProp (run, (xmlChar*) "fill", (xmlChar*) buf);
					g_free (buf);
					break;
				case PANGO_ATTR_UNDERLINE: {
					if (decorated)
						break;
					PangoUnderline u = (PangoUnderline) ((PangoAttrInt*) attr)->value;
					if (u != PANGO_UNDERLINE_NONE) {
						xmlNewProp (run, (xmlChar*) "text-decoration", (xmlChar*) "underline");
						decorated = TRUE;
					}
					break;
				}
				case PANGO_ATTR_STRIKETHROUGH:
					if (!decorated && ((PangoAttrInt*) attr)->value) {
						xmlNewProp (run, (xmlChar*) "text-decoration", (xmlChar*) "line-through");
						decorated = TRUE;
					}
					break;
				case PANGO_ATTR_SHAPE:
					g_warning ("Pango attribute PANGO_ATTR_SHAPE not supported");
					break;
				case PANGO_ATTR_SCALE:
					g_warning ("Pango attribute PANGO_ATTR_SCALE not supported");
					break;
				default:
					break;
				}
				extra_attrs_list = extra_attrs_list->next;
			}
		} while (pango_layout_iter_next_run (iter));
		text = g_utf8_next_char (text);
	} while (pango_layout_iter_next_line (iter));
	pango_layout_iter_free (iter);
}
