/* 
 * gnome-print-pango.h
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

#ifndef GCP_PRINT_PANGO_H
#define GCP_PRINT_PANGO_H

#include <libgnomeprint/gnome-print.h>
#include <pango/pango.h>

G_BEGIN_DECLS

#ifdef GP_HAS_PANGO
void pango_layout_print (GnomePrintContext *gpc, PangoLayout* pl);
#else
void gpc_print_pango_layout_print (GnomePrintContext *gpc, PangoLayout* pl);
#endif

G_END_DECLS

#endif
