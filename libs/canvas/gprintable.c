/* 
 * gprintable.c 
 *
 * Copyright (C) 2003 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gprintable.h"

GType
g_printable_get_type (void)
{
	static GType g_printable_type = 0;

	if (!g_printable_type) {
		static const GTypeInfo g_printable_info = {
	  		sizeof (GPrintableIface),
	  		NULL, NULL, NULL, NULL, NULL, 0, 0, NULL
		};
		g_printable_type = g_type_register_static (G_TYPE_INTERFACE, "GPrintable", &g_printable_info, 0);
	}

	return g_printable_type;
}

void
g_printable_print (GPrintable *gprintable, GnomePrintContext *pc)
{
	GPrintableIface *iface;

	g_return_if_fail (G_IS_PRINTABLE (gprintable));
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (pc));

	iface = G_PRINTABLE_GET_IFACE (gprintable);

	if (iface->print)
		iface->print (gprintable, pc);
}

void
g_printable_export_svg (GPrintable *gprintable, xmlDocPtr doc, xmlNodePtr node)
{
	GPrintableIface *iface;

	g_return_if_fail (G_IS_PRINTABLE (gprintable));
	g_return_if_fail (doc != NULL);
	g_return_if_fail (node != NULL);

	iface = G_PRINTABLE_GET_IFACE (gprintable);

	if (iface->export_svg)
		iface->export_svg (gprintable, doc, node);
}

void
g_printable_draw_cairo (GPrintable *gprintable, cairo_t *cr)
{
	GPrintableIface *iface;

	g_return_if_fail (G_IS_PRINTABLE (gprintable));
	g_return_if_fail (cr != NULL);

	iface = G_PRINTABLE_GET_IFACE (gprintable);

	if (iface->draw_cairo)
		iface->draw_cairo (gprintable, cr);
}
